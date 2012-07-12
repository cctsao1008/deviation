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
#include "misc.h"
#include "gui/gui.h"

void event_loop(void *);
void channel_scanner();
void dump_bootloader();
void SignOn();
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
    SPI_FlashBlockWriteEnable(1); //Enable writing to all banks of SPIFlash
    SignOn();
    CONFIG_ReadDisplay();
    LCD_Clear(0x0000);
    LCD_SetFont(DEFAULT_FONT.font);
    LCD_SetFontColor(DEFAULT_FONT.color);
    if(! FS_Mount()) {
        LCD_PrintStringXY(10, 10, "Install filesystem, thn press 'ENT'");
        USB_Connect();
    }
    MUSIC_Play(MUSIC_STARTUP);

#if 0
    printf("Showing display\n");
    LCD_PrintStringXY(10, 20, "Hello");
    while(1) {
        if(PWR_CheckPowerSwitch())
            PWR_Shutdown();
    }
#endif
#ifdef BL_DUMP
    dump_bootloader();
#endif
#ifdef STATUS_SCREEN
    TEST_init_mixer();
    PAGE_Init();
#ifdef HAS_EVENT_LOOP
    start_event_loop();
#else
    while(1)
        event_loop(NULL);
#endif
    return 0;
#endif
}

void event_loop(void *param)
{
    /* Some needed variables */
    static u32 last_buttons = 0;
    static u8 touch_down = 0;
    static u32 last_redraw = 0;
    (void)(param);

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
            for(i = 0; i < 32; i++)
                buttonstring[i] = (buttons & (1 << i)) ? '0' : '1';
            buttonstring[32] = 0;
            printf("Buttons: %s\n",buttonstring);
            last_buttons = buttons;
            MIX_UpdateTrim(buttons);
            if(CHAN_ButtonIsPressed(buttons, BUT_RIGHT)) {
                PAGE_Change(1);
            } else if(CHAN_ButtonIsPressed(buttons, BUT_LEFT)) {
                PAGE_Change(-1);
            }
        }
    }
    if(SPITouch_IRQ()) {
        struct touch t = SPITouch_GetCoords();
        //printf("x : %4d y : %4d\n", t.x, t.y);
        if (! touch_down) {
            GUI_CheckTouch(&t, 0);
            touch_down = 1;
        }
    } else {
        if(touch_down)
            GUI_TouchRelease();
        touch_down = 0;
    }
    MIX_CalcChannels();

    PAGE_Event();

    if (CLOCK_getms() > last_redraw + 100) {
        if (touch_down) {
            if (touch_down++ >= 5) {
                //Long-press
                struct touch t = SPITouch_GetCoords();
                GUI_CheckTouch(&t, 1);
            }
        }
        /* Redraw everything */
        GUI_RefreshScreen();
        last_redraw = CLOCK_getms();
    }
}

void SignOn()
{
    u8 Power = CYRF_MaxPower();
    u8 mfgdata[6];
    u8 tmp[12];
    ModelName(tmp, 12);
    printf("\nOpen Whatever\n");
    /* Check CPU type */

    printf("BootLoader    : '%s'\n",tmp);
    printf("Power         : '%s'\n",Power == CYRF_PWR_100MW ? "100mW" : "10mW" );
    printf("SPI Flash     : '%X'\n",(unsigned int)SPIFlash_ReadID());
    CYRF_GetMfgData(mfgdata);
    printf("CYRF Mfg Data : '%02X %02X %02X %02X %02X %02X'\n",
            mfgdata[0],
            mfgdata[1],
            mfgdata[2],
            mfgdata[3],
            mfgdata[4],
            mfgdata[5]);
    
}
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
