#ifndef SOTN_PLAYER_H
#define SOTN_PLAYER_H

#include <sotn/animation.h>
#include <sotn/bat_command.h>
#include <sotn/character.h>
#include <sotn/collision.h>
#include <sotn/mist_effect.h>
#include <sotn/player_afterimage.h>
#include <sotn/player_blink.h>
#include <sotn/player_factory.h>
#include <sotn/wolf_runtime.h>
#include <sotn/wolf_transition.h>
#include <sotn/world.h>

/** @brief Original PS1 button bits accepted by normal Alucard and supported hand attacks. */
typedef enum SotnPlayerButton
{
    SOTN_BUTTON_SELECT = 0x0100,
    SOTN_BUTTON_WOLF = 0x0002,
    SOTN_BUTTON_MIST = 0x0004,
    SOTN_BUTTON_BAT = 0x0008,
    SOTN_BUTTON_START = 0x0800,
    SOTN_BUTTON_CANCEL = 0x0010,
    SOTN_BUTTON_JUMP = 0x0040,
    SOTN_BUTTON_RIGHT_HAND = 0x0020,
    SOTN_BUTTON_LEFT_HAND = 0x0080,
    SOTN_BUTTON_UP = 0x1000,
    SOTN_BUTTON_RIGHT = 0x2000,
    SOTN_BUTTON_DOWN = 0x4000,
    SOTN_BUTTON_LEFT = 0x8000
} SotnPlayerButton;

/** @brief Copied original stage depth globals; independent feet, waist and head samples. */
typedef struct SotnPlayerWater
{
    Uint16 feet, waist, head;
} SotnPlayerWater;

/** @brief Original normal-Alucard movement state IDs. */
typedef enum SotnPlayerStep
{
    SOTN_PLAYER_STAND = 0,
    SOTN_PLAYER_WALK,
    SOTN_PLAYER_CROUCH,
    SOTN_PLAYER_FALL,
    SOTN_PLAYER_JUMP,
    SOTN_PLAYER_BAT = 5,
    SOTN_PLAYER_STUCK = 6,
    SOTN_PLAYER_MIST = 7,
    SOTN_PLAYER_HIGH_JUMP = 8,
    SOTN_PLAYER_UNMORPH_BAT = 9,
    SOTN_PLAYER_HIT = 10,
    SOTN_PLAYER_STONE = 11,
    SOTN_PLAYER_BOSS_GRAB = 12,
    SOTN_PLAYER_UNMORPH_MIST = 14,
    SOTN_PLAYER_KILL = 16,
    SOTN_PLAYER_WOLF = 24,
    SOTN_PLAYER_UNMORPH_WOLF = 25,
    SOTN_PLAYER_DARK = 32,
    SOTN_PLAYER_SUMMON = 33,
    SOTN_PLAYER_HELLFIRE = 34,
    SOTN_PLAYER_TETRA = 35,
    SOTN_PLAYER_SOUL = 37,
    SOTN_PLAYER_SCRIPT = 49
} SotnPlayerStep;

/** @brief Original grab handshake used by Hippogryph; its release clears the state to NONE.
 * Other bosses' thrown-release state 4 and stone reactions are not integrated here. */
typedef enum SotnPlayerGrabState
{
    SOTN_GRAB_NONE = 0,
    SOTN_GRAB_START = 1,
    SOTN_GRAB_HELD = 2,
    SOTN_GRAB_DAMAGE = 3
} SotnPlayerGrabState;

/** @brief Transient boss-owned request and player-owned acknowledgement, never ordinary save data.
 */
typedef struct SotnPlayerGrab
{
    SotnPlayerGrabState state;
    Uint16 damage; /**< Original raw damage before physical defense and poison. */
    Sint32 source_x, source_velocity; /**< Whole-pixel source and signed 16.16 velocity. */
} SotnPlayerGrab;

/** @brief Runtime supported hand-attack hitbox, in whole pixels relative to Alucard's origin. */
typedef struct SotnPlayerAttack
{
    Sint16 offset_x, offset_y;
    Uint16 width, height, element, hit_type, hit_effect, stun_frames;
    Uint8 invincibility_frames, critical_rate;
    bool active, right_hand, sound;
    Uint16 power, sound_id, equipment, sprite;
} SotnPlayerAttack;

/** @brief Owned transient dive-kick hitbox; generation zero means inactive.
 * The original child follows the player independently of either hand/shield. */
