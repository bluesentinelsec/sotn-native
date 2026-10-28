// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/wolfForm.c:func_801309B4 and selected alu_anim.c:UpdateAnim semantics,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Tables remain private.
#include <SDL3/SDL_error.h>
#include <sotn/game_math.h>
#include <sotn/wolf_head.h>
static Sint16 Read16(const Uint8 *p)
{
    const Uint16 bits = (Uint16)(p[0] | (Uint16)p[1] << 8);
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint32 Shift(Sint32 value, Uint16 bits)
{
    const Sint64 scale = (Sint64)1 << bits;
    return (Sint32)(value >= 0 ? value / scale : -((-(Sint64)value + scale - 1) / scale));
}
static Sint32 Add(Sint32 value, Sint32 delta)
{
    const Uint32 bits = (Uint32)value + (Uint32)delta;
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint32 Read32(const Uint8 *p)
{
    const Uint32 bits = p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
bool SotnWolfHead_Tick(SotnWolfHead *head, SotnWolfArticulation *body,
                       const SotnWolfControllerFrame *player, Uint16 palette, bool attack_requested,
                       SotnDataView dra, SotnDataView executable, SotnWolfHeadEvents *out)
{
    if (!head || !body || !player || !out || !dra.bytes || dra.size < 0x10b0c ||
        body->parts[3].step > 1 || head->pose >= 5 || head->remaining < -1 ||
        player->motion.animation.pose >= 8 || player->motion.substep > 9 ||
        player->motion.mode > 4 || player->motion.body_angle < 0 ||
        player->motion.body_angle > 4095 || player->priority > 509)
        return SDL_SetError(
            "Wolf head requires actual bounded child, animation and private snapshots");
    SotnWolfHead next = *head;
    SotnWolfPart part = body->parts[3];
    SotnWolfHeadEvents events = {0};
    const SotnWolfJumpFrame *m = &player->motion;
    if (!part.step)
    {
        part.bank = 15;
        part.texture_offset = 0x7e;
        part.width = 10;
        part.height = 8;
        part.sprite = 72;
        next.pose = 4;
        next.remaining = 1;
        ++part.step;
    }
    part.priority = (Uint16)(player->priority + 2U);
    part.facing_left = m->facing_left;
    part.x = body->parts[1].x;
    part.y = Add(body->parts[1].y, 4 * 65536);
    part.flags = 0x04160000U;
    part.palette = palette;
    Sint32 angle = SDL_clamp(m->body_angle - 2048, -384, 384), distance = 11;
    switch (m->substep)
    {
    case 1:
        if (m->mode == 1)
            angle += 256;
        break;
    case 2:
        if (!m->mode && (m->animation.sprite == 33 || m->animation.sprite == 34))
        {
            if (m->animation.sprite == 33)
                part.x = Add(part.x, m->facing_left ? -4 * 65536 : 4 * 65536);
            part.sprite = m->animation.sprite == 33 ? 73 : 74;
            part.flags &= ~0x100000U;
            goto done;
        }
        if (m->mode == 2)
        {
            const Sint32 bob = Read32(dra.bytes + 0x10aec + m->animation.pose * 4U);
            if (bob < -512 || bob > 512)
                return SDL_SetError("Wolf private head bob exceeds original geometry bounds");
            distance += bob;
            angle -= 128;
            if (m->body_angle == 1536 || m->body_angle == 2560)
                angle += 128;
        }
        break;
    case 3:
        angle = SDL_max(angle - 256, -384);
        break;
    case 4:
        if (m->mode == 2)
            angle -= 64;
        break;
    }
    distance -= Shift(angle, 7);
    if (distance < -512 || distance > 512)
        return SDL_SetError("Wolf private head attachment exceeds original geometry bounds");
    if (m->facing_left)
        angle = 2048 - angle;
    SotnMathTrig trig = {0};
    if (!SotnMath_Trig(executable, angle, &trig))
        return false;
    part.x = Add(part.x, Shift(Shift(trig.cosine, 4) * distance, 8) * 65536);
    part.y = Add(part.y, -Shift(Shift(trig.sine, 4) * distance, 8) * 65536);
    if (m->substep != 8 && m->substep && attack_requested && next.remaining == -1)
    {
        events.sound = 0x6f7;
        next.pose = 0;
        next.remaining = 0;
    }
    const Sint32 shift = Read32(dra.bytes + 0x10ad4 + next.pose * 4U);
    if (shift < -512 || shift > 512)
        return SDL_SetError("Wolf private bite offset exceeds original geometry bounds");
    part.x = Add(part.x, (m->facing_left ? -shift : shift) * 65536);
    if (m->substep == 2 && m->mode == 4)
        events.spell = 4;
    else if (part.sprite != 72 && part.sprite != 73 && part.sprite != 74)
        events.spell = 14;
    else
        part.hitbox_state = 0;
    if (events.spell)
        part.enemy_id = 3;
    if (next.remaining < 0)
    {
        if (next.breath_timer)
            --next.breath_timer;
        else if (player->fall_water_level > 24)
        {
            events.bubble = true;
            events.bubble_source =
                (SotnPlayerFactorySource){.x = Add(part.x, m->facing_left ? -4 * 65536 : 4 * 65536),
                                          .y = Add(part.y, 2 * 65536),
                                          .flags = part.flags,
                                          .priority = part.priority,
                                          .facing_left = part.facing_left};
            next.breath_timer = 64;
        }
    }
done:
    *head = next;
    body->parts[3] = part;
    *out = events;
    return true;
}
bool SotnWolfHead_Animate(SotnWolfHead *head, SotnWolfPart *part, SotnDataView dra)
{
    if (!head || !part || !part->step || head->pose >= 5 || head->remaining < -1 || !dra.bytes ||
        dra.size < 0x10ad4)
        return SDL_SetError("Wolf head animation requires its actual bounded private cursor");
    if (!(part->flags & 0x100000U))
        return true;
    SotnWolfHead next = *head;
    SotnWolfPart image = *part;
    if (!next.remaining)
        next.remaining = Read16(dra.bytes + 0x10abc + next.pose * 4U);
    else if (next.remaining != -1 && --next.remaining == 0)
    {
        ++next.pose;
        const Sint16 duration = Read16(dra.bytes + 0x10abc + next.pose * 4U);
        if (duration == -1)
        {
            --next.pose;
            next.remaining = -1;
        }
        else
            next.remaining = duration;
    }
    if (next.pose >= 5 || next.remaining < -1 || !next.remaining)
        return SDL_SetError("Wolf head animation has invalid private duration/end metadata");
    const Uint16 sprite = (Uint16)Read16(dra.bytes + 0x10abe + next.pose * 4U);
    if (sprite > 511)
        return SDL_SetError("Wolf head animation has invalid private sprite metadata");
    image.sprite = sprite;
    *head = next;
    *part = image;
    return true;
}
