// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected LIB e_breakable.c and st_common.h:ReplaceBreakableWithItemDrop,
// plus e_misc.h:PlaySfxPositional at the pinned reconstruction. Tables stay private.
#include "actors_internal.h"
static bool Drop(SotnActors *s, Actor *a)
{
    a->permanent = true;
    if (!(s->world->relics[SOTN_RELIC_CUBE] & 2))
    {
        a->remove = true;
        return true;
    }
    const Uint16 item = a->state.parameters & 0xfffU;
    a->state.id = item < 128 ? 3 : 0xa;
    a->state.parameters = item < 128 ? item : (Uint16)(item - 128);
    a->state.step = a->pose = a->remaining = a->state.frame = 0;
    a->dynamic = true;
    a->timer = 16;
    return true;
}
static bool Breakable(SotnActors *s, Actor *a)
{
    const Uint16 type = (Uint16)(a->state.parameters >> 12);
    const Uint16 persistent = (Uint16)(type == 8 ? 3 : a->state.parameters & 0x1ffU);
    if (type >= 10 || (type >= 7 && persistent >= 11))
        return SDL_SetError("Library breakable has an invalid original type or persistent drop");
    SotnDataView animation = {0};
    if (!Slice(s->nz0, 0xba8 + type * 4U, 4, &animation))
        return false;
    const Uint32 address = Read32(animation.bytes);
    if (address < 0x80180000 || address & 1U)
        return SDL_SetError("Library breakable animation pointer is invalid");
    if (!a->state.step)
    {
        SotnDataView height = {0}, palette = {0}, bank = {0}, blend = {0}, texture = {0},
                     offset = {0};
        if (!Slice(s->nz0, 0xbd0 + type, 1, &height) ||
            !Slice(s->nz0, 0xbe8 + type * 2U, 2, &palette) ||
            !Slice(s->nz0, 0xbfc + type * 2U, 2, &bank) ||
            !Slice(s->nz0, 0xc10 + type, 1, &blend) ||
            !Slice(s->nz0, 0xc1c + type * 2U, 2, &texture) ||
            !Slice(s->nz0, 0xc30 + type * 2U, 2, &offset) || !ActorInitialize(s, a, 0x800))
            return false;
        if (height.bytes[0] > 127)
            return SDL_SetError("Library breakable hitbox exceeds its signed byte");
        a->state.priority = 128;
        if (type < 5)
        {
            Actor *helper = NULL;
            if (!ActorLibrarySpawnAt(s, a, 0x3f, type, (Uint32)a->slot + 1, &helper))
                return false;
            helper->state.priority = a->state.priority;
        }
        a->state.blend_mode = blend.bytes[0];
        a->state.hitbox_height = height.bytes[0];
        a->state.animation_set = Read16(bank.bytes);
        a->state.texture_offset = Read16(texture.bytes);
        a->state.palette = Read16(palette.bytes);
        a->state.hitbox_y = (Sint16)Signed16(offset.bytes);
    }
    Uint8 animated = 0;
    if (!ActorAnimate(s, a, address - 0x80180000, &animated))
        return false;
    if (!a->hit_params)
        return true;
    SotnDataView explosion = {0};
    if (!Slice(s->nz0, 0xbdc + type, 1, &explosion) || explosion.bytes[0] >= 5)
        return SDL_SetError("Library breakable explosion profile is invalid");
    if (!ActorExplosion(s, a, explosion.bytes[0]))
        return false;
    if (type == 0)
    {
        if (!ActorLibrarySound(s, a, 0x61c))
            return false;
    }
    else if (type == 2 || type == 3)
    {
        if (!ActorLibrarySound(s, a, 0x61d))
            return false;
    }
    else if (!ActorSound(s, type >= 7 ? 0x61d : 0x634))
        return false;
    if (type >= 7)
    {
        Actor *child = NULL;
        if (type == 9 && !ActorSpawn(s, a, 0x47, 256, 96, 128, &child))
            return false;
        if (!ActorSpawn(s, a, 0xc, persistent, 96, 128, &child))
            return false;
        a->permanent = a->remove = true;
        return true;
    }
    return Drop(s, a);
}
static bool Helper(SotnActors *s, Actor *a)
{
    if (a->state.parameters >= 5 || a->slot == 0)
        return SDL_SetError("Library lamp helper lacks its original type or adjacent owner");
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x830))
            return false;
        a->state.blend_mode = 0x30;
        a->state.animation_set = 0x8001;
    }
    Uint8 animation = 0;
    if (!ActorAnimate(s, a, a->state.parameters ? 0xb60 : 0xb58, &animation))
        return false;
    if (s->slots[a->slot - 1].state.id != 1)
        a->state.frame = a->state.parameters ? 14 : 13;
    return true;
}
// e_candle_table.c: original adjacent shadow becomes persistent item 1 when struck.
static bool ClearTableChild(SotnActors *s, Actor *a, bool dropping)
{
    const Uint32 slot = (Uint32)a->slot + 1U;
    if (slot >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Library candle table lacks its adjacent drop reservation");
    Actor *previous = &s->slots[slot];
    if (previous->state.generation)
    {
        SotnActorPlacement placement = {0};
        Uint32 placed_generation = 0;
        if (!SotnRoomActors_Get(s->spawns, slot, &placement, &placed_generation))
            return false;
        const bool shadow = previous->state.id == 0x3d && previous->state.parameters == 0x100;
        const bool pickup =
            !dropping && previous->state.id == 0xc && previous->state.parameters == 1;
        if (placed_generation || (!shadow && !pickup) ||
            previous->enemy_state.owner_slot != a->slot ||
            (dropping && previous->enemy_state.owner_generation != a->state.generation))
            return SDL_SetError("Library candle-table drop would overwrite another actor");
        ActorPrimitivesClear(s, previous);
        *previous = (Actor){0};
    }
    return true;
}
static bool TableDrop(SotnActors *s, Actor *a)
{
    Actor *drop = NULL, *explosion = NULL;
    if (!ClearTableChild(s, a, true) ||
        !ActorLibrarySpawnAt(s, a, 0xc, 1, (Uint32)a->slot + 1U, &drop) ||
        !ActorSpawn(s, a, 2, 2, 160, 192, &explosion))
        return false;
    drop->state.x = Pixel(a->state.x) * 65536;
    drop->state.y = Pixel(a->state.y) * 65536;
    drop->state.facing_left = false;
    if (explosion)
    {
        explosion->state.x = Pixel(a->state.x) * 65536;
        explosion->state.y = (Pixel(a->state.y) - 8) * 65536;
        explosion->state.facing_left = false;
    }
    a->state.hitbox_state = 0;
    a->state.step = 3;
    a->state.frame = 8;
    return true;
}
static bool CandleTable(SotnActors *s, Actor *a)
{
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x8fc))
            return false;
        a->state.priority = 0x74;
        if (a->state.parameters & 0x600U)
        {
            a->state.frame = a->state.parameters & 0x400U ? 17 : 18;
            a->state.step = 4;
            return true;
        }
        if (a->state.parameters & 0x100U)
        {
            a->state.blend_mode = 0x30;
            a->state.opacity_set = true;
            a->state.opacity = 192;
        }
        else
        {
            a->state.priority += 4;
            a->state.hitbox_width = 8;
            a->state.hitbox_height = 12;
            a->state.hitbox_x = 0;
            a->state.hitbox_y = -10;
            a->state.hitbox_state = 2;
            Actor *shadow = NULL;
            if (!ClearTableChild(s, a, false) ||
                !ActorLibrarySpawnAt(s, a, 0x3d, 0x100, (Uint32)a->slot + 1U, &shadow))
                return false;
            shadow->state.x = Pixel(a->state.x) * 65536;
            shadow->state.y = Pixel(a->state.y) * 65536;
            shadow->state.facing_left = false;
        }
    }
    switch (a->state.step)
    {
    case 1: {
        Uint8 animation = 0;
        const bool shadow = (a->state.parameters & 0x100U) != 0;
        if (!ActorAnimate(s, a, shadow ? 0x1a78 : 0x1a68, &animation))
            return false;
        if (!shadow && a->hit_flags)
            SetStep(a, 2);
        return true;
    }
    case 2:
        return TableDrop(s, a);
    case 3:
        a->state.frame = 8;
        return true;
    case 4:
        return true;
    default:
        return SDL_SetError("Library candle table entered an invalid original state");
    }
}
bool ActorLibraryBreakable(SotnActors *s, Actor *a)
{
    if (s == NULL || a == NULL || s->stage_id != 2)
        return SDL_SetError("Library breakables require their original stage service");
    if (a->state.id == 1)
        return Breakable(s, a);
    if (a->state.id == 0x3d)
        return CandleTable(s, a);
    if (a->state.id == 0x3f)
        return Helper(s, a);
    if (a->state.id != 0x47)
        return SDL_SetError("Library breakable dispatch received a foreign actor");
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, a->state.parameters & 256U ? 0x830 : 0x818))
            return false;
        if (a->state.parameters & 256U)
        {
            a->state.animation_set = 0x800c;
            a->state.texture_offset = 91;
            a->state.palette = 0x26c;
            a->state.frame = 21;
            a->state.priority = 106;
            a->state.step = 256;
        }
    }
    return true;
}
