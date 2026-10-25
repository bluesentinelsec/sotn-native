// SPDX-License-Identifier: AGPL-3.0-or-later
// DAI e_chair.c/e_end_chime.c, sotn-decomp 7bc54d8157b6. Profiles remain private.
#include "chapel_confessional_internal.h"
bool ConfessionalRead(SotnActors *s, Uint16 parameters, ConfessionalProfile *out)
{
    SotnDataView record = {0};
    if ((parameters & ~0x101U) || !Slice(s->nz0, 0xe40U + (parameters & 1U) * 16U, 16, &record))
        return SDL_SetError("Confessional ghost has invalid private variant/profile");
    const Uint32 address = Read32(record.bytes + 12);
    ConfessionalProfile p = {.left = Signed16(record.bytes),
                             .edge = Signed16(record.bytes + 2),
                             .closed = Signed16(record.bytes + 4),
                             .speed = Signed16(record.bytes + 6),
                             .target = Signed16(record.bytes + 8),
                             .curtain_speed = Signed16(record.bytes + 10)};
    if ((address != 0x80180e20 && address != 0x80180e30) || p.left < 0 || p.left > 256 ||
        p.edge < 0 || p.edge > 256 || p.closed < 0 || p.closed > 256 || p.target < 0 ||
        p.target > 256 || (p.speed != -16384 && p.speed != 16384) ||
        (p.curtain_speed != -4 && p.curtain_speed != 4))
        return SDL_SetError("Confessional ghost profile exceeds bounded motion or curtain data");
    p.animations = address - 0x80180000U;
    *out = p;
    return true;
}
static bool Chair(SotnActors *s, Actor *a)
{
    const Uint16 kind = a->state.parameters & 255U, side = a->state.parameters >> 8;
    if (kind > 4 || side > 2)
        return SDL_SetError("Chapel chair has an invalid pose or confessional side");
    switch (a->state.step)
    {
    case 0:
        if (side)
        {
            Actor *child = NULL;
            if (!ActorOwnedSpawnAt(s, a, 0x23, side == 2 ? 1 : 0, (Uint32)a->slot + 1, &child))
                return false;
            child->state.x = (s->camera_x + (side == 2 ? 64 : 176)) * 65536;
            child->state.y = (s->camera_y + 128) * 65536;
            if (ActorRandom(s) & 1U)
                child->state.parameters |= 0x100;
        }
        return ActorInitialize(s, a, 0x98c);
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
        {
            if (side)
            {
                if ((Uint32)a->slot + 1 >= SOTN_STAGE_ACTOR_SLOTS)
                    return SDL_SetError("Confessional chair child slot exceeds its owner");
                Actor *child = &s->slots[a->slot + 1U];
                if (child->state.generation &&
                    (child->state.id != 0x23 || child->enemy_state.owner_slot != a->slot ||
                     child->enemy_state.owner_generation != a->state.generation))
                    return SDL_SetError("Confessional chair lost its original owned ghost");
                if (child->state.generation && child->state.step == 1)
                    ConfessionalStep(child, 2);
            }
            ++a->state.step;
        }
        if (!ConfessionalSeated(s))
            a->state.step = 1;
        return true;
    case 4:
        s->motion.chair_kind = kind;
        if (!ConfessionalSeated(s))
            a->state.step = 1;
        return true;
    default:
        return SDL_SetError("Chapel chair entered an invalid original state");
    }
}
static bool EndChime(SotnActors *s, Actor *a)
{
    if (!s->confessional_chime)
    {
        a->remove = true;
        return true;
    }
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x968))
            return false;
        s->combat.sequence_change = true;
        s->combat.sequence = 0;
        a->state.step = 2;
        if (s->music_playing)
        {
            s->combat.cd_command = 0x80;
            ++a->state.step;
        }
        return true;
    case 1:
        if (!s->music_playing)
            ++a->state.step;
        return true;
    case 2:
        s->combat.restart_stage_music = true;
        s->confessional_chime = false;
        ++a->state.step;
        return true;
    case 3:
        return true;
    default:
        return SDL_SetError("Confessional end chime entered an invalid original state");
    }
}
bool ActorChapelConfessional(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 6)
        return SDL_SetError("Confessional requires its original Chapel owner and actor");
    if (a->state.id == 0x22)
        return Chair(s, a);
    if (a->state.id == 0x23)
        return ConfessionalGhost(s, a);
    if (a->state.id == 0x24 || a->state.id == 0x25)
        return ConfessionalBlades(s, a);
    if (a->state.id == 0x2d)
        return EndChime(s, a);
    return SDL_SetError("Confessional actor has an invalid original identity");
}
