// SPDX-License-Identifier: AGPL-3.0-or-later
// Reconstructed BO0 313A8.c appendage controllers/helpers at
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Profiles/art stay private.
#include "olrox_monster_internal.h"

typedef struct Point
{
    Sint16 x, y;
} Point;
static Sint16 Word(Sint32 n)
{
    return (Sint16)Wrap16(n);
}
static Sint32 Scale(Sint32 a, Sint32 b)
{
    return ActorShift((Sint64)a * b, 12);
}
static Sint32 ByteSigned(Uint8 n)
{
    return n < 128 ? n : (Sint32)n - 256;
}
static void Whole(Sint32 *position, Sint32 pixel)
{
    *position = (Sint32)((Sint64)Word(pixel) * 65536 + ((Uint32)*position & 65535U));
}
static bool Trig(SotnActors *s, Sint32 angle, SotnMathTrig *out)
{
    return SotnMath_Trig(s->executable, Word(angle), out);
}
static Point Add(Point p, Sint32 x, Sint32 y)
{
    return (Point){Word(p.x + x), Word(p.y + y)};
}
static bool Available(const SotnActors *s, Uint16 count)
{
    Uint16 free_count = 0;
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY && free_count < count; ++i)
        if (!s->primitives[i].owner)
            ++free_count;
    return free_count >= count;
}
static bool Packet(SotnActors *s, Actor *a, Uint16 index, SotnActorPrimitive **out)
{
    ActorPrimitive *p = ActorPrimitiveGet(s, a, index);
    if (!p)
        return false;
    *out = &p->state;
    return true;
}
static void Geometry(SotnActorPrimitive *p, const Point xy[4])
{
    p->x = p->y = 0;
    p->quad = p->textured = p->screen_space = true;
    for (Uint16 i = 0; i < 4; ++i)
    {
        p->offset_x[i] = xy[i].x;
        p->offset_y[i] = xy[i].y;
    }
}
static bool Override(SotnActors *s, Actor *a, const Actor *body)
{
    for (Uint16 i = 0; i < a->state.primitive_count; ++i)
    {
        SotnActorPrimitive *p = NULL;
        if (!Packet(s, a, i, &p))
            return false;
        if (a->state.palette & 0x8000U)
            p->palette = a->state.palette & 0xfffU;
        if (body->olrox.death_phase)
        {
            p->page = 0x15;
            p->palette = 0x219;
        }
    }
    return true;
}
static bool TailPose(SotnActors *s, Actor *a, size_t offset, Sint16 speed, bool smooth,
                     bool *arrived)
{
    SotnDataView pose = {0};
    if (!Slice(s->nz0, offset, 16, &pose))
        return false;
    *arrived = true;
    for (Uint16 i = 0; i < 8; ++i)
    {
        bool reached = false;
        const Sint16 target = (Sint16)Signed16(pose.bytes + i * 2U);
        if (smooth ? !OlroxMonster_Smooth(&a->olrox_tail.angle[i], &a->olrox_tail.velocity[i],
                                          target, speed, &reached)
                   : !OlroxMonster_Approach(&a->olrox_tail.angle[i], target, speed, &reached))
            return false;
        *arrived = *arrived && reached;
    }
    return true;
}
static bool TailGeometry(SotnActors *s, Actor *a)
{
    Point origin = {Word(Pixel(a->state.x) - s->camera_x), Word(Pixel(a->state.y) - s->camera_y)},
          previous_plus = {0}, previous_minus = {0};
    Sint16 previous_angle = -384;
    for (Uint16 i = 0; i < 9; ++i)
    {
        const Sint16 angle = a->olrox_tail.angle[i];
        Sint16 across = Word(((Sint32)previous_angle + angle) / 2 + 1024);
        SotnMathTrig curve = {0}, t = {0};
        if (!Trig(s, across - angle, &curve))
            return false;
        Sint32 plus_length = Scale(curve.cosine, 6) + 8, minus_length = 8 - Scale(curve.cosine, 6);
        if (!i)
        {
            minus_length = 12;
            across = 608;
        }
        const Sint32 width = a->olrox_tail.growth / 2 + 2048;
        plus_length = Word(Scale(plus_length, width));
        minus_length = Word(Scale(minus_length, width));
        if (!Trig(s, a->state.facing_left ? 2048 - across : across, &t))
            return false;
        const Point plus = Add(origin, Scale(t.cosine, plus_length), -Scale(t.sine, plus_length)),
                    minus =
                        Add(origin, -Scale(t.cosine, minus_length), Scale(t.sine, minus_length));
        const Sint32 length = Scale(a->olrox_tail.growth, 11);
        if (!Trig(s, angle, &t))
            return false;
        origin = Add(origin, (a->state.facing_left ? -1 : 1) * Scale(t.cosine, length),
                     -Scale(t.sine, length));
        previous_angle = angle;
        if (i)
        {
            SotnActorPrimitive *p = NULL;
            if (!Packet(s, a, (Uint16)(i - 1U), &p))
                return false;
            p->page = 0x14;
            p->palette = 0x20b;
            Geometry(p, (Point[4]){previous_plus, plus, previous_minus, minus});
        }
        previous_plus = plus;
        previous_minus = minus;
    }
    return true;
}
static bool Tail(SotnActors *s, Actor *a, Actor *body)
{
    a->state.facing_left = body->state.facing_left;
    a->state.palette = body->state.palette;
    if (!s->olrox_morph.transforming)
    {
        a->state.x = body->state.x;
        a->state.y = body->state.y;
        Whole(&a->state.x, Pixel(a->state.x) + (a->state.facing_left ? -4 : 4));
    }
    if (a->state.flags & 0x100U)
        a->state.hitbox_state = 0;
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x708))
            return false;
        a->state.frame = a->state.hitbox_state = 0;
        a->state.flags |= 0x08000000U;
        if (!Available(s, 8))
        {
            a->state.step = 0;
            return true;
        }
        if (!ActorPrimitives(s, a, 8))
            return false;
        for (Uint16 i = 0; i < 8; ++i)
        {
            SotnActorPrimitive *p = NULL;
            if (!Packet(s, a, i, &p))
                return false;
            *p = (SotnActorPrimitive){.active = true,
                                      .page = 0x14,
                                      .palette = 0x20b,
                                      .priority_set = true,
                                      .priority = 0xa1};
            p->u[0] = p->u[2] = (Uint8)(128U + i * 11U);
            p->u[1] = p->u[3] = (Uint8)(139U + i * 11U);
            p->v[0] = p->v[1] = 80;
            p->v[2] = p->v[3] = 96;
            a->olrox_tail.angle[i] = -384;
        }
        a->olrox_tail.growth = 0;
    }
    a->state.flags |= 0x800000U;
    bool arrived = false;
    size_t pose = 0;
    switch (a->state.step)
    {
    case 1:
        if (!s->olrox_morph.transforming)
            ++a->state.step;
        break;
    case 2:
        if (a->olrox_tail.growth < 4096)
            a->olrox_tail.growth = Word(a->olrox_tail.growth + 512);
        else
        {
            a->olrox_tail.growth = 4096;
            ++a->state.step;
        }
        break;
    case 3:
    case 9: {
        SotnDataView pointer = {0};
        if (a->olrox_tail.phase > 1 ||
            !Slice(s->nz0, 0x12d0 + a->olrox_tail.phase * 4U, 4, &pointer))
            return SDL_SetError("Olrox tail phase exceeds its original pose roles");
        const Uint32 address = Read32(pointer.bytes);
        if (address < 0x80180000 || address % 2)
            return SDL_SetError("Olrox tail pose points outside its private stage");
        if (!TailPose(s, a, address - 0x80180000, a->state.step == 9 ? 64 : 32, a->state.step == 3,
                      &arrived))
            return false;
        if (arrived)
            a->olrox_tail.phase ^= 1U;
        break;
    }
    case 4:
        pose = 0x1288;
        break;
    case 6:
        pose = 0x1298;
        break;
    case 7:
        pose = 0x12a8;
        break;
    case 8:
        pose = 0x12b8;
        break;
    case 5:
        break;
    default:
        return SDL_SetError("Olrox tail entered unsupported original state %u", a->state.step);
    }
    if (pose && !TailPose(s, a, pose,
                          a->state.step == 7   ? 128
                          : a->state.step == 4 ? 32
                                               : 96,
                          true, &arrived))
        return false;
    return TailGeometry(s, a) && Override(s, a, body);
}
static bool ArmsPose(Actor *a, const Sint16 target[4], const Sint16 speed[4], bool *arrived)
{
    *arrived = true;
    for (Uint16 role = 0; role < 4; ++role)
    {
        const Uint16 i = (Uint16)((role + 2U) % 4U);
        bool reached = false;
        if (!OlroxMonster_Smooth(&a->olrox_arms.angle[i], &a->olrox_arms.velocity[i], target[i],
                                 speed[i], &reached))
            return false;
        *arrived = *arrived && reached;
    }
    return true;
}
static bool ArmGeometry(SotnActors *s, Actor *a, Actor *body, Uint16 side, Uint16 first)
{
    const bool facing = a->state.facing_left;
    const Uint16 index = (Uint16)(side * 2U);
    const Sint16 upper = a->olrox_arms.angle[index], lower = a->olrox_arms.angle[index + 1U];
    const Point origin = {Word(Pixel(a->state.x) - s->camera_x),
                          Word(Pixel(a->state.y) - s->camera_y)};
    SotnMathTrig t = {0};
    if (!Trig(s, facing ? 2048 - upper : upper, &t))
        return false;
    const Point shoulder_minus = Add(origin, Scale(-t.cosine, 8), Scale(t.sine, 8)),
                shoulder_plus = Add(origin, Scale(t.cosine, 8), Scale(-t.sine, 8));
    const Sint32 extension = a->olrox_arms.extension[side] + 256, upper_length = extension / 8,
                 lower_length = extension * 24 / 256, sign = facing ? -1 : 1;
    if (!Trig(s, upper - 1024, &t))
        return false;
    const Point elbow =
        Add(origin, sign * Scale(t.cosine, upper_length), Scale(-t.sine, upper_length));
    if (!Trig(s, facing ? 2048 - upper : upper, &t))
        return false;
    const Point elbow_minus = Add(elbow, Scale(-t.cosine, 6), Scale(t.sine, 6)),
                elbow_plus = Add(elbow, Scale(t.cosine, 6), Scale(-t.sine, 6));
    if (!Trig(s, lower + 1024, &t))
        return false;
    const Point forearm = Add(elbow, sign * Scale(t.cosine, 6), Scale(-t.sine, 6));
    if (!Trig(s, facing ? 2048 - lower : lower, &t))
        return false;
    const Point forearm_minus = Add(forearm, Scale(-t.cosine, 6), Scale(t.sine, 6)),
                forearm_plus = Add(forearm, Scale(t.cosine, 6), Scale(-t.sine, 6));
    if (!Trig(s, lower - 1024, &t))
        return false;
    const Point wrist =
        Add(forearm, sign * Scale(t.cosine, lower_length), Scale(-t.sine, lower_length));
    if (!Trig(s, facing ? 2048 - lower : lower, &t))
        return false;
    const Point wrist_minus = Add(wrist, Scale(-t.cosine, 6), Scale(t.sine, 6)),
                wrist_plus = Add(wrist, Scale(t.cosine, 6), Scale(-t.sine, 6));
    Actor *hand = OlroxChild(s, body, side ? 9 : 8);
    if (!hand || hand->state.id != 0x38)
        return SDL_SetError("Olrox arm geometry lost its original owned hand");
    Whole(&hand->state.x, wrist.x + s->camera_x);
    Whole(&hand->state.y, wrist.y + s->camera_y);
    hand->state.frame = a->olrox_arms.hand_frame[side];
    hand->state.rotation = a->olrox_arms.rotate_hands ? Word(-((Sint32)lower + 768)) : 0;
    SotnActorPrimitive *p = NULL, *q = NULL;
    if (!Packet(s, a, first, &p) || !Packet(s, a, (Uint16)(first + 1U), &q))
        return false;
    const Uint16 palette = side ? 0x20b : 0x218,
                 priority = (Uint16)((Sint32)s->olrox_monster.priority - 3 + (Sint32)side * 6);
    *p = (SotnActorPrimitive){.active = true,
                              .page = 0x14,
                              .palette = palette,
                              .priority_set = true,
                              .priority = priority};
    *q = *p;
    Geometry(p, (Point[4]){elbow_minus, shoulder_minus, elbow_plus, shoulder_plus});
    Geometry(q, (Point[4]){wrist_minus, forearm_minus, wrist_plus, forearm_plus});
    const Uint8 pu[4] = {200, 200, 212, 218}, pv[4] = {30, 4, 32, 4}, qu[4] = {136, 156, 140, 162},
                qv[4] = {116, 109, 128, 119};
    SDL_memcpy(p->u, pu, sizeof(pu));
    SDL_memcpy(p->v, pv, sizeof(pv));
    SDL_memcpy(q->u, qu, sizeof(qu));
    SDL_memcpy(q->v, qv, sizeof(qv));
    return true;
}
static bool HandAttack(SotnActors *s, Actor *body, Uint16 enemy)
{
    Actor *hand = OlroxChild(s, body, 9);
    SotnDataView pointer = {0}, value = {0};
    if (!hand || hand->state.id != 0x38 || !Slice(s->dra, 0x98, 4, &pointer))
        return SDL_SetError("Olrox arm attack lost its original front hand or enemy table");
    const Uint32 address = Read32(pointer.bytes);
    if (address < 0x800a0000 || address % 4 || enemy >= 400 ||
        !Slice(s->dra, address - 0x800a0000 + (size_t)enemy * 40U + 6U, 2, &value))
        return SDL_SetError("Olrox arm attack references an invalid private enemy definition");
    hand->attack = Read16(value.bytes);
    return true;
}
static bool Arms(SotnActors *s, Actor *a, Actor *body)
{
    Actor *back = OlroxChild(s, body, 8), *front = OlroxChild(s, body, 9);
    if (!back || !front || back->state.id != 0x38 || front->state.id != 0x38)
        return SDL_SetError("Olrox arms require both generation-owned hands");
    a->state.facing_left = body->state.facing_left;
    a->state.palette = body->state.palette;
    if (!body->olrox.death_phase)
    {
        a->olrox_arms.damaged = (Uint8)(((back->state.flags & 0x100U) ? 2U : 0U) |
                                        ((front->state.flags & 0x100U) ? 1U : 0U));
        if (a->olrox_arms.damaged && a->state.step != 10)
            a->state.step = 9;
    }
    if (a->state.flags & 0x100U)
        a->state.hitbox_state = 0;
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x708))
            return false;
        a->state.hitbox_width = 12;
        a->state.hitbox_height = 8;
        a->state.hitbox_x = 0;
        a->state.hitbox_y = 8;
        a->state.frame = 0;
        if (!Available(s, 6))
        {
            a->state.step = 0;
            return true;
        }
        if (!ActorPrimitives(s, a, 6))
            return false;
        a->olrox_arms.angle[2] = 384;
        a->olrox_arms.angle[3] = -384;
        a->olrox_arms.angle[0] = 256;
        a->olrox_arms.angle[1] = -128;
        a->olrox_arms.hand_frame[1] = 29;
        a->olrox_arms.hand_frame[0] = 33;
        a->olrox_arms.rotate_hands = 0;
    }
    a->state.flags |= 0x800000U;
    bool pose = false, arrived = false, finish = false;
    Sint16 target[4] = {0}, speed[4] = {0};
    Uint16 attack = 0;
    const Uint16 state = a->state.step;
    switch (state)
    {
    case 1:
        a->substep = 0;
        attack = 55;
        break;
    case 2:
        a->olrox_arms.rotate_hands = 0;
        a->olrox_arms.hand_frame[1] = 29;
        a->olrox_arms.hand_frame[0] = 33;
        pose = true;
        if (a->substep == 0)
        {
            SDL_memcpy(target, (Sint16[4]){256, -768, -64, -512}, sizeof(target));
        }
        else if (a->substep == 1)
        {
            SDL_memcpy(target, (Sint16[4]){0, -640, 128, -768}, sizeof(target));
        }
        else
            return SDL_SetError("Olrox walking arms have an invalid subphase");
        for (Uint16 i = 0; i < 4; ++i)
            speed[i] = 16;
        break;
    case 3:
        pose = finish = true;
        a->olrox_arms.hand_frame[1] = 31;
        a->olrox_arms.hand_frame[0] = 32;
        a->olrox_arms.rotate_hands = 0;
        SDL_memcpy(target, (Sint16[4]){256, -640, 256, -768}, sizeof(target));
        for (Uint16 i = 0; i < 4; ++i)
            speed[i] = 16;
        break;
    case 4:
        pose = true;
        a->olrox_arms.hand_frame[1] = 31;
        a->olrox_arms.hand_frame[0] = 32;
        a->olrox_arms.rotate_hands = 0;
        SDL_memcpy(target, (Sint16[4]){512, -768, -576, -576}, sizeof(target));
        for (Uint16 i = 0; i < 4; ++i)
            speed[i] = 96;
        attack = 57;
        break;
    case 5:
        a->olrox_arms.rotate_hands = 0;
        if (ByteSigned(a->olrox_arms.extension[1]) > 0)
            a->olrox_arms.extension[1] = (Uint8)(a->olrox_arms.extension[1] - 12U);
        else
        {
            a->olrox_arms.extension[1] = 0;
            a->state.step = 1;
        }
        break;
    case 6:
        pose = finish = true;
        a->olrox_arms.hand_frame[1] = 31;
        a->olrox_arms.hand_frame[0] = 32;
        a->olrox_arms.rotate_hands = 0;
        SDL_memcpy(target, (Sint16[4]){0, -896, 128, -640}, sizeof(target));
        for (Uint16 i = 0; i < 4; ++i)
            speed[i] = 32;
        break;
    case 7:
        pose = finish = true;
        a->olrox_arms.hand_frame[1] = 29;
        a->olrox_arms.hand_frame[0] = 32;
        a->olrox_arms.rotate_hands = 1;
        SDL_memcpy(target, (Sint16[4]){0, -512, 256, -256}, sizeof(target));
        for (Uint16 i = 0; i < 4; ++i)
            speed[i] = 16;
        break;
    case 8:
        pose = finish = true;
        a->olrox_arms.hand_frame[1] = 31;
        a->olrox_arms.hand_frame[0] = 33;
        a->olrox_arms.rotate_hands = 1;
        SDL_memcpy(target, (Sint16[4]){0, -320, 384, -256}, sizeof(target));
        for (Uint16 i = 0; i < 4; ++i)
            speed[i] = 32;
        break;
    case 9:
        pose = true;
        a->olrox_arms.hand_frame[1] = 29;
        a->olrox_arms.hand_frame[0] = 32;
        a->olrox_arms.rotate_hands = 1;
        target[0] = a->olrox_arms.damaged == 2 ? 0 : 512;
        target[1] = -256;
        speed[0] = speed[1] = a->olrox_arms.damaged == 2 ? 24 : 16;
        target[2] = a->olrox_arms.damaged == 1 ? 0 : 512;
        target[3] = -256;
        speed[2] = speed[3] = a->olrox_arms.damaged == 1 ? 24 : 16;
        break;
    case 10:
        if (a->substep == 0)
        {
            if (body->olrox.death_phase == 2)
                ++a->substep;
        }
        else if (a->substep == 1 || a->substep == 2)
        {
            pose = true;
            a->olrox_arms.rotate_hands = 1;
            a->olrox_arms.hand_frame[1] = 29;
            a->olrox_arms.hand_frame[0] = 33;
            if (a->substep == 1)
            {
                SDL_memcpy(target, (Sint16[4]){-256, -1024, -512, -768}, sizeof(target));
                for (Uint16 i = 0; i < 4; ++i)
                    speed[i] = 16;
            }
            else
            {
                SDL_memcpy(target, (Sint16[4]){256, -512, 0, -256}, sizeof(target));
                speed[0] = speed[1] = 10;
                speed[2] = speed[3] = 6;
            }
        }
        break;
    case 11:
        pose = finish = true;
        a->olrox_arms.rotate_hands = 1;
        a->olrox_arms.hand_frame[1] = 31;
        a->olrox_arms.hand_frame[0] = 32;
        SDL_memcpy(target, (Sint16[4]){-512, -1024, 640, -512}, sizeof(target));
        speed[0] = speed[1] = 32;
        speed[2] = speed[3] = 24;
        break;
    case 12:
        pose = finish = true;
        a->olrox_arms.rotate_hands = 1;
        a->olrox_arms.hand_frame[1] = 29;
        a->olrox_arms.hand_frame[0] = 33;
        if (ByteSigned(a->olrox_arms.extension[1]) < 64)
            a->olrox_arms.extension[1] = (Uint8)(a->olrox_arms.extension[1] + 8U);
        SDL_memcpy(target, (Sint16[4]){384, -512, -1280, -1920}, sizeof(target));
        speed[0] = speed[1] = 64;
        speed[2] = 96;
        speed[3] = 120;
        attack = 58;
        break;
    case 13:
        pose = finish = true;
        a->olrox_arms.rotate_hands = 1;
        a->olrox_arms.hand_frame[1] = 31;
        a->olrox_arms.hand_frame[0] = 33;
        a->olrox_arms.extension[1] = (Uint8)(a->olrox_arms.extension[1] - 12U);
        if (a->olrox_arms.extension[1] & 128U)
            a->olrox_arms.extension[1] = 0;
        SDL_memcpy(target, (Sint16[4]){0, -128, 256, -512}, sizeof(target));
        speed[0] = speed[1] = speed[2] = 64;
        speed[3] = 80;
        break;
    case 14:
        pose = finish = true;
        a->olrox_arms.rotate_hands = 1;
        a->olrox_arms.hand_frame[1] = 31;
        a->olrox_arms.hand_frame[0] = 32;
        a->olrox_arms.extension[1] = (Uint8)(a->olrox_arms.extension[1] - 12U);
        if (a->olrox_arms.extension[1] & 128U)
            a->olrox_arms.extension[1] = 0;
        SDL_memcpy(target, (Sint16[4]){256, -768, -128, -512}, sizeof(target));
        for (Uint16 i = 0; i < 4; ++i)
            speed[i] = 32;
        break;
    case 15:
        pose = finish = true;
        a->olrox_arms.rotate_hands = 1;
        a->olrox_arms.hand_frame[1] = 29;
        a->olrox_arms.hand_frame[0] = 32;
        SDL_memcpy(target, (Sint16[4]){0, -512, 128, -256}, sizeof(target));
        for (Uint16 i = 0; i < 4; ++i)
            speed[i] = 32;
        break;
    default:
        return SDL_SetError("Olrox arms entered unsupported original state %u", state);
    }
    if (pose && !ArmsPose(a, target, speed, &arrived))
        return false;
    if (state == 2 && arrived)
        a->substep ^= 1U;
    if (state == 4)
    {
        if (a->olrox_arms.angle[2] < 128)
            body->olrox.body_style = 0;
        if (a->olrox_arms.angle[2] < -128)
            body->olrox.body_style = 3;
        if (arrived)
        {
            if (ByteSigned(a->olrox_arms.extension[1]) < 96)
                a->olrox_arms.extension[1] = (Uint8)(a->olrox_arms.extension[1] + 16U);
            else
            {
                a->state.step = 5;
                attack = 55;
                if (!ActorPositionalSound(s, a, 0x7c8))
                    return false;
            }
        }
    }
    if (state == 9 && !a->olrox_arms.damaged)
        a->state.step = 1;
    if (state == 10 && pose)
    {
        if (a->substep == 1 && arrived)
            ++a->substep;
        if (body->olrox.death_phase == 3)
            a->substep = 3;
    }
    if (finish && arrived)
        a->state.step = 1;
    if (attack && !HandAttack(s, body, attack))
        return false;
    return ArmGeometry(s, a, body, 1, 0) && ArmGeometry(s, a, body, 0, 2) && Override(s, a, body);
}
static bool Foot(SotnActors *s, Actor *a, Actor *body, Uint16 role)
{
    if (role > 2 || !role)
        return SDL_SetError("Olrox foot has an invalid original leg role");
    const Actor *previous = role == 1 ? body : OlroxChild(s, body, 1);
    if (!previous)
        return SDL_SetError("Olrox foot lost its preceding owned facing source");
    a->state.facing_left = previous->state.facing_left;
    if (a->state.flags & 0x100U)
        a->state.hitbox_state = 0;
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x708))
            return false;
        a->state.hitbox_width = 12;
        a->state.hitbox_height = 4;
        a->state.hitbox_x = -12;
        a->state.frame = 0;
        return true;
    }
    if (a->state.step == 1)
    {
        a->state.x = s->olrox_monster.legs[role - 1U].foot_x;
        a->state.y = s->olrox_monster.legs[role - 1U].foot_y;
    }
    return true;
}
static bool Knee(SotnActors *s, Actor *a, Uint16 role)
{
    if (role < 5 || role > 6)
        return SDL_SetError("Olrox knee has an invalid original leg role");
    if (a->state.flags & 0x100U)
        a->state.hitbox_state = 0;
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x708))
            return false;
        a->state.hitbox_y = 4;
        a->state.hitbox_width = 4;
        a->state.frame = 0;
        a->state.hitbox_height = 16;
    }
    const OlroxLeg *l = &s->olrox_monster.legs[role - 5U];
    Whole(&a->state.x, Pixel(l->knee_x));
    Whole(&a->state.y, Pixel(l->knee_y));
    return true;
}
static bool Explosions(SotnActors *s, const Actor *a)
{
    const Uint8 kind = (Uint8)(ActorRandom(s) & 3U);
    const Sint16 angle = (Sint16)((Sint32)(ActorRandom(s) & 15U) * 256 - 2048);
    for (Uint16 i = 0; i < 6; ++i)
    {
        Actor *effect = NULL;
        if (!ActorSpawn(s, a, 0x28, 0, 160, 192, &effect))
            return false;
        if (effect)
        {
            effect->state.x = Pixel(a->state.x) * 65536;
            effect->state.y = Pixel(a->state.y) * 65536;
            effect->state.facing_left = false;
            effect->olrox.angle12 = angle;
            effect->olrox.angle = kind;
            effect->olrox.decision = (Uint8)(6U - i);
        }
    }
    return true;
}
static bool Hand(SotnActors *s, Actor *a, Actor *body, Uint16 role)
{
    if ((role != 8 && role != 9) || a->state.parameters != (role == 8 ? 1 : 0))
        return SDL_SetError("Olrox hand requires its original reserved role and parameter");
    a->state.facing_left = body->state.facing_left;
    if (body->state.palette & 0x8000U)
        a->state.palette = body->state.palette;
    else if (!(a->state.flags & 15U))
        a->state.palette = 0x20b;
    if (a->state.flags & 0x100U)
        a->state.hitbox_state = 0;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x708))
            return false;
        a->state.hit_points = 128;
        a->state.hitbox_width = 12;
        a->state.hitbox_height = 6;
        a->state.transform_flags_set = true;
        a->state.transform_flags |= SOTN_ACTOR_ROTATE;
        a->state.rotated = true;
        a->state.frame = a->state.parameters ? 32 : 31;
        break;
    case 1:
        if (a->state.flags & 0x100U)
        {
            a->state.hitbox_state = 0;
            a->state.step = 2;
            a->substep = 0;
        }
        break;
    case 2:
        switch (a->substep)
        {
        case 0:
            if (!Explosions(s, a))
                return false;
            a->olrox.timer = 256;
            a->state.frame = 0;
            a->state.scale_x = a->state.scale_y = 0;
            a->state.transform_flags |= SOTN_ACTOR_SCALE_X | SOTN_ACTOR_SCALE_Y;
            ++a->substep;
            a->state.flags &= 0xffffff00U;
            break;
        case 1:
            if (!--a->olrox.timer)
                ++a->substep;
            break;
        case 2:
            a->state.scale_x = (Uint16)(a->state.scale_x + 2U);
            a->state.scale_y = a->state.scale_x;
            if (Word(a->state.scale_x) >= 257)
            {
                a->state.hit_points = 128;
                a->state.transform_flags = SOTN_ACTOR_ROTATE;
                a->state.hitbox_state = 3;
                a->state.palette = 0x20b;
                a->state.step = 1;
                a->state.flags &= ~0x100U;
                a->stun = 0;
                a->substep = 0;
            }
            break;
        default:
            return SDL_SetError("Olrox hand recovery has an invalid subphase");
        }
        break;
    case 3:
        a->state.transform_flags = SOTN_ACTOR_ROTATE;
        switch (a->substep)
        {
        case 0:
            a->state.hitbox_state = 0;
            if (!Available(s, 1))
            {
                a->substep = 2;
                break;
            }
            a->state.flags |= 0x800000U;
            if (!ActorPrimitives(s, a, 1) || !OlroxMonster_FreezeSprite(s, a, body, 0))
                return false;
            {
                SotnActorPrimitive *p = NULL;
                if (!Packet(s, a, 0, &p))
                    return false;
                p->page = 0x14;
                p->palette = (Uint16)(p->palette + 0x20bU);
                p->active = true;
                p->priority_set = true;
                p->priority = a->state.priority;
            }
            ++a->substep;
            break;
        case 1:
            a->state.palette = 0x219;
            a->state.texture_offset = 0x54;
            if (body->olrox.death_phase)
            {
                SotnActorPrimitive *p = NULL;
                if (!Packet(s, a, 0, &p))
                    return false;
                p->active = false;
                ++a->substep;
            }
            break;
        case 2:
            a->state.palette = 0x8219;
            a->state.texture_offset = 0x54;
            if (body->olrox.death_phase == 3)
            {
                if (!a->state.primitive_count)
                    break;
                if (!OlroxMonster_FreezeSprite(s, a, body, 0))
                    return false;
                SotnActorPrimitive *p = NULL;
                if (!Packet(s, a, 0, &p))
                    return false;
                p->page = 0x15;
                p->palette = 0x219;
                p->active = true;
                p->priority_set = true;
                p->priority = a->state.priority;
                ++a->substep;
            }
            break;
        case 3:
            break;
        default:
            return SDL_SetError("Olrox hand defeat has an invalid subphase");
        }
        break;
    default:
        return SDL_SetError("Olrox hand entered unsupported original state %u", a->state.step);
    }
    a->state.priority =
        (Uint16)((Sint32)s->olrox_monster.priority + (a->state.parameters ? -4 : 2));
    if (body->state.flags & 0x100U)
        a->state.hitbox_state = 0;
    SotnMathTrig t = {0};
    if (!Trig(s, a->state.rotation - 256, &t))
        return false;
    a->state.hitbox_x = Word(-Scale(t.cosine, 10));
    a->state.hitbox_y = Word(-Scale(t.sine, 10));
    return true;
}
bool ActorOlroxAppendage(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 30 || !a->state.generation ||
        (a->state.id != 0x34 && a->state.id != 0x36 && a->state.id != 0x37 && a->state.id != 0x38 &&
         a->state.id != 0x3b))
        return SDL_SetError("Olrox appendage requires its active original BO0 identity");
    if (!a->enemy_state.owner_generation || a->enemy_state.owner_slot >= SOTN_STAGE_ACTOR_SLOTS)
    {
        a->remove = true;
        return true;
    }
    Actor *body = &s->slots[a->enemy_state.owner_slot];
    if (body->state.id != 0x33 || body->state.generation != a->enemy_state.owner_generation ||
        a->slot <= body->slot || a->slot - body->slot > 13 ||
        !OlroxOwnsChild(s, body, a, (Uint16)(a->slot - body->slot)))
    {
        a->remove = true;
        return true;
    }
    if (!OlroxMonster_Bind(s, body))
        return false;
    const Uint16 role = (Uint16)(a->slot - body->slot);
    switch (a->state.id)
    {
    case 0x34:
        return Foot(s, a, body, role);
    case 0x36:
        if (role != 4)
            return SDL_SetError("Olrox tail occupies an invalid owned role");
        return Tail(s, a, body);
    case 0x37:
        if (role != 7)
            return SDL_SetError("Olrox arms occupy an invalid owned role");
        return Arms(s, a, body);
    case 0x38:
        return Hand(s, a, body, role);
    case 0x3b:
        return Knee(s, a, role);
    default:
        return false;
    }
}
bool ActorOlroxFirework(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 30 || a->state.id != 0x28 || !a->state.generation)
        return SDL_SetError("Olrox firework requires its active original BO0 identity");
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x60c))
            return false;
        a->state.animation_set = 2;
        a->state.palette = 0x81b6;
        a->state.transform_flags_set = true;
        a->state.transform_flags |= SOTN_ACTOR_ROTATE | SOTN_ACTOR_OPACITY;
        a->state.blend_mode |= 0x30U;
        a->state.opacity = 112;
        a->state.priority = 192;
        if (a->olrox.angle == 1 && a->olrox.decision > 3)
        {
            a->olrox.decision = (Uint8)(a->olrox.decision - 3U);
            a->olrox.angle12 = Word(a->olrox.angle12 - 2048);
        }
        else if (a->olrox.angle == 2)
            a->olrox.angle12 = Word(a->olrox.angle12 + a->olrox.decision * 192);
        a->olrox.angle12 = (Sint16)((Uint16)a->olrox.angle12 & 4095U);
        a->state.rotation = a->olrox.angle12;
        SotnMathTrig t = {0};
        if (!Trig(s, a->olrox.angle12, &t))
            return false;
        const Sint32 speed = a->olrox.decision * 320 / 24;
        a->vx = speed * t.sine;
        a->vy = -speed * t.cosine;
    }
    if (a->pose > 12)
    {
        a->vx = (a->vx / 4) * 3;
        a->vy = (a->vy / 4) * 3;
    }
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Olrox firework exceeds its fixed-point motion bounds");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    Uint8 animated = 0;
    if (!ActorAnimate(s, a, 0x1cd4, &animated))
        return false;
    if (!animated)
        a->remove = true;
    return true;
}
