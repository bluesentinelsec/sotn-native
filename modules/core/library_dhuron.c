// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/lib/e_dhuron.c from the pinned reconstruction. Private LIB data
// supplies original animation, hitbox, sensor and fragment profiles.
#include "actors_internal.h"
static Sint16 ByteSigned(Uint8 v)
{
    return (Sint16)(v < 128 ? v : (Sint32)v - 256);
}
static bool Box(SotnActors *s, Actor *a, Uint16 frame, size_t lookup, size_t boxes)
{
    SotnDataView index = {0}, box = {0};
    if (frame >= 28 || !Slice(s->nz0, lookup + frame, 1, &index) || index.bytes[0] >= 6 ||
        !Slice(s->nz0, boxes + index.bytes[0] * 4U, 4, &box))
        return SDL_SetError("Dhuron hitbox pose or selector exceeds its private profile");
    if (ByteSigned(box.bytes[2]) < 0 || ByteSigned(box.bytes[3]) < 0)
        return SDL_SetError("Dhuron hitbox has negative private dimensions");
    a->state.hitbox_x = ByteSigned(box.bytes[0]);
    a->state.hitbox_y = ByteSigned(box.bytes[1]);
    a->state.hitbox_width = box.bytes[2];
    a->state.hitbox_height = box.bytes[3];
    return true;
}
static Actor *Owner(SotnActors *s, Actor *a)
{
    if (a->enemy_state.owner_slot >= SOTN_STAGE_ACTOR_SLOTS)
        return NULL;
    Actor *root = &s->slots[a->enemy_state.owner_slot];
    return root->state.id == 0x33 && root->state.generation != 0 &&
                   root->state.generation == a->enemy_state.owner_generation
               ? root
               : NULL;
}
static void Child(Actor *a)
{
    a->state.x = Pixel(a->state.x) * 65536;
    a->state.y = Pixel(a->state.y) * 65536;
    a->state.facing_left = false;
}
static void Substep(Actor *a, Uint16 step)
{
    a->substep = step;
    a->pose = a->remaining = 0;
}
static bool Weapon(SotnActors *s, Actor *a)
{
    if (!a->state.step)
        return ActorInitialize(s, a, 0x8a8);
    Actor *root = Owner(s, a);
    if (!root)
    {
        a->remove = true;
        return true;
    }
    a->state.facing_left = root->state.facing_left;
    a->state.x = Pixel(root->state.x) * 65536 + (Sint32)((Uint32)a->state.x & 65535U);
    a->state.y = Pixel(root->state.y) * 65536 + (Sint32)((Uint32)a->state.y & 65535U);
    if (!Box(s, a, root->state.frame, 0x2a48, 0x2a30))
        return false;
    if (a->dhuron.charged)
    {
        if (a->attack > SDL_MAX_UINT16 / 2)
            return SDL_SetError("Dhuron charged attack would overflow original width");
        a->attack_element = (Uint16)((a->attack_element | 0x4000U) & ~0x40U);
        a->attack = (Uint16)(a->attack * 2U);
        a->dhuron.charged = false;
    }
    return true;
}
static bool Fragment(SotnActors *s, Actor *a)
{
    if (a->state.parameters >= 7)
        return SDL_SetError("Dhuron fragment exceeds its seven private profiles");
    SotnDataView profile = {0};
    if (!Slice(s->nz0, 0x2a64 + a->state.parameters * 12U, 12, &profile))
        return false;
    const Sint32 vx = Signed32(profile.bytes), vy = Signed32(profile.bytes + 4),
                 rotation = Signed16(profile.bytes + 8), duration = Signed16(profile.bytes + 10);
    if (vx < -8 * 65536 || vx > 8 * 65536 || vy < -8 * 65536 || vy > 8 * 65536 ||
        rotation < -4096 || rotation > 4096 || duration < 1 || duration > 512)
        return SDL_SetError("Dhuron fragment motion exceeds its bounded private profile");
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x8b4))
            return false;
        if (a->state.priority > 511 - a->state.parameters)
            return SDL_SetError("Dhuron fragment priority overflows the original table");
        a->state.rotated = true;
        a->state.frame = (Uint16)(a->state.parameters + 27);
        a->state.priority = (Uint16)(a->state.priority + a->state.parameters);
        a->vx = a->state.facing_left ? vx : -vx;
        a->vy = vy;
        a->dhuron.timer = (Uint16)duration;
        break;
    case 1: {
        const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
        if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32 ||
            a->vy > SDL_MAX_SINT32 - 10240)
            return SDL_SetError("Dhuron fragment motion exceeds native fixed-point bounds");
        a->state.x = (Sint32)x;
        a->state.y = (Sint32)y;
        a->vy += 10240;
        a->state.rotation = (Sint16)Wrap16((Sint32)a->state.rotation + rotation);
        if (!a->dhuron.timer)
            return SDL_SetError("Dhuron fragment timer underflow");
        if (--a->dhuron.timer == 0)
        {
            if (!ActorReplaceExplosion(s, a, 0) || !ActorLibrarySound(s, a, 0x691))
                return false;
        }
        if (a->state.parameters == 6)
        {
            SotnCollider c = {0};
            if (!ActorQuery(s, Pixel(a->state.x), Wrap16((Sint64)Pixel(a->state.y) + 4), &c))
                return false;
            if (c.effects)
            {
                if (!ActorLibrarySound(s, a, 0x6a5))
                    return false;
                ShiftY(a, c.tile_top);
                a->vy = -a->vy;
                a->vy -= a->vy / 4;
            }
        }
        break;
    }
    default:
        return SDL_SetError("Dhuron fragment entered an invalid step");
    }
    return true;
}
static bool Death(SotnActors *s, Actor *a)
{
    a->state.hitbox_state = 0;
    for (Uint16 i = 0; i < 7; ++i)
    {
        Actor *p = NULL;
        if (!ActorSpawn(s, a, 0x36, i, 160, 192, &p))
            return false;
        if (!p)
            break;
        Child(p);
        p->state.facing_left = a->state.facing_left;
    }
    Actor *effect = NULL;
    if (!ActorSpawn(s, a, 2, 3, 160, 192, &effect))
        return false;
    if (effect)
        Child(effect);
    if (!ActorLibrarySound(s, a, 0x629))
        return false;
    a->remove = true;
    return true;
}
static bool Body(SotnActors *s, Actor *a)
{
    if ((a->state.flags & 0x100U) && a->state.step < 8)
        SetStep(a, 8);
    Uint8 animation = 0;
    Uint16 collision = 0;
    switch (a->state.step)
    {
    case 0: {
        if (!ActorInitialize(s, a, 0x89c))
            return false;
        Actor *weapon = NULL;
        if (!ActorLibrarySpawnAt(s, a, 0x34, 0, (Uint32)a->slot + 1, &weapon))
            return false;
        Child(weapon);
        break;
    }
    case 1:
        if (!ActorGround(s, a, 0x2954, &collision))
            return false;
        if (collision & 1U)
            SetStep(a, 2);
        break;
    case 2:
        a->state.facing_left = Side(a, s->player) == 0;
        if (Distance(a, s->player) < 112)
            SetStep(a, 3);
        break;
    case 3:
        if (!a->substep)
        {
            a->state.facing_left = Side(a, s->player) == 0;
            a->dhuron.direction =
                Distance(a, s->player) < 80 ? !a->state.facing_left : a->state.facing_left;
            ++a->substep;
        }
        if (a->state.frame == 6 || a->state.frame == 1)
            a->vx = 0;
        if (!ActorWalk(s, a, 0x2964, &collision) || !ActorAnimate(s, a, 0x296c, &animation))
            return false;
        if (!animation)
        {
            a->state.facing_left = Side(a, s->player) == 0;
            a->dhuron.direction = a->state.facing_left;
            const Sint64 distance = Distance(a, s->player);
            if (!a->dhuron.charged && Pixel(a->state.y) - s->camera_y < 256 && distance > 80 &&
                !(ActorRandom(s) & 7U))
            {
                SetStep(a, 5);
                break;
            }
            if (distance < 64)
                SetStep(a, a->dhuron.charged ? 6 : 4);
        }
        a->vx = a->dhuron.direction ? 163840 : -163840;
        break;
    case 4:
        if (!a->substep)
        {
            a->vx = 0;
            ++a->substep;
        }
        if (!ActorAnimate(s, a, 0x2984, &animation))
            return false;
        if (!animation)
        {
            SetStep(a, 3);
            a->pose = 6;
        }
        if (!ActorWalk(s, a, 0x2964, &collision))
            return false;
        a->vx -= a->vx / 16;
        if (a->pose == 5)
        {
            a->vx = a->dhuron.direction ? 3 * 65536 : -3 * 65536;
            if (!a->remaining && !ActorLibrarySound(s, a, 0x66d))
                return false;
        }
        break;
    case 6:
        if (a->substep == 0)
        {
            if (!ActorAnimate(s, a, 0x29c4, &animation))
                return false;
            if (!animation)
            {
                a->dhuron.timer = 8;
                Substep(a, 1);
            }
        }
        else if (a->substep == 1)
        {
            if (!ActorAnimate(s, a, 0x29d4, &animation))
                return false;
            if (!animation)
            {
                if (!a->dhuron.timer)
                    return SDL_SetError("Dhuron rapid attack timer underflow");
                --a->dhuron.timer;
                if ((Side(a, s->player) != 0) == a->state.facing_left ||
                    Distance(a, s->player) > 96 || !a->dhuron.timer)
                    Substep(a, 2);
            }
            if (!a->remaining && a->pose == 6 && !ActorLibrarySound(s, a, 0x66d))
                return false;
        }
        else if (a->substep == 2)
        {
            if (!ActorAnimate(s, a, 0x29f0, &animation))
                return false;
            if (!animation)
                SetStep(a, 3);
        }
        else
            return SDL_SetError("Dhuron rapid attack substep is invalid");
        break;
    case 5:
        if (a->substep == 0)
        {
            if (!ActorAnimate(s, a, 0x29a4, &animation))
                return false;
            if (!animation)
            {
                a->dhuron.timer = 32;
                ++a->substep;
            }
            if (a->pose == 10 && !a->remaining)
            {
                if (!ActorLibrarySound(s, a, 0x665))
                    return false;
                if (!a->state.priority)
                    return SDL_SetError("Dhuron charge priority is invalid");
                Actor *effect = NULL;
                if (!ActorSpawn(s, a, 0x35, 0, 160, 192, &effect))
                    return false;
                if (effect)
                {
                    Child(effect);
                    effect->state.facing_left = a->state.facing_left;
                    effect->state.priority = (Uint16)(a->state.priority - 1);
                    effect->enemy_state.owner_slot = a->slot;
                    effect->enemy_state.owner_generation = a->state.generation;
                }
            }
        }
        else if (a->substep == 1)
        {
            if (!a->dhuron.timer)
                return SDL_SetError("Dhuron charge wait underflow");
            if (--a->dhuron.timer == 0)
            {
                if ((Uint32)a->slot + 1 >= SOTN_STAGE_ACTOR_SLOTS)
                    return SDL_SetError("Dhuron charge lacks its adjacent weapon");
                Actor *weapon = &s->slots[a->slot + 1];
                if (weapon->state.id != 0x34 ||
                    weapon->enemy_state.owner_generation != a->state.generation)
                    return SDL_SetError("Dhuron charge weapon generation is stale");
                a->dhuron.charged = true;
                weapon->dhuron.charged = true;
                a->state.blend_mode |= 0x30;
                SetStep(a, 3);
            }
        }
        else
            return SDL_SetError("Dhuron charge substep is invalid");
        break;
    case 8:
        return Death(s, a);
    default:
        return SDL_SetError("Dhuron entered an unsupported gameplay step");
    }
    if (a->dhuron.charged)
        a->state.palette = (s->game_frame & 2U) ? 0x201 : 0x202;
    return Box(s, a, a->state.frame, 0x2a14, 0x29fc);
}
bool ActorLibraryDhuron(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 2 || a->state.id < 0x33 || a->state.id > 0x36)
        return SDL_SetError("Dhuron dispatch requires original Library identities");
    if (a->state.id == 0x34)
        return Weapon(s, a);
    if (a->state.id == 0x35)
        return ActorLibraryDhuronCharge(s, a);
    return a->state.id == 0x36 ? Fragment(s, a) : Body(s, a);
}
