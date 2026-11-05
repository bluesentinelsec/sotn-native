// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/7E4BC.c:func_8011F074 and alu_anim.c:UpdateAnim, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Animation/artwork remain private.
#include <SDL3/SDL_error.h>
#include <sotn/combat.h>
#include <sotn/dark_hit.h>
static Sint32 Fixed(Uint32 bits)
{
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint32 Shift(Sint32 value, Sint16 pixels)
{
    return Fixed((Uint32)value + (Uint32)(Sint32)pixels * 65536U);
}
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
bool SotnDarkHit_Tick(SotnDarkHit *effect, SotnPlayerFactorySource source, Uint16 priority,
                      Uint32 *blend_phase, Uint32 *random)
{
    if (!effect || !blend_phase || !random || effect->removed || effect->phase > 1 ||
        priority > 509 || effect->remaining < -1 || effect->pose >= 13)
        return SDL_SetError("Dark hit requires its actual live child and shared blend phase");
    SotnDarkHit next = *effect;
    Uint32 rng = *random;
    if (!next.phase)
    {
        next = (SotnDarkHit){.x = source.x,
                             .y = source.y,
                             .priority = (Uint16)(priority + 2U),
                             .opacity = 255,
                             .blend_mode = (Uint8)(*blend_phase & 1U ? 0x70 : 0x10),
                             .scale_x = 64,
                             .scale_y = 64,
                             .phase = 1};
        next.y = Shift(next.y, (Sint16)((Sint32)(SotnCombat_Random(&rng) % 35U) - 15));
        next.x = Shift(next.x, (Sint16)((Sint32)(SotnCombat_Random(&rng) % 20U) - 10));
        next.velocity_y = -24576 - (Sint32)(SotnCombat_Random(&rng) & 0x3fffU);
        ++*blend_phase;
    }
    else
    {
        if (next.opacity > 16)
            next.opacity = (Uint8)(next.opacity - 8U);
        next.y = Fixed((Uint32)next.y + (Uint32)next.velocity_y);
        next.scale_x = (Uint16)(next.scale_x + 8U);
        next.scale_y = (Uint16)(next.scale_y + 8U);
        next.removed = next.remaining < 0;
    }
    *effect = next;
    *random = rng;
    return true;
}
bool SotnDarkHit_Animate(SotnDarkHit *effect, SotnDataView dra)
{
    if (!effect || !effect->phase || effect->removed || effect->pose >= 13 ||
        effect->remaining < -1 || !dra.bytes || dra.size < 0xdc7cU)
        return SDL_SetError("Dark hit lost its bounded private animation owner");
    SotnDarkHit next = *effect;
    Uint16 duration = (Uint16)next.remaining;
    if (!next.remaining || (next.remaining > 0 && --next.remaining == 0))
    {
        if (duration)
            ++next.pose;
        duration = Read16(dra.bytes + 0xdc44U + next.pose * 4U);
        if (duration == 0xffffU && next.pose)
        {
            --next.pose;
            next.remaining = -1;
        }
        else
        {
            if (!duration || duration > 32767 || next.pose >= 13)
                return SDL_SetError("Dark hit animation escapes its original finite timeline");
            next.remaining = (Sint16)duration;
        }
    }
    const Uint16 frame = Read16(dra.bytes + 0xdc44U + next.pose * 4U + 2U);
    if (!frame || frame > 511)
        return SDL_SetError("Dark hit has an invalid private sprite frame");
    next.frame = frame;
    *effect = next;
    return true;
}
bool SotnDarkHit_Rebase(SotnDarkHit *effect, Sint16 x, Sint16 y)
{
    if (!effect || effect->removed)
        return SDL_SetError("Dark hit translation requires its actual live child");
    if (effect->phase)
    {
        effect->x = Shift(effect->x, x);
        effect->y = Shift(effect->y, y);
    }
    return true;
}
