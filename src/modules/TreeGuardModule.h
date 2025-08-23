#pragma once
#include "mesh/SinglePortModule.h"
#include "meshtastic/tree_guard.pb.h"

// Forward declared in the main tree
class MeshService;
extern MeshService *service;

class TreeGuardModule : public SinglePortModule
{
  public:
    TreeGuardModule();

    void setup() override;

    // We’re not consuming inbound packets yet.
    bool wantPacket(const meshtastic_MeshPacket *p) override;

    // Send our protobuf event; dest==0 => broadcast.
    bool sendEvent(const meshtastic_tg_TreeGuardEvent &e, NodeNum dest = 0);
};

// Global for the classifier helper
extern TreeGuardModule *gTreeGuard;
