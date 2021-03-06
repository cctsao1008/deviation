FILESYSTEM := devo8
SCREENSIZE := 320x240x16
DFU_ARGS   := -c 8 -b 0x08004000

include $(SDIR)/target/common/devo/Makefile.inc

ifndef BUILD_TARGET
SRC_C   += $(SDIR)/target/devo8/backlight.c \
           $(SDIR)/target/devo8/lcd.c \
           $(SDIR)/target/devo8/ili9341.c \
           $(SDIR)/target/devo8/hx8347.c
endif
