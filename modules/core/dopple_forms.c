// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted from sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5,
// src/boss/bo4/unk_465F0.c:bat/mist form and unmorph controls.
#include "dopple_body_internal.h"
static Sint32 BatFinished(DoppleFrame *frame)
{
    if (frame->body.substep == 0)
    {
        return false;
    }
    if (frame->body.tapped & 8U)
    {
        DoppleSetStep(frame, 10);
        DoppleSetAnimation(frame, 202);
        frame->body.form_pose = 6;
        frame->body.palette = 0x820d;
        frame->body.form_phase = 0;
        frame->body.form_counter = 0;
        DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 44, 0x21);
        frame->body.motion.vy =
            (Sint32)(frame->body.motion.vy >= 0 ? frame->body.motion.vy / 2
                                                : -((-(Sint64)frame->body.motion.vy + 1) / 2));
        return true;
    }
    return false;
}

static void RotateToward(DoppleFrame *frame, Sint16 arg0)
{
    if (frame->body.rotation < arg0)
    {
        frame->body.rotation = (Sint16)(frame->body.rotation + 16);
        if (frame->body.rotation > arg0)
        {
            frame->body.rotation = arg0;
        }
    }
    if (frame->body.rotation > arg0)
    {
        frame->body.rotation = (Sint16)(frame->body.rotation - 16);
        if (frame->body.rotation < arg0)
        {
            frame->body.rotation = arg0;
        }
    }
}

static Sint32 WingSmash(DoppleFrame *frame)
{

    if (frame->body.tapped & 0x80U)
    {
        return true;
    }
    return false;
}

