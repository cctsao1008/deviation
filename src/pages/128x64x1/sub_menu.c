#include "common.h"
#include "pages.h"
#include "icons.h"
#include "gui/gui.h"

static struct sub_menu_page * const smp = &pagemem.u.sub_menu_page;
static u8 action_cb(u32 button, u8 flags, void *data);
static const char *idx_string_cb(guiObject_t *obj, const void *data);
static void show_menuItems(u8 startIdx);
static void get_menu_itemName();
static void press_cb(guiObject_t *obj, s8 press_type, const void *data);
static void navigate_item(short direction, u8 step);

#define SUBMENU_COUNT 3
static const char *title0[] = {"Function Config", "Model Config", "Tx Config"};  // _tr(
static const char *menu_item_name_traditional0[] =  // _tr(
    {"Reverse Switch", "Travel Adjust", "Sub Trim", "D/R & Exp", "Thro hold", "Mix To Throttle",
      "Tail Curve", "Fail Safe", "Trainer", 0};
static const char *menu_item_name_traditional1[] =  // _tr(
    {"Model Select", "Model Name", "Protocol&Binding", "Mixer Mode","Type Select", "Swash Type", "Trim Step", "Power Amplifier", "Sensor Setting",
      "Stick Position", "Device Select","Device Output",  "Model Copy", "Model Transmit", "Model Receive", 0};
static const char *menu_item_name_traditional2[] = // _tr(
    {"Language", "Display", "Battery Alarm", "Calibration", "Buzzer", "Vibrator", "Stick Mode", "Stick Direction",
      "About", 0};

static const char *title1[] = {"Model Config", "Tx Config"};  // _tr(
static const char *menu_item_name_deviation0[] = // _tr(
    {"Mixer", "Model", "Protocol&Binding", "Timer", "Trim", "Mixer Mode", 0};
static const char *menu_item_name_deviation1[] = // _tr(
    {"Basic Config", "Monitor", "Scaner", "USB", 0};

u8 sub_menu_item = 0;  // global variable to let other page get back to the right sub menu
static u8 selected_menu_idx[SUBMENU_COUNT] = {0, 0, 0};
static u8 current_page_start_idx[SUBMENU_COUNT]  = {0, 0, 0};


/*
 * Main Menu page
 * KEY_UP,KEY_DOWN, KEY_LEFT, KEY_RIGHT: navigate among menu items
 * KEY_ENT: enter sub menu item or function page
 * Key_EXIT: back to the main page
 */
void PAGE_SubMenuInit(int page)
{
    sub_menu_item = page;
    PAGE_SetModal(0);
    PAGE_SetActionCB(action_cb);

    get_menu_itemName();
    smp->menu_item_count = 0;
    while (smp->menu_item_name[smp->menu_item_count] !=0) smp->menu_item_count++;
    show_menuItems(current_page_start_idx[sub_menu_item]);
    GUI_SetSelected(smp->menuItemObj[selected_menu_idx[sub_menu_item] - current_page_start_idx[sub_menu_item]]);
}

void PAGE_SubMenuExit()
{
}

void show_menuItems(u8 startIdx)
{
    GUI_RemoveAllObjects();
    if (Traditional_mixer == 1)
        PAGE_ShowHeader(_tr(title0[sub_menu_item]));
    else
        PAGE_ShowHeader(_tr(title1[sub_menu_item]));

    int i;
    u8 row = MENU_ITEM_START_ROW;
    u8 col = 1;
    struct LabelDesc labelDesc;
    labelDesc.font = DEFAULT_FONT.font;
    labelDesc.font_color = 0xffff;
    labelDesc.style = LABEL_LEFT;
    labelDesc.outline_color = 1;
    labelDesc.fill_color = labelDesc.outline_color; // not to draw box
    u8 idx_stringWidth = 15;
    u8 idx_stringYOffset = 1;
    current_page_start_idx[sub_menu_item] = startIdx;
    for (i = 0; i < PAGE_ITEM_COUNT; i++) {
        GUI_CreateLabelBox(col, row + idx_stringYOffset, idx_stringWidth, MENU_ITEM_HEIGHT,
            &TINY_FONT, idx_string_cb, NULL, (void *)((long)startIdx +1));
        smp->menuItemObj[i] =GUI_CreateLabelBox(col + idx_stringWidth +2, row, 0, 0,
            &labelDesc, PAGE_menuitem_cb, press_cb, (const void *)smp->menu_item_name[startIdx++]);
        if (startIdx >= smp->menu_item_count) {
            break;
        }
        row += MENU_ITEM_HEIGHT;
    }

    u8 y = MENU_ITEM_HEIGHT + 1;
    u8 x = LCD_WIDTH - ARROW_WIDTH +1;
    u8 h = LCD_HEIGHT - y ;
    smp->scroll_bar = GUI_CreateScrollbar(x, y, h, smp->menu_item_count, NULL, NULL, NULL);
}

