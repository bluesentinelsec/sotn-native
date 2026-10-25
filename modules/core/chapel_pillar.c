// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/dai/e_bone_pillar.c at sotn-decomp 7bc54d8157b6.
// Original animation, sensors, angles and sprite UVs remain private runtime inputs.
#include "actors_internal.h"
static bool Move(Actor *a)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Chapel Bone Pillar exceeds fixed-point motion bounds");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    return true;
}
static bool Spawn(SotnActors *s, const Actor *a, Uint16 id, Uint16 parameters, Sint16 x, Sint16 y,
                  Actor **out)
{
    if (!ActorSpawn(s, a, id, parameters, 160, 192, out))
        return false;
    if (*out)
    {
        (*out)->state.x = Wrap16((Sint64)Pixel(a->state.x) + x) * 65536;
        (*out)->state.y = Wrap16((Sint64)Pixel(a->state.y) + y) * 65536;
        (*out)->state.facing_left = false;
    }
    return true;
}
static bool SkullDeath(SotnActors *s, Actor *a)
{
    const Uint16 start = a->state.parameters ? 0 : 2;
    for (Uint16 i = 0; i < 2; ++i)
    {
        SotnDataView offsets = {0};
        Actor *part = NULL;
        if (!Slice(s->nz0, 0x1c8cU + (start + i) * 4U, 4, &offsets) ||
            !Spawn(s, a, 0x2b, (Uint16)(start + i), (Sint16)Signed16(offsets.bytes),
                   (Sint16)Signed16(offsets.bytes + 2), &part))
            return false;
    }
    if (!ActorExplosionInPool(s, a, 2, 160, 192, 0) || !ActorPositionalSound(s, a, 0x683))
        return false;
    if (!a->state.parameters)
    {
        if ((Uint32)a->slot + 1U >= SOTN_STAGE_ACTOR_SLOTS)
            return SDL_SetError("Chapel Bone Pillar base has no adjacent child slot");
        Actor *child = &s->slots[a->slot + 1U];
        if ((child->state.id == 0x29 || child->state.id == 0x2c) &&
            child->enemy_state.owner_slot == a->slot &&
            child->enemy_state.owner_generation == a->state.generation)
            child->pillar.bottom_dead = true;
    }
    a->remove = true;
    return true;
}
static bool Bounce(SotnActors *s, Actor *a)
{
    if (!a->substep)
    {
        a->state.frame = 24;
        a->state.hitbox_x = a->state.hitbox_y = 0;
        a->state.rotated = true;
        a->vx = -49152;
        a->vy = -65536;
        a->state.flags = (a->state.flags & ~0x20000000U) | 0x02000000U;
        ++a->substep;
    }
    a->state.rotation = (Sint16)Wrap16((Sint64)a->state.rotation - 32);
    if (!Move(a))
        return false;
    a->vy += 12288;
    for (Uint16 i = 0; i < 4; ++i)
    {
        SotnDataView angle = {0};
        SotnMathTrig trig = {0};
        SotnCollider c = {0};
        if (!Slice(s->nz0, 0x1ca4U + i * 2U, 2, &angle) ||
            !SotnMath_Trig(s->executable, Wrap16((Sint64)a->state.rotation + Signed16(angle.bytes)),
                           &trig) ||
            !ActorQuery(s, Pixel(a->state.x) + ActorShift((Sint64)trig.cosine * 14, 12),
                        Pixel(a->state.y) + ActorShift((Sint64)trig.sine * 14, 12), &c))
            return false;
        if (c.effects & SOTN_EFFECT_SOLID)
        {
            if (!ActorPositionalSound(s, a, 0x6a6))
                return false;
            ShiftY(a, c.top);
            a->vy = -a->vy;
            a->vy -= a->vy / 2;
            break;
        }
    }
    if (a->state.flags & 0x100U)
    {
        if (!ActorPositionalSound(s, a, 0x683))
            return false;
        a->state.hitbox_state = 0;
        a->state.rotated = false;
        // Original callback conversion retains the current frame until explosion init.
        a->state.id = 2;
        a->state.step = 0;
        a->state.parameters = 1;
    }
    return true;
}
static bool Skull(SotnActors *s, Actor *a)
{
    if ((a->state.flags & 0x100U) && a->state.step < 8)
        SetStep(a, 8);
    if (a->pillar.bottom_dead)
    {
        a->pillar.bottom_dead = false;
        SetStep(a, 10);
    }
    SotnDataView palette = {0};
    if (!Slice(s->nz0, 0x9fe, 2, &palette))
        return false;
    const Uint16 base = Read16(palette.bytes);
    Uint8 animated = 0;
    Uint16 ground = 0;
    switch (a->state.step)
    {
    case 0:
        if (a->state.parameters != 0 && a->state.parameters != 1 && a->state.parameters != 256)
            return SDL_SetError("Chapel Bone Pillar skull has unsupported initial parameters");
        if (!ActorInitialize(s, a, 0x9f8))
            return false;
        a->state.palette = (Uint16)(base + 1U);
        a->state.frame = 1;
        a->state.flags |= 0x20000000U;
        if (a->state.parameters & 255U)
        {
            a->state.hitbox_x = -1;
            a->state.hitbox_y = -14;
            a->state.frame = 9;
            SetStep(a, 2);
            return true;
        }
        a->state.hitbox_x = -3;
        a->state.hitbox_y = 13;
        SDL_FALLTHROUGH;
    case 1:
        if (!ActorGround(s, a, 0x1c54, &ground))
            return false;
        if (ground & 1U)
        {
            const bool ball = (a->state.parameters & 256U) != 0;
            Actor *child = NULL;
            if (!ActorOwnedSpawnAt(s, a, ball ? 0x2c : 0x29, ball ? 0 : 1, (Uint32)a->slot + 1U,
                                   &child))
                return false;
            child->state.x = Wrap16((Sint64)Pixel(a->state.x) + (ball ? 8 : 0)) * 65536;
            child->state.y = Wrap16((Sint64)Pixel(a->state.y) - (ball ? 8 : 0)) * 65536;
            child->state.facing_left = false;
            if (ball)
                a->state.parameters = 0;
            SetStep(a, 2);
        }
        return true;
    case 2:
        if (Distance(a, s->player) < 112)
            SetStep(a, 3);
        return true;
    case 3:
        if (!a->substep)
        {
            SotnDataView interval = {0};
            if (a->pillar.interval >= 8)
                return SDL_SetError("Chapel Bone Pillar interval index exceeds its private cycle");
            if (!Slice(s->nz0, 0x1d30U + a->pillar.interval * 2U, 2, &interval))
                return false;
            a->pillar.timer = Read16(interval.bytes);
            if (!a->pillar.timer || a->pillar.timer > 1024)
                return SDL_SetError("Chapel Bone Pillar attack wait exceeds its private bounds");
            a->pillar.interval = (Uint16)((a->pillar.interval + 1U) & 7U);
            ++a->substep;
        }
        a->state.frame = a->state.parameters ? 9 : 1;
        if (a->state.parameters == (Uint16)(Side(a, s->player) ^ 1U))
        {
            if (!a->pillar.timer)
                return SDL_SetError("Chapel Bone Pillar retained an expired attack wait");
            if (!--a->pillar.timer)
                SetStep(a, 4);
        }
        if (Distance(a, s->player) > 128)
            SetStep(a, 3);
        return true;
    case 4:
        a->state.palette = (Uint16)(base + ((s->service_ticks & 2U) ? 0 : 1));
        if (!ActorAnimate(s, a, a->state.parameters ? 0x1ce4 : 0x1cac, &animated))
            return false;
        if (!animated)
        {
            a->state.palette = (Uint16)(base + 1U);
            SetStep(a, 3);
        }
        if (a->pose == 17 && (animated & 128U))
        {
            Actor *fire = NULL;
            if (!Spawn(s, a, 0x2a, 0, 0, a->state.parameters ? -14 : 8, &fire))
                return false;
            if (fire)
                fire->state.facing_left = a->state.parameters != 0;
        }
        return true;
    case 8:
        return SkullDeath(s, a);
    case 10:
        return Bounce(s, a);
    default:
        return SDL_SetError("Chapel Bone Pillar skull entered an invalid original state");
    }
}
static bool Fire(SotnActors *s, Actor *a)
{
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, 0xa10))
            return false;
        a->state.animation_set = 0;
        if (!ActorPrimitives(s, a, 16))
        {
            a->remove = true;
            return true;
        }
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
        if (!p)
            return false;
        p->state = (SotnActorPrimitive){.x = Pixel(a->state.x) * 65536,
                                        .y = Pixel(a->state.y) * 65536,
                                        .page = 19,
                                        .palette = 0x221,
                                        .priority = a->state.priority,
                                        .priority_set = true,
                                        .active = true,
                                        .textured = true,
                                        .quad = true};
        const bool left = a->state.facing_left;
        p->state.offset_x[0] = left ? 48 : -64;
        p->state.offset_x[1] = -4;
        p->state.offset_x[2] = left ? 64 : -48;
        p->state.offset_x[3] = 4;
        p->state.offset_y[0] = left ? -96 : 0;
        p->state.offset_y[1] = -8;
        p->state.offset_y[2] = left ? -32 : 64;
        p->state.offset_y[3] = 8;
        a->state.hitbox_width = 21;
        a->state.hitbox_height = 24;
        a->state.hitbox_x = -32;
        a->state.hitbox_y = left ? -24 : 30;
        if (!ActorPositionalSound(s, a, 0x660))
            return false;
    }
    if (a->state.step == 1)
    {
        Uint8 animated = 0;
        if (!ActorAnimate(s, a, 0x1d18, &animated))
            return false;
        if (!animated)
        {
            a->state.hitbox_state = 0;
            ++a->state.step;
            return true;
        }
        Actor frame = *a;
        frame.state.animation_set = 0x8005;
        SotnDataView parts = {0};
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
        if (!p || !ActorStageParts(s, &frame, &parts))
            return false;
        const Uint8 *d = parts.bytes + 2;
        p->state.u[0] = p->state.u[2] = (Uint8)Read16(d + 14);
        p->state.v[0] = p->state.v[1] = (Uint8)Read16(d + 16);
        p->state.u[1] = p->state.u[3] = (Uint8)Read16(d + 18);
        p->state.v[2] = p->state.v[3] = (Uint8)Read16(d + 20);
        return true;
    }
    if (a->state.step == 2)
    {
        for (Uint16 i = 0; i < 16; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (!p)
                return false;
            const Sint32 dx = a->state.facing_left ? (Sint32)(ActorRandom(s) & 31U) + 16
                                                   : -(Sint32)(ActorRandom(s) & 31U) - 16;
            const Sint32 dy = -dx - (Sint32)(ActorRandom(s) & 31U) + 8;
            p->state = (SotnActorPrimitive){.x = Wrap16((Sint64)Pixel(a->state.x) + dx) * 65536,
                                            .y = Wrap16((Sint64)Pixel(a->state.y) + dy) * 65536,
                                            .width = 1,
                                            .page = 32,
                                            .height = 1,
                                            .red = 224,
                                            .green = 160,
                                            .blue = 136,
                                            .priority = (Uint16)(a->state.priority + 1U),
                                            .priority_set = true,
                                            .active = true,
                                            .blend = true};
            p->timer = (Uint16)((ActorRandom(s) & 7U) + 1U);
        }
        a->pillar.timer = 48;
        ++a->state.step;
    }
    if (a->state.step != 3 || !a->pillar.timer)
        return SDL_SetError("Chapel Bone Pillar fire entered an invalid ember phase");
    for (Uint16 i = 0; i < 16; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p || !p->timer)
            return SDL_SetError("Chapel Bone Pillar ember has no bounded cadence");
        if (!(s->service_ticks % p->timer))
            p->state.y -= 65536;
        p->state.red -= 2;
        p->state.green -= 2;
        p->state.blue -= 2;
    }
    if (!--a->pillar.timer)
        a->remove = true;
    return true;
}
static bool Part(SotnActors *s, Actor *a)
{
    if (a->state.parameters > 3)
        return SDL_SetError("Chapel Bone Pillar death part index exceeds its private table");
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0xa04))
            return false;
        a->state.hitbox_state = 0;
        a->state.frame = (Uint16)(a->state.parameters + 18U);
        a->state.priority = (Uint16)(a->state.priority + a->state.parameters);
        a->pillar.timer = (a->state.parameters & 1U) ? 24 : 12;
        return true;
    case 1:
        if (!a->pillar.timer)
            return SDL_SetError("Chapel Bone Pillar death part lost its initial delay");
        if (!--a->pillar.timer)
            ++a->state.step;
        return true;
    case 2: {
        SotnDataView offset = {0};
        SotnCollider c = {0};
        if (!Move(a) || !Slice(s->nz0, 0x1c9cU + a->state.parameters * 2U, 2, &offset) ||
            !ActorQuery(s, Pixel(a->state.x),
                        Wrap16((Sint64)Pixel(a->state.y) + Signed16(offset.bytes)), &c))
            return false;
        a->vy += 10240;
        if (c.effects & SOTN_EFFECT_SOLID)
        {
            ShiftY(a, c.top);
            a->pillar.timer = 14;
            ++a->state.step;
        }
        return true;
    }
    case 3:
        if (!a->pillar.timer)
            return SDL_SetError("Chapel Bone Pillar death part lost its explosion delay");
        if (!--a->pillar.timer)
        {
            a->state.id = 2;
            a->state.step = a->state.parameters = 0;
        }
        return true;
    default:
        return SDL_SetError("Chapel Bone Pillar part entered an invalid original state");
    }
}
bool ActorChapelPillar(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 6)
        return SDL_SetError("Chapel Bone Pillar requires its original stage owner");
    switch (a->state.id)
    {
    case 0x29:
        return Skull(s, a);
    case 0x2a:
        return Fire(s, a);
    case 0x2b:
        return Part(s, a);
    default:
        return SDL_SetError("Chapel Bone Pillar received an invalid original actor identity");
    }
}
