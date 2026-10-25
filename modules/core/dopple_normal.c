// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted from sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5,
// src/boss/bo4/unk_46040.c and normal-form steps in unk_465F0.c.
#include "dopple_body_internal.h"
bool DoppleBranches(DoppleFrame *frame, Uint32 arg0)
{

    if (arg0 & 8)
    {
        if (frame->body.attack_state == 0)
        {
            DoppleDirection(frame);
        }
    }

    if (arg0 & 0x8000)
    {
        frame->body.motion.vy += 11264;
        if (frame->body.motion.vy > 458752)
        {
            frame->body.motion.vy = 458752;
        }
    }
    if (arg0 & 0x10000)
    {
        if (frame->body.motion.vy < 24576 && frame->body.motion.vy > -8192 &&
            !(frame->body.jump_flags & 0x20) && (frame->body.pressed & 0x40U))
        {
            frame->body.motion.vy += 2252;
        }
        else
        {

            frame->body.motion.vy += 11264;
            if (frame->body.motion.vy > 458752)
            {
                frame->body.motion.vy = 458752;
            }
        }
    }

    if (arg0 & 0x80)
    {
        if (frame->body.motion.contact & 2U)
        {
            if (frame->body.motion.vy < -65536)
            {
                frame->body.motion.vy = -65536;
            }
        }
    }
    if (arg0 & 0x200)
    {
        if (frame->body.motion.vy < 24576 && frame->body.motion.vy > -8192)
        {
            frame->body.motion.vy += 5632;
        }
        else
        {
            frame->body.motion.vy += 11264;
            if (frame->body.motion.vy > 458752)
            {
                frame->body.motion.vy = 458752;
            }
        }
    }

    if (frame->body.motion.vy >= 0)
    {
        if ((arg0 & 1) && (frame->body.motion.contact & 1U))
        {
            if (frame->body.attack_state)
            {
                if ((frame->body.attack_state & 0x7FFF) == 0xFF)
                {
                    DoppleStand(frame, 0);
                    DoppleCancelAttack(frame);
                    DoppleRequest(frame, SOTN_DOPPLE_SOUND, 0x64c, 0);
                    return true;
                }
                if (frame->body.motion.vy > 450560)
                {
                    DoppleCrouch(frame, 1, 0U);
                    DoppleRequest(frame, SOTN_DOPPLE_SOUND, 0x647, 0);
                    DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 0, 0);
                }
                else
                {
                    if (frame->body.jump_flags & 0x10)
                    {
                        DoppleWalk(frame, 1);
                    }
                    else
                    {
                        DoppleStand(frame, 0);
                    }
                    DoppleRequest(frame, SOTN_DOPPLE_SOUND, 0x64c, 0);
                }
                DoppleCancelAttack(frame);
                return true;
            }
            if (frame->body.motion.vy > 450560)
            {
                if (frame->body.substep == 0x70 || frame->body.step == 5)
                {
                    DoppleCrouch(frame, 3, frame->body.motion.vx / 2);
                }
                else
                {
                    DoppleCrouch(frame, 1, 0);
                }
                DoppleRequest(frame, SOTN_DOPPLE_SOUND, 0x647, 0);
                DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 0, 0);
            }
            else if (frame->body.jump_flags & 0x10)
            {
                DoppleWalk(frame, 1);
                DoppleRequest(frame, SOTN_DOPPLE_SOUND, 0x64c, 0);
            }
            else if (SDL_abs(frame->body.motion.vx) > 131072)
            {
                DoppleRequest(frame, SOTN_DOPPLE_SOUND, 0x647, 0);
                DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 0, 0);
                DoppleStand(frame, frame->body.motion.vx);
            }
            else
            {
                DoppleRequest(frame, SOTN_DOPPLE_SOUND, 0x64c, 0);
                DoppleStand(frame, 0);
            }
            return true;
        }

        if ((arg0 & 0x20000) && (frame->body.motion.contact & 1U))
        {
            DoppleCrouch(frame, 3, frame->body.motion.vx);
            DoppleRequest(frame, SOTN_DOPPLE_SOUND, 0x647, 0);
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 0, 0);
            return true;
        }
    }

    if ((arg0 & 4) && !(frame->body.motion.contact & 1U))
    {
        DoppleFall(frame);
        return true;
    }

    if ((arg0 & 0x1000) && (frame->body.tapped & (0x80U | 0x20U)) && DoppleAttack(frame))
    {
        return true;
    }

    if (!(frame->body.attack_state & 0x8000))
    {
        if ((arg0 & 0x10) && (frame->body.tapped & 0x40U))
        {
            DoppleJump(frame);
            return true;
        }

        if ((arg0 & 0x20) && (frame->body.tapped & 0x40U) && !(frame->body.jump_flags & 1))
        {
            DoppleDoubleJump(frame);
            return true;
        }

        if ((arg0 & 0x2000) && (frame->body.pressed & 0x4000U))
        {
            DoppleCrouch(frame, 2, 0U);
            return true;
        }

        if ((arg0 & 0x40000) && (frame->body.tapped & 0x10U) && frame->body.animation.id != 0xDB)
        {
            DoppleBackdash(frame);
            return true;
        }
    }

    return false;
}

