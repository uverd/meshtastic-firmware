#pragma once
#include <stdint.h>

// one-time init from TreeGuardModule::setup()
void tg_init();

// called on a schedule by a Periodic worker; return next delay (ms)
int32_t tg_tick();
