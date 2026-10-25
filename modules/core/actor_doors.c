// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected normal-Alucard door paths from st/e_red_door.h and e_sealed_door.h,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Art/UV/tile tables stay private.
#include "actors_internal.h"
static bool Near(const SotnActors *s, const Actor *a, Uint32 extent)
{
    return Distance(a, s->player) <= extent &&
           SDL_abs(Pixel(s->player.y) - Pixel(a->state.y)) <= 32;
}
static bool DoorTiles(SotnActors *s, const Actor *a, bool sealed)
{
    const Uint16 params = a->state.parameters & 255;
    if (params >= (sealed ? 4 : SotnStageProfile_Get(s->stage_id)->door_types))
        return SDL_SetError("Door tile layout index is invalid");
    const Sint32 x = Pixel(a->state.x), y = Pixel(a->state.y) - 24;
    if (x < 0 || y < 0 || (Uint32)x >= s->map.width * 16 || (Uint32)y + 48 >= s->map.height * 16)
        return SDL_SetError("Door tiles extend outside their room");
    const Uint32 index = (Uint32)x / 16 + (Uint32)y / 16 * s->map.width;
    return ActorTilePatch(s, index, 4, 1, s->map.width,
                          (sealed ? 0x26d0U : ActorTable(s, 0xd8c, 0xda0, 0xd78)) +
                              (size_t)params * 16 + (a->state.frame ? 0 : 8));
}
static bool Panel(SotnActors *s, Actor *a, Uint16 index, Sint32 x0, Sint32 x1, Sint32 y,
                  const Uint8 *uv, Uint16 page, Uint16 palette)
{
    ActorPrimitive *p = ActorPrimitiveGet(s, a, index);
    if (p == NULL)
        return false;
    if (x0 == x1)
    {
        p->state.active = false;
        return true;
    }
    const Sint32 left = SDL_min(x0, x1), right = SDL_max(x0, x1);
    p->state = (SotnActorPrimitive){.x = (left + right) * 32768,
                                    .y = (y + 31) * 65536,
                                    .width = (Uint16)(right - left),
                                    .height = 62,
                                    .page = page,
                                    .palette = palette,
                                    .red = 127,
                                    .green = 127,
                                    .blue = 127,
                                    .active = true,
                                    .textured = true};
    for (Uint32 i = 0; i < 4; ++i)
    {
        p->state.u[i] = uv[i ^ (x0 > x1 ? 1U : 0U)];
        p->state.v[i] = uv[i + 4];
    }
    return true;
}
bool ActorDoor(SotnActors *s, Actor *a)
{
    const bool sealed = a->state.id == 0x4a, right = (a->state.parameters & 0x100) != 0;
    const Sint32 offset = right ? -4 : 4;
    const Uint32 range = sealed || s->stage_id == 1 ? 24 : 16;
    SotnDataView uv = {0};
    if (!Slice(s->nz0, sealed ? 0x26b8 : ActorTable(s, 0x1960, 0x20ac, 0x1a38), 24, &uv))
        return false;
    if (a->state.step == 0)
    {
        if (s->stage_id == 1 && !sealed)
        {
            a->mechanism.changed = (a->state.parameters & 0x10) != 0;
            a->state.parameters &= (Uint16)~0x10;
        }
        if (!ActorInitialize(s, a, ActorTable(s, 0xc1c, 0xb00, 0xa90)) ||
            !ActorPrimitives(s, a, sealed ? 4 : 3))
            return false;
        a->state.animation_set = sealed ? 0 : 7;
        a->state.frame = 1;
        a->state.priority = a->mechanism.changed ? 0x58 : 0x60;
        a->state.facing_left = false;
        ShiftY(a, 31);
        a->state.x += offset * 65536;
        a->enemy_state.angle_delta = 0xc00;
        if (sealed)
            a->state.palette = 0x233;
        if (Near(s, a, range))
        {
            a->enemy_state.angle_delta = right ? 0x800 : 0x1000;
            a->state.frame = 0;
            a->state.step = 4;
            s->control = (SotnActorControl){.timer = 24};
            s->motion.halt_vertical = true;
            s->player.velocity_y = 0;
            if (!sealed && !ActorSound(s, 0x642))
                return false;
        }
    }
    else if (sealed && a->state.step == 1)
    {
        Uint16 collision = 0;
        if (!ActorObjectCollision(s, a, 8, 32, 5, &collision))
            return false;
        if (Near(s, a, 24) && s->player.step == SOTN_PLAYER_WALK &&
            s->player.facing_left != (Side(a, s->player) != 0) &&
            (!a->mechanism.pressed || (s->world->relics[SOTN_RELIC_JEWEL] & 2)))
        {
            if (s->world->relics[SOTN_RELIC_JEWEL] & 2)
            {
                if (!ActorSound(s, 0x642))
                    return false;
                a->state.frame = 0;
                a->state.step = 2;
                s->control = (SotnActorControl){.timer = 2};
            }
            else
            {
                SDL_Log("Magically sealed: Jewel of Open is not active");
                a->mechanism.pressed = true;
            }
        }
    }
    else
        switch (a->state.step)
        {
        case 1:
            if (s->player.facing_left != (Side(a, s->player) != 0) &&
                s->player.step == SOTN_PLAYER_WALK && Near(s, a, range))
            {
                if (!ActorSound(s, 0x642))
                    return false;
                a->state.frame = 0;
                s->control = (SotnActorControl){.timer = 2};
                a->state.step = 2;
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
            s->control = (SotnActorControl){.buttons = right ? SOTN_BUTTON_RIGHT : SOTN_BUTTON_LEFT,
                                            .timer = 3};
            break;
        case 4:
            s->control = (SotnActorControl){.buttons = right ? SOTN_BUTTON_LEFT : SOTN_BUTTON_RIGHT,
                                            .timer = 4};
            if (!Near(s, a, range))
                a->state.step = 5;
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
                if (!sealed && !ActorSound(s, 0x64f))
                    return false;
            }
            break;
        default:
            return SDL_SetError("Door step is invalid");
        }
    for (Uint16 i = 0; i < 3; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (p == NULL)
            return false;
        p->state.active = false;
    }
    const Sint32 x = Pixel(a->state.x) - offset + (right ? -1 : 1), y = Pixel(a->state.y) - 31;
    if (a->state.frame == 0 || sealed)
    {
        const Sint32 angle = a->enemy_state.angle_delta;
        const Sint32 end = x + ActorShift((Sint64)ActorShift(ActorSin(angle + 1024), 8) * 32, 4);
        Sint32 edge =
            end + (right ? 1 : -1) * ActorShift((Sint64)ActorShift(ActorSin(angle), 4) * 6, 8);
        if ((right && angle < 0x880) || (!right && angle > 0xf80))
            edge = end + (right ? -1 : 1);
        const Uint16 page = sealed ? 0x15 : 0x1f, palette = sealed ? 0x232 : 0x198;
        if (!Panel(s, a, 0, right ? edge : end, right ? end : edge, y, uv.bytes, page, palette) ||
            !Panel(s, a, right ? 2 : 1, right ? end - 1 : x, right ? x : end, y,
                   uv.bytes + (right ? 16 : 8), page, palette))
            return false;
    }
    if (sealed)
    {
        ActorPrimitive *glow = ActorPrimitiveGet(s, a, 3);
        if (glow == NULL)
            return false;
        Sint32 phase = Wrap16((Sint64)a->mechanism.count + 32);
        if (phase > 0x380)
            phase = -phase;
        a->mechanism.count = (Uint16)phase;
        a->state.palette = (Uint16)(0x233 + (SDL_abs(phase) >> 8));
        if (!ActorSprite(s, a, 3, a->state.x - 8 * 65536, a->state.y - 36 * 65536, 64, 0, 16, 72,
                         0x15, a->state.palette))
            return false;
        glow->state.active = a->state.frame != 0;
        glow->state.blend = true;
    }
    if (a->state.step != 1)
    {
        s->motion.protect = true;
        s->player.invulnerable = true;
    }
    return DoorTiles(s, a, sealed);
}
