// SPDX-License-Identifier: AGPL-3.0-or-later
// Reconstructed US BO0 func_801AF31C and shared no2_unk_372E8.h at
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. All tables stay private.
#include "olrox_internal.h"

static void Whole(Sint32 *position, Sint32 pixel)
{
    *position = (Sint32)((Sint64)Wrap16(pixel) * 65536 + ((Uint32)*position & 65535U));
}
static bool Move(Actor *a)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Olrox fire/effect movement exceeds fixed-point coordinates");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    return true;
}
static bool Fireball(SotnActors *s, Actor *a)
{
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x6fc))
            return false;
        Whole(&a->state.x, Pixel(a->state.x) + (a->state.facing_left ? 28 : -28));
        Whole(&a->state.y, Pixel(a->state.y) - 12);
        if (!ActorPrimitives(s, a, 1))
        {
            SDL_ClearError();
            a->remove = true;
            return true;
        }
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
        if (!p)
            return false;
        a->state.flags |= 0x800000U;
        p->state = (SotnActorPrimitive){.active = true,
                                        .textured = true,
                                        .quad = true,
                                        .blend = true,
                                        .modulate = true,
                                        .gouraud = true,
                                        .page = 0x32,
                                        .palette = 0x212,
                                        .priority_set = true,
                                        .priority = (Uint16)(a->state.priority + 4U),
                                        .x = Pixel(a->state.x) * 65536,
                                        .y = Pixel(a->state.y) * 65536,
                                        .u = {0xa0, 0xd0, 0xa0, 0xd0},
                                        .v = {0, 0, 16, 16},
                                        .vertex_red = {0, 160, 0, 160},
                                        .vertex_green = {0, 160, 0, 160},
                                        .vertex_blue = {0, 160, 0, 160}};
        a->vx = a->state.facing_left ? -32768 : 32768;
        a->vy = 0;
        a->olrox.fire_length = (a->state.facing_left ? -48 : 48) * 65536;
        a->state.hitbox_width = 0;
        a->state.hitbox_height = 5;
        if (!ActorPositionalSound(s, a, 0x7d1))
            return false;
    }
    else if (a->state.step == 1)
    {
        if (!Move(a))
            return false;
        a->vx = (Sint32)((Uint32)a->vx + (a->state.facing_left ? 0xffffe000U : 8192U));
        a->olrox.fire_length =
            (Sint32)((Uint32)a->olrox.fire_length + (a->state.facing_left ? 0xfffb0000U : 327680U));
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
        if (!p)
            return false;
        const Sint16 length = (Sint16)Pixel(a->olrox.fire_length);
        p->state.x = Pixel(a->state.x) * 65536;
        p->state.y = Pixel(a->state.y) * 65536;
        p->state.offset_x[1] = p->state.offset_x[3] = length;
        p->state.offset_y[0] = p->state.offset_y[1] = -8;
        p->state.offset_y[2] = p->state.offset_y[3] = 8;
        const Sint32 size = SDL_abs((Sint32)length);
        a->state.hitbox_width = (Uint8)(size / 4);
        a->state.hitbox_x = Wrap16(size - a->state.hitbox_width - 4);
    }
    else
        return SDL_SetError("Olrox fireball entered an invalid phase");
    if (SDL_abs(Pixel(a->state.x) - s->camera_x) >= 641)
        a->remove = true;
    return true;
}
static bool Retirement(SotnActors *s, Actor *a)
{
    Uint8 animated = 0;
    if (!a->state.step)
    {
        const Uint16 kind = a->state.parameters & 15U;
        SotnDataView config = {0};
        if (kind > 2 || !Slice(s->nz0, 0x1350 + kind * 12U, 12, &config))
            return SDL_SetError("Olrox retirement effect requires an original configured variant");
        const Uint32 animation = Read32(config.bytes + 8);
        if (animation < 0x80180000 || animation - 0x80180000 >= s->nz0.size || (animation & 1U))
            return SDL_SetError("Olrox retirement animation address exceeds private stage data");
        if (!ActorInitialize(s, a, 0x60c))
            return false;
        a->state.animation_set = Read16(config.bytes);
        a->state.texture_offset = Read16(config.bytes + 2);
        a->state.palette = (Uint16)(Read16(config.bytes + 4) + 0x226U);
        a->state.blend_mode = (Uint8)Read16(config.bytes + 6);
        a->animation_stream = animation - 0x80180000;
        a->state.step = (Uint16)(kind + 1U);
        if (a->state.parameters & 0xff00U)
            a->state.priority = a->state.parameters >> 8;
        if (a->state.parameters & 0xf0U)
        {
            a->state.palette = 0x819f;
            a->state.blend_mode = 0x10;
            a->state.facing_left = true;
        }
        return true;
    }
    if (a->state.step > 3 || !Move(a))
        return SDL_SetError("Olrox retirement effect has invalid phase or movement");
    if (a->state.step < 3)
        a->vy = -65536;
    if (a->state.step == 2 && a->substep == 1)
    {
        a->state.scale_x = a->state.scale_y = (Uint16)(a->state.scale_y + 4U);
        a->state.opacity = (Uint8)(a->state.opacity - 16U);
        a->remove = a->state.opacity == 0;
        return true;
    }
    if (!ActorAnimate(s, a, a->animation_stream, &animated))
        return false;
    if (!animated)
    {
        if (a->state.step == 2 && a->substep == 0)
        {
            a->state.transform_flags_set = true;
            a->state.transform_flags |=
                SOTN_ACTOR_SCALE_X | SOTN_ACTOR_SCALE_Y | SOTN_ACTOR_OPACITY;
            a->state.scale_set = a->state.opacity_set = true;
            a->state.scale_x = a->state.scale_y = 256;
            a->state.opacity = 128;
            ++a->substep;
        }
        else
            a->remove = true;
        return true;
    }
    if (a->state.step == 3)
    {
        switch (a->substep)
        {
        case 0:
            a->state.transform_flags_set = true;
            a->state.transform_flags = SOTN_ACTOR_OPACITY;
            a->state.opacity_set = true;
            a->state.opacity = 128;
            ++a->substep;
            break;
        case 1:
            if (a->pose == 5)
                ++a->substep;
            break;
        case 2:
            a->state.opacity = (Uint8)(a->state.opacity - 4U);
            break;
        default:
            return SDL_SetError("Olrox retirement effect entered an invalid fade phase");
        }
    }
    return true;
}
bool ActorOlroxFire(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 30 || !a->state.generation)
        return SDL_SetError("Olrox fire/effects require an active BO0 owner");
    if (a->state.id == 0x30)
        return Fireball(s, a);
    if (a->state.id == 0x47)
        return Retirement(s, a);
    return SDL_SetError("Olrox fire/effect adapter received an invalid identity");
}
