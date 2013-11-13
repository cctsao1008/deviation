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

static struct tx_configure_page * const cp = &pagemem.u.tx_configure_page;  // MACRO is not good when debugging
#define guic (&gui_objs.u.calibrate)

enum {
#ifndef NO_LANGUAGE_SUPPORT
    ITEM_LANG,
#endif
    ITEM_MODE,
    ITEM_STICKS,
    ITEM_BUZZ,
    ITEM_HAPTIC,
    ITEM_PWR_ALARM,
    ITEM_BATT,
    ITEM_ALARM_INTV,
    ITEM_PWRDN_ALARM,
    ITEM_BACKLIGHT,
    ITEM_CONTRAST,
    ITEM_DIMTIME,
    ITEM_DIMVAL,
    ITEM_PREALERT,
    ITEM_PREALERT_IVAL,
    ITEM_TIMEUP,
    ITEM_TELEMTEMP,
    ITEM_TELEMLEN,
    ITEM_LAST,
};

enum calibType {
    CALIB_NONE,
    CALIB_TOUCH,
    CALIB_STICK,
    CALIB_STICK_TEST,
    SET_CLOCK,
};

static inline guiObject_t *_get_obj(int idx, int objid);
#define XCOORD 20
#define YCOORD 20
enum calibrateState {
    CALI_CENTER,
    CALI_MAXMIN,
    CALI_SUCCESS,
    CALI_SUCCESSEXIT,
    CALI_EXIT,
};
static enum calibrateState calibrate_state;
static const char *auto_dimmer_time_cb(guiObject_t *obj, int dir, void *data);
static u8 _action_cb_calibrate(u32 button, u8 flags, void *data)
{
    (void)data;
    u8 i;
    if (flags & BUTTON_PRESS) {
        if (CHAN_ButtonIsPressed(button, BUT_EXIT)) {
            // bug fix: when most users see the "Calibration done", it is very likely tha they will press ext to exit,
            // then all calibration data are rollback, and cause the calibration failed -- what a tough bug!!
            if (calibrate_state == CALI_SUCCESS)
                calibrate_state = CALI_SUCCESSEXIT;
            else
                calibrate_state = CALI_EXIT;
        } else if (CHAN_ButtonIsPressed(button, BUT_ENTER)) {
            switch (calibrate_state){
            case CALI_CENTER:
                for (i = 0; i < INP_HAS_CALIBRATION; i++) {
                    s32 value = CHAN_ReadRawInput(i + 1);
                    Transmitter.calibration[i].max = 0x0000;
                    Transmitter.calibration[i].min = 0xFFFF;
                    Transmitter.calibration[i].zero = value;
                }
                sprintf(cp->tmpstr, "%s", _tr("Move sticks and knobs\nto max & min positions\nthen press ENT"));
                GUI_Redraw(&guic->msg);
                calibrate_state = CALI_MAXMIN;
                break;
            case CALI_MAXMIN:
                for (i = 0; i < INP_HAS_CALIBRATION; i++) {
                    printf("Input %d: Max: %d Min: %d Zero: %d\n", i+1, Transmitter.calibration[i].max, Transmitter.calibration[i].min, Transmitter.calibration[i].zero);
                }
                sprintf(cp->tmpstr, "%s", _tr("Calibration done."));
                GUI_Redraw(&guic->msg);
                calibrate_state = CALI_SUCCESS;
                break;
            case CALI_SUCCESS:
                calibrate_state = CALI_SUCCESSEXIT;
                break;
            default: break;
            }
        }else {
            // only one callback can handle a button press, so we don't handle BUT_ENTER here, let it handled by press cb
            return 0;
        }
    }
    return 1;
}

static void calibrate_sticks(void)
{
    // bug fix: should turn of safety dialog during calibrating, or it might fail when stick is not calibrated and safety setting is on
    PAGE_DisableSafetyDialog(1);
    PROTOCOL_DeInit();
    PAGE_SetModal(1);
    PAGE_RemoveAllObjects();
    PAGE_SetActionCB(_action_cb_calibrate);
    sprintf(cp->tmpstr, "%s",  _tr("Center all \nsticks and knobs\nthen press ENT"));
    GUI_CreateLabelBox(&guic->msg, 1, 10, LCD_WIDTH -1, LCD_HEIGHT - 10,
            LCD_HEIGHT > 70? &NARROW_FONT:&DEFAULT_FONT, NULL, NULL, cp->tmpstr);
    memcpy(cp->calibration, Transmitter.calibration, sizeof(cp->calibration));

    while(1) {
        CLOCK_ResetWatchdog();
        if(PWR_CheckPowerSwitch())
            PWR_Shutdown();
        if(priority_ready & (1 << MEDIUM_PRIORITY)) {
            BUTTON_Handler();
            priority_ready &= ~(1 << MEDIUM_PRIORITY);
        }
        if(priority_ready & (1 << LOW_PRIORITY)) {
            //Only sample every 100msec
            GUI_RefreshScreen();
            priority_ready = 0;
        }
        for (u8 i = 0; i < INP_HAS_CALIBRATION; i++) {
            s32 value = CHAN_ReadRawInput(i + 1);
            if (value > Transmitter.calibration[i].max)
                Transmitter.calibration[i].max = value;
            else if (value < Transmitter.calibration[i].min)
                Transmitter.calibration[i].min = value;
        }
        if (calibrate_state == CALI_SUCCESSEXIT || calibrate_state == CALI_EXIT)
            break;
    }
    if (calibrate_state == CALI_EXIT)
        memcpy(Transmitter.calibration, cp->calibration, sizeof(cp->calibration));

    PAGE_SetActionCB(NULL);
    PROTOCOL_Init(0);
    PAGE_TxConfigureInit(-1);   // should be -1 so that devo10 can get back to previous item selection
    PAGE_DisableSafetyDialog(0);
}

