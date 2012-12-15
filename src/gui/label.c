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
#define ENABLE_GUIOBJECT
#include "gui.h"
#include "config/display.h"

static u16  blink_fontcolor = 0xffff;
guiObject_t *GUI_CreateLabelBox(u16 x, u16 y, u16 width, u16 height, const struct LabelDesc *desc,
             const char *(*strCallback)(guiObject_t *, const void *),
             void (*pressCallback)(guiObject_t *obj, s8 press_type, const void *data),
             const void *data)
{
    struct guiObject *obj = GUI_GetFreeObj();
    struct guiLabel  *label;
    struct guiBox    *box;

    if (obj == NULL)
        return NULL;

    box = &obj->box;
    label = &obj->o.label;
    box->x = x;
    box->y = y;
    box->width = width;
    box->height = height;

    obj->Type = Label;
    OBJ_SET_TRANSPARENT(obj, 0);  //Deal with transparency during drawing
    OBJ_SET_SELECTABLE(obj, pressCallback ? 1 :0);
    connect_object(obj);

    label->desc = *desc;
    if ((width == 0 || height == 0) && label->desc.style != LABEL_UNDERLINE && label->desc.style != LABEL_UNDERLINE)
        label->desc.style = LABEL_NO_BOX;
    label->strCallback = strCallback;
    label->pressCallback = pressCallback;
    label->cb_data = data;
    if (! label->desc.font)
        label->desc.font = DEFAULT_FONT.font;
    return obj;
}

void GUI_DrawLabel(struct guiObject *obj)
{
    struct guiLabel *label = &obj->o.label;
    const char *str;
    //Set font here so the callback can get its dimensions
    LCD_SetFont(label->desc.font);
    if (label->strCallback)
        str = label->strCallback(obj, label->cb_data);
    else
        str = (const char *)label->cb_data;
    GUI_DrawLabelHelper(obj->box.x, obj->box.y, obj->box.width, obj->box.height, str, &label->desc, obj == objSELECTED);
}

/**
 * this hepler is created to let TextSelect share the label drawing behavior for Devo10
 */
