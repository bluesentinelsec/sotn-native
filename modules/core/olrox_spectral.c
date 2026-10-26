// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted st/no2/e_spectral_sword.c and st/e_spectral_sword_aura.h at
// sotn-decomp 7bc54d8157b6. Runtime profiles, artwork and SDK tables stay private.
#include "actors_internal.h"
#include "boss_projection.h"

Actor *ActorNo2SpectralOwner(SotnActors *s, const Actor *a)
{
    if (!s || !a || s->stage_id != 4 || !a->state.generation ||
        (a->state.id != 0x3f && a->state.id != 0x40) ||
        a->enemy_state.owner_slot >= SOTN_STAGE_ACTOR_SLOTS)
        return NULL;
    Actor *root = &s->slots[a->enemy_state.owner_slot];
    if (root->state.id != 0x3e || !root->state.generation ||
        root->state.generation != a->enemy_state.owner_generation)
        return NULL;
    if (a->state.id == 0x40 &&
        (!a->state.parameters || a->state.parameters > 15 ||
         (Uint32)root->slot + a->state.parameters != a->slot ||
         root->no2_spectral.generation[a->state.parameters] != a->state.generation))
        return NULL;
    return root;
}
static bool Children(SotnActors *s, const Actor *a, bool creating)
{
    if ((Uint32)a->slot + 15U >= SOTN_STAGE_ACTOR_SLOTS ||
        (creating && s->effect_serial > 0x7fffffffU - 15U))
        return SDL_SetError("NO2 Spectral Sword cannot reserve fifteen original weapons");
    for (Uint16 i = 1; i <= 15; ++i)
    {
        const Actor *child = &s->slots[a->slot + i];
        SotnActorPlacement placement = {0};
        Uint32 placed = 0;
        if (!SotnRoomActors_Get(s->spawns, a->slot + i, &placement, &placed))
            return false;
        if (placed ||
            (child->state.generation &&
             (child->state.id != 0x40 || child->enemy_state.owner_slot != a->slot)) ||
            (!creating && (!child->state.generation ||
                           child->enemy_state.owner_generation != a->state.generation ||
                           child->state.generation != a->no2_spectral.generation[i])))
            return SDL_SetError(
                "NO2 Spectral Sword lost or conflicts with a required owned weapon");
    }
    return true;
}
static bool Move(Actor *a, Sint32 gravity)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy,
                 vy = (Sint64)a->vy + gravity;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32 ||
        vy < SDL_MIN_SINT32 || vy > SDL_MAX_SINT32)
        return SDL_SetError("NO2 Spectral Sword motion exceeds fixed-point bounds");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    a->vy = (Sint32)vy;
    return true;
}
static Sint32 High(Sint32 old, Sint32 high)
{
    return (Sint32)((Sint64)Wrap16(high) * 65536 + ((Uint32)old & 65535U));
}
static bool Length(SotnActors *s, Sint32 x, Sint32 y, Sint32 *out)
{
    const Sint64 squared = (Sint64)x * x + (Sint64)y * y;
    Uint32 value = 0;
    if (squared > SDL_MAX_SINT32 || !SotnMath_Sqrt12(s->executable, (Uint32)squared, &value))
        return SDL_SetError("NO2 Spectral Sword direction exceeds the positive SDK range");
    *out = (Sint32)(value >> 6);
    return true;
}
static bool Aim(SotnActors *s, Actor *a, Sint32 dx, Sint32 dy, Sint32 cap, Sint32 divisor)
{
    Sint32 length = 0;
    Sint16 angle = 0;
    SotnMathTrig trig = {0};
    if (!Length(s, dx, dy, &length) || !SotnMath_Atan2(s->executable, dy, dx, &angle) ||
        !SotnMath_Trig(s->executable, angle, &trig))
        return false;
    length = SDL_min(length / divisor, cap);
    a->vx = length * trig.cosine;
    a->vy = length * trig.sine;
    return true;
}
static bool Toward(Sint16 *value, Sint32 target, Sint32 step)
{
    if (SDL_abs((Sint32)*value - target) < step)
    {
        *value = (Sint16)Wrap16(target);
        return true;
    }
    if (*value > target)
        *value = (Sint16)Wrap16((Sint32)*value - step);
    if (*value < target)
        *value = (Sint16)Wrap16((Sint32)*value + step);
    return false;
}
static Uint16 Limit(Uint16 base, Uint16 target)
{
    const Uint16 difference = (Uint16)(target - base);
    const Uint16 magnitude =
        difference & 0x800U ? (Uint16)((0x800U - difference) & 0x7ffU) : difference;
    return magnitude > 32 ? (Uint16)(base + ((difference & 0x800U) ? -32 : 32)) : target;
}
static bool Aura(SotnActors *s, Actor *a)
{
    const Actor *root = ActorNo2SpectralOwner(s, a);
    if (!root)
    {
        a->remove = true;
        return true;
    }
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x97c))
            return false;
        a->state.frame = 1;
        a->state.palette = (Uint16)(a->state.palette + 2U);
        a->state.scale_set = a->state.opacity_set = true;
        a->state.blend_mode = 0x30;
        a->state.scale_x = a->state.scale_y = 256;
        a->state.opacity = 128;
    }
    if (a->state.step != 1 || a->state.scale_x > 65529 || a->state.scale_y > 65529 ||
        a->state.opacity < 4)
        return SDL_SetError("NO2 Spectral Sword aura has an invalid scale, opacity or state");
    a->state.x = root->state.x;
    a->state.y = root->state.y;
    a->state.scale_x = (Uint16)(a->state.scale_x + 6U);
    a->state.scale_y = (Uint16)(a->state.scale_y + 6U);
    a->state.opacity = (Uint16)(a->state.opacity - 4U);
    a->remove = a->state.opacity < 32;
    return true;
}
static bool Weapon(SotnActors *s, Actor *a)
{
    Actor *root = ActorNo2SpectralOwner(s, a);
    if (!root)
    {
        a->remove = true;
        return true;
    }
    if ((a->state.flags & 0x100U) && a->state.step < 16)
        SetStep(a, 16);
    SotnDataView profile = {0};
    if (!Slice(s->nz0, 0x33d0 + a->state.parameters * 2U, 2, &profile))
        return false;
    const Sint16 offset = (Sint16)Signed16(profile.bytes);
    Sint16 angle = 0;
    SotnMathTrig trig = {0};
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x988))
            return false;
        a->state.frame = (Uint16)(a->state.parameters + 1U);
        a->state.rotated = true;
        if (!Slice(s->nz0, 0x336c + a->state.frame * 4U, 4, &profile))
            return false;
        a->state.hitbox_x =
            (Sint16)(profile.bytes[0] < 128 ? profile.bytes[0] : profile.bytes[0] - 256);
        a->state.hitbox_y =
            (Sint16)(profile.bytes[1] < 128 ? profile.bytes[1] : profile.bytes[1] - 256);
        a->state.hitbox_width = profile.bytes[2];
        a->state.hitbox_height = profile.bytes[3];
        a->state.hitbox_state = 0;
        a->spectral.timer = 64;
        a->state.step = 1;
        break;
    case 1:
        if (a->no2_spectral.cursor)
        {
            a->no2_spectral.cursor = 0;
            SetStep(a, 3);
        }
        break;
    case 2:
        if (!Move(a, 0))
            return false;
        (void)Toward(&a->state.rotation, 0, 8);
        if (!--a->spectral.timer)
            a->state.hitbox_state = 3;
        // The source's signed timer wraps after zero while motion continues.
        if (!Aim(s, a, Pixel(a->spectral.target_x) - Pixel(a->state.x),
                 Pixel(a->spectral.target_y) - Pixel(a->state.y), 56, 1))
            return false;
        if (a->no2_spectral.cursor)
        {
            if (!ActorPositionalSound(s, a, 0x66d))
                return false;
            a->no2_spectral.cursor = 0;
            SetStep(a, 3);
        }
        break;
    case 3:
        switch (a->substep)
        {
        case 0:
            if (!SotnMath_Atan2(s->executable, Pixel(s->player.y) - Pixel(a->state.y),
                                Pixel(s->player.x) - Pixel(a->state.x), &angle))
                return false;
            a->spectral.spin = (Sint16)Wrap16((Sint32)angle + 4096);
            ++a->substep;
            SDL_FALLTHROUGH;
        case 1:
            if (Toward(&a->state.rotation, (Sint32)a->spectral.spin - offset, 256))
                ++a->substep;
            break;
        case 2:
            a->spectral.spin = (Sint16)((Uint16)a->spectral.spin & 4095U);
            a->state.rotation = (Sint16)((Uint16)a->state.rotation & 4095U);
            if (!SotnMath_Trig(s->executable, a->spectral.spin, &trig) ||
                !ActorPositionalSound(s, a, 0x6c8))
                return false;
            a->vx = trig.cosine * 64;
            a->vy = trig.sine * 64;
            a->spectral.timer = 48;
            ++a->substep;
            SDL_FALLTHROUGH;
        case 3:
            if (!a->spectral.timer || !Move(a, 0))
                return SDL_SetError("NO2 poltergeist throw has an invalid timer or position");
            if (!--a->spectral.timer || Pixel(a->state.y) - s->camera_y > 224)
                SetStep(a, 2);
            break;
        default:
            return SDL_SetError("NO2 poltergeist entered an invalid throw substate");
        }
        break;
    case 16:
        switch (a->substep)
        {
        case 0:
            if (!root->spectral.count)
                return SDL_SetError(
                    "NO2 poltergeist death would underflow its body's weapon count");
            --root->spectral.count;
            a->state.hitbox_state = 0;
            if (!ActorPositionalSound(s, a, 0x655))
                return false;
            ++a->substep;
            SDL_FALLTHROUGH;
        case 1:
            if (!Move(a, 8192) || (!(s->service_ticks & 7U) &&
                                   !ActorExplosionInPool(s, a, 1, 160, 192, a->state.priority)))
                return false;
            if (Pixel(a->state.y) - s->camera_y > 320)
            {
                ++a->substep;
                a->state.frame = 0;
            }
            break;
        case 2:
            a->state.frame = 0;
            break;
        default:
            return SDL_SetError("NO2 poltergeist entered an invalid death substate");
        }
        break;
    default:
        return SDL_SetError("NO2 poltergeist entered an invalid state");
    }
    a->state.rotated = a->state.rotation != 0;
    if (!Slice(s->nz0, 0x33b0 + a->state.parameters * 2U, 2, &profile) || profile.bytes[0] > 127 ||
        profile.bytes[1] > 127 ||
        !SotnMath_Trig(s->executable, (Sint32)a->state.rotation + offset, &trig))
        return SDL_SetError("NO2 poltergeist references an invalid private weapon hitbox");
    a->state.hitbox_width = a->state.hitbox_height = profile.bytes[1];
    a->state.hitbox_x = (Sint16)ActorShift((Sint64)profile.bytes[0] * trig.cosine, 12);
    a->state.hitbox_y = (Sint16)ActorShift((Sint64)profile.bytes[0] * trig.sine, 12);
    return true;
}
static bool Initialize(SotnActors *s, Actor *a)
{
    if (!Children(s, a, true) || !ActorInitialize(s, a, 0x970))
        return false;
    a->state.frame = 1;
    a->state.blend_mode = 0x30;
    a->state.hitbox_x = 0;
    a->state.hitbox_y = 4;
    a->state.priority = 176;
    if (!ActorPrimitives(s, a, 1))
    {
        SDL_ClearError();
        a->remove = true;
        return true;
    }
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    if (!p)
        return false;
    p->state = (SotnActorPrimitive){.width = 2,
                                    .height = 2,
                                    .red = 192,
                                    .green = 192,
                                    .blue = 192,
                                    .priority_set = true,
                                    .priority = 192};
    for (Uint16 i = 1; i <= 15; ++i)
    {
        Actor *child = NULL;
        if (!ActorOwnedSpawnAt(s, a, 0x40, i, (Uint32)a->slot + i, &child))
            return false;
        a->no2_spectral.generation[i] = child->state.generation;
        child->state.x = Pixel(a->state.x) * 65536;
        child->state.y = Wrap16((Sint64)s->camera_y + 1408) * 65536;
        child->state.facing_left = false;
        child->state.priority = a->state.priority;
    }
    a->state.step = 1;
    a->no2_spectral.formation = 0;
    a->spectral.count = 15;
    return true;
}
static void Ready(SotnActors *s, Actor *a, Uint16 step, bool reposition)
{
    for (Uint16 i = 1; i <= 15; ++i)
    {
        Actor *child = &s->slots[a->slot + i];
        if (!reposition && (child->state.flags & 0x100U))
            continue;
        child->state.step = step;
        if (reposition)
            child->state.y = High(child->state.y, s->camera_y + 288);
    }
}
static bool Formations(SotnActors *s, Actor *a)
{
    if (!a->spectral.count)
        a->no2_spectral.formation = 0;
    const Sint16 missing = (Sint16)(15 - a->spectral.count);
    const Sint16 radius = (Sint16)(missing * 8 + 64);
    const Uint16 interval = a->spectral.count ? (Uint16)(4096 / a->spectral.count) : 0;
    switch (a->no2_spectral.formation)
    {
    case 0:
        break;
    case 1:
        a->state.rotation = (Sint16)Wrap16((Sint32)a->state.rotation + missing * 4 + 8);
        for (Uint16 i = 0; i < 15; ++i)
        {
            Actor *child = &s->slots[a->slot + i + 1U];
            if (child->state.flags & 0x100U)
                continue;
            SotnMathTrig trig = {0};
            if (!SotnMath_Trig(s->executable, i * interval + a->state.rotation, &trig))
                return false;
            const Sint64 x = (Sint64)a->state.x + radius * trig.cosine * 16LL,
                         y = (Sint64)a->state.y + radius * trig.sine * 16LL;
            if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 ||
                y > SDL_MAX_SINT32)
                return SDL_SetError("NO2 Spectral Sword orbit exceeds fixed-point bounds");
            child->spectral.target_x = (Sint32)x;
            child->spectral.target_y = (Sint32)y;
        }
        break;
    case 2: {
        Sint32 x = Pixel(a->state.x) - (a->spectral.count / 2 + 1) * 16;
        for (Uint16 i = 1; i <= 15; ++i)
        {
            Actor *child = &s->slots[a->slot + i];
            if (child->state.flags & 0x100U)
                continue;
            x += 16;
            if (x == Pixel(a->state.x))
                x += 16;
            child->spectral.target_x = High(child->spectral.target_x, x);
            child->spectral.target_y = High(child->spectral.target_y, Pixel(a->state.y));
        }
        break;
    }
    case 3: {
        a->no2_spectral.rot_x = (Sint16)Wrap16((Sint32)a->no2_spectral.rot_x + 8);
        a->no2_spectral.rot_z = (Sint16)Wrap16((Sint32)a->no2_spectral.rot_z + missing * 4 + 8);
        SotnProjectedPoint positions[15] = {0};
        Uint16 depths[15] = {0};
        for (Uint16 i = 0; i < 15; ++i)
            if (!SotnNo2_ProjectOrbit(s->executable, a->no2_spectral.rot_x, a->no2_spectral.rot_y,
                                      (Sint16)Wrap16((Sint32)a->no2_spectral.rot_z + interval * i),
                                      radius, Pixel(a->state.x) - s->camera_x,
                                      Pixel(a->state.y) - s->camera_y, &positions[i], &depths[i]))
                return false;
        Uint16 next = 0;
        for (Uint16 i = 1; i <= 15; ++i)
        {
            Actor *child = &s->slots[a->slot + i];
            if (child->state.flags & 0x100U)
                continue;
            const SotnProjectedPoint point = positions[next];
            child->spectral.target_x = High(child->spectral.target_x, point.x + s->camera_x);
            child->spectral.target_y = High(child->spectral.target_y, point.y + s->camera_y);
            child->state.priority =
                (Uint16)SDL_max(170, (Sint32)a->state.priority - ((Sint32)depths[next] - 256));
            ++next;
        }
        break;
    }
    default:
        return SDL_SetError("NO2 Spectral Sword selected an invalid weapon formation");
    }
    if (!(a->state.flags & 15U))
        a->state.palette = (s->service_ticks & 1U) ? 0x241 : 0x242;
    if (!(s->service_ticks & 15U))
    {
        Actor *aura = NULL;
        if (!ActorSpawn(s, a, 0x3f, 0, 160, 192, &aura))
            return false;
        if (aura)
        {
            aura->enemy_state.owner_slot = a->slot;
            aura->enemy_state.owner_generation = a->state.generation;
            aura->state.x = Pixel(a->state.x) * 65536;
            aura->state.y = Pixel(a->state.y) * 65536;
            aura->state.facing_left = false;
        }
    }
    return true;
}
static bool Body(SotnActors *s, Actor *a)
{
    if ((a->state.flags & 0x100U) && a->state.step < 8)
        SetStep(a, 8);
    if (a->state.step && !Children(s, a, false))
        return false;
    if (a->spectral.count > 15)
        return SDL_SetError("NO2 Spectral Sword exceeds its original fifteen-weapon count");
    ActorPrimitive *p = NULL;
    Sint16 angle = 0;
    SotnMathTrig trig = {0};
    switch (a->state.step)
    {
    case 0:
        if (!Initialize(s, a))
            return false;
        if (a->remove)
            return true;
        break;
    case 1:
        if (Distance(a, s->player) < 64 && SDL_abs(Pixel(a->state.y) - Pixel(s->player.y)) < 96)
            SetStep(a, 2);
        break;
    case 2:
        if (!a->substep)
        {
            a->spectral.timer = 256;
            a->no2_spectral.formation = 1;
            Ready(s, a, 2, true);
            ++a->substep;
        }
        if (a->substep != 1 || !a->spectral.timer)
            return SDL_SetError("NO2 Spectral Sword preparation has an invalid timer or state");
        if (!--a->spectral.timer)
            SetStep(a, 4);
        break;
    case 3:
        SetStep(a, 4);
        break;
    case 4:
        switch (a->substep)
        {
        case 0:
            a->spectral.moves = 512;
            a->no2_spectral.formation = 3;
            Ready(s, a, 2, false);
            ++a->substep;
            SDL_FALLTHROUGH;
        case 1:
            if (!SotnMath_Trig(s->executable, (Sint32)ActorRandom(s) * 8, &trig))
                return false;
            p = ActorPrimitiveGet(s, a, 0);
            if (!p)
                return false;
            p->state.x = Wrap16((Sint64)Pixel(s->player.x) - s->camera_x +
                                ActorShift((Sint64)trig.cosine * 96, 12)) *
                         65536;
            p->state.y = Wrap16((Sint64)Pixel(s->player.y) - s->camera_y +
                                ActorShift((Sint64)trig.sine * -96, 12)) *
                         65536;
            a->spectral.timer = 128;
            ++a->substep;
            SDL_FALLTHROUGH;
        case 2:
            p = ActorPrimitiveGet(s, a, 0);
            if (!p || !a->spectral.timer || !a->spectral.moves)
                return SDL_SetError("NO2 Spectral Sword patrol lost its cursor or interval");
            if (!--a->spectral.timer)
                a->substep = 1;
            if (SDL_abs(Pixel(p->state.x) - (Pixel(a->state.x) - s->camera_x)) < 8 &&
                SDL_abs(Pixel(p->state.y) - (Pixel(a->state.y) - s->camera_y)) < 8)
                a->substep = 1;
            if (!--a->spectral.moves)
                SetStep(a, 5);
            break;
        default:
            return SDL_SetError("NO2 Spectral Sword entered an invalid patrol substate");
        }
        if (!Move(a, 0))
            return false;
        p = ActorPrimitiveGet(s, a, 0);
        if (!p ||
            !SotnMath_Atan2(s->executable, Pixel(p->state.y) - (Pixel(a->state.y) - s->camera_y),
                            Pixel(p->state.x) - (Pixel(a->state.x) - s->camera_x), &angle))
            return false;
        a->spectral.spin = (Sint16)Wrap16(Limit((Uint16)a->spectral.spin, (Uint16)angle));
        if (!SotnMath_Trig(s->executable, a->spectral.spin, &trig))
            return false;
        a->vx = trig.cosine * 16;
        a->vy = trig.sine * 16;
        break;
    case 5:
        switch (a->substep)
        {
        case 0:
            a->no2_spectral.formation = 2;
            a->spectral.timer = 256;
            Ready(s, a, 2, false);
            ++a->substep;
            SDL_FALLTHROUGH;
        case 1:
            if (!a->spectral.timer)
                return SDL_SetError("NO2 Spectral Sword attack preparation lost its timer");
            if (!--a->spectral.timer)
            {
                if (Distance(a, s->player) < 64)
                    ++a->substep;
                else
                    SetStep(a, 4);
            }
            break;
        case 2:
            Ready(s, a, 1, false);
            a->no2_spectral.formation = 1;
            a->spectral.moves = 32;
            a->no2_spectral.cursor = 0;
            ++a->substep;
            SDL_FALLTHROUGH;
        case 3:
            if (!a->spectral.moves)
                return SDL_SetError("NO2 Spectral Sword lost its staggered throw interval");
            if (!--a->spectral.moves)
            {
                a->spectral.moves = 32;
                do
                {
                    ++a->no2_spectral.cursor;
                } while (a->no2_spectral.cursor <= 15 &&
                         (s->slots[a->slot + a->no2_spectral.cursor].state.flags & 0x100U));
                if (a->no2_spectral.cursor <= 15)
                    s->slots[a->slot + a->no2_spectral.cursor].no2_spectral.cursor |= 1U;
                else
                {
                    a->spectral.timer = 64;
                    ++a->substep;
                }
            }
            break;
        case 4:
            if (!a->spectral.timer)
                return SDL_SetError("NO2 Spectral Sword lost its staggered recovery interval");
            if (!--a->spectral.timer)
                SetStep(a, 4);
            break;
        default:
            return SDL_SetError("NO2 Spectral Sword entered an invalid attack substate");
        }
        if (!Move(a, 0) || !Aim(s, a, s->camera_x + 128 - Pixel(a->state.x),
                                Pixel(s->player.y) - 96 - Pixel(a->state.y), 48, 2))
            return false;
        break;
    case 8:
        switch (a->substep)
        {
        case 0:
            a->state.hitbox_state = 0;
            for (Uint16 i = 1; i <= 15; ++i)
                s->slots[a->slot + i].state.flags |= 0x100U;
            a->state.rotated = true;
            a->state.rotation = 0;
            if (!ActorPositionalSound(s, a, 0x697))
                return false;
            ++a->substep;
            break;
        case 1:
            if (!Move(a, 6144) || (!(s->service_ticks & 7U) &&
                                   !ActorExplosionInPool(s, a, 3, 160, 192, a->state.priority)))
                return false;
            a->state.rotation = (Sint16)Wrap16((Sint32)a->state.rotation - 4);
            a->state.flags |= 0x80000000U;
            break;
        case 2:
            break;
        default:
            return SDL_SetError("NO2 Spectral Sword entered an invalid death substate");
        }
        break;
    default:
        return SDL_SetError("NO2 Spectral Sword entered an invalid body state");
    }
    return (a->state.flags & 0x100U) || Formations(s, a);
}
bool ActorNo2Spectral(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 4)
        return SDL_SetError("NO2 Spectral Sword requires its original stage owner");
    switch (a->state.id)
    {
    case 0x3e:
        return Body(s, a);
    case 0x3f:
        return Aura(s, a);
    case 0x40:
        return Weapon(s, a);
    default:
        return SDL_SetError("NO2 Spectral Sword received an invalid actor identity");
    }
}
