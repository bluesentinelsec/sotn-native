#ifndef SOTN_LABORATORY_H
#define SOTN_LABORATORY_H

#include <SDL3/SDL_events.h>
#include <sotn/game_data.h>

/** @brief Owned development Laboratory session, borrowing the application's core snapshot. */
typedef struct SotnLaboratory SotnLaboratory;

/** @brief Apply a named, explicit development fixture before play; never use for Continue.
 * @param laboratory Borrowed fresh checkpoint. @param name inventory, progression, dagger,
 * axe, holy-water, or stopwatch. @return Success or SDL error. Fixtures grant test resources;
 * they are not an original new-game start or a gameplay enhancement. */
bool SotnLaboratory_TestProfile(SotnLaboratory *laboratory, const char *name);

/**
 * @brief Create the project's Laboratory actor integration checkpoint.
 * @param data Borrowed verified core snapshot, alive until after session destruction.
 * @param directory Borrowed private resource directory used only during initialization.
 * @param start_room Verified development room: 1, 3..6, 9, 10, 14, 16..18, 20, or 22..25.
 * Room 16 starts at the boss entrance; rooms 23..25 are save rooms.
 * @param out Receives ownership on success, NULL on failure when non-NULL.
 * @return True on success, false with SDL_GetError() on unsupported or missing data.
 * @pre SDL video is initialized and calls occur on the main thread.
 * @note Integrates selected weapons/enemies, mechanisms, paired bosses, camera follow/locks,
 * original sound requests/music, and ordinary Laboratory saves in a development profile.
 * Complete effects/audio, castle scripts, other stages, and whole-game/timing parity remain
 * incomplete. Private runtime inputs remain borrowed and are never distributed.
 */
bool SotnLaboratory_Create(const SotnGameData *data, const char *directory, Uint32 start_room,
                           SotnLaboratory **out);

/** @brief Create a first-visit Entrance development session, never an original new game.
 * @param data Borrowed verified snapshot. @param directory Private runtime directory.
 * @param room Original room index. @param out Receives ownership or NULL.
 * @return Success or SDL error. Current checkpoint character and incomplete actors are explicit.
 */
bool SotnLaboratory_CreateEntrance(const SotnGameData *data, const char *directory, Uint32 room,
                                   SotnLaboratory **out);

/** @brief Select supported development hand equipment and synchronize character ownership.
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
 * @note Respawns transient actors after installing supported character, exploration, and permanent
 * Laboratory event progress. Map UI, full castle scripts, and PS1 cards remain unsupported. Pins
 * saved RNG streams after room initialization rather than serializing transient execution state. */
bool SotnLaboratory_Continue(const SotnGameData *data, const char *directory, Uint32 slot,
                             SotnLaboratory **out);

/** @brief Show the native checkpoint/Continue shell and scan slots without modifying files.
 * @param laboratory Borrowed initialized session. @return Success or SDL error.
 * @note Original title/new-game/card menus remain unintegrated. Held input is suppressed. */
bool SotnLaboratory_ShowLauncher(SotnLaboratory *laboratory);
/** @brief Apply an explicit launcher request transactionally on the main thread.
 * @param data Borrowed core snapshot, outliving either session. @param laboratory Owned pointer,
 * replaced only after successful initialization. @return False for invalid arguments; failed
 * file/resource operations retain the owner and display an explicit error dialog. Save files
 * are never written by this operation. @note A successful replacement invalidates old borrows. */
bool SotnLaboratory_UpdateSession(const SotnGameData *data, SotnLaboratory **laboratory);

/** @brief Query pending completion of game over, before showing the launcher.
 * @param laboratory Borrowed session or NULL. @return True after the fade has finished. */
bool SotnLaboratory_Finished(const SotnLaboratory *laboratory);

/**
 * @brief Release session resources before closing the application's core snapshot.
 * @param laboratory Owned session, or NULL.
 */
void SotnLaboratory_Destroy(SotnLaboratory *laboratory);

#endif