void GUI_DrawLabelHelper(u16 obj_x, u16 obj_y, u16 obj_width, u16 obj_height, const char *str,
        const struct LabelDesc *desc, u8 is_selected)
{
    u16 txt_w, txt_h;
    u16 txt_x, txt_y;
    u16 w = obj_width;
    u16 h = obj_height;
    LCD_SetFont(desc->font);
    LCD_GetStringDimensions((const u8 *)str, &txt_w, &txt_h);
    if (desc->style == LABEL_BOX || desc->style == LABEL_BRACKET || desc->style == LABEL_SQUAREBOX) {
        // draw round rect for the textsel widget when it is pressable
        if (obj_width == 0)
            w = txt_w;
        if (obj_height == 0)
            h = txt_h;
        GUI_DrawBackground(obj_x, obj_y, w, h);
        if (is_selected) {
            if (desc->style == LABEL_SQUAREBOX)
                LCD_FillRect(obj_x, obj_y, w, h , 1);
            else
                LCD_FillRoundRect(obj_x, obj_y, w, h , 3, 1);
        }  else {
            if (desc->style == LABEL_SQUAREBOX)
                if (desc->fill_color == 0)
                    LCD_DrawRect(obj_x, obj_y, w, h, 1);
                else
                    LCD_FillRect(obj_x, obj_y, w, h, desc->fill_color);
            else if (desc->style == LABEL_BRACKET) {
                u16 y1 = obj_y + 2;
                u16 y2 = obj_y + obj_height -3;
                u16 x1 = obj_x + obj_width - 1;
                LCD_DrawLine(obj_x, y1, obj_x + 2, obj_y, 1);
                LCD_DrawLine(obj_x, y2, obj_x + 2, obj_y + obj_height -1, 1);
                LCD_DrawLine(obj_x, y1, obj_x, y2, 1);
                LCD_DrawLine(x1, y1, x1 - 2, obj_y, 1);
                LCD_DrawLine(x1, y2, x1 - 2, obj_y + obj_height -1, 1);
                LCD_DrawLine(x1, y1, x1, y2, 1);
            } else
                LCD_DrawRoundRect(obj_x, obj_y, w, h , 3,  1);
        }
        if (obj_height > txt_h)
            txt_y = obj_y + (obj_height - txt_h) / 2;
        else
            txt_y = obj_y;
        if (obj_width > txt_w && desc->style != LABEL_LEFT)
            txt_x = obj_x + (obj_width - txt_w) / 2;
        else
            txt_x = obj_x;
    }
    else if (desc->style == LABEL_INVERTED || is_selected) {
        if (w < txt_w)
            w = txt_w;
        if (h < txt_h)
            h = txt_h;
        LCD_FillRect(obj_x, obj_y, w, h, 0xffff);
        if (obj_height > txt_h)
            txt_y = obj_y + (obj_height - txt_h) / 2;
        else
            txt_y = obj_y;
        if (obj_width > txt_w && desc->style != LABEL_LEFT)
            txt_x = obj_x + (obj_width - txt_w) / 2;
        else
            txt_x = obj_x;
    } else if (desc->style == LABEL_NO_BOX) {
        txt_x = obj_x;
        txt_y = obj_y;
        u16 old_w = obj_width;
        u16 old_h = obj_height;
        if (old_w < txt_w)
            old_w = txt_w;
        if (old_h < txt_h)
            old_h = txt_h;
        obj_width = txt_w;
        obj_height = txt_h;
        GUI_DrawBackground(obj_x, obj_y, old_w, old_h);
    } else if (desc->style == LABEL_UNDERLINE) {
        txt_x = obj_x;
        txt_y = obj_y;
        u16 old_w = obj_width;
        u16 old_h = obj_height;
        if (old_w < txt_w)
            old_w = txt_w;
        if (old_h < txt_h)
            old_h = txt_h;
        GUI_DrawBackground(obj_x, obj_y, old_w, old_h);
        LCD_DrawFastHLine(obj_x, obj_y + old_h, old_w, 1); // bug fix: get right y position and give 1 moreline space
        if (obj_height > txt_h)
            txt_y = obj_y + (obj_height - txt_h) / 2;
        else
            txt_y = obj_y;
    } else {
        u8 y_off = 0;
        if (desc->style == LABEL_TRANSPARENT || desc->style == LABEL_CENTER || desc->style == LABEL_LEFT ||
                desc->style == LABEL_LEFTCENTER || desc->style ==LABEL_BLINK) {
           GUI_DrawBackground(obj_x, obj_y, obj_width, obj_height);
        } else {
            LCD_FillRect(obj_x, obj_y, obj_width, obj_height, desc->fill_color);
            y_off = 1;
        }
        if (desc->style == LABEL_TRANSPARENT || desc->fill_color != desc->outline_color) {
            LCD_DrawRect(obj_x, obj_y, obj_width, obj_height, desc->outline_color);
            y_off = 1;
        }
        if (obj_height > txt_h)
            txt_y = obj_y + (obj_height - txt_h) / 2;
        else
            txt_y = obj_y + y_off;
        if (obj_width > txt_w && desc->style != LABEL_LEFT &&  desc->style != LABEL_LEFTCENTER)
            txt_x = obj_x + (obj_width - txt_w) / 2;
        else
            txt_x = obj_x;
    }

    if (desc->style == LABEL_BLINK ) {
        blink_fontcolor = ~blink_fontcolor;
        LCD_SetFontColor(blink_fontcolor);
    } else if (desc->style == LABEL_INVERTED || is_selected) {
        LCD_SetFontColor(~desc->font_color);
    } else {
        LCD_SetFontColor(desc->font_color);
    }
    LCD_PrintStringXY(txt_x, txt_y, str);
}

u8 GUI_TouchLabel(struct guiObject *obj, struct touch *coords, s8 press_type)
{
    (void)coords;
    struct guiLabel *label = &obj->o.label;
    label->pressCallback(obj, press_type, label->cb_data);
    return 1;
}
void GUI_SetLabelDesc(struct guiObject *obj, struct LabelDesc *desc)
{
    struct guiLabel *label = &obj->o.label;
    if (memcmp(&label->desc, desc, sizeof(struct LabelDesc)) != 0)
        OBJ_SET_DIRTY(obj, 1);
    label->desc = *desc;
}
