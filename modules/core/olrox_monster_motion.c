// SPDX-License-Identifier: AGPL-3.0-or-later
// Reconstructed BO0 313A8.c helpers 801B1590..801B24CC at
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. SDK tables stay private.
#include "olrox_monster_internal.h"

static Sint16 Word(Sint32 value)
{
    return (Sint16)Wrap16(value);
}
static Sint32 Fixed(Sint64 value)
{
    const Uint32 bits = (Uint32)value;
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
bool OlroxMonster_Bind(SotnActors *s, const Actor *a)
{
    if (!s || !a || s->stage_id != 30 || a->state.id != 0x33 || !a->state.generation)
        return SDL_SetError("Olrox articulation requires an active original BO0 monster");
    if (s->olrox_monster.generation && s->olrox_monster.generation != a->state.generation)
        return SDL_SetError("Olrox articulation is owned by a different body generation");
    s->olrox_monster.generation = a->state.generation;
    return true;
}
bool OlroxMonster_Approach(Sint16 *value, Sint16 target, Sint16 step, bool *arrived)
{
    if (!value || !arrived || step <= 0 || step > 4096)
        return SDL_SetError("Olrox angle approach requires an angle, result and positive step");
    *arrived = SDL_abs((Sint32)*value - target) < step;
    if (*arrived)
        *value = target;
    else
    {
        if (target < *value)
            *value = Word((Sint32)*value - step);
        if (*value < target)
            *value = Word((Sint32)*value + step);
    }
    return true;
}
static bool Smooth(Sint16 *value, Sint16 *velocity, Sint16 target, Sint16 speed)
{
    const Sint16 delta = Word((Sint32)target - *value);
    const Sint32 acceleration = SDL_max(1, speed / 16);
    const Sint16 next = Word(
        (Sint32)*velocity +
        ((delta > 0 ? *velocity >= speed : *velocity > -speed) ? -acceleration : acceleration));
    if (SDL_abs((Sint32)delta) >= SDL_abs((Sint32)next))
    {
        *value = Word((Sint32)*value + next);
        *velocity = next;
        return false;
    }
    *value = target;
    *velocity = delta;
    return true;
}
bool OlroxMonster_Smooth(Sint16 *value, Sint16 *velocity, Sint16 target, Sint16 speed,
                         bool *arrived)
{
    if (!value || !velocity || value == velocity || !arrived || speed <= 0 || speed > 4096)
        return SDL_SetError("Olrox angle smoothing requires initialized words and positive speed");
    *arrived = Smooth(value, velocity, target, speed);
    return true;
}
static bool Joint(Sint16 *angle, Sint16 *velocity, Sint16 target, Sint16 amount)
{
    const Sint16 delta = Word((Sint32)target - *angle);
    if (SDL_abs((Sint32)delta) < amount)
    {
        *angle = target;
        *velocity = delta;
        return true;
    }
    const Sint16 change = delta > 0 ? amount : (Sint16)-amount;
    *angle = Word((Sint32)*angle + change);
    *velocity = change;
    return false;
}
static bool Pair(OlroxLeg *leg, Sint16 upper, Sint16 lower, Sint16 amount)
{
    const bool a = Joint(&leg->upper, &leg->upper_velocity, upper, amount);
    const bool b = Joint(&leg->lower, &leg->lower_velocity, lower, amount);
    return a && b;
}
static void Advance(OlroxLeg *leg, Sint16 upper, Sint16 lower, Sint16 amount)
{
    if (Pair(leg, upper, lower, amount))
        ++leg->phase;
}
static void Forward(Actor *a, OlroxLeg *leg, bool support, bool slow)
{
    if (!support)
    {
        if (leg->phase == 0)
            Advance(leg, -1024, 512, slow ? 8 : 24);
        else if (leg->phase == 1)
            Advance(leg, -1024, -64, slow ? 8 : 32);
        else if (leg->phase == 2)
            Advance(leg, -768, -64, slow ? 12 : 32);
    }
    else if (leg->phase == 0)
    {
        Advance(leg, -64, 64, slow ? 8 : 28);
        if (!slow && a->olrox.body_angle >= -255)
            a->olrox.body_angle = Word((Sint32)a->olrox.body_angle - 12);
    }
    else if (leg->phase == 1)
    {
        Advance(leg, -256, slow ? 896 : 768, slow ? 20 : 24);
        if (!slow && a->olrox.body_angle < 256)
            a->olrox.body_angle = Word((Sint32)a->olrox.body_angle + 8);
    }
}
static void Backward(Actor *a, OlroxLeg *leg, bool support)
{
    if (!support)
    {
        if (leg->phase == 0)
            Advance(leg, -512, 768, 24);
        else if (leg->phase == 1)
            Advance(leg, 0, 640, 28);
        else if (leg->phase == 2)
            Advance(leg, 128, 384, 24);
    }
    else if (leg->phase == 0)
    {
        Advance(leg, -128, 128, 28);
        if (a->olrox.body_angle >= -255)
            a->olrox.body_angle = Word((Sint32)a->olrox.body_angle - 16);
    }
    else if (leg->phase == 1)
        Advance(leg, -256, 0, 24);
    else if (leg->phase == 2)
    {
        Advance(leg, -640, -192, 20);
        if (a->olrox.body_angle < 256)
            a->olrox.body_angle = Word((Sint32)a->olrox.body_angle + 16);
    }
}
static bool Takeoff(OlroxLeg *a, OlroxLeg *b)
{
    if (a->phase == 0)
    {
        const bool first = Pair(a, -384, 0, 32), second = Pair(b, 128, 640, 32);
        if (first && second)
            ++a->phase;
    }
    else if (a->phase == 1)
    {
        const bool first = Pair(a, -512, -256, 64);
        Pair(b, -256, 640, 64);
        if (first)
            ++a->phase;
    }
    else if (a->phase == 2)
    {
        const bool first = Pair(a, -640, -512, 96);
        Pair(b, -384, 768, 64);
        return first;
    }
    return false;
}
bool OlroxMonster_Pose(SotnActors *s, Actor *body, Uint16 index, OlroxLegPose pose, bool *arrived)
{
    if (!arrived || index > 1 || pose < OLROX_FORWARD_SWING || pose > OLROX_SLOW_SUPPORT ||
        !OlroxMonster_Bind(s, body))
        return SDL_SetError("Olrox pose requires its owned body, leg and original controller");
    OlroxLeg *a = &s->olrox_monster.legs[index], *b = &s->olrox_monster.legs[index ^ 1U];
    *arrived = false;
    switch (pose)
    {
    case OLROX_FORWARD_SWING:
    case OLROX_FORWARD_SUPPORT:
    case OLROX_SLOW_SWING:
    case OLROX_SLOW_SUPPORT:
        Forward(body, a, pose == OLROX_FORWARD_SUPPORT || pose == OLROX_SLOW_SUPPORT,
                pose == OLROX_SLOW_SWING || pose == OLROX_SLOW_SUPPORT);
        break;
    case OLROX_BACKWARD_SWING:
    case OLROX_BACKWARD_SUPPORT:
        Backward(body, a, pose == OLROX_BACKWARD_SUPPORT);
        break;
    case OLROX_TAKEOFF:
        *arrived = Takeoff(a, b);
        break;
    case OLROX_LOWER_BODY: {
        Smooth(&body->olrox.body_angle, &body->olrox.body_velocity, -256, 12);
        const bool first = Pair(a, -448, 640, 16), second = Pair(b, -896, 320, 16);
        *arrived = first && second;
        break;
    }
    case OLROX_RAISE_BODY: {
        const bool tilt = Smooth(&body->olrox.body_angle, &body->olrox.body_velocity, 384, 64);
        if (a->phase == 0)
        {
            Pair(a, -96, 832, 24);
            if (Pair(b, -1280, 128, 32))
                ++a->phase;
        }
        else if (a->phase == 1)
        {
            const bool first = Pair(a, -64, 832, 24), second = Pair(b, -960, 224, 32);
            *arrived = tilt && first && second;
        }
        break;
    }
    case OLROX_WIDE_STANCE:
    case OLROX_LANDING:
    case OLROX_CROUCH: {
        Smooth(&body->olrox.body_angle, &body->olrox.body_velocity, 0,
               pose == OLROX_CROUCH ? 16 : 12);
        const bool first = pose == OLROX_WIDE_STANCE ? Pair(a, -704, 704, 40)
                           : pose == OLROX_LANDING   ? Pair(a, -64, 64, 96)
                                                     : Pair(a, -512, 512, 96);
        const bool second = pose == OLROX_WIDE_STANCE ? Pair(b, -256, 960, 56)
                            : pose == OLROX_LANDING   ? Pair(b, -512, 640, 80)
                                                      : Pair(b, -512, 384, 64);
        *arrived = first && second;
        break;
    }
    case OLROX_RECOVER:
        if (a->phase == 0)
        {
            const bool first = Pair(a, -128, 128, 32), second = Pair(b, 0, 384, 12);
            if (first && second)
                ++a->phase;
        }
        else if (a->phase == 1)
        {
            const bool first = Pair(a, -768, 0, 40), second = Pair(b, 128, 256, 12);
            *arrived = first && second;
        }
        break;
    }
    return true;
}
bool OlroxMonster_ChangeStep(SotnActors *s, Actor *body, Uint16 step)
{
    if (!OlroxMonster_Bind(s, body))
        return false;
    body->state.step = (Uint16)(step & 255U);
    body->substep = 0;
    s->olrox_monster.legs[0].phase = s->olrox_monster.legs[1].phase = 0;
    return true;
}
static bool Trig(SotnActors *s, const Actor *a, Sint16 angle, SotnMathTrig *out)
{
    const Sint16 rotated = Word(3072 + (a->state.facing_left ? -(Sint32)angle : angle));
    return SotnMath_Trig(s->executable, rotated, out);
}
bool OlroxMonster_Anchor(SotnActors *s, Actor *body, bool from_hip)
{
    if (!OlroxMonster_Bind(s, body) || body->olrox.support_leg > 1)
        return SDL_SetError("Olrox anchoring requires its body and original support leg");
    Actor *left = OlroxChild(s, body, 1), *right = OlroxChild(s, body, 2);
    if (!left || !right || left->state.id != 0x34 || right->state.id != 0x34)
        return SDL_SetError("Olrox anchoring lost an owned original foot generation");
    OlroxLeg *legs = s->olrox_monster.legs;
    legs[0].foot_x = left->state.x;
    legs[0].foot_y = left->state.y;
    legs[1].foot_x = right->state.x;
    legs[1].foot_y = right->state.y;
    SotnMathTrig upper[2] = {{0}}, lower[2] = {{0}};
    for (Uint16 i = 0; i < 2; ++i)
        if (!Trig(s, body, legs[i].upper, &upper[i]) || !Trig(s, body, legs[i].lower, &lower[i]))
            return false;
    Sint32 hip_x = body->state.x, hip_y = body->state.y;
    const Uint16 support = body->olrox.support_leg;
    if (!from_hip)
    {
        OlroxLeg *leg = &legs[support];
        leg->knee_x = Fixed((Sint64)leg->foot_x - lower[support].cosine * 672);
        leg->knee_y = Fixed((Sint64)leg->foot_y + lower[support].sine * 672);
        hip_x = Fixed((Sint64)leg->knee_x - upper[support].cosine * 448);
        hip_y = Fixed((Sint64)leg->knee_y + upper[support].sine * 448);
        const Sint32 distance = Fixed((Sint64)leg->foot_x - hip_x);
        leg->horizontal = body->state.facing_left ? distance : Fixed(-(Sint64)distance);
        body->state.x = hip_x;
        body->state.y = Fixed((Sint64)hip_y - 4 * 65536);
    }
    for (Uint16 i = 0; i < 2; ++i)
    {
        if (!from_hip && i == support)
            continue;
        OlroxLeg *leg = &legs[i];
        leg->knee_x = Fixed((Sint64)hip_x + upper[i].cosine * 448);
        leg->knee_y = Fixed((Sint64)hip_y - upper[i].sine * 448);
        leg->foot_x = Fixed((Sint64)leg->knee_x + lower[i].cosine * 672);
        leg->foot_y = Fixed((Sint64)leg->knee_y - lower[i].sine * 672);
        const Sint32 distance = Fixed((Sint64)leg->foot_x - hip_x);
        const bool negate = from_hip ? body->state.facing_left : !body->state.facing_left;
        leg->horizontal = negate ? Fixed(-(Sint64)distance) : distance;
    }
    return true;
}
bool OlroxMonster_Floor(SotnActors *s, Actor *body, Uint16 index, Sint16 *adjustment)
{
    if (!adjustment || index > 1 || !OlroxMonster_Bind(s, body))
        return SDL_SetError("Olrox floor adjustment requires its owned leg and result");
    OlroxLeg *leg = &s->olrox_monster.legs[index];
    const Sint32 y = Pixel(leg->foot_y);
    leg->contact = y >= 460 ? 1 : 0;
    *adjustment = leg->contact ? Word(460 - y) : 0;
    return true;
}
