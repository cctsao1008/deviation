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
#include "telemetry.h"
#include "gui/gui.h"

#include "../common/_telemtest_page.c"

typedef enum {
    telemetry_basic,
    telemetry_gps,
    telemetry_off,
} TeleMetryMonitorType;

static u8 _action_cb(u32 button, u8 flags, void *data);
static void _press_cb(guiObject_t *obj, const void *data);
static const char *idx_cb(guiObject_t *obj, const void *data);

static TeleMetryMonitorType current_page = telemetry_basic;

struct telem_layout {
    u8 row_type;
    u8 x;
    u8 width;
    u8 source;
};
#define TYPE_INDEX  0x10
#define TYPE_LABEL  0x20
#define TYPE_LABEL3 0x30
#define TYPE_VALUE  0x80
#define TYPE_VALUE2 0x90
#define TYPE_VALUE4 0xa0

#define TEMP_LABEL 1
#define VOLT_LABEL 2
#define RPM_LABEL 3
#define GPS_LABEL 4
#define ARROW_LABEL 0xff
struct telem_layout2 {
    const struct telem_layout *header;
    const struct telem_layout *layout;
    u8 num_items;
    u8 row_height;
};

const struct telem_layout devo_header_basic[] = {
        {TYPE_LABEL,  8, 35, TEMP_LABEL},
        {TYPE_LABEL, 45, 35, VOLT_LABEL},
        {TYPE_LABEL, 88, 35, RPM_LABEL},
        {TYPE_LABEL, LCD_WIDTH - 11, 10, ARROW_LABEL},
        {0, 0, 0, 0},
};

const struct telem_layout devo_layout_basic[] = {
    {TYPE_INDEX | 0,  0, 8,  1},
    {TYPE_VALUE | 0,  8, 35, TELEM_TEMP1},
    {TYPE_VALUE | 0, 48, 35, TELEM_VOLT1},
    {TYPE_VALUE | 0, 87, 35, TELEM_RPM1},
    {TYPE_INDEX | 1,  0, 8,  2},
    {TYPE_VALUE | 1,  8, 35, TELEM_TEMP2},
    {TYPE_VALUE | 1, 48, 35, TELEM_VOLT2},
    {TYPE_VALUE | 1, 87, 35, TELEM_RPM2},
    {TYPE_INDEX | 2,  0, 8,  3},
    {TYPE_VALUE | 2,  8, 35, TELEM_TEMP3},
    {TYPE_VALUE | 2, 48, 35, TELEM_VOLT3},
    {TYPE_INDEX | 3,  0, 8,  4},
    {TYPE_VALUE | 3,  8, 35, TELEM_TEMP4},
    {0, 0, 0, 0},
};

const struct telem_layout devo_header_gps[] = {
        {TYPE_LABEL,  8, 35, GPS_LABEL},
        {TYPE_LABEL, LCD_WIDTH - 11, 10, ARROW_LABEL},
        {0, 0, 0, 0},
};
const struct telem_layout devo_layout_gps[] = {
    {TYPE_LABEL  | 0,  0, 0,  TELEM_GPS_LAT},
    {TYPE_VALUE2 | 0, 0, LCD_WIDTH - ARROW_WIDTH - 3, TELEM_GPS_LAT},
    {TYPE_LABEL3 | 0, 0, 0,  TELEM_GPS_LONG},
    {TYPE_VALUE4 | 0, 0, LCD_WIDTH - ARROW_WIDTH - 3, TELEM_GPS_LONG},

    {TYPE_LABEL  | 1,  0, 0,  TELEM_GPS_ALT},
    {TYPE_VALUE2 | 1, 0, LCD_WIDTH - ARROW_WIDTH - 3, TELEM_GPS_ALT},
    {TYPE_LABEL3 | 1, 0, 0,  TELEM_GPS_SPEED},
    {TYPE_VALUE4 | 1, 0, LCD_WIDTH - ARROW_WIDTH - 3, TELEM_GPS_SPEED},

    {TYPE_LABEL  | 2,  0, 0,  TELEM_GPS_TIME},
    {TYPE_VALUE2 | 2, 0, LCD_WIDTH - ARROW_WIDTH - 3, TELEM_GPS_TIME},
    {0, 0, 0, 0},
};

const struct telem_layout2 devo_page[] = {
    {devo_header_basic, devo_layout_basic, 4, ITEM_SPACE},
    {devo_header_gps, devo_layout_gps, 3, 4 * ITEM_HEIGHT + 4},
};
static const char *header_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    int source = (long)data;
    switch(source) {
        case TEMP_LABEL: return _tr("Temp");
        case VOLT_LABEL: return _tr("Volt");
        case RPM_LABEL: return _tr("RPM");
        case GPS_LABEL: return _tr("GPS");
        case ARROW_LABEL: return current_page== telemetry_gps ? "<-" : "->";
    }
    return "";
}

static guiObject_t *getobj_cb(int relrow, int col, void *data)
{
    (void)data;
    (void)relrow;
    (void)col;
    return (guiObject_t *)&gui->box[0];
}

