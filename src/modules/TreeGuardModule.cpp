// src/modules/TreeGuardModule.cpp

#include "MeshPacket.h"     // defines meshtastic_MeshPacket
#include "MeshService.h"    // brings in MeshModule, service.sendToMesh(), etc.
#include "ProtobufModule.h" // base for ProtobufModule<>
#include "configuration.h"  // VEXT_CTRL_PIN, INTERRUPT_PIN, etc.
#include "main.h"           // your global defs

#include <FS.h> // LittleFS deps
#include <LittleFS.h>

#include <Wire.h>
#include <esp_sleep.h>

#include "meshtastic/tree_guard.pb.h" // your new proto

class TreeGuardModule : public ProtobufModule<meshtastic_TreeGuardEvent>
{
  public:
    TreeGuardModule()
        : ProtobufModule<meshtastic_TreeGuardEvent>(
              meshtastic_PortNum_TREE_GUARD_APP // ← must match the new enum in portnums.proto
          )
    {
    }

    // called once at boot
    void setup() override
    {
        MeshModule::setup();
        initSensors();
        LittleFS.begin(true);

        auto reason = esp_sleep_get_wakeup_cause();
        switch (reason) {
        case ESP_SLEEP_WAKEUP_EXT0:
            classifyAndSend();
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            sendBatteryStatus();
            break;
        default:
            sendBatteryStatus();
            break;
        }

        enterDeepSleep();
    }

  protected:
    // **THIS** is the one pure‑virtual you **must** implement
    void handleReceivedProtobuf(const meshtastic_TreeGuardEvent &msg, const meshtastic_MeshPacket *request) override
    {
        // called whenever someone sends you a TreeGuardEvent
        (void)msg;
        (void)request;
        classifyAndSend();
    }

    // if you want to send replies *back* to the sender, override allocReply()
    meshtastic_MeshPacket *allocReply(const meshtastic_MeshPacket * /*request*/) override
    {
        static const char replyStr[] = "ACK";
        auto reply = allocDataPacket();
        reply->decoded.payload.size = sizeof(replyStr) - 1;
        memcpy(reply->decoded.payload.bytes, replyStr, reply->decoded.payload.size);
        return reply;
    }

  private:
    void initSensors()
    {
        // copy over your MPU/MAX init code
    }

    void classifyAndSend()
    {
        // collect data, run classifier, then:
        //   auto pkt = allocReply(nullptr);
        //   service.sendToMesh(pkt);
    }

    void sendBatteryStatus()
    {
        // read fuel gauge & send a small protobuf or raw packet
    }

    void enterDeepSleep()
    {
        // your deep‑sleep dance
    }
};

// this static instance gets your ctor run at startup
static TreeGuardModule _treeGuardModule;
