// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/7E4BC.c:func_8011E4BC profiles1/9/13, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Profile tables remain private.
#include <sotn/combat.h>
#include <sotn/player_particles.h>
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static Uint32 Read32(const Uint8 *p)
{
    return (Uint32)Read16(p) | (Uint32)Read16(p + 2) << 16;
}
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint32 Fixed(Uint32 bits)
{
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint16 Pixel(Sint32 value)
{
    return (Sint16)(value >= 0 ? value / 65536 : -((-(Sint64)value + 65535) / 65536));
}
static bool Profile(SotnDataView dra, Uint16 parameter, const Uint8 **out)
{
    const Uint16 type = parameter >> 8;
    if (!out || !dra.bytes || (type != 1 && type != 9 && type != 13) || dra.size < 0xdbd4)
        return SDL_SetError("Player particles require a selected private profile1/9/13");
    const Uint32 pointer = Read32(dra.bytes + 0xdb98 + type * 4U);
    if (pointer < 0x800a0000 || pointer % 4 || pointer - 0x800a0000U > dra.size ||
        dra.size - (pointer - 0x800a0000U) < 16)
        return SDL_SetError("Player particle profile points outside its private image");
    const Uint8 *p = dra.bytes + (size_t)(pointer - 0x800a0000U);
    if (!p[0] || p[0] > SOTN_PLAYER_PARTICLE_LIMIT || !p[4] || !p[5] || Read16(p + 10) != type)
        return SDL_SetError("Player particle profile has invalid count, geometry or behavior");
    *out = p;
    return true;
}
bool SotnPlayerParticles_Rows(SotnDataView dra, Uint16 parameter, Uint16 *count)
{
    const Uint8 *p = NULL;
    if (!count)
        return SDL_InvalidParamError("count");
    if (!Profile(dra, parameter, &p))
        return false;
    *count = (Uint16)(p[0] + 1U);
    return true;
}
bool SotnPlayerParticles_Tick(SotnPlayerParticles *particles, SotnDataView dra,
                              SotnPlayerParticlesInput in, Uint32 *random)
{
    if (!particles || !random || particles->phase > 1 ||
        particles->count > SOTN_PLAYER_PARTICLE_LIMIT || particles->removed ||
        in.player_priority > 509 || in.water_level < 0 || in.water_level > 32767)
        return SDL_SetError("Player particles require actual bounded child/player/RNG snapshots");
    const Uint8 *profile = NULL;
    if (!Profile(dra, in.parameter, &profile))
        return false;
    if (particles->phase &&
        (particles->parameter != in.parameter || particles->count != profile[0]))
        return SDL_SetError("Player particles changed their actual retained private profile");
    SotnPlayerParticles next = *particles;
    Uint32 rng = *random;
    const Uint16 type = in.parameter >> 8;
    if (!next.phase)
    {
        if (!in.allocated)
        {
            next.removed = true;
            goto done;
        }
        const Sint32 priority = (Sint32)in.player_priority + Word(Read16(profile + 6));
        if (priority < 0 || priority > 511)
            return SDL_SetError("Player particle profile priority escapes the original table");
        next.count = profile[0];
        next.parameter = in.parameter;
        next.priority = (Uint16)priority;
        next.flags = Read32(profile + 12);
        next.lifetime = type == 13 ? 256 : 0;
        for (Uint16 i = 0; i < next.count; ++i)
        {
            SotnPlayerParticle *t = &next.tiles[i];
            *t = (SotnPlayerParticle){.x = Pixel(in.source.x) * 65536,
                                      .y = Pixel(in.source.y) * 65536,
                                      .screen_x = Pixel(in.source.x),
                                      .screen_y = Pixel(in.source.y),
                                      .red = profile[1],
                                      .green = profile[2],
                                      .blue = profile[3],
                                      .width = profile[4],
                                      .height = profile[5],
                                      .draw_mode = Read16(profile + 8)};
            if (type == 13)
            {
                const Sint32 speed = (Sint32)(SotnCombat_Random(&rng) & 32767U) - 16384;
                t->velocity_x = speed >= 0 ? speed / 2 : -((-speed + 1) / 2);
                t->velocity_y = -(Sint32)((SotnCombat_Random(&rng) & 8191U) + 24576U);
                t->timer = (Uint16)(i * 4U);
            }
            else
            {
                t->velocity_x = ((Sint32)(SotnCombat_Random(&rng) & 511U) - 256) * 256;
                t->velocity_y = ((Sint32)(SotnCombat_Random(&rng) & 511U) - 256) * 256;
            }
        }
        next.phase = 1;
        goto done;
    }
    // Original case13 falls through into the second lifetime decrement.
    if (type == 13 && (!next.lifetime || --next.lifetime == 0 || --next.lifetime == 0))
    {
        next.removed = true;
        goto done;
    }
    for (Uint16 i = 0; i < next.count; ++i)
    {
        SotnPlayerParticle *t = &next.tiles[i];
        t->x = Fixed((Uint32)(Sint32)t->screen_x * 65536U | ((Uint32)t->x & 65535U));
        t->y = Fixed((Uint32)(Sint32)t->screen_y * 65536U | ((Uint32)t->y & 65535U));
        if (type == 13 && t->timer)
            --t->timer;
        else
        {
            const Uint8 fade = type == 13 ? 1 : 3;
            t->red = (Uint8)(t->red - fade);
            t->green = (Uint8)(t->green - fade);
            t->blue = (Uint8)(t->blue - fade);
            t->x = Fixed((Uint32)t->x + (Uint32)t->velocity_x);
            t->y = Fixed((Uint32)t->y + (Uint32)t->velocity_y);
            if (type == 13)
            {
                t->draw_mode &= (Uint16)~8U;
                if (!in.water_level || Pixel(t->y) <= (Sint32)in.player_y - in.water_level + 25)
                    t->draw_mode |= 8;
            }
            else
            {
                t->velocity_y = Fixed((Uint32)t->velocity_y + 10240U);
                if (t->red < 8)
                {
                    next.removed = true;
                    goto done;
                }
            }
        }
        t->screen_x = Pixel(t->x);
        t->screen_y = Pixel(t->y);
    }
done:
    *particles = next;
    *random = rng;
    return true;
}
bool SotnPlayerParticles_Rebase(SotnPlayerParticles *p, Sint16 x, Sint16 y)
{
    if (!p || p->phase > 1 || p->count > SOTN_PLAYER_PARTICLE_LIMIT)
        return SDL_SetError("Player particle rebase lost its actual bounded child");
    for (Uint16 i = 0; i < p->count; ++i)
    {
        SotnPlayerParticle *t = &p->tiles[i];
        t->x = Fixed((Uint32)t->x + (Uint32)(Sint32)x * 65536U);
        t->y = Fixed((Uint32)t->y + (Uint32)(Sint32)y * 65536U);
        t->screen_x = Word(t->screen_x + x);
        t->screen_y = Word(t->screen_y + y);
    }
    return true;
}
