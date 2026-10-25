// SPDX-License-Identifier: AGPL-3.0-or-later
// st/lib/first_c_file.c and st/e_misc.h:UnkRecursivePrimFunc2, pinned 7bc54d8157b6.
// Private model/normal/face tables remain runtime data. Midpoints round toward +infinity.
#include "actors_internal.h"
#include "boss_projection.h"
typedef struct Vertex
{
    Sint16 x, y;
    Uint8 u, v, r, g, b;
} Vertex;
static Vertex Midpoint(Vertex a, Vertex b)
{
    return (Vertex){
        (Sint16)ActorShift((Sint64)a.x + b.x + 1, 1), (Sint16)ActorShift((Sint64)a.y + b.y + 1, 1),
        (Uint8)(((Uint16)a.u + b.u + 1U) / 2U),       (Uint8)(((Uint16)a.v + b.v + 1U) / 2U),
        (Uint8)(((Uint16)a.r + b.r + 1U) / 2U),       (Uint8)(((Uint16)a.g + b.g + 1U) / 2U),
        (Uint8)(((Uint16)a.b + b.b + 1U) / 2U)};
}
static Vertex ReadVertex(const SotnActorPrimitive *p, Uint16 i)
{
    return (Vertex){p->offset_x[i],   p->offset_y[i],     p->u[i],          p->v[i],
                    p->vertex_red[i], p->vertex_green[i], p->vertex_blue[i]};
}
static void WriteVertex(SotnActorPrimitive *p, Uint16 i, Vertex v)
{
    p->offset_x[i] = v.x;
    p->offset_y[i] = v.y;
    p->u[i] = v.u;
    p->v[i] = v.v;
    p->vertex_red[i] = v.r;
    p->vertex_green[i] = v.g;
    p->vertex_blue[i] = v.b;
}
static bool Subdivide(SotnActors *s, Actor *a, const SotnActorPrimitive *source, Uint16 levels,
                      Uint16 *next)
{
    Vertex grid[9] = {0};
    grid[0] = ReadVertex(source, 0);
    grid[2] = ReadVertex(source, 1);
    grid[6] = ReadVertex(source, 2);
    grid[8] = ReadVertex(source, 3);
    grid[1] = Midpoint(grid[0], grid[2]);
    grid[7] = Midpoint(grid[6], grid[8]);
    grid[3] = Midpoint(grid[0], grid[6]);
    grid[5] = Midpoint(grid[2], grid[8]);
    grid[4] = Midpoint(grid[3], grid[5]);
    for (Uint16 row = 0; row < 2; ++row)
        for (Uint16 col = 0; col < 2; ++col)
        {
            SotnActorPrimitive q = *source;
            const Uint16 first = (Uint16)(row * 3U + col);
            for (Uint16 i = 0; i < 4; ++i)
                WriteVertex(&q, i, grid[first + (i / 2U) * 3U + i % 2U]);
            if (levels > 1)
            {
                if (!Subdivide(s, a, &q, (Uint16)(levels - 1U), next))
                    return false;
            }
            else
            {
                if (*next >= 37)
                    return SDL_SetError("Library bookcase exceeds its original subdivision budget");
                ActorPrimitive *p = ActorPrimitiveGet(s, a, (*next)++);
                if (!p)
                    return false;
                p->state = q;
                p->state.active = true;
            }
        }
    return true;
}
bool ActorLibraryShelfGeometry(SotnActors *s, Actor *a)
{
    SotnShelfProjection model = {0};
    SotnDataView faces = {0};
    if (!SotnLibrary_ProjectShelf(
            s->executable, s->nz0, a->shelf.angle, a->state.parameters ? a->shelf.depth : 0,
            Pixel(a->state.x) - s->camera_x, Pixel(a->state.y) - s->camera_y, &model) ||
        !Slice(s->nz0, 0xd48, 64, &faces))
        return false;
    Uint16 next = 5;
    for (Uint16 face = 0; face < 4; ++face)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, face);
        if (!p)
            return false;
        p->state.active = false;
        for (Uint16 i = 0; i < 4; ++i)
        {
            const Uint32 index = Read32(faces.bytes + (face * 4U + i) * 4U);
            if (index >= 8)
                return SDL_SetError("Library bookcase face references an invalid private vertex");
            p->state.offset_x[i] = (Sint16)model.point[index].x;
            p->state.offset_y[i] = (Sint16)model.point[index].y;
            p->state.vertex_red[i] = model.color[index][0];
            p->state.vertex_green[i] = model.color[index][1];
            p->state.vertex_blue[i] = model.color[index][2];
        }
        const Sint32 x0 = p->state.offset_x[0], x1 = p->state.offset_x[1],
                     x2 = p->state.offset_x[2], y0 = p->state.offset_y[0],
                     y1 = p->state.offset_y[1], y2 = p->state.offset_y[2];
        const Sint32 winding = (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0);
        if (winding > 0 && !Subdivide(s, a, &p->state, a->state.parameters ? 1 : 2, &next))
            return false;
    }
    while (next < 37)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, next++);
        if (!p)
            return false;
        p->state.active = false;
    }
    if (!a->state.parameters)
    {
        SotnMathTrig trig = {0};
        ActorPrimitive *shadow = ActorPrimitiveGet(s, a, 4);
        if (!shadow || !SotnMath_Trig(s->executable, a->shelf.angle, &trig))
            return false;
        shadow->state.red = shadow->state.green = shadow->state.blue =
            (Uint8)(SDL_abs(trig.cosine) * 40 / 4096);
    }
    return true;
}
