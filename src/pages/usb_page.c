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
#include "gui/gui.h"

static const char firmware_version[] = HGVERSION;
static struct usb_page * const up = &pagemem.u.usb_page;

static void draw_page(u8 enable)
{
    PAGE_RemoveAllObjects();
    PAGE_ShowHeader(_tr("USB"));

    sprintf(up->tmpstr, "%s" HGVERSION "\n%s%s",
            _tr("Deviation FW\nversion: "),
            _tr("USB File System is disabled\nPress 'Ent' to "),
            enable == 0 ? _tr("enable") : _tr("disable"));
    GUI_CreateLabel(100, 80, NULL, DEFAULT_FONT, up->tmpstr);
}

static void wait_press()
{
    printf("Wait Press\n");
    while(1) {
        u32 buttons = ScanButtons();
        if (CHAN_ButtonIsPressed(buttons, BUT_ENTER))
            break;
        if(PWR_CheckPowerSwitch())
            PWR_Shutdown();
    }
    printf("Pressed\n");
}

static void wait_release()
{
    printf("Wait Release\n");
    while(1) {
        u32 buttons = ScanButtons();
        if (! CHAN_ButtonIsPressed(buttons, BUT_ENTER))
            break;
        if(PWR_CheckPowerSwitch())
            PWR_Shutdown();
    }
    printf("Released\n");
}

u8 usb_cb(u32 button, u8 flags, void *data)
{
    (void)button;
    (void)data;
    if(flags == BUTTON_RELEASE) {
        draw_page(1);
        GUI_RefreshScreen();
        USB_Enable(1);
        wait_release();
        wait_press();
        wait_release();
        USB_Disable(1);
        draw_page(0);
    }
    return 1;
}

void PAGE_USBInit(int page)
{
    (void)page;
    PAGE_SetModal(0);
    draw_page(0);
    BUTTON_RegisterCallback(&up->action, CHAN_ButtonMask(BUT_ENTER), BUTTON_PRESS | BUTTON_RELEASE, usb_cb, NULL);
}

void PAGE_USBExit()
{
    BUTTON_UnregisterCallback(&up->action);
}

void PAGE_USBEvent()
{
}

void USB_Connect()
{
    USB_Enable(1);
    wait_release();
    wait_press();
    wait_release();
    USB_Disable(1);
}
