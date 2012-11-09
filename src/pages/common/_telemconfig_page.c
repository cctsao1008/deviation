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

#define tp pagemem.u.telemconfig_page
static const char *label_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    sprintf(tp.str, "%s%d", _tr("Alarm"), (int)((long)data)+1);
    return tp.str;
}

static const char *telem_name_cb(guiObject_t *obj, int dir, void *data)
{
    (void)obj;
    int val = (long)data;
    u8 changed;
    Model.telem_alarm[val] = GUI_TextSelectHelper(Model.telem_alarm[val],
        0, TELEM_RPM2, dir, 1, 1, &changed);
    if (changed) {
        GUI_Redraw(tp.valueObj[val]);
    }
    return TELEMETRY_ShortName(tp.str, Model.telem_alarm[val]);
}

static const char *gtlt_cb(guiObject_t *obj, int dir, void *data)
{
    (void)obj;
    int val = (long)data;
    u8 changed;
    u8 type = (Model.telem_flags & (1 << val)) ? 1 : 0;
    type = GUI_TextSelectHelper(type, 0, 1, dir, 1, 1, &changed);
    if (changed) {
        if (type) {
            Model.telem_flags |= 1 << val;
        } else {
            Model.telem_flags &= ~(1 << val);
        }
    }
    return type ? "<=" : ">=";
}

static const char *limit_cb(guiObject_t *obj, int dir, void *data)
{
    (void)obj;
    int val = (long)data;
    if (Model.telem_alarm[val] == 0) {
        return "----";
    }
    s32 max = TELEMETRY_GetMaxValue(Model.telem_alarm[val]);
    
    u8 small_step = 1;
    u8 big_step = 10;
    if (Model.telem_alarm[val] == TELEM_RPM1 || Model.telem_alarm[val] == TELEM_RPM2) {
        small_step = 10;
        big_step = 100;
    }

    Model.telem_alarm_val[val] = GUI_TextSelectHelper(Model.telem_alarm_val[val],
        0, max, dir, small_step, big_step, NULL);
    return TELEMETRY_GetValueStrByValue(tp.str, Model.telem_alarm[val], Model.telem_alarm_val[val]);
}

static const char *units_cb(guiObject_t *obj, int dir, void *data)
{
    (void)obj;
    u8 changed;
    u8 mask = data ? TELEMFLAG_FAREN : TELEMFLAG_FEET;
    u8 type = (Model.telem_flags & mask) ? 1 : 0;
    type = GUI_TextSelectHelper(type, 0, 1, dir, 1, 1, &changed);
    if (changed) {
        if (type) {
            Model.telem_flags |= mask;
        } else {
            Model.telem_flags &= ~mask;
        }
        if (data) {
            //Celcius/Farenheit: redraw values
            for(int i = 0; i < TELEM_NUM_ALARMS; i++)
                GUI_Redraw(tp.valueObj[i]);
        }
    }
    if (data) {
        return type ? _tr("Fahrenheit") : _tr("Celcius");
    } else {
        return type ? _tr("Feet") : _tr("Meter");
    }
}

void PAGE_TelemconfigEvent() {
}
