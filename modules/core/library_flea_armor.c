// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/e_flea_armor.h and e_misc.h at the pinned reconstruction.
// Original timelines, sensors, debris vectors and hitboxes remain private LIB data.
#include "actors_internal.h"
static void Position(Actor *a, Sint32 x, Sint32 y)
{
    a->state.x = Wrap16(x) * 65536 + (Sint32)((Uint32)a->state.x & 65535U);
    a->state.y = Wrap16(y) * 65536 + (Sint32)((Uint32)a->state.y & 65535U);
}
static void Child(Actor *a)
{
    a->state.x = Pixel(a->state.x) * 65536;
    a->state.y = Pixel(a->state.y) * 65536;
    a->state.facing_left = false;
}
static bool Hitbox(SotnActors *s, Actor *a)
{
    if (!a->state.step && !ActorInitialize(s, a, 0x9c8))
        return false;
    if (!a->slot)
        return SDL_SetError("Flea Armor attack lacks an adjacent owner");
    Actor *parent = &s->slots[a->slot - 1];
    if (parent->state.id != 0x4a || parent->state.generation != a->enemy_state.owner_generation)
    {
        a->remove = true;
        return true;
    }
    const Uint32 index = parent->state.frame == 9 ? 1 : 0;
    SotnDataView offsets = {0}, size = {0};
    if (!Slice(s->nz0, 0x2dd4 + index * 4, 4, &offsets) ||
        !Slice(s->nz0, 0x2ddc + index * 4, 4, &size))
        return false;
    const Uint16 width = Read16(size.bytes), height = Read16(size.bytes + 2);
    if (width > 128 || height > 128)
        return SDL_SetError("Flea Armor attack dimensions exceed their private profile");
    a->state.hitbox_x = (Sint16)Signed16(offsets.bytes);
    a->state.hitbox_y = (Sint16)Signed16(offsets.bytes + 2);
    a->state.hitbox_width = (Uint8)width;
    a->state.hitbox_height = (Uint8)height;
    a->state.facing_left = parent->state.facing_left;
    a->state.hitbox_state = parent->state.hitbox_state;
    Position(a, Pixel(parent->state.x), Pixel(parent->state.y));
    return true;
}
static bool BreakArmor(SotnActors *s, Actor *a)
{
    if (!ActorLibrarySound(s, a, 0x6ca))
        return false;
    Actor *last = NULL;
    for (Uint16 i = 0; i < 6; ++i)
    {
        Actor *part = NULL;
        if (!ActorSpawn(s, a, 0x4a, 0, 160, 192, &part))
            return false;
        last = part;
        if (!part)
            continue;
        SotnDataView vector = {0};
        if (!Slice(s->nz0, 0x2da4 + i * 8U, 8, &vector))
            return false;
        const Sint32 vx = Signed32(vector.bytes), vy = Signed32(vector.bytes + 4);
        if (vx < -8 * 65536 || vx > 8 * 65536 || vy < -8 * 65536 || vy > 8 * 65536)
            return SDL_SetError("Flea Armor debris exceeds its original velocity bounds");
        Child(part);
        part->state.animation_set = a->state.animation_set;
        part->state.texture_offset = a->state.texture_offset;
        part->state.palette = a->state.palette;
        part->state.priority = a->state.priority;
        part->state.flags = 0xcd002000U;
        part->state.facing_left = a->state.facing_left;
        part->state.frame = (Uint16)(i + 10);
        part->vx = a->state.facing_left ? vx : -vx;
        part->vy = vy;
        part->state.step = 9;
    }
    if (last)
        last->state.rotated = true;
    Actor *explosion = NULL;
    if (!ActorSpawn(s, a, 2, 1, 160, 192, &explosion))
        return false;
    if (explosion)
        Child(explosion);
    a->vx = 0;
    a->state.flags = 0x08000000U;
    a->state.rotated = false;
    a->state.scale_x = a->state.scale_y = 0;
    a->state.hit_points = SDL_MAX_SINT16;
    a->enemy_state.timer = 16;
    a->state.step = 0;
    a->state.id = 0x4c;
    return true;
}
static bool Body(SotnActors *s, Actor *a)
{
    if (s->library_enemy_flags & 1U)
        a->state.flags |= 0x100U;
    if (a->state.step < 8 && (a->state.flags & 0x100U))
        a->state.step = 8;
    Uint8 animation = 0;
    Uint16 collision = 0;
    switch (a->state.step)
    {
    case 0: {
        if (!ActorInitialize(s, a, 0x9bc))
            return false;
        Actor *weapon = NULL;
        if (!ActorLibrarySpawnAt(s, a, 0x4b, 0, (Uint32)a->slot + 1, &weapon))
            return false;
        Child(weapon);
        a->state.blend_mode |= 0x30;
        break;
    }
    case 1:
        if (!ActorGround(s, a, 0x2d8c, &collision))
            return false;
        if (collision & 1U)
            ++a->state.step;
        break;
    case 2:
        if (!ActorAnimate(s, a, 0x2d78, &animation))
            return false;
        a->state.facing_left = Side(a, s->player) == 0;
        if (Distance(a, s->player) < 112)
        {
            a->vx = a->state.facing_left ? 2 * 65536 : -2 * 65536;
            a->vy = -32768;
            a->enemy_state.counter = 0;
            a->pose = a->remaining = 0;
            a->state.step = 3;
        }
        break;
    case 3:
        if (!a->vy && a->enemy_state.counter)
        {
            if (!ActorAnimate(s, a, 0x2d84, &animation) || !ActorWalk(s, a, 0x2d9c, &collision))
                return false;
            if (++a->enemy_state.counter2 % 4 == 0)
            {
                if (!ActorOuterDust(s, a, 1, 1, 0, 16, 3, a->state.facing_left ? -2 : 2))
                    return false;
                a->enemy_state.counter2 = 0;
            }
            if (!(ActorRandom(s) & 31U))
            {
                a->enemy_state.counter = 0;
                a->vy = -2 * 65536;
            }
            else if (Distance(a, s->player) < 64)
                SetStep(a, 4);
            if (collision & 0x80U)
            {
                if (collision & 15U)
                    a->state.facing_left = !a->state.facing_left;
                SetStep(a, 4);
            }
        }
        else
        {
            if (!ActorGround(s, a, 0x2d8c, &collision))
                return false;
            if (collision & 1U)
            {
                a->vx = a->state.facing_left ? 2 * 65536 : -2 * 65536;
                a->enemy_state.counter = 1;
                return true;
            }
        }
        break;
    case 4: {
        if (!a->substep)
        {
            a->vx = a->state.facing_left ? 2 * 65536 : -2 * 65536;
            if (ActorRandom(s) & 1U)
            {
                a->vy = -229376;
                a->vx /= 4;
                if (!ActorLibrarySound(s, a, 0x64a))
                    return false;
            }
            else
            {
                a->vy = -7 * 65536;
                if (!ActorLibrarySound(s, a, 0x649))
                    return false;
            }
            a->enemy_state.flag = false;
            ++a->substep;
        }
        const Sint32 x = Wrap16((Sint64)Pixel(a->state.x) + (a->vx > 0 ? 16 : a->vx < 0 ? -16 : 0));
        const Sint32 y = Pixel(a->state.y);
        SotnCollider collider = {0};
        if (!ActorQuery(s, x, y, &collider))
            return false;
        if (collider.effects & SOTN_EFFECT_SOLID)
            a->vx = 0;
        if (a->vy < 0)
        {
            if (!ActorQuery(s, x, Wrap16((Sint64)y - 16), &collider))
                return false;
            if (collider.effects & SOTN_EFFECT_SOLID)
                a->vy = 0;
        }
        if (!ActorGround(s, a, 0x2d8c, &collision))
            return false;
        if (collision & 1U)
        {
            a->substep = 0;
            a->state.step = 2;
        }
        else
        {
            if (a->vy < 0)
                a->state.frame = 7;
            if (a->vy > -4096)
            {
                a->state.frame = 8;
                if (!a->enemy_state.flag)
                {
                    if (!ActorLibrarySound(s, a, 0x6c8))
                        return false;
                    a->enemy_state.flag = true;
                }
            }
            if (a->vy > 4096)
                a->state.frame = 9;
        }
        break;
    }
    case 8:
        return BreakArmor(s, a);
    case 9: {
        const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
        if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32 ||
            a->vy > SDL_MAX_SINT32 - 16384)
            return SDL_SetError("Flea Armor debris exceeded bounded fixed-point motion");
        a->state.x = (Sint32)x;
        a->state.y = (Sint32)y;
        a->vy += 16384;
        a->state.rotation = (Sint16)Wrap16((Sint32)a->state.rotation + 256);
        break;
    }
    default:
        return SDL_SetError("Flea Armor entered an unsupported gameplay step");
    }
    return true;
}
bool ActorLibraryFleaArmor(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 2 || (a->state.id != 0x4a && a->state.id != 0x4b))
        return SDL_SetError("Flea Armor dispatch requires original Library identities");
    return a->state.id == 0x4b ? Hitbox(s, a) : Body(s, a);
}
