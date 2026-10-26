// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted st/e_hammer.h and st/e_blade.h at sotn-decomp 7bc54d8157b6.
// Original construction, poses, enemy definitions and SDK tables stay private.
#include "giant_knights_internal.h"

Actor *ActorGiantOwner(SotnActors *s, const Actor *a)
{
    if (!s || !a || s->stage_id != 4 || !a->state.generation ||
        (a->state.id != 0x38 && a->state.id != 0x39 && a->state.id != 0x3d))
        return NULL;
    const Uint16 slot = a->enemy_state.owner_slot;
    if (slot >= SOTN_STAGE_ACTOR_SLOTS || a->slot <= slot || a->slot - slot >= 20)
        return NULL;
    Actor *root = &s->slots[slot];
    return (root->state.id == 0x37 || root->state.id == 0x3c) && root->state.generation &&
                   root->state.generation == a->enemy_state.owner_generation &&
                   root->giant.generation[a->slot - slot] == a->state.generation
               ? root
               : NULL;
}
void ActorGiantHit(SotnActors *s, Actor *root, const Actor *hit, Uint32 channel, Uint8 cooldown)
{
    if (!s || !root || !hit || s->stage_id != 4 ||
        (root->state.id != 0x37 && root->state.id != 0x3c) ||
        channel >= SDL_arraysize(root->cooldown))
        return;
    root->cooldown[channel] = cooldown;
    for (Uint16 i = 1; i < 20 && (Uint32)root->slot + i < SOTN_STAGE_ACTOR_SLOTS; ++i)
    {
        Actor *part = &s->slots[root->slot + i];
        if (part->state.id != 0x38 || ActorGiantOwner(s, part) != root)
            continue;
        part->cooldown[channel] = (Uint8)(cooldown + 1U);
        if (!(hit->state.flags & 0x400000U))
            part->stun = root->stun;
        if (root->state.flags & 0x100U)
        {
            part->state.flags |= 0x10c100U;
            part->state.flags &= ~0x20000000U;
        }
    }
}

