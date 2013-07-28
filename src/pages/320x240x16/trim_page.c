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

#include "../common/_trim_page.c"

#define gui (&gui_objs.u.trim)
#define gui_ed (&gui_objs.u.trimedit)

#define PCOL1 (4 + ((LCD_WIDTH - 320) / 2))
#define PCOL2 (72 + ((LCD_WIDTH - 320) / 2))
#define PCOL3 (134 + ((LCD_WIDTH - 320) / 2))
#define PCOL4 (196 + ((LCD_WIDTH - 320) / 2))
#define PROW1 (40 + ((LCD_HEIGHT - 240) / 2))
#define PROW2 (66 + ((LCD_HEIGHT - 240) / 2))
#define PROW3 (PROW2 + 2)

static guiObject_t *getobj_cb(int relrow, int col, void *data)
{
    (void)data;
    col = (col + 2) % 2;
    return col ? (guiObject_t *)&gui->step[relrow] : (guiObject_t *)&gui->src[relrow];
}

static int row_cb(int absrow, int relrow, int y, void *data)
{
    (void)data;
    struct Trim *trim = MIXER_GetAllTrims();
    GUI_CreateButton(&gui->src[relrow], PCOL1, y, BUTTON_64x16,
        trimsource_name_cb, 0x0000, _edit_cb, (void *)((long)absrow));
    GUI_CreateLabel(&gui->neg[relrow], PCOL2 + 6, y, NULL, DEFAULT_FONT, (void *)INPUT_ButtonName(trim[absrow].neg));
    GUI_CreateLabel(&gui->pos[relrow], PCOL3 + 6, y, NULL, DEFAULT_FONT, (void *)INPUT_ButtonName(trim[absrow].pos));
    GUI_CreateTextSelect(&gui->step[relrow], PCOL4 + 6, y, TEXTSELECT_96, NULL, set_trimstep_cb, &trim[absrow].step);
    return 2;
}

static void _show_page()
{
    if (Model.mixer_mode == MIXER_STANDARD)
        PAGE_ShowHeader_ExitOnly(PAGE_GetName(PAGEID_TRIM), MODELMENU_Show);
    else
        PAGE_ShowHeader(PAGE_GetName(PAGEID_TRIM));
    GUI_CreateLabelBox(&gui->inplbl, PCOL1, PROW1, 64, 15, &NARROW_FONT, NULL, NULL, _tr("Input"));
    GUI_CreateLabelBox(&gui->neglbl, PCOL2, PROW1, 64, 15, &NARROW_FONT, NULL, NULL, _tr("Trim -"));
    GUI_CreateLabelBox(&gui->poslbl, PCOL3, PROW1, 64, 15, &NARROW_FONT, NULL, NULL, _tr("Trim +"));
    GUI_CreateLabelBox(&gui->steplbl, PCOL4, PROW1, 108, 15, &NARROW_FONT, NULL, NULL, _tr("Trim Step"));

    GUI_CreateScrollable(&gui->scrollable,
         PCOL1, PROW2,  LCD_WIDTH - 2 * PCOL1, NUM_TRIM_ROWS * 24 - 8,
         24, NUM_TRIMS, row_cb, getobj_cb, NULL, NULL);
}

static void _edit_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    struct Trim *trim = MIXER_GetAllTrims();
    PAGE_SetModal(1);
    tp->index = (long)data;
    tp->trim = trim[tp->index];

    PAGE_RemoveAllObjects();
    PAGE_CreateCancelButton(LCD_WIDTH-160, 4, okcancel_cb);
    PAGE_CreateOkButton(LCD_WIDTH-56, 4, okcancel_cb);

    #define COL1 (8 + ((LCD_WIDTH - 320) / 2))
    #define COL2 (104 + ((LCD_WIDTH - 320) / 2))
    #define ROW1 (48 + ((LCD_HEIGHT - 240) / 2))
    #define ROW2 (ROW1 + 24)
    #define ROW3 (ROW1 + 48)
    #define ROW4 (ROW1 + 72)
    #define ROW5 (ROW1 + 96)
    //Row 1
    GUI_CreateLabel(&gui_ed->srclbl, COL1, ROW1, NULL, DEFAULT_FONT, _tr("Input"));
    GUI_CreateTextSelect(&gui_ed->src, COL2, ROW1, TEXTSELECT_96, NULL, set_source_cb, &tp->trim.src);
    //Row 2
    GUI_CreateLabel(&gui_ed->neglbl, COL1, ROW2, NULL, DEFAULT_FONT, _tr("Trim +"));
    GUI_CreateTextSelect(&gui_ed->neg, COL2, ROW2, TEXTSELECT_96, NULL, set_trim_cb, &tp->trim.pos);
    //Row 3
    GUI_CreateLabel(&gui_ed->poslbl, COL1, ROW3, NULL, DEFAULT_FONT, _tr("Trim -"));
    GUI_CreateTextSelect(&gui_ed->pos, COL2, ROW3, TEXTSELECT_96, NULL, set_trim_cb, &tp->trim.neg);
    //Row 4
    GUI_CreateLabel(&gui_ed->steplbl, COL1, ROW4, NULL, DEFAULT_FONT, _tr("Trim Step"));
    GUI_CreateTextSelect(&gui_ed->step, COL2, ROW4, TEXTSELECT_96, NULL, set_trimstep_cb, &tp->trim.step);
    //Row 5
    GUI_CreateLabel(&gui_ed->swlbl, COL1, ROW5, NULL, DEFAULT_FONT, _tr("Switch"));
    GUI_CreateTextSelect(&gui_ed->sw, COL2, ROW5, TEXTSELECT_96, NULL, set_switch_cb, &tp->trim.sw);
}
