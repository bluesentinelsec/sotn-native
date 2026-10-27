// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/7879C.c:EntityWingSmashTrail and 4B758.c:func_800EB758/RenderEntities,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Artwork/SDK tables stay private.
#include <SDL3/SDL.h>
#include <sotn/bat_trail.h>

static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint32 Scale(Sint16 value, Uint16 scale)
{
    const Sint32 product = value * scale;
    // The SDK adds 0x80000000 before shifting; that term vanishes in the signed-word result.
    return product >= 0 ? product / 256 : -((-product + 255) / 256);
}
bool SotnBatTrail_Tick(SotnBatTrail *trail, SotnBatTrailInput in)
{
    if (!trail || trail->removed ||
        (in.wing_smashing &&
         (in.frame >= 512 || in.bank != 13 || in.priority < 2 || in.priority > 511)) ||
        trail->scale > 256 || trail->opacity > 128)
        return SDL_SetError("Bat trail requires an actual bounded child and current Bat image");
    SotnBatTrail next = *trail;
    if (!in.wing_smashing)
        next.removed = true;
    else if (!next.initialized)
    {
        next.initialized = true;
        next.bank = in.bank;
        next.frame = in.frame | 0x8000U;
        next.priority = (Uint16)(in.priority - 2);
        next.rotation = in.rotation;
        next.facing_left = in.facing_left;
        next.rotated = in.rotated;
        next.opacity = 128;
        next.scale = 256;
    }
    else
    {
        if (next.scale < 8)
            return SDL_SetError("Bat trail scale underflow precedes its original removal");
        next.scale = (Uint16)(next.scale - 8);
        next.frame = in.frame | 0x8000U;
        if (next.opacity >= 5)
            next.opacity = (Uint8)(next.opacity - 5);
        else
            next.removed = true;
    }
    *trail = next;
    return true;
}
bool SotnBatTrail_Quad(const SotnBatTrail *trail, SotnPlayerImage image, SotnDataView executable,
                       Sint16 x, Sint16 y, SotnPlayerEffectQuad *out)
{
    if (!trail || !out || !trail->initialized || trail->removed || trail->scale > 256 ||
        trail->opacity > 128 || trail->bank != 13 || image.width < 1 || image.width > 252 ||
        image.height < 1 || image.height > 254 || image.pivot_x < -512 || image.pivot_x > 512 ||
        image.pivot_y < -512 || image.pivot_y > 512 || trail->priority > 509)
        return SDL_SetError("Bat trail packet requires its live child and actual bounded image");
    const Sint32 left =
        trail->facing_left ? x - image.pivot_x - image.width + 1 : x + image.pivot_x;
    SotnPlayerEffectQuad q = {
        .page = 0x18, .palette = 0x102, .priority = trail->priority, .draw_mode = 0x25};
    for (Uint16 v = 0; v < 4; ++v)
    {
        q.x[v] = Word(left + ((v & 1U) ? image.width : 0));
        q.y[v] = Word(y + image.pivot_y + ((v & 2U) ? image.height : 0));
        q.u[v] = (Uint8)(trail->facing_left ? ((v & 1U) ? 3 : 3 + image.width)
                                            : ((v & 1U) ? 4 + image.width : 4));
        q.v[v] = (Uint8)(1 + ((v & 2U) ? image.height : 0));
        q.red[v] = q.green[v] = q.blue[v] = trail->opacity;
    }
    const Sint16 x0 = Word(x + Scale(Word((Sint32)q.x[0] - x), trail->scale)),
                 x1 = Word(x + Scale(Word((Sint32)q.x[1] - x), trail->scale)),
                 y0 = Word(y + Scale(Word((Sint32)q.y[0] - y), trail->scale)),
                 y2 = Word(y + Scale(Word((Sint32)q.y[2] - y), trail->scale));
    q.x[0] = q.x[2] = x0;
    q.x[1] = q.x[3] = x1;
    q.y[0] = q.y[1] = y0;
    q.y[2] = q.y[3] = y2;
    if (trail->rotated &&
        !SotnPlayerEffect_Rotate(executable, x, y, trail->rotation, trail->facing_left, &q))
        return false;
    *out = q;
    return true;
}
