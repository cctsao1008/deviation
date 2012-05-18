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
#include "misc.h"
#include "gui/gui.h"

void initialize_status();
void event_loop(void *);
void channel_scanner();
void dump_bootloader();
extern void start_event_loop();
extern void TEST_init_mixer();

int main() {
    PWR_Init();
    CLOCK_Init();
    Initialize_ButtonMatrix();
    Delay(0x2710);

    LCD_Init();
    CHAN_Init();

    SPIFlash_Init();
    CYRF_Initialize();
    UART_Initialize();
    SPITouch_Init();
    SOUND_Init();
    FS_Mount();
    SPI_FlashBlockWriteEnable(1); //Enable writing to all banks of SPIFlash
    SignOn();
    LCD_Clear(0x0000);
    LCD_SetFont(6);
    LCD_SetFontColor(0xffff);
#if 0
    printf("Showing display\n");
    LCD_PrintStringXY(10, 10, "Hello");
    while(1) {
        if(PWR_CheckPowerSwitch())
            PWR_Shutdown();
    }
#endif
#ifdef BL_DUMP
    dump_bootloader();
#endif
#if SCANNER
    channel_scanner();
#endif
#ifdef STATUS_SCREEN
    //PAGE_InitMixer(0);
    initialize_status();
#ifdef HAS_EVENT_LOOP
    start_event_loop();
#else
    while(1)
        event_loop(NULL);
#endif
    return 0;
#endif
}

static char buttonstr[80];
static char voltagestr[8];
static char te[40];
static char ra[40];
static char t1[40];
static char t2[40];
static int show_mixer = 0;
static int touch_down = 0;
static guiObject_t *bar[NUM_CHANNELS];
static guiObject_t *lblButtons;
static guiObject_t *lblVoltage;
void initialize_status()
{
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
        GUI_RemoveObj(obj);
    }
    s16 xy_cb(s16 val, void *data) {
        (void)data;
        return val;
    }
    const char *ts_cb(guiObject_t *obj, int value, void *data) {
        (void)data;
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
    guiObject_t *frmStatusBar = GUI_CreateFrame(0,0,320,24,statusBar);
    guiObject_t *frmBattery = GUI_CreateFrame(270,1,48,22,batteryImg);
    guiObject_t *lblSPIFlash = GUI_CreateLabel(10,60,strSPIFlash,0x0000);
    guiObject_t *lblBootLoader = GUI_CreateLabel(10,45,strBootLoader,0x0000);
    guiObject_t *lblMfg = GUI_CreateLabel(10,30,strMfg,0x0000);
    lblButtons = GUI_CreateLabel(10,75,buttonstr,0x0000);
    lblVoltage = GUI_CreateLabel(267,8,voltagestr,0x0f00);
    guiObject_t *lblTE = GUI_CreateLabel(10,110,te,0xffff);
    guiObject_t *lblRA = GUI_CreateLabel(10,140,ra,0xffff);
    guiObject_t *lblT1 = GUI_CreateLabel(10,125,t1,0xffff);
    guiObject_t *lblT2 = GUI_CreateLabel(10,155,t2,0xffff);
    guiObject_t *lblButtonMessage = GUI_CreateLabel(100,170,buttonmessage,0xffff);
    guiObject_t *tstXYGraph = GUI_CreateXYGraph(40, 110, 100, 100, -10, -10, 40, 40, xy_cb, NULL);
    guiObject_t *testTxtSel1 = GUI_CreateTextSelect(10,180,128,16,0x0000, NULL, ts_cb, NULL);
    guiObject_t *testButton1 = GUI_CreateButton(10,200,89,23,button1,0x0000,PushMeButton1);
    guiObject_t *testButton2 = GUI_CreateButton(110,200,89,23,button2,0x0000,PushMeButton2);
    guiObject_t *testButton3 = GUI_CreateButton(210,200,89,23,button3,0x0000,PushMeButton3);
    guiObject_t *openDialog = GUI_CreateDialog(70,50,180,130,"Deviation","Welcome to\nDeviation",0xffff,0x0000,openDialogPush,dtOk);

    /* little bit of code to stop variable warnings */
    if (frmStatusBar && frmBattery && lblButtonMessage && lblButtons &&
        lblTE && lblRA && lblT1 && lblT2 && lblSPIFlash && lblBootLoader && lblMfg &&
        testButton1 && testButton2 && testButton3 && openDialog)
    {/*Just here to avoid warnings*/}
    TEST_init_mixer();
    // do a master redraw
    GUI_DrawScreen();
}

