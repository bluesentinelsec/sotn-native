// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/e_thornweed_corpseweed.h at the pinned reconstruction.
// All original hitbox, animation and sensor arrays remain in private LIB data.
#include "actors_internal.h"
bool ActorLibraryPlantBox(SotnActors *s, Actor *a)
{
    SotnDataView index = {0}, box = {0};
    if (a->state.frame >= 29 || !Slice(s->nz0, 0x2c18 + a->state.frame, 1, &index) ||
        index.bytes[0] >= 11 || !Slice(s->nz0, 0x2bec + index.bytes[0] * 4U, 4, &box))
        return SDL_SetError("Library plant hitbox exceeds its private pose profile");
    if (box.bytes[2] > 127 || box.bytes[3] > 127)
        return SDL_SetError("Library plant hitbox has negative private dimensions");
    a->state.hitbox_x = (Sint16)(box.bytes[0] < 128 ? box.bytes[0] : (Sint32)box.bytes[0] - 256);
    a->state.hitbox_y = (Sint16)(box.bytes[1] < 128 ? box.bytes[1] : (Sint32)box.bytes[1] - 256);
    a->state.hitbox_width = box.bytes[2];
    a->state.hitbox_height = box.bytes[3];
    return true;
}
static bool Animation(SotnActors *s, Actor *a, Uint16 kind, Uint8 *out)
{
    SotnDataView pointer = {0};
    if (kind >= 4 || !Slice(s->nz0, 0x2bdc + kind * 4U, 4, &pointer))
        return false;
    const Uint32 address = Read32(pointer.bytes);
    if (address < 0x80180000 || address & 1U)
        return SDL_SetError("Library plant animation pointer is invalid");
    return ActorAnimate(s, a, address - 0x80180000, out);
}
static bool GrownAttributes(SotnActors *s, Actor *a)
{
    SotnDataView api = {0}, enemy = {0};
    if (!Slice(s->dra, 0x98, 4, &api))
        return false;
    const Uint32 address = Read32(api.bytes);
    if (address < 0x800a0000 || address % 4 ||
        !Slice(s->dra, address - 0x800a0000 + 158U * 40U, 40, &enemy))
        return SDL_SetError("Grown Corpseweed references an invalid private enemy definition");
    // The original changes enemyId, preserving copied contact power and current HP.
    a->enemy = (SotnCombatEnemy){.defense = (Sint16)Signed16(enemy.bytes + 10),
                                 .weaknesses = Read16(enemy.bytes + 14),
                                 .strengths = Read16(enemy.bytes + 16),
                                 .immunes = Read16(enemy.bytes + 18),
                                 .absorbs = Read16(enemy.bytes + 20)};
    a->level = Read16(enemy.bytes + 22);
    a->experience = Read16(enemy.bytes + 24);
    return true;
}
static bool Root(SotnActors *s, Actor *a)
{
    if (a->state.parameters > 1)
        return SDL_SetError("Thornweed form exceeds the two original plant profiles");
    if ((a->state.flags & 0x100U) && a->state.step < 6)
    {
        if (a->state.parameters && a->plant.spawned)
            SetStep(a, 6);
        else
        {
            Actor *effect = NULL;
            if (!ActorSpawn(s, a, 2, 0, 160, 192, &effect))
                return false;
            if (effect)
            {
                effect->state.x = Pixel(a->state.x) * 65536;
                effect->state.y = Wrap16((Sint64)Pixel(a->state.y) - 4) * 65536;
                effect->state.facing_left = false;
            }
            if (!ActorSound(s, 0x691))
                return false;
            a->remove = true;
            return true;
        }
    }
    Uint8 animation = 0;
    Uint16 grounded = 0;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x8cc))
            return false;
        a->state.frame = a->state.parameters ? 9 : 1;
        break;
    case 1:
        if (!ActorGround(s, a, 0x2b80, &grounded))
            return false;
        if (grounded & 1U)
            SetStep(a, 2);
        break;
    case 2:
        if (Distance(a, s->player) < 80)
            SetStep(a, 3);
        break;
    case 3:
        if (!Animation(s, a, a->state.parameters, &animation))
            return false;
        if (!animation)
        {
            a->plant.timer = 64;
            SetStep(a, 4);
        }
        break;
    case 4:
        if (!a->substep)
        {
            if (a->state.parameters)
            {
                if (!a->plant.timer)
                    return SDL_SetError("Corpseweed spawn wait underflow");
                if (--a->plant.timer == 0)
                {
                    Actor *head = NULL;
                    if (!ActorLibrarySpawnAt(s, a, 0x3b, 0, (Uint32)a->slot + 1, &head))
                        return false;
                    head->state.x = Pixel(a->state.x) * 65536;
                    head->state.y = Pixel(a->state.y) * 65536;
                    head->state.facing_left = Side(a, s->player) != 0;
                    if (!GrownAttributes(s, a))
                        return false;
                    a->plant.child_generation = head->state.generation;
                    a->plant.spawned = true;
                    ++a->substep;
                }
            }
            else
                ++a->substep;
        }
        if (!Animation(s, a, (Uint16)(a->state.parameters + 2), &animation))
            return false;
        break;
    case 6:
        if (!a->substep)
        {
            if ((Uint32)a->slot + 1 >= SOTN_STAGE_ACTOR_SLOTS)
                return SDL_SetError("Corpseweed root lacks an adjacent head");
            Actor *head = &s->slots[a->slot + 1];
            if (head->state.id == 0x3b && head->state.generation == a->plant.child_generation)
                head->state.flags |= 0x100U;
            a->state.hitbox_state = 0;
            a->state.opacity = 128;
            a->state.opacity_set = true;
            ++a->substep;
        }
        if (!a->state.opacity)
            return SDL_SetError("Corpseweed root fade underflow");
        if (--a->state.opacity == 0)
        {
            Actor *effect = NULL;
            if (!ActorSpawn(s, a, 2, 2, 160, 192, &effect))
                return false;
            if (effect)
            {
                effect->state.x = Pixel(a->state.x) * 65536;
                effect->state.y = Wrap16((Sint64)Pixel(a->state.y) - 12) * 65536;
                effect->state.facing_left = false;
            }
            if (!ActorSound(s, 0x691))
                return false;
            a->remove = true;
            return true;
        }
        break;
    default:
        return SDL_SetError("Thornweed entered an unsupported gameplay step");
    }
    return ActorLibraryPlantBox(s, a);
}
bool ActorLibraryPlant(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 2 || a->state.id < 0x3a || a->state.id > 0x3c)
        return SDL_SetError("Plant dispatch requires original Library identities");
    if (a->state.id == 0x3b)
        return ActorLibraryCorpseweed(s, a);
    return a->state.id == 0x3c ? ActorLibraryPlantProjectile(s, a) : Root(s, a);
}
