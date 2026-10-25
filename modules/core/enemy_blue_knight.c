// SPDX-License-Identifier: AGPL-3.0-or-later
// st/e_blue_axe_knight.h at sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// All animation, hitbox, velocity and death-position tables remain private.
#include "actors_internal.h"
static bool Box(SotnActors *s, Actor *a, Uint16 frame, bool weapon)
{
    SotnDataView index = {0}, box = {0};
    if (frame >= (weapon ? 56 : 48) ||
        !Slice(s->nz0, (weapon ? 0x25bc : 0x2554) + frame, 1, &index) ||
        index.bytes[0] >= (weapon ? 14 : 4) ||
        !Slice(s->nz0, (weapon ? 0x2584 : 0x2544) + index.bytes[0] * 4U, 4, &box))
        return SDL_SetError("Blue Axe Knight frame/hitbox exceeds its private table");
    a->state.hitbox_x = (Sint16)(box.bytes[0] < 128 ? box.bytes[0] : (Sint32)box.bytes[0] - 256);
    a->state.hitbox_y = (Sint16)(box.bytes[1] < 128 ? box.bytes[1] : (Sint32)box.bytes[1] - 256);
    a->state.hitbox_width = box.bytes[2];
    a->state.hitbox_height = box.bytes[3];
    if (a->state.hitbox_width > 64 || a->state.hitbox_height > 64)
        return SDL_SetError("Blue Axe Knight hitbox dimensions exceed their private bounds");
    return true;
}
static bool Weapon(SotnActors *s, Actor *a)
{
    const Uint16 slot = a->enemy_state.owner_slot;
    if (slot >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Blue Axe Knight weapon owner slot is invalid");
    const Actor *p = &s->slots[slot];
    if (p->state.id != 0x20 || p->state.generation != a->enemy_state.owner_generation)
    {
        a->remove = true;
        return true;
    }
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x9ec))
            return false;
        a->state.animation_set = a->state.frame = 0;
        a->state.hitbox_state = 1;
        a->state.hit_points = 0x7fff;
        a->state.flags = 0x0d202000;
    }
    a->state.facing_left = p->state.facing_left;
    a->state.x = Pixel(p->state.x) * 65536;
    a->state.y = Pixel(p->state.y) * 65536;
    return Box(s, a, p->state.frame, true);
}
static bool Axe(SotnActors *s, Actor *a)
{
    if (a->state.flags & 0x100)
    {
        if (!ActorExplosionInPool(s, a, 0, 160, 192, 0) || !ActorSound(s, 0x66b))
            return false;
        a->remove = true;
        return true;
    }
    if (!a->state.step)
    {
        SotnDataView vx = {0}, vy = {0};
        if (a->state.parameters > 2)
            return SDL_SetError("Blue Axe Knight projectile type exceeds its private table");
        if (!ActorInitialize(s, a, 0x9ec) ||
            !Slice(s->nz0, 0x25f4 + a->state.parameters * 4U, 4, &vx) ||
            !Slice(s->nz0, 0x2600 + a->state.parameters * 4U, 4, &vy))
            return false;
        const Sint32 speed = Signed32(vx.bytes);
        a->vy = Signed32(vy.bytes);
        if (speed < 0 || speed > 4 * 65536 || a->vy < -8 * 65536 || a->vy > 8 * 65536)
            return SDL_SetError("Blue Axe Knight projectile velocity exceeds its private bounds");
        a->vx = speed * (a->state.facing_left ? 1 : -1);
        a->state.rotated = true;
        a->blue.delay = -64;
        if (a->state.parameters == 2)
            ++a->state.step;
        return true;
    }
    a->state.rotation = (Sint16)((a->state.rotation + (a->state.parameters ? 128 : -128)) & 4095);
    if (a->state.step == 1)
    {
        if (a->blue.delay >= 0 && a->blue.delay < 32)
            a->vx += a->state.facing_left ? -8192 : 8192;
        // The original signed sixteen-bit timer wraps on long-lived projectiles.
        a->blue.delay =
            a->blue.delay == SDL_MAX_SINT16 ? SDL_MIN_SINT16 : (Sint16)(a->blue.delay + 1);
    }
    else if (a->state.step == 2)
        a->vy += 8192;
    else
        return SDL_SetError("Blue Axe Knight projectile entered an invalid state");
    a->state.x += a->vx;
    a->state.y += a->vy;
    return true;
}
static bool Walk(SotnActors *s, Actor *a)
{
    Uint16 collision = 0;
    if (!ActorWalk(s, a, 0x2450, &collision))
        return false;
    if (collision & 0x60)
    {
        a->state.x -= a->vx;
        a->vx = 0;
    }
    return true;
}
static void Resume(SotnActors *s, Actor *a)
{
    a->blue.backward = Distance(a, s->player) <= 88;
    SetStep(a, a->blue.backward ? 3 : 2);
}
static void Decide(SotnActors *s, Actor *a)
{
    const Sint64 dy = SDL_abs((Sint64)Pixel(s->player.y) - Pixel(a->state.y));
    if (a->blue.delay > 0)
        a->blue.delay = (Sint16)(a->blue.delay - (dy < 32 ? 3 : 1));
    else
    {
        Uint16 step = 7;
        if (dy < 32)
        {
            const Uint32 choice = ActorRandom(s) & 7;
            step = choice < 4 ? 4 : choice < 7 ? 5 : 6;
        }
        SetStep(a, step);
        a->blue.delay = 256;
    }
}
static bool Throw(SotnActors *s, Actor *a, Uint16 phase)
{
    Uint8 animation = 0;
    const size_t stream =
        phase == 5 ? (a->blue.armored ? 0x24b0 : 0x24f0) : (a->blue.armored ? 0x2498 : 0x2514);
    if (!ActorAnimate(s, a, stream, &animation))
        return false;
    if (!animation)
        Resume(s, a);
    else if ((animation & 0x80) && a->pose == (phase == 5 ? 6 : 7))
    {
        if (!ActorSound(s, 0x766) || !ActorSound(s, 0x6c7))
            return false;
        Actor *axe = NULL;
        if (!ActorSpawn(s, a, 0x21, phase == 5 ? 1 : phase == 7 ? 2 : 0, 96, 128, &axe))
            return false;
        if (axe != NULL)
        {
            axe->state.facing_left = a->state.facing_left;
            axe->state.x =
                (Pixel(a->state.x) + (a->state.facing_left ? 1 : -1) * (phase == 7 ? 16 : 8)) *
                65536;
            axe->state.y = (Pixel(a->state.y) + (phase == 5 ? 12 : phase == 7 ? -40 : -12)) * 65536;
        }
    }
    return true;
}
static bool Charge(SotnActors *s, Actor *a)
{
    Uint8 animation = 0;
    switch (a->substep)
    {
    case 0:
        a->timer = 32;
        a->state.frame = 4;
        ++a->substep;
        break;
    case 1:
        if (a->timer)
            --a->timer;
        else
        {
            a->timer = 64;
            a->vx = (a->state.facing_left ? 1 : -1) * 163840;
            ++a->substep;
        }
        break;
    case 2:
        if (!ActorAnimate(s, a, a->blue.armored ? 0x24c4 : 0x252c, &animation))
            return false;
        if (!a->timer || Distance(a, s->player) < 32)
        {
            if (!ActorSound(s, 0x766))
                return false;
            a->pose = a->remaining = 0;
            ++a->substep;
        }
        else
            --a->timer;
        return Walk(s, a);
    case 3:
        if (!ActorAnimate(s, a, a->blue.armored ? 0x24d0 : 0x2504, &animation))
            return false;
        if (!animation)
        {
            a->state.facing_left = Side(a, s->player) == 0;
            Resume(s, a);
        }
        else if (a->vx)
        {
            a->vx += a->vx < 0 ? 16384 : -16384;
            return Walk(s, a);
        }
        break;
    default:
        return SDL_SetError("Blue Axe Knight charge substate is invalid");
    }
    return true;
}
bool ActorBlueKnight(SotnActors *s, Actor *a)
{
    if (a->state.id == 0x21)
        return Axe(s, a);
    if (a->state.id == 0x22)
        return Weapon(s, a);
    if (a->state.hit_points < 8 && a->blue.armored)
    {
        a->state.frame = (Uint16)(a->state.frame + 23);
        a->blue.armored = false;
        Actor *effect = NULL;
        if (!ActorSpawn(s, a, 2, 2, 160, 192, &effect))
            return false;
        // The US routine allocates once, then overwrites that same actor twice.
        if (effect != NULL)
        {
            effect->state.x = (Pixel(a->state.x) + (a->state.facing_left ? 12 : -12)) * 65536;
            effect->state.y = Pixel(a->state.y) * 65536;
            effect->state.facing_left = false;
        }
    }
    if ((a->state.flags & 0x100) && a->state.step != 8)
    {
        if (!ActorSound(s, 0x767))
            return false;
        a->state.hitbox_state = 0;
        a->state.priority = (Uint16)(a->state.priority - 16);
        a->timer = 65;
        SetStep(a, 8);
        if (ActorRandom(s) & 1)
        {
            a->substep = 16;
            a->timer = 0;
        }
    }
    Uint8 animation = 0;
    Uint16 collision = 0;
    switch (a->state.step)
    {
    case 0: {
        if ((Uint32)a->slot + 1 >= SOTN_STAGE_ACTOR_SLOTS || s->slots[a->slot + 1].state.generation)
            return SDL_SetError("Blue Axe Knight requires its free adjacent weapon slot");
        if (!ActorInitialize(s, a, 0x9e0))
            return false;
        a->state.facing_left = Side(a, s->player) == 0;
        a->state.hitbox_y = 10;
        if (a->state.parameters)
            a->state.palette = (Uint16)(a->state.palette + 2);
        a->blue.armored = true;
        a->blue.delay = 512;
        Actor *weapon = NULL;
        if (!ActorSpawnAt(s, a, 0x22, 0, a->slot + 1U, &weapon))
            return false;
        break;
    }
    case 1:
        if (!ActorGround(s, a, 0x2458, &collision))
            return false;
        if (collision & 1)
        {
            a->state.facing_left = Side(a, s->player) == 0;
            SetStep(a, 2);
        }
        break;
    case 2:
    case 3: {
        const bool back = a->state.step == 3;
        if (!a->substep)
        {
            a->vx = (a->state.facing_left ? 1 : -1) * (back ? -24576 : 20480);
            ++a->substep;
        }
        if (!ActorAnimate(s, a, a->blue.armored ? 0x2488 : 0x24e0, &animation))
            return false;
        if (!animation)
        {
            a->state.facing_left = Side(a, s->player) == 0;
            a->vx = (a->state.facing_left ? 1 : -1) * (back ? -24576 : 20480);
            if ((!back && Distance(a, s->player) < 96) || (back && Distance(a, s->player) > 80))
            {
                SetStep(a, back ? 2 : 3);
                a->blue.backward = !back;
            }
        }
        Sint32 delta = (a->pose == 1 || a->pose == 4) ? 1536 : -1536;
        delta *= (a->state.facing_left ? 1 : -1) * (back ? -1 : 1);
        a->vx += delta;
        if (!Walk(s, a))
            return false;
        Decide(s, a);
        break;
    }
    case 4:
    case 5:
    case 7:
        if (!Throw(s, a, a->state.step))
            return false;
        break;
    case 6:
        if (!Charge(s, a))
            return false;
        break;
    case 8:
        if (a->timer)
        {
            const Uint16 tick = --a->timer;
            if ((tick & 7) == 0)
            {
                SotnDataView offset = {0};
                // The first US tick reads entry eight in adjacent private animation bytes.
                if (tick / 8 > 8 || !Slice(s->nz0, 0x2468 + (tick / 8) * 4U, 4, &offset))
                    return SDL_SetError("Blue Axe Knight death offset exceeds its private table");
                Actor origin = *a;
                origin.state.x = (Pixel(a->state.x) + Signed16(offset.bytes)) * 65536;
                origin.state.y = (Pixel(a->state.y) + Signed16(offset.bytes + 2)) * 65536;
                if (!ActorExplosionInPool(s, &origin, 2, 160, 192, 0))
                    return false;
            }
        }
        if (!ActorAnimate(s, a, 0x2538, &animation))
            return false;
        if (!animation)
        {
            bool done = false;
            if (!ActorBlueDeath(s, a, &done))
                return false;
            if (done)
            {
                a->remove = true;
                return true;
            }
            if (a->substep < 16)
                a->state.frame = 0;
        }
        break;
    default:
        return SDL_SetError("Blue Axe Knight entered an invalid original state");
    }
    return Box(s, a, a->state.frame, false);
}
