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
static struct chantest_page * const cp = &pagemem.u.chantest_page;
static struct chantest_obj * const gui = &gui_objs.u.chantest;
static s16 showchan_cb(void *data);
static const char *value_cb(guiObject_t *obj, const void *data);
static const char *channum_cb(guiObject_t *obj, const void *data);
static void _handle_button_test();
static inline guiObject_t *_get_obj(int chan, int objid);
static int get_channel_idx();
static int _get_channel_idx(int chan);

enum {
    ITEM_GRAPH,
    ITEM_VALUE,
    ITEM_GRAPH2,
    ITEM_VALUE2,
};

const char *lockstr_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    (void)data;
    if(cp->is_locked == 1 || cp->is_locked == 2)
        return _tr("Touch to Unlock");
    else
        return _tr("Touch to Lock");
}

const char *button_str_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    int button = (long)data;
    return INPUT_ButtonName(button + 1);
}

static void okcancel_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    (void)data;
    if(cp->return_page) {
        PAGE_SetModal(0);
        PAGE_RemoveAllObjects();
        cp->return_page(cp->return_val);
    }
}

unsigned button_capture_cb(u32 button, unsigned flags, void *data)
{
    (void)button;
    (void)flags;
    (void)data;
    return 1;
}

void PAGE_ChantestEvent()
{
    int i;
    if(cp->type == MONITOR_BUTTONTEST) {
        _handle_button_test();
        return;
    }
    volatile s16 *raw = MIXER_GetInputs();
    for(i = 0; i < cp->num_bars; i++) {
        int ch = get_channel_idx(i);
        int v = raw[ch + 1];
        if (ch >= NUM_INPUTS && ch < NUM_OUT_CHANNELS) {
            v = Channels[ch - NUM_INPUTS];
        }
        v = RANGE_TO_PCT(v) ;
        if (v != cp->pctvalue[i]) {
            guiObject_t *obj = _get_obj(i, ITEM_GRAPH);
            if (obj) {
                GUI_Redraw(obj);
                GUI_Redraw(_get_obj(i, ITEM_VALUE));
            }
            cp->pctvalue[i] = v;
        }
    }
}

void PAGE_ChantestExit()
{
    BUTTON_UnregisterCallback(&cp->action);
}
static s16 showchan_cb(void *data)
{
    long ch = (long)data;
    return cp->pctvalue[ch];
}

static const char *value_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    long ch = (long)data;
    sprintf(tempstring, "%d", cp->pctvalue[ch]);
    return tempstring;
}

static const char *channum_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    long ch = (long)data;

    if (! cp->type) {
        ch -= NUM_INPUTS;
        if (ch < NUM_OUT_CHANNELS) {
            sprintf(tempstring, "\n%d", (int)ch+1);
        } else {
            ch -= NUM_OUT_CHANNELS;
            if (Model.virtname[ch][0]) {
                tempstring_cpy(Model.virtname[ch]) ;
            } else {
                sprintf(tempstring, "%s%d", _tr("Virt"), ch + 1);
            }
        }
    } else {
        char *p = tempstring;
        if (ch & 0x01) {
            *p = '\n';
            p++;
        }
        CONFIG_EnableLanguage(0);  //Disable translation because tiny font is limited in character set
        INPUT_SourceName(p, ch+1);
        CONFIG_EnableLanguage(1);
        if (! (ch & 0x01)) {
           sprintf(p + strlen(p), "\n");
        }
    }
    return tempstring;
}

static int _get_channel_idx(int chan)
{
    int i;
    if (! cp->type) {
        for (i = 0; i < NUM_CHANNELS; i+= 1) {
            if (Model.templates[i] != MIXERTEMPLATE_NONE) {
                chan--;
            }
            if (chan < 0)
                break;
        }
        return i + NUM_INPUTS;
    } else {
        int ppms = Model.num_ppmin & 0x3f;
        if (chan < ppms)
            return NUM_INPUTS + NUM_OUT_CHANNELS + NUM_VIRT_CHANNELS + chan;
        chan -= ppms;
        for (i = 0; i < NUM_INPUTS; i++) {
            if (!(Transmitter.ignore_src & (1 << (i+1)))) {
                chan--;
            }
            if (chan < 0)
                break;
        }
        return i;
    }
}
