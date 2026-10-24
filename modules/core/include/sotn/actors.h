#ifndef SOTN_ACTORS_H
#define SOTN_ACTORS_H

#include <sotn/actor_layout.h>
#include <sotn/camera.h>
#include <sotn/player.h>
#include <sotn/world.h>

/** @brief Owned development actor simulation, borrowing private images and room data. */
typedef struct SotnActors SotnActors;

/** @brief Original short simulated-pad interval used by stage door/event scripts. */
typedef struct SotnActorControl
{
    Uint16 buttons, timer;
} SotnActorControl;
/** @brief Copy current script-owned pad interval; timer zero permits user control.
 * @param actors Borrowed simulation. @param out Copied initialized result.
 * @return Success or SDL error. */
bool SotnActors_GetControl(const SotnActors *actors, SotnActorControl *out);
/** @brief Original object shoves accumulated after the player update. */
typedef struct SotnActorMotion
{
    Sint32 x, y;                 /**< Signed 16.16 displacement, at most 64 pixels per axis. */
    Uint32 contacts;             /**< Original ground/ceiling/wall plus object-contact bits. */
    bool halt_vertical, protect; /**< Original door/event velocity stop and 32-tick protection. */
} SotnActorMotion;
/** @brief Borrow the verified boot image for original SDK tables and provide scene clock/audio.
 * @param actors Borrowed simulation. @param executable Immutable US boot image, alive through
 * actor destruction. @param seconds Original game-time seconds. @param music_playing Scene music
 * activity. @return Success or SDL error. No input bytes are copied or executed. */
bool SotnActors_SetServices(SotnActors *actors, SotnDataView executable, Uint32 seconds,
                            bool music_playing);
/** @brief Borrow the scene camera for original room-lock actors; NULL selects a room fallback.
 * @param actors Borrowed simulation. @param camera Borrowed writable scene-owned state.
 * @return Success or SDL error. The camera must outlive the actors. */
bool SotnActors_SetCamera(SotnActors *actors, SotnCamera *camera);
/** @brief Borrow persistent castle progress through actor destruction; NULL selects empty state.
 * @param actors Borrowed simulation. @param world Borrowed writable validated state.
 * @return Success or SDL error; does not take ownership. */
bool SotnActors_SetWorld(SotnActors *actors, SotnWorldState *world);
/** @brief Set held original pad bits for room mechanisms. @param actors Borrowed simulation.
 * @param buttons Original pad bits. @return Success or SDL error. */
bool SotnActors_SetButtons(SotnActors *actors, Uint16 buttons);
/** @brief Borrow current mutable room map, valid until next entry/destruction.
 * @param actors Entered simulation. @param out Receives read-only view; never free it.
 * @return Success or SDL error. */
bool SotnActors_GetMap(const SotnActors *actors, SotnCollisionMap *out);
/** @brief Copy this tick's object shoves for applying once after actor updates.
 * @param actors Entered simulation. @param out Receives copied motion.
 * @return Success or SDL error. */
bool SotnActors_GetMotion(const SotnActors *actors, SotnActorMotion *out);

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
    Uint16 scale_x, scale_y, opacity; /**< Zero selects original unit scale/neutral color. */
} SotnActorState;

/** @brief Copied stage primitive, owned by its actor; positions are room-local 16.16 pixels. */
typedef struct SotnActorPrimitive
{
    Sint32 x, y;
    Uint16 width, height, page, palette;
    Uint8 u[4], v[4], red, green, blue;
    bool active, textured, blend, modulate;
} SotnActorPrimitive;
/** @brief Read one owned actor primitive without exposing mutable storage.
 * @param actors Borrowed entered simulation. @param slot Stage slot. @param index Primitive 0..192.
 * @param out Copied state; inactive for unused indices. @return Success or SDL error. */
bool SotnActors_GetPrimitive(const SotnActors *actors, Uint32 slot, Uint32 index,
                             SotnActorPrimitive *out);

/** @brief Per-tick game sound requests and development inventory/contact state. */
typedef struct SotnActorCombat
{
    Uint16 sounds[256]; /**< Original sound IDs; only sound_count entries are initialized. */
    Uint16 sound_count, hearts, hearts_max, subweapon;
    Uint32 gold, kills;
    Uint16 music; /**< Original music ID, zero for stop, when music_change is set. */
    bool music_change;
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
 * @param map Borrowed room map; tile IDs are copied, definitions remain borrowed until next entry.
 * @param camera_x Room-local camera X, 0..16384.
 * @param camera_y Room-local camera Y, 0..16384.
 * @return True on success, false with SDL_GetError() on invalid input.
 * @note Clears room actors/destruction flags while preserving explicit world progress and RNG
 * state.
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
