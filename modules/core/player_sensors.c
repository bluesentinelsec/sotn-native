// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted from sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5,
// src/dra/6BF64.c, src/dra/70394.c, and the normal-form path of
// src/dra/692E8.c:CheckStageCollision.
#include "player_internal.h"

#define PLAYER_STATUS_BAT_FORM 1U
#define PLAYER_STATUS_MIST_FORM 2U
#define PLAYER_STATUS_TRANSFORM 7U

static Sint16 Wrap16(Sint64 value)
{
    const Uint16 bits = (Uint16)(Uint64)value;
    return bits <= 32767 ? (Sint16)bits : (Sint16)((Sint32)bits - 65536);
}

static void Query(SotnPlayer *player, Sint32 x, Sint32 y, SotnCollider *out)
{
    if (player->failed)
    {
        *out = (SotnCollider){0};
        return;
    }
    if (!SotnCollision_Query(&player->map, x, y, out))
    {
        player->failed = true;
        *out = (SotnCollider){0};
    }
}
static void CheckFloor(SotnPlayer *player)
{
    SotnCollider collider = {0};

    Sint32 var_s8 = 0;
    Sint16 var_s6 = Wrap16(0);
    Sint16 argX = Wrap16(0);
    Sint16 argY = Wrap16(0);
    Uint32 var_s2 = 0;
    Uint32 var_s1 = 0;

    Sint16 sp5e = 0;
    Sint32 i = 0;
    Sint16 *yPosPtr = &player->y.parts.high;
    Sint16 *xPosPtr = &player->x.parts.high;
    Sint32 *vram_ptr = &player->contact;
    Sint32 sp30 = 0;

    if (player->fixed_stage)
    {
        *vram_ptr = 1;
        return;
    }
    if ((player->vy == 0) && (player->previous_contact & 1) &&
        !(player->status & (PLAYER_STATUS_BAT_FORM | PLAYER_STATUS_MIST_FORM)))
    {
        sp5e = 4;
    }
    else
    {
        i = 1;
    }
    for (; i < 4; i++, sp5e = 0)
    {
        if (player->floor[i].effects & SOTN_EFFECT_SOLID_FROM_BELOW)
        {
            continue;
        }
        if (!((player->floor[i].effects & SOTN_EFFECT_SIDE) || (player->vy >= 0) ||
              (player->status & PLAYER_STATUS_TRANSFORM) || (SDL_abs(player->vx) >= 131072)))
        {
            continue;
        }
        var_s2 = player->floor[i].effects &
                 (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SOLID);
        if ((var_s2 == SOTN_EFFECT_SOLID) || (var_s2 & SOTN_EFFECT_SLOPE_CEILING))
        {
            if ((player->status & (PLAYER_STATUS_BAT_FORM | PLAYER_STATUS_MIST_FORM)) &&
                !(var_s2 & SOTN_EFFECT_SOLID))
            {
                continue;
            }
            argX = Wrap16(*xPosPtr + player->floor_sensors[i].x);
            argY = Wrap16(*yPosPtr + player->floor_sensors[i].y);
            argY = Wrap16((Sint64)argY + player->floor[i].top - 1);
            Query(player, argX, argY, &collider);
            if ((player->status & PLAYER_STATUS_MIST_FORM) &&
                (collider.effects & SOTN_EFFECT_MIST_ONLY))
            {
                collider.effects &= ~SOTN_EFFECT_BLOCK;
            }
            var_s1 = collider.effects;
            if (!(var_s1 & SOTN_EFFECT_SOLID))
            {
                if (((player->floor[i].effects != SOTN_EFFECT_SOLID) &&
                     (player->floor[i].effects !=
                      (SOTN_EFFECT_SOLID_FROM_ABOVE | SOTN_EFFECT_SOLID))) ||
                    (player->vy >= 0))
                {
                    if (var_s2 & SOTN_EFFECT_SLOPE_CEILING)
                    {
                        *yPosPtr = Wrap16((Sint64)*yPosPtr + (sp5e + player->floor[i].tile_top));
                    }
                    else
                    {
                        *yPosPtr = Wrap16((Sint64)*yPosPtr + (sp5e + player->floor[i].top));
                    }
                    *vram_ptr |= 1;
                    return;
                }
                continue;
            }
            if ((var_s1 & (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_BLOCK)) ==
                (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SOLID))
            {
                if (i < 2)
                {
                    *vram_ptr |= ((var_s1 & (SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_QUARTER |
                                             SOTN_EFFECT_SLOPE_HALF)) +
                                  0x7FFF + 2);
                    *yPosPtr = Wrap16((Sint64)*yPosPtr + player->floor[i].tile_top + collider.top -
                                      1 + sp5e);
                    return;
                }
                if ((i == 2) && ((var_s1 & (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT |
                                            SOTN_EFFECT_SOLID)) ==
                                 (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SOLID)))
                {
                    player->floor[2].effects = var_s1;
                    player->floor[2].tile_bottom = player->floor[2].tile_top;
                }
                if ((i == 3) &&
                    ((var_s1 &
                      (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SOLID)) ==
                     (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SOLID)))
                {
                    player->floor[3].effects = var_s1;
                    player->floor[3].tile_bottom = player->floor[3].tile_top;
                }
            }
        }
        if ((var_s2 != (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SOLID)) || (i >= 2))
        {
            continue;
        }
        *vram_ptr |= player->floor[i].effects &
                     (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_QUARTER |
                      SOTN_EFFECT_SLOPE_HALF | SOTN_EFFECT_SOLID);
        *yPosPtr = Wrap16((Sint64)*yPosPtr + (sp5e + player->floor[i].top));
        return;
    }
    if (player->floor[1].effects & SOTN_EFFECT_SINK)
    {
        *vram_ptr |= 0x11;
        if ((player->frame_number & 3) == 0)
        {
            *yPosPtr = Wrap16((Sint64)*yPosPtr + 1);
        }
        return;
    }
    if (player->floor[1].effects & SOTN_EFFECT_WATER)
    {
        *vram_ptr |= 0x80;
    }
    if (player->vy < 0)
    {
        return;
    }
    argX = Wrap16(*xPosPtr + player->floor_sensors[0].x);
    argY = Wrap16(*yPosPtr + player->floor_sensors[0].y + 10);
    Query(player, argX, argY, &collider);
    if ((collider.effects & (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SOLID)) != SOTN_EFFECT_NONE)
    {
        return;
    }

    for (i = 2; i < 4; i++)
    {
        if ((player->floor[3].effects & SOTN_EFFECT_SLOPE_FLOOR) &&
            (player->floor[2].effects & SOTN_EFFECT_SLOPE_FLOOR))
        {
            return;
        }
        var_s2 = player->floor[i].effects;
        sp30 = ((var_s2 &
                 (SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_QUARTER | SOTN_EFFECT_SLOPE_HALF)) +
                0x7FFF + 2);
        if (!(var_s2 & SOTN_EFFECT_SLOPE_FLOOR))
        {
            continue;
        }
        if (i == 2)
        {
            var_s1 = SOTN_EFFECT_SLOPE_LEFT;
            var_s6 = Wrap16(player->floor[2].tile_left);
            var_s8 = var_s6 + 8;
        }
        else
        {
            var_s1 = 0;
            var_s6 = Wrap16(player->floor[3].tile_right);
            var_s8 = 8 - var_s6;
        }
        if ((var_s2 & SOTN_EFFECT_SLOPE_LEFT) == var_s1)
        {
            argX = Wrap16(var_s6 + (*xPosPtr + player->floor_sensors[i].x));
            argY = Wrap16(*yPosPtr + player->floor_sensors[i].y);
            Query(player, argX, argY, &collider);
            if (collider.effects & SOTN_EFFECT_SOLID)
            {
                *yPosPtr = Wrap16((Sint64)*yPosPtr + (collider.top));
                *vram_ptr |= sp30;
                return;
            }
            continue;
        }
        if (var_s8 <= 0)
        {
            continue;
        }
        if (!(var_s2 & 1))
        {
            continue;
        }
        argX = Wrap16(var_s6 + (*xPosPtr + player->floor_sensors[i].x));
        argY = Wrap16(*yPosPtr + player->floor_sensors[i].y + player->floor[i].tile_bottom);
        Query(player, argX, argY, &collider);
        if (collider.effects & SOTN_EFFECT_SOLID)
        {
            *yPosPtr = Wrap16((Sint64)*yPosPtr + ((collider.top + player->floor[i].tile_bottom)));
            *vram_ptr |= sp30;
            return;
        }
    }
}

