// SPDX-License-Identifier: AGPL-3.0-or-later
// Pinned sotn-decomp bo4/doors.c:EntityUnkId19/EntityUnkId1A.
// Native initialized primitive owners replace global linked GPU packets.
#include "actors_internal.h"
#include "boss_projection.h"
static bool Trig(SotnActors *s, Sint32 angle, SotnMathTrig *out)
{
    return SotnMath_Trig(s->executable, angle, out);
}
static void Tile(SotnActorPrimitive *p, Sint32 x, Sint32 y, Uint16 width, Uint16 height)
{
    p->screen_space = true;
    p->x = x * 65536;
    p->y = y * 65536;
    p->width = width;
    p->height = height;
    p->active = width && height;
}
static bool Ring(SotnActors *s, Actor *a)
{
    const Sint32 x = Pixel(a->state.x) - s->camera_x, y = Pixel(a->state.y) - s->camera_y;
    for (Uint16 i = 0; i < 16; ++i)
    {
        ActorPrimitive *primitive = ActorPrimitiveGet(s, a, i);
        SotnMathTrig first = {0}, next = {0};
        if (primitive == NULL || !Trig(s, i * 256, &first) || !Trig(s, (i + 1) * 256, &next))
            return false;
        primitive->state = (SotnActorPrimitive){.x = x * 65536,
                                                .y = y * 65536,
                                                .active = true,
                                                .textured = true,
                                                .quad = true,
                                                .gouraud = true,
                                                .screen_space = true,
                                                .priority_set = true,
                                                .priority = 0x40,
                                                .page = 0x1a,
                                                .palette = 0x15f};
        SotnActorPrimitive *p = &primitive->state;
        p->offset_x[0] = (Sint16)ActorShift((Sint64)ActorShift(first.cosine, 4) * 64, 8);
        p->offset_y[0] = (Sint16)-ActorShift((Sint64)ActorShift(first.sine, 4) * 64, 8);
        p->offset_x[1] = (Sint16)ActorShift((Sint64)ActorShift(next.cosine, 4) * 64, 8);
        p->offset_y[1] = (Sint16)-ActorShift((Sint64)ActorShift(next.sine, 4) * 64, 8);
        p->u[0] = (Uint8)(32 + ActorShift((Sint64)ActorShift(first.cosine, 4) * 30, 8));
        p->v[0] = (Uint8)(224 - ActorShift((Sint64)ActorShift(first.sine, 4) * 30, 8));
        p->u[1] = (Uint8)(32 + ActorShift((Sint64)ActorShift(next.cosine, 4) * 30, 8));
        p->v[1] = (Uint8)(224 - ActorShift((Sint64)ActorShift(next.sine, 4) * 30, 8));
        p->u[2] = p->u[3] = 32;
        p->v[2] = p->v[3] = 224;
    }
    SotnDataView phases = {0};
    if (!Slice(s->nz0, 0x5ec, 64, &phases))
        return false;
    for (Uint16 i = 0; i < 16; ++i)
        a->arena.phases[i] = Read32(phases.bytes + i * 4U);
    return true;
}
static bool RingColors(SotnActors *s, Actor *a)
{
    a->arena.phase = (Uint16)(a->arena.phase + 16);
    SotnMathTrig wave = {0};
    if (!Trig(s, a->arena.phase, &wave))
        return false;
    const Sint32 brightness = SDL_clamp(ActorShift(wave.sine, 8) + 208, 0, 255);
    const Sint32 amplitude = ActorShift(wave.cosine, 8) + 208;
    for (Uint16 i = 0; i < 16; ++i)
    {
        ActorPrimitive *primitive = ActorPrimitiveGet(s, a, i);
        if (primitive == NULL)
            return false;
        SotnActorPrimitive *p = &primitive->state;
        Uint8 *colors[3] = {p->vertex_red, p->vertex_green, p->vertex_blue};
        for (Uint16 channel = 0; channel < 3; ++channel)
        {
            SotnMathTrig first = {0}, next = {0};
            if (!Trig(s, (Sint32)(a->arena.phases[(i + channel * 5U) % 16] & 4095U), &first) ||
                !Trig(s, (Sint32)(a->arena.phases[(i + channel * 5U + 1) % 16] & 4095U), &next))
                return false;
            colors[channel][0] = (Uint8)((first.sine + 4096) / 64 * amplitude / 256);
            colors[channel][1] = (Uint8)((next.sine + 4096) / 64 * amplitude / 256);
            colors[channel][2] = colors[channel][3] = (Uint8)brightness;
        }
        a->arena.phases[i] += 32;
    }
    return true;
}
bool ActorDoppleLight(SotnActors *s, Actor *a)
{
    if (s == NULL || a == NULL || s->stage_id != 26 || a->state.id != 0x19)
        return SDL_SetError("BO4 opening light requires its original actor");
    switch (a->state.step)
    {
    case 0:
        return ActorInitialize(s, a, 0x3d4);
    case 1:
        if (a->arena.triggered)
        {
            ++a->state.step;
            a->arena.triggered = false;
        }
        return true;
    case 2:
        if (!ActorPrimitivesContiguous(s, a, 48))
        {
            a->remove = true;
            return true;
        }
        for (Uint16 i = 0; i < 48; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (p == NULL)
                return false;
            p->state = (SotnActorPrimitive){.red = 8,
                                            .green = 8,
                                            .blue = 8,
                                            .blend = true,
                                            .page = 0x20,
                                            .screen_space = true,
                                            .priority_set = true,
                                            .priority = 0xd0};
        }
        ++a->state.step;
        return true;
    case 3: {
        if (a->slot == 0)
            return SDL_SetError("BO4 light has no preceding opening plate");
        const Sint32 signed_offset = s->slots[a->slot - 1].arena.offset / 65536;
        const Sint32 offset = signed_offset < 0 ? -signed_offset : signed_offset;
        if (offset > 256)
            return SDL_SetError("BO4 opening light offset exceeds its original box");
        const Sint32 x = Pixel(a->state.x) - s->camera_x, y = Pixel(a->state.y) - s->camera_y - 40;
        for (Uint16 i = 0; i < 48; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (p == NULL)
                return false;
            if (i < 32)
                Tile(&p->state, x - offset - 32 + i, y - 32 + i, (Uint16)(offset * 2 + 64 - i * 2),
                     (Uint16)(148 - i * 2));
            else
                Tile(&p->state, x - offset, y, (Uint16)(offset * 2), 84);
        }
        if (a->arena.triggered)
        {
            ActorPrimitive *last = ActorPrimitiveGet(s, a, 47);
            if (last == NULL)
                return false;
            Tile(&last->state, 0, 0, 255, 255);
            if (last->state.red < 248)
                last->state.green = last->state.blue = last->state.red =
                    (Uint8)(last->state.red + 8);
            else
            {
                if (!ActorDoppleTrigger(s, (Uint32)a->slot + 1))
                    return false;
                ++a->state.step;
            }
        }
        return true;
    }
    case 4: {
        for (Uint16 i = 0; i < 48; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (p == NULL)
                return false;
            p->state.active = i == 0 || i == 47;
            if (i == 0)
            {
                p->state.red = p->state.green = p->state.blue = 255;
                p->state.priority = 0x40;
                p->state.blend = false;
            }
        }
        Actor *root = NULL;
        if (!ActorSpawnAt(s, a, 0x16, 0, 0, &root) || !ActorDoppleTrigger(s, (Uint32)a->slot + 1) ||
            !ActorSound(s, 0x7cc))
            return false;
        if (!SotnWorld_VisitEvent(s->world, SOTN_TIME_DOPPLEGANGER))
            return false;
        ++a->state.step;
        return true;
    }
    case 5: {
        ActorPrimitive *last = ActorPrimitiveGet(s, a, 47);
        if (last == NULL)
            return false;
        if (last->state.red > 8)
        {
            last->state.green = last->state.blue = last->state.red = (Uint8)(last->state.red - 8);
            return true;
        }
        ActorPrimitivesClear(s, a);
        a->state.primitive_count = 0;
        // Releasing 48 consecutive records always leaves room for the original 16-record ring.
        if (!ActorPrimitivesContiguous(s, a, 16) || !Ring(s, a))
            return false;
        ++a->state.step;
        SDL_FALLTHROUGH;
    }
    case 6:
        return RingColors(s, a);
    default:
        return SDL_SetError("BO4 opening light entered an invalid step");
    }
}
bool ActorDoppleShock(SotnActors *s, Actor *a)
{
    if (s == NULL || a == NULL || s->stage_id != 26 || a->state.id != 0x1a)
        return SDL_SetError("BO4 opening shock requires its original actor");
    switch (a->state.step)
    {
    case 0:
        return ActorInitialize(s, a, 0x3d4);
    case 1:
        if (a->arena.triggered)
            ++a->state.step;
        return true;
    case 2: {
        a->arena.triggered = false;
        if (!ActorPrimitivesContiguous(s, a, 384))
        {
            a->remove = true;
            return true;
        }
        for (Uint16 i = 0; i < 384; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (p == NULL)
                return false;
            p->state = (SotnActorPrimitive){.page = 0x3a,
                                            .palette = 0x15f,
                                            .textured = true,
                                            .quad = true,
                                            .blend = true,
                                            .modulate = true,
                                            .screen_space = true,
                                            .priority_set = true,
                                            .priority = 0xd0,
                                            .red = 16,
                                            .green = 16,
                                            .blue = 16,
                                            .u = {32, 40, 32, 40},
                                            .v = {160, 160, 168, 168}};
        }
        SotnDataView angles = {0};
        if (!Slice(s->nz0, 0x62c, 16, &angles))
            return false;
        for (Uint16 i = 0; i < 8; ++i)
            a->arena.angles[i] = (Sint16)Signed16(angles.bytes + i * 2U);
        a->arena.growth = 0;
        if (!ActorSound(s, 0x7cb))
            return false;
        ++a->state.step;
        return true;
    }
    case 3: {
        SotnDataView speeds = {0}, sizes = {0};
        if (!Slice(s->nz0, 0x640, 16, &speeds) || !Slice(s->nz0, 0x654, 16, &sizes))
            return false;
        const Sint32 x = Pixel(a->state.x) - s->camera_x, y = Pixel(a->state.y) - s->camera_y;
        Uint16 used = 0;
        for (Uint16 i = 0; i < 8; ++i)
        {
            const Sint32 original_size = Signed16(sizes.bytes + i * 2U);
            if (original_size < 0 || original_size > 32)
                return SDL_SetError("BO4 shock diamond exceeds its original private geometry");
            const Sint16 size = (Sint16)SDL_max(0, original_size - a->arena.growth / 16);
            for (Sint16 distance = 0, j = 0; distance < 192 && used <= 368; distance += 3, ++j)
            {
                ActorPrimitive *primitive = ActorPrimitiveGet(s, a, used++);
                if (primitive == NULL)
                    return false;
                SotnActorPrimitive *p = &primitive->state;
                p->active = true;
                p->x = x * 65536;
                p->y = y * 65536;
                const Sint16 vx[4] = {distance, (Sint16)(distance + size),
                                      (Sint16)(distance - size), distance};
                const Sint16 vy[4] = {(Sint16)-size, 0, 0, size};
                for (Uint16 vertex = 0; vertex < 4; ++vertex)
                {
                    SotnProjectedPoint point = {0};
                    if (!SotnBoss_ProjectIntro(s->executable, x, y, a->arena.angles[i], vx[vertex],
                                               vy[vertex], &point))
                        return false;
                    p->offset_x[vertex] = (Sint16)(point.x - x);
                    p->offset_y[vertex] = (Sint16)(point.y - y);
                }
                p->red = (Uint8)SDL_max(0, 32 - (i % 3) * 8 - j / 4);
                p->green = (Uint8)SDL_max(0, 32 - (i % 2) * 8 - j / 4);
                p->blue = (Uint8)SDL_max(0, 32 - (i % 4) * 8 - j / 4);
            }
            a->arena.angles[i] =
                (Sint16)Wrap16(a->arena.angles[i] + Signed16(speeds.bytes + i * 2U));
        }
        for (; used < 384; ++used)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, used);
            if (p == NULL)
                return false;
            p->state.active = false;
        }
        if (a->arena.triggered)
            a->arena.growth = (Uint16)(a->arena.growth + 4);
        if (a->arena.growth > 512)
            a->remove = true;
        if (s->control.timer == 0)
        {
            ActorPrimitivesClear(s, a);
            a->state.primitive_count = 0;
            s->combat.music_change = true;
            s->combat.music = 0;
            a->state.step = 4;
        }
        return true;
    }
    case 4:
        if (!s->music_playing)
        {
            s->combat.music_change = true;
            s->combat.music = 0x31d;
            a->remove = true;
        }
        return true;
    default:
        return SDL_SetError("BO4 opening shock entered an invalid step");
    }
}
