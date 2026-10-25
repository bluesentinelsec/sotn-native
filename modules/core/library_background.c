// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/lib/e_library_bg.c at sotn-decomp 7bc54d8157b6.
// Model vertices, lighting grids and anchor tables remain private runtime inputs.
#include "actors_internal.h"
#include "boss_projection.h"
static bool Rectangle(SotnActorPrimitive *p, Sint32 left, Sint32 top, Sint32 right, Sint32 bottom)
{
    if (left < SDL_MIN_SINT16 || right > SDL_MAX_SINT16 || top < SDL_MIN_SINT16 ||
        bottom > SDL_MAX_SINT16 || right < left || bottom < top)
        return SDL_SetError("Library background rectangle exceeds signed GPU coordinates");
    p->offset_x[0] = p->offset_x[2] = (Sint16)left;
    p->offset_x[1] = p->offset_x[3] = (Sint16)right;
    p->offset_y[0] = p->offset_y[1] = (Sint16)top;
    p->offset_y[2] = p->offset_y[3] = (Sint16)bottom;
    return true;
}
static bool Initialize(SotnActors *s, Actor *a, Uint16 kind)
{
    const Uint16 count = kind == 3 ? 16 : kind == 2 ? 10 : 4;
    if (!ActorInitialize(s, a, 0x818) || !ActorPrimitives(s, a, count))
        return false;
    if (kind == 3)
    {
        SotnDataView point = {0};
        if (a->state.parameters >= 4 ||
            !Slice(s->nz0, 0xe78U + a->state.parameters * 4U, 4, &point))
            return SDL_SetError("Distant Library shelves have an invalid private anchor");
        a->state.x = Signed16(point.bytes) * 65536 + (Sint32)((Uint32)a->state.x & 65535U);
        a->state.y = Signed16(point.bytes + 2) * 65536 + (Sint32)((Uint32)a->state.y & 65535U);
        s->backdrop_color = (64U / 8U) | ((56U / 8U) << 5) | ((40U / 8U) << 10);
    }
    else if (!kind)
        s->backdrop_color = (16U / 8U) | ((16U / 8U) << 5) | ((16U / 8U) << 10);
    for (Uint16 i = 0; i < a->state.primitive_count; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        p->state = (SotnActorPrimitive){
            .active = true, .screen_space = true, .quad = true, .priority_set = true};
        SotnActorPrimitive *q = &p->state;
        q->priority = kind == 3 ? 0x20 : kind == 2 ? 0x60 : kind == 1 ? 0x50 : 0x40;
        if (kind == 2)
        {
            q->blend = true;
            continue;
        }
        q->textured = true;
        q->page = 0xf;
        q->palette = kind == 3 ? 0x2f : kind == 1 ? 0x34 : 0x35;
        const Uint8 left = kind == 3 ? ((i & 1U) ? 0x88 : 0xfe) : kind == 1 ? 128 : 130;
        const Uint8 right = kind == 3 ? ((i & 1U) ? 0xfe : 0x88) : kind == 1 ? 248 : 253;
        q->u[0] = q->u[2] = left;
        q->u[1] = q->u[3] = right;
        q->v[0] = q->v[1] = kind == 3 ? 0x81 : kind == 1 ? 0 : 50;
        q->v[2] = q->v[3] = kind == 3 ? 0xff : kind == 1 ? 28 : 125;
        q->gouraud = q->modulate = kind == 3;
    }
    return true;
}
static bool Distant(SotnActors *s, Actor *a, SotnProjectedPoint position)
{
    SotnDataView pointer = {0}, colors = {0};
    if (a->state.parameters >= 4 || !Slice(s->nz0, 0xe68U + a->state.parameters * 4U, 4, &pointer))
        return SDL_SetError("Distant Library lighting grid index is invalid");
    const Uint32 address = Read32(pointer.bytes);
    if (address < 0x80180df8 || address > 0x80180e4c || (address - 0x80180df8) % 28U ||
        !Slice(s->nz0, address - 0x80180000U, 25, &colors))
        return SDL_SetError("Distant Library lighting grid references invalid private data");
    for (Uint16 i = 0; i < 16; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        const Sint32 row = i / 4, col = i % 4;
        const Sint32 x = position.x + col * 120 - 240, y = position.y + row * 128 - 256;
        if (!Rectangle(&p->state, x, y, x + 120, y + 128))
            return false;
        p->state.active = x <= 256 && x + 120 >= 0 && y <= 256 && y + 128 >= 0;
        const Uint8 values[4] = {colors.bytes[row * 5 + col], colors.bytes[row * 5 + col + 1],
                                 colors.bytes[(row + 1) * 5 + col],
                                 colors.bytes[(row + 1) * 5 + col + 1]};
        for (Uint16 vertex = 0; vertex < 4; ++vertex)
            p->state.vertex_red[vertex] = p->state.vertex_green[vertex] =
                p->state.vertex_blue[vertex] = values[vertex];
    }
    return true;
}
bool ActorLibraryBackground(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 2 || a->state.id < 0x16 || a->state.id > 0x19)
        return SDL_SetError("Library background requires its own parallax actor");
    const Uint16 kind = (Uint16)(a->state.id - 0x16U);
    if (!a->state.step && !Initialize(s, a, kind))
        return false;
    if (a->state.step != 1)
        return SDL_SetError("Library background entered an invalid original state");
    const Sint32 screen_y = Pixel(a->state.y) - s->camera_y;
    SotnProjectedPoint projected = {0};
    if (!SotnLibrary_ProjectBackground(s->nz0, kind, Pixel(a->state.x) - s->camera_x, screen_y,
                                       &projected))
        return false;
    if (kind == 3)
        return Distant(s, a, projected);
    const Sint32 spacing = kind == 2 ? 56 : kind == 1 ? 120 : 124;
    Sint32 x = projected.x;
    if (x > 0)
        while (x > -64)
            x -= spacing;
    else
        while (x < -128)
            x += spacing;
    bool visible = true;
    for (Uint16 i = 0; i < a->state.primitive_count; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        const Sint32 width = kind == 2 ? 24 : spacing;
        const Sint32 height = kind == 2 ? 64 : kind == 1 ? 28 : 76;
        if (!Rectangle(&p->state, x, screen_y - height, x + width, screen_y))
            return false;
        p->state.active = visible;
        x += spacing;
        if (x > 256)
            visible = false;
    }
    if (kind == 2)
    {
        ActorPrimitive *last = ActorPrimitiveGet(s, a, 9);
        if (!last)
            return false;
        if (!Rectangle(&last->state, 0, screen_y - 112, 255, screen_y - 64))
            return false;
        last->state.active = true;
    }
    return true;
}
