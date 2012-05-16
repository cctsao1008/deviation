/*
 * gui.h
 *
 *  Created on: Apr 29, 2012
 *      Author: matcat
 *      GUI Handling
 */

#ifndef GUI_H_
#define GUI_H_

enum DialogType {
    dtOk, dtOkCancel
};

enum BarGraphDirection {
    BAR_HORIZONTAL,
    BAR_VERTICAL
};

enum GUIType {
    UnknownGUI,
    Button,
    Label,
    Frame,
    CheckBox,
    Dropdown,
    Dialog,
    XYGraph,
    BarGraph,
};
struct guiImage {
    const char *file;
    u16 x_off;
    u16 y_off;
};

struct guiBox {
    u16 x;
    u16 y;
    u16 width;
    u16 height;
    struct guiImage image;
};
struct guiLabel {
    const char *text;
    u16 fontColor;
    u8 inuse;
};
struct guiFrame {
    u8 inuse;
};
struct guiButton {
    const char *text;
    u16 text_x_off;
    u16 text_y_off;
    u16 fontColor;
    u8 inuse;
};

struct guiXYGraph {
    s16 min_x;
    s16 min_y;
    s16 max_x;
    s16 max_y;
    s16 (*CallBack)(s16 xval, void * data);
    void *cb_data;
    u8 inuse;
};

struct guiBarGraph {
    s16 min;
    s16 max;
    u8 direction;
    s16 (*CallBack)(void * data);
    void *cb_data;
    u8 inuse;
};

struct guiDialogReturn {
    u8 buttonPushed;
    char strInput[80];
    int intInput;
};

struct guiDialog {
    const char *text;
    const char *title;
    enum DialogType Type;
    u16 fontColor;
    u16 titleColor;
    int buttonid[4];
    void (*CallBack)(int ObjID, struct guiDialogReturn gDR);
    u8 inuse;
};
struct guiObject {
    enum GUIType Type;
    void (*CallBack)(int ObjID);
    struct guiBox box;
    int GUIID;
    int TypeID;
    u8 Disabled; /* bool: Means this UI element is not 'active' */
    u8 Modal; /* bool: Means this UI element is active and all non-model elements are not */
    int parent;
};
u8 GUI_CheckModel(void);
int GUI_CreateDialog(u16 x, u16 y, u16 width, u16 height, const char *title,
        const char *text, u16 titleColor, u16 fontColor,
        void (*CallBack)(int ObjID, struct guiDialogReturn),
        enum DialogType dgType);
int GUI_CreateLabel(u16 x, u16 y, const char *text, u16 fontColor);
int GUI_CreateFrame(u16 x, u16 y, u16 width, u16 height, const char *image);
int GUI_CreateButton(u16 x, u16 y, u16 width, u16 height, const char *text,
        u16 fontColor, void (*CallBack)(int ObjID));
int GUI_CreateXYGraph(u16 x, u16 y, u16 width, u16 height, s16 min_x,
        s16 min_y, s16 max_x, s16 max_y, s16 (*Callback)(s16 xval, void * data), void * cb_data);
int GUI_CreateBarGraph(u16 x, u16 y, u16 width, u16 height, s16 min,
        s16 max, u8 direction, s16 (*Callback)(void * data), void * cb_data);
u8 GUI_CheckTouch(struct touch coords);
void GUI_DrawScreen(void);
void GUI_DrawObject(int ObjID);
void GUI_DrawObjects(void);
void GUI_RemoveObj(int objID);
int GUI_GetFreeObj(void);
int GUI_GetFreeGUIObj(enum GUIType guiType);
void dgCallback(int ObjID);
void GUI_DrawWindow(int ObjID);
#endif /* GUI_H_ */
