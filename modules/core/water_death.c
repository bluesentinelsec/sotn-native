// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/71830.c:PlayerStepKillWater, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. No private data is embedded.
#include <SDL3/SDL_error.h>
#include <sotn/water_death.h>

static void Action(SotnWaterDeathEvents *e, SotnWaterDeathActionKind kind, Uint32 value)
{
    e->actions[e->count++] = (SotnWaterDeathAction){kind, value};
}
bool SotnWaterDeath_Tick(SotnWaterDeath *state, SotnWaterDeathInput in, SotnWaterDeathEvents *out)
{
    if (!state || !out || state->step != 13 || state->phase > 1 || in.fade > 0xfff)
        return SDL_SetError("Water death requires its actual state13 controller and fade");
    SotnWaterDeath s = *state;
    SotnWaterDeathEvents e = {0};
    s.draw_flags = 4;
    if (in.fade == 0xfff && s.phase)
    {
        s.step = 17;
        s.phase = 0;
        s.velocity_x = s.velocity_y = 0;
    }
    else if (!s.phase)
    {
        s.velocity_x = 0;
        s.velocity_y = in.tight_space ? 0 : -106496;
        Action(&e, SOTN_WATER_DEATH_SOUND, 0x6f6);
        Action(&e, SOTN_WATER_DEATH_RESET_NORMAL, 0);
        Action(&e, SOTN_WATER_DEATH_ANIMATION, 0xc1);
        s.draw_flags = 0;
        s.blend = 0x30;
        s.rotation = 512;
        Action(&e, SOTN_WATER_DEATH_GLOBAL_EFFECT, 1);
        Action(&e, SOTN_WATER_DEATH_FACTORY, 0x59002c);
        Action(&e, SOTN_WATER_DEATH_FACTORY, 0x60031);
        s.red = s.blue = 128;
        s.color_blend = true;
        s.phase = 1;
    }
    else
    {
        if (s.blue < 248)
            ++s.blue;
        if (s.red > 8)
            --s.red;
        const Uint32 velocity = (Uint32)s.velocity_y + 4096U;
        SDL_memcpy(&s.velocity_y, &velocity, sizeof(velocity));
        if (s.velocity_y > 16384)
            s.velocity_y = 4096;
        if (in.remaining < 0)
        {
            Action(&e, SOTN_WATER_DEATH_CAPTURE, 0);
            e.begin_disintegration = true;
            s.step = 16;
            ++s.phase;
        }
    }
    *state = s;
    *out = e;
    return true;
}
