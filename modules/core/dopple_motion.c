// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted from sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5,
// src/boss/dop_collision.h, dop_check_st_collision.h and bo4/doppleganger.c.
#include <SDL3/SDL.h>
#include <sotn/dopple_motion.h>

typedef struct MotionFrame
{
    SotnDoppleMotion motion;
    SotnCollisionMap map;
    Sint32 px, py;
    bool failed;
} MotionFrame;

static Sint32 Wrap16(Sint64 value)
{
    const Uint16 bits = (Uint16)(Uint64)value;
    return bits <= 32767 ? bits : (Sint32)bits - 65536;
}
static Sint32 Wrap32(Sint64 value)
{
    const Uint32 bits = (Uint32)(Uint64)value;
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint32 Pixel(Sint32 value)
{
    return value >= 0 ? value / 65536 : (Sint32)(-((-(Sint64)value + 65535) / 65536));
}
static void Query(MotionFrame *frame, Sint32 x, Sint32 y, SotnCollider *out)
{
    *out = (SotnCollider){0};
    if (!frame->failed && !SotnCollision_Query(&frame->map, Wrap16(x), Wrap16(y), out))
        frame->failed = true;
}

static void CheckFloor(MotionFrame *frame)
{
    SotnCollider collider = {0};
    Sint32 temp_v1 = 0;
    Uint32 effects = 0;
    Sint32 var_a1 = 0;
    Sint32 posX = 0;
    Sint32 posY = 0;
    Uint32 maskedEffects = 0;

    Sint32 sp5e = 0;
    Sint32 i = 0;
    Sint32 *dopPosY = &frame->py;
    Sint32 *dopPosX = &frame->px;
    Uint32 *vram_flag_p = &frame->motion.contact;
    Sint32 sp30 = 0;

    if ((frame->motion.vy == 0) && (frame->motion.previous_contact & 1) &&
        !(frame->motion.status & (0x40000U | 0x80U | 2U | 1U)))
    {
        sp5e = 4;
    }
    else
    {
        i = 1;
    }

    for (; i < 4; i++, sp5e = 0)
    {

        if (frame->motion.floor[i].effects & SOTN_EFFECT_SOLID_FROM_BELOW)
        {
            continue;
        }

        if (!((frame->motion.floor[i].effects & SOTN_EFFECT_SIDE) || (frame->motion.vy >= 0) ||
              (frame->motion.status & (1U | 2U)) || (SDL_abs(frame->motion.vx) >= 131072)))
        {
            continue;
        }
        effects = frame->motion.floor[i].effects & (0x8000U | 0x0800U | SOTN_EFFECT_SOLID);
        if ((effects == SOTN_EFFECT_SOLID) || (effects & 0x0800U))
        {
            if ((frame->motion.status & (1U | 2U)) && !(effects & SOTN_EFFECT_SOLID))
            {
                continue;
            }
            posX = Wrap16(*dopPosX + frame->motion.floor_sensors[i].x);
            posY = Wrap16(*dopPosY + frame->motion.floor_sensors[i].y);
            posY = Wrap16((Sint64)posY + (((Sint64)frame->motion.floor[i].top - 1)));
            Query(frame, posX, posY, &collider);
            if ((frame->motion.status & 2U) && (collider.effects & SOTN_EFFECT_MIST_ONLY))
            {
                collider.effects &= ~SOTN_EFFECT_BLOCK;
            }
            maskedEffects = collider.effects;
            if (!(maskedEffects & SOTN_EFFECT_SOLID))
            {
                if ((frame->motion.floor[i].effects != SOTN_EFFECT_SOLID) ||
                    (frame->motion.vy >= 0))
                {
                    if (effects & 0x0800U)
                    {
                        *dopPosY =
                            Wrap16((Sint64)*dopPosY + (sp5e + frame->motion.floor[i].tile_top));
                    }
                    else
                    {
                        *dopPosY =
                            Wrap16((Sint64)*dopPosY + ((Sint64)sp5e + frame->motion.floor[i].top));
                    }
                    *vram_flag_p |= 1;
                    return;
                }
                continue;
            }
            if ((maskedEffects & (0x8000U | SOTN_EFFECT_BLOCK)) == (0x8000U | SOTN_EFFECT_SOLID))
            {
                if (i < 2)
                {
                    *vram_flag_p |= ((maskedEffects & (0x4000U | 0x2000U | 0x1000U)) + 0x7FFF + 2);
                    *dopPosY = Wrap16((Sint64)*dopPosY + ((Sint64)frame->motion.floor[i].tile_top +
                                                          collider.top - 1 + sp5e));
                    return;
                }
                if ((i == 2) && ((maskedEffects & (0x8000U | 0x4000U | SOTN_EFFECT_SOLID)) ==
                                 (0x8000U | SOTN_EFFECT_SOLID)))
                {
                    frame->motion.floor[2].effects = maskedEffects;
                    frame->motion.floor[2].tile_bottom = frame->motion.floor[2].tile_top;
                }
                if ((i == 3) && ((maskedEffects & (0x8000U | 0x4000U | SOTN_EFFECT_SOLID)) ==
                                 (0x8000U | 0x4000U | SOTN_EFFECT_SOLID)))
                {
                    frame->motion.floor[3].effects = maskedEffects;
                    frame->motion.floor[3].tile_bottom = frame->motion.floor[3].tile_top;
                }
            }
        }
        if ((effects != (0x8000U | SOTN_EFFECT_SOLID)) || (i >= 2))
        {
            continue;
        }
        *vram_flag_p |= frame->motion.floor[i].effects &
                        (0x8000U | 0x4000U | 0x2000U | 0x1000U | SOTN_EFFECT_SOLID);
        *dopPosY = Wrap16((Sint64)*dopPosY + ((Sint64)sp5e + frame->motion.floor[i].top));
        return;
    }

    if (frame->motion.vy < 0)
    {
        return;
    }
    posX = Wrap16(*dopPosX + frame->motion.floor_sensors[0].x);
    posY = Wrap16(*dopPosY + frame->motion.floor_sensors[0].y + 10);
    Query(frame, posX, posY, &collider);
    if ((collider.effects & (0x8000U | SOTN_EFFECT_SOLID)) != SOTN_EFFECT_NONE)
    {
        return;
    }

    for (i = 2; i < 4; i++)
    {
        if ((frame->motion.floor[3].effects & 0x8000U) &&
            (frame->motion.floor[2].effects & 0x8000U))
        {
            return;
        }
        effects = frame->motion.floor[i].effects;
        sp30 = ((effects & (0x4000U | 0x2000U | 0x1000U)) + 0x7FFF + 2);
        if (!(effects & 0x8000U))
        {
            continue;
        }
        if (i == 2)
        {
            maskedEffects = 0x4000U;
            var_a1 = Wrap16(frame->motion.floor[2].tile_left);
            temp_v1 = var_a1 + 8;
        }
        else
        {
            maskedEffects = 0;
            var_a1 = Wrap16(frame->motion.floor[3].tile_right);
            temp_v1 = 8 - var_a1;
        }
        if ((effects & 0x4000U) == maskedEffects)
        {
            posX = Wrap16(var_a1 + (*dopPosX + frame->motion.floor_sensors[i].x));
            posY = Wrap16(*dopPosY + frame->motion.floor_sensors[i].y);
            Query(frame, posX, posY, &collider);
            if (collider.effects & SOTN_EFFECT_SOLID)
            {
                *dopPosY = Wrap16((Sint64)*dopPosY + (collider.top));
                *vram_flag_p |= sp30;
                return;
            }
            continue;
        }
        if (temp_v1 <= 0)
        {
            continue;
        }
        if (!(effects & 1))
        {
            continue;
        }
        posX = Wrap16(var_a1 + (*dopPosX + frame->motion.floor_sensors[i].x));
        posY = Wrap16(*dopPosY + frame->motion.floor_sensors[i].y +
                      frame->motion.floor[i].tile_bottom);
        Query(frame, posX, posY, &collider);
        if (collider.effects & SOTN_EFFECT_SOLID)
        {
            *dopPosY = Wrap16((Sint64)*dopPosY +
                              (((Sint64)collider.top + frame->motion.floor[i].tile_bottom)));
            *vram_flag_p |= sp30;
            return;
        }
    }
}

static void CheckCeiling(MotionFrame *frame)
{
    SotnCollider collider = {0};
    Sint32 temp_fp = 0;
    Sint32 temp_v1 = 0;
    Uint32 effects = 0;
    Sint32 var_a1 = 0;
    Sint32 posX = 0;
    Sint32 posY = 0;
    Uint32 maskedEffects = 0;

    Sint32 i = 1;
    Sint32 *dopPosY = &frame->py;
    Sint32 *dopPosX = &frame->px;
    Uint32 *vram_flag_p = &frame->motion.contact;

    for (; i < 4; i++)
    {
        if (frame->motion.ceiling[i].effects & SOTN_EFFECT_SOLID_FROM_ABOVE)
        {
            continue;
        }

        if ((frame->motion.status & 2U) && (collider.effects & SOTN_EFFECT_MIST_ONLY))
        {
            collider.effects &= ~SOTN_EFFECT_BLOCK;
        }

        effects = frame->motion.ceiling[i].effects & (0x8000U | 0x0800U | SOTN_EFFECT_SOLID);

        if (effects == SOTN_EFFECT_SOLID || (effects & 0x8000U))
        {
            if ((frame->motion.status & (1U | 2U)) && !(effects & SOTN_EFFECT_SOLID))
            {
                continue;
            }

            posX = Wrap16(*dopPosX + frame->motion.ceiling_sensors[i].x);
            posY = Wrap16(*dopPosY + frame->motion.ceiling_sensors[i].y +
                          frame->motion.ceiling[i].tile_bottom + 1);
            Query(frame, posX, posY, &collider);
            if ((frame->motion.status & 2U) && (collider.effects & SOTN_EFFECT_MIST_ONLY))
            {
                collider.effects &= ~SOTN_EFFECT_BLOCK;
            }

            maskedEffects = collider.effects;
            if (!(maskedEffects & SOTN_EFFECT_SOLID))
            {
                if (frame->motion.ceiling[i].effects != SOTN_EFFECT_SOLID || frame->motion.vy <= 0)
                {
                    *vram_flag_p |= 2;
                    if (!(*vram_flag_p & 1))
                    {
                        if (frame->motion.ceiling[i].effects & 0x8000U)
                        {
                            *dopPosY =
                                Wrap16((Sint64)*dopPosY + (frame->motion.ceiling[i].tile_bottom));
                        }
                        else
                        {
                            *dopPosY = Wrap16((Sint64)*dopPosY + (frame->motion.ceiling[i].bottom));
                        }
                    }
                    return;
                }
                continue;
            }
            if ((maskedEffects & (0x0800U | SOTN_EFFECT_BLOCK)) == (0x0800U | SOTN_EFFECT_SOLID))
            {
                if (i < 2)
                {
                    *vram_flag_p |= 0x802 | ((maskedEffects >> 4) & 0x700);
                    if (!(*vram_flag_p & 1))
                    {
                        *dopPosY = Wrap16(
                            (Sint64)*dopPosY +
                            ((Sint64)frame->motion.ceiling[i].tile_bottom + collider.bottom + 1));
                    }
                    return;
                }
                if ((i == 2) && (maskedEffects & (0x4000U | 0x0800U | SOTN_EFFECT_SOLID)) ==
                                    (0x0800U | SOTN_EFFECT_SOLID))
                {
                    frame->motion.floor[2].effects = maskedEffects;
                    frame->motion.floor[2].tile_top = frame->motion.floor[2].tile_bottom;
                }
                if ((i == 3) && ((maskedEffects & (0x4000U | 0x0800U | SOTN_EFFECT_SOLID)) ==
                                 (0x4000U | 0x0800U | SOTN_EFFECT_SOLID)))
                {
                    frame->motion.floor[3].effects = maskedEffects;
                    frame->motion.floor[3].tile_top = frame->motion.floor[3].tile_bottom;
                }
            }
        }

        if (effects == (0x0800U | SOTN_EFFECT_SOLID) && i < 2)
        {
            *vram_flag_p |=
                (0x0800U | SOTN_EFFECT_SIDE) |
                ((frame->motion.ceiling[i].effects >> 4) & (0x0400U | 0x0200U | 0x0100U));
            if (!(*vram_flag_p & 1))
            {
                *dopPosY = Wrap16((Sint64)*dopPosY + (frame->motion.ceiling[i].bottom));
            }
            return;
        }
    }

    if (frame->motion.vy > 0)
    {
        return;
    }

    posX = Wrap16(*dopPosX + frame->motion.ceiling_sensors[0].x);
    posY = Wrap16(*dopPosY + frame->motion.ceiling_sensors[0].y - 10);
    Query(frame, posX, posY, &collider);
    if ((collider.effects & SOTN_EFFECT_SOLID) != SOTN_EFFECT_NONE)
    {
        return;
    }

    for (i = 2; i < 4; i++)
    {
        if ((frame->motion.ceiling[3].effects & 0x0800U) &&
            (frame->motion.ceiling[2].effects & 0x0800U))
        {
            return;
        }

        effects = frame->motion.ceiling[i].effects;
        temp_fp = ((effects >> 4) & 0x700) + 0x802;
        if (effects & 0x0800U)
        {
            if (i == 2)
            {
                maskedEffects = 0x4000U;
                var_a1 = Wrap16(frame->motion.ceiling[2].tile_left);
                temp_v1 = var_a1 + 8;
            }
            else
            {
                maskedEffects = 0;
                var_a1 = Wrap16(frame->motion.ceiling[3].tile_right);
                temp_v1 = 8 - var_a1;
            }

            if ((effects & 0x4000U) == maskedEffects)
            {
                posX = Wrap16(*dopPosX + frame->motion.ceiling_sensors[i].x + var_a1);
                posY = Wrap16(*dopPosY + frame->motion.ceiling_sensors[i].y);
                Query(frame, posX, posY, &collider);
                if (collider.effects & SOTN_EFFECT_SOLID)
                {
                    *vram_flag_p |= temp_fp;
                    if (!(*vram_flag_p & 1))
                    {
                        *dopPosY = Wrap16((Sint64)*dopPosY + (collider.bottom));
                    }
                    return;
                }
            }
            else if ((temp_v1 > 0) && (effects & 1))
            {
                posX = Wrap16(*dopPosX + frame->motion.ceiling_sensors[i].x + var_a1);
                posY = Wrap16((*dopPosY + frame->motion.ceiling_sensors[i].y +
                               frame->motion.ceiling[i].tile_top));

                Query(frame, posX, posY, &collider);
                if (collider.effects & SOTN_EFFECT_SOLID)
                {
                    if (!(*vram_flag_p & 1))
                    {
                        *dopPosY = Wrap16((Sint64)*dopPosY + ((Sint64)collider.bottom +
                                                              frame->motion.ceiling[i].tile_top));
                    }
                    *vram_flag_p |= temp_fp;
                    return;
                }
            }
        }
    }
}

static void CheckWallRight(MotionFrame *frame)
{
    SotnCollider collider = {0};

    Sint32 *dopY;
    Sint32 *dopX;
    Sint32 effects = 0;
    Sint32 i = 0;
    Uint32 *pVramFlag;

    Sint32 offsetX = 0, offsetY = 0;

    dopY = &frame->py;
    dopX = &frame->px;

    pVramFlag = &frame->motion.contact;
    effects = frame->motion.previous_contact &
              (0x8000U | 0x4000U | 0x0800U | 0x0400U | SOTN_EFFECT_BLOCK);
    if (effects == (0x8000U | SOTN_EFFECT_BLOCK) || effects == (0x0800U | SOTN_EFFECT_BLOCK) ||
        effects == (0x8000U | 0x0800U | SOTN_EFFECT_BLOCK))
    {
        *pVramFlag |= 4;
        return;
    }

    for (i = 0; i < 7; i++)
    {
        effects = frame->motion.wall[i].effects & (0x8000U | 0x4000U | 0x0800U | SOTN_EFFECT_BLOCK);
        if ((effects == (0x8000U | 0x4000U | SOTN_EFFECT_SOLID)) ||
            (effects == (0x8000U | 0x4000U | SOTN_EFFECT_SIDE | SOTN_EFFECT_SOLID)) ||
            (effects == (0x4000U | 0x0800U | SOTN_EFFECT_SOLID)) ||
            (effects == (0x4000U | 0x0800U | SOTN_EFFECT_SIDE | SOTN_EFFECT_SOLID)) ||
            (effects == (0x8000U | SOTN_EFFECT_BLOCK)) ||
            (effects == (0x0800U | SOTN_EFFECT_BLOCK)) || (effects == SOTN_EFFECT_BLOCK))
        {

            offsetX = Wrap16(*dopX + frame->motion.wall_sensors[i].x +
                             frame->motion.wall[i].tile_left - 1);
            offsetY = Wrap16(*dopY + frame->motion.wall_sensors[i].y);
            Query(frame, offsetX, offsetY, &collider);

            if (!(collider.effects & SOTN_EFFECT_SOLID))
            {
                *pVramFlag |= 4;
                *dopX = Wrap16((Sint64)*dopX + (frame->motion.wall[i].tile_left));
                return;
            }
        }

        if ((effects & (0x8000U | 0x4000U | 0x0800U)) == 0x8000U && i != 0 &&
            ((frame->motion.wall[0].effects & 0x0800U) ||
             !(frame->motion.wall[0].effects & (0x8000U | 0x0800U | SOTN_EFFECT_SIDE))))
        {
            *pVramFlag |= 4;
            *dopX = Wrap16((Sint64)*dopX + (frame->motion.wall[i].tile_left));
            return;
        }

        if ((effects & (0x8000U | 0x4000U | 0x0800U)) == 0x0800U && i != 6 &&
            ((frame->motion.wall[6].effects & 0x8000U) ||
             !(frame->motion.wall[6].effects & (0x8000U | 0x0800U | SOTN_EFFECT_SIDE))))
        {
            *pVramFlag |= 4;
            *dopX = Wrap16((Sint64)*dopX + (frame->motion.wall[i].tile_left));
            return;
        }
    }
}

static void CheckWallLeft(MotionFrame *frame)
{
    SotnCollider collider = {0};

    Sint32 *dopY;
    Sint32 *dopX;
    Sint32 effects = 0;
    Sint32 i = 0;
    Uint32 *pVramFlag;

    Sint32 offsetX = 0, offsetY = 0;
    dopY = &frame->py;
    dopX = &frame->px;

    pVramFlag = &frame->motion.contact;
    effects = frame->motion.previous_contact &
              (0x8000U | 0x4000U | 0x0800U | 0x0800U | 0x0400U | SOTN_EFFECT_BLOCK);
    if (effects == (0x8000U | 0x4000U | SOTN_EFFECT_BLOCK) ||
        effects == (0x0800U | 0x0400U | SOTN_EFFECT_BLOCK) ||
        effects == (0x8000U | 0x4000U | 0x0800U | 0x0400U | SOTN_EFFECT_BLOCK))
    {
        *pVramFlag |= 8;
        return;
    }

    for (i = 7; i < 14; i++)
    {
        effects = frame->motion.wall[i].effects & (0x8000U | 0x4000U | 0x0800U | SOTN_EFFECT_BLOCK);
        if (effects == (0x8000U | SOTN_EFFECT_SOLID) || effects == (0x8000U | SOTN_EFFECT_BLOCK) ||
            effects == (0x0800U | SOTN_EFFECT_SOLID) || effects == (0x0800U | SOTN_EFFECT_BLOCK) ||
            effects == (0x8000U | 0x4000U | SOTN_EFFECT_SIDE | SOTN_EFFECT_SOLID) ||
            effects == (0x4000U | 0x0800U | SOTN_EFFECT_SIDE | SOTN_EFFECT_SOLID) ||
            effects == SOTN_EFFECT_BLOCK)
        {

            offsetX = Wrap16(*dopX + frame->motion.wall_sensors[i].x +
                             frame->motion.wall[i].tile_right + 1);
            offsetY = Wrap16(*dopY + frame->motion.wall_sensors[i].y);
            Query(frame, offsetX, offsetY, &collider);

            if (!(collider.effects & SOTN_EFFECT_SOLID))
            {
                *pVramFlag |= 8;
                *dopX = Wrap16((Sint64)*dopX + (frame->motion.wall[i].tile_right));
                return;
            }
        }

        if ((effects & (0x8000U | 0x4000U | 0x0800U)) == (0x8000U | 0x4000U) && i != 7 &&
            ((frame->motion.wall[7].effects & 0x0800U) ||
             !(frame->motion.wall[7].effects & (0x8000U | 0x0800U | SOTN_EFFECT_SIDE))))
        {
            *pVramFlag |= 8;
            *dopX = Wrap16((Sint64)*dopX + (frame->motion.wall[i].tile_right));
            return;
        }

        if (((effects & (0x8000U | 0x4000U | 0x0800U)) == (0x4000U | 0x0800U)) && i != 13 &&
            ((frame->motion.wall[13].effects & 0x8000U) ||
             !(frame->motion.wall[13].effects & (0x8000U | 0x0800U | SOTN_EFFECT_SIDE))))
        {
            *pVramFlag |= 8;
            *dopX = Wrap16((Sint64)*dopX + (frame->motion.wall[i].tile_right));
            return;
        }
    }
}

static Sint16 ReadSigned16(const Uint8 *bytes)
{
    const Uint16 bits = (Uint16)(bytes[0] | (Uint16)bytes[1] << 8);
    return bits <= 32767 ? (Sint16)bits : (Sint16)((Sint32)bits - 65536);
}
static bool SensorsValid(const SotnDoppleMotion *motion)
{
    for (size_t i = 0; i < 4; ++i)
    {
        if (SDL_abs(motion->floor_sensors[i].x) > 64 || SDL_abs(motion->floor_sensors[i].y) > 64 ||
            SDL_abs(motion->ceiling_sensors[i].x) > 64 ||
            SDL_abs(motion->ceiling_sensors[i].y) > 64)
            return SDL_SetError("BO4 horizontal sensor exceeds its supported geometry");
    }
    for (size_t i = 0; i < 14; ++i)
        if (SDL_abs(motion->wall_sensors[i].x) > 64 || SDL_abs(motion->wall_sensors[i].y) > 64)
            return SDL_SetError("BO4 wall sensor exceeds its supported geometry");
    return true;
}

bool SotnDoppleMotion_Init(SotnDataView overlay, SotnDoppleMotion *out)
{
    if (out == NULL || overlay.bytes == NULL || overlay.size < 0x12e4)
        return SDL_SetError("Initializing BO4 movement requires its complete private sensors");
    SotnDoppleMotion next = {0};
    for (size_t i = 0; i < 4; ++i)
    {
        next.ceiling_sensors[i] = (SotnDoppleSensor){ReadSigned16(overlay.bytes + 0x128c + i * 4),
                                                     ReadSigned16(overlay.bytes + 0x128e + i * 4)};
        next.floor_sensors[i] = (SotnDoppleSensor){ReadSigned16(overlay.bytes + 0x129c + i * 4),
                                                   ReadSigned16(overlay.bytes + 0x129e + i * 4)};
    }
    for (size_t i = 0; i < 14; ++i)
        next.wall_sensors[i] = (SotnDoppleSensor){ReadSigned16(overlay.bytes + 0x12ac + i * 4),
                                                  ReadSigned16(overlay.bytes + 0x12ae + i * 4)};
    if (!SensorsValid(&next))
        return false;
    next.initialized = true;
    *out = next;
    return true;
}

static bool RefreshSensors(SotnDataView overlay, SotnDoppleMotion *motion)
{
    const size_t base = motion->status & 3U ? 0x124c : 0x126c;
    for (size_t i = 0; i < 4; ++i)
    {
        motion->ceiling_sensors[i].y = ReadSigned16(overlay.bytes + base + i * 2);
        motion->floor_sensors[i].y = ReadSigned16(overlay.bytes + base + 8 + i * 2);
    }
    for (size_t i = 0; i < 7; ++i)
    {
        const Sint16 y = ReadSigned16(overlay.bytes + base + 16 + i * 2);
        motion->wall_sensors[i].y = y;
        motion->wall_sensors[i + 7].y = y;
    }
    return SensorsValid(motion);
}

static bool Collide(MotionFrame *frame, SotnDataView overlay, bool refresh)
{
    SotnDoppleMotion *motion = &frame->motion;
    motion->previous_contact = motion->contact;
    motion->contact = 0;
    if (refresh && !RefreshSensors(overlay, motion))
        return false;
    Sint32 speed = motion->vx;
    const Uint32 slope = motion->previous_contact & 0xf000U;
    if (motion->vx < 0 && !(motion->previous_contact & 8U))
    {
        if (slope == 0xc000U)
            speed = speed * 10 / 16;
        if (slope == 0xd000U)
            speed = speed * 13 / 16;
        motion->x = Wrap32((Sint64)motion->x + speed);
    }
    if (motion->vx > 0 && !(motion->previous_contact & 4U))
    {
        if (slope == 0x8000U)
            speed = speed * 10 / 16;
        if (slope == 0x9000U)
            speed = speed * 13 / 16;
        motion->x = Wrap32((Sint64)motion->x + speed);
    }
    if ((motion->vy < 0 && !(motion->previous_contact & 2U)) ||
        (motion->vy > 0 && !(motion->previous_contact & 1U)))
        motion->y = Wrap32((Sint64)motion->y + motion->vy);
    frame->px = Pixel(motion->x);
    frame->py = Pixel(motion->y);
    for (size_t i = 0; i < 4; ++i)
        Query(frame, frame->px + motion->floor_sensors[i].x, frame->py + motion->floor_sensors[i].y,
              &motion->floor[i]);
    CheckFloor(frame);
    for (size_t i = 0; i < 4; ++i)
        Query(frame, frame->px + motion->ceiling_sensors[i].x,
              frame->py + motion->ceiling_sensors[i].y, &motion->ceiling[i]);
    CheckCeiling(frame);
    const Uint32 y_fraction =
        ((motion->contact & 1U) && motion->vy >= 0) || ((motion->contact & 2U) && motion->vy <= 0)
            ? 0U
            : (Uint32)motion->y & 65535U;
    for (size_t i = 0; i < 14; ++i)
        Query(frame, frame->px + motion->wall_sensors[i].x, frame->py + motion->wall_sensors[i].y,
              &motion->wall[i]);
    CheckWallRight(frame);
    CheckWallLeft(frame);
    const Uint32 x_fraction =
        ((motion->contact & 4U) && motion->vx > 0) || ((motion->contact & 8U) && motion->vx < 0)
            ? 0U
            : (Uint32)motion->x & 65535U;
    motion->x = Wrap32((Sint64)frame->px * 65536 + x_fraction);
    motion->y = Wrap32((Sint64)frame->py * 65536 + y_fraction);
    if ((motion->contact & 0x8000U) || !(motion->floor[1].effects & SOTN_EFFECT_SOLID) ||
        !(motion->floor[2].effects & SOTN_EFFECT_SOLID) ||
        !(motion->floor[3].effects & SOTN_EFFECT_SOLID))
        motion->contact |= 0x20U;
    return !frame->failed;
}

bool SotnDoppleMotion_Advance(SotnDataView overlay, const SotnCollisionMap *map, bool high_jump,
                              SotnDoppleMotion *motion)
{
    if (overlay.bytes == NULL || overlay.size < 0x12e4 || map == NULL || motion == NULL ||
        !motion->initialized || motion->vx < -2097152 || motion->vx > 2097152 ||
        motion->vy < -2097152 || motion->vy > 2097152)
        return SDL_SetError(
            "Advancing BO4 movement requires initialized sensors, a map and bounded speeds");
    SotnCollider outside = {0};
    if (!SensorsValid(motion) ||
        !SotnCollision_Query(map, SDL_MIN_SINT32, SDL_MIN_SINT32, &outside))
        return false;
    MotionFrame frame = {.motion = *motion, .map = *map};
    const Uint32 previous = motion->contact;
    if ((motion->status & 1U) || high_jump || SDL_abs(motion->vx) > 131072)
    {
        frame.motion.vx =
            motion->vx >= 0 ? motion->vx / 4 : (Sint32)(-((-(Sint64)motion->vx + 3) / 4));
        frame.motion.vy =
            motion->vy >= 0 ? motion->vy / 4 : (Sint32)(-((-(Sint64)motion->vy + 3) / 4));
        for (size_t i = 0; i < 4 && Pixel(frame.motion.y) >= 0; ++i)
            if (!Collide(&frame, overlay, i == 0 && !(motion->status & 0x400000U)))
                return false;
        if (Pixel(frame.motion.y) < 0)
            frame.motion.y = -65536;
        frame.motion.vx *= 4;
        frame.motion.vy *= 4;
    }
    else if (!Collide(&frame, overlay, !(motion->status & 0x800000U)))
        return false;
    frame.motion.previous_contact = previous;
    *motion = frame.motion;
    return true;
}
