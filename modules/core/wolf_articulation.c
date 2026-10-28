// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected linked parts from dra/wolfForm.c, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Pose/artwork tables remain private.
#include <SDL3/SDL_error.h>
#include <sotn/game_math.h>
#include <sotn/wolf_articulation.h>

static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint32 Add(Sint32 value, Sint32 delta)
{
    const Uint32 bits = (Uint32)value + (Uint32)delta;
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint32 Shift(Sint32 value, Uint16 bits)
{
    const Sint64 divisor = (Sint64)1 << bits;
    return (Sint32)(value >= 0 ? value / divisor : -((-(Sint64)value + divisor - 1) / divisor));
}
static Sint16 Whole(Sint32 value)
{
    return (Sint16)Shift(value, 16);
}
static Sint16 Read16(const Uint8 *p)
{
    return Word(p[0] | (Sint32)p[1] << 8);
}
static void MoveY(SotnWolfPart *p, Sint32 pixels)
{
    p->y = Add(p->y, pixels * 65536);
}
static void MoveX(SotnWolfPart *p, Sint32 pixels)
{
    p->x = Add(p->x, pixels * 65536);
}
static void Initialize(SotnWolfPart *p, Uint16 sprite, Uint16 palette)
{
    p->bank = 15;
    p->texture_offset = 0x7e;
    p->palette = palette;
    p->flags = 0x04060000U;
    p->sprite = sprite;
    ++p->step;
}
static Sint16 Pose(SotnDataView dra, size_t address, Uint16 index)
{
    return Read16(dra.bytes + address - 0x800a0000U + (size_t)index * 2U);
}
static void Opacity(SotnWolfPart *part, Sint32 speed, Sint32 cap)
{
    part->blend = 0;
    part->draw_flags = (Uint8)(part->draw_flags & 0xf7U);
    if (speed > 3 * 65536)
    {
        part->draw_flags |= 8;
        part->blend = 0x70;
        part->opacity = (Uint8)(255 - SDL_min(Shift(speed - 3 * 65536, 12), cap));
    }
}
static bool Front(SotnWolfArticulation *b, SotnWolfControllerFrame *p, Uint16 palette,
                  SotnDataView dra, SotnWolfArticulationEvents *e)
{
    SotnWolfPart *s = &b->parts[1];
    const SotnWolfJumpFrame *m = &p->motion;
    if (!s->step)
    {
        Initialize(s, 1, palette);
        s->draw_flags = 4;
        s->pivot_x = 0;
        s->pivot_y = 9;
        s->hitbox_x = -4;
        s->hitbox_y = 10;
        s->width = 20;
        s->height = 6;
    }
    s->priority = p->priority;
    s->facing_left = m->facing_left;
    s->x = p->x;
    s->y = p->y;
    s->rotation = Word(2048 - m->body_angle);
    switch (m->substep)
    {
    case 1:
        if (m->mode == 1)
            s->rotation = Word(s->rotation - 384);
        break;
    case 2:
        if (m->mode == 0)
        {
            if (m->animation.sprite == 33)
                s->sprite = 31;
            if (m->animation.sprite == 34)
                s->sprite = 32;
        }
        else if (m->mode <= 4)
        {
            const size_t at = m->mode == 2 ? 0x800b0a6cU : m->mode == 4 ? 0x800b0a7cU : 0x800b0a5cU;
            const Sint32 offset = Pose(dra, at, m->animation.pose);
            MoveY(s, offset);
            if (m->mode != 4)
                s->rotation = Word(s->rotation - offset * 32);
        }
        if (m->contacts & 0x8000U)
            MoveY(s, 2);
        break;
    case 3:
        if (m->animation.sprite == 18)
            MoveY(s, 4);
        if (m->animation.sprite == 19 || m->animation.sprite == 20)
            MoveY(s, 8);
        break;
    }
    if (m->body_angle == 2560)
        MoveY(s, 2);
    s->palette = palette;
    s->hitbox_state = 0;
    if (SDL_abs(m->velocity_x) > 3 * 65536 && !(m->substep == 2 && m->mode == 4))
    {
        e->rush_attack = true;
        s->enemy_id = 3;
        if (s->hit_flags)
        {
            const size_t at = 0x8410U + 13U * 28U;
            if (dra.size < at + 28U)
                return SDL_SetError(
                    "Wolf rush feedback requires its complete private spell-thirteen definition");
            const Uint16 cost = dra.bytes[at + 12];
            if (p->mp >= cost)
            {
                p->mp = (Uint16)(p->mp - cost);
                e->mp_spent = cost;
            }
            else
            {
                p->motion.mode = 3;
                p->motion.animation.id = 0xe0;
                p->turn_timer = 24;
            }
        }
    }
    s->hit_flags = 0;
    return true;
}
static void Back(SotnWolfArticulation *b, const SotnWolfControllerFrame *p, Uint16 palette,
                 SotnDataView dra)
{
    SotnWolfPart *s = &b->parts[2];
    const SotnWolfJumpFrame *m = &p->motion;
    if (!s->step)
    {
        Initialize(s, 35, palette);
        s->draw_flags = 4;
        s->pivot_x = 1;
        s->pivot_y = 8;
    }
    s->priority = (Uint16)(p->priority - 2U);
    s->facing_left = m->facing_left;
    s->x = b->parts[0].x;
    s->y = b->parts[0].y;
    s->rotation = Word(-((m->body_angle - 2048) / 2));
    switch (m->substep)
    {
    case 1:
        if (m->mode == 1)
        {
            s->rotation = Word(s->rotation - 256);
            MoveY(s, 8);
        }
        if (m->body_angle < 1537)
            MoveY(s, 2);
        break;
    case 2:
        if (m->mode == 0)
        {
            if (m->animation.sprite == 33)
                s->sprite = 65;
            if (m->animation.sprite == 34)
                s->sprite = 66;
        }
        else if (m->mode <= 4)
        {
            const size_t at = m->mode == 2 ? 0x800b0a9cU : m->mode == 4 ? 0x800b0aacU : 0x800b0a8cU;
            MoveY(s, Pose(dra, at, m->animation.pose));
        }
        if (m->body_angle < 1793)
            MoveY(s, 1);
        if (m->body_angle < 1537)
            MoveY(s, 2);
        break;
    case 3:
        if (m->animation.sprite == 18)
            MoveY(s, 4);
        if (m->animation.sprite == 19 || m->animation.sprite == 20)
            MoveY(s, 8);
        if (m->body_angle <= 1536)
            MoveY(s, 2);
        break;
    case 4:
        if (m->mode == 0)
        {
            MoveY(s, 1);
            s->rotation = Word(s->rotation - 384);
        }
        break;
    }
    s->palette = palette;
    Opacity(s, SDL_abs(m->velocity_x), 160);
}
static bool Spine(SotnWolfArticulation *b, const SotnWolfControllerFrame *p, Uint16 palette,
                  SotnDataView dra, SotnDataView executable, Uint16 index)
{
    SotnWolfPart *s = &b->parts[index + 4U];
    const SotnWolfJumpFrame *m = &p->motion;
    if (!s->step)
    {
        Initialize(s, (Uint16)Pose(dra, 0x800b0b0cU, index), palette);
        b->wave = 0;
        s->tail_rate = 32;
        s->rotation = Word(m->body_angle);
    }
    s->priority = (Uint16)(p->priority - 3U);
    s->facing_left = m->facing_left;
    Sint32 x = index ? b->parts[index + 3U].x
                     : Add(b->parts[2].x, m->facing_left ? 3 * 65536 : -3 * 65536);
    Sint32 y = index ? b->parts[index + 3U].y : Add(b->parts[2].y, 7 * 65536);
    const Sint32 length = m->animation.sprite == 33 ? 1 : m->animation.sprite == 34 ? 0 : 2;
    if (!index)
    {
        Sint32 offset = 0, amplitude = 0;
        const Uint32 phase = b->wave;
        switch (m->substep)
        {
        case 1:
        case 9:
            b->wave += 24;
            amplitude = 256;
            offset = m->substep == 1 && m->mode == 1 ? -128 : 128;
            break;
        case 2:
            switch (m->mode)
            {
            case 0:
                offset = -768;
                break;
            case 1:
            case 3:
                b->wave += 256;
                amplitude = 64;
                offset = -512;
                break;
            case 2:
                b->wave += 256;
                amplitude = 64;
                offset = -256 + SDL_min(Shift(SDL_abs(m->velocity_x) - 3 * 65536, 10), 256);
                break;
            }
            break;
        case 3:
            b->wave += 16;
            amplitude = 64;
            offset = 128;
            break;
        case 4:
            if (!m->mode)
                offset = m->velocity_y < 0 ? 1536 : -512;
            else
            {
                amplitude = 32;
                offset = m->velocity_y < 0 ? 256 : -256;
            }
            break;
        case 5:
        case 6:
        case 8:
            offset = -512;
            break;
        case 7:
            offset = -128;
            break;
        }
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(executable, (Sint32)(phase & 4095U), &trig))
            return false;
        b->target_angle = m->body_angle + offset + Shift(Shift(trig.sine, 8) * amplitude, 4);
    }
    const Sint32 weight = Pose(dra, 0x800b0b20U, index);
    const Sint32 target = (b->target_angle - m->body_angle) * weight / 256 + m->body_angle;
    if (target < s->rotation)
        s->rotation = Word(s->rotation - s->tail_rate);
    if (s->rotation < target)
        s->rotation = Word(s->rotation + s->tail_rate);
    const Sint32 angle = m->facing_left ? 2048 - s->rotation : s->rotation;
    SotnMathTrig trig = {0};
    if (!SotnMath_Trig(executable, angle, &trig))
        return false;
    s->x = Add(x, trig.cosine * length * 16);
    s->y = Add(y, -trig.sine * length * 16);
    s->palette = palette;
    Opacity(s, SDL_abs(m->velocity_x), 128);
    return true;
}
static void Tail(SotnWolfArticulation *b, const SotnWolfControllerFrame *p, Uint16 palette)
{
    SotnWolfPart *s = &b->parts[13];
    const SotnWolfJumpFrame *m = &p->motion;
    if (!s->step)
    {
        Initialize(s, 80, palette);
        s->draw_flags = 4;
        s->pivot_x = -8;
    }
    s->sprite = 80;
    s->facing_left = m->facing_left;
    s->x = b->parts[2].x;
    s->y = b->parts[2].y;
    s->priority = (Uint16)(p->priority + (m->facing_left ? 5 : -5));
    MoveX(s, m->facing_left ? -8 : 8);
    MoveY(s, 3);
    s->rotation = b->parts[1].rotation;
    if (m->substep == 1 && m->mode == 1)
    {
        MoveY(s, -2);
        MoveX(s, m->facing_left ? 8 : -8);
    }
    if (m->substep == 2 && m->mode == 0)
    {
        if (m->animation.sprite == 33)
        {
            s->sprite = 81;
            MoveX(s, m->facing_left ? 6 : 3);
        }
        if (m->animation.sprite == 34)
        {
            s->sprite = 82;
            MoveX(s, m->facing_left ? 13 : 3);
        }
    }
    s->palette = palette;
}
bool SotnWolfArticulation_Tick(SotnWolfArticulation *body, SotnWolfControllerFrame *player,
                               Uint16 slot, Uint16 palette, SotnDataView dra,
                               SotnDataView executable, SotnWolfArticulationEvents *out)
{
    if (!body || !player || !out || slot < 18 || slot > 30 || slot == 20 || !dra.bytes ||
        dra.size < 0x10b32 || player->motion.substep > 9 || player->motion.mode > 4 ||
        player->motion.animation.pose >= 8 || player->motion.body_angle < 0 ||
        player->motion.body_angle > 4095 || player->motion.velocity_x < -16 * 65536 ||
        player->motion.velocity_x > 16 * 65536 || player->priority < 5 || player->priority > 506 ||
        player->mp > 9999)
        return SDL_SetError("Wolf articulation requires an actual bounded follower/controller and "
                            "private pose snapshot");
    const SotnWolfPart *part = &body->parts[slot - 17U];
    if (part->step > 1 ||
        (slot >= 21 && slot <= 29 && (part->tail_rate < 0 || part->tail_rate > 512)))
        return SDL_SetError("Wolf follower has an invalid lifecycle or angular rate");
    if (slot >= 22 && slot <= 29 && (body->target_angle < -8192 || body->target_angle > 8192))
        return SDL_SetError("Wolf follower requires the preceding spine's bounded target angle");
    for (Uint16 i = 0; i < 9; ++i)
        if (Pose(dra, 0x800b0b20U, i) < 0 || Pose(dra, 0x800b0b20U, i) > 512)
            return SDL_SetError("Wolf private spine interpolation weight exceeds original bounds");
    SotnWolfArticulation next = *body;
    SotnWolfControllerFrame frame = *player;
    SotnWolfArticulationEvents events = {0};
    if (slot == 18)
    {
        if (!Front(&next, &frame, palette, dra, &events))
            return false;
    }
    else if (slot == 19)
        Back(&next, &frame, palette, dra);
    else if (slot < 30)
    {
        if (!Spine(&next, &frame, palette, dra, executable, (Uint16)(slot - 21U)))
            return false;
    }
    else
    {
        Tail(&next, &frame, palette);
        SotnWolfBodyPositions positions = {.player = {Whole(frame.x), Whole(frame.y)},
                                           .facing_left = frame.motion.facing_left};
        for (Uint16 i = 0; i < 14; ++i)
            positions.parts[i] = (SotnWolfPoint){Whole(next.parts[i].x), Whole(next.parts[i].y)};
        if (!SotnWolf_CaptureBounds(&positions, &next.capture))
            return false;
    }
    *body = next;
    *player = frame;
    *out = events;
    return true;
}