typedef struct SotnPlayerKick
{
    SotnPlayerAttack attack;
    Uint32 generation;
} SotnPlayerKick;

enum
{
    SOTN_PLAYER_GRAVITY_BEAMS = 4
};
/** @brief One owned cosmetic Gravity Boots beam, independent of player movement state. */
typedef struct SotnPlayerGravityBeam
{
    Sint32 x, y;  /**< Anchored room-local 16.16 origin; endpoint follows the current player. */
    Uint16 width; /**< Original narrowing timer, 0..1536; first quad half-width is width/256. */
    Uint8 phase;
    bool active;
} SotnPlayerGravityBeam;
/** @brief Selected original high-jump presentation and duration, never ordinary save state. */
typedef struct SotnPlayerGravity
{
    Uint16 timer;
    bool upside_down;
    bool ceiling_impact; /**< One simulation update; an intent, never child/allocation readiness. */
    SotnPlayerFactorySource impact_source; /**< Captured room-space origin before facing flip. */
    SotnPlayerGravityBeam beams[SOTN_PLAYER_GRAVITY_BEAMS];
} SotnPlayerGravity;

/** @brief Selected original physical-death effect cursor; mask cells repeat every 16 pixels. */
typedef struct SotnPlayerDeath
{
    Uint8 phase, red, blue, mask[32];
    Uint16 timer, cursor;
    Sint16 rotation;
} SotnPlayerDeath;

/** @brief Selected petrification presentation and directional escape cursor. */
typedef struct SotnPlayerStone
{
    Uint16 palette, shakes, death_wait;
    Sint16 rotation, pivot;
    bool active, hidden, impact;
} SotnPlayerStone;

/** @brief One original floating sleep letter, owned by the player simulation. */
typedef struct SotnPlayerSleep
{
    Sint32 x, y, velocity_x; /**< Signed 16.16 room-local pixels. */
    Uint16 pose, frame;
    Sint16 remaining;
    bool active;
} SotnPlayerSleep;

/** @brief Copied player requests for its real, generation-owned Mist visual child. */
typedef struct SotnPlayerMist
{
    Uint32 generation;
    Uint16 capture_sprite, capture_bank;
    bool active, restoring, started, request_restore, retire;
} SotnPlayerMist;

/** @brief Selected Bat controller requests for actual generation-owned transformation effects. */
typedef struct SotnPlayerBat
{
    Uint32 generation;
    Sint16 rotation, settle_y;
    Uint8 signal;
    bool active, hidden, started, return_started, cramped;
} SotnPlayerBat;

/** @brief Copied requests and drawing/controller state for actual generation-owned Wolf bodies. */
typedef struct SotnPlayerWolf
{
    SotnWolfControllerFrame controller;
    SotnWolfControllerEvents commands; /**< Owned player exports captured room-local sources. */
    Uint32 generation, body_generation;
    Uint16 capture_bank, capture_sprite, palette, texture, priority;
    Sint16 rotation;
    Uint8 signal, draw_flags, blend, opacity;
    bool active, hidden, started, body_started, finish_flash, return_started, cramped;
    bool return_factory_missing; /**< Original unchecked allocation failed; no readiness exists. */
} SotnPlayerWolf;

/** @brief Captured original ordinary-movement factory intent; blueprint zero is valid.
 * A request is not a successful factory/child allocation. Sources remain room-local. */
typedef struct SotnPlayerMovementEffects
{
    SotnPlayerFactorySource smoke_source;
    Uint32 smoke_blueprint;
    bool smoke;
} SotnPlayerMovementEffects;

/** @brief Original deferred blood-healing state and this update's owned effect requests. */
typedef struct SotnPlayerHealing
{
    SotnPlayerFactorySource source;
    Uint32 contacts;
    Uint16 pending;
    bool applied, effects;
} SotnPlayerHealing;

/** @brief Selected owned Dark Metamorphosis timer and one-update cast request. */
typedef struct SotnPlayerDark
{
    SotnPlayerFactorySource source;
    Uint16 ticks;
    bool started;
} SotnPlayerDark;

/** @brief One-update original spirit cast, fulfilled only by its actual native owner. */
typedef struct SotnPlayerSpirit
{
    SotnPlayerFactorySource source;
    Uint32 blueprint;
    Uint16 spell;
    bool started;
} SotnPlayerSpirit;

