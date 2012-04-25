#ifndef _TARGET_H_
#define _TARGET_H_
#include <stdint.h>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

/* General functions */
void SignOn();

/* Display Functions */
void LCD_Init();

    /* Primitives */
void LCD_DrawPixel(unsigned int color);
void LCD_DrawPixelXY(unsigned int x, unsigned int y, unsigned int color);
void LCD_DrawStart(void);
void LCD_DrawStop(void);
void LCD_SetDrawArea(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1);
void LCD_Clear(unsigned int color);
    /* Strings */
void LCD_PrintCharXY(unsigned int x, unsigned int y, const char c);
void LCD_PrintChar(const char c);
void LCD_PrintStringXY(unsigned int x, unsigned int y, const char *str);
void LCD_PrintString(const char *str);
void LCD_SetXY(unsigned int x, unsigned int y);
void LCD_SetFont(unsigned int idx);
    /* Graphics */
void LCD_DrawCircle(u16 x0, u16 y0, u16 r, u16 color);
void LCD_FillCircle(u16 x0, u16 y0, u16 r, u16 color);
void LCD_DrawLine(u16 x0, u16 y0, u16 x1, u16 y1, u16 color);
void LCD_DrawRect(u16 x, u16 y, u16 w, u16 h, u16 color);
void LCD_FillRect(u16 x, u16 y, u16 w, u16 h, u16 color);
void LCD_DrawRoundRect(u16 x, u16 y, u16 w, u16 h, u16 r, u16 color);
void LCD_FillRoundRect(u16 x, u16 y, u16 w, u16 h, u16 r, u16 color);
void LCD_DrawTriangle(u16 x0, u16 y0, u16 x1, u16 y1, u16 x2, u16 y2, u16 color);
void LCD_FillTriangle(u16 x0, u16 y0, u16 x1, u16 y1, u16 x2, u16 y2, u16 color);
void LCD_DrawImageFromSPI(u16 x, u16 y, const u16 *bitmap, u16 w, u16 h);

/* Touchscreen */
struct touch {
    u8 x;
    u8 y;
    u8 z1;
    u8 z2;
};
void SPITouch_Init();
struct touch SPITouch_GetCoords();
int SPITouch_IRQ();


/* Buttons and switches */
void Initialize_ButtonMatrix();
u32 ScanButtons();

/* Power functions */
void Initialize_Clock(void);
void Initialize_PowerSwitch(void);
int CheckPowerSwitch();
void PowerDown();

/* Sticks */
void Initialize_Channels();
u16 ReadThrottle();
u16 ReadRudder();
u16 ReadElevator();
u16 ReadAileron();

/* SPI Flash */
void SPIFlash_Init();
u32  SPIFlash_ReadID();
void SPIFlash_EraseSector(u32 sectorAddress);
void SPIFlash_BulkErase();
void SPIFlash_WriteBytes(u32 writeAddress, u32 length, u8 * buffer);
void SPIFlash_ReadBytes(u32 readAddress, u32 length, u8 * buffer);

/* SPI CYRF6936 */
void CYRF_Initialize();
void CYRF_GetMfgData(u8 data[]);

void CYRF_ConfigRxTx(u32 TxRx);
void CYRF_ConfigRFChannel(u8 ch);
void CYRF_ConfigCRCSeed(u8 crc);
void CYRF_StartReceive();
void CYRF_ConfigSOPCode(u32 idx);


u8 CYRF_ReadRSSI(u32 dodummyread);
void CYRF_StartReceive();

void CYRF_ReadDataPacket(u8 dpbuffer[]); 

/* UART & Debug */
void UART_Initialize();

#ifndef SKIP_PRINTF_DEFS
int printf(const char *format, ...);
int sprintf(char *out, const char *format, ...);
#endif
#endif
