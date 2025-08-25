#include "modules/treeguard/TreeGuardSend.h"
#include "modules/TreeGuardModule.h"
#include <Arduino.h>
#include <math.h>

void send_from_classifier(const Features &f, meshtastic_tg_TreeGuardEvent_Decision decision, uint32_t vbat_mV, uint32_t soc_x10)
{
    // Get the singleton instance of our module
    TreeGuardModule *module = TreeGuardModule::getInstance();
    if (!module) {
        LOG_ERROR("TreeGuardModule instance is null, can't send packet.");
        return;
    }

    meshtastic_tg_TreeGuardEvent e = meshtastic_tg_TreeGuardEvent_init_default;
    e.decision = decision;
    e.ts_s = (uint32_t)(millis() / 1000); // Or use Meshtastic's time service if available
    e.vbat_mV = vbat_mV;
    e.soc_x10 = soc_x10;

    // Populate feature fields
    e.hf_energy_x1000 = (int16_t)lroundf(f.hf_energy * 1000.0f);
    e.fft_0_25_ratio_x1000 = (int16_t)lroundf(f.fft_0_25_ratio * 1000.0f);
    e.fft_125_200_ratio_x1000 = (int16_t)lroundf(f.fft_125_200_ratio * 1000.0f);
    e.strike_count = (uint16_t)f.strike_count;
    e.offset_x_y = (int16_t)f.offset_x_y;
    e.offset_y_z = (int16_t)f.offset_y_z;
    e.max_y = (int16_t)f.max_y;
    e.max_z = (int16_t)f.max_z;

    // Call the sendEvent method on our module instance
    module->sendEvent(e);
}