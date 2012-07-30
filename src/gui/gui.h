/*
 * gui.h
 *
 *  Created on: Apr 29, 2012
 *      Author: matcat
 *      GUI Handling
 */

#ifndef GUI_H_
#define GUI_H_

#define RGB888_to_RGB565(r, g, b) (((r & 0xf8) << 8) | ((g & 0xfc) << 3) | ((b & 0xf8) >>3))
enum DialogType {
    dtOk, dtOkCancel
};

enum BarGraphDirection {
    BAR_HORIZONTAL,
    BAR_VERTICAL,
    TRIM_HORIZONTAL,
    TRIM_VERTICAL,
};
enum TextSelectType {
    TEXTSELECT_128,
    TEXTSELECT_64,
    TEXTSELECT_96,
};

enum ButtonType {
    BUTTON_96,
    BUTTON_48,
    BUTTON_96x16,
    BUTTON_64x16,
    BUTTON_48x16,
    BUTTON_32x16,
};

enum KeyboardType {
    KEYBOARD_ALPHA,
    KEYBOARD_NUM,
    KEYBOARD_SPECIAL,
};

enum LabelType {
    NO_BOX,
    CENTER,
    FILL,
    TRANSPARENT,
};

struct guiDialogReturn {
    u8 buttonPushed;
    char strInput[80];
    int intInput;
};

struct LabelDesc {
    u8 font;
    u16 font_color;
    u16 fill_color;
    u16 outline_color;
    enum LabelType style;
};

struct ImageMap {
    const char *file;
    u8 width;
    u8 height;
    u16 x_off;
    u16 y_off;
};
#ifndef ENABLE_GUIOBJECT
typedef void guiObject_t;
#else
typedef struct guiObject guiObject_t;

#include "buttons.h"

enum ImageNames {
    FILE_BTN96_24,
    FILE_BTN48_24,
    FILE_BTN96_16,
    FILE_BTN64_16,
    FILE_BTN48_16,
    FILE_BTN32_16,
    FILE_SPIN96,
    FILE_SPIN64,
    FILE_SPIN32,
    FILE_SPINPRESS96,
    FILE_SPINPRESS64,
    FILE_SPINPRESS32,
    FILE_ARROW_16_UP,
    FILE_ARROW_16_DOWN,
    FILE_ARROW_16_RIGHT,
    FILE_ARROW_16_LEFT,
};
const struct ImageMap image_map[FILE_ARROW_16_LEFT+1];

enum GUIType {
    UnknownGUI,
    Button,
    Label,
    Image,
    CheckBox,
    Dropdown,
    Dialog,
    XYGraph,
    BarGraph,
    TextSelect,
    Listbox,
    Keyboard,
    Scrollbar,
    Rect,
};
struct guiImage {
    const char *file;
    u16 x_off;
    u16 y_off;
    void (*callback)(struct guiObject *obj, s8 press_type, void *data);
    void *cb_data;
};

struct guiBox {
    u16 x;
    u16 y;
    u16 width;
    u16 height;
};

struct guiLabel {
    struct LabelDesc desc;
    const char *(*strCallback)(struct guiObject *obj, void *data);
    void (*pressCallback)(struct guiObject *obj, s8 press_type, void *data);
    void *cb_data;
};

struct guiKeyboard {
    u8 lastchar;
    char *text;
    u8 num_chars;
    u8 caps;
    enum KeyboardType type;
    buttonAction_t action;
    void (*CallBack)(struct guiObject *obj, void *data);
    void *cb_data;
};

struct guiScrollbar {
    u8 state;
    u8 num_items;
    u8 cur_pos;
    buttonAction_t action;
    struct guiObject *parent;
    u8 (*callback)(struct guiObject *obj, u8 pos, s8 dir, void *data);
    void *cb_data;
};

struct guiButton {
    const struct ImageMap *image;
    u16 fontColor;
    const char *(*strCallback)(struct guiObject *obj, void *data);
    void (*CallBack)(struct guiObject *obj, void *data);
    void *cb_data;
};

struct guiListbox {
    u8 text_height;
    u8 entries_per_page;
    u8 item_count;
    u8 cur_pos;
    s16 selected;
    struct guiObject *scrollbar;
    const char * (*string_cb)(u8 idx, void * data);
    void (*select_cb)(struct guiObject *obj, u16 selected, void * data);
    void (*longpress_cb)(struct guiObject *obj, u16 selected, void * data);
    void *cb_data;
};

struct guiXYGraph {
    s16 min_x;
    s16 min_y;
    s16 max_x;
    s16 max_y;
    u16 grid_x;
    u16 grid_y;
    s16 (*CallBack)(s16 xval, void * data);
    u8 (*point_cb)(s16 *x, s16 *y, u8 pos, void *data);
    u8 (*touch_cb)(s16 x, s16 y, void *data);
    void *cb_data;
};

struct guiBarGraph {
    s16 min;
    s16 max;
    u8 direction;
    s16 (*CallBack)(void * data);
    void *cb_data;
};

