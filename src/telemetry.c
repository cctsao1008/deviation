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
#include "music.h"
#include "config/model.h"
#include "config/tx.h"
#include "telemetry.h"

static void _get_volt_str(char *str, u32 value);
static void _get_temp_str(char *str, int value);
#include "telemetry/telem_devo.c"
#include "telemetry/telem_dsm.c"

#define CAP_DSM 1

struct Telemetry Telemetry;
static u32 alarm_duration[TELEM_NUM_ALARMS] = {0, 0, 0, 0, 0, 0};
static u8 telem_idx = 0;
static u8 alarm = 0;
static u32 alarm_time = 0;
static u8 last_updated[TELEM_UPDATE_SIZE];
static u32 last_time;
#define CHECK_DURATION 500
#define MUSIC_INTERVAL 2000 // DON'T need to play music in every 100ms

void _get_volt_str(char *str, u32 value)
{
    sprintf(str, "%d.%dV", (int)value /10, (int)value % 10);
}

void _get_temp_str(char *str, int value)
{
    if (value == 0) {
        strcpy(str, "----");
    } else {
        if (Transmitter.telem & TELEMUNIT_FAREN) {
            sprintf(str, "%dF", ((int)value * 9 + 160)/ 5);
        } else {
            sprintf(str, "%dC", (int)value);
        }
    }
}
u32 TELEMETRY_IsUpdated(int val)
{
    if (val == 0xff) {
        for(int i = 0; i < TELEM_UPDATE_SIZE; i++) {
            if (last_updated[i] | Telemetry.updated[i])
                return 1;
        }
        return 0;
    }
    return (last_updated[val / 8] | Telemetry.updated[val / 8]) & (1 << val % 8);
}

s32 TELEMETRY_GetValue(int idx)
{
    return _TELEMETRY_GetValue(&Telemetry, idx);
}

s32 _TELEMETRY_GetValue(struct Telemetry *t, int idx)
{
    switch (idx) {
    case TELEM_GPS_LONG:
        return t->gps.longitude;
    case TELEM_GPS_LAT:
        return t->gps.latitude;
    case TELEM_GPS_ALT:
        return t->gps.altitude;
    case TELEM_GPS_SPEED:
        return t->gps.velocity;
    case TELEM_GPS_TIME:
        return t->gps.time;
    }
    return TELEMETRY_Type() == TELEM_DEVO ? _devo_value(t, idx) : _dsm_value(t, idx);
}