/** @brief Original Soul Steal cast/pose-seven requests, consumed by actual native owners. */
typedef struct SotnPlayerSoul
{
    SotnPlayerFactorySource source;
    bool started, attack;
} SotnPlayerSoul;

/** @brief Read-only copy of a player's current normal-movement state. */
typedef struct SotnPlayerState
{
    SotnPlayerWater water;
    Sint32 x, y, velocity_x, velocity_y; /**< Signed 16.16 room-local pixels. */
    Uint32 status;   /**< Original status bits for stage scripts and enemy observations. */
    Uint32 contacts; /**< Original floor/ceiling/wall and slope flags. */
    SotnAnimation animation;
    SotnPlayerStep step;
    Uint16 substep;   /**< Original movement substate; standing 4 is seated. */
    bool sleep_event; /**< Original sleeping-letter factory requested this tick. */
    SotnPlayerSleep sleep;
    bool facing_left;
    SotnPlayerAttack attack;
    Uint16 hp, hp_max;
    bool invulnerable;
    Uint16 sound; /**< Original one-shot sound request for the current tick, or zero. */
    SotnPlayerDeath death;
    bool looking_up, can_pause;
    Uint32 random; /**< Owned physical-damage/weapon random state for ordinary saves. */
    Uint16 solid_width, solid_height; /**< Current original stage-object sensor extents. */
    SotnPlayerAttack shield; /**< Independent selected ordinary shield hitbox and sprite. */
    bool subweapon_starting; /**< Original selected subweapon-use timer is active. */
    bool weapon_starting;    /**< Original ALU_T_9 weapon-startup timer is active. */
    SotnPlayerStone stone;
    Uint16 poison_ticks; /**< Transient original poison duration, 0..4095 simulation ticks. */
    Uint16 curse_ticks;  /**< Transient original curse duration; independent of ordinary saves. */
    Uint16 extra_sound;  /**< Second original sound request, after sound; zero selects none. */
    SotnPlayerGrab grab;
    Uint16 priority; /**< Current normal-form ordering-table priority. */
    SotnPlayerKick kick;
    SotnPlayerGravity gravity;
    Uint16 mp, mp_spent; /**< Owned MP snapshot and this tick's ability cost for the session. */
    SotnPlayerMist mist;
    SotnPlayerBat bat;
    SotnPlayerWolf wolf;
    SotnBatCommandEvents bat_commands; /**< Actual this-tick special-ability factory requests. */
    Uint16 animation_bank;             /**< Actual normal (1) or Bat (13) private artwork bank. */
    bool rotated; /**< This simulation frame's selected original transform draw flag. */
    Sint16 draw_pivot_x, draw_pivot_y; /**< Retained original selected player effect pivots. */
    SotnPlayerAfterimage afterimage; /**< Original global display state, not copied frame pixels. */
    Uint32 afterimage_generation;    /**< Actual common stage lease, zero for unbound components. */
    SotnPlayerMovementEffects movement_effects;
    SotnPlayerHealing healing;
    SotnPlayerDark dark;
    SotnPlayerSpirit spirit;
    SotnPlayerSoul soul;
    bool hellfire_started,
        hellfire_hidden;             /**< Accepted cast intent and actual palette visibility. */
    Uint16 buttons, hellfire_signal; /**< Current original controls and committed beam signal. */
    Uint8 spell_use_ticks; /**< Original timer12, independent of subweapon use/invincibility. */
} SotnPlayerState;

/** @brief Owned normal-Alucard simulation, borrowing room collision and DRA bytes. */
typedef struct SotnPlayer SotnPlayer;

/** @brief Bind the actual common stage lease and current original engine-frame clock.
 * @param player Owned simulation. @param generation Actual nonzero common row owner.
 * @param timer Original system timer, including paused frames; not the saved gameplay timer.
 * @return Success or SDL error without mutation. The caller validates/commits the actual rows.
 * A bound player cannot change lease or advance twice on one clock. Full stage reset creates
 * a fresh player. Rebinding context alone never advances capture, fades or timer15. */
bool SotnPlayer_SetAfterimageContext(SotnPlayer *player, Uint32 generation, Uint32 timer);
/** @brief Apply an actual child/script's original clear/reset request after player updates.
 * @param player Owned simulation with a real common lease. @param duration Original timer15
 * request.
 * @return Success or SDL error without mutation. Advances no clock/cursor or effect readiness. */
bool SotnPlayer_ResetAfterimages(SotnPlayer *player, Sint32 duration);

