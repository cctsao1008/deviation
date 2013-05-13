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

#include "../common/_model_config.c"

#define gui (&gui_objs.u.modelcfg)

enum {
    ITEM_SWASHTYPE,
    ITEM_ELEINV,
    ITEM_AILINV,
    ITEM_COLINV,
    ITEM_ELEMIX,
    ITEM_AILMIX,
    ITEM_COLMIX,
    ITEM_LAST,
};

static u8 _action_cb(u32 button, u8 flags, void *data);
static void show_titlerow(const char *header)
{
    PAGE_ShowHeader(header);
    //u8 w = 40;
    // I don't think there is a need for the save button
    //GUI_CreateButtonPlateText(LCD_WIDTH - w -5, 0,
    //        w, ITEM_HEIGHT, &DEFAULT_FONT, NULL, 0x0000, okcancel_cb, _tr("Save"));
}

static guiObject_t *getobj_cb(int relrow, int col, void *data)
{
    (void)col;
    (void)data;
    return (guiObject_t *)&gui->value[relrow];
}
static int row_cb(int absrow, int relrow, int y, void *data)
{
    u8 w = 60;
    u8 x = 63;
    const void *label = NULL;
    void *value = NULL;
    void *tgl = NULL;
    switch(absrow) {
        case ITEM_SWASHTYPE:
            label = _tr("SwashType");
            value = swash_val_cb;
            break;
        case ITEM_ELEINV:
            label = _tr("ELE Inv");
            tgl = swashinv_press_cb; value = swashinv_val_cb; data = (void *)1L;
            break;
        case ITEM_AILINV:
            label = _tr("AIL Inv");
            tgl = swashinv_press_cb; value = swashinv_val_cb; data = (void *)2L;
            break;
        case ITEM_COLINV:
            label = _tr("COL Inv");
            tgl = swashinv_press_cb; value = swashinv_val_cb; data = (void *)4L;
            break;
        case ITEM_ELEMIX:
            label = _tr("ELE Mix");
            value = swashmix_val_cb; data = (void *)1L;
            break;
        case ITEM_AILMIX:
            label = _tr("AIL Mix");
            value = swashmix_val_cb; data = (void *)0L;
            break;
        case ITEM_COLMIX:
            label = _tr("COL Mix");
            value = swashmix_val_cb; data = (void *)2L;
            break;
    }
    GUI_CreateLabelBox(&gui->label[relrow], 0, y,
                0, ITEM_HEIGHT, &DEFAULT_FONT, NULL, NULL, label);
    GUI_CreateTextSelectPlate(&gui->value[relrow], x, y,
                w, ITEM_HEIGHT, &DEFAULT_FONT, tgl, value, data);
    return 1;
}
void MODELPAGE_Config()
{
    PAGE_SetModal(1);
    PAGE_SetActionCB(_action_cb);
    show_titlerow(Model.type == 0 ? _tr("Helicopter") : _tr("Airplane"));

    if (Model.type == 0) {
        GUI_CreateScrollable(&gui->scrollable, 0, ITEM_HEIGHT + 1, LCD_WIDTH, LCD_HEIGHT - ITEM_HEIGHT -1,
                         ITEM_SPACE, ITEM_LAST, row_cb, getobj_cb, NULL, NULL);
        GUI_SetSelected(GUI_ShowScrollableRowOffset(&gui->scrollable, 0));
    }
}

static u8 _action_cb(u32 button, u8 flags, void *data)
{
    (void)data;
    if ((flags & BUTTON_PRESS) || (flags & BUTTON_LONGPRESS)) {
        if (CHAN_ButtonIsPressed(button, BUT_EXIT)) {
            PAGE_ModelInit(-1);
        }
        else {
            // only one callback can handle a button press, so we don't handle BUT_ENTER here, let it handled by press cb
            return 0;
        }
    }
    return 1;
}

static int row2_cb(int absrow, int relrow, int y, void *data)
{
    (void)data;
    u8 w = 60;
    u8 x = 63;
    int idx = 0;
    int pos = 0;
    while(idx < absrow) {
        while(proto_strs[++pos])
            ;
        pos++;
        idx++;
    }
    GUI_CreateLabelBox(&gui->label[relrow], 0, y,
            0, ITEM_HEIGHT, &DEFAULT_FONT, NULL, NULL, _tr(proto_strs[pos]));
    GUI_CreateTextSelectPlate(&gui->value[relrow], x, y,
            w, ITEM_HEIGHT, &DEFAULT_FONT, NULL, proto_opt_cb, (void *)(long)absrow);
    return 1;
}

static int row3_cb(int absrow, int relrow, int y, void *data)
{
    (void)data;
    u8 w = 60;
    u8 x = 63;
    void *ts;
    void *ts_press = NULL;
    void *ts_data = NULL;
    char *label = NULL;
    void *label_cmd = NULL;

    switch (absrow) {
    case 0:
        label = _tr_noop("Center PW");
        ts = set_train_cb; ts_data = (void *)1L;
        break;
    case 1:
        label = _tr_noop("Delta PW");
        ts = set_train_cb; ts_data = (void *)2L;
        break;
    case 2:
        if (PPMin_Mode() == 1) {
            label = _tr_noop("Trainer Sw");
            ts = set_source_cb; ts_press = sourceselect_cb; ts_data = (void *)&Model.train_sw;
        } else {
            label = _tr_noop("Num Channels");
            ts = set_train_cb; ts_data = (void *)0L;
        }
        break;
    default:
        label_cmd = input_chname_cb; label = (void *)((long)absrow - 3);
        ts = set_chmap_cb; ts_data = label;
        break;
    }
    GUI_CreateLabelBox(&gui->label[relrow], 0, y,
            0, ITEM_HEIGHT, &DEFAULT_FONT, label_cmd, NULL, label_cmd ? label : _tr(label));
    GUI_CreateTextSelectPlate(&gui->value[relrow], x, y,
            w, ITEM_HEIGHT, &DEFAULT_FONT, NULL, ts, ts_data);
    return 1;
}

void MODELPROTO_Config()
{
    PAGE_SetModal(1);
    PAGE_SetActionCB(_action_cb);
    show_titlerow(ProtocolNames[Model.protocol]);

    proto_strs = PROTOCOL_GetOptions();
    int idx = 0;
    int pos = 0;
    while(idx < NUM_PROTO_OPTS) {
        if(proto_strs[pos] == NULL)
            break;
        while(proto_strs[++pos])
            ;
        pos++;
        idx++;
    }

    GUI_CreateScrollable(&gui->scrollable, 0, ITEM_HEIGHT + 1, LCD_WIDTH, LCD_HEIGHT - ITEM_HEIGHT -1,
                         ITEM_SPACE, idx, row2_cb, getobj_cb, NULL, NULL);
    GUI_SetSelected(GUI_ShowScrollableRowOffset(&gui->scrollable, 0));
}

void MODELTRAIN_Config()
{
    PAGE_SetModal(1);
    PAGE_SetActionCB(_action_cb);
    show_titlerow((Model.num_ppmin & 0xC0) == 0x40
                  ? _tr("Trainer Config")
                  : _tr("PPMIn Config"));
    GUI_CreateScrollable(&gui->scrollable, 0, ITEM_HEIGHT + 1, LCD_WIDTH, LCD_HEIGHT - ITEM_HEIGHT -1,
                         ITEM_SPACE, PPMin_Mode() == 1 ? 3 + MAX_PPM_IN_CHANNELS : 3, row3_cb, getobj_cb, NULL, NULL);
    GUI_SetSelected(GUI_ShowScrollableRowOffset(&gui->scrollable, 0));
}
