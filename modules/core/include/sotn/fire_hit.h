#ifndef SOTN_FIRE_HIT_H
#define SOTN_FIRE_HIT_H
#include <sotn/player_factory.h>
/** @brief Actual rowless entity16. Factory/particle intents require real shared owners. */
typedef struct SotnFireHit
{
    Sint32 x, y, velocity_x, velocity_y;
    Uint16 parameter, priority, scale_x, scale_y, bank, palette, pose, frame;
    Sint16 remaining;
    Uint8 phase, draw_flags, blend_mode;
    bool particle, removed;
} SotnFireHit;
/** @brief Actual pre-camera factory/player snapshot, not substituted parent motion. */
typedef struct SotnFireHitInput
{
    SotnPlayerFactorySource source;
    Sint32 player_x, player_y, player_velocity_x;
    Sint16 hitbox_y;
    Uint16 priority, parameter;
} SotnFireHitInput;
/** @brief Execute the original callback before culling/animation.
 * Charges three character RNG calls on birth only. A particle flag requests real
 * BP4/profile4 from this actual entity; it does not complete allocation or descendants. */
bool SotnFireHit_Tick(SotnFireHit *effect, SotnFireHitInput input, Uint32 *random);
/** @brief Advance the actual private finite timeline after callback/offscreen culling. */
bool SotnFireHit_Animate(SotnFireHit *effect, SotnDataView dra);
/** @brief Translate an actual retained entity without animation, motion or RNG updates. */
bool SotnFireHit_Rebase(SotnFireHit *effect, Sint16 x, Sint16 y);
#endif