struct guiDialog {
    const char *file;
    const char *text;
    const char *title;
    enum DialogType Type;
    u16 fontColor;
    u16 titleColor;
    struct guiObject *button[4];
    void (*CallBack)(guiObject_t *obj, struct guiDialogReturn gDR);
};

struct guiTextSelect {
    const struct ImageMap *button;
    u8 state;
    enum TextSelectType type;
    u16 fontColor;
    const char *(*ValueCB)(guiObject_t *obj, int dir, void *data);
    void (*SelectCB)(guiObject_t *obj, void *data);
    void *cb_data;
};

struct guiRect {
    struct LabelDesc desc;
};

struct guiObject {
    enum GUIType Type;
    struct guiBox box;
    u8 flags;
    struct guiObject *next;
    union {
        struct guiImage image;
        struct guiLabel label;
        struct guiButton button;
        struct guiXYGraph xy;
        struct guiBarGraph bar;
        struct guiDialog   dialog;
        struct guiTextSelect textselect;
        struct guiListbox listbox;
        struct guiKeyboard keyboard;
        struct guiScrollbar scrollbar;
        struct guiRect rect;
    } o;
};

#define OBJ_IS_USED(x)        ((x)->flags & 0x01) /* bool: UI element is in use */
#define OBJ_IS_HIDDEN(x)      ((x)->flags & 0x02) /* bool: UI element is not visible */
#define OBJ_IS_MODAL(x)       ((x)->flags & 0x04) /* bool: UI element is active and all non-model elements are not */
#define OBJ_IS_DIRTY(x)       ((x)->flags & 0x08) /* bool: UI element needs redraw */
#define OBJ_IS_TRANSPARENT(x) ((x)->flags & 0x10) /* bool: UI element has transparency */
#define OBJ_IS_SELECTABLE(x)  ((x)->flags & 0x20) /* bool: UI element can be selected */
#define OBJ_SET_USED(x,y)        (x)->flags = (y) ? (x)->flags | 0x01 : (x)->flags & ~0x01
#define OBJ_SET_HIDDEN(x,y)      (x)->flags = (y) ? (x)->flags | 0x02 : (x)->flags & ~0x02
#define OBJ_SET_MODAL(x,y)       (x)->flags = (y) ? (x)->flags | 0x04 : (x)->flags & ~0x04
#define OBJ_SET_DIRTY(x,y)       (x)->flags = (y) ? (x)->flags | 0x08 : (x)->flags & ~0x08
#define OBJ_SET_TRANSPARENT(x,y) (x)->flags = (y) ? (x)->flags | 0x10 : (x)->flags & ~0x10
#define OBJ_SET_SELECTABLE(x,y)  (x)->flags = (y) ? (x)->flags | 0x20 : (x)->flags & ~0x20

#define DRAW_NORMAL  0
#define DRAW_PRESSED 1
#define ARROW_LEFT  (&image_map[FILE_ARROW_16_LEFT])
#define ARROW_RIGHT (&image_map[FILE_ARROW_16_RIGHT])
#define ARROW_UP    (&image_map[FILE_ARROW_16_UP])
#define ARROW_DOWN  (&image_map[FILE_ARROW_16_DOWN])
#define ARROW_WIDTH 16
#define ARROW_HEIGHT 16

/* internal use only */
extern struct guiObject *objHEAD;
extern struct guiObject *objTOUCHED;
extern struct guiObject *objSELECTED;

void GUI_DrawKeyboard(struct guiObject *obj);
u8 GUI_TouchKeyboard(struct guiObject *obj, struct touch *coords, s8 press_type);

void GUI_DrawTextSelect(struct guiObject *obj);
u8 GUI_TouchTextSelect(struct guiObject *obj, struct touch *coords, s8 press_type);
void GUI_PressTextSelect(struct guiObject *obj, u32 button, u8 press_type);

void GUI_DrawXYGraph(struct guiObject *obj);
u8 GUI_TouchXYGraph(struct guiObject *obj, struct touch *coords, u8 long_press);

void GUI_DrawListbox(struct guiObject *obj, u8 redraw_all);
u8 GUI_TouchListbox(struct guiObject *obj, struct touch *coords, u8 long_press);
void GUI_PressListbox(struct guiObject *obj, u32 button, u8 press_type);

void GUI_DrawLabel(struct guiObject *obj);
u8 GUI_TouchLabel(struct guiObject *obj, struct touch *coords, s8 press_type);

void GUI_DrawDialog(struct guiObject *obj);

void GUI_DrawImage(struct guiObject *obj);
u8 GUI_TouchImage(struct guiObject *obj, struct touch *coords, s8 press_type);

void GUI_DrawButton(struct guiObject *obj);
void GUI_DrawBarGraph(struct guiObject *obj);
void GUI_DrawScrollbar(struct guiObject *obj);
u8 GUI_TouchScrollbar(struct guiObject *obj, struct touch *coords, s8 press_type);
void GUI_SetScrollbar(struct guiObject *obj, u8 pos);

void GUI_DrawRect(struct guiObject *obj);

