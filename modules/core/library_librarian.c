// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/lib/e_shop.c controls/chair from sotn-decomp 7bc54d8157b6.
#include "actors_internal.h"
static void Pad(SotnActors *s, Uint16 buttons, Uint16 timer, bool hold)
{
    s->control = (SotnActorControl){.buttons = buttons, .timer = timer, .hold = hold};
}
static bool Align(SotnActors *s, Sint32 x)
{
    const Sint32 delta = x + s->camera_x - Pixel(s->player.x);
    if (delta < -64 || delta > 64)
        return SDL_SetError("Librarian alignment exceeds its bounded original entry interval");
    s->motion.x += delta * 65536;
    return true;
}
static bool Trigger(SotnActors *s, Actor *a)
{
    const Sint32 x = Pixel(s->player.x) - s->camera_x, y = Pixel(s->player.y);
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x824))
            return false;
        if (y < 256 && x > 192)
        {
            s->combat.music_change = true;
            s->combat.music = 0;
        }
        return true;
    }
    if (a->state.step == 1)
    {
        a->state.hitbox_width = 16;
        a->state.hitbox_height = 32;
        if (x > 224 && y >= 96 && y < 160)
        {
            Actor *dialogue = NULL;
            if (!ActorLibrarySpawnAt(s, a, 0x25, 0, 128, &dialogue))
                return false;
            s->death_event.library_reply = 0;
            ++a->state.step;
        }
    }
    return a->state.step <= 2 || SDL_SetError("Librarian entry trigger step is invalid");
}
static bool LeaveShop(SotnActors *s, Actor *a)
{
    if (s->death_event.flags & 0x100U)
    {
        s->death_event.flags |= 0x2000U;
        a->state.step = 16;
        return true;
    }
    return Align(s, 116);
}
static bool Manager(SotnActors *s, Actor *a)
{
    const Sint32 x = Pixel(s->player.x) - s->camera_x;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x824))
            return false;
        if (x < 256)
        {
            s->combat.sequence_change = true;
            s->combat.sequence = 0x202;
        }
        if (s->world->flags[SOTN_LIBRARY_MET])
            a->state.step = 8;
        break;
    case 1:
        Pad(s, SOTN_BUTTON_LEFT, 1, false);
        SetStep(a, 2);
        break;
    case 2:
        Pad(s, x > 232 ? SOTN_BUTTON_LEFT : 0, 1, false);
        if (x <= 232)
        {
            s->death_event.flags |= 1U;
            ++a->state.step;
            return Align(s, 232);
        }
        break;
    case 3:
        Pad(s, 0, 1, false);
        if (s->death_event.flags & 0x40U)
        {
            if (x > 116)
                s->control.buttons = SOTN_BUTTON_LEFT;
            else
            {
                ++a->state.step;
                return Align(s, 116);
            }
        }
        else
            return Align(s, 232);
        break;
    case 4:
        Pad(s, 0, 1, true);
        ++a->state.step;
        break;
    case 5:
        s->world->flags[SOTN_LIBRARY_MET] = 1;
        if (!SotnWorld_RecordTime(s->world, SOTN_TIME_LIBRARIAN, s->game_seconds))
            return false;
        s->control.timer = 1;
        ++a->state.step;
        return LeaveShop(s, a);
    case 6:
        return LeaveShop(s, a);
    case 8:
        ++a->state.step;
        SDL_FALLTHROUGH;
    case 9:
        if (x > 255)
        {
            s->combat.sequence_change = true;
            s->combat.sequence = 0;
            a->remove = true;
            return true;
        }
        if (x < 117)
        {
            switch (a->substep)
            {
            case 0:
                Pad(s, 0, 16, false);
                ++a->substep;
                s->death_event.flags |= 1U;
                break;
            case 1:
                Pad(s, SOTN_BUTTON_LEFT, 1, false);
                ++a->substep;
                break;
            case 2:
                Pad(s, 0, 128, false);
                SetStep(a, 10);
                break;
            default:
                return SDL_SetError("Librarian return-control substep is invalid");
            }
            return Align(s, 116);
        }
        break;
    case 10:
        if (!s->control.timer && (s->player.contacts & 1U))
        {
            Pad(s, 0, 1, true);
            ++a->state.step;
        }
        return Align(s, 116);
    case 11:
        Pad(s, 0, 1, true);
        if (s->death_event.flags & 0x100U)
        {
            s->death_event.flags |= 0x2000U;
            a->state.step = 16;
        }
        break;
    case 16:
        Pad(s, SOTN_BUTTON_RIGHT, 32, false);
        ++a->state.step;
        break;
    case 17:
        if (!s->control.timer)
            SetStep(a, 9);
        break;
    default:
        return SDL_SetError("Librarian player manager step is invalid");
    }
    return true;
}
static bool Gift(SotnActors *s, const Actor *a, Uint8 mask, Uint16 id, Uint16 parameters)
{
    if (s->world->flags[SOTN_LIBRARY_DROPS] & mask)
        return true;
    Actor *drop = NULL;
    if (!ActorSpawn(s, a, id, parameters, 96, 128, &drop))
        return false;
    if (drop)
    {
        drop->state.x = Pixel(a->state.x) * 65536;
        drop->state.y = Pixel(a->state.y) * 65536;
        drop->state.facing_left = false;
        s->world->flags[SOTN_LIBRARY_DROPS] |= mask;
    }
    return true;
}
static bool Chair(SotnActors *s, Actor *a)
{
    Uint8 animation = 0;
    if (Pixel(s->player.x) - s->camera_x < 56)
    {
        a->state.priority = 0xc0;
        if (a->state.step < 16 && Pixel(s->player.y) < 201 && s->player.velocity_y < 0)
        {
            SetStep(a, 16);
            if (!ActorSound(s, 0x683))
                return false;
            a->librarian.total_hits = (Uint16)(a->librarian.total_hits + 1U);
            a->librarian.consecutive_hits = (Uint16)(a->librarian.consecutive_hits + 1U);
            if (!Gift(s, a, 1, 3, 23) ||
                (a->librarian.total_hits >= 64 && !Gift(s, a, 2, 0xa, SOTN_HAND_ITEMS + 25)) ||
                (a->librarian.consecutive_hits >= 16 &&
                 !Gift(s, a, 4, 0xa, SOTN_HAND_ITEMS + 75)) ||
                (a->librarian.consecutive_hits >= 24 && s->world->flags[SOTN_INVERTED_UNLOCKED] &&
                 !Gift(s, a, 8, 0xa, SOTN_HAND_ITEMS + 23)))
                return false;
        }
    }
    else
        a->state.priority = 0x80;
    if (s->player.contacts & 1U)
        a->librarian.consecutive_hits = 0;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x824))
            return false;
        a->state.priority = 0x80;
        a->state.animation_set = 0x800b;
        a->state.frame = 2;
        a->state.palette = 0x210;
        a->state.texture_offset = 0x48;
        a->librarian.timer = 32;
        a->state.flags &= ~0x80000000U;
        s->death_event.flags = 0;
        break;
    case 1:
        if (s->death_event.flags & 2U)
        {
            a->librarian.timer = (Uint16)(a->librarian.timer - 1U);
            if (a->librarian.timer)
                SetStep(a, 2);
        }
        else if (s->death_event.flags & 0x80U)
            SetStep(a, 9);
        break;
    case 2:
    case 9:
        if (!ActorAnimate(s, a, 0x11fc, &animation))
            return false;
        if (!animation)
            SetStep(a, (Uint16)(a->state.step == 2 ? 3 : 10));
        break;
    case 3:
    case 4:
    case 5:
    case 6: {
        const size_t streams[] = {0x1204, 0x1210, 0x121c, 0x1234};
        if (!ActorAnimate(s, a, streams[a->state.step - 3], &animation))
            return false;
        if (s->death_event.flags & (1U << (a->state.step - 1)))
            SetStep(a, (Uint16)(a->state.step + 1U));
        break;
    }
    case 7:
    case 10:
        if (!ActorAnimate(s, a, a->state.step == 7 ? 0x1244 : 0x12c0, &animation))
            return false;
        if (!animation)
            SetStep(a, 11);
        break;
    case 11:
        if (s->death_event.flags & 0x800U)
        {
            SetStep(a, 12);
            a->librarian.timer = 48;
        }
        if (s->death_event.flags & 0x1000U)
            SetStep(a, 13);
        break;
    case 12:
        if (!ActorAnimate(s, a, 0x12c8, &animation))
            return false;
        a->librarian.timer = (Uint16)(a->librarian.timer - 1U);
        if (!a->librarian.timer)
        {
            s->death_event.flags &= ~0x800U;
            a->state.frame = 2;
            SetStep(a, 11);
        }
        if (s->death_event.flags & 0x1000U)
        {
            a->state.frame = 2;
            SetStep(a, 13);
        }
        break;
    case 13:
        break;
    case 16:
        if (!ActorAnimate(s, a, 0x12d0, &animation))
            return false;
        a->state.y =
            (Sint32)((Sint64)(Pixel(s->player.y) - 28) * 65536 + ((Uint32)a->state.y & 65535U));
        a->vy = s->player.velocity_y;
        if (Pixel(a->state.y) >= 172)
        {
            a->state.y = (Sint32)(172 * 65536 + ((Uint32)a->state.y & 65535U));
            a->state.frame = 2;
            SetStep(a, 1);
        }
        break;
    default:
        return SDL_SetError("Librarian chair step is invalid");
    }
    return true;
}
bool ActorLibraryLibrarian(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 2 || !s->world)
        return SDL_SetError("Librarian requires initialized Library and progress owners");
    switch (a->state.id)
    {
    case 0x23:
        return Manager(s, a);
    case 0x24:
        return Chair(s, a);
    case 0x25:
        return ActorDialogue(s, a);
    case 0x2b:
        return Trigger(s, a);
    case 0x2c: {
        Uint16 contact = 0;
        if (!a->state.step)
            return ActorInitialize(s, a, 0x824);
        return ActorObjectCollision(s, a, 8, 72, 1, &contact);
    }
    default:
        return SDL_SetError("Unknown Library NPC actor");
    }
}
