// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted from sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5,
// bo4/unk_465F0.c:DopplegangerHandleDamage, DopplegangerStepKill and damage helpers.
// Sound/effect profiles and artwork remain private runtime input.
#include "dopple_body_internal.h"
#include <sotn/combat.h>

static Uint16 Random(DoppleFrame *frame)
{
    if (!frame->has_random)
    {
        frame->failed = true;
        SDL_SetError("BO4 damage initialization requires the owned original libc RNG");
        return 0;
    }
    return SotnCombat_Random(&frame->random);
}
static void Reset(DoppleFrame *frame)
{
    frame->body.animation.pose = 0;
    frame->body.animation.remaining = 0;
    frame->body.animation_set = 0x8001;
    frame->body.blend_mode = 0;
    frame->body.jump_flags = frame->body.attack_state = 0;
    frame->body.rotation = 0;
    if (frame->body.mist_active)
        DoppleCancelAttack(frame);
}
static void Speed(DoppleFrame *frame, Sint32 value)
{
    frame->body.motion.vx = frame->body.knockback_left ? -value : value;
}
static Uint16 AirAnimation(DoppleFrame *frame)
{
    const Uint16 value = Random(frame);
    frame->body.animation.id = (Uint16)(0x2e + value % 3U);
    return value % 16U;
}
void DoppleDamage(DoppleFrame *frame)
{
    const Uint16 effects = frame->body.damage_effects;
    switch (frame->body.substep)
    {
    case 0: {
        Reset(frame);
        Uint16 sound = 0;
        if (frame->body.damage_kind == 3)
        {
            sound = (Uint16)((Random(frame) & 1U) + 3);
            frame->body.motion.vy = -262144;
            Speed(frame, -54613);
            frame->body.substep = 1;
            if (!AirAnimation(frame))
                frame->body.animation.id = 0x40;
        }
        else
        {
            sound = (Uint16)((Random(frame) & 1U) + 5);
            switch (frame->body.damage_step)
            {
            case 1:
            case 2:
                frame->body.motion.vy = 0;
                Speed(frame, -109226);
                frame->body.substep = 6;
                frame->body.animation.id =
                    frame->body.knockback_left != frame->body.facing_left ? 0x33 : 0x31;
                DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 0, 6);
                break;
            case 3:
                frame->body.motion.vy = 0;
                Speed(frame, -81920);
                frame->body.substep = 7;
                frame->body.animation.id = 0x23;
                DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 0, 0);
                break;
            default:
                frame->body.motion.vy = -131072;
                Speed(frame, -81920);
                frame->body.substep = 1;
                AirAnimation(frame);
                break;
            }
        }
        const Uint16 sound_id = (Uint16)DoppleSigned16(frame, 0x133c + sound * 2U);
        DoppleRequest(frame, SOTN_DOPPLE_SOUND, sound_id, 0);
        frame->body.damage_palette = 0x8166;
        frame->body.timers[2] = 6;
        if (effects & 0x8000U)
        {
            DoppleRequest(frame, SOTN_DOPPLE_SOUND, 0x690, 0);
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 44, 0x45);
            frame->body.damage_palette = 0x8160;
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 17, 1);
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 18, 0);
            frame->body.timers[2] = 16;
        }
        else if (effects & 0x100U)
        {
            frame->body.timers[1] = 1024;
            frame->body.damage_palette = 0x8165;
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 44, 23);
        }
        else if (effects & 0x80U)
        {
            frame->body.timers[0] = 1024;
            frame->body.damage_palette = 0x8164;
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 44, 22);
        }
        else if (effects & 0x4000U)
        {
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 45, 0);
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 44, 0x46);
            frame->body.timers[2] = 24;
            frame->body.damage_palette = 0x8202;
        }
        else if (effects & 0x2000U)
        {
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 46, 0);
            frame->body.timers[2] = 12;
            frame->body.damage_palette = 0x8169;
            frame->body.animation.id = 0x2e;
        }
        else if (effects & 0x1000U)
        {
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 114, 0);
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 44, 0x63);
            frame->body.timers[2] = 8;
            frame->body.damage_palette = 0x8164;
        }
        else if (effects & 0x800U)
        {
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 113, 0);
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 44, 0x62);
            frame->body.timers[2] = 16;
            frame->body.damage_palette = 0x8164;
        }
        else if (!(effects & 0xfac0U))
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 44, 0x58);
        break;
    }
    case 1:
        if (!DoppleBranches(frame, 0x20280) && frame->body.animation.remaining < 0)
        {
            DoppleSetAnimation(frame, 0x1c);
            frame->body.facing_left = !frame->body.facing_left;
        }
        break;
    case 8:
        frame->body.palette = 0x8200;
        SDL_FALLTHROUGH;
    case 6:
    case 7:
        DoppleDecelerate(frame, 8192);
        if (!(frame->body.motion.contact & 1U))
            DoppleFall(frame);
        if (frame->body.animation.remaining < 0)
        {
            if (frame->body.substep == 6)
                DoppleStand(frame, 0);
            else
                DoppleCrouch(frame, 0, frame->body.motion.vx);
        }
        break;
    default:
        frame->failed = true;
        SDL_SetError("BO4 damage entered unsupported original substep %u", frame->body.substep);
    }
}
static void Erase(SotnDoppleDeath *death)
{
    // Each original column visit clears sixteen nibbles in the 64x64 scratch image.
    for (Uint32 visit = 0; visit < 4; ++visit)
    {
        const Uint32 column = death->cursor;
        const Uint32 start = ((column >> 1) & 7U) + ((column & 255U) >> 4) * 32U;
        for (Uint32 pixel = 0; pixel < 16; ++pixel)
        {
            const Uint32 byte = start + (pixel & 3U) * 8U + (pixel >> 2) * 512U;
            const Uint32 bit = byte * 2U + (column & 1U);
            death->erased[bit / 8] |= (Uint8)(1U << (bit & 7U));
        }
        death->cursor = (Uint16)((column + 35U) & 255U);
    }
}
void DoppleKill(DoppleFrame *frame)
{
    SotnDoppleDeath *death = &frame->body.death;
    frame->body.rotated = true;
    switch (frame->body.substep)
    {
    case 0:
        frame->body.motion.vx = frame->body.motion.vy = 0;
        if (frame->body.damage_step == 12)
        {
            frame->failed = true;
            SDL_SetError("BO4 stone-shatter death requires the unintegrated stone child");
            return;
        }
        DoppleRequest(frame, SOTN_DOPPLE_SOUND, 0x7e6, 0);
        Reset(frame);
        frame->body.motion.vy = -212992;
        Speed(frame, -81920);
        frame->body.animation.id = 0xc0;
        *death = (SotnDoppleDeath){.red = 128, .green = 128, .blue = 128, .shading = true};
        if (frame->body.damage_effects & 0x8000U)
        {
            death->element = 1;
            DoppleRequest(frame, SOTN_DOPPLE_DEATH_PROFILE, 3, 0);
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 44, 0x4f);
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 51, 2);
        }
        else if (frame->body.damage_effects & 0x4000U)
        {
            death->element = 2;
            DoppleRequest(frame, SOTN_DOPPLE_DEATH_PROFILE, 9, 0);
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 44, 0x59);
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 45, 1);
        }
        else if (frame->body.damage_effects & 0x2000U)
        {
            death->element = 3;
            DoppleRequest(frame, SOTN_DOPPLE_DEATH_PROFILE, 10, 0);
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 44, 0x5a);
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 46, 0);
            frame->body.blend_mode = 0x30;
        }
        else
        {
            DoppleRequest(frame, SOTN_DOPPLE_DEATH_PROFILE, 1, 0);
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 44, 0x53);
            DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 49, 5);
        }
        ++frame->body.substep;
        break;
    case 1:
        if (death->element == 0)
        {
            if (death->red < 248)
                ++death->red;
            if (death->blue > 8)
                --death->blue;
            death->green = death->blue;
        }
        if (death->element == 1 || death->element == 2)
        {
            if (death->blue > 8)
                --death->blue;
            death->red = death->green = death->blue;
        }
        if (death->element == 3)
        {
            // Preserve the source's decreasing red branch, including its byte wrap.
            if (death->red < 248)
                --death->red;
            death->green = death->red;
            if (death->blue < 248)
                ++death->blue;
        }
        frame->body.motion.vy += 5632;
        if (frame->body.motion.vy > 16384)
            frame->body.motion.vy = 4096;
        if (frame->body.animation.remaining < 0)
        {
            death->captured = true;
            death->cursor = 0;
            death->timer = 64;
            ++frame->body.substep;
        }
        break;
    case 2:
        if (!death->captured || death->timer == 0)
        {
            frame->failed = true;
            SDL_SetError("BO4 dissolve requires its captured frame and bounded timer");
            return;
        }
        Erase(death);
        if (--death->timer == 0)
        {
            frame->body.motion.vy = 0;
            death->shading = false;
            frame->body.substep = 0x80;
        }
        break;
    case 0x80:
        death->complete = true;
        break;
    default:
        frame->failed = true;
        SDL_SetError("BO4 death entered unsupported original substep %u", frame->body.substep);
        return;
    }
    DoppleDecelerate(frame, 1024);
    if (frame->body.animation.pose >= 15)
    {
        if (frame->body.animation.pose == 22 && frame->body.animation.remaining == 1)
            frame->body.rotation = (Sint16)(frame->body.rotation - 256);
        frame->body.rotation = (Sint16)SDL_max(-640, frame->body.rotation - 6);
    }
}
