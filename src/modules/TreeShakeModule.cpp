#include "TreeShakeModule.h"
#include "Default.h"
#include "MeshService.h"
#include "NodeDB.h"
#include "RTC.h"
#include "Router.h"
#include "configuration.h"
#include "main.h"
#include <Throttle.h>
#include "modules/tree_vibration/sensorManager.h"

#include "modules/tree_vibration/sensorManager.h"
#include "modules/tree_vibration/spiffsManager.h"
#include "modules/tree_vibration/powerManager.h"
#include "modules/tree_vibration/commandHandler.h"
#include "SPIFFS.h"

TreeShakeModule *treeShakeModule;

bool TreeShakeModule::handleReceivedProtobuf(const meshtastic_MeshPacket &mp, meshtastic_TreeShake *pptr)
{
    // auto p = *pptr;

    // bool hasChanged = nodeDB->updateUser(getFrom(&mp), p, mp.channel);

    // bool wasBroadcast = isBroadcast(mp.to);

    // Show new nodes on LCD screen
    // if (wasBroadcast) {
    //     String lcd = String("Joined: ") + p.long_name + "\n";
    //     if (screen)
    //         screen->print(lcd.c_str());
    // }

    // if user has changed while packet was not for us, inform phone
    // if (hasChanged && !wasBroadcast && !isToUs(&mp))
    //     service->sendToPhone(packetPool.allocCopy(mp));

    // LOG_DEBUG("did handleReceived");
    return false; // Let others look at this message also if they want
}

void TreeShakeModule::sendOurTreeShake(NodeNum dest, bool wantReplies, uint8_t channel, bool _shorterTimeout)
{
    // cancel any not yet sent (now stale) position packets
    
    if (prevPacketId) // if we wrap around to zero, we'll simply fail to cancel in that rare case (no big deal)
        service->cancelSending(prevPacketId);
    shorterTimeout = _shorterTimeout;
    meshtastic_MeshPacket *p = allocReply();
    if (p) { // Check whether we didn't ignore it
        p->to = dest;
        p->decoded.want_response = (config.device.role !=
        meshtastic_Config_DeviceConfig_Role_TRACKER &&
                                    config.device.role !=
                                    meshtastic_Config_DeviceConfig_Role_SENSOR)
                                    &&
                                   wantReplies;
        if (_shorterTimeout)
            p->priority = meshtastic_MeshPacket_Priority_DEFAULT;
        else
            p->priority = meshtastic_MeshPacket_Priority_BACKGROUND;
        if (channel > 0) {
            LOG_DEBUG("Send ourTreeShake to channel %d", channel);
            p->channel = channel;
        }

        prevPacketId = p->id;

        service->sendToMesh(p);
        shorterTimeout = false;
    }
}

meshtastic_MeshPacket *TreeShakeModule::allocReply()
{
    if (!airTime->isTxAllowedChannelUtil(false)) {
        ignoreRequest = true; // Mark it as ignored for MeshModule
        LOG_DEBUG("Skip send TreeShake > 40%% ch. util");
        return NULL;
    }
    // If we sent our TreeShake less than 5 min. ago, don't send it again as it may be still underway.
    if (!shorterTimeout && lastSentToMesh && Throttle::isWithinTimespanMs(lastSentToMesh, 5 * 60 * 1000)) {
        LOG_DEBUG("Skip send TreeShake since we sent it <5min ago");
        ignoreRequest = true; // Mark it as ignored for MeshModule
        return NULL;
    } else if (shorterTimeout && lastSentToMesh && Throttle::isWithinTimespanMs(lastSentToMesh, 60 * 1000)) {
        LOG_DEBUG("Skip send TreeShake since we sent it <60s ago");
        ignoreRequest = true; // Mark it as ignored for MeshModule
        return NULL;
    } else {
        ignoreRequest = false; // Don't ignore requests anymore
        meshtastic_TreeShake data;
        monitorSensors();
        readSensorsToPB(data);
        lastSentToMesh = millis();
        return allocDataProtobuf(data);
    }
}

TreeShakeModule::TreeShakeModule()
    //Private APP Port num is OK for now
    : ProtobufModule("treeshake", meshtastic_PortNum_PRIVATE_APP, &meshtastic_TreeShake_msg), concurrency::OSThread("TreeShake")
{

    Serial.println("Starting setup...");

    setupPower();               // Power up Vext
    delay(3000);                // Allow extra time for MPU6050 to stabilize

    if (!setupSensors()) {      // Initialize sensors and check for success
        Serial.println("Sensor setup failed. Restarting...");
        while (1);              // Halt if setup fails, for debugging
    }

    Serial.println("Setting up SPIFFS...");
    setupSPIFFS();              // Only set up SPIFFS after sensor initialization

    Serial.println("Setting up commands...");
    setupCommands();            // Initialize command handler for serial commands
    Serial.println("Setup complete.");

    isPromiscuous = true; // We always want to update our nodedb, even if we are sniffing on others
    setIntervalFromNow(30 *
                       1000); // Send our initial owner announcement 30 seconds after we start (to give network time to setup)
}

int32_t TreeShakeModule::runOnce()
{
    // If we changed channels, ask everyone else for their latest info
    bool requestReplies = currentGeneration != radioGeneration;
    currentGeneration = radioGeneration;

    if (airTime->isTxAllowedAirUtil() && config.device.role != meshtastic_Config_DeviceConfig_Role_CLIENT_HIDDEN) {
        LOG_INFO("Send our treeshake to mesh (wantReplies=%d)", requestReplies);
        sendOurTreeShake(NODENUM_BROADCAST, requestReplies); // Send our info (don't request replies)
    }
    return Default::getConfiguredOrDefaultMs(config.device.node_info_broadcast_secs, default_node_info_broadcast_secs);
}
