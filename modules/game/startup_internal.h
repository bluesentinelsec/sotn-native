#ifndef SOTN_STARTUP_INTERNAL_H
#define SOTN_STARTUP_INTERNAL_H
#include <sotn/laboratory.h>
#include <sotn/save.h>
#include <sotn/sound.h>
/** Actual startup save-store ownership transfers only after successful saved-session preparation.
 * No new-game/checkpoint fallback, resource grants, save writes or additional player tick. */
bool Laboratory_ContinueStartup(const SotnGameData *data, const char *directory,
                                SotnSaveStore *store, Uint32 slot, Uint32 engine_clock,
                                Uint16 buttons, const SotnSound *sound, SotnLaboratory **out);
#endif
