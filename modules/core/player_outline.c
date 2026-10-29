// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/7879C.c:EntityPlayerOutline and 4B758.c:func_800EB758,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Profiles/artwork stay private.
#include <SDL3/SDL_error.h>
#include <sotn/player_outline.h>

static Sint16 Word(Sint64 value)
{
    const Uint16 bits = (Uint16)(Uint64)value;
    return bits <= 32767U ? (Sint16)bits : (Sint16)((Sint32)bits - 65536);
}
static Sint16 Read16(const Uint8 *p)
{
    return Word(p[0] | (Uint16)p[1] << 8);
}
static Sint32 Scale(Sint16 distance, Sint16 scale)
{
    const Sint32 product = distance * scale;
    // The SDK's added0x80000000 vanishes in the final signed-word coordinate.
    return product >= 0 ? product / 256 : -((-product + 255) / 256);
}
static bool Geometry(SotnPlayerOutline *e, SotnPlayerOutlineInput in, const Sint16 profile[5])
{
    const Sint32 width = (Uint8)(4U + in.image.width) - 4,
                 height = (Uint8)(1U + in.image.height) - 1;
    const Sint16 left = Word(in.x +
                             (in.facing_left ? -in.image.pivot_x - width + 1 : in.image.pivot_x)),
                 top = Word(in.y + in.image.pivot_y),
                 px = Word(in.x + (in.facing_left ? -e->pivot_x : e->pivot_x)),
                 py = Word(in.y + e->pivot_y);
    SotnPlayerEffectQuad q = e->quad;
    for (Uint16 v = 0; v < 4; ++v)
    {
        q.x[v] = Word(left + ((v & 1U) ? width : 0));
        q.y[v] = Word(top + ((v & 2U) ? height : 0));
        q.u[v] = (Uint8)(in.facing_left ? ((v & 1U) ? 3 : 3 + in.image.width)
                                        : ((v & 1U) ? 4 + in.image.width : 4));
        q.v[v] = (Uint8)(1U + ((v & 2U) ? in.image.height : 0));
        q.red[v] = (Uint8)(profile[0] * e->brightness / 256);
        q.green[v] = (Uint8)(profile[1] * e->brightness / 256);
        q.blue[v] = (Uint8)(profile[2] * e->brightness / 256);
    }
    const Sint16 x0 = Word(px + Scale(Word((Sint32)q.x[0] - px), e->scale_x)),
                 x1 = Word(px + Scale(Word((Sint32)q.x[1] - px), e->scale_x)),
                 y0 = Word(py + Scale(Word((Sint32)q.y[0] - py), e->scale_y)),
                 y2 = Word(py + Scale(Word((Sint32)q.y[2] - py), e->scale_y));
    q.x[0] = q.x[2] = x0;
    q.x[1] = q.x[3] = x1;
    q.y[0] = q.y[1] = y0;
    q.y[2] = q.y[3] = y2;
    if ((e->draw_flags & 4U) &&
        !SotnPlayerEffect_Rotate(in.executable, px, py, e->rotation, in.facing_left, &q))
        return false;
    q.priority = (Uint16)(in.priority + 2U);
    e->quad = q;
    return true;
}
bool SotnPlayerOutline_Tick(SotnPlayerOutline *effect, SotnPlayerOutlineInput in,
                            SotnPlayerOutlineEvents *out)
{
    if (!effect || !out || effect->removed || effect->phase > 3 ||
        ((effect->parameter & 0x7f00U) >> 8) >= 24 || effect->brightness < 0 ||
        effect->brightness > 512 || effect->timer > 8 ||
        ((effect->phase == 1 || effect->phase == 3) && !effect->timer))
        return SDL_SetError("Player outline requires its original bounded child state");
    SotnPlayerOutline next = *effect;
    SotnPlayerOutlineEvents events = {0};
    if ((in.status & (0x1000000U | 0x40000U | 0x80U | 7U)) || !(in.frame & 0x7fffU) || !in.bank ||
        (in.step == 34 && in.palette == 0x810d))
    {
        next.removed = true;
        *effect = next;
        *out = events;
        return true;
    }
    const Uint16 parameter = (effect->parameter & 0x7f00U) >> 8;
    const size_t at = 0xd9b8U + parameter * 10U;
    if (!in.dra.bytes || at > in.dra.size || in.dra.size - at < 10 || !in.image.width ||
        in.image.width > 255 || !in.image.height || in.image.height > 255 || in.priority > 509)
        return SDL_SetError(
            "Player outline requires private profile and live normal-image geometry");
    Sint16 profile[5] = {0};
    for (Uint16 i = 0; i < 5; ++i)
        profile[i] = Read16(in.dra.bytes + at + i * 2U);
    if (profile[0] < 0 || profile[0] > 255 || profile[1] < 0 || profile[1] > 255 ||
        profile[2] < 0 || profile[2] > 255 || profile[3] < 0 || profile[3] > 511 ||
        profile[4] != 49)
        return SDL_SetError("Player outline private color/palette/draw profile is invalid");
    next.x = in.x;
    next.y = in.y;
    next.rotation = in.rotation;
    next.draw_flags |= 3U;
    const bool shrinking = (parameter >= 3 && parameter <= 7) || parameter == 14 ||
                           parameter == 15 || parameter == 16 || parameter == 19 || parameter == 21;
    switch (next.phase)
    {
    case 0:
        if (in.allocation_failed)
        {
            next.removed = true;
            break;
        }
        next.quad = (SotnPlayerEffectQuad){
            .page = 0x18, .palette = (Uint16)profile[3], .draw_mode = (Uint16)(profile[4] + 0x304)};
        next.brightness = parameter == 1 ? 256 : parameter == 2 ? 192 : 128;
        next.scale_x = Word(in.scale_x + (shrinking ? 96 : 0));
        next.scale_y = Word(in.scale_y + (shrinking ? 96 : 0));
        next.pivot_x = in.pivot_x;
        next.pivot_y = in.pivot_y;
        next.timer = 8;
        next.phase = 1;
        break;
    case 1:
        if ((parameter >= 3 && parameter <= 7) || parameter == 14 || parameter == 15 ||
            parameter == 16 || parameter == 18 || parameter == 21)
            next.brightness = Word(next.brightness + 16);
        if (!--next.timer)
            next.phase = 2;
        break;
    case 2:
        if ((parameter >= 3 && parameter <= 7) || parameter == 14 || parameter == 19 ||
            parameter == 21)
        {
            next.scale_x = Word(next.scale_x - 8);
            next.scale_y = Word(next.scale_y - 8);
            if (next.scale_x <= 256)
            {
                next.scale_x = next.scale_y = 256;
                next.timer = 8;
                next.phase = 3;
            }
        }
        else
        {
            next.scale_x = Word(next.scale_x + (parameter == 1 ? 2 : 8));
            next.scale_y = Word(next.scale_y + (parameter == 1 ? 2 : 8));
            next.brightness = Word(next.brightness - (parameter == 1 ? 16 : 5));
            if (next.brightness < 0)
                next.removed = true;
        }
        break;
    case 3:
        if (!--next.timer)
            next.removed = true;
        break;
    }
    if (!next.removed)
    {
        if (!Geometry(&next, in, profile))
            return false;
        events.reset_afterimages = true;
    }
    *effect = next;
    *out = events;
    return true;
}
bool SotnPlayerOutline_Translate(SotnPlayerOutline *e, Sint32 dx, Sint32 dy)
{
    if (!e || dx < SDL_MIN_SINT16 || dx > SDL_MAX_SINT16 || dy < SDL_MIN_SINT16 ||
        dy > SDL_MAX_SINT16)
        return SDL_SetError("Player outline rebase requires signed-word offsets and an owner");
    if (e->phase && !e->removed)
    {
        e->x = Word(e->x + dx);
        e->y = Word(e->y + dy);
        for (Uint16 i = 0; i < 4; ++i)
        {
            e->quad.x[i] = Word(e->quad.x[i] + dx);
            e->quad.y[i] = Word(e->quad.y[i] + dy);
        }
    }
    return true;
}
