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
#include "config/ini.h"
#include <stdlib.h>

#include "../common/_model_loadsave.c"

static unsigned _action_cb(u32 button, unsigned flags, void *data);
static void _press_cb(guiObject_t *obj, u16 selected, void *data) _UNUSED;

static u8 load_save;

static void _show_buttons(int loadsave)
{
    (void)show_loadsave_cb;
    load_save = loadsave;
    PAGE_SetActionCB(_action_cb);
    if (loadsave == LOAD_TEMPLATE || loadsave == LOAD_MODEL || loadsave == LOAD_ICON)
        PAGE_ShowHeader(_tr("Press ENT to load"));
    else if (loadsave == SAVE_MODEL)
        PAGE_ShowHeader(_tr("Press ENT to copy to"));
    //u8 w = 40;
    //GUI_CreateButtonPlateText(LCD_WIDTH -w -5, 0, w, ITEM_HEIGHT, &DEFAULT_FONT, show_loadsave_cb, 0x0000, okcancel_cb, (void *)(loadsave+1L));
}

static const char *iconstr_cb(guiObject_t *obj, int dir, void *data) _UNUSED;
static const char *iconstr_cb(guiObject_t *obj, int dir, void *data)
{
    (void)obj;
    long num_icons = (long)data;
    u8 changed;
    mp->selected = GUI_TextSelectHelper(mp->selected, 1, num_icons, dir, 1, 1, &changed);
    if (changed)
        select_cb(NULL, mp->selected-1, (void *)LOAD_ICON);
    return string_cb(mp->selected-1, (void *)LOAD_ICON);
}

static void iconpress_cb(guiObject_t *obj, void *data) _UNUSED;
static void iconpress_cb(guiObject_t *obj, void *data)
{
    (void)obj;
    (void)data;
    okcancel_cb(NULL, (void *)(LOAD_ICON+1));
}

static const char *model_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    int absrow = (long)data;
    return string_cb(absrow, (void *)(long)LOAD_MODEL);
}

void modelpress_cb(guiObject_t *obj, s8 press_type, const void *data)
{
    (void)obj;
    (void)press_type;
    int absrow = (long)data;
    mp->selected = absrow+1;
    okcancel_cb(NULL, (void *)(long)(load_save +1));
}

static int row_cb(int absrow, int relrow, int y, void *data)
{
    (void) data;
    GUI_CreateLabelBox(&gui->label[relrow], 0, y,
       LCD_WIDTH, 0, &DEFAULT_FONT, model_cb, modelpress_cb, (void *)(long)absrow);
    return 1;
}
static void _show_list(int loadsave,u8 num_models)
{
    (void) loadsave;
    GUI_CreateScrollable(&gui->scrollable, 0, ITEM_HEIGHT, LCD_WIDTH, LCD_HEIGHT - ITEM_HEIGHT,
                         ITEM_SPACE, num_models, row_cb, NULL, NULL, NULL);
    GUI_SetSelected(GUI_ShowScrollableRowOffset(&gui->scrollable, mp->selected-1));
}

static void _press_cb(guiObject_t *obj, u16 selected, void *data)
{
    (void)obj;
    (void)data;
    mp->selected = selected + 1;
    okcancel_cb(NULL, (void *)(long)(load_save +1));
}

static unsigned _action_cb(u32 button, unsigned flags, void *data)
{
    (void)data;
    if ((flags & BUTTON_PRESS) || (flags & BUTTON_LONGPRESS)) {
        if (CHAN_ButtonIsPressed(button, BUT_EXIT) || load_save == LOAD_LAYOUT) {
            okcancel_cb(NULL, 0);
        }
        else {
            // only one callback can handle a button press, so we don't handle BUT_ENTER here, let it handled by press cb
            return 0;
        }
    }
    return 1;
}
