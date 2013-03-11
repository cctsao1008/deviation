#ifndef _INTERFACE_H_
#define _INTERFACE_H_

enum ProtoCmds {
    PROTOCMD_INIT,
    PROTOCMD_DEINIT,
    PROTOCMD_BIND,
    PROTOCMD_CHECK_AUTOBIND,
    PROTOCMD_NUMCHAN,
    PROTOCMD_DEFAULT_NUMCHAN,
    PROTOCMD_CURRENT_ID,
    PROTOCMD_SET_TXPOWER,
    PROTOCMD_GETOPTIONS,
    PROTOCMD_SETOPTIONS,
    PROTOCMD_TELEMETRYSTATE,
};

#ifndef MODULAR
#define PROTODEF(proto, map, cmd, name) extern const void * cmd(enum ProtoCmds);
#include "protocol.h"
#undef PROTODEF
#endif

#ifdef PROTO_HAS_A7105
#include "iface_a7105.h"
#endif

#ifdef PROTO_HAS_CYRF6936
#include "iface_cyrf6936.h"
#endif

#ifdef PROTO_HAS_CC2500
#include "iface_cc2500.h"
#endif

#endif //_INTERFACE_H_
