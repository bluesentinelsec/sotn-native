// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/7E4BC.c:EntityTeleport/func_80124164 and dra/7879C.c:func_80119E78,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Colors/UVs stay private.
#include <sotn/combat.h>
#include <sotn/teleport_effect.h>

static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint32 Add(Sint32 a, Sint32 b)
{
    const Uint32 bits = (Uint32)a + (Uint32)b;
    Sint32 value = 0;
    SDL_memcpy(&value, &bits, sizeof(value));
    return value;
}
static Sint16 Pixel(Sint32 value)
{
    return (Sint16)(value >= 0 ? value / 65536 : -((-(Sint64)value + 65535) / 65536));
}
static Sint32 High(Sint32 value, Sint16 high)
{
    const Uint32 bits = (Uint32)(Uint16)high << 16 | ((Uint32)value & 65535U);
    SDL_memcpy(&value, &bits, sizeof(value));
    return value;
}
static bool Gradient(SotnPlayerEffectQuad *q, SotnDataView dra, Sint32 color, Sint16 y,
                     Sint32 radius, bool other_castle)
{
    q->y[0] = q->y[1] = Word(y - radius);
    q->y[2] = q->y[3] = Word(y + radius);
    if (q->y[0] < 0)
        q->y[0] = q->y[1] = 0;
    // Preserve the original top-coordinate test, including the unclamped ordinary bottom.
    if (q->y[0] > 240)
        q->y[2] = q->y[3] = 240;
    for (Uint16 c = 0; c < 3; ++c)
    {
        Uint8 *channel = c == 0 ? q->red : c == 1 ? q->green : q->blue;
        for (Uint16 side = 0; side < 2; ++side)
        {
            const size_t at = 0xe270U + (side ? 0U : other_castle ? 24U : 12U) + c * 4U;
            Uint32 value = 0;
            for (Uint16 b = 0; b < 4; ++b)
                value |= (Uint32)dra.bytes[at + b] << (b * 8U);
            if (value > 255)
                return SDL_SetError("Teleport gradient requires original private color words");
            channel[side] = channel[side + 2U] = (Uint8)(value * (Uint32)color / 256U);
        }
    }
    return true;
}
static void Particle(SotnPlayerEffectQuad *q, SotnTeleportShared *shared, Uint16 i,
                     SotnDataView dra)
{
    if (!q->green[0])
    {
        if (!--q->green[1])
            ++q->green[0];
    }
    else if (q->green[0] == 1)
    {
        const Sint16 x = shared->points[i].x, y = shared->points[i].y;
        shared->points[i].y = Word(y - 16);
        const Uint8 frame = q->blue[0];
        if (frame == 6)
        {
            q->draw_mode |= 8U;
            ++q->green[0];
            return;
        }
        const Sint16 half = frame >= 3 ? 4 : 6;
        const Uint8 *uv = dra.bytes + 0xd094U + frame * 8U;
        for (Uint16 v = 0; v < 4; ++v)
        {
            q->x[v] = Word(x + (v & 1U ? half : -half));
            q->y[v] = Word(y + (v & 2U ? half : -half));
            q->u[v] = uv[v * 2U];
            q->v[v] = uv[v * 2U + 1U];
        }
        ++q->blue[1];
        if (!(q->blue[1] & 1U))
            ++q->blue[0];
        q->draw_mode &= 0xfff7U;
    }
}
bool SotnTeleportEffect_Tick(SotnTeleportEffect *effect, SotnTeleportShared *shared,
                             SotnTeleportEffectInput in, Uint32 *random,
                             SotnTeleportEffectEvents *out)
{
    if (!effect || !shared || !random || !out || effect->removed ||
        !(effect->phase <= 6 || (effect->phase >= 20 && effect->phase <= 23)))
        return SDL_SetError("Teleport requires its actual live child/shared-point/RNG owners");
    if (!effect->phase && !in.allocated)
    {
        *out = (SotnTeleportEffectEvents){0};
        return true;
    }
    if (!in.allocated || !in.dra.bytes || in.dra.size < 0xe294U || effect->width < 0 ||
        effect->width > 16 || effect->height < 0 || effect->height > 272 || effect->color < 0 ||
        effect->color > 256 || effect->alpha < 0 || effect->alpha > 255)
        return SDL_SetError("Teleport requires real36-row ownership and private color/UV words");
    SotnTeleportEffect n = *effect;
    SotnTeleportShared points = *shared;
    SotnTeleportEffectEvents events = {0};
    Uint32 rng = *random;
    const Uint16 upper = n.parameter & 0xfe00U;
    bool particles = false, tiles = false;
    switch (n.phase)
    {
    case 0:
        for (Uint16 i = 0; i < 2; ++i)
        {
            n.quads[i] = (SotnPlayerEffectQuad){.priority = 509, .draw_mode = 0x39};
            n.quads[i].x[0] = (Sint16)(192U * i);
            n.quads[i].u[0] = 192;
            n.quads[i].v[0] = 240;
            n.quads[i + 2U] = (SotnPlayerEffectQuad){.priority = 504, .draw_mode = 0x31};
        }
        for (Uint16 i = 0; i < SOTN_TELEPORT_PARTICLES; ++i)
        {
            SotnPlayerEffectQuad *q = &n.quads[i + 4U];
            points.points[i].x = Word(Pixel(in.player_x) + SotnCombat_Random(&rng) % 28 - 14);
            points.points[i].y = (Sint16)(224U - (SotnCombat_Random(&rng) & 63U));
            *q = (SotnPlayerEffectQuad){
                .page = 0x1a, .palette = 0x1ba, .priority = 496, .draw_mode = 8};
            q->green[1] = (Uint8)((SotnCombat_Random(&rng) & 31U) + 1U);
        }
        n.width = 1;
        n.height = 16;
        n.color = 128;
        n.alpha = 0;
        n.phase = 1;
        if (n.parameter & 0x100U)
        {
            n.width = 16;
            n.height = 256;
            n.alpha = 255;
            n.phase = 20;
            tiles = true;
            events.sounds[events.sound_count++] = 0x8bb;
        }
        else
        {
            events.sounds[events.sound_count++] = 0x635;
            events.sounds[events.sound_count++] = 0x8ba;
        }
        break;
    case 1:
        n.height += 32;
        if (n.height > 256)
            ++n.phase;
        break;
    case 2:
        if (++n.width >= 16)
        {
            n.width = 16;
            n.timer = 128;
            ++n.phase;
        }
        break;
    case 3:
        particles = true;
        n.color = SDL_min(256, n.color + 4);
        n.timer = Add(n.timer, -1);
        if (!n.timer)
        {
            events.palette = 0x810d;
            ++n.phase;
        }
        break;
    case 4:
        events.palette = 0x810d;
        if (--n.width <= 0)
        {
            n.width = 0;
            ++n.phase;
        }
        break;
    case 5:
        events.palette = 0x810d;
        tiles = true;
        n.alpha += 4;
        if (n.alpha >= 256)
        {
            n.alpha = 255;
            n.timer = 32;
            ++n.phase;
        }
        break;
    case 6:
        events.palette = 0x810d;
        tiles = true;
        n.timer = Add(n.timer, -1);
        if (!n.timer)
        {
            n.alpha = 0;
            events.warp_mode = upper == 0 ? 6 : upper == 0x200 ? 4 : upper == 0x400 ? 5 : 0;
        }
        break;
    case 20:
        tiles = true;
        n.alpha = 255;
        n.timer = 32;
        ++n.phase;
        break;
    case 21:
        tiles = true;
        n.timer = Add(n.timer, -1);
        if (!n.timer)
            ++n.phase;
        break;
    case 22:
        tiles = true;
        n.alpha -= 4;
        if (n.alpha <= 0)
        {
            n.alpha = 0;
            ++n.phase;
        }
        break;
    case 23:
        if (--n.width < 2)
        {
            n.width = 0;
            n.timer = 4;
            ++n.phase;
            n.removed = true;
            events.warp_acknowledged = true;
            events.sounds[events.sound_count++] = 0x636;
            goto done;
        }
        break;
    }
    n.x = High(n.x, Pixel(in.player_x));
    n.y = High(n.y, Pixel(in.player_y));
    for (Uint16 i = 0; i < 2; ++i)
    {
        SotnPlayerEffectQuad *q = &n.quads[i];
        q->red[0] = q->green[0] = q->blue[0] = (Uint8)n.alpha;
        q->draw_mode = (Uint16)(tiles ? q->draw_mode & 0xfff7U : q->draw_mode | 8U);
        q = &n.quads[i + 2U];
        const Sint16 x = Pixel(in.player_x);
        q->x[1] = q->x[3] = x;
        q->x[0] = q->x[2] = Word(x + (i ? n.width : -n.width));
        if (!Gradient(q, in.dra, n.color, Pixel(in.player_y), n.height, upper != 0))
            return false;
    }
    if (particles)
    {
        for (Uint16 i = 0; i < SOTN_TELEPORT_PARTICLES; ++i)
        {
            const SotnPlayerEffectQuad *q = &n.quads[i + 4U];
            if (q->green[0] > 2 || q->blue[0] > 6)
                return SDL_SetError("Teleport lost its original particle animation bytes");
            Particle(&n.quads[i + 4U], &points, i, in.dra);
        }
    }
    else
        // The original loop omits next: it hides only this first particle row32 times.
        n.quads[4].draw_mode |= 8U;
done:
    *effect = n;
    *shared = points;
    *random = rng;
    *out = events;
    return true;
}
bool SotnTeleportEffect_Rebase(SotnTeleportEffect *n, Sint16 x, Sint16 y, bool room)
{
    if (!n || n->removed || !(n->phase <= 6 || (n->phase >= 20 && n->phase <= 23)))
        return SDL_SetError("Teleport rebase requires its actual live child");
    if (!n->phase)
        return true;
    if (!room)
    {
        n->x = Add(n->x, (Sint32)x * 65536);
        n->y = Add(n->y, (Sint32)y * 65536);
    }
    for (Uint16 i = 0; i < SOTN_TELEPORT_ROWS; ++i)
        if (n->quads[i].draw_mode & (room ? 0x100U : 2U))
            for (Uint16 v = 0; v < 4; ++v)
            {
                n->quads[i].x[v] = Word(n->quads[i].x[v] + x);
                n->quads[i].y[v] = Word(n->quads[i].y[v] + y);
            }
    return true;
}
