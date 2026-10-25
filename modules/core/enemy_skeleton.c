// SPDX-License-Identifier: AGPL-3.0-or-later
// Skeleton, Bone Scimitar, Blood Skeleton, bones and death parts from
// st/e_skeleton.h, e_bone_scimitar.h, e_blood_skeleton.h at sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. All arrays remain private NZ0 data.
#include "actors_internal.h"
static bool Read(SotnActors *s, size_t offset, size_t count, SotnDataView *out)
{
    return Slice(s->nz0, offset, count, out);
}
static bool Pieces(SotnActors *s, Actor *a, bool scimitar)
{
    const Uint16 count = scimitar ? 7 : 6;
    if (!ActorSound(s, 0x62b))
        return false;
    for (Uint16 i = 0; i < count; ++i)
    {
        Actor *p = NULL;
        SotnDataView t = {0}, x = {0}, y = {0}, vx = {0}, vy = {0};
        if (!ActorSpawn(s, a, scimitar ? (s->stage_id == 65 ? 0x47 : 0x28) : 0x30,
                        (Uint16)(i | (scimitar ? a->state.parameters << 8 : 0)), 160, 192, &p))
            return false;
        if (p == NULL)
            break;
        if (!Read(s, (scimitar ? ActorTable(s, 0x20f4, 0x3bb8) : 0x2430) + i, 1, &t) ||
            !Read(s, (scimitar ? ActorTable(s, 0x2134, 0x3bf8) : 0x2468) + i * 2, 2, &x) ||
            !Read(s, (scimitar ? ActorTable(s, 0x2144, 0x3c08) : 0x2474) + i * 2, 2, &y) ||
            !Read(s, (scimitar ? ActorTable(s, 0x20fc, 0x3bc0) : 0x2438) + i * 4, 4, &vx) ||
            !Read(s, (scimitar ? ActorTable(s, 0x2118, 0x3bdc) : 0x2450) + i * 4, 4, &vy))
            return false;
        p->timer = t.bytes[0];
        p->vx = Signed32(vx.bytes);
        p->vy = Signed32(vy.bytes);
        p->state.x += (a->state.facing_left ? -Signed16(x.bytes) : Signed16(x.bytes)) * 65536;
        p->state.y += Signed16(y.bytes) * 65536;
    }
    a->remove = true;
    return true;
}
static bool Part(SotnActors *s, Actor *a, bool scimitar)
{
    const Uint16 id = a->state.parameters & 255;
    if (id >= (scimitar ? 7 : 6))
        return SDL_SetError("Skeleton piece index exceeds its private definition");
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, scimitar ? ActorTable(s, 0xc58, 0xb84) : 0xc94))
            return false;
        a->state.frame = (Uint16)(id + (scimitar ? 16 : 15));
        if (scimitar)
            a->state.palette = (Uint16)(a->state.palette + (a->state.parameters >> 8));
        a->state.rotated = true;
        if (a->state.facing_left)
            a->vx = -a->vx;
        return true;
    }
    if (a->timer == 0)
        return SDL_SetError("Skeleton piece has no destruction timer");
    if (--a->timer == 0)
    {
        a->state.id = 2;
        a->state.parameters = 0;
        a->state.step = 0;
        a->state.rotated = false;
        return true;
    }
    SotnDataView rotation = {0};
    if (!Read(s,
              (scimitar ? ActorTable(s, 0x20e4, 0x3ba8) : 0x2424) +
                  (scimitar && s->stage_id == 65 ? a->state.parameters : id) * 2U,
              2, &rotation))
        return false;
    a->state.rotation = (Sint16)((a->state.rotation + Read16(rotation.bytes)) & 4095);
    if (a->vy < 6 * 65536)
        a->vy += 16384;
    a->state.x += a->vx;
    a->state.y += a->vy;
    return true;
}
static bool Bone(SotnActors *s, Actor *a, SotnPlayerState player)
{
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, 0xca0))
            return false;
        a->state.y -= 4096;
        const Uint32 index = (Uint32)SDL_min(7LL, Distance(a, player) / 32);
        SotnDataView velocity = {0};
        if (!Read(s, 0x2488 + index * 4, 4, &velocity))
            return false;
        a->vx = Signed32(velocity.bytes);
        if (a->state.facing_left)
            a->vx = -a->vx;
        a->vy = -294912;
        a->state.rotated = true;
        return true;
    }
    if (a->state.flags & 0x100)
    {
        if (!ActorExplosion(s, a, 0))
            return false;
        a->remove = true;
        return true;
    }
    a->state.rotation = (Sint16)((a->state.rotation + 128) & 4095);
    a->vy += 9216;
    a->state.x += a->vx;
    a->state.y += a->vy;
    if (Pixel(a->state.y) - s->camera_y > 240)
        a->remove = true;
    return true;
}
static bool WalkCheck(SotnActors *s, Actor *a, SotnPlayerState player, bool scimitar)
{
    Uint16 ground = 0, field = 0;
    if (!ActorWalk(s, a, scimitar ? ActorTable(s, 0x216c, 0x3c30) : 0x24b8, &ground))
        return false;
    if (!scimitar && (ground & 0x60) == 0x60)
        a->state.x -= a->vx;
    if (!ActorField(s, a, scimitar ? ActorTable(s, 0x2174, 0x3c38) : 0x24c0, 3, false, &field))
        return false;
    if ((scimitar && ground == 0x80) || (field & 2))
    {
        SetStep(a, 5);
        return true;
    }
    if (scimitar)
    {
        if (a->enemy_state.attack_timer == 0)
        {
            if (Distance(a, player) < 64 && (a->state.facing_left != (Side(a, player) != 0)))
                SetStep(a, 4);
        }
        else
            --a->enemy_state.attack_timer;
    }
    else if (--a->enemy_state.attack_timer == 0)
        SetStep(a, 4);
    return true;
}
static bool Skeleton(SotnActors *s, Actor *a, SotnPlayerState player, bool scimitar)
{
    Uint8 anim = 0;
    Uint16 collision = 0;
    if (a->state.flags & 0x100)
        a->state.step = scimitar ? 7 : 6;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, scimitar ? ActorTable(s, 0xc4c, 0xb78) : 0xc88))
            return false;
        if (scimitar && a->state.parameters != 0)
        {
            if (s->stage_id != 65 || a->state.parameters > 2)
                return SDL_SetError("Bone Scimitar has an invalid special parameter");
            a->state.palette = (Uint16)(a->state.palette + a->state.parameters);
            a->state.flags &= ~(0xc0000c00U);
            a->enemy_state.initial_x = Pixel(a->state.x);
            if (s->entrance_scimitars & a->state.parameters)
                a->remove = true;
        }
        a->enemy_state.attack_timer = 80;
        break;
    case 1:
        if (!ActorGround(s, a, scimitar ? ActorTable(s, 0x215c, 0x3c20) : 0x24a8, &collision))
            return false;
        if (collision != 0)
            SetStep(a, scimitar && a->state.parameters ? 6 : 2);
        break;
    case 2:
    case 3:
        if (!scimitar)
            a->state.facing_left = Side(a, player) == 0;
        if (!ActorAnimate(s, a,
                          scimitar ? (a->state.step == 2 ? ActorTable(s, 0x2090, 0x3b54)
                                                         : ActorTable(s, 0x20a0, 0x3b64))
                                   : (a->state.step == 2 ? 0x23dc : 0x23ec),
                          &anim))
            return false;
        if (scimitar && anim == 0)
            a->state.facing_left = Side(a, player) == 0;
        a->enemy_state.movement_facing = a->state.facing_left != (a->state.step == 3);
        a->vx = a->enemy_state.movement_facing ? 32768 : -32768;
        if (a->state.step == 2 && Distance(a, player) < 76)
            a->state.step = 3;
        else if (a->state.step == 3 && Distance(a, player) > 92)
            a->state.step = 2;
        if (!WalkCheck(s, a, player, scimitar))
            return false;
        break;
    case 4:
        if (!ActorAnimate(s, a, scimitar ? ActorTable(s, 0x20b0, 0x3b74) : 0x23fc, &anim))
            return false;
        if (scimitar)
        {
            if (a->state.frame == 12)
            {
                a->state.hitbox_width = 20;
                a->state.hitbox_height = 17;
                a->state.hitbox_x = -11;
                a->state.hitbox_y = -14;
            }
            else
            {
                a->state.hitbox_width = 8;
                a->state.hitbox_height = 18;
                a->state.hitbox_x = -1;
                a->state.hitbox_y = 0;
            }
            if (a->pose == 7 && a->remaining == 0 && !ActorSound(s, 0x66d))
                return false;
        }
        if (anim == 0)
        {
            SetStep(a, scimitar && a->state.parameters ? 6 : 3);
            a->enemy_state.attack_index = (Uint16)((a->enemy_state.attack_index + 1) & 3);
            SotnDataView timer = {0};
            const size_t index =
                (size_t)(a->state.parameters & 1) * 4 + a->enemy_state.attack_index;
            if (!Read(s, (scimitar ? ActorTable(s, 0x2154, 0x3c18) : 0x2480) + index, 1, &timer))
                return false;
            a->enemy_state.attack_timer = timer.bytes[0];
        }
        else if (!scimitar && (anim & 0x80) && a->state.frame == 10)
        {
            const Sint32 x = Pixel(a->state.x) - s->camera_x, y = Pixel(a->state.y) - s->camera_y;
            if (x >= -16 && x <= 272 && y >= 0 && y <= 240)
            {
                Actor *bone = NULL;
                if (!ActorSpawn(s, a, 0x2f, 0, 96, 128, &bone))
                    return false;
                if (bone != NULL)
                {
                    if (!ActorSound(s, 0x6c8))
                        return false;
                    bone->state.x += (a->state.facing_left ? -8 : 8) * 65536;
                    bone->state.y -= 16 * 65536;
                }
            }
        }
        break;
    case 5:
        if (a->substep == 0)
        {
            if (!ActorAnimate(s, a, scimitar ? ActorTable(s, 0x20cc, 0x3b90) : 0x240c, &anim))
                return false;
            if (!(anim & 1))
            {
                bool right = a->enemy_state.movement_facing;
                if ((ActorRandom(s) & 3) == 0)
                    right = !right;
                a->vx = right ? 2 * 65536 : -2 * 65536;
                a->vy = -3 * 65536;
                a->pose = a->remaining = 0;
                ++a->substep;
            }
        }
        else if (a->substep == 1)
        {
            if (!ActorGround(s, a, scimitar ? ActorTable(s, 0x215c, 0x3c20) : 0x24a8, &collision))
                return false;
            if (collision != 0)
                ++a->substep;
            if (!ActorField(s, a, scimitar ? ActorTable(s, 0x2174, 0x3c38) : 0x24c0, 2, true,
                            &collision))
                return false;
        }
        else if (a->substep == 2)
        {
            if (!ActorAnimate(s, a, scimitar ? ActorTable(s, 0x20d8, 0x3b9c) : 0x2418, &anim))
                return false;
            // The selected US Skeleton path intentionally lands on a nonzero animation result.
            if (scimitar ? anim == 0 : (anim & 1) != 0)
                SetStep(a, 3);
        }
        else
            return SDL_SetError("Skeleton jump substate is invalid");
        break;
    case 6:
        if (scimitar)
        {
            a->state.facing_left = Side(a, player) == 0;
            const bool forward = (a->vx < 0) != a->state.facing_left;
            if (!ActorWalk(s, a, ActorTable(s, 0x216c, 0x3c30), &collision) ||
                !ActorAnimate(
                    s, a, forward ? ActorTable(s, 0x2090, 0x3b54) : ActorTable(s, 0x20a0, 0x3b64),
                    &anim))
                return false;
            a->vx = a->substep == 0 ? 32768 : -32768;
            const Sint32 distance = Pixel(a->state.x) - a->enemy_state.initial_x;
            if (a->substep == 0 && distance > 32)
                a->substep = 1;
            else if (a->substep == 1 && distance < -32)
                a->substep = 0;
            if (a->enemy_state.attack_timer)
                --a->enemy_state.attack_timer;
            else if (Distance(a, player) < 48 && SDL_abs(Pixel(a->state.y) - Pixel(player.y)) < 32)
                SetStep(a, 4);
            return true;
        }
        return Pieces(s, a, false);
    case 7:
        if (a->state.parameters)
        {
            Actor *drop = NULL;
            if (!ActorSpawnAt(s, a, 0xa, (Uint16)(((a->state.parameters & 1) ? 19 : 26) | 0x8000),
                              (Uint32)a->slot + 1, &drop))
                return false;
            s->entrance_scimitars |= a->state.parameters;
        }
        return Pieces(s, a, true);
    default:
        return SDL_SetError("Skeleton entered an invalid original state");
    }
    return true;
}
static bool Blood(SotnActors *s, Actor *a, SotnPlayerState player)
{
    Uint8 anim = 0;
    Uint16 ground = 0;
    if ((a->state.flags & 0x100) && a->state.step < 3)
    {
        if (!ActorSound(s, 0x6cb))
            return false;
        a->state.hitbox_state = 0;
        SetStep(a, 3);
    }
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0xc40))
            return false;
        a->state.facing_left = (ActorRandom(s) & 1) != 0;
        a->state.frame = 1;
        a->state.flags &= ~(0x80000000U | 0x40000000U | 0x20000000U);
        a->state.palette = (Uint16)(a->state.palette + a->state.parameters);
        break;
    case 1:
        if (!ActorGround(s, a, 0x2694, &ground))
            return false;
        if (ground & 1)
        {
            a->substep = 0;
            ++a->state.step;
        }
        break;
    case 2: {
        if (a->remaining == 0)
        {
            if (a->pose >= 10)
                return SDL_SetError("Blood Skeleton walk offset exceeds its private table");
            SotnDataView motion = {0};
            if (!Read(s, 0x2624 + a->pose * 2, 2, &motion))
                return false;
            const Sint32 dx = Signed16(motion.bytes) * (a->state.facing_left ? 1 : -1);
            a->state.x += dx * 65536;
        }
        if (!ActorAnimate(s, a, 0x2610, &anim))
            return false;
        if (anim == 0 && SDL_abs(Pixel(a->state.y) - Pixel(player.y)) < 48 &&
            (ActorRandom(s) & 3) == 0)
            a->state.facing_left = Side(a, player) == 0;
        Uint8 flags = 0;
        for (Uint32 i = 0; i < 2; ++i)
        {
            SotnDataView offsets = {0};
            SotnCollider c = {0};
            if (!Read(s, 0x26ac + i * 4, 4, &offsets) ||
                !ActorQuery(s,
                            Pixel(a->state.x) +
                                (a->state.facing_left ? 1 : -1) * Signed16(offsets.bytes),
                            Pixel(a->state.y) + Signed16(offsets.bytes + 2), &c))
                return false;
            flags = (Uint8)((flags << 1) | ((c.effects & SOTN_EFFECT_SOLID) ? 1 : 0));
        }
        if (flags != 2)
            a->state.facing_left = !a->state.facing_left;
        break;
    }
    case 3:
        if (!ActorAnimate(s, a, 0x2638, &anim))
            return false;
        if (anim == 0)
        {
            a->state.flags &= ~0x100U;
            a->enemy_state.timer = a->state.parameters ? 4 : 240;
            SetStep(a, 4);
        }
        break;
    case 4:
        if (a->substep == 0)
        {
            if (a->enemy_state.timer == 0)
                return SDL_SetError("Blood Skeleton recovery timer underflow");
            if (--a->enemy_state.timer == 0)
            {
                a->state.rotated = true;
                a->state.rotation = 0;
                if (!ActorSound(s, 0x6cc))
                    return false;
                ++a->substep;
                return true;
            }
        }
        else if (a->substep == 1)
        {
            if (s->ticks % 3 == 0)
            {
                ++a->enemy_state.timer;
                a->state.rotation = (Sint16)(a->enemy_state.timer % 2 ? 16 : -16);
            }
            if (a->enemy_state.timer > 8)
            {
                a->state.rotated = false;
                a->state.rotation = 0;
                ++a->substep;
            }
        }
        else if (a->substep == 2)
        {
            if (!ActorAnimate(s, a, a->state.parameters ? 0x2670 : 0x2654, &anim))
                return false;
            if (anim == 0)
            {
                Actor reset = {0};
                if (!ActorInitialize(s, &reset, 0xc40))
                    return false;
                a->state.hit_points = 0;
                a->state.hitbox_state = 3;
                a->state.flags = reset.state.flags & ~(0x80000000U | 0x40000000U | 0x20000000U);
                SetStep(a, 2);
            }
        }
        else
            return SDL_SetError("Blood Skeleton recovery substate is invalid");
        break;
    default:
        return SDL_SetError("Blood Skeleton entered an invalid state");
    }
    return true;
}
bool ActorSkeleton(SotnActors *s, Actor *a, SotnPlayerState player)
{
    if (s->stage_id == 65)
        return a->state.id == 0x46 ? Skeleton(s, a, player, true) : Part(s, a, true);
    switch (a->state.id)
    {
    case 0x26:
        return Blood(s, a, player);
    case 0x27:
        return Skeleton(s, a, player, true);
    case 0x28:
        return Part(s, a, true);
    case 0x2e:
        return Skeleton(s, a, player, false);
    case 0x2f:
        return Bone(s, a, player);
    case 0x30:
        return Part(s, a, false);
    default:
        return SDL_SetError("Skeleton dispatcher received an unrelated actor");
    }
}
