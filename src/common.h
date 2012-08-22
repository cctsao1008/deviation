#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#include "target.h"

extern volatile s16 Channels[NUM_CHANNELS];

/* Temproary definition until we have real translation */
#define _tr(x) x
#define _tr_noop(x) x

/* LCD primitive functions */
void LCD_Clear(unsigned int color);
    /* Strings */
void LCD_PrintCharXY(unsigned int x, unsigned int y, const char c);
void LCD_PrintChar(const char c);
void LCD_PrintStringXY(unsigned int x, unsigned int y, const char *str);
void LCD_PrintString(const char *str);
void LCD_SetXY(unsigned int x, unsigned int y);
void LCD_GetStringDimensions(const u8 *str, u16 *width, u16 *height);
void LCD_GetCharDimensions(u8 c, u16 *width, u16 *height);
u8 LCD_SetFont(unsigned int idx);
u8  LCD_GetFont();
void LCD_SetFontColor(u16 color);
    /* Graphics */
void LCD_DrawCircle(u16 x0, u16 y0, u16 r, u16 color);
void LCD_FillCircle(u16 x0, u16 y0, u16 r, u16 color);
void LCD_DrawLine(u16 x0, u16 y0, u16 x1, u16 y1, u16 color);
void LCD_DrawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
void LCD_DrawFastHLine(u16 x, u16 y, u16 w, u16 color);
void LCD_DrawDashedVLine(int16_t x, int16_t y, int16_t h, int16_t space, uint16_t color);
void LCD_DrawDashedHLine(int16_t x, int16_t y, int16_t w, int16_t space, uint16_t color);
void LCD_DrawRect(u16 x, u16 y, u16 w, u16 h, u16 color);
void LCD_FillRect(u16 x, u16 y, u16 w, u16 h, u16 color);
void LCD_DrawRoundRect(u16 x, u16 y, u16 w, u16 h, u16 r, u16 color);
void LCD_FillRoundRect(u16 x, u16 y, u16 w, u16 h, u16 r, u16 color);
void LCD_DrawTriangle(u16 x0, u16 y0, u16 x1, u16 y1, u16 x2, u16 y2, u16 color);
void LCD_FillTriangle(u16 x0, u16 y0, u16 x1, u16 y1, u16 x2, u16 y2, u16 color);
void LCD_DrawWindowedImageFromFile(u16 x, u16 y, const char *file, s16 w, s16 h, u16 x_off, u16 y_off);
void LCD_DrawImageFromFile(u16 x, u16 y, const char *file);
u8 LCD_ImageIsTransparent(const char *file);
u8 LCD_ImageDimensions(const char *file, u16 *w, u16 *h);

/* Music */
enum Music {
    MUSIC_STARTUP = 0,
    MUSIC_ALARM1,
    MUSIC_ALARM2,
    MUSIC_BATT_ALARM,
    MUSIC_DONE_BINDING,
};
void MUSIC_Play(enum Music music);

/* Mixer functions */
void MIXER_CalcChannels();

/* GUI Pages */
void PAGE_Init();
void PAGE_Change(int dir);
void PAGE_Event();
void PAGE_ShowSafetyDialog();
void PAGE_CloseBindingDialog();
void PAGE_ShowBindingDialog(u8 update);

/* Protocol */
enum Protocols {
    PROTOCOL_NONE,
#ifdef PROTO_HAS_CYRF6936
    PROTOCOL_DEVO,
    PROTOCOL_WK2801,
    PROTOCOL_WK2601,
    PROTOCOL_WK2401,
    PROTOCOL_DSM2,
    PROTOCOL_J6PRO,
#endif
#ifdef PROTO_HAS_A7105
    PROTOCOL_FLYSKY,
#endif
    PROTOCOL_COUNT,
};
extern const u8 *ProtocolChannelMap[PROTOCOL_COUNT];
#define PROTO_MAP_LEN 5

enum ModelType {
    MODELTYPE_HELI,
    MODELTYPE_PLANE,
};

enum TxPower {
    TXPOWER_100uW,
    TXPOWER_300uW,
    TXPOWER_1mW,
    TXPOWER_3mW,
    TXPOWER_10mW,
    TXPOWER_30mW,
    TXPOWER_100mW,
    TXPOWER_150mW,
    TXPOWER_LAST,
};

void PROTOCOL_Init(u8 force);
void PROTOCOL_DeInit();
u8 PROTOCOL_WaitingForSafe();
u32 PROTOCOL_CheckSafe();
u32 PROTOCOL_Binding();
void PROTOCOL_SetBindState(u32 msec);
void PROTOCOL_CheckDialogs();

/* Input */
const char *INPUT_SourceName(char *str, u8 src);
const char *INPUT_ButtonName(u8 src);

/* Misc */
void Delay(u32 count);
u32 Crc(const void *buffer, u32 size);

#endif
