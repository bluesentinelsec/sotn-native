// SPDX-License-Identifier: AGPL-3.0-or-later
// st/e_bone_archer.h, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Original NO1 attack recipes, animations and debris motion remain private.
#include "actors_internal.h"
static bool Arrow(SotnActors *s, Actor *a)
{
    if (a->state.step == 0)
    {
        if (a->state.parameters > 2)
            return SDL_SetError("Bone Archer arrow fragment index is invalid");
        if (!ActorInitialize(s, a, 0xad0))
            return false;
        a->state.frame = 22;
        a->vx = a->state.facing_left ? 4 * 65536 : -4 * 65536;
        if (a->state.parameters)
        {
            a->state.hitbox_state = 0;
            a->state.rotated = true;
            a->state.frame = (Uint16)(a->state.frame + a->state.parameters);
            a->vx = (a->state.parameters == 1 ? -32768 : 32768) * (a->state.facing_left ? -1 : 1);
            a->state.step = 3;
            a->state.flags |= 0xc0202000;
            return true;
        }
    }
    switch (a->state.step)
    {
    case 1:
        a->state.x += a->vx;
        a->state.y += a->vy;
        if (a->state.flags & 0x100)
        {
            for (Uint16 i = 1; i <= 2; ++i)
            {
                Actor *part = NULL;
                if (!ActorSpawn(s, a, 0x45, i, 160, 192, &part))
                    return false;
                if (part != NULL)
                {
                    part->state.x = Pixel(a->state.x) * 65536;
                    part->state.y = Pixel(a->state.y) * 65536;
                    part->state.facing_left = false;
                }
            }
            a->remove = true;
            return true;
        }
        if (a->hit_flags & 0x80)
        {
            a->archer.x = Wrap16((Sint64)Pixel(s->player.x) - Pixel(a->state.x));
            a->archer.y = Wrap16((Sint64)Pixel(s->player.y) - Pixel(a->state.y));
            a->timer = 32;
            a->state.hitbox_state = 0;
            ++a->state.step;
        }
        break;
    case 2:
        a->state.x = Wrap16((Sint64)Pixel(s->player.x) - a->archer.x) * 65536;
        a->state.y = Wrap16((Sint64)Pixel(s->player.y) - a->archer.y) * 65536;
        if (!a->timer)
            return SDL_SetError("Bone Archer attached arrow timer underflow");
        if (--a->timer == 0)
            a->remove = true;
        break;
    case 3:
        a->state.x += a->vx;
        a->state.y += a->vy;
        a->vy += 8192;
        a->state.rotation =
            (Sint16)((a->state.rotation + (a->state.parameters == 1 ? -32 : 32)) & 4095);
        break;
    default:
        return SDL_SetError("Bone Archer arrow entered invalid state %u", a->state.step);
    }
    return true;
}
static bool Fire(SotnActors *s, const Actor *a, SotnDataView recipe)
{
    const Uint8 first = recipe.bytes[4], second = recipe.bytes[5];
    if (a->state.frame != first && a->state.frame != second)
        return true;
    if (a->state.frame != first && !Slice(s->nz0, 0x2cd4, 12, &recipe))
        return false;
    Actor *arrow = NULL;
    if (!ActorSpawn(s, a, 0x45, 0, 96, 128, &arrow))
        return false;
    if (arrow != NULL)
    {
        if (!ActorSound(s, 0x627))
            return false;
        const Sint32 x = Signed16(recipe.bytes + 6), y = Signed16(recipe.bytes + 8);
        if (SDL_abs(x) > 128 || SDL_abs(y) > 128)
            return SDL_SetError("Bone Archer private shot offset exceeds its bounded range");
        /* The original US routine adds xOffset for both facings. */
        arrow->state.x = (Pixel(a->state.x) + x) * 65536;
        arrow->state.y = (Pixel(a->state.y) + y) * 65536;
    }
    return true;
}
static bool Body(SotnActors *s, Actor *a)
{
    if (a->state.flags & 0x100)
        SetStep(a, 11);
    Uint16 collision = 0;
    Uint8 animation = 0;
    switch (a->state.step)
    {
    case 0:
        if (a->state.parameters > 8)
            return SDL_SetError(
                "Bone Archer body fragment index exceeds its private motion tables");
        if (!ActorInitialize(s, a, 0xac4))
            return false;
        if (a->state.parameters)
        {
            a->state.hitbox_state = 0;
            a->state.frame = (Uint16)(a->state.parameters + 24);
            a->state.flags |= 0xc0202000;
            a->state.step = 12;
        }
        else
            a->state.facing_left = Side(a, s->player) == 0;
        break;
    case 1:
        if (!ActorGround(s, a, 0x2bd8, &collision))
            return false;
        if (collision & 1)
            a->state.step = 2;
        break;
    case 2:
        a->state.facing_left = Side(a, s->player) == 0;
        if (Distance(a, s->player) < 128)
            SetStep(a, 3);
        break;
    case 3:
        if (!a->substep)
        {
            a->enemy_state.attack_timer = 64;
            ++a->substep;
        }
        a->state.frame = 1;
        a->state.facing_left = Side(a, s->player) == 0;
        if (Distance(a, s->player) < 96)
            SetStep(a, 13);
        if (!a->enemy_state.attack_timer)
            return SDL_SetError("Bone Archer attack timer underflow");
        if (--a->enemy_state.attack_timer == 0)
        {
            SetStep(a, (Uint16)(7 + a->enemy_state.attack_index));
            a->enemy_state.attack_index = (Uint16)((a->enemy_state.attack_index + 1) % 3);
        }
        break;
    case 13:
    case 14:
        if (!ActorAnimate(s, a, a->state.step == 13 ? 0x2c04 : 0x2c0c, &animation))
            return false;
        if (!animation)
            SetStep(a, a->state.step == 13 ? 5 : 3);
        break;
    case 4:
    case 5: {
        if (!a->substep)
        {
            a->enemy_state.timer = 64;
            ++a->substep;
        }
        if (!ActorAnimate(s, a, 0x2bf8, &animation))
            return false;
        if (!animation)
            a->state.facing_left = Side(a, s->player) == 0;
        if (!ActorWalk(s, a, 0x2be8, &collision))
            return false;
        a->vx = 49152 * (a->state.facing_left ? 1 : -1) * (a->state.step == 5 ? -1 : 1);
        Uint16 next = 0;
        if (a->state.step == 4)
        {
            if (Distance(a, s->player) < 64)
                SetStep(a, 5);
            if (collision & 128)
            {
                if (!ActorField(s, a, 0x2bf0, 2, false, &collision))
                    return false;
                if (collision & 2)
                    SetStep(a, 6);
            }
        }
        else
        {
            if (Distance(a, s->player) > 128)
                SetStep(a, 4);
            if (collision & 128)
            {
                a->state.frame = 1;
                a->enemy_state.timer = 1;
                next = 10;
            }
        }
        if (!a->enemy_state.timer)
            return SDL_SetError("Bone Archer movement timer underflow");
        if (--a->enemy_state.timer == 0)
        {
            SetStep(a, next ? next : (Uint16)(7 + a->enemy_state.attack_index));
            a->enemy_state.attack_index = (Uint16)((a->enemy_state.attack_index + 1) % 3);
        }
        break;
    }
    case 6:
        if (!a->substep)
        {
            a->vx = a->state.facing_left ? 65536 : -65536;
            a->vy = -4 * 65536;
            ++a->substep;
            a->state.frame = 14;
        }
        if (!ActorGround(s, a, 0x2bd8, &collision))
            return false;
        if (collision & 1)
            SetStep(a, 4);
        break;
    case 7:
    case 8:
    case 9:
    case 10:
        if (a->substep == 0)
            a->substep = a->state.step == 9 ? (a->state.hitbox_height == 14 ? 2 : 3)
                                            : (a->state.hitbox_height == 22 ? 1 : 3);
        else if (a->substep == 1 || a->substep == 2)
        {
            if (!ActorAnimate(s, a, a->substep == 1 ? 0x2c0c : 0x2c04, &animation))
                return false;
            if (!animation)
            {
                a->substep = 3;
                a->pose = a->remaining = 0;
            }
        }
        else if (a->substep == 3)
        {
            SotnDataView recipe = {0};
            if (!Slice(s->nz0, 0x2cd4 + (a->state.step - 7U) * 12U, 12, &recipe))
                return false;
            const Uint32 address = Read32(recipe.bytes);
            if (address < 0x80180000 || address - 0x80180000 >= s->nz0.size)
                return SDL_SetError("Bone Archer attack recipe animation pointer is invalid");
            const Uint16 pose = a->state.step == 7   ? 5
                                : a->state.step == 8 ? 1
                                : a->state.step == 9 ? 4
                                                     : 2;
            if (!a->remaining && a->pose == pose && !ActorSound(s, 0x6c9))
                return false;
            if (!ActorAnimate(s, a, address - 0x80180000, &animation))
                return false;
            if (!animation)
            {
                a->state.facing_left = Side(a, s->player) == 0;
                a->enemy_state.attack_timer = 64;
                SetStep(a, a->state.step == 9 ? 14 : 3);
            }
            else if (!a->remaining && !Fire(s, a, recipe))
                return false;
        }
        else
            return SDL_SetError("Bone Archer attack substate is invalid");
        break;
    case 11:
        for (Uint16 i = 0; i < 8; ++i)
        {
            Actor *part = NULL;
            if (!ActorSpawn(s, a, 0x44, (Uint16)(i + 1), 160, 192, &part))
                return false;
            if (part == NULL)
                break;
            SotnDataView vx = {0}, vy = {0}, time = {0};
            if (!Slice(s->nz0, 0x2d04 + i * 4U, 4, &vx) ||
                !Slice(s->nz0, 0x2d24 + i * 4U, 4, &vy) || !Slice(s->nz0, 0x2d44 + i, 1, &time))
                return false;
            const Sint32 horizontal = Signed32(vx.bytes), vertical = Signed32(vy.bytes);
            if (horizontal < -8 * 65536 || horizontal > 8 * 65536 || vertical < -8 * 65536 ||
                vertical > 8 * 65536 || !time.bytes[0])
                return SDL_SetError(
                    "Bone Archer fragment motion or lifetime exceeds its private bounds");
            part->state.x = Pixel(a->state.x) * 65536;
            part->state.y = Pixel(a->state.y) * 65536;
            part->vx = horizontal * (Side(a, s->player) ? 1 : -1);
            part->vy = vertical;
            part->timer = time.bytes[0];
        }
        /* The original death sound is conditional on the final explosion allocation. */
        {
            Actor *effect = NULL;
            if (!ActorSpawn(s, a, 2, 2, 160, 192, &effect))
                return false;
            if (effect != NULL)
            {
                effect->state.x = Pixel(a->state.x) * 65536;
                effect->state.y = Pixel(a->state.y) * 65536;
                effect->state.facing_left = false;
                if (!ActorSound(s, 0x62a))
                    return false;
            }
        }
        a->remove = true;
        return true;
    case 12:
        a->state.x += a->vx;
        a->state.y += a->vy;
        a->vy += 16384;
        if (!a->timer)
            return SDL_SetError("Bone Archer fragment timer underflow");
        if (--a->timer == 0)
        {
            if (!ActorExplosionInPool(s, a, 0, 160, 192, 0))
                return false;
            a->remove = true;
            return true;
        }
        break;
    default:
        return SDL_SetError("Bone Archer entered invalid state %u", a->state.step);
    }
    const Uint16 frame = a->state.frame;
    const bool standing = (frame > 10 && frame < 15) || frame > 32;
    a->state.hitbox_x = standing ? 4 : 0;
    a->state.hitbox_y = standing ? -7 : 1;
    a->state.hitbox_width = standing ? 8 : 17;
    a->state.hitbox_height = standing ? 22 : 14;
    return true;
}
bool ActorArcher(SotnActors *s, Actor *a)
{
    return a->state.id == 0x45 ? Arrow(s, a) : Body(s, a);
}
