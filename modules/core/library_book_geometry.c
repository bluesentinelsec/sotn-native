// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/lib/e_spellbook_magic_tome.c at the pinned reconstruction.
// Original geometry, UVs and SDK tables stay in the user's private data.
#include "actors_internal.h"
#include "boss_projection.h"
static bool Vertex(SotnActors *s, Uint32 address, SotnBookVertex *out)
{
    SotnDataView v = {0};
    if (address < 0x80182f44 || address > 0x80182fc4 || (address - 0x80182f44) % 8 ||
        !Slice(s->nz0, address - 0x80180000, 8, &v))
        return SDL_SetError("Library book vertex references an invalid private model");
    *out = (SotnBookVertex){(Sint16)Signed16(v.bytes), (Sint16)Signed16(v.bytes + 2),
                            (Sint16)Signed16(v.bytes + 4)};
    return true;
}
bool ActorLibraryBookGeometry(SotnActors *s, Actor *a)
{
    SotnBookTransform transform = {a->book.angle[0],
                                   a->book.angle[1],
                                   a->book.angle[2],
                                   a->book.depth,
                                   Pixel(a->state.x) - s->camera_x,
                                   Pixel(a->state.y) - s->camera_y};
    for (Uint16 face = 0; face < 12; ++face)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, face);
        if (!p)
            return false;
        if (face % 6 == 5 && !a->book.opening)
        {
            p->state.active = false;
            continue;
        }
        SotnDataView pointers = {0};
        SotnBookVertex vertices[4] = {0};
        if (!Slice(s->nz0, 0x2fcc + face * 16U, 16, &pointers))
            return false;
        for (Uint16 i = 0; i < 4; ++i)
            if (!Vertex(s, Read32(pointers.bytes + i * 4U), &vertices[i]))
                return false;
        transform.z = (Sint16)Wrap16((Sint32)a->book.angle[2] - a->book.opening / 2 +
                                     (face < 6 ? 0 : a->book.opening));
        SotnBookProjection projected = {0};
        if (!SotnBook_Project(s->executable, transform, vertices, 4, &projected))
            return false;
        p->state.active = projected.winding >= 0;
        p->state.palette =
            (Uint16)((a->state.palette & 0x8000U) ? a->state.palette & 0xfffU
                                                  : a->state.palette + (face % 6 > 1 ? 1 : 0));
        p->state.priority = projected.priority;
        for (Uint16 i = 0; i < 4; ++i)
        {
            p->state.offset_x[i] = (Sint16)projected.point[i].x;
            p->state.offset_y[i] = (Sint16)projected.point[i].y;
        }
    }
    SotnBookVertex center = {0};
    SotnBookProjection projected = {0};
    transform.z = a->book.angle[2];
    if (!Vertex(s, 0x80182fc4, &center) ||
        !SotnBook_Project(s->executable, transform, &center, 1, &projected))
        return false;
    a->book.center_x = (Sint16)(projected.point[0].x - transform.anchor_x);
    a->book.center_y = (Sint16)(projected.point[0].y - transform.anchor_y);
    a->book.priority = projected.priority;
    a->state.hitbox_x = (Sint16)(a->book.center_x / 2);
    a->state.hitbox_y = (Sint16)(a->book.center_y / 2);
    return true;
}
bool ActorLibraryBookFaces(SotnActors *s, Actor *a)
{
    const size_t table = a->state.id == 0x30 ? 0x30dc : 0x310c;
    for (Uint16 face = 0; face < 12; ++face)
    {
        SotnDataView pointer = {0}, uv = {0};
        if (!Slice(s->nz0, table + face * 4U, 4, &pointer))
            return false;
        const Uint32 address = Read32(pointer.bytes);
        if (address < 0x80180000 || !Slice(s->nz0, address - 0x80180000, 4, &uv))
            return SDL_SetError("Library book face has an invalid private UV pointer");
        ActorPrimitive *p = ActorPrimitiveGet(s, a, face);
        if (!p)
            return false;
        p->state = (SotnActorPrimitive){.quad = true,
                                        .textured = true,
                                        .screen_space = true,
                                        .priority_set = true,
                                        .page = 0x15,
                                        .u = {uv.bytes[0], uv.bytes[2], uv.bytes[0], uv.bytes[2]},
                                        .v = {uv.bytes[1], uv.bytes[1], uv.bytes[3], uv.bytes[3]}};
    }
    return true;
}
