#include "real_classifier_port.h"

// Your MPU6050 driver (local copy)
#include "modules/treeguard/MPU6050/I2Cdev.h"
#include "modules/treeguard/MPU6050/MPU6050.h"

// We DO NOT include your FFT implementation here.
// The classifier TU should be the sole owner of FFT symbols.
#include "modules/treeguard/TreeGuardSend.h" // send_from_classifier(...)
#include "modules/treeguard/classifier.h"    // defines Features, and your classify/extract fns

#include <Wire.h>
#include <math.h>

// Battery (optional, robust fallback to 0)
#ifdef USE_MAX1704X
#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>
static SFE_MAX1704X s_fg;
static bool s_fg_ok = false;
#endif

// ====== Sampling params taken from your project ======
#ifndef TG_ACCEL_SAMPLE_RATE_HZ
#define TG_ACCEL_SAMPLE_RATE_HZ 1000 // adjust to your original
#endif
#ifndef TG_ACCEL_NUM_SAMPLES
#define TG_ACCEL_NUM_SAMPLES 1024 // adjust to your original
#endif
#ifndef TG_PERIOD_MS
#define TG_PERIOD_MS 60000 // how often to run (default 60s)
#endif

static MPU6050 s_mpu;
static bool s_mpu_ok = false;

static float s_ax[TG_ACCEL_NUM_SAMPLES];
static float s_ay[TG_ACCEL_NUM_SAMPLES];
static float s_az[TG_ACCEL_NUM_SAMPLES];

static void tg_disableGyro()
{
    s_mpu.setSleepEnabled(false);
    delay(100);
    s_mpu.setStandbyXGyroEnabled(true);
    s_mpu.setStandbyYGyroEnabled(true);
    s_mpu.setStandbyZGyroEnabled(true);
}

static bool tg_initMPU()
{
    s_mpu.initialize();
    if (!s_mpu.testConnection())
        return false;
    tg_disableGyro();
    return true;
}

static void tg_collectBlock()
{
    const unsigned long dt_us = 1000000UL / TG_ACCEL_SAMPLE_RATE_HZ;
    unsigned long t_next = micros();

    for (int i = 0; i < TG_ACCEL_NUM_SAMPLES; i++) {
        int16_t ax, ay, az, gx, gy, gz;
        s_mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

        s_ax[i] = ax;
        s_ay[i] = ay;
        s_az[i] = az;

        t_next += dt_us;
        while ((long)(micros() - t_next) < 0) { /* precise wait */
        }
    }
}

static void tg_readBattery(uint32_t &vbat_mV, uint32_t &soc_x10)
{
    vbat_mV = 0;
    soc_x10 = 0;
#ifdef USE_MAX1704X
    if (s_fg_ok) {
        float v = s_fg.getVoltage(); // volts
        float s = s_fg.getSOC();     // percent
        vbat_mV = (uint32_t)lroundf(v * 1000.0f);
        soc_x10 = (uint32_t)lroundf(s * 10.0f);
    }
#endif
}

void tg_init()
{
    // I2C is already begun by Meshtastic main; don’t call Wire.begin() here.

    s_mpu_ok = tg_initMPU();

#ifdef USE_MAX1704X
    // Probe the fuel gauge (non-fatal if missing)
    s_fg_ok = s_fg.begin(Wire);
#endif
}

int32_t tg_tick()
{
    if (!s_mpu_ok) {
        // Try once more if the bus got ready later
        s_mpu_ok = tg_initMPU();
        if (!s_mpu_ok)
            return TG_PERIOD_MS;
    }

    tg_collectBlock();

    // ---- Your classifier API (adapt names to your actual header) ----
    // Expecting something like:
    //   Features f = extract_features(s_ax, s_ay, s_az, TG_ACCEL_NUM_SAMPLES);
    //   auto decision = classify(f);
    Features f = extract_features(s_ax, s_ay, s_az, TG_ACCEL_NUM_SAMPLES);
    meshtastic_tg_TreeGuardEvent_Decision decision = classify(f);

    uint32_t vbat_mV = 0, soc_x10 = 0;
    tg_readBattery(vbat_mV, soc_x10);

    // Ship over the mesh via your module helper
    send_from_classifier(f, decision, vbat_mV, soc_x10);

    return TG_PERIOD_MS; // run again later
}
