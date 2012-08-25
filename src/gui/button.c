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

guiObject_t *GUI_CreateButton(u16 x, u16 y, enum ButtonType type,
    const char *(*strCallback)(struct guiObject *, void *), u16 fontColor,
    void (*CallBack)(struct guiObject *obj, void *data), void *cb_data)
{
    struct guiObject *obj    = GUI_GetFreeObj();
    struct guiButton *button;
    struct guiBox    *box;

    if (obj == NULL)
        return NULL;

    box = &obj->box;
    button = &obj->o.button;

    switch (type) {
        case BUTTON_96: button->image = &image_map[FILE_BTN96_24]; break;
        case BUTTON_48: button->image = &image_map[FILE_BTN48_24]; break;
        case BUTTON_96x16: button->image = &image_map[FILE_BTN96_16]; break;
        case BUTTON_64x16: button->image = &image_map[FILE_BTN64_16]; break;
        case BUTTON_48x16: button->image = &image_map[FILE_BTN48_16]; break;
        case BUTTON_32x16: button->image = &image_map[FILE_BTN32_16]; break;
    }

    box->x = x;
    box->y = y;
    box->width = button->image->width;
    box->height = button->image->height;

    obj->Type = Button;
    //Even though the image cannot be overlapped, the file can change under press and select states
    //So we need transparency set
    OBJ_SET_TRANSPARENT(obj, 1);
    OBJ_SET_SELECTABLE(obj, 1);
    connect_object(obj);

    button->fontColor = fontColor;
    button->strCallback = strCallback;
    button->CallBack = CallBack;
    button->cb_data = cb_data;

    return obj;
}

guiObject_t *GUI_CreateIcon(u16 x, u16 y, const struct ImageMap *image,
        void (*CallBack)(struct guiObject *obj, void *data), void *cb_data)
{
    struct guiObject *obj    = GUI_GetFreeObj();
    struct guiButton *button;
    struct guiBox    *box;

    if (obj == NULL)
        return NULL;

    box = &obj->box;
    button = &obj->o.button;
    button->image = image;

    box->x = x;
    box->y = y;
    box->width = button->image->width;
    box->height = button->image->height;

    obj->Type = Button;
    //Even though the image cannot be overlapped, the file can change under press and select states
    //So we need transparency set
    OBJ_SET_TRANSPARENT(obj, 1);
    OBJ_SET_SELECTABLE(obj, 1);
    connect_object(obj);

    button->strCallback = NULL;
    button->CallBack = CallBack;
    button->cb_data = cb_data;

    return obj;
}

void GUI_DrawButton(struct guiObject *obj)
{
    struct guiButton *button = &obj->o.button;
    struct guiBox *box = &obj->box;
    const char *txt;
    u16 x_off, y_off;

    GUI_DrawImageHelper(box->x, box->y, button->image, obj == objTOUCHED ? DRAW_PRESSED : DRAW_NORMAL);
    if (button->strCallback) {
        u16 text_w, text_h;
        txt = button->strCallback(obj, button->cb_data);
        if (txt) {
            LCD_GetStringDimensions((u8 *) txt, &text_w, &text_h);
            x_off = (box->width - text_w) / 2 + box->x;
            y_off = (box->height - text_h) / 2 + box->y + 1;
           LCD_SetFontColor(button->fontColor);
           LCD_PrintStringXY(x_off, y_off, txt);
        }
    }
}
