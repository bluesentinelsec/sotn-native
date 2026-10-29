#ifndef SOTN_ACTORS_H
#define SOTN_ACTORS_H

#include <sotn/actor_layout.h>
#include <sotn/bat_runtime.h>
#include <sotn/camera.h>
#include <sotn/dialogue.h>
#include <sotn/gas_cloud.h>
#include <sotn/palette.h>
#include <sotn/player.h>
#include <sotn/primitive_slots.h>
#include <sotn/stage_route.h>
#include <sotn/wolf_runtime.h>
#include <sotn/world.h>

/** @brief Owned development actor simulation, borrowing private images and room data. */
typedef struct SotnActors SotnActors;
/** @brief Bounded native original-actor primitive budgets. */
enum
{
    SOTN_ACTOR_PRIMITIVES_MAX = SOTN_PRIMITIVE_SLOTS,
    SOTN_ACTOR_PRIMITIVES_PER_ACTOR_MAX = 384
};

/** @brief Original short simulated-pad interval used by stage door/event scripts. */
typedef struct SotnActorControl
{
    Uint16 buttons, timer;
    bool hold;            /**< Original simulated-pad mode 2: freeze normal-form player control. */
    bool block_stopwatch; /**< Original pauseEnemies gate, not a generic actor freeze. */
    bool pause_disabled;  /**< Original cutscene pause gate, also used by the native map menu. */
} SotnActorControl;
/** @brief Copy current script-owned pad interval; timer zero permits user control unless hold
 * remains set.
 * @param actors Borrowed simulation. @param out Copied initialized result.
 * @return Success or SDL error. */
bool SotnActors_GetControl(const SotnActors *actors, SotnActorControl *out);
/** @brief Original object shoves accumulated after the player update. */
typedef struct SotnActorMotion
{
    Sint32 x, y;     /**< Signed 16.16 displacement, at most 64 pixels per axis. */
    Uint32 contacts; /**< Original ground/ceiling/wall plus object-contact bits. */
    bool halt_vertical, protect,
        stop_velocity;    /**< Original door/event velocity stop and 32-tick protection. */
    bool bell_inside;     /**< Original bell concealment/status interval, recomputed each tick. */
    bool stop_horizontal; /**< Original tower-exit death guard; preserve vertical velocity. */
    Uint16 chair_kind;    /**< Transient original seating pose/facing hint, zero or 1..4. */
    bool align_horizontal;
    Sint32 alignment_x; /**< Original script's whole-pixel position override, retaining fraction. */
    bool align_vertical;
    Sint32 alignment_y; /**< Boss-owned whole-pixel vertical override, retaining fraction. */
    bool grab_changed;
    SotnPlayerGrab
        grab; /**< Original boss handshake request consumed before the next player tick. */
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
/** @brief Copy transient original room limits after actor updates and before room lookup.
 * @param actors Entered simulation. @param out Copied window, unchanged on failure.
 * @return Success or SDL error. Room entry resets all limits; no save state is changed. */
bool SotnActors_GetRoomWindow(const SotnActors *actors, SotnRoomWindow *out);
/** @brief Copy this tick's stage-owned palette interpolation request for the renderer.
 * @param actors Entered simulation. @param out Copied command, unchanged on failure.
 * @return Success or SDL error. Tick/room entry clears the command; private inputs stay borrowed.
 */
bool SotnActors_GetPaletteBlend(const SotnActors *actors, SotnPaletteBlend *out);
/** @brief Copy this tick's NO2 palette red increment; tick/room entry clears the request.
 * @param actors Borrowed owner. @param out Copied command, unchanged on invalid input.
 * @return Success or SDL error; the renderer owns and validates the affected palette memory. */
bool SotnActors_GetPaletteRedShift(const SotnActors *actors, SotnPaletteRedShift *out);

/** @brief Selected original sprite-transform bits, independently enabling each stored value. */
typedef enum SotnActorTransformFlag
{
    SOTN_ACTOR_SCALE_X = 1,
    SOTN_ACTOR_SCALE_Y = 2,
    SOTN_ACTOR_ROTATE = 4,
    SOTN_ACTOR_OPACITY = 8
} SotnActorTransformFlag;

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
    Sint16 rotation, bat_offset;          /**< BO4's original current-pose horizontal adjustment. */
    bool rotated, opacity_set, scale_set; /**< Legacy explicit-zero selectors. */
    Uint8 transform_flags; /**< Selected original bits above; stored values may remain disabled. */
    bool transform_flags_set; /**< When true, these bits govern transforms, including flags zero. */
    Uint16 primitive_count;
    Uint16 slot;
    Uint16 scale_x, scale_y, opacity; /**< Cached words; explicit transform bits override legacy
                                         zero/default selectors. */
} SotnActorState;

