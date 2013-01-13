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
#include "config/model.h"

enum {
    COMPLEX_MIXER,
    COMPLEX_PAGE,
    COMPLEX_SWITCH,
    COMPLEX_MUX,
    COMPLEX_SRC,
    COMPLEX_CURVE,
    COMPLEX_SCALE,
    COMPLEX_OFFSET,
    COMPLEX_TRIM,
    COMPLEX_LAST,
};

#include "../../common/advanced/_mixer_setup.c"

#define FIRST_PAGE_ITEM_IDX  2  // 0 is the template obj and 1 is the button obj
#define LEFT_VIEW_WIDTH  60
#define LEFT_VIEW_ID 0
#define RIGHT_VIEW_ID 1
#define RIGHT_VIEW_HEIGHT 49
static s8 current_selected_item = 0;

static u8 action_cb(u32 button, u8 flags, void *data);
static void notify_cb(guiObject_t * obj);

static void _show_titlerow()
{
    PAGE_SetActionCB(action_cb);
    mp->entries_per_page = 2;
    memset(gui, 0, sizeof(*gui));

    labelDesc.style = LABEL_UNDERLINE;
    labelDesc.font_color = labelDesc.fill_color = labelDesc.outline_color = 0xffff;
    GUI_CreateLabelBox(&gui->chan, 0, 0 , LCD_WIDTH, ITEM_HEIGHT, &labelDesc,
            MIXPAGE_ChanNameProtoCB, NULL, (void *)((long)mp->cur_mixer->dest));
    u8 x =40;
    u8 w = 50;
    labelDesc.style = LABEL_CENTER;
    GUI_CreateTextSelectPlate(&gui->tmpl, x, 0,  w, ITEM_HEIGHT, &labelDesc, NULL, templatetype_cb, (void *)((long)mp->channel));
    w = 38;
    GUI_CreateButtonPlateText(&gui->save, LCD_WIDTH - w, 0, w, ITEM_HEIGHT, &labelDesc, NULL, 0, okcancel_cb, (void *)_tr("Save"));
}

static guiObject_t *simple_getobj_cb(int relrow, int col, void *data)
{
    (void)data;
    (void)col;
    return (guiObject_t *)&gui->value[relrow];
}
enum {
    SIMPLE_SRC,
    SIMPLE_CURVE,
    SIMPLE_SCALE,
    SIMPLE_OFFSET,
    SIMPLE_LAST,
};
static int simple_row_cb(int absrow, int relrow, int y, void *data)
{
    const char *label = NULL;
    void *tgl = NULL;
    void *value = NULL;
    data = NULL;
    switch(absrow) {
        case SIMPLE_SRC:
            label = _tr_noop("Src:");
            tgl = sourceselect_cb; value = set_source_cb; data = &mp->mixer[0].src;
            break;
        case SIMPLE_CURVE:
            label = _tr_noop("Curve:");
            tgl = curveselect_cb; value = set_curvename_cb; data = &mp->mixer[0];
            break;
        case SIMPLE_SCALE:
            label = _tr_noop("Scale:");
            value = set_number100_cb; data = &mp->mixer[0].scalar;
            break;
        case SIMPLE_OFFSET:
            label = _tr_noop("Offset:");
            value = set_number100_cb; data = &mp->mixer[0].offset;
            break;
    }
    int x = 0;
    u8 w = LEFT_VIEW_WIDTH;
    labelDesc.style = LABEL_LEFTCENTER;
    GUI_CreateLabelBox(&gui->label[relrow], x, y, w, ITEM_HEIGHT,
            &labelDesc, NULL, NULL, _tr(label));
    labelDesc.style = LABEL_CENTER;
    GUI_CreateTextSelectPlate(&gui->value[relrow].ts, x, y + ITEM_HEIGHT + 1,
            w, ITEM_HEIGHT, &labelDesc, tgl, value, data);
    return 1;
}
static void _show_simple()
{
    GUI_SelectionNotify(NULL);
    GUI_Select1stSelectableObj(); // bug fix: muset reset to 1st selectable item, otherwise ,the focus will be wrong
    GUI_CreateScrollable(&gui->scrollable, 0, ITEM_HEIGHT + 1, LEFT_VIEW_WIDTH + ARROW_WIDTH, LCD_HEIGHT - ITEM_HEIGHT -1,
                         2 * ITEM_SPACE, SIMPLE_LAST, simple_row_cb, simple_getobj_cb, NULL, NULL);
    // The following items are not draw in the logical view;
    GUI_CreateXYGraph(&gui->graph, 77, LCD_HEIGHT - RIGHT_VIEW_HEIGHT - 1, RIGHT_VIEW_HEIGHT, RIGHT_VIEW_HEIGHT,
                              CHAN_MIN_VALUE, CHAN_MIN_VALUE,
                              CHAN_MAX_VALUE, CHAN_MAX_VALUE,
                              0, 0, eval_mixer_cb, curpos_cb, touch_cb,
                              &mp->mixer[0]);
    OBJ_SET_USED(&gui->bar, 0);
}