static void StepStand(DoppleFrame *frame)
{
    Sint32 anim = 0;
    Uint16 var_s0 = 0;

    var_s0 = 3;
    anim = 0;
    if (frame->body.motion.contact & 0x20U)
    {
        anim = 1;
    }

    if (DoppleBranches(frame, 0x4301C) == false)
    {
        DoppleDecelerate(frame, 8192);
        switch (frame->body.substep)
        {
        case 0:
        case 2:
            break;
        case 1:
            var_s0 = 1;
            if (!(frame->body.pressed & 0x1000U))
            {
                var_s0 = 5;
            }
            break;
        case 3:
            var_s0 = 0;
            if (frame->body.animation.pose > 3)
            {
                var_s0 = 1;
            }
            if (frame->body.animation.pose > 6 || frame->body.animation.remaining < 0)
            {
                var_s0 = 7;
            }
            break;
        case 0x40:
        case 0x41:
        case 0x42:
            DoppleAfterImage(frame, true, 1);
            if (frame->body.animation.pose < frame->body.attack_cancel_pose)
            {
                var_s0 = 0;
            }
            else
            {
                frame->body.attack_state &= 0x7fffU;
                var_s0 = 0x1B;
                if (frame->body.animation.remaining < 0)
                {
                    var_s0 = 0xF;
                }
            }
            break;
        case 0x51:

            DoppleAfterImage(frame, true, 1);
            var_s0 = 0;
            if (frame->body.animation.remaining < 0)
            {
                var_s0 = 0xF;
            }

            break;
        }

        if (var_s0 & 4)
        {
            DoppleStand(frame, 0);
            var_s0 |= 0x8000;
        }
        if (var_s0 & 2 && frame->body.pressed & 0x1000U && !frame->body.busy)
        {
            DoppleSetAnimation(frame, anim);
            frame->body.substep = 1;
            var_s0 |= 0x8000;
        }

        if (var_s0 & 1 && DoppleDirection(frame) != 0)
        {
            DoppleWalk(frame, 0);
            var_s0 |= 0x8000;
        }
        if (var_s0 & 0x8000 && var_s0 & 8)
        {
            DoppleCancelAttack(frame);
        }
    }
}

static void StepWalk(DoppleFrame *frame)
{
    if (DoppleBranches(frame, 0x4301C) == false)
    {
        DoppleSpeed(frame, 98304);
        if (DoppleDirection(frame) == 0)
        {
            DoppleStand(frame, 0);
        }
    }
}

static void StepJump(DoppleFrame *frame)
{
    Sint32 moveDirection = 0;
    Sint16 index = 0;

    DoppleDecelerate(frame, 4096);
    if (frame->body.motion.vy < -65536)
    {
        if (!(frame->body.jump_flags & 0x40) && !(frame->body.pressed & 0x40U))
        {
            frame->body.motion.vy = -65536;
        }
        if (frame->body.motion.contact & 2U)
        {
            frame->body.motion.vy = -16384;
            frame->body.jump_flags |= 0x20;
        }
    }

    if (DoppleBranches(frame, 0x11029))
    {
        return;
    }

    switch (frame->body.substep)
    {
    case 0:
        moveDirection = DoppleDirection(frame);
        if (moveDirection)
        {
            if (frame->body.animation.id == 22 || frame->body.animation.id == 25)
            {
                DoppleSetAnimation(frame, 24);
            }
            DoppleSpeed(frame, 98304);
        }
        else if (frame->body.animation.id == 26 || frame->body.animation.id == 24)
        {
            DoppleSetAnimation(frame, 25);
        }
        if (moveDirection <= 0)
        {
            frame->body.jump_flags &= 0xffefU;
        }
        if (frame->body.motion.vy > 0)
        {
            if (frame->body.animation.id != 27)
            {
                DoppleSetAnimation(frame, 27);
            }
            frame->body.substep = 1;
        }
        break;
    case 1:
        moveDirection = DoppleDirection(frame);
        if (moveDirection != 0)
        {
            DoppleSpeed(frame, 98304);
        }
        if (moveDirection <= 0)
        {
            frame->body.jump_flags &= 0xffefU;
        }

        break;

    case 0x40:
    case 0x41:
    case 0x42:
    case 0x51:
        DoppleAfterImage(frame, true, 1);
        if (frame->body.pressed & 0x8000U)
        {
            frame->body.motion.vx = -98304;
        }
        if (frame->body.pressed & 0x2000U)
        {
            frame->body.motion.vx = 98304;
        }
        if (frame->body.animation.remaining < 0)
        {
            if (frame->body.motion.vy > 65536)
            {
                index = 0;
            }
            else
            {
                index = 2;
            }
            frame->body.substep = DoppleSigned16(frame, 0x1334 + (size_t)index * 2);
            DoppleSetAnimation(frame,
                               (Uint8)DoppleSigned16(frame, 0x1334 + (size_t)(index + 1) * 2));
            DoppleCancelAttack(frame);
        }
        break;
    }
}

