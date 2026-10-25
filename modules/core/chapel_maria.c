// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected US st/dai/e_cutscene_actors.c, sotn-decomp 7bc54d8157b6.
// Animation streams and dialogue remain private runtime inputs.
#include "actors_internal.h"
static bool Manager(SotnActors *s, Actor *a)
{
    s->control.timer = 1;
    s->control.buttons = 0;
    s->control.pause_disabled = true;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x95c))
            return false;
        s->control.block_stopwatch = true;
        s->control.buttons = SOTN_BUTTON_LEFT;
        return true;
    case 1:
        if (Pixel(s->player.x) > 368)
            s->control.buttons = SOTN_BUTTON_LEFT;
        else
        {
            s->motion.align_horizontal = true;
            s->motion.alignment_x = 368;
            s->motion.x = 0;
            s->player.x = 368 * 65536 + (Sint32)((Uint32)s->player.x & 65535U);
            s->death_event.flags |= 1U;
            ++a->state.step;
        }
        return true;
    case 2:
        if (!SotnCamera_Pan(s->camera, 176))
            return false;
        if (s->death_event.flags & 2U)
            ++a->state.step;
        return true;
    case 3:
        if (!SotnCamera_Pan(s->camera, 128))
            return false;
        if (s->camera->anchor_x == 128 && (s->death_event.flags & 8U))
        {
            s->control.pause_disabled = false;
            s->control.block_stopwatch = false;
            a->remove = true;
        }
        return true;
    default:
        return SDL_SetError("Chapel Maria manager entered invalid original state");
    }
}
static bool Move(Actor *a)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Chapel Maria exceeds fixed-point motion bounds");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    return true;
}
static void Step(Actor *a, Uint16 step)
{
    a->state.step = step;
    a->substep = a->pose = a->remaining = 0;
}
static bool Run(SotnActors *s, Actor *a)
{
    Uint8 result = 0;
    if (!ActorAnimate(s, a, 0x1030, &result))
        return false;
    if ((result & 0x80U) && (a->pose == 3 || a->pose == 7) &&
        !ActorSoundVolumePan(s, 0x64c, 80,
                             (Sint8)SDL_clamp((Pixel(a->state.x) - s->camera_x - 120) / 16, -8, 8)))
        return false;
    return Move(a);
}
static bool Maria(SotnActors *s, Actor *a)
{
    const Sint32 x = Pixel(a->state.x), y = Pixel(a->state.y);
    Uint8 animation = 0;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x968))
            return false;
        a->state.animation_set = 0x800f;
        a->state.frame = 10;
        a->state.texture_offset = 72;
        a->state.palette = 0x251;
        return true;
    case 1:
        if (s->death_event.flags & 4U)
            ++a->state.step;
        return true;
    case 2:
        if (!ActorAnimate(s, a, 0x1024, &animation))
            return false;
        if (!animation)
        {
            Step(a, 3);
            a->vx = 98304;
        }
        return true;
    case 3:
    case 5:
    case 7:
        if (!Run(s, a))
            return false;
        if (a->state.step == 3 && x > 368)
        {
            Step(a, 4);
            a->vy = -4 * 65536;
        }
        else if (a->state.step == 5 && x > 448)
        {
            Step(a, 6);
            a->vy = -4 * 65536;
            s->death_event.flags |= 8U;
        }
        else if (a->state.step == 7 && x > 544)
            a->remove = true;
        return true;
    case 4:
    case 6:
        if (!ActorAnimate(s, a, 0x1044, &animation))
            return false;
        a->vy += 12288;
        if (!Move(a))
            return false;
        if (a->vy > 0 && y > (a->state.step == 4 ? 151 : 135))
        {
            a->state.y =
                (a->state.step == 4 ? 151 : 135) * 65536 + (Sint32)((Uint32)a->state.y & 65535U);
            a->vy = 0;
            Step(a, a->state.step == 4 ? 5 : 7);
        }
        return true;
    default:
        return SDL_SetError("Chapel Maria entered invalid original state");
    }
}
bool ActorChapelMaria(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 6)
        return SDL_SetError("Chapel Maria requires its original stage owner and actor");
    if (a->state.id == 0x45)
        return ActorDialogue(s, a);
    if (a->state.id == 0x46)
        return Manager(s, a);
    if (a->state.id == 0x47)
        return Maria(s, a);
    return SDL_SetError("Chapel Maria dispatcher received an unrelated original actor");
}