/** @brief Copied stage primitive, owned by its actor; positions are room-local 16.16 pixels. */
typedef struct SotnActorPrimitive
{
    Sint32 x, y;
    Uint16 width, height, page, palette;
    Uint8 u[4], v[4], red, green, blue;
    bool active, textured, blend, modulate;
    bool framebuffer;  /**< Sample the ordered RGB555 framebuffer snapshot, not stage artwork. */
    bool screen_space; /**< Projected screen coordinates; camera is already applied. */
    bool camera_fixed; /**< Original DRAW_ABSPOS/DRAW_MENU: omit shared camera-shake offsets. */
    bool priority_set; /**< Use this primitive's original ordering-table priority. */
    Uint16 priority;
    bool line; /**< Two-color line through offset vertices 0 and 1. */
    Uint8 line_red, line_green, line_blue, line_blend_mode;
    bool gouraud;
    Uint8 vertex_red[4], vertex_green[4], vertex_blue[4];
    bool quad; /**< Use explicit pixel offsets from x/y for a textured or colored quadrilateral. */
    Sint16 offset_x[4], offset_y[4];
    bool clip; /**< Ordered draw-environment change; bounds are screen pixels, right/bottom
                  exclusive. */
    Sint16 clip_left, clip_top, clip_right, clip_bottom;
    Uint8 target; /**< Ordered target: 0 normal, 1/2 strict begin/restore,
                      3/4 original replaceable environment begin/restore. */
    Sint16 target_offset_x, target_offset_y;
    bool target_clear;
} SotnActorPrimitive;
/** @brief Report whether live owned packets contain active offscreen environment commands.
 * @param actors Borrowed owner or NULL. @return True when a simulation frame needs its target
 * side effects rendered before actor state can advance; false for no commands or NULL. */
bool SotnActors_HasTargets(const SotnActors *actors);
/** @brief Read one owned actor primitive without exposing mutable storage.
 * @param actors Borrowed entered simulation. @param slot Stage slot. @param index Primitive 0..383.
 * @param out Copied state; inactive for unused indices. @return Success or SDL error. */
bool SotnActors_GetPrimitive(const SotnActors *actors, Uint32 slot, Uint32 index,
                             SotnActorPrimitive *out);

/** @brief Copy actual stage/player primitive occupancy for transactional ability planning.
 * @param actors Borrowed pool owner. @param out Receives all 1024 occupied bits.
 * @return Success or SDL error, leaving output unchanged on failure. */
bool SotnActors_PrimitiveSlots(const SotnActors *actors, SotnPrimitiveSlots *out);
/** @brief Atomically commit selected Bat Blink/trail/Echo reservations to the actual pool.
 * @param actors Borrowed session/pool owner. @param runtime Borrowed successful runtime snapshot.
 * @return Success or SDL error without pool mutation on invalid/colliding rows or generations.
 * Retires previous selected Bat rows only; stage-owned rows and ordinary progress survive.
 * Full shared Mist/other-ability allocation remains incomplete. */