static int complex_size_cb(int absrow, void *data) {
    (void)data;
    return (absrow == COMPLEX_TRIM) ? 2 : 1;
}

static int complex_row_cb(int absrow, int relrow, int y, void *data)
{
    const char *label = NULL;
    void *tgl = NULL;
    void *value = NULL;
    data = NULL;
    int x = 0;
    int w = LEFT_VIEW_WIDTH;
    if (absrow == COMPLEX_TRIM) {
        GUI_CreateButtonPlateText(&gui->value[relrow].but, x, y,
            w, ITEM_HEIGHT, &labelDesc, show_trim_cb, 0x0000, toggle_trim_cb, NULL);
        if (! MIXER_SourceHasTrim(MIXER_SRC(mp->mixer[0].src)))
            GUI_SetHidden((guiObject_t *)&gui->label[relrow], 1);
        return 1;
    }
    switch(absrow) {
        case COMPLEX_MIXER:
            label = _tr_noop("Mixers:");
            value = set_nummixers_cb;
            break;
        case COMPLEX_PAGE:
            label = _tr_noop("Page:");
            tgl = reorder_cb; value = set_mixernum_cb;
            break;
        case COMPLEX_SWITCH:
            label = _tr_noop("Switch:");
            tgl = sourceselect_cb; value = set_drsource_cb; data = &mp->cur_mixer->sw;
            break;
        case COMPLEX_MUX:
            label = _tr_noop("Mux:");
            value = set_mux_cb;
            break;
        case COMPLEX_SRC:
            label = _tr_noop("Src:");
            tgl = sourceselect_cb; value = set_source_cb; data = &mp->cur_mixer->src;
            break;
        case COMPLEX_CURVE:
            label = _tr_noop("Curve:");
            tgl = curveselect_cb; value = set_curvename_cb; data = mp->cur_mixer;
            break;
        case COMPLEX_SCALE:
            label = _tr_noop("Scale:");
            value = set_number100_cb; data = &mp->cur_mixer->scalar;
            break;
        case COMPLEX_OFFSET:
            label = _tr_noop("Offset:");
            value = set_number100_cb; data = &mp->cur_mixer->offset;
            break;
    }
    labelDesc.style = LABEL_LEFTCENTER;
    GUI_CreateLabelBox(&gui->label[relrow], x, y, w, ITEM_HEIGHT,
            &labelDesc, NULL, NULL, _tr(label));
    labelDesc.style = LABEL_CENTER;
    GUI_CreateTextSelectPlate(&gui->value[relrow].ts, x, y + ITEM_HEIGHT + 1,
            w, ITEM_HEIGHT, &labelDesc, tgl, value, data);
    return 1;
}

static void _show_complex()
{
    GUI_SelectionNotify(NULL);
    mp->max_scroll = 2;
    GUI_Select1stSelectableObj(); // bug fix: muset reset to 1st selectable item, otherwise ,the focus will be wrong

    GUI_CreateScrollable(&gui->scrollable, 0, ITEM_HEIGHT + 1, LEFT_VIEW_WIDTH + ARROW_WIDTH, LCD_HEIGHT - ITEM_HEIGHT -1,
                         2 * ITEM_SPACE, COMPLEX_LAST, complex_row_cb, simple_getobj_cb, complex_size_cb, NULL);
    // The following items are not draw in the logical view;
    GUI_CreateBarGraph(&gui->bar, LEFT_VIEW_WIDTH +10, LCD_HEIGHT - RIGHT_VIEW_HEIGHT -1, 5, RIGHT_VIEW_HEIGHT,
                              CHAN_MIN_VALUE, CHAN_MAX_VALUE, BAR_VERTICAL, eval_chan_cb, NULL);
    GUI_CreateXYGraph(&gui->graph, 77, LCD_HEIGHT - RIGHT_VIEW_HEIGHT -1, RIGHT_VIEW_HEIGHT, RIGHT_VIEW_HEIGHT,
                                  CHAN_MIN_VALUE, CHAN_MIN_VALUE,
                                  CHAN_MAX_VALUE, CHAN_MAX_VALUE,
                                  0, 0, eval_mixer_cb, curpos_cb, touch_cb, mp->cur_mixer);
}

