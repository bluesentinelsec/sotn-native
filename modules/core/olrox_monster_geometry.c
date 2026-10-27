// SPDX-License-Identifier: AGPL-3.0-or-later
// Reconstructed BO0 313A8.c helpers 801B13A8, 801B2690 and 801B30AC at
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Profiles/art stay private.
#include "olrox_monster_internal.h"

typedef struct Point
{
    Sint16 x, y;
} Point;
static Sint16 Word(Sint32 value)
{
    return (Sint16)Wrap16(value);
}
static Sint32 Scale(Sint32 value, Sint32 amount)
{
    return ActorShift((Sint64)value * amount, 12);
}
static Point Add(Point p, Sint32 x, Sint32 y)
{
    return (Point){Word(p.x + x), Word(p.y + y)};
}
static bool Trig(SotnActors *s, Sint32 angle, SotnMathTrig *out)
{
    return SotnMath_Trig(s->executable, Word(angle), out);
}
static Point Offset(Point p, SotnMathTrig t, Sint32 amount)
{
    return Add(p, Scale(t.cosine, amount), Scale(-t.sine, amount));
}
static Point Subtract(Point p, SotnMathTrig t, Sint32 amount)
{
    return Add(p, -Scale(t.cosine, amount), -Scale(-t.sine, amount));
}
static Point SubtractSine(Point p, SotnMathTrig t, Sint32 amount)
{
    return Add(p, Scale(t.cosine, amount), -Scale(t.sine, amount));
}
static bool Parts(SotnActors *s, Actor *body, Uint16 first, Uint16 count, SotnActorPrimitive **out)
{
    if (!OlroxMonster_Bind(s, body) || first > body->state.primitive_count ||
        count > body->state.primitive_count - first)
        return SDL_SetError("Olrox geometry requires its original owned primitive range");
    for (Uint16 i = 0; i < count; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, body, (Uint16)(first + i));
        if (!p)
            return false;
        out[i] = &p->state;
    }
    return true;
}
static void Geometry(SotnActorPrimitive *p, const Point points[4])
{
    p->x = p->y = 0;
    p->quad = p->textured = p->screen_space = true;
    for (Uint16 i = 0; i < 4; ++i)
    {
        p->offset_x[i] = points[i].x;
        p->offset_y[i] = points[i].y;
    }
}
static void Packet(SotnActorPrimitive *p, Uint16 palette, Uint16 priority, const Point points[4])
{
    *p = (SotnActorPrimitive){.active = true,
                              .priority_set = true,
                              .priority = priority,
                              .page = 0x14,
                              .palette = palette};
    Geometry(p, points);
}
static void UV(SotnActorPrimitive *p, Sint32 u0, Sint32 u1, Sint32 u2, Sint32 u3, Sint32 v0,
               Sint32 v1, Sint32 v2, Sint32 v3)
{
    p->u[0] = (Uint8)u0;
    p->u[1] = (Uint8)u1;
    p->u[2] = (Uint8)u2;
    p->u[3] = (Uint8)u3;
    p->v[0] = (Uint8)v0;
    p->v[1] = (Uint8)v1;
    p->v[2] = (Uint8)v2;
    p->v[3] = (Uint8)v3;
}
static bool Bend(SotnActors *s, const Actor *body, Point start, Sint16 *angle, Sint32 length,
                 Sint32 scroll, Point *out)
{
    SotnMathTrig t = {0};
    if (!Trig(s, (Sint32)*angle + 2048, &t))
        return false;
    *out = SubtractSine(start, t, length);
    if (Word(out->y + scroll) < 464)
        return true;
    const Sint32 difference = Word(start.y + scroll - 464),
                 squared = length * length - difference * difference;
    Uint32 root = 0;
    if (squared < 0)
        return SDL_SetError("Olrox foot projection exceeds its original circular floor arc");
    if (!SotnMath_Sqrt12(s->executable, (Uint32)squared, &root))
        return false;
    const Sint32 distance = (Sint32)(root >> 6);
    out->x = Word(start.x + (body->state.facing_left ? distance : -distance));
    out->y = Word(464 - scroll);
    return SotnMath_Atan2(s->executable, -difference, start.x - out->x, angle);
}
bool OlroxMonster_LegGeometry(SotnActors *s, Actor *body, Uint16 index, Uint16 first, Uint16 side)
{
    SotnActorPrimitive *p[4] = {0};
    if (index > 1 || side > 1 || !Parts(s, body, first, 4, p))
        return SDL_SetError("Olrox leg geometry requires its original leg, side and packet owner");
    const OlroxLeg leg = s->olrox_monster.legs[index];
    const bool facing = body->state.facing_left;
    const Sint16 upper_angle = Word(facing ? -(Sint32)leg.upper : leg.upper + 2048),
                 lower_angle = Word(facing ? -(Sint32)leg.lower : leg.lower + 2048),
                 middle_angle = Word(upper_angle + ((Sint32)lower_angle - upper_angle) / 2);
    Sint16 spread = Word(facing ? leg.upper - ((Sint32)leg.lower - 2048)
                                : upper_angle - ((Sint32)lower_angle - 2048));
    if (spread >= 1025)
        spread = 1024;
    SotnMathTrig upper = {0}, lower = {0}, middle = {0}, spread_trig = {0};
    if (!Trig(s, upper_angle, &upper) || !Trig(s, lower_angle, &lower) ||
        !Trig(s, middle_angle, &middle) || !Trig(s, spread, &spread_trig))
        return false;
    const Point hip = {Word(Pixel(body->state.x) - s->camera_x),
                       Word(Pixel(body->state.y) - s->camera_y + 4)},
                knee = {Word(Pixel(leg.knee_x) - s->camera_x),
                        Word(Pixel(leg.knee_y) - s->camera_y)},
                foot = {Word(Pixel(leg.foot_x) - s->camera_x),
                        Word(Pixel(leg.foot_y) - s->camera_y)};
    const Point hip_plus = Offset(hip, upper, 12), hip_minus = Subtract(hip, upper, 12),
                knee_plus = Offset(knee, middle, 12),
                knee_minus = Subtract(knee, middle, Scale(spread_trig.cosine, 24) + 12),
                foot_plus = Offset(foot, lower, 6), foot_minus = Offset(foot, lower, -6);
    const Sint32 scroll = s->olrox_morph.transforming ? 224 : s->camera_y;
    Sint16 angle = Word(facing ? 2048 - (Sint32)leg.lower : leg.lower);
    SotnMathTrig direction = {0}, across = {0};
    if (!Trig(s, angle, &direction) || !Trig(s, angle + (facing ? 1024 : -1024), &across))
        return false;
    Point toe = Offset(Offset(foot, direction, 8), across, 4);
    if (Word(toe.y + scroll) >= 462)
    {
        toe.y = Word(464 - scroll);
        angle = facing ? 2048 : 0;
    }
    Point heel = {0};
    if (!Bend(s, body, toe, &angle, 16, scroll, &heel) ||
        !Trig(s, angle + (facing ? -1024 : 1024), &across))
        return false;
    const Point toe_side = SubtractSine(toe, across, 16);
    if (!Trig(s, angle / 2 + (facing ? 0 : 1024), &across))
        return false;
    const Point heel_side = Offset(heel, across, 16);
    Point back = {0};
    if (!Bend(s, body, heel, &angle, 24, scroll, &back) ||
        !Trig(s, angle + (facing ? -1024 : 1024), &across))
        return false;
    const Point back_side = Offset(back, across, 16);
    const Uint16 palette = side ? 0x20b : 0x218,
                 priority = (Uint16)(s->olrox_monster.priority + side * 2U);
    Packet(p[0], palette, priority, (Point[4]){knee_plus, hip_plus, knee_minus, hip_minus});
    UV(p[0], 0x4c, 0x6c, 0x5c, 0x6c, 0x72, 0x64, 0x80, 0x80);
    Packet(p[1], palette, priority, (Point[4]){foot_plus, knee_plus, foot_minus, knee_minus});
    UV(p[1], 0x74, 0x68, 0x80, 0x76, 0x4e, 0x30, 0x46, 0x20);
    Packet(p[2], palette, (Uint16)(priority + 1U), (Point[4]){back_side, heel_side, back, heel});
    UV(p[2], 0xd8, 0xf0, 0xd8, 0xf0, 0x50, 0x50, 0x5e, 0x5e);
    Packet(p[3], palette, (Uint16)(priority + 1U), (Point[4]){heel_side, toe_side, heel, toe});
    UV(p[3], 0xf0, 0xff, 0xf0, 0xff, 0x50, 0x50, 0x5e, 0x5e);
    return true;
}
static void Whole(Sint32 *position, Sint32 pixel)
{
    *position = (Sint32)((Sint64)Word(pixel) * 65536 + ((Uint32)*position & 65535U));
}
bool OlroxMonster_BodyGeometry(SotnActors *s, Actor *body, Uint16 first)
{
    SotnActorPrimitive *p[2] = {0};
    if (!Parts(s, body, first, 2, p) || body->olrox.body_style > 3)
        return SDL_SetError("Olrox torso geometry requires its owned packets and private style");
    Actor *third = OlroxChild(s, body, 3), *seventh = OlroxChild(s, body, 7);
    if (!third || !seventh || third->state.id != 0x35 || seventh->state.id != 0x37)
        return SDL_SetError("Olrox torso geometry lost an original owned child generation");
    SotnDataView profile = {0};
    if (!Slice(s->nz0, 0x1194 + body->olrox.body_style * 14U, 14, &profile))
        return false;
    const Sint32 shift = Signed16(profile.bytes), first_length = Signed16(profile.bytes + 6),
                 second_length = Signed16(profile.bytes + 10);
    if (SDL_abs(shift) > 512 || SDL_abs(first_length) > 512 || SDL_abs(second_length) > 512)
        return SDL_SetError("Olrox torso profile exceeds original bounded attachment distances");
    const bool facing = body->state.facing_left;
    const Point base = {Word(Pixel(body->state.x) - s->camera_x + (facing ? -shift : shift)),
                        Word(Pixel(body->state.y) - s->camera_y)};
    const Sint16 half = (Sint16)(body->olrox.body_angle / 2),
                 first_axis = Word(facing ? 2048 - half : half),
                 first_perpendicular = Word(facing ? first_axis - 1024 : first_axis + 1024),
                 second_axis =
                     Word(facing ? 2048 - body->olrox.body_angle : body->olrox.body_angle),
                 second_perpendicular = Word(facing ? second_axis - 1024 : second_axis + 1024);
    SotnMathTrig t = {0};
    if (!Trig(s, first_perpendicular, &t))
        return false;
    const Point joint = Offset(base, t, 16);
    if (!Trig(s, first_axis, &t))
        return false;
    const Point joint_plus = Offset(joint, t, 8), joint_minus = Offset(joint, t, -32);
    if (!Trig(s, second_perpendicular, &t))
        return false;
    const Point upper = Offset(joint, t, 32);
    if (!Trig(s, second_axis, &t))
        return false;
    const Point upper_plus = Offset(upper, t, 8), upper_minus = Offset(upper, t, -32),
                base_plus = Add(base, facing ? -8 : 8, 0),
                base_minus = Add(base, facing ? 32 : -32, 0);
    SotnMathTrig first_attachment = {0}, second_attachment = {0};
    if (!Trig(s, body->olrox.body_angle + Signed16(profile.bytes + 8), &first_attachment) ||
        !Trig(s, body->olrox.body_angle + Signed16(profile.bytes + 12), &second_attachment))
        return false;
    const Sint32 sign = facing ? -1 : 1;
    const Point third_position =
                    Add(upper_minus, sign * Scale(first_attachment.cosine, first_length),
                        Scale(-first_attachment.sine, first_length)),
                seventh_position =
                    Add(upper_minus, sign * Scale(second_attachment.cosine, second_length),
                        Scale(-second_attachment.sine, second_length));
    const Uint16 priority = (Uint16)(s->olrox_monster.priority - 1U);
    const Sint32 u = Signed16(profile.bytes + 2), v = Signed16(profile.bytes + 4);
    Packet(p[0], 0x20b, priority, (Point[4]){upper_plus, upper_minus, joint_plus, joint_minus});
    UV(p[0], u + 40, u, u + 40, u, v, v, v + 32, v + 32);
    Packet(p[1], 0x20b, priority, (Point[4]){joint_plus, joint_minus, base_plus, base_minus});
    UV(p[1], u + 40, u, u + 40, u, v + 32, v + 32, v + 48, v + 48);
    Whole(&third->state.x, third_position.x + s->camera_x);
    Whole(&third->state.y, third_position.y + s->camera_y);
    third->state.priority =
        (Uint16)(s->olrox_monster.priority - (body->olrox.body_style == 2 ? 0U : 2U));
    Whole(&seventh->state.x, seventh_position.x + s->camera_x);
    Whole(&seventh->state.y, seventh_position.y + s->camera_y);
    return true;
}
static bool SpriteGeometry(SotnActors *s, const Actor *sprite, Actor *body, Actor *owner,
                           Uint16 index)
{
    ActorPrimitive *packet = NULL;
    if (!sprite || !sprite->state.generation || !OlroxMonster_Bind(s, body) ||
        (owner != body && owner != sprite) || !(packet = ActorPrimitiveGet(s, owner, index)))
        return SDL_SetError("Olrox sprite packet requires its active body or owned child");
    if (sprite != body && (sprite->slot <= body->slot || sprite->slot - body->slot > 13 ||
                           !OlroxOwnsChild(s, body, sprite, (Uint16)(sprite->slot - body->slot))))
        return SDL_SetError("Olrox sprite packet source belongs to another generation");
    SotnActorPrimitive *p[1] = {&packet->state};
    const Actor frame = {.state = {.animation_set = 0x8007, .frame = sprite->state.frame}};
    SotnDataView parts = {0};
    if (!ActorStageParts(s, &frame, &parts))
        return false;
    const Uint8 *part = parts.bytes + 2;
    const Sint32 width = Signed16(part + 6), height = Signed16(part + 8);
    if (width <= 0 || width > 512 || height <= 0 || height > 512)
        return SDL_SetError("Olrox sprite packet requires original bounded part dimensions");
    const bool facing = sprite->state.facing_left;
    const Sint32 anchor = Pixel(sprite->state.x) - s->camera_x, pivot = Signed16(part + 2),
                 start = anchor + (facing ? -pivot : pivot),
                 left = facing ? start - width + 1 : start,
                 right = facing ? start + 1 : start + width,
                 top = Pixel(sprite->state.y) - s->camera_y + Signed16(part + 4);
    const Point corners[4] = {{Word(left), Word(top)},
                              {Word(right), Word(top)},
                              {Word(left), Word(top + height)},
                              {Word(right), Word(top + height)}};
    const Uint16 flags = Read16(part + 12), offset = sprite->state.texture_offset;
    const Sint32 u = (offset & 1U) * 127 + (flags & 1U ? 127 : 0),
                 v = ((offset & 2U) >> 1) * 127 + (flags & 2U ? 127 : 0),
                 u0 = u + Signed16(part + 14), u1 = u + Signed16(part + 18),
                 v0 = v + Signed16(part + 16), v1 = v + Signed16(part + 20);
    Geometry(p[0], corners);
    p[0]->palette = Read16(part + 10);
    UV(p[0], facing ? u1 : u0, facing ? u0 : u1, facing ? u1 : u0, facing ? u0 : u1, v0, v0, v1,
       v1);
    return true;
}

