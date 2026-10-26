// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected NO2 EntitySealedDoor from st/e_sealed_door_no2.h, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. UV/tile/SDK tables remain private.
#include "actors_internal.h"

static bool NearDoor(const SotnActors *s, const Actor *a)
{
    const Sint64 dx = (Sint64)Pixel(s->player.x) - Pixel(a->state.x),
                 dy = (Sint64)Pixel(s->player.y) - Pixel(a->state.y);
    return dx >= -24 && dx <= 24 && dy >= -32 && dy <= 32;
}
static bool TileColumn(SotnActors *s, const Actor *a)
{
    const Sint32 x = Pixel(a->state.x), y = Pixel(a->state.y) - 24;
    if (x < 0 || y < 0 || (Uint32)x >= (Uint64)s->map.width * 16 ||
        (Uint32)y + 48 >= (Uint64)s->map.height * 16)
        return SDL_SetError("NO2 stone door tile column extends outside its room");
    const Uint32 index = (Uint32)x / 16 + (Uint32)y / 16 * s->map.width;
    return ActorTilePatch(s, index, 4, 1, s->map.width,
                          0xfac + (a->state.parameters & 255U) * 16U + (a->state.frame ? 0 : 8));
}
static bool DrawDoor(SotnActors *s, Actor *a)
{
    const bool right = (a->state.parameters & 0x100) != 0,
               alternate = (a->state.parameters & 0x1000) != 0;
    const Sint32 angle = a->enemy_state.angle_delta;
    SotnMathTrig trig = {0};
    SotnDataView uv = {0};
    if (!SotnMath_Trig(s->executable, angle, &trig) ||
        !Slice(s->nz0, 0xf7c + (alternate ? 24 : 0), 24, &uv))
        return false;
    const Sint32 hinge = right ? 4 : -4;
    const Sint32 x = Pixel(a->state.x) + hinge + (right ? -1 : 1);
    const Sint32 end = x + ActorShift((Sint64)ActorShift(trig.cosine, 8) * 32, 4);
    Sint32 edge = end + (right ? 1 : -1) * ActorShift((Sint64)ActorShift(trig.sine, 4) * 6, 8);
    if ((right && angle < 0x880) || (!right && angle > 0xf80))
        edge = end + (right ? -1 : 1);
    for (Uint16 i = 0; i < 3; ++i)
    {
        ActorPrimitive *owned = ActorPrimitiveGet(s, a, i);
        if (owned == NULL)
            return false;
        SotnActorPrimitive *p = &owned->state;
        p->active = (i == 0 || i == (right ? 2 : 1)) && (!a->state.frame || (alternate && i == 0));
        if (!p->active)
            continue;
        Sint32 left = 0, right_x = 0;
        Uint8 left_color = 127, right_color = 127;
        const Uint8 shade = (Uint8)((angle & 0x3ff) >> 4);
        if (i == 0)
        {
            left = right ? edge : end;
            right_x = right ? end : edge;
            const bool narrow = right ? angle < 0xa00 : angle > 0xe00;
            if (!right || angle != 0xa00)
            {
                p->u[0] = p->u[2] = (Uint8)(alternate ? (narrow ? 4 : 3) : (narrow ? 0xb2 : 0xb1));
                p->u[1] = p->u[3] =
                    (Uint8)(alternate ? (narrow ? 12 : 13) : (narrow ? 0xb6 : 0xb7));
            }
            if (right)
                left_color = angle == 0x800 ? 127 : (Uint8)(63 + shade);
            else
                right_color = angle == 0x1000 ? 63 : (Uint8)(127 - shade);
        }
        else if (right)
        {
            left = end - 1;
            right_x = x;
            right_color = angle == 0x800 ? 63 : (Uint8)(63 - shade);
        }
        else
        {
            left = x;
            right_x = end;
            left_color = angle == 0x1000 ? 63 : shade;
        }
        const Sint32 origin = Pixel(a->state.x);
        if (left - origin < SDL_MIN_SINT16 || left - origin > SDL_MAX_SINT16 ||
            right_x - origin < SDL_MIN_SINT16 || right_x - origin > SDL_MAX_SINT16)
            return SDL_SetError("NO2 stone door projection exceeds its owned primitive");
        p->x = origin * 65536;
        p->y = Pixel(a->state.y) * 65536;
        p->width = (Uint16)SDL_abs(right_x - left);
        p->height = 62;
        p->quad = p->textured = p->modulate = p->gouraud = p->priority_set = true;
        p->priority = a->state.priority;
        p->page = alternate ? 0x12 : 0x1f;
        p->palette = alternate ? 0x21b : 0x198;
        for (Uint16 vertex = 0; vertex < 4; ++vertex)
        {
            p->offset_x[vertex] = (Sint16)(((vertex & 1) ? right_x : left) - origin);
            p->offset_y[vertex] = (vertex & 2) ? 31 : -31;
            p->v[vertex] = uv.bytes[i * 8U + 4 + vertex];
            if (i != 0)
                p->u[vertex] = uv.bytes[i * 8U + vertex];
            const Uint8 color = (vertex & 1) ? right_color : left_color;
            p->vertex_red[vertex] = p->vertex_green[vertex] = p->vertex_blue[vertex] = color;
        }
    }
    return true;
}
bool ActorNo2Door(SotnActors *s, Actor *a)
{
    if (s == NULL || a == NULL || s->stage_id != 4 || a->state.id != 0x33 ||
        (a->state.parameters & 255U) >= 4 || a->state.step > 5)
        return SDL_SetError("NO2 stone door requires its stage, identity, tile layout and state");
    const bool right = (a->state.parameters & 0x100) != 0;
    const Sint8 pan = right ? 6 : -6;
    switch (a->state.step)
    {
    case 0:
        if (Pixel(a->state.y) > 32736 || Pixel(a->state.x) < -32764 || Pixel(a->state.x) > 32763)
            return SDL_SetError("NO2 stone door origin exceeds original coordinate bounds");
        if (!ActorInitialize(s, a, 0x874))
            return false;
        if (!ActorPrimitives(s, a, 3))
        {
            a->remove = true;
            return true;
        }
        a->state.animation_set = (a->state.parameters & 0x1000) ? 0 : 7;
        a->state.frame = 1;
        a->state.priority = (Uint16)(s->player.priority - 0x20U);
        a->state.facing_left = false;
        ShiftY(a, 31);
        a->state.x += (right ? -4 : 4) * 65536;
        a->enemy_state.angle_delta = 0xc00;
        // Retain the edge UVs when the right-hand angle equals exactly 0xA00.
        for (Uint16 i = 0; i < 3; ++i)
        {
            SotnDataView uv = {0};
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (p == NULL ||
                !Slice(s->nz0, 0xf7c + (a->state.parameters & 0x1000 ? 24 : 0) + i * 8U, 8, &uv))
                return false;
            SDL_memcpy(p->state.u, uv.bytes, 4);
        }
        if (NearDoor(s, a))
        {
            if (!ActorSoundVolumePan(s, 0x642, 96, pan))
                return false;
            a->enemy_state.angle_delta = right ? 0x800 : 0x1000;
            a->state.frame = 0;
            a->state.step = 4;
            s->motion.halt_vertical = true;
            s->player.velocity_y = 0;
            s->control = (SotnActorControl){.timer = 24};
        }
        break;
    case 1:
        if (s->player.step == SOTN_PLAYER_WALK && NearDoor(s, a) &&
            s->player.facing_left != (Side(a, s->player) != 0))
        {
            if (!ActorSoundVolumePan(s, 0x642, 96, pan) || !ActorSound(s, 0x642))
                return false;
            a->state.frame = 0;
            a->state.step = 2;
            s->control = (SotnActorControl){.timer = 2};
        }
        break;
    case 2:
        s->control = (SotnActorControl){.timer = 24};
        a->enemy_state.angle_delta = (Sint16)(a->enemy_state.angle_delta + (right ? -32 : 32));
        if ((right && a->enemy_state.angle_delta <= 0x800) ||
            (!right && a->enemy_state.angle_delta >= 0x1000))
        {
            a->enemy_state.angle_delta = right ? 0x800 : 0x1000;
            a->state.step = 3;
        }
        break;
    case 3:
        if (s->control.timer >= 4)
            return true;
        s->control =
            (SotnActorControl){.buttons = right ? SOTN_BUTTON_RIGHT : SOTN_BUTTON_LEFT, .timer = 3};
        break;
    case 4:
        s->control =
            (SotnActorControl){.buttons = right ? SOTN_BUTTON_LEFT : SOTN_BUTTON_RIGHT, .timer = 4};
        if (!NearDoor(s, a))
        {
            if (!ActorSound(s, 0x642))
                return false;
            a->state.step = 5;
        }
        break;
    case 5:
        s->control = (SotnActorControl){.timer = 4};
        a->enemy_state.angle_delta = (Sint16)(a->enemy_state.angle_delta + (right ? 32 : -32));
        if ((right && a->enemy_state.angle_delta >= 0xc00) ||
            (!right && a->enemy_state.angle_delta <= 0xc00))
        {
            a->enemy_state.angle_delta = 0xc00;
            a->state.frame = 1;
            a->state.step = 1;
            if (!ActorSoundVolumePan(s, 0x64f, 96, pan))
                return false;
        }
        break;
    default:
        return SDL_SetError("NO2 stone door state is invalid");
    }
    if (a->state.step != 1)
    {
        s->motion.protect = true;
        s->player.invulnerable = true;
    }
    return DrawDoor(s, a) && TileColumn(s, a);
}
