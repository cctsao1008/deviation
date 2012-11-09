/*
 This project is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Deviation is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Deviation.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "common.h"
#include "pages.h"
#include "gui/gui.h"
#include "config/model.h"

#include "../common/_chantest_page.c"

static u8 _action_cb(u32 button, u8 flags, void *data);
static const char *_channum_cb(guiObject_t *obj, const void *data);
static const char *_title_cb(guiObject_t *obj, const void *data);
static const char *_page_cb(guiObject_t *obj, const void *data);
static void _navigate_items(s8 direction);
#define VIEW_ID 0
static u8 current_page = 0;
static u8 total_items = 3;
static u8 view_height;
static guiObject_t *scroll_bar;

static void _show_bar_page(u8 num_bars)
{
    current_page = 0;
    int i;
    if (num_bars > 18)
        num_bars = 18;
    cp->num_bars = num_bars;
    memset(cp->pctvalue, 0, sizeof(cp->pctvalue));

    GUI_CreateLabelBox(0 , 0, 50, 12, &DEFAULT_FONT, _title_cb, NULL, (void *)NULL);
    struct LabelDesc desc;
    desc.font = DEFAULT_FONT.font;
    desc.outline_color = desc.fill_color = 1;
    desc.style = LABEL_LEFTCENTER;  // bug fix: must initialize to avoid unpredictable drawing
    desc.font_color = 1; // bug fix: if the font_color doesn't assign, the string might not be shown
    GUI_CreateRect(0, ITEM_HEIGHT , LCD_WIDTH, 1, &desc);

    u8 x = 0;
    u8 height = 7;
    u8 width = 59; // better to be even
    u8 page_item_count = 8;
    if (cp->type == MONITOR_RAWINPUT) {
        desc.font = DEFAULT_FONT.font;  // Could be translated to other languages, hence using 12normal
        height = 12;
        page_item_count = 6;
        view_height = 51; // can only show 3 rows: (12 + 5) x 3
    } else {
        desc.font = TINY_FONT.font;  // only digits, can use smaller font to show more channels
        height = 7;
        page_item_count = 8;
        view_height = 48; // can only show 4 rows: (7 + 5) x 4
    }
    u8 space = height + 5;
    s8 y = -space;
    // Create a logical view
    u8 view_origin_absoluteX = 0;
    u8 view_origin_absoluteY = ITEM_HEIGHT + 1;
    GUI_SetupLogicalView(VIEW_ID, 0, 0, LCD_WIDTH, view_height, view_origin_absoluteX, view_origin_absoluteY);

    for(i = 0; i < num_bars; i++) {
        if (i%2 ==0) {
            x = 0;
            y += space;
        } else {
            x = 63;
        }
        GUI_CreateLabelBox(GUI_MapToLogicalView(VIEW_ID,x) , GUI_MapToLogicalView(VIEW_ID, y),
                0, height, &desc, _channum_cb, NULL, (void *)(long)i);
        cp->value[i] = GUI_CreateLabelBox(GUI_MapToLogicalView(VIEW_ID, x + 37), GUI_MapToLogicalView(VIEW_ID, y),
                        23, height, &TINY_FONT, value_cb, NULL, (void *)((long)i));
        cp->bar[i] = GUI_CreateBarGraph(GUI_MapToLogicalView(VIEW_ID, x) , GUI_MapToLogicalView(VIEW_ID, y + height),
                width, 4, -125, 125, TRIM_HORIZONTAL, showchan_cb, (void *)((long)i));
    }

    u8 w = 10;
    GUI_CreateLabelBox(LCD_WIDTH -w, 0, w, 7, &TINY_FONT, _page_cb, NULL, NULL);
    u8 total_items = num_bars/page_item_count;
    if (num_bars%page_item_count!= 0)
        total_items++;
    if (total_items > 1)
        scroll_bar = GUI_CreateScrollbar(LCD_WIDTH - ARROW_WIDTH, view_origin_absoluteY,
                LCD_HEIGHT - view_origin_absoluteY, total_items, NULL, NULL, NULL);
    else
        scroll_bar = NULL;
}

void PAGE_ChantestInit(int page)
{
    (void)channum_cb; // remove compile warning as this method is not used here
    (void)show_button_page; // to remove compile warning as this method is not used in devo10
    (void)okcancel_cb;
    PAGE_SetModal(0);
    PAGE_SetActionCB(_action_cb);
    GUI_RemoveAllObjects();
    cp->return_page = NULL;
    if (page > 0)
        cp->return_val = page;
    if(cp->type == MONITOR_RAWINPUT )
        _show_bar_page(NUM_INPUTS);
    else {
        cp->type =  MONITOR_CHANNELOUTPUT;// cp->type may not be initialized yet, so do it here
        _show_bar_page(Model.num_channels);
    }

}

void PAGE_ChantestModal(void(*return_page)(int page), int page)
{
    cp->type = MONITOR_CHANNELOUTPUT;
    PAGE_ChantestInit(page);
    cp->return_page = return_page;
    cp->return_val = page;
}

static void _navigate_items(s8 direction)
{
    if (scroll_bar == NULL) // no page scroll
        return;
    current_page += direction;
    if (current_page <=0) {
        current_page = 0;
        GUI_SetRelativeOrigin(VIEW_ID, 0, 0);
    }  else {
        if (current_page >= total_items)
            current_page = total_items -1;
        GUI_ScrollLogicalView(VIEW_ID, direction* view_height);
    }
    GUI_SetScrollbar(scroll_bar, current_page);
}

static void _navigate_pages(s8 direction)
{
    if ((direction == -1 && cp->type == MONITOR_RAWINPUT) ||
            (direction == 1 && cp->type == MONITOR_CHANNELOUTPUT)) {
        cp->type = cp->type == MONITOR_RAWINPUT?MONITOR_CHANNELOUTPUT: MONITOR_RAWINPUT;
        PAGE_ChantestInit(0);
    }
}

static u8 _action_cb(u32 button, u8 flags, void *data)
{
    (void)data;
    if (flags & BUTTON_PRESS) {
        if (CHAN_ButtonIsPressed(button, BUT_EXIT)) {
            if (cp->return_val == 2) // indicating this page is entered from calibration page, so back to its parent page
                PAGE_ChangeByName("TxConfig", -1);
            else
                PAGE_ChangeByName("SubMenu", sub_menu_item);
        }  else if (CHAN_ButtonIsPressed(button, BUT_UP)) {
            _navigate_items(-1);
        }  else if (CHAN_ButtonIsPressed(button,BUT_DOWN)) {
            _navigate_items(1);
        } else if (CHAN_ButtonIsPressed(button, BUT_RIGHT)) {
            _navigate_pages(1);
        }  else if (CHAN_ButtonIsPressed(button,BUT_LEFT)) {
            _navigate_pages(-1);
        }
        else {
            // only one callback can handle a button press, so we don't handle BUT_ENTER here, let it handled by press cb
            return 0;
        }
    }
    return 1;
}

static const char *_channum_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    long ch = (long)data;
    if (cp->type == MONITOR_RAWINPUT) {
       INPUT_SourceName(cp->tmpstr, ch+1);
    } else {
       sprintf(cp->tmpstr, "%d", (int)ch+1);
    }
    return cp->tmpstr;
}

static const char *_title_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    (void)data;
    if (cp->type == MONITOR_RAWINPUT) {
        strcpy(cp->tmpstr, (const char *)_tr("Stick input"));
    } else {
        strcpy(cp->tmpstr, (const char *)_tr("Channel output"));
    }
    return cp->tmpstr;
}

static const char *_page_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    (void)data;
    strcpy(cp->tmpstr, (const char *)"->");  //this is actually used as an icon don't translate t
    if (cp->type == MONITOR_RAWINPUT) {
        strcpy(cp->tmpstr, (const char *)"<-");
    }
    return cp->tmpstr;
}
