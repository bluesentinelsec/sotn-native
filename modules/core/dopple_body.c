// SPDX-License-Identifier: AGPL-3.0-or-later
// Original BO4 normal-form control adaptation; see docs/doppleganger-runtime.md.
#include "dopple_body_internal.h"

Uint8 DoppleByte(DoppleFrame *frame, size_t offset)
{
    if (frame->overlay.bytes == NULL || offset >= frame->overlay.size)
    {
        if (!frame->failed)
            SDL_SetError("BO4 body control table is truncated at offset %zu", offset);
        frame->failed = true;
        return 0;
    }
    return frame->overlay.bytes[offset];
}
Sint16 DoppleSigned16(DoppleFrame *frame, size_t offset)
{
    const Uint16 low = DoppleByte(frame, offset);
    const Uint16 high = DoppleByte(frame, offset + 1);
    const Uint16 bits = (Uint16)(low | high << 8);
    return bits <= 32767 ? (Sint16)bits : (Sint16)((Sint32)bits - 65536);
}
void DoppleRequest(DoppleFrame *frame, Uint16 kind, Uint16 id, Uint16 parameter)
{
    if (frame->body.request_count >= SDL_arraysize(frame->body.requests))
    {
        if (!frame->failed)
            SDL_SetError("BO4 body request capacity exceeded");
        frame->failed = true;
        return;
    }
    frame->body.requests[frame->body.request_count++] =
        (SotnDoppleRequest){.kind = kind,
                            .id = id,
                            .parameter = parameter,
                            .x = frame->body.motion.x,
                            .y = frame->body.motion.y,
                            .facing_left = frame->body.facing_left};
}
void DoppleSetStep(DoppleFrame *frame, Uint16 step)
{
    frame->body.step = step;
    frame->body.substep = 0;
}
void DoppleSetAnimation(DoppleFrame *frame, Uint16 id)
{
    if (!frame->failed && !SotnAnimation_SetStage(&frame->body.animation, 26, id))
        frame->failed = true;
}
void DoppleSpeed(DoppleFrame *frame, Sint32 speed)
{
    frame->body.motion.vx = frame->body.facing_left ? -speed : speed;
}
void DoppleDecelerate(DoppleFrame *frame, Sint32 amount)
{
    Sint32 *speed = &frame->body.motion.vx;
    if (*speed < 0)
        *speed = SDL_min(0, *speed + amount);
    else
        *speed = SDL_max(0, *speed - amount);
}
Sint32 DoppleDirection(DoppleFrame *frame)
{
    if (frame->body.jump_flags & 2U)
        return 0;
    if (frame->body.facing_left)
    {
        if (frame->body.pressed & 0x2000U)
        {
            frame->body.facing_left = false;
            frame->body.turned = true;
            return -1;
        }
        if (frame->body.pressed & 0x8000U)
            return 1;
    }
    else
    {
        if (frame->body.pressed & 0x2000U)
            return 1;
        if (frame->body.pressed & 0x8000U)
        {
            frame->body.facing_left = true;
            frame->body.turned = true;
            return -1;
        }
    }
    return 0;
}
void DoppleAfterImage(DoppleFrame *frame, bool reset, Uint16 ticks)
{
    frame->body.afterimage_disabled = true;
    if (reset)
        frame->body.afterimage_reset = true;
    if (ticks)
        frame->body.timers[15] = SDL_max(4, ticks);
}
void DoppleCancelAttack(DoppleFrame *frame)
{
    DoppleRequest(frame, SOTN_DOPPLE_CLEAR_ATTACK, 0, 0);
    frame->body.attack_state = 0;
    frame->body.mist_active = false;
}
void DoppleCrouch(DoppleFrame *frame, Uint16 index, Sint32 speed)
{
    if (index >= 4)
    {
        SDL_SetError("BO4 crouch transition exceeds its private profile");
        frame->failed = true;
        return;
    }
    frame->body.motion.vx = speed;
    frame->body.motion.vy = 0;
    frame->body.step = 3;
    frame->body.substep = DoppleByte(frame, 0x1318 + index * 2U);
    DoppleSetAnimation(frame, DoppleByte(frame, 0x1319 + index * 2U));
}
void DoppleStand(DoppleFrame *frame, Sint32 speed)
{
    Uint16 index = 0;
    bool edge = (frame->body.motion.contact & 0x20U) != 0;
    frame->body.motion.vx = speed;
    frame->body.motion.vy = 0;
    DoppleSetStep(frame, 1);
    if (frame->body.busy)
    {
        frame->body.substep = 2;
        edge = false;
    }
    switch (frame->body.previous_step)
    {
    case 10:
        index = 4;
        break;
    case 2:
        index = 4;
        if (frame->body.animation.id == 9)
        {
            frame->body.animation.id = DoppleByte(frame, 0x1320 + 2U + (Uint32)edge);
            return;
        }
        if (frame->body.animation.id == 7)
            index = 0;
        break;
    case 4:
    case 5:
        index = 6;
        if (SDL_abs(frame->body.motion.vx) > 163840)
            index = 4;
        break;
    default:
        index = 8;
        break;
    }
    index = (Uint16)(index + (Uint16)edge);
    DoppleSetAnimation(frame, DoppleByte(frame, 0x1320 + index));
}
void DoppleWalk(DoppleFrame *frame, bool force)
{
    const bool edge = (frame->body.motion.contact & 0x20U) != 0;
    DoppleSpeed(frame, 98304);
    frame->body.motion.vy = 0;
    DoppleSetStep(frame, 2);
    if (force)
    {
        if (frame->body.animation.id != 13)
            DoppleSetAnimation(frame, 13);
    }
    else
    {
        DoppleSetAnimation(frame, 7);
        DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 1, 5);
    }
    if (frame->body.turned)
        frame->body.animation.id = 9;
    if (frame->body.animation.id == 7 && edge)
        frame->body.animation.pose = 1;
    if (frame->body.previous_step == 3)
        frame->body.animation.pose = 4;
}
void DoppleJump(DoppleFrame *frame)
{
    if (DoppleDirection(frame))
    {
        DoppleSetAnimation(frame, 26);
        DoppleSpeed(frame, 98304);
        frame->body.jump_flags = 0;
    }
    else
    {
        DoppleSetAnimation(frame, 22);
        frame->body.motion.vx = 0;
        frame->body.jump_flags = 4;
    }
    frame->body.motion.vy = -319488;
    DoppleSetStep(frame, 5);
    if (frame->body.previous_step == 2)
        frame->body.jump_flags |= 0x10U;
}
void DoppleDoubleJump(DoppleFrame *frame)
{
    frame->body.jump_flags |= 0x21U;
    frame->body.motion.vy = -278528;
    DoppleSetAnimation(frame, 32);
    frame->body.substep = 0;
}
void DoppleFall(DoppleFrame *frame)
{
    DoppleSetStep(frame, 4);
    if (frame->body.previous_step != 2)
        DoppleSetAnimation(frame, 28);
    frame->body.motion.vx = 0;
    frame->body.motion.vy = 131072;
    frame->body.timers[5] = frame->body.timers[6] = 8;
    frame->body.jump_flags = 0x10;
}
void DoppleBackdash(DoppleFrame *frame)
{
    frame->body.step = 1;
    frame->body.substep = 3;
    DoppleSpeed(frame, -229376);
    frame->body.motion.vy = 0;
    DoppleSetAnimation(frame, 219);
    DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 0, 0);
}

