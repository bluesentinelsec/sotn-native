// SPDX-License-Identifier: AGPL-3.0-or-later
// st/e_bone_musket.h at sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Original coordinated stances, animations, offsets and hitboxes remain private NO1 data.
#include "actors_internal.h"
static void Substep(Actor *a, Uint16 next)
{
    a->substep = next;
    a->pose = a->remaining = 0;
}
static Actor *Neighbor(SotnActors *s, const Actor *a)
{
    const Uint16 slot = a->musket.neighbor;
    if (slot >= SOTN_STAGE_ACTOR_SLOTS)
        return NULL;
    Actor *p = &s->slots[slot];
    return p->state.id == 0x46 && p->state.generation == a->musket.generation ? p : NULL;
}
static bool Shot(SotnActors *s, const Actor *a)
{
    Actor *shot = NULL;
    if (!ActorSpawn(s, a, 0x47, 0, 96, 128, &shot))
        return false;
    if (shot != NULL)
    {
        shot->state.x = Pixel(a->state.x) * 65536;
        shot->state.y = Pixel(a->state.y) * 65536;
    }
    return true;
}
static bool Body(SotnActors *s, Actor *a)
{
    Uint16 collision = 0;
    Uint8 animation = 0;
    if ((a->state.flags & 0x100) && a->state.step < 8)
        SetStep(a, 8);
    switch (a->state.step)
    {
    case 0:
        if ((a->state.parameters & ~0x100U) > 2)
            return SDL_SetError("Bone Musket requires an original group index");
        if (!ActorInitialize(s, a, 0xa94))
            return false;
        if (a->state.parameters & 0x100)
        {
            a->state.facing_left = true;
            a->state.parameters &= 255;
        }
        a->state.palette = (Uint16)(a->state.palette + a->state.parameters);
        a->state.frame = a->state.parameters == 1 ? 26 : 1;
        SDL_FALLTHROUGH;
    case 1:
        if (!ActorGround(s, a, 0x2998, &collision))
            return false;
        if (collision & 1)
        {
            const Uint32 index = a->state.parameters;
            if (index == 0)
            {
                if ((Uint32)a->slot + 2 >= SOTN_STAGE_ACTOR_SLOTS)
                    return SDL_SetError("Bone Musket requires its original three-slot group");
                for (Uint16 i = 1; i < 3; ++i)
                {
                    SotnDataView offset = {0};
                    Actor *p = NULL;
                    if (!Slice(s->nz0, 0x2a4c + i * 2U, 2, &offset) ||
                        SDL_abs(Signed16(offset.bytes)) > 128 ||
                        !ActorSpawnAt(s, a, 0x46, i, (Uint32)a->slot + i, &p))
                        return false;
                    p->state.x = (Pixel(a->state.x) +
                                  (a->state.facing_left ? -1 : 1) * Signed16(offset.bytes)) *
                                 65536;
                    p->state.y = Pixel(a->state.y) * 65536;
                }
            }
            SotnDataView relative = {0};
            if (index > 2 || !Slice(s->nz0, 0x2a54 + index * 4, 4, &relative))
                return false;
            const Sint32 offset = Signed32(relative.bytes);
            if (offset < -2 || offset > 2)
                return SDL_SetError("Bone Musket neighbor offset exceeds its bounded group");
            const Sint32 slot = a->slot + offset;
            if (slot < 0 || slot >= SOTN_STAGE_ACTOR_SLOTS || s->slots[slot].state.id != 0x46 ||
                !s->slots[slot].state.generation)
                return SDL_SetError("Bone Musket neighbor is outside its original group");
            a->musket.neighbor = (Uint16)slot;
            a->musket.generation = s->slots[slot].state.generation;
            SetStep(a, 2);
        }
        break;
    case 2:
        if (a->state.parameters == 0 && Distance(a, s->player) < 192)
        {
            if ((Uint32)a->slot + 2 >= SOTN_STAGE_ACTOR_SLOTS)
                return SDL_SetError("Bone Musket attack group exceeds its reserved slots");
            SotnDataView setup = {0};
            if (!Slice(s->nz0, 0x2a60, 6, &setup))
                return false;
            for (Uint32 i = 0; i < 3; ++i)
            {
                Actor *p = i == 0 ? a : &s->slots[a->slot + i];
                if (p->state.id != 0x46 || p->state.parameters != i || setup.bytes[i * 2] < 3 ||
                    setup.bytes[i * 2] > 7 || setup.bytes[i * 2 + 1] > 1)
                    return SDL_SetError("Bone Musket coordinated stance or child is invalid");
                p->state.step = setup.bytes[i * 2];
                p->substep = setup.bytes[i * 2 + 1];
            }
        }
        break;
    case 3:
        if (a->substep > 3)
            return SDL_SetError("Bone Musket firing substate is invalid");
        if (!ActorAnimate(s, a,
                          a->substep == 0   ? 0x2a38
                          : a->substep == 1 ? 0x29b0
                          : a->substep == 2 ? 0x29c8
                                            : 0x2a24,
                          &animation))
            return false;
        if (!animation)
        {
            if (a->substep == 0)
                a->musket.repeat = false;
            else if (a->substep == 2)
            {
                Actor *next = Neighbor(s, a);
                if (next != NULL)
                    next->musket.signal = true;
            }
            if (a->substep == 3)
                SetStep(a, 4);
            else
                Substep(a, (Uint16)(a->substep + 1));
        }
        if ((a->substep == 1 && a->pose == 6 && !a->remaining) ||
            (a->substep == 2 && a->pose == 4 && !a->remaining))
            return Shot(s, a);
        break;
    case 4:
    case 6: {
        const bool returning = a->state.step == 6;
        if (!a->substep)
        {
            a->vx = (returning ? 57344 : -32768) * (a->state.facing_left ? 1 : -1);
            a->musket.origin = Pixel(a->state.x);
            ++a->substep;
        }
        if (!ActorWalk(s, a, 0x29a8, &collision) || !ActorAnimate(s, a, 0x2a2c, &animation))
            return false;
        const Sint32 displacement =
            (Pixel(a->state.x) - a->musket.origin) * ((a->state.facing_left == returning) ? 1 : -1);
        const Sint32 distance = returning ? 20 : 40;
        if (displacement > distance)
        {
            const Sint32 x =
                a->musket.origin + distance * ((a->state.facing_left == returning) ? 1 : -1);
            a->state.x += (x - Pixel(a->state.x)) * 65536;
            SetStep(a, returning ? a->musket.repeat ? 3 : 7 : 5);
        }
        break;
    }
    case 5:
        if (a->substep > 2)
            return SDL_SetError("Bone Musket reload substate is invalid");
        if (!ActorAnimate(s, a,
                          a->substep == 0   ? 0x2a38
                          : a->substep == 1 ? 0x29e0
                                            : 0x2a24,
                          &animation))
            return false;
        if (!animation)
        {
            if (a->substep == 2)
            {
                SetStep(a, 6);
                a->musket.repeat = false;
            }
            else
                Substep(a, (Uint16)(a->substep + 1));
        }
        if (a->substep == 1 && a->pose == 28 && !a->remaining && !ActorSound(s, 0x6bb))
            return false;
        break;
    case 7:
        if (a->musket.signal)
        {
            a->musket.signal = false;
            SetStep(a, 6);
            a->musket.repeat = true;
        }
        break;
    case 8:
        if (!ActorSound(s, 0x680) || !ActorExplosionInPool(s, a, 2, 160, 192, 0))
            return false;
        a->state.frame = a->state.hitbox_state = 0;
        SetStep(a, 9);
        break;
    case 9: {
        Actor *next = Neighbor(s, a);
        if (next == NULL || (next->state.flags & 0x100))
        {
            a->remove = true;
            return true;
        }
        next->musket.signal = true;
        break;
    }
    default:
        return SDL_SetError("Bone Musket entered invalid state %u", a->state.step);
    }
    const Uint32 box = a->state.frame > 26 ? 2 : a->state.frame > 23 ? 1 : 0;
    SotnDataView hitbox = {0};
    if (!Slice(s->nz0, 0x2a68 + box * 8, 8, &hitbox) || Signed16(hitbox.bytes + 4) < 0 ||
        Signed16(hitbox.bytes + 4) > 64 || Signed16(hitbox.bytes + 6) < 0 ||
        Signed16(hitbox.bytes + 6) > 64)
        return SDL_SetError("Bone Musket private hitbox dimensions are invalid");
    a->state.hitbox_x = (Sint16)Signed16(hitbox.bytes);
    a->state.hitbox_y = (Sint16)Signed16(hitbox.bytes + 2);
    a->state.hitbox_width = Read16(hitbox.bytes + 4);
    a->state.hitbox_height = Read16(hitbox.bytes + 6);
    return true;
}
static bool Line(SotnActors *s, Actor *a, bool visible)
{
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    if (p == NULL)
        return false;
    p->state = (SotnActorPrimitive){
        .x = (SDL_min(a->musket.line_start, a->musket.line_end) + s->camera_x) * 65536,
        .y = (a->musket.line_y + s->camera_y) * 65536,
        .width = (Uint16)SDL_abs(a->musket.line_end - a->musket.line_start),
        .height = 1,
        .red = 104,
        .green = 104,
        .blue = 112,
        .active = visible};
    return true;
}
static bool Bullet(SotnActors *s, Actor *a)
{
    switch (a->state.step)
    {
    case 0: {
        const Sint32 screen_x = Pixel(a->state.x) - s->camera_x;
        a->state.x += (SDL_clamp(screen_x, -96, 352) - screen_x) * 65536;
        if (!ActorInitialize(s, a, 0xaa0))
            return false;
        a->state.hitbox_y = -8;
        a->state.hitbox_height = 1;
        if (!ActorPrimitives(s, a, 1))
        {
            a->remove = true;
            return true;
        }
        a->musket.line_y = Pixel(a->state.y) - s->camera_y - 9;
        a->musket.line_start = Pixel(a->state.x) - s->camera_x + (a->state.facing_left ? 32 : -32);
        if (!ActorSound(s, 0x685))
            return false;
        SDL_FALLTHROUGH;
    }
    case 1: {
        const Sint32 player_x = Pixel(s->player.x) - s->camera_x;
        a->musket.line_end = a->state.facing_left
                                 ? (a->musket.line_start < player_x ? player_x : 256)
                                 : (player_x < a->musket.line_start ? player_x : 0);
        a->state.hitbox_width = (Uint16)(SDL_abs(a->musket.line_end - a->musket.line_start) / 2);
        a->state.hitbox_x = (Sint16)(-a->state.hitbox_width - 32);
        a->state.hitbox_state = 4;
        ++a->state.step;
        if (!Line(s, a, false))
            return false;
        break;
    }
    case 2:
        ++a->state.step;
        if (a->hit_params)
            a->state.hitbox_state = 0;
        else
        {
            a->state.hitbox_state = 9;
            a->musket.line_end = a->state.facing_left ? 256 : 0;
            a->state.hitbox_width =
                (Uint16)(SDL_abs(a->musket.line_end - a->musket.line_start) / 2);
            a->state.hitbox_x = (Sint16)(-a->state.hitbox_width - 32);
        }
        if (!Line(s, a, true))
            return false;
        break;
    case 3:
        a->state.hitbox_state = 0;
        if (!Line(s, a, false))
            return false;
        break;
    default:
        return SDL_SetError("Bone Musket shot entered invalid phase");
    }
    Uint8 animation = 0;
    if (!ActorAnimate(s, a, 0x2a40, &animation))
        return false;
    if (!animation)
        a->remove = true;
    return true;
}
bool ActorMusket(SotnActors *s, Actor *a)
{
    return a->state.id == 0x47 ? Bullet(s, a) : Body(s, a);
}
