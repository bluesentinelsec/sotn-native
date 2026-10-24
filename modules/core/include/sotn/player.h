#ifndef SOTN_PLAYER_H
#define SOTN_PLAYER_H

#include <sotn/animation.h>
#include <sotn/collision.h>

/** @brief Original PS1 button bits accepted by normal Alucard and unarmed attacks. */
typedef enum SotnPlayerButton
{
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
    SOTN_PLAYER_JUMP
} SotnPlayerStep;

/** @brief Runtime unarmed hitbox, in whole pixels relative to Alucard's origin. */
typedef struct SotnPlayerAttack
{
    Sint16 offset_x, offset_y;
    Uint16 width, height, element, hit_type, hit_effect, stun_frames;
    Uint8 invincibility_frames, critical_rate;
    bool active, right_hand, sound;
} SotnPlayerAttack;

/** @brief Read-only copy of a player's current normal-movement state. */
typedef struct SotnPlayerState
{
    Sint32 x, y, velocity_x, velocity_y; /**< Signed 16.16 room-local pixels. */
    Uint32 contacts;                     /**< Original floor/ceiling/wall and slope flags. */
    SotnAnimation animation;
    SotnPlayerStep step;
    bool facing_left;
    SotnPlayerAttack attack;
} SotnPlayerState;

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
 * Empty-hand attack animations/hitboxes are supported. Equipped weapons, damage resolution,
 * transformations, and stage scripts are not implemented here.
 */
bool SotnPlayer_Create(SotnDataView dra, SotnCollisionMap map, Sint32 x, Sint32 y,
                       SotnPlayer **out);

/**
 * @brief Advance one simulation tick with original movement and sensor rules.
 * @param player Borrowed owned simulation, updated atomically on success.
 * @param buttons Held original button bits. Taps derive from the previous tick.
 * @return True on success, false with SDL_GetError() for unsupported input or invalid state.
 * @note Caller owns pacing. One call advances one game tick independently of rendering.
 */
bool SotnPlayer_Tick(SotnPlayer *player, Uint16 buttons);

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

/**
 * @brief Copy current state without exposing simulation ownership or internal pointers.
 * @param player Borrowed open simulation.
 * @param out Receives copied state on success, unchanged on failure.
 * @return True on success, false with SDL_GetError() for invalid arguments.
 */
bool SotnPlayer_GetState(const SotnPlayer *player, SotnPlayerState *out);

/**
 * @brief Release the player without freeing its borrowed image or collision data.
 * @param player Owned player to release, or NULL.
 */
void SotnPlayer_Close(SotnPlayer *player);

#endif
