// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted st/no2/e_secrets.c and selected st/prim_helpers.h at sotn-decomp
// 7bc54d8157b6. Tile patches, shard profiles, artwork and SDK tables stay private.
#include "actors_internal.h"

static Sint32 Word(Sint64 value)
{
    const Uint32 bits = (Uint32)value;
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 0x100000000LL);
}
static Sint32 Whole(Sint32 position, Sint32 integer)
{
    return (Sint32)((Sint64)Wrap16(integer) * 65536 + ((Uint32)position & 65535U));
}
static bool Patch(SotnActors *s, bool ceiling, bool opened)
{
    SotnDataView indices = {0}, tiles = {0};
    const Uint16 count = ceiling ? 8 : 10;
    if (!Slice(s->nz0, ceiling ? 0xe14 : 0xdec, count * 2U, &indices) ||
        !Slice(s->nz0, ceiling ? 0xe24 + (opened ? 16U : 0U) : 0xe00, count * 2U, &tiles))
        return false;
    // Original arrays use each room's actual indexing; unlike NO2 spikes, no 96-tile stride.
    for (Uint16 i = 0; i < count; ++i)
        if (!s->mutable_tiles ||
            Read16(indices.bytes + i * 2U) >= (Uint64)s->map.width * s->map.height ||
            Read16(tiles.bytes + i * 2U) >= s->map.types.size)
            return SDL_SetError("NO2 secret tile patch exceeds the owned room or definitions");
    for (Uint16 i = 0; i < count; ++i)
        if (!ActorTile(s, Read16(indices.bytes + i * 2U), Read16(tiles.bytes + i * 2U)))
            return false;
    return true;
}
static bool Reserve(SotnActors *s, Actor *a, Uint16 count, bool consecutive)
{
    const bool allocated =
        consecutive ? ActorPrimitivesContiguous(s, a, count) : ActorPrimitives(s, a, count);
    if (allocated)
        a->state.flags |= 0x800000U;
    return allocated;
}
static bool Pair(SotnActors *s, Actor *a, Uint16 index, ActorPrimitive **p)
{
    if ((Uint32)index + 1U >= a->state.primitive_count)
        return SDL_SetError("NO2 secret shard pair exceeds its owned primitive list");
    ActorPrimitive *meta = ActorPrimitiveGet(s, a, (Uint16)(index + 1U));
    *p = ActorPrimitiveGet(s, a, index);
    if (!meta || !*p)
        return false;
    // PrimResetNext leaves displayed geometry untouched until the next helper projection.
    (*p)->secret.x = (*p)->secret.y = (*p)->vx = (*p)->vy = 0;
    (*p)->secret.width = (*p)->secret.height = 0;
    (*p)->secret.tag = 8;
    (*p)->secret.kind = 0;
    (*p)->phase = (*p)->timer = 0;
    (*p)->corner_angle = 0;
    meta->secret.tag = 1;
    meta->state.active = false;
    return true;
}
static bool FreePair(SotnActors *s, Actor *a, Uint16 index)
{
    ActorPrimitive *p = ActorPrimitiveGet(s, a, index),
                   *meta = ActorPrimitiveGet(s, a, (Uint16)(index + 1U));
    if (!p || !meta)
        return false;
    p->secret.tag = meta->secret.tag = 0;
    p->state.active = meta->state.active = false;
    return true;
}
static bool FindPair(SotnActors *s, Actor *a, ActorPrimitive **out)
{
    *out = NULL;
    for (Uint16 i = 0; (Uint32)i + 1U < a->state.primitive_count; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i),
                       *next = ActorPrimitiveGet(s, a, (Uint16)(i + 1U));
        if (!p || !next)
            return false;
        if (!p->secret.tag && !next->secret.tag)
            return Pair(s, a, i, out);
    }
    return true;
}
static bool Project(SotnActors *s, ActorPrimitive *p)
{
    SotnMathTrig trig = {0};
    if (!SotnMath_MatrixTrig(s->executable, p->corner_angle, &trig))
        return false;
    p->secret.x = Word((Sint64)p->secret.x + p->vx);
    p->secret.y = Word((Sint64)p->secret.y + p->vy);
    p->state.x = Pixel(p->secret.x) * 65536;
    p->state.y = Pixel(p->secret.y) * 65536;
    p->state.active = p->state.quad = p->state.textured = p->state.priority_set = true;
    p->state.width = p->secret.width;
    p->state.height = p->secret.height;
    // Selected UnkPrimHelper: identity X/Y, axial Z, depth/screen1024, no scale.
    for (Uint16 i = 0; i < 4; ++i)
    {
        const Sint32 x = (i & 1U) ? p->secret.width / 2 : -(Sint32)(p->secret.width / 2),
                     y = (i & 2U) ? p->secret.height / 2 : -(Sint32)(p->secret.height / 2);
        p->state.offset_x[i] =
            (Sint16)ActorShift((Sint64)trig.cosine * x - (Sint64)trig.sine * y, 12);
        p->state.offset_y[i] =
            (Sint16)ActorShift((Sint64)trig.sine * x + (Sint64)trig.cosine * y, 12);
    }
    return true;
}
static void Texture(ActorPrimitive *p, bool small)
{
    const Uint8 kind = p->secret.kind;
    p->state.page = 15;
    p->state.palette = 0x33;
    p->state.priority = small ? 0x6a : 0x68;
    p->state.u[0] = p->state.u[2] = small ? 0xb8 : (kind & 1U) ? 0xb8 : 0xa8;
    p->state.u[1] = p->state.u[3] = small ? 0xc8 : (kind & 1U) ? 0xa8 : 0xb8;
    p->state.v[0] = p->state.v[1] = !small && ((kind & 3U) >= 2) ? 0xd8 : 0xc8;
    p->state.v[2] = p->state.v[3] = !small && ((kind & 3U) >= 2) ? 0xc8 : 0xd8;
}
static bool Turn(SotnActors *s, ActorPrimitive *p, bool falling)
{
    SotnDataView angle = {0};
    if (p->secret.kind >= 18 || !Slice(s->nz0, 0xdc8 + p->secret.kind * 2U, 2, &angle))
        return SDL_SetError("NO2 secret shard references an invalid private rotation profile");
    const Sint32 speed = falling && !p->secret.kind ? -64 : Signed16(angle.bytes);
    p->corner_angle = (Sint16)Wrap16((Sint32)p->corner_angle + speed);
    return true;
}
static bool Shard(SotnActors *s, Actor *a, Uint16 index)
{
    ActorPrimitive *p = ActorPrimitiveGet(s, a, index);
    if (!p || p->secret.tag != 8 || (Uint32)index + 1U >= a->state.primitive_count)
        return SDL_SetError("NO2 secret shard requires a complete owned active pair");
    SotnCollider collision = {0};
    Sint32 x = Pixel(p->secret.x), y = Pixel(p->secret.y);
    switch (p->phase)
    {
    case 0: {
        SotnDataView velocity = {0};
        if (p->secret.kind >= 16 || !Slice(s->nz0, 0xd48 + p->secret.kind * 8U, 8, &velocity))
            return SDL_SetError("NO2 secret shard references an invalid private launch profile");
        Texture(p, false);
        p->vx = Signed32(velocity.bytes);
        p->vy = Signed32(velocity.bytes + 4);
        p->secret.width = p->secret.height = 16;
        p->phase = 1;
        if (p->secret.kind > 7 && p->secret.kind < 12)
        {
            p->secret.width = p->secret.height = 8;
            p->phase = 4;
        }
        break;
    }
    case 1:
        p->vy = Word((Sint64)p->vy + (p->secret.kind > 11 ? 6144 : 2048));
        if (!Turn(s, p, false) || !ActorQuery(s, x, Wrap16((Sint64)y + 4), &collision))
            return false;
        if (collision.effects & SOTN_EFFECT_SOLID)
        {
            y = Wrap16((Sint64)y + 4 + collision.top);
            for (Uint16 i = 0; i < 3; ++i)
            {
                ActorPrimitive *child = NULL;
                if (!FindPair(s, a, &child))
                    return false;
                if (child)
                {
                    child->phase = p->secret.kind < 12 ? 2 : 3;
                    child->secret.x = Wrap16((Sint64)x + (p->secret.kind < 12 ? i * 4 : 0)) * 65536;
                    child->secret.y = Wrap16((Sint64)y - 8) * 65536;
                }
            }
            return FreePair(s, a, index);
        }
        break;
    case 2:
    case 3:
        Texture(p, true);
        p->vx = p->phase == 2 ? -(Sint32)(ActorRandom(s) & 7U) * 0x2800
                              : 0xe000 - (Sint32)(ActorRandom(s) & 7U) * 0x4000;
        p->vy = -0x18000 - (Sint32)(ActorRandom(s) & 7U) * 0x2000;
        p->secret.width = p->secret.height = (Uint16)((ActorRandom(s) & 3U) * 2U + 8U);
        if (p->phase == 3)
            p->secret.kind = p->vx > 0 ? 16 : 17;
        p->phase = 4;
        break;
    case 4:
        p->vy = Word((Sint64)p->vy + 8192);
        if (!Turn(s, p, true) ||
            !ActorQuery(s, x, Wrap16((Sint64)y + p->secret.width / 2), &collision))
            return false;
        if (collision.effects & SOTN_EFFECT_SOLID)
        {
            p->secret.y = Whole(p->secret.y, Wrap16((Sint64)y + collision.top));
            p->vy = Word(-(Sint64)p->vy / 2);
            if (p->vy > -16384)
            {
                Actor *effect = NULL;
                if (!ActorSpawn(s, a, 6, 0x10, 160, 192, &effect))
                    return false;
                if (effect)
                {
                    effect->state.x = x * 65536;
                    effect->state.y = Wrap16((Sint64)y + p->secret.width / 2) * 65536;
                    effect->state.facing_left = false;
                }
                return FreePair(s, a, index);
            }
        }
        break;
    default:
        return SDL_SetError("NO2 secret shard entered an invalid original phase");
    }
    return Project(s, p);
}
static bool Shards(SotnActors *s, Actor *a, bool *finished)
{
    *finished = true;
    for (Uint16 i = 0; i < a->state.primitive_count; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        if (p->secret.tag & 8U)
        {
            *finished = false; // Original completion observes the pre-update active bit.
            if (!Shard(s, a, i))
                return false;
        }
    }
    return true;
}
static bool Burst(SotnActors *s, Actor *a, bool ceiling)
{
    Actor *effect = NULL;
    if (!ActorSpawn(s, a, 2, 0xaa13, 160, 192, &effect))
        return false;
    if (effect)
    {
        effect->state.x = Pixel(a->state.x) * 65536;
        effect->state.y = Wrap16((Sint64)Pixel(a->state.y) + 32) * 65536;
        effect->state.facing_left = false;
    }
    for (Uint16 i = 0; i < 8; ++i)
    {
        if (!ActorSpawn(s, a, 6, 0xaa10, 160, 192, &effect))
            return false;
        if (effect)
        {
            const Sint32 dx = 15 - (Sint32)(ActorRandom(s) & 31U),
                         dy = ceiling ? (Sint32)(ActorRandom(s) & 31U)
                                      : 15 - (Sint32)(ActorRandom(s) & 31U);
            effect->state.x = Wrap16((Sint64)Pixel(a->state.x) + dx) * 65536;
            effect->state.y = Wrap16((Sint64)Pixel(a->state.y) + dy) * 65536;
            effect->state.facing_left = false;
        }
    }
    return ActorSound(s, 0x644);
}
static bool Wall(SotnActors *s, Actor *a)
{
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x8a4))
            return false;
        a->state.frame = 0;
        if (s->world->flags[SOTN_NO2_SECRET_WALL])
        {
            if (!Patch(s, false, true))
                return false;
            a->remove = true;
            return true;
        }
        a->state.priority = 0xa8;
        a->state.hitbox_state = 2;
        a->state.hit_points = 32767;
        a->state.hitbox_width = 16;
        a->state.hitbox_height = 40;
        break;
    case 1:
        if (a->hit_flags)
        {
            if (!ActorPositionalSound(s, a, 0x644))
                return false;
            a->secret.timer = 16;
            ++a->secret.hits;
            ++a->state.step;
        }
        if (a->secret.hits == 3)
        {
            a->state.hitbox_state = 0;
            a->state.step = 3;
        }
        break;
    case 2:
        if (a->secret.hits == 1)
            a->state.frame = 12;
        if (a->secret.hits == 2)
            a->state.frame = 13;
        if (!a->secret.timer)
            return SDL_SetError("NO2 secret wall recovery lost its original timer");
        if (!--a->secret.timer)
            --a->state.step;
        break;
    case 3:
        if (!Reserve(s, a, 32, false))
        {
            SDL_ClearError();
            if (!Patch(s, false, true))
                return false;
            s->world->flags[SOTN_NO2_SECRET_WALL] |= 1U;
            a->remove = true;
            return true;
        }
        for (Uint16 i = 0; i < 2; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (!p)
                return false;
            p->state = (SotnActorPrimitive){.x = a->state.x,
                                            .y = a->state.y,
                                            .active = true,
                                            .quad = true,
                                            .textured = true,
                                            .priority_set = true,
                                            .priority = 0xa8,
                                            .page = 15,
                                            .palette = 0x21,
                                            .u = {0x80, 0x97, 0x80, 0x97},
                                            .v = {0xf0, 0xf0, 0xff, 0xff}};
            const Sint16 top = i ? 8 : -24;
            p->state.offset_x[0] = p->state.offset_x[2] = -17;
            p->state.offset_x[1] = p->state.offset_x[3] = 7;
            p->state.offset_y[0] = p->state.offset_y[1] = top;
            p->state.offset_y[2] = p->state.offset_y[3] = (Sint16)(top + 16);
        }
        a->secret.timer = 32;
        ++a->state.step;
        break;
    case 4:
        if (!a->secret.timer)
            return SDL_SetError("NO2 secret wall reveal lost its original timer");
        if (!--a->secret.timer)
            ++a->state.step;
        break;
    case 5:
        if (!Patch(s, false, true))
            return false;
        a->state.frame = 0;
        s->world->flags[SOTN_NO2_SECRET_WALL] |= 1U;
        for (Uint16 i = 0; i < 8; ++i)
        {
            ActorPrimitive *p = NULL;
            if (!Pair(s, a, (Uint16)(i * 2U), &p))
                return false;
            p->secret.x = Wrap16((Sint64)Pixel(a->state.x) - 8 + (i % 2U) * 16U) * 65536;
            p->secret.y = Wrap16((Sint64)Pixel(a->state.y) - 24 + (i / 2U) * 16U) * 65536;
            p->secret.kind = (Uint8)i;
        }
        if (!Burst(s, a, false))
            return false;
        a->secret.timer = 384;
        ++a->state.step;
        break;
    case 6: {
        bool done = false;
        if (!Shards(s, a, &done) || !a->secret.timer)
            return SDL_SetError("NO2 secret wall debris has invalid state or timer");
        if (!--a->secret.timer)
        {
            s->world->flags[SOTN_NO2_SECRET_WALL] |= 2U;
            a->remove = true;
        }
        break;
    }
    default:
        return SDL_SetError("NO2 secret wall entered an invalid original state");
    }
    return true;
}
static bool Dust(SotnActors *s, const Actor *a, ActorPrimitive *p)
{
    if (!p->secret.initialized)
    {
        p->state = (SotnActorPrimitive){.active = true,
                                        .width = 1,
                                        .height = 1,
                                        .red = 128,
                                        .green = 128,
                                        .blue = 192,
                                        .priority_set = true,
                                        .priority = 0x68};
        p->secret.x = Pixel(a->state.x) * 65536;
        p->secret.y = Wrap16((Sint64)Pixel(a->state.y) + 8) * 65536;
        p->vx = 0x7000 - (Sint32)(ActorRandom(s) & 7U) * 0x2000;
        p->vy = 0x7000 - (Sint32)(ActorRandom(s) & 7U) * 0x2000;
        p->secret.initialized = true;
        p->timer = 32;
    }
    p->secret.x = Word((Sint64)p->secret.x + p->vx);
    p->secret.y = Word((Sint64)p->secret.y + p->vy);
    p->state.x = Pixel(p->secret.x) * 65536;
    p->state.y = Pixel(p->secret.y) * 65536;
    p->vy = Word((Sint64)p->vy + 8192);
    if (!p->timer)
        return SDL_SetError("NO2 ceiling dust retained an expired lifetime");
    if (!--p->timer)
    {
        p->secret.initialized = false;
        p->secret.tag = 0;
        p->state.active = false;
    }
    return true;
}
static bool Ceiling(SotnActors *s, Actor *a)
{
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x8a4))
            return false;
        a->state.frame = 0;
        a->state.rotated = true;
        a->state.rotation = 3072;
        if (s->world->flags[SOTN_NO2_SECRET_CEILING])
        {
            if (!Patch(s, true, true))
                return false;
            a->remove = true;
            return true;
        }
        if (!Patch(s, true, false))
            return false;
        a->state.hitbox_state = 2;
        a->state.hit_points = 128;
        a->state.hitbox_width = 16;
        a->state.hitbox_height = 40;
        if (!Reserve(s, a, 30, true))
        {
            SDL_ClearError();
            a->remove = true;
        }
        break;
    case 1:
        if (a->state.primitive_count != 30)
            return SDL_SetError("NO2 ceiling lost its original dust reservation");
        if (a->hit_flags)
            for (Uint16 n = 0; n < 16; ++n)
                for (Uint16 i = 0; i < 30; ++i)
                {
                    ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
                    if (!p)
                        return false;
                    if (!p->secret.tag)
                    {
                        p->secret.tag = 1;
                        break;
                    }
                }
        for (Uint16 i = 0; i < 30; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (!p || (p->secret.tag && !Dust(s, a, p)))
                return false;
            if (i == 29)
            {
                // Original zero-width final tile is invisible but retains its particle lifetime.
                p->secret.x = (Sint32)((Uint32)p->secret.x & 65535U);
                p->secret.y = (Sint32)((Uint32)p->secret.y & 65535U);
                p->state.active = false;
                p->state.width = 0;
            }
        }
        if (a->state.flags & 0x100U)
        {
            a->state.frame = 0;
            ++a->state.step;
        }
        break;
    case 2:
        ActorPrimitivesClear(s, a);
        a->state.primitive_count = 0;
        a->state.flags &= ~0x800000U;
        if (!Patch(s, true, true))
            return false;
        s->world->flags[SOTN_NO2_SECRET_CEILING] = 1;
        if (!Reserve(s, a, 24, false))
        {
            SDL_ClearError();
            a->remove = true;
            return true;
        }
        for (Uint16 i = 0; i < 4; ++i)
        {
            ActorPrimitive *p = NULL;
            if (!Pair(s, a, (Uint16)(i * 2U), &p))
                return false;
            p->secret.x = Wrap16((Sint64)Pixel(a->state.x) - 8 + (i % 2U) * 16U) * 65536;
            p->secret.y = Wrap16((Sint64)Pixel(a->state.y) + (i / 2U) * 16U) * 65536;
            p->secret.kind = (Uint8)(i + 12U);
        }
        if (!Burst(s, a, true))
            return false;
        ++a->state.step;
        break;
    case 3: {
        bool done = false;
        if (!Shards(s, a, &done))
            return false;
        a->remove = done;
        break;
    }
    default:
        return SDL_SetError("NO2 secret ceiling entered an invalid original state");
    }
    return true;
}
static bool Aftermath(SotnActors *s, Actor *a)
{
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x8a4))
            return false;
        a->state.hitbox_state = a->state.frame = 0;
        break;
    case 1:
        if (s->world->flags[SOTN_NO2_SECRET_WALL] & 2U)
        {
            a->remove = true;
            return true;
        }
        if (s->world->flags[SOTN_NO2_SECRET_WALL])
            s->world->flags[SOTN_NO2_SECRET_WALL] |= 2U;
        if (!Reserve(s, a, 8, false))
        {
            SDL_ClearError();
            a->remove = true;
            return true;
        }
        for (Uint16 i = 0; i < 4; ++i)
        {
            ActorPrimitive *p = NULL;
            if (!Pair(s, a, (Uint16)(i * 2U), &p))
                return false;
            p->secret.x = Pixel(a->state.x) * 65536;
            p->secret.y = Pixel(a->state.y) * 65536;
            p->secret.kind = (Uint8)(i + 8U);
        }
        ++a->state.step;
        break;
    case 2: {
        bool done = false;
        if (!Shards(s, a, &done))
            return false;
        a->remove = done;
        break;
    }
    default:
        return SDL_SetError("NO2 secret wall aftermath entered an invalid original state");
    }
    return true;
}
bool ActorOlroxSecret(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 4 || !s->world || !a->state.generation)
        return SDL_SetError("NO2 secret requires an active original actor and persistent world");
    switch (a->state.id)
    {
    case 0x24:
        return Wall(s, a);
    case 0x25:
        return Ceiling(s, a);
    case 0x26:
        return Aftermath(s, a);
    default:
        return SDL_SetError("NO2 secret received an invalid actor identity");
    }
}
