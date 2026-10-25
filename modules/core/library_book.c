// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected Spellbook/Magic Tome logic from st/lib/e_spellbook_magic_tome.c
// at the pinned reconstruction. Runtime geometry, recipes and attributes remain private.
#include "actors_internal.h"
bool ActorLibraryBookMove(Actor *a)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Library book motion exceeds native fixed-point bounds");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    return true;
}
static Actor *Beam(SotnActors *s, const Actor *a)
{
    if ((Uint32)a->slot + 1 >= SOTN_STAGE_ACTOR_SLOTS)
        return NULL;
    Actor *p = &s->slots[a->slot + 1];
    return p->state.id == 0x3e && p->state.generation &&
                   p->state.generation == a->book.child_generation &&
                   p->enemy_state.owner_generation == a->state.generation
               ? p
               : NULL;
}
static bool Aim(SotnActors *s, const Actor *a, Sint16 *out)
{
    return SotnMath_Atan2(s->executable, Pixel(s->player.y) - Pixel(a->state.y),
                          Pixel(s->player.x) - Pixel(a->state.x), out);
}
static void Advance(Actor *a)
{
    for (Uint16 i = 0; i < 3; ++i)
        a->book.angle[i] = (Sint16)Wrap16((Sint32)a->book.angle[i] + a->book.delta[i]);
}
static void RandomTurn(SotnActors *s, Actor *a)
{
    for (Uint16 i = 0; i < 3; ++i)
        a->book.delta[i] = (Sint16)((Sint32)(ActorRandom(s) & 31U) - 16);
}
static bool Approach(Sint16 *value, Sint16 target, Sint16 step)
{
    if (SDL_abs((Sint32)*value - target) < step)
    {
        *value = target;
        return true;
    }
    // Preserve the source's two independent comparisons and strict threshold.
    if (*value > target)
        *value = (Sint16)(*value - step);
    if (*value < target)
        *value = (Sint16)(*value + step);
    return false;
}
static bool Initialize(SotnActors *s, Actor *a)
{
    const bool tome = a->state.id == 0x32;
    if (!ActorInitialize(s, a, tome ? 0x944 : 0x914))
        return false;
    a->state.hitbox_state = 0;
    if (!ActorPrimitives(s, a, 12))
    {
        a->remove = true;
        return true;
    }
    a->state.flags |= 0x800000U;
    a->state.palette = tome ? 0x25e : 0x25a;
    if (!ActorLibraryBookFaces(s, a))
        return false;
    Actor *child = NULL;
    if (!ActorLibrarySpawnAt(s, a, 0x3e, tome ? 2 : 0, (Uint32)a->slot + 1, &child))
        return false;
    child->state.x = Pixel(a->state.x) * 65536;
    child->state.y = Pixel(a->state.y) * 65536;
    child->state.facing_left = false;
    a->book.child_generation = child->state.generation;
    a->book.angle[0] = 1024;
    a->book.depth = 768;
    SetStep(a, 2);
    return true;
}
static bool Flying(SotnActors *s, Actor *a)
{
    const bool tome = a->state.id == 0x32;
    if (!a->substep)
    {
        a->book.wait = tome ? 96 : 512;
        a->substep = 1;
        if (tome)
        {
            const Sint32 dx = Pixel(s->player.x) - Pixel(a->state.x),
                         dy = Pixel(s->player.y) - Pixel(a->state.y);
            a->vx = SDL_abs(dx) > SDL_abs(dy) ? (dx > 0 ? 131072 : -131072) : 0;
            a->vy = SDL_abs(dx) > SDL_abs(dy) ? 0 : (dy > 0 ? 131072 : -131072);
            RandomTurn(s, a);
            a->book.timer = 64;
            a->substep = 2;
        }
    }
    if (!ActorLibraryBookMove(a))
        return false;
    if (!tome)
    {
        Sint16 target = 0;
        if (!Aim(s, a, &target))
            return false;
        const Uint16 diff = (Uint16)((Uint16)target - (Uint16)a->book.direction);
        const Uint16 magnitude = (diff & 0x800U) ? (Uint16)((0x800U - diff) & 0x7ffU) : diff;
        const Sint16 angle =
            magnitude > 16 ? (Sint16)Wrap16((Sint32)a->book.direction + (diff & 0x800U ? -16 : 16))
                           : target;
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(s->executable, angle, &trig))
            return false;
        a->vx = ActorShift((Sint64)a->book.magnitude * trig.cosine, 12);
        a->vy = ActorShift((Sint64)a->book.magnitude * trig.sine, 12);
        a->book.direction = angle;
        a->book.magnitude = SDL_min(49152, a->book.magnitude + 1024);
        if (a->hit_flags & 3U)
            a->book.magnitude = -49152;
        if (!a->book.timer)
        {
            a->book.timer = 64;
            RandomTurn(s, a);
        }
        else
            --a->book.timer;
    }
    Advance(a);
    if (!a->book.wait)
        return SDL_SetError("Library book flight timer underflow");
    if (--a->book.wait == 0)
        SetStep(a, 5);
    return true;
}
static bool Attack(SotnActors *s, Actor *a)
{
    Actor *child = Beam(s, a);
    if (!child)
        return SDL_SetError("Library book lost its required owned attack child");
    if (!a->substep)
    {
        Sint16 angle = 0;
        if (!Aim(s, a, &angle))
            return false;
        const Sint16 target[3] = {640, (Sint16)(angle + 2048), 1088};
        for (Uint16 i = 0; i < 3; ++i)
        {
            // RECT delta is narrowed before the source's signed remainder/division.
            Sint32 delta = Wrap16((Sint32)target[i] - a->book.angle[i]);
            delta = (delta + 8192) % 4096;
            if (delta > 2048)
                delta -= 4096;
            a->book.delta[i] = (Sint16)(delta / 32);
        }
        a->book.timer = 32;
        ++a->substep;
    }
    switch (a->substep)
    {
    case 1:
        Advance(a);
        if (!a->book.timer)
            return SDL_SetError("Library book attack turn timer underflow");
        if (--a->book.timer == 0)
            ++a->substep;
        break;
    case 2:
        if (Approach(&a->book.opening, 768, a->state.id == 0x30 ? 32 : 16))
            ++a->substep;
        break;
    case 3:
        child->book.command = true;
        ++a->substep;
        break;
    case 4:
        if (!child->book.command)
            ++a->substep;
        break;
    case 5:
        if (Approach(&a->book.opening, 0, 16))
            SetStep(a, 4);
        break;
    default:
        return SDL_SetError("Library book attack entered an invalid substep");
    }
    return true;
}
static bool Death(SotnActors *s, Actor *a)
{
    if ((ActorRandom(s) & 15U) == 0)
    {
        Actor *flame = NULL;
        if (!ActorSpawn(s, a, 2, 1, 160, 192, &flame))
            return false;
        if (flame)
        {
            flame->state.x = Wrap16((Sint64)Pixel(a->state.x) + a->book.center_x / 2 +
                                    (Sint32)(ActorRandom(s) & 31U) - 16) *
                             65536;
            flame->state.y = Wrap16((Sint64)Pixel(a->state.y) + a->book.center_y / 2 +
                                    (Sint32)(ActorRandom(s) & 31U) - 16) *
                             65536;
            flame->state.facing_left = false;
            flame->state.priority = a->state.id == 0x30 ? 0xd0 : 0xc0;
        }
    }
    if (!(s->game_frame & 15U) && !ActorLibrarySound(s, a, 0x655))
        return false;
    if (!a->substep)
    {
        for (Uint16 i = 0; i < 3; ++i)
            a->book.delta[i] = 64;
        a->book.timer = 128;
        a->state.hitbox_state = 0;
        ++a->substep;
    }
    if (a->substep == 1)
    {
        Approach(&a->book.opening, 512, 16);
        Advance(a);
        if (s->game_frame & 1U)
        {
            Actor *page = NULL;
            if (!ActorSpawn(s, a, 0x31, 0, 160, 192, &page))
                return false;
            if (page)
            {
                Sint16 angle = 0;
                SotnMathTrig trig = {0};
                if (!SotnMath_Atan2(s->executable, a->book.center_y, a->book.center_x, &angle))
                    return false;
                page->state.rotation = angle;
                page->state.priority = a->book.priority;
                const Sint32 offset = (Sint32)(ActorRandom(s) & 31U) - 16;
                if (!SotnMath_Trig(s->executable, (Sint32)angle + 1024, &trig))
                    return false;
                page->state.x = Wrap16((Sint64)Pixel(a->state.x) + a->book.center_x / 2 +
                                       ActorShift((Sint64)offset * trig.cosine, 12)) *
                                65536;
                page->state.y = Wrap16((Sint64)Pixel(a->state.y) + a->book.center_y / 2 +
                                       ActorShift((Sint64)offset * trig.sine, 12)) *
                                65536;
                page->state.facing_left = false;
            }
        }
        if (!(a->book.timer & 7U) &&
            !(a->state.id == 0x30 ? ActorLibrarySound(s, a, 0x78d) : ActorSound(s, 0x78d)))
            return false;
        if (!a->book.timer)
            return SDL_SetError("Library book death timer underflow");
        if (--a->book.timer == 0)
        {
            a->state.flags |= 0x80000000U;
            ++a->substep;
        }
    }
    else if (a->substep == 2)
    {
        if (!ActorLibraryBookMove(a) || a->vy > SDL_MAX_SINT32 - 4096)
            return SDL_SetError("Library falling book exceeds native gravity bounds");
        Approach(&a->book.opening, 1792, 8);
        a->vy += 4096;
    }
    else
        return SDL_SetError("Library book death entered an invalid substep");
    return true;
}
bool ActorLibraryBook(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 2 ||
        !((a->state.id >= 0x30 && a->state.id <= 0x32) || a->state.id == 0x3e))
        return SDL_SetError("Book dispatch requires original Library actor identities");
    if (a->state.id == 0x31 || a->state.id == 0x3e)
        return ActorLibraryBookBeam(s, a);
    if ((a->state.flags & 0x100U) && a->state.step < 8)
        SetStep(a, 8);
    switch (a->state.step)
    {
    case 0:
        if (!Initialize(s, a))
            return false;
        if (a->remove)
            return true;
        break;
    case 2:
        if (Distance(a, s->player) < (a->state.id == 0x30 ? 64 : 104) &&
            SDL_abs(Pixel(a->state.y) - Pixel(s->player.y)) < (a->state.id == 0x30 ? 64 : 96))
            SetStep(a, 3);
        break;
    case 3:
        a->book.depth = (Sint16)(a->book.depth - 16);
        if (a->book.depth < 0)
        {
            a->book.depth = 0;
            a->state.hitbox_state = 3;
            SetStep(a, 4);
        }
        break;
    case 4:
        if (!Flying(s, a))
            return false;
        break;
    case 5:
        if (!Attack(s, a))
            return false;
        break;
    case 8:
        if (!Death(s, a))
            return false;
        break;
    default:
        return SDL_SetError("Library book entered an unsupported gameplay step");
    }
    return ActorLibraryBookGeometry(s, a);
}