const char * TELEMETRY_GetValueStrByValue(char *str, unsigned telem, s32 value)
{
    int h, m, s, ss;
    char letter = ' ';
    int unit = 0;    // rBE-OPT: Optimizing string usage, saves some bytes
    switch(telem) {
        case TELEM_GPS_LONG:
            // allowed values: +/-180° = +/- 180*60*60*1000; W if value<0, E if value>=0; -180° = 180°
            if (value < 0) {
                letter = 'W';
                value = -value;
            } else
                letter = 'E';
            h = value / 1000 / 60 / 60;
            m = (value - h * 1000 * 60 * 60) / 1000 / 60;
            s = (value - h * 1000 * 60 * 60 - m * 1000 * 60) / 1000;
            ss = value % 1000;
            sprintf(str, "%c %3d° %02d' %02d.%03d\"", letter, h, m, s, ss);
            break;
        case TELEM_GPS_LAT:
            // allowed values: +/-90° = +/- 90*60*60*1000; S if value<0, N if value>=0
            if (value < 0) {
                letter = 'S';
                value = -value;
            } else
                letter = 'N';
            h = value / 1000 / 60 / 60;
            m = (value - h * 1000 * 60 * 60) / 1000 / 60;
            s = (value - h * 1000 * 60 * 60 - m * 1000 * 60) / 1000;
            ss = value % 1000;
            sprintf(str, "%c %3d° %02d' %02d.%03d\"", letter, h, m, s, ss);
            break;
        case TELEM_GPS_ALT:
            if (Transmitter.telem & TELEMUNIT_FEET) {
                value = value * 328 / 100;
                if (value < 0) {
                    letter = '-';
                    value =-value;
                }
                unit = 1;
            }
            sprintf(str, "%c%d.%03d%s", letter, (int)value / 1000, (int)value % 1000, unit ? "ft" : "m");
            break;
        case TELEM_GPS_SPEED:
            if (Transmitter.telem & TELEMUNIT_FEET) {
                value = value * 2237 / 1000;
                unit = 1;
            }
            sprintf(str, "%d.%03d%s", (int)value / 1000, (int)value % 1000, unit ? "mph" : "m/s");
            break;
        case TELEM_GPS_TIME:
        {
            int year = 2000 + (((u32)Telemetry.gps.time >> 26) & 0x3F);
            int month = ((u32)Telemetry.gps.time >> 22) & 0x0F;
            int day = ((u32)Telemetry.gps.time >> 17) & 0x1F;
            int hour = ((u32)Telemetry.gps.time >> 12) & 0x1F;
            int min = ((u32)Telemetry.gps.time >> 6) & 0x3F;
            int sec = ((u32)Telemetry.gps.time >> 0) & 0x3F;
            sprintf(str, "%2d:%02d:%02d %4d-%02d-%02d",
                    hour, min, sec, year, month, day);
            break;
        }
        default:
           return TELEMETRY_Type() == TELEM_DEVO
                  ? _devo_str_by_value(str, telem, value)
                  : _dsm_str_by_value(str, telem, value);
    }
    return str;
}

const char * TELEMETRY_GetValueStr(char *str, unsigned telem)
{
    s32 value = TELEMETRY_GetValue(telem);
    return TELEMETRY_GetValueStrByValue(str, telem, value);
}

const char * TELEMETRY_Name(char *str, unsigned telem)
{
   return (TELEMETRY_Type() == TELEM_DEVO)
          ? _devo_name(str, telem)
          : _dsm_name(str, telem);
}

const char * TELEMETRY_ShortName(char *str, unsigned telem)
{
    switch(telem) {
        case TELEM_GPS_LONG:   strcpy(str, _tr("Longitude")); break;
        case TELEM_GPS_LAT:    strcpy(str, _tr("Latitude")); break;
        case TELEM_GPS_ALT:    strcpy(str, _tr("Altitude")); break;
        case TELEM_GPS_SPEED:  strcpy(str, _tr("Speed")); break;
        case TELEM_GPS_TIME:   strcpy(str, _tr("Time")); break;
        default:
            return TELEMETRY_Type() == TELEM_DEVO
                   ? _devo_short_name(str, telem)
                   : _dsm_short_name(str, telem);
    }
    return str;
}
s32 TELEMETRY_GetMaxValue(unsigned telem)
{
   return TELEMETRY_Type() == TELEM_DEVO
          ? _devo_get_max_value(telem)
          : _dsm_get_max_value(telem);
}

void TELEMETRY_SetUpdated(int telem)
{
    Telemetry.updated[telem/8] |= (1 << telem % 8);
}

int TELEMETRY_Type()
{
    return (Telemetry.capabilities & CAP_DSM) ? TELEM_DSM : TELEM_DEVO;
}

void TELEMETRY_SetTypeByProtocol(enum Protocols protocol)
{
    if (protocol == PROTOCOL_DSM2 || protocol == PROTOCOL_DSMX)
        TELEMETRY_SetType(TELEM_DSM);
    else
        TELEMETRY_SetType(TELEM_DEVO);
}

void TELEMETRY_SetType(int type)
{
    if (type == TELEM_DSM)
        Telemetry.capabilities |= CAP_DSM;
    else
        Telemetry.capabilities &= ~CAP_DSM;
}

