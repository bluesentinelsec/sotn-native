// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/e_thornweed_corpseweed.h growing head at the pinned reconstruction.
// Original SDK trigonometry and animation data remain private runtime inputs.
#include "actors_internal.h"
static Actor *Root(SotnActors *s, const Actor *a)
{
    if (!a->slot)
        return NULL;
    Actor *p = &s->slots[a->slot - 1];
    return p->state.id == 0x3a && p->state.generation != 0 &&
                   p->state.generation == a->enemy_state.owner_generation
               ? p
               : NULL;
}
static Sint16 Mean(SotnActors *s, const ActorPrimitive *p, bool x)
{
    const Sint32 base = Pixel(x ? p->state.x : p->state.y) - (x ? s->camera_x : s->camera_y);
    const Sint16 *offset = x ? p->state.offset_x : p->state.offset_y;
    // Preserve signed C truncation in original screen coordinates, including odd negative sums.
    return (Sint16)((base * 2 + offset[2] + offset[3]) / 2 - base);
}
static void Brighten(ActorPrimitive *p, Uint8 decrement)
{
    p->state.modulate = true;
    p->state.red = (Uint8)SDL_max(0, (Sint32)p->state.red - decrement);
    p->state.green = (Uint8)SDL_max(0, (Sint32)p->state.green - decrement);
    p->state.blue = (Uint8)SDL_max(0, (Sint32)p->state.blue - decrement);
}
static bool Bob(SotnActors *s, Actor *a, ActorPrimitive *leaves, ActorPrimitive *stalk)
{
    if (a->plant.bob_timer)
    {
        --a->plant.bob_timer;
        a->plant.bob_angle += (a->plant.bob_timer & 16U) ? 1 : -1;
    }
    else
        a->plant.bob_angle = 8;
    SotnMathTrig trig = {0};
    if (a->plant.leaves_grown)
    {
        a->plant.leaves_x = (Sint16)Wrap16((Sint32)a->plant.leaves_x + 256);
        if (!SotnMath_Trig(s->executable, a->plant.leaves_x, &trig))
            return false;
        Sint32 amount = ActorShift((Sint64)trig.cosine * 2, 12), center = Mean(s, leaves, true);
        leaves->state.offset_x[0] = (Sint16)(center - 12 + amount);
        leaves->state.offset_x[1] = (Sint16)(center + 12 + amount);
        a->plant.leaves_y = (Sint16)Wrap16((Sint32)a->plant.leaves_y + 512);
        if (!SotnMath_Trig(s->executable, a->plant.leaves_y, &trig))
            return false;
        amount = ActorShift((Sint64)trig.sine * 4, 12);
        center = leaves->state.offset_y[2];
        leaves->state.offset_y[0] = (Sint16)(center - 40 + amount);
        leaves->state.offset_y[1] = (Sint16)(center - 40 - amount);
    }
    if (a->plant.stalk_grown)
    {
        a->plant.stalk_x = (Sint16)Wrap16((Sint32)a->plant.stalk_x + 56);
        if (!SotnMath_Trig(s->executable, a->plant.stalk_x, &trig))
            return false;
        Sint32 amount = ActorShift((Sint64)a->plant.bob_angle * trig.sine, 12),
               center = Mean(s, stalk, true);
        stalk->state.offset_x[0] = (Sint16)(center + (a->state.facing_left ? -20 : 20) + amount);
        stalk->state.offset_x[1] = (Sint16)(center + (a->state.facing_left ? 20 : -20) + amount);
        a->plant.stalk_y = (Sint16)Wrap16((Sint32)a->plant.stalk_y + 100);
        if (!SotnMath_Trig(s->executable, a->plant.stalk_y, &trig))
            return false;
        amount = ActorShift((Sint64)trig.sine * 4, 12);
        center = stalk->state.offset_y[2];
        stalk->state.offset_y[0] = (Sint16)(center - 112 + amount);
        stalk->state.offset_y[1] = (Sint16)(center - 112 - amount);
        const Sint32 x = Wrap16((Sint64)Pixel(stalk->state.x) + stalk->state.offset_x[0] +
                                (a->state.facing_left ? 12 : -12));
        const Sint32 y = Wrap16((Sint64)Pixel(stalk->state.y) + stalk->state.offset_y[0] + 12);
        a->state.x = x * 65536 + (Sint32)((Uint32)a->state.x & 65535U);
        a->state.y = y * 65536 + (Sint32)((Uint32)a->state.y & 65535U);
    }
    return true;
}
static bool Initialize(SotnActors *s, Actor *a)
{
    if (!ActorInitialize(s, a, 0x8e4))
        return false;
    a->state.frame = 0;
    a->state.hitbox_x = 2;
    a->state.hitbox_y = 9;
    a->state.hitbox_state = 0;
    a->plant.scale = 0;
    a->plant.bob_angle = 8;
    if (!ActorPrimitives(s, a, 2))
    {
        a->remove = true;
        return true;
    }
    if (a->state.priority < 2)
        return SDL_SetError("Corpseweed primitive priority underflow");
    for (Uint16 i = 0; i < 2; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        p->state = (SotnActorPrimitive){.x = Pixel(a->state.x) * 65536,
                                        .y = Pixel(a->state.y) * 65536,
                                        .quad = true,
                                        .textured = true,
                                        .active = true,
                                        .page = 0x14,
                                        .palette = 0x252,
                                        .red = 128,
                                        .green = 128,
                                        .blue = 128,
                                        .priority_set = true,
                                        .priority = (Uint16)(a->state.priority - 1 - i)};
        if (!i)
        {
            p->state.u[1] = p->state.u[3] = 24;
            p->state.v[0] = p->state.v[1] = 40;
            p->state.v[2] = p->state.v[3] = 80;
        }
        else
        {
            p->state.u[0] = p->state.u[2] = 64;
            p->state.u[1] = p->state.u[3] = 24;
            p->state.v[2] = p->state.v[3] = 112;
            p->state.offset_x[2] = a->state.facing_left ? -20 : 20;
            p->state.offset_x[3] = a->state.facing_left ? 20 : -20;
        }
    }
    return true;
}
static bool GrowLeaves(Actor *a, ActorPrimitive *p)
{
    if (!a->substep)
    {
        p->state.offset_x[0] = --p->state.offset_x[2];
        p->state.offset_x[1] = ++p->state.offset_x[3];
        if (p->state.offset_x[1] - p->state.offset_x[0] >= 24)
            ++a->substep;
        p->state.offset_y[0] = --p->state.offset_y[1];
        if (p->state.offset_y[2] - p->state.offset_y[0] >= 40)
            ++a->substep;
    }
    else if (a->substep == 1)
    {
        p->state.offset_y[0] = --p->state.offset_y[1];
        if (p->state.offset_y[2] - p->state.offset_y[0] >= 40)
            ++a->substep;
    }
    else if (a->substep == 2)
    {
        a->plant.leaves_grown = true;
        SetStep(a, 2);
    }
    else
        return SDL_SetError("Corpseweed leaf growth substep is invalid");
    return true;
}
static bool GrowStalk(Actor *a, ActorPrimitive *p)
{
    switch (a->substep)
    {
    case 0:
        p->state.offset_y[0] -= 2;
        p->state.offset_y[1] = p->state.offset_y[0];
        if (SDL_abs(p->state.offset_x[0]) < 10)
            p->state.offset_x[0] += a->state.facing_left ? -1 : 1;
        p->state.offset_x[1] += a->state.facing_left ? 1 : -1;
        if (SDL_abs(p->state.offset_x[1]) >= 16)
            ++a->substep;
        break;
    case 1: {
        Sint32 height = 0;
        if (a->state.facing_left)
        {
            --p->state.offset_y[0];
            height = p->state.offset_y[2] - p->state.offset_y[0];
            if (height < 89)
                --p->state.offset_y[1];
        }
        else
        {
            --p->state.offset_y[1];
            height = p->state.offset_y[2] - p->state.offset_y[1];
            if (height < 89)
                --p->state.offset_y[0];
        }
        if (height >= 112)
            ++a->substep;
        break;
    }
    case 2: {
        Sint32 height = 0;
        if (a->state.facing_left)
        {
            --p->state.offset_y[1];
            height = p->state.offset_y[3] - p->state.offset_y[1];
        }
        else
        {
            --p->state.offset_y[0];
            height = p->state.offset_y[3] - p->state.offset_y[0];
        }
        if (height >= 112)
            ++a->substep;
        break;
    }
    case 3: {
        Uint16 done = 0;
        if (SDL_abs(p->state.offset_x[0]) < 20)
            p->state.offset_x[0] += a->state.facing_left ? -1 : 1;
        else
            ++done;
        if (SDL_abs(p->state.offset_x[1]) < 20)
            p->state.offset_x[1] += a->state.facing_left ? 1 : -1;
        else
            ++done;
        if (done == 2)
        {
            a->plant.stalk_grown = true;
            a->state.hitbox_state = 3;
            SetStep(a, 3);
        }
        break;
    }
    default:
        return SDL_SetError("Corpseweed stalk growth substep is invalid");
    }
    return true;
}
static bool Attack(SotnActors *s, Actor *a)
{
    if (!a->substep)
    {
        a->plant.timer = 32;
        ++a->substep;
    }
    if (a->substep == 1)
    {
        Uint8 animation = 0;
        if (!ActorAnimate(s, a, 0x2bc0, &animation))
            return false;
        if (!a->plant.timer)
            return SDL_SetError("Corpseweed charge wait underflow");
        if (--a->plant.timer == 0)
        {
            a->substep = 2;
            a->pose = a->remaining = 0;
        }
    }
    else
    {
        if (a->substep == 2)
        {
            a->state.frame = 21;
            Actor *shot = NULL;
            if (!ActorSpawn(s, a, 0x3c, 0, 96, 128, &shot))
                return false;
            if (shot)
            {
                if (!ActorLibrarySound(s, a, 0x70d))
                    return false;
                if (a->state.priority >= 511)
                    return SDL_SetError("Corpseweed projectile priority overflow");
                shot->state.x =
                    Wrap16((Sint64)Pixel(a->state.x) + (a->state.facing_left ? -4 : 4)) * 65536;
                shot->state.y = Wrap16((Sint64)Pixel(a->state.y) + 13) * 65536;
                shot->state.priority = (Uint16)(a->state.priority + 1);
                shot->state.facing_left = a->state.facing_left;
            }
            a->plant.timer = 16;
            ++a->substep;
        }
        if (a->substep != 3 || !a->plant.timer)
            return SDL_SetError("Corpseweed attack reset substep is invalid");
        if (--a->plant.timer == 0)
            SetStep(a, 4);
    }
    return true;
}
static bool Death(SotnActors *s, Actor *a, ActorPrimitive *leaves, ActorPrimitive *stalk,
                  Actor *root)
{
    if (!a->substep)
    {
        a->plant.leaves_grown = a->plant.stalk_grown = false;
        a->state.hitbox_state = 0;
        a->state.rotated = true;
        a->state.scale_x = a->state.scale_y = 0;
        ++a->substep;
    }
    if (a->substep == 1)
    {
        const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
        if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32 ||
            a->vy > SDL_MAX_SINT32 - 6144)
            return SDL_SetError("Corpseweed falling head exceeds native fixed-point bounds");
        a->state.x = (Sint32)x;
        a->state.y = (Sint32)y;
        a->vy += 6144;
        a->state.rotation = (Sint16)Wrap16((Sint32)a->state.rotation + 32);
        SotnCollider c = {0};
        if (!ActorQuery(s, Pixel(a->state.x), Wrap16((Sint64)Pixel(a->state.y) + 8), &c))
            return false;
        if (c.effects & SOTN_EFFECT_SOLID)
        {
            if (!ActorSound(s, 0x683))
                return false;
            ShiftY(a, c.tile_top);
            Actor *effect = NULL;
            if (!ActorSpawn(s, a, 2, 1, 160, 192, &effect))
                return false;
            if (effect)
            {
                effect->state.x = Pixel(a->state.x) * 65536;
                effect->state.y = Pixel(a->state.y) * 65536;
                effect->state.facing_left = false;
            }
            a->state.frame = 0;
            root->state.flags |= 0x100U;
            if (!ActorLibrarySound(s, a, 0x672))
                return false;
            ++a->substep;
        }
    }
    else if (a->substep == 2)
    {
        ++a->plant.timer;
        if (a->plant.timer & 1U)
        {
            if (leaves->state.offset_x[1] > leaves->state.offset_x[0])
            {
                ++leaves->state.offset_x[0];
                --leaves->state.offset_x[1];
            }
            if (leaves->state.offset_y[0] < leaves->state.offset_y[2])
                ++leaves->state.offset_y[0];
            if (leaves->state.offset_y[1] < leaves->state.offset_y[3])
                ++leaves->state.offset_y[1];
            Brighten(leaves, 3);
        }
        if (a->state.facing_left)
        {
            if (stalk->state.offset_x[1] > stalk->state.offset_x[0])
            {
                ++stalk->state.offset_x[0];
                --stalk->state.offset_x[1];
            }
        }
        else if (stalk->state.offset_x[0] > stalk->state.offset_x[1])
        {
            ++stalk->state.offset_x[1];
            --stalk->state.offset_x[0];
        }
        if (stalk->state.offset_y[0] < stalk->state.offset_y[2])
            ++stalk->state.offset_y[0];
        if (stalk->state.offset_y[1] < stalk->state.offset_y[3])
            ++stalk->state.offset_y[1];
        if (stalk->state.v[2])
            --stalk->state.v[2];
        if (stalk->state.v[3])
            --stalk->state.v[3];
        Brighten(stalk, 1);
    }
    else
        return SDL_SetError("Corpseweed death substep is invalid");
    return true;
}
bool ActorLibraryCorpseweed(SotnActors *s, Actor *a)
{
    Actor *root = Root(s, a);
    if (!root)
    {
        a->remove = true;
        return true;
    }
    if ((a->state.flags & 0x100U) && a->state.step < 6)
        SetStep(a, 6);
    if (!a->state.step)
    {
        if (!Initialize(s, a))
            return false;
        if (a->remove)
            return true;
    }
    ActorPrimitive *leaves = ActorPrimitiveGet(s, a, 0), *stalk = ActorPrimitiveGet(s, a, 1);
    if (!leaves || !stalk)
        return false;
    switch (a->state.step)
    {
    case 1:
        if (!GrowLeaves(a, leaves))
            return false;
        break;
    case 2:
        if (!GrowStalk(a, stalk))
            return false;
        break;
    case 3:
        a->state.frame = 19;
        if (a->plant.scale < 256)
        {
            a->plant.scale = (Uint16)(a->plant.scale + 8);
            a->state.scale_x = a->state.scale_y = a->plant.scale;
        }
        else
        {
            a->state.scale_x = a->state.scale_y = 0;
            SetStep(a, 4);
        }
        break;
    case 4:
        if (!a->substep)
        {
            a->plant.timer = 128;
            ++a->substep;
        }
        a->state.frame = 19;
        if (!a->plant.timer)
            return SDL_SetError("Corpseweed attack wait underflow");
        if (--a->plant.timer == 0)
        {
            if ((Side(a, s->player) != 0) == a->state.facing_left)
                SetStep(a, 5);
            else
                a->plant.timer = 32;
        }
        if (a->hit_flags & 2U)
            a->plant.bob_timer = 32;
        break;
    case 5:
        if (!Attack(s, a))
            return false;
        break;
    case 6:
        if (!Death(s, a, leaves, stalk, root))
            return false;
        break;
    default:
        return SDL_SetError("Corpseweed entered an unsupported gameplay step");
    }
    return Bob(s, a, leaves, stalk);
}
