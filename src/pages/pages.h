#ifndef _PAGES_H_
#define _PAGES_H_

#include "mixer_page.h"
#include "main_page.h"
#include "trim_page.h"
#include "timer_page.h"
#include "model_page.h"
#include "chantest_page.h"
#include "usb_page.h"

#define SECTION_MAIN    0
#define SECTION_MODEL   1
#define SECTION_OPTIONS 2
struct pagemem {
    union {
        struct main_page main_page;
        struct mixer_page mixer_page;
        struct trim_page trim_page;
        struct model_page model_page;
        struct timer_page timer_page;
        struct chantest_page chantest_page;
        struct usb_page usb_page;
    } u;
};

extern struct pagemem pagemem;

void PAGE_ShowHeader(const char *title);
u8 PAGE_SetModal(u8 _modal);
void PAGE_SetSection(u8 section);

/* Main */
void PAGE_MainInit(int page);
void PAGE_MainEvent();

/* Mixer */
void PAGE_MixerInit(int page);
void PAGE_MixerEvent();

/* Trim */
void PAGE_TrimInit(int page);
void PAGE_TrimEvent();

/* Timer */
void PAGE_TimerInit(int page);
void PAGE_TimerEvent();

/* Model */
void PAGE_ModelInit(int page);
void PAGE_ModelEvent();
void MODELPage_ShowLoadSave(int loadsave, void(*return_page)(int page));

/* Test */
void PAGE_TestInit(int page);
void PAGE_TestEvent();

/* Chantest */
void PAGE_ChantestInit(int page);
void PAGE_ChantestEvent();

/* Scanner */
void PAGE_ScannerInit(int page);
void PAGE_ScannerEvent();

/* USB */
void PAGE_USBInit(int page);
void PAGE_USBEvent();
void PAGE_USBExit();
#endif
