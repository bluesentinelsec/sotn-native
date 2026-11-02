#ifndef SOTN_NEUTRON_H
#define SOTN_NEUTRON_H
#include <sotn/player_blink.h>
/** @brief Original weapon19 callback state; actual factories/rows/combat belong to its owner. */
typedef struct SotnNeutron
{
    SotnPlayerEffectQuad halo;
    Sint32 x, y;
    Uint32 flags;
    Sint16 extent, scale_x, scale_y, rotation;
    Uint16 priority, draw_flags, blend;
    Uint8 phase, opacity;
    bool facing_left, removed;
} SotnNeutron;
typedef struct SotnNeutronInput
{
    Sint32 player_x, player_y;
    Uint16 priority, pose;
    Sint16 remaining;
    bool allocated;
} SotnNeutronInput;
typedef struct SotnNeutronEvents
{
    Uint16 sounds[2];
    Uint8 use_ticks;
    bool properties, begin_animation, disable_attack;
} SotnNeutronEvents;
bool SotnNeutron_Tick(SotnNeutron *, SotnNeutronInput, SotnNeutronEvents *);
/** @brief Original post-callback UpdateAnim cursor. Draws never advance it. */
typedef struct SotnNeutronAnimation
{
    Uint16 pose, frame;
    Sint16 remaining;
} SotnNeutronAnimation;
/** @brief Advance the private weapon19 timeline after a surviving callback/cull. */
bool SotnNeutron_Animate(SotnNeutronAnimation *, SotnDataView overlay);
#endif
