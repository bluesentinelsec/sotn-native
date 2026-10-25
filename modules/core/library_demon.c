// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/lib/e_lesser_demon.c body/dummy at the pinned reconstruction.
#include "library_demon_internal.h"
static void Face(SotnActors *s, Actor *a)
{
    a->state.facing_left = Side(a, s->player) == 0;
}
static bool Animate(SotnActors *s, Actor *a, size_t stream, bool *out)
{
    Uint8 state = 0;
    if (!ActorAnimate(s, a, stream, &state))
        return false;
    *out = state == 0;
    return true;
}
static bool Ground(SotnActors *s, Actor *a, bool *out)
{
    Uint16 collision = 0;
    if (!ActorGround(s, a, 0x1ad4, &collision))
        return false;
    *out = (collision & 1U) != 0;
    return true;
}
static bool Flap(SotnActors *s, Actor *a, size_t stream)
{
    bool ended = false;
    if (!Animate(s, a, stream, &ended))
        return false;
    return a->remaining || a->pose != 7 || ActorLibrarySound(s, a, 0x68d);
}
static Actor *Punch(SotnActors *s, const Actor *a)
{
    if (a->demon.child_slot >= SOTN_STAGE_ACTOR_SLOTS)
        return NULL;
    Actor *child = &s->slots[a->demon.child_slot];
    return child->state.id == 0x22 && child->state.generation &&
                   child->state.generation == a->demon.child_generation &&
                   child->enemy_state.owner_generation == a->state.generation
               ? child
               : NULL;
}
static bool DestroyPunch(SotnActors *s, Actor *a)
{
    Actor *child = Punch(s, a);
    if (!child)
        return true;
    if (!SotnRoomActors_Release(s->spawns, child->slot, false))
        return false;
    ActorPrimitivesClear(s, child);
    *child = (Actor){0};
    a->demon.child_slot = SDL_MAX_UINT16;
    a->demon.child_generation = 0;
    return true;
}
static bool Dive(SotnActors *s, Actor *a)
{
    bool ended = false;
    if (!a->substep)
    {
        const Sint32 raw_x = Pixel(s->player.x) - Pixel(a->state.x),
                     raw_y = Pixel(s->player.y) - Pixel(a->state.y);
        const Sint32 x = Wrap16(raw_x + (raw_x < 0 ? 32 : -32)), y = Wrap16((Sint64)raw_y + 26);
        Sint16 angle = 0;
        SotnMathTrig trig = {0};
        if (!SotnMath_Atan2(s->executable, -y, x, &angle) ||
            !SotnMath_Trig(s->executable, angle, &trig))
            return false;
        a->vx = (Sint32)trig.cosine * 64;
        a->vy = SDL_max(65536, -(Sint32)trig.sine * 64);
        a->state.facing_left = a->vx > 0;
        Actor *child = NULL;
        if (!ActorSpawn(s, a, 0x22, 0, 96, 128, &child))
            return false;
        a->demon.child_slot = SDL_MAX_UINT16;
        a->demon.child_generation = 0;
        if (child)
        {
            child->state.x = Pixel(a->state.x) * 65536;
            child->state.y = Pixel(a->state.y) * 65536;
            child->state.facing_left = false;
            child->enemy_state.owner_slot = a->slot;
            child->enemy_state.owner_generation = a->state.generation;
            a->demon.child_slot = child->slot;
            a->demon.child_generation = child->state.generation;
        }
        a->demon.effect_step = 0;
        a->demon.attack_active = false;
        ++a->substep;
    }
    if (a->substep == 1)
    {
        SotnCollider c = {0};
        if (!ActorDemonMove(a) ||
            !ActorQuery(s, Pixel(a->state.x), Wrap16((Sint64)Pixel(a->state.y) + 32), &c))
            return false;
        if (c.effects & SOTN_EFFECT_SOLID)
        {
            if (!ActorLibrarySound(s, a, 0x647))
                return false;
            ShiftY(a, c.tile_top);
            Face(s, a);
            ++a->substep;
            if (!Punch(s, a))
                SetStep(a, 3);
        }
        else
            a->state.frame = 24;
    }
    else if (a->substep == 2)
    {
        Actor *child = Punch(s, a);
        if (!child)
            return SDL_SetError("Lesser Demon lost its owned punch hitbox");
        if (!Animate(s, a, 0x1b14, &ended))
            return false;
        if (ended && !(a->state.flags & 0x800000U))
        {
            a->demon.attack_active = false;
            if (!DestroyPunch(s, a))
                return false;
            SetStep(a, 3);
            return true;
        }
        if (!a->remaining && a->pose == 5 && !ActorLibrarySound(s, a, 0x78f))
            return false;
        if (a->pose > 4 && a->pose < 11)
        {
            child->state.x = Pixel(a->state.x) * 65536;
            child->state.y = Pixel(a->state.y) * 65536;
            child->state.facing_left = a->state.facing_left;
            child->state.hitbox_state = 1;
            a->demon.attack_active = true;
            if (!ActorDemonPuffs(s, a))
                return false;
        }
        else
            child->state.hitbox_state = 0;
    }
    else
        return SDL_SetError("Lesser Demon dive entered an invalid substep");
    return !a->demon.attack_active || ActorDemonPunchEffect(s, a);
}
static bool Leap(SotnActors *s, Actor *a)
{
    bool ended = false, landed = false;
    switch (a->substep)
    {
    case 0:
        if (!Animate(s, a, 0x1b2c, &ended))
            return false;
        if (ended)
        {
            a->pose = a->remaining = 0;
            ++a->substep;
            a->vy = -393216;
            a->vx = a->state.facing_left ? 98304 : -98304;
        }
        break;
    case 1:
        if (!Animate(s, a, 0x1b34, &ended) || !Ground(s, a, &landed))
            return false;
        if (landed)
        {
            if (!ActorLibrarySound(s, a, 0x647))
                return false;
            a->pose = a->remaining = 0;
            ++a->substep;
        }
        else
            a->vy -= 8192;
        break;
    case 2:
        if (!Animate(s, a, 0x1b44, &ended))
            return false;
        if (ended)
            SetStep(a, 3);
        break;
    default:
        return SDL_SetError("Lesser Demon leap entered an invalid substep");
    }
    return true;
}
static bool Hover(SotnActors *s, Actor *a)
{
    if (!a->substep)
    {
        Face(s, a);
        SotnDataView wait = {0};
        if (!Slice(s->nz0, 0x1c34 + (ActorRandom(s) & 3U) * 2U, 2, &wait))
            return false;
        a->demon.timer = Read16(wait.bytes);
        if (!a->demon.timer || a->demon.timer > 1024)
            return SDL_SetError("Lesser Demon hover wait exceeds its private profile");
        ++a->substep;
    }
    if (!Flap(s, a, 0x1b4c) || !ActorDemonMove(a))
        return false;
    a->vx = a->state.facing_left ? 65536 : -65536;
    if (--a->demon.timer == 0)
    {
        --a->substep;
        const Sint32 screen_x = Pixel(a->state.x) - s->camera_x;
        if ((Uint16)Wrap16(screen_x) & 0xff00U)
            SetStep(a, (ActorRandom(s) & 3U) ? 14 : 10);
        else if (Distance(a, s->player) > 64)
        {
            if ((ActorRandom(s) & 7U) == 0)
                SetStep(a, 4);
            else
            {
                SetStep(a, 12);
                if ((ActorRandom(s) & 3U) == 0)
                    SetStep(a, 14);
            }
        }
        else
            SetStep(a, 14);
        if (Pixel(a->state.y) - s->camera_y < 0)
            SetStep(a, 11);
    }
    return true;
}
static bool Spit(SotnActors *s, Actor *a)
{
    bool ended = false;
    switch (a->substep)
    {
    case 0:
        Face(s, a);
        if (!Animate(s, a, 0x1b84, &ended))
            return false;
        if (a->pose == 2)
        {
            if (!ActorLibrarySound(s, a, 0x70e))
                return false;
            a->demon.effect_step = 0;
            ++a->substep;
            a->demon.timer = 0;
        }
        break;
    case 1: {
        bool ready = false;
        if (!ActorDemonWindup(s, a, &ready))
            return false;
        if (ready)
        {
            Actor *spit = NULL;
            if (!ActorSpawn(s, a, 0x20, 0, 96, 128, &spit))
                return false;
            if (spit)
            {
                spit->state.facing_left = a->state.facing_left;
                spit->state.x =
                    Wrap16((Sint64)Pixel(a->state.x) + (a->state.facing_left ? 12 : -12)) * 65536;
                spit->state.y = Wrap16((Sint64)Pixel(a->state.y) - 8) * 65536;
            }
            a->demon.effect_step = 0;
            if (!ActorLibrarySound(s, a, 0x65d))
                return false;
            ++a->substep;
        }
        break;
    }
    case 2:
        if (!Animate(s, a, 0x1b84, &ended))
            return false;
        if (ended)
            SetStep(a, 9);
        break;
    default:
        return SDL_SetError("Lesser Demon spit entered an invalid substep");
    }
    return true;
}
static bool Bolt(SotnActors *s, Actor *a)
{
    if (!a->substep)
    {
        a->demon.effect_step = 0;
        ++a->substep;
    }
    bool ended = false;
    if (!Animate(s, a, 0x1b90, &ended))
        return false;
    if (ended)
    {
        a->demon.effect_step = 0;
        SetStep(a, 3);
        return true;
    }
    if (a->pose == 3 && !a->remaining)
    {
        if (!ActorLibrarySound(s, a, 0x7d1))
            return false;
        a->demon.effect_step = 2;
        Actor *bolt = NULL;
        if (!ActorSpawn(s, a, 0x21, 0, 96, 128, &bolt))
            return false;
        if (bolt)
        {
            bolt->state.x = Pixel(a->state.x) * 65536;
            bolt->state.y = Pixel(a->state.y) * 65536;
            bolt->state.facing_left = a->state.facing_left;
        }
    }
    return a->pose <= 1 || ActorDemonStarEffect(s, a);
}
static bool Summon(SotnActors *s, Actor *a)
{
    bool ended = false;
    switch (a->substep)
    {
    case 0: {
        bool floor = false;
        for (Uint16 i = 0; i < 10; ++i)
        {
            SotnCollider c = {0};
            if (!ActorQuery(s, Pixel(a->state.x), Wrap16((Sint64)Pixel(a->state.y) + i * 16), &c))
                return false;
            if (c.effects)
            {
                if (i < 3)
                {
                    a->demon.attack_active = false;
                    SetStep(a, 9);
                    return true;
                }
                floor = true;
                break;
            }
        }
        if (!floor)
        {
            a->demon.attack_active = false;
            SetStep(a, 9);
            return true;
        }
        if (!Animate(s, a, 0x1b9c, &ended))
            return false;
        Face(s, a);
        if (a->pose == 3)
        {
            a->demon.effect_step = 0;
            if (!ActorLibrarySound(s, a, 0x6dd))
                return false;
            ++a->substep;
            a->demon.saved_hp = (Uint16)a->state.hit_points;
            a->state.hit_points = 32767;
            a->demon.timer = 128;
        }
        break;
    }
    case 1:
        a->state.palette = (a->demon.timer & 1U) ? 0x249 : 0x24e;
        if (!a->demon.timer)
            return SDL_SetError("Lesser Demon summoning HP hold timer underflow");
        if (--a->demon.timer == 0)
        {
            a->state.palette = 0x249;
            a->state.flags &= ~0xfU;
            if (!ActorLibrarySound(s, a, 0x635))
                return false;
            a->state.hit_points = (Sint16)a->demon.saved_hp;
            ++a->substep;
            ++a->demon.effect_step;
        }
        return ActorDemonSummoning(s, a);
    case 2:
        a->demon.timer = (Uint16)(a->demon.timer - 1);
        if (!Animate(s, a, 0x1b9c, &ended))
            return false;
        if (ended && !(a->state.flags & 0x800000U))
        {
            a->demon.attack_active = false;
            SetStep(a, 9);
        }
        else
            return ActorDemonSummoning(s, a);
        break;
    default:
        return SDL_SetError("Lesser Demon summoning entered an invalid body substep");
    }
    return true;
}
static bool Flame(SotnActors *s, Actor *a, Uint16 id, bool tall)
{
    Actor *p = NULL;
    if (!ActorSpawn(s, a, id, tall ? 0 : 1, 160, 192, &p))
        return false;
    if (p)
    {
        p->state.x =
            Wrap16((Sint64)Pixel(a->state.x) + (Sint32)(ActorRandom(s) & 31U) - 16) * 65536;
        p->state.y = Wrap16((Sint64)Pixel(a->state.y) +
                            (Sint32)(ActorRandom(s) & (tall ? 63U : 31U)) - (tall ? 32 : 16)) *
                     65536;
        p->state.facing_left = false;
    }
    return true;
}
static bool Death(SotnActors *s, Actor *a)
{
    bool landed = false, ended = false, wall = false;
    a->state.palette = 0x8160;
    if (s->service_ticks % 7 == 0 && !Flame(s, a, 6, true))
        return false;
    if (!(s->service_ticks & 15U) && !ActorLibrarySound(s, a, 0x655))
        return false;
    if (!a->substep)
    {
        a->state.hitbox_state = 0;
        s->library_enemy_flags |= 2U;
        ++a->substep;
    }
    switch (a->substep)
    {
    case 1:
        if (!Flap(s, a, 0x1b4c) || !Ground(s, a, &landed))
            return false;
        if (landed)
        {
            ++a->substep;
            a->pose = a->remaining = 0;
        }
        else
            a->vy -= 14336;
        break;
    case 2:
        if (!ActorDemonWalkFrame(s, a, &wall) || !Animate(s, a, 0x1bac, &ended))
            return false;
        if (ended || wall)
        {
            a->pose = a->remaining = 0;
            a->state.rotated = a->state.scale_set = false;
            a->state.scale_x = a->state.scale_y = 0;
            a->state.opacity_set = true;
            a->state.opacity = 128;
            a->state.blend_mode = 0x30;
            a->demon.timer = 64;
            ++a->substep;
        }
        break;
    case 3:
        if (!Animate(s, a, 0x1bbc, &ended))
            return false;
        if (a->state.opacity < 2 || !a->demon.timer)
            return SDL_SetError("Lesser Demon death fade underflow");
        a->state.opacity -= 2;
        if (s->service_ticks % 5 == 0 && !Flame(s, a, 2, false))
            return false;
        if (--a->demon.timer == 0)
        {
            if (!ActorLibrarySound(s, a, 0x65e))
                return false;
            a->state.frame = 0;
            s->library_enemy_flags |= 4U;
            ++a->substep;
        }
        break;
    case 4:
        a->remove = true;
        break;
    default:
        return SDL_SetError("Lesser Demon death entered an invalid substep");
    }
    return true;
}
static bool Dummy(SotnActors *s, Actor *a)
{
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x98c))
            return false;
        a->state.hitbox_width = 28;
        a->state.hitbox_height = 4;
        a->state.hitbox_x = -20;
        a->state.hitbox_state = 0;
    }
    const Uint16 owner = a->enemy_state.owner_slot;
    if ((s->library_enemy_flags & 2U) || owner >= SOTN_STAGE_ACTOR_SLOTS ||
        s->slots[owner].state.id != 0x1f || !s->slots[owner].state.generation ||
        s->slots[owner].state.generation != a->enemy_state.owner_generation)
        a->remove = true;
    return true;
}
bool ActorLibraryDemon(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 2 || (a->state.id != 0x1f && a->state.id != 0x22))
        return SDL_SetError("Lesser Demon dispatch requires original Library identities");
    if (a->state.id == 0x22)
        return Dummy(s, a);
    if (a->vx < -8 * 65536 || a->vx > 8 * 65536 || a->vy < -8 * 65536 || a->vy > 8 * 65536)
        return SDL_SetError("Lesser Demon velocity exceeds its selected original range");
    if ((a->state.flags & 0x100U) && a->state.step < 15)
    {
        ActorDemonReleaseEffects(s, a);
        a->state.hitbox_state = 0;
        SetStep(a, 15);
    }
    bool ended = false, wall = false, landed = false;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x968))
            return false;
        a->state.flags &= ~0xc00U;
        if (a->state.priority < 2)
            return SDL_SetError("Lesser Demon body priority underflow");
        a->state.priority = (Uint16)(a->state.priority - 2);
        a->state.hitbox_x = 0;
        a->state.hitbox_y = 2;
        break;
    case 1:
        if (!Ground(s, a, &landed))
            return false;
        if (landed)
            SetStep(a, 2);
        break;
    case 2:
        a->state.hitbox_state = 0;
        Face(s, a);
        if (s->library_enemy_flags & 1U)
        {
            a->state.hitbox_state = 3;
            SetStep(a, 3);
        }
        break;
    case 3:
        if (!a->substep)
        {
            Face(s, a);
            a->demon.timer = 48;
            ++a->substep;
        }
        if (!ActorDemonWalkFrame(s, a, &wall))
            return false;
        if (wall)
            a->state.facing_left = !a->state.facing_left;
        if (!Animate(s, a, 0x1b04, &ended))
            return false;
        if (ended)
            Face(s, a);
        if (Pixel(a->state.x) < 352)
        {
            a->state.facing_left = true;
            SetStep(a, 6);
        }
        if (Pixel(a->state.x) > 688)
        {
            a->state.facing_left = false;
            SetStep(a, 6);
        }
        if (!a->demon.timer)
            return SDL_SetError("Lesser Demon ground decision timer underflow");
        if (--a->demon.timer == 0)
        {
            const Sint32 x = Pixel(a->state.x) - s->camera_x;
            if (!((Uint16)Wrap16(x) & 0xff00U))
            {
                if (ActorRandom(s) & 3U)
                {
                    Face(s, a);
                    a->demon.effect_step = 0;
                    SetStep(a, 13);
                    if (Distance(a, s->player) < 64 && (ActorRandom(s) & 3U))
                        SetStep(a, 4);
                }
                else
                    SetStep(a, 8);
            }
            else
                SetStep(a, 8);
        }
        break;
    case 4:
        if (!Dive(s, a))
            return false;
        break;
    case 6:
        if (!Leap(s, a))
            return false;
        break;
    case 8:
        if (!Flap(s, a, 0x1b4c) || !ActorDemonMove(a))
            return false;
        a->vx = 0;
        a->vy = -98304;
        if (Pixel(a->state.y) - s->camera_y < 80)
        {
            a->vy = 0;
            SetStep(a, 9);
        }
        break;
    case 9:
        if (!Hover(s, a))
            return false;
        break;
    case 10:
        if (!a->substep)
        {
            Face(s, a);
            a->vx = a->state.facing_left ? 131072 : -131072;
            a->demon.timer = 96;
            ++a->substep;
            break;
        }
        if (!ActorDemonMove(a) || !Animate(s, a, 0x1b68, &ended))
            return false;
        if (s->service_ticks % 8 == 0 && !ActorLibrarySound(s, a, 0x68d))
            return false;
        if (!a->demon.timer)
            return SDL_SetError("Lesser Demon flight timer underflow");
        if (--a->demon.timer == 0)
            SetStep(a, 9);
        break;
    case 11:
        if (!Flap(s, a, 0x1b4c) || !Ground(s, a, &landed))
            return false;
        if (landed)
        {
            if (!ActorLibrarySound(s, a, 0x647))
                return false;
            SetStep(a, 3);
        }
        if (Pixel(a->state.y) - s->camera_y > 208)
            SetStep(a, 8);
        break;
    case 12:
        if (!Spit(s, a))
            return false;
        break;
    case 13:
        if (!Bolt(s, a))
            return false;
        break;
    case 14:
        if (!Summon(s, a))
            return false;
        break;
    case 15:
        if (!Death(s, a))
            return false;
        if (a->remove)
            return true;
        break;
    default:
        return SDL_SetError("Lesser Demon entered an unsupported gameplay step");
    }
    const Sint32 x = SDL_clamp(Pixel(a->state.x), 312, 712),
                 y = SDL_clamp(Pixel(a->state.y), 560, 672);
    a->state.x = x * 65536 + (Sint32)((Uint32)a->state.x & 65535U);
    a->state.y = y * 65536 + (Sint32)((Uint32)a->state.y & 65535U);
    return true;
}
