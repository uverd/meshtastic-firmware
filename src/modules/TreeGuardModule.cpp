#include "modules/TreeGuardModule.h"
#include "MeshService.h"
#include <Arduino.h> // millis
#include <cstring>   // memcpy
#include <pb_encode.h>

TreeGuardModule *gTreeGuard = nullptr;

TreeGuardModule::TreeGuardModule() : SinglePortModule("TreeGuard", meshtastic_PortNum_PRIVATE_APP)
{
    gTreeGuard = this;
}

void TreeGuardModule::setup()
{
    MeshModule::setup();
}

bool TreeGuardModule::wantPacket(const meshtastic_MeshPacket *p)
{
    (void)p;
    return false; // do not claim inbound packets
}

bool TreeGuardModule::sendEvent(const meshtastic_tg_TreeGuardEvent &e, NodeNum dest)
{
    // Encode to a stack buffer first
    uint8_t buf[meshtastic_tg_TreeGuardEvent_size];
    pb_ostream_t s = pb_ostream_from_buffer(buf, sizeof(buf));
    if (!pb_encode(&s, &meshtastic_tg_TreeGuardEvent_msg, &e))
        return false;

    // Compile-time sanity: message max must fit into a mesh payload
    static_assert(meshtastic_tg_TreeGuardEvent_size <= sizeof(((meshtastic_MeshPacket *)nullptr)->decoded.payload.bytes),
                  "TreeGuardEvent might not fit into a mesh packet payload");

    // Allocate a packet (no-arg in this tree)
    meshtastic_MeshPacket *p = this->allocDataPacket();
    if (!p)
        return false;

    if (dest)
        p->to = dest; // 0 == broadcast

    // Copy encoded bytes + set length
    memcpy(p->decoded.payload.bytes, buf, s.bytes_written);
    p->decoded.payload.size = s.bytes_written;

    // Queue for TX (void return here)
    service->sendToMesh(p);
    return true;
}
