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
#include "gui/gui.h"

static void draw_page(u8 enable)
{
    GUI_RemoveAllObjects();
    if(enable == 0) {
        GUI_CreateLabel(100, 100, NULL, DEFAULT_FONT, "USB File System is disabled\nPress 'Ent' to enable");
    } else {
        GUI_CreateLabel(100, 100, NULL, DEFAULT_FONT, "USB File System is enabled\nPress 'Ent' to disable");
    }
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

void PAGE_USBInit(int page)
{
    (void)page;
    PAGE_SetModal(0);
    draw_page(0);
}

void PAGE_USBEvent()
{
    u32 buttons = ScanButtons();
    if (CHAN_ButtonIsPressed(buttons, BUT_ENTER)) {
        draw_page(1);
        GUI_RefreshScreen();
        USB_Enable(1);
        wait_release();
        wait_press();
        wait_release();
        USB_Disable(1);
        draw_page(0);
    }
}

void USB_Connect()
{
    USB_Enable(1);
    wait_release();
    wait_press();
    wait_release();
    USB_Disable(1);
}
