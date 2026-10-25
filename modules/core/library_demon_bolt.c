// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/lib/e_lesser_demon.c:func_us_801BCC10/func_us_801BC814.
// Original animation, SDK tables and artwork stay in private runtime inputs.
#include "actors_internal.h"
static ActorPrimitive *Available(SotnActors *s, const Actor *a)
{
    for (Uint16 i = 0; i < a->state.primitive_count; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return NULL;
        if (!p->demon.used)
            return p;
    }
    return NULL;
}
static bool Line(SotnActors *s, Actor *a, ActorPrimitive *p)
{
    switch (p->demon.phase)
    {
    case 0: {
        p->state.red = p->state.green = p->state.blue = 128;
        p->state.line_red = p->state.line_green = p->state.line_blue = 128;
        p->state.priority = (Uint16)(a->state.priority + 2);
        p->state.active = p->state.line = p->state.blend = p->state.priority_set = true;
        p->state.line_blend_mode = 1;
        p->demon.angle = (Sint16)(((Uint32)p->demon.angle + (Uint32)(Sint32)p->demon.turn) & 4095U);
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(s->executable, p->demon.angle, &trig))
            return false;
        const Sint64 x = (Sint64)Pixel(p->state.x) * 65536 + p->demon.fraction_x +
                         (Sint32)trig.cosine * 128,
                     y = (Sint64)Pixel(p->state.y) * 65536 + p->demon.fraction_y -
                         (Sint32)trig.sine * 128;
        if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
            return SDL_SetError("Lesser Demon bolt line exceeds native fixed-point bounds");
        const Sint32 end_x = Pixel((Sint32)x), end_y = Pixel((Sint32)y);
        p->state.offset_x[1] = (Sint16)Wrap16(end_x - Pixel(p->state.x));
        p->state.offset_y[1] = (Sint16)Wrap16(end_y - Pixel(p->state.y));
        p->demon.fraction_x = (Uint16)x;
        p->demon.fraction_y = (Uint16)y;
        if (!p->demon.hops)
        {
            p->demon.hops = 5;
            const Sint32 rx = (Sint32)(ActorRandom(s) & 1U) * 16,
                         target_x = Pixel(a->state.x) + (a->state.facing_left ? rx : -rx),
                         target_y = Pixel(a->state.y) + (Sint32)(ActorRandom(s) & 3U) * 8 - 12;
            Sint16 angle = 0;
            if (!SotnMath_Atan2(s->executable, end_y - target_y, target_x - end_x, &angle))
                return false;
            Sint32 delta = Wrap16((Sint32)angle - p->demon.angle);
            if (delta > 2048)
                delta = 4096 - delta;
            if (delta < -2048)
                delta = 4096 + delta;
            p->demon.turn = (Sint16)(delta / 5);
        }
        p->demon.wait = 1;
        ++p->demon.phase;
        break;
    }
    case 1:
        // Source decrements even after an exhausted pool; retain its byte wrap/retry.
        p->demon.wait = (Uint8)(p->demon.wait - 1);
        if (!p->demon.wait)
        {
            ActorPrimitive *child = Available(s, a);
            if (child)
            {
                child->demon.used = true;
                child->demon.phase = 0;
                child->state = (SotnActorPrimitive){
                    .x = Wrap16((Sint64)Pixel(p->state.x) + p->state.offset_x[1] +
                                (a->state.facing_left ? 1 : -1)) *
                         65536,
                    .y = Wrap16((Sint64)Pixel(p->state.y) + p->state.offset_y[1]) * 65536};
                child->demon.fraction_x = p->demon.fraction_x;
                child->demon.fraction_y = p->demon.fraction_y;
                child->demon.angle = p->demon.angle;
                child->demon.turn = p->demon.turn;
                child->demon.hops = (Uint8)(p->demon.hops - 1);
                p->demon.wait = 8;
                ++p->demon.phase;
            }
        }
        break;
    case 2: {
        Uint8 *end[3] = {&p->state.line_red, &p->state.line_green, &p->state.line_blue},
              *start[3] = {&p->state.red, &p->state.green, &p->state.blue};
        for (Uint16 i = 0; i < 3; ++i)
        {
            *end[i] = (Uint8)(*end[i] - ((ActorRandom(s) & 1U) ? 24 : 12));
            if (*end[i] > 192)
                *end[i] = 0;
        }
        for (Uint16 i = 0; i < 3; ++i)
        {
            *start[i] = (Uint8)(*start[i] - ((ActorRandom(s) & 1U) ? 16 : 8));
            if (*start[i] > 192)
                *start[i] = 0;
        }
        if (!p->demon.wait)
            return SDL_SetError("Lesser Demon bolt line fade timer underflow");
        if (--p->demon.wait == 0)
        {
            p->demon.phase = 0;
            p->demon.used = false;
            p->state.active = false;
        }
        break;
    }
    default:
        return SDL_SetError("Lesser Demon bolt line entered an invalid phase");
    }
    return true;
}
bool ActorLibraryDemonBolt(SotnActors *s, Actor *a)
{
    Uint8 animation = 0;
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x980))
            return false;
        a->state.flags |= 0x80000000U;
        a->state.scale_x = 384;
        a->vx = a->state.facing_left ? 327680 : -327680;
        if (!ActorPrimitives(s, a, 60))
        {
            a->remove = true;
            return true;
        }
        a->state.flags |= 0x800000U;
        if (a->state.priority > 507)
            return SDL_SetError("Lesser Demon bolt priority exceeds ordering bounds");
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
        if (!p)
            return false;
        p->demon.used = true;
        p->state = (SotnActorPrimitive){.x = Pixel(a->state.x) * 65536,
                                        .y = Pixel(a->state.y) * 65536,
                                        .active = true,
                                        .quad = true,
                                        .textured = true,
                                        .blend = true,
                                        .gouraud = true,
                                        .page = 0x3a,
                                        .palette = 0x169,
                                        .u = {224, 255, 224, 255},
                                        .v = {64, 64, 95, 95},
                                        .offset_x = {0, 16, -16, 0},
                                        .offset_y = {14, 0, 0, -14},
                                        .priority_set = true,
                                        .priority = (Uint16)(a->state.priority + 4),
                                        .vertex_red = {96, 0, 0, 96},
                                        .vertex_green = {96, 0, 0, 96},
                                        .vertex_blue = {96, 0, 0, 96}};
        const Uint8 first = a->state.facing_left ? 192 : 16,
                    second = a->state.facing_left ? 16 : 192;
        p->state.vertex_red[1] = p->state.vertex_green[1] = p->state.vertex_blue[1] = first;
        p->state.vertex_red[2] = p->state.vertex_green[2] = p->state.vertex_blue[2] = second;
        // US reconstruction dereferences an uninitialized packet pointer for x3.
        // Initialize it to x0, as every subsequent update and the PSP path do.
        for (Uint16 i = 0; i < 3; ++i)
        {
            p = ActorPrimitiveGet(s, a, (Uint16)(i + 1));
            if (!p)
                return false;
            p->demon.used = true;
            p->demon.hops = 3;
            p->demon.angle = a->state.facing_left ? 0 : 2048;
            p->state.x =
                Wrap16((Sint64)Pixel(a->state.x) + (a->state.facing_left ? -15 : 15)) * 65536;
            p->state.y = Wrap16((Sint64)Pixel(a->state.y) + i * 2 - 2) * 65536;
        }
        return true;
    }
    if (a->state.step != 1)
        return SDL_SetError("Lesser Demon bolt entered an invalid gameplay step");
    if (!ActorAnimate(s, a, 0x1bf0, &animation))
        return false;
    const Sint64 x = (Sint64)a->state.x + a->vx;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32)
        return SDL_SetError("Lesser Demon bolt motion exceeds native bounds");
    a->state.x = (Sint32)x;
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    if (!p)
        return false;
    const Sint32 dx = Pixel(a->state.x) - Pixel(p->state.x);
    if (dx < -32700 || dx > 32700)
        return SDL_SetError("Lesser Demon bolt trail exceeds signed vertex bounds");
    p->state.offset_x[0] = p->state.offset_x[3] = (Sint16)dx;
    if (a->state.facing_left)
    {
        p->state.offset_x[1] = (Sint16)(dx + 16);
        if (p->state.offset_x[1] - p->state.offset_x[2] > 128)
            p->state.offset_x[2] = (Sint16)(p->state.offset_x[1] - 128);
    }
    else
    {
        p->state.offset_x[2] = (Sint16)(dx - 16);
        if (p->state.offset_x[1] - p->state.offset_x[2] > 128)
            p->state.offset_x[1] = (Sint16)(p->state.offset_x[2] + 128);
    }
    for (Uint16 i = 1; i < 60; ++i)
    {
        p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        if (p->demon.used && !Line(s, a, p))
            return false;
    }
    return true;
}
