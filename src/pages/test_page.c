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
#include "protocol/interface.h"
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
static char buttonmessage[30];

void PushMeButton(guiObject_t *obj, void *data) {
    GUI_RemoveObj(obj);
    sprintf(buttonmessage,"%s",(const char *)data);
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
void PAGE_TestInit(int page)
{
    (void)page;
    static char strBootLoader[80],strSPIFlash[80],strMfg[80];
    static const char *button1 = "Button 1";
    static const char *button2 = "Button 2";
    static const char *button3 = "Button 3";
    //static const char *statusBar = "media/bar.bmp";
    static const char *batteryImg = "media/bat.bmp";

    CLOCK_StopTimer();
    sprintf(buttonstr,"buttons");
    sprintf(buttonmessage," ");
    sprintf(t1," ");
    sprintf(t2," ");
    /* GUI Callbacks */

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
    //GUI_CreateImage(0,0,320,24,statusBar);
    GUI_CreateImage(270,1,48,22,batteryImg);
    GUI_CreateLabel(10,60,NULL, DEFAULT_FONT, strSPIFlash);
    GUI_CreateLabel(10,48,NULL, DEFAULT_FONT, strBootLoader);
    GUI_CreateLabel(10,30,NULL, DEFAULT_FONT, strMfg);
    GUI_CreateLabel(10,75,NULL, DEFAULT_FONT, buttonstr);
    lblVoltage = GUI_CreateLabel(267,8,NULL, BATTERY_FONT, voltagestr);
    lblTE = GUI_CreateLabel(10,110,NULL, MISC1_FONT, te);
    lblRA = GUI_CreateLabel(10,140,NULL, MISC1_FONT, ra);
    GUI_CreateLabel(10,125,NULL, MISC1_FONT, t1);
    GUI_CreateLabel(10,155,NULL, MISC1_FONT, t2);
    GUI_CreateLabel(100,170,NULL, MISC1_FONT, buttonmessage);
    GUI_CreateButton(10,200,BUTTON_96,button1,0x0000,PushMeButton, "Button 1 Pushed");
    GUI_CreateButton(110,200,BUTTON_96,button2,0x0000,PushMeButton, "Button 2 Pushed");
    GUI_CreateButton(210,200,BUTTON_96,button3,0x0000,PushMeButton, "Button 3 Pushed");
    GUI_CreateDialog(70,50,180,130,"Deviation","Welcome to\nDeviation",0xffff,0x0000,openDialogPush,dtOk);
    //GUI_CreateKeyboard(KEYBOARD_CHAR, buttonstr, 20, &PushMeButton, buttonstr);
    //GUI_CreateListBox(100, 20, 200, 202, 20, 5, &string_cb, NULL, NULL, NULL);

    // do a master redraw
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