static void CheckCeiling(SotnPlayer *player)
{
    SotnCollider collider = {0};
    Sint32 temp_fp = 0;
    Uint32 var_s2 = 0;
    Sint32 temp_v1 = 0;
    Uint32 var_s1 = 0;
    Sint16 var_a1 = Wrap16(0);

    Sint16 argX = Wrap16(0);
    Sint16 argY = Wrap16(0);

    Sint32 i = 1;

    Sint16 *yPosPtr = &player->y.parts.high;
    Sint16 *xPosPtr = &player->x.parts.high;
    Sint32 *vram_ptr = &player->contact;

    if (player->fixed_stage)
    {
        return;
    }
    for (; i < 4; i++)
    {
        if (player->ceiling[i].effects & SOTN_EFFECT_SOLID_FROM_ABOVE)
        {
            continue;
        }
        if ((player->status & PLAYER_STATUS_MIST_FORM) &&
            (collider.effects & SOTN_EFFECT_MIST_ONLY))
        {
            collider.effects &= ~SOTN_EFFECT_BLOCK;
        }
        var_s2 = player->ceiling[i].effects &
                 (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SOLID);
        if ((var_s2 == SOTN_EFFECT_SOLID) || (var_s2 & SOTN_EFFECT_SLOPE_FLOOR))
        {
            if ((player->status & (PLAYER_STATUS_BAT_FORM | PLAYER_STATUS_MIST_FORM)) &&
                !(var_s2 & SOTN_EFFECT_SOLID))
            {
                continue;
            }

            argX = Wrap16(*xPosPtr + player->ceiling_sensors[i].x);
            argY = Wrap16(*yPosPtr + player->ceiling_sensors[i].y + player->ceiling[i].tile_bottom +
                          1);
            Query(player, argX, argY, &collider);
            // If in mist form, and collider is mist, then disable collision on
            // that collider!
            if (player->status & PLAYER_STATUS_MIST_FORM &&
                collider.effects & SOTN_EFFECT_MIST_ONLY)
            {
                collider.effects &= ~SOTN_EFFECT_BLOCK;
            }
            var_s1 = collider.effects;
            if (!(var_s1 & SOTN_EFFECT_SOLID))
            {
                if ((player->ceiling[i].effects != SOTN_EFFECT_SOLID) || (player->vy <= 0))
                {
                    *vram_ptr |= 2;
                    if (!(*vram_ptr & 1) && ((player->previous_contact &
                                              (SOTN_EFFECT_SOLID_FROM_ABOVE | SOTN_EFFECT_SOLID)) !=
                                             (SOTN_EFFECT_SOLID_FROM_ABOVE | SOTN_EFFECT_SOLID)))
                    {
                        if (player->ceiling[i].effects & SOTN_EFFECT_SLOPE_FLOOR)
                        {
                            *yPosPtr = Wrap16((Sint64)*yPosPtr + (player->ceiling[i].tile_bottom));
                        }
                        else
                        {
                            *yPosPtr = Wrap16((Sint64)*yPosPtr + (player->ceiling[i].bottom));
                        }
                    }
                    return;
                }
                continue;
            }
            if ((var_s1 & (SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_BLOCK)) ==
                (SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SOLID))
            {
                if (i < 2)
                {
                    *vram_ptr |= (SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SIDE |
                                  ((var_s1 >> 4) & (0x0400 | 0x0200 | 0x0100)));
                    if (!(*vram_ptr & 1))
                    {
                        *yPosPtr = Wrap16((Sint64)*yPosPtr + player->ceiling[i].tile_bottom +
                                          collider.bottom + 1);
                    }
                    return;
                }
                if ((i == 2) && ((var_s1 & (SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_CEILING |
                                            SOTN_EFFECT_SOLID)) ==
                                 (SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SOLID)))
                {
                    player->floor[2].effects = var_s1;
                    player->floor[2].tile_top = player->floor[2].tile_bottom;
                }
                if ((i == 3) &&
                    ((var_s1 &
                      (SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SOLID)) ==
                     (SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SOLID)))
                {
                    player->floor[3].effects = var_s1;
                    player->floor[3].tile_top = player->floor[3].tile_bottom;
                }
            }
            if ((var_s1 & SOTN_EFFECT_SLOPE_CEILING) == SOTN_EFFECT_NONE)
            {
                *vram_ptr |= (SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SIDE |
                              ((var_s1 >> 4) & (0x0400 | 0x0200 | 0x0100)));
                if (!(*vram_ptr & 1))
                {
                    *yPosPtr = Wrap16((Sint64)*yPosPtr + player->ceiling[i].tile_bottom +
                                      collider.bottom + 1);
                }
                return;
            }
        }
        if ((var_s2 == (SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SOLID)) && (i < 2))
        {
            *vram_ptr |= (SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SIDE |
                          ((player->ceiling[i].effects >> 4) & (0x0400 | 0x0200 | 0x0100)));
            if (!(*vram_ptr & 1))
            {
                *yPosPtr = Wrap16((Sint64)*yPosPtr + (player->ceiling[i].bottom));
            }
            return;
        }
    }
    if (player->vy > 0)
    {
        return;
    }
    argX = Wrap16(*xPosPtr + player->ceiling_sensors[0].x);
    argY = Wrap16((*yPosPtr + player->ceiling_sensors[0].y) - 10);
    Query(player, argX, argY, &collider);
    if ((collider.effects & SOTN_EFFECT_SOLID) != SOTN_EFFECT_NONE)
    {
        return;
    }
    for (i = 2; i < 4; i++)
    {
        if ((player->ceiling[3].effects & SOTN_EFFECT_SLOPE_CEILING) &&
            (player->ceiling[2].effects & SOTN_EFFECT_SLOPE_CEILING))
        {
            return;
        }
        var_s2 = player->ceiling[i].effects;
        temp_fp = ((var_s2 >> 4) & (0x0400 | 0x0200 | 0x0100)) +
                  (SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SIDE);
        if (var_s2 & SOTN_EFFECT_SLOPE_CEILING)
        {
            if (i == 2)
            {
                var_s1 = SOTN_EFFECT_SLOPE_LEFT;
                var_a1 = Wrap16(player->ceiling[2].tile_left);
                temp_v1 = var_a1 + 8;
            }
            else
            {
                var_s1 = 0;
                var_a1 = Wrap16(player->ceiling[3].tile_right);
                temp_v1 = 8 - var_a1;
            }
            if ((var_s2 & SOTN_EFFECT_SLOPE_LEFT) == var_s1)
            {
                argX = Wrap16(var_a1 + (*xPosPtr + player->ceiling_sensors[i].x));
                argY = Wrap16(*yPosPtr + player->ceiling_sensors[i].y);
                Query(player, argX, argY, &collider);
                if (collider.effects & SOTN_EFFECT_SOLID)
                {
                    *vram_ptr |= temp_fp;
                    if (!(*vram_ptr & 1))
                    {
                        *yPosPtr = Wrap16((Sint64)*yPosPtr + (collider.bottom));
                    }
                    return;
                }
            }
            else if ((temp_v1 > 0) && (var_s2 & 1))
            {
                argX = Wrap16(var_a1 + (*xPosPtr + player->ceiling_sensors[i].x));
                argY =
                    Wrap16(*yPosPtr + player->ceiling_sensors[i].y + player->ceiling[i].tile_top);
                Query(player, argX, argY, &collider);
                if (collider.effects & SOTN_EFFECT_SOLID)
                {
                    if (!(*vram_ptr & 1))
                    {
                        *yPosPtr = Wrap16((Sint64)*yPosPtr + collider.bottom +
                                          player->ceiling[i].tile_top);
                    }
                    *vram_ptr |= temp_fp;
                    return;
                }
            }
        }
    }
}

