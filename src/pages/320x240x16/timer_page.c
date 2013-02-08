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

static void _show_page()
{
    if (Model.mixer_mode == MIXER_SIMPLE)
        PAGE_ShowHeader_ExitOnly(PAGE_GetName(PAGEID_TIMER), MODELMENU_Show);
    else
        PAGE_ShowHeader(PAGE_GetName(PAGEID_TIMER));

    for (u8 i = 0; i < NUM_TIMERS; i++) {
        u8 x = 48 + i * 96;
        //Row 1
        GUI_CreateLabel(&gui->timer[i], 8, x, timer_str_cb, DEFAULT_FONT, (void *)(long)i);
        GUI_CreateTextSelect(&gui->type[i], 72, x, TEXTSELECT_96, toggle_timertype_cb, set_timertype_cb, (void *)(long)i);
        //Row 2
        GUI_CreateLabel(&gui->switchlbl[i], 8, x+22, NULL, DEFAULT_FONT, _tr("Switch:"));
        GUI_CreateTextSelect(&gui->src[i], 72, x+22, TEXTSELECT_96, toggle_source_cb, set_source_cb, (void *)(long)i);
        //Row 3
	GUI_CreateLabel(&gui->resetlbl[i], 8, x+44, NULL, DEFAULT_FONT, _tr("Reset sw:"));
        GUI_CreateTextSelect(&gui->resetsrc[i], 72, x+44, TEXTSELECT_96, toggle_resetsrc_cb, set_resetsrc_cb, (void *)(long)i);
        //Row 4
        GUI_CreateLabelBox(&gui->startlbl[i], 8, x+66, 50, 12, &DEFAULT_FONT, NULL, NULL, _tr("Start:"));
        GUI_CreateTextSelect(&gui->start[i], 72, x+66, TEXTSELECT_96, NULL, set_start_cb, (void *)(long)i);
        update_countdown(i);
    }
}

void update_countdown(u8 idx)
{
    u8 hide = Model.timer[idx].type == TIMER_STOPWATCH;
    GUI_SetHidden((guiObject_t *)&gui->start[idx], hide);
    GUI_SetHidden((guiObject_t *)&gui->startlbl[idx], hide);
}

