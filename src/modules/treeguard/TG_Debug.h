#pragma once
#ifndef UVERD_TG_DEBUG_H
#define UVERD_TG_DEBUG_H
#endif
#include <Arduino.h>

// ==============================
// 🔹 DEBUG CONFIGURATION
// ==============================

// Define DEBUG levels (0 = off, 1 = minimal, 2 = full)
#ifndef TG_DEBUG_LEVEL
#define TG_DEBUG_LEVEL 1 // Default to minimal debugging
#endif

#if TG_DEBUG_LEVEL > 0
#define TG_INIT_DEBUG_SERIAL() Serial.begin(115200) // Use Serial if debugging is enabled
#define TG_LOG_DEBUG(...) Serial.printf(__VA_ARGS__)
#define TG_LOG_DEBUGLN(...) Serial.println(__VA_ARGS__)
#define TG_DUMP_SD_PINS()                                                                                                        \
    do {                                                                                                                         \
        Serial.printf("CS: %d, MOSI: %d, MISO: %d, SCK: %d\n", digitalRead(SD_CS_PIN), digitalRead(SD_MOSI_PIN),                 \
                      digitalRead(SD_MISO_PIN), digitalRead(SD_SCK_PIN));                                                        \
    } while (0)
#else
#define TG_INIT_DEBUG_SERIAL() // Do nothing if debugging is off
#define TG_LOG_DEBUG(...)      // No-op when debugging is off
#define TG_LOG_DEBUGLN(...)
#endif

// ==============================
// 🔹 FEATURE TOGGLES
// ==============================

// Enable Power-Saving Mode (use `-DPOWER_SAVE_MODE` in platformio.ini)
#ifdef TG_POWER_SAVE_MODE
#define TG_DISABLE_UART 1
#define TG_DISABLE_WIFI 1
#define TG_DISABLE_BLUETOOTH 1
#else
#define TG_DISABLE_UART 0
#define TG_DISABLE_WIFI 0
#define TG_DISABLE_BLUETOOTH 0
#endif

// ==============================
// 🔹 FEATURE TOGGLE LOGGING
// ==============================

#if TG_DEBUG_LEVEL > 1
#define TG_LOG_FEATURE_TOGGLE()                                                                                                  \
    do {                                                                                                                         \
        TG_LOG_DEBUGLN("Feature Toggles Active:");                                                                               \
        TG_LOG_DEBUG(" - DISABLE_UART: ");                                                                                       \
        TG_LOG_DEBUGLN(DISABLE_UART);                                                                                            \
        TG_LOG_DEBUG(" - DISABLE_WIFI: ");                                                                                       \
        TG_LOG_DEBUGLN(DISABLE_WIFI);                                                                                            \
        TG_LOG_DEBUG(" - DISABLE_BLUETOOTH: ");                                                                                  \
        TG_LOG_DEBUGLN(DISABLE_BLUETOOTH);                                                                                       \
    } while (0)
#else
#define TG_LOG_FEATURE_TOGGLE() // No-op when debugging is off
#endif

// ==============================
// 🔹 SYSTEM MODE SELECTION
// ==============================

#define TG_TEST_MODE 0
#define TG_OPERATION_MODE 1

// Set this to TEST_MODE or OPERATION_MODE before compiling

#define TG_ACTIVE_MODE TG_OPERATION_MODE