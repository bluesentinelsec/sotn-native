#ifndef SOTN_LABORATORY_H
#define SOTN_LABORATORY_H

#include <SDL3/SDL_events.h>
#include <sotn/game_data.h>

/** @brief Owned development Laboratory session, borrowing the application's core snapshot. */
typedef struct SotnLaboratory SotnLaboratory;

/**
 * @brief Create the project's Laboratory actor integration checkpoint.
 * @param data Borrowed verified core snapshot, alive until after session destruction.
 * @param directory Borrowed private resource directory used only during initialization.
 * @param start_room Verified development start: room 1 (entry), 3 (zombie corridor), or 23..25
 * (save rooms).
 * @param out Receives ownership on success, NULL on failure when non-NULL.
 * @return True on success, false with SDL_GetError() on unsupported or missing data.
 * @pre SDL video is initialized and calls occur on the main thread.
 * @note Supports selected empty-hand/Gladius/Mace combat, candle prizes, original sound effects,
 * and streaming Laboratory music in an explicit development profile. Other weapons, full
 * effects/audio parity, full progression saves, scripts, original camera/timing, and progression
 * remain incomplete.
 */
bool SotnLaboratory_Create(const SotnGameData *data, const char *directory, Uint32 start_room,
                           SotnLaboratory **out);

/** @brief Select supported development hand equipment; no inventory menu is implied.
 * @param laboratory Borrowed session. @param left Left-hand item ID. @param right Right-hand ID.
 * @return Success, or false with SDL_GetError(); stop after resource failure. */
bool SotnLaboratory_SetEquipment(SotnLaboratory *laboratory, Uint16 left, Uint16 right);

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

/** @brief Load supported ordinary progress into a fresh save-room session.
 * @param data Borrowed core snapshot. @param directory Private runtime resource directory.
 * @param slot Native slot, 1..15. @param out Receives ownership, NULL on failure.
 * @return Success, or false with SDL_GetError(); existing save remains unchanged.
 * @note Respawns transient actors; full inventory/map/progression and PS1 cards are unsupported. */
bool SotnLaboratory_Continue(const SotnGameData *data, const char *directory, Uint32 slot,
                             SotnLaboratory **out);

/** @brief Query completion of the selected original game-over presentation.
 * @param laboratory Borrowed session or NULL. @return True after the fade has finished. */
bool SotnLaboratory_Finished(const SotnLaboratory *laboratory);

/**
 * @brief Release session resources before closing the application's core snapshot.
 * @param laboratory Owned session, or NULL.
 */
void SotnLaboratory_Destroy(SotnLaboratory *laboratory);

#endif
