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
#include "pages.h"
#include "icons.h"
#include "gui/gui.h"
#include "config/model.h"

static struct main_page * const mp = &pagemem.u.main_page;
s16 trim_cb(void * data);
const char *show_throttle_cb(guiObject_t *obj, void *data);
const char *voltage_cb(guiObject_t *obj, void *data);
s16 trim_cb(void * data);
void press_icon_cb(guiObject_t *obj, s8 press_type, void *data);
void press_icon2_cb(guiObject_t *obj, void *data);
void press_timer_cb(guiObject_t *obj, s8 press_type, void *data);
const char *show_timer_cb(guiObject_t *obj, void *data);

extern s16 Channels[NUM_CHANNELS];
extern s8 Trims[NUM_TRIMS];


void PAGE_MainInit(int page)
{
    (void)page;
    int i;
    PAGE_SetModal(0);
    for (i = 0; i < TRIMS_TO_SHOW; i++)
        mp->trims[i] = Trims[i];
    mp->throttle = Channels[0];
    GUI_CreateIcon(0, 1, &icons[ICON_OPTIONS], press_icon2_cb, (void *)0);
    mp->nameObj = GUI_CreateLabelBox(96, 8, 128, 24, &MODELNAME_FONT,
                                      NULL, press_icon_cb, Model.name);

    //Throttle
    mp->trimObj[2] = GUI_CreateBarGraph(130, 75, 10, 140, -100, 100, TRIM_VERTICAL, trim_cb, &Trims[2]);
    //Rudder
    mp->trimObj[3] = GUI_CreateBarGraph(5, 220, 125, 10, -100, 100, TRIM_HORIZONTAL, trim_cb, &Trims[3]);
    //Elevator
    mp->trimObj[1] = GUI_CreateBarGraph(180, 75, 10, 140, -100, 100, TRIM_VERTICAL, trim_cb, &Trims[1]);
    //Aileron
    mp->trimObj[0] = GUI_CreateBarGraph(190, 220, 125, 10, -100, 100, TRIM_HORIZONTAL, trim_cb, &Trims[0]);
    //Trim_L
    mp->trimObj[4] = GUI_CreateBarGraph(145, 40, 10, 140, -100, 100, TRIM_VERTICAL, trim_cb, &Trims[1]);
    //Trim_R
    mp->trimObj[5] = GUI_CreateBarGraph(165, 40, 10, 140, -100, 100, TRIM_VERTICAL, trim_cb, &Trims[1]);
    //Throttle
    mp->throttleObj = GUI_CreateLabelBox(16, 40, 100, 40, &THROTTLE_FONT,
                                         show_throttle_cb, NULL, &Channels[INP_THROTTLE - 1]);
    //Pitch
    mp->pitchObj = GUI_CreateLabelBox(16, 90, 100, 40, &THROTTLE_FONT,
                                      show_throttle_cb, NULL, &Channels[5]);
    //Timer
    mp->timerObj = GUI_CreateLabelBox(16, 150, 100, 24, &TIMER_FONT,
                                      show_timer_cb, press_timer_cb, (void *)0);
    //Telemetry value
    mp->telemetryObj = GUI_CreateLabelBox(16, 185, 100, 24, &TIMER_FONT,
                                      show_timer_cb, press_timer_cb, (void *)1);
    //Icon
    mp->iconObj = GUI_CreateImageOffset(205, 40, 96, 96, 0, 0, CONFIG_GetCurrentIcon(), press_icon_cb, (void *)1);
 
    //Battery
    if (Display.flags & SHOW_BAT_ICON) {
        GUI_CreateImage(270,1,48,22,"media/bat.bmp");
    } else {
        GUI_CreateLabelBox(275,10, 0, 0, &BATTERY_FONT, voltage_cb, NULL, NULL);
    }
    //TxPower
    GUI_CreateImageOffset(225,4, 48, 24, 48 * Model.tx_power, 0, "media/txpower.bmp", NULL, NULL);
}

void PAGE_MainEvent()
{
    int i;
    for(i = 0; i < TRIMS_TO_SHOW; i++) {
        if (mp->trims[i] != Trims[i]) {
            mp->trims[i] = Trims[i];
            GUI_Redraw(mp->trimObj[i]);
        }
    }
    
    if(mp->throttle != Channels[INP_THROTTLE - 1]) {
        mp->throttle = Channels[INP_THROTTLE - 1];
        GUI_Redraw(mp->throttleObj);
    }
    if(mp->timer[0] != TIMER_GetValue(0)) {
        mp->timer[0] = TIMER_GetValue(0);
        GUI_Redraw(mp->timerObj);
    }
    if(mp->timer[1] != TIMER_GetValue(1)) {
        mp->timer[1] = TIMER_GetValue(1);
        GUI_Redraw(mp->telemetryObj);
    }
}

const char *show_throttle_cb(guiObject_t *obj, void *data)
{
    (void)obj;
    s16 *throttle = (s16 *)data;
    sprintf(mp->tmpstr, "%3d%%", RANGE_TO_PCT(*throttle));
    return mp->tmpstr;
}

const char *voltage_cb(guiObject_t *obj, void *data) {
    (void)obj;
    (void)data;
    u16 voltage = PWR_ReadVoltage();
    sprintf(mp->tmpstr, "%2d.%02dV", voltage >> 12, (voltage & 0x0fff) / 10);
    return mp->tmpstr;
}

s16 trim_cb(void * data)
{
    s8 *trim = (s8 *)data;
    return *trim;
}

void press_icon_cb(guiObject_t *obj, s8 press_type, void *data)
{
    (void)obj;
    if(press_type == -1) {
        if ((long)data == 0) {
            PAGE_SetSection(SECTION_OPTIONS);
        } else if ((long)data == 1) {
            PAGE_SetSection(SECTION_MODEL);
        } else {
            PAGE_SetModal(1);
            MODELPage_ShowLoadSave(0, PAGE_MainInit);
        }
    }
}

void press_icon2_cb(guiObject_t *obj, void *data)
{
    press_icon_cb(obj, -1, data);
}

void press_timer_cb(guiObject_t *obj, s8 press_type, void *data)
{
    (void)obj;
    if(press_type == -1 && ! mp->ignore_release) 
        TIMER_StartStop((long)data);
    mp->ignore_release = 0;
    if(press_type == 1) {
        TIMER_Reset((long)data);
        mp->ignore_release = 1;
    }
}

const char *show_timer_cb(guiObject_t *obj, void *data)
{
    (void)obj;
    TIMER_SetString(mp->tmpstr, TIMER_GetValue((long)data));
    return mp->tmpstr;
}
