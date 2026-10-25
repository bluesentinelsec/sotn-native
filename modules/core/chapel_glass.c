// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/e_stained_glass{,_recurse}.h, sotn-decomp 7bc54d8157b6.
// Models, subdivision indices, panel poses, rotations and colors remain private.
#include "chapel_glass_internal.h"

bool ChapelGlass_Reserve(SotnActors *s, Actor *a, Uint16 count)
{
    if (!ActorInitialize(s, a, 0x968))
        return false;
    if (!ActorPrimitives(s, a, count))
    {
        a->remove = true;
        return true;
    }
    a->state.flags |= 0x800000U;
    return true;
}
bool ChapelGlass_Model(SotnActors *s, size_t offset, SotnGlassVertex *out, Uint16 count)
{
    SotnDataView model = {0};
    if (!out || (count != 1 && count != 4) || !Slice(s->nz0, offset, count * 8U, &model))
        return SDL_SetError("Chapel glass model is truncated or has an invalid vertex count");
    for (Uint16 i = 0; i < count; ++i)
    {
        const Uint8 *p = model.bytes + i * 8U;
        out[i] = (SotnGlassVertex){(Sint16)Signed16(p), (Sint16)Signed16(p + 2),
                                   (Sint16)Signed16(p + 4)};
        if (out[i].x < -512 || out[i].x > 512 || out[i].y < -512 || out[i].y > 512 || out[i].z)
            return SDL_SetError("Chapel glass model exceeds its selected plane bounds");
    }
    return true;
}
bool ChapelGlass_Project(SotnActors *s, SotnGlassTransform pose, const SotnGlassVertex model[4],
                         SotnActorPrimitive *out)
{
    SotnProjectedPoint point[4] = {0};
    if (!SotnChapel_ProjectGlass(s->executable, pose, model, 4, point))
        return false;
    for (Uint16 i = 0; i < 4; ++i)
    {
        out->offset_x[i] = (Sint16)point[i].x;
        out->offset_y[i] = (Sint16)point[i].y;
    }
    return true;
}
static void Gray(SotnActorPrimitive *q, Uint8 near, Uint8 far)
{
    q->gouraud = q->modulate = true;
    for (Uint16 i = 0; i < 4; ++i)
        q->vertex_red[i] = q->vertex_green[i] = q->vertex_blue[i] = i >= 2 ? far : near;
}
static bool Store(SotnActors *s, Actor *a, Uint16 *used, SotnActorPrimitive q)
{
    if (*used >= a->state.primitive_count)
        return SDL_SetError("Chapel glass subdivision/light exceeds its owned primitive capacity");
    ActorPrimitive *p = ActorPrimitiveGet(s, a, *used);
    if (!p)
        return false;
    q.active = true;
    p->state = q;
    ++*used;
    return true;
}
static bool Split(SotnActors *s, Actor *a, SotnGlassTransform pose, const SotnGlassVertex model[4],
                  SotnActorPrimitive q, Uint16 depth, Uint16 *used)
{
    SotnDataView indices = {0};
    if (depth == 0 || depth > 3 || !Slice(s->nz0, 0x2564, 16, &indices))
        return SDL_SetError("Chapel glass subdivision has invalid depth or private indices");
    SotnGlassVertex points[9] = {0};
    Uint8 u[9] = {0}, v[9] = {0};
    for (Uint16 i = 0; i < 4; ++i)
    {
        const Uint16 index = (Uint16)((i / 2U) * 6U + (i % 2U) * 2U);
        points[index] = model[i];
        u[index] = q.u[i];
        v[index] = q.v[i];
    }
    for (Uint16 side = 0; side < 2; ++side)
    {
        const Uint16 top = (Uint16)(side * 2U), bottom = (Uint16)(top + 6),
                     middle = (Uint16)(top + 3);
        points[middle] =
            (SotnGlassVertex){(Sint16)ActorShift((Sint64)points[top].x + points[bottom].x + 1, 1),
                              (Sint16)ActorShift((Sint64)points[top].y + points[bottom].y + 1, 1),
                              (Sint16)ActorShift((Sint64)points[top].z + points[bottom].z + 1, 1)};
        u[middle] = (Uint8)(((Uint16)u[top] + u[bottom] + 1U) / 2U);
        v[middle] = (Uint8)(((Uint16)v[top] + v[bottom] + 1U) / 2U);
    }
    for (Uint16 half = 0; half < 2; ++half)
    {
        SotnGlassVertex sub[4] = {0};
        SotnActorPrimitive child = q;
        for (Uint16 i = 0; i < 4; ++i)
        {
            const Uint16 index = Read16(indices.bytes + (half * 4U + i) * 2U);
            if (index > 8 || index == 1 || index == 4 || index == 7)
                return SDL_SetError("Chapel glass subdivision references an uninitialized point");
            sub[i] = points[index];
            child.u[i] = u[index];
            child.v[i] = v[index];
        }
        if (!ChapelGlass_Project(s, pose, sub, &child))
            return false;
        if (child.offset_x[1] >= 0 && child.offset_x[0] <= 256 &&
            !(depth == 1 ? Store(s, a, used, child)
                         : Split(s, a, pose, sub, child, (Uint16)(depth - 1), used)))
            return false;
    }
    return true;
}
static bool Lights(SotnActors *s, Actor *a, SotnActorPrimitive glass, SotnGlassTransform pose,
                   const Uint8 color[4], Uint16 *used)
{
    SotnActorPrimitive light = glass;
    light.palette = 0;
    light.page = 15;
    light.priority = 85;
    light.blend = false;
    Gray(&light, color[3], color[3] / 2);
    if (!Store(s, a, used, light))
        return false;
    SotnDataView rotation = {0};
    SotnGlassVertex model[4] = {0};
    if (!Slice(s->nz0, 0x26bc, 6, &rotation) || Read16(rotation.bytes) ||
        Read16(rotation.bytes + 2) || Read16(rotation.bytes + 4) ||
        !ChapelGlass_Model(s, 0x2594, model, 4))
        return SDL_SetError("Chapel light requires its original identity base rotation and model");
    pose.z_angle = -1024;
    light = glass;
    light.page = 0x6f;
    light.palette = 0x204;
    light.blend = true;
    if (!ChapelGlass_Project(s, pose, model, &light) || !Store(s, a, used, light))
        return false;
    const Sint16 midpoint_x = (Sint16)((light.offset_x[0] + light.offset_x[2]) / 2),
                 midpoint_y = (Sint16)((light.offset_y[0] + light.offset_y[1]) / 2);
    light = glass;
    light.page = 0x2f;
    light.palette = 0;
    light.priority = 87;
    light.blend = true;
    for (Uint16 i = 0; i < 4; ++i)
    {
        light.vertex_red[i] = i < 2 ? 24 : i == 2 ? 8 : 0;
        light.vertex_green[i] = i < 2 ? 8 : i == 2 ? 24 : 0;
        light.vertex_blue[i] = i < 2 ? 32 : i == 2 ? 8 : 0;
    }
    light.offset_x[0] = glass.offset_x[1];
    light.offset_y[0] = glass.offset_y[1];
    light.offset_x[1] = glass.offset_x[0];
    light.offset_y[1] = glass.offset_y[0];
    light.offset_x[2] = glass.offset_x[3];
    light.offset_y[2] = glass.offset_y[3];
    light.offset_x[3] = midpoint_x;
    light.offset_y[3] = midpoint_y;
    return Store(s, a, used, light);
}
static bool Glass(SotnActors *s, Actor *a)
{
    if (!a->state.step)
    {
        if (!ChapelGlass_Reserve(s, a, 60))
            return false;
        if (a->remove)
            return true;
        for (Uint16 i = 0; i < 12; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (!p)
                return false;
            p->state = (SotnActorPrimitive){.textured = true,
                                            .screen_space = true,
                                            .quad = true,
                                            .priority_set = true,
                                            .priority = 86,
                                            .page = 0x2f,
                                            .palette = 14,
                                            .blend = true};
            p->state.u[0] = p->state.u[2] = 4;
            p->state.u[1] = p->state.u[3] = 28;
            p->state.v[0] = p->state.v[1] = 1;
            p->state.v[2] = p->state.v[3] = 174;
            Gray(&p->state, 96, 96);
        }
        return true;
    }
    if (a->state.step != 1 || a->state.primitive_count != 60)
        return SDL_SetError("Chapel stained glass entered an invalid original state");
    for (Uint16 i = 12; i < 60; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        p->state.active = false;
    }
    SotnGlassVertex model[4] = {0};
    if (!ChapelGlass_Model(s, 0x2574, model, 4))
        return false;
    Uint16 used = 12;
    const Sint32 x = Pixel(a->state.x) - s->camera_x - 128,
                 y = Pixel(a->state.y) - s->camera_y - 160;
    for (Uint16 i = 0; i < 12; ++i)
    {
        SotnDataView record = {0}, angle = {0}, color = {0};
        if (!Slice(s->nz0, 0x25b4U + i * 16U, 16, &record) ||
            !Slice(s->nz0, 0x2674U + i * 2U, 2, &angle) ||
            !Slice(s->nz0, 0x268cU + i * 4U, 4, &color))
            return false;
        const Sint32 px = Signed32(record.bytes), py = Signed32(record.bytes + 4),
                     depth = Signed32(record.bytes + 8), recurse = Signed32(record.bytes + 12);
        if (px < -512 || px > 512 || py < -512 || py > 512 || depth < 0 || depth > 4096 ||
            recurse < 0 || recurse > 3)
            return SDL_SetError("Chapel glass panel has an invalid private pose/subdivision depth");
        SotnGlassTransform pose = {.y_angle = (Sint16)Signed16(angle.bytes),
                                   .x = px + x,
                                   .y = py + y,
                                   .depth = depth + 1024};
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        SotnActorPrimitive q = p->state;
        q.page = px > 0 ? 0x2f : 15;
        q.blend = px > 0;
        q.active = true;
        for (Uint16 vertex = 0; vertex < 4; ++vertex)
        {
            q.vertex_red[vertex] = color.bytes[0];
            q.vertex_green[vertex] = color.bytes[1];
            q.vertex_blue[vertex] = color.bytes[2];
        }
        if ((recurse && !Split(s, a, pose, model, q, (Uint16)recurse, &used)) ||
            !ChapelGlass_Project(s, pose, model, &q))
            return false;
        q.active = recurse == 0;
        p->state = q;
        pose.x = 96 + x;
        pose.y = 256 + y;
        if (px > 0 && !Lights(s, a, q, pose, color.bytes, &used))
            return false;
    }
    return true;
}
bool ActorChapelGlass(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 6 || (a->state.id != 0x17 && a->state.id != 0x18))
        return SDL_SetError("Chapel glass requires its original actor and stage owner");
    return a->state.id == 0x18 ? ChapelGlass_Background(s, a) : Glass(s, a);
}