static void Bat(DoppleFrame *frame)
{

    Sint32 pressingCross = 0;
    Sint16 x_offset = 0;
    Uint32 directionsPressed = 0;

    if (BatFinished(frame))
    {
        return;
    }

    directionsPressed = frame->body.pressed & (0x1000U | 0x2000U | 0x4000U | 0x8000U);
    pressingCross = frame->body.pressed & 0x40U;
    frame->body.rotated = true;

    if (WingSmash(frame) && (frame->body.substep))
    {
        DoppleSetAnimation(frame, 0xC6);
        DoppleSpeed(frame, 393216);
        frame->body.substep = 3;
        DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 44, 0x5c);
        DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 67, 0);
        frame->body.wing_timer = 0x40;
    }

    switch (frame->body.substep)
    {
    case 0:
        frame->body.rotation = 0;
        frame->body.jump_flags = frame->body.attack_state = 0;
        frame->body.busy = false;
        if (frame->body.mist_active)
        {
            if (frame->body.mist_step != 5)
            {
                if (frame->body.mist_step < 3)
                {
                    frame->body.attack_state = 0;
                    frame->body.mist_step = 3;
                }
                return;
            }
            else
            {
                DoppleCancelAttack(frame);
                DoppleSetAnimation(frame, 0xC3);
            }
        }
        else
        {
            if (frame->body.form_phase == 0)
            {
                DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 44, 0x20);

                DoppleCancelAttack(frame);
                frame->body.form_phase++;
            }
            DoppleDecelerate(frame, 1152);
            DoppleDecelerateY(frame, 1152);
            if (SDL_abs(frame->body.motion.vy) > 81920)
            {
                if (frame->body.motion.vy > 0)
                {
                    frame->body.motion.vy = 81920;
                }
                else
                {
                    frame->body.motion.vy = -81920;
                }
            }
            DoppleSetAnimation(frame, 0xCA);
            frame->body.form_pose = frame->body.animation.sprite;
            frame->body.palette = 0x820d;
            if (frame->body.form_phase == 1)
            {
                return;
            }
            if (frame->body.form_phase == 2)
            {
                frame->body.animation_set = 0x8002;
                frame->body.form_pose = 6;
                return;
            }
        }
        DoppleSetAnimation(frame, 0xC3);
        frame->body.animation.remaining = 1;
        frame->body.animation.pose = 2;
        frame->body.palette = 0x8200;
        DoppleDirection(frame);
        frame->body.substep++;
        break;
    case 1:
        if (directionsPressed && !pressingCross)
        {
            if (frame->body.animation.id == 0xC3)
            {
                frame->body.animation.pose /= 3;
            }
            frame->body.substep++;
        }
        else
        {
            RotateToward(frame, 0);
            DoppleDecelerate(frame, 4608);
            DoppleDecelerateY(frame, 4608);
            break;
        }
        SDL_FALLTHROUGH;
    case 2:

        if (pressingCross)
        {
            directionsPressed = 0;
        }
        switch (directionsPressed)
        {
        case 0:
        default:
            DoppleSetAnimation(frame, 0xC3);
            frame->body.substep = 1;
            break;
        case 0x1000U:
            frame->body.animation.id = 0xC2;
            if (frame->body.motion.vy < -81920)
            {
                DoppleDecelerateY(frame, 4608);
            }
            else
            {
                frame->body.motion.vy = -81920;
            }
            RotateToward(frame, -0x80);
            DoppleDecelerate(frame, 4608);
            break;
        case 0x4000U:
            if (frame->body.motion.contact & 1U)
            {
                frame->body.animation.id = 0xC4;
            }
            else
            {
                frame->body.animation.id = 0xC5;
            }
            if (frame->body.motion.vy > 81920)
            {
                DoppleDecelerateY(frame, 4608);
            }
            else
            {
                frame->body.motion.vy = 81920;
            }
            RotateToward(frame, 0);
            DoppleDecelerate(frame, 4608);
            break;
        case 0x2000U:
            frame->body.animation.id = 0xC2;
            frame->body.facing_left = 0;
            RotateToward(frame, 0x180);
            if (frame->body.motion.vx > 81920)
            {
                DoppleDecelerate(frame, 4608);
            }
            else
            {
                frame->body.motion.vx = 81920;
            }
            DoppleDecelerateY(frame, 4608);
            break;
        case 0x8000U:
            frame->body.animation.id = 0xC2;
            frame->body.facing_left = 1;
            RotateToward(frame, 0x180);
            if (frame->body.motion.vx < -81920)
            {
                DoppleDecelerate(frame, 4608);
            }
            else
            {
                frame->body.motion.vx = -81920;
            }
            DoppleDecelerateY(frame, 4608);

            break;
        case 0x2000U | 0x1000U:
            frame->body.animation.id = 0xC2;
            frame->body.facing_left = 0;
            RotateToward(frame, 0x80);
            if (frame->body.motion.vx > 57344)
            {
                DoppleDecelerate(frame, 3072);
            }
            else
            {
                frame->body.motion.vx = 57344;
            }
            if (frame->body.motion.vy < -57344)
            {
                DoppleDecelerateY(frame, 3072);
            }
            else
            {
                frame->body.motion.vy = -57344;
            }
            break;
        case 0x8000U | 0x1000U:
            frame->body.animation.id = 0xC2;
            frame->body.facing_left = 1;
            RotateToward(frame, 0x80);
            if (frame->body.motion.vx < -57344)
            {
                DoppleDecelerate(frame, 3072);
            }
            else
            {
                frame->body.motion.vx = -57344;
            }
            if (frame->body.motion.vy < -57344)
            {
                DoppleDecelerateY(frame, 3072);
            }
            else
            {
                frame->body.motion.vy = -57344;
            }
            break;
        case 0x2000U | 0x4000U:
            if (frame->body.motion.contact & 1U)
            {
                frame->body.animation.id = 0xC4;
            }
            else
            {
                frame->body.animation.id = 0xC5;
            }
            frame->body.facing_left = 0;
            RotateToward(frame, 0);
            if (frame->body.motion.vx > 57344)
            {
                DoppleDecelerate(frame, 3072);
            }
            else
            {
                frame->body.motion.vx = 57344;
            }
            if (frame->body.motion.vy > 114688)
            {
                DoppleDecelerateY(frame, 3072);
            }
            else
            {
                frame->body.motion.vy = 114688;
            }
            break;
        case 0x8000U | 0x4000U:
            if (frame->body.motion.contact & 1U)
            {
                frame->body.animation.id = 0xC4;
            }
            else
            {
                frame->body.animation.id = 0xC5;
            }
            frame->body.facing_left = 1;
            RotateToward(frame, 0);
            if (frame->body.motion.vx < -57344)
            {
                DoppleDecelerate(frame, 3072);
            }
            else
            {
                frame->body.motion.vx = -57344;
            }
            if (frame->body.motion.vy > 114688)
            {
                DoppleDecelerateY(frame, 3072);
            }
            else
            {
                frame->body.motion.vy = 114688;
            }
            break;
        }
        break;
    case 3:
        if ((!frame->body.facing_left && (frame->body.motion.contact & 4U)) ||
            (frame->body.facing_left && (frame->body.motion.contact & 8U)))
        {
            frame->body.tapped = 8U;
            BatFinished(frame);
            DoppleRequest(frame, SOTN_DOPPLE_SHAKE, 2, 0);
            DoppleRequest(frame, SOTN_DOPPLE_SOUND, 0x644, 0);
            frame->body.motion.vx = 0;
            frame->body.simulated_timer = 32;
            frame->body.simulated_buttons = 0;
            break;
        }

        if (frame->body.wing_timer == 0)
        {
            SDL_SetError("BO4 wing-smash timer underflow");
            frame->failed = true;
            return;
        }
        if (--frame->body.wing_timer == 0)
        {
            frame->body.tapped = 8U;
            BatFinished(frame);
            frame->body.simulated_timer = 32;
            frame->body.simulated_buttons = 0;
        }
        else
        {
            if (directionsPressed & 0x1000U)
            {
                frame->body.motion.vy -= 8192;
                RotateToward(frame, 0x80);
            }
            if (directionsPressed & 0x4000U)
            {
                frame->body.motion.vy += 8192;
            }
            if (!(directionsPressed & 0x1000U))
            {
                RotateToward(frame, 0x180);
            }
            if (!(directionsPressed & (0x4000U | 0x1000U)))
            {
                DoppleDecelerateY(frame, 8192);
            }
            if (frame->body.motion.contact & 0x800U)
            {
                if ((frame->body.facing_left && (frame->body.motion.contact & 0x400U)) ||
                    (!frame->body.facing_left && !(frame->body.motion.contact & 0x400U)))
                {
                    frame->body.motion.vy = 393216;
                }
            }
            if (frame->body.motion.contact & 0x8000U)
            {
                if ((frame->body.facing_left && (frame->body.motion.contact & 0x4000U)) ||
                    (!frame->body.facing_left && !(frame->body.motion.contact & 0x4000U)))
                {
                    frame->body.motion.vy = -393216;
                }
            }
            if (frame->body.motion.vy < -393216)
            {
                frame->body.motion.vy = -393216;
            }
            if (frame->body.motion.vy > 393216)
            {
                frame->body.motion.vy = 393216;
            }
            if (frame->tick % 3 == 0)
            {
                DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 65, 0);
                if (frame->body.motion.contact & 1U)
                {
                    DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 69, 9);
                }
                if (frame->body.motion.contact & 2U)
                {
                    x_offset = 3;
                    if (frame->body.facing_left)
                    {
                        x_offset = -x_offset;
                    }
                    DoppleShift(frame, 0, -(8));
                    DoppleShift(frame, x_offset, 0);
                    DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 4, 1);
                    DoppleShift(frame, 0, 8);
                    DoppleShift(frame, -(x_offset), 0);
                }
            }
        }
        break;
    }

    if (frame->body.bat_offset != 0)
    {
        if (frame->body.motion.vx > 0)
        {
            frame->body.motion.vx = 0;
        }
    }
    if (frame->body.bat_offset != 0)
    {
        if (frame->body.bat_offset > 0)
        {
            frame->body.bat_offset--;
            DoppleShift(frame, 0, 1);
        }
        else
        {
            frame->body.bat_offset++;
            DoppleShift(frame, 0, -1);
        }
    }
}

