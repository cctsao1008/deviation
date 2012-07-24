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

static struct mixer_page * const mp = &pagemem.u.mixer_page;
static void sourceselect_cb(guiObject_t *obj, void *data);
static const char *set_source_cb(guiObject_t *obj, int dir, void *data);
static const char *reverse_cb(guiObject_t *obj, int dir, void *data);
static void toggle_reverse_cb(guiObject_t *obj, void *data);
static void show_titlerow();
static const char *set_limits_cb(guiObject_t *obj, int dir, void *data);
static const char *set_failsafe_cb(guiObject_t *obj, int dir, void *data);
static void toggle_failsafe_cb(guiObject_t *obj, void *data);

void MIXPAGE_EditLimits()
{
    GUI_RemoveAllObjects();
    show_titlerow();
    //Row 1
    GUI_CreateLabel(8, 40, NULL, DEFAULT_FONT, "Reverse:");
    GUI_CreateTextSelect(64, 40, TEXTSELECT_96, 0x0000, toggle_reverse_cb, reverse_cb, (void *)((long)mp->channel));
    //Row 2
    GUI_CreateLabel(8, 64, NULL, DEFAULT_FONT, "Failsafe:");
    GUI_CreateTextSelect(64, 64, TEXTSELECT_96, 0x0000, toggle_failsafe_cb, set_failsafe_cb, NULL);
    //Row 3
    GUI_CreateLabel(8, 88, NULL, DEFAULT_FONT, "Safety:");
    GUI_CreateTextSelect(64, 88, TEXTSELECT_96, 0x0000, sourceselect_cb, set_source_cb, &mp->limit.safetysw);
    GUI_CreateLabel(176, 88, NULL, DEFAULT_FONT, "Value:");
    GUI_CreateTextSelect(216, 88, TEXTSELECT_96, 0x0000, NULL, PAGEMIX_SetNumberCB, &mp->limit.safetyval);
    //Row 4
    GUI_CreateLabel(8, 112, NULL, DEFAULT_FONT, "Min:");
    GUI_CreateTextSelect(64, 112, TEXTSELECT_96, 0x0000, NULL, set_limits_cb, &mp->limit.min);
    GUI_CreateLabel(176, 112, NULL, DEFAULT_FONT, "Max:");
    GUI_CreateTextSelect(216, 112, TEXTSELECT_96, 0x0000, NULL, set_limits_cb, &mp->limit.max);
}

void sourceselect_cb(guiObject_t *obj, void *data)
{
    u8 *source = (u8 *)data;
    MIX_SET_SRC_INV(*source, ! MIX_SRC_IS_INV(*source));
    GUI_Redraw(obj);
}

const char *set_source_cb(guiObject_t *obj, int dir, void *data)
{
    (void) obj;
    u8 *source = (u8 *)data;
    u8 is_neg = MIX_SRC_IS_INV(*source);
    *source = GUI_TextSelectHelper(MIX_SRC(*source), 0, NUM_INPUTS + NUM_CHANNELS, dir, 1, 1, NULL);
    MIX_SET_SRC_INV(*source, is_neg);
    return MIXER_SourceName(mp->tmpstr, *source);
}

const char *set_limits_cb(guiObject_t *obj, int dir, void *data)
{
    (void)obj;
    s8 *value = (s8 *)data;
    if (value == &mp->limit.min)
        *value = GUI_TextSelectHelper(*value, -125, mp->limit.max, dir, 1, 5, NULL);
    else
        *value = GUI_TextSelectHelper(*value, mp->limit.min, 125, dir, 1, 5, NULL);
    sprintf(mp->tmpstr, "%d", *value);
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
        return "Off";
    return PAGEMIX_SetNumberCB(obj, dir, &mp->limit.failsafe);
}

static void okcancel_cb(guiObject_t *obj, void *data)
{
    (void)obj;
    if (data) {
        //Save mixer here
        MIX_SetLimit(mp->channel, &mp->limit);
        MIX_SetTemplate(mp->channel, mp->cur_template);
        MIX_SetMixers(mp->mixer, mp->num_mixers);
    }
    GUI_RemoveAllObjects();
    PAGE_MixerInit(0);
}

static void show_titlerow()
{
    GUI_CreateLabel(8, 10, MIXPAGE_ChannelNameCB, TITLE_FONT, NULL);
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
    return (mp->limit.flags & CH_REVERSE) ? "Reversed" : "Normal";
}

void toggle_reverse_cb(guiObject_t *obj, void *data)
{
    (void)obj;
    (void)data;
    mp->limit.flags = (mp->limit.flags & CH_REVERSE)
          ? (mp->limit.flags & ~CH_REVERSE)
          : (mp->limit.flags | CH_REVERSE);
}

