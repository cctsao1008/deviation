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
#include "mixer.h"
#include "standard.h"
#include "mixer_standard.h"

#if HAS_STANDARD_GUI
#include "../../common/standard/_gyrosense_page.c"

static u8 _action_cb(u32 button, u8 flags, void *data);

void PAGE_GyroSenseInit(int page)
{
    (void)page;
    PAGE_SetActionCB(_action_cb);
    PAGE_SetModal(0);
    PAGE_RemoveAllObjects();
    memset(mp, 0, sizeof(*mp));
    int expected = INPUT_NumSwitchPos(mapped_std_channels.switches[SWITCHFUNC_GYROSENSE]);
    int count = STDMIX_GetMixers(mp->mixer_ptr, mapped_std_channels.aux2, GYROMIXER_COUNT);
    if (! count) {
        count = STDMIX_GetMixers(mp->mixer_ptr, mapped_std_channels.gear, GYROMIXER_COUNT);
    }
    if (count != expected) {
        GUI_CreateLabelBox(&gui->msg, 0, 10, 0, ITEM_HEIGHT, &DEFAULT_FONT, NULL, NULL, "Invalid model ini!");// must be invalid model ini
        return;
    }
    gyro_output = mp->mixer_ptr[0]->dest;
    convert_output_to_percentile();
    PAGE_ShowHeader(_tr("Gyro sense"));

    u8 w = 65;
    u8 x = 63;
    u8 row = ITEM_SPACE;
    GUI_CreateLabelBox(&gui->chanlbl, 0, row, 0, ITEM_HEIGHT, &DEFAULT_FONT, NULL, NULL, _tr("Channel"));
    GUI_CreateTextSelectPlate(&gui->chan, x, row, w, ITEM_HEIGHT, &DEFAULT_FONT, NULL, gyro_output_cb, NULL);

    row += ITEM_SPACE;
    w = 40;
    GUI_CreateLabelBox(&gui->gyrolbl[0], 0, row, 0, ITEM_HEIGHT, &DEFAULT_FONT, label_cb, NULL, (void *)(long)0);
    GUI_CreateTextSelectPlate(&gui->gyro[0], x, row, w, ITEM_HEIGHT, &DEFAULT_FONT, NULL, gyro_val_cb, (void *)0);

    row += ITEM_SPACE;
    GUI_CreateLabelBox(&gui->gyrolbl[1], 0, row, 0, ITEM_HEIGHT, &DEFAULT_FONT, label_cb, NULL, (void *)(long)1);
    GUI_CreateTextSelectPlate(&gui->gyro[1], x, row, w, ITEM_HEIGHT, &DEFAULT_FONT, NULL, gyro_val_cb, (void *)1);

    if(INPUT_NumSwitchPos(mapped_std_channels.switches[SWITCHFUNC_GYROSENSE]) == 3) {
        row += ITEM_SPACE;
        GUI_CreateLabelBox(&gui->gyrolbl[2], 0, row, 0, ITEM_HEIGHT, &DEFAULT_FONT, label_cb, NULL, (void *)(long)2);
        GUI_CreateTextSelectPlate(&gui->gyro[2], x, row, w, ITEM_HEIGHT, &DEFAULT_FONT, NULL, gyro_val_cb, (void *)2);
    }

    GUI_Select1stSelectableObj();
}

static u8 _action_cb(u32 button, u8 flags, void *data)
{
    (void)data;
    //u8 total_items = 2;
    if ((flags & BUTTON_PRESS) || (flags & BUTTON_LONGPRESS)) {
        if (CHAN_ButtonIsPressed(button, BUT_EXIT)) {
            PAGE_ChangeByID(PAGEID_MENU, PREVIOUS_ITEM);
        }
        else {
            // only one callback can handle a button press, so we don't handle BUT_ENTER here, let it handled by press cb
            return 0;
        }
    }
    return 1;
}
#endif //HAS_STANDARD_GUI