void DoppleDecelerateY(DoppleFrame *frame, Sint32 amount)
{
    Sint32 *speed = &frame->body.motion.vy;
    *speed = *speed < 0 ? SDL_min(0, *speed + amount) : SDL_max(0, *speed - amount);
}
void DoppleShift(DoppleFrame *frame, Sint32 dx, Sint32 dy)
{
    const Uint32 x = (Uint32)frame->body.motion.x + (Uint32)dx * 65536U;
    const Uint32 y = (Uint32)frame->body.motion.y + (Uint32)dy * 65536U;
    frame->body.motion.x = x <= SDL_MAX_SINT32 ? (Sint32)x : (Sint32)((Sint64)x - 4294967296LL);
    frame->body.motion.y = y <= SDL_MAX_SINT32 ? (Sint32)y : (Sint32)((Sint64)y - 4294967296LL);
}
void DoppleRestoreStatusEffects(DoppleFrame *frame)
{
    if (frame->body.timers[1])
        DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 44, 0x17);
    if (frame->body.timers[0])
        DoppleRequest(frame, SOTN_DOPPLE_FACTORY, 44, 0x16);
}

bool SotnDoppleBody_Init(SotnDataView overlay, Sint32 x, Sint32 y, bool facing_left,
                         SotnDoppleBody *out)
{
    if (out == NULL)
        return SDL_SetError("BO4 body initialization requires writable state");
    DoppleFrame frame = {.overlay = overlay};
    if (!SotnDoppleMotion_Init(overlay, &frame.body.motion))
        return false;
    frame.body.motion.x = x;
    frame.body.motion.y = y;
    frame.body.facing_left = facing_left;
    frame.body.palette = frame.body.damage_palette = 0x8200;
    frame.body.animation_set = 0x8001;
    frame.body.form_pose = (Uint16)DoppleSigned16(&frame, 0x3b0e);
    DoppleFall(&frame);
    if (frame.failed)
        return false;
    DoppleAfterImage(&frame, true, 48);
    *out = frame.body;
    return true;
}

