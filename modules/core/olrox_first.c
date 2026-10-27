// SPDX-License-Identifier: AGPL-3.0-or-later
// Reconstructed US BO0 func_801AD338 and EntityOlroxAfterImage at pinned reference
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5; original US instructions supply control flow.
// Private timelines, decisions, sensors, initialization and artwork stay in runtime data.
#include "olrox_internal.h"

void OlroxSubstep(Actor *a, Uint16 substep)
{
    a->substep = substep;
    a->pose = a->remaining = 0;
}
static void WholeX(Actor *a, Sint32 x)
{
    a->state.x = (Sint32)((Sint64)Wrap16(x) * 65536 + ((Uint32)a->state.x & 65535U));
}
static void WholeY(Actor *a, Sint32 y)
{
    a->state.y = (Sint32)((Sint64)Wrap16(y) * 65536 + ((Uint32)a->state.y & 65535U));
}
static bool Move(Actor *a)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Olrox motion exceeds fixed-point coordinates");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    return true;
}
Actor *OlroxChild(SotnActors *s, const Actor *a, Uint16 relative)
{
    if (!s || !a || relative < 1 || relative > 13 ||
        (Uint32)a->slot + relative >= SOTN_STAGE_ACTOR_SLOTS)
        return NULL;
    Actor *child = &s->slots[a->slot + relative];
    return child->state.generation && child->state.generation == a->olrox.children[relative] &&
                   child->enemy_state.owner_slot == a->slot &&
                   child->enemy_state.owner_generation == a->state.generation
               ? child
               : NULL;
}
bool OlroxOwnsChild(SotnActors *s, const Actor *parent, const Actor *child, Uint16 relative)
{
    const Actor *owned = OlroxChild(s, parent, relative);
    return child && owned && owned->slot == child->slot &&
           owned->state.generation == child->state.generation &&
           owned->state.id == child->state.id && child->enemy_state.owner_slot == parent->slot &&
           child->enemy_state.owner_generation == parent->state.generation;
}
bool OlroxReserved(SotnActors *s, Actor *a, Uint16 relative, Uint16 id, Uint16 parameters,
                   Actor **out)
{
    if (!s || !a || !out || s->stage_id != 30 || a->state.id != 0x2b || !a->state.generation ||
        relative < 1 || relative > 13 || (Uint32)a->slot + relative >= SOTN_STAGE_ACTOR_SLOTS ||
        s->effect_serial >= 0x7fffffffU || id < 0x2c || id > 0x32)
        return SDL_SetError("Olrox child requires its active owner and original reserved block");
    *out = NULL;
    const Uint16 slot = (Uint16)(a->slot + relative);
    Actor *old = &s->slots[slot];
    SotnActorPlacement placement = {0};
    Uint32 placed = 0;
    if (!SotnRoomActors_Get(s->spawns, slot, &placement, &placed))
        return false;
    if (placed || (old->state.generation && !OlroxChild(s, a, relative)))
        return SDL_SetError("Olrox child would replace an unrelated actor in slot %u", slot);
    if (old->state.generation)
    {
        ActorPrimitivesClear(s, old);
        *old = (Actor){0};
    }
    if (!ActorSpawnAt(s, a, id, parameters, slot, out))
        return false;
    (*out)->state.x = Pixel(a->state.x) * 65536;
    (*out)->state.y = Pixel(a->state.y) * 65536;
    (*out)->state.facing_left = false;
    a->olrox.children[relative] = (*out)->state.generation;
    return true;
}
static bool Animate(SotnActors *s, Actor *a, size_t offset, bool *finished)
{
    Uint8 result = 0;
    if (!ActorAnimate(s, a, offset, &result))
        return false;
    *finished = result == 0;
    return true;
}
static bool AnimateNext(SotnActors *s, Actor *a, size_t offset, bool next_state)
{
    bool finished = false;
    if (!Animate(s, a, offset, &finished))
        return false;
    if (finished)
    {
        if (next_state)
            SetStep(a, 3);
        else
            OlroxSubstep(a, 1);
    }
    return true;
}
static bool BeginMusic(SotnActors *s)
{
    s->combat.music_change = true;
    s->combat.music = 0x334;
    return true;
}
static bool PreviousDestroyed(const SotnActors *s, const Actor *a)
{
    return a->slot > 0 && (s->slots[a->slot - 1].state.flags & 0x100U);
}
static void Face(SotnActors *s, Actor *a)
{
    a->state.facing_left = Side(a, s->player) != 0;
}
static bool Decision(SotnActors *s, Actor *a)
{
    const Sint32 x = Pixel(a->state.x), dx = Wrap16((Sint64)Pixel(s->player.x) - x);
    const Uint16 bits = (Uint16)dx;
    const Uint32 distance = (Uint32)(dx < 0 ? -dx : dx);
    if ((x < 80 && bits < 64) || (x >= 433 && bits > 0xffbfU))
        SetStep(a, 13);
    else if (distance < 36)
    {
        Face(s, a);
        SetStep(a, 5);
    }
    else if ((x < 128 && bits < 112) || (x >= 385 && bits > 0xff8fU))
        SetStep(a, 4);
    else if (distance < 64)
    {
        Face(s, a);
        SetStep(a, 6);
        a->substep = 1;
    }
    else if (distance >= 177)
    {
        Face(s, a);
        SetStep(a, 6);
    }
    else
    {
        SotnDataView definition = {0}, api = {0}, choice = {0};
        if (!Slice(s->dra, 0x98, 4, &api))
            return false;
        const Uint32 address = Read32(api.bytes);
        if (address < 0x800a0000U || address % 4U != 0 ||
            !Slice(s->dra, address - 0x800a0000U + 50U * 40U, 6, &definition))
            return SDL_SetError("Olrox decision requires its original enemy definition");
        if (Signed16(definition.bytes + 4) <= 0)
            return SDL_SetError("Olrox decision requires a positive original maximum HP");
        Face(s, a);
        Uint8 index = a->olrox.decision;
        if (a->state.hit_points < Signed16(definition.bytes + 4) * 3 / 4)
            index |= 8;
        if (index >= 16 || !Slice(s->nz0, 0xbac + index, 1, &choice) || choice.bytes[0] < 3 ||
            choice.bytes[0] > 13)
            return SDL_SetError("Olrox decision references an invalid private action");
        SetStep(a, choice.bytes[0]);
        a->olrox.decision = (Uint8)((index + 1) & 7U);
    }
    return true;
}
static bool Afterimage(SotnActors *s, Actor *a)
{
    a->olrox.timer = (Uint16)(a->olrox.timer - 1U);
    if (a->olrox.timer)
        return true;
    a->olrox.timer = 8;
    Actor *child = NULL;
    if (!ActorSpawn(s, a, 0x2c, a->state.frame, 160, 192, &child))
        return false;
    if (child)
    {
        child->state.x = Pixel(a->state.x) * 65536;
        child->state.y = Pixel(a->state.y) * 65536;
        child->state.facing_left = a->state.facing_left;
    }
    return true;
}
static bool Grounded(SotnActors *s, Actor *a, bool *landed)
{
    Uint16 collision = 0;
    if (!ActorGround(s, a, 0xbbc, &collision))
        return false;
    *landed = (collision & 1U) != 0;
    return true;
}
static bool Jump(SotnActors *s, Actor *a)
{
    bool finished = false, landed = false;
    switch (a->substep)
    {
    case 0:
        if ((Pixel(a->state.x) < 96 && a->state.facing_left) ||
            (Pixel(a->state.x) >= 433 && !a->state.facing_left))
        {
            SetStep(a, 3);
            return true;
        }
        if (!Animate(s, a, 0xd00, &finished))
            return false;
        if (finished)
        {
            a->vy = -294912;
            a->vx = 0;
            a->state.frame = 25;
            a->olrox.movement_left = Pixel(s->player.x) < Pixel(a->state.x);
            OlroxSubstep(a, 1);
        }
        return true;
    case 1:
        if (!Move(a))
            return false;
        a->vy += a->vy <= -98304 ? 8192 : 4096;
        if (a->vy >= 0)
        {
            a->vx = a->olrox.movement_left ? -196608 : 196608;
            a->state.frame = a->state.facing_left == a->olrox.movement_left ? 25 : 45;
            a->olrox.timer = 24;
            ++a->substep;
        }
        return true;
    case 2:
        if (--a->olrox.timer == 0)
        {
            a->olrox.timer = 8;
            ++a->substep;
        }
        return true;
    case 3:
        if (!Afterimage(s, a) || !Move(a))
            return false;
        Face(s, a);
        a->state.frame = a->olrox.movement_left == a->state.facing_left ? 25 : 45;
        if (Pixel(a->state.x) < 56 && a->vx < 0)
            ++a->substep;
        if (Pixel(a->state.x) >= 457 && a->vx > 0)
            ++a->substep;
        if (Distance(a, s->player) >= 81 && a->olrox.movement_left != (Side(a, s->player) != 0))
            ++a->substep;
        return true;
    case 4:
        if (Pixel(a->state.x) < 56)
        {
            a->vx = 0;
            WholeX(a, 56);
        }
        if (Pixel(a->state.x) >= 457)
        {
            a->vx = 0;
            WholeX(a, 456);
        }
        a->vy -= 8192;
        if (!Grounded(s, a, &landed))
            return false;
        if (landed)
        {
            ++a->substep;
            return ActorPositionalSound(s, a, 0x647);
        }
        return true;
    case 5:
        return AnimateNext(s, a, 0xd10, true);
    default:
        return SDL_SetError("Olrox aerial pass has an invalid subphase");
    }
}
static bool Backjump(SotnActors *s, Actor *a)
{
    bool finished = false, landed = false;
    switch (a->substep)
    {
    case 0:
        if (!Animate(s, a, 0xcdc, &finished))
            return false;
        if (finished)
        {
            a->vx = a->state.facing_left ? 262144 : -262144;
            a->vy = -196608;
            a->state.frame = 45;
            a->olrox.timer = 8;
            OlroxSubstep(a, 1);
        }
        return true;
    case 1:
        if (!Afterimage(s, a))
            return false;
        if (Pixel(a->state.x) < 56)
            WholeX(a, 56);
        if (Pixel(a->state.x) >= 457)
            WholeX(a, 456);
        if (!Grounded(s, a, &landed))
            return false;
        if (landed)
        {
            ++a->substep;
            return ActorPositionalSound(s, a, 0x647);
        }
        return true;
    case 2:
        return AnimateNext(s, a, 0xcec, true);
    default:
        return SDL_SetError("Olrox retreat has an invalid subphase");
    }
}
static bool Dash(SotnActors *s, Actor *a)
{
    switch (a->substep)
    {
    case 0:
    case 1:
        a->vx = a->state.facing_left ? -147456 : 147456;
        if (a->substep)
            a->vx = -a->vx;
        a->state.frame = a->substep ? 45 : 25;
        a->olrox.timer = 8;
        a->substep = 2;
        return true;
    case 2: {
        Uint16 collision = 0;
        if (!ActorWalk(s, a, 0xbcc, &collision) || !Afterimage(s, a))
            return false;
        if ((a->state.frame == 45 && (!a->vx || Distance(a, s->player) >= 97)) ||
            (a->state.frame == 25 && (!a->vx || Distance(a, s->player) < 112)))
        {
            Face(s, a);
            SetStep(a, 3);
        }
        return true;
    }
    default:
        return SDL_SetError("Olrox dash has an invalid subphase");
    }
}
static bool Teleport(SotnActors *s, Actor *a)
{
    bool finished = false;
    switch (a->substep)
    {
    case 0:
        if (!Animate(s, a, 0xcc4, &finished))
            return false;
        if (finished)
        {
            OlroxSubstep(a, 1);
            a->state.scale_y = 256;
            a->state.scale_x = 256;
            a->state.scale_set = true;
            if (!ActorPositionalSound(s, a, 0x6d9))
                return false;
        }
        if (a->pose >= 2)
            a->state.hitbox_state = 0;
        return true;
    case 1:
        a->state.scale_y = (Uint16)(a->state.scale_y - 32U);
        if (!a->state.scale_y)
        {
            a->state.scale_set = false;
            a->state.frame = 0;
            OlroxSubstep(a, 2);
        }
        return true;
    case 2: {
        const Sint32 previous = Pixel(a->state.x);
        const Sint32 offset = (Sint32)(ActorRandom(s) & 63U) + 32;
        WholeX(a, Pixel(s->player.x) + (previous < 256 ? offset : -offset));
        const Sint32 x = Pixel(a->state.x);
        // Preserve original screen-space clamps: these high words intentionally omit scrollX.
        if (x < 48)
            WholeX(a, 48 + s->camera_x);
        if (x >= 465)
            WholeX(a, 208 + s->camera_x);
        Face(s, a);
        OlroxSubstep(a, 3);
        return true;
    }
    case 3:
        if (!Animate(s, a, 0xcd0, &finished))
            return false;
        if (finished)
            SetStep(a, 3);
        if (a->pose >= 5)
            a->state.hitbox_state = 3;
        return true;
    default:
        return SDL_SetError("Olrox teleport has an invalid subphase");
    }
}
static bool Orbs(SotnActors *s, Actor *a)
{
    bool finished = false;
    switch (a->substep)
    {
    case 0:
        return AnimateNext(s, a, 0xc58, false);
    case 1:
        for (Uint16 i = 1; i <= 9; ++i)
        {
            const Uint16 slot = (Uint16)(a->slot + i);
            if (slot >= SOTN_STAGE_ACTOR_SLOTS)
                return SDL_SetError("Olrox orb block exceeds the actor pool");
            if (s->slots[slot].state.generation)
            {
                if (!OlroxChild(s, a, i))
                    return SDL_SetError("Olrox orb block contains an unrelated actor");
                continue;
            }
            SotnDataView position = {0};
            Actor *child = NULL;
            if (!Slice(s->nz0, 0xbd4 + (i - 1U) * 4U, 4, &position) ||
                !OlroxReserved(s, a, i, 0x2d, (Uint16)(i - 1U), &child))
                return false;
            WholeX(child, Signed16(position.bytes) + s->camera_x);
            WholeY(child, Signed16(position.bytes + 2) + s->camera_y);
        }
        a->olrox.release_orbs = 0;
        ++a->substep;
        return ActorPositionalSound(s, a, 0x816);
    case 2: {
        bool ready = true;
        for (Uint16 i = 1; i <= 9; ++i)
        {
            const Actor *child = OlroxChild(s, a, i);
            if (child && !child->state.hitbox_state)
                ready = false;
        }
        if (ready)
            ++a->substep;
        return true;
    }
    case 3:
        if (!Animate(s, a, 0xc6c, &finished))
            return false;
        if (finished)
            ++a->substep;
        if (a->pose == 5)
        {
            if (!a->remaining && !ActorPositionalSound(s, a, 0x818))
                return false;
            a->olrox.release_orbs = 1;
        }
        return true;
    case 4:
        SetStep(a, 3);
        return true;
    default:
        return SDL_SetError("Olrox orb command has an invalid subphase");
    }
}
static bool Portal(SotnActors *s, Actor *a)
{
    switch (a->substep)
    {
    case 0: {
        Uint16 occupied = 0;
        for (Uint16 i = 96; i < 104; ++i)
            occupied += s->slots[i].state.generation != 0;
        if (occupied >= 5)
        {
            SetStep(a, 3);
            return true;
        }
        return AnimateNext(s, a, 0xcac, false);
    }
    case 1: {
        Actor *child = NULL;
        if (!OlroxReserved(s, a, 10, 0x2e, 0, &child))
            return false;
        WholeY(child, Pixel(child->state.y) - 70);
        a->olrox.timer = 96;
        a->olrox.close_portal = 0;
        ++a->substep;
        return ActorPositionalSound(s, a, 0x636);
    }
    case 2:
        if (--a->olrox.timer == 0)
        {
            a->olrox.close_portal = 1;
            ++a->substep;
        }
        return true;
    case 3:
        return AnimateNext(s, a, 0xcbc, true);
    default:
        return SDL_SetError("Olrox portal command has an invalid subphase");
    }
}
static bool SkullCommand(SotnActors *s, Actor *a)
{
    bool finished = false;
    switch (a->substep)
    {
    case 0: {
        if ((Uint32)a->slot + 11 >= SOTN_STAGE_ACTOR_SLOTS)
            return SDL_SetError("Olrox skull block exceeds the actor pool");
        if (s->slots[a->slot + 11].state.generation)
        {
            SetStep(a, 3);
            return true;
        }
        const Sint32 visible_left = s->camera_x + 32;
        Sint32 x = visible_left < 80 ? 80 : visible_left >= 273 ? 272 - s->camera_x : 32;
        Uint8 pattern = (Uint8)(ActorRandom(s) & 3U);
        for (Uint16 i = 11; i <= 13; ++i)
        {
            Actor *child = NULL;
            if (!OlroxReserved(s, a, i, 0x31, pattern, &child))
                return false;
            WholeX(child, x + s->camera_x);
            WholeY(child, Pixel(a->state.y) + 30);
            x += 80;
            pattern = (Uint8)((pattern + 1) & 3U);
        }
        a->olrox.fire_skulls = 0;
        ++a->substep;
        return ActorSound(s, 0x684);
    }
    case 1:
        if (!Animate(s, a, 0xc80, &finished))
            return false;
        if (finished)
            SetStep(a, 3);
        if (a->pose == 16 && !a->remaining)
            a->olrox.fire_skulls = 1;
        return true;
    default:
        return SDL_SetError("Olrox skull command has an invalid subphase");
    }
}
static bool DefeatedForm(SotnActors *s, Actor *a)
{
    bool finished = false, landed = false;
    switch (a->substep)
    {
    case 0:
        a->vx = 0;
        if (!Grounded(s, a, &landed))
            return false;
        if (landed)
            ++a->substep;
        return true;
    case 1:
        if (a->slot < 16)
            return SDL_SetError("Olrox defeat lacks the original furnishing block");
        for (Uint16 i = (Uint16)(a->slot - 16); i < a->slot; ++i)
            if (s->slots[i].state.generation && s->slots[i].state.id != 0x2a &&
                s->slots[i].state.id != 0x4b)
                return SDL_SetError("Olrox defeat would damage an unrelated furnishing slot");
        for (Uint16 i = (Uint16)(a->slot - 16); i < a->slot; ++i)
            if (s->slots[i].state.generation)
                s->slots[i].state.flags |= 0x100U;
        OlroxSubstep(a, 4);
        if ((Uint16)(Pixel(a->state.x) - 96) >= 321)
        {
            a->state.scale_y = a->state.scale_x = 256;
            a->state.scale_set = true;
            OlroxSubstep(a, 2);
        }
        return true;
    case 2:
        if (!Animate(s, a, 0xcc4, &finished))
            return false;
        if (finished)
        {
            a->state.scale_y = (Uint16)(a->state.scale_y - 32U);
            if (!a->state.scale_y)
            {
                a->state.scale_set = false;
                a->state.frame = 0;
                WholeX(a, Pixel(a->state.x) + (Pixel(a->state.x) < 96 ? 96 : -96));
                OlroxSubstep(a, 3);
            }
        }
        return true;
    case 3:
        if (!Animate(s, a, 0xcd0, &finished))
            return false;
        if (finished)
            OlroxSubstep(a, 4);
        return true;
    case 4:
        s->olrox_flags |= 1;
        if (!Animate(s, a, 0xd2c, &finished))
            return false;
        if (finished)
            ++a->substep;
        return true;
    case 5: {
        Actor *child = NULL;
        if (!OlroxReserved(s, a, 1, 0x32, 0, &child))
            return false;
        WholeY(child, 464);
        child->state.facing_left = !a->state.facing_left;
        a->olrox.timer = 160;
        ++a->substep;
        return true;
    }
    case 6:
        if (--a->olrox.timer == 0)
        {
            a->state.frame = 0;
            a->state.step = 64;
        }
        return true;
    default:
        return SDL_SetError("Olrox first-form defeat has an invalid subphase");
    }
}