static int row_cb(int absrow, int relrow, int y, void *data)
{
    (void)relrow;
    const struct telem_layout *layout = (const struct telem_layout *)data;
    int i = 0;
    struct LabelDesc *font;
    void *cmd = NULL;
    int orig_y = y;
    for(const struct telem_layout *ptr = layout; ptr->source; ptr++, i++) {
        if((ptr->row_type & 0x0f) < absrow)
            continue;
        if((ptr->row_type & 0x0f) > absrow)
            break;
        y = orig_y;
        font = &DEFAULT_FONT;
        switch (ptr->row_type & 0xf0) {
            case TYPE_INDEX:  font = &TINY_FONT; cmd = idx_cb; break;
            case TYPE_LABEL:  cmd = label_cb; break;
            case TYPE_LABEL3: cmd = label_cb; y =orig_y + 2*ITEM_HEIGHT; break;
            case TYPE_VALUE:  font = &tp.font;  cmd = telem_cb; break;
            case TYPE_VALUE2: font = &tp.font;  cmd = telem_cb; y = orig_y + ITEM_HEIGHT;break;
            case TYPE_VALUE4: font = &tp.font;  cmd = telem_cb; y =orig_y + 3*ITEM_HEIGHT; break;
        }
        GUI_CreateLabelBox(&gui->box[i], ptr->x, y, ptr->width, ITEM_HEIGHT,
                font, cmd, NULL, (void *)(long)ptr->source);
    }
    return 0;
}

static void _show_page(const struct telem_layout2 *page)
{
    PAGE_RemoveAllObjects();
    tp.font.font = TINY_FONT.font;
    tp.font.font_color = 0xffff;
    tp.font.fill_color = 0;
    tp.font.style = LABEL_SQUAREBOX;
    long i = 0;
    for(const struct telem_layout *ptr = page->header; ptr->source; ptr++, i++) {
        GUI_CreateLabelBox(&gui->header[i], ptr->x, 0, ptr->width, ITEM_HEIGHT,
                           ptr->source == ARROW_LABEL ? &TINY_FONT : &DEFAULT_FONT,
                           header_cb, NULL, (void *)(long)ptr->source);
    }
    PAGE_ShowHeader(_tr_noop("")); // to draw a underline only
    GUI_CreateScrollable(&gui->scrollable, 0, ITEM_HEIGHT + 1, LCD_WIDTH, LCD_HEIGHT - ITEM_HEIGHT -1,
                         page->row_height, page->num_items, row_cb, getobj_cb, NULL, (void *)page->layout);
    tp.telem = Telemetry;
}

static const char *idx_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    u8 idx = (long)data;
    sprintf(tp.str, "%d", idx);
    return tp.str;
}

void PAGE_TelemtestInit(int page)
{
    (void)okcancel_cb;
    (void)page;
    PAGE_SetModal(0);
    PAGE_SetActionCB(_action_cb);
    if (telem_state_check() == 0) {
        current_page = telemetry_off;
        GUI_CreateLabelBox(&gui->msg, 20, 10, 0, 0, &DEFAULT_FONT, NULL, NULL, tp.str);
        return;
    }

    _show_page(&devo_page[current_page]);
}

void PAGE_TelemtestEvent() {
    long i = 0;
    if (current_page == telemetry_off)
        return;
    struct Telemetry cur_telem = Telemetry;
    u32 updated = TELEMETRY_IsUpdated();
    int current_row = GUI_ScrollableCurrentRow(&gui->scrollable);
    int visible_rows = GUI_ScrollableVisibleRows(&gui->scrollable);
    for (const struct telem_layout *ptr = devo_page[current_page].layout; ptr->source; ptr++, i++) {
        if ((ptr->row_type & 0x0f) < current_row)
            continue;
        if ((ptr->row_type & 0x0f) >= current_row + visible_rows)
            break;
        if (!( ptr->row_type & 0x80))
            continue;
        long cur_val = _TELEMETRY_GetValue(&cur_telem, ptr->source);
        long last_val = _TELEMETRY_GetValue(&tp.telem, ptr->source);
        struct LabelDesc *font;
        font = &TELEM_FONT;
        if (cur_val != last_val) {
            GUI_Redraw(&gui->box[i]);
        } else if(! (updated & (1 << ptr->source))) {
            font = &TELEM_ERR_FONT;
        }
        GUI_SetLabelDesc(&gui->box[i], font);
    }
    tp.telem = cur_telem;
}

void PAGE_TelemtestModal(void(*return_page)(int page), int page)
{
    (void)return_page;
    (void)page;
}

static void _press_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    (void)data;
    current_page = current_page == telemetry_gps?telemetry_basic: telemetry_gps;
    _show_page(&devo_page[current_page]);
}

static void _navigate_pages(s8 direction)
{
    if ((direction == -1 && current_page == telemetry_gps) ||
            (direction == 1 && current_page == telemetry_basic)) {
        _press_cb(NULL, NULL);
    }
}

static u8 _action_cb(u32 button, u8 flags, void *data)
{
    (void)data;
    if ((flags & BUTTON_PRESS) || (flags & BUTTON_LONGPRESS)) {
        if (CHAN_ButtonIsPressed(button, BUT_EXIT)) {
            labelDesc.font = DEFAULT_FONT.font;  // set it back to 12x12 font
            PAGE_ChangeByID(PAGEID_MENU, PREVIOUS_ITEM);
        } else if (current_page != telemetry_off) {
            // this indicates whether telem is off or not supported
            if (CHAN_ButtonIsPressed(button, BUT_RIGHT)) {
                _navigate_pages(1);
            }  else if (CHAN_ButtonIsPressed(button,BUT_LEFT)) {
                _navigate_pages(-1);
            } else {
                return 0;
            }
        }
        else {
            // only one callback can handle a button press, so we don't handle BUT_ENTER here, let it handled by press cb
            return 0;
        }
    }
    return 1;
}
static inline guiObject_t *_get_obj(int idx, int objid) {
    return GUI_GetScrollableObj(&gui->scrollable, idx, objid);
}
