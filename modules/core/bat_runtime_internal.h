#ifndef SOTN_BAT_RUNTIME_INTERNAL_H
#define SOTN_BAT_RUNTIME_INTERNAL_H
#include <sotn/bat_runtime.h>
/** @brief Validate selected Hellfire child ownership identically before update and row commit. */
bool SotnBatRuntime_HellfireChildValid(const SotnBatRuntimeChild *child, Uint16 index);
#endif
