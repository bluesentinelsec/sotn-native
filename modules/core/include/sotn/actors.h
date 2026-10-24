#ifndef SOTN_ACTORS_H
#define SOTN_ACTORS_H

#include <sotn/actor_layout.h>
#include <sotn/player.h>

/** @brief Owned development actor simulation, borrowing private images and room data. */
typedef struct SotnActors SotnActors;

/** @brief Copied actor state used by native presentation and replay inspection. */
typedef struct SotnActorState
{
    Sint32 x, y; /**< Original signed 16.16 coordinates, represented relative to the room. */
    Uint32 generation, flags;
    Uint16 id, parameters, step, animation_set, frame, palette, texture_offset, blend_mode,
        priority;
    bool facing_left, unsupported;
    Sint16 hit_points, hitbox_x, hitbox_y;
    Uint16 hitbox_width, hitbox_height, hitbox_state;
    Sint16 rotation;
    bool rotated;
    Uint8 primitive_count;
    Uint16 slot;
} SotnActorState;

/** @brief Copied stage primitive, owned by its actor; positions are room-local 16.16 pixels. */
typedef struct SotnActorPrimitive
{
    Sint32 x, y;
    Uint16 width, height, page, palette;
    Uint8 u[4], v[4], red, green, blue;
    bool active, textured, blend;
} SotnActorPrimitive;
/** @brief Read one owned actor primitive without exposing mutable storage.
 * @param actors Borrowed entered simulation. @param slot Stage slot. @param index Primitive 0..31.
 * @param out Copied state; inactive for unused indices. @return Success or SDL error. */
bool SotnActors_GetPrimitive(const SotnActors *actors, Uint32 slot, Uint32 index,
                             SotnActorPrimitive *out);

/** @brief Per-tick game sound requests and development inventory/contact state. */
typedef struct SotnActorCombat
{
    Uint16 sounds[256]; /**< Original sound IDs; only sound_count entries are initialized. */
    Uint16 sound_count, hearts, hearts_max, subweapon;
    Uint32 gold, kills;
    Uint16 contact_damage;
    Sint32 contact_x, contact_velocity;
} SotnActorCombat;

/** @brief Supported ordinary-save progress; transient room actors are excluded. */
typedef struct SotnActorProgress
{
    Uint16 hearts, subweapon;
    Uint32 gold, kills, random, combat_random;
} SotnActorProgress;

/** @brief Copy supported progress and owned RNG streams.
 * @param actors Borrowed entered simulation. @param out Copied progress on success.
 * @return Success, or false with SDL_GetError() and unchanged output. */
bool SotnActors_GetProgress(const SotnActors *actors, SotnActorProgress *out);
/** @brief Restore supported progress on a freshly entered save room.
 * @param actors Borrowed simulation. @param progress Validated supported progress.
 * @return Success, or false with SDL_GetError() and unchanged state on invalid input.
 * @note Caller recreates the session when loading; this does not restore temporary actor flags. */
bool SotnActors_Restore(SotnActors *actors, SotnActorProgress progress);

/**
 * @brief Initialize the supported normal Laboratory actor behaviors.
 * @param dra Borrowed immutable US main image, alive until destruction.
 * @param nz0 Borrowed immutable US stage image, alive until destruction.
 * @param out Receives ownership, NULL on failure when non-NULL.
 * @return True on success, false with SDL_GetError() for invalid input or allocation failure.
 * @note This checkpoint includes candles, red-eye busts, globe tables, and Bloody Zombie
 * motion, selected hand combat, death, and candle/prize collection in a development profile.
 * Complete audio/primitives, other equipped weapons, and other actor families remain incomplete.
 */
bool SotnActors_Create(SotnDataView dra, SotnDataView nz0, SotnActors **out);
/**
 * @brief Enter a room using its immutable layout and collision data.
 * @param actors Borrowed owned simulation.
 * @param layout Borrowed validated placement lists, alive until next entry or destruction.
 * @param map Borrowed immutable room collision map with the same lifetime.
 * @param camera_x Room-local camera X, 0..16384.
 * @param camera_y Room-local camera Y, 0..16384.
 * @return True on success, false with SDL_GetError() on invalid input.
 * @note Clears room actors while preserving stage-session persistence and random state.
 */
bool SotnActors_Enter(SotnActors *actors, const SotnActorLayout *layout, SotnCollisionMap map,
                      Sint32 camera_x, Sint32 camera_y);
/**
 * @brief Advance one original actor-update tick after player and camera updates.
 * @param actors Borrowed entered simulation.
 * @param player Copied player state within the current room's 256-pixel transition margin.
 * @param camera_x Room-local camera X, 0..16384.
 * @param camera_y Room-local camera Y, 0..16384.
 * @return True on success, false with SDL_GetError() on invalid or unsupported state/data.
 * @note Unsupported actor families are exposed explicitly in Get rather than simulated.
 * On failure stop the session; a partially advanced multi-actor tick is not resumable.
 */
bool SotnActors_Tick(SotnActors *actors, SotnPlayerState player, Sint32 camera_x, Sint32 camera_y);
/**
 * @brief Copy a stage-relative slot for inspection or rendering.
 * @param actors Borrowed simulation.
 * @param slot Stage-relative slot below SOTN_STAGE_ACTOR_SLOTS.
 * @param out Receives state; generation zero means inactive, unsupported means not integrated.
 * @return True on success, false with SDL_GetError() for invalid arguments.
 */
bool SotnActors_Get(const SotnActors *actors, Uint32 slot, SotnActorState *out);
/** @brief Copy this tick's sound/contact requests and the stage's development inventory.
 * @param actors Borrowed entered simulation. @param out Receives copied state on success.
 * @return Success, or false with SDL_GetError() for invalid arguments.
 * @note Requests are replaced on the next Tick. Inventory is a development profile, not a save. */
bool SotnActors_GetCombat(const SotnActors *actors, SotnActorCombat *out);
/** @brief Borrow shared character ownership for inventory pickups and derived combat stats.
 * @param actors Borrowed actor simulation. @param character Borrowed character, alive through
 * actor destruction. @return Success or false with SDL_GetError(). NULL retains legacy test
 * profile. */
bool SotnActors_SetCharacter(SotnActors *actors, SotnCharacter *character);
/** @brief Bind a borrowed projectile service, alive through actor destruction; NULL disables it.
 * @param actors Borrowed actors. @param service Borrowed service. @return Success or SDL error. */
bool SotnActors_SetSubweapons(SotnActors *actors, SotnSubweapons *service);
/** @brief Release actor state without freeing borrowed images/layout/map. @param actors Owned
 * simulation or NULL. */
void SotnActors_Destroy(SotnActors *actors);

#endif
