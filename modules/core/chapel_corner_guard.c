// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/dai/e_corner_guard.c and st/prim_helpers.h at sotn-decomp 7bc54d8157b6.
// Original animation, sensors, hitboxes and sprite parts remain private inputs.
#include "actors_internal.h"
static bool Walk(SotnActors *s, Actor *a)
{
    Uint16 floor = 0;
    SotnDataView points = {0};
    SotnCollider c = {0};
    if (!ActorWalk(s, a, 0x1b9c, &floor) ||
        !Slice(s->nz0, 0x1ba4U + (a->vx > 0 ? 4U : 0U), 4, &points) ||
        !ActorQuery(s, Wrap16((Sint64)Pixel(a->state.x) + Signed16(points.bytes)),
                    Wrap16((Sint64)Pixel(a->state.y) + Signed16(points.bytes + 2)), &c))
        return false;
    if (!(c.effects & SOTN_EFFECT_SLOPE_FLOOR))
    {
        a->state.x -= a->vx;
        a->vx = 0;
    }
    return true;
}
static bool Walking(SotnActors *s, Actor *a)
{
    const bool left = a->state.step == 5;
    Uint8 animated = 0;
    if (!a->substep)
    {
        SotnDataView interval = {0};
        if (!Slice(s->nz0, 0x1bacU + (ActorRandom(s) & 3U) * 2U, 2, &interval))
            return false;
        a->corner.timer = Read16(interval.bytes);
        if (a->corner.timer > 1024)
            return SDL_SetError("Chapel Corner Guard attack interval exceeds its bounded wait");
        a->vx = left ? -49152 : 32768;
        if (a->corner.previous_left != a->corner.movement_left)
            a->vx = left ? 32768 : -49152;
        ++a->substep;
    }
    if (!ActorAnimate(s, a, left ? 0x1bb4 : 0x1bfc, &animated) || !Walk(s, a))
        return false;
    if (a->corner.previous_left != a->corner.movement_left && Distance(a, s->player) > 96)
    {
        a->corner.movement_left = a->corner.previous_left;
        a->vx = left ? -49152 : 32768;
    }
    if (a->corner.timer)
        --a->corner.timer;
    else
    {
        if (Distance(a, s->player) < (left ? 96 : 48))
            SetStep(a, left ? 7 : 6);
        if ((Side(a, s->player) != 0) != a->corner.previous_left)
            SetStep(a, 3);
    }
    return true;
}
static bool Lunge(SotnActors *s, Actor *a)
{
    Uint8 animated = 0;
    switch (a->substep)
    {
    case 0:
        if (!ActorAnimate(s, a, 0x1bc0, &animated))
            return false;
        if (!animated)
        {
            a->substep = 1;
            a->pose = a->remaining = 0;
        }
        return true;
    case 1:
        a->vx = -196608;
        if (!ActorPositionalSound(s, a, 0x6c7) || !ActorPositionalSound(s, a, 0x616))
            return false;
        ++a->substep;
        SDL_FALLTHROUGH;
    case 2:
        if (!Walk(s, a))
            return false;
        a->vx -= a->vx / 16;
        if (!(s->service_ticks & 7U) && SDL_abs(a->vx) > 49152 &&
            !ActorOuterDust(s, a, 1, 1, 12, 8, 5, 1))
            return false;
        if (!ActorAnimate(s, a, 0x1bd4, &animated))
            return false;
        if (!animated)
            ++a->substep;
        return true;
    case 3:
        a->vx -= a->vx / 8;
        if (SDL_abs(a->vx) < 16384)
            ++a->substep;
        return true;
    case 4:
        if ((Side(a, s->player) != 0) != a->corner.previous_left)
            SetStep(a, 3);
        else
        {
            if (Distance(a, s->player) < 64)
                a->corner.movement_left = !a->corner.previous_left;
            SetStep(a, 5);
        }
        return true;
    default:
        return SDL_SetError("Chapel Corner Guard entered an invalid lunge phase");
    }
}
static bool Weapon(SotnActors *s, Actor *a)
{
    const Uint16 slot = a->enemy_state.owner_slot;
    if (slot >= SOTN_STAGE_ACTOR_SLOTS || s->slots[slot].state.id != 0x27 ||
        s->slots[slot].state.generation != a->enemy_state.owner_generation)
    {
        a->remove = true;
        return true;
    }
    if (!a->state.step && !ActorInitialize(s, a, 0x9ec))
        return false;
    const Actor *parent = &s->slots[slot];
    SotnDataView index = {0}, box = {0};
    if (parent->state.frame >= 28 || !Slice(s->nz0, 0x1c38U + parent->state.frame, 1, &index) ||
        index.bytes[0] >= 4 || !Slice(s->nz0, 0x1c28U + index.bytes[0] * 4U, 4, &box))
        return SDL_SetError("Chapel Corner Guard weapon frame or hitbox index is invalid");
    if (box.bytes[2] > 127 || box.bytes[3] > 127)
        return SDL_SetError("Chapel Corner Guard weapon has a negative private hitbox extent");
    a->state.x = parent->state.x;
    a->state.y = parent->state.y;
    a->state.facing_left = parent->state.facing_left;
    a->state.hitbox_x = box.bytes[0] < 128 ? box.bytes[0] : (Sint16)(box.bytes[0] - 256);
    a->state.hitbox_y = box.bytes[1] < 128 ? box.bytes[1] : (Sint16)(box.bytes[1] - 256);
    a->state.hitbox_width = box.bytes[2];
    a->state.hitbox_height = box.bytes[3];
    return true;
}
static bool RetireWeapon(SotnActors *s, const Actor *a)
{
    if ((Uint32)a->slot + 1U >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Chapel Corner Guard has no adjacent weapon slot");
    Actor *weapon = &s->slots[a->slot + 1U];
    if (!weapon->state.generation)
        return true;
    SotnActorPlacement placement = {0};
    Uint32 placed = 0;
    if (!SotnRoomActors_Get(s->spawns, a->slot + 1U, &placement, &placed))
        return false;
    if (weapon->state.id != 0x28 || placed || weapon->enemy_state.owner_slot != a->slot ||
        weapon->enemy_state.owner_generation != a->state.generation)
        return SDL_SetError("Chapel Corner Guard cannot retire an unrelated weapon slot");
    weapon->remove = true;
    return true;
}
static bool DeathInit(SotnActors *s, Actor *a)
{
    SotnDataView parts = {0}, palette = {0};
    if (!ActorStageParts(s, a, &parts) || !Slice(s->nz0, 0x9e6, 2, &palette))
        return false;
    const Uint16 count = Read16(parts.bytes);
    if (!ActorPrimitives(s, a, (Uint16)(count * 2U)))
    {
        // Original optional allocation failure destroys the body before weapon cleanup.
        a->remove = true;
        return true;
    }
    const Uint16 u = (a->state.texture_offset & 1U) ? 127 : 0;
    const Uint16 v = (a->state.texture_offset & 2U) ? 127 : 0;
    for (Uint16 i = 0; i < count; ++i)
    {
        const Uint8 *d = parts.bytes + 2U + i * 22U;
        ActorPrimitive *p = ActorPrimitiveGet(s, a, (Uint16)(i * 2U));
        if (!p)
            return false;
        const Sint32 w = Signed16(d + 6), h = Signed16(d + 8);
        if (w <= 0 || w > 512 || h <= 0 || h > 512)
            return SDL_SetError("Chapel Corner Guard death part dimensions are invalid");
        const Sint32 x = Wrap16((Sint64)Pixel(a->state.x) +
                                (a->state.facing_left ? -1 : 1) * (Signed16(d + 2) + w / 2));
        const Sint32 y = Wrap16((Sint64)Pixel(a->state.y) + Signed16(d + 4) + h / 2);
        p->state = (SotnActorPrimitive){.x = x * 65536,
                                        .y = y * 65536,
                                        .width = (Uint16)w,
                                        .height = (Uint16)h,
                                        .page = 18,
                                        .palette = Read16(palette.bytes),
                                        .priority_set = true,
                                        .priority = (Uint16)(a->state.priority + 2U),
                                        .active = true,
                                        .textured = true,
                                        .quad = true};
        const Uint8 left = (Uint8)(u + Read16(d + 14));
        const Uint8 right = (Uint8)(u + Read16(d + 18));
        p->state.u[0] = p->state.u[2] = a->state.facing_left ? right : left;
        p->state.u[1] = p->state.u[3] = a->state.facing_left ? left : right;
        p->state.v[0] = p->state.v[1] = (Uint8)(v + Read16(d + 16));
        p->state.v[2] = p->state.v[3] = (Uint8)(v + Read16(d + 20));
    }
    if (!RetireWeapon(s, a) || !ActorExplosionInPool(s, a, 2, 160, 192, 0) ||
        !ActorPositionalSound(s, a, 0x6ce))
        return false;
    a->state.flags |= 0x80a00000U;
    a->state.hitbox_state = a->state.frame = 0;
    ++a->substep;
    return true;
}
static bool Fragment(SotnActors *s, const Actor *a, ActorPrimitive *p)
{
    // Selected UnkPrimHelper: equal projection depth/screen distance 1024,
    // identity X/Y rotation and no scale. Move/project OLD velocity/angle first.
    const Sint64 x = (Sint64)p->state.x + p->vx, y = (Sint64)p->state.y + p->vy;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Chapel Corner Guard fragment exceeds fixed-point motion bounds");
    p->state.x = (Sint32)x;
    p->state.y = (Sint32)y;
    SotnMathTrig trig = {0};
    if (!SotnMath_MatrixTrig(s->executable, p->corner_angle, &trig))
        return false;
    for (Uint16 i = 0; i < 4; ++i)
    {
        const Sint32 dx = (i & 1U) ? p->state.width / 2 : -(Sint32)(p->state.width / 2);
        const Sint32 dy = (i & 2U) ? p->state.height / 2 : -(Sint32)(p->state.height / 2);
        p->state.offset_x[i] =
            (Sint16)ActorShift((Sint64)trig.cosine * dx - (Sint64)trig.sine * dy, 12);
        p->state.offset_y[i] =
            (Sint16)ActorShift((Sint64)trig.sine * dx + (Sint64)trig.cosine * dy, 12);
    }
    if (!p->phase)
    {
        p->vx = (Sint32)(ActorRandom(s) & 31U) * 4096 - 65536;
        p->vy = -(Sint32)(ActorRandom(s) & 31U) * 4096;
        p->timer = (Uint16)((ActorRandom(s) & 15U) + 8U);
        p->phase = 1;
    }
    p->corner_angle = (Sint16)Wrap16((Sint64)p->corner_angle + (p->vx > 0 ? -32 : 32));
    p->vy += 6144;
    if (!p->timer)
        return SDL_SetError("Chapel Corner Guard retained an expired death fragment");
    if (!--p->timer)
    {
        Actor source = *a;
        source.state.x = Pixel(p->state.x) * 65536;
        source.state.y = Pixel(p->state.y) * 65536;
        if (!ActorExplosionInPool(s, &source, 0, 160, 192, 0))
            return false;
        p->state.active = false;
    }
    return true;
}
static bool Death(SotnActors *s, Actor *a)
{
    if (!a->substep)
        return DeathInit(s, a);
    bool active = false;
    for (Uint16 i = 0; i < a->state.primitive_count; i = (Uint16)(i + 2U))
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        if (!p->state.active)
            continue;
        active = true;
        if (!Fragment(s, a, p))
            return false;
    }
    a->remove = !active;
    return true;
}
bool ActorChapelCornerGuard(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 6 || (a->state.id != 0x27 && a->state.id != 0x28))
        return SDL_SetError("Chapel Corner Guard requires its original stage identity");
    if (a->state.id == 0x28)
        return Weapon(s, a);
    if ((a->state.flags & 0x100U) && a->state.step < 8)
        SetStep(a, 8);
    Uint8 animated = 0;
    Uint16 collision = 0;
    switch (a->state.step)
    {
    case 0: {
        if (!ActorInitialize(s, a, 0x9e0))
            return false;
        a->state.frame = 1;
        Actor *child = NULL;
        if (!ActorOwnedSpawnAt(s, a, 0x28, 0, (Uint32)a->slot + 1U, &child))
            return false;
        child->state.x = Pixel(a->state.x) * 65536;
        child->state.y = Pixel(a->state.y) * 65536;
        child->state.facing_left = false;
        SDL_FALLTHROUGH;
    }
    case 1:
        if (!ActorGround(s, a, 0x1b8c, &collision))
            return false;
        if (collision & 1U)
        {
            a->corner.previous_left = a->corner.movement_left = (ActorRandom(s) & 1U) != 0;
            SetStep(a, 2);
        }
        break;
    case 2:
        a->state.frame = 1;
        if (Distance(a, s->player) < 96)
            SetStep(a, (Side(a, s->player) != 0) != a->corner.previous_left ? 3
                       : a->corner.previous_left                            ? 5
                                                                            : 4);
        break;
    case 3:
        if (!ActorAnimate(s, a, 0x1bec, &animated))
            return false;
        if (!animated)
        {
            a->corner.previous_left = a->corner.movement_left = Side(a, s->player) != 0;
            SetStep(a, a->corner.previous_left ? 5 : 4);
        }
        break;
    case 4:
    case 5:
        if (!Walking(s, a))
            return false;
        break;
    case 6:
        if (!a->substep)
        {
            if (!ActorPositionalSound(s, a, 0x66d))
                return false;
            ++a->substep;
        }
        if (!ActorAnimate(s, a, 0x1c08, &animated))
            return false;
        if (!animated)
            SetStep(a, (Side(a, s->player) != 0) != a->corner.previous_left ? 3 : 4);
        break;
    case 7:
        if (!Lunge(s, a))
            return false;
        break;
    case 8:
        if (!Death(s, a))
            return false;
        break;
    default:
        return SDL_SetError("Chapel Corner Guard entered an invalid original state");
    }
    const bool crouched = a->state.frame >= 10 && a->state.frame < 14;
    a->state.hitbox_width = crouched ? 17 : 7;
    a->state.hitbox_height = crouched ? 9 : 22;
    a->state.hitbox_x = crouched ? -6 : 0;
    a->state.hitbox_y = crouched ? 12 : 2;
    return true;
}
