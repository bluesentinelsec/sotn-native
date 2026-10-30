// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/692E8.c timer15/status/capture ordering and dra/71830.c reset hooks,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Profiles remain private.
#include "player_internal.h"

bool SotnPlayer_SetAfterimageContext(SotnPlayer *p, Uint32 generation, Uint32 timer)
{
    if (!p || !generation || !p->dra.bytes || p->dra.size < 0xcf4c ||
        (p->afterimage_generation && p->afterimage_generation != generation) ||
        (p->afterimage_ticked && timer == p->afterimage_last_clock))
        return SDL_SetError(
            "Afterimage context requires its actual stage lease and a new engine clock");
    p->afterimage_generation = generation;
    p->afterimage_clock = timer;
    p->afterimage_pending = true;
    return true;
}
bool SotnPlayer_ResetAfterimages(SotnPlayer *p, Sint32 duration)
{
    if (!p || !p->afterimage_generation)
        return SDL_SetError("Afterimage reset requires its actual common player owner");
    return SotnPlayerAfterimage_Reset(&p->afterimage, true, duration, &p->afterimage_timer15);
}
void SotnPlayer_AfterimageReset(SotnPlayer *p)
{
    if (p->afterimage_generation && !SotnPlayer_ResetAfterimages(p, 1))
        p->failed = true;
}
bool SotnPlayer_AfterimageBegin(SotnPlayer *p)
{
    if (!p->afterimage_generation)
        return true;
    if (!p->afterimage_pending)
        return SDL_SetError("Afterimage player tick requires its next engine-clock context");
    if (!SotnPlayerStep_IsDead(p->step) && p->afterimage_timer15)
    {
        if (!SotnPlayerAfterimage_Reset(&p->afterimage, false, 0, &p->afterimage_timer15))
            return false;
        if (!--p->afterimage_timer15 && !SotnPlayerAfterimage_Enable(&p->afterimage))
            return false;
    }
    return true;
}
static bool NormalStep(SotnPlayerStep step)
{
    return step == SOTN_PLAYER_STAND || step == SOTN_PLAYER_WALK || step == SOTN_PLAYER_CROUCH ||
           step == SOTN_PLAYER_JUMP || step == SOTN_PLAYER_FALL;
}
bool SotnPlayer_AfterimageCapture(SotnPlayer *p)
{
    if (!p->afterimage_generation)
        return true;
    // Status is rebuilt before animation/collision, as in the original player update.
    if (SotnPlayer_Status(p) & 0x8000000U)
        SotnPlayer_AfterimageReset(p);
    const bool recovering = p->previous_step == SOTN_PLAYER_HIT ||
                            p->previous_step == SOTN_PLAYER_STONE ||
                            p->previous_step == SOTN_PLAYER_BOSS_GRAB;
    if (recovering && NormalStep(p->step) && !p->poison_ticks && !p->curse_ticks)
        p->afterimage_timer15 = 4;
    if (p->failed)
        return false;
    if (p->step == SOTN_PLAYER_BOSS_GRAB || p->step == SOTN_PLAYER_STUCK)
        return true; // Original status40/10 bypasses both capture and display updates.
    return SotnPlayerAfterimage_Capture(&p->afterimage, p->dra,
                                        (SotnAfterimageInput){.x = p->x.value,
                                                              .y = p->y.value,
                                                              .velocity_y = p->vy,
                                                              .pressed = p->buttons,
                                                              .held = p->held,
                                                              .tapped = p->tapped,
                                                              .animation = p->animation.id,
                                                              .sprite = p->animation.sprite,
                                                              .palette = 0x8100,
                                                              .facing_left = p->facing_left});
}
bool SotnPlayer_AfterimagePresent(SotnPlayer *p)
{
    if (!p->afterimage_generation)
        return true;
    if (p->step != SOTN_PLAYER_BOSS_GRAB && p->step != SOTN_PLAYER_STUCK &&
        !SotnPlayerAfterimage_Present(&p->afterimage, p->dra, p->afterimage_clock, 148))
        return false;
    p->afterimage_pending = false;
    p->afterimage_ticked = true;
    p->afterimage_last_clock = p->afterimage_clock;
    return true;
}
