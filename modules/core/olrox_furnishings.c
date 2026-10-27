// SPDX-License-Identifier: AGPL-3.0-or-later
// BO0 func_us_801B001C and st/e_chair.h, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. All furniture profiles remain private.
#include "olrox_internal.h"

static bool Move(Actor *a, Sint32 gravity)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy,
                 velocity = (Sint64)a->vy + gravity;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32 ||
        velocity < SDL_MIN_SINT32 || velocity > SDL_MAX_SINT32)
        return SDL_SetError("Olrox furniture debris exceeds fixed-point motion bounds");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    a->vy = (Sint32)velocity;
    return true;
}
static bool Spawn(SotnActors *s, Actor *a, Uint16 id, Uint16 parameters, Sint32 dx, Sint32 dy,
                  Actor **out)
{
    if (!ActorSpawn(s, a, id, parameters, 160, 192, out))
        return false;
    if (*out)
    {
        (*out)->state.x = Wrap16((Sint64)Pixel(a->state.x) + dx) * 65536;
        (*out)->state.y = Wrap16((Sint64)Pixel(a->state.y) + dy) * 65536;
        (*out)->state.facing_left = false;
    }
    return true;
}
static void Broken(Actor *a)
{
    a->state.frame = 0;
    a->state.step = 32;
}
static bool Furniture(SotnActors *s, Actor *a)
{
    const Uint16 kind = a->state.parameters & 255U;
    if (!kind || kind > 13 || (a->state.parameters & ~0x1ffU))
        return SDL_SetError("Olrox furniture requires an original variant 1..13");
    switch (a->state.step)
    {
    case 0: {
        SotnDataView profile = {0};
        if (kind < 5 && !Slice(s->nz0, 0xd94U + (kind - 1U) * 12U, 12, &profile))
            return false;
        if (!ActorInitialize(s, a, 0x624))
            return false;
        a->state.animation_set = 0x8006;
        a->state.texture_offset = 0x48;
        a->state.palette = 0x209;
        a->state.hitbox_state = 2;
        if (a->state.parameters & 0x100U)
            a->state.facing_left = true;
        a->state.priority = (Uint16)(a->state.priority - 4U);
        a->state.frame = kind;
        if (kind < 5)
        {
            a->state.hitbox_width = profile.bytes[0];
            a->state.hitbox_height = profile.bytes[2];
            a->state.hitbox_x = (Sint16)Signed16(profile.bytes + 4);
            a->state.hitbox_y = (Sint16)Signed16(profile.bytes + 6);
            a->state.hit_points = (Sint16)Signed16(profile.bytes + 8);
            a->state.priority = Read16(profile.bytes + 10);
        }
        else
            a->state.step = (Uint16)(a->state.step + kind);
        return true;
    }
    case 1:
        if (a->state.flags & 0x100U)
        {
            a->state.hitbox_state = 0;
            a->state.step = (Uint16)(a->state.step + kind);
        }
        return true;
    case 2:
        a->state.frame = 10;
        a->vx = Side(a, s->player) & 1U ? 65536 : -65536;
        a->state.step = 11;
        return true;
    case 3: {
        // The table's next three placed furniture records are part of its destruction group.
        if ((Uint32)a->slot + 3U >= SOTN_STAGE_ACTOR_SLOTS)
            return SDL_SetError("Olrox table group exceeds its original placed actor range");
        for (Uint16 i = 1; i <= 3; ++i)
            if (s->slots[a->slot + i].state.id != 0x2a || !s->slots[a->slot + i].state.generation)
                return SDL_SetError("Olrox table lost its original adjacent furniture group");
        Actor *piece = NULL;
        for (Uint16 i = 0; i < 4; ++i)
            if (!Spawn(s, a, 0x2a, (Uint16)(i + 5U), 48 - (Sint32)i * 30, 0, &piece))
                return false;
        if (piece)
            piece->state.parameters = 0x105;
        for (Uint16 i = 0; i < 4; ++i)
            if (!Spawn(s, a, 2, 19, 48 - (Sint32)i * 30, 0, &piece))
                return false;
        for (Uint16 i = 1; i <= 3; ++i)
            s->slots[a->slot + i].state.flags |= 0x100U;
        Broken(a);
        return true;
    }
    case 4: {
        Actor *piece = NULL;
        for (Uint16 i = 0; i < 2; ++i)
        {
            if (!Spawn(s, a, 0x2a, (Uint16)(i + 8U), 0, i * 8, &piece))
                return false;
            if (piece && i)
            {
                piece->vx = Side(a, s->player) & 1U ? 32768 : -32768;
                piece->vy = -65536;
            }
        }
        if (!ActorPositionalSound(s, a, 0x691))
            return false;
        Broken(a);
        return true;
    }
    case 5: {
        Actor *piece = NULL;
        for (Uint16 i = 0; i < 3; ++i)
            if (!Spawn(s, a, 0x2a, (Uint16)(i + 11U), 0, -28 + (Sint32)i * 28, &piece))
                return false;
        if (!ActorPositionalSound(s, a, 0x67f))
            return false;
        Broken(a);
        return true;
    }
    case 6:
    case 14:
        a->state.priority = 0x6a;
        return true;
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13: {
        const Uint16 step = a->state.step;
        const bool settled = step == 7 || step == 8 || step == 11;
        const Sint32 gravity = settled ? 0x1000 : step == 10 ? 0x2000 : 0x1800;
        const Sint32 sensor = step == 7 || step == 8 ? 8
                              : step == 11           ? 10
                              : step == 9            ? 4
                              : step == 10           ? 0
                                                     : 6;
        a->state.priority = settled ? 0x6a : step == 9 || step == 10 ? 0x6c : 0x6b;
        if (!settled)
        {
            a->state.rotated = true;
            a->state.rotation = (Sint16)Wrap16(a->state.rotation + (step == 9    ? -16
                                                                    : step == 10 ? 128
                                                                    : step == 12 ? -8
                                                                                 : 40));
        }
        if (!Move(a, gravity))
            return false;
        SotnCollider collision = {0};
        if (!ActorQuery(s, Pixel(a->state.x), Pixel(a->state.y) + sensor, &collision))
            return false;
        if (collision.effects & SOTN_EFFECT_SOLID)
        {
            if (settled)
                a->state.step = 32;
            else
            {
                // The original changes only the update identity/parameter/step; its rotation
                // and draw flags survive into the explosion's initialization.
                a->state.id = 2;
                a->state.parameters = 0;
                a->state.step = 0;
            }
        }
        return true;
    }
    case 32:
        return true;
    default:
        return SDL_SetError("Olrox furniture has an invalid original movement phase");
    }
}
static bool Chair(SotnActors *s, Actor *a)
{
    const Uint16 kind = a->state.parameters;
    if (kind < 1 || kind > 4 || a->state.step > 4)
        return SDL_SetError("Olrox chair requires a bounded original sitting pose");
    if (a->state.step)
    {
        const Sint32 slot = (Sint32)a->slot + (kind == 1 ? -1 : 5);
        if (slot < 0 || slot >= SOTN_STAGE_ACTOR_SLOTS)
            return SDL_SetError("Olrox chair furniture neighbor exceeds stage slots");
        if (s->slots[slot].state.flags & 0x100U)
        {
            a->remove = true;
            return true;
        }
    }
    switch (a->state.step)
    {
    case 0:
        return ActorInitialize(s, a, 0x5f4);
    case 1:
        if (s->buttons == SOTN_BUTTON_UP && SDL_abs(Pixel(a->state.x) - Pixel(s->player.x)) <= 4 &&
            SDL_abs(Pixel(a->state.y) - Pixel(s->player.y)) <= 4)
        {
            s->control.timer = 10;
            s->control.buttons = SOTN_BUTTON_UP;
            s->motion.chair_kind = kind;
            ++a->state.step;
        }
        return true;
    case 2: {
        s->motion.chair_kind = kind;
        const Sint32 dx = Pixel(a->state.x) - Pixel(s->player.x);
        if (!dx)
            ++a->state.step;
        else if (!ActorDisplace(s, dx > 0 ? 65536 : -65536, 0, 0))
            return false;
        return true;
    }
    case 3:
        s->motion.chair_kind = kind;
        if (s->player.animation.remaining < 0)
            ++a->state.step;
        SDL_FALLTHROUGH;
    case 4:
        s->motion.chair_kind = kind;
        if (s->player.step != 0 || s->player.substep != 4)
            a->state.step = 1;
        return true;
    default:
        return false;
    }
}
bool ActorOlroxFurnishing(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 30 || !a->state.generation)
        return SDL_SetError("Olrox furnishings require an active BO0 stage owner");
    if (a->state.id == 0x2a)
        return Furniture(s, a);
    if (a->state.id == 0x4b)
        return Chair(s, a);
    return SDL_SetError("Olrox furnishings require their original table or chair identity");
}
