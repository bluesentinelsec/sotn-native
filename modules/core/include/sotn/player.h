#ifndef SOTN_PLAYER_H
#define SOTN_PLAYER_H

#include <sotn/animation.h>
#include <sotn/character.h>
#include <sotn/collision.h>

/** @brief Original PS1 button bits accepted by normal Alucard and supported hand attacks. */
typedef enum SotnPlayerButton
{
    SOTN_BUTTON_SELECT = 0x0001,
    SOTN_BUTTON_START = 0x0008,
    SOTN_BUTTON_CANCEL = 0x0010,
    SOTN_BUTTON_JUMP = 0x0040,
    SOTN_BUTTON_RIGHT_HAND = 0x0020,
    SOTN_BUTTON_LEFT_HAND = 0x0080,
    SOTN_BUTTON_UP = 0x1000,
    SOTN_BUTTON_RIGHT = 0x2000,
    SOTN_BUTTON_DOWN = 0x4000,
    SOTN_BUTTON_LEFT = 0x8000
} SotnPlayerButton;

/** @brief Original normal-Alucard movement state IDs. */
typedef enum SotnPlayerStep
{
    SOTN_PLAYER_STAND = 0,
    SOTN_PLAYER_WALK,
    SOTN_PLAYER_CROUCH,
    SOTN_PLAYER_FALL,
    SOTN_PLAYER_JUMP,
    SOTN_PLAYER_HIT = 10,
    SOTN_PLAYER_STONE = 11,
    SOTN_PLAYER_KILL = 16,
    SOTN_PLAYER_SCRIPT = 49
} SotnPlayerStep;

/** @brief Runtime supported hand-attack hitbox, in whole pixels relative to Alucard's origin. */
typedef struct SotnPlayerAttack
{
    Sint16 offset_x, offset_y;
    Uint16 width, height, element, hit_type, hit_effect, stun_frames;
    Uint8 invincibility_frames, critical_rate;
    bool active, right_hand, sound;
    Uint16 power, sound_id, equipment, sprite;
} SotnPlayerAttack;

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

/** @brief Read-only copy of a player's current normal-movement state. */
typedef struct SotnPlayerState
{
    Sint32 x, y, velocity_x, velocity_y; /**< Signed 16.16 room-local pixels. */
    Uint32 status;   /**< Original status bits for stage scripts and enemy observations. */
    Uint32 contacts; /**< Original floor/ceiling/wall and slope flags. */
    SotnAnimation animation;
    SotnPlayerStep step;
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
} SotnPlayerState;

typedef struct SotnSubweapons SotnSubweapons;

/** @brief Owned normal-Alucard simulation, borrowing room collision and DRA bytes. */
typedef struct SotnPlayer SotnPlayer;

/**
 * @brief Initialize normal Alucard movement at a development room start.
 * @param dra Borrowed immutable US DRA animation data, alive until player is closed.
 * @param map Borrowed immutable room collision map, alive until replaced or closed.
 * @param x Starting room-local horizontal position in whole pixels.
 * @param y Starting room-local vertical position in whole pixels.
 * @param out Receives ownership on success, NULL on failure when non-NULL.
 * @return True on success, false with SDL_GetError() for invalid data or bounds.
 * @note This slice supports standing, walking, crouching, falling, and jumping.
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

/** @brief Bind a borrowed subweapon service, alive until player closure; NULL disables casting.
 * @param player Borrowed simulation. @param service Borrowed service. @return Success or SDL error.
 */
bool SotnPlayer_SetSubweapons(SotnPlayer *player, SotnSubweapons *service);

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
/** @brief Stop both velocity axes at an original scripted ride/door handoff.
 * @param player Borrowed simulation. @return Success or SDL error. Position, animation,
 * health and ordinary movement state remain intact. Dead players are unchanged. */
bool SotnPlayer_StageStop(SotnPlayer *player);

/**
 * @brief Release the player without freeing its borrowed image or collision data.
 * @param player Owned player to release, or NULL.
 */
void SotnPlayer_Close(SotnPlayer *player);

#endif
