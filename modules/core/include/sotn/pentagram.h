#ifndef SOTN_PENTAGRAM_H
#define SOTN_PENTAGRAM_H
#include <sotn/player_blink.h>
#define SOTN_PENTAGRAM_TRIANGLES 24
#define SOTN_PENTAGRAM_ROWS (SOTN_PENTAGRAM_TRIANGLES + 1)
/** @brief Original weapon17 local vector and triangle-center coordinates. */
typedef struct SotnPentagramVector
{
    Sint16 x, y, z;
} SotnPentagramVector;
/** @brief Original weapon17 BSS shard words, distinct from allocated GPU rows. */
typedef struct SotnPentagramShard
{
    Sint32 x, y, z, velocity_x, velocity_y, velocity_z;
    Sint16 angle_x, angle_y, angle_z, speed_x, speed_y, speed_z;
} SotnPentagramShard;
/** @brief Actual per-hand program scratch shared by all weapon17 callbacks.
 * Birth overwrites vectors; the later breakup overwrites shards. Never save it or
 * substitute per-child copies for this original shared state. */
typedef struct SotnPentagramShared
{
    SotnPentagramVector vectors[SOTN_PENTAGRAM_TRIANGLES][4];
    SotnPentagramShard shards[SOTN_PENTAGRAM_TRIANGLES];
    bool initialized;
} SotnPentagramShared;
/** @brief Original callback state; real factories, properties, GPU rows and combat are separate. */
typedef struct SotnPentagram
{
    SotnPlayerEffectQuad polygons[SOTN_PENTAGRAM_ROWS];
    Sint32 x, y;
    Uint32 flags;
    Uint16 parameter;
    Sint16 lifetime;
    Uint8 phase;
    bool removed;
} SotnPentagram;
typedef struct SotnPentagramInput
{
    SotnDataView overlay, executable;
    Uint32 game_timer;
    Uint16 parameter, hand;
    bool allocated;
} SotnPentagramInput;
typedef struct SotnPentagramEvents
{
    Uint16 sound;
    Uint8 flash, shake, use_ticks;
    bool properties, clear_attack;
} SotnPentagramEvents;
/** @brief Run one original callback using the actual per-hand scratch and libc cursor.
 * Allocation is the real25-row outcome. Errors preserve callback/shared/RNG/output state.
 * Does not allocate rows, apply damage or enable either item by itself. */
bool SotnPentagram_Tick(SotnPentagram *, SotnPentagramShared *, SotnPentagramInput, Uint32 *random,
                        SotnPentagramEvents *);
#endif