bool SotnActors_CommitBatPrimitives(SotnActors *actors, const SotnBatRuntime *runtime);
/** @brief Commit the actual Wolf root/common-environment row plan to the stage pool.
 * @param actors Actual stage primitive owner. @param runtime Borrowed current body owner.
 * @return Success or SDL error, without pool mutation on invalid/overlapping/stale reservations.
 * Replaces only this selected root/environment owner's rows; never replaces stage or Bat rows.
 * Call before other allocators advance. This does not allocate bodies or enable Wolf controls. */
bool SotnActors_CommitWolfPrimitives(SotnActors *actors, const SotnWolfRuntime *runtime);
/** @brief Commit the single actual slot-47 Echo child's primitive reservation and packets.
 * @param actors Borrowed actual pool. @param generation Nonzero child identity, or zero to retire.
 * @param indices Forty-nine unique actual rows, or NULL when retiring.
 * @param echo Borrowed initialized component, or NULL when retiring. Copies all visible packets.
 * @return Success or SDL error without mutation on invalid/colliding ownership. Echo shares this
 * pool with stage actors and selected Bat flashes/trails. Full Mist sharing remains incomplete. */
bool SotnActors_CommitBatEcho(SotnActors *actors, Uint32 generation, const Uint16 *indices,
                              const SotnBatEcho *echo);
/** @brief Copy one actual Echo strip from its generation-owned primitive row.
 * @param actors Borrowed pool owner. @param generation Current nonzero child identity.
 * @param index Actual row 0..1023. @param out Copied packet, unchanged on stale ownership.
 * @return Success or SDL error; no simulation or allocation occurs. */
bool SotnActors_GetBatEchoPrimitive(const SotnActors *actors, Uint32 generation, Uint16 index,
                                    SotnActorPrimitive *out);

/** @brief Read the original scene backdrop color set by initialized stage actors.
 * @param actors Borrowed owner. @param out Receives RGB555, initially black.
 * @return Success or SDL error, unchanged output for invalid arguments. */
bool SotnActors_GetBackdrop(const SotnActors *actors, Uint16 *out);

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
    bool restart_stage_music; /**< Restart the current stage's private music profile. */
    Uint16 sequence;          /**< Original SEQ ID; zero stops only sequence-owned voices. */
    bool sequence_change;
    bool ambience_change; /**< Original channel 22/23 gain update without key-on. */
    Uint16 ambience_volume;
    Sint8 ambience_pan;
    Uint16 cd_command; /**< Original XA pause/resume command, zero when unchanged. */
    Uint16 voice;
    bool voice_change;
    bool voice_stop; /**< Stop XA speech independently of the sequence owner. */
    Uint16 contact_damage, contact_element;
    Uint16 blood_heal, blood_contacts; /**< Deferred first-heal request and consumed contacts. */
    Sint32 contact_x, contact_velocity;
    Uint32 kick_generation; /**< Current owned dive-kick child receiving contact feedback. */
    Uint16 kick_hit;        /**< Original hit result, 1..3; zero means no hit this tick. */
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
/** @brief Borrow current generation-owned Gas Cloud children; NULL disables this source.
 * @param actors Borrowed scene. @param cloud Borrowed owner, alive until replaced or destruction.
 * @return Success or SDL error. All children share one original group-four cooldown;
 * changing generations never resets an enemy's existing cooldown. */
bool SotnActors_SetGasCloud(SotnActors *actors, const SotnGasCloud *cloud);
/** @brief Borrow actual selected Bat attack children and original attack-group identities.
 * @param actors Borrowed scene. @param runtime Borrowed mutable owner for generation-checked impact
 * feedback; alive until replaced/destruction.
 * @return Success or SDL error. NULL disables this source; group cooldowns survive replacement. */
bool SotnActors_SetBatRuntime(SotnActors *actors, SotnBatRuntime *runtime);
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
