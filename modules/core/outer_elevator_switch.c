// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected NO1 unk_38F50.c/unk_39184.c gear, lever and pressure-platform rules.
#include "actors_internal.h"
static bool Gear(SotnActors *s, Actor *a)
{
    if (a->state.parameters >= 9)
        return SDL_SetError("Outer Wall gear variant exceeds its private animation table");
    if (!a->state.step)
    {
        SotnDataView frame = {0};
        if (!ActorInitialize(s, a, 0xa1c) ||
            !Slice(s->nz0, 0x13fc + a->state.parameters, 1, &frame))
            return false;
        a->state.frame = frame.bytes[0];
        a->state.priority = 0x6a;
    }
    else if (a->state.step == 1)
    {
        if (s->world->flags[SOTN_OUTER_ELEVATOR])
            ++a->state.step;
    }
    else if (a->state.step == 2)
    {
        SotnDataView stream = {0};
        if (!Slice(s->nz0, 0x1408 + a->state.parameters * 4U, 4, &stream))
            return false;
        const Uint32 address = Read32(stream.bytes);
        if (address < 0x801813ac || address >= 0x801813fc)
            return SDL_SetError("Outer Wall gear animation pointer exceeds its private streams");
        Uint8 animation = 0;
        if (!ActorAnimate(s, a, address - 0x80180000, &animation))
            return false;
    }
    else
        return SDL_SetError("Outer Wall gear entered an invalid state");
    return true;
}
static bool LeverParent(SotnActors *s, const Actor *a, Actor **out)
{
    const Uint16 slot = a->enemy_state.owner_slot;
    if (slot >= SOTN_STAGE_ACTOR_SLOTS || s->slots[slot].state.id != 0x33 ||
        s->slots[slot].state.generation != a->enemy_state.owner_generation)
        return SDL_SetError("Outer Wall lever lost its checked adjacent parent");
    *out = &s->slots[slot];
    return true;
}
static bool Lever(SotnActors *s, Actor *a)
{
    Actor *parent = NULL;
    switch (a->state.step)
    {
    case 0:
        if (a->state.parameters > 1 || !ActorInitialize(s, a, 0xa1c))
            return SDL_SetError("Outer Wall lever variant or initialization is invalid");
        a->state.frame = 28;
        a->state.priority = 0x6c;
        if (!a->state.parameters)
        {
            if ((Uint32)a->slot + 1 >= SOTN_STAGE_ACTOR_SLOTS ||
                s->slots[a->slot + 1].state.generation)
                return SDL_SetError("Outer Wall lever requires its free adjacent handle slot");
            Actor *child = NULL;
            if (!ActorSpawnAt(s, a, 0x33, 1, a->slot + 1U, &child))
                return false;
            child->state.x = Pixel(a->state.x) * 65536;
            child->state.y = (Pixel(a->state.y) + 16) * 65536;
            a->state.step = 3;
        }
        else
        {
            a->state.palette = 0x8041;
            a->state.hitbox_state = 3;
            a->state.hitbox_width = 6;
            a->state.hitbox_height = 18;
            a->state.hitbox_y = -4;
            a->state.rotated = true;
            a->state.rotation = -128;
            a->state.frame = 29;
            if (s->world->flags[SOTN_OUTER_ELEVATOR])
            {
                if (!ActorSoundVolumePan(s, 0x7aa, 0, 0))
                    return false;
                a->state.hitbox_state = 0;
                a->state.step = 2;
            }
        }
        break;
    case 1:
        if (!LeverParent(s, a, &parent))
            return false;
        a->state.hitbox_x = (Sint16)-ActorShift((Sint64)ActorSin(a->state.rotation) * 32, 12);
        a->state.hitbox_y = (Sint16)ActorShift((Sint64)ActorSin(a->state.rotation + 1024) * 32, 12);
        parent->state.x =
            (Pixel(a->state.x) + ActorShift((Sint64)ActorSin(a->state.rotation) * 18, 12)) * 65536;
        parent->state.y =
            (Pixel(a->state.y) + ActorShift(-(Sint64)ActorSin(a->state.rotation + 1024) * 18, 12)) *
            65536;
        if (!a->substep)
        {
            if (a->hit_flags)
            {
                if (!ActorSound(s, 0x642))
                    return false;
                a->timer = 16;
                ++a->substep;
            }
        }
        else if (a->substep == 1)
        {
            a->state.rotation = (Sint16)(a->state.rotation + 4);
            if (--a->timer == 0)
                --a->substep;
            if (a->state.rotation >= 32)
            {
                a->state.hitbox_state = 0;
                if (!ActorSound(s, 0x62f) || !ActorSoundVolumePan(s, 0x7aa, 127, 0))
                    return false;
                s->world->flags[SOTN_OUTER_ELEVATOR] = 1;
                a->state.step = 2;
            }
        }
        else
            return SDL_SetError("Outer Wall lever entered an invalid strike substate");
        break;
    case 2: {
        if (!LeverParent(s, a, &parent))
            return false;
        a->state.rotation = 32;
        parent->state.x = (Pixel(a->state.x) + ActorShift((Sint64)ActorSin(32) * 18, 12)) * 65536;
        parent->state.y =
            (Pixel(a->state.y) + ActorShift(-(Sint64)ActorSin(1056) * 18, 12)) * 65536;
        const Sint32 screen = Pixel(a->state.x) - s->camera_x - 128;
        Sint32 pan = SDL_clamp((SDL_abs(screen) - 32) / 32, 0, 8);
        if (screen < 0)
            pan = -pan;
        const Sint64 dx = Distance(a, s->player) / 2;
        const Sint64 dy = (Sint64)Pixel(a->state.y) - Pixel(s->player.y);
        const Sint32 distance = (Sint32)SDL_sqrt((double)(dx * dx + dy * dy));
        s->combat.ambience_change = true;
        s->combat.ambience_volume = (Uint16)SDL_clamp(160 - distance, 0, 127);
        s->combat.ambience_pan = (Sint8)pan;
        break;
    }
    case 3:
        break;
    default:
        return SDL_SetError("Outer Wall lever entered an invalid state");
    }
    return true;
}
static bool Pressure(SotnActors *s, Actor *a)
{
    Uint16 contact = 0;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0xa04))
            return false;
        if (s->world->flags[SOTN_OUTER_PRESSURE])
        {
            a->state.y = 195 * 65536;
            a->state.step = 3;
        }
        break;
    case 1:
        a->state.y = 192 * 65536;
        if (!ActorObjectCollision(s, a, 16, 7, 4, &contact))
            return false;
        if (contact)
            ++a->state.step;
        break;
    case 2:
        if (!ActorObjectCollision(s, a, 16, 7, 4, &contact))
            return false;
        if (s->ticks % 16 == 0)
            ShiftY(a, 1);
        if (Pixel(a->state.y) > 194)
        {
            s->world->flags[SOTN_OUTER_PRESSURE] = 1;
            ++a->state.step;
        }
        if (s->ticks % 10 == 0)
            for (Uint16 i = 0; i < 2; ++i)
            {
                Actor *effect = NULL;
                if (!ActorSpawn(s, a, 6, 0x10, 160, 192, &effect))
                    return false;
                if (effect != NULL)
                {
                    effect->state.x = (Pixel(a->state.x) + (i ? 11 : -11)) * 65536;
                    effect->state.y = 188 * 65536;
                    effect->state.rotated = true;
                    effect->state.rotation = i ? 768 : -768;
                }
            }
        break;
    case 3:
        a->state.y = 195 * 65536;
        break;
    default:
        return SDL_SetError("Outer Wall pressure platform entered an invalid state");
    }
    return true;
}
bool ActorOuterLiftSwitch(SotnActors *s, Actor *a)
{
    if (a->state.id == 0x32)
        return Gear(s, a);
    if (a->state.id == 0x33)
        return Lever(s, a);
    if (a->state.id == 0x2c)
        return Pressure(s, a);
    return SDL_SetError("Unknown Outer Wall elevator mechanism identity");
}
