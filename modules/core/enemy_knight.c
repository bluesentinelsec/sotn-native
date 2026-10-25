// SPDX-License-Identifier: AGPL-3.0-or-later
// Green Axe Knight and its thrown axes from st/entity_axe_knight.h, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Tables/part art remain private inputs.
#include "actors_internal.h"
bool ActorKnightShatter(SotnActors *s, Actor *a, bool *done)
{
    *done = false;
    if (a->substep == 0)
    {
        SotnDataView parts = {0};
        if (!ActorStageParts(s, a, &parts))
            return false;
        const Uint16 count = Read16(parts.bytes);
        Uint16 available = 0;
        for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
            available = (Uint16)(available + (s->primitives[i].owner == 0));
        if (available < count * 2U)
        {
            *done = true;
            return true;
        }
        if (!ActorPrimitives(s, a, (Uint16)(count * 2U)))
            return false;
        for (Uint16 i = 0; i < count; ++i)
        {
            const Uint8 *d = parts.bytes + 2 + i * 22;
            ActorPrimitive *p = ActorPrimitiveGet(s, a, (Uint16)(i * 2U));
            if (p == NULL)
                return false;
            const Sint32 w = Signed16(d + 6), h = Signed16(d + 8);
            if (w <= 0 || w > 512 || h <= 0 || h > 512)
                return SDL_SetError("Axe Knight death part dimensions are invalid");
            const Sint32 x =
                Pixel(a->state.x) + (a->state.facing_left ? -1 : 1) * (Signed16(d + 2) + w / 2);
            const Sint32 y = Pixel(a->state.y) + Signed16(d + 4) + h / 2;
            const Uint16 texture = (Uint16)(a->state.texture_offset + Read16(d + 12));
            const Uint8 u = (texture & 1) ? 127 : 0, v = (texture & 2) ? 127 : 0;
            p->state = (SotnActorPrimitive){.x = x * 65536,
                                            .y = y * 65536,
                                            .width = (Uint16)w,
                                            .height = (Uint16)h,
                                            .page = (Uint16)(texture / 4),
                                            .palette = (Uint16)(a->state.palette + Read16(d + 10)),
                                            .textured = true,
                                            .active = true,
                                            .red = 128,
                                            .green = 128,
                                            .blue = 128};
            const Uint8 left = (Uint8)(u + Read16(d + 14)), right = (Uint8)(u + Read16(d + 18));
            p->state.u[0] = p->state.u[2] = a->state.facing_left ? right : left;
            p->state.u[1] = p->state.u[3] = a->state.facing_left ? left : right;
            p->state.v[0] = p->state.v[1] = (Uint8)(v + Read16(d + 16));
            p->state.v[2] = p->state.v[3] = (Uint8)(v + Read16(d + 20));
        }
        if (!ActorSound(s, 0x65b))
            return false;
        ++a->substep;
        return true;
    }
    bool active = false;
    for (Uint16 i = 0; i < a->state.primitive_count; i = (Uint16)(i + 2))
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (p == NULL)
            return false;
        if (!p->state.active)
            continue;
        active = true;
        p->state.x += p->vx;
        p->state.y += p->vy;
        if (p->phase == 0)
        {
            p->vx = 0;
            p->vy = -65536;
            p->timer = 256;
            p->phase = 1;
            continue;
        }
        SotnCollider c = {0};
        if (!ActorQuery(s, Pixel(p->state.x), Pixel(p->state.y) + p->state.height / 3, &c))
            return false;
        if (c.effects & SOTN_EFFECT_SOLID)
        {
            p->state.y += c.top * 65536;
            if (p->vy < 16384)
                p->timer = 1;
            p->vy = -p->vy;
            p->vy -= p->vy / 2;
        }
        p->vy += 6144;
        if (--p->timer == 0)
        {
            Actor origin = *a;
            origin.state.x = p->state.x;
            origin.state.y = p->state.y;
            if (!ActorExplosionInPool(s, &origin, 0, 160, 192, 0) || !ActorSound(s, 0x63b))
                return false;
            p->state.active = false;
        }
    }
    *done = !active;
    return true;
}
static bool Axe(SotnActors *s, Actor *a)
{
    if (a->state.flags & 0x100)
    {
        if (!ActorSound(s, 0x66b) || !ActorExplosion(s, a, 0))
            return false;
        a->remove = true;
        return true;
    }
    if (a->state.step == 0)
    {
        if (a->state.parameters > 2)
            return SDL_SetError("Axe Knight projectile type exceeds its table");
        if (!ActorInitialize(s, a, ActorStageOffset(s, 0xc70)))
            return false;
        SotnDataView vx = {0}, vy = {0};
        if (!Slice(s->nz0, ActorStageOffset(s, 0x22bc) + a->state.parameters * 4, 4, &vx) ||
            !Slice(s->nz0, ActorStageOffset(s, 0x22c8) + a->state.parameters * 4, 4, &vy))
            return false;
        a->vx = Signed32(vx.bytes) * (a->state.facing_left ? 1 : -1);
        a->vy = Signed32(vy.bytes);
        a->state.rotated = true;
        a->fall_speed = -64;
        if (a->state.parameters == 2)
            ++a->state.step;
        return true;
    }
    const Sint32 turn = a->state.parameters ? 128 : -128;
    a->state.rotation = (Sint16)((a->state.rotation + turn) & 4095);
    if (a->state.step == 1)
    {
        if (a->fall_speed >= 0 && a->fall_speed < 32)
            a->vx += a->state.facing_left ? -8192 : 8192;
        ++a->fall_speed;
    }
    else if (a->state.step == 2)
        a->vy += 8192;
    else
        return SDL_SetError("Axe Knight projectile entered an invalid state");
    a->state.x += a->vx;
    a->state.y += a->vy;
    return true;
}
bool ActorKnight(SotnActors *s, Actor *a, SotnPlayerState player)
{
    if (a->state.id == ActorStageId(s, 0x2a))
        return Axe(s, a);
    Uint8 anim = 0;
    Uint16 collision = 0;
    if ((a->state.flags & 0x100) && a->state.step != 8)
    {
        if (!ActorSound(s, 0x767))
            return false;
        a->state.hitbox_state = 0;
        a->state.priority = (Uint16)(a->state.priority - 16);
        a->enemy_state.counter = 65;
        SetStep(a, 8);
    }
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, ActorStageOffset(s, 0xc64)))
            return false;
        a->state.facing_left = Side(a, player) == 0;
        a->state.hitbox_y = 10;
        a->enemy_state.attack_timer = 512;
        /* fall through */
    case 1:
        if (!ActorGround(s, a, ActorStageOffset(s, 0x2188), &collision))
            return false;
        if (collision & 1)
        {
            a->state.facing_left = Side(a, player) == 0;
            SetStep(a, 2);
        }
        break;
    case 2:
    case 3: {
        const bool back = a->state.step == 3;
        if (a->substep == 0)
        {
            a->vx = (a->state.facing_left ? 1 : -1) * (back ? -12288 : 12288);
            ++a->substep;
        }
        if (!ActorAnimate(s, a, ActorStageOffset(s, 0x2210), &anim))
            return false;
        if (a->remaining == 0)
            a->state.facing_left = Side(a, player) == 0;
        if (anim == 0)
        {
            a->vx = (a->state.facing_left ? 1 : -1) * (back ? -12288 : 12288);
            if ((!back && Distance(a, player) < 96) || (back && Distance(a, player) > 80))
            {
                SetStep(a, back ? 2 : 3);
                a->enemy_state.movement_facing = !back;
            }
        }
        Sint32 delta = (a->pose == 1 || a->pose == 4) ? 1 : -1;
        delta *= a->state.facing_left ? 1 : -1;
        delta *= back ? -512 : 768;
        a->vx += delta;
        if (!ActorWalk(s, a, ActorStageOffset(s, 0x2180), &collision))
            return false;
        if (collision & 0x60)
        {
            a->state.x -= a->vx;
            a->vx = 0;
        }
        if (a->enemy_state.attack_timer > 0)
            a->enemy_state.attack_timer =
                (Uint16)(a->enemy_state.attack_timer >= 3 ? a->enemy_state.attack_timer - 3 : 0);
        else
        {
            SotnDataView step = {0};
            const Uint32 index = ActorRandom(s) & 7;
            if (!Slice(s->nz0, ActorStageOffset(s, 0x22b4) + index, 1, &step))
                return false;
            if (step.bytes[0] != 4 && step.bytes[0] != 5)
                return SDL_SetError("Axe Knight attack table contains an unsupported state");
            SetStep(a, step.bytes[0]);
            a->enemy_state.attack_timer = 256;
        }
        break;
    }
    case 4:
    case 5:
    case 7: {
        const Uint16 phase = a->state.step;
        if (!ActorAnimate(s, a, ActorStageOffset(s, phase == 5 ? 0x2220 : 0x2244), &anim))
            return false;
        if (anim == 0)
        {
            const bool back = Distance(a, player) <= 88;
            SetStep(a, back ? 3 : 2);
            a->enemy_state.movement_facing = back;
        }
        else if ((anim & 0x80) && a->pose == (phase == 5 ? 6 : 7))
        {
            if (!ActorSound(s, 0x766))
                return false;
            Actor *projectile = NULL;
            if (!ActorSpawn(s, a, ActorStageId(s, 0x2a),
                            phase == 5   ? 1
                            : phase == 7 ? 2
                                         : 0,
                            96, 128, &projectile))
                return false;
            if (projectile != NULL)
            {
                projectile->state.x +=
                    (a->state.facing_left ? 1 : -1) * (phase == 7 ? 16 : 8) * 65536;
                projectile->state.y += (phase == 5 ? 12 : phase == 7 ? -40 : -12) * 65536;
            }
        }
        break;
    }
    case 8:
        if (a->enemy_state.counter != 0)
        {
            const Uint16 tick = --a->enemy_state.counter;
            if ((tick & 7) == 0)
            {
                SotnDataView offset = {0};
                Actor origin = *a;
                if (!Slice(s->nz0, ActorStageOffset(s, 0x2198) + (tick >> 3) * 4, 4, &offset))
                    return false;
                origin.state.x += Signed16(offset.bytes) * 65536;
                origin.state.y += Signed16(offset.bytes + 2) * 65536;
                if (!ActorExplosion(s, &origin, 2))
                    return false;
            }
        }
        if (!ActorAnimate(s, a, ActorStageOffset(s, 0x2268), &anim))
            return false;
        if (anim == 0)
        {
            bool done = false;
            if (!ActorKnightShatter(s, a, &done))
                return false;
            if (done)
            {
                a->remove = true;
                return true;
            }
            if (a->substep < 16)
                a->state.frame = 0;
        }
        break;
    default:
        return SDL_SetError("Axe Knight entered an invalid original state");
    }
    if (a->state.frame >= 48)
        return SDL_SetError("Axe Knight frame exceeds its hitbox lookup");
    SotnDataView index = {0}, box = {0};
    if (!Slice(s->nz0, ActorStageOffset(s, 0x2284) + a->state.frame, 1, &index) ||
        index.bytes[0] > 3 ||
        !Slice(s->nz0, ActorStageOffset(s, 0x2274) + index.bytes[0] * 4, 4, &box))
        return false;
    a->state.hitbox_x = (Sint16)(box.bytes[0] < 128 ? box.bytes[0] : (Sint32)box.bytes[0] - 256);
    a->state.hitbox_y = (Sint16)(box.bytes[1] < 128 ? box.bytes[1] : (Sint32)box.bytes[1] - 256);
    a->state.hitbox_width = box.bytes[2];
    a->state.hitbox_height = box.bytes[3];
    return true;
}
