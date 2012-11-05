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
#include "config/model.h"
#include "icons.h"

#include "../common/_mixer_page.c"

#define ENTRIES_PER_PAGE (8 > NUM_CHANNELS ? NUM_CHANNELS : 8)
static u8 scroll_cb(guiObject_t *parent, u8 pos, s8 direction, void *data);

static void _show_title(int page)
{
    mp->max_scroll = Model.num_channels + NUM_VIRT_CHANNELS > ENTRIES_PER_PAGE ? Model.num_channels + NUM_VIRT_CHANNELS - ENTRIES_PER_PAGE : Model.num_channels + NUM_VIRT_CHANNELS;
    PAGE_ShowHeader(_tr("Mixer"));
    GUI_CreateIcon(192, 0, &icons[ICON_CHANTEST], show_chantest_cb, NULL);
    GUI_CreateIcon(224, 0, &icons[ICON_ORDER], reorder_cb, NULL);
    guiObject_t *obj = GUI_CreateScrollbar(304, 32, 208, mp->max_scroll, NULL, scroll_cb, NULL);
    GUI_SetScrollbar(obj, page);
}

static void _show_page()
{
    int init_y = 40;
    int i;
    if (mp->firstObj) {
        GUI_RemoveHierObjects(mp->firstObj);
        mp->firstObj = NULL;
    }
    struct Mixer *mix = MIXER_GetAllMixers();
    for (i = 0; i < ENTRIES_PER_PAGE; i++) {
        guiObject_t *obj;
        u8 idx;
        int row = init_y + 24 * i;
        u8 ch = mp->top_channel + i;
        if (ch >= Model.num_channels)
            ch += (NUM_OUT_CHANNELS - Model.num_channels);
        if (ch < NUM_OUT_CHANNELS)
            obj = GUI_CreateButton(4, row, BUTTON_64x16, MIXPAGE_ChanNameProtoCB,
                                   0x0000, limitselect_cb, (void *)((long)ch));
        else
            obj = GUI_CreateLabelBox(4, row, 64, 16, &DEFAULT_FONT,
                                   MIXPAGE_ChanNameProtoCB, NULL, (void *)((long)ch));
        if (! mp->firstObj)
            mp->firstObj = obj;
        GUI_CreateButton(132, row, BUTTON_64x16, template_name_cb, 0x0000,
                         templateselect_cb, (void *)((long)ch));
        for (idx = 0; idx < NUM_MIXERS; idx++)
            if (mix[idx].src && mix[idx].dest == ch)
                break;
        if (idx != NUM_MIXERS) {
            enum TemplateType template = MIXER_GetTemplate(ch);
            GUI_CreateLabelBox(68, row, 60, 16, &NARROW_FONT, show_source, NULL, &mix[idx].src);
            if (template == MIXERTEMPLATE_EXPO_DR) {
                if (mix[idx].src == mix[idx+1].src && mix[idx].dest == mix[idx+1].dest && mix[idx+1].sw) {
                    GUI_CreateLabelBox(200, row, 52, 16, &SMALL_FONT, show_source, NULL, &mix[idx+1].sw);
                }
                if (mix[idx].src == mix[idx+2].src && mix[idx].dest == mix[idx+2].dest && mix[idx+2].sw) {
                    GUI_CreateLabelBox(252, row, 52, 16, &SMALL_FONT, show_source, NULL, &mix[idx+2].sw);
                }
            }
        }
    }
}

static void _determine_save_in_live()
{
    //only support in devo10, do nothing for devo8
}

static u8 scroll_cb(guiObject_t *parent, u8 pos, s8 direction, void *data) {
    (void)parent;
    (void)data;
    s16 newpos;
    if (direction > 0) {
        newpos = pos + (direction > 1 ? ENTRIES_PER_PAGE : 1);
        if (newpos > mp->max_scroll)
            newpos = mp->max_scroll;
    } else {
        newpos = pos - (direction < -1 ? ENTRIES_PER_PAGE : 1);
        if (newpos < 0)
            newpos = 0;
    }
    if (newpos != pos) {
        mp->top_channel = newpos;
        _show_page();
    }
    return newpos;
}