s16 showchan_cb(void *data)
{
    long ch = (long)data;
    return Channels[ch];
}

void event_loop(void *param)
{
    /* Some needed variables */
    static u32 last_buttons = 0;
    char s[80];
    (void)(param);

    int ReDraw=0;
    int i;
    if(PWR_CheckPowerSwitch())
        PWR_Shutdown();
    {
        u32 buttons = ScanButtons();

        /* GUI Handling
         * We beed to handle screen redraws here
         * */
        if(buttons != last_buttons) {
            char buttonstring[33];
            last_buttons = buttons;
            if((buttons & 0x01) == 0)
                LCD_CalibrateTouch();
            if((buttons & 0x02) == 0)
                USB_Connect();
            if((buttons & 0x04) == 0)
                show_mixer = 0x80 | (show_mixer & 0x01 ? 0 : 1);
            for(i = 0; i < 32; i++)
                buttonstring[i] = (buttons & (1 << i)) ? '0' : '1';
            buttonstring[32] = 0;
            printf("Buttons: %s\n",buttonstring);
            //sprintf(buttonstr,"Buttons:\n%s",buttonstring);
            //GUI_Redraw(lblButtons);
            //GUI_DrawWindow(lblButtons);
            //ReDraw = 1;
        }
    }
    if(1) {
        u16 voltage = PWR_ReadVoltage();
        sprintf(s, "%2d.%03d\n", voltage >> 12, voltage & 0x0fff);
        if (strcmp(s,voltagestr) != 0) {
            sprintf(voltagestr,"%s",s);
            GUI_Redraw(lblVoltage);
            ReDraw = 1;
        }
    }
    if(1) {
        u16 throttle = CHAN_ReadInput(INP_THROTTLE);
        u16 rudder   = CHAN_ReadInput(INP_RUDDER);
        u16 aileron  = CHAN_ReadInput(INP_AILERON);
        u16 elevator = CHAN_ReadInput(INP_ELEVATOR);
        sprintf(s, "Throttle: %04X Elevator: %04X\n", throttle, elevator);
        if (strcmp(s,te) != 0) {
            sprintf(te,"%s",s);
            //ReDraw = 1;
        }
        sprintf(s, "Rudder  : %04X Aileron : %04X\n", rudder, aileron);
        if (strcmp(s,ra) != 0) {
            sprintf(ra,"%s",s);
            //ReDraw = 1;
        }
    }
    if(SPITouch_IRQ()) {
        struct touch t = SPITouch_GetCoords();
        printf("x : %4d y : %4d\n", t.x, t.y);
        sprintf(s, "x : %4d y : %4d\n", t.x, t.y);
        if (! touch_down) {
            ReDraw |= GUI_CheckTouch(t);
            touch_down = 1;
        }
    } else {
        touch_down = 0;
    }
    if (1 && (show_mixer & 0x80)) {
        show_mixer &= 0x7F;
        if(show_mixer) {
            TEST_init_mixer();
            for(i = 0; i < NUM_CHANNELS; i++) {
                bar[i] = GUI_CreateBarGraph(10 + 20 * i, 10, 10, 220, CHAN_MIN_VALUE, CHAN_MAX_VALUE, BAR_VERTICAL, showchan_cb, (void *)((long)i));
            }
        } else {
            for(i = 0; i < NUM_CHANNELS; i++) {
                GUI_RemoveObj(bar[i]);
            }
        }
        ReDraw = 1;
    }
    MIX_CalcChannels();
    if (show_mixer) {
        for(i = 0; i < NUM_CHANNELS; i++) {
            GUI_Redraw(bar[i]);
        }
        ReDraw = 1;
    }

    if (ReDraw == 1) {
        /* Redraw everything */
        GUI_RefreshScreen();
        ReDraw = 0;
    }
}

