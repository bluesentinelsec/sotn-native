#ifndef SOTN_HELLFIRE_FIREBALL_H
#define SOTN_HELLFIRE_FIREBALL_H
#include <sotn/game_data.h>
/** @brief One original normal/big Hellfire projectile; zero except actual source/parameter/role.
 * This owns no factory slot, animation, texture, attack group, RNG or save data. */
typedef struct SotnHellfireFireball
{
    Sint32 x, y, velocity_x, velocity_y;
    Uint32 flags;
    Uint16 parameter, phase, bank, priority;
    Sint16 timer, rotation;
    Uint8 width, height, draw_flags;
    bool big, facing_left, removed;
} SotnHellfireFireball;
/** @brief Actual private launch data, pre-animation pose, clock, RNG and hit feedback.
 * Supply one actual rand result only for a big phase-one pose>=23 on an even game clock. */
typedef struct SotnHellfireFireballInput
{
    SotnDataView dra;
    Uint32 game_timer;
    Uint16 player_priority, pose, random;
    Uint8 hit_flags;
    bool player_facing_left, random_ready;
} SotnHellfireFireballInput;
/** @brief Original spell/factory/sound intents; no request establishes actual attack readiness. */
typedef struct SotnHellfireFireballEvents
{
    Uint32 factory, animation_offset;
    Uint16 spell, sound;
    bool initialize, random_used;
} SotnHellfireFireballEvents;
/** @brief Advance original normal spread/impact or big delayed acceleration/piercing once.
 * @param fireball Actual owned cursor. @param input Copied live owner context/private data.
 * @param out Receives one-update intents. @return Success or SDL error preserving state/output.
 * Normal parameters0..2 index private launch tables; first birth does not move. Normal impact
 * retires before motion and its twentieth curved update still applies Y before horizontal flight.
 * Big birth retains the raw fixed-point speed16; actual animation pose gates acceleration/RNG,
 * and hit feedback does not retire it. Owner binds original factories, spell/group/RNG ownership,
 * animation, offscreen culling and native combat before any Hellfire control becomes available. */
bool SotnHellfireFireball_Tick(SotnHellfireFireball *fireball, SotnHellfireFireballInput input,
                               SotnHellfireFireballEvents *out);
#endif
