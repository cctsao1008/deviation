#define NUM_TRIM_ELEMS 6
#define NUM_BOX_ELEMS 8
#define NUM_BAR_ELEMS 8
#define NUM_TOGGLE_ELEMS 4

#ifndef _MODEL_H_
#define _MODEL_H_

#include "mixer.h"
#include "timer.h"
#include "rtc.h"
#include "telemetry.h"
#include "datalog.h"
#include "pagecfg.h"

/* INI file consts */
const char *MODEL_NAME;
const char *MODEL_ICON;
const char *MODEL_TYPE;
const char *MODEL_TEMPLATE;
#define UNKNOWN_ICON "media/noicon.bmp"

//This cannot be computed, and must be manually updated
#define NUM_PROTO_OPTS 4
#define VIRT_NAME_LEN 10 

struct Model {
    char name[24];
    char icon[24];
    enum ModelType type;
    enum Protocols protocol;
    s16 proto_opts[NUM_PROTO_OPTS];
    u8 num_channels;
    u8 num_ppmin;
    u16 ppmin_centerpw;
    u16 ppmin_deltapw;
    u8 train_sw;
    s8 ppm_map[MAX_PPM_IN_CHANNELS];
    u32 fixed_id;
    enum TxPower tx_power;
    enum SwashType swash_type;
    u8 swash_invert;
    u8 swashmix[3];
    struct Trim trims[NUM_TRIMS];
    struct Mixer mixers[NUM_MIXERS];
    struct Limit limits[NUM_OUT_CHANNELS];
    char virtname[NUM_VIRT_CHANNELS][VIRT_NAME_LEN];
    struct Timer timer[NUM_TIMERS];
    u8 templates[NUM_CHANNELS];
    struct PageCfg2 pagecfg2;
    u8 safety[NUM_SOURCES+1];
    u8 telem_alarm[TELEM_NUM_ALARMS];
    s32 telem_alarm_val[TELEM_NUM_ALARMS];
    u8 telem_flags;
    MixerMode mixer_mode;
    u32 permanent_timer;
#if HAS_DATALOG
    struct datalog datalog;
#endif
#if HAS_VIDEO
    u8 videosrc;
    u8 videoch;
#endif
};
extern struct Model Model;
extern const char * const RADIO_TX_POWER_VAL[TXPOWER_LAST];

u8 CONFIG_ReadModel(u8 model_num);
u8 CONFIG_WriteModel(u8 model_num);
u8 CONFIG_GetCurrentModel();
const char *CONFIG_GetIcon(enum ModelType type);
const char *CONFIG_GetCurrentIcon();
enum ModelType CONFIG_ParseModelType(const char *value);
void CONFIG_ParseIconName(char *name, const char *value);
void CONFIG_ResetModel();
u8 CONFIG_ReadTemplateByIndex(u8 template_num);
u8 CONFIG_ReadTemplate(const char *filename);
u8 CONFIG_ReadLayout(const char *filename);
#endif /*_MODEL_H_*/