static void UnmorphBat(DoppleFrame *frame)
{
    Sint32 i = 0;
    Sint32 count = 0;

    frame->body.rotated = true;
    DoppleDecelerate(frame, 8192);
    if (frame->body.motion.contact & (2U | 1U))
    {
        frame->body.motion.vy = 0;
    }
    DoppleDecelerateY(frame, 8192);
    RotateToward(frame, 0);
    count = 0;

    switch (frame->body.substep)
    {
    case 0:
        for (i = 0; i < 4; i++)
        {
            if (frame->body.motion.floor_sensors[i].y <
                DoppleSigned16(frame, 0x1274 + (size_t)i * 2))
            {
                frame->body.motion.floor_sensors[i].y++;
            }
            else
            {
                count++;
            }

            if (frame->body.motion.ceiling_sensors[i].y >
                DoppleSigned16(frame, 0x126c + (size_t)i * 2))
            {
                frame->body.motion.ceiling_sensors[i].y--;
            }
            else
            {
                count++;
            }

            if (i == 0 && (frame->body.motion.contact & 0x8000U))
            {
                DoppleShift(frame, 0, -1);
            }
        }

        if (count == 8)
        {
            frame->body.animation_set = 0x8001;
            frame->body.rotated = false;
            frame->body.rotation = 0;
            frame->body.form_phase = 1;
            frame->body.substep = 1;
            frame->body.form_pose = 0x5F;
        }
        break;

    case 1:
        if (frame->body.form_phase == 3)
        {
            DoppleJump(frame);
            if (!(frame->body.motion.contact & 0x8000U))
            {
                frame->body.motion.vy = -65536;
            }
            frame->body.palette = 0x8200;
            DoppleRestoreStatusEffects(frame);
        }
        break;
    }
}

