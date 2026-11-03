#ifndef SOTN_KARMA_COIN_H
#define SOTN_KARMA_COIN_H
#include <sotn/collision.h>
#include <sotn/player_blink.h>
#define SOTN_KARMA_ROWS 26
/** @brief Original per-hand writable weapon20 data and BSS; not save data.
 * Angles initialize from private program bytes. The counter wraps as a word and
 * original signed remainders select child variants. Room cleanup does not reset it. */
typedef struct SotnKarmaShared
{
    Sint32 angles[24];
    Uint32 child_counter;
    bool loaded;
} SotnKarmaShared;
typedef struct SotnKarmaCoin
{
    SotnPlayerEffectQuad quads[SOTN_KARMA_ROWS];
    Sint32 x, y, velocity_x, velocity_y;
    Uint32 flags;
    Sint16 timer, width, volume_index, angular_speed, brightness, scale_x, scale_y;
    Uint16 bank, texture, palette, priority, frame, animation, pose;
    Sint16 remaining;
    Sint8 hitbox_x, hitbox_y;
    Uint8 hitbox_width, hitbox_height;
    Uint8 phase, draw_flags, blend, opacity;
    bool facing_left, removed;
} SotnKarmaCoin;
typedef struct SotnKarmaInput
{
    SotnDataView overlay, executable;
    SotnCollisionMap collision;
    Uint32 game_timer;
    Uint16 priority, hand, bank, texture;
    bool facing_left, allocated, parent_exists, child;
} SotnKarmaInput;
typedef struct SotnKarmaEvents
{
    Uint16 sound, volume;
    Uint32 factory;
    Uint8 flash, use_ticks, protect_ticks;
    bool volume_sound, clear_shield, properties, fixed_properties;
} SotnKarmaEvents;
/** @brief Initialize actual newly loaded weapon20 data/BSS from private program bytes.
 * Genuine reloads alone call this; output remains unchanged on malformed data. */
bool SotnKarmaCoin_LoadProgram(SotnKarmaShared *, SotnDataView overlay);
/** @brief Execute one selected original root/child callback atomically.
 * Mandatory root26-row allocation, raw parents, actual program data, collision,
 * properties/groups, factories, sound, timers and native damage remain owner obligations.
 * This component alone enables no equipment or hand controls. */
bool SotnKarmaCoin_Tick(SotnKarmaCoin *, SotnKarmaShared *, SotnKarmaInput, Uint32 *random,
                        SotnKarmaEvents *);
/** @brief Execute the actual private timeline after callback/culling when flag100000 is set.
 * Internal tail updates use frame properties inside Tick instead. Output is atomic. */
bool SotnKarmaCoin_Animate(SotnKarmaCoin *, SotnDataView overlay);
#endif
