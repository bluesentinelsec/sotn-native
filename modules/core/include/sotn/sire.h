#ifndef SOTN_SIRE_H
#define SOTN_SIRE_H
#include <sotn/game_math.h>
/** @brief Original weapon18 callbacks; actual factory/combat/animation/render owners are separate.
 */
typedef struct SotnSireParent
{
    Sint32 x, y;
    Uint32 flags;
    Sint16 scale_x, scale_y, pivot_x, lifetime;
    Uint16 bank, texture, palette, priority;
    Uint8 phase, blend, draw_flags, animation;
    bool facing_left, removed;
} SotnSireParent;
typedef struct SotnSireParentInput
{
    Sint32 player_x, player_y;
    Sint16 remaining;
    Uint16 priority, hand;
    bool factory_allocated;
} SotnSireParentInput;
typedef struct SotnSireParentEvents
{
    Sint8 properties;
    Uint16 sound;
    Uint8 flash, shake, use_ticks;
    bool clear_attack, reset_pose, burst_factory, particle_factory;
} SotnSireParentEvents;
bool SotnSireParent_Tick(SotnSireParent *, SotnSireParentInput, SotnSireParentEvents *);
typedef struct SotnSireChild
{
    Sint32 x, y, velocity_x, velocity_y;
    Uint32 flags;
    Sint16 scale_x, scale_y, lifetime;
    Uint16 bank, texture, palette, priority;
    Uint8 phase, blend, draw_flags, animation;
    bool facing_left, removed;
} SotnSireChild;
typedef struct SotnSireChildInput
{
    SotnDataView executable;
    Uint16 parent_item, parent_palette, parent_texture, priority, hand;
} SotnSireChildInput;
typedef struct SotnSireChildEvents
{
    Uint16 sound;
} SotnSireChildEvents;
bool SotnSireChild_Tick(SotnSireChild *, SotnSireChildInput, Uint32 *, SotnSireChildEvents *);
/** @brief Original post-callback animation cursor; timelines remain private. */
typedef struct SotnSireAnimation
{
    Uint16 pose, frame;
    Sint16 remaining;
} SotnSireAnimation;
/** @brief Advance actual bat1/portrait2/burst3 private timeline, after callback and cull. */
bool SotnSire_Animate(SotnSireAnimation *, SotnDataView overlay, Uint8 timeline);
#endif