typedef struct SotnSubweapons SotnSubweapons;

/** @brief Bind the actual Wolf body/effect/factory context before the next player tick.
 * @param player Actual player. @param body Borrowed stage-owned body.
 * @param effect Actual current transformation child or NULL. @param pending Actual factory or NULL.
 * @param generation Effect/factory lineage, or zero for neither. @param available Actual free
 * factory.
 * @param camera_x Room-local whole camera X. @param camera_y Room-local whole camera Y.
 * @return Success or SDL error without mutation for stale/invalid ownership. Does not grant relics.
 */
bool SotnPlayer_SetWolfContext(SotnPlayer *player, const SotnWolfRuntime *body,
                               const SotnPlayerBlink *effect, const SotnPlayerFactory *pending,
                               Uint32 generation, bool available, Sint32 camera_x, Sint32 camera_y);
/** @brief Commit this tick's generation-owned body feedback after actual physical integration.
 * @param player Actual player. @param body Actual stage-owned linked body.
 * @param frame Root/front-modified screen-space controller for this same update.
 * @param camera_x Whole room camera X. @param camera_y Whole room camera Y.
 * @return Success or SDL error on stale identity, moved origin or invalid body state.
 * This commits original motion/drawing and MP feedback only; the session owns MP accounting. */
bool SotnPlayer_CommitWolfBody(SotnPlayer *player, const SotnWolfRuntime *body,
                               const SotnWolfControllerFrame *frame, Sint32 camera_x,
                               Sint32 camera_y);

/**
 * @brief Initialize normal Alucard movement at a development room start.
 * @param dra Borrowed immutable US DRA animation data, alive until player is closed.
 * @param map Borrowed immutable room collision map, alive until replaced or closed.
 * @param x Starting room-local horizontal position in whole pixels.
 * @param y Starting room-local vertical position in whole pixels.
 * @param out Receives ownership on success, NULL on failure when non-NULL.
 * @return True on success, false with SDL_GetError() for invalid data or bounds.
 * @note Supports standing, walking, crouching, falling, jumping and an active Leap
 * Stone's double jump after SotnPlayer_SetRelics supplies its persistent ownership.
 * The selected normal-Jump dive kick has its own attack and delayed rebound.
 * Empty-hand, Basilard/Gladius/Mace/Short sword, and ordinary Alucard sword/shield paths run.
 * Ordinary physical damage is supported separately by SotnPlayer_Damage. Other weapons,
 * transformations, and stage scripts are not implemented here.
 */
bool SotnPlayer_Create(SotnDataView dra, SotnCollisionMap map, Sint32 x, Sint32 y,
                       SotnPlayer **out);

/** @brief Select the original normal-form simulated-pad hold used by NPC scripts.
 * @param player Borrowed normal-form owner. @param hold Freeze inputs/velocity in original step49.
 * @return Success or SDL error; a living hold cannot replace an active death sequence.
 * The next simulation tick retains animation, refreshes original protection and handles release.
 * No advancement occurs in this setter. Call false when the script interval expires. */
bool SotnPlayer_SetScriptHold(SotnPlayer *player, bool hold);
/** @brief Supply a stage chair's transient original seating hint for the next player update.
 * @param player Borrowed simulation. @param kind Zero disables, 1..4 choose original pose/facing.
 * @return Success or SDL error. Does not force a state; normal input/transitions retain control.
 * The player update consumes and clears the hint. */
bool SotnPlayer_SetChairKind(SotnPlayer *player, Uint16 kind);
/** @brief Apply an original script's absolute whole-pixel horizontal alignment.
 * @param player Borrowed simulation. @param x Room-local target in pixels.
 * @return Success or SDL error; validate room bounds before modifying position.
 * Fractional position, velocity, health and movement state remain intact. */
bool SotnPlayer_StageAlignX(SotnPlayer *player, Sint32 x);
/** @brief Apply a boss's whole-pixel vertical alignment, retaining the fractional position.
 * @param player Borrowed simulation. @param y Bounded room-local target in pixels.
 * @return Success or SDL error without mutation; does not advance movement or animation. */
bool SotnPlayer_StageAlignY(SotnPlayer *player, Sint32 y);