enum {
    EXPO_SRC,
    EXPO_HIGHCURVE,
    EXPO_SCALE,
    EXPO_SWITCH1,
    EXPO_LINK1,
    EXPO_CURVE1,
    EXPO_SCALE1,
    EXPO_SWITCH2,
    EXPO_LINK2,
    EXPO_CURVE2,
    EXPO_SCALE2,
    EXPO_LAST,
};

static int expo_size_cb(int absrow, void *data)
{
    (void)data;
    switch(absrow) {
        case EXPO_LINK1:
        case EXPO_CURVE1:
        case EXPO_LINK2:
        case EXPO_CURVE2:
            return 1;
    }
    return 2;
}
static int expo_row_cb(int absrow, int relrow, int y, void *data)
{
    const char *label = NULL;
    int underline = 0;
    void * label_cb = NULL;
    void *tgl = NULL;
    void *value = NULL;
    data = NULL;
    void *but_tgl = NULL;
    void *but_txt = NULL;
    void *but_data = NULL;
    int disable = 0;

    int x = 0;
    int w = LEFT_VIEW_WIDTH;
    switch(absrow) {
        case EXPO_SRC:
            label = _tr("Src:");
            tgl = sourceselect_cb; value = set_source_cb; data = &mp->cur_mixer->src;
            break;
        case EXPO_HIGHCURVE:
            label = _tr("High-Rate");
            tgl = curveselect_cb; value = set_curvename_cb; data = &mp->mixer[0];
            break;
        case EXPO_SCALE:
            label = (void *)0; label_cb = scalestring_cb;
            value = set_number100_cb; data = &mp->mixer[0].scalar;
            break;
        case EXPO_SWITCH1:
            label = _tr("Switch1"); underline = 1;
            tgl = sourceselect_cb; value = set_drsource_cb; data = &mp->mixer[1].sw;
            break;
        case EXPO_LINK1:
            but_tgl = toggle_link_cb; but_txt = show_rate_cb; but_data = (void *)0;
            if(! MIXER_SRC(mp->mixer[1].sw))
                disable = 1;
            break;
        case EXPO_CURVE1:
            tgl = curveselect_cb; value = set_curvename_cb; data = &mp->mixer[1];
            if(! MIXER_SRC(mp->mixer[1].sw) || mp->link_curves & 0x01)
                disable = 1;
            break;
        case EXPO_SCALE1:
            label = (void *)1; label_cb = scalestring_cb;
            value = set_number100_cb; data = &mp->mixer[1].scalar;
            if(! MIXER_SRC(mp->mixer[1].sw))
                disable = 1;
            break;
        case EXPO_SWITCH2:
            label = _tr("Switch2"); underline = 1;
            tgl = sourceselect_cb; value = set_drsource_cb; data = &mp->mixer[2].sw;
            break;
        case EXPO_LINK2:
            but_tgl = toggle_link_cb; but_txt = show_rate_cb; but_data = (void *)1;
            if(! MIXER_SRC(mp->mixer[2].sw))
                disable = 1;
            break;
        case EXPO_CURVE2:
            tgl = curveselect_cb; value = set_curvename_cb; data = &mp->mixer[2];
            if(! MIXER_SRC(mp->mixer[2].sw) || mp->link_curves & 0x02)
                disable = 1;
            break;
        case EXPO_SCALE2:
            label = (void *)2; label_cb = scalestring_cb;
            value = set_number100_cb; data = &mp->mixer[2].scalar;
            if(! MIXER_SRC(mp->mixer[2].sw))
                disable = 1;
            break;
    }
    if(label || label_cb) {
        labelDesc.style = LABEL_LEFTCENTER;
        GUI_CreateLabelBox(&gui->label[relrow], x, y, w, ITEM_HEIGHT,
            &labelDesc, label_cb, NULL, label);
        if(underline)
            GUI_CreateRect(&gui->rect1, x, y, LEFT_VIEW_WIDTH, 1, &labelDesc);
        y += ITEM_HEIGHT + 1;
    }
    labelDesc.style = LABEL_CENTER;
    if(but_tgl) {
        GUI_CreateButtonPlateText(&gui->value[relrow].but, x, y,
            w, ITEM_HEIGHT, &labelDesc, but_txt, 0xffff, but_tgl, but_data);
        if(disable) {
            GUI_ButtonEnable((guiObject_t *)&gui->value[relrow].but, 0);
        }
    } else {
        GUI_CreateTextSelectPlate(&gui->value[relrow].ts, x, y,
            w, ITEM_HEIGHT, &labelDesc, tgl, value, data);
        if(disable) {
            GUI_TextSelectEnable(&gui->value[relrow].ts, 0);
        }
    }
    return 1;
}

