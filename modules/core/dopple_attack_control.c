// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted from sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5,
// src/boss/bo4/unk_45648.c:func_us_801C5B68, func_us_801C5CF8.
#include "dopple_body_internal.h"

static Sint32 Subweapon(DoppleFrame *frame)
{
    if (!(frame->body.pressed & 0x1000U))
        return 1;
    const Uint16 edge = (frame->body.motion.contact & 0x20U) != 0;
    if (frame->knives >= 3)
        return -1;
    DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 3, 0);
    frame->body.timers[10] = 4;
    if (frame->body.substep >= 0x40)
        return 0;
    switch (frame->body.step)
    {
    case 1:
        DoppleSetAnimation(frame, (Uint16)(0x5d + edge));
        break;
    case 3:
        if (frame->body.substep == 2)
        {
            DoppleSetAnimation(frame, (Uint16)(0x5d + edge));
            DoppleSetStep(frame, 1);
        }
        else
            DoppleSetAnimation(frame, 0x5f);
        break;
    }
    return 0;
}

bool DoppleAttack(DoppleFrame *frame)
{
    const Uint16 edge = (frame->body.motion.contact & 0x20U) != 0;
    const Uint16 pressed = frame->body.tapped & 0xa0U;
    const Sint32 subweapon = Subweapon(frame);
    if (!subweapon)
        return true;
    if (subweapon < 0 || (frame->body.attack_state & 0x8000U))
        return false;
    if (frame->body.timers[1])
    {
        DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 57, 1);
        switch (frame->body.step)
        {
        case 1:
        case 2:
            DoppleSetAnimation(frame, 0xb5);
            frame->body.step = 1;
            break;
        case 3:
            DoppleSetAnimation(frame, 0xb6);
            frame->body.step = 3;
            break;
        case 4:
        case 5:
            DoppleSetAnimation(frame, 0xb7);
            frame->body.step = 5;
            break;
        }
        frame->body.attack_state = 0x8012;
        frame->body.attack_cancel_pose = 255;
        frame->body.substep = 0x51;
        DoppleRequest(frame, SOTN_DOPPLE_SOUND, 0x7d7, 0);
        return true;
    }
    Uint16 base = 0;
    if (pressed == 0x80)
    {
        DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 48, 0);
        frame->body.substep = 0x41;
        frame->body.attack_state = 0x8002;
        frame->body.attack_cancel_pose = 13;
        base = 0x41;
    }
    else
    {
        frame->body.attack_state = 0x8003;
        frame->body.substep = 0x42;
        DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 48, 1);
        frame->body.attack_cancel_pose = 8;
        base = 0xa7;
    }
    Uint16 offset = 0;
    switch (frame->body.step)
    {
    case 1:
    case 2:
        frame->body.motion.vx = frame->body.motion.vx >= 0
                                    ? frame->body.motion.vx / 2
                                    : (Sint32)(-((-(Sint64)frame->body.motion.vx + 1) / 2));
        frame->body.step = 1;
        offset = edge;
        break;
    case 3:
        offset = 2;
        if (frame->body.pressed & 0xa000U)
            ++offset;
        // The original branch observes the already-selected attack substep.
        if (frame->body.substep == 2)
        {
            offset = edge;
            frame->body.step = 1;
        }
        break;
    case 4:
    case 5:
        offset = 4;
        if (frame->body.motion.vy > 0)
        {
            ++offset;
            if (frame->body.pressed & 0x4000U)
                ++offset;
        }
        break;
    }
    DoppleSetAnimation(frame, (Uint16)(base + offset));
    frame->body.timers[9] = 4;
    return true;
}