/** @brief Request Hippogryph's original normal-form grab, damage pulse or drop.
 * @param player Borrowed simulation. @param request START, DAMAGE or NONE; HELD is an
 * acknowledgement produced by the player tick, not a caller command.
 * Raw attack power is 0..9999; defense/poison apply on the next player tick. Source coordinates
 * and velocity use the same bounds as ordinary physical contact and select lethal recoil.
 * @return Success or SDL error without mutation for an invalid request/transition.
 * START supersedes ordinary hit protection as in the US game. DAMAGE requires HELD and may
 * kill despite protection. NONE cancels a pending start or drops a held player into hit recovery.
 * This setter advances no time and borrows no boss pointer. Other grab reactions/releases remain
 * unsupported; the owning boss must clear the handshake when it stops holding the player. */
bool SotnPlayer_RequestGrab(SotnPlayer *player, SotnPlayerGrab request);

/** @brief Bind a borrowed subweapon service, alive until player closure; NULL disables casting.
 * @param player Borrowed simulation. @param service Borrowed service. @return Success or SDL error.
 */
bool SotnPlayer_SetSubweapons(SotnPlayer *player, SotnSubweapons *service);

/** @brief Copy persistent relic ownership into the normal-form simulation.
 * @param player Borrowed simulation. @param world Borrowed progress; only its thirty
 * relic found/active bytes are read and copied. No pointer is retained.
 * @return Success or SDL error without mutation for NULL input or invalid relic bits.
 * Found-but-disabled relics do not enable abilities. This setter advances no time,
 * grants no relics and does not replenish the current airborne jump allowance. */
bool SotnPlayer_SetRelics(SotnPlayer *player, const SotnWorldState *world);
/** @brief Copy the actual preceding stage controller's three water depth words.
 * @param player Actual player. @param water Validated feet/waist/head, each0..32767.
 * @return Success or SDL error without mutation. This advances no clock and grants no
 * relics, protection or HP. The stage owner supplies real volumes after committed updates;
 * never substitute a collision-tile boolean. Room entry clears this transient context.
 */
bool SotnPlayer_SetWaterContext(SotnPlayer *player, SotnPlayerWater water);

/** @brief Bind this tick's actual Mist child and original simulation/environment snapshot.
 * @param player Borrowed player. @param effect Borrowed actual owned effect, copied immediately;
 * NULL is valid before entry or after retirement, never during an active transformation.
 * @param generation Child's current player-issued generation, zero when absent.
 * @param game_timer Absolute original wrapping simulation timer. @param underwater Actual
 * nonzero waist depth, never a tile flag or feet-only surface contact.
 * @return Success or SDL error without mutation for stale/missing/removed child state.
 * This enables the selected core controller only after the owner opts in. It never grants relics,
 * creates effects, advances time or manufactures readiness; the owner must run the actual effect
 * and consume generation-tagged start/restore/retire requests before supplying the next snapshot.
 */
bool SotnPlayer_SetMistContext(SotnPlayer *player, const SotnMistEffect *effect, Uint32 generation,
                               Uint32 game_timer, bool underwater);

/** @brief Bind this tick's actual selected Bat effect and owned allocation availability.
 * @param player Borrowed player. @param effect Actual owned current-generation effect, copied
 * immediately; NULL is allowed before birth, during flight after its finishing flash,
 * or while returning from a real Mist child. @param generation Current child identity or zero.
 * @param factory_available The owner's selected factory/primitive budget has an actual free slot.
 * @return Success or SDL error without mutation for missing/stale active effects.
 * Readiness derives from this actual controller's phases, never a supplied completion timer.
 * The caller must create started/return_started children and advance them before the next bind. */
bool SotnPlayer_SetBatContext(SotnPlayer *player, const SotnPlayerBlink *effect, Uint32 generation,
                              bool factory_available);

/** @brief Bind the actual Bat runtime, enabling selected Wing Smash commands.
 * @param player Borrowed player. @param effect Actual current transformation child or NULL.
 * @param pending Actual live transformation factory while its child awaits allocation, or NULL.
 * @param generation Current nonzero owner identity if effect/pending exists, otherwise zero.
 * @param factory_available Whether a real factory slot is available for a new request.
 * @return Success or SDL error without mutation for stale/missing/incompatible owners.
 * Effect and pending are mutually exclusive and copied immediately. A pending factory never
 * advances visual readiness. The caller runs the real factories, children and combat service. */
bool SotnPlayer_SetBatRuntimeContext(SotnPlayer *player, const SotnPlayerBlink *effect,
                                     const SotnPlayerFactory *pending, Uint32 generation,
                                     bool factory_available);

