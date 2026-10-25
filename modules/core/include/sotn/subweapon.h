#ifndef SOTN_SUBWEAPON_H
#define SOTN_SUBWEAPON_H
#include <sotn/player.h>

enum
{
    SOTN_SUBWEAPON_SLOTS = 16
};
/** @brief Copied selected projectile state; coordinates use signed 16.16 room pixels. */
typedef struct SotnSubweaponState
{
    Sint32 x, y, velocity_x, velocity_y;
    Uint32 generation;
    Uint16 id, phase, timer, age, angle;
    SotnPlayerAttack attack;
    bool facing_left, visible;
    Sint32 visual_y;
} SotnSubweaponState;
/** @brief Owned bounded projectile service borrowing immutable DRA/map and the character. */
typedef struct SotnSubweapons SotnSubweapons;
/** @brief Allocate selected original projectile rules; no private tables are copied publicly.
 * @param dra Immutable private DRA, alive through destruction. @param character Borrowed profile.
 * @param map Borrowed current collision data. @param out Owned result, NULL on failure.
 * @return Success or false with SDL_GetError(). */
bool SotnSubweapons_Create(SotnDataView dra, SotnCharacter *character, SotnCollisionMap map,
                           SotnSubweapons **out);
/** @brief Request Up+attack: reserve capacity before consuming hearts or RNG.
 * @param service Borrowed simulation. @param player Copied living player state.
 * @param used Receives success of the action; false permits ordinary hand-attack fallback.
 * @param animation Original standing animation on use. @return API success or SDL error.
 * @note Selected dagger, axe, holy water and stopwatch paths are integrated. Other collected
 * subweapons report an explicit checkpoint limitation without charging hearts. */
bool SotnSubweapons_Use(SotnSubweapons *service, SotnPlayerState player, bool *used,
                        Uint16 *animation);
/** @brief Advance once before stage actor hits; never invoke while a menu pauses gameplay.
 * @param service Borrowed simulation. @param player Current copied pose.
 * @param camera_x Room-local camera. @param camera_y Room-local camera.
 * @return Success or false with SDL_GetError(); stop the session after failure. */
bool SotnSubweapons_Tick(SotnSubweapons *service, SotnPlayerState player, Sint32 camera_x,
                         Sint32 camera_y);
/** @brief Copy a bounded projectile slot. Inactive slots have generation zero.
 * @param service Borrowed simulation. @param slot Index below SOTN_SUBWEAPON_SLOTS.
 * @param out Copied state. @return Success or SDL error. */
bool SotnSubweapons_Get(const SotnSubweapons *service, Uint32 slot, SotnSubweaponState *out);
/** @brief Deliver original normal/metal hit flags for processing on the next tick.
 * @param service Borrowed simulation. @param slot Bounded active slot. @param flags 1..3.
 * @return Success or SDL error. */
bool SotnSubweapons_Hit(SotnSubweapons *service, Uint32 slot, Uint16 flags);
/** @brief Inspect the original stopwatch freeze flag; NULL returns false. */
bool SotnSubweapons_Frozen(const SotnSubweapons *service);
/** @brief Apply the original stage pauseEnemies gate to stopwatch use/termination.
 * @param service Borrowed owner. @param paused Stage interaction disables the stopwatch.
 * @return Success or SDL error for NULL. Other subweapons continue normally. */
bool SotnSubweapons_SetPaused(SotnSubweapons *service, bool paused);
/** @brief Drain bounded one-shot sound requests without replaying them on menu ticks.
 * @param service Borrowed simulation. @param sounds Writable output. @param capacity Entries.
 * @param count Receives written count. @return Success or SDL error; failure does not drain. */
bool SotnSubweapons_Sounds(SotnSubweapons *service, Uint16 *sounds, size_t capacity, Uint16 *count);
/** @brief Clear transient projectiles/freeze when entering another room.
 * @param service Borrowed simulation. @param map New immutable borrowed collision map.
 * @return Success or SDL error. Ordinary saves deliberately omit projectiles. */
bool SotnSubweapons_Enter(SotnSubweapons *service, SotnCollisionMap map);
/** @brief Release service, retaining borrowed resources. @param service Owned or NULL. */
void SotnSubweapons_Destroy(SotnSubweapons *service);
#endif
