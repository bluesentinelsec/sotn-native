// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected e_bloody_zombie.h blood children, prim_helpers.h packet control, and
// dra/5087C.c camera-word behavior. Native owned records replace packet aliasing.
#include "actors_internal.h"
static bool Add(Sint32 *value, Sint32 delta)
{
    const Sint64 next = (Sint64)*value + delta;
    if (next < SDL_MIN_SINT32 || next > SDL_MAX_SINT32)
        return SDL_SetError("Library blood motion exceeds fixed-point bounds");
    *value = (Sint32)next;
    return true;
}
static bool TextureAnimation(SotnActors *s, ActorPrimitive *p, size_t offset)
{
    SotnDataView duration = {0}, row = {0};
    if (p->blood.pose >= 51 || !Slice(s->nz0, offset + (size_t)p->blood.pose * 5, 1, &duration))
        return SDL_SetError("Blood texture animation cursor exceeds its private stream");
    if (!p->blood.remaining)
    {
        if (duration.bytes[0] == 255)
        {
            p->blood.ended = true;
            p->state.active = false;
            return true;
        }
        const bool loop = duration.bytes[0] == 0;
        if (loop)
        {
            p->blood.pose = 0;
        }
        if (!Slice(s->nz0, offset + (size_t)p->blood.pose * 5, 5, &row) || !row.bytes[0] ||
            row.bytes[0] == 255)
            return SDL_SetError("Blood texture animation cannot restart at a control row");
        p->blood.remaining = row.bytes[0];
        const Uint16 origin = Read16(row.bytes + 1), delta = Read16(row.bytes + 3);
        const Uint16 uv[4] = {origin, (Uint16)(origin + (delta & 255U)),
                              (Uint16)(origin + (delta & 0xff00U)), (Uint16)(origin + delta)};
        for (Uint16 i = 0; i < 4; ++i)
        {
            p->state.u[i] = (Uint8)uv[i];
            p->state.v[i] = (Uint8)(uv[i] >> 8);
        }
        ++p->blood.pose;
        if (loop)
        {
            p->blood.ended = true;
            p->state.active = false;
            return true;
        }
    }
    --p->blood.remaining;
    return true;
}
static void CameraFractions(SotnActors *s, Actor *a, ActorPrimitive *p)
{
    if (p->blood.ended)
        return;
    // DRAW_UNK02 also moves the two packed control fraction words, without carry.
    const Uint16 x = (Uint16)((Uint32)p->state.x - (Uint32)(s->camera_y - a->flying.camera_y));
    const Uint16 y = (Uint16)((Uint32)p->state.y - (Uint32)(s->camera_x - a->flying.camera_x));
    p->state.x = Pixel(p->state.x) * 65536 + x;
    p->state.y = Pixel(p->state.y) * 65536 + y;
}
static bool Quad(SotnActors *s, ActorPrimitive *p)
{
    if (p->blood.ended)
        return true;
    if (!Add(&p->state.x, p->vx) || !Add(&p->state.y, p->vy))
        return false;
    SotnMathTrig trig = {0};
    if (!SotnMath_MatrixTrig(s->executable, p->blood.angle, &trig))
        return false;
    // These packets use identity X/Y/scale, Z rotation and equal depth/screen 1024.
    // Their bounded on-screen path has unit perspective and no SXY saturation.
    const Sint32 left = -p->blood.width / 2, right = p->blood.width / 2;
    const Sint32 top = -p->blood.height / 2, bottom = p->blood.height / 2;
    const Sint32 x[4] = {left, right, left, right}, y[4] = {top, top, bottom, bottom};
    if (p->blood.width < 0 || p->blood.width > 512 || p->blood.height < 0 || p->blood.height > 512)
        return SDL_SetError("Blood quadrilateral exceeds its original bounded geometry");
    for (Uint16 i = 0; i < 4; ++i)
    {
        p->state.offset_x[i] =
            (Sint16)ActorShift((Sint64)trig.cosine * x[i] - (Sint64)trig.sine * y[i], 12);
        p->state.offset_y[i] =
            (Sint16)ActorShift((Sint64)trig.sine * x[i] + (Sint64)trig.cosine * y[i], 12);
    }
    p->state.red = p->state.green = p->state.blue = p->blood.brightness;
    p->state.active = true;
    return true;
}
static bool Splatter(SotnActors *s, Actor *a)
{
    switch (a->state.step)
    {
    case 0:
        return ActorInitialize(s, a, 0x818);
    case 1:
        if (!ActorPrimitives(s, a, 8))
        {
            a->remove = true;
            return true;
        }
        ++a->state.step;
        return true;
    case 2:
        if (a->state.priority > 509)
            return SDL_SetError("Blood primitive priority exceeds the original table");
        for (Uint16 i = 0; i < 2; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i * 2U);
            if (p == NULL)
                return false;
            p->state = (SotnActorPrimitive){
                .x = (Pixel(a->state.x) + (i == 0 ? (a->state.facing_left ? 4 : -4) : 0)) * 65536,
                .y = Pixel(a->state.y) * 65536,
                .quad = true,
                .textured = true,
                .modulate = true,
                .page = 0x1a,
                .palette = 0x1b7,
                .priority_set = true,
                .priority = (Uint16)(a->state.priority + 2)};
            p->blood.width = i == 0 ? 16 : 8;
            p->blood.height = 16;
            p->blood.brightness = (Uint8)(i == 0 ? 96 : 128);
            p->vx = a->state.facing_left ? (i == 0 ? 40960 : 49152) : (i == 0 ? -40960 : -49152);
            p->vy = i == 0 ? -24576 : -32768;
            p->blood.angle = a->state.facing_left ? 512 : -512;
            p->state.u[0] = p->state.u[1] = (Uint8)(i == 0 ? 80 : 32);
            p->state.u[2] = p->state.u[3] = (Uint8)(i == 0 ? 64 : 0);
            p->state.v[0] = p->state.v[2] = (Uint8)(i == 0 ? 48 : 32);
            p->state.v[1] = p->state.v[3] = 64;
        }
        a->flying.camera_x = s->camera_x;
        a->flying.camera_y = s->camera_y;
        ++a->state.step;
        return true;
    case 3: {
        ActorPrimitive *small = ActorPrimitiveGet(s, a, 0), *big = ActorPrimitiveGet(s, a, 2);
        if (small == NULL || big == NULL)
            return false;
        CameraFractions(s, a, small);
        CameraFractions(s, a, big);
        a->flying.camera_x = s->camera_x;
        a->flying.camera_y = s->camera_y;
        if (!Quad(s, small))
            return false;
        if (s->game_frame & 1U)
        {
            ++small->blood.width;
            ++small->blood.height;
            small->state.palette = 0x1b7;
        }
        else
            small->state.palette = 0x196;
        small->vy += 3072;
        small->blood.angle = (Sint16)Wrap16(small->blood.angle + (a->state.facing_left ? 24 : -24));
        small->blood.brightness = (Uint8)(small->blood.brightness - 2);
        if (!TextureAnimation(s, small, 0x27cc) || !Quad(s, big))
            return false;
        big->blood.width = (Sint16)(big->blood.width + 2);
        big->blood.height = (Sint16)(big->blood.height + 2);
        if (big->blood.pose > 3)
            big->blood.height = 16;
        big->blood.brightness = (Uint8)(big->blood.brightness - 3);
        if (big->blood.brightness > 240)
            big->blood.brightness = 0;
        if (!TextureAnimation(s, big, 0x27e4))
            return false;
        if (a->timer++ > 128)
            a->remove = true;
        return true;
    }
    default:
        return SDL_SetError("Blood splatter entered an invalid step");
    }
}
static bool Drips(SotnActors *s, Actor *a)
{
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x818))
            return false;
        if (!ActorPrimitives(s, a, 1))
        {
            a->remove = true;
            return true;
        }
        if (a->state.priority > 510)
            return SDL_SetError("Blood line priority exceeds the original table");
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
        if (p == NULL)
            return false;
        a->state.hitbox_state = 0;
        p->state = (SotnActorPrimitive){.x = Pixel(a->state.x) * 65536,
                                        .y = Pixel(a->state.y) * 65536,
                                        .active = true,
                                        .line = true,
                                        .blend = true,
                                        .line_blend_mode = 1,
                                        .red = 255,
                                        .blue = 48,
                                        .line_red = 32,
                                        .line_blue = 16,
                                        .priority_set = true,
                                        .priority = (Uint16)(a->state.priority + 1)};
        p->blood.tail_y = Pixel(a->state.y);
        return true;
    }
    if (a->state.step != 1)
        return SDL_SetError("Blood drip entered an invalid step");
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    bool blocked = false;
    if (p == NULL || !ActorLibraryBlocked(s, a, 0x2804, false, &blocked))
        return false;
    if (blocked)
    {
        p->blood.tail_y += 2;
        if (!a->substep)
            a->substep = 1;
    }
    else
    {
        a->vy += 6144;
        if (!Add(&a->state.y, a->vy))
            return false;
        if (Pixel(p->state.y) - p->blood.tail_y > 8)
            p->blood.tail_y = Pixel(p->state.y) - 8;
    }
    p->state.x = Pixel(a->state.x) * 65536;
    p->state.y = Pixel(a->state.y) * 65536;
    const Sint32 tail = p->blood.tail_y - Pixel(a->state.y);
    if (tail < SDL_MIN_SINT16 || tail > SDL_MAX_SINT16)
        return SDL_SetError("Blood line tail exceeds its original signed coordinates");
    p->state.offset_y[1] = (Sint16)tail;
    if (Pixel(a->state.y) < p->blood.tail_y)
        a->remove = true;
    return true;
}
bool ActorLibraryBlood(SotnActors *s, Actor *a)
{
    if (s == NULL || a == NULL || s->stage_id != 2 || (a->state.id != 0x1d && a->state.id != 0x1e))
        return SDL_SetError("Blood dispatch requires original Library actor identities");
    return a->state.id == 0x1d ? Drips(s, a) : Splatter(s, a);
}
