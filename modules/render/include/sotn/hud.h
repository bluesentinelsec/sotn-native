#ifndef SOTN_HUD_H
#define SOTN_HUD_H

#include <sotn/tilemap.h>

/** @brief Original Alucard status values consumed by the HUD. */
typedef struct SotnHudStatus
{
    Uint16 hp, hp_max, mp, mp_max, hearts, subweapon;
} SotnHudStatus;

/** @brief Owned original HUD presentation, borrowing immutable private artwork/tables. */
typedef struct SotnHud SotnHud;

/** @brief Initialize original HUD sprites and palettes from private DRA/common data.
 * @param dra Borrowed immutable US DRA, alive until destruction.
 * @param common Borrowed verified F_GAME.BIN, alive until destruction.
 * @param status Initial bounded status, with nonzero maximum HP/MP.
 * @param out Receives ownership, NULL on failure when non-NULL.
 * @return Success, or false with SDL_GetError() after releasing partial initialization. */
bool SotnHud_Create(SotnDataView dra, SotnDataView common, SotnHudStatus status, SotnHud **out);
/** @brief Advance original HP interpolation and HUD animation by one simulation tick.
 * @param hud Borrowed owned presentation. @param status Current bounded status.
 * @return Success, or false with SDL_GetError() and unchanged state on invalid status.
 * @note Rendering never advances the cursor. Gameplay regeneration remains in the game core. */
bool SotnHud_Tick(SotnHud *hud, SotnHudStatus status);
/** @brief Compose original HUD artwork, counters, and MP gauge over the RGB555 frame.
 * @param hud Borrowed presentation. @param frame Full writable framebuffer, not aliasing inputs.
 * @param count Available pixels, at least SOTN_FRAME_PIXELS.
 * @return Success, or false with SDL_GetError() and unchanged output on invalid arguments. */
bool SotnHud_Draw(const SotnHud *hud, Uint16 *frame, size_t count);
/** @brief Inspect the interpolated HP counter without advancing it.
 * @param hud Borrowed presentation, or NULL. @return Displayed HP, or zero for NULL. */
Uint16 SotnHud_DisplayedHp(const SotnHud *hud);
/** @brief Release presentation without freeing borrowed files. @param hud Owned object or NULL. */
void SotnHud_Destroy(SotnHud *hud);

#endif
