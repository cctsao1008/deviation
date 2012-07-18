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

#if 0
guiObject_t *GUI_CreateLabel(u16 x, u16 y, const char *(*Callback)(guiObject_t *, void *), struct LabelDesc font, void *data)
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
    box->width = 0;
    box->height = 0;

    obj->Type = Label;
    OBJ_SET_TRANSPARENT(obj, 0);  //Deal with transparency during drawing
    OBJ_SET_USED(obj, 1);
    connect_object(obj);

    label->type = TRANSPARENT;
    label->CallBack = Callback;
    label->cb_data = data;
    label->desc.font = font.font;
    label->desc.font_color = font.color;
    label->desc.style = NO_BOX;

    if (! label->desc.font)
        label->desc.font = DEFAULT_FONT.font;

    return obj;
}
#endif

guiObject_t *GUI_CreateLabelBox(u16 x, u16 y, u16 width, u16 height, struct LabelDesc *desc, const char *(*Callback)(guiObject_t *, void *), void *data)
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
    OBJ_SET_USED(obj, 1);
    connect_object(obj);

    label->desc = *desc;
    if (x == 0 || y == 0)
        label->desc.style = NO_BOX;
    label->CallBack = Callback;
    label->cb_data = data;
    if (! label->desc.font)
        label->desc.font = DEFAULT_FONT.font;

    return obj;
}

void GUI_DrawLabel(struct guiObject *obj)
{
    struct guiLabel *label = &obj->o.label;
    const char *str;
    u16 txt_w, txt_h;
    u16 txt_x, txt_y;
    if (label->CallBack)
        str = label->CallBack(obj, label->cb_data);
    else
        str = (const char *)label->cb_data;
    u8 old_font = LCD_SetFont(label->desc.font);
    LCD_GetStringDimensions((const u8 *)str, &txt_w, &txt_h);
    if (label->desc.style == NO_BOX) {
        txt_x = obj->box.x;
        txt_y = obj->box.y;
        u16 old_w = obj->box.width;
        u16 old_h = obj->box.height;
        if (old_w < txt_w)
            old_w = txt_w;
        if (old_h < txt_h)
            old_h = txt_h;
        obj->box.width = txt_w;
        obj->box.height = txt_h;
        if (OBJ_IS_SHOWN(obj))
            GUI_DrawBackground(obj->box.x, obj->box.y, old_w, old_h);
    } else {
        if (label->desc.style == TRANSPARENT || label->desc.style == CENTER) {
            if (OBJ_IS_SHOWN(obj)) {
                GUI_DrawBackground(obj->box.x, obj->box.y, obj->box.width, obj->box.height);
            }
        } else {
            LCD_FillRect(obj->box.x, obj->box.y, obj->box.width, obj->box.height, label->desc.fill_color);
        }
        if (label->desc.style == TRANSPARENT || label->desc.fill_color != label->desc.outline_color) {
            LCD_DrawRect(obj->box.x, obj->box.y, obj->box.width, obj->box.height, label->desc.outline_color);
        }
        if (obj->box.height > txt_h)
            txt_y = obj->box.y + (obj->box.height - txt_h) / 2;
        else
            txt_y = obj->box.y;
        if (obj->box.width > txt_w)
            txt_x = obj->box.x + (obj->box.width - txt_w) / 2;
        else
            txt_x = obj->box.x;
    }
    LCD_SetFontColor(label->desc.font_color);
    LCD_PrintStringXY(txt_x, txt_y, str);
    LCD_SetFont(old_font);
}