//#define DEBUG_TELEMALARM
void TELEMETRY_Alarm()
{
    //Update 'updated' state every time we get here
    u32 current_time = CLOCK_getms();
    if (current_time - last_time > TELEM_ERROR_TIME) {
        last_time = current_time;
        for(int i = 0; i < TELEM_UPDATE_SIZE; i++) {
            last_updated[i] = Telemetry.updated[i];
            Telemetry.updated[i] = 0;
        }
    }
    // don't need to check all the 6 telem-configs at one time, this is not a critical and urgent task
    // instead, check 1 of them at a time
    telem_idx = (telem_idx + 1) % TELEM_NUM_ALARMS;
    if(! Model.telem_alarm[telem_idx]) {
        alarm &= ~(1 << telem_idx); // clear this set
        return;
    }
    unsigned idx = Model.telem_alarm[telem_idx];
    s32 value = TELEMETRY_GetValue(idx);
    if (value == 0) {
        alarm &= ~(1 << telem_idx); // clear this set
        return;
    }

    if (! TELEMETRY_IsUpdated(0xff)) {
        // bug fix: do not alarm when no telem packet is received, it might caused by RX is powered off
        alarm &= ~(1 << telem_idx); // clear this set
        return;
    }

    if (Model.telem_flags & (1 << telem_idx)) {
        if (! (alarm & (1 << telem_idx)) && (value <= Model.telem_alarm_val[telem_idx])) {
            if (alarm_duration[telem_idx] == 0) {
                alarm_duration[telem_idx] = current_time;
            } else if (current_time - alarm_duration[telem_idx] > CHECK_DURATION) {
                alarm_duration[telem_idx] = 0;
                alarm |= 1 << telem_idx;
#ifdef DEBUG_TELEMALARM
                printf("set: 0x%x\n\n", alarm);
#endif
            }
        } else if ((alarm & (1 << telem_idx)) && (value > (s32)Model.telem_alarm_val[telem_idx])) {
            if (alarm_duration[telem_idx] == 0) {
                alarm_duration[telem_idx] = current_time;
            } else if (current_time - alarm_duration[telem_idx] > CHECK_DURATION) {
                alarm_duration[telem_idx] = 0;
                alarm &= ~(1 << telem_idx);
#ifdef DEBUG_TELEMALARM
                printf("clear: 0x%x\n\n", alarm);
#endif
            }
        } else
            alarm_duration[telem_idx] = 0;
    } else {
        if (! (alarm & (1 << telem_idx)) && (value >= Model.telem_alarm_val[telem_idx])) {
            if (alarm_duration[telem_idx] == 0) {
                alarm_duration[telem_idx] = current_time;
            } else if (current_time - alarm_duration[telem_idx] > CHECK_DURATION) {
                alarm_duration[telem_idx] = 0;
                alarm |= 1 << telem_idx;
#ifdef DEBUG_TELEMALARM
                printf("set: 0x%x\n\n", alarm);
#endif
            }
        } else if ((alarm & (1 << telem_idx)) && (value < (s32)Model.telem_alarm_val[telem_idx])) {
            if (alarm_duration[telem_idx] == 0) {
                alarm_duration[telem_idx] = current_time;
            } else if (current_time - alarm_duration[telem_idx] > CHECK_DURATION) {
                alarm_duration[telem_idx] = 0;
                alarm &= ~(1 << telem_idx);
#ifdef DEBUG_TELEMALARM
                printf("clear: 0x%x\n\n", alarm);
#endif
            }
        } else
            alarm_duration[telem_idx] = 0;
    }

    if ((alarm & (1 << telem_idx))) {
        if (current_time >= alarm_time + MUSIC_INTERVAL) {
            alarm_time = current_time;
#ifdef DEBUG_TELEMALARM
            printf("beep: %d\n\n", telem_idx);
#endif
            MUSIC_Play(MUSIC_TELEMALARM1 + telem_idx);
        }
    }
}

int TELEMETRY_HasAlarm(int src)
{
    for(int i = 0; i < TELEM_NUM_ALARMS; i++)
        if(Model.telem_alarm[i] == src && (alarm & (1 << i)))
            return 1;
    return 0;
}
