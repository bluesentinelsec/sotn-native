// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected DAI e_confessional.c at sotn-decomp 7bc54d8157b6. Tables remain private.
#include "chapel_confessional_internal.h"
static bool Animate(SotnActors *s, Actor *a, ConfessionalProfile profile, Uint16 index,
                    Uint8 *result)
{
    SotnDataView pointer = {0};
    if (index > 3 || !Slice(s->nz0, profile.animations + index * 4U, 4, &pointer))
        return false;
    const Uint32 address = Read32(pointer.bytes);
    if (address < 0x80180d54 || address >= 0x80180de8 || address % 2)
        return SDL_SetError("Confessional ghost references an invalid private animation");
    return ActorAnimate(s, a, address - 0x80180000U, result);
}
static bool Blade(SotnActors *s, Actor *a, Uint16 parameters)
{
    Actor *child = NULL;
    return ActorSpawn(s, a, 0x24, parameters, 96, 128, &child);
}
bool ConfessionalGhost(SotnActors *s, Actor *a)
{
    ConfessionalProfile p = {0};
    Uint8 animation = 0;
    Uint16 collision = 0;
    if (!ConfessionalRead(s, a->state.parameters, &p))
        return false;
    s->control.block_stopwatch = true;
    const bool parish = (a->state.parameters & 1U) != 0, bad = (a->state.parameters & 0x100U) != 0;
    ActorPrimitive *curtain = NULL;
    if (a->state.step)
    {
        curtain = ActorPrimitiveGet(s, a, 0);
        if (!curtain)
            return false;
    }
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x9bc))
            return false;
        a->state.palette = bad ? 0xce : 0xc9;
        a->state.opacity = 0;
        a->state.opacity_set = true;
        a->state.blend_mode = 0x30;
        if (!ActorPrimitives(s, a, 1))
        {
            a->remove = true;
            return true;
        }
        a->state.flags |= 0x800000U;
        curtain = ActorPrimitiveGet(s, a, 0);
        if (!curtain)
            return false;
        curtain->state = (SotnActorPrimitive){.active = true,
                                              .textured = true,
                                              .quad = true,
                                              .screen_space = true,
                                              .priority_set = true,
                                              .priority = 159,
                                              .page = 18,
                                              .palette = 0xcc};
        curtain->state.u[1] = curtain->state.u[3] = 39;
        curtain->state.v[2] = curtain->state.v[3] = 64;
        curtain->state.offset_x[0] = curtain->state.offset_x[2] = (Sint16)p.left;
        curtain->state.offset_x[1] = curtain->state.offset_x[3] = (Sint16)p.edge;
        curtain->state.offset_y[0] = curtain->state.offset_y[1] = 122;
        curtain->state.offset_y[2] = curtain->state.offset_y[3] = 186;
        if (parish)
        {
            s->combat.cd_command = 0x90;
            s->combat.sequence_change = true;
            s->combat.sequence = 0x204;
            s->confessional_chime = true;
        }
        return true;
    case 1:
        return true;
    case 2:
        if (!ActorGround(s, a, 0xdf0, &collision))
            return false;
        if (collision & 1U)
            ++a->state.step;
        return true;
    case 3:
        a->state.opacity = (Uint16)SDL_min(192, a->state.opacity + 4);
        if (!Animate(s, a, p, 0, &animation) || !ActorWalk(s, a, 0xde8, &collision))
            return false;
        a->state.x += p.speed;
        if (Pixel(a->state.x) - s->camera_x == p.target)
        {
            a->state.x = (p.target + s->camera_x) * 65536 + (Sint32)((Uint32)a->state.x & 65535U);
            ConfessionalStep(a, bad ? 6 : 4);
        }
        if (!ConfessionalSeated(s))
            ConfessionalStep(a, 13);
        return true;
    case 4:
        if (!Animate(s, a, p, 1, &animation))
            return false;
        if (!parish)
        {
            if (!a->substep)
            {
                a->timer = 256;
                a->substep = 1;
            }
            else
            {
                if (!a->timer || a->substep != 1)
                    return SDL_SetError("Confessional priest has invalid prayer timing");
                if (!--a->timer)
                    ConfessionalStep(a, 5);
            }
            if (!ConfessionalSeated(s))
            {
                a->timer = 1;
                ConfessionalStep(a, 5);
            }
        }
        else if (!ConfessionalSeated(s))
            ConfessionalStep(a, 5);
        return true;
    case 5:
        if (!Animate(s, a, p, 2, &animation))
            return false;
        a->state.opacity = (Uint8)(a->state.opacity - 1U);
        if (!a->state.opacity)
        {
            if (!parish && !a->timer && !(ActorRandom(s) & 3U))
            {
                Actor *gift = NULL;
                if (!ActorSpawn(s, a, 0xa, 56, 96, 128, &gift))
                    return false;
            }
            ConfessionalStep(a, 128);
        }
        return true;
    case 6:
        if (!Animate(s, a, p, 3, &animation))
            return false;
        if (!animation)
        {
            if (!ActorSound(s, 0x7bb))
                return false;
            ++a->state.step;
        }
        return true;
    case 7:
        curtain->state.offset_x[3] = (Sint16)(curtain->state.offset_x[3] + p.curtain_speed);
        curtain->state.offset_x[1] = curtain->state.offset_x[3];
        if (curtain->state.offset_x[1] == p.closed)
        {
            a->timer = 16;
            ++a->state.step;
        }
        return true;
    case 8:
        if (!a->timer || a->timer > 16)
            return SDL_SetError("Confessional curtain has invalid shake timing");
        if (!--a->timer)
            ConfessionalStep(a, parish ? 10 : 9);
        curtain->state.offset_x[2] = (Sint16)(p.left + ((a->timer & 2U) ? -1 : 1));
        curtain->state.offset_x[3] = (Sint16)(p.closed + ((a->timer & 2U) ? -1 : 1));
        return true;
    case 9:
    case 10:
        if (!a->substep)
        {
            a->timer = 32;
            a->enemy_state.counter = 0;
            a->substep = 1;
        }
        if (!a->timer)
            return SDL_SetError("Confessional blade sequence has invalid timing");
        if (a->substep == 1)
        {
            if (!--a->timer)
            {
                if (!Blade(s, a, parish ? 4 : a->enemy_state.counter))
                    return false;
                ++a->enemy_state.counter;
                if (parish || a->enemy_state.counter > 3)
                {
                    a->timer = 128;
                    a->substep = 2;
                }
                else
                    a->timer = 16;
            }
        }
        else if (a->substep == 2)
        {
            if (!--a->timer)
                ConfessionalStep(a, 11);
        }
        else
            return SDL_SetError("Confessional blade sequence has an invalid original substate");
        return true;
    case 11:
        a->state.opacity = (Uint8)(a->state.opacity - 32U);
        if (a->state.opacity > 192)
            a->state.frame = 0;
        curtain->state.offset_x[1] = (Sint16)(curtain->state.offset_x[1] - p.curtain_speed);
        curtain->state.offset_x[3] = curtain->state.offset_x[1];
        if (curtain->state.offset_x[1] == p.edge)
            ConfessionalStep(a, 14);
        return true;
    case 13:
        a->state.opacity = (Uint8)(a->state.opacity - 4U);
        if (a->state.opacity > 192)
        {
            a->state.frame = 0;
            ConfessionalStep(a, 14);
        }
        return true;
    case 14:
    case 128:
        a->state.frame = 0;
        return true;
    default:
        return SDL_SetError("Confessional ghost entered an invalid original state");
    }
}
