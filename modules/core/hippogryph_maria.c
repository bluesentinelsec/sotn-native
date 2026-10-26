// SPDX-License-Identifier: AGPL-3.0-or-later
// Reconstructed US BO5 func_us_801A5F88 / func_us_801A62B4 and CutsceneCameraPan,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Dialogue/poses stay private.
#include "actors_internal.h"

static bool Pan(SotnActors *s, Sint32 target)
{
    const Sint32 anchor = 384 - target;
    if (anchor < 1 || anchor > 255)
        return SDL_SetError("Hippogryph cutscene pan exceeds the supported camera anchor");
    return SotnCamera_Pan(s->camera, (Uint16)anchor);
}
static bool Reward(SotnActors *s, Actor *a)
{
    switch (a->substep)
    {
    case 0:
        // Preserve the original full scan and last match, including its per-match phase count.
        for (Uint16 i = 96; i < 128; ++i)
            if (s->slots[i].state.generation && s->slots[i].state.id == 3)
            {
                a->hippogryph.linked_slot = i;
                a->hippogryph.linked_generation = s->slots[i].state.generation;
                ++a->substep;
            }
        return true;
    case 1: {
        const Uint16 slot = a->hippogryph.linked_slot;
        if (slot < 96 || slot >= 128 || !a->hippogryph.linked_generation)
            return SDL_SetError("Hippogryph cutscene manager lost its reward reservation");
        const Actor *reward = &s->slots[slot];
        if (reward->state.generation != a->hippogryph.linked_generation || reward->state.id != 3)
        {
            s->hippogryph_encounter = 0;
            s->death_event.flags |= 0x8000U;
            ++a->substep;
        }
        return true;
    }
    case 2:
        SetStep(a, 2);
        return true;
    default:
        return true;
    }
}
static bool Manager(SotnActors *s, Actor *a)
{
    switch (a->state.step)
    {
    case 0:
        return ActorInitialize(s, a, 0x478);
    case 1:
        return Reward(s, a);
    case 2:
        s->control.pause_disabled = true;
        s->control.block_stopwatch = true;
        s->control.buttons = s->player.status & 4U ? 2 : SOTN_BUTTON_RIGHT;
        s->control.timer = 1;
        ++a->state.step;
        return true;
    case 3:
        s->control.buttons = 0;
        if (s->player.status & 4U)
        {
            if (s->service_ticks & 1U)
                s->control.buttons = 2;
        }
        else
        {
            const Sint32 x = Pixel(s->player.x);
            s->control.buttons = SOTN_BUTTON_RIGHT;
            if (x > 288)
                s->death_event.flags |= 1U;
            if (x > 320)
            {
                s->control.buttons = 0;
                s->motion.align_horizontal = true;
                s->motion.alignment_x = 320;
                s->motion.x = 0;
                s->player.x = 320 * 65536 + (Sint32)((Uint32)s->player.x & 65535U);
                ++a->state.step;
            }
            if (!Pan(s, SDL_max(256, x)))
                return false;
        }
        s->control.timer = 1;
        return true;
    case 4:
        if (!Pan(s, 320))
            return false;
        if (s->death_event.flags & 2U)
            ++a->state.step;
        break;
    case 5:
        if (!Pan(s, 256))
            return false;
        if (s->camera->anchor_x == 128)
        {
            s->control.pause_disabled = false;
            s->control.block_stopwatch = false;
            a->remove = true;
        }
        break;
    default:
        return true;
    }
    s->control.buttons = 0;
    s->control.timer = 1;
    return true;
}
static bool Move(Actor *a)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Hippogryph Maria exceeds fixed-point movement bounds");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    return true;
}
static bool Maria(SotnActors *s, Actor *a)
{
    if (s->hippogryph_skip && a->state.step < 13)
        SetStep(a, 13);
    Uint8 animation = 0;
    if (a->state.step >= 3 && a->state.step <= 12)
    {
        const size_t poses[] = {0xc24, 0xc48, 0xc5c, 0xc6c, 0xc74,
                                0xc84, 0xc90, 0xc98, 0xca8, 0xcc0};
        if (!ActorAnimate(s, a, poses[a->state.step - 3U], &animation))
            return false;
        if (s->death_event.flags & (1U << (a->state.step + 1U)))
            SetStep(a, (Uint16)(a->state.step + 1U));
        return true;
    }
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x460))
            return false;
        a->state.animation_set = 0x8004;
        a->state.frame = 1;
        a->state.texture_offset = 80;
        a->state.palette = 0x220;
        a->state.hitbox_state = 0;
        a->state.x = 448 * 65536 + (Sint32)((Uint32)a->state.x & 65535U);
        return true;
    case 1:
        if (s->death_event.flags & 4U)
            ++a->state.step;
        return true;
    case 2:
        a->state.frame = 2;
        if (s->death_event.flags & 8U)
            ++a->state.step;
        return true;
    case 13:
        if (!ActorAnimate(s, a, 0xccc, &animation))
            return false;
        if (!animation)
        {
            a->vx = 49152;
            SetStep(a, 14);
        }
        return true;
    case 14:
        if (!ActorAnimate(s, a, 0xcd4, &animation) || !Move(a))
            return false;
        if (Pixel(a->state.x) - s->camera_x > 272)
        {
            s->death_event.flags |= 2U;
            a->remove = true;
        }
        return true;
    default:
        return true;
    }
}
bool ActorHippogryphMaria(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 25 || !a->state.generation || s->camera_x < 0 ||
        s->camera_x > 16384 || s->camera_y < 0 || s->camera_y > 16384)
        return SDL_SetError("Hippogryph Maria requires bounded original stage/actor owners");
    if (a->state.id == 0x2b)
        return Manager(s, a);
    if (a->state.id == 0x2c)
        return Maria(s, a);
    return SDL_SetError("Hippogryph Maria received an unrelated original actor");
}
