// SPDX-License-Identifier: AGPL-3.0-or-later
// st/e_sword_lord.h, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Animation, hitbox, delay and fragment data remain in private NO1 tables.
#include "actors_internal.h"
static bool AttackBox(SotnActors *s, Actor *a)
{
    const Uint16 slot = a->enemy_state.owner_slot;
    if (slot >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Sword Lord attack owner slot is invalid");
    const Actor *p = &s->slots[slot];
    if (p->state.id != 0x48 || p->state.generation != a->enemy_state.owner_generation)
    {
        a->remove = true;
        return true;
    }
    if (!a->state.step && !ActorInitialize(s, a, 0xab8))
        return false;
    SotnDataView index = {0}, box = {0};
    if (!Slice(s->nz0, 0x2bb8 + (p->state.frame < 30 ? p->state.frame : 0), 1, &index) ||
        index.bytes[0] >= 21 || !Slice(s->nz0, 0x2b64 + index.bytes[0] * 4U, 4, &box))
        return SDL_SetError("Sword Lord attack hitbox exceeds its private table");
    const Uint8 *d = box.bytes;
    a->state.hitbox_x = (Sint16)(d[0] < 128 ? d[0] : (Sint32)d[0] - 256);
    a->state.hitbox_y = (Sint16)(d[1] < 128 ? d[1] : (Sint32)d[1] - 256);
    a->state.hitbox_width = d[2];
    a->state.hitbox_height = d[3];
    if (a->state.hitbox_width > 64 || a->state.hitbox_height > 64)
        return SDL_SetError("Sword Lord attack dimensions exceed their original bounds");
    a->state.facing_left = p->state.facing_left;
    a->state.x = Pixel(p->state.x) * 65536;
    a->state.y = Pixel(p->state.y) * 65536;
    return true;
}
static bool FragmentPosition(SotnActors *s, const Actor *a, Sint32 *x, Sint32 *y)
{
    SotnDataView position = {0};
    if (a->state.parameters >= 6 || !Slice(s->nz0, 0x2b4c + a->state.parameters * 4U, 4, &position))
        return SDL_SetError("Sword Lord fragment index exceeds its private positions");
    const Sint32 dx = Signed16(position.bytes), dy = Signed16(position.bytes + 2);
    if (SDL_abs(dx) > 128 || SDL_abs(dy) > 128)
        return SDL_SetError("Sword Lord fragment contact offsets exceed their private bounds");
    *x = Pixel(a->state.x) + dx * (a->state.facing_left ? 1 : -1);
    *y = Pixel(a->state.y) + dy;
    return true;
}
static bool Fragment(SotnActors *s, Actor *a)
{
    if (a->state.step == 7)
    {
        SotnDataView gravity = {0};
        if (a->state.parameters >= 6 || !ActorInitialize(s, a, 0xaac) ||
            !Slice(s->nz0, 0x2b34 + a->state.parameters * 4U, 4, &gravity))
            return false;
        a->gravity = Signed32(gravity.bytes);
        if (a->gravity < 0 || a->gravity > 65536)
            return SDL_SetError("Sword Lord fragment gravity exceeds its private bounds");
        a->state.hitbox_state = 0;
        a->state.frame = (Uint16)(a->state.parameters + 30);
        a->state.flags |= 0xc0202000;
        --a->state.priority;
        a->sword.fade = 160;
        return true;
    }
    Sint32 x = 0, y = 0;
    if (a->state.step == 8)
    {
        a->state.x += a->vx;
        a->state.y += a->vy;
        a->vy += a->gravity;
        if (!FragmentPosition(s, a, &x, &y))
            return false;
        SotnCollider c = {0};
        if (!ActorQuery(s, x, y, &c))
            return false;
        if (c.effects)
        {
            if (a->state.parameters < 2)
            {
                Actor origin = *a;
                origin.state.x = x * 65536;
                origin.state.y = y * 65536;
                if (!ActorExplosionInPool(s, &origin, 2, 160, 192, 0))
                    return false;
                a->remove = true;
            }
            else
            {
                a->state.opacity_set = true;
                a->state.opacity = a->sword.fade;
                ++a->state.step;
            }
        }
    }
    else if (a->state.step == 9)
    {
        if (!FragmentPosition(s, a, &x, &y))
            return false;
        if (ActorRandom(s) & 15)
        {
            Actor origin = *a;
            origin.state.x = x * 65536;
            origin.state.y = y * 65536;
            if (!ActorExplosionInPool(s, &origin, 2, 160, 192, 0))
                return false;
        }
        a->sword.fade = (Uint8)(a->sword.fade - 4);
        a->state.opacity = a->sword.fade;
        a->remove = a->sword.fade == 0;
    }
    else
        return SDL_SetError("Sword Lord fragment entered invalid state %u", a->state.step);
    return true;
}
static bool Pulse(SotnActors *s, Actor *a)
{
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    if (p == NULL)
        return false;
    switch (a->sword.phase)
    {
    case 0:
        a->sword.scale = 0;
        a->sword.brightness = 128;
        a->sword.palette = 0x19d;
        a->sword.u = 0;
        ++a->sword.phase;
        if (!ActorSound(s, 0x672))
            return false;
        SDL_FALLTHROUGH;
    case 1:
        a->sword.scale = (Uint16)(a->sword.scale + 0x300);
        if (a->sword.scale > 0x1800)
        {
            a->sword.palette = 0x19c;
            a->sword.brightness = 40;
            ++a->sword.phase;
        }
        break;
    case 2:
        a->sword.scale = (Uint16)(a->sword.scale + 0x180);
        if (a->sword.scale > 0x2000)
        {
            a->sword.palette = 0x19e;
            a->sword.u = 64;
            a->sword.brightness = 64;
            ++a->sword.phase;
        }
        break;
    case 3:
        a->sword.scale = (Uint16)(a->sword.scale + 64);
        if (a->sword.scale > 0x2800)
            ++a->sword.phase;
        SDL_FALLTHROUGH;
    case 4:
        a->sword.scale = (Uint16)(a->sword.scale + 64);
        if (--a->sword.brightness == 0)
            ++a->sword.phase;
        break;
    case 5:
        break;
    default:
        return SDL_SetError("Sword Lord death pulse entered an invalid phase");
    }
    if (a->sword.scale > 0x4000)
        return SDL_SetError("Sword Lord death pulse exceeds its original scale");
    const Uint16 size = (Uint16)((64U * a->sword.scale) / 4096);
    if (!ActorSprite(s, a, 0, a->state.x, a->state.y, a->sword.u, 192, 63, 63, 0x3a,
                     a->sword.palette))
        return false;
    p->state.x = Pixel(a->state.x) * 65536;
    p->state.y = Pixel(a->state.y) * 65536;
    p->state.width = p->state.height = size;
    p->state.red = p->state.green = p->state.blue = a->sword.brightness;
    p->state.blend = p->state.modulate = true;
    p->state.active = size != 0 && a->sword.brightness != 0;
    return true;
}
static bool SpawnFragment(SotnActors *s, const Actor *a, Uint16 part)
{
    Actor *p = NULL;
    if (!ActorSpawn(s, a, 0x48, part, 160, 192, &p))
        return false;
    if (p != NULL)
    {
        p->state.x = Pixel(a->state.x) * 65536;
        p->state.y = Pixel(a->state.y) * 65536;
        p->state.step = 7;
        p->state.flags |= 0x2000;
    }
    return true;
}
static bool Death(SotnActors *s, Actor *a)
{
    Uint8 animation = 0;
    switch (a->substep)
    {
    case 0:
        if ((ActorRandom(s) & 7) == 0)
        {
            Actor *effect = NULL;
            if (!ActorSpawn(s, a, 6, 0, 160, 192, &effect))
                return false;
            if (effect != NULL)
            {
                effect->state.x = (Pixel(a->state.x) + 32 - (Sint32)(ActorRandom(s) & 47)) * 65536;
                effect->state.y = (Pixel(a->state.y) + 32 - (Sint32)(ActorRandom(s) & 63)) * 65536;
                effect->state.facing_left = false;
            }
        }
        if (!ActorAnimate(s, a, 0x2b00, &animation))
            return false;
        if (!animation)
        {
            Actor *effect = NULL;
            if (!ActorSpawn(s, a, 2, 3, 160, 192, &effect))
                return false;
            if (effect != NULL)
            {
                effect->state.x = Pixel(a->state.x) * 65536;
                effect->state.y = Pixel(a->state.y) * 65536;
                effect->state.facing_left = false;
                if (!ActorSound(s, 0x695))
                    return false;
            }
            a->pose = a->remaining = 0;
            ++a->substep;
        }
        break;
    case 1:
        if (a->state.frame == 29)
        {
            if (!SpawnFragment(s, a, 0))
                return false;
            ++a->substep;
        }
        SDL_FALLTHROUGH;
    case 2:
        if (!a->remaining && a->pose == 1 && !ActorSound(s, 0x640))
            return false;
        if (!ActorAnimate(s, a, 0x2b10, &animation))
            return false;
        if (!animation)
        {
            for (Uint16 i = 1; i <= 5; ++i)
                if (!SpawnFragment(s, a, i))
                    return false;
            a->state.frame = 0;
            a->sword.timer = 48;
            ++a->substep;
        }
        break;
    case 3:
        if (--a->sword.timer == 0)
        {
            if (!ActorSound(s, 0x696))
                return false;
            a->remove = true;
            return true;
        }
        break;
    default:
        return SDL_SetError("Sword Lord death substate is invalid");
    }
    if (a->substep && a->sword.pulse)
        return Pulse(s, a);
    return true;
}
static bool Delay(SotnActors *s, Actor *a)
{
    SotnDataView time = {0};
    if (!Slice(s->nz0, 0x2a98 + (ActorRandom(s) & 3), 1, &time))
        return false;
    a->sword.delay = time.bytes[0];
    return true;
}
static bool Walk(SotnActors *s, Actor *a, bool reverse)
{
    Uint16 collision = 0;
    if (!ActorWalk(s, a, 0x2a80, &collision))
        return false;
    if (collision & 96)
    {
        a->state.x -= a->vx;
        a->vx = reverse ? -a->vx : 0;
    }
    return true;
}
static bool ResetAttack(SotnActors *s, Actor *a)
{
    a->pose = a->remaining = a->substep = 0;
    if (!Delay(s, a))
        return false;
    a->vx = a->state.facing_left ? 65536 : -65536;
    a->sword.smashed = false;
    a->state.step = 3;
    return true;
}
bool ActorSwordLord(SotnActors *s, Actor *a)
{
    if (a->state.id == 0x49)
        return AttackBox(s, a);
    if (a->state.step >= 7)
        return Fragment(s, a);
    if ((Uint32)a->slot + 1 >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Sword Lord requires its adjacent original attack actor");
    Actor *attack = &s->slots[a->slot + 1];
    if ((a->state.flags & 0x100) && a->state.step < 6)
    {
        if (!ActorSound(s, 0x77f))
            return false;
        a->state.hitbox_state = 0;
        attack->state.hitbox_state = 0;
        SetStep(a, 6);
        --a->state.priority;
        Uint16 available = 0;
        for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
            available = (Uint16)(available + (s->primitives[i].owner == 0));
        a->sword.pulse = available >= 2;
        if (a->sword.pulse && !ActorPrimitives(s, a, 2))
            return false;
    }
    Uint8 animation = 0;
    Uint16 collision = 0;
    switch (a->state.step)
    {
    case 0: {
        if (attack->state.generation)
            return SDL_SetError("Sword Lord reserved attack slot is occupied");
        if (!ActorInitialize(s, a, 0xaac))
            return false;
        a->state.facing_left = Side(a, s->player) != 0;
        Actor *child = NULL;
        return ActorSpawnAt(s, a, 0x49, 0, a->slot + 1U, &child);
    }
    case 1:
        if (!ActorGround(s, a, 0x2a88, &collision))
            return false;
        if (collision & 1)
            a->state.step = 2;
        break;
    case 2:
        if (!ActorAnimate(s, a, 0x2b1c, &animation))
            return false;
        a->state.facing_left = Side(a, s->player) != 0;
        if (Distance(a, s->player) < 96)
        {
            a->pose = a->remaining = 0;
            if (!Delay(s, a))
                return false;
            a->state.step = 3;
            a->vx = a->state.facing_left ? 65536 : -65536;
        }
        break;
    case 3:
        if (!ActorAnimate(s, a, 0x2a9c, &animation) || !Walk(s, a, true))
            return false;
        if (Distance(a, s->player) > 96)
            a->vx = a->state.facing_left ? -65536 : 65536;
        if (Distance(a, s->player) < 64)
            a->vx = a->state.facing_left ? 65536 : -65536;
        if (a->sword.timer++ > a->sword.delay)
        {
            a->pose = a->remaining = 0;
            a->sword.timer = 0;
            a->vx = 0;
            if (ActorRandom(s) & 1)
                a->state.step = 4;
            else
            {
                a->sword.smashed = true;
                a->state.step = 5;
            }
        }
        break;
    case 4:
        if (!Walk(s, a, false) || !ActorAnimate(s, a, 0x2aac, &animation))
            return false;
        if (!animation && !ResetAttack(s, a))
            return false;
        if (a->state.frame == 9 && !a->substep)
        {
            if (!ActorSound(s, 0x77d) || !ActorSound(s, 0x6c7))
                return false;
            a->vx = a->state.facing_left ? -4 * 65536 : 4 * 65536;
            a->substep = 1;
        }
        if (a->state.frame == 13)
        {
            a->vx = 0;
            if (!a->sword.smashed)
            {
                const Sint16 gap = a->state.facing_left ? 4 : -4;
                if (!ActorOuterDust(s, a, 4, 1, -16, 30, 1, gap) ||
                    !ActorOuterDust(s, a, 4, 1, 16, 30, 1, gap))
                    return false;
                a->sword.smashed = true;
            }
        }
        break;
    case 5:
        if (!Walk(s, a, false))
            return false;
        a->substep ^= 1;
        if (!ActorAnimate(s, a, 0x2ad4, &animation))
            return false;
        if (!animation && !ResetAttack(s, a))
            return false;
        if (a->state.frame == 19 && a->substep)
        {
            if (!a->remaining && !ActorSound(s, 0x64a))
                return false;
            if (!ActorOuterDust(s, a, 2, 1, a->state.facing_left ? 8 : -8, 30, 0,
                                a->state.facing_left ? 4 : -4))
                return false;
        }
        if (a->state.frame == 18)
        {
            a->vx = a->state.facing_left ? -4 * 65536 : 4 * 65536;
            a->sword.smashed = false;
        }
        if (a->state.frame == 17)
        {
            a->vx = 0;
            if (!a->sword.smashed)
            {
                const Sint16 gap = a->state.facing_left ? 4 : -4;
                if (!ActorOuterDust(s, a, 5, 3, -16, 30, 0, gap) ||
                    !ActorOuterDust(s, a, 5, 3, 16, 30, 0, gap) || !ActorSound(s, 0x77e) ||
                    !ActorSound(s, 0x60b) || !ActorSound(s, 0x649))
                    return false;
                a->sword.smashed = true;
            }
        }
        break;
    case 6:
        return Death(s, a);
    default:
        return SDL_SetError("Sword Lord entered invalid state %u", a->state.step);
    }
    return true;
}
