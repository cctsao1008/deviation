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
#include "telemetry.h"
#include "pages.h"
#include "gui/gui.h"
#include "config/model.h"

#include "../common/_telemconfig_page.c"

void PAGE_TelemconfigInit(int page)
{
   (void)page;
    const u8 row_height = 25;
    PAGE_SetModal(0);
    PAGE_ShowHeader(_tr("Telemetry config")); // using the same name as related menu item to reduce language strings

    for (long i = 0; i < TELEM_NUM_ALARMS; i++) {
        GUI_CreateLabelBox(10, 70 + row_height * i, 55, 16, &DEFAULT_FONT,
           label_cb, NULL, (void *)i);
        GUI_CreateTextSelect(65, 70 + row_height * i, TEXTSELECT_96, 0x0000, NULL, telem_name_cb, (void *)i);
        GUI_CreateTextSelect(166, 70 + row_height * i, TEXTSELECT_64, 0x0000, NULL, gtlt_cb, (void *)i);
        tp.valueObj[i] = GUI_CreateTextSelect(235, 70 + row_height * i, TEXTSELECT_64, 0x0000, NULL, limit_cb, (void *)i);
    }
}
