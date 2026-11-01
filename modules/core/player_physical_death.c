// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/71830.c:PlayerStepKill, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Image/SDK tables remain private.
#include "player_internal.h"

static Sint32 Add(Sint32 value, Uint32 amount)
{
    const Uint32 bits = (Uint32)value + amount;
    SDL_memcpy(&value, &bits, sizeof(value));
    return value;
}
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
void SotnPlayer_PhysicalDeathTick(SotnPlayer *p)
{
    SotnPlayerDeath d = p->death;
    Sint32 vx = p->vx, vy = p->vy;
    if (p->step != SOTN_PLAYER_KILL || !d.native || !d.generation ||
        d.generation != p->death_serial || d.variant > 3 || !p->death_capture || !p->death_decay ||
        !p->death_context || (d.phase != 1 && d.phase != 2 && d.phase != 128))
    {
        p->failed = true;
        SDL_SetError("Physical death lost its actual controller/image ownership");
        return;
    }
    if (d.phase == 1)
    {
        if (!d.variant)
        {
            if (d.red < 248)
                ++d.red;
            if (d.blue > 8)
                --d.blue;
        }
        else if (d.variant == 1 || d.variant == 2)
        {
            if (d.blue > 8)
                --d.blue;
            d.red = d.blue;
        }
        else
        {
            // Original ice branch decrements below248, including byte underflow at zero.
            if (d.red < 248)
                d.red = (Uint8)(d.red - 1U);
            if (d.blue < 248)
                ++d.blue;
        }
        vy = Add(vy, 5632U);
        if (vy > 16384)
            vy = 4096;
        if (p->animation.remaining < 0)
        {
            if (!p->death_capture(p->death_context, d.generation))
            {
                p->failed = true;
                return;
            }
            d.captured = true;
            d.phase = 2;
        }
    }
    else if (d.phase == 2)
    {
        bool finished = false;
        if (!d.captured || !p->death_decay(p->death_context, d.generation, &finished))
        {
            if (!d.captured)
                SDL_SetError("Physical death cannot decay before its actual resident capture");
            p->failed = true;
            return;
        }
        if (finished)
        {
            vy = 0;
            d.color_blend = false;
            d.phase = 128;
        }
    }
    if (vx > 0)
        vx = SDL_max(0, vx - 1024);
    else if (vx < 0)
        vx = SDL_min(0, vx + 1024);
    if (p->animation.pose >= 15)
    {
        if (p->animation.pose == 22 && p->animation.remaining == 1)
            d.rotation = Word(d.rotation - 256);
        d.rotation = (Sint16)SDL_max(-640, Word(d.rotation - 6));
    }
    p->death = d;
    p->substep = d.phase;
    p->form_rotated = true;
    p->vx = vx;
    p->vy = vy;
}