static void CheckWallRight(SotnPlayer *player)
{
    SotnCollider collider = {0};
    Sint32 temp_s0 = 0;
    Sint32 i = 0;

    Sint16 argX = Wrap16(0);
    Sint16 argY = Wrap16(0);

    Sint16 *yPosPtr = &player->y.parts.high;
    Sint16 *xPosPtr = &player->x.parts.high;
    Sint32 *vram_ptr = &player->contact;

    if (player->fixed_stage)
    {
        return;
    }
    temp_s0 = player->previous_contact & (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT |
                                          SOTN_EFFECT_SLOPE_CEILING | 0x0400 | SOTN_EFFECT_BLOCK);
    if ((temp_s0 == (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_BLOCK)) ||
        (temp_s0 == (SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_BLOCK)) ||
        (temp_s0 == (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_BLOCK)))
    {
        *vram_ptr |= 4;
        return;
    }

    for (i = 0; i < 7; i++)
    {
        temp_s0 = player->wall[i].effects & (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT |
                                             SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_BLOCK);
        if ((temp_s0 == (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SOLID)) ||
            (temp_s0 == (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SIDE |
                         SOTN_EFFECT_SOLID)) ||
            (temp_s0 == (SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SOLID)) ||
            (temp_s0 == (SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SIDE |
                         SOTN_EFFECT_SOLID)) ||
            (temp_s0 == (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_BLOCK)) ||
            (temp_s0 == (SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_BLOCK)) ||
            (temp_s0 == SOTN_EFFECT_BLOCK))
        {
            argX = Wrap16(*xPosPtr + player->wall_sensors[i].x + player->wall[i].tile_left - 1);
            argY = Wrap16(*yPosPtr + player->wall_sensors[i].y);
            Query(player, argX, argY, &collider);
            if ((collider.effects & SOTN_EFFECT_SOLID) == SOTN_EFFECT_NONE)
            {
                *vram_ptr |= 4;
                *xPosPtr = Wrap16((Sint64)*xPosPtr + (player->wall[i].tile_left));
                return;
            }
        }

        if (!(*vram_ptr & 1))
        {
            if ((temp_s0 & (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT |
                            SOTN_EFFECT_SLOPE_CEILING)) == SOTN_EFFECT_SLOPE_FLOOR &&
                (i != 0) &&
                ((player->wall[0].effects & SOTN_EFFECT_SLOPE_CEILING) ||
                 !(player->wall[0].effects &
                   (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SIDE))))
            {
                *vram_ptr |= 4;
                *xPosPtr = Wrap16((Sint64)*xPosPtr + (player->wall[i].tile_left));
                return;
            }
            if (((temp_s0 & (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT |
                             SOTN_EFFECT_SLOPE_CEILING)) == SOTN_EFFECT_SLOPE_CEILING) &&
                (i != 6) &&
                ((player->wall[6].effects & SOTN_EFFECT_SLOPE_FLOOR) ||
                 !(player->wall[6].effects &
                   (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SIDE))))
            {
                *vram_ptr |= 4;
                *xPosPtr = Wrap16((Sint64)*xPosPtr + (player->wall[i].tile_left));
                return;
            }
        }
    }
}

