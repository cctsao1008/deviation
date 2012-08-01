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

static struct chantest_page * const cp = &pagemem.u.chantest_page;

static s16 showchan_cb(void *data);
static const char *value_cb(guiObject_t *obj, void *data);
static const char *channum_cb(guiObject_t *obj, void *data);

static void show_page()
{
    #define SEPERATION 36
    int i;
    u8 height;
    u8 count;
    if (Model.num_channels > 8) {
        height = 70;
        count = (Model.num_channels + 1) / 2;
    } else {
        height = 155;
        count = Model.num_channels;
    }
  
    u16 offset = (320 + (SEPERATION - 10) - SEPERATION * count) / 2;
    for(i = 0; i < count; i++) {
        GUI_CreateLabelBox(offset + SEPERATION * i - (SEPERATION - 10)/2, 40,
                                      SEPERATION, 10, &TINY_FONT, channum_cb, NULL, (void *)((long)i));
        cp->bar[i] = GUI_CreateBarGraph(offset + SEPERATION * i, 50, 10, height,
                                    CHAN_MIN_VALUE, CHAN_MAX_VALUE, BAR_VERTICAL,
                                    showchan_cb, (void *)((long)i));
        cp->value[i] = GUI_CreateLabelBox(offset + SEPERATION * i - (SEPERATION - 10)/2, 53 + height,
                                      SEPERATION, 10, &TINY_FONT, value_cb, NULL, (void *)((long)i));
        cp->pctvalue[i] = RANGE_TO_PCT(Channels[i]);
    }
    for(i = count; i < Model.num_channels; i++) {
        GUI_CreateLabelBox(offset + SEPERATION * (i - count) - (SEPERATION - 10)/2, 215 - height,
                                      SEPERATION, 10, &TINY_FONT, channum_cb, NULL, (void *)((long)i));
        cp->bar[i] = GUI_CreateBarGraph(offset + SEPERATION * (i - count), 224 - height, 10, height,
                                    CHAN_MIN_VALUE, CHAN_MAX_VALUE, BAR_VERTICAL,
                                    showchan_cb, (void *)((long)i));
        cp->value[i] = GUI_CreateLabelBox(offset + SEPERATION * (i - count) - (SEPERATION - 10)/2, 225,
                                      SEPERATION, 10, &TINY_FONT, value_cb, NULL, (void *)((long)i));
        cp->pctvalue[i] = RANGE_TO_PCT(Channels[i]);
    }
}
void PAGE_ChantestInit(int page)
{
    (void)page;
    PAGE_SetModal(0);
    PAGE_ShowHeader("Channels");
    cp->return_page = NULL;
    show_page();
}

static void okcancel_cb(guiObject_t *obj, void *data)
{
    (void)obj;
    (void)data;
    if(cp->return_page) {
        PAGE_SetModal(0);
        PAGE_RemoveAllObjects();
        cp->return_page(1);
    }
}

void PAGE_ChantestModal(void(*return_page)(int page))
{
    PAGE_SetModal(1);
    cp->return_page = return_page;
    PAGE_RemoveAllObjects();

    //PAGE_CreateOkButton(264, 4, okcancel_cb);
    GUI_CreateIcon(0, 0, &icons[ICON_EXIT], okcancel_cb, (void *)0);
    GUI_CreateLabel(40, 10, NULL, TITLE_FONT, "Channels");

    show_page();
}

void PAGE_ChantestEvent()
{
    int i;
    for(i = 0; i < Model.num_channels; i++) {
        s8 v = RANGE_TO_PCT(Channels[i]);
        if (v != cp->pctvalue[i]) {
            GUI_Redraw(cp->bar[i]);
            GUI_Redraw(cp->value[i]);
            cp->pctvalue[i] = v;
        }
    }
}

static s16 showchan_cb(void *data)
{
    long ch = (long)data;
    return Channels[ch];
}

static const char *value_cb(guiObject_t *obj, void *data)
{
    (void)obj;
    long ch = (long)data;
    sprintf(cp->tmpstr, "%d", cp->pctvalue[ch]);
    return cp->tmpstr;
}

static const char *channum_cb(guiObject_t *obj, void *data)
{
    (void)obj;
    long ch = (long)data;
    sprintf(cp->tmpstr, "%d", (int)ch+1);
    return cp->tmpstr;
}