bool GiantCreate(SotnActors *s, Actor *root, bool blade)
{
    if (!s || !root || s->stage_id != 4 || !root->state.generation ||
        root->state.id != (blade ? 0x3c : 0x37))
        return SDL_SetError("Giant Knight construction requires the matching active NO2 body");
    SotnDataView table = {0};
    const Uint16 count = blade ? 15 : 14, weapons = blade ? 2 : 1;
    const size_t at = blade ? 0x2c18 : 0x21ac;
    bool reserved[20] = {false};
    if (!Slice(s->nz0, at, (count + 1U) * 10U, &table) ||
        s->effect_serial > 0x7fffffffU - count - weapons)
        return SDL_SetError("Giant Knight cannot reserve its bounded private construction");
    // Validate every record and reservation before replacing any surviving child.
    for (Uint16 i = 0; i < count + weapons; ++i)
    {
        const Uint16 offset = i < count ? Read16(table.bytes + i * 10U) : (Uint16)(15 + i - count);
        const Uint16 parent = i < count ? Read16(table.bytes + i * 10U + 2U) : 5;
        if (!offset || offset >= 20 || parent >= 20 || offset == parent || reserved[offset] ||
            (i < count && Read16(table.bytes + i * 10U + 6U) > 255) ||
            (Uint32)root->slot + offset >= SOTN_STAGE_ACTOR_SLOTS)
            return SDL_SetError("Giant Knight construction has invalid or repeated joint indices");
        reserved[offset] = true;
        const Uint16 id = i < count ? 0x38 : blade ? 0x3d : 0x39;
        Actor *old = &s->slots[root->slot + offset];
        SotnActorPlacement placement = {0};
        Uint32 placed = 0;
        if (!SotnRoomActors_Get(s->spawns, root->slot + offset, &placement, &placed))
            return false;
        if (placed || (old->state.generation &&
                       (old->state.id != id || old->enemy_state.owner_slot != root->slot)))
            return SDL_SetError("Giant Knight joint reservation conflicts with another actor");
    }
    if (Read16(table.bytes + count * 10U))
        return SDL_SetError("Giant Knight construction has no expected private terminator");
    for (Uint16 i = 0; i < count; ++i)
    {
        const Uint16 parent = Read16(table.bytes + i * 10U + 2U);
        if (parent && !reserved[parent])
            return SDL_SetError("Giant Knight construction references an absent parent joint");
    }
    for (Uint16 i = 0; i < count + weapons; ++i)
    {
        const Uint8 *record = table.bytes + SDL_min(i, count) * 10U;
        const Uint16 offset = i < count ? Read16(record) : (Uint16)(15 + i - count);
        const Uint16 params = i < count ? (Uint16)(Read16(record + 6) + (blade ? 0x200U : 0U))
                              : blade   ? 19
                                        : 18;
        Actor *part = NULL;
        if (!ActorOwnedSpawnAt(s, root,
                               i < count ? 0x38
                               : blade   ? 0x3d
                                         : 0x39,
                               params, (Uint32)root->slot + offset, &part))
            return false;
        root->giant.generation[offset] = part->state.generation;
        part->state.x = Pixel(root->state.x) * 65536;
        part->state.y = Pixel(root->state.y) * 65536;
        part->state.facing_left = false;
        part->giant.length = i < count ? (Sint16)Signed16(record + 4) : 12;
        part->giant.parent = i < count ? Read16(record + 2) : offset == 16 ? 8 : 5;
        part->state.priority = (Uint16)(root->state.priority + (i < count ? Signed16(record + 8)
                                                                : offset == 16 ? -4
                                                                               : 3));
    }
    return true;
}
bool GiantDeparted(const SotnActors *s, const Actor *root, Uint16 offset)
{
    if (!s || !root || root->state.id != 0x3c || root->state.step != 24 || root->substep < 3 ||
        !root->giant.released || offset != 15 ||
        (Uint32)root->slot + offset >= SOTN_STAGE_ACTOR_SLOTS)
        return false;
    const Actor *sword = &s->slots[root->slot + offset];
    return sword->state.id != 0x3d || !sword->state.generation ||
           sword->state.generation != root->giant.generation[offset] ||
           sword->enemy_state.owner_slot != root->slot ||
           sword->enemy_state.owner_generation != root->state.generation;
}
bool GiantMotion(SotnActors *s, Actor *root, size_t list, size_t pose, Uint16 mode)
{
    return GiantPose(s, root, list, pose, 0) && GiantAdvance(s, root, list, mode);
}
bool GiantListed(SotnActors *s, Actor *root, size_t list, Uint16 index, Actor **part)
{
    SotnDataView item = {0};
    return (s && root && part && index < 20 && list <= s->nz0.size &&
            index * 2U <= s->nz0.size - list && Slice(s->nz0, list + index * 2U, 2, &item) &&
            Read16(item.bytes) && GiantPart(s, root, Read16(item.bytes), part)) ||
           SDL_SetError("Giant Knight required list entry is missing or invalid");
}
bool GiantMove(Actor *a, Sint32 gravity)
{
    if (!a)
        return SDL_SetError("Giant Knight movement requires an initialized actor");
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy,
                 vy = (Sint64)a->vy + gravity;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32 ||
        vy < SDL_MIN_SINT32 || vy > SDL_MAX_SINT32)
        return SDL_SetError("Giant Knight motion exceeds fixed-point bounds");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    a->vy = (Sint32)vy;
    return true;
}
bool GiantDisable(SotnActors *s, Actor *root, size_t list, bool launch)
{
    if (!s || !root || list > s->nz0.size || s->nz0.size - list < 40)
        return SDL_SetError("Giant Knight death list requires bounded private storage");
    for (Uint16 i = 0; i < 20; ++i)
    {
        SotnDataView item = {0};
        Actor *part = NULL;
        if (!Slice(s->nz0, list + i * 2U, 2, &item))
            return false;
        const Uint16 offset = Read16(item.bytes);
        if (!offset)
            return true;
        if (GiantDeparted(s, root, offset))
            continue;
        if (!GiantPart(s, root, offset, &part))
            return false;
        part->state.hitbox_state = 0;
        if (launch)
        {
            part->state.step = 24;
            part->substep = 0;
        }
    }
    return SDL_SetError("Giant Knight death list lacks a bounded terminator");
}
static bool Launch(SotnActors *s, Actor *a, bool timed)
{
    const Sint32 speed = (Sint32)(ActorRandom(s) & 31U) + 16;
    const Sint32 angle = Wrap16((Sint64)ActorRandom(s) * 6 + 0x900);
    SotnMathTrig trig = {0};
    if (!SotnMath_Trig(s->executable, angle, &trig))
        return false;
    a->vx = speed * trig.cosine / 2;
    a->vy = speed * trig.sine;
    if (timed)
        a->giant.timer = (Sint16)((ActorRandom(s) & 31U) + 32U);
    a->state.flags |= 0x80000000U;
    a->state.hitbox_state = 0;
    ++a->substep;
    return true;
}
static bool FootAttack(SotnActors *s, Actor *a)
{
    SotnDataView pointer = {0}, enemy = {0};
    SotnMathTrig trig = {0};
    if (!SotnMath_Trig(s->executable, a->state.rotation, &trig) ||
        !Slice(s->dra, 0x98, 4, &pointer))
        return false;
    const Uint32 address = Read32(pointer.bytes);
    if (address < 0x800a0000 || address % 4 ||
        !Slice(s->dra, address - 0x800a0000U + (a->giant.attack_active ? 0xc0U : 0xbeU) * 40U, 10,
               &enemy))
        return SDL_SetError("Giant Knight foot references an invalid private attack profile");
    a->attack = Read16(enemy.bytes + 6);
    a->attack_element = Read16(enemy.bytes + 8);
    a->state.hitbox_x = a->giant.attack_active ? (Sint16)ActorShift((Sint64)trig.sine * -8, 12) : 0;
    a->state.hitbox_y =
        a->giant.attack_active ? (Sint16)ActorShift((Sint64)trig.cosine * 8, 12) : 0;
    return true;
}
static bool Part(SotnActors *s, Actor *a)
{
    // Launched pieces become independent before the body leaves its slot.
    if (a->state.step == 24 && a->substep == 1)
    {
        if (!GiantMove(a, 8192) || a->giant.timer <= 0)
            return SDL_SetError("Giant Knight death piece has an invalid motion or timer");
        a->state.rotation = (Sint16)Wrap16((Sint32)a->state.rotation + a->giant.rotation_velocity);
        if (!--a->giant.timer)
            return ActorReplaceExplosion(s, a, 0);
        return true;
    }
    Actor *root = ActorGiantOwner(s, a), *parent = NULL;
    if (!root)
    {
        a->remove = true;
        return true;
    }
    switch (a->state.step)
    {
    case 0:
        if ((a->state.parameters >> 8) != (root->state.id == 0x3c ? 2 : 0) ||
            !ActorInitialize(s, a, root->state.id == 0x3c ? 0x958 : 0x928))
            return SDL_SetError("Giant Knight part has an invalid initialization profile");
        a->state.parameters &= 255U;
        a->state.frame = a->state.parameters;
        a->state.rotated = true;
        a->state.hitbox_width = a->state.hitbox_height = 6;
        break;
    case 1:
        a->state.rotation = a->giant.rotation;
        break;
    case 24:
        if (a->substep || !Launch(s, a, true))
            return SDL_SetError("Giant Knight part has an invalid launch state");
        break;
    default:
        return SDL_SetError("Giant Knight part entered an invalid state");
    }
    SotnCollider collider = {.top = 9};
    switch (a->state.parameters)
    {
    case 8:
    case 14:
        return GiantGround(s, root, a, &collider);
    case 9:
    case 15:
        if (!GiantPart(s, root, a->giant.parent, &parent))
            return false;
        if (parent->giant.terrain)
            a->giant.rotation = parent->giant.terrain == 2   ? 512
                                : parent->giant.terrain == 4 ? -512
                                                             : 0;
        return FootAttack(s, a);
    case 5:
    case 11:
        if (!GiantPart(s, root, a->giant.parent, &parent))
            return false;
        parent->giant.rotation = a->giant.rotation;
        break;
    }
    return true;
}
static bool Trail(SotnActors *s, Actor *a, bool initialize)
{
    if (initialize)
    {
        // Original US code dereferences NULL after exhaustion; optional graphics must not
        // remove a gameplay weapon. Preserve motion/contact and omit only its trail.
        if (!ActorPrimitives(s, a, 6))
        {
            SDL_ClearError();
            return true;
        }
        for (Uint16 i = 0; i < 6; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (!p)
                return false;
            p->state = (SotnActorPrimitive){.x = Pixel(a->state.x) * 65536,
                                            .y = Pixel(a->state.y) * 65536,
                                            .quad = true,
                                            .gouraud = true,
                                            .blend = true,
                                            .page = 32,
                                            .priority_set = true,
                                            .priority = a->state.priority};
            for (Uint16 j = 0; j < 4; ++j)
                p->state.vertex_red[j] = p->state.vertex_green[j] = p->state.vertex_blue[j] =
                    (Uint8)((i + (j >= 2 ? 1U : 0U)) * 10U);
        }
    }
    if (!a->state.primitive_count)
        return true;
    SotnMathTrig trig = {0};
    if (!SotnMath_Trig(s->executable,
                       a->state.facing_left ? -(Sint32)a->state.rotation : a->state.rotation,
                       &trig))
        return false;
    // Keep packed original integer endpoints in room space, independent of render count.
    for (Uint16 i = 0; i < 5; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i),
                       *next = ActorPrimitiveGet(s, a, (Uint16)(i + 1U));
        if (!p || !next)
            return false;
        p->state.x = next->state.x;
        p->state.y = next->state.y;
        SDL_memcpy(p->state.offset_x, next->state.offset_x, sizeof(p->state.offset_x));
        SDL_memcpy(p->state.offset_y, next->state.offset_y, sizeof(p->state.offset_y));
        p->state.active = a->giant.attack_active != 0;
    }
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 5);
    if (!p)
        return false;
    const Sint32 x = Pixel(a->state.x), y = Pixel(a->state.y);
    // Rebase the previous edge before adopting the new integer origin.
    for (Uint16 i = 0; i < 2; ++i)
    {
        p->state.offset_x[i + 2U] =
            (Sint16)Wrap16((Sint64)Pixel(p->state.x) + p->state.offset_x[i] - x);
        p->state.offset_y[i + 2U] =
            (Sint16)Wrap16((Sint64)Pixel(p->state.y) + p->state.offset_y[i] - y);
    }
    p->state.x = x * 65536;
    p->state.y = y * 65536;
    p->state.offset_x[0] = (Sint16)ActorShift((Sint64)trig.sine * -20, 12);
    p->state.offset_y[0] = (Sint16)ActorShift((Sint64)trig.cosine * 20, 12);
    p->state.offset_x[1] = (Sint16)ActorShift((Sint64)trig.sine * 4, 12);
    p->state.offset_y[1] = (Sint16)ActorShift((Sint64)trig.cosine * -4, 12);
    p->state.active = a->giant.attack_active != 0;
    return true;
}
static bool Weapon(SotnActors *s, Actor *a, bool blade)
{
    bool initialize = false;
    if (blade && a->giant.released)
        a->state.step = 8;
    if (a->state.step != 24 && a->state.step != 8 && !ActorGiantOwner(s, a))
    {
        a->remove = true;
        return true;
    }
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, blade ? 0x964 : 0x934))
            return false;
        a->state.rotated = true;
        a->state.hitbox_width = a->state.hitbox_height = blade ? 6 : 10;
        initialize = blade;
        SDL_FALLTHROUGH;
    case 1:
        a->state.rotation = a->giant.rotation;
        break;
    case 8:
        if (!blade)
            return SDL_SetError("Hammer weapon entered a Blade-only throw state");
        a->vx = a->state.facing_left ? 8 * 65536 : -8 * 65536;
        if (!GiantMove(a, 0))
            return false;
        a->state.flags |= 0x80000000U;
        break;
    case 24:
        if (blade)
        {
            a->state.flags |= 0x40000000U;
            a->state.hitbox_state = 0;
        }
        else if (!a->substep)
        {
            if (!Launch(s, a, false))
                return false;
        }
        else if (a->substep == 1)
        {
            if (!GiantMove(a, 8192))
                return false;
            (void)GiantToward(&a->state.rotation, 0x800, 0x20);
        }
        else
            return SDL_SetError("Hammer weapon entered an invalid death substate");
        break;
    default:
        return SDL_SetError("Giant Knight weapon entered an invalid state");
    }
    if (!blade && a->state.step == 24)
        return true;
    SotnMathTrig trig = {0};
    if (!SotnMath_Trig(s->executable, a->state.rotation, &trig))
        return false;
    a->state.hitbox_x = (Sint16)ActorShift((Sint64)trig.sine * (blade ? -13 : 52), 12);
    a->state.hitbox_y = (Sint16)ActorShift((Sint64)trig.cosine * (blade ? 13 : -52), 12);
    return !blade || Trail(s, a, initialize);
}
bool ActorGiant(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 4)
        return SDL_SetError("Giant Knights require their original NO2 owner");
    switch (a->state.id)
    {
    case 0x37:
        return GiantHammer(s, a);
    case 0x38:
        return Part(s, a);
    case 0x39:
        return Weapon(s, a, false);
    case 0x3c:
        return GiantBlade(s, a);
    case 0x3d:
        return Weapon(s, a, true);
    default:
        return SDL_SetError("Giant Knight dispatcher received an unsupported identity");
    }
}