static void StepFall(DoppleFrame *frame)
{
    if (DoppleBranches(frame, 0x9029) == false)
    {
        DoppleDecelerate(frame, 4096);
        if (DoppleDirection(frame) != 0)
        {
            DoppleSpeed(frame, 49152);
        }
    }
}

static void StepCrouch(DoppleFrame *frame)
{
    Uint16 var_s0 = 0;

    var_s0 = 0;

    if (DoppleBranches(frame, 0x100C) == false)
    {
        DoppleDecelerate(frame, 8192);
        switch (frame->body.substep)
        {
        case 0:
            var_s0 = 6;
            break;

        case 1:
            if (!(frame->body.pressed & 0x4000U))
            {

                var_s0 = 1;
                DoppleSetAnimation(frame, 0x13);
                frame->body.substep = 2;
                frame->body.animation.pose = 1;
                return;
            }

            if (frame->body.animation.id == 0x65)
            {
                frame->body.substep = 0;
            }
            else if (frame->body.animation.remaining < 0)
            {
                var_s0 = 0x20;
            }
            break;
        case 4:
        case 3:
            if (frame->body.animation.remaining < 0)
            {
                var_s0 = 0x20;
            }
            break;
        case 2:
            var_s0 = 1;
            if (frame->body.animation.remaining < 0)
            {
                DoppleStand(frame, 0);
            }
            break;
        case 0x40:
        case 0x41:
        case 0x42:
            DoppleAfterImage(frame, true, 1);
            if (frame->body.animation.pose < frame->body.attack_cancel_pose)
            {
                var_s0 = 0;
            }
            else
            {
                frame->body.attack_state &= 0x7fffU;
                var_s0 = 0xE;
                if (frame->body.animation.remaining < 0)
                {
                    var_s0 = 0x2E;
                }
            }
            break;
        case 0x51:
            DoppleAfterImage(frame, true, 1);
            if (frame->body.animation.remaining < 0)
            {
                var_s0 = 0x2E;
            }
            break;
        }

        if (var_s0 & 0x20)
        {
            DoppleCrouch(frame, 0, 0);
            var_s0 |= 0x8000;
        }

        if (var_s0 & 2 && frame->body.turned)
        {
            DoppleSetAnimation(frame, 0x14);
            frame->body.substep = 0;
            var_s0 |= 0x8000;
        }

        if (var_s0 & 4 && !(frame->body.pressed & 0x4000U))
        {
            DoppleSetAnimation(frame, 0x13);
            frame->body.substep = 2;
            var_s0 |= 0x8000;
        }

        if (var_s0 & 1 && DoppleDirection(frame))
        {
            DoppleWalk(frame, 0);
            var_s0 |= 0x8000;
        }

        if (var_s0 & 0x8000 && var_s0 & 8)
        {
            DoppleCancelAttack(frame);
        }
    }
}

void DoppleNormal(DoppleFrame *frame)
{
    switch (frame->body.step)
    {
    case 1:
        StepStand(frame);
        break;
    case 2:
        StepWalk(frame);
        break;
    case 3:
        StepCrouch(frame);
        break;
    case 4:
        StepFall(frame);
        break;
    case 5:
        StepJump(frame);
        break;
    default:
        SDL_SetError("BO4 normal controls do not support body step %u", frame->body.step);
        frame->failed = true;
        break;
    }
}
