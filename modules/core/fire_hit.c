// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/7E4BC.c:func_8011EDA8 and alu_anim.c:UpdateAnim, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Animation/artwork remain private.
#include <SDL3/SDL_error.h>
#include <sotn/combat.h>
#include <sotn/fire_hit.h>
static Sint32 Fixed(Uint32 bits)
{
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint32 Shift(Sint32 value, Sint16 pixels)
{
    return Fixed((Uint32)value + (Uint32)(Sint32)pixels * 65536U);
}
static Sint32 Pixel(Sint32 value)
{
    return (Sint32)(value >= 0 ? value / 65536 : -((-(Sint64)value + 65535) / 65536));
}
static Sint32 High(Sint32 value, Sint32 pixels)
{
    return Fixed((Uint32)pixels * 65536U | ((Uint32)value & 65535U));
}
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
bool SotnFireHit_Tick(SotnFireHit *effect, SotnFireHitInput in, Uint32 *random)
{
    if (!effect || !random || effect->removed || effect->phase > 1 || in.parameter > 0x2ff ||
        in.priority < 2 || in.priority > 509 || effect->remaining < -1 ||
        (effect->phase && effect->parameter != in.parameter))
        return SDL_SetError("Fire hit requires its actual live child/player/RNG snapshot");
    SotnFireHit next = *effect;
    Uint32 rng = *random;
    const Uint16 high = in.parameter >> 8, low = in.parameter & 255U;
    next.particle = false;
    if (!next.phase)
    {
        const bool scaled = high == 1 || (low & 3U);
        next = (SotnFireHit){.x = in.source.x,
                             .y = in.source.y,
                             .parameter = in.parameter,
                             .scale_x = high == 1 ? 192
                                        : scaled  ? 288
                                                  : 0,
                             .scale_y = high == 1 ? 192
                                        : scaled  ? 288
                                                  : 0,
                             .bank = scaled ? 2 : 5,
                             .palette = scaled ? 0 : 0x8170,
                             .draw_flags = scaled ? 3 : 0,
                             .phase = 1};
        next.priority = (Uint16)(in.priority + (SotnCombat_Random(&rng) & 3U ? 2 : -2));
        const Uint32 x = SotnCombat_Random(&rng);
        next.x = High(next.x, Pixel(in.player_x) + (Sint32)(high == 2 ? x % 44U : x & 15U) -
                                  (high == 2 ? 22 : 8));
        next.y = High(next.y, Pixel(in.player_y) + in.hitbox_y +
                                  (Sint32)(SotnCombat_Random(&rng) & 31U) - 16);
        next.velocity_x = in.player_velocity_x >= 0
                              ? in.player_velocity_x / 4
                              : (Sint32)(-((-(Sint64)in.player_velocity_x + 3) / 4));
        next.velocity_y = -32768;
    }
    else
    {
        next.scale_x = (Uint16)(next.scale_x - 4U);
        next.scale_y = (Uint16)(next.scale_y - 4U);
        next.y = Fixed((Uint32)next.y + (Uint32)next.velocity_y);
        next.x = Fixed((Uint32)next.x + (Uint32)next.velocity_x);
        if (next.pose == 8 && next.bank != 5)
        {
            next.blend_mode = 0x10;
            next.particle = !(low & 1U) && next.remaining == 1;
        }
        if (next.pose == 16 && next.bank == 5)
            next.blend_mode = 0x10;
        next.removed = next.remaining < 0;
    }
    *effect = next;
    *random = rng;
    return true;
}
bool SotnFireHit_Animate(SotnFireHit *effect, SotnDataView dra)
{
    if (!effect || !effect->phase || effect->removed || effect->parameter > 0x2ff ||
        effect->remaining < -1)
        return SDL_SetError("Fire hit lost its private finite animation owner");
    const Uint32 offset = effect->parameter >> 8 == 1 ? 0xdc10U
                          : effect->bank == 5         ? 0xd57cU
                                                      : 0xdbd4U;
    const Uint16 limit = offset == 0xdc10U ? 13 : offset == 0xd57cU ? 25 : 15;
    if (!dra.bytes || dra.size < offset + limit * 4U || effect->pose >= limit - 1U)
        return SDL_SetError("Fire hit animation escapes its bounded private timeline");
    SotnFireHit next = *effect;
    const bool advance = next.remaining > 0;
    if (!next.remaining || (advance && --next.remaining == 0))
    {
        if (advance)
            ++next.pose;
        const Uint16 duration = Read16(dra.bytes + offset + next.pose * 4U);
        if (duration == 0xffffU && next.pose)
        {
            --next.pose;
            next.remaining = -1;
        }
        else
        {
            if (!duration || duration > 32767 || next.pose >= limit - 1U)
                return SDL_SetError("Fire hit has invalid private duration/end metadata");
            next.remaining = (Sint16)duration;
        }
    }
    next.frame = Read16(dra.bytes + offset + next.pose * 4U + 2U);
    if (next.frame > 511)
        return SDL_SetError("Fire hit has an invalid private sprite frame");
    *effect = next;
    return true;
}
bool SotnFireHit_Rebase(SotnFireHit *effect, Sint16 x, Sint16 y)
{
    if (!effect || effect->removed)
        return SDL_SetError("Fire hit translation requires its actual live child");
    if (effect->phase)
    {
        effect->x = Shift(effect->x, x);
        effect->y = Shift(effect->y, y);
    }
    return true;
}
