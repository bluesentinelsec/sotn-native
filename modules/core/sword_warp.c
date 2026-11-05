// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/704D0.c:CheckQuarterCircleForwardInput and weapon/w_043.c callbacks04/08,
// sotn-decomp7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Runtime tables stay private.
#include <SDL3/SDL.h>
#include <sotn/sword_warp.h>

static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint32 Add(Sint32 a, Sint32 b)
{
    const Uint32 bits = (Uint32)a + (Uint32)b;
    Sint32 value;
    SDL_memcpy(&value, &bits, sizeof(value));
    return value;
}
void SotnWeaponCommand_Tick(SotnWeaponCommand *c, Uint16 buttons, bool left, bool cramped)
{
    const Uint16 direction = buttons & 0xf000U, forward = c->facing_left ? 0x8000 : 0x2000;
    if (c->phase == 0)
        c->facing_left = left;
    const bool matched = (c->phase == 0 && direction == 0x4000) ||
                         (c->phase == 1 && direction == (0x4000 | forward)) ||
                         (c->phase == 2 && direction == forward);
    if (c->phase <= 2 && matched)
    {
        c->timer = 20;
        c->phase = c->phase == 2 ? 255 : Word(c->phase + 1);
    }
    else if (c->phase <= 2 || c->timer)
    {
        if (c->phase > 2 && cramped)
            c->phase = 0;
        else if ((c->timer = Word(c->timer - 1)) == 0)
        {
            if (c->phase > 2)
                c->facing_left = left;
            c->phase = 0;
        }
    }
}
void SotnWeaponBackForward_Tick(SotnWeaponCommand *c, Uint16 buttons, bool left, bool cramped)
{
    const Uint16 direction = buttons & 0xf000U;
    if (c->phase == 0)
    {
        c->facing_left = left;
        if (direction == (left ? 0x2000 : 0x8000))
        {
            c->timer = 15;
            c->phase = 1;
        }
        else if ((c->timer = Word(c->timer - 1)) == 0)
            c->phase = 0;
    }
    else if (c->phase == 1)
    {
        if (direction == (c->facing_left ? 0x8000 : 0x2000))
        {
            c->timer = 15;
            c->phase = 255;
        }
        else if ((c->timer = Word(c->timer - 1)) == 0)
            c->phase = 0;
    }
    else if (cramped)
        c->phase = 0;
    else if (c->timer && (c->timer = Word(c->timer - 1)) == 0)
    {
        c->phase = 0;
        c->facing_left = left;
    }
}
bool SotnSwordWarp_Tick(SotnSwordWarp *state, Uint16 pose, Sint32 remaining, Uint16 frame,
                        SotnSwordWarpEvents *out)
{
    if (!state || !out || state->phase > 3 || state->removed || remaining < -1 || remaining > 32767)
        return SDL_SetError("Sword warp requires its actual bounded slot16 cursor");
    SotnSwordWarp s = *state;
    SotnSwordWarpEvents e = {0};
    s.frame = (Uint16)(frame + 0x8000U);
    switch (s.phase)
    {
    case 0:
        s.blend = 0x10;
        s.velocity_x = s.facing_left ? -851968 : 851968;
        s.velocity_y = -8192;
        ++s.phase;
        break;
    case 1:
    case 3:
        if (s.phase == 3)
            s.blend = 0x10;
        s.x = Add(s.x, s.velocity_x);
        s.y = Add(s.y, s.velocity_y);
        s.velocity_x =
            s.velocity_x < 0 ? SDL_min(0, s.velocity_x + 30720) : SDL_max(0, s.velocity_x - 30720);
        if (s.phase == 1 && pose == 5 && remaining == 1)
        {
            s.facing_left = !s.facing_left;
            e.strike = true;
            ++s.phase;
        }
        else if (s.phase == 3 && pose == 19)
            s.removed = true;
        break;
    case 2:
        s.blend = 0;
        if (pose == 14)
        {
            s.facing_left = !s.facing_left;
            s.velocity_x = s.facing_left ? 851968 : -851968;
            s.velocity_y = 8192;
            ++s.phase;
        }
        break;
    }
    *state = s;
    *out = e;
    return true;
}
bool SotnSwordStrike_Tick(SotnSwordStrike *state, SotnDataView overlay, Uint16 *sound)
{
    if (!state || !sound || state->phase > 1 || state->removed || state->pose > 20 ||
        state->remaining < -1 || !overlay.bytes || overlay.size < 0xf40 + 21 * 4)
        return SDL_SetError("Sword strike requires its actual child and private weapon43 timeline");
    SotnSwordStrike s = *state;
    Uint16 request = 0;
    if (!s.phase)
        s.phase = 1;
    else
    {
        request = s.pose == 1 || s.pose == 11 ? 0x6a8 : 0;
        s.height = (Uint16)(s.frame - 0x24U) < 6 ? 8 : 0;
        if (s.remaining < 0)
            s.removed = true;
    }
    if (!s.removed)
    {
        if (s.remaining > 0)
        {
            --s.remaining;
            if (!s.remaining)
                ++s.pose;
        }
        if (s.pose > 20)
            return SDL_SetError("Sword strike exceeded its private end record");
        const Uint8 *record = overlay.bytes + 0xf40 + s.pose * 4U;
        const Uint16 duration = (Uint16)(record[0] | (Uint16)record[1] << 8),
                     frame = (Uint16)(record[2] | (Uint16)record[3] << 8);
        if (duration == 0xffff)
        {
            if (!s.pose)
                return SDL_SetError("Sword strike cannot end before its first image");
            --s.pose;
            s.remaining = -1;
            const Uint8 *last = overlay.bytes + 0xf40 + s.pose * 4U;
            s.frame = (Uint16)((last[2] | (Uint16)last[3] << 8) & 511U);
        }
        else if (!duration || duration > 32767 || frame > 511)
            return SDL_SetError("Sword strike has an invalid private pose record");
        else
        {
            if (!s.remaining)
                s.remaining = (Sint16)duration;
            s.frame = frame & 511U;
        }
    }
    *state = s;
    *sound = request;
    return true;
}
