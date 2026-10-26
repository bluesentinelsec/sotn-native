// SPDX-License-Identifier: AGPL-3.0-or-later
// BO0 e_boss_doors.c at sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Original initialization, tile definitions and artwork remain private runtime data.
#include "actors_internal.h"

static bool DoorTiles(SotnActors *s, const Actor *a, bool closed)
{
    if (s->map.width != 32)
        return SDL_SetError("BO0 boss doors require the original 32-column arena tilemap");
    const bool right = a->state.parameters != 0;
    return ActorTilePatch(s, right ? 0xdd : 0xc0, 4, 3, 32,
                          (right ? 0x13a4 : 0x1374) + (closed ? 24U : 0U));
}

static bool MoveDoor(Actor *a)
{
    const Sint64 x = (Sint64)a->state.x + a->vx;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32)
        return SDL_SetError("BO0 boss door movement exceeds fixed-point coordinates");
    a->state.x = (Sint32)x;
    return true;
}

static void SetDoorX(Actor *a, Sint32 pixel)
{
    // The original writes only posX.i.hi; retain its fractional word at either stop.
    a->state.x = (Sint32)((Sint64)pixel * 65536 + ((Uint32)a->state.x & 65535U));
}

static bool BossDoor(SotnActors *s, Actor *a)
{
    if (a->state.parameters > 1 || a->state.step > 6 || a->substep > 1)
        return SDL_SetError("BO0 boss door has an invalid side or movement phase");
    const bool right = a->state.parameters != 0;
    switch (a->state.step)
    {
    case 0:
        if (s->olrox_defeated)
        {
            a->remove = true;
            return true;
        }
        if (!ActorInitialize(s, a, 0x738))
            return false;
        a->state.priority = 0x69;
        if (right)
            SetDoorX(a, 536);
        return DoorTiles(s, a, false);
    case 1:
        if (Pixel(s->player.x) < 488)
        {
            if (right)
            {
                Actor *child = NULL;
                if (!ActorOwnedSpawnAt(s, a, 0x49, 0, (Uint32)a->slot + 1, &child))
                    return false;
                // CreateEntityFromEntity copies integer coordinates into a cleared child.
                child->state.x = -24 * 65536;
                child->state.y = Pixel(a->state.y) * 65536;
                child->state.facing_left = false;
            }
            if (!ActorPositionalSound(s, a, 0x608))
                return false;
            ++a->state.step;
        }
        return true;
    case 2: {
        if (!a->substep)
        {
            a->vx = right ? -32768 : 32768;
            ++a->substep;
        }
        Uint16 collision = 0;
        if (!ActorObjectCollision(s, a, 24, 32, 5, &collision) || !MoveDoor(a) ||
            (!(s->ticks & 15U) && !ActorPositionalSound(s, a, 0x608)))
            return false;
        if ((right && Pixel(a->state.x) < 488) || (!right && Pixel(a->state.x) > 24))
        {
            SetDoorX(a, right ? 488 : 24);
            ++a->state.step;
        }
        return true;
    }
    case 3:
        if (!DoorTiles(s, a, true))
            return false;
        ++a->state.step;
        return true;
    case 4:
        if (s->olrox_defeated)
            ++a->state.step;
        return true;
    case 5:
        if (!DoorTiles(s, a, false))
            return false;
        a->vx = right ? 32768 : -32768;
        ++a->state.step;
        return true;
    case 6:
        if (!MoveDoor(a))
            return false;
        if ((right && Pixel(a->state.x) > 536) || (!right && Pixel(a->state.x) < -24))
            a->remove = true;
        return true;
    default:
        return SDL_SetError("BO0 boss door phase is invalid");
    }
}

bool ActorOlroxArena(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 30)
        return SDL_SetError("Olrox arena objects require their original BO0 stage owner");
    switch (a->state.id)
    {
    case 8:
    case 0x11:
        return ActorRoomDecoration(s, a);
    case 0x12:
        return ActorCamera(s, a);
    case 0x49:
        return BossDoor(s, a);
    default:
        a->state.unsupported = true;
        return true;
    }
}
