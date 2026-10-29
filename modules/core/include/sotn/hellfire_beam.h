#ifndef SOTN_HELLFIRE_BEAM_H
#define SOTN_HELLFIRE_BEAM_H
#include <sotn/player_blink.h>
/** @brief Original Hellfire beam controller, borrowing one actual primitive row.
 * Initialize to zero except its actual source x/y; retain fractions across following.
 * This owns no factory, row, texture, palette, player motion or combat readiness. */
typedef struct SotnHellfireBeam
{
    SotnPlayerEffectQuad quad;
    Sint32 x, y;
    Uint32 flags;
    Sint16 timer, width, height;
    Uint16 phase;
    bool removed;
} SotnHellfireBeam;
/** @brief Actual player state, real one-row allocation and original game clock. */
typedef struct SotnHellfireBeamInput
{
    Sint16 player_x;
    Uint32 game_timer;
    bool casting, up, allocated;
} SotnHellfireBeamInput;
/** @brief Original one-update requests, consumed only by real native owners. */
typedef struct SotnHellfireBeamEvents
{
    Uint32 factory;
    Uint16 palette, sound, signal;
    bool signal_changed, upload;
} SotnHellfireBeamEvents;
/** @brief Advance the original beam and its retained quad once.
 * @param beam Actual child state, unchanged after malformed input or removed state.
 * @param input Current real player/context. @param out Receives one-update intents.
 * @return Success or SDL error preserving state/output. Actual row exhaustion retires
 * and reports original signalFFFF; never invent readiness. Upload requests require the
 * two private original gradient images and actual target ownership before rendering.
 * The owner supplies player/controller/factory/texture/palette/combat integration.
 * No Hellfire controls, MP charges, learning or native ability are enabled here. */
bool SotnHellfireBeam_Tick(SotnHellfireBeam *beam, SotnHellfireBeamInput input,
                           SotnHellfireBeamEvents *out);
#endif
