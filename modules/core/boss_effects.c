// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected nz0 explosion-puff paths from st/e_explosion_puff_opaque.h,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Configuration/frames stay private.
#include "actors_internal.h"
bool ActorBossPuff(SotnActors *s, Actor *a)
{
    Uint8 animation = 0;
    if (a->state.step == 0)
    {
        const Uint16 kind = a->state.parameters & 15;
        SotnDataView config = {0};
        if (kind >= 4 || !ActorInitialize(s, a, ActorTable(s, 0xbe0, 0xac4, 0xa54)) ||
            !Slice(s->nz0, ActorTable(s, 0xfe0, 0x1c5c, 0x129c) + (size_t)kind * 12, 12, &config))
            return false;
        const Uint32 address = Read32(config.bytes + 8);
        if (address < 0x80180000 || address - 0x80180000 >= s->nz0.size)
            return SDL_SetError("Boss puff animation address is outside private stage data");
        a->state.animation_set = Read16(config.bytes);
        a->state.texture_offset = Read16(config.bytes + 2);
        a->state.palette = (Uint16)(Read16(config.bytes + 4) + (s->stage_id != 12 ? 0xd0 : 0x2e0));
        a->state.blend_mode = Read16(config.bytes + 6);
        a->animation_stream = address - 0x80180000;
        a->state.step = (Uint16)(kind + 1);
        if (a->state.parameters & 0xff00)
            a->state.priority = (Uint16)(a->state.parameters >> 8);
        if (a->state.parameters & 0xf0)
        {
            a->state.palette = 0x819f;
            a->state.blend_mode = 0x10;
            a->state.facing_left = true;
        }
        return true;
    }
    if (a->state.step == 1)
    {
        a->state.x += a->vx;
        a->state.y += a->vy;
        a->vy = s->stage_id != 12 ? 65536 : -65536;
    }
    else if (a->state.step == 2)
    {
        if (!ActorAnimate(s, a, a->animation_stream, &animation))
            return false;
        if (animation == 0)
        {
            a->remove = true;
            return true;
        }
        if (a->substep == 0)
        {
            a->state.opacity = 128;
            a->substep = 1;
        }
        else if (a->substep == 1)
        {
            if (a->pose == 5)
                ++a->substep;
        }
        else
        {
            if (a->state.opacity < 4)
                return SDL_SetError("Puff opacity expired before its animation");
            a->state.opacity -= 4;
        }
        return true;
    }
    else if (a->state.step == 3)
    {
        if (a->substep == 0)
        {
            a->state.rotated = true;
            a->state.rotation = (Sint16)(a->boss.angle & 4095);
            const Sint32 speed = a->boss.speed * 320 / 28;
            a->vx = speed * ActorSin(a->state.rotation);
            a->vy = -speed * ActorSin(a->state.rotation + 1024);
            ++a->substep;
        }
        if (a->pose > 12)
        {
            a->vx -= a->vx / 4;
            a->vy -= a->vy / 4;
        }
        a->state.x += a->vx;
        a->state.y += a->vy;
    }
    else if (a->state.step == 4)
    {
        if (a->substep == 0)
        {
            a->state.facing_left = (ActorRandom(s) & 1) != 0;
            a->state.scale_x = 192;
            a->vy = -49152;
            ++a->substep;
        }
        a->state.x += a->vx;
        a->state.y += a->vy;
    }
    else
        return SDL_SetError("Boss puff step is invalid");
    if (!ActorAnimate(s, a, a->animation_stream, &animation))
        return false;
    if (animation == 0)
        a->remove = true;
    return true;
}
