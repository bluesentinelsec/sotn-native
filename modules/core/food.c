// SPDX-License-Identifier: AGPL-3.0-or-later
// weapon/w_021.c:EntityWeaponAttack/func_ptr_80170004/func_97000_8017AF2C,
// sotn-decomp7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. No private tables copied.
#include <sotn/combat.h>
#include <sotn/food.h>

static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return bits < 32768U ? (Sint16)bits : (Sint16)((Sint32)bits - 65536);
}
static Sint32 Fixed(Uint32 bits)
{
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint16 Pixel(Sint32 value)
{
    return Word((Sint32)((Uint32)value >> 16));
}
static Sint32 Add(Sint32 a, Sint32 b)
{
    return Fixed((Uint32)a + (Uint32)b);
}
static Sint32 High(Sint32 value, Sint32 pixel)
{
    return Fixed(((Uint32)value & 65535U) | (Uint32)(Uint16)pixel << 16);
}
static bool Caught(SotnFood f, SotnFoodPlayer p, bool grounded, SotnFoodEvents *e)
{
    if (!grounded && f.velocity_y < 2 * 65536)
        return false;
    const Sint16 x = Pixel(f.x), y = Pixel(f.y), px = Pixel(p.x), py = Pixel(p.y);
    if (((f.parameter >> 8) & 127U) == 6)
    {
        if (p.frame != 0x7b || p.animation || y > py - 16 || y < py - 24 || x > px + 4 ||
            x < px - 4)
            return false;
        e->peanut_pose = true;
        return true;
    }
    const Sint16 dx = Word(SDL_abs((Sint32)x - px + p.offset_x));
    const Sint16 dy = Word(SDL_abs((Sint32)y - py + p.offset_y));
    return dx <= 8 + p.width && dy <= 8 + p.height;
}
static bool Contact(SotnFood *f, SotnFoodInput in, bool grounded, SotnFoodEvents *e)
{
    if (!Caught(*f, in.player, grounded, e))
        return false;
    if (f->inventory)
    {
        e->collect = true;
        e->item = (Uint16)((f->parameter >> 8) & 127U);
        f->removed = true;
    }
    else if (((f->parameter >> 8) & 127U) == 7)
    {
        f->timer = 12;
        f->phase = 4;
        f->frame = 0;
        e->simulated_ticks = 20;
        e->crouch = !grounded;
    }
    else if (!in.player.healing_pending)
    {
        e->heal = true;
        e->heal_amount = f->attack;
        f->removed = true;
    }
    else
    {
        f->timer = 8;
        f->frame = 0;
        f->phase = 3;
    }
    return true;
}
static bool Move(SotnFood *f, SotnFoodInput in)
{
    f->x = Add(f->x, f->velocity_x);
    f->y = Add(f->y, f->velocity_y);
    f->velocity_y = Add(f->velocity_y, 10240);
    SotnCollider c = {0};
    if (!SotnCollision_Query(&in.collision, Pixel(f->x), Word(Pixel(f->y) + 8), &c))
        return false;
    const bool peanut = ((f->parameter >> 8) & 127U) == 6;
    if (c.effects & SOTN_EFFECT_SOLID)
    {
        f->y = High(f->y, Pixel(f->y) + c.top);
        if (!f->inventory && peanut)
        {
            f->bounce /= 2;
            f->velocity_y = f->bounce;
            if (f->velocity_y <= -65536)
                f->y = Add(f->y, f->velocity_y);
            else
            {
                f->removed = true;
                return true;
            }
        }
        else
        {
            f->timer = f->inventory ? 512 : 128;
            f->phase = 2;
            return true;
        }
    }
    const Sint16 shift = f->velocity_x < 0 ? -4 : 4;
    if (!SotnCollision_Query(&in.collision, Word(Pixel(f->x) + shift), Pixel(f->y), &c))
        return false;
    if (c.effects & SOTN_EFFECT_SIDE)
    {
        f->x = High(f->x, Pixel(f->x) + (shift < 0 ? c.tile_right : c.tile_left));
        f->velocity_x /= -2;
        if (!f->inventory && peanut)
        {
            f->removed = true;
            return true;
        }
    }
    if (!SotnCollision_Query(&in.collision, Pixel(f->x),
                             Word(Pixel(f->y) + (!f->inventory && peanut ? -4 : -8)), &c))
        return false;
    if (c.effects & SOTN_EFFECT_SOLID)
    {
        f->y = High(f->y, Pixel(f->y) + 1 + c.bottom);
        f->velocity_x /= 2;
        f->velocity_y = 65536;
    }
    return true;
}
bool SotnFood_Tick(SotnFood *food, SotnFoodInput in, Uint32 *random, SotnFoodEvents *out)
{
    if (!food || !random || !out || food->removed || food->phase > (food->inventory ? 2 : 6) ||
        in.player.priority > 509 ||
        ((food->parameter >> 8) & 127U) > (food->inventory ? 69U : 41U) ||
        (food->inventory && ((food->parameter >> 8) & 127U) < 29))
        return SDL_SetError("Food requires its actual bounded weapon21 cursor and player");
    SotnFood f = *food;
    SotnFoodEvents e = {0};
    Uint32 rng = *random;
    switch (f.phase)
    {
    case 0:
        if (!f.inventory && ((f.parameter >> 8) & 127U) == 41)
        {
            const Uint16 selected = SotnCombat_Random(&rng) % 41U;
            f.parameter = (Uint16)((f.parameter & 0x80ffU) | selected << 8);
            f.item = (Uint16)(f.item - 41U + selected);
            f.phase = 5;
            f.frame = 0;
            break;
        }
        f.priority = (Uint16)(in.player.priority + 2U);
        f.facing_left = in.player.facing_left;
        f.flags = 0x08000000U;
        f.frame = f.inventory ? (Uint16)(((f.parameter >> 8) & 127U) - 28U)
                              : (Uint16)(((f.parameter >> 8) & 127U) + 1U);
        f.y = High(f.y, Pixel(f.y) - 4);
        f.velocity_x = (SotnCombat_Random(&rng) & 0x3fffU) + 57344;
        if (f.facing_left)
            f.velocity_x = -f.velocity_x;
        f.velocity_y = -131072;
        if (!f.inventory && ((f.parameter >> 8) & 127U) == 6)
        {
            f.x = High(f.x, Pixel(f.x) + (f.facing_left ? -8 : 8));
            f.velocity_x = (SotnCombat_Random(&rng) & 4095U) | 4096U;
            if (f.facing_left)
                f.velocity_x = -f.velocity_x;
            f.bounce = f.velocity_y = -360448;
        }
        if (!f.inventory)
        {
            // GetEquipProperties consumes its critical-rate rand before returning raw food ATK.
            (void)SotnCombat_Random(&rng);
            f.attack = in.attack;
        }
        e.clear_weapon = true;
        f.facing_left = false;
        f.phase = 1;
        break;
    case 1:
        if (!Contact(&f, in, false, &e) && !Move(&f, in))
            return false;
        break;
    case 2:
        if (f.inventory)
        {
            f.velocity_y = 0;
            f.timer = Word(f.timer - 1);
            if (!f.timer)
                f.removed = true;
            else
                (void)Contact(&f, in, true, &e);
        }
        else
        {
            if (f.velocity_x < 0)
                f.velocity_x = SDL_min(0, f.velocity_x + 18432);
            else
                f.velocity_x = SDL_max(0, f.velocity_x - 18432);
            f.x = Add(f.x, f.velocity_x);
            if (!Contact(&f, in, true, &e))
            {
                if (f.timer == 24)
                    f.draw_flags |= 0x80;
                f.timer = Word(f.timer - 1);
                f.removed = f.timer == 0;
            }
        }
        break;
    case 3:
        f.timer = Word(f.timer - 1);
        if (!f.timer)
        {
            if (!in.player.healing_pending)
            {
                e.heal = true;
                e.heal_amount = f.attack;
                f.removed = true;
            }
            else
                f.timer = 8;
        }
        break;
    case 4:
        f.timer = Word(f.timer - 1);
        if (!f.timer)
        {
            e.poison = true;
            f.removed = true;
        }
        break;
    case 5: {
        bool allocated = false;
        const Uint32 packed =
            56U + ((f.parameter & 0x8000U) ? 0x8000U : 0x4000U) + ((Uint32)f.item << 16);
        if (!in.factory || !in.factory(in.context, f.x, f.y, packed, &allocated))
            return SDL_SetError("Meal Ticket requires its actual optional factory service");
        if (allocated)
        {
            f.timer = 4;
            f.phase = 6;
        }
        break;
    }
    case 6:
        f.timer = Word(f.timer - 1);
        f.removed = f.timer == 0;
        break;
    }
    *food = f;
    *random = rng;
    *out = e;
    return true;
}
