// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted NO2 st/e_valhalla_knight.h and selected st/collision.h at
// sotn-decomp 7bc54d8157b6. All original runtime arrays remain private.
#include "actors_internal.h"

Actor *ActorValhallaOwner(SotnActors *s, const Actor *a)
{
    if (!s || !a || s->stage_id != 4 || a->state.id != 0x35 || !a->state.generation ||
        a->state.parameters > 1)
        return NULL;
    const Uint16 slot = a->enemy_state.owner_slot;
    if (slot >= SOTN_STAGE_ACTOR_SLOTS || (Uint32)slot + a->state.parameters + 1U != a->slot)
        return NULL;
    Actor *body = &s->slots[slot];
    return body->state.id == 0x34 && body->state.generation &&
                   body->state.generation == a->enemy_state.owner_generation &&
                   body->valhalla.child_generation[a->state.parameters] == a->state.generation
               ? body
               : NULL;
}

void ActorValhallaHit(SotnActors *s, Actor *root, const Actor *hit, Uint32 channel, Uint8 cooldown)
{
    if (!s || !root || !hit || channel >= SDL_arraysize(root->cooldown) || s->stage_id != 4 ||
        root->state.id != 0x34)
        return;
    root->cooldown[channel] = cooldown;
    const Uint32 slot = (Uint32)root->slot + 1U;
    if (slot >= SOTN_STAGE_ACTOR_SLOTS)
        return;
    Actor *child = &s->slots[slot];
    if (ActorValhallaOwner(s, child) != root || child->state.parameters)
        return;
    child->cooldown[channel] = (Uint8)(cooldown + 1U);
    if (!(hit->state.flags & 0x400000U))
        child->stun = root->stun;
    if (root->state.flags & 0x100U)
    {
        child->state.flags |= 0x10c100U;
        child->state.flags &= ~0x20000000U;
    }
}

static bool Motion(const Actor *a, bool walking)
{
    const Sint64 x = (Sint64)a->state.x + a->vx,
                 y = (Sint64)a->state.y + (walking ? 3 * 65536 : a->vy);
    return (x >= SDL_MIN_SINT32 && x <= SDL_MAX_SINT32 && y >= SDL_MIN_SINT32 + 32 * 65536 &&
            y <= SDL_MAX_SINT32 - 32 * 65536) ||
           SDL_SetError("Valhalla Knight motion exceeds fixed-point sensor bounds");
}

