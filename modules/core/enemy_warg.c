// SPDX-License-Identifier: AGPL-3.0-or-later
// Warg behavior from st/no3/e_warg.c at sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. No original tables are embedded.
#include "actors_internal.h"

static bool Attack(SotnActors *s, Actor *a)
{
    a->vx = 0;
    a->warg.attack_timer = 256;
    SetStep(a, 6);
    a->warg.timer = 32;
    return ActorSound(s, 0x783);
}

static bool Decide(SotnActors *s, Actor *a)
{
    if (a->state.facing_left == (Side(a, s->player) != 0))
    {
        SetStep(a, 5);
        return true;
    }
    if (a->warg.attack_timer == 0)
        return Attack(s, a);
    const Sint32 distance = a->warg.anchor_x - Pixel(a->state.x);
    if (distance > 16 || distance < -16)
    {
        SetStep(a, 3);
        a->warg.backward = distance > 16 ? !a->state.facing_left : a->state.facing_left;
    }
    else
        SetStep(a, 7);
    a->warg.timer = 0;
    a->warg.decision = 32;
    return true;
}

static void AttackTimer(SotnActors *s, Actor *a)
{
    if (a->warg.attack_timer > 0)
        a->warg.attack_timer =
            (Sint16)SDL_max(0, a->warg.attack_timer - (Distance(a, s->player) < 96 ? 2 : 1));
}

static bool Hitbox(SotnActors *s, Actor *a, const Actor *body, bool head)
{
    Uint16 frame = body->state.frame;
    if (frame == 0)
        return true;
    if (body->state.parameters != 0)
        frame = frame >= 57 ? (Uint16)(frame - 57) : 13;
    else
        frame = (Uint16)(frame >= 96 ? frame - 48 : frame - 1);
    SotnDataView box = {0};
    if (frame >= 61 || !Slice(s->nz0, 0x32ec + (size_t)frame * 16 + (head ? 8 : 0), 8, &box))
        return SDL_SetError("Warg frame exceeds its original hitbox table");
    a->state.hitbox_x = (Sint16)Signed16(box.bytes);
    a->state.hitbox_y = (Sint16)Signed16(box.bytes + 2);
    a->state.hitbox_width = Read16(box.bytes + 4);
    a->state.hitbox_height = Read16(box.bytes + 6);
    return true;
}

static bool Head(SotnActors *s, Actor *a)
{
    const Uint32 owner = a->enemy_state.owner_slot;
    if (owner >= SOTN_STAGE_ACTOR_SLOTS ||
        s->slots[owner].state.generation != a->enemy_state.owner_generation)
    {
        a->remove = true;
        return true;
    }
    const Actor *body = &s->slots[owner];
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, 0xb9c))
            return false;
        a->state.frame = 0;
        a->state.hit_points = 0x7fff;
    }
    a->state.x = body->state.x;
    a->state.y = body->state.y;
    a->state.facing_left = body->state.facing_left;
    if (body->state.flags & 0x100)
        a->state.hitbox_state = 0;
    return Hitbox(s, a, body, true);
}

static bool Death(SotnActors *s, Actor *a)
{
    Uint8 animation = 0;
    Uint16 collision = 0;
    if (++a->warg.timer & 1)
    {
        Actor *puff = NULL;
        if (!ActorSpawn(s, a, 0x61, 1, 160, 192, &puff))
            return false;
        if (puff != NULL)
        {
            puff->state.priority = (Uint16)(0x88 - (ActorRandom(s) & 7) - 8);
            const Sint32 dx = (Sint32)(ActorRandom(s) & 15) - (a->state.facing_left ? 8 : 7);
            puff->state.x += dx * 8 * 65536;
            puff->state.y += ((Sint32)(ActorRandom(s) & 7) - 2) * 8 * 65536;
        }
    }
    if (a->substep == 0)
    {
        if (!ActorGround(s, a, 0x327c, &collision))
            return false;
        if (collision & 1)
        {
            a->state.opacity = 128;
            a->substep = 1;
        }
    }
    else
    {
        if (!ActorAnimate(s, a, 0x31a0, &animation))
            return false;
        if (a->state.opacity < 2)
            return SDL_SetError("Warg death opacity underflow");
        a->state.opacity -= 2;
        if (a->substep == 1 && a->state.opacity == 64 && !ActorSound(s, 0x780))
            return false;
        if (a->state.opacity == 0)
        {
            if (a->substep == 1)
            {
                /* The original second pass uses a captured offscreen white silhouette.
                 * Keep its duration; framebuffer feedback is not yet presented here. */
                a->state.palette = 0x819f;
                a->state.opacity = 128;
                a->substep = 2;
            }
            else
            {
                a->remove = true;
                if ((Uint32)a->slot + 1 < SOTN_STAGE_ACTOR_SLOTS)
                    s->slots[a->slot + 1].remove = true;
            }
        }
    }
    return true;
}

