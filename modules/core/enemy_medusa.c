// SPDX-License-Identifier: AGPL-3.0-or-later
// st/e_medusa_head.h at sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Original spawn profiles, flight initialization and animation bytes stay private.
#include "actors_internal.h"
static bool Spawner(SotnActors *s, Actor *a)
{
    if (a->state.flags & 0x100)
    {
        a->remove = true;
        return true;
    }
    const Uint8 index = (Uint8)a->state.parameters;
    SotnDataView profile = {0};
    if (index >= 8 || !Slice(s->nz0, 0x33d0 + index * 12U, 12, &profile))
        return SDL_SetError("Medusa spawner profile exceeds its private table");
    const Sint32 minimum = Signed16(profile.bytes), maximum = Signed16(profile.bytes + 2),
                 delay = Signed16(profile.bytes + 4), count = Signed16(profile.bytes + 6),
                 chance = Signed16(profile.bytes + 8), priority = Signed16(profile.bytes + 10);
    if (minimum < 0 || maximum < minimum || maximum > 16384 || delay < 1 || delay > 4096 ||
        count < 1 || count > 32 || chance < 0 || chance > 16 || priority < 0 || priority > 0x1ff)
        return SDL_SetError("Medusa spawner profile has invalid window, cadence or pool bounds");
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x95c))
            return false;
        a->state.flags &= ~0x2000U;
    }
    const Sint32 player_y = Pixel(s->player.y) - s->camera_y;
    if (s->camera_y < minimum || s->camera_y > maximum || player_y < 32 || player_y >= 193)
        return true;
    if (a->timer)
    {
        --a->timer;
        return true;
    }
    Actor *head = NULL;
    if (!ActorSpawn(s, a, 0x5a, 0, 64, (Uint32)(64 + count), &head))
        return false;
    if (head != NULL)
    {
        // The libc roll is consumed even when the profile's yellow chance is zero.
        head->state.id = (SotnCombat_Random(&s->combat_random) & 15) < chance ? 0x5b : 0x5a;
        head->state.x = head->state.y = 0;
        head->state.facing_left = false;
        head->state.priority = (Uint16)priority;
        a->timer = (Uint16)delay;
    }
    else
        ++a->timer;
    return true;
}
bool ActorMedusa(SotnActors *s, Actor *a)
{
    if (a->state.id == 0x59)
        return Spawner(s, a);
    if (a->state.id != 0x5a && a->state.id != 0x5b)
        return SDL_SetError("Medusa dispatch received a foreign actor ID");
    if (a->state.id == 0x5b)
        a->state.parameters = 1;
    if (a->state.flags & 0x100)
        return ActorReplaceExplosion(s, a, 0);
    if (a->state.step)
    {
        Uint8 animation = 0;
        if (!ActorAnimate(s, a, 0x3430, &animation))
            return false;
        if (a->state.frame < 1 || a->state.frame > 2)
            return SDL_SetError("Medusa animation exceeds its original flight frames");
        if (a->vy > 0)
            a->state.frame = (Uint16)(a->state.frame + 2);
        a->vy += a->gravity;
        if (SDL_abs(a->vy) >= 163840)
            a->gravity = -a->gravity;
        a->state.x += a->vx;
        a->state.y += a->vy;
        return true;
    }
    if (!ActorInitialize(s, a, a->state.parameters ? 0xb54 : 0xb48))
        return false;
    a->state.y = Pixel(s->player.y) * 65536;
    const Sint32 player_x = Pixel(s->player.x) - s->camera_x;
    Uint32 side = 0;
    if (player_x < 80)
        side = 1;
    else if (player_x < 177)
        side = (SotnCombat_Random(&s->combat_random) & 3) == 0 ? (Uint32)s->player.facing_left
                                                               : (Uint32)!s->player.facing_left;
    SotnDataView motion = {0};
    if (!Slice(s->nz0, 0x3438 + side * 8, 8, &motion))
        return false;
    const Sint32 x = Signed16(motion.bytes + 4), facing = Signed16(motion.bytes + 6);
    a->vx = Signed32(motion.bytes);
    if (a->vx < -2 * 65536 || a->vx > 2 * 65536 || x < -128 || x > 384 || facing < 0 || facing > 1)
        return SDL_SetError("Medusa entry position or flight speed exceeds its private bounds");
    a->state.x = (s->camera_x + x) * 65536;
    a->state.facing_left = facing != 0;
    a->vy = 163840 - (Sint32)((ActorRandom(s) & 15U) * 163840U / 8U);
    a->gravity = a->vy > 0 ? -10240 : 10240;
    return true;
}