static const char *calibratestr_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    (void)data;
    return _tr("Calibrate");
}

#if HAS_RTC
static const char *clockstr_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    (void)data;
    return _tr("Set");
}
#endif

static const char *modeselect_cb(guiObject_t *obj, int dir, void *data)
{
    (void)data;
    (void)obj;
    Transmitter.mode = GUI_TextSelectHelper(Transmitter.mode, MODE_1, MODE_4, dir, 1, 1, NULL);
    sprintf(cp->tmpstr, _tr("Mode %d"), Transmitter.mode);
    return cp->tmpstr;
}

static const char *brightness_select_cb(guiObject_t *obj, int dir, void *data)
{
    (void)data;
    (void)obj;
    u8 changed;
    Transmitter.brightness = GUI_TextSelectHelper(Transmitter.brightness,
                                  MIN_BRIGHTNESS, 10, dir, 1, 1, &changed);
    if (changed)
        BACKLIGHT_Brightness(Transmitter.brightness);
    if (Transmitter.brightness == 0)
        return _tr("Off");
    sprintf(cp->tmpstr, "%d", Transmitter.brightness);
    return cp->tmpstr;
}

static const char *common_select_cb(guiObject_t *obj, int dir, void *data)
{
    (void)data;
    u8 *unsigned_data = (u8 *)data;
    if (GUI_IsTextSelectEnabled(obj)) {
        *unsigned_data = GUI_TextSelectHelper(*unsigned_data, 0, 10, dir, 1, 1, NULL);
    }
    if (*unsigned_data == 0)
        return _tr("Off");
    sprintf(cp->tmpstr, "%d", *unsigned_data);
    return cp->tmpstr;
}

static const char *auto_dimmer_time_cb(guiObject_t *obj, int dir, void *data)
{
    (void)obj;
    (void)data;
    u8 changed;
    u16 dimmer_timmer = Transmitter.auto_dimmer.timer/1000;
    dimmer_timmer = GUI_TextSelectHelper(dimmer_timmer,
            MIN_BACKLIGHT_DIMTIME, MAX_BACKLIGHT_DIMTIME, dir, 5, 10, &changed);
    if (changed)
        Transmitter.auto_dimmer.timer = dimmer_timmer * 1000;
    guiObject_t *dimobj = _get_obj(ITEM_DIMVAL, 0);
    if (dimmer_timmer == 0) {
        if(dimobj)
            GUI_TextSelectEnable((guiTextSelect_t *)dimobj, 0);
        return _tr("Off");
    }
    if (dimobj)
        GUI_TextSelectEnable((guiTextSelect_t *)dimobj, 1);
    TIMER_SetString(cp->tmpstr, Transmitter.auto_dimmer.timer);
    return cp->tmpstr;
}

static const char *prealert_time_cb(guiObject_t *obj, int dir, void *data)
{
    (void)obj;
    (void)data;
    u8 changed;
    u16 prealert_time = Transmitter.countdown_timer_settings.prealert_time/1000;
    prealert_time = GUI_TextSelectHelper(prealert_time,
            MIN_PERALERT_TIME, MAX_PERALERT_TIME, dir, 5, 10, &changed);
    if (changed)
        Transmitter.countdown_timer_settings.prealert_time = prealert_time * 1000;
    if (prealert_time == 0)
        return _tr("Off");
    TIMER_SetString(cp->tmpstr, Transmitter.countdown_timer_settings.prealert_time);
    return cp->tmpstr;
}

static const char *timer_interval_cb(guiObject_t *obj, int dir, void *data)
{
    (void)obj;
    u16 *value = (u16 *)data;
    u8 changed;
    u16 interval = *value/1000;
    interval = GUI_TextSelectHelper(interval, 0, 60, dir, 1, 5, &changed);
    if (changed)
        *value = interval * 1000;
    if (interval == 0)
        return _tr("Off");
    sprintf(cp->tmpstr, "%d", interval);
    return cp->tmpstr;
}

