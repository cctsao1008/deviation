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
#include "mixer_simple.h"
#include "simple.h"
#include "../../common/simple/_curves_page.c"

static const char *curvepos[] = {
  _tr_noop("L"), "2", "3", "4", _tr_noop("M"), "6", "7", "8", _tr_noop("H")
};

static const char *buttonstr_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    (void)data;
    return _tr("Auto");
}

static const char *lockstr_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    int pos = (long)data;
    return (selectable_bitmaps[curve_mode * 4 + pit_mode] & (1 << (pos - 1))) ? _tr("Unlocked") : _tr("Locked");
}

static void press_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    u8 point_num = (long)data;
    u8 *selectable_bitmap = &selectable_bitmaps[curve_mode * 4 + pit_mode];
    if (*selectable_bitmap >> (point_num-1) & 0x01) {
        GUI_TextSelectEnable(&gui->val[point_num], 0);
        *selectable_bitmap &= ~(1 << (point_num-1));
    } else {
        GUI_TextSelectEnable(&gui->val[point_num], 1);
        *selectable_bitmap |= 1 << (point_num-1);
    }
}

static void update_textsel_state()
{
    for (u8 i = 1; i < 8; i++) {
        u8 selectable_bitmap = selectable_bitmaps[curve_mode * 4 + pit_mode];
        if (selectable_bitmap >> (i-1) & 0x01) {
            GUI_TextSelectEnable(&gui->val[i], 1);
        } else {
            GUI_TextSelectEnable(&gui->val[i], 0);
        }
    }
}

static void show_page(CurvesMode _curve_mode, int page)
{
    (void)page;
    curve_mode = _curve_mode;
    memset(mp, 0, sizeof(*mp));
    if (curve_mode == CURVESMODE_PITCH) {
        PAGE_ShowHeader_ExitOnly(PAGE_GetName(PAGEID_PITCURVES), MODELMENU_Show);
        SIMPLEMIX_GetMixers(mp->mixer_ptr, mapped_simple_channels.pitch, PITCHMIXER_COUNT);
    } else {
        PAGE_ShowHeader_ExitOnly(PAGE_GetName(PAGEID_THROCURVES), MODELMENU_Show);
        SIMPLEMIX_GetMixers(mp->mixer_ptr, mapped_simple_channels.throttle, THROTTLEMIXER_COUNT);
    }
    if (!mp->mixer_ptr[0] || !mp->mixer_ptr[1] || !mp->mixer_ptr[2]) {
        GUI_CreateLabelBox(&gui->msg, 0, 120, 240, 16, &NARROW_FONT, NULL, NULL, "Invalid model ini!");// must be invalid model ini
        return;
    }
    u8 mode_count = 3;
    if (mp->mixer_ptr[3] == NULL)
        pit_hold_state = 0;
    else {
        mode_count = 4;
        pit_hold_state = 1;
    }
    for (u8 i = 0; i < mode_count; i++) {
        if (mp->mixer_ptr[i]->curve.type != CURVE_9POINT) // the 1st version uses 7point curve, need to convert to 13point
            mp->mixer_ptr[i]->curve.type = CURVE_9POINT;
    }
    /* Row 1 */
    GUI_CreateButton(&gui->auto_, 20, 40, BUTTON_64x16, buttonstr_cb, 0x0000, auto_generate_cb, NULL);
    GUI_CreateLabelBox(&gui->modelbl, 92, 40, 0, 16, &DEFAULT_FONT, NULL, NULL, _tr("Mode"));
    GUI_CreateTextSelect(&gui->mode, 140, 40, TEXTSELECT_96, 0x0000, NULL, set_mode_cb, (void *)(long)curve_mode);
    GUI_CreateTextSelect(&gui->hold, 246, 40, TEXTSELECT_64, 0x0000, NULL, set_holdstate_cb, NULL);
    if (pit_mode != PITTHROMODE_HOLD)
        GUI_SetHidden((guiObject_t *)&gui->hold, 1);

    #define COL1 4
    #define COL2 20
    #define COL3 92
    /* Row 2 */
    for(long i = 0; i < 9; i++) {
        const char *label = curvepos[i];
        if(label[0] > '9')
            label = _tr(label);
        GUI_CreateLabelBox(&gui->vallbl[i], COL1, 60+20*i, 0, 16, &DEFAULT_FONT, NULL, NULL, label);
        GUI_CreateTextSelect(&gui->val[i], COL2, 60+20*i, TEXTSELECT_64, 0x0000, NULL, set_pointval_cb, (void *)i);
        if (i > 0 && i < 8)
            GUI_CreateButton(&gui->lock[i-1], COL3, 60+20*i, BUTTON_64x16, lockstr_cb, 0x0000, press_cb, (void *)i);
    }
    update_textsel_state();
    GUI_CreateXYGraph(&gui->graph, 160, 80, 150, 150,
                  CHAN_MIN_VALUE, CHAN_MIN_VALUE,
                  CHAN_MAX_VALUE, CHAN_MAX_VALUE,
                  0, 0,
                  show_curve_cb, curpos_cb, NULL, NULL);
}

void PAGE_ThroCurvesInit(int page)
{
    show_page(CURVESMODE_THROTTLE, page);
}

void PAGE_PitCurvesInit(int page)
{
    show_page(CURVESMODE_PITCH, page);
}

