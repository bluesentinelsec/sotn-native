// SPDX-License-Identifier: AGPL-3.0-or-later
// st/no0/e_slinger.c, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Original motion, timers, frames and sensors are read from private NO0 tables.
#include "actors_internal.h"
static bool Pieces(SotnActors *s, Actor *a)
{
    const Uint16 index = a->state.parameters;
    if (index >= 7)
        return SDL_SetError("Slinger part index exceeds its private table");
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, 0xbc0))
            return false;
        a->state.frame = (Uint16)(index + 15);
        a->state.rotated = true;
        if (a->state.facing_left)
            a->vx = -a->vx;
        return true;
    }
    if (!a->timer)
        return SDL_SetError("Slinger part has no destruction timer");
    if (--a->timer == 0)
        return ActorReplaceExplosion(s, a, 0);
    SotnDataView rotation = {0};
    if (!Slice(s->nz0, 0x21dc + index * 2U, 2, &rotation))
        return false;
    a->state.rotation = (Sint16)((a->state.rotation + Read16(rotation.bytes)) & 4095);
    if (a->vy < 6 * 65536)
        a->vy += 16384;
    a->state.x += a->vx;
    a->state.y += a->vy;
    return true;
}
static bool Rib(SotnActors *s, Actor *a)
{
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, 0xbcc))
            return false;
        a->state.y -= 4096;
        a->vx = a->state.facing_left ? 114688 : -114688;
        a->vy = 0;
        a->state.rotated = true;
    }
    else if (a->state.flags & 0x100)
        return ActorReplaceExplosion(s, a, 0);
    else
    {
        a->state.rotation = (Sint16)((a->state.rotation + 128) & 4095);
        a->state.x += a->vx;
        a->state.y += a->vy;
        if (Pixel(a->state.y) - s->camera_y > 240)
            a->remove = true;
    }
    return true;
}
static bool AttackCheck(SotnActors *s, Actor *a)
{
    Uint16 ground = 0, movement = 0;
    if (!ActorWalk(s, a, 0x2254, &ground) || !ActorField(s, a, 0x225c, 3, false, &movement))
        return false;
    if (ground == 128 && (movement & 2))
        SetStep(a, 5);
    else
    {
        if (a->enemy_state.attack_timer == 0)
            return SDL_SetError("Slinger attack timer underflow");
        if (--a->enemy_state.attack_timer == 0)
            SetStep(a, 4);
    }
    return true;
}
bool ActorSlinger(SotnActors *s, Actor *a)
{
    if (a->state.id == 0x3b)
        return Rib(s, a);
    if (a->state.id == 0x3c)
        return Pieces(s, a);
    Uint8 animation = 0;
    Uint16 collision = 0;
    if (a->state.flags & 0x100)
        a->state.step = 6;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0xbb4))
            return false;
        a->enemy_state.attack_timer = 80;
        a->enemy_state.movement_facing = false;
        a->enemy_state.attack_index = 0;
        break;
    case 1:
        if (!ActorGround(s, a, 0x2254, &collision))
            return false;
        if (collision)
            ++a->state.step;
        break;
    case 2:
    case 3:
        a->state.facing_left = Side(a, s->player) == 0;
        a->enemy_state.movement_facing = a->state.facing_left != (a->state.step == 3);
        if (!ActorAnimate(s, a, a->state.step == 2 ? 0x2194 : 0x21a4, &animation))
            return false;
        a->vx = a->enemy_state.movement_facing ? 32768 : -32768;
        if (a->state.step == 2 && Distance(a, s->player) < 76)
            a->state.step = 3;
        else if (a->state.step == 3 && Distance(a, s->player) > 92)
            a->state.step = 2;
        return AttackCheck(s, a);
    case 4:
        if (!ActorAnimate(s, a, 0x21b4, &animation))
            return false;
        if (animation == 0)
        {
            SetStep(a, 3);
            a->enemy_state.attack_index = (Uint16)((a->enemy_state.attack_index + 1) & 3);
            SotnDataView time = {0};
            if (!Slice(s->nz0,
                       0x224c + (a->state.parameters & 1) * 4U + a->enemy_state.attack_index, 1,
                       &time))
                return false;
            a->enemy_state.attack_timer = time.bytes[0];
        }
        else if ((animation & 0x80) && a->state.frame == 10)
        {
            const Sint32 x = Pixel(a->state.x) - s->camera_x, y = Pixel(a->state.y) - s->camera_y;
            if (x >= -16 && x <= 272 && y >= 0 && y <= 240)
            {
                Actor *rib = NULL;
                if (!ActorSpawn(s, a, 0x3b, 0, 96, 128, &rib))
                    return false;
                if (rib != NULL)
                {
                    if (!ActorSound(s, 0x6c8))
                        return false;
                    rib->state.x += (a->state.facing_left ? -8 : 8) * 65536;
                    rib->state.y -= 16 * 65536;
                }
            }
        }
        break;
    case 5:
        if (a->substep == 0)
        {
            if (!ActorAnimate(s, a, 0x21c4, &animation))
                return false;
            if (!(animation & 1))
            {
                bool right = a->enemy_state.movement_facing;
                if ((ActorRandom(s) & 3) == 0)
                    right = !right;
                a->vx = right ? 2 * 65536 : -2 * 65536;
                a->vy = -3 * 65536;
                a->pose = a->remaining = 0;
                ++a->substep;
            }
        }
        else if (a->substep == 1)
        {
            if (!ActorGround(s, a, 0x2254, &collision))
                return false;
            if (collision)
                ++a->substep;
            if (!ActorField(s, a, 0x225c, 2, true, &collision))
                return false;
        }
        else if (a->substep == 2)
        {
            if (!ActorAnimate(s, a, 0x21d0, &animation))
                return false;
            if (animation & 1)
                SetStep(a, 3);
        }
        else
            return SDL_SetError("Slinger jump substate is invalid");
        break;
    case 6:
        if (!ActorSound(s, 0x62a))
            return false;
        for (Uint16 i = 0; i < 7; ++i)
        {
            Actor *part = NULL;
            SotnDataView timer = {0}, x = {0}, y = {0}, vx = {0}, vy = {0};
            if (!ActorSpawn(s, a, 0x3c, i, 160, 192, &part))
                return false;
            if (part == NULL)
                break;
            if (!Slice(s->nz0, 0x21ec + i, 1, &timer) || !Slice(s->nz0, 0x222c + i * 2U, 2, &x) ||
                !Slice(s->nz0, 0x223c + i * 2U, 2, &y) || !Slice(s->nz0, 0x21f4 + i * 4U, 4, &vx) ||
                !Slice(s->nz0, 0x2210 + i * 4U, 4, &vy))
                return false;
            if (!timer.bytes[0] || Signed32(vx.bytes) < -8 * 65536 ||
                Signed32(vx.bytes) > 8 * 65536 || Signed32(vy.bytes) < -8 * 65536 ||
                Signed32(vy.bytes) > 8 * 65536 || SDL_abs(Signed16(x.bytes)) > 128 ||
                SDL_abs(Signed16(y.bytes)) > 128)
                return SDL_SetError("Slinger part motion exceeds its private bounds");
            part->timer = timer.bytes[0];
            part->vx = Signed32(vx.bytes);
            part->vy = Signed32(vy.bytes);
            part->state.x += Signed16(x.bytes) * (a->state.facing_left ? -1 : 1) * 65536;
            ShiftY(part, Signed16(y.bytes));
        }
        a->remove = true;
        break;
    default:
        return SDL_SetError("Slinger entered invalid state %u", a->state.step);
    }
    return true;
}
