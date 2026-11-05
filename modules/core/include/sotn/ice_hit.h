#ifndef SOTN_ICE_HIT_H
#define SOTN_ICE_HIT_H
#include <sotn/player_blink.h>
#include <sotn/player_factory.h>
enum
{
    SOTN_ICE_HIT_ROWS = 24
};
/** @brief Actual entity33 and its twenty-four untextured G4 shards. UV bytes retain the
 * original per-row phase, delay and signed vertical velocity. */
typedef struct SotnIceHit
{
    SotnPlayerEffectQuad quads[SOTN_ICE_HIT_ROWS];
    Sint32 x, y;
    Uint32 flags;
    Sint16 rotation, timer, airborne, death, death_timer;
    Uint16 phase, sound;
    bool removed;
} SotnIceHit;
/** @brief Actual pre-camera player state and real row-allocation outcome. */
typedef struct SotnIceHitInput
{
    SotnDataView dra, executable;
    SotnPlayerFactorySource source;
    Sint32 player_x, player_y, velocity_x, velocity_y;
    Uint32 status, contacts;
    Uint16 priority, player_step, player_substep;
    bool facing_left, allocated;
} SotnIceHitInput;
/** @brief Advance the original controller once; malformed data preserves state/RNG.
 * Allocation failure retires before RNG. Sound remains an intent for the native owner. */
bool SotnIceHit_Tick(SotnIceHit *effect, SotnIceHitInput input, Uint32 *random);
/** @brief Apply flag-based entity/packet camera or room translation without advancing time. */
bool SotnIceHit_Rebase(SotnIceHit *effect, Sint16 x, Sint16 y, bool room);
/** @brief Apply actual player-lock displacement to entity coordinates only. */
bool SotnIceHit_Displace(SotnIceHit *effect, Sint32 x, Sint32 y);
#endif