static void CheckWallLeft(SotnPlayer *player)
{
    SotnCollider collider = {0};
    Sint32 temp_s0 = 0;
    Sint32 i = 0;
    Sint16 argX = Wrap16(0);
    Sint16 argY = Wrap16(0);

    Sint16 *yPosPtr = &player->y.parts.high;
    Sint16 *xPosPtr = &player->x.parts.high;
    Sint32 *vram_ptr = &player->contact;

    if (player->fixed_stage)
    {
        return;
    }
    temp_s0 = player->previous_contact & (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT |
                                          SOTN_EFFECT_SLOPE_CEILING | 0x0400 | SOTN_EFFECT_BLOCK);
    if ((temp_s0 == (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_BLOCK)) ||
        (temp_s0 == (SOTN_EFFECT_SLOPE_CEILING | 0x0400 | SOTN_EFFECT_BLOCK)) ||
        (temp_s0 == (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_CEILING |
                     0x0400 | SOTN_EFFECT_BLOCK)))
    {
        *vram_ptr |= 8;
        return;
    }
    for (i = 7; i < 7 * 2; i++)
    {
        temp_s0 = player->wall[i].effects & (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT |
                                             SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_BLOCK);
        if ((temp_s0 == (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SOLID)) ||
            (temp_s0 == (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_BLOCK)) ||
            (temp_s0 == (SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SOLID)) ||
            (temp_s0 == (SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_BLOCK)) ||
            (temp_s0 == (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SIDE |
                         SOTN_EFFECT_SOLID)) ||
            (temp_s0 == (SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SIDE |
                         SOTN_EFFECT_SOLID)) ||
            (temp_s0 == SOTN_EFFECT_BLOCK))
        {
            argX = Wrap16(*xPosPtr + player->wall_sensors[i].x + player->wall[i].tile_right + 1);
            argY = Wrap16(*yPosPtr + player->wall_sensors[i].y);
            Query(player, argX, argY, &collider);
            if ((collider.effects & SOTN_EFFECT_SOLID) == SOTN_EFFECT_NONE)
            {
                *vram_ptr |= 8;
                *xPosPtr = Wrap16((Sint64)*xPosPtr + (player->wall[i].tile_right));
                return;
            }
        }
        if (!(*vram_ptr & 1))
        {
            if (((temp_s0 &
                  (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_CEILING)) ==
                 (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT)) &&
                (i != 7) &&
                ((player->wall[7].effects & SOTN_EFFECT_SLOPE_CEILING) ||
                 !(player->wall[7].effects &
                   (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SIDE))))
            {
                *vram_ptr |= 8;
                *xPosPtr = Wrap16((Sint64)*xPosPtr + (player->wall[i].tile_right));
                return;
            }
            if (((temp_s0 &
                  (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_CEILING)) ==
                 (SOTN_EFFECT_SLOPE_LEFT | SOTN_EFFECT_SLOPE_CEILING)) &&
                (i != 13) &&
                ((player->wall[7 + 6].effects & SOTN_EFFECT_SLOPE_FLOOR) ||
                 !(player->wall[7 + 6].effects &
                   (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SLOPE_CEILING | SOTN_EFFECT_SIDE))))
            {
                *vram_ptr |= 8;
                *xPosPtr = Wrap16((Sint64)*xPosPtr + (player->wall[i].tile_right));
                return;
            }
        }
    }
}

