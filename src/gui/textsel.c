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

#include "target.h"
#define ENABLE_GUIOBJECT
#include "gui.h"
#include "config/display.h"

guiObject_t *GUI_CreateTextSelect(u16 x, u16 y, enum TextSelectType type, u16 fontColor,
        void (*select_cb)(guiObject_t *obj, void *data),
        const char *(*value_cb)(guiObject_t *obj, int value, void *data),
        void *cb_data)
{
    struct guiObject *obj = GUI_GetFreeObj();
    struct guiTextSelect *select;
    struct guiBox *box;

    if (obj == NULL)
        return NULL;

    box = &obj->box;
    select = &obj->o.textselect;

    switch (type) {
        case TEXTSELECT_128:
            select->button = &image_map[FILE_SPIN96];
            break;
        case TEXTSELECT_64:
            select->button = &image_map[FILE_SPIN32];
            break;
        case TEXTSELECT_96:
            select->button = &image_map[FILE_SPIN64];
            break;
    }

    box->height = select->button->height;
    box->width = select->button->width + 2 * ARROW_WIDTH;

    box->x = x;
    box->y = y;

    obj->Type = TextSelect;
    OBJ_SET_TRANSPARENT(obj, TEST_SELECT); //Even if the bmp has transparency, the redraw function will handle it
    OBJ_SET_USED(obj, 1);
    connect_object(obj);

    select->state     = 0;
    select->fontColor = fontColor;
    select->ValueCB   = value_cb;
    select->SelectCB  = select_cb;
    select->cb_data   = cb_data;

    return obj;
}

void GUI_DrawTextSelect(struct guiObject *obj)
{
    u16 x, y, w, h;
    struct guiBox *box = &obj->box;
    struct guiTextSelect *select = &obj->o.textselect;
    GUI_DrawImageHelper(box->x + ARROW_WIDTH,
                        box->y, select->button, DRAW_NORMAL);
    GUI_DrawImageHelper(box->x, box->y, ARROW_LEFT,
                        select->state & 0x01 ? DRAW_PRESSED : DRAW_NORMAL);
    GUI_DrawImageHelper(box->x + box->width - ARROW_WIDTH,
                        box->y, ARROW_RIGHT,
                        select->state & 0x02 ? DRAW_PRESSED : DRAW_NORMAL);

    const char *str =select->ValueCB(obj, 0, select->cb_data);
    LCD_SetFontColor(select->fontColor);
    LCD_GetStringDimensions((const u8 *)str, &w, &h);
    x = box->x + (box->width - w) / 2;
    y = box->y + 2 + (box->height - h) / 2;
    LCD_PrintStringXY(x, y, str);
}

s32 GUI_TextSelectHelper(s32 value, s32 min, s32 max, s8 dir, u8 shortstep, u8 longstep, u8 *_changed)
{
    u8 changed = 0;
    if (dir > 0) {
        if (value < max) {
           value += (dir > 1) ? longstep : shortstep;
           if (value > max)
               value = max;
           changed = 1;
        }
    } else if (dir < 0) {
        if (value > min) {
           value -= (dir < -1) ? longstep : shortstep;
           if (value < min)
               value = min;
           changed = 1;
        }
    }
    if(_changed)
        *_changed = changed;
    return value;
}

u8 GUI_TouchTextSelect(struct guiObject *obj, struct touch *coords, s8 press_type)
{
    struct guiBox box = obj->box;
    struct guiTextSelect *select = &obj->o.textselect;

    if (press_type < 0) {
        if(! select->state) {
            return 0;
        } else if(select->state & 0x80) {
            select->state = 0;
            OBJ_SET_DIRTY(obj, 1);
            return 1;
        } else if(select->state == 0x01) {
            select->state = 0;
            OBJ_SET_DIRTY(obj, 1);
            select->ValueCB(obj, -1, select->cb_data);
            return 1;
        } else if(select->state == 0x02) {
            select->state = 0;
            OBJ_SET_DIRTY(obj, 1);
            select->ValueCB(obj, 1, select->cb_data);
            return 1;
        } else if(select->state == 0x04) {
            select->state = 0;
            OBJ_SET_DIRTY(obj, 1);
            select->SelectCB(obj, select->cb_data);
            return 1;
        }
        printf("Error: Should not get here\n");
        return 0;
    }
    box.width = ARROW_WIDTH;
    if (coords_in_box(&box, coords)) {
        if (! press_type) {
            if (! select->state) {
                select->state = 0x01;
                OBJ_SET_DIRTY(obj, 1);
            }
        } else if (select->ValueCB) {
            OBJ_SET_DIRTY(obj, 1);
            select->ValueCB(obj, -2, select->cb_data);
            select->state |= 0x80;
        }
        return 1;
    }
    box.x = obj->box.x + obj->box.width - ARROW_WIDTH;
    if (coords_in_box(&box, coords)) {
        if (! press_type) {
            if (! select->state) {
                select->state = 0x02;
                OBJ_SET_DIRTY(obj, 1);
            }
        } else if (select->ValueCB) {
            OBJ_SET_DIRTY(obj, 1);
            select->ValueCB(obj, 2, select->cb_data);
            select->state |= 0x80;
        }
        return 1;
    }
    if (! press_type && ! select->state && select->SelectCB) {
        OBJ_SET_DIRTY(obj, 1);
        select->state = 0x04;
        return 1;
    }
    return 0;
}

