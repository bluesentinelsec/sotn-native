// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/e_flying_zombie.h from the pinned reconstruction. Original motion,
// timelines, hover, sine and enemy attributes remain in private LIB/DRA/boot data.
#include "actors_internal.h"
static void Position(Actor *a, Sint32 x, Sint32 y)
{
    a->state.x = Wrap16(x) * 65536 + (Sint32)((Uint32)a->state.x & 65535U);
    a->state.y = Wrap16(y) * 65536 + (Sint32)((Uint32)a->state.y & 65535U);
}
static void ShiftX(Actor *a, Sint32 delta)
{
    Position(a, Wrap16((Sint64)Pixel(a->state.x) + delta), Pixel(a->state.y));
}
static void NewChild(Actor *a)
{
    a->state.x = Pixel(a->state.x) * 65536;
    a->state.y = Pixel(a->state.y) * 65536;
    a->state.facing_left = false;
}
static bool Move(Actor *a)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Flying Zombie motion exceeds native fixed-point bounds");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    return true;
}
static Actor *Partner(SotnActors *s, const Actor *a)
{
    const bool legs = a->state.id == 0x1b;
    if ((!legs && a->slot == 0) || (legs && (Uint32)a->slot + 1 >= SOTN_STAGE_ACTOR_SLOTS))
        return NULL;
    Actor *other = &s->slots[legs ? (Uint32)a->slot + 1 : (Uint32)a->slot - 1];
    return other->state.generation == a->flying.partner_generation &&
                   other->state.generation != 0 && other->state.id == (legs ? 0x1c : 0x1b)
               ? other
               : NULL;
}
static bool EnemyHP(SotnActors *s, Uint16 id, Sint16 *out)
{
    SotnDataView api = {0}, definition = {0};
    if (!Slice(s->dra, 0x98, 4, &api))
        return false;
    const Uint32 address = Read32(api.bytes);
    if (address < 0x800a0000 || address % 4 || id >= 400 ||
        !Slice(s->dra, address - 0x800a0000 + (size_t)id * 40, 40, &definition) ||
        Signed16(definition.bytes + 4) <= 0)
        return SDL_SetError(
            "Flying Zombie HP reset references an invalid private enemy definition");
    *out = (Sint16)Signed16(definition.bytes + 4);
    return true;
}
static bool Animation(SotnActors *s, Actor *a, size_t offset, bool *ended)
{
    Uint8 result = 0;
    if (!ActorAnimate(s, a, offset, &result))
        return false;
    *ended = result == 0;
    return true;
}
static bool Walk(SotnActors *s, Actor *a)
{
    Uint16 collision = 0;
    bool blocked = false;
    if (!ActorWalk(s, a, 0x2834, &collision) ||
        !ActorLibraryBlocked(s, a, 0x282c, a->state.facing_left, &blocked))
        return false;
    if (blocked)
        a->vx = 0;
    return true;
}
static bool DestroyPartner(SotnActors *s, Actor *a)
{
    Actor *other = Partner(s, a);
    if (other == NULL)
        return true;
    if (!SotnRoomActors_Release(s->spawns, other->slot, false))
        return false;
    ActorPrimitivesClear(s, other);
    *other = (Actor){0};
    a->flying.partner_generation = 0;
    return true;
}
static bool Legs(SotnActors *s, Actor *a)
{
    if (!a->flying.separated && (a->hit_flags & 3U) && a->state.step != 3)
    {
        a->state.hitbox_state = 0;
        if (!EnemyHP(s, 15, &a->state.hit_points) || !ActorLibrarySound(s, a, 0x714))
            return false;
        SetStep(a, 3);
    }
    if (a->state.flags & 0x100U)
    {
        if (!a->flying.separated)
        {
            a->state.hitbox_state = 0;
            a->state.flags &= ~0x100U;
            if (!EnemyHP(s, 15, &a->state.hit_points) || !ActorLibrarySound(s, a, 0x714))
                return false;
            SetStep(a, 3);
        }
        else if (a->state.step != 7)
        {
            a->state.hitbox_state = 0;
            SetStep(a, 7);
        }
    }
    bool ended = false;
    switch (a->state.step)
    {
    case 0: {
        if (!ActorInitialize(s, a, 0x878))
            return false;
        if (a->state.priority < 2)
            return SDL_SetError("Flying Zombie legs priority underflow");
        a->state.priority = (Uint16)(a->state.priority - 2);
        a->state.hitbox_x = 1;
        a->state.hitbox_y = 10;
        Actor *torso = NULL;
        if (!ActorLibrarySpawnAt(s, a, 0x1c, 0, (Uint32)a->slot + 1, &torso))
            return false;
        NewChild(torso);
        a->flying.partner_generation = torso->state.generation;
        torso->flying.partner_generation = a->state.generation;
        a->state.frame = 1;
        SDL_FALLTHROUGH;
    }
    case 1: {
        Uint16 grounded = 0;
        if (!ActorGround(s, a, 0x280c, &grounded))
            return false;
        if (grounded & 1U)
        {
            a->substep = 0;
            ++a->state.step;
        }
        break;
    }
    case 2:
        if (a->substep == 0)
        {
            a->state.facing_left = Side(a, s->player) == 0;
            a->vx = a->state.facing_left ? 8192 : -8192;
            ++a->substep;
        }
        else if (a->substep == 1)
        {
            if (!Animation(s, a, 0x283c, &ended) || !Walk(s, a))
                return false;
            if (a->flying.timer++ > 128)
            {
                a->flying.timer = 0;
                --a->substep;
            }
            if (a->state.frame == 7)
                a->vx += a->state.facing_left ? -512 : 512;
            if (a->state.frame == 8)
                a->vx += a->state.facing_left ? 512 : -512;
        }
        else
            return SDL_SetError("Flying Zombie walk substep is invalid");
        break;
    case 3: {
        Actor *torso = Partner(s, a);
        if (torso == NULL)
        {
            a->remove = true;
            return true;
        }
        if (!a->substep)
        {
            if (!ActorLibrarySound(s, a, 0x716))
                return false;
            a->flying.separated = true;
            for (Uint16 i = 0; i < 2; ++i)
            {
                Actor *blood = NULL;
                if (!ActorSpawn(s, a, 0x1e, 0, 160, 192, &blood))
                    return false;
                if (blood)
                {
                    NewChild(blood);
                    blood->state.facing_left = i != 0;
                    ShiftX(blood, 4 - (Sint32)i * 8);
                }
            }
            torso->state.step = 2;
            torso->state.hitbox_state = 0;
            ++a->substep;
        }
        if (!Animation(s, a, 0x2874, &ended))
            return false;
        if (ended)
        {
            SetStep(torso, 3);
            torso->state.hitbox_x = torso->state.hitbox_y = 0;
            torso->state.facing_left = a->state.facing_left;
            Position(torso, Pixel(torso->state.x), Pixel(a->state.y) - 26);
            torso->state.flags &= ~0x100U;
            if (!EnemyHP(s, 14, &a->state.hit_points))
                return false;
            a->state.frame = 18;
            a->state.hitbox_state = 3;
            SetStep(a, 5);
        }
        break;
    }
    case 4:
        if (!a->substep)
        {
            Actor *torso = Partner(s, a);
            if (torso == NULL)
            {
                a->remove = true;
                return true;
            }
            torso->state.step = 3;
            torso->state.frame = 16;
            torso->pose = torso->remaining = 0;
            torso->state.facing_left = a->state.facing_left;
            if (torso->state.priority < 8)
                return SDL_SetError("Flying Zombie torso priority underflow");
            torso->state.priority = (Uint16)(torso->state.priority - 8);
            Position(torso, Pixel(torso->state.x), Pixel(a->state.y) - 10);
            a->flying.separated = true;
            a->state.frame = 18;
            a->flying.timer = 8;
            a->flying.hover = 0;
            ++a->substep;
        }
        else if (a->substep == 1)
        {
            if (!a->flying.timer)
                return SDL_SetError("Flying Zombie separation wait underflow");
            if (--a->flying.timer == 0)
            {
                a->flying.hover = 0;
                a->flying.timer = 2;
                ++a->substep;
            }
        }
        else if (a->substep == 2)
        {
            if (!a->flying.timer)
                return SDL_SetError("Flying Zombie burst wait underflow");
            if (--a->flying.timer == 0)
            {
                Actor *effect = NULL;
                if (!ActorSpawn(s, a, 2, 1, 160, 192, &effect))
                    return false;
                if (effect)
                {
                    NewChild(effect);
                    ShiftY(effect, -(Sint32)a->flying.hover * 8);
                }
                a->flying.timer = 6;
                ++a->flying.hover;
                if (a->flying.hover > 1)
                {
                    if (!ActorLibrarySound(s, a, 0x694) || !DestroyPartner(s, a))
                        return false;
                    SetStep(a, 5);
                }
            }
        }
        else
            return SDL_SetError("Flying Zombie burst substep is invalid");
        break;
    case 5:
        if (!Animation(s, a, 0x288c, &ended))
            return false;
        if (ended)
        {
            a->pose = a->remaining = 0;
            ++a->state.step;
        }
        break;
    case 6:
        if (a->substep == 0)
        {
            SotnDataView timer = {0};
            if (!Slice(s->nz0, 0x2898 + (ActorRandom(s) & 3U) * 2U, 2, &timer))
                return false;
            const Sint32 value = Signed16(timer.bytes);
            if (value <= 0 || value > 2048)
                return SDL_SetError("Flying Zombie walk duration is invalid");
            a->flying.timer = (Uint16)value;
            a->vx = a->state.facing_left ? 4096 : -4096;
            ++a->substep;
        }
        else if (a->substep == 1)
        {
            if (!Animation(s, a, 0x2854, &ended) || !Walk(s, a))
                return false;
            if (a->state.frame == 1)
                a->vx += a->state.facing_left ? -160 : 160;
            if (a->state.frame == 2)
                a->vx += a->state.facing_left ? 160 : -160;
            if (!a->flying.timer)
                return SDL_SetError("Flying Zombie leg walk timer underflow");
            if (--a->flying.timer == 0)
            {
                a->pose = a->remaining = 0;
                ++a->substep;
            }
        }
        else if (a->substep == 2)
        {
            if (!Animation(s, a, 0x2868, &ended))
                return false;
            if (ended)
            {
                a->state.facing_left = (ActorRandom(s) & 1U) != 0;
                a->pose = a->remaining = a->substep = 0;
            }
        }
        else
            return SDL_SetError("Flying Zombie leg substep is invalid");
        break;
    case 7: {
        Actor *effect = NULL;
        if (!ActorSpawn(s, a, 2, 2, 160, 192, &effect) || !ActorLibrarySound(s, a, 0x694))
            return false;
        if (effect)
            NewChild(effect);
        a->remove = true;
        break;
    }
    default:
        return SDL_SetError("Flying Zombie entered an unsupported gameplay step");
    }
    return true;
}
static bool Drip(SotnActors *s, Actor *a)
{
    Actor *blood = NULL;
    if (!ActorSpawn(s, a, 0x1d, 0, 160, 192, &blood))
        return false;
    if (blood)
    {
        NewChild(blood);
        ShiftX(blood, (Sint32)(ActorRandom(s) & 8U) - 4);
        ShiftY(blood, 13);
    }
    return true;
}
static bool Dive(SotnActors *s, Actor *a)
{
    Sint16 angle = 0;
    if (!SotnMath_Atan2(s->executable, Wrap16((Sint64)Pixel(s->player.y) - Pixel(a->state.y)),
                        Wrap16((Sint64)Pixel(s->player.x) - Pixel(a->state.x)), &angle))
        return false;
    const Uint8 index = (Uint8)(ActorShift(angle, 4) + 64);
    SotnDataView x = {0}, y = {0};
    if (!Slice(s->nz0, 0x210c + (Uint32)index * 2U, 2, &x) ||
        !Slice(s->nz0, 0x210c + (Uint32)(Uint8)(index - 64) * 2U, 2, &y))
        return false;
    if (SDL_abs(Signed16(x.bytes)) > 4096 || SDL_abs(Signed16(y.bytes)) > 4096)
        return SDL_SetError("Flying Zombie dive sine exceeds its original amplitude");
    a->vx = Signed16(x.bytes) * 6;
    a->vy = Signed16(y.bytes) * 6;
    return true;
}
static bool Torso(SotnActors *s, Actor *a)
{
    if ((a->state.flags & 0x100U) && a->state.step > 5)
    {
        SetStep(a, 5);
        a->state.hitbox_state = 0;
        a->flying.hover = 0;
        a->flying.hovering = false;
        if (!ActorLibrarySound(s, a, 0x715) || !ActorLibrarySound(s, a, 0x691))
            return false;
    }
    bool ended = false;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x884))
            return false;
        a->state.hitbox_x = -4;
        a->state.hitbox_y = -14;
        a->flying.hovering = false;
        break;
    case 1: {
        Actor *legs = Partner(s, a);
        if (legs == NULL)
        {
            a->remove = true;
            return true;
        }
        a->state.facing_left = legs->state.facing_left;
        if ((a->hit_flags & 3U) || (a->state.flags & 0x100U))
        {
            a->state.hitbox_state = 0;
            a->state.flags &= ~0x100U;
            if (!EnemyHP(s, 14, &a->state.hit_points))
                return false;
            SetStep(legs, 4);
            ++a->state.step;
        }
        break;
    }
    case 2:
        break;
    case 3:
        a->state.hitbox_state = 3;
        if (!a->substep)
        {
            a->vy = -53248;
            ++a->substep;
        }
        if (!Move(a))
            return false;
        a->vy += 2048;
        if (!Animation(s, a, 0x2884, &ended))
            return false;
        if (ended)
        {
            a->substep = 0;
            a->state.step = 6;
        }
        break;
    case 5:
        if (!a->substep)
        {
            const Sint64 dx = (Sint64)a->state.x - s->player.x,
                         dy = (Sint64)a->state.y - s->player.y;
            Sint16 angle = 0;
            SotnMathTrig trig = {0};
            if (dx < SDL_MIN_SINT32 || dx > SDL_MAX_SINT32 || -dy < SDL_MIN_SINT32 ||
                -dy > SDL_MAX_SINT32)
                return SDL_SetError("Flying Zombie death direction exceeds fixed-point bounds");
            if (!SotnMath_Atan2Vector(s->executable, (Sint32)dx, (Sint32)-dy, &angle) ||
                !SotnMath_Trig(s->executable, angle, &trig))
                return false;
            a->state.rotation = angle;
            a->vx = trig.sine * 40;
            a->vy = trig.cosine * -40;
            a->flying.timer = 2;
            ++a->substep;
        }
        else if (a->substep == 1)
        {
            Uint16 grounded = 0;
            if (!ActorGround(s, a, 0x281c, &grounded))
                return false;
            if (grounded & 1U)
            {
                Actor *effect = NULL;
                if (!ActorSpawn(s, a, 2, 3, 96, 128, &effect))
                    return false;
                a->remove = true;
            }
            else
            {
                if (!a->flying.timer)
                    return SDL_SetError("Flying Zombie death-trail timer underflow");
                if (--a->flying.timer == 0)
                {
                    Actor *effect = NULL;
                    if (!ActorSpawn(s, a, 2, 1, 96, 128, &effect))
                        return false;
                    if (effect)
                    {
                        NewChild(effect);
                        effect->state.rotated = true;
                        effect->state.rotation = a->state.rotation;
                    }
                    a->flying.timer = 6;
                }
            }
        }
        else
            return SDL_SetError("Flying Zombie torso death substep is invalid");
        break;
    case 6:
        if (!Move(a))
            return false;
        a->vy += 2048;
        if (a->vy > 32768)
        {
            a->vy = 0;
            a->state.step = 7;
        }
        break;
    case 7:
        if (!(ActorRandom(s) & 127U))
            a->flying.hovering = true;
        if (!(ActorRandom(s) & 31U) && !Drip(s, a))
            return false;
        if (!a->substep)
        {
            a->state.facing_left = Side(a, s->player) == 0;
            a->vx = a->state.facing_left ? 32768 : -32768;
            a->flying.timer = 96;
            ++a->substep;
        }
        else
        {
            if (!Move(a))
                return false;
            if (!Animation(s, a, 0x2848, &ended))
                return false;
            if (!a->flying.timer)
                return SDL_SetError("Flying Zombie flight timer underflow");
            if (--a->flying.timer == 0)
            {
                a->pose = a->remaining = a->substep = 0;
                ++a->state.step;
            }
        }
        break;
    case 8:
        if (!a->substep)
        {
            a->flying.timer = 64;
            if (!Dive(s, a))
                return false;
            ++a->substep;
        }
        else
        {
            if (!(ActorRandom(s) & 127U))
                a->flying.hovering = true;
            if (!(ActorRandom(s) & 31U) && !Drip(s, a))
                return false;
            if (!Move(a))
                return false;
            if (!Animation(s, a, 0x2848, &ended))
                return false;
            if (!a->flying.timer)
                return SDL_SetError("Flying Zombie dive timer underflow");
            if (--a->flying.timer == 0)
            {
                a->vy = 0;
                a->pose = a->remaining = a->substep = 0;
                --a->state.step;
            }
        }
        break;
    default:
        return SDL_SetError("Flying Zombie torso entered an unsupported gameplay step");
    }
    if (a->state.step < 2)
    {
        Actor *legs = Partner(s, a);
        if (legs == NULL)
        {
            a->remove = true;
            return true;
        }
        Position(a, Pixel(legs->state.x), Pixel(legs->state.y));
    }
    if (a->flying.hovering && s->service_ticks % 3 == 0)
    {
        SotnDataView amount = {0};
        if (a->flying.hover >= 24 || !Slice(s->nz0, 0x28a0 + a->flying.hover * 2U, 2, &amount))
            return SDL_SetError("Flying Zombie hover cursor exceeds its private profile");
        const Sint32 delta = Signed16(amount.bytes);
        if (delta < -32 || delta > 32)
            return SDL_SetError("Flying Zombie hover offset is invalid");
        ShiftY(a, delta);
        if (++a->flying.hover >= 24)
        {
            a->flying.hover = 0;
            a->flying.hovering = false;
        }
    }
    return true;
}
bool ActorLibraryFlying(SotnActors *s, Actor *a)
{
    if (s == NULL || a == NULL || s->stage_id != 2 || (a->state.id != 0x1b && a->state.id != 0x1c))
        return SDL_SetError("Flying Zombie dispatch requires original Library actors");
    return a->state.id == 0x1b ? Legs(s, a) : Torso(s, a);
}
