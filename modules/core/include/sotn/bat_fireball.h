#ifndef SOTN_BAT_FIREBALL_H
#define SOTN_BAT_FIREBALL_H

#include <SDL3/SDL.h>

/** @brief Motion of one actual Bat fireball, created at its factory's 16.16 origin.
 * Zero-initialize, then assign x/y from the real child. This cursor owns no slot,
 * attack group, private artwork or ordinary save state. */
typedef struct SotnBatFireball
{
    Sint32 x, y, velocity_x;
    Uint16 scale_x, scale_y, priority, bank;
    Uint8 width, height;
    bool initialized, removed, facing_left;
} SotnBatFireball;

/** @brief Actual player context at birth and this child's preceding combat feedback. */
typedef struct SotnBatFireballInput
{
    Uint16 player_priority;
    Uint8 hit_flags;
    bool player_facing_left;
} SotnBatFireballInput;

/** @brief Original one-shot requests; allocation and successful initialization remain external. */
typedef struct SotnBatFireballEvents
{
    Uint16 sound, spell;
    Uint8 player_use_ticks;
    bool initialize;
} SotnBatFireballEvents;

/** @brief Advance original fireball launch, travel/growth or impact retirement once.
 * @param fireball Owned actual child cursor, rebased externally with its room/camera.
 * @param input Current copied player context and actual child hit flags.
 * @param events Receives this tick's requests, unchanged on failure.
 * @return Success or SDL error, preserving both outputs on invalid state.
 * Birth requests spell nine, bank-nine private animation and four ticks of original player timer
 * ten (subweapon use). The owner must bind real attack/RNG/group ownership, advance private
 * animation, render original scale and retire offscreen children. This does not enable Fire of Bat.
 */
bool SotnBatFireball_Tick(SotnBatFireball *fireball, SotnBatFireballInput input,
                          SotnBatFireballEvents *events);

#endif