static Sint32 MistFinished(DoppleFrame *frame)
{
    if (frame->body.substep == 0)
    {
        return false;
    }
    if (frame->body.tapped & 2U)
    {
        DoppleDirection(frame);
        DoppleSetStep(frame, 15);
        return true;
    }
    return false;
}

static void Mist(DoppleFrame *frame)
{
    Uint32 padDirection = 0;

    if (MistFinished(frame) == 0)
    {
        padDirection = frame->body.pressed & 0xf000U;
        switch (frame->body.substep)
        {
        case 0:
            DoppleDirection(frame);
            frame->body.jump_flags = frame->body.attack_state = 0;
            frame->body.busy = false;
            DoppleRequest(frame, SOTN_DOPPLE_PALETTE_BEGIN, 0x8003, 0);
            DoppleCancelAttack(frame);
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 73, 0);
            if (frame->body.motion.vx > 0)
            {
                frame->body.motion.vx = 65536;
            }
            if (frame->body.motion.vx < 0)
            {
                frame->body.motion.vx = -65536;
            }
            if (frame->body.motion.vy > 0)
            {
                frame->body.motion.vy = 65536;
            }
            if (frame->body.motion.vy < 0)
            {
                frame->body.motion.vy = -65536;
            }
            DoppleSetAnimation(frame, 0xCA);
            frame->body.form_pose = frame->body.animation.sprite;
            DoppleRequest(frame, SOTN_DOPPLE_PALETTE_CLEAR, 5, 0);
            DoppleRequest(frame, SOTN_DOPPLE_PALETTE_BEGIN, 0x8801U, 0);
            frame->body.substep++;
            break;

        case 1:
            switch (padDirection)
            {
            case 0:
            default:
                DoppleDecelerate(frame, 768);
                DoppleDecelerateY(frame, 768);
                break;
            case 0x1000U:
                if (frame->body.motion.vy < -65536)
                {
                    DoppleDecelerateY(frame, 768);
                }
                else
                {
                    frame->body.motion.vy = -65536;
                }
                DoppleDecelerate(frame, 768);
                break;
            case 0x4000U:
                if (frame->body.motion.vy > 65536)
                {
                    DoppleDecelerateY(frame, 768);
                }
                else
                {
                    frame->body.motion.vy = 65536;
                }
                DoppleDecelerate(frame, 768);
                break;
            case 0x2000U:
                frame->body.facing_left = false;
                if (frame->body.motion.vx > 65536)
                {
                    DoppleDecelerate(frame, 768);
                }
                else
                {
                    frame->body.motion.vx = 65536;
                }
                DoppleDecelerateY(frame, 768);
                break;
            case 0x8000U:
                frame->body.facing_left = true;
                if (frame->body.motion.vx < -65536)
                {
                    DoppleDecelerate(frame, 768);
                }
                else
                {
                    frame->body.motion.vx = -65536;
                }
                DoppleDecelerateY(frame, 768);
                break;

            case (0x1000U | 0x2000U):
                frame->body.facing_left = false;
                if (frame->body.motion.vx > 40960)
                {
                    DoppleDecelerate(frame, 512);
                }
                else
                {
                    frame->body.motion.vx = 40960;
                }

                if (frame->body.motion.vy < -40960)
                {
                    DoppleDecelerateY(frame, 512);
                }
                else
                {
                    frame->body.motion.vy = -40960;
                }
                break;
            case (0x1000U | 0x8000U):
                frame->body.facing_left = true;
                if (frame->body.motion.vx < -40960)
                {
                    DoppleDecelerate(frame, 512);
                }
                else
                {
                    frame->body.motion.vx = -40960;
                }

                if (frame->body.motion.vy < -40960)
                {
                    DoppleDecelerateY(frame, 512);
                }
                else
                {
                    frame->body.motion.vy = -40960;
                }
                break;

            case (0x4000U | 0x2000U):
                frame->body.facing_left = false;
                if (frame->body.motion.vx > 40960)
                {
                    DoppleDecelerate(frame, 512);
                }
                else
                {
                    frame->body.motion.vx = 40960;
                }
                if (frame->body.motion.vy > 40960)
                {
                    DoppleDecelerateY(frame, 512);
                }
                else
                {
                    frame->body.motion.vy = 40960;
                }
                break;
            case (0x4000U | 0x8000U):
                frame->body.facing_left = 1;
                if (frame->body.motion.vx < -40960)
                {
                    DoppleDecelerate(frame, 512);
                }
                else
                {
                    frame->body.motion.vx = -40960;
                }

                if (frame->body.motion.vy > 40960)
                {
                    DoppleDecelerateY(frame, 512);
                }
                else
                {
                    frame->body.motion.vy = 40960;
                }
                break;
            }
            break;

        default:
            break;
        }

        if (frame->body.bat_offset != 0)
        {
            if (frame->body.bat_offset > 0)
            {
                frame->body.bat_offset--;
                DoppleShift(frame, 0, 1);
            }
            else
            {
                frame->body.bat_offset++;
                DoppleShift(frame, 0, -1);
            }
        }
    }
}

