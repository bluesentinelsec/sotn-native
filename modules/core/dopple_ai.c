// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted from sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5,
// src/boss/bo4/doppleganger.c:DopplegangerThinking (US branch).
#include <SDL3/SDL.h>
#include <sotn/combat.h>
#include <sotn/dopple_ai.h>
typedef enum
{
    THINK_STEP_0 = 0,
    THINK_STEP_1,
    THINK_STEP_2,
    THINK_STEP_3,
    THINK_STEP_4,
    THINK_STEP_5,
    THINK_STEP_6,
    THINK_STEP_7,
    THINK_STEP_14 = 14,
    THINK_STEP_15,
    THINK_STEP_GAME_OVER,
    THINK_STEP_17,
    THINK_STEP_BACKDASH,
    THINK_STEP_WAIT,
} ThinkStep;

static void Think(const SotnDoppleObservation *observation, Uint32 *random, SotnDoppleAI *ai)
{
    Sint32 world_x = 0;
    Sint32 distance_x = 0;
    bool toward_left = false;

    world_x = observation->world_x;
    ai->buttons = 0;

    if (observation->player_status & 0x40000U)
    {
        ai->step = THINK_STEP_GAME_OVER;
    }
    if (observation->boss_status & 0x10000U)
    {
        ai->step = THINK_STEP_0;
    }
    if (observation->boss_status & 0x80U)
    {
        ai->step = THINK_STEP_15;
    }

    if ((observation->player_casting) && (ai->step != THINK_STEP_GAME_OVER) &&
        !(observation->boss_status & 2U))
    {
        ai->buttons = 2U;
        ai->timer = 64;
        ai->step = THINK_STEP_14;
    }

    toward_left = false;
    if ((observation->boss_x - observation->player_x) >= 0)
    {
        toward_left = true;
    }

    distance_x = SDL_abs(observation->boss_x - observation->player_x);

    switch (ai->step)
    {
    case THINK_STEP_0:
        if (!(observation->boss_status & 0x10000U))
        {
            if (observation->player_subweapon_starting)
            {
                if (SotnCombat_Random(random) & 1)
                {
                    if (toward_left)
                    {
                        ai->buttons = 0x8000U;
                    }
                    else
                    {
                        ai->buttons = 0x2000U;
                    }
                    ai->buttons |= 0x40U;
                    ai->step = THINK_STEP_7;
                }
                else
                {
                    ai->buttons |= 0x4000U;
                    ai->step = THINK_STEP_5;
                    ai->timer = 72;
                }
            }
            else if (SDL_abs(observation->boss_y - observation->player_y) > 32)
            {
                if (toward_left)
                {
                    ai->buttons = 0x8000U;
                }
                else
                {
                    ai->buttons = 0x2000U;
                }
                ai->buttons |= 0x40U;
                ai->step = THINK_STEP_7;
            }
            else if (distance_x > 80)
            {
                if (toward_left)
                {
                    ai->buttons = 0x8000U;
                }
                else
                {
                    ai->buttons = 0x2000U;
                }
            }
            else
            {
                ai->timer = 16;
                ai->step = THINK_STEP_1;
            }
        }
        break;

    case THINK_STEP_1:
        if (observation->facing_left != toward_left)
        {
            if (toward_left)
            {
                ai->buttons = 0x8000U;
            }
            else
            {
                ai->buttons = 0x2000U;
            }
        }
        if (distance_x > 80)
        {
            ai->step = THINK_STEP_0;
        }
        else
        {
            if ((world_x < 96 && observation->facing_left == false) ||
                (world_x > 416 && observation->facing_left))
            {
                ai->timer = 64;
                ai->buttons |= 8U;
                ai->step = THINK_STEP_3;
                break;
            }

            if ((observation->player_weapon_starting) ||
                ((observation->player_status & 0x20U) && (SotnCombat_Random(random) & 1)))
            {
                switch (SotnCombat_Random(random) & 7)
                {
                case 0:
                case 6:
                case 7:
                    ai->buttons |= 0x40U;
                    ai->step = THINK_STEP_6;
                    ai->timer = 72;
                    break;
                case 1:
                    ai->buttons |= 0x4000U;
                    ai->step = THINK_STEP_5;
                    ai->timer = 72;
                    break;
                case 2:
                    if (observation->player_status & 0x20U)
                    {
                        ai->buttons |= 0x4000U;
                        ai->step = THINK_STEP_5;
                        ai->timer = 72;
                        break;
                    }
                    /* Fall through: standing opponents permit the backdash. */
                case 3:
                case 4:
                case 5:
                    ai->buttons |= 0x10U;
                    ai->step = THINK_STEP_2;
                    ai->timer = 32;
                    break;
                }
            }
            else if (observation->player_subweapon_starting)
            {
                if (SotnCombat_Random(random) & 1)
                {
                    if (toward_left)
                    {
                        ai->buttons = 0x8000U;
                    }
                    else
                    {
                        ai->buttons = 0x2000U;
                    }
                    ai->buttons |= 0x40U;
                    ai->step = THINK_STEP_7;
                }
                else
                {
                    ai->buttons |= 0x4000U;
                    ai->step = THINK_STEP_5;
                    ai->timer = 72;
                }
            }
            else
            {
                if (distance_x < 56)
                {
                    if (ai->attack_timer)
                    {
                        ai->attack_timer--;
                    }
                    else
                    {
                        /* Preserve the original repeated close-range attack lock. */
                        ai->buttons |= 0x80U;
                        ai->attack_timer = 32;
                    }
                }
                else if (--ai->timer == 0)
                {
                    ai->buttons |= 0x1000U | 0x80U;
                    ai->timer = 112;
                    ai->attack_timer = 32;
                }
            }
        }
        break;
    case THINK_STEP_2:
        if (--ai->timer == 0)
        {
            ai->buttons = 0x1000U | 0x20U;
            ai->step = THINK_STEP_0;
            ai->timer = 112;
        }
        break;

    case THINK_STEP_3:
        if (observation->substep)
        {
            ai->buttons = 0x80U;
            ai->timer = 24;
            ai->step = THINK_STEP_4;
        }
        break;

    case THINK_STEP_4:
        if (!(observation->tick % 2))
        {
            ai->buttons = 0x1000U;
        }

        if (--ai->timer == 0)
        {
            ai->buttons = 8U;
            ai->step = THINK_STEP_0;
        }
        break;

    case THINK_STEP_5:
        if ((--ai->timer == 0) || !(observation->boss_status & 0x20U))
        {
            ai->step = THINK_STEP_0;
            ai->attack_timer = 32;
        }
        if (observation->facing_left != toward_left)
        {
            if (toward_left)
            {
                ai->buttons = 0x8000U;
            }
            else
            {
                ai->buttons = 0x2000U;
            }
        }
        ai->buttons |= 0x4000U;
        if (distance_x < 56)
        {
            if (ai->attack_timer)
            {
                ai->attack_timer--;
            }
            else
            {

                ai->buttons |= 0x4000U | 0x80U;

                ai->attack_timer = 32;
            }
        }
        break;
    case THINK_STEP_6:
        if (observation->contact & 1U)
        {
            ai->step = THINK_STEP_0;
        }
        else
        {
            ai->buttons |= 0x40U;
            if (!(observation->tick % 32))
            {
                ai->buttons |= 0x1000U | 0x20U;
            }
        }
        break;
    case THINK_STEP_7:
        if (observation->contact & 1U)
        {
            ai->step = THINK_STEP_0;
        }
        else
        {
            if (distance_x > 16)
            {
                if (observation->facing_left)
                {
                    ai->buttons = 0x8000U;
                }
                else
                {
                    ai->buttons = 0x2000U;
                }
            }

            ai->buttons |= 0x40U;
            if (observation->velocity_y > 0)
            {
                if (observation->velocity_y > 131072)
                {
                    ai->buttons |= 0x4000U | 0x80U | 0x40U;
                }

                if (SDL_abs(observation->boss_y - observation->player_y) < 16)
                {
                    ai->buttons |= 0x4000U | 0x80U;
                }
            }
        }
        break;
    case THINK_STEP_14:
        if (!(observation->tick % 8))
        {
            ai->buttons = 0x1000U;
        }
        if (!(observation->boss_status & 2U) && (observation->tick & 1))
        {
            ai->buttons = 2U;
        }

        if (--ai->timer == 0)
        {
            if (observation->boss_status & 2U)
            {
                ai->buttons = 2U;
            }
            ai->step = THINK_STEP_0;
        }
        break;
    case THINK_STEP_15:
        if (!(observation->boss_status & 0x80U))
        {
            ai->step = THINK_STEP_0;
        }
        if (!(observation->tick & 0xF))
        {
            ai->buttons = 0x1000U;
        }
        break;
    case THINK_STEP_GAME_OVER:
        if (!(observation->player_status & 0x40000U))
        {
            ai->step = THINK_STEP_0;
        }
        ai->buttons = 0x1000U;

        if (observation->boss_status & 1U)
        {
            ai->buttons = 8U;
        }
        if (observation->boss_status & 2U)
        {
            ai->buttons = 2U;
        }
        break;
    case THINK_STEP_17:
        if (SDL_abs(observation->boss_y - observation->player_y) < 8)
        {
            ai->step = THINK_STEP_0;
        }
        break;
    case THINK_STEP_BACKDASH:
        if (observation->step == 1)
        {

            ai->buttons |= 0x10U;
            ai->step = THINK_STEP_WAIT;
            ai->attack_timer = 64;
        }
        break;
    case THINK_STEP_WAIT:
        if (--ai->attack_timer == 0)
        {
            ai->step = THINK_STEP_0;
        }
        break;
    }
}

bool SotnDoppleAI_Init(SotnDoppleAI *out)
{
    if (out == NULL)
        return SDL_SetError("Initializing Doppleganger thinking requires a writable cursor");
    *out = (SotnDoppleAI){.step = THINK_STEP_BACKDASH};
    return true;
}

bool SotnDoppleAI_Tick(const SotnDoppleObservation *observation, Uint32 *libc_random,
                       SotnDoppleAI *ai)
{
    if (observation == NULL || libc_random == NULL || ai == NULL ||
        (ai->step > 7 && (ai->step < 14 || ai->step > 19)) || ai->timer < 0 || ai->timer > 32767 ||
        ai->attack_timer < 0 || ai->attack_timer > 32767 || observation->world_x < -32768 ||
        observation->world_x > 32767 || observation->velocity_y < -2097152 ||
        observation->velocity_y > 2097152 || observation->step > 17)
        return SDL_SetError("Doppleganger thinking received invalid observations or cursor");
    SotnDoppleAI next = *ai;
    Uint32 random = *libc_random;
    Think(observation, &random, &next);
    if (next.timer < 0 || next.attack_timer < 0)
        return SDL_SetError("Doppleganger thinking reached an invalid timer transition");
    *ai = next;
    *libc_random = random;
    return true;
}