/** @brief Deliver original contact feedback to the current owned dive-kick child.
 * @param player Borrowed simulation. @param generation Current nonzero child identity.
 * @param flags Original normal/deflection/projectile result, 1..3.
 * @return Success or SDL error without mutation for invalid or stale feedback.
 * Delivery advances no time. The child publishes a normal-hit bounce request after
 * the next player step; the following player step consumes that request. */
bool SotnPlayer_KickHit(SotnPlayer *player, Uint32 generation, Uint16 flags);

/**
 * @brief Advance one simulation tick with original movement and sensor rules.
 * @param player Borrowed owned simulation, updated atomically on success.
 * @param buttons Held original button bits. Taps derive from the previous tick.
 * @return True on success, false with SDL_GetError() for unsupported input or invalid state.
 * @note Caller owns pacing. One call advances one game tick independently of rendering.
 */
bool SotnPlayer_Tick(SotnPlayer *player, Uint16 buttons);

/** @brief Set the checkpoint's hand equipment without changing movement or health.
 * @param player Borrowed simulation; an active attack must finish first.
 * @param weapons Borrowed immutable WEAPON0.BIN, alive until equipment is replaced or player
 * closes.
 * @param left Original left-hand item ID. @param right Original right-hand item ID.
 * @return Success, or false with SDL_GetError() and unchanged state for unsupported items.
 * @note Supports empty hands, Gladius, and Mace; this is not an inventory menu. */
bool SotnPlayer_SetEquipment(SotnPlayer *player, SotnDataView weapons, Uint16 left, Uint16 right);

/**
 * @brief Rebase an existing player into an adjacent room without resetting motion.
 * @param player Borrowed owned simulation.
 * @param map New immutable borrowed collision map, alive until replaced or closed.
 * @param offset_x Whole pixels added to room-local X to preserve the stage position.
 * @param offset_y Whole pixels added to room-local Y to preserve the stage position.
 * @return True on success, false with SDL_GetError() and unchanged state on failure.
 */
bool SotnPlayer_ChangeRoom(SotnPlayer *player, SotnCollisionMap map, Sint32 offset_x,
                           Sint32 offset_y);
/** @brief Restore an original transient curse timer across a connected stage arrival.
 * @param player Borrowed living simulation. @param ticks Remaining simulation ticks, 0..1024.
 * @return Success or SDL error without mutation on invalid input. Does not reset motion/attacks.
 * @note Ordinary Continue starts without this transient timer; this is not save-state support. */
bool SotnPlayer_RestoreCurse(SotnPlayer *player, Uint16 ticks);
/** @brief Apply selected physical contact and original constitution-adjusted curse duration.
 * @param player Borrowed simulation. @param amount Damage after scene defense, 0..9999.
 * Caller resolves equipment immunity/absorption before applying this contact.
 * @param source_x Whole-pixel contact source. @param source_velocity Fixed-point contact motion.
 * @return Success, or SDL error without mutation. Invulnerability and lethal hits do not add curse.
 * @note Original failure animations/sounds run for equipped weapons; full curse effects remain
 * pending. */
bool SotnPlayer_CurseDamage(SotnPlayer *player, Uint16 amount, Sint32 source_x,
                            Sint32 source_velocity);
/** @brief Retain transient poison across a connected stage arrival.
 * @param player Borrowed living simulation. @param ticks Remaining ticks, 0..4095.
 * @return Success or SDL error with unchanged state. Ordinary Continue starts unpoisoned. */
bool SotnPlayer_RestorePoison(SotnPlayer *player, Uint16 ticks);
/** @brief Apply normal contact and constitution-adjusted original poison duration.
 * @param player Borrowed simulation. @param amount Already-resolved damage, 0..9999.
 * @param source_x Contact origin in whole pixels. @param source_velocity Fixed-point motion.
 * @return Success or SDL error without mutation. Invulnerability, stone and lethal hits
 * do not add poison. Caller resolves equipment immunity/absorption and existing-poison damage.
 * @note Selected mechanics/sound only; complete palette, text and recovery effects are pending. */
bool SotnPlayer_PoisonDamage(SotnPlayer *player, Uint16 amount, Sint32 source_x,
                             Sint32 source_velocity);
/** @brief Set original arrival facing on a freshly created stage player.
 * @param player Borrowed player before gameplay. @param facing_left Arrival orientation.
 * @return Success or SDL error. Does not move the player or change animation/timing. */
