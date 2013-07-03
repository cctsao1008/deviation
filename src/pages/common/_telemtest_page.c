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

#define tp (pagemem.u.telemtest_page)
#define gui (&gui_objs.u.telemtest1)

#define TELEM_FONT NORMALBOX_FONT
#define TELEM_TXT_FONT DEFAULT_FONT
#define TELEM_ERR_FONT NORMALBOXNEG_FONT

static u8 telem_state_check()
{
    if (PAGE_TelemStateCheck(tp.str, sizeof(tp.str))==0) {
        memset(gui, 0, sizeof(*gui));
        return 0;
    }
    return 1;
}

static const char *telem_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    u32 val = (long)data;
    return TELEMETRY_GetValueStr(tp.str, val);
}

static const char *label_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    long val = (long)data;
    char str[25];
    sprintf(tp.str, "%s:", TELEMETRY_ShortName(str, val));
    return tp.str;
}

static void okcancel_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    (void)data;
    if(tp.return_page) {
        PAGE_SetModal(0);
        PAGE_RemoveAllObjects();
        tp.return_page(tp.return_val);
    }
}

