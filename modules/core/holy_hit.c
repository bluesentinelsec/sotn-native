// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/7879C.c:func_80119F70/func_80119E78, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. UV metadata stays private.
#include <sotn/combat.h>
#include <sotn/holy_hit.h>
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint32 Add(Sint32 a, Sint32 b)
{
    const Uint32 bits = (Uint32)a + (Uint32)b;
    Sint32 value;
    SDL_memcpy(&value, &bits, sizeof(value));
    return value;
}
static Sint16 Pixel(Sint32 value)
{
    return (Sint16)(value >= 0 ? value / 65536 : -((-(Sint64)value + 65535) / 65536));
}
bool SotnHolyHit_Tick(SotnHolyHit *effect, SotnHolyHitShared *shared, SotnHolyHitInput in,
                      Uint32 *random)
{
    if (!effect || !shared || !random || effect->removed || effect->phase > 1 || in.priority > 507)
        return SDL_SetError("Holy hit requires its actual child/global points/player/RNG owners");
    SotnHolyHit n = *effect;
    SotnHolyHitShared points = *shared;
    Uint32 rng = *random;
    if (!n.phase && !in.allocated)
    {
        n.removed = true;
        *effect = n;
        return true;
    }
    if (!in.allocated || !in.dra.bytes || in.dra.size < 0xd094 + 6 * 8)
        return SDL_SetError("Holy hit requires its sixteen real rows and private UV metadata");
    for (Uint16 i = 0; i < SOTN_HOLY_HIT_ROWS; ++i)
        if (n.animation[i].phase > 2 || n.animation[i].frame > 6)
            return SDL_SetError("Holy hit lost its original private animation cursor");
    if (!n.phase)
    {
        n.x = in.source.x;
        n.y = in.source.y;
        const Sint16 x = Word((Sint32)Pixel(in.player_x) + in.hitbox_x),
                     y = Word((Sint32)Pixel(in.player_y) + in.hitbox_y);
        for (Uint16 i = 0; i < SOTN_HOLY_HIT_ROWS; ++i)
        {
            points.points[i].x = Word(x + SotnCombat_Random(&rng) % 24 - 12);
            points.points[i].y = Word(y + SotnCombat_Random(&rng) % 48 - 24);
            SDL_zero(n.animation[i]);
            n.animation[i].delay = (Uint8)((SotnCombat_Random(&rng) & 7U) + 1U);
            n.quads[i] = (SotnPlayerEffectQuad){
                .page = 0x1a,
                .palette = 0x1b2,
                .priority = (Uint16)(in.priority + 4),
                .draw_mode = (Uint16)(0x11bU | (SotnCombat_Random(&rng) & 1U ? 0x60U : 0))};
        }
        n.phase = 1;
    }
    else
    {
        n.x = Add(n.x, in.entity_shift_x);
        n.y = Add(n.y, in.entity_shift_y);
        if (!(in.status & 0x10000U))
        {
            n.removed = true;
            goto done;
        }
    }
    for (Uint16 i = 0; i < SOTN_HOLY_HIT_ROWS; ++i)
    {
        if (!n.animation[i].phase)
        {
            if (!--n.animation[i].delay)
                ++n.animation[i].phase;
        }
        else if (n.animation[i].phase == 1)
        {
            SotnPlayerEffectQuad *q = &n.quads[i];
            const Uint8 frame = n.animation[i].frame;
            const Sint16 x = points.points[i].x, y = points.points[i].y;
            points.points[i].y = Word((Sint32)y - 1);
            if (frame == 6)
            {
                q->draw_mode |= 8U;
                ++n.animation[i].phase;
                continue;
            }
            const Sint16 half = frame >= 3 ? 4 : 6;
            const Uint8 *uv = in.dra.bytes + 0xd094 + frame * 8U;
            for (Uint16 v = 0; v < 4; ++v)
            {
                q->x[v] = Word(x + (v & 1U ? half : -half));
                q->y[v] = Word(y + (v & 2U ? half : -half));
                q->u[v] = uv[v * 2U];
                q->v[v] = uv[v * 2U + 1U];
            }
            ++n.animation[i].age;
            if (!(n.animation[i].age & 1U))
                ++n.animation[i].frame;
            q->draw_mode = (Uint16)(q->draw_mode & 0xfff7U);
        }
    }
done:
    *effect = n;
    *shared = points;
    *random = rng;
    return true;
}
bool SotnHolyHit_Rebase(SotnHolyHit *n, Sint16 x, Sint16 y, bool room)
{
    if (!n || n->removed || n->phase > 1)
        return SDL_SetError("Holy hit rebase requires its actual live child");
    if (!n->phase)
        return true;
    for (Uint16 i = 0; i < SOTN_HOLY_HIT_ROWS; ++i)
        if (n->quads[i].draw_mode & (room ? 0x100U : 2U))
            for (Uint16 v = 0; v < 4; ++v)
            {
                n->quads[i].x[v] = Word((Sint32)n->quads[i].x[v] + x);
                n->quads[i].y[v] = Word((Sint32)n->quads[i].y[v] + y);
            }
    return SotnHolyHit_Displace(n, (Sint32)x * 65536, (Sint32)y * 65536);
}
bool SotnHolyHit_Displace(SotnHolyHit *n, Sint32 x, Sint32 y)
{
    if (!n || n->removed || n->phase > 1)
        return SDL_SetError("Holy hit displacement requires its actual live child");
    if (n->phase)
    {
        n->x = Add(n->x, x);
        n->y = Add(n->y, y);
    }
    return true;
}
