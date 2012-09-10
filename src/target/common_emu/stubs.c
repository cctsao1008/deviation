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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#include "common.h"
#include "protocol/interface.h"

void TxName(u8 *var, u8 len) {
    const u8 model[] = "DEVO-08-Emu";
    if(len > 12)
         len = 12;
    memcpy(var, model, len - 1);
    var[len-1] = 0;
}
void USB_Enable(u8 use_interrupt) {(void)use_interrupt;}
void USB_Disable() {}
void Initialize_ButtonMatrix() {}
void PWR_Init(void) {}
u16  PWR_ReadVoltage() { return (5500); }
void CHAN_Init() {}

void CLOCK_StartWatchdog() {}
void CLOCK_ResetWatchdog() {}

void SPIFlash_Init() {}
u32  SPIFlash_ReadID() { return 0x12345678; }
void SPI_FlashBlockWriteEnable(u8 enable) {(void)enable;}
void SPITouch_Init() {}

u8 *BOOTLOADER_Read(int idx) {
    static u8 str[3][80] = {
        "",
        "Devo8 Emu"
        };
    u8 ret = 0;
    switch(idx) {
        case BL_ID: ret = 1; break;
    }
    return str[ret];
}
    
void UART_Initialize() {}
int FS_Mount() {return ! chdir("filesystem");}

static DIR *dh;
int FS_OpenDir(const char *path)
{
    dh = opendir(path);
    return (dh != NULL);
}
int FS_ReadDir(char *path)
{
    struct dirent *dir = readdir(dh);
    if (! dir)
        return 0;
    strncpy(path, dir->d_name, 13);
    return 1;
}

void FS_CloseDir() {
    closedir(dh);
}

void BACKLIGHT_Init() {}
void BACKLIGHT_Brightness(u8 brightness) { printf("Brightness: %d\n", brightness); }