bool SotnPlayer_SetArrivalFacing(SotnPlayer *player, bool facing_left);

/**
 * @brief Copy current state without exposing simulation ownership or internal pointers.
 * @param player Borrowed open simulation.
 * @param out Receives copied state on success, unchanged on failure.
 * @return True on success, false with SDL_GetError() for invalid arguments.
 */
bool SotnPlayer_GetState(const SotnPlayer *player, SotnPlayerState *out);

/** @brief Apply ordinary physical contact damage to the development player after the scene resolves
 * physical defense.
 * @param player Borrowed simulation, with the current character maximum.
 * @param amount Resolved physical damage, 1..9999; zero does nothing.
 * @param source_x Whole-pixel source hitbox center within the room margin.
 * @param source_velocity Original signed 16.16 source velocity.
 * @return Success, or false with SDL_GetError() on invalid input or data failure; failure leaves
 * state unchanged.
 * @note Preserves normal physical hit reactions and recovery invulnerability. Elemental
 * ailments and complete defenses remain incomplete. Lethal physical damage enters the original
 * selected death animation/dissolution; phase 128 signals completion to the scene. */
bool SotnPlayer_Damage(SotnPlayer *player, Uint16 amount, Sint32 source_x, Sint32 source_velocity);

/** @brief Enter selected original petrification or apply a resolved hit to a petrified player.
 * @param player Borrowed simulation. @param amount Resolved damage, 0..9999; zero may petrify.
 * @param source_x Bounded whole-pixel contact center. @param source_velocity Signed 16.16 velocity.
 * @return Success or SDL error, preserving state on failure. Invulnerable hits do nothing.
 * @note Selected stone control/death timing is supported; full statue/chunk effects remain
 * separate.
 */
bool SotnPlayer_Petrify(SotnPlayer *player, Uint16 amount, Sint32 source_x, Sint32 source_velocity);

/** @brief Restore supported health and random state on a living, non-attacking player.
 * @param player Borrowed simulation. @param hp Health within the current maximum. @param random
 * Saved RNG state.
 * @return Success, or false with SDL_GetError() and unchanged state on invalid input. */
bool SotnPlayer_Restore(SotnPlayer *player, Uint16 hp, Uint32 random);

/** @brief Apply living character health and calculated combat attributes after equipment/level
 * changes.
 * @param player Borrowed simulation. @param state Validated ordinary profile.
 * @param stats Derived attributes. @return Success or false with SDL_GetError().
 * @note Does not reset movement, attacks, hit recovery, or owned damage RNG. */
bool SotnPlayer_SetCharacter(SotnPlayer *player, const SotnCharacterState *state,
                             SotnCharacterStats stats);

/** @brief Apply stage-object displacement and contact flags after the original actor update.
 * @param player Borrowed simulation. @param dx Signed 16.16 room displacement.
 * @param dy Signed 16.16 room displacement. @param contacts Original stage-object contact bits.
 * @return Success or SDL error, with unchanged state on invalid bounds. Does not reset animation.
 */
bool SotnPlayer_Displace(SotnPlayer *player, Sint32 dx, Sint32 dy, Uint32 contacts);

/** @brief Apply the original short stage-event protection and optional vertical stop.
 * @param player Borrowed living simulation. @param halt_vertical Clear vertical velocity.
 * @return Success or SDL error; does not revive a dead player. */
bool SotnPlayer_StageGuard(SotnPlayer *player, bool halt_vertical);
/** @brief Apply the original bell's transient concealment/status bits after stage updates.
 * @param player Borrowed simulation. @param inside Inside a bell this tick.
 * @return Success or SDL error for NULL. Next player tick masks hand input and clears these bits;
 * protection and simulated-pad duration remain explicit stage services. */
bool SotnPlayer_BellInside(SotnPlayer *player, bool inside);
/** @brief Stop both velocity axes at an original scripted ride/door handoff.
 * @param player Borrowed simulation. @return Success or SDL error. Position, animation,
 * health and ordinary movement state remain intact. Dead players are unchanged. */
bool SotnPlayer_StageStop(SotnPlayer *player);
/** @brief Stop horizontal velocity for the original tower-exit death guard.
 * @param player Borrowed living or dead simulation. @return Success or SDL error for NULL.
 * Position, vertical velocity, animation, health and movement/death state remain unchanged. */
bool SotnPlayer_StageStopHorizontal(SotnPlayer *player);