static void UnmorphMist(DoppleFrame *frame)
{
    Sint32 i = 0;
    Sint32 count = 0;

    if ((frame->body.motion.contact & 1U) && frame->body.motion.vy > 0)
    {
        frame->body.motion.vy = 0;
    }
    if ((frame->body.motion.contact & 2U) && frame->body.motion.vy < 0)
    {
        frame->body.motion.vy = 0;
    }

    DoppleDecelerate(frame, 512);
    DoppleDecelerateY(frame, 512);
    count = 0;

    for (i = 0; i < 4; i++)
    {

        if (frame->body.motion.floor_sensors[i].y < DoppleSigned16(frame, 0x1274 + (size_t)i * 2))
        {
            frame->body.motion.floor_sensors[i].y++;
        }
        else
        {
            count++;
        }
        if (frame->body.motion.ceiling_sensors[i].y > DoppleSigned16(frame, 0x126c + (size_t)i * 2))
        {
            frame->body.motion.ceiling_sensors[i].y--;
        }
        else
        {
            count++;
        }
        if (i == 0 && (frame->body.motion.contact & 0x8000U))
        {
            DoppleShift(frame, 0, -1);
        }
    }

    if (count == 8)
    {
        frame->body.animation_set = 0x8001;
        DoppleSetAnimation(frame, 0xCB);
        if (frame->body.substep)
        {
            DoppleSetAnimation(frame, 0xCC);
        }

        if (frame->body.mist_step < 3)
        {
            frame->body.mist_step = 3;
            return;
        }
        if (frame->body.mist_step == 5)
        {
            frame->body.palette = 0x8200;
            DoppleCancelAttack(frame);
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 44, 0x5B);
            DoppleJump(frame);
            if (!(frame->body.motion.contact & 0x8000U))
            {
                frame->body.motion.vy = -65536;
            }
            DoppleRestoreStatusEffects(frame);
        }
    }
}

void DoppleForms(DoppleFrame *frame)
{
    switch (frame->body.step)
    {
    case 6:
        Bat(frame);
        break;
    case 8:
        Mist(frame);
        break;
    case 10:
        UnmorphBat(frame);
        break;
    case 15:
        UnmorphMist(frame);
        break;
    default:
        SDL_SetError("BO4 form control entered unsupported step %u", frame->body.step);
        frame->failed = true;
        break;
    }
}
