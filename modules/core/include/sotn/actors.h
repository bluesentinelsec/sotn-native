#ifndef SOTN_ACTORS_H
#define SOTN_ACTORS_H

#include <sotn/actor_layout.h>
#include <sotn/camera.h>
#include <sotn/dialogue.h>
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
    Sint32 x, y;     /**< Signed 16.16 displacement, at most 64 pixels per axis. */
    Uint32 contacts; /**< Original ground/ceiling/wall plus object-contact bits. */
    bool halt_vertical, protect,
        stop_velocity; /**< Original door/event velocity stop and 32-tick protection. */
} SotnActorMotion;
/** @brief Borrow the verified boot image for original SDK tables and provide scene clock/audio.
 * @param actors Borrowed simulation. @param executable Immutable US boot image, alive through
 * actor destruction. @param ticks Original elapsed simulation ticks, capped at 99999 hours. @param
 * music_playing Scene music activity. @return Success or SDL error. No input bytes are copied or
 * executed. */
bool SotnActors_SetServices(SotnActors *actors, SotnDataView executable, Uint64 ticks,
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
/** @brief Own a writable copy of the room's background tile IDs for original stage mechanisms.
 * @param actors Entered owner. @param map Borrowed immutable layer; all-zero disables the layer.
 * @param out Receives its read-only view, valid until room entry/replacement/destruction.
 * Definitions remain borrowed. @return Success or SDL error; previous layer/output survive failure.
 */
bool SotnActors_SetBackground(SotnActors *actors, SotnCollisionMap map, SotnCollisionMap *out);
/** @brief Borrow foreground tile artwork metadata for original breakable-tile effects.
 * @param actors Entered service. @param pages Original texture-page bytes.
 * @param graphics Original packed UV-index bytes. @param palettes Original palette bytes.
 * Views must cover current tile definitions and live until next room entry/destruction.
 * @return Success or SDL error, retaining prior metadata on failure. No bytes are copied. */
bool SotnActors_SetTileArt(SotnActors *actors, SotnDataView pages, SotnDataView graphics,
                           SotnDataView palettes);
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
    bool tinted;
    Uint8 red, green, blue;
    Sint16 rotation, bat_offset; /**< BO4's original current-pose horizontal adjustment. */
    bool rotated, opacity_set;   /**< Explicit zero opacity is valid when opacity_set is true. */
    Uint16 primitive_count;
    Uint16 slot;
    Uint16 scale_x, scale_y,
        opacity; /**< Zero selects unit scale/neutral color unless opacity_set is true. */
} SotnActorState;

/** @brief Copied stage primitive, owned by its actor; positions are room-local 16.16 pixels. */
typedef struct SotnActorPrimitive
{
    Sint32 x, y;
    Uint16 width, height, page, palette;
    Uint8 u[4], v[4], red, green, blue;
    bool active, textured, blend, modulate;
    bool line; /**< Two-color line through offset vertices 0 and 1. */
    Uint8 line_red, line_green, line_blue, line_blend_mode;
    bool gouraud;
    Uint8 vertex_red[4], vertex_green[4], vertex_blue[4];
    bool quad; /**< Use explicit pixel offsets from x/y for a textured quadrilateral. */
    Sint16 offset_x[4], offset_y[4];
} SotnActorPrimitive;
/** @brief Read one owned actor primitive without exposing mutable storage.
 * @param actors Borrowed entered simulation. @param slot Stage slot. @param index Primitive 0..383.
 * @param out Copied state; inactive for unused indices. @return Success or SDL error. */
bool SotnActors_GetPrimitive(const SotnActors *actors, Uint32 slot, Uint32 index,
                             SotnActorPrimitive *out);

/** @brief Per-tick game sound requests and development inventory/contact state. */
typedef struct SotnActorCombat
{
    Uint16 volumes[256]; /**< Original volume multiplier; UINT16_MAX selects default. */
    Sint8 pans[256];     /**< Original signed stereo distance, -8..8. */
    Uint16 sounds[256];  /**< Original sound IDs; only sound_count entries are initialized. */
    Uint16 sound_count, hearts, hearts_max, subweapon;
    Uint32 gold, kills;
    Uint16 music; /**< Original music ID, zero for stop, when music_change is set. */
    bool music_change;
    bool ambience_change; /**< Original channel 22/23 gain update without key-on. */
    Uint16 ambience_volume;
    Sint8 ambience_pan;
    Uint16 cd_command; /**< Original XA pause/resume command, zero when unchanged. */
    Uint16 voice;
    bool voice_change;
    Uint16 contact_damage, contact_element;
    Sint32 contact_x, contact_velocity;
} SotnActorCombat;

/** @brief Copy dialogue state and feed native speech playback status into script waits.
 * @param actors Borrowed owner. @param voice_playing Native XA speech is still playing.
 * @param out Optional presentation snapshot. @return Success or SDL error.
 */
bool SotnActors_Dialogue(SotnActors *actors, bool voice_playing, SotnDialogueState *out);

/** @brief Feed completion of native XA pause/resume into the shared dialogue waits.
 * @param actors Borrowed owner. @param busy A CD transition is still in progress.
 * @return Success or SDL error for a missing owner. */
bool SotnActors_SetDialogueBusy(SotnActors *actors, bool busy);

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
/** @brief Create a stage-specific native dispatcher, borrowing immutable images.
 * @param stage_id Original ID: 12 (Laboratory), 65 (first-visit Entrance), or 7 (revisited
 * Entrance).
 * @param dra Borrowed DRA. @param overlay Borrowed stage image.
 * @param out Receives ownership, NULL on failure. @return Success or SDL error.
 * Unsupported actors remain explicitly marked; opening a profile does not imply complete parity.
 */
bool SotnActors_CreateStage(SotnDataView dra, SotnDataView overlay, Uint16 stage_id,
                            SotnActors **out);
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

/** @brief Borrow the BO4 simulation's death erasure mask after original capture timing.
 * @param actors Borrowed stage service. @param out Receives an empty view until capture;
 * otherwise a 512-byte bitset valid until the next update, room change or destruction.
 * @return Success or SDL error; no artwork bytes are returned. */
bool SotnActors_DoppleDeathMask(const SotnActors *actors, SotnDataView *out);

#endif