void get_menu_itemName()
{
    if (Traditional_mixer == 1) {
        switch(sub_menu_item) {
        case 0:
            smp->menu_item_name = (char **)&menu_item_name_traditional0[0];
            break;
        case 1:
            smp->menu_item_name = (char **)&menu_item_name_traditional1[0];
            break;
        case 2:
            smp->menu_item_name = (char **)&menu_item_name_traditional2[0];
            break;
        default:
            return;
        }
    } else {
        switch(sub_menu_item) {
        case 0:
            smp->menu_item_name = (char **)&menu_item_name_deviation0[0];
            break;
        case 1:
            smp->menu_item_name = (char **)&menu_item_name_deviation1[0];
            break;
        default:
            return;
        }
    }
}

const char *idx_string_cb(guiObject_t *obj, const void *data)
{
    (void)obj;
    u8 idx = (long)data;
    snprintf(smp->tmpstr, 8, "%d.", idx);
    return smp->tmpstr;
}

void navigate_item(short direction, u8 step)
{
    short expectedIdx = selected_menu_idx[sub_menu_item] + direction* step;
    if (expectedIdx < 0) {
        selected_menu_idx[sub_menu_item] = 0;
    } else if (expectedIdx >= smp->menu_item_count) {
        selected_menu_idx[sub_menu_item] = smp->menu_item_count -1;
    }else {
        selected_menu_idx[sub_menu_item] = expectedIdx;
    }
    if (selected_menu_idx[sub_menu_item] < current_page_start_idx[sub_menu_item] ||
        selected_menu_idx[sub_menu_item] > current_page_start_idx[sub_menu_item] + PAGE_ITEM_COUNT -1) {
        show_menuItems(selected_menu_idx[sub_menu_item]);
    }
    GUI_SetScrollbar(smp->scroll_bar, selected_menu_idx[sub_menu_item]);
    GUI_SetSelected(smp->menuItemObj[selected_menu_idx[sub_menu_item] - current_page_start_idx[sub_menu_item]]);

}

u8 action_cb(u32 button, u8 flags, void *data)
{
    (void)data;
    if ((flags & BUTTON_PRESS) || (flags & BUTTON_LONGPRESS)) {
        if (CHAN_ButtonIsPressed(button, BUT_EXIT)) {
            PAGE_ChangeByName("MainMenu", 0);
        } else if (CHAN_ButtonIsPressed(button, BUT_UP)) {
            navigate_item(-1, 1);
        }else if (CHAN_ButtonIsPressed(button, BUT_DOWN)) {
            navigate_item(1, 1);
        } else if (CHAN_ButtonIsPressed(button, BUT_RIGHT)) {
            navigate_item(-1 * PAGE_ITEM_COUNT, 1);
        }  else if (CHAN_ButtonIsPressed(button, BUT_LEFT)) {
            navigate_item(PAGE_ITEM_COUNT, 1);
        } else {
            // only one callback can handle a button press, so we don't handle BUT_ENTER here, let it handled by press cb
            return 0;
        }
    }
    return 1;
}

/*
 *  invoked when the obj is selected with only ENTER/UP/DOWN is pressed
 */
void press_cb(guiObject_t *obj, s8 press_type, const void *data){
    (void)obj;
    (void)press_type;
    const char *str = (const char *)data;
    if (!strcmp("Calibration", str)) {
        PAGE_ChangeByName("Calibrate", 0);
    } else if (!strcmp("Protocol&Binding", str)) {
        PAGE_ChangeByName("MulItems", protocol);
    } else if (!strcmp("Language", str)) {
        PAGE_ChangeByName("SingItem", language);
    } else if (!strcmp("Stick Mode", str)) {
        PAGE_ChangeByName("SingItem", stickMode);
    } else if (!strcmp("Power Amplifier", str)) {
        PAGE_ChangeByName("SingItem", powerAmplifier);
    } else if (!strcmp("Swash Type", str)) {
        PAGE_ChangeByName("SingItem", swashType);
    } else if (!strcmp("Model Name", str)) {
        PAGE_ChangeByName("SingItem", modelName);
    } else if (!strcmp("Display", str)) {
        PAGE_ChangeByName("MulItems", display);
    } else if (!strcmp("Mixer Mode", str)) {
        //PAGE_ChangeByName("SingItem", mixerMode);
    } else if (!strcmp("Mixer", str)) {
        PAGE_ChangeByName("Mixer", 0);
    } else if (!strcmp("Basic Config", str)) {
        PAGE_ChangeByName("TxConfig", 0);
    } else if (!strcmp("Model", str)) {
        PAGE_ChangeByName("ModelCon", 0);
    } else if (!strcmp("Monitor", str)) {
        PAGE_ChangeByName("Monitor", 0);
    } else if (!strcmp("Trim", str)) {
        PAGE_ChangeByName("Trim", 0);
    } else if (!strcmp("Timer", str)) {
        PAGE_ChangeByName("Timer", 0);
    }
}
