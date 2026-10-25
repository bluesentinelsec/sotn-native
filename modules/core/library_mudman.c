// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/lib/e_mudman.c at the pinned reconstruction.
// Original timelines, sensors, attack boxes and fragment motion remain private.
#include "actors_internal.h"
static void Whole(Actor *a)
{
    a->state.x = Pixel(a->state.x) * 65536;
    a->state.y = Pixel(a->state.y) * 65536;
    a->state.facing_left = false;
}
static void ReleaseShape(SotnActors *s, Actor *a)
{
    ActorPrimitivesClear(s, a);
    a->state.primitive_count = 0;
    a->state.flags &= ~0x800000U;
}
static void Walk(Actor *a)
{
    a->vx = a->state.facing_left ? 16384 : -16384;
    a->state.frame = 1;
    a->state.step = 6;
}
static bool Shape(SotnActors *s, Actor *a, bool sinking)
{
    if (!ActorPrimitives(s, a, 1))
    {
        a->remove = true;
        return true;
    }
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    if (!p)
        return false;
    p->state = (SotnActorPrimitive){.x = Pixel(a->state.x) * 65536,
                                    .y = Pixel(a->state.y) * 65536,
                                    .active = true,
                                    .quad = true,
                                    .textured = true,
                                    .gouraud = true,
                                    .page = 0x14,
                                    .palette = 0x277,
                                    .priority_set = true,
                                    .priority = (Uint16)(a->state.priority + 2),
                                    .u = {128, 160, 128, 160},
                                    .v = {128, 128, 168, 168},
                                    .offset_x = {1, 1, 1, 1},
                                    .offset_y = {20, 20, 22, 22},
                                    .vertex_red = {128, 128, 66, 66},
                                    .vertex_green = {128, 128, 66, 66},
                                    .vertex_blue = {128, 128, 66, 66}};
    a->state.flags |= 0x800000U;
    if (sinking)
    {
        p->state.offset_x[0] = p->state.offset_x[2] = a->state.facing_left ? 17 : -15;
        p->state.offset_x[1] = p->state.offset_x[3] = a->state.facing_left ? -15 : 17;
        p->state.offset_y[0] = p->state.offset_y[1] = -19;
        p->state.offset_y[2] = p->state.offset_y[3] = 21;
        for (Uint16 i = 0; i < 4; ++i)
            p->state.vertex_red[i] = p->state.vertex_green[i] = p->state.vertex_blue[i] = 128;
    }
    return true;
}
static bool Weapon(SotnActors *s, Actor *a)
{
    if (!a->state.step && !ActorInitialize(s, a, 0x9ec))
        return false;
    if (a->enemy_state.owner_slot >= SOTN_STAGE_ACTOR_SLOTS)
    {
        a->remove = true;
        return true;
    }
    Actor *root = &s->slots[a->enemy_state.owner_slot];
    if (root->state.id != 0x4d || !root->state.generation ||
        root->state.generation != a->enemy_state.owner_generation)
    {
        a->remove = true;
        return true;
    }
    const Uint16 frame = root->state.frame;
    const Uint16 index = frame == 14 || frame == 15 ? (Uint16)(frame - 13) : 0;
    SotnDataView offsets = {0}, size = {0};
    if (!Slice(s->nz0, 0x2ea8 + index * 4U, 4, &offsets) ||
        !Slice(s->nz0, 0x2eb4 + index * 4U, 4, &size))
        return false;
    if (Read16(size.bytes) > 128 || Read16(size.bytes + 2) > 128)
        return SDL_SetError("Mudman weapon exceeds its private hitbox bounds");
    a->state.hitbox_x = (Sint16)Signed16(offsets.bytes);
    a->state.hitbox_y = (Sint16)Signed16(offsets.bytes + 2);
    a->state.hitbox_width = Read16(size.bytes);
    a->state.hitbox_height = Read16(size.bytes + 2);
    a->state.facing_left = root->state.facing_left;
    a->state.hitbox_state = root->state.hitbox_state;
    a->state.x = Pixel(root->state.x) * 65536 + (Sint32)((Uint32)a->state.x & 65535U);
    a->state.y = Pixel(root->state.y) * 65536 + (Sint32)((Uint32)a->state.y & 65535U);
    return true;
}
static bool Death(SotnActors *s, Actor *a)
{
    SotnDataView palette = {0};
    if (!Slice(s->nz0, 0x9e6, 2, &palette))
        return false;
    for (Uint16 i = 0; i < 4; ++i)
    {
        Actor *piece = NULL;
        if (!ActorSpawn(s, a, 0x4d, 0, 160, 192, &piece))
            return false;
        if (!piece)
            continue;
        SotnDataView motion = {0};
        if (!Slice(s->nz0, 0x2e88 + i * 8U, 8, &motion))
            return false;
        const Sint32 vx = Signed32(motion.bytes), vy = Signed32(motion.bytes + 4);
        if (vx < -8 * 65536 || vx > 8 * 65536 || vy < -8 * 65536 || vy > 8 * 65536)
            return SDL_SetError("Mudman fragment motion exceeds its bounded private recipe");
        Whole(piece);
        piece->state.animation_set = a->state.animation_set;
        piece->state.texture_offset = a->state.texture_offset;
        piece->state.priority = a->state.priority;
        piece->state.palette =
            (a->state.palette & 0x8000U) ? Read16(palette.bytes) : a->state.palette;
        piece->state.flags = 0xcd002000U;
        piece->state.frame = (Uint16)((ActorRandom(s) & 1U) + 16);
        piece->state.step = 11;
        piece->vx = vx;
        piece->vy = vy;
    }
    Actor *flame = NULL;
    if (!ActorSpawn(s, a, 2, 2, 160, 192, &flame))
        return false;
    if (flame)
        Whole(flame);
    if (!ActorLibrarySound(s, a, 0x62b))
        return false;
    a->remove = true;
    return true;
}
static bool Morph(SotnActors *s, Actor *a)
{
    if (s->service_ticks % 2)
        return true;
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    if (!p)
        return false;
    const Sint16 direction = a->state.facing_left ? 1 : -1;
    if (a->state.step == 5)
    {
        if (a->mudman.counter < 62)
        {
            p->state.offset_x[0] += direction;
            p->state.offset_x[1] -= direction;
        }
        else
        {
            --p->state.offset_y[0];
            --p->state.offset_y[1];
            p->state.offset_x[0] -= direction;
            p->state.offset_x[1] += direction;
        }
        p->state.offset_x[2] = p->state.offset_x[0];
        p->state.offset_x[3] = p->state.offset_x[1];
        if (++a->mudman.counter >= 107)
        {
            ReleaseShape(s, a);
            Walk(a);
        }
        return true;
    }
    if (a->state.step == 2 && a->mudman.counter == 16)
        p->state.offset_x[0] -= (Sint16)(direction * 8);
    if (a->mudman.counter < 16)
    {
        p->state.offset_x[0] += direction;
        p->state.offset_x[1] -= direction;
        p->state.offset_x[2] = p->state.offset_x[0];
        p->state.offset_x[3] = p->state.offset_x[1];
    }
    else
    {
        --p->state.offset_y[1];
        if (a->mudman.counter >= 32)
            --p->state.offset_y[0];
        if (a->mudman.counter >= 54)
        {
            ++p->state.offset_y[1];
            --p->state.offset_y[0];
        }
    }
    if (a->state.step == 2 && a->mudman.counter >= 61)
        p->state.offset_x[0] += direction;
    ++a->mudman.counter;
    const Uint8 shade = (Uint8)(a->mudman.counter * 2 + 4);
    p->state.vertex_red[2] = p->state.vertex_red[3] = shade;
    p->state.vertex_green[2] = p->state.vertex_green[3] = shade;
    p->state.vertex_blue[2] = p->state.vertex_blue[3] = shade;
    if (a->mudman.counter >= 62)
    {
        if (a->state.step == 2)
        {
            a->mudman.counter = 0;
            ++a->state.step;
        }
        else
        {
            ReleaseShape(s, a);
            Walk(a);
        }
    }
    return true;
}
bool ActorLibraryMudman(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 2 || (a->state.id != 0x4d && a->state.id != 0x4e))
        return SDL_SetError("Mudman requires its original Library actor identity");
    if (a->state.id == 0x4e)
        return Weapon(s, a);
    if (s->library_enemy_flags & 2U)
        a->state.flags |= 0x100U;
    if ((a->state.flags & 0x100U) && a->state.step < 10)
        a->state.step = 10;
    Uint8 animation = 0;
    Uint16 collision = 0;
    switch (a->state.step)
    {
    case 0: {
        if (!ActorInitialize(s, a, 0x9e0))
            return false;
        a->state.priority = 120;
        Actor *child = NULL;
        if (!ActorSpawn(s, a, 0x4e, 0, 0, 128, &child))
            return false;
        if (child)
        {
            Whole(child);
            child->enemy_state.owner_slot = a->slot;
            child->enemy_state.owner_generation = a->state.generation;
        }
        a->state.hitbox_state = 0;
        a->state.flags |= 0x2000U;
        a->state.facing_left = Side(a, s->player) == 0;
        break;
    }
    case 1:
        if (!ActorGround(s, a, 0x2e70, &collision))
            return false;
        if (collision & 1U)
        {
            ++a->state.step;
            return Shape(s, a, false);
        }
        break;
    case 2:
    case 4:
    case 5:
        return Morph(s, a);
    case 3:
        if (s->service_ticks % 6 == 0)
        {
            a->substep = 6;
            ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
            if (!p)
                return false;
            p->state.offset_x[0] += (Sint16)(a->state.facing_left ? 1 : -1);
            if (a->mudman.counter++ > 3)
            {
                ReleaseShape(s, a);
                Walk(a);
                a->mudman.counter = 0;
            }
        }
        break;
    case 6:
        if (!ActorAnimate(s, a, 0x2e3c, &animation) || !ActorWalk(s, a, 0x2e80, &collision))
            return false;
        a->state.hitbox_state = 3;
        if (Distance(a, s->player) < 64)
        {
            a->pose = a->remaining = 0;
            a->state.step = 7;
        }
        if (collision & 0xe0U)
            a->state.step = 8;
        break;
    case 7:
        if (!ActorAnimate(s, a, 0x2e4c, &animation))
            return false;
        if (!animation)
        {
            a->pose = a->remaining = 0;
            a->state.step = 6;
        }
        if (!a->remaining && a->pose == 3 && !ActorLibrarySound(s, a, 0x78c))
            return false;
        break;
    case 8:
        if (!Shape(s, a, true))
            return false;
        if (a->remove)
            return true;
        a->state.frame = 0;
        a->mudman.counter = a->substep = 0;
        a->state.hitbox_state = 0;
        if (!ActorLibrarySound(s, a, 0x672))
            return false;
        ++a->state.step;
        break;
    case 9:
        if (s->service_ticks % 2 == 0)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
            if (!p)
                return false;
            if (a->mudman.counter < 16)
                p->state.offset_x[0] += (Sint16)(a->state.facing_left ? 1 : -1);
            if (a->mudman.counter < 38)
            {
                ++p->state.offset_y[0];
                ++p->state.offset_y[1];
            }
            if (a->mudman.counter++ > 38)
                a->remove = true;
        }
        break;
    case 10:
        return Death(s, a);
    case 11: {
        const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
        if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32 ||
            a->vy > SDL_MAX_SINT32 - 16384)
            return SDL_SetError("Mudman falling fragment exceeds native fixed-point bounds");
        a->state.x = (Sint32)x;
        a->state.y = (Sint32)y;
        a->state.flags |= 0xc0000000U;
        a->vy += 16384;
        break;
    }
    default:
        return SDL_SetError("Mudman entered an unsupported gameplay step");
    }
    return true;
}
