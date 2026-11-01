// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/7E4BC.c:EntityHitByIce, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Point/SDK tables remain private.
#include <sotn/combat.h>
#include <sotn/ice_hit.h>
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint32 Add(Sint32 a, Sint32 b)
{
    const Uint32 bits = (Uint32)a + (Uint32)b;
    Sint32 result;
    SDL_memcpy(&result, &bits, sizeof(result));
    return result;
}
static Sint16 Pixel(Sint32 value)
{
    return (Sint16)(value >= 0 ? value / 65536 : -((-(Sint64)value + 65535) / 65536));
}
static Uint32 Read32(const Uint8 *p)
{
    return (Uint32)p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
}
static bool Geometry(SotnPlayerEffectQuad *q, SotnIceHitInput in, Uint16 row, Sint16 x, Sint16 y,
                     Sint16 rotation)
{
    if (!in.dra.bytes || in.dra.size < 0xdcc8U + 72U * 4U)
        return SDL_SetError("Ice hit requires its private triangle pointer table");
    SotnPlayerEffectQuad geometry = *q;
    for (Uint16 v = 0; v < 3; ++v)
    {
        const Uint32 pointer = Read32(in.dra.bytes + 0xdcc8U + (row * 3U + v) * 4U);
        if ((pointer & 1U) || pointer < 0x800a0000U ||
            (size_t)(pointer - 0x800a0000U) > in.dra.size ||
            in.dra.size - (size_t)(pointer - 0x800a0000U) < 4)
            return SDL_SetError("Ice hit triangle points exceed the actual private DRA image");
        const Uint8 *point = in.dra.bytes + (pointer - 0x800a0000U);
        geometry.x[v] = Word(x + Word(point[0] | (Uint16)point[1] << 8));
        geometry.y[v] = Word(y + Word(point[2] | (Uint16)point[3] << 8));
    }
    geometry.x[3] = geometry.x[2];
    geometry.y[3] = geometry.y[2];
    if (!SotnPlayerEffect_Rotate(in.executable, x, y, rotation, false, &geometry))
        return false;
    *q = geometry;
    return true;
}
bool SotnIceHit_Tick(SotnIceHit *effect, SotnIceHitInput in, Uint32 *random)
{
    if (!effect || !random || effect->removed || effect->phase > 2 || in.priority > 509 ||
        (effect->phase && !in.allocated) ||
        (effect->phase == 2 && (effect->timer <= 0 || effect->timer > 64)))
        return SDL_SetError("Ice hit requires its actual child/player/rows/RNG owners");
    SotnIceHit n = *effect;
    Uint32 rng = *random;
    n.sound = 0;
    if (!n.phase)
    {
        n.x = in.source.x;
        n.y = in.source.y;
    }
    // The original callback overwrites only the high words, even after detachment.
    n.x = Add((Sint32)((Uint32)n.x & 65535U), (Sint32)Pixel(in.player_x) * 65536);
    n.y = Add((Sint32)((Uint32)n.y & 65535U), (Sint32)Pixel(in.player_y) * 65536);
    bool breaking = !(in.status & 0x10000U);
    if (!n.phase)
    {
        if (!in.allocated)
        {
            n.removed = true;
            goto done;
        }
        n.flags = 0x00860000U;
        for (Uint16 row = 0; row < SOTN_ICE_HIT_ROWS; ++row)
        {
            SotnPlayerEffectQuad *q = &n.quads[row];
            SDL_zero(*q);
            const Uint8 red = (Uint8)((SotnCombat_Random(&rng) & 15U) + 48U),
                        blue = (Uint8)((SotnCombat_Random(&rng) & 127U) + 128U),
                        green = (Uint8)((SotnCombat_Random(&rng) & 31U) + 48U);
            q->draw_mode = (SotnCombat_Random(&rng) & 1U) ? 0x335 : 0x315;
            q->priority = (Uint16)(in.priority + 2U);
            for (Uint16 v = 0; v < 4; ++v)
            {
                q->red[v] = red;
                q->green[v] = green;
                q->blue[v] = blue;
            }
        }
        n.airborne = in.velocity_y != 0;
        if (in.player_step == 16)
        {
            n.death = 1;
            n.death_timer = 20;
            n.airborne = 0;
        }
        n.rotation =
            in.velocity_y ? (in.facing_left ? 256 : -256) : (in.velocity_x > 0 ? 128 : 3968);
        n.sound = 0x69d;
        n.phase = 1;
    }
    else if (n.phase == 1)
    {
        if ((n.death && (n.death_timer = Word(n.death_timer - 1)) == 0) ||
            (n.airborne &&
             ((in.contacts & 12U) || (in.player_step == 10 && in.player_substep == 5))))
            breaking = true;
        if (breaking)
        {
            n.timer = 64;
            n.sound = 0x61a;
            n.phase = 2;
        }
    }
    else if (--n.timer == 0)
    {
        n.removed = true;
        goto done;
    }
    for (Uint16 row = 0; row < SOTN_ICE_HIT_ROWS; ++row)
    {
        SotnPlayerEffectQuad *q = &n.quads[row];
        if (q->u[0] > 2 || q->priority > 511 ||
            (q->draw_mode != 0x335 && q->draw_mode != 0x315 && q->draw_mode != 0x37 &&
             q->draw_mode != 0x17 && q->draw_mode != 0x3f && q->draw_mode != 0x1f))
            return SDL_SetError("Ice hit lost its retained G4 phase or draw metadata");
        if (q->u[0] < 2 && !Geometry(q, in, row, Pixel(n.x), Pixel(n.y), n.rotation))
            return false;
        if (!q->u[0] && breaking)
        {
            q->u[0] = 1;
            q->v[0] = (Uint8)((SotnCombat_Random(&rng) & 15U) + 1U);
        }
        if (q->u[0] == 1 && --q->v[0] == 0)
        {
            q->u[0] = 2;
            q->v[0] = 32;
            q->u[2] = 240;
        }
        if (q->u[0] == 2)
        {
            if (q->u[2] < 112 || q->u[2] > 208)
                q->u[2] = (Uint8)(q->u[2] + 4U);
            const Sint32 velocity = q->u[2] < 128 ? q->u[2] : (Sint32)q->u[2] - 256;
            const Sint32 dy = velocity >= 0 ? velocity / 16 : -((-velocity + 15) / 16);
            if (q->red[3] < 4)
                q->red[3] = (Uint8)(q->red[3] - 4U);
            if (q->green[3] < 4)
                q->green[3] = (Uint8)(q->green[3] - 4U);
            if (q->blue[3] < 4)
                q->blue[3] = (Uint8)(q->blue[3] - 4U);
            for (Uint16 v = 0; v < 4; ++v)
            {
                q->y[v] = Word(q->y[v] + dy);
                q->red[v] = q->red[3];
                q->green[v] = q->green[3];
                q->blue[v] = q->blue[3];
            }
            q->draw_mode = (Uint16)((q->draw_mode | 2U) & 0xfcffU);
            n.flags = (n.flags | 0x08000000U) & ~0x60000U;
            if (--q->v[0] == 0)
                q->draw_mode |= 8U;
        }
    }
done:
    *effect = n;
    *random = rng;
    return true;
}
bool SotnIceHit_Displace(SotnIceHit *n, Sint32 x, Sint32 y)
{
    if (!n || n->removed || n->phase > 2)
        return SDL_SetError("Ice displacement requires its live actual owner");
    if (n->flags & 0x40000U)
    {
        n->x = Add(n->x, x);
        n->y = Add(n->y, y);
    }
    return true;
}
bool SotnIceHit_Rebase(SotnIceHit *n, Sint16 x, Sint16 y, bool room)
{
    if (!n || n->removed || n->phase > 2)
        return SDL_SetError("Ice translation requires its live actual owner");
    if (n->phase)
    {
        if (room || (n->flags & 0x08000000U))
        {
            n->x = Add(n->x, (Sint32)x * 65536);
            n->y = Add(n->y, (Sint32)y * 65536);
        }
        for (Uint16 row = 0; row < SOTN_ICE_HIT_ROWS; ++row)
            if (n->quads[row].draw_mode & (room ? 0x100U : 2U))
                for (Uint16 v = 0; v < 4; ++v)
                {
                    n->quads[row].x[v] = Word(n->quads[row].x[v] + x);
                    n->quads[row].y[v] = Word(n->quads[row].y[v] + y);
                }
    }
    return true;
}
