// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected US st/e_stained_glass.h at sotn-decomp 7bc54d8157b6.
// Original plane vertices, anchor and translation/rotation records stay private.
#include "chapel_glass_internal.h"

static void Gray(SotnActorPrimitive *q, Uint8 a, Uint8 b, Uint8 c, Uint8 d)
{
    const Uint8 value[4] = {a, b, c, d};
    q->gouraud = q->modulate = true;
    for (Uint16 i = 0; i < 4; ++i)
        q->vertex_red[i] = q->vertex_green[i] = q->vertex_blue[i] = value[i];
}
static bool Initialize(SotnActors *s, Actor *a)
{
    if (!ChapelGlass_Reserve(s, a, 19))
        return false;
    if (a->remove)
        return true;
    for (Uint16 i = 0; i < 19; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        p->state = (SotnActorPrimitive){.textured = true,
                                        .screen_space = true,
                                        .quad = true,
                                        .priority_set = true,
                                        .priority = 84,
                                        .page = 15,
                                        .palette = 15};
        SotnActorPrimitive *q = &p->state;
        q->u[0] = q->u[2] = i < 4 ? 129 : 35;
        q->u[1] = q->u[3] = i < 4 ? 174 : 92;
        q->v[0] = q->v[1] = 1;
        q->v[2] = q->v[3] = i < 4 ? 126 : 62;
        if (i < 4)
        {
            q->palette = i & 1U ? 0 : 5;
            q->priority = i & 1U ? 81 : 80;
            q->blend = (i & 1U) != 0;
            Gray(q, 24, 24, 24, 24);
            if (i == 0)
                Gray(q, 32, 48, 64, 48);
            else if (i == 2)
                Gray(q, 80, 48, 160, 48);
        }
        if (i == 4)
        {
            q->textured = false;
            q->priority = 82;
            Gray(q, 128, 128, 128, 128);
            q->vertex_red[0] = q->vertex_red[1] = 64;
            q->vertex_green[0] = q->vertex_green[1] = 24;
            q->vertex_blue[0] = q->vertex_blue[1] = 72;
            // The US assignment copies the initialized r3 triplet back over r2.
        }
    }
    s->palette_blend = (SotnPaletteBlend){.source = 15,
                                          .destination = 0x205,
                                          .steps = 16,
                                          .red = 24,
                                          .green = 24,
                                          .blue = 24,
                                          .active = true};
    return true;
}
static bool Rectangle(SotnActorPrimitive *q, Sint32 left, Sint32 top, Sint32 right, Sint32 bottom)
{
    if (left < SDL_MIN_SINT16 || left > SDL_MAX_SINT16 || right < SDL_MIN_SINT16 ||
        right > SDL_MAX_SINT16 || top < SDL_MIN_SINT16 || top > SDL_MAX_SINT16 ||
        bottom < SDL_MIN_SINT16 || bottom > SDL_MAX_SINT16)
        return SDL_SetError("Chapel background rectangle exceeds signed GPU coordinates");
    q->active = true;
    q->offset_x[0] = q->offset_x[2] = (Sint16)left;
    q->offset_x[1] = q->offset_x[3] = (Sint16)right;
    q->offset_y[0] = q->offset_y[1] = (Sint16)top;
    q->offset_y[2] = q->offset_y[3] = (Sint16)bottom;
    return true;
}
static bool Windows(SotnActors *s, Actor *a, SotnProjectedPoint center)
{
    for (Uint16 i = 0; i < 4; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p || !Rectangle(&p->state, center.x + (i >= 2 ? 45 : -45), center.y - 62, center.x,
                             center.y + 62))
            return false;
    }
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 4);
    if (!p)
        return false;
    p->state.active = true;
    p->state.offset_x[0] = (Sint16)(center.x - 8);
    p->state.offset_x[1] = (Sint16)(center.x + 8);
    p->state.offset_x[2] = (Sint16)(Pixel(a->state.x) - s->camera_x - 24);
    p->state.offset_x[3] = (Sint16)(Pixel(a->state.x) - s->camera_x + 24);
    p->state.offset_y[0] = p->state.offset_y[1] = (Sint16)(center.y + 56);
    p->state.offset_y[2] = p->state.offset_y[3] = (Sint16)(Pixel(a->state.y) - s->camera_y + 194);
    return true;
}
static SotnProjectedPoint Midpoint(SotnProjectedPoint a, SotnProjectedPoint b)
{
    // Preserve the US packed-XY average, including low-word carry and negative-X
    // sign extension. Use defined arithmetic instead of aliasing RECT/s32 storage.
    const Sint32 packed_a = a.y * 65536 + (Uint16)a.x, packed_b = b.y * 65536 + (Uint16)b.x;
    const Uint32 packed = ((Uint32)((packed_a + packed_b) / 2) & 0xffff0000U) |
                          (Uint32)ActorShift((Sint64)a.x + b.x, 1);
    return (SotnProjectedPoint){Wrap16(packed & 65535U), Wrap16(packed >> 16)};
}
bool ChapelGlass_Background(SotnActors *s, Actor *a)
{
    s->backdrop_color = 3U | (3U << 5) | (3U << 10);
    if (!a->state.step)
        return Initialize(s, a);
    if (a->state.step != 1 || a->state.primitive_count != 19)
        return SDL_SetError("Chapel glass background entered an invalid original state");
    SotnDataView rotation = {0};
    if (!Slice(s->nz0, 0x277c, 6, &rotation) || Read16(rotation.bytes) ||
        Read16(rotation.bytes + 2) || Read16(rotation.bytes + 4))
        return SDL_SetError("Chapel glass background requires its private identity rotation");
    for (Uint16 i = 0; i < 19; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        p->state.active = false;
    }
    Uint16 used = 5;
    for (Uint16 i = 0; i < 8; ++i)
    {
        SotnDataView translation = {0};
        SotnGlassVertex model[4] = {0};
        if (!Slice(s->nz0, 0x26ecU + i * 16U, 12, &translation) ||
            !ChapelGlass_Model(s, i == 0 ? 0x26e4 : 0x26c4, model, i == 0 ? 1 : 4))
            return false;
        const Sint32 tx = Signed32(translation.bytes), ty = Signed32(translation.bytes + 4),
                     depth = Signed32(translation.bytes + 8);
        if (tx < -512 || tx > 512 || ty < -512 || ty > 512 || depth < 0 || depth > 4096)
            return SDL_SetError("Chapel background has an invalid private translation");
        const SotnGlassTransform pose = {.x = tx + Pixel(a->state.x) - s->camera_x - 128,
                                         .y = ty + Pixel(a->state.y) - s->camera_y - 160,
                                         .depth = depth + 1024};
        SotnProjectedPoint point[4] = {0};
        if (!SotnChapel_ProjectGlass(s->executable, pose, model, i == 0 ? 1 : 4, point))
            return false;
        if (i == 0)
        {
            if (!Windows(s, a, point[0]))
                return false;
        }
        else if (point[2].y >= 0)
        {
            const SotnProjectedPoint top = Midpoint(point[0], point[1]),
                                     bottom = Midpoint(point[2], point[3]);
            for (Uint16 side = 0; side < 2; ++side)
            {
                ActorPrimitive *p = ActorPrimitiveGet(s, a, used++);
                if (!p)
                    return false;
                const SotnProjectedPoint corners[4] = {point[side], top, point[side + 2U], bottom};
                p->state.active = true;
                p->state.palette = (Uint16)(0x205U + i * 2U);
                for (Uint16 vertex = 0; vertex < 4; ++vertex)
                {
                    p->state.offset_x[vertex] = (Sint16)corners[vertex].x;
                    p->state.offset_y[vertex] = (Sint16)corners[vertex].y;
                }
            }
        }
    }
    return true;
}
