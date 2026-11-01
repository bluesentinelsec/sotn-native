#ifndef SOTN_TELEPORT_EFFECT_H
#define SOTN_TELEPORT_EFFECT_H
#include <sotn/player_blink.h>

enum
{
    SOTN_TELEPORT_PARTICLES = 32,
    SOTN_TELEPORT_ROWS = 36
};
/** @brief Original DRA global point array shared by every actual teleport child. */
typedef struct SotnTeleportShared
{
    struct
    {
        Sint16 x, y;
    } points[SOTN_TELEPORT_PARTICLES];
} SotnTeleportShared;
/** @brief Entity65: two TILEs, two G4s and thirty-two GT4s in original row order.
 * Particle green/blue bytes retain original delay/animation metadata. */
typedef struct SotnTeleportEffect
{
    SotnPlayerEffectQuad quads[SOTN_TELEPORT_ROWS];
    Sint32 x, y, width, height, timer, color, alpha;
    Uint16 parameter, phase;
    bool removed;
} SotnTeleportEffect;
/** @brief Actual decoded player position, private data and real primitive reservation result. */
typedef struct SotnTeleportEffectInput
{
    SotnDataView dra;
    Sint32 player_x, player_y;
    bool allocated;
} SotnTeleportEffectInput;
/** @brief Original requests, committed by their real player/audio/stage owners after success.
 * warp_mode6/4/5 requests Library/other-castle travel; it is not a completed handoff.
 * Arrival acknowledgement is emitted only on the actual terminal child callback. */
typedef struct SotnTeleportEffectEvents
{
    Uint16 sounds[2], palette;
    Uint8 sound_count, warp_mode;
    bool warp_acknowledged;
} SotnTeleportEffectEvents;
/** @brief Advance original EntityTeleport once with its shared points and character RNG.
 * Allocation failure at birth retries before RNG. All copied owners commit after success.
 * The caller owns actual36-row leases, drawing, sound, palette and warp completion.
 * No controls or stage availability are enabled by this component. */
bool SotnTeleportEffect_Tick(SotnTeleportEffect *effect, SotnTeleportShared *shared,
                             SotnTeleportEffectInput input, Uint32 *random,
                             SotnTeleportEffectEvents *out);
/** @brief Apply original entity/packet camera flags without changing global particle points. */
bool SotnTeleportEffect_Rebase(SotnTeleportEffect *effect, Sint16 x, Sint16 y, bool room);
#endif
