#ifndef _INTERFACE_H_
#define _INTERFACE_H_

enum ProtoCmds {
    PROTOCMD_INIT,
    PROTOCMD_BIND,
    PROTOCMD_CHECK_AUTOBIND,
    PROTOCMD_NUMCHAN,
};

#ifdef PROTO_HAS_A7105
#include "iface_a7105.h"
#endif

#ifdef PROTO_HAS_CYRF6936
#include "iface_cyrf6936.h"
#endif

#endif //_INTERFACE_H_
