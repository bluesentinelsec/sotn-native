// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/7E4BC.c:EntityHitByLightning, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. SDK/CLUT tables stay private.
#include <SDL3/SDL_error.h>
#include <sotn/combat.h>
#include <sotn/thunder_hit.h>
static Sint32 Fixed(Uint32 bits)
{
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint32 Shift(Sint32 value, Uint16 bits)
{
    const Sint64 divisor = (Sint64)1 << bits;
    return (Sint32)(value >= 0 ? value / divisor : -((-(Sint64)value + divisor - 1) / divisor));
}
static Sint16 Pixel(Sint32 value)
{
    return (Sint16)Shift(value, 16);
}
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
bool SotnThunderHit_Tick(SotnThunderHit *effect, SotnThunderHitInput in, Uint32 *random)
{
    if (!effect || !random || effect->removed || effect->phase > 2 || in.priority < 2 ||
        in.priority > 509 || (effect->phase && effect->parameter != in.parameter) ||
        (effect->phase == 2 && (effect->fade <= 0 || effect->fade > 31)) || effect->thickness < 0 ||
        effect->thickness > 16)
        return SDL_SetError("Thunder hit requires actual bounded child/player/RNG ownership");
    if (effect->phase)
        for (Uint16 row = 0; row < 6; ++row)
            if (effect->quads[row].palette >= 768 || effect->quads[row].priority > 511 ||
                effect->quads[row].page != 0x1a || effect->quads[row].draw_mode != 0x133)
                return SDL_SetError("Thunder hit lost its actual retained GT4 metadata");
    SotnThunderHit next = *effect;
    Uint32 rng = *random;
    next.sound_count = 0;
    bool stop = false;
    if (in.parameter & 0xff00U)
    {
        next.age = Word((Sint32)next.age + 1);
        stop = next.age > 168;
    }
    else
        stop = in.player_step != 10;
    if (!next.phase)
    {
        if (!in.allocated)
        {
            next.removed = true;
            goto done;
        }
        if (!in.dra.bytes || in.dra.size < 0xdc80U)
            return SDL_SetError("Thunder hit lost its private birth CLUT pair");
        if (Read16(in.dra.bytes + 0xdc7cU) >= 768 || Read16(in.dra.bytes + 0xdc7eU) >= 768)
            return SDL_SetError("Thunder hit private CLUT pair exceeds actual palette ownership");
        const Sint16 age = next.age;
        next = (SotnThunderHit){.x = in.source.x,
                                .y = in.source.y,
                                .age = age,
                                .phase = 1,
                                .parameter = in.parameter,
                                .thickness = 16};
        next.angle = (Sint16)(((in.parameter & 15U) << 9) + (SotnCombat_Random(&rng) & 63U));
        next.wave = (Sint16)SotnCombat_Random(&rng);
        next.speed = (Sint16)((SotnCombat_Random(&rng) & 511U) + 256U);
        for (Uint16 row = 0; row < 6; ++row)
        {
            SotnPlayerEffectQuad *q = &next.quads[row];
            q->page = 0x1a;
            q->palette = Read16(in.dra.bytes + 0xdc7cU + (SotnCombat_Random(&rng) & 1U) * 2U);
            q->priority = (Uint16)(in.priority - 2U);
            q->draw_mode = 0x133;
            for (Uint16 v = 0; v < 4; ++v)
            {
                q->x[v] = Pixel(next.x);
                q->y[v] = Pixel(next.y);
                q->red[v] = q->green[v] = q->blue[v] = 128;
            }
        }
        next.airborne = in.player_velocity_y != 0 && in.player_step != 16;
        next.sounds[0] = 0x69d;
        next.sounds[1] = 0x665;
        next.sound_count = 2;
    }
    else
    {
        const bool fading = next.phase == 2;
        if (fading)
        {
            if (--next.fade == 0)
            {
                next.removed = true;
                goto done;
            }
            if (next.thickness > 0)
                --next.thickness;
        }
        next.angle = (Sint16)(((in.parameter & 15U) << 9) +
                              (SotnCombat_Random(&rng) & (fading ? 255U : 511U)));
        SotnMathTrig wave = {0}, angle = {0};
        if (!SotnMath_Trig(in.executable, next.wave, &wave) ||
            !SotnMath_Trig(in.executable, next.angle, &angle))
            return false;
        next.wave = Word((Sint32)next.wave + next.speed);
        const Sint32 x_factor = fading ? (Sint32)(SotnCombat_Random(&rng) % 8U) + 8 : 12,
                     y_factor = fading ? (Sint32)(SotnCombat_Random(&rng) % 8U) + 10 : 14;
        const Sint32 x = Shift((Sint32)angle.cosine * wave.sine, 7) * x_factor,
                     y = -Shift((Sint32)angle.sine * wave.sine, 7) * y_factor;
        next.x = Fixed((Uint32)in.player_x + (Uint32)x);
        next.y = Fixed((Uint32)in.player_y + (Uint32)y + (fading ? (Uint32)next.drift : 0U));
        if (fading)
            next.drift = Fixed((Uint32)next.drift - 32768U);
        else if (stop || (next.airborne && (in.contacts & 14U)))
        {
            next.fade = (Sint16)((SotnCombat_Random(&rng) & 15U) + 16U);
            next.phase = 2;
        }
    }
    const Sint16 x = Word(Pixel(next.x) + (Sint32)(SotnCombat_Random(&rng) & 7U) - 4),
                 y = Word(Pixel(next.y) + (Sint32)(SotnCombat_Random(&rng) & 31U) - 24);
    SotnMathTrig wave = {0}, left = {0}, right = {0};
    if (!SotnMath_Trig(in.executable, next.wave, &wave) ||
        !SotnMath_Trig(in.executable, (Sint32)next.angle + 1024, &right) ||
        !SotnMath_Trig(in.executable, (Sint32)next.angle - 1024, &left))
        return false;
    const Sint32 thickness = Shift((Sint32)next.thickness * wave.sine, 12);
    for (Uint16 row = 0; row < 6; ++row)
    {
        SotnPlayerEffectQuad *q = &next.quads[row];
        if (row < 5)
            *q = next.quads[row + 1U];
        else
        {
            q->x[0] = q->x[1];
            q->y[0] = q->y[1];
            q->x[2] = q->x[3];
            q->y[2] = q->y[3];
            q->x[1] = Word(x + Shift(Shift(right.cosine, 4) * thickness, 8));
            q->y[1] = Word(y - Shift(Shift(right.sine, 4) * thickness, 8));
            q->x[3] = Word(x + Shift(Shift(left.cosine, 4) * thickness, 8));
            q->y[3] = Word(y - Shift(Shift(left.sine, 4) * thickness, 8));
            const Uint16 phase = (Uint16)next.wave & 4095U;
            q->priority = (Uint16)(in.priority + (phase < 1024 || phase >= 3072 ? 2 : -2));
        }
        q->u[0] = q->u[2] = (Uint8)(144U + row * 16U);
        q->u[1] = q->u[3] = (Uint8)(160U + row * 16U);
        q->v[0] = q->v[1] = 192;
        q->v[2] = q->v[3] = 207;
    }
done:
    *effect = next;
    *random = rng;
    return true;
}
bool SotnThunderHit_Rebase(SotnThunderHit *effect, Sint16 x, Sint16 y, bool room)
{
    if (!effect || effect->removed)
        return SDL_SetError("Thunder translation requires its actual live child");
    if (effect->phase)
    {
        effect->x = Fixed((Uint32)effect->x + (Uint32)(Sint32)x * 65536U);
        effect->y = Fixed((Uint32)effect->y + (Uint32)(Sint32)y * 65536U);
        for (Uint16 row = 0; row < 6; ++row)
            if (effect->quads[row].draw_mode & (room ? 0x100U : 2U))
                for (Uint16 v = 0; v < 4; ++v)
                {
                    effect->quads[row].x[v] = Word(effect->quads[row].x[v] + x);
                    effect->quads[row].y[v] = Word(effect->quads[row].y[v] + y);
                }
    }
    return true;
}