#if SCANNER
void channel_scanner()
{
#define NUM_CHANNELS    0x50

    u32 i,j,k;
    u8 dpbuffer[16];
    u8 channelnoise[NUM_CHANNELS];
    u8 channel = 0x04;

    CYRF_ConfigRxTx(1);
    CYRF_ConfigCRCSeed(0);
    CYRF_ConfigSOPCode(0);

    while(1)
    {
        if(PWR_CheckPowerSwitch())
        PWR_Shutdown();

        CYRF_ConfigRFChannel(channel);
        CYRF_StartReceive();
        Delay(10);

        CYRF_ReadDataPacket(dpbuffer);
        channelnoise[channel] = CYRF_ReadRSSI(1);

        printf("%02X : %d\n",channel,channelnoise[channel]);

        channel++;
        if(channel == NUM_CHANNELS)
        {
            channel = 0x04;
            LCD_Clear(0x0000);

            for(i=4;i<NUM_CHANNELS;i++)
            {
                LCD_SetDrawArea(30 + (3*i), 30, 31 + (3*i), 190);
                LCD_DrawStart();
                for(k=0;k<16; k++)
                {
                    for(j=0; j<2; j++)
                    {
                        if(k < (15 - channelnoise[i]))
                        {
                            LCD_DrawPixel(0xF000);
                            LCD_DrawPixel(0xF000);
                            LCD_DrawPixel(0xF000);
                            LCD_DrawPixel(0xF000);
                            LCD_DrawPixel(0xF000);
                            LCD_DrawPixel(0xF000);
                            LCD_DrawPixel(0xF000);
                            LCD_DrawPixel(0xF000);
                            LCD_DrawPixel(0xF000);
                            LCD_DrawPixel(0xF000);
                        }
                        else
                        {
                            LCD_DrawPixel(0xFFFF);
                            LCD_DrawPixel(0xFFFF);
                            LCD_DrawPixel(0xFFFF);
                            LCD_DrawPixel(0xFFFF);
                            LCD_DrawPixel(0xFFFF);
                            LCD_DrawPixel(0xFFFF);
                            LCD_DrawPixel(0xFFFF);
                            LCD_DrawPixel(0xFFFF);
                            LCD_DrawPixel(0xFFFF);
                            LCD_DrawPixel(0xFFFF);
                        }
                    }
                }
                LCD_DrawStop();
            }
        }
    }
}
#endif

#ifdef BL_DUMP
void dump_bootloader()
{
    LCD_PrintStringXY(40, 10, "Dumping");

    printf("Erase...\n");

    SPIFlash_EraseSector(0x2000);
    SPIFlash_EraseSector(0x3000);
    SPIFlash_EraseSector(0x4000);
    SPIFlash_EraseSector(0x5000);

    printf("Pgm 2\n");
    SPIFlash_WriteBytes(0x2000, 0x1000, (u8*)0x08000000);
    printf("Pgm 3\n");
    SPIFlash_WriteBytes(0x3000, 0x1000, (u8*)0x08001000);
    printf("Pgm 4\n");
    SPIFlash_WriteBytes(0x4000, 0x1000, (u8*)0x08002000);
    printf("Pgm 5\n");
    SPIFlash_WriteBytes(0x5000, 0x1000, (u8*)0x08003000);
    printf("Done\n");

    LCD_Clear(0x0000);
    LCD_PrintStringXY(40, 10, "Done");

    while(1)
    {
        if(PWR_CheckPowerSwitch())
        PWR_Shutdown();
    }
}
#endif
