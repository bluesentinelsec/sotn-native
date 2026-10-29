// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/84B88.c:EntityHellfireNormalFireball/EntityHellfireBigFireball,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Tables/artwork remain private.
#include <SDL3/SDL.h>
#include <sotn/hellfire_fireball.h>
static Sint32 Fixed(Uint32 value)
{
    return value <= SDL_MAX_SINT32 ? (Sint32)value : (Sint32)((Sint64)value - 4294967296LL);
}
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits <= 32767U ? bits : (Sint32)bits - 65536);
}
static Sint32 Read32(const Uint8 *p)
{
    return Fixed((Uint32)p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24);
}
bool SotnHellfireFireball_Tick(SotnHellfireFireball *fireball, SotnHellfireFireballInput in,
                               SotnHellfireFireballEvents *out)
{
    if (!fireball || !out || fireball->removed || fireball->phase > (fireball->big ? 1 : 2) ||
        (!fireball->big && fireball->parameter > 2) ||
        (!fireball->phase && in.player_priority > 509))
        return SDL_SetError(
            "Hellfire projectile requires its actual role/parameter/phase/priority");
    SotnHellfireFireball b = *fireball;
    SotnHellfireFireballEvents e = {0};
    if (!b.phase)
    {
        if (!b.big && (!in.dra.bytes || in.dra.size < 0x10848))
            return SDL_SetError("Normal Hellfire launch requires its private three-entry vectors");
        b.flags = 0x08100000U;
        b.bank = 9;
        b.priority = (Uint16)(in.player_priority + 2U);
        b.facing_left = !in.player_facing_left;
        b.width = b.height = b.big ? 8 : 4;
        e.initialize = true;
        e.spell = 2;
        e.animation_offset = b.big ? 0x107c8U : 0x10798U;
        if (b.big)
        {
            b.y = Fixed((Uint32)b.y + (b.parameter ? 16U : (0U - 4U)) * 65536U);
            b.draw_flags |= 4U;
            b.rotation = 0;
            b.velocity_x = b.facing_left ? 16 : -16;
            if (b.parameter)
                e.sound = 0x683;
        }
        else
        {
            const Sint32 velocity = Read32(in.dra.bytes + 0x10830U + b.parameter * 4U);
            b.velocity_x = b.facing_left ? Fixed(0U - (Uint32)velocity) : velocity;
            b.velocity_y = Read32(in.dra.bytes + 0x1083cU + b.parameter * 4U);
            b.timer = 20;
            if (!b.parameter)
                e.sound = 0x660;
        }
        ++b.phase;
    }
    else if (!b.big)
    {
        if (in.hit_flags)
            b.removed = true;
        else
        {
            if (b.phase == 1)
            {
                b.timer = Word((Sint32)b.timer - 1);
                if (!b.timer)
                    ++b.phase;
                b.y = Fixed((Uint32)b.y + (Uint32)b.velocity_y);
            }
            b.x = Fixed((Uint32)b.x + (Uint32)b.velocity_x);
        }
    }
    else if (in.pose >= 23)
    {
        const bool even = (in.game_timer & 1U) == 0;
        if (even && (!in.random_ready || in.random > 32767))
            return SDL_SetError("Big Hellfire update requires its actual fifteen-bit RNG result");
        if (!(in.game_timer & 3U))
            b.rotation = Word((Sint32)b.rotation + 1024);
        b.velocity_x = Fixed((Uint32)b.velocity_x + (b.velocity_x < 0 ? (0U - 6144U) : 6144U));
        if (even)
        {
            e.random_used = true;
            if (in.random & 1U)
                e.factory = 0x00010024U;
        }
        b.x = Fixed((Uint32)b.x + (Uint32)b.velocity_x);
        b.y = Fixed((Uint32)b.y + (Uint32)b.velocity_y);
    }
    *fireball = b;
    *out = e;
    return true;
}
