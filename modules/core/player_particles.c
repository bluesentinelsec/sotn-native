// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/7E4BC.c:func_8011E4BC profiles0/1/3/4/5/6/7/8/9/10/11/12/13 and 692E8.c:AluInit,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Profile tables remain private.
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
static Sint32 Shift(Sint32 value, Uint16 bits)
{
    const Sint64 scale = (Sint64)1 << bits;
    return (Sint32)(value >= 0 ? value / scale : -((-(Sint64)value + scale - 1) / scale));
}
// Behaviour 3: Fire's sparks (4), Fire boomerang's smoke (11) and holy water flame smoke (7).
static bool Rising(Uint16 type)
{
    return type == 4 || type == 7 || type == 11;
}
static bool VectorProfile(Uint16 type)
{
    return type == 5 || type == 6;
}
bool SotnPlayerParticleVectors_Init(SotnPlayerParticleVectors *vectors, SotnDataView executable,
                                    Uint32 *random)
{
    if (!vectors || !random)
        return SDL_SetError("Particle vectors require their actual initialization and RNG owner");
    SotnPlayerParticleVectors next = {0};
    Uint32 rng = *random;
    for (Uint16 i = 0; i < 32; ++i)
    {
        const Uint16 radius = (Uint16)((SotnCombat_Random(&rng) & 1023U) + 256U);
        const Sint32 intensity = (Sint32)((SotnCombat_Random(&rng) & 255U) + 256U);
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(executable, radius, &trig))
            return false;
        next.x[i] = Shift((Sint32)trig.cosine * 16 * intensity, 8);
        next.y[i] = -Shift((Sint32)trig.sine * 16 * intensity, 7);
    }
    next.initialized = true;
    *vectors = next;
    *random = rng;
    return true;
}
static bool Profile(SotnDataView dra, Uint16 parameter, const Uint8 **out)
{
    const Uint16 type = parameter >> 8;
    if (!out || !dra.bytes ||
        (type != 0 && type != 1 && type != 3 && !Rising(type) && !VectorProfile(type) &&
         type != 8 && type != 9 && type != 10 && type != 11 && type != 12 && type != 13) ||
        dra.size < 0xdbd4)
        return SDL_SetError("Player particles require a selected private profile0/1/3..13");
    const Uint32 pointer = Read32(dra.bytes + 0xdb98 + type * 4U);
    if (pointer < 0x800a0000 || pointer % 4 || pointer - 0x800a0000U > dra.size ||
        dra.size - (pointer - 0x800a0000U) < 16)
        return SDL_SetError("Player particle profile points outside its private image");
    const Uint8 *p = dra.bytes + (size_t)(pointer - 0x800a0000U);
    if (!p[0] || p[0] > SOTN_PLAYER_PARTICLE_LIMIT || !p[4] || !p[5] ||
        Read16(p + 10) != (type == 3             ? 2
                           : VectorProfile(type) ? 4
                           : Rising(type)        ? 3
                                                 : type))
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
    // Evaluated once: GCC 13 Release (-O3) mis-branched the birth chain when this predicate was
    // inlined into it (profile0 tinks took the generic branch).
    const bool rising = Rising(type);
    if (VectorProfile(type) && next.phase && (!next.lifetime || next.lifetime > 47))
        return SDL_SetError("Vector particles lost their actual bounded lifetime");
    if (rising && next.phase && (!next.lifetime || next.lifetime > 31))
        return SDL_SetError("Fire particles lost their actual bounded lifetime");
    if (!next.phase)
    {
        if (!in.allocated)
        {
            next.removed = true;
            goto done;
        }
        if (VectorProfile(type) && (!in.vectors || !in.vectors->initialized))
            return SDL_SetError("Vector particles require actual initialized AluInit vectors");
        const Sint32 priority = (Sint32)in.player_priority + Word(Read16(profile + 6));
        if (priority < 0 || priority > 511)
            return SDL_SetError("Player particle profile priority escapes the original table");
        next.count = profile[0];
        next.parameter = in.parameter;
        next.priority = (Uint16)priority;
        next.flags = Read32(profile + 12);
        next.lifetime = type == 13                              ? 256
                        : type == 8 || type == 10 || type == 12 ? 63
                        : type == 3                             ? 47
                        : rising                                ? 31
                        : VectorProfile(type)                   ? 47
                                                                : 0;
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
            if (VectorProfile(type))
            {
                t->x = Word((Sint32)Pixel(in.source.x) + (Sint32)(SotnCombat_Random(&rng) & 31U) -
                            16) *
                       65536;
                t->y = Word((Sint32)Pixel(in.source.y) + (Sint32)(SotnCombat_Random(&rng) & 31U) -
                            20) *
                       65536;
                const Uint16 at = (Uint16)(SotnCombat_Random(&rng) & 31U);
                t->velocity_x = in.vectors->x[at];
                t->velocity_y = in.vectors->y[at];
                t->screen_x = Pixel(t->x);
                t->screen_y = Pixel(t->y);
            }
            else if (rising)
            {
                t->x =
                    Word(Pixel(in.source.x) + (Sint32)(SotnCombat_Random(&rng) & 15U) - 7) * 65536;
                t->y = Word(Pixel(in.source.y) - (Sint32)(SotnCombat_Random(&rng) & 31U)) * 65536;
                t->velocity_y = -49152 - (Sint32)(SotnCombat_Random(&rng) & 32767U);
                t->velocity_x = in.parent_velocity_x;
                t->timer = (Uint16)(i * 2U + 15U);
                t->screen_x = Pixel(t->x);
                t->screen_y = Pixel(t->y);
            }
            else if (type == 3 || type == 8)
            {
                const Uint16 first = SotnCombat_Random(&rng);
                const Sint32 velocity =
                    type == 8 ? (Sint32)first * 4 - 65536 : (Sint32)first - 16384;
                t->velocity_y = -(Sint32)(SotnCombat_Random(&rng) & 16383U);
                t->x = Fixed((Uint32)t->x + (Uint32)velocity * (type == 8 ? 35U : 19U) +
                             (type == 8 ? (in.facing_left ? 12U : (Uint32)-12) * 65536U : 0));
                t->velocity_x = type == 8 ? Shift(velocity, 2) : velocity;
                const Uint16 third = SotnCombat_Random(&rng);
                t->y = Word(Pixel(in.source.y) +
                            (type == 8 ? -(Sint32)(third & 31U) : (Sint32)(third & 7U) - 3)) *
                       65536;
                t->timer = (Uint16)((SotnCombat_Random(&rng) & (type == 8 ? 31U : 15U)) + 16U);
                t->screen_x = Pixel(t->x);
                t->screen_y = Pixel(t->y);
            }
            else if (type == 10)
            {
                t->x = (Sint32)in.player_x * 65536;
                t->y = (Sint32)in.player_y * 65536;
                t->screen_x = in.player_x;
                t->screen_y = in.player_y;
                t->velocity_y = -(Sint32)(i * i * 4096U + 16384U);
                t->timer = 63;
            }
            else if (type == 12)
            {
                const Sint32 index = i < 10 ? i : i - 10;
                const Sint32 velocity = index * index * 6144 + 8192;
                t->velocity_y = i < 10 ? -velocity : velocity;
                t->timer = 63;
            }
            else if (type == 0)
            {
                // Rebound Stone's tink: rand() & 0x7FFF is both the angle and the speed shift.
                const Uint16 angle = (Uint16)(SotnCombat_Random(&rng) & 32767U);
                SotnMathTrig trig = {0};
                if (!SotnMath_Trig(in.executable, angle, &trig))
                    return false;
                const Sint32 scale = (angle & 1U) ? 8 : 4;
                t->velocity_x = (Sint32)trig.cosine * scale;
                t->velocity_y = -((Sint32)trig.sine * scale);
            }
            else if (type == 13)
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
    if (type == 10 && in.player_step != 34)
    {
        next.removed = true;
        goto done;
    }
    // Original cases3/4/8/10/11/12/13 fall through into the second lifetime decrement.
    if ((type == 3 || rising || type == 8 || type == 10 || type == 12 || type == 13) &&
        (!next.lifetime || --next.lifetime == 0 || --next.lifetime == 0))
    {
        next.removed = true;
        goto done;
    }
    if (VectorProfile(type) && (!next.lifetime || --next.lifetime == 0))
    {
        next.removed = true;
        goto done;
    }
    for (Uint16 i = 0; i < next.count; ++i)
    {
        SotnPlayerParticle *t = &next.tiles[i];
        t->x = Fixed((Uint32)(Sint32)t->screen_x * 65536U | ((Uint32)t->x & 65535U));
        t->y = Fixed((Uint32)(Sint32)t->screen_y * 65536U | ((Uint32)t->y & 65535U));
        if (VectorProfile(type))
        {
            t->x = Fixed((Uint32)t->x + (Uint32)t->velocity_x);
            t->y = Fixed((Uint32)t->y + (Uint32)t->velocity_y);
            t->velocity_y = Fixed((Uint32)t->velocity_y - (Uint32)Shift(t->velocity_y, 5));
            if (!(next.lifetime & 7U))
            {
                t->velocity_x = Shift(t->velocity_x, 1);
                t->velocity_y = Shift(t->velocity_y, 1);
                if (next.lifetime & 32U)
                    t->velocity_y = Shift(t->velocity_y, 1);
                if (next.lifetime == 24)
                    t->draw_mode = 2;
            }
            t->width = t->height =
                (Uint8)(3U - (next.lifetime < 9 ? 1U : 0U) - (next.lifetime < 4 ? 1U : 0U));
        }
        else if (type == 3 || type == 8)
        {
            t->timer = (Uint16)(t->timer - 1U);
            if (!t->timer)
                t->draw_mode |= 8U;
            t->x = Fixed((Uint32)t->x + (Uint32)t->velocity_x);
            t->y = Fixed((Uint32)t->y + (Uint32)t->velocity_y);
            t->velocity_y = Fixed((Uint32)t->velocity_y + 5120U);
        }
        else if (rising || type == 10 || type == 12)
        {
            t->y = Fixed((Uint32)t->y + (Uint32)t->velocity_y);
            t->timer = (Uint16)Word((Sint32)t->timer - 1);
            if (Word(t->timer) < 0)
                t->draw_mode |= 8U;
        }
        else if (type == 13 && t->timer)
            --t->timer;
        else if (type == 0)
        {
            t->y = Fixed((Uint32)t->y + (Uint32)t->velocity_y);
            t->x = Fixed((Uint32)t->x + (Uint32)t->velocity_x);
            t->red = (Uint8)(t->red - 6U);
            t->green = (Uint8)(t->green - 6U);
            t->blue = (Uint8)(t->blue - 6U);
            if (t->red < 8)
            {
                next.removed = true;
                goto done;
            }
        }
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
                t->draw_mode = (Uint16)(t->draw_mode & 0xfff7U);
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
