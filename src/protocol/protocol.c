/*
 This project is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Deviation is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Deviation.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "common.h"
#include "interface.h"
#include "config/model.h"

#define PROTODEF(proto, map, cmd, name) extern u32 cmd(enum ProtoCmds);
#include "protocol.h"
#undef PROTODEF

static const u8 const EATRG[PROTO_MAP_LEN] =
    { INP_ELEVATOR, INP_AILERON, INP_THROTTLE, INP_RUDDER, INP_GEAR };
static const u8 const TAERG[PROTO_MAP_LEN] = 
    { INP_THROTTLE, INP_AILERON, INP_ELEVATOR, INP_RUDDER, INP_GEAR };
static const u8 const AETRG[PROTO_MAP_LEN] = 
    { INP_AILERON, INP_ELEVATOR, INP_THROTTLE, INP_RUDDER, INP_GEAR };

static u8 proto_state;
static u32 bind_time;
#define PROTO_READY   0x01
#define PROTO_BINDING 0x02
#define PROTO_BINDDLG 0x04

#define PROTODEF(proto, map, cmd, name) map,
const u8 *ProtocolChannelMap[PROTOCOL_COUNT] = {
    NULL,
    #include "protocol.h"
};
#undef PROTODEF

#define PROTODEF(proto, map, cmd, name) name,
const char * const ProtocolNames[PROTOCOL_COUNT] = {
    "None",
    #include "protocol.h"
};
#undef PROTODEF

void PROTOCOL_Init(u8 force)
{
    proto_state = 0;
    if (! force && PROTOCOL_CheckSafe()) {
        return;
    }
    proto_state |= PROTO_READY;
    
    #define PROTODEF(proto, map, cmd, name) case proto: cmd(PROTOCMD_INIT); break;
    switch(Model.protocol) {
        #include "protocol.h"
        case PROTOCOL_NONE:
        default:
            CLOCK_StopTimer();
            break;
    }
    #undef PROTODEF
}

void PROTOCOL_DeInit()
{
    CLOCK_StopTimer();
    proto_state = PROTO_READY;
}

u8 PROTOCOL_WaitingForSafe()
{
    return (proto_state & PROTO_READY) ? 0 : 1;
}

u32 PROTOCOL_Binding()
{

    if (proto_state & PROTO_BINDING) {
        if (bind_time == 0xFFFFFFFF)
            return bind_time;
        s32 tmp = bind_time - CLOCK_getms();
        return tmp > 0 ? tmp : 1;
    }
    return 0;
}

void PROTOCOL_SetBindState(u32 msec)
{
    if (msec) {
        if (msec == 0xFFFFFFFF)
            bind_time = msec;
        else
            bind_time = CLOCK_getms() + msec;
        proto_state |= PROTO_BINDING;
    } else {
        proto_state &= ~PROTO_BINDING;
    }
}

u32 PROTOCOL_CheckSafe()
{
    int i;
    s16 *raw = MIXER_GetInputs();
    u32 unsafe = 0;
    for(i = 0; i < NUM_SOURCES; i++) {
        if (! Model.safety[i])
            continue;
        s16 val = RANGE_TO_PCT((i < NUM_INPUTS)
                      ? raw[i+1]
                      : MIXER_GetChannel(i - (NUM_INPUTS), APPLY_SAFETY));
        if (Model.safety[i] == SAFE_MIN && val > -99)
            unsafe |= 1 << i;
        else if (Model.safety[i] == SAFE_ZERO && (val < -1 || val > 1))
            unsafe |= 1 << i;
        else if (Model.safety[i] == SAFE_MAX && val < 99)
            unsafe |= 1 << i;
    }
    return unsafe;
}

u8 PROTOCOL_AutoBindEnabled()
{
    u8 binding = 0;
    #define PROTODEF(proto, map, cmd, name) case proto: binding = cmd(PROTOCMD_CHECK_AUTOBIND); break;
    switch(Model.protocol) {
        #include "protocol.h"
        case PROTOCOL_NONE:
        default:
            binding = 1;
    }
    #undef PROTODEF
    return binding;
}

void PROTOCOL_Bind()
{
    #define PROTODEF(proto, map, cmd, name) case proto: cmd(PROTOCMD_BIND); break;
    switch(Model.protocol) {
        #include "protocol.h"
        case PROTOCOL_NONE:
        default: break;
    }
    #undef PROTODEF
}

void PROTOCOL_CheckDialogs()
{
    if (PROTOCOL_WaitingForSafe()) {
        PAGE_ShowSafetyDialog();
    } else {
        if (PROTOCOL_Binding()) {
            PAGE_ShowBindingDialog(proto_state & PROTO_BINDDLG);
            proto_state |= PROTO_BINDDLG;
        } else if (proto_state & PROTO_BINDDLG) {
            PAGE_CloseBindingDialog();
            MUSIC_Play(MUSIC_DONE_BINDING);
            proto_state &= ~PROTO_BINDDLG;
        }
    }
}
