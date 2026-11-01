// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/7879C.c:EntitySmokePuff profiles0/1/4/5/6/8/10 and alu_anim.c:UpdateAnim,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Tables stay private.
#include <SDL3/SDL_error.h>
#include <sotn/collision.h>
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
    return SotnPlayerSmoke_TickOwned(smoke, dra, source, parameter, priority, contacts, status,
                                     NULL);
}
bool SotnPlayerSmoke_TickOwned(SotnPlayerSmoke *smoke, SotnDataView dra,
                               SotnPlayerFactorySource source, Uint16 parameter, Uint16 priority,
                               Uint32 contacts, Uint32 status,
                               const SotnPlayerSmokeEnvironment *environment)
{
    const Uint16 high = parameter >> 8, low = parameter & 255U;
    const bool wall = high == 4 || high == 8;
    if (!smoke || smoke->phase > 1 || smoke->pose >= 24 || smoke->remaining < -1 ||
        smoke->removed || priority > 509 ||
        (high != 0 && high != 1 && high != 4 && high != 5 && high != 6 && high != 8 &&
         high != 10) ||
        low > 5 || (high == 5 && low > 2) || (high == 8 && low > 3) || !dra.bytes ||
        dra.size < (wall ? (high == 4 ? 0xd5eeU : 0xd5faU) : 0xd5e0U))
        return SDL_SetError(
            "Player smoke requires selected private puff tables and bounded child metadata");
    if (!smoke->phase && (status & 0x20000U))
        return SDL_SetError("US smoke birth uses an unwritten parameter for player status0x20000");
    if (smoke->phase && smoke->parameter != parameter)
        return SDL_SetError("Player smoke changed its actual retained parameter");
    if (!smoke->phase && (wall || high == 6) &&
        (!environment || (wall && !environment->wall_owned)))
        return SDL_SetError(
            "Player wall/skid smoke requires actual live player/collision ownership");
    SotnPlayerSmoke next = *smoke;
    if (!next.phase)
    {
        const bool slide = high == 1, walking = high == 5;
        Sint32 offset =
            walking ? -6 : Word(Read16(dra.bytes + 0xd54c + low * 2U)) + (slide ? -8 : 6);
        if (slide && (contacts & 0x8000U))
            offset /= 2;
        if (high == 6)
            offset = environment->velocity_x > 0 ? 24 : -24;
        else if (source.facing_left)
            offset = -offset;
        if (high == 10)
            offset =
                (Word(Read16(dra.bytes + 0xd54c + low * 2U)) - 2) * (source.facing_left ? -1 : 1);
        next.x = Fixed((Uint32)source.x + (Uint32)offset * 65536U);
        next.y = Fixed((Uint32)source.y + 24U * 65536U);
        const Uint16 index = slide ? 1 : low;
        const Uint16 velocity_index = walking ? (Uint16)(4U - low * 2U) : index;
        const Uint8 *velocity = dra.bytes + 0xd558 + velocity_index * 4U;
        const Uint32 bits = (Uint32)Read16(velocity) | (Uint32)Read16(velocity + 2) << 16;
        next.velocity_y = slide ? -16384 : Fixed(bits);
        next.velocity_x = slide ? (source.facing_left ? 12288 : -12288) : 0;
        next.scale = (Uint16)(Read16(dra.bytes + 0xd570 + index * 2U) + 64U);
        next.priority = (Uint16)(priority + 2U);
        next.parameter = parameter;
        next.facing_left = source.facing_left;
        next.phase = 1;
        if (high == 10)
            next.y = Fixed((Uint32)next.y - 6U * 65536U);
        if (wall)
        {
            const Uint32 table = high == 4 ? 0xd5e0U : 0xd5f0U;
            const Uint16 count = high == 4 ? 14 : 10;
            for (Uint16 i = 0; i < count; ++i)
                if (dra.bytes[table + i] >= 14)
                    return SDL_SetError("Player smoke has an invalid private wall sensor index");
            Uint16 i = (Uint16)(low * 2U);
            while (i < count &&
                   !(environment->wall[dra.bytes[table + i]].effects & SOTN_EFFECT_BLOCK))
                ++i;
            if (i == count)
            {
                next.removed = true;
                *smoke = next;
                return true;
            }
            const Uint16 sensor = dra.bytes[table + i];
            const Uint16 x =
                (Uint16)((Uint16)((Uint32)environment->x >> 16) + environment->wall[sensor].x);
            const Uint16 y =
                (Uint16)((Uint16)((Uint32)environment->y >> 16) + environment->wall[sensor].y);
            next.x = Fixed((Uint32)x << 16 | ((Uint32)source.x & 65535U));
            next.y = Fixed((Uint32)y << 16 | ((Uint32)source.y & 65535U));
            next.velocity_x = 0;
            if (high == 4)
                next.velocity_y = -16384;
            next.scale = (Uint16)(Read16(dra.bytes + 0xd570 + (high == 4 ? 1U : low) * 2U) +
                                  (high == 4 ? 64U : 32U));
        }
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