void GUI_DrawObject(struct guiObject *obj);
void GUI_DrawBackground(u16 x, u16 y, u16 w, u16 h);
void GUI_DrawImageHelper(u16 x, u16 y, const struct ImageMap *map, u8 idx);
u8 coords_in_box(struct guiBox *box, struct touch *coords);
struct guiObject *GUI_GetFreeObj(void);
void connect_object(struct guiObject *obj);
#endif

guiObject_t *GUI_CreateDialog(u16 x, u16 y, u16 width, u16 height, const char *title,
        const char *text, u16 titleColor, u16 fontColor,
        void (*CallBack)(guiObject_t *obj, struct guiDialogReturn),
        enum DialogType dgType);
#define GUI_CreateLabel(x, y, cb, desc, data) GUI_CreateLabelBox(x, y, 0, 0, &desc, cb, NULL, data)
guiObject_t *GUI_CreateLabelBox(u16 x, u16 y, u16 width, u16 height, const struct LabelDesc *desc,
             const char *(*strCallback)(guiObject_t *, void *),
             void (*pressCallback)(guiObject_t *obj, s8 press_type, void *data),void *data);

#define GUI_CreateImage(x, y, w,h, file) GUI_CreateImageOffset(x, y, w, h, 0, 0, file, NULL, NULL)
guiObject_t *GUI_CreateImageOffset(u16 x, u16 y, u16 width, u16 height, u16 x_off, u16 y_off, const char *file,
        void (*CallBack)(guiObject_t *obj, s8 press_type, void *data), void *cb_data);

guiObject_t *GUI_CreateButton(u16 x, u16 y, enum ButtonType type, const char *(*strCallback)(guiObject_t *, void *),
        u16 fontColor, void (*CallBack)(guiObject_t *obj, void *data), void *cb_data);

guiObject_t *GUI_CreateIcon(u16 x, u16 y, const struct ImageMap *image,
        void (*CallBack)(guiObject_t *obj, void *data), void *cb_data);

guiObject_t *GUI_CreateListBox(u16 x, u16 y, u16 width, u16 height, u8 item_count, s16 selected,
        const char *(*string_cb)(u8 idx, void *data),
        void (*select_cb)(guiObject_t *obj, u16 selected, void *data),
        void (*longpress_cb)(guiObject_t *obj, u16 selected, void *data),
        void *cb_data);
guiObject_t *GUI_CreateXYGraph(u16 x, u16 y, u16 width, u16 height,
                      s16 min_x, s16 min_y, s16 max_x, s16 max_y,
                      u16 gridx, u16 gridy,
                      s16 (*Callback)(s16 xval, void *data), 
                      u8 (*point_cb)(s16 *x, s16 *y, u8 pos, void *data),
                      u8 (*touch_cb)(s16 x, s16 y, void *data),
                      void *cb_data);
guiObject_t *GUI_CreateBarGraph(u16 x, u16 y, u16 width, u16 height, s16 min,
        s16 max, u8 direction, s16 (*Callback)(void * data), void * cb_data);
guiObject_t *GUI_CreateTextSelect(u16 x, u16 y, enum TextSelectType type, u16 fontColor,
        void (*select_cb)(guiObject_t *obj, void *data),
        const char *(*value_cb)(guiObject_t *obj, int value, void *data),
        void *cb_data);
guiObject_t *GUI_CreateKeyboard(enum KeyboardType type, char *text, u8 num_chars,
        void (*CallBack)(guiObject_t *obj, void *data), void *cb_data);
guiObject_t *GUI_CreateScrollbar(u16 x, u16 y, u16 height,
        u8 num_items, guiObject_t *parent,
        u8 (*press_cb)(guiObject_t *parent, u8 pos, s8 direction, void *data), void *data);
guiObject_t *GUI_CreateRect(u16 x, u16 y, u16 width, u16 height, const struct LabelDesc *desc);

u8 GUI_CheckTouch(struct touch *coords, u8 long_press);
void GUI_TouchRelease();
void GUI_DrawScreen(void);
void GUI_RefreshScreen(void);
void GUI_Redraw(guiObject_t *obj);
void GUI_RedrawAllObjects();
void GUI_RemoveObj(guiObject_t *obj);
void GUI_RemoveAllObjects();
void GUI_RemoveHierObjects(guiObject_t *obj);
void GUI_SetHidden(guiObject_t *obj, u8 state);
guiObject_t *GUI_GetSelected();
void GUI_SetSelected(guiObject_t *obj);
void GUI_SetSelectable(guiObject_t *obj, u8 selectable);

guiObject_t *GUI_IsModal(void);
void GUI_HandleButtons(u8 enable);

s32 GUI_TextSelectHelper(s32 value, s32 min, s32 max, s8 dir, u8 shortstep, u8 longstep, u8 *_changed);
void GUI_TextSelectEnablePress(guiObject_t *obj, u8 enable);
void GUI_ChangeImage(guiObject_t *obj, const char *file, u16 x_off, u16 y_off);

#include "config/display.h"

#endif /* GUI_H_ */
