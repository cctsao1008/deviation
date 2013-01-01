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
#include "../pages.h"
#include "gui/gui.h"
#include "config/model.h"
#include "simple.h"
#include "../../common/simple/_switchassign_page.c"

static const char *switch_cb2(guiObject_t *obj, int dir, void *data)
{
    const char *str = switch_cb(obj, dir, data);
    if (dir)
        save_changes();
    return str;
}

void PAGE_SwitchAssignInit(int page)
{
    (void)page;
    PAGE_ShowHeader_ExitOnly(PAGE_GetName(PAGEID_SWITCHASSIGN), MODELMENU_Show);
    refresh_switches();

    #define COL1 10
    #define COL2 150
    #define ROW_SPACE 20
    /* Row 1 */
    int row = 40;
    GUI_CreateLabelBox(COL1, row, 0, 16, &DEFAULT_FONT, NULL, NULL, _tr("Fly mode"));
    GUI_CreateTextSelect(COL2, row, TEXTSELECT_128, 0x0000, NULL, switch_cb2, (void *)(long)SWITCHFUNC_FLYMODE);

    /* Row 2 */
    row += ROW_SPACE;
    GUI_CreateLabelBox(COL1, row, 0, 16, &DEFAULT_FONT, NULL, NULL, _tr("Thr hold"));
    GUI_CreateTextSelect(COL2, row, TEXTSELECT_128, 0x0000, NULL, switch_cb2, (void *)(long)SWITCHFUNC_HOLD);

    /* Row 3 */
    row += ROW_SPACE;
    GUI_CreateLabelBox(COL1, row, 0, 16, &DEFAULT_FONT, NULL, NULL, _tr("Gyro sense"));
    GUI_CreateTextSelect(COL2, row, TEXTSELECT_128, 0x0000, NULL, switch_cb2, (void *)(long)SWITCHFUNC_GYROSENSE);
}

