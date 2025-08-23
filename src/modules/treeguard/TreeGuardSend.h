#pragma once
#include "meshtastic/tree_guard.pb.h"
#include "modules/TreeGuardModule.h"
#include <Arduino.h>
#include <math.h>

// Bring in your Features definition from your classifier if present.
// Fall back to a minimal definition so the code compiles even if
// you haven’t wired the classifier header yet.
#if __has_include("modules/treeguard/real_classifier.h")
#include "modules/treeguard/real_classifier.h" // defines struct Features
#elif __has_include("modules/treeguard/TreeGuardFeatures.h")
#include "modules/treeguard/TreeGuardFeatures.h" // defines struct Features
#else
struct Features {
    float hf_energy = 0, fft_0_25_ratio = 0, fft_125_200_ratio = 0;
    int strike_count = 0;
    float offset_x_y = 0, offset_y_z = 0, max_y = 0, max_z = 0;
};
#endif

extern TreeGuardModule *gTreeGuard;

inline void send_from_classifier(const Features &f, meshtastic_tg_TreeGuardEvent_Decision decision, uint32_t vbat_mV,
                                 uint32_t soc_x10)
{
    meshtastic_tg_TreeGuardEvent e = meshtastic_tg_TreeGuardEvent_init_default;
    e.decision = decision;
    e.ts_s = (uint32_t)(millis() / 1000);
    e.vbat_mV = vbat_mV;
    e.soc_x10 = soc_x10;
    e.hf_energy_x1000 = (int16_t)lroundf(f.hf_energy * 1000.0f);
    e.fft_0_25_ratio_x1000 = (int16_t)lroundf(f.fft_0_25_ratio * 1000.0f);
    e.fft_125_200_ratio_x1000 = (int16_t)lroundf(f.fft_125_200_ratio * 1000.0f);
    e.strike_count = (uint16_t)f.strike_count;
    e.offset_x_y = (int16_t)lroundf(f.offset_x_y);
    e.offset_y_z = (int16_t)lroundf(f.offset_y_z);
    e.max_y = (int16_t)lroundf(f.max_y);
    e.max_z = (int16_t)lroundf(f.max_z);

    if (gTreeGuard)
        gTreeGuard->sendEvent(e); // broadcast
}