bool ActorWarg(SotnActors *s, Actor *a)
{
    if (a->state.id == 0x4b)
        return Head(s, a);
    Uint8 animation = 0;
    Uint16 collision = 0;
    if ((a->state.flags & 0x100) && a->state.step != 11)
    {
        if (a->state.frame >= 43 && a->state.frame <= 48)
            a->state.facing_left = !a->state.facing_left;
        a->vx = a->vy = 0;
        if (!ActorSound(s, 0x697))
            return false;
        SetStep(a, 11);
    }
    else if (!(a->state.flags & 0x100) && (a->hit_flags & 15))
    {
        if (a->warg.attack_timer >= 16)
            a->warg.attack_timer /= 2;
        if (a->state.step == 2 || a->state.step == 3 || a->state.step == 7)
            SetStep(a, 10);
    }
    switch (a->state.step)
    {
    case 0: {
        Actor *head = NULL;
        if (!ActorSpawnAt(s, a, 0x4b, 0, (Uint32)a->slot + 1, &head) ||
            !ActorInitialize(s, a, 0xb9c))
            return false;
        if (head != NULL)
        {
            head->enemy_state.owner_slot = a->slot;
            head->enemy_state.owner_generation = a->state.generation;
        }
        a->state.facing_left = Side(a, s->player) == 0;
        a->state.x += (a->state.facing_left ? -32 : 32) * 65536;
        a->warg.attack_timer = 64;
        break;
    }
    case 1:
        if (!ActorAnimate(s, a, 0x30fc, &animation) || !ActorGround(s, a, 0x327c, &collision))
            return false;
        if (collision & 1)
        {
            SetStep(a, 2);
            a->warg.anchor_x = Pixel(a->state.x);
        }
        break;
    case 2:
        if (!ActorAnimate(s, a, 0x30fc, &animation))
            return false;
        if (Distance(a, s->player) < 128 || a->hit_flags)
            SetStep(a, 7);
        break;
    case 3: {
        AttackTimer(s, a);
        const Sint32 distance = a->warg.anchor_x - Pixel(a->state.x);
        if (a->warg.timer)
        {
            --a->warg.timer;
            a->remaining = 0;
            break;
        }
        if (!ActorAnimate(s, a, a->warg.backward ? 0x30dc : 0x30bc, &animation))
            return false;
        if (animation != 1)
        {
            SotnDataView velocity = {0};
            if (a->pose == 0 || a->pose > 6 ||
                !Slice(s->nz0, (a->warg.backward ? 0x32a4 : 0x328c) + (a->pose - 1U) * 4, 4,
                       &velocity))
                return SDL_SetError("Warg walk pose exceeds private velocity table");
            a->vx = Signed32(velocity.bytes);
            if (a->state.facing_left == a->warg.backward)
                a->vx = -a->vx;
            const bool reverse = a->warg.backward
                                     ? (a->state.facing_left ? distance > 16 : distance < -16)
                                     : (a->state.facing_left ? distance < -16 : distance > 16);
            if (reverse)
            {
                a->warg.backward = !a->warg.backward;
                a->pose = (Uint16)(7 - a->pose);
                a->warg.timer = 16;
            }
        }
        if (!ActorWalk(s, a, 0x3274, &collision))
            return false;
        if (a->warg.decision)
            --a->warg.decision;
        else if (a->state.facing_left == (Side(a, s->player) != 0))
            SetStep(a, 5);
        else if (a->warg.attack_timer == 0)
            return Attack(s, a);
        else if (distance < 16 && distance > -16)
        {
            a->vx = 0;
            SetStep(a, 7);
        }
        break;
    }
    case 5:
        AttackTimer(s, a);
        if (!ActorAnimate(s, a, 0x3130, &animation))
            return false;
        if (animation == 0)
        {
            a->state.facing_left = !a->state.facing_left;
            a->state.frame = 14;
            if (!Decide(s, a))
                return false;
        }
        break;
    case 6:
        if (a->substep == 0)
        {
            if (!ActorAnimate(s, a, 0x3178, &animation))
                return false;
            if (a->warg.timer == 0)
                return SDL_SetError("Warg charge timer underflow");
            if (--a->warg.timer == 0)
            {
                a->substep = 1;
                a->pose = a->remaining = 0;
            }
        }
        else
        {
            if (!ActorAnimate(s, a, 0x3148, &animation))
                return false;
            Actor *head = a->slot + 1 < SOTN_STAGE_ACTOR_SLOTS ? &s->slots[a->slot + 1] : NULL;
            if (a->vx)
                a->vx += a->vx < 0 ? 24576 : -24576;
            else if (head != NULL)
            {
                head->attack = a->attack;
                head->attack_element = a->attack_element;
            }
            if ((animation & 0x80) && a->pose == 7)
            {
                a->vx = a->state.facing_left ? 6 * 65536 : -6 * 65536;
                if (!ActorSound(s, 0x782))
                    return false;
                if (head != NULL)
                {
                    SotnDataView api = {0}, enemy = {0};
                    if (!Slice(s->dra, 0x98, 4, &api) || Read32(api.bytes) < 0x800a0000 ||
                        !Slice(s->dra, Read32(api.bytes) - 0x800a0000 + 0xb0U * 40, 10, &enemy))
                        return false;
                    head->attack = Read16(enemy.bytes + 6);
                    head->attack_element = Read16(enemy.bytes + 8);
                }
            }
            if (!ActorWalk(s, a, 0x3274, &collision) || (animation == 0 && !Decide(s, a)))
                return false;
        }
        break;
    case 7:
        ++a->warg.timer;
        AttackTimer(s, a);
        if (!ActorAnimate(s, a, 0x3180, &animation))
            return false;
        if (a->warg.decision)
            --a->warg.decision;
        else if (a->state.facing_left == (Side(a, s->player) != 0))
            SetStep(a, 5);
        else if (a->warg.attack_timer == 0)
            return Attack(s, a);
        break;
    case 8:
        if (!ActorAnimate(s, a, 0x31a0, &animation) || (animation == 0 && !Decide(s, a)))
            return false;
        break;
    case 10:
        AttackTimer(s, a);
        if (a->substep == 0)
        {
            a->substep = 1;
            a->pose = a->remaining = 0;
            a->vx = a->state.facing_left ? -2 * 65536 : 2 * 65536;
            if (!ActorSound(s, 0x781))
                return false;
        }
        else if (a->substep == 1)
        {
            if (!ActorWalk(s, a, 0x3274, &collision))
                return false;
            if (a->vx)
                a->vx += a->state.facing_left ? 8192 : -8192;
            if (!ActorAnimate(s, a, 0x31fc, &animation))
                return false;
            if (animation == 0)
            {
                a->substep = 2;
                a->pose = a->remaining = 0;
            }
        }
        else if (!ActorAnimate(s, a, 0x3204, &animation) || (animation == 0 && !Decide(s, a)))
            return false;
        break;
    case 11:
        return Death(s, a);
    case 12:
        break;
    default:
        return SDL_SetError("Warg entered unsupported state %u", a->state.step);
    }
    return Hitbox(s, a, a, false);
}
