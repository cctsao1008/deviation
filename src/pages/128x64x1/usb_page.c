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

static struct usb_page * const up = &pagemem.u.usb_page;

static void draw_page(u8 enable)
{
    PAGE_RemoveAllObjects();
    PAGE_ShowHeader(_tr("USB"));

    sprintf(up->tmpstr, "%s%s\n%s%s",
            _tr("Deviation FW\nversion: "), DeviationVersion,
            _tr("Press 'Ent' to turn USB Filesystem: "),
            enable == 0 ? _tr("On") : _tr("Off"));
    GUI_CreateLabelBox(20, 80, 280, 100, &NARROW_FONT, NULL, NULL, up->tmpstr);
}

static void wait_press()
{
    printf("Wait Press\n");
    while(1) {
        CLOCK_ResetWatchdog();
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
        CLOCK_ResetWatchdog();
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
    PAGE_SetActionCB(usb_cb);
}

void PAGE_USBExit()
{
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