static const char *poweralarm_select_cb(guiObject_t *obj, int dir, void *data)
{
    (void)data;
    (void)obj;
    u8 changed;
    Transmitter.power_alarm = GUI_TextSelectHelper(Transmitter.power_alarm,
            0 , MAX_POWER_ALARM, dir, 1 , 5 , &changed);
    if( 0 == Transmitter.power_alarm)
	return _tr("Off");
    sprintf(cp->tmpstr, "%2dmn", Transmitter.power_alarm);
    return cp->tmpstr;
}

static const char *batalarm_select_cb(guiObject_t *obj, int dir, void *data)
{
    (void)data;
    (void)obj;
    u8 changed;
    Transmitter.batt_alarm = GUI_TextSelectHelper(Transmitter.batt_alarm, 
            MIN_BATTERY_ALARM, MAX_BATTERY_ALARM, dir, MIN_BATTERY_ALARM_STEP, 500, &changed);
    sprintf(cp->tmpstr, "%2d.%02dV", Transmitter.batt_alarm / 1000, (Transmitter.batt_alarm % 1000) / 10);
    return cp->tmpstr;
}

static const char *batalarmwarn_select_cb(guiObject_t *obj, int dir, void *data)
{
    (void)obj;
    (void)data;
    u8 changed;
    u16 batt_warning = Transmitter.batt_warning_interval;
    batt_warning = GUI_TextSelectHelper(batt_warning,
            MIN_BATTERY_WARNING_INTERVAL, MAX_BATTERY_WARNING_INTERVAL, dir, 5, 30, &changed);
    if (changed)
        Transmitter.batt_warning_interval = batt_warning;
    if( 0 == batt_warning)
	return _tr("Off");
    TIMER_SetString(cp->tmpstr, Transmitter.batt_warning_interval * 1000);
    return cp->tmpstr;
}

static void press_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    cp->enable = (long)data;
#if HAS_RTC
    if (cp->enable == SET_CLOCK) {
        PAGE_ChangeByID(PAGEID_RTC);
        return;
    }
#endif
#if HAS_TOUCH
    if (cp->enable == CALIB_TOUCH)
        init_touch_calib();
    else if (cp->enable == CALIB_STICK)
#endif
        calibrate_state = CALI_CENTER; // bug fix: must reset state before calibrating
}

#ifndef NO_LANGUAGE_SUPPORT
static const char *langstr_cb(guiObject_t *obj, const void *data)
{
    (void)data;
    (void)obj;
    return _tr("Change");
}

static void lang_select_cb(guiObject_t *obj, const void *data)
{
    (void)data;
    (void)obj;
    PAGE_SetModal(1);
    LANGPage_Select(PAGE_TxConfigureInit);
}
#endif

static const char *units_cb(guiObject_t *obj, int dir, void *data)
{
    (void)obj;
    u8 changed;
    u8 mask = data ? TELEMUNIT_FAREN : TELEMUNIT_FEET;
    u8 type = (Transmitter.telem & mask) ? 1 : 0;
    type = GUI_TextSelectHelper(type, 0, 1, dir, 1, 1, &changed);
    if (changed) {
        if (type) {
            Transmitter.telem |= mask;
        } else {
            Transmitter.telem &= ~mask;
        }
    }
    if (data) {
        return type ? _tr("Fahren") : _tr("Celsius");
    } else {
        return type ? _tr("Foot") : _tr("Meter");
    }
}

void PAGE_TxConfigureEvent()
{
    switch(cp->enable) {
#if HAS_TOUCH
    case CALIB_TOUCH:
        calibrate_touch();
        break;
#endif
    case CALIB_STICK:
        calibrate_sticks();
        break;
    case CALIB_STICK_TEST:
    default: break;
    }
}

static const char *_music_shutdown_cb(guiObject_t *obj, int dir, void *data)
{
    (void)data;
    (void)obj;
    Transmitter.music_shutdown = GUI_TextSelectHelper(Transmitter.music_shutdown, 0, 1, dir, 1, 1, NULL);
    if (Transmitter.music_shutdown == 0)
        return _tr("Off");
    else
        return _tr("On");
}

static const char *_buzz_vol_cb(guiObject_t *obj, int dir, void *data)
{
    (void)data;
    u8 *unsigned_data = (u8 *)data;
    static u8 mem = 0;
    if (GUI_IsTextSelectEnabled(obj)) {
        *unsigned_data = GUI_TextSelectHelper(*unsigned_data, 0, 10, dir, 1, 1, NULL);
    }
    if( mem != *unsigned_data ) {
        mem = *unsigned_data;
        MUSIC_Play(MUSIC_VOLUME);
    }
    if (*unsigned_data == 0)
        return _tr("Off");
    sprintf(cp->tmpstr, "%d", *unsigned_data);
    return cp->tmpstr;
}