static void _show_expo_dr()
{
    GUI_SelectionNotify(notify_cb);
    GUI_Select1stSelectableObj();

    sync_mixers();

    GUI_CreateScrollable(&gui->scrollable, 0, ITEM_HEIGHT + 1, LEFT_VIEW_WIDTH + ARROW_WIDTH, LCD_HEIGHT - ITEM_HEIGHT -1,
                         ITEM_SPACE, EXPO_LAST, expo_row_cb, simple_getobj_cb, expo_size_cb, NULL);

    GUI_CreateXYGraph(&gui->graph, 77, LCD_HEIGHT - RIGHT_VIEW_HEIGHT - 1, RIGHT_VIEW_HEIGHT, RIGHT_VIEW_HEIGHT,
                              CHAN_MIN_VALUE, CHAN_MIN_VALUE,
                              CHAN_MAX_VALUE, CHAN_MAX_VALUE,
                              0, 0, eval_mixer_cb, curpos_cb, NULL, NULL);

    mp->cur_mixer = &mp->mixer[0];
    //Enable/Disable the relevant widgets
}

static void _update_rate_widgets(u8 idx)
{
    u8 mix = idx + 1;
    guiObject_t *link = GUI_GetScrollableObj(&gui->scrollable, idx ? EXPO_LINK2 : EXPO_LINK1, 0);
    guiObject_t *curve = GUI_GetScrollableObj(&gui->scrollable, idx ? EXPO_CURVE2 : EXPO_CURVE1, 0);
    guiObject_t *scale = GUI_GetScrollableObj(&gui->scrollable, idx ? EXPO_SCALE2 : EXPO_SCALE1, 0);
    if (MIXER_SRC(mp->mixer[mix].sw)) {
        if(link)
            GUI_ButtonEnable(link, 1);
        if(curve) {
            if(mp->link_curves & mix ) {
                GUI_TextSelectEnable((guiTextSelect_t *)curve, 0);
            } else {
                GUI_TextSelectEnable((guiTextSelect_t *)curve, 1);
            }
        }
        if(scale)
            GUI_TextSelectEnable((guiTextSelect_t *)scale, 1);
    } else {
        if(link)
            GUI_ButtonEnable(link, 0);
        if(curve)
            GUI_TextSelectEnable((guiTextSelect_t *)curve, 0);
        if(scale)
            GUI_TextSelectEnable((guiTextSelect_t *)scale, 0);
    }
}

static void notify_cb(guiObject_t * obj)
{
    if(obj && mp->cur_template == MIXERTEMPLATE_EXPO_DR && OBJ_IS_SCROLLABLE(obj)) {
        /* We exploit the fact that each row has only one selecteable object */
        int row_offset = GUI_ScrollableGetObjRowOffset(&gui->scrollable, obj);
        int idx = (row_offset >> 8) + (row_offset & 0xff);
        if(idx >= EXPO_SWITCH1 && idx <= EXPO_SCALE1) {
            if(mp->cur_mixer != &mp->mixer[1]) {
                sync_mixers();
                mp->cur_mixer = &mp->mixer[1];
                GUI_Redraw(&gui->graph);
            }
        } else if(idx >= EXPO_SWITCH2 && idx <= EXPO_SCALE2) {
            if(mp->cur_mixer != &mp->mixer[2]) {
                sync_mixers();
                mp->cur_mixer = &mp->mixer[2];
                GUI_Redraw(&gui->graph);
            }
        } else {
            if(mp->cur_mixer != &mp->mixer[0]) {
                mp->cur_mixer = &mp->mixer[0];
                GUI_Redraw(&gui->graph);
            }
        }
    }
}

static u8 action_cb(u32 button, u8 flags, void *data)
{
    (void)data;
    if ((flags & BUTTON_PRESS) || (flags & BUTTON_LONGPRESS)) {
        if (CHAN_ButtonIsPressed(button, BUT_EXIT)) {
            GUI_SelectionNotify(NULL);
            GUI_RemoveAllObjects();  // Discard unsaved items and exit to upper page
            PAGE_MixerInit(mp->top_channel);
        } else if (CHAN_ButtonIsPressed(button, BUT_ENTER)&& (flags & BUTTON_LONGPRESS)) {
            // long press enter = save without exiting
            PAGE_SaveMixerSetup(mp);
        }
        else {
            // only one callback can handle a button press, so we don't handle BUT_ENTER here, let it handled by press cb
            return 0;
        }
    }
    return 1;
}