bool ActorOlroxFirst(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 30 || !a->state.generation || s->camera_x < 0 ||
        s->camera_x > 16384 || s->camera_y < 0 || s->camera_y > 16384 ||
        (a->state.id != 0x2b && a->state.id != 0x2c) || a->state.x < -1024 * 65536 ||
        a->state.x > 17408 * 65536 || a->state.y < -1024 * 65536 || a->state.y > 17408 * 65536 ||
        a->vx < -16 * 65536 || a->vx > 16 * 65536 || a->vy < -16 * 65536 || a->vy > 16 * 65536)
        return SDL_SetError("Olrox body requires an active bounded BO0 body/image");
    if (a->state.id == 0x2c)
    {
        if (!a->state.step)
        {
            if (!ActorInitialize(s, a, 0x6cc))
                return false;
            a->state.palette = 0x217;
            a->state.opacity_set = true;
            a->state.opacity = 128;
            a->state.hitbox_state = 0;
            a->state.blend_mode = 0x30;
            a->state.frame = a->state.parameters;
            a->state.priority = (Uint16)(a->state.priority - 2U);
        }
        a->state.opacity = (Uint8)(a->state.opacity - 2U);
        if (!a->state.opacity)
            a->remove = true;
        return true;
    }
    if ((a->state.flags & 0x100U) && a->state.step < 14)
    {
        a->state.hitbox_state = 0;
        SetStep(a, 14);
    }
    if (!a->state.step && s->world->time_attack[SOTN_TIME_OLROX])
    {
        a->remove = true;
        return true;
    }
    bool finished = false;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x6cc))
            return false;
        a->state.frame = 14;
        a->state.hitbox_state = 0;
        break;
    case 1:
        if (Distance(a, s->player) < 96 && s->player.contacts == 1)
        {
            if (!SotnWorld_VisitEvent(s->world, SOTN_TIME_OLROX))
                return false;
            SetStep(a, 2);
            a->olrox.timer = 192;
        }
        if (PreviousDestroyed(s, a))
        {
            if (!BeginMusic(s) || !SotnWorld_VisitEvent(s->world, SOTN_TIME_OLROX))
                return false;
            SetStep(a, 13);
            a->state.hitbox_state = 3;
        }
        break;
    case 2:
        if (PreviousDestroyed(s, a))
        {
            if (!BeginMusic(s))
                return false;
            SetStep(a, 13);
            a->state.hitbox_state = 3;
        }
        else if (a->substep == 0)
        {
            if (!AnimateNext(s, a, 0xc00, false))
                return false;
        }
        else if (a->substep == 1)
        {
            if (!Animate(s, a, 0xc0c, &finished))
                return false;
            if (finished)
            {
                if (!BeginMusic(s))
                    return false;
                a->state.hitbox_state = 3;
                SetStep(a, 3);
            }
        }
        else
            return SDL_SetError("Olrox introduction has an invalid subphase");
        break;
    case 3:
        if (!Decision(s, a))
            return false;
        break;
    case 4:
        if (!Jump(s, a))
            return false;
        break;
    case 5:
        if (!Backjump(s, a))
            return false;
        break;
    case 6:
        if (!Dash(s, a))
            return false;
        break;
    case 7:
        if (a->substep == 0)
        {
            if (!AnimateNext(s, a, 0xc40, false))
                return false;
        }
        else if (a->substep == 1)
            ++a->substep;
        else if (a->substep == 2)
        {
            if (!AnimateNext(s, a, 0xc4c, true))
                return false;
        }
        else
            return SDL_SetError("Olrox gesture has an invalid subphase");
        break;
    case 8:
        if (!Orbs(s, a))
            return false;
        break;
    case 9:
        if (!Portal(s, a))
            return false;
        break;
    case 10:
        if (!a->substep)
        {
            if ((Pixel(a->state.x) - s->camera_x) & 256)
            {
                SetStep(a, 3);
                break;
            }
            ++a->substep;
        }
        if (!Animate(s, a, 0xc24, &finished))
            return false;
        if (finished)
            SetStep(a, 3);
        if (a->pose == 4 && !a->remaining)
        {
            Actor *child = NULL;
            if (!ActorPositionalSound(s, a, 0x7d1) || !ActorSpawn(s, a, 0x30, 0, 96, 128, &child))
                return false;
            if (child)
            {
                child->state.x = Pixel(a->state.x) * 65536;
                child->state.y = Pixel(a->state.y) * 65536;
                child->state.facing_left = a->state.facing_left;
            }
        }
        break;
    case 11:
        if (!SkullCommand(s, a))
            return false;
        break;
    case 12: {
        if (!a->substep)
            a->vx = -32768; // Both original facing branches assign the same velocity.
        Uint16 collision = 0;
        if (!ActorWalk(s, a, 0xbcc, &collision) || !AnimateNext(s, a, 0xd20, true))
            return false;
        break;
    }
    case 13:
        if (!Teleport(s, a))
            return false;
        break;
    case 14:
        if (!DefeatedForm(s, a))
            return false;
        break;
    case 64:
        break;
    default:
        return SDL_SetError("Olrox humanoid entered an unsupported original state %u",
                            a->state.step);
    }
    a->state.hitbox_width = 5;
    a->state.hitbox_height = 28;
    a->state.hitbox_x = a->state.hitbox_y = 1;
    if (a->state.frame >= 45 && a->state.frame < 47)
    {
        a->state.hitbox_width = 4;
        a->state.hitbox_height = 18;
        a->state.hitbox_x = 0;
    }
    if (a->state.frame >= 48 && a->state.frame < 51)
    {
        a->state.hitbox_width = 4;
        a->state.hitbox_height = 13;
        a->state.hitbox_x = 0;
        a->state.hitbox_y = 10;
    }
    return true;
}
