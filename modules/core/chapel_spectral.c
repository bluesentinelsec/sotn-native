// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/dai/e_spectral_sword.c and st/step_towards.h at sotn-decomp 7bc54d8157b6.
// Artwork, initialization, hitboxes and SDK lookup tables remain private inputs.
#include "actors_internal.h"
static Actor *Parent(SotnActors *s, const Actor *a)
{
    const Uint16 slot = a->enemy_state.owner_slot;
    if (slot >= SOTN_STAGE_ACTOR_SLOTS || s->slots[slot].state.id != 0x3f ||
        !s->slots[slot].state.generation ||
        s->slots[slot].state.generation != a->enemy_state.owner_generation)
        return NULL;
    return &s->slots[slot];
}
static bool Children(SotnActors *s, const Actor *a, bool initializing)
{
    if ((Uint32)a->slot + 16U >= SOTN_STAGE_ACTOR_SLOTS ||
        (initializing && s->effect_serial > 0x7fffffffU - 16U))
        return SDL_SetError("Chapel Spectral Sword cannot reserve sixteen owned weapons");
    for (Uint16 i = 1; i <= 16; ++i)
    {
        const Actor *child = &s->slots[a->slot + i];
        SotnActorPlacement placement = {0};
        Uint32 placed = 0;
        if (!SotnRoomActors_Get(s->spawns, a->slot + i, &placement, &placed))
            return false;
        if (placed ||
            (child->state.generation &&
             (child->state.id != 0x41 || child->enemy_state.owner_slot != a->slot ||
              (!initializing && child->enemy_state.owner_generation != a->state.generation))) ||
            (!initializing && !child->state.generation))
            return SDL_SetError("Chapel Spectral Sword lost or conflicts with an owned weapon");
    }
    return true;
}
static bool Move(Actor *a, Sint32 gravity)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy,
                 vy = (Sint64)a->vy + gravity;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32 ||
        vy < SDL_MIN_SINT32 || vy > SDL_MAX_SINT32)
        return SDL_SetError("Chapel Spectral Sword exceeds fixed-point motion bounds");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    a->vy = (Sint32)vy;
    return true;
}
static bool Length(SotnActors *s, Sint32 x, Sint32 y, Sint32 *out)
{
    const Sint64 squared = (Sint64)x * x + (Sint64)y * y;
    Uint32 root = 0;
    if (squared > SDL_MAX_SINT32)
        return SDL_SetError("Chapel Spectral Sword direction exceeds the SDK square-root range");
    if (!SotnMath_Sqrt12(s->executable, (Uint32)squared, &root))
        return false;
    // The selected positive SquareRoot0 path shares Sqrt12's normalization/table.
    *out = (Sint32)(root >> 6);
    return true;
}
static bool Velocity(SotnActors *s, Actor *a, Sint32 x, Sint32 y, Sint32 cap, Sint32 divisor,
                     Sint32 *distance)
{
    Sint32 length = 0;
    Sint16 angle = 0;
    SotnMathTrig trig = {0};
    if (!Length(s, x, y, &length) || !SotnMath_Atan2(s->executable, y, x, &angle) ||
        !SotnMath_Trig(s->executable, angle, &trig))
        return false;
    *distance = SDL_min(length / divisor, cap);
    a->vx = *distance * trig.cosine;
    a->vy = *distance * trig.sine;
    return true;
}
static bool Toward(Sint16 *value, Sint32 target, Sint32 step)
{
    if (SDL_abs((Sint32)*value - target) < step)
    {
        *value = (Sint16)target;
        return true;
    }
    if (*value > target)
        *value = (Sint16)(*value - step);
    if (*value < target)
        *value = (Sint16)(*value + step);
    // Reaching the target by one exact step completes on the following call.
    return false;
}
static bool Aura(SotnActors *s, Actor *a)
{
    Actor *parent = Parent(s, a);
    if (!parent)
    {
        a->remove = true;
        return true;
    }
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0xaac))
            return false;
        a->state.hitbox_state = 0;
        a->state.frame = 1;
        a->state.palette = (Uint16)(a->state.palette + 13U);
        a->state.scale_set = a->state.opacity_set = true;
        a->state.blend_mode = 0x30;
        a->state.scale_x = a->state.scale_y = 256;
        a->state.opacity = 128;
    }
    if (a->state.step != 1 || a->state.scale_x > 65529 || a->state.scale_y > 65529 ||
        a->state.opacity < 4)
        return SDL_SetError("Chapel Spectral Sword aura has invalid scale or opacity state");
    a->state.x = parent->state.x;
    a->state.y = parent->state.y;
    a->state.scale_x = (Uint16)(a->state.scale_x + 6U);
    a->state.scale_y = (Uint16)(a->state.scale_y + 6U);
    a->state.opacity = (Uint16)(a->state.opacity - 4U);
    a->remove = a->state.opacity < 32;
    return true;
}
static bool Weapon(SotnActors *s, Actor *a)
{
    Actor *parent = Parent(s, a);
    if (!parent)
    {
        a->remove = true;
        return true;
    }
    if (a->state.parameters > 1)
        return SDL_SetError("Chapel Spectral Sword weapon size is outside its original variants");
    a->state.palette = 0x235;
    if ((a->state.flags & 0x100U) && a->state.step < 16)
        SetStep(a, 16);
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0xab8))
            return false;
        a->state.hitbox_state = 0;
        a->state.frame = a->state.parameters ? 2 : 3;
        a->state.priority = (Uint16)(a->state.priority + (a->state.parameters ? 1 : -1));
        a->state.rotated = true;
        a->state.step = 1;
        break;
    case 1:
        break;
    case 2: {
        Sint32 distance = 0;
        if (!Move(a, 0) ||
            !Velocity(s, a, Pixel(a->spectral.target_x) - Pixel(a->state.x),
                      Pixel(a->spectral.target_y) - Pixel(a->state.y), 56, 1, &distance))
            return false;
        if (distance < 4)
            SetStep(a, 4);
        break;
    }
    case 4:
        // Original ready weapons replace only integer positions, retaining their fractions.
        a->state.x =
            (Sint32)((Sint64)Pixel(a->spectral.target_x) * 65536 + ((Uint32)a->state.x & 65535U));
        a->state.y =
            (Sint32)((Sint64)Pixel(a->spectral.target_y) * 65536 + ((Uint32)a->state.y & 65535U));
        break;
    case 16:
        if (!a->substep)
        {
            if (!parent->spectral.count || parent->spectral.count > 16)
                return SDL_SetError("Chapel Spectral Sword weapon count would underflow");
            a->state.hitbox_state = 0;
            --parent->spectral.count;
            ++a->substep;
        }
        if (a->substep == 1)
        {
            if (!Move(a, 8192))
                return false;
            if (!(s->service_ticks & 7U) &&
                (!ActorPositionalSound(s, a, 0x652) || !ActorExplosionInPool(s, a, 1, 160, 192, 0)))
                return false;
            if (Pixel(a->state.y) - s->camera_y > 320)
            {
                ++a->substep;
                a->state.frame = 0;
            }
        }
        else if (a->substep == 2)
            a->state.frame = 0;
        else
            return SDL_SetError("Chapel Spectral Sword weapon entered an invalid death phase");
        break;
    default:
        return SDL_SetError("Chapel Spectral Sword weapon entered an invalid original state");
    }
    a->state.rotated = a->state.rotation != 0;
    SotnDataView box = {0};
    SotnMathTrig trig = {0};
    if (a->state.frame >= 10 || !Slice(s->nz0, 0x2294U + a->state.frame * 2U, 2, &box) ||
        box.bytes[0] > 127 || box.bytes[1] > 127)
        return SDL_SetError("Chapel Spectral Sword weapon hitbox exceeds its private frame table");
    if (!SotnMath_Trig(s->executable, (Sint32)a->state.rotation + 1024, &trig))
        return false;
    a->state.hitbox_width = a->state.hitbox_height = box.bytes[1];
    a->state.hitbox_x = (Sint16)ActorShift((Sint64)box.bytes[0] * trig.cosine, 12);
    a->state.hitbox_y = (Sint16)ActorShift((Sint64)box.bytes[0] * trig.sine, 12);
    return true;
}
static bool Initialize(SotnActors *s, Actor *a)
{
    if (!ActorInitialize(s, a, 0xaa0))
        return false;
    a->state.frame = 1;
    a->state.blend_mode = 0x30;
    a->state.hitbox_x = 0;
    a->state.hitbox_y = 4;
    if (!ActorPrimitives(s, a, 1))
    {
        a->remove = true;
        return true;
    }
    a->state.flags |= 0x800000U;
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    if (!p)
        return false;
    p->state = (SotnActorPrimitive){.width = 2,
                                    .height = 2,
                                    .red = 192,
                                    .green = 192,
                                    .blue = 192,
                                    .priority_set = true,
                                    .priority = 192,
                                    .screen_space = true};
    if (!Children(s, a, true))
        return false;
    for (Uint16 i = 1; i <= 16; ++i)
    {
        Actor *child = NULL;
        if (!ActorOwnedSpawnAt(s, a, 0x41, i > 12 ? 1 : 0, (Uint32)a->slot + i, &child))
            return false;
        child->state.x = Pixel(a->state.x) * 65536;
        child->state.y = Pixel(a->state.y) * 65536;
        child->state.facing_left = false;
    }
    a->state.step = 1;
    a->spectral.has_weapons = false;
    a->spectral.radius = 48;
    a->spectral.count = 16;
    return true;
}
static bool ReadyWeapons(SotnActors *s, Actor *a)
{
    if (!a->substep)
    {
        a->spectral.timer = 128;
        a->spectral.has_weapons = true;
        for (Uint16 i = 1; i <= 16; ++i)
            s->slots[a->slot + i].state.step = 2;
        ++a->substep;
    }
    if (a->substep != 1)
        return SDL_SetError("Chapel Spectral Sword entered an invalid weapon preparation phase");
    Uint16 ready = 0;
    for (Uint16 i = 1; i <= 16; ++i)
    {
        const Actor *child = &s->slots[a->slot + i];
        ready = (Uint16)(ready + (child->state.step == 4 || (child->state.flags & 0x100U)));
    }
    if (ready == 16)
    {
        for (Uint16 i = 1; i <= 16; ++i)
            s->slots[a->slot + i].state.hitbox_state = 3;
        SetStep(a, 3);
    }
    return true;
}
static bool Movement(SotnActors *s, Actor *a)
{
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    if (!p)
        return false;
    switch (a->substep)
    {
    case 0:
        a->spectral.moves = (Uint16)((ActorRandom(s) & 3U) + 3U);
        a->spectral.has_weapons = true;
        ++a->substep;
        SDL_FALLTHROUGH;
    case 1: {
        const Sint32 angle = (Sint32)(ActorRandom(s) * 4U + 512U);
        SotnMathTrig trig = {0};
        Sint32 length = 0;
        if (!SotnMath_Trig(s->executable, angle, &trig))
            return false;
        const Sint32 x = ActorShift((Sint64)trig.cosine * 96, 12),
                     y = ActorShift((Sint64)trig.sine * -96, 12);
        if (!Length(s, x, y, &length))
            return false;
        p->state.x = Wrap16((Sint64)Pixel(s->player.x) - s->camera_x + x) * 65536;
        p->state.y = Wrap16((Sint64)Pixel(s->player.y) - s->camera_y + y) * 65536;
        a->spectral.spin_target = (Sint16)(x < 0 ? -length : length);
        a->spectral.timer = 128;
        if (a->spectral.count && !ActorPositionalSound(s, a, 0x6d5))
            return false;
        ++a->substep;
        SDL_FALLTHROUGH;
    }
    case 2:
        if (Toward(&a->spectral.spin, a->spectral.spin_target, 4))
            ++a->substep;
        return true;
    case 3: {
        if (!a->spectral.timer)
            return SDL_SetError("Chapel Spectral Sword lost its movement interval");
        if (!Move(a, 0))
            return false;
        if (!--a->spectral.timer)
            a->substep = 1;
        const Sint32 x = Pixel(p->state.x) - (Pixel(a->state.x) - s->camera_x),
                     y = Pixel(p->state.y) - (Pixel(a->state.y) - s->camera_y);
        Sint32 distance = 0;
        if (!Velocity(s, a, x, y, 48, 2, &distance))
            return false;
        if (distance < 6)
        {
            a->substep = 1;
            if (!a->spectral.moves)
                return SDL_SetError("Chapel Spectral Sword lost its remaining movement count");
            if (!--a->spectral.moves)
                SetStep(a, 5);
        }
        Toward(&a->spectral.spin, x < 0 ? -distance : distance, 4);
        return true;
    }
    default:
        return SDL_SetError("Chapel Spectral Sword entered an invalid movement phase");
    }
}
static bool Attack(SotnActors *s, Actor *a)
{
    switch (a->substep)
    {
    case 0:
        if (a->spectral.count && !ActorPositionalSound(s, a, 0x6d4))
            return false;
        ++a->substep;
        SDL_FALLTHROUGH;
    case 1:
        if (Toward(&a->spectral.spin, 128, 2))
            ++a->substep;
        return true;
    case 2:
        if (Toward(&a->spectral.spin, 0, 2))
        {
            a->spectral.timer = 32;
            ++a->substep;
        }
        return true;
    case 3:
        if (!a->spectral.timer)
            return SDL_SetError("Chapel Spectral Sword lost its attack delay");
        if (!--a->spectral.timer)
        {
            if (a->spectral.count && !ActorPositionalSound(s, a, 0x689))
                return false;
            ++a->substep;
        }
        return true;
    case 4:
        if (a->spectral.radius < 48 || a->spectral.radius > 160)
            return SDL_SetError("Chapel Spectral Sword expansion exceeds its selected radius");
        a->spectral.radius = (Sint16)(a->spectral.radius + 8);
        if (a->spectral.radius > 160)
            ++a->substep;
        return true;
    case 5:
        if (Toward(&a->spectral.radius, 48, 1))
            SetStep(a, 4);
        return true;
    default:
        return SDL_SetError("Chapel Spectral Sword entered an invalid attack phase");
    }
}
static bool Death(SotnActors *s, Actor *a)
{
    switch (a->substep)
    {
    case 0:
        a->state.hitbox_state = 0;
        for (Uint16 i = 1; i <= 16; ++i)
            s->slots[a->slot + i].state.flags |= 0x100U;
        a->state.rotated = true;
        a->state.rotation = 0;
        ++a->substep;
        return true;
    case 1:
        if (!Move(a, 6144))
            return false;
        a->state.rotation = (Sint16)Wrap16((Sint64)a->state.rotation - 4);
        if (!(s->service_ticks & 7U) &&
            (!ActorPositionalSound(s, a, 0x655) || !ActorExplosionInPool(s, a, 3, 160, 192, 0)))
            return false;
        if (Pixel(a->state.y) - s->camera_y > 352)
            ++a->substep;
        return true;
    case 2:
        for (Uint16 i = 1; i <= 16; ++i)
            if (s->slots[a->slot + i].state.frame)
                return true;
        for (Uint16 i = 1; i <= 16; ++i)
            s->slots[a->slot + i].remove = true;
        a->remove = true;
        return true;
    default:
        return SDL_SetError("Chapel Spectral Sword entered an invalid death phase");
    }
}
static bool Orbit(SotnActors *s, Actor *a)
{
    const Sint32 lost = 16 - a->spectral.count;
    a->state.rotation = (Sint16)Wrap16((Sint64)a->state.rotation + a->spectral.spin +
                                       (a->spectral.spin > 0 ? lost * 2 : -lost * 2));
    for (Uint16 i = 0; i < 16; ++i)
    {
        const bool large = i >= 12;
        const Sint32 angle = (large ? a->state.rotation / 2 : a->state.rotation) +
                             (large ? (i - 12) * 1024 : i * 341);
        const Sint32 radius = large ? a->spectral.radius : (a->spectral.radius * 2) / 3;
        Actor *child = &s->slots[a->slot + i + 1U];
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(s->executable, angle, &trig))
            return false;
        child->state.rotation = (Sint16)Wrap16((Sint64)angle - 1024);
        if (!(child->state.flags & 0x100U))
        {
            const Sint64 x = (Sint64)a->state.x + (Sint64)radius * trig.cosine * 16,
                         y = (Sint64)a->state.y + (Sint64)radius * trig.sine * 16;
            if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 ||
                y > SDL_MAX_SINT32)
                return SDL_SetError("Chapel Spectral Sword orbit exceeds fixed-point bounds");
            child->spectral.target_x = (Sint32)x;
            child->spectral.target_y = (Sint32)y;
        }
    }
    return true;
}
bool ActorChapelSpectral(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 6 || a->state.id < 0x3f || a->state.id > 0x41)
        return SDL_SetError("Chapel Spectral Sword requires its original stage identity");
    if (a->state.id == 0x40)
        return Aura(s, a);
    if (a->state.id == 0x41)
        return Weapon(s, a);
    if ((a->state.flags & 0x100U) && a->state.step < 8)
        SetStep(a, 8);
    if (a->state.step && !Children(s, a, false))
        return false;
    if (a->spectral.count > 16)
        return SDL_SetError("Chapel Spectral Sword exceeds its original weapon count");
    switch (a->state.step)
    {
    case 0:
        if (!Initialize(s, a))
            return false;
        if (a->remove)
            return true;
        break;
    case 1:
        if (Distance(a, s->player) < 64)
            SetStep(a, 2);
        break;
    case 2:
        if (!ReadyWeapons(s, a))
            return false;
        break;
    case 3:
        if (!a->substep)
        {
            a->spectral.timer = 16;
            a->spectral.spin = 0;
            ++a->substep;
        }
        if (a->substep != 1 || !a->spectral.timer)
            return SDL_SetError("Chapel Spectral Sword entered an invalid hold interval");
        if (!--a->spectral.timer)
            SetStep(a, 4);
        break;
    case 4:
        if (!Movement(s, a))
            return false;
        break;
    case 5:
        if (!Attack(s, a))
            return false;
        break;
    case 8:
        if (!Death(s, a))
            return false;
        break;
    default:
        return SDL_SetError("Chapel Spectral Sword entered an invalid original state");
    }
    if (!(a->state.flags & 0x100U))
    {
        if (!a->spectral.count)
            a->spectral.has_weapons = false;
        if (a->spectral.has_weapons && !Orbit(s, a))
            return false;
        if (!(a->state.flags & 15U))
            a->state.palette = (s->service_ticks & 1U) ? 0x235 : 0x236;
        if (!(s->service_ticks & 15U))
        {
            Actor *aura = NULL;
            if (!ActorSpawn(s, a, 0x40, 0, 160, 192, &aura))
                return false;
            if (aura)
            {
                aura->state.x = Pixel(a->state.x) * 65536;
                aura->state.y = Pixel(a->state.y) * 65536;
                aura->state.facing_left = false;
                aura->enemy_state.owner_slot = a->slot;
                aura->enemy_state.owner_generation = a->state.generation;
            }
        }
    }
    return true;
}
