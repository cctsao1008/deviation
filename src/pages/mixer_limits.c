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
#include <stdlib.h>

static struct mixer_page * const mp = &pagemem.u.mixer_page;
static void sourceselect_cb(guiObject_t *obj, void *data);
static const char *set_source_cb(guiObject_t *obj, int dir, void *data);
static const char *reverse_cb(guiObject_t *obj, int dir, void *data);
static void toggle_reverse_cb(guiObject_t *obj, void *data);
static void show_titlerow();
static const char *set_limits_cb(guiObject_t *obj, int dir, void *data);
static const char *set_trimstep_cb(guiObject_t *obj, int dir, void *data);
static const char *set_failsafe_cb(guiObject_t *obj, int dir, void *data);
static void toggle_failsafe_cb(guiObject_t *obj, void *data);

void MIXPAGE_EditLimits()
{
    PAGE_RemoveAllObjects();
    show_titlerow();
    int y = 40;
    //Row 1
    GUI_CreateLabel(8, y, NULL, DEFAULT_FONT, _tr("Reverse:"));
    GUI_CreateTextSelect(128, y, TEXTSELECT_96, 0x0000, toggle_reverse_cb, reverse_cb, (void *)((long)mp->channel));
    y += 24;
    //Row 2
    GUI_CreateLabel(8, y, NULL, DEFAULT_FONT, _tr("Failsafe:"));
    GUI_CreateTextSelect(128, y, TEXTSELECT_96, 0x0000, toggle_failsafe_cb, set_failsafe_cb, NULL);
    y += 24;
    //Row 3
    GUI_CreateLabel(8, y, NULL, DEFAULT_FONT, _tr("Safety:"));
    GUI_CreateTextSelect(128, y, TEXTSELECT_96, 0x0000, sourceselect_cb, set_source_cb, &mp->limit.safetysw);
    y += 24;
    //Row 4
    GUI_CreateLabel(8, y, NULL, DEFAULT_FONT, _tr("Safe Val:"));
    GUI_CreateTextSelect(128, y, TEXTSELECT_96, 0x0000, NULL, PAGEMIXER_SetNumberCB, &mp->limit.safetyval);
    y += 24;
    //Row 5
    GUI_CreateLabel(8, y, NULL, DEFAULT_FONT, _tr("Min:"));
    GUI_CreateTextSelect(128, y, TEXTSELECT_96, 0x0000, NULL, set_limits_cb, &mp->limit.min);
    y += 24;
    //Row 6
    GUI_CreateLabel(8, y, NULL, DEFAULT_FONT, _tr("Max:"));
    GUI_CreateTextSelect(128, y, TEXTSELECT_96, 0x0000, NULL, set_limits_cb, &mp->limit.max);
    y += 24;
    //Row 5
    GUI_CreateLabel(8, y, NULL, DEFAULT_FONT, _tr("Scale:"));
    GUI_CreateTextSelect(128, y, TEXTSELECT_96, 0x0000, NULL, set_limits_cb, &mp->limit.servoscale);
    y += 24;
    //Row 6
    GUI_CreateLabel(8, y, NULL, DEFAULT_FONT, _tr("Subtrim:"));
    GUI_CreateTextSelect(128, y, TEXTSELECT_96, 0x0000, NULL, set_trimstep_cb, &mp->limit.subtrim);
}

void sourceselect_cb(guiObject_t *obj, void *data)
{
    u8 *source = (u8 *)data;
    if(MIXER_SRC(*source)) {
        MIXER_SET_SRC_INV(*source, ! MIXER_SRC_IS_INV(*source));
        GUI_Redraw(obj);
    }
}

const char *set_source_cb(guiObject_t *obj, int dir, void *data)
{
    (void) obj;
    u8 *source = (u8 *)data;
    u8 is_neg = MIXER_SRC_IS_INV(*source);
    *source = GUI_TextSelectHelper(MIXER_SRC(*source), 0, NUM_SOURCES, dir, 1, 1, NULL);
    MIXER_SET_SRC_INV(*source, is_neg);
    GUI_TextSelectEnablePress(obj, MIXER_SRC(*source));
    return INPUT_SourceName(mp->tmpstr, *source);
}

const char *set_limits_cb(guiObject_t *obj, int dir, void *data)
{
    (void)obj;
    u8 *ptr = (u8 *)data;
    int value = *ptr;
    if (ptr == &mp->limit.min) {
        value = GUI_TextSelectHelper(-value, -250, 0, dir, 1, 5, NULL);
        *ptr = -value;
    } else {
        value = GUI_TextSelectHelper(value, 0, 250, dir, 1, 5, NULL);
        *ptr = value;
    }
    sprintf(mp->tmpstr, "%d", value);
    return mp->tmpstr;
}

const char *set_trimstep_cb(guiObject_t *obj, int dir, void *data)
{
    (void)obj;
    s8 *value = (s8 *)data;
    *value = GUI_TextSelectHelper(*value, -100, 100, dir, 1, 5, NULL);
    sprintf(mp->tmpstr, "%s%d.%d", *value < 0 ? "-" : "", abs(*value) / 10, abs(*value) % 10);
    return mp->tmpstr;
}

void toggle_failsafe_cb(guiObject_t *obj, void *data)
{
    (void)obj;
    (void)data;
    mp->limit.flags = (mp->limit.flags & CH_FAILSAFE_EN)
          ? (mp->limit.flags & ~CH_FAILSAFE_EN)
          : (mp->limit.flags | CH_FAILSAFE_EN);
}

const char *set_failsafe_cb(guiObject_t *obj, int dir, void *data)
{
    (void)obj;
    (void)data;
    if (!(mp->limit.flags & CH_FAILSAFE_EN))
        return _tr("Off");
    return PAGEMIXER_SetNumberCB(obj, dir, &mp->limit.failsafe);
}

static void okcancel_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    if (data) {
        //Save mixer here
        MIXER_SetLimit(mp->channel, &mp->limit);
    }
    GUI_RemoveAllObjects();
    PAGE_MixerInit(mp->top_channel);
}

static void show_titlerow()
{
    GUI_CreateLabel(8, 10, MIXPAGE_ChanNameProtoCB, TITLE_FONT, (void *)(long)mp->channel);
    PAGE_CreateCancelButton(160, 4, okcancel_cb);
    PAGE_CreateOkButton(264, 4, okcancel_cb);
}

const char *reverse_cb(guiObject_t *obj, int dir, void *data)
{
    (void)obj;
    (void)dir;
    (void)data;
    if (dir > 0)
        mp->limit.flags |= CH_REVERSE;
    else if (dir < 0)
        mp->limit.flags &= ~CH_REVERSE;
    return (mp->limit.flags & CH_REVERSE) ? _tr("Reversed") : _tr("Normal");
}

void toggle_reverse_cb(guiObject_t *obj, void *data)
{
    (void)obj;
    (void)data;
    mp->limit.flags = (mp->limit.flags & CH_REVERSE)
          ? (mp->limit.flags & ~CH_REVERSE)
          : (mp->limit.flags | CH_REVERSE);
}