static bool Ground(SotnActors *s, Actor *a, Uint16 *result)
{
    return Motion(a, false) && ActorGround(s, a, 0x2028, result);
}
static bool Walk(SotnActors *s, Actor *a, Uint16 *result)
{
    return Motion(a, true) && ActorWalk(s, a, 0x2038, result);
}
static void Substep(Actor *a, Uint16 step)
{
    a->substep = step;
    a->pose = a->remaining = 0;
}
static bool Puffs(SotnActors *s, const Actor *a)
{
    for (Uint16 i = 0; i < 5; ++i)
    {
        Actor *p = NULL;
        if (!ActorSpawn(s, a, 0x15, i, 96, 128, &p))
            return false;
        if (p)
        {
            const Sint32 dx =
                (a->state.facing_left ? -4 : 4) + (Sint32)i * (a->state.facing_left ? 4 : -4);
            p->state.x = Wrap16((Sint64)Pixel(a->state.x) + dx) * 65536;
            p->state.y = Wrap16((Sint64)Pixel(a->state.y) + 40) * 65536;
            p->state.facing_left = false;
            p->state.priority = (Uint16)(a->state.priority + 1U);
        }
    }
    return true;
}
static bool Hitbox(SotnActors *s, Actor *a, size_t at)
{
    SotnDataView v = {0};
    if (!Slice(s->nz0, at, 4, &v))
        return false;
    a->state.hitbox_x = (Sint16)(v.bytes[0] < 128 ? v.bytes[0] : v.bytes[0] - 256);
    a->state.hitbox_y = (Sint16)(v.bytes[1] < 128 ? v.bytes[1] : v.bytes[1] - 256);
    a->state.hitbox_width = v.bytes[2];
    a->state.hitbox_height = v.bytes[3];
    if (v.bytes[2] > 127 || v.bytes[3] > 127)
        return SDL_SetError("Valhalla Knight hitbox exceeds its signed private dimensions");
    return true;
}
static bool Child(SotnActors *s, Actor *a)
{
    if (a->state.parameters > 1)
        return SDL_SetError("Valhalla Knight hitbox has an invalid child parameter");
    const Actor *body = ActorValhallaOwner(s, a);
    if (!body)
    {
        a->remove = true;
        return true;
    }
    if (!a->state.step && !ActorInitialize(s, a, a->state.parameters ? 0x91c : 0x910))
        return false;
    if (a->state.step != 1 || body->state.frame >= 20)
        return SDL_SetError("Valhalla Knight child has an invalid state or parent frame");
    a->state.facing_left = body->state.facing_left;
    a->state.x = body->state.x;
    a->state.y = body->state.y;
    SotnDataView index = {0};
    const bool weapon = a->state.parameters != 0;
    if (!Slice(s->nz0, (weapon ? 0x20e0 : 0x20b8) + body->state.frame, 1, &index) ||
        index.bytes[0] > (weapon ? 4 : 8))
        return SDL_SetError("Valhalla Knight child frame references an invalid private hitbox");
    return Hitbox(s, a, (weapon ? 0x20cc : 0x2094) + index.bytes[0] * 4U);
}
static bool Death(SotnActors *s, Actor *a)
{
    SotnDataView offsets = {0};
    if (!Slice(s->nz0, 0x20f4, 12, &offsets))
        return false;
    for (Uint16 i = 0; i < 16; ++i)
    {
        Actor *p = NULL;
        if (!ActorSpawn(s, a, i < 3 ? 2 : 0x36, i < 3 ? 3 : (Uint16)(i - 3), 160, 192, &p))
            return false;
        if (!p)
            continue;
        Sint32 dx = 0, dy = 0;
        if (i < 3)
        {
            dx = Signed16(offsets.bytes + i * 4U);
            dy = Signed16(offsets.bytes + i * 4U + 2U);
            if (a->state.facing_left)
                dx = -dx;
        }
        else
        {
            p->state.facing_left = a->state.facing_left;
            p->vx = a->vx;
            p->vy = a->vy;
        }
        p->state.x = Wrap16((Sint64)Pixel(a->state.x) + dx) * 65536;
        p->state.y = Wrap16((Sint64)Pixel(a->state.y) + dy) * 65536;
        if (i < 3)
            p->state.facing_left = false;
    }
    if (!ActorSound(s, 0x654))
        return false;
    a->remove = true;
    return true;
}
static bool Parts(SotnActors *s, Actor *a)
{
    if (a->state.parameters >= 13)
        return SDL_SetError(
            "Valhalla Knight death piece exceeds its private thirteen-part profile");
    SotnDataView profile = {0};
    if (!Slice(s->nz0, 0x2100 + a->state.parameters * 12U, 12, &profile))
        return false;
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x904))
            return false;
        const Sint32 dx = Signed32(profile.bytes), dy = Signed32(profile.bytes + 4);
        const Sint64 vx = (Sint64)a->vx + (a->state.facing_left ? -(Sint64)dx : dx),
                     vy = (Sint64)a->vy + dy;
        if (vx < SDL_MIN_SINT32 || vx > SDL_MAX_SINT32 || vy < SDL_MIN_SINT32 ||
            vy > SDL_MAX_SINT32)
            return SDL_SetError("Valhalla Knight piece launch exceeds fixed-point bounds");
        a->vx = (Sint32)vx;
        a->vy = (Sint32)vy;
        a->state.frame = (Uint16)(a->state.parameters + 18U);
        a->state.priority = (Uint16)(a->state.priority + a->state.parameters);
        a->state.flags |= 0x40000000U;
        a->state.rotated = true;
    }
    if (a->state.step != 1 || !Motion(a, false) || a->vy > SDL_MAX_SINT32 - 4096)
        return SDL_SetError("Valhalla Knight piece has an invalid state or velocity");
    a->state.x += a->vx;
    a->state.y += a->vy;
    a->vy += 4096;
    a->state.rotation = (Sint16)Wrap16((Sint64)a->state.rotation + Signed16(profile.bytes + 8));
    if (!(a->state.parameters & 3U) && ((s->service_ticks - (a->state.parameters >> 2)) & 7U) == 0)
    {
        Actor *p = NULL;
        if (!ActorSpawn(s, a, 2, 0, 160, 192, &p))
            return false;
        if (p)
        {
            p->state.x = Pixel(a->state.x) * 65536;
            p->state.y = Pixel(a->state.y) * 65536;
            p->state.facing_left = false;
            p->state.parameters = (Uint16)(ActorRandom(s) & 1U);
            p->state.priority = (Uint16)(a->state.priority + 1U);
        }
    }
    return a->state.parameters || (s->service_ticks & 7U) || ActorSound(s, 0x653);
}
static bool Body(SotnActors *s, Actor *a)
{
    if (a->state.flags & 0x100U)
        SetStep(a, 6);
    Uint16 collision = 0;
    Uint8 animation = 0;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x8f8))
            return false;
        a->state.frame = 1;
        a->valhalla.anchor_x = Pixel(a->state.x);
        for (Uint16 i = 0; i < 2; ++i)
        {
            Actor *child = NULL;
            if (!ActorOwnedSpawnAt(s, a, 0x35, i, (Uint32)a->slot + i + 1U, &child))
                return false;
            child->state.x = Pixel(a->state.x) * 65536;
            child->state.y = Pixel(a->state.y) * 65536;
            child->state.facing_left = false;
            a->valhalla.child_generation[i] = child->state.generation;
        }
        SDL_FALLTHROUGH;
    case 1:
        if (!Ground(s, a, &collision))
            return false;
        if (collision & 1U)
        {
            a->state.facing_left = Side(a, s->player) != 0;
            SetStep(a, 2);
        }
        break;
    case 2:
        if (!a->substep)
        {
            a->valhalla.timer = 128;
            ++a->substep;
        }
        if (!ActorAnimate(s, a, 0x204c, &animation) ||
            (!animation && !ActorPositionalSound(s, a, 0x719)) || !Walk(s, a, &collision))
            return false;
        a->vx = a->state.facing_left ? -163840 : 163840;
        if (collision == 0x80)
        {
            SetStep(a, 5);
            a->substep = 2;
        }
        if ((a->state.facing_left ? a->valhalla.anchor_x - Pixel(a->state.x)
                                  : Pixel(a->state.x) - a->valhalla.anchor_x) > 96)
            SetStep(a, 3);
        break;
    case 3:
        if (!a->substep)
        {
            if (!ActorPositionalSound(s, a, 0x718))
                return false;
            ++a->substep;
        }
        if (!Walk(s, a, &collision))
            return false;
        a->vx -= a->vx / 32;
        if ((!a->remaining && !Puffs(s, a)) || !ActorAnimate(s, a, 0x2058, &animation))
            return false;
        if (!animation)
        {
            a->state.facing_left = !a->state.facing_left;
            if (s->player.velocity_y && Distance(a, s->player) < 128)
            {
                a->state.frame = 1;
                SetStep(a, 5);
            }
            else
            {
                a->state.frame = 6;
                SetStep(a, 2);
            }
        }
        break;
    case 5:
        switch (a->substep)
        {
        case 0:
            a->vx = a->state.facing_left ? -163840 : 163840;
            a->valhalla.timer = 16;
            ++a->substep;
            SDL_FALLTHROUGH;
        case 1:
            if (!ActorAnimate(s, a, 0x2040, &animation) ||
                (!animation && !ActorPositionalSound(s, a, 0x719)) || !Walk(s, a, &collision))
                return false;
            if (!a->valhalla.timer)
            {
                if (Distance(a, s->player) < 80)
                    a->substep = 2;
            }
            else
                --a->valhalla.timer;
            if ((a->state.facing_left ? a->valhalla.anchor_x - Pixel(a->state.x)
                                      : Pixel(a->state.x) - a->valhalla.anchor_x) > 96)
                SetStep(a, 3);
            break;
        case 2:
            a->vy = -196608;
            a->vx = a->state.facing_left ? -229376 : 229376;
            a->state.frame = 15;
            ++a->substep;
            SDL_FALLTHROUGH;
        case 3:
            if (!Ground(s, a, &collision) || a->vy < SDL_MIN_SINT32 + 10240)
                return SDL_SetError("Valhalla Knight jump exceeds fixed-point bounds");
            a->vy -= 10240;
            if (a->vy > -49152)
                a->state.frame = 16;
            if (a->vy > 49152)
            {
                a->state.frame = 17;
                ++a->substep;
            }
            break;
        case 4:
            if (!Ground(s, a, &collision))
                return false;
            if (collision & 1U)
            {
                if (!ActorSound(s, 0x647) || !Puffs(s, a))
                    return false;
                a->vx = a->state.facing_left ? -163840 : 163840;
                Substep(a, 5);
            }
            break;
        case 5:
            if (!Walk(s, a, &collision) || !ActorAnimate(s, a, 0x2064, &animation))
                return false;
            if (!animation)
                SetStep(a, 2);
            break;
        default:
            return SDL_SetError("Valhalla Knight entered an invalid leap substate");
        }
        break;
    case 6:
        return Death(s, a);
    default:
        return SDL_SetError("Valhalla Knight entered an invalid body state");
    }
    return Hitbox(s, a,
                  0x2074 + (a->state.frame == 11 || a->state.frame == 12   ? 4U
                            : a->state.frame == 13 || a->state.frame == 14 ? 8U
                                                                           : 0U));
}
bool ActorValhalla(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 4)
        return SDL_SetError("Valhalla Knight requires its original NO2 owner");
    switch (a->state.id)
    {
    case 0x34:
        return Body(s, a);
    case 0x35:
        return Child(s, a);
    case 0x36:
        return Parts(s, a);
    default:
        return SDL_SetError("Valhalla Knight received an unsupported actor identity");
    }
}
