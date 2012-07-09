#ifndef _MODEL_H_
#define _MODEL_H_

#include "mixer.h"
struct Model {
    char name[24];
    enum Protocols protocol;
    u8 num_channels;
    u16 fixed_id;
    enum TxPower tx_power;
    enum SwashType swash_type;
    enum Mode mode;
    u8 swash_invert;
    u8 Elevator_Stick;
    u8 Aileron_Stick;
    u8 Collective_Stick;
    struct Trim trims[NUM_TRIMS];
    struct Mixer mixers[NUM_MIXERS];
    struct Limit limits[NUM_CHANNELS];
    u8 template[NUM_CHANNELS];
};
extern struct Model Model;

u8 CONFIG_ReadModel(u8 model_num);
u8 CONFIG_WriteModel(u8 model_num);
u8 CONFIG_SaveModelIfNeeded();
u8 CONFIG_GetCurrentModel();
#endif /*_MODEL_H_*/
