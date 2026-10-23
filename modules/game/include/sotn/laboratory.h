#ifndef SOTN_LABORATORY_H
#define SOTN_LABORATORY_H

#include <SDL3/SDL_events.h>
#include <sotn/game_data.h>

/** @brief Owned development Laboratory session, borrowing the application's core snapshot. */
typedef struct SotnLaboratory SotnLaboratory;

/**
 * @brief Create the project's normal-Alucard movement integration checkpoint.
 * @param data Borrowed verified core snapshot, alive until after session destruction.
 * @param directory Borrowed private resource directory used only during initialization.
 * @param out Receives ownership on success, NULL on failure when non-NULL.
 * @return True on success, false with SDL_GetError() on unsupported or missing data.
 * @pre SDL video is initialized and calls occur on the main thread.
 * @note Starts in development room 1. Combat, enemies, audio, saves, stage scripts,
 * original camera behavior, and full-game progression are not implemented.
 */
bool SotnLaboratory_Create(const SotnGameData *data, const char *directory, SotnLaboratory **out);

/**
 * @brief Advance one game tick using explicitly supplied original button bits.
 * @param laboratory Borrowed owned session on the main thread.
 * @param buttons Original PS1 held-button bits accepted by the player slice.
 * @return True on success, false with SDL_GetError() on unsupported behavior.
 * @note Allows repeatable private-data tests independently of native input and wall time.
 */
bool SotnLaboratory_Step(SotnLaboratory *laboratory, Uint16 buttons);

/**
 * @brief Handle native gamepad and window lifecycle events.
 * @param laboratory Borrowed owned session.
 * @param event Borrowed SDL event.
 * @return True on success, false with SDL_GetError() on initialization failure.
 */
bool SotnLaboratory_Event(SotnLaboratory *laboratory, const SDL_Event *event);

/**
 * @brief Poll native input, advance a fixed 60-tick development clock, and draw.
 * @param laboratory Borrowed owned session.
 * @return True on success, false with SDL_GetError() on unsupported behavior.
 * @note Focus loss and long debugger pauses freeze the development clock.
 * This pacing adapter is not a claim of original NTSC timing parity.
 */
bool SotnLaboratory_Iterate(SotnLaboratory *laboratory);

/**
 * @brief Present the current session without advancing simulation.
 * @param laboratory Borrowed owned session.
 * @return True on success, false with SDL_GetError() on rendering errors.
 */
bool SotnLaboratory_Draw(SotnLaboratory *laboratory);

/**
 * @brief Release session resources before closing the application's core snapshot.
 * @param laboratory Owned session, or NULL.
 */
void SotnLaboratory_Destroy(SotnLaboratory *laboratory);

#endif
