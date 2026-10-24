#ifndef SOTN_GAME_OVER_H
#define SOTN_GAME_OVER_H

#include <sotn/tilemap.h>

/** @brief Owned selected original US game-over presentation and captured scene. */
typedef struct SotnGameOver SotnGameOver;
/** @brief Capture the dying scene and borrow private game-over artwork.
 * @param artwork Verified F_GO.BIN, alive until destruction.
 * @param frame Borrowed full RGB555 capture, copied before return. @param count Pixel capacity.
 * @param out Receives ownership, NULL on failure. @return Success, or false with SDL_GetError(). */
bool SotnGameOver_Create(SotnDataView artwork, const Uint16 *frame, size_t count,
                         SotnGameOver **out);
/** @brief Advance selected original column fall, title reveal, hold, and fade clocks.
 * @param game_over Borrowed presentation. @param start Tapped original Start action.
 * @return Success, or false with SDL_GetError() for invalid state. */
bool SotnGameOver_Tick(SotnGameOver *game_over, bool start);
/** @brief Render without advancing presentation state.
 * @param game_over Borrowed presentation. @param frame Full writable RGB555 frame.
 * @param count Pixel capacity. @return Success, or false with SDL_GetError(). */
bool SotnGameOver_Draw(const SotnGameOver *game_over, Uint16 *frame, size_t count);
/** @brief Query completion of the fade back to the unintegrated title flow.
 * @param game_over Borrowed presentation or NULL. @return True only after completion. */
bool SotnGameOver_Finished(const SotnGameOver *game_over);
/** @brief Release the capture without freeing borrowed artwork. @param game_over Owned or NULL. */
void SotnGameOver_Destroy(SotnGameOver *game_over);

#endif
