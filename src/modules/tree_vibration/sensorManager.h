#pragma once
#include "MPU6050.h"
#include <variant.h>

#include "generated/meshtastic/treeshake.pb.h"

extern MPU6050 mpu;
extern volatile bool wakeup_flag;
extern int remainingCycles;
extern float inputBuffer[];

bool setupSensors();
bool monitorSensors();
void readSensorsToPB(meshtastic_TreeShake & msg);
bool initializeMPU();
void readAccelerometerDataForPhase(int phase);
bool checkFIFOOverflow();
void powerCycleMPU(bool on);
void enterDeepSleep();
void handleWakeUpInterrupt();