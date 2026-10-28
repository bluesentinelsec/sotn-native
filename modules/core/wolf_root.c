// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/wolfForm.c:func_8012F894/func_8012F83C, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Root frame/pose tables remain private.
#include <SDL3/SDL_error.h>
#include <sotn/game_math.h>
#include <sotn/wolf_root.h>
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static Uint32 Read32(const Uint8 *p)
{
    return p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
}
static Sint32 Shift(Sint32 value, Uint16 bits)
{
    const Sint64 scale = (Sint64)1 << bits;
    return (Sint32)(value >= 0 ? value / scale : -((-(Sint64)value + scale - 1) / scale));
}
static Sint32 Fixed(Sint32 position, Sint32 fraction)
{
    const Uint32 bits = (Uint32)(Uint16)(Uint32)position * 65536U | ((Uint32)fraction & 65535U);
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static bool Query(SotnWolfCollisionQuery query, void *context, Sint32 x, Sint32 y,
                  SotnCollider *out)
{
    SotnCollider collider = {0};
    if (!query(context, x, y, &collider))
        return false;
    if (collider.top < -1024 || collider.top > 1024)
        return SDL_SetError("Wolf root collision returned an invalid tile-top distance");
    *out = collider;
    return true;
}
static bool Distance(SotnDataView executable, Sint32 x, Sint32 y, Sint32 root_x, Sint32 root_y,
                     Sint32 distance, bool *reached)
{
    const Sint64 dx = (Sint64)x - root_x, dy = (Sint64)y - root_y;
    const Sint64 square = dx * dx + dy * dy;
    Uint32 length = 0;
    if (square > 0x7ffff || !SotnMath_Sqrt12(executable, (Uint32)square * 4096U, &length))
        return SDL_SetError("Wolf root floor search exceeds original SDK distance bounds");
    *reached = (Sint32)(length / 4096U) >= distance;
    return true;
}
static bool Project(SotnDataView executable, Sint32 x, Sint32 y, Sint32 angle, Sint32 distance,
                    Sint32 *root_x, Sint32 *root_y)
{
    SotnMathTrig trig = {0};
    if (!SotnMath_Trig(executable, angle, &trig))
        return false;
    *root_x = x + Shift(Shift(trig.cosine, 4) * distance, 8);
    *root_y = y - Shift(Shift(trig.sine, 4) * distance, 8);
    return true;
}
static Sint32 SlopeAngle(Uint32 contacts)
{
    switch (contacts & 0xf000U)
    {
    case 0xe000:
        return 0x760;
    case 0xd000:
        return 0x6d2;
    case 0xc000:
        return 0x600;
    case 0x8000:
        return 0xa00;
    case 0x9000:
        return 0x92e;
    case 0xa000:
        return 0x8a0;
    default:
        return 0x800;
    }
}
static bool Ground(SotnWolfControllerFrame *p, SotnDataView executable,
                   SotnWolfCollisionQuery query, void *context, Sint32 x, Sint32 y, Sint32 *root_x,
                   Sint32 *root_y, Sint32 *angle, Sint32 *control)
{
    const bool left = p->motion.facing_left;
    const Sint32 distance = p->joint_distance;
    Sint32 advance = 1, height = y + 2;
    for (Uint16 attempt = 0; attempt < 2048; ++attempt)
    {
        const Sint32 horizontal = x + (left ? advance : -advance);
        height += 2;
        SotnCollider first = {0}, second = {0};
        if (!Query(query, context, horizontal, height, &first))
            return false;
        if (first.effects & 1U)
        {
            if (!Query(query, context, horizontal, height + first.top - 1, &second))
                return false;
            bool slope_pair = false;
            if (!(second.effects & 1U))
                height += first.top;
            else
            {
                if ((second.effects & 0x8003U) != 0x8001U)
                    continue;
                height += first.top - 1 + second.top;
                slope_pair = true;
            }
            bool reached = false;
            if (!Distance(executable, x, y, horizontal, height, distance, &reached))
                return false;
            if (!reached)
            {
                ++advance;
                continue;
            }
            *control = 1;
            if (slope_pair || (first.effects & 0x8000U))
            {
                *control = 3;
                if ((left && (first.effects & 0x4000U)) || (!left && !(first.effects & 0x4000U)))
                    *control = 2;
            }
            Sint16 direction = 0;
            if (!SotnMath_Atan2(executable, y - height, horizontal - x, &direction))
                return false;
            *angle = direction;
            *root_x = horizontal;
            *root_y = height;
            return true;
        }
        if (advance < 5)
        {
            ++advance;
            height -= 2;
            continue;
        }
        *angle = SlopeAngle(p->motion.contacts) - (left ? 2048 : 0);
        if (!Project(executable, x, y, *angle, distance, root_x, root_y))
            return false;
        *control = 0;
        if (advance <= 10 && (left || x - distance >= 0) && (!left || x + distance < 257) &&
            (p->motion.substep == 1 || p->motion.substep == 3))
        {
            if (!Query(query, context, x + (left ? distance + 1 : -distance - 1),
                       Shift(p->y, 16) + 23, &first))
                return false;
            if (!(first.effects & 1U))
            {
                p->motion.velocity_x = left ? 65536 : -65536;
                p->motion.velocity_y = 0;
                p->motion.animation.pose = 0;
                p->motion.animation.remaining = 0;
                p->motion.mode = 0;
                p->motion.animation.id = 0xde;
                p->motion.substep = 7;
            }
        }
        return true;
    }
    return SDL_SetError("Wolf root floor search exhausted its bounded collision probes");
}
bool SotnWolfRoot_Tick(SotnWolfRoot *root, SotnWolfArticulation *body,
                       SotnWolfControllerFrame *player, bool primitives_allocated, SotnDataView dra,
                       SotnDataView executable, SotnWolfCollisionQuery query, void *context,
                       SotnWolfRootEvents *out)
{
    if (!root || !body || !player || !out || !dra.bytes || dra.size < 0x10a5c || !query ||
        player->motion.substep > 9 || player->motion.mode > 4 ||
        player->motion.animation.pose >= 8 || player->motion.animation.id < 0xde ||
        player->motion.animation.id > 0xed || player->motion.body_angle < 0 ||
        player->motion.body_angle > 4095 || player->joint_distance < 0 ||
        player->joint_distance > 64 || player->motion.velocity_x < -16 * 65536 ||
        player->motion.velocity_x > 16 * 65536 || body->parts[0].step > 1 || player->priority < 2 ||
        player->priority > 511)
        return SDL_SetError(
            "Wolf root requires actual bounded player/body, collision and private frame snapshots");
    if (body->parts[0].step && !primitives_allocated)
        return SDL_SetError("Wolf root lost its actual six-primitive ownership");
    SotnWolfRoot next = *root;
    SotnWolfPart part = body->parts[0];
    SotnWolfControllerFrame p = *player;
    SotnWolfJumpFrame *m = &p.motion;
    SotnWolfRootEvents events = {.image_index = -1};
    if (m->animation.id == 0xe1)
    {
        if (m->animation.pose >= 6)
            return SDL_SetError("Wolf turning root requires one of its six private distances");
        const Uint32 distance = Read32(dra.bytes + 0x10924 + m->animation.pose * 4U);
        if (distance > 64)
            return SDL_SetError("Wolf turning root has an invalid private distance");
        p.joint_distance = (Sint32)distance;
    }
    if (!part.step)
    {
        if (!primitives_allocated)
            goto done;
        part.bank = 15;
        part.texture_offset = 0x7e;
        part.palette = next.player_palette;
        part.flags = 0x04860000U;
        ++part.step;
    }
    if (p.joint_distance < 0 || p.joint_distance > 64)
        return SDL_SetError("Wolf private root distance exceeds original body bounds");
    part.priority = (Uint16)(p.priority - 2U);
    part.facing_left = m->facing_left;
    const Sint32 speed = SDL_abs(m->velocity_x);
    if (speed > 3 * 65536)
    {
        part.draw_flags |= 8;
        next.player_draw_flags = part.draw_flags;
        next.player_blend = part.blend = 0x70;
        next.player_opacity = part.opacity =
            (Uint8)(255 - SDL_min(Shift(speed - 3 * 65536, 11), 128));
    }
    else
    {
        part.draw_flags = (Uint8)(part.draw_flags & 0xf7U);
        next.player_draw_flags = part.draw_flags;
        next.player_blend = part.blend = 0;
    }
    switch (m->substep)
    {
    case 1:
        if (m->mode == 1)
            p.joint_distance = 8;
        break;
    case 2:
        if (m->mode == 1 || m->mode == 3)
            p.joint_distance -= Word(Read16(dra.bytes + 0x10a3c + m->animation.pose * 2U));
        if (m->mode == 2)
            p.joint_distance -= Word(Read16(dra.bytes + 0x10a4c + m->animation.pose * 2U));
        break;
    case 4:
        if (m->velocity_y < 0)
        {
            if (m->body_angle > 1792)
                m->body_angle -= 32;
            p.joint_distance = 10;
        }
        else
        {
            if (m->body_angle < 1984)
                m->body_angle += 32;
            p.joint_distance = 12;
        }
        break;
    case 5:
    case 8:
        p.joint_distance = 12;
        break;
    case 7:
        if (m->body_angle < 2304)
            m->body_angle += 32;
        break;
    case 9:
        p.joint_distance -= Word(Read16(dra.bytes + 0x10a3c + m->animation.pose * 2U));
        break;
    }
    if (p.joint_distance < 0 || p.joint_distance > 64)
        return SDL_SetError("Wolf private pose produced an invalid articulated root distance");
    const Sint32 x = Shift(p.x, 16), y = Shift(p.y, 16) + 25;
    Sint32 root_x = 0, root_y = 0, angle = m->body_angle, control = 0;
    if (!(m->contacts & 1U) || m->substep == 7)
    {
        if (m->facing_left)
            angle = 2048 - angle;
        if (p.raw_contacts & 0x40U)
            angle = m->facing_left ? 0 : 2048;
        if (!Project(executable, x, y, angle, p.joint_distance, &root_x, &root_y))
            return false;
    }
    else if (!Ground(&p, executable, query, context, x, y, &root_x, &root_y, &angle, &control))
        return false;
    if ((!m->facing_left && x - p.joint_distance < 0) ||
        (m->facing_left && x + p.joint_distance > 256))
    {
        root_x = x + (m->facing_left ? p.joint_distance : -p.joint_distance);
        root_y = y;
        angle = m->facing_left ? 0 : 2048;
    }
    m->body_angle = (Sint32)((Uint32)(m->facing_left ? 2048 - angle : angle) & 4095U);
    part.x = Fixed(root_x, p.x);
    part.y = Fixed(root_y - 23, p.y);
    Uint16 index = (Uint16)(m->animation.id - 0xdeU);
    if (m->substep == 1 && !m->mode)
    {
        if (control == 2)
            index = 1;
        if (control == 1)
            index = 0;
        if (control == 3)
            index = 16;
    }
    const Uint32 pointer = Read32(dra.bytes + 0x109f8 + index * 4U);
    const size_t offset = pointer >= 0x800a0000U ? pointer - 0x800a0000U : dra.size;
    if (offset > dra.size || dra.size - offset < ((size_t)m->animation.pose + 1U) * 2U)
        return SDL_SetError("Wolf root frame points outside its private DRA image");
    part.sprite = Read16(dra.bytes + offset + m->animation.pose * 2U);
    if (part.sprite == 0xffffU)
    {
        if (!m->animation.pose)
            return SDL_SetError("Wolf root terminal frame lacks its preceding private sprite");
        part.sprite = Read16(dra.bytes + offset + (m->animation.pose - 1U) * 2U);
    }
    if (part.sprite > 511)
        return SDL_SetError("Wolf root has invalid private sprite metadata");
    if (SDL_abs(m->velocity_x) > 163840 && p.game_timer % 6U == 0)
    {
        events.image_index = next.image_cycle % 5;
        const Uint32 bits = (Uint32)next.image_cycle + 1U;
        next.image_cycle =
            bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
    }
    if (SDL_abs(m->velocity_x) > 3 * 65536)
    {
        events.rush_image = true;
        next.player_palette = 0x810d;
    }
    part.palette = next.player_palette;
done:
    *root = next;
    body->parts[0] = part;
    *player = p;
    *out = events;
    return true;
}
