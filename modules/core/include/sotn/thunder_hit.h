#ifndef SOTN_THUNDER_HIT_H
#define SOTN_THUNDER_HIT_H
#include <sotn/player_blink.h>
#include <sotn/player_factory.h>
/** @brief Actual entity30 and its six owned GT4 history rows. */
typedef struct SotnThunderHit
{
    SotnPlayerEffectQuad quads[6];
    Sint32 x, y, drift;
    Sint16 angle, wave, speed, fade, airborne, thickness, age;
    Uint16 phase, parameter, sounds[2];
    Uint8 sound_count;
    bool removed;
} SotnThunderHit;
/** @brief Actual pre-camera player/factory state and real six-row allocation outcome. */
typedef struct SotnThunderHitInput
{
    SotnDataView dra, executable;
    SotnPlayerFactorySource source;
    Sint32 player_x, player_y, player_velocity_y;
    Uint32 contacts;
    Uint16 priority, player_step, parameter;
    bool allocated;
} SotnThunderHitInput;
/** @brief Execute original controller and quad-history work once.
 * @return Success or SDL error preserving state/RNG. Failed birth allocation retires before
 * RNG/sounds. Sound words are intents; the native owner commits real rows/audio after success. */
bool SotnThunderHit_Tick(SotnThunderHit *effect, SotnThunderHitInput input, Uint32 *random);
/** @brief Apply the engine's flag2 camera or flag100 room translation without a tick. */
bool SotnThunderHit_Rebase(SotnThunderHit *effect, Sint16 x, Sint16 y, bool room);
#endif
