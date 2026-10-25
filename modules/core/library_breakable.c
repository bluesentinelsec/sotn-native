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
bool ActorLibraryBreakable(SotnActors *s, Actor *a)
{
    if (s == NULL || a == NULL || s->stage_id != 2)
        return SDL_SetError("Library breakables require their original stage service");
    if (a->state.id == 1)
        return Breakable(s, a);
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
