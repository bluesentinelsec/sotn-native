#ifndef SOTN_STARTUP_H
#define SOTN_STARTUP_H
#include <sotn/game_data.h>
#include <sotn/laboratory.h>
#include <sotn/title_screen.h>
/** @brief Native resident/SEL/title owner, before any gameplay session exists. */
typedef struct SotnStartup SotnStartup;
typedef struct SotnStartupState
{
    SotnTitleScreen title;
    Uint32 engine_clock, random;
    Uint16 previous_buttons, primitive_count, stage;
    Uint8 resident_step, sel_step;
    bool title_active, menu_requested, continue_active;
    Uint8 continue_slot;
} SotnStartupState;
/** @brief Prepare verified resident/SEL/title resources, actual SPU and a native display.
 * @param data Borrowed verified core snapshot, retained until Close. @param directory Trusted
 * private resource directory. @param out Owned startup context, NULL on failure.
 * @return Success or SDL error. Requires SDL video on its main thread. Creates no player/stage,
 * grants or saves. Native file preparation replaces console-disc loading; the explicit native
 * logo omission follows InitSel's VERSION_PC branch and claims no movie completion. */
bool SotnStartup_Open(const SotnGameData *data, const char *directory, SotnStartup **out);
/** @brief Attach the existing native filesystem Continue adapter without creating gameplay.
 * Opens only the verified common font and native save-store owner; reads retain invalid saves.
 * Original SEL memory-card UI, name entry and New Game/prologue are separate, unavailable owners.
 */
bool SotnStartup_EnableContinue(SotnStartup *startup);
/** @brief Transfer a successfully prepared actual saved session; NULL output if none is ready.
 * Load failure retains startup, saves, SPU and menu, and publishes a recoverable error.
 * Successful transfer carries engine clock/held input/SPU and save-store ownership without
 * checkpoint/new-game fallback or save writes. Caller closes startup only after taking the game. */
bool SotnStartup_TakeGame(SotnStartup *startup, SotnLaboratory **out);
/** @brief Drive actual host input/focus and fixed60Hz presentation for the startup owner. */
bool SotnStartup_Event(SotnStartup *startup, const SDL_Event *event);
bool SotnStartup_Iterate(SotnStartup *startup);
/** @brief Advance one startup engine update with actual physical button words.
 * Stop at the MainMenu request unless EnableContinue attached the native filesystem adapter.
 * Original SEL menus/New Game/prologue remain separate, unavailable owners. */
bool SotnStartup_Step(SotnStartup *startup, Uint16 buttons);
/** @brief Present the current owned pixels without ticking input, sound, RNG or any controller. */
bool SotnStartup_Draw(SotnStartup *startup);
/** @brief Copy an immutable current snapshot without adding an update. */
bool SotnStartup_GetState(const SotnStartup *startup, SotnStartupState *out);
/** @brief Pause/clear host playback after a focus/debugger gap; simulation words remain held. */
bool SotnStartup_ResetPlayback(SotnStartup *startup);
/** @brief Release display, audio, primitive and private resource owners. */
void SotnStartup_Close(SotnStartup *startup);
#endif
