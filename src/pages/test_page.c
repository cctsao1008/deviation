/*
 This project is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Foobar is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "target.h"
#include "pages.h"
#include "gui/gui.h"

static char buttonstr[80];
static char voltagestr[8];
static char te[40];
static char ra[40];
static char t1[40];
static char t2[40];
static guiObject_t *lblVoltage;
static guiObject_t *lblTE;
static guiObject_t *lblRA;

void PAGE_TestInit(int page)
{
    (void)page;
    static char strBootLoader[80],strSPIFlash[80],strMfg[80];
    static char buttonmessage[30];
    static const char *button1 = "Button 1";
    static const char *button2 = "Button 2";
    static const char *button3 = "Button 3";
    static const char *statusBar = "bar.bmp";
    static const char *batteryImg = "bat.bmp";

    sprintf(buttonstr,"buttons");
    sprintf(buttonmessage," ");
    sprintf(t1," ");
    sprintf(t2," ");
    /* GUI Callbacks */

    void PushMeButton1(guiObject_t *obj) {
        GUI_RemoveObj(obj);
        sprintf(buttonmessage,"%s","Button 1 Pushed");
    }
    void PushMeButton2(guiObject_t *obj) {
        GUI_RemoveObj(obj);
        sprintf(buttonmessage,"%s","Button 2 Pushed");
    }
    void PushMeButton3(guiObject_t *obj) {
        GUI_RemoveObj(obj);
        sprintf(buttonmessage,"%s","Button 3 Pushed");
    }
    void openDialogPush(guiObject_t *obj, struct guiDialogReturn gDR) {
        (void)gDR;
        GUI_RemoveObj(obj);
    }
    s16 xy_cb(s16 val, void *data) {
        (void)data;
        return val;
    }
    const char *ts_cb(guiObject_t *obj, int value, void *data) {
        (void)data;
        (void)obj;
        static int idx = 0;
        const char *str[] = {
             "None",
             "String 1",
             "String 2",
             };
        if(value > 0) {
            if(idx < 2)
                idx++;
        } else if(value < 0) {
            if(idx)
                idx--;
        }
        return str[idx];
    }
    s16 bar_cb(void *data) {
        (void)data;
        static int val = 100;
        return val++;
    }
    {
        u8 * pBLString = BOOTLOADER_Read(BL_ID);
        u8 mfgdata[6];
        sprintf(strBootLoader, "BootLoader   : %s\n",pBLString);
        sprintf(strSPIFlash, "SPI Flash    : %X\n",(unsigned int)SPIFlash_ReadID());
        CYRF_GetMfgData(mfgdata);
        sprintf(strMfg, "CYRF Mfg Data: %02X%02X%02X%02X%02X%02X\n",
                mfgdata[0],
                mfgdata[1],
                mfgdata[2],
                mfgdata[3],
                mfgdata[4],
                mfgdata[5]);
    }

    /* Create some GUI elements */
    GUI_CreateFrame(0,0,320,24,statusBar);
    GUI_CreateFrame(270,1,48,22,batteryImg);
    GUI_CreateLabel(10,60,strSPIFlash,0x0000);
    GUI_CreateLabel(10,45,strBootLoader,0x0000);
    GUI_CreateLabel(10,30,strMfg,0x0000);
    GUI_CreateLabel(10,75,buttonstr,0x0000);
    lblVoltage = GUI_CreateLabel(267,8,voltagestr,0x0f00);
    lblTE = GUI_CreateLabel(10,110,te,0xffff);
    lblRA = GUI_CreateLabel(10,140,ra,0xffff);
    GUI_CreateLabel(10,125,t1,0xffff);
    GUI_CreateLabel(10,155,t2,0xffff);
    GUI_CreateLabel(100,170,buttonmessage,0xffff);
    GUI_CreateXYGraph(40, 110, 100, 100, -10, -10, 40, 40, xy_cb, NULL);
    GUI_CreateTextSelect(10,180,128,16,0x0000, NULL, ts_cb, NULL);
    GUI_CreateButton(10,200,89,23,button1,0x0000,PushMeButton1);
    GUI_CreateButton(110,200,89,23,button2,0x0000,PushMeButton2);
    GUI_CreateButton(210,200,89,23,button3,0x0000,PushMeButton3);
    GUI_CreateDialog(70,50,180,130,"Deviation","Welcome to\nDeviation",0xffff,0x0000,openDialogPush,dtOk);

    // do a master redraw
    GUI_DrawScreen();
}

void PAGE_TestEvent()
{
    char s[80];

    u16 voltage = PWR_ReadVoltage();
    sprintf(s, "%2d.%03d\n", voltage >> 12, voltage & 0x0fff);
    if (strcmp(s,voltagestr) != 0) {
        sprintf(voltagestr,"%s",s);
        GUI_Redraw(lblVoltage);
    }

    u16 throttle = CHAN_ReadInput(INP_THROTTLE);
    u16 rudder   = CHAN_ReadInput(INP_RUDDER);
    u16 aileron  = CHAN_ReadInput(INP_AILERON);
    u16 elevator = CHAN_ReadInput(INP_ELEVATOR);
    sprintf(s, "Throttle: %04X Elevator: %04X\n", throttle, elevator);
    if (strcmp(s,te) != 0) {
        sprintf(te,"%s",s);
        GUI_Redraw(lblTE);
    }
    sprintf(s, "Rudder  : %04X Aileron : %04X\n", rudder, aileron);
    if (strcmp(s,ra) != 0) {
        sprintf(ra,"%s",s);
        GUI_Redraw(lblRA);
    }
}

int PAGE_TestCanChange()
{
    return (! GUI_IsModal());
}