bool SotnDoppleBody_Update(SotnDataView overlay, const SotnCollisionMap *map, SotnDoppleInput input,
                           Uint32 *random, SotnDoppleBody *body)
{
    if (body == NULL || !body->motion.initialized ||
        (body->step < 1 || (body->step > 6 && body->step != 8 && body->step != 10 &&
                            body->step != 11 && body->step != 15 && body->step != 17)) ||
        input.knives > 16 || input.effect_index >= 16 || input.player_x < -32767 ||
        input.player_x > 32767 || body->motion.vx < -2097152 || body->motion.vx > 2097152 ||
        body->motion.vy < -2097152 || body->motion.vy > 2097152 || body->animation.id >= 220 ||
        body->rotation < -4096 || body->rotation > 4096 || body->bat_offset < -64 ||
        body->bat_offset > 64 || body->form_phase > 3)
        return SDL_SetError(
            "BO4 body tick requires an initialized, bounded supported control state");
    DoppleFrame frame = {.body = *body,
                         .overlay = overlay,
                         .knives = input.knives,
                         .tick = body->tick,
                         .random = random ? *random : 0,
                         .has_random = random != NULL};
    const Uint16 buttons = input.buttons;
    const Uint32 previous_status = body->motion.status;
    frame.body.request_count = 0;
    SDL_zero(frame.body.requests);
    frame.body.turned = false;
    frame.body.rotated = false;
    frame.body.afterimage_reset = false;
    const bool new_death = input.lethal && !(previous_status & 0x40000U);
    if (!new_death && !(previous_status & 0x40000U))
    {
        for (size_t i = 0; i < SDL_arraysize(frame.body.timers); ++i)
        {
            if (frame.body.timers[i])
            {
                if (i == 2)
                    frame.body.palette = frame.body.damage_palette;
                if (--frame.body.timers[i] == 0)
                {
                    if (i == 2)
                        frame.body.palette = 0x8200;
                    if (i == 15)
                        frame.body.afterimage_disabled = false;
                }
            }
        }
        frame.body.held = frame.body.pressed;
        frame.body.pressed = buttons;
        frame.body.tapped = (Uint16)((frame.body.held ^ buttons) & buttons);
        if (frame.body.motion.status & 8U)
        {
            frame.body.tapped &= 0xff5fU;
            frame.body.pressed &= 0xff5fU;
        }
    }
    if (new_death || (input.hit && !(previous_status & 0x40000U) && !frame.body.timers[13]))
    {
        Uint32 effect = 0;
        for (Uint32 byte = 0; byte < 4; ++byte)
            effect |= (Uint32)DoppleByte(&frame, 0x120c + input.effect_index * 4U + byte)
                      << (byte * 8U);
        if (effect > 65535 || (!new_death && (effect & 0x200U)))
            return SDL_SetError(
                "BO4 hit requires a supported private effect; stone status is pending");
        frame.body.damage_effects = (Uint16)effect;
        frame.body.damage_kind = input.hp_lost >= 10 ? 3 : 2;
        frame.body.damage_step = frame.body.step;
        const Sint32 boss_x = frame.body.motion.x >= 0
                                  ? frame.body.motion.x / 65536
                                  : (Sint32)(-((-(Sint64)frame.body.motion.x + 65535) / 65536));
        frame.body.knockback_left = boss_x > input.player_x;
        DoppleSetStep(&frame, new_death ? 17 : 11);
    }
    const bool can_morph = frame.body.step <= 6 || frame.body.step == 8;
    if ((frame.body.tapped & 8U) && can_morph && frame.body.step != 6)
    {
        if (frame.body.step == 3)
            frame.body.bat_offset = 12;
        DoppleRequest(&frame, SOTN_DOPPLE_SOUND, 0x6a0, 0);
        if (frame.body.rotation == 128)
        {
            frame.body.rotation = 0;
            frame.body.animation.sprite = 0x9d;
            frame.body.facing_left = !frame.body.facing_left;
        }
        DoppleSetStep(&frame, 6);
        frame.body.form_phase = 0;
    }
    if ((frame.body.tapped & 2U) && can_morph && frame.body.step != 8)
    {
        if (frame.body.rotation == 128)
        {
            frame.body.rotation = 0;
            frame.body.animation.sprite = 0x9d;
            frame.body.facing_left = !frame.body.facing_left;
        }
        DoppleSetStep(&frame, 8);
        DoppleRequest(&frame, SOTN_DOPPLE_SOUND, 0x6a0, 0);
    }
    frame.body.previous_step = frame.body.step;
    frame.body.previous_substep = frame.body.substep;
    if (frame.body.step <= 5)
        DoppleNormal(&frame);
    else if (frame.body.step == 11)
        DoppleDamage(&frame);
    else if (frame.body.step == 17)
        DoppleKill(&frame);
    else
        DoppleForms(&frame);
    if (frame.failed)
        return false;
    frame.body.motion.status = 0;
    switch (frame.body.step)
    {
    case 3:
        if (frame.body.substep != 2)
            frame.body.motion.status = 0x20U;
        SDL_FALLTHROUGH;
    case 1:
    case 2:
        frame.body.animation_set = 0x8001;
        break;
    case 4:
    case 5:
        frame.body.motion.status = 0x2000;
        frame.body.animation_set = 0x8001;
        break;
    case 6:
        DoppleAfterImage(&frame, true, 1);
        if (frame.body.substep == 3)
            frame.body.timers[14] = SDL_max(frame.body.timers[14], 4);
        frame.body.motion.status = 0x100001U;
        if (frame.body.substep)
            frame.body.animation_set = 0x8002;
        break;
    case 8:
        DoppleAfterImage(&frame, true, 1);
        frame.body.timers[14] = SDL_max(frame.body.timers[14], 4);
        frame.body.motion.status = 0x100002U;
        frame.body.palette = 0x820d;
        break;
    case 10:
        DoppleAfterImage(&frame, true, 1);
        frame.body.timers[14] = SDL_max(frame.body.timers[14], 4);
        frame.body.motion.status = 0x500001U;
        if (frame.body.substep == 0)
            frame.body.animation_set = 0x8002;
        break;
    case 11:
    case 17:
        DoppleAfterImage(&frame, true, 1);
        frame.body.timers[14] = SDL_max(frame.body.timers[14], 4);
        frame.body.animation_set = 0x8001;
        frame.body.motion.status = 0x110000U;
        if (frame.body.step == 17)
        {
            frame.body.motion.status |= 0x40000U;
            if (frame.body.substep == 0x80)
                frame.body.motion.status |= 0x80000U;
        }
        break;
    case 15:
        DoppleAfterImage(&frame, true, 1);
        frame.body.timers[14] = SDL_max(frame.body.timers[14], 4);
        frame.body.motion.status = 0x900002U;
        frame.body.palette = 0x820d;
        break;
    }
    if ((previous_status & 0x10000U) && !(frame.body.motion.status & 0x50000U))
    {
        frame.body.timers[15] = 4;
        frame.body.palette = 0x8200;
    }
    if (frame.body.timers[0])
    {
        frame.body.motion.status |= 0x100100U;
        DoppleAfterImage(&frame, true, 1);
    }
    if (frame.failed)
        return false;
    if (!SotnAnimation_TickDopple(overlay, frame.body.form_pose, &frame.body.animation) ||
        !SotnDoppleMotion_Advance(overlay, map, false, &frame.body.motion))
        return false;
    if (frame.body.step == 17 && frame.body.animation.remaining < 0)
        frame.body.animation.sprite |= 0x8000U;
    frame.body.tick = body->tick + 1U;
    if (random)
        *random = frame.random;
    *body = frame.body;
    return true;
}

bool SotnDoppleBody_Tick(SotnDataView overlay, const SotnCollisionMap *map, Uint16 buttons,
                         Uint8 knives, SotnDoppleBody *body)
{
    return SotnDoppleBody_Update(
        overlay, map, (SotnDoppleInput){.buttons = buttons, .knives = knives}, NULL, body);
}
