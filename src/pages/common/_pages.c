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

static buttonAction_t button_action;
static u8 (*ActionCB)(u32 button, u8 flags, void *data);

void PAGE_ChangeQuick(int dir);

struct pagemem pagemem;
static u8 modal;
static u8 cur_page;

void PAGE_Event()
{
    if(pages[cur_page].event)
        pages[cur_page].event();
}

void PAGE_Exit()
{
    if(pages[cur_page].exit)
        pages[cur_page].exit();
}

u8 PAGE_SetModal(u8 _modal)
{
    u8 old = modal;
    modal = _modal;
    return old;
}

u8 PAGE_GetModal()
{
    return modal;
}


void PAGE_SetActionCB(u8 (*callback)(u32 button, u8 flags, void *data))
{
    ActionCB = callback;
}

u8 PAGE_TelemStateCheck(char *str, int strlen)
{
    s8 state = PROTOCOL_GetTelemetryState();
    if (state == -1) {
        snprintf(str, strlen, "%s%s%s",
            _tr("Telemetry"),
            LCD_DEPTH == 1?"\n":" ", // no translate for this string
            _tr("is not supported"));
        return 0;
    }
    else if (state == 0) {
        snprintf(str, strlen, "%s%s%s",
            _tr("Telemetry"),
            LCD_DEPTH == 1?"\n":" ",  // no translate for this string
            _tr("is turned off"));
        return 0;
    }
    return 1;
}
