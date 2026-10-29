// SPDX-License-Identifier: AGPL-3.0-or-later
// st/e_particles.h:EntityEnemyBlood and st_common.h:UnkEntityFunc0,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. SDK tables stay private.
#include <SDL3/SDL_error.h>
#include <sotn/enemy_blood.h>

static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return bits <= 32767U ? (Sint16)bits : (Sint16)((Sint32)bits - 65536);
}
static Sint32 Fixed(Uint32 bits)
{
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint16 Pixel(Sint32 value)
{
    return Word((Sint32)((Uint32)value >> 16));
}
static Sint32 Add(Sint32 a, Sint32 b)
{
    return Fixed((Uint32)a + (Uint32)b);
}
static Uint32 Random(Uint32 *state)
{
    *state = *state * 0x01010101U + 1U;
    return *state >> 24;
}
bool SotnEnemyBlood_Tick(SotnEnemyBlood *blood, SotnEnemyBloodInput in, Uint32 *random,
                         SotnEnemyBloodEvents *out)
{
    if (!blood || !random || !out || blood->removed || blood->phase > 1 ||
        (blood->phase && (!blood->timer || blood->timer > 48)))
        return SDL_SetError("Enemy blood requires an actual bounded actor/RNG/healing snapshot");
    SotnEnemyBlood next = *blood;
    SotnEnemyBloodEvents events = {0};
    Uint32 rng = *random;
    if (!next.phase)
    {
        if (!in.allocated)
        {
            next.removed = true;
            goto done;
        }
        next.x = in.x;
        next.y = in.y;
        next.timer = 48;
        next.hitbox = true;
        next.hitbox_height = 8;
        next.hitbox_width = 0;
        next.size = 0;
        for (Uint16 i = 0; i < SOTN_ENEMY_BLOOD_PARTICLES; ++i)
        {
            SotnEnemyBloodParticle *p = &next.particles[i];
            const Uint16 n = (Uint16)(SOTN_ENEMY_BLOOD_PARTICLES - i);
            p->x = Word(Pixel(in.x) - 5 + (Sint32)(Random(&rng) & 7U));
            p->y = Word(Pixel(in.y) - 5 + (Sint32)(Random(&rng) & 7U));
            p->fixed_x = p->fixed_y = 0;
            p->width = p->height = 4;
            const Sint32 angle = in.facing_left ? 0xcc0 + n * 64 : 0xb40 - n * 64,
                         speed = (Sint32)(Random(&rng) & 15U) * 16 + 0x180;
            SotnMathTrig trig = {0};
            if (!SotnMath_Trig(in.executable, angle, &trig))
                return false;
            p->velocity_x = trig.cosine * speed / 16;
            p->velocity_y = trig.sine * speed / 16;
            p->acceleration_x = -(p->velocity_x / 64);
            p->acceleration_y = -(p->velocity_y / 48) + 0xc00;
            p->timer = p->interval = (Sint16)((Random(&rng) & 7U) + 7U);
        }
        next.velocity_x = in.facing_left ? 81920 : -81920;
        next.acceleration = in.facing_left ? -512 : 512;
        next.phase = 1;
    }
    else
    {
        if (!--next.timer)
        {
            next.removed = true;
            goto done;
        }
        if (next.hitbox)
        {
            if (!in.absorbing)
                next.hitbox = false;
            else
            {
                next.velocity_x = Add(next.velocity_x, next.acceleration);
                const Sint16 before = Pixel(next.x);
                next.x = Add(next.x, next.velocity_x);
                const Sint16 delta = Word(before - Pixel(next.x));
                const Uint16 distance = (Uint16)(delta < 0 ? -(Sint32)delta : delta);
                if (next.timer > 16)
                {
                    next.size = (Uint16)(next.size + distance);
                    next.hitbox_width = (Uint8)(next.size / 2U);
                    next.hitbox_height = (Uint8)(next.size / 4U + 8U);
                }
                else
                    next.hitbox = false;
                if (in.hit)
                {
                    events.heal = in.heal_pending ? 0 : in.bloodstone ? 16 : 8;
                    events.contact = true;
                    next.hitbox = false;
                }
            }
        }
        for (Uint16 i = 0; i < SOTN_ENEMY_BLOOD_PARTICLES; ++i)
        {
            SotnEnemyBloodParticle *p = &next.particles[i];
            if (p->width > 4 || p->height != p->width || p->interval < 7 || p->interval > 14)
                return SDL_SetError("Enemy blood tile lost its original dimensions/interval");
            p->fixed_x = Fixed(((Uint32)p->fixed_x & 65535U) | (Uint32)(Uint16)p->x << 16);
            p->fixed_y = Fixed(((Uint32)p->fixed_y & 65535U) | (Uint32)(Uint16)p->y << 16);
            p->velocity_x = Add(p->velocity_x, p->acceleration_x);
            p->velocity_y = Add(p->velocity_y, p->acceleration_y);
            p->fixed_x = Add(p->fixed_x, p->velocity_x);
            p->fixed_y = Add(p->fixed_y, p->velocity_y);
            p->x = Pixel(p->fixed_x);
            p->y = Pixel(p->fixed_y);
            p->timer = Word((Sint32)p->timer - 1);
            if (!p->timer && p->width)
            {
                --p->width;
                --p->height;
                if (!(p->width & 1U))
                {
                    p->x = Word((Sint32)p->x + 1);
                    p->y = Word((Sint32)p->y + 1);
                }
                p->timer = p->interval;
            }
        }
    }
done:
    *blood = next;
    *random = rng;
    *out = events;
    return true;
}
bool SotnEnemyBlood_Translate(SotnEnemyBlood *blood, Sint32 dx, Sint32 dy)
{
    if (!blood || dx < SDL_MIN_SINT16 || dx > SDL_MAX_SINT16 || dy < SDL_MIN_SINT16 ||
        dy > SDL_MAX_SINT16)
        return SDL_SetError("Enemy blood rebase requires an owner and signed-word shifts");
    if (blood->phase && !blood->removed)
    {
        blood->x = Add(blood->x, Fixed((Uint32)dx * 65536U));
        blood->y = Add(blood->y, Fixed((Uint32)dy * 65536U));
        for (Uint16 i = 0; i < SOTN_ENEMY_BLOOD_PARTICLES; ++i)
        {
            blood->particles[i].x = Word(blood->particles[i].x + dx);
            blood->particles[i].y = Word(blood->particles[i].y + dy);
        }
    }
    return true;
}
