// SPDX-License-Identifier: AGPL-3.0-or-later
// st/e_flea_man.h at sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Motion, animation and collision-sensor arrays remain in private NO0/LIB data.
#include "actors_internal.h"
bool ActorFlea(SotnActors *s, Actor *a)
{
    const bool library = s->stage_id == 2;
    if (library && (s->library_enemy_flags & 1U))
        a->state.flags |= 0x100U;
    if (a->state.flags & 0x100)
    {
        if (!(library ? ActorLibrarySound(s, a, 0x691) : ActorSound(s, 0x691)) ||
            !ActorReplaceExplosion(s, a, 1))
            return false;
        return true;
    }
    if (a->enemy_state.timer)
    {
        --a->enemy_state.timer;
        a->state.hitbox_state = 0;
    }
    else
        a->state.hitbox_state = 19;
    Uint8 animation = 0;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, (library ? 0x9d4 : 0xc2c)))
            return false;
        a->state.frame = 16;
        if (a->state.parameters & 1)
        {
            a->state.facing_left = true;
            a->vx = 0x140;
            a->vy = 0xa0;
            a->state.step = 2;
        }
        if (a->enemy_state.timer)
            a->state.step = 3;
        break;
    case 1:
        if (!ActorAnimate(s, a, (library ? 0x2e0c : 0x2640), &animation))
            return false;
        a->state.facing_left = Side(a, s->player) == 0;
        if (Distance(a, s->player) < 96 && SDL_abs(Pixel(a->state.y) - Pixel(s->player.y)) < 64)
            a->state.step = 3;
        break;
    case 2:
        a->state.priority = 0;
        a->state.x += a->vx;
        a->state.y += a->vy;
        break;
    case 3: {
        a->state.facing_left = Side(a, s->player) == 0;
        Uint32 index = 2;
        if (!a->hit_params || a->hit_params >= 4)
        {
            const Uint32 random = ActorRandom(s) & 7;
            const Sint64 distance = Distance(a, s->player);
            if (distance < 80)
                index = random < 2 ? 3 : random == 2 ? 2 : random == 3 ? 1 : 0;
            else if (distance <= 127)
                index = random == 0                                     ? 3
                        : random == 1                                   ? 2
                        : random == 2                                   ? 0
                        : a->state.facing_left != s->player.facing_left ? 0
                                                                        : 1;
            else
                index = 4;
        }
        SotnDataView vx = {0}, vy = {0};
        if (!Slice(s->nz0, (library ? 0x2de4 : 0x2618) + index * 4, 4, &vx) ||
            !Slice(s->nz0, (library ? 0x2df8 : 0x262c) + index * 4, 4, &vy))
            return false;
        const Sint32 horizontal = Signed32(vx.bytes), vertical = Signed32(vy.bytes);
        if (horizontal < -8 * 65536 || horizontal > 8 * 65536 || vertical < -8 * 65536 ||
            vertical > 8 * 65536)
            return SDL_SetError("Flea Man motion exceeds its bounded private velocities");
        a->vx = horizontal * (a->state.facing_left ? -1 : 1);
        a->vy = vertical;
        if ((index == 1 && !(library ? ActorLibrarySound(s, a, 0x6c0) : ActorSound(s, 0x6c0))) ||
            ((index == 0 || index == 2) &&
             !(library ? ActorLibrarySound(s, a, 0x6c1) : ActorSound(s, 0x6c1))))
            return false;
        ++a->state.step;
        break;
    }
    case 4: {
        a->state.frame = a->vy > -16384 && a->vy < 16384 ? 19 : a->vy < 0 ? 20 : 18;
        Uint16 collision = 0;
        if (!ActorGround(s, a, (library ? 0x2e18 : 0x264c), &collision))
            return false;
        if (collision & 1)
            a->state.step = 3;
        if (a->vy < 0)
        {
            SotnDataView sensors = {0};
            if (!Slice(s->nz0, (library ? 0x2e28 : 0x265c), 12, &sensors))
                return false;
            Sint32 x = Pixel(a->state.x), y = Pixel(a->state.y);
            for (Uint32 i = 0; i < 3; ++i)
            {
                x = Wrap16((Sint64)x + Signed16(sensors.bytes + i * 4));
                y = Wrap16((Sint64)y + Signed16(sensors.bytes + i * 4 + 2));
                SotnCollider ceiling = {0};
                if (!ActorQuery(s, x, y, &ceiling))
                    return false;
                if (ceiling.effects & SOTN_EFFECT_SOLID)
                {
                    ShiftY(a, ceiling.bottom);
                    break;
                }
            }
        }
        if (!ActorField(s, a, (library ? 0x2e34 : 0x2668), 2, true, &collision))
            return false;
        break;
    }
    default:
        return SDL_SetError("Flea Man entered invalid state %u", a->state.step);
    }
    return true;
}
