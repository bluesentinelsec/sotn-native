// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/lib/e_lesser_demon.c windup/punch/star and st/prim_helpers.h.
// Native initialized owned records replace packed pointer/control aliasing.
#include "boss_projection.h"
#include "library_demon_internal.h"
void ActorDemonReleaseEffects(SotnActors *s, Actor *a)
{
    ActorPrimitivesClear(s, a);
    a->state.primitive_count = 0;
    a->state.flags &= ~0x800000U;
}
static bool Pair(SotnActors *s, Actor *a, bool star)
{
    if (a->state.priority > 509)
        return SDL_SetError("Lesser Demon effect priority exceeds the original ordering table");
    if (!ActorPrimitives(s, a, 2))
    {
        a->demon.effect_step = 4;
        return true;
    }
    a->state.flags |= 0x800000U;
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    if (!p)
        return false;
    p->state = (SotnActorPrimitive){
        .quad = true,
        .textured = true,
        .blend = true,
        .modulate = true,
        .x = Wrap16((Sint64)Pixel(a->state.x) +
                    (star ? (a->state.facing_left ? -20 : 20) : (a->state.facing_left ? 4 : -4))) *
             65536,
        .y = Wrap16((Sint64)Pixel(a->state.y) + 1) * 65536,
        .page = (Uint16)(star ? 0x3a : 0x33),
        .palette = (Uint16)(star ? 0x169 : 0x24a),
        .priority_set = true,
        .priority = (Uint16)(a->state.priority + 2)};
    if (star)
    {
        p->state.u[0] = p->state.u[2] = 224;
        p->state.u[1] = p->state.u[3] = 255;
        p->state.v[0] = p->state.v[1] = 64;
        p->state.v[2] = p->state.v[3] = 95;
    }
    else
    {
        p->state.u[0] = p->state.u[2] = a->state.facing_left ? 47 : 0;
        p->state.u[1] = p->state.u[3] = a->state.facing_left ? 0 : 47;
        p->state.v[0] = p->state.v[1] = 112;
        p->state.v[2] = p->state.v[3] = 127;
    }
    a->demon.width = star ? 16 : 40;
    a->demon.height = 16;
    a->demon.brightness = 128;
    a->demon.scale = star ? 256 : 4096;
    a->demon.angle = 0;
    a->demon.effect_step = 1;
    return true;
}
static bool Quad(SotnActors *s, Actor *a)
{
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    if (!p)
        return false;
    const Sint32 x = Pixel(p->state.x) - s->camera_x, y = Pixel(p->state.y) - s->camera_y;
    SotnProjectedPoint points[4] = {0};
    if (!SotnDemon_ProjectQuad(s->executable,
                               (SotnDemonQuadPose){a->demon.angle, a->demon.width, a->demon.height,
                                                   a->demon.scale, x, y},
                               points))
        return false;
    for (Uint16 i = 0; i < 4; ++i)
    {
        p->state.offset_x[i] = (Sint16)(points[i].x - x);
        p->state.offset_y[i] = (Sint16)(points[i].y - y);
    }
    p->state.red = p->state.green = p->state.blue = a->demon.brightness;
    p->state.active = true;
    return true;
}
bool ActorDemonPunchEffect(SotnActors *s, Actor *a)
{
    switch (a->demon.effect_step)
    {
    case 0:
        return Pair(s, a, false);
    case 1: {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
        if (!p)
            return false;
        p->state.palette = p->phase ? 0x252 : 0x24a;
        p->phase = (Uint16)(p->phase ^ 1U);
        if (!Quad(s, a))
            return false;
        a->demon.width += 2;
        --a->demon.height;
        a->demon.brightness = (Uint8)(a->demon.brightness - 4);
        if (a->demon.width > 56)
            ++a->demon.effect_step;
        break;
    }
    case 2:
        ActorDemonReleaseEffects(s, a);
        ++a->demon.effect_step;
        break;
    case 3:
    case 4:
        break;
    default:
        return SDL_SetError("Lesser Demon punch effect entered an invalid phase");
    }
    return true;
}
bool ActorDemonStarEffect(SotnActors *s, Actor *a)
{
    switch (a->demon.effect_step)
    {
    case 0:
        if (!Pair(s, a, true) || !ActorLibrarySound(s, a, 0x614))
            return false;
        break;
    case 1:
        a->demon.angle = (Sint16)Wrap16((Sint32)a->demon.angle + 64);
        if (a->demon.scale > 16384 - 64)
            return SDL_SetError("Lesser Demon star scale exceeds bounded original control");
        a->demon.scale += 64;
        return Quad(s, a);
    case 2:
        ActorDemonReleaseEffects(s, a);
        ++a->demon.effect_step;
        break;
    case 3:
    case 4:
        break;
    default:
        return SDL_SetError("Lesser Demon star entered an invalid phase");
    }
    return true;
}
bool ActorDemonWindup(SotnActors *s, Actor *a, bool *out)
{
    *out = false;
    switch (a->demon.effect_step)
    {
    case 0:
        if (!ActorPrimitives(s, a, 24))
        {
            *out = true;
            a->demon.effect_step = 3;
            a->demon.timer = 0;
            return true;
        }
        a->state.flags |= 0x800000U;
        for (Uint16 i = 0; i < 24; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (!p)
                return false;
            const Uint8 raw =
                (Uint8)((Sint32)(ActorRandom(s) & 127U) + (a->state.facing_left ? -64 : 64));
            SotnMathTrig trig = {0};
            if (!SotnMath_Trig(s->executable, (Sint32)raw * 16, &trig))
                return false;
            const Sint32 x = Wrap16((Sint64)Pixel(a->state.x) +
                                    ActorShift((Sint64)trig.cosine * 96, 12) +
                                    (Sint32)(ActorRandom(s) & 63U) - 31),
                         y = Wrap16((Sint64)Pixel(a->state.y) +
                                    ActorShift((Sint64)trig.sine * 96, 12) +
                                    (Sint32)(ActorRandom(s) & 63U) - 31);
            p->state = (SotnActorPrimitive){.x = x * 65536,
                                            .y = y * 65536,
                                            .active = true,
                                            .width = 2,
                                            .height = 2,
                                            .blend = true,
                                            .page = 0x20,
                                            .priority_set = true,
                                            .priority = (Uint16)(a->state.priority + 2)};
            const Sint64 vx = -((Sint64)x -
                                (Pixel(a->state.x) + (a->state.facing_left ? 10 : -10))) *
                              65536 / 48,
                         vy = -((Sint64)y - (Pixel(a->state.y) - 11)) * 65536 / 48;
            if (vx < SDL_MIN_SINT32 || vx > SDL_MAX_SINT32 || vy < SDL_MIN_SINT32 ||
                vy > SDL_MAX_SINT32)
                return SDL_SetError(
                    "Lesser Demon windup velocity exceeds native fixed-point bounds");
            p->vx = (Sint32)vx;
            p->vy = (Sint32)vy;
        }
        a->demon.timer = 0;
        ++a->demon.effect_step;
        break;
    case 1:
        for (Uint16 i = 0; i < 24; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (!p)
                return false;
            const Sint64 x = (Sint64)p->state.x + p->vx, y = (Sint64)p->state.y + p->vy;
            if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 ||
                y > SDL_MAX_SINT32)
                return SDL_SetError(
                    "Lesser Demon windup particle exceeds native fixed-point bounds");
            p->state.x = (Sint32)x;
            p->state.y = (Sint32)y;
            p->state.red = (Uint8)(p->state.red + 3);
            p->state.active = (a->demon.timer & 1U) == 0;
        }
        if (a->demon.timer++ > 48)
        {
            ActorDemonReleaseEffects(s, a);
            ++a->demon.effect_step;
            a->demon.timer = 0;
        }
        break;
    case 2:
        *out = true;
        break;
    case 3:
        break;
    default:
        return SDL_SetError("Lesser Demon windup entered an invalid phase");
    }
    return true;
}
