// SPDX-License-Identifier: AGPL-3.0-or-later
// Original simulated input and sixteen-word player timers/exit helpers in dra/692E8.c, 6DF70.c and
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
    if (!p || input.mode > 2 || input.remaining > SDL_MAX_SINT32 || (input.buttons & ~0xf0feU) ||
        (input.mode == 1 && !p->script_executable.bytes) ||
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

static Sint32 SignedTimer(Uint16 bits)
{
    return bits < 32768 ? bits : (Sint32)bits - 65536;
}
void SotnPlayer_Protect(SotnPlayer *p, Sint16 duration)
{
    if (SignedTimer(p->invincibility) <= duration)
        p->invincibility = (Uint16)duration;
}
bool SotnPlayer_Protected(const SotnPlayer *p)
{
    return (p->invincibility_flash_ticks | p->invincibility) != 0;
}
static void RecoveryTimers(SotnPlayer *p)
{
    p->timers[SOTN_PLAYER_TIMER_COLOR] = 12;
    p->afterimage_timer15 = 12;
    SotnPlayer_Protect(p, 12);
}
bool SotnPlayer_TimersTick(SotnPlayer *p)
{
    // EntityAlucard processes each word's action, decrement and expiry before the next word.
    // The existing afterimage owner processes word15 after this loop, including unbound callers.
    for (Uint16 i = 0; i < SOTN_PLAYER_TIMER_AFTERIMAGE; ++i)
    {
        if (!p->timers[i])
            continue;
        if (i == SOTN_PLAYER_TIMER_HIT_EFFECT)
            p->palette = p->damage_palette;
        else if (i == SOTN_PLAYER_TIMER_PALETTE)
        {
            p->palette = p->gravity.timer;
            p->afterimage_timer15 = 12;
        }
        else if (i == SOTN_PLAYER_TIMER_COLOR && p->script_executable.bytes)
        {
            const Uint16 offsets[4] = {0, 512, 1536, 1024};
            for (Uint16 v = 0; v < 4; ++v)
            {
                SotnMathTrig trig = {0};
                if (!SotnMath_Trig(p->script_executable,
                                   (Sint32)((p->damage_clock & 15U) * 256U) + offsets[v], &trig))
                    return false;
                p->body_grey[v] = (Uint8)((trig.sine + 4096) / 64 + 96);
            }
            p->body_color_blend = true;
        }
        --p->timers[i]; // Raw unsigned bits give the original signed-word wrap without C overflow.
        if (p->timers[i])
            continue;
        switch (i)
        {
        case SOTN_PLAYER_TIMER_POISON:
        case SOTN_PLAYER_TIMER_CURSE:
            if (!(p->timer_status & (0x80U | 0x8000U)))
                RecoveryTimers(p);
            break;
        case SOTN_PLAYER_TIMER_HIT_EFFECT:
            p->palette = 0x8100;
            break;
        case SOTN_PLAYER_TIMER_PALETTE:
            p->palette = 0x8100;
            if (!(p->timer_status & (0x80U | 0x4000U | 0x8000U)))
                RecoveryTimers(p);
            break;
        case SOTN_PLAYER_TIMER_COLOR:
            p->body_color_blend = false;
            break;
        case SOTN_PLAYER_TIMER_FALL:
            if (p->step == SOTN_PLAYER_FALL && p->animation.id != 28)
            {
                if (!SotnAnimation_Set(&p->animation, 28))
                    return false;
                p->movement_flags &= ~0x10U;
            }
            break;
        case SOTN_PLAYER_TIMER_INVINCIBLE:
            SotnPlayer_Protect(p, 16);
            break;
        default:
            break; // Including actual impact8, weapon9, subweapon10 and Dark11/spell12 words.
        }
    }
    return true;
}

void SotnPlayer_ReactionRecovery(SotnPlayer *p)
{
    if ((p->previous_step != SOTN_PLAYER_HIT && p->previous_step != SOTN_PLAYER_SCRIPT_REACTION &&
         p->previous_step != SOTN_PLAYER_SCRIPT) ||
        (SotnPlayer_Status(p) & (0x10000U | 0x40000U)))
        return;
    SotnPlayer_Protect(p, 12);
    if (p->script_executable.bytes && !p->poison_ticks && !p->curse_ticks)
    {
        p->timers[4] = 12;
        p->afterimage_timer15 = 4;
        p->palette = 0x8100;
    }
}
