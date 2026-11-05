#ifndef SOTN_DARK_HIT_H
#define SOTN_DARK_HIT_H
#include <sotn/player_factory.h>
/** @brief Actual rowless entity54. The engine owns culling before animation. */
typedef struct SotnDarkHit
{
    Sint32 x, y, velocity_y;
    Uint16 scale_x, scale_y, priority, pose, frame;
    Sint16 remaining;
    Uint8 phase, opacity, blend_mode;
    bool removed;
} SotnDarkHit;
/** @brief Original controller, charging three character RNG calls only on birth.
 * @param blend_phase Single actual DRA global, retained across entity/session reset.
 * @return Success or SDL error preserving all inputs. This allocates no slot or rows.
 * The owner culls after this callback, then invokes Animate for a surviving entity. */
bool SotnDarkHit_Tick(SotnDarkHit *effect, SotnPlayerFactorySource source, Uint16 priority,
                      Uint32 *blend_phase, Uint32 *random);
/** @brief Advance the actual private finite animation after controller/culling. */
bool SotnDarkHit_Animate(SotnDarkHit *effect, SotnDataView dra);
/** @brief Apply actual camera/room translation without motion, RNG or animation ticks. */
bool SotnDarkHit_Rebase(SotnDarkHit *effect, Sint16 x, Sint16 y);
#endif
