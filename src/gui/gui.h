/*
 * gui.h
 *
 *  Created on: Apr 29, 2012
 *      Author: matcat
 *      GUI Handling
 */

#ifndef GUI_H_
#define GUI_H_

enum GUIType {
	Button,
	Label,
	Frame,
	CheckBox,
	Dropdown
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
	char text[512];
	struct guiBox box;
};
struct guiFrame {
	const char *text;
	struct guiBox box;
};
struct guiButton {
	char text[80];
	struct guiBox box;

};
struct guiObject {
	enum GUIType Type;
	void (*CallBack)(int ObjID);
	int GUIID;
	int TypeID;
};

char* GUI_GetText(int GUIID);
void GUI_SetText(int GUIID,const char *text);
int GUI_CreateLabel(u16 x, u16 y, const char *text);
int GUI_CreateButton(u16 x, u16 y, u16 width, u16 height, const char *text, void (*CallBack)(int ObjID));
void GUI_CheckTouch(struct touch coords);
void GUI_DrawScreen(void);
void GUI_DrawObjects(void);
void GUI_RemoveObj(int objID);
int GUI_GetFreeObj(void);
int GUI_GetFreeGUIObj(enum GUIType guiType);

#endif /* GUI_H_ */
