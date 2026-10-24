// SPDX-License-Identifier: AGPL-3.0-or-later
// nz0/e_cannon.c, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
#include "actors_internal.h"
bool ActorCannon(SotnActors *s, Actor *a)
{
    switch (a->state.id)
    {
    case 0x1c:
        switch (a->state.step)
        {
        case 0:
            if (!ActorInitialize(s, a, 0xbf8) || !ActorPrimitives(s, a, 1))
                return false;
            a->state.hitbox_width = 4;
            a->state.hitbox_height = 20;
            a->state.hitbox_state = 2;
            a->state.priority = 0x70;
            if (Pixel(s->player.x) - s->camera_x < 128)
                a->state.hitbox_state = 0;
            break;
        case 1:
            if (a->hit_params)
            {
                a->vx = -4 * 65536;
                a->state.step = 2;
            }
            break;
        case 2:
            a->state.x += a->vx;
            a->state.y += a->vy;
            a->vx -= a->vx / 16;
            // Preserve the original comparison-of-a-boolean, rather than changing game behavior.
            if (a->vx < 8192)
                a->state.step = 3;
            break;
        case 3:
            s->cannon_fired = true;
            break;
        default:
            return SDL_SetError("Cannon lever step is invalid");
        }
        if (s->world->flags[SOTN_LAB_CANNON])
            a->state.hitbox_state = 0;
        return ActorSprite(s, a, 0, a->state.x - 4 * 65536, a->state.y - 20 * 65536, 104, 128, 8,
                           40, 15, 9);
    case 0x1d:
        switch (a->state.step)
        {
        case 0:
            if (!ActorInitialize(s, a, 0xbf8) || !ActorPrimitives(s, a, 2))
                return false;
            a->state.priority = 0x70;
            a->enemy_state.initial_x = a->state.x;
            if (!ActorSprite(s, a, 1, a->state.x - 8 * 65536, 120 * 65536, 40, 128, 64, 40, 15, 9))
                return false;
            if (s->world->flags[SOTN_LAB_CANNON])
                a->state.step = 3;
            break;
        case 1:
            if (s->cannon_fired)
            {
                Actor *shot = NULL;
                if (!ActorSound(s, 0x6ac) || !ActorExplosion(s, a, 0x13) ||
                    !ActorSpawnAt(s, a, 0x1e, 0, (Uint32)a->slot + 1, &shot))
                    return false;
                a->vx = 8 * 65536;
                a->state.step = 2;
            }
            break;
        case 2:
            a->state.x = a->enemy_state.initial_x + a->vx;
            a->state.y += a->vy;
            a->vx -= a->vx / 8;
            if (a->vx < 8192)
                a->state.step = 3;
            break;
        case 3:
            break;
        default:
            return SDL_SetError("Cannon step is invalid");
        }
        return ActorSprite(s, a, 0, a->state.x - 24 * 65536, a->state.y - 16 * 65536, 40, 168, 56,
                           32, 15, 9);
    case 0x1e:
        if (a->state.step == 0)
        {
            if (!ActorInitialize(s, a, 0xbf8))
                return false;
            a->state.animation_set = 2;
            a->state.frame = 1;
            a->state.palette = 0x81af;
            a->state.priority = 0x6f;
            a->vx = -8 * 65536;
        }
        a->state.x += a->vx;
        a->state.y += a->vy;
        if (Pixel(a->state.x) < 112)
        {
            if (!ActorExplosion(s, a, 3))
                return false;
            s->world->flags[SOTN_LAB_CANNON] = 1;
            a->remove = true;
        }
        return true;
    case 0x1f:
        if (a->state.step == 0)
        {
            if (!ActorInitialize(s, a, 0xbf8) ||
                !ActorTilePatch(s, 0x46, 6, 1, 16,
                                0xed4 + (s->world->flags[SOTN_LAB_CANNON] ? 12 : 0)))
                return false;
            if (s->world->flags[SOTN_LAB_CANNON])
            {
                a->remove = true;
                return true;
            }
        }
        if (a->state.step == 1 && s->world->flags[SOTN_LAB_CANNON])
            a->state.step = 2;
        else if (a->state.step == 2)
        {
            if (!ActorSound(s, 0x644) || !ActorTilePatch(s, 0x46, 6, 1, 16, 0xee0))
                return false;
            a->remove = true;
        }
        return true;
    default:
        return SDL_SetError("Unknown cannon actor");
    }
}
