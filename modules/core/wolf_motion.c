// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/wolfForm.c:func_8012C600/func_8012CCE4 and PlayerStepMorphWolf post-step,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. No original tables/artwork embedded.
#include <sotn/player.h>
#include <sotn/wolf_motion.h>

bool SotnWolf_CaptureBounds(const SotnWolfBodyPositions *in, SotnWolfCaptureBounds *out)
{
    if (!in || !out)
        return SDL_SetError(
            "Wolf capture bounds require copied actual body positions and an output");
    Sint32 right = (Sint32)in->parts[3].x + 8, left = (Sint32)in->parts[3].x - 8;
    Sint32 top = in->parts[4].y, bottom = in->parts[4].y;
    for (Uint16 i = 4; i < 13; ++i)
    {
        right = SDL_max(right, (Sint32)in->parts[i].x + 4);
        left = SDL_min(left, (Sint32)in->parts[i].x - 4);
        top = SDL_min(top, in->parts[i].y);
        bottom = SDL_max(bottom, in->parts[i].y);
    }
    top = SDL_min(top - 4, (Sint32)in->parts[13].y - 12);
    top = SDL_min(top, (Sint32)in->parts[3].y - 8);
    bottom = SDL_max(bottom + 4, (Sint32)in->player.y + 28);
    bottom = SDL_max(bottom, (Sint32)in->parts[0].y + 28);
    *out =
        (SotnWolfCaptureBounds){.right = SDL_min(right - in->player.x, in->facing_left ? 44 : 20),
                                .bottom = SDL_min(bottom - in->player.y, 39),
                                .left = SDL_max(left - in->player.x, in->facing_left ? -20 : -44),
                                .top = SDL_max(top - in->player.y, -24)};
    return true;
}
static bool SpeedValid(Sint32 speed)
{
    return speed >= -16 * 65536 && speed <= 16 * 65536;
}
static Sint32 AngleAdd(Sint32 angle, Uint32 amount)
{
    const Uint32 bits = (Uint32)angle + amount;
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
bool SotnWolf_Jump(SotnWolfJumpFrame *frame)
{
    if (!frame || frame->substep > 9 || frame->mode > 4 || !SpeedValid(frame->velocity_x))
        return SDL_SetError("Wolf jump requires bounded original controller state");
    SotnWolfJumpFrame f = *frame;
    f.velocity_y = -229376;
    Uint16 animation = 0xe6;
    if (f.substep == 2 && f.mode == 2)
    {
        animation = 0xe7;
        const Uint32 slope = f.contacts & 0xf000U;
        if (slope == (f.facing_left ? 0xc000U : 0x8000U))
            f.velocity_y = -(SDL_abs(f.velocity_x) + 229376);
        if (slope == (f.facing_left ? 0x8000U : 0xc000U))
            f.velocity_y = -32768;
        f.mode = 2;
    }
    else if ((f.buttons & (SOTN_BUTTON_LEFT | SOTN_BUTTON_RIGHT)) &&
             (f.substep != 2 || f.mode != 0) && f.substep != 9)
    {
        animation = 0xe7;
        f.mode = 1;
        if (f.buttons & SOTN_BUTTON_UP)
            f.velocity_y = -319488;
    }
    else
    {
        f.mode = 0;
        if (f.buttons & SOTN_BUTTON_UP)
            f.velocity_y = -319488;
    }
    if (!SotnAnimation_Set(&f.animation, animation))
        return false;
    f.substep = 4;
    f.body_angle = AngleAdd(f.body_angle, 128);
    *frame = f;
    return true;
}
bool SotnWolf_Cadence(Sint32 velocity_x, Sint32 body_angle, SotnWolfCadence *out)
{
    if (!out || !SpeedValid(velocity_x))
        return SDL_SetError("Wolf cadence requires bounded original speed and an output");
    const Sint32 speed = SDL_abs(velocity_x);
    Uint16 duration = 4;
    if (speed >= 4 * 65536)
        --duration;
    if (speed >= 5 * 65536)
        --duration;
    if (speed >= 6 * 65536)
        --duration;
    SotnWolfCadence next = {.body_angle = SDL_clamp(body_angle, 0x600, 0xa00)};
    for (Uint16 i = 0; i < 8; ++i)
        next.durations[i] = duration;
    *out = next;
    return true;
}