Uint8 SotnPlayer_TightSpace(SotnPlayer *player)
{
    static const Sint16 heights[] = {0, -8, -16, -22};
    static const Sint16 horizontal[] = {0, 4, -4};
    for (Uint8 i = 0; i < 4; ++i)
    {
        for (size_t j = 0; j < SDL_arraysize(horizontal); ++j)
        {
            SotnCollider collider = {0};
            Query(player, player->x.parts.high + horizontal[j], player->y.parts.high + heights[i],
                  &collider);
            if ((collider.effects & (SOTN_EFFECT_SOLID_FROM_ABOVE | SOTN_EFFECT_SOLID)) ==
                SOTN_EFFECT_SOLID)
                return (Uint8)(i + 1);
        }
    }
    return 0;
}

void SotnPlayer_Collide(SotnPlayer *player)
{
    const Sint16 floor_y[] = {29, 25, 25, 25};
    const Sint16 wall_y[] = {24, 17, 9, 1, -7, -14, -21};
    const Sint16 crouch_y[] = {24, 17, 9, 5, 5, 1, 1};
    const Sint16 horizontal[] = {0, 0, 4, -4};
    const bool crouching = player->step == SOTN_PLAYER_CROUCH;
    for (int i = 0; i < 4; ++i)
    {
        player->floor_sensors[i] = (PlayerSensor){horizontal[i], floor_y[i]};
        player->ceiling_sensors[i] = (PlayerSensor){horizontal[i], crouching ? 0 : -22};
    }
    for (int i = 0; i < 7; ++i)
    {
        const Sint16 y = crouching ? crouch_y[i] : wall_y[i];
        player->wall_sensors[i] = (PlayerSensor){7, y};
        player->wall_sensors[i + 7] = (PlayerSensor){-7, y};
    }
    player->previous_contact = player->contact;
    player->contact = 0;
    Sint32 speed = player->vx;
    const Sint32 slope = player->previous_contact & 0xf000;
    if (player->vx < 0 && !(player->previous_contact & 8))
    {
        if (slope == 0xc000)
            speed = speed * 10 / 16;
        if (slope == 0xd000)
            speed = speed * 13 / 16;
        player->x.value += speed;
    }
    if (player->vx > 0 && !(player->previous_contact & 4))
    {
        if (slope == 0x8000)
            speed = speed * 10 / 16;
        if (slope == 0x9000)
            speed = speed * 13 / 16;
        player->x.value += speed;
    }
    player->y.value += player->vy;
    for (int i = 0; i < 4; ++i)
    {
        const Sint32 x = player->x.parts.high + player->floor_sensors[i].x;
        const Sint32 y = player->y.parts.high + player->floor_sensors[i].y;
        Query(player, x, y, &player->floor[i]);
        if (player->timers[7] && (player->floor[i].effects & SOTN_EFFECT_SOLID_FROM_ABOVE))
        {
            SotnCollider below = {0};
            Query(player, x, y + 12, &below);
            if (!(below.effects & SOTN_EFFECT_SOLID))
                player->floor[i].effects = 0;
        }
    }
    CheckFloor(player);
    for (int i = 0; i < 4; ++i)
        Query(player, player->x.parts.high + player->ceiling_sensors[i].x,
              player->y.parts.high + player->ceiling_sensors[i].y, &player->ceiling[i]);
    CheckCeiling(player);
    if (((player->contact & 1) && player->vy >= 0) || ((player->contact & 2) && player->vy <= 0))
        player->y.parts.low = 0;
    for (int i = 0; i < 14; ++i)
        Query(player, player->x.parts.high + player->wall_sensors[i].x,
              player->y.parts.high + player->wall_sensors[i].y, &player->wall[i]);
    CheckWallRight(player);
    CheckWallLeft(player);
    if (((player->contact & 4) && player->vx > 0) || ((player->contact & 8) && player->vx < 0))
        player->x.parts.low = 0;
    if ((player->contact & 0x8000) || !(player->floor[1].effects & SOTN_EFFECT_SOLID) ||
        !(player->floor[2].effects & SOTN_EFFECT_SOLID) ||
        !(player->floor[3].effects & SOTN_EFFECT_SOLID))
        player->contact |= 0x20;
}
