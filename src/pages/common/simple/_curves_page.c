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

static struct mixer_page * const mp = &pagemem.u.mixer_page;

static PitThroMode pit_mode = PITTHROMODE_NORMAL;
static CurvesMode curve_mode;
static u8 pit_hold_state = 0;
static u8 selectable_bitmaps[7] = {0, 0, 0 ,0 ,0, 0, 0};

static void update_textsel_state();

static const char *set_mode_cb(guiObject_t *obj, int dir, void *data)
{
    (void)obj;
    curve_mode = (long)data;
    u8 max = curve_mode == CURVESMODE_PITCH?PITTHROMODE_HOLD: PITTHROMODE_IDLE2;
    u8 changed;
    pit_mode = GUI_TextSelectHelper(pit_mode, PITTHROMODE_NORMAL, max, dir, 1, 1, &changed);
    u8 i;
    if (changed) {
        if (pit_mode == PITTHROMODE_HOLD)
            GUI_SetHidden(mp->itemObj[9], 0);
        else
            GUI_SetHidden(mp->itemObj[9],  1);
        update_textsel_state();
        GUI_Redraw(mp->graphs[0]);
        for ( i = 0; i < 9; i++)
            GUI_Redraw(mp->itemObj[i]);
    }
    strcpy(mp->tmpstr, (const char *)SIMPLEMIX_ModeName(pit_mode));
    return mp->tmpstr;
}

static void set_hold_state(u8 state) {
    if (state == 0) {
        mp->mixer_ptr[PITTHROMODE_HOLD]->src = 0;
        mp->mixer_ptr[PITTHROMODE_HOLD] = NULL;
    } else {
        memcpy(&mp->mixer[0], mp->mixer_ptr[0], sizeof(mp->mixer[0]));
        memcpy(&mp->mixer[1], mp->mixer_ptr[1], sizeof(mp->mixer[1]));
        memcpy(&mp->mixer[2], mp->mixer_ptr[2], sizeof(mp->mixer[2]));
        mp->num_mixers = 4;
        mp->mixer[3].src    = mp->mixer_ptr[0]->src;
        mp->mixer[3].dest   = mp->mixer_ptr[0]->dest;
        mp->mixer[3].sw = INP_RUD_DR;
        mp->mixer[3].mux    = MUX_REPLACE;
        mp->mixer[3].offset = 0;
        mp->mixer[3].scalar = mp->mixer_ptr[0]->scalar;
        mp->mixer[3].apply_trim = mp->mixer_ptr[0]->apply_trim;
        mp->mixer[3].curve = mp->mixer_ptr[0]->curve;
        MIXER_SetMixers(mp->mixer, mp->num_mixers);
        mp->mixer_ptr[PITTHROMODE_HOLD] = &mp->mixer[3];
    }
    GUI_Redraw(mp->graphs[0]);
    for (u8 i = 0; i < 9; i++)
        GUI_Redraw(mp->itemObj[i]);
}

static const char *set_holdstate_cb(guiObject_t *obj, int dir, void *data)
{
    (void)dir;
    (void)data;
    (void)obj;
    u8 changed;
    pit_hold_state = GUI_TextSelectHelper(pit_hold_state, 0, 1, dir, 1, 1, &changed);
    if (changed) {
        set_hold_state(pit_hold_state);
    }
    if (pit_mode != PITTHROMODE_HOLD)
        strcpy(mp->tmpstr, "");
    else if (pit_hold_state)
        strcpy(mp->tmpstr, _tr("On"));
    else
        strcpy(mp->tmpstr, _tr("Off"));
    return mp->tmpstr;
}


static void auto_generate_cb(guiObject_t *obj, const void *data)
{
    (void)data;
    (void)obj;
    if (mp->mixer_ptr[pit_mode] == NULL) // Bug fix: do not do auto gen when pit curve is off
        return;
    struct Curve *curve = &(mp->mixer_ptr[pit_mode]->curve);
    s16 y_diff = 0;
    s16 x_start = 0;
    s16 x_end = 8;
    u8 selectable_bitmap = selectable_bitmaps[curve_mode * 4 + pit_mode];
    u8 j;
    for (u8 i = 1; i < 9; i++) {
        if (i == 8 || selectable_bitmap >> (i-1) & 0x01) {
            x_end = i;
            if (x_end - x_start > 1) {
                y_diff = curve->points[x_end] - curve->points[x_start];
                u16 x_diff =  x_end - x_start;
                for (j = x_start + 1; j < x_end; j++) {
                    curve->points[j] = y_diff * (j - x_start)/x_diff + curve->points[x_start];
                    GUI_Redraw(mp->itemObj[j]);
                }
            }
            x_start = x_end; // no need to calculate
        }
    }
    GUI_Redraw(mp->graphs[0]);
}

static const char *set_pointval_cb(guiObject_t *obj, int dir, void *data)
{
    if (mp->mixer_ptr[pit_mode] == NULL)
        return "";
    u8 point_num = (long)data;
    struct Curve *curve = &(mp->mixer_ptr[pit_mode]->curve);
    if (GUI_IsTextSelectEnabled(obj) == 1) {
        u8 changed = 1;
        curve->points[point_num] = GUI_TextSelectHelper(curve->points[point_num], -100, 100, dir, 1, LONG_PRESS_STEP, &changed);
        if (changed)
            GUI_Redraw(mp->graphs[0]);
    }
    sprintf(mp->tmpstr, "%d", curve->points[point_num]);
    return mp->tmpstr;
}

static u8 curpos_cb(s16 *x, s16 *y, u8 pos, void *data)
{
    (void)data;
    if (mp->mixer_ptr[pit_mode] == NULL)
        return 0;
    if (pos != 0)
        return 0;
    *x = mp->raw[MIXER_SRC(mp->mixer_ptr[pit_mode]->src)];
    if (*x > CHAN_MAX_VALUE)
        *x = CHAN_MAX_VALUE;
    else if (*x  < CHAN_MIN_VALUE)
        *x = CHAN_MIN_VALUE;
    *y = SIMPLEMIX_EvalMixerCb(*x, mp->mixer_ptr[pit_mode], CHAN_MAX_VALUE, CHAN_MIN_VALUE);
    return 1;
}

static s16 show_curve_cb(s16 xval, void *data)
{
    (void)data;
    if (mp->mixer_ptr[pit_mode] == NULL)
        return 0;
    s16 yval = CURVE_Evaluate(xval, &(mp->mixer_ptr[pit_mode]->curve));
    return yval;
}

static u8 event_interval = 0;
void PAGE_CurvesEvent()
{
    //if (event_interval++ <EVENT_REFRESH_INTERVAL) // reduce the refresh frequency
    //    return;
    event_interval = 0;
    if (mp->graphs[0]) {
        if(MIXER_GetCachedInputs(mp->raw, CHAN_MAX_VALUE / 100)) { // +/-1%
            GUI_Redraw(mp->graphs[0]);
            if (mp->graphs[1])
                GUI_Redraw(mp->graphs[1]);
            if (mp->graphs[2])
                GUI_Redraw(mp->graphs[2]);
        }
    }
}


