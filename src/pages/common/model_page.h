#ifndef __MODEL_PAGE_H_
#define __MODEL_PAGE_H_

enum {
   LOAD_MODEL,
   SAVE_MODEL,
   LOAD_TEMPLATE,
   LOAD_ICON,
};

struct model_page {
    char tmpstr[30];
    char iconstr[24];
    char fixed_id[7];
    u8 file_state;
/*Load save */
    s8 selected;
    u8 total_items;
    void(*return_page)(int page);
    enum ModelType modeltype;
};

#endif
