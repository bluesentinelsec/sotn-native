// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/7879C.c:EntitySmokePuff profiles0/1 and alu_anim.c:UpdateAnim,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Tables stay private.
#include <SDL3/SDL_error.h>
#include <sotn/player_smoke.h>
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static Sint32 Fixed(Uint32 bits)
{
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint16 Word(Uint16 bits)
{
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static bool Animate(SotnPlayerSmoke *s, SotnDataView dra)
{
    if (!s->remaining)
        s->remaining = Word(Read16(dra.bytes + 0xd57c + s->pose * 4U));
    else if (s->remaining != -1 && --s->remaining == 0)
    {
        ++s->pose;
        const Sint16 duration = Word(Read16(dra.bytes + 0xd57c + s->pose * 4U));
        if (duration == -1)
        {
            --s->pose;
            s->remaining = -1;
        }
        else
            s->remaining = duration;
    }
    if (s->pose >= 24 || s->remaining < -1 || !s->remaining)
        return SDL_SetError("Player smoke has invalid private animation duration/end metadata");
    s->sprite = Read16(dra.bytes + 0xd57e + s->pose * 4U);
    if (s->sprite > 511)
        return SDL_SetError("Player smoke has an invalid private sprite frame");
    return true;
}
bool SotnPlayerSmoke_Tick(SotnPlayerSmoke *smoke, SotnDataView dra, SotnPlayerFactorySource source,
                          Uint16 parameter, Uint16 priority, Uint32 contacts, Uint32 status)
{
    if (!smoke || smoke->phase > 1 || smoke->pose >= 24 || smoke->remaining < -1 ||
        smoke->removed || priority > 509 || parameter >> 8 > 1 || (parameter & 255U) > 5 ||
        !dra.bytes || dra.size < 0xd5e0)
        return SDL_SetError("Player smoke requires selected private ground/slide snapshots");
    if (!smoke->phase && (status & 0x20000U))
        return SDL_SetError("US smoke birth uses an unwritten parameter for player status0x20000");
    if (smoke->phase && smoke->parameter != parameter)
        return SDL_SetError("Player smoke changed its actual retained parameter");
    SotnPlayerSmoke next = *smoke;
    if (!next.phase)
    {
        const Uint16 low = parameter & 255U;
        const bool slide = (parameter >> 8) == 1;
        Sint32 offset = Word(Read16(dra.bytes + 0xd54c + low * 2U)) + (slide ? -8 : 6);
        if (slide && (contacts & 0x8000U))
            offset /= 2;
        if (source.facing_left)
            offset = -offset;
        next.x = Fixed((Uint32)source.x + (Uint32)offset * 65536U);
        next.y = Fixed((Uint32)source.y + 24U * 65536U);
        const Uint16 index = slide ? 1 : low;
        const Uint8 *velocity = dra.bytes + 0xd558 + index * 4U;
        const Uint32 bits = (Uint32)Read16(velocity) | (Uint32)Read16(velocity + 2) << 16;
        next.velocity_y = slide ? -16384 : Fixed(bits);
        next.velocity_x = slide ? (source.facing_left ? 12288 : -12288) : 0;
        next.scale = (Uint16)(Read16(dra.bytes + 0xd570 + index * 2U) + 64U);
        next.priority = (Uint16)(priority + 2U);
        next.parameter = parameter;
        next.facing_left = source.facing_left;
        next.phase = 1;
    }
    else
    {
        next.y = Fixed((Uint32)next.y + (Uint32)next.velocity_y);
        next.x = Fixed((Uint32)next.x + (Uint32)next.velocity_x);
        if (next.remaining < 0)
        {
            next.removed = true;
            *smoke = next;
            return true;
        }
    }
    if (!Animate(&next, dra))
        return false;
    *smoke = next;
    return true;
}
bool SotnPlayerSmoke_Rebase(SotnPlayerSmoke *s, Sint16 x, Sint16 y)
{
    if (!s || s->phase > 1)
        return SDL_SetError("Player smoke rebase requires its actual bounded child");
    s->x = Fixed((Uint32)s->x + (Uint32)(Sint32)x * 65536U);
    s->y = Fixed((Uint32)s->y + (Uint32)(Sint32)y * 65536U);
    return true;
}