/**
 * @brief Release the player without freeing its borrowed image or collision data.
 * @param player Owned player to release, or NULL.
 */
void SotnPlayer_Close(SotnPlayer *player);

/** @brief Apply the original subweapon-use timer after an actual ability child is born.
 * @param player Borrowed owner. @param ticks Original timer value, 0..32.
 * @return Success or SDL error without mutation. This grants no invincibility. */
bool SotnPlayer_SetUseTimer(SotnPlayer *player, Uint8 ticks);
/** @brief Queue actual blood contact healing for the following player update.
 * @param player Borrowed owner. @param amount Original first-request amount0/8/16.
 * @param contacts Actual consumed contacts this stage update,1..32.
 * @return Success or SDL error without mutation. An existing pending amount wins;
 * the original contact counter still advances with unsigned word wrap. This changes no HP,
 * grants no ability and writes no save; successful player ticks consume the request once. */
bool SotnPlayer_QueueBloodHealing(SotnPlayer *player, Uint16 amount, Uint16 contacts);

/** @brief Actual shared factory reservation, called once at the original player boundary.
 * false availability is real exhaustion; success never implies child readiness. */
typedef bool (*SotnPlayerHellfireFactory)(void *context, SotnPlayerFactorySource source,
                                          Uint32 packed, bool *allocated);
/** @brief Bind real native casting/effect ownership before the next player update.
 * @param player Actual player. @param factory Real shared reservation callback or NULL to disable.
 * @param context Borrowed stable owner, required with callback. @param camera_x Actual room camera.
 * @param signal Current original beam signal. @param palette Current actual beam palette or zero.
 * @return Success or SDL error without mutation. Refresh after session replacement; binding never
 * ticks or creates children. The caller rolls back real reservations if a later player Tick fails.
 * Native owner must fulfill gradients, rendering, combat and feedback.
 */
bool SotnPlayer_SetHellfireContext(SotnPlayer *player, SotnPlayerHellfireFactory factory,
                                   void *context, Sint32 camera_x, Uint16 signal, Uint16 palette);
/** @brief Commit real child signal/palette after successful shared stage-row commit.
 * @param player Actual bound player. @param signal Actual original shared word.
 * @param palette Actual beam palette or zero before its first write.
 * @return Success or SDL error without mutation; no simulation or readiness advances. */
bool SotnPlayer_CommitHellfire(SotnPlayer *player, Uint16 signal, Uint16 palette);

/** @brief Bind the actual native Dark spell/effect/palette/healing owner, enabling its command.
 * @param player Borrowed owner. @param enabled Actual complete selected owner availability.
 * @return Success or SDL error without mutation. Default false; no relic or MP is granted. */
bool SotnPlayer_SetDarkContext(SotnPlayer *player, bool enabled);
/** @brief Bind actual shared portal/capture/freeze/target/combat owners for both spirit commands.
 * @param player Borrowed simulation. @param enabled Complete selected native availability.
 * @return Success or SDL error without mutation. Defaults false; no ownership/resources granted.
 * Component tests may bind a fixture. Gameplay must leave this false until every owner is wired. */
bool SotnPlayer_SetSpiritContext(SotnPlayer *player, bool enabled);
/** @brief Bind the actual shared Soul Steal factory/global-attack/orb/healing owners.
 * @param player Actual player. @param enabled True only after all native owners connect.
 * @return Success or SDL error for a missing owner/private metadata. Defaults to disabled;
 * grants no MP, learning or spell readiness. Cannot detach during actual casting. */
bool SotnPlayer_SetSoulContext(SotnPlayer *player, bool enabled);
/** @brief Run original engine-five animation selection without a normal player update.
 * @param player Borrowed owner. @param buttons Current physical US buttons for next-frame edges.
 * @param animate False on the original release-only frame; true runs only BA/BB/BE/F0/F1.
 * @return Success or SDL error without mutation. Clears consumed one-frame requests; retains
 * motion, collision, timers, resources, pending healing, commands and simulation frame number.
 * Updating the native previous-buttons adapter prevents replaying a tap held during freeze. */
bool SotnPlayer_TickSpellFreeze(SotnPlayer *player, Uint16 buttons, bool animate);
/** @brief Apply original engine-five release protection (timer14=max(timer14,48)).
 * @param player Borrowed owner. @return Success or SDL error. No simulation or animation tick. */
bool SotnPlayer_EndSpellFreeze(SotnPlayer *player);
#endif
