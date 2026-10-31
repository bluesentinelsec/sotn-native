// SPDX-License-Identifier: AGPL-3.0-or-later
// Original simulated input and timer2/4/exit helpers in dra/692E8.c, 6DF70.c and
// PlayerStepUnk48/49 in 71830.c, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
#include "player_internal.h"

bool SotnPlayer_SetScriptContext(SotnPlayer *p, SotnDataView executable)
{
    if (!p || !executable.bytes || executable.size < 0x1dbcc + 2050)
        return SDL_SetError("Script presentation requires its actual private US SDK table");
    p->script_executable = executable;
    return true;
}

bool SotnPlayer_SetSimulatedPad(SotnPlayer *p, SotnPlayerSimulatedPad input)
{
    if (!p || input.mode > 2 || input.remaining > SDL_MAX_SINT32 ||
        (input.buttons & (Uint16)~0xf0feU) || (input.mode == 1 && !p->script_executable.bytes) ||
        ((input.remaining || input.mode) &&
         (!p->hp || SotnPlayerStep_IsDead(p->step) || p->step == SOTN_PLAYER_BOSS_GRAB ||
          p->grab.state != SOTN_GRAB_NONE)))
        return SDL_SetError("Simulated input requires a living owner and supported original mode");
    p->simulated_pad = input;
    p->script_hold = input.mode == 2;
    p->legacy_script_hold = false;
    return true;
}

void SotnPlayer_HoldInput(SotnPlayer *p, Uint32 ticks)
{
    p->simulated_pad = (SotnPlayerSimulatedPad){.remaining = ticks};
    p->script_hold = false;
    p->legacy_script_hold = false;
}

bool SotnPlayer_ScriptTimers(SotnPlayer *p)
{
    if (!p->script_executable.bytes)
        return true;
    if (p->timers[2] && p->damage_palette)
        p->palette = p->timers[2] == 1 ? 0x8100 : p->damage_palette;
    if (p->timers[4])
    {
        const Uint16 offsets[4] = {0, 512, 1536, 1024};
        for (Uint16 i = 0; i < 4; ++i)
        {
            SotnMathTrig trig = {0};
            if (!SotnMath_Trig(p->script_executable,
                               (Sint32)((p->damage_clock & 15U) * 256U) + offsets[i], &trig))
                return false;
            p->body_grey[i] = (Uint8)((trig.sine + 4096) / 64 + 96);
        }
        p->body_color_blend = p->timers[4] != 1;
    }
    return true;
}

void SotnPlayer_ScriptRecovery(SotnPlayer *p)
{
    if ((p->previous_step != SOTN_PLAYER_SCRIPT_REACTION &&
         p->previous_step != SOTN_PLAYER_SCRIPT) ||
        (SotnPlayer_Status(p) & (0x10000U | 0x40000U)))
        return;
    p->invincibility = (Uint16)SDL_max(p->invincibility, 12);
    if (p->script_executable.bytes && !p->poison_ticks && !p->curse_ticks)
    {
        p->timers[4] = 12;
        p->afterimage_timer15 = 4;
        p->palette = 0x8100;
    }
}