bool OlroxMonster_SpriteGeometry(SotnActors *s, const Actor *sprite, Actor *body, Uint16 index)
{
    return SpriteGeometry(s, sprite, body, body, index);
}
bool OlroxMonster_FreezeSprite(SotnActors *s, Actor *sprite, Actor *body, Uint16 index)
{
    if (!SpriteGeometry(s, sprite, body, sprite, index))
        return false;
    ActorPrimitive *packet = ActorPrimitiveGet(s, sprite, index);
    if (!packet)
        return false;
    SotnActorPrimitive *p = &packet->state;
    const Sint32 px = Pixel(sprite->state.x) - s->camera_x,
                 py = Pixel(sprite->state.y) - s->camera_y;
    const Uint8 flags = sprite->state.transform_flags;
    for (Uint16 i = 0; i < 4; ++i)
    {
        Sint16 x = Word((Sint32)p->offset_x[i] - px), y = Word((Sint32)p->offset_y[i] - py);
        if (flags & SOTN_ACTOR_SCALE_X)
            x = Word(ActorShift((Sint64)sprite->state.scale_x * x, 8));
        if (flags & SOTN_ACTOR_SCALE_Y)
            y = Word(ActorShift((Sint64)sprite->state.scale_y * y, 8));
        if (flags & SOTN_ACTOR_ROTATE)
        {
            const Sint64 square = (Sint64)x * x + (Sint64)y * y;
            Uint32 distance = 0;
            Sint16 angle = 0;
            SotnMathTrig t = {0};
            if (square > 0x7ffff ||
                !SotnMath_Sqrt12(s->executable, (Uint32)square * 4096U, &distance) ||
                !SotnMath_Atan2(s->executable, y, x, &angle) ||
                !Trig(s,
                      angle + (sprite->state.facing_left ? -sprite->state.rotation
                                                         : sprite->state.rotation),
                      &t))
                return SDL_SetError(
                    "Olrox captured sprite exceeds its original SDK rotation bounds");
            x = Word(ActorShift((Sint64)ActorShift(t.cosine, 4) * distance + 524288, 20));
            y = Word(ActorShift((Sint64)ActorShift(t.sine, 4) * distance + 524288, 20));
        }
        p->offset_x[i] = Word(px + x);
        p->offset_y[i] = Word(py + y);
    }
    return true;
}
