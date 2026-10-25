// SPDX-License-Identifier: AGPL-3.0-or-later
// st/nz0/maria.c at sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Streams, portraits, dialogue and speech remain in private runtime files.
#include "actors_internal.h"

static bool Manager(SotnActors *s, Actor *a)
{
    s->control = (SotnActorControl){.timer = 2};
    s->motion.protect = true;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0xbec))
            return false;
        s->control.buttons = SOTN_BUTTON_LEFT;
        break;
    case 1:
        if (Pixel(s->player.x) - s->camera_x < 176)
            ++a->state.step;
        else
            s->control.buttons = SOTN_BUTTON_LEFT;
        break;
    case 2:
        if (s->death_event.flags & 0x2000)
        {
            s->control = (SotnActorControl){0};
            a->remove = true;
        }
        break;
    default:
        return SDL_SetError("Maria player manager entered invalid state %u", a->state.step);
    }
    return true;
}

static bool Maria(SotnActors *s, Actor *a)
{
    Uint8 animation = 0;
    const Sint32 x = Pixel(a->state.x) - s->camera_x;
    Actor *door = &s->slots[128];
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0xbec))
            return false;
        a->state.animation_set = 0x800f;
        a->state.frame = 1;
        a->state.texture_offset = 0x48;
        a->state.palette = 0x237;
        a->state.x = (s->camera_x - 16) * 65536;
        break;
    case 1:
        if (s->death_event.flags & 4)
        {
            if (door->state.id != 5 || door->state.generation == 0 || door->state.step != 1)
                return SDL_SetError("Maria entrance requires the original initialized red door");
            if (!ActorSound(s, 0x642))
                return false;
            door->state.frame = 0;
            door->state.step = 2;
            ++a->state.step;
        }
        break;
    case 2:
        if (door->state.step == 3)
        {
            a->vx = 49152;
            ++a->state.step;
        }
        break;
    case 3:
    case 4:
        if (!ActorAnimate(s, a, 0x1474, &animation))
            return false;
        a->state.x += a->vx;
        if (a->state.step == 3 && x > 24)
        {
            s->death_event.flags |= 2;
            door->state.step = 5;
            ++a->state.step;
        }
        else if (a->state.step == 4 && x > 80)
        {
            s->death_event.flags |= 1;
            a->vx = 0;
            SetStep(a, 5);
        }
        break;
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13: {
        /* These are offsets into the private original animation streams, not frames. */
        const size_t streams[] = {0x1490, 0x14a0, 0x14bc, 0x14c8, 0x14e8,
                                  0x14f8, 0x1510, 0x151c, 0x1530};
        if (!ActorAnimate(s, a, streams[a->state.step - 5], &animation))
            return false;
        if (s->death_event.flags & (1U << (a->state.step - 2)))
        {
            const Uint16 before = a->state.step;
            SetStep(a, (Uint16)(before + 1));
            if (before == 5 && !ActorAnimate(s, a, streams[1], &animation))
                return false;
            if (before == 13)
                a->vx = 49152;
        }
        break;
    }
    case 14:
    case 15:
        if (!ActorAnimate(s, a, 0x1538, &animation))
            return false;
        a->state.x += a->vx;
        if (a->state.step == 14 && x > 224)
        {
            s->death_event.flags |= 0x1000;
            ++a->state.step;
        }
        else if (a->state.step == 15 && x > 272)
            a->remove = true;
        break;
    default:
        return SDL_SetError("Maria entered invalid state %u", a->state.step);
    }
    return true;
}

bool ActorMaria(SotnActors *s, Actor *a)
{
    if (s->stage_id != 12)
        return SDL_SetError("Laboratory Maria requires its original stage profile");
    switch (a->state.id)
    {
    case 0x4b:
        return ActorDialogue(s, a);
    case 0x4c:
        return Maria(s, a);
    case 0x4d:
        return Manager(s, a);
    default:
        return SDL_SetError("Maria dispatcher received unrelated actor %u", a->state.id);
    }
}
