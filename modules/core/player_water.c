// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/692E8.c water context/collision scaling, pinned sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. No private tables are embedded.
#include "player_internal.h"

bool SotnPlayer_SetWaterContext(SotnPlayer *p, SotnPlayerWater water)
{
    if (!p || water.feet > 32767 || water.waist > water.feet || water.head > water.waist)
        return SDL_SetError("Player water requires actual bounded feet/waist/head depths");
    p->water = water;
    return true;
}
Sint32 SotnPlayer_WaterScale(Sint32 velocity, bool restore)
{
    const Uint32 bits = (Uint32)velocity * (restore ? 4U : 3U);
    const Sint32 product =
        bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
    return product / (restore ? 3 : 4);
}
bool SotnPlayer_WaterDrag(const SotnPlayer *p)
{
    const Uint32 status = SotnPlayer_Status(p);
    return !(status & 0x50U) && !p->collision_invincibility &&
           (p->water.feet >= 41 || ((status & 4U) && p->water.feet > 12));
}
bool SotnPlayer_SetWaterBreathTimer(SotnPlayer *p, Uint16 timer)
{
    if (!p || timer > 96)
        return SDL_SetError("Normal water breathing requires the actual bounded DRA counter");
    p->water_breath_timer = timer;
    return true;
}
static Sint32 Shift(Sint32 value, Sint32 pixels)
{
    const Uint32 bits = (Uint32)value + (Uint32)pixels * 65536U;
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
void SotnPlayer_WaterBreath(SotnPlayer *p, bool crouched, bool shielded)
{
    if ((!crouched && shielded) || p->failed)
        return;
    if (p->water_breath_timer > 96 || p->movement_effects.water_bubble)
    {
        p->failed = true;
        SDL_SetError("Normal water breathing lost its bounded counter or one-update factory");
        return;
    }
    if (p->water_breath_timer)
        --p->water_breath_timer;
    else if (!shielded && p->water.feet > (crouched ? 24U : 48U))
    {
        const Sint32 dx = (p->facing_left ? -1 : 1) * (crouched ? 12 : 4);
        p->movement_effects.water_bubble = true;
        p->movement_effects.bubble_source =
            (SotnPlayerFactorySource){.x = Shift(p->x.value, dx),
                                      .y = Shift(p->y.value, crouched ? 2 : -16),
                                      .priority = 148,
                                      .facing_left = p->facing_left};
        p->water_breath_timer = 96;
    }
}
