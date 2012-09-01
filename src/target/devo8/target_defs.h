#ifndef _DEVO8_TARGET_H_
#define _DEVO8_TARGET_H_

#define MIN_BRIGHTNESS 1 
//Protocols
#define PROTO_HAS_CYRF6936
#define PROTO_HAS_A7105

#define NUM_OUT_CHANNELS 12
#define NUM_VIRT_CHANNELS 10

#define NUM_TRIMS 6
#define MAX_POINTS 13
#define NUM_MIXERS ((NUM_OUT_CHANNELS + NUM_VIRT_CHANNELS) * 4)

#define INP_HAS_CALIBRATION 4

#define CHAN_ButtonMask(btn) (btn ? (1 << (btn - 1)) : 0)
#endif //_DEVO8_TARGET_H_
