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

#include "../common/_timer_page.c"

static u8 _action_cb(u32 button, u8 flags, void *data);
static u16 current_selected = 0;

static guiObject_t *getobj_cb(int relrow, int col, void *data)
{
    (void)relrow;
    (void)data;
    col = (3 + col) % 3;
    if (col == 0)
        return (guiObject_t *)&gui->type;
    else if (col == 1)
        return (guiObject_t *)&gui->src;
    else
        return (guiObject_t *)&gui->start;
}

static int row_cb(int absrow, int relrow, int y, void *data)
{
    (void)data;
    (void)relrow;
    u8 space = ITEM_HEIGHT + 1;
    u8 w = 65;
    u8 x = 55;
    //Row 1
    GUI_CreateLabelBox(&gui->name, 0, y,
            0, ITEM_HEIGHT, &DEFAULT_FONT, timer_str_cb, NULL, (void *)(long)absrow);
    GUI_CreateTextSelectPlate(&gui->type, x, y,
            w, ITEM_HEIGHT, &DEFAULT_FONT, toggle_timertype_cb, set_timertype_cb, (void *)(long)absrow);

    //Row 2
    y += space;
    GUI_CreateLabelBox(&gui->switchlbl, 0, y,
            0, ITEM_HEIGHT,&DEFAULT_FONT, NULL, NULL, _tr("Switch:"));
    GUI_CreateTextSelectPlate(&gui->src, x, y,
            w, ITEM_HEIGHT, &DEFAULT_FONT, toggle_source_cb, set_source_cb, (void *)(long)absrow);
    //Row 3
    y += space;
     GUI_CreateLabelBox(&gui->resetlbl, 0, y,
            55, ITEM_HEIGHT,&DEFAULT_FONT, NULL, NULL, _tr("Reset sw:"));
    GUI_CreateTextSelectPlate(&gui->resetsrc, x, y,
            w, ITEM_HEIGHT, &DEFAULT_FONT, toggle_resetsrc_cb, set_resetsrc_cb, (void *)(long)absrow);
    //Row 4
    y += space;
    GUI_CreateLabelBox(&gui->startlbl, 0, y,
            50, // bug fix: label width and height can't be 0, otherwise, the label couldn't be hidden dynamically
            ITEM_HEIGHT, &DEFAULT_FONT, NULL, NULL, _tr("Start:"));
    GUI_CreateTextSelectPlate(&gui->start, x, y,
            w, ITEM_HEIGHT, &DEFAULT_FONT,NULL, set_start_cb, (void *)(long)absrow);

    update_countdown(absrow);
    return 4;
}

static void _show_page()
{
    PAGE_ShowHeader(_tr("Timers")); // using the same name as related menu item to reduce language strings
    PAGE_SetActionCB(_action_cb);

    GUI_CreateScrollable(&gui->scrollable, 0, ITEM_HEIGHT + 1, LCD_WIDTH, LCD_HEIGHT - ITEM_HEIGHT -1,
                     LCD_HEIGHT - ITEM_HEIGHT, NUM_TIMERS, row_cb, getobj_cb, NULL, NULL);
    GUI_SetSelected(GUI_ShowScrollableRowOffset(&gui->scrollable, current_selected));
}

void PAGE_TimerExit()
{
    current_selected = GUI_ScrollableGetObjRowOffset(&gui->scrollable, GUI_GetSelected());
}

static u8 _action_cb(u32 button, u8 flags, void *data)
{
    (void)data;
    if (flags & BUTTON_PRESS || (flags & BUTTON_LONGPRESS)) {
        if (CHAN_ButtonIsPressed(button, BUT_EXIT)) {
            PAGE_ChangeByID(PAGEID_MENU, PREVIOUS_ITEM);
        } else {
            // only one callback can handle a button press, so we don't handle BUT_ENTER here, let it handled by press cb
            return 0;
        }
    }
    return 1;
}
void update_countdown(u8 idx)
{
    u8 hide = Model.timer[idx].type == TIMER_STOPWATCH || Model.timer[idx].type == TIMER_PERMANENT;
    GUI_SetHidden((guiObject_t *)&gui->start, hide);
    GUI_SetHidden((guiObject_t *)&gui->startlbl, hide);
    // Permanent timer do not have reset command
    hide = Model.timer[idx].type == TIMER_PERMANENT;
    GUI_SetHidden((guiObject_t *)&gui->resetsrc, hide);
    GUI_SetHidden((guiObject_t *)&gui->resetlbl, hide);
}
