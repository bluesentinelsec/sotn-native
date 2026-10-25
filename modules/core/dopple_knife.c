// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted from sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5,
// src/boss/bo4/unk_50D4C.c:EntitySubwpnKnife.
#include "actors_internal.h"

static Sint32 AddPosition(Sint32 value, Sint32 delta)
{
    const Uint32 bits = (Uint32)value + (Uint32)delta;
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static bool DrawKnife(SotnActors *s, Actor *a, bool bounce)
{
    ActorPrimitive *picture = ActorPrimitiveGet(s, a, 0), *trail = ActorPrimitiveGet(s, a, 1);
    if (picture == NULL || trail == NULL)
        return false;
    SotnActorPrimitive *p = &picture->state;
    p->x = a->state.x;
    p->y = a->state.y;
    p->active = p->textured = p->quad = true;
    p->page = p->blend ? 0x3c : 0x1c;
    p->palette = 0x1ab;
    p->width = 24;
    p->height = 8;
    const Sint16 extent = a->state.facing_left ? -12 : 12;
    if (!bounce)
    {
        p->offset_x[0] = p->offset_x[2] = (Sint16)-extent;
        p->offset_x[1] = p->offset_x[3] = extent;
        p->offset_y[0] = p->offset_y[1] = -4;
        p->offset_y[2] = p->offset_y[3] = 4;
    }
    else
    {
        const Sint16 turn = a->state.facing_left ? 128 : -128;
        a->state.rotation = (Sint16)((a->state.rotation + turn) & 4095);
        const Sint32 angles[4] = {a->state.facing_left ? 0xd2 : 0x800 - 0xd2,
                                  a->state.facing_left ? 0x800 - 0xd2 : 0xd2,
                                  a->state.facing_left ? -0xd2 : 0x800 + 0xd2,
                                  a->state.facing_left ? 0x800 + 0xd2 : -0xd2};
        for (size_t i = 0; i < 4; ++i)
        {
            const Sint32 angle = angles[i] + a->state.rotation;
            p->offset_x[i] = (Sint16)ActorShift((Sint64)ActorSin(angle + 1024) * 0xca0, 20);
            p->offset_y[i] = (Sint16)-ActorShift(-(Sint64)ActorSin(angle) * 0xca0, 20);
        }
    }
    p->u[0] = p->u[1] = 0x18;
    p->u[2] = p->u[3] = 0x20;
    p->v[0] = p->v[2] = 0x18;
    p->v[1] = p->v[3] = 0;
    const Sint16 line = a->state.facing_left ? -8 : 8;
    const Sint32 tail = -(Sint32)extent * (a->timer / 2);
    if (tail < SDL_MIN_SINT16 || tail > SDL_MAX_SINT16)
        return SDL_SetError("BO4 knife trail exceeds its original bounded lifetime");
    trail->state = (SotnActorPrimitive){.x = a->state.x,
                                        .y = a->state.y,
                                        .line = true,
                                        .blend = true,
                                        .active = !bounce,
                                        .red = 127,
                                        .green = 63,
                                        .line_blend_mode = 1,
                                        .offset_x = {(Sint16)-line, (Sint16)tail},
                                        .offset_y = {-1, -1}};
    return true;
}

bool ActorDoppleKnife(SotnActors *s, Actor *a)
{
    if (s == NULL || a == NULL || s->stage_id != 26 || a->state.id != 0x104)
        return SDL_SetError("BO4 knife dispatch requires its original child type");
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x428))
            return false;
        if (s->dopple.body.motion.status & 0x100U)
            a->attack /= 2;
        if (!ActorPrimitives(s, a, 2))
        {
            a->remove = true;
            return true;
        }
        a->state.flags |= 0x800000U;
        a->state.facing_left = s->dopple.body.facing_left;
        a->state.hitbox_width = 8;
        a->state.hitbox_height = 5;
        a->state.hitbox_x = 4;
        a->state.hitbox_y = 0;
        if (!(s->dopple.body.motion.status & 0x20U))
            ShiftY(a, -9);
        a->vx = a->state.facing_left ? -524288 : 524288;
        if (!ActorSound(s, 0x60c))
            return false;
        s->dopple.body.timers[10] = 4;
        return true;
    case 1: {
        if ((a->vx != 524288 && a->vx != -524288) || a->timer >= 4096)
            return SDL_SetError("BO4 flying knife has invalid velocity or lifetime");
        ++a->timer;
        const Sint16 front = a->vx > 0 ? 8 : -8;
        for (Uint32 i = 0; i < 8; ++i)
        {
            a->state.x = AddPosition(a->state.x, a->vx > 0 ? 65536 : -65536);
            SotnCollider collider = {0};
            if (!ActorQuery(s, Pixel(a->state.x) + front, Pixel(a->state.y), &collider))
                return false;
            if ((collider.effects & SOTN_EFFECT_BLOCK) || (a->state.flags & 0x100U))
            {
                a->timer = 64;
                a->vx = -ActorShift(a->vx, 3);
                a->vy = -163840;
                a->state.hitbox_state = 0;
                const SotnDoppleRequest request = {.kind = SOTN_DOPPLE_FACTORY,
                                                   .id = 10,
                                                   .x = Wrap16((Sint64)Pixel(a->state.x) + front) *
                                                        65536,
                                                   .y = a->state.y,
                                                   .facing_left = a->state.facing_left};
                if (!ActorDoppleFactoryRequest(s, a, &request) || !ActorSound(s, 0x6a4))
                    return false;
                a->state.step = 2;
                return true;
            }
        }
        if (a->hit_flags & 0x80U)
        {
            a->timer = 4;
            a->state.step = 3;
            a->state.hitbox_state = 0;
            return true;
        }
        return DrawKnife(s, a, false);
    }
    case 2: {
        if (!a->timer || a->timer > 64 || (a->vx != -65536 && a->vx != 65536) || a->vy < -163840 ||
            a->vy > 360448)
            return SDL_SetError("BO4 bouncing knife timer is invalid");
        if (--a->timer == 0)
        {
            a->remove = true;
            return true;
        }
        ActorPrimitive *picture = ActorPrimitiveGet(s, a, 0);
        if (picture == NULL)
            return false;
        if (a->timer == 32)
        {
            picture->state.modulate = picture->state.blend = true;
            picture->state.red = picture->state.green = picture->state.blue = 96;
        }
        a->state.x = AddPosition(a->state.x, a->vx);
        a->state.y = AddPosition(a->state.y, a->vy);
        a->vy += 8192;
        if (a->timer < 33)
        {
            if (picture->state.red < 2)
                return SDL_SetError("BO4 knife fade intensity underflow");
            picture->state.red = (Uint8)(picture->state.red - 2);
            picture->state.green = picture->state.blue = picture->state.red;
        }
        return DrawKnife(s, a, true);
    }
    case 3:
        if (!a->timer || a->timer > 4)
            return SDL_SetError("BO4 attached knife timer is invalid");
        if (--a->timer == 0)
            a->remove = true;
        return true;
    default:
        return SDL_SetError("BO4 knife entered unsupported state %u", a->state.step);
    }
}
