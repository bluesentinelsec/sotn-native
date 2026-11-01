// SPDX-License-Identifier: AGPL-3.0-or-later
// weapon/w_045.c callbacks00/04/08/0C/10 and dra/71830.c:PlayerStepUnk50,
// sotn-decomp7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Tables/artwork stay private.
#include <SDL3/SDL_error.h>
#include <sotn/axe_armor.h>

static bool Valid(const SotnAxeArmorFrame *f)
{
    return f && f->animation.id < 247 && f->animation.pose < 4096 && f->animation.remaining >= -1 &&
           f->animation.remaining <= 32767;
}
static void Animate(SotnAxeArmorFrame *f, Uint16 animation)
{
    f->animation.id = animation;
    f->animation.pose = 0;
    f->animation.remaining = 0;
}
static void Speed(SotnAxeArmorFrame *f, Sint32 speed)
{
    f->velocity_x = f->facing_left ? -speed : speed;
}
static void Decelerate(SotnAxeArmorFrame *f, Sint32 amount)
{
    if (f->velocity_x < 0)
        f->velocity_x = SDL_min(0, f->velocity_x + amount);
    else
        f->velocity_x = SDL_max(0, f->velocity_x - amount);
}
static int Direction(SotnAxeArmorFrame *f, Uint16 pressed)
{
    if (f->movement_flags & 2U)
        return 0;
    const bool right = (pressed & 0x2000U) != 0, left = (pressed & 0x8000U) != 0;
    if (f->facing_left ? right : !right && left)
    {
        f->facing_left = !f->facing_left;
        f->turned = true;
        return -1;
    }
    return f->facing_left ? (left ? 1 : 0) : (right ? 1 : 0);
}
static void Attack(SotnAxeArmorFrame *f, SotnAxeArmorEvents *e)
{
    Animate(f, 0xce);
    f->substep = 1;
    e->sounds[e->sound_count++] = 0x6f0;
    e->factories[e->factory_count++] = 87;
}
static Sint32 Half(Sint32 n)
{
    return (Sint32)(n >= 0 ? n / 2 : -((-(Sint64)n + 1) / 2));
}
static void Gravity(SotnAxeArmorFrame *f, Sint32 amount)
{
    const Uint32 bits = (Uint32)f->velocity_y + (Uint32)amount;
    SDL_memcpy(&f->velocity_y, &bits, sizeof(bits));
    f->velocity_y = SDL_min(7 * 65536, f->velocity_y);
}
bool SotnAxeArmor_Enter(SotnAxeArmorFrame *frame, Uint32 contacts)
{
    if (!Valid(frame))
        return SDL_SetError("Axe Armor entry requires actual bounded player animation fields");
    SotnAxeArmorFrame f = *frame;
    f.step = (contacts & 1U) ? 40 : 42;
    f.substep = 0;
    f.velocity_x = f.velocity_y = 0;
    Animate(&f, (contacts & 1U) ? 0xcf : 0xcd);
    *frame = f;
    return true;
}
bool SotnAxeArmor_Tick(SotnAxeArmor *state, SotnAxeArmorFrame *frame, SotnAxeArmorInput in,
                       SotnAxeArmorEvents *out)
{
    if (!state || !out || !Valid(frame) ||
        (frame->step != 40 && frame->step != 41 && frame->step != 42 && frame->step != 43 &&
         frame->step != 50) ||
        ((frame->step == 40 || frame->step == 42) && frame->substep > 1) ||
        (frame->step == 43 && frame->substep > 2))
        return SDL_SetError("Axe Armor requires its original40..43/50 callback and substate");
    SotnAxeArmor s = *state;
    SotnAxeArmorFrame f = *frame;
    SotnAxeArmorEvents e = {0};
    switch (f.step)
    {
    case 40:
        Decelerate(&f, 8192);
        if (!(in.contacts & 1U))
        {
            f.step = 42;
            f.substep = 0;
            f.velocity_x = f.velocity_y = 0;
            Animate(&f, 0xd0);
        }
        else if (!f.substep)
        {
            if (Direction(&f, in.pressed))
            {
                f.step = 41;
                f.substep = 0;
                Speed(&f, 163840);
                Animate(&f, 0xcd);
            }
            else if (in.tapped & 0x80U)
                Attack(&f, &e);
            else if ((in.tapped & 0x40U) && !in.tight_space)
            {
                f.step = 42;
                f.substep = 0;
                f.velocity_y = -4 * 65536;
                Animate(&f, 0xd0);
                (void)Direction(&f, in.pressed);
            }
            else if (f.animation.pose == 1 || f.animation.pose == 4 || f.animation.pose == 7)
                e.sounds[e.sound_count++] = 0x6ee;
            else if (f.animation.pose == 10)
                e.sounds[e.sound_count++] = 0x6ef;
        }
        else if (f.animation.remaining < 0)
        {
            f.substep = 0;
            Animate(&f, 0xcf);
        }
        break;
    case 41:
        Speed(&f, 163840);
        if (!(in.contacts & 1U))
        {
            f.step = 42;
            f.substep = 0;
            f.velocity_y = 0;
            Animate(&f, 0xd0);
        }
        else if (!Direction(&f, in.pressed))
        {
            f.step = 40;
            f.substep = 0;
            f.velocity_x = Half(f.velocity_x);
            Animate(&f, 0xcf);
        }
        else if (in.tapped & 0x80U)
        {
            f.step = 40;
            f.velocity_x = Half(f.velocity_x);
            Attack(&f, &e);
        }
        else if ((in.tapped & 0x40U) && !in.tight_space)
        {
            f.step = 42;
            f.substep = 0;
            f.velocity_y = -4 * 65536;
            Animate(&f, 0xd0);
            (void)Direction(&f, in.pressed);
        }
        else if (!(in.clock & 7U))
        {
            e.sounds[e.sound_count++] = 0x64b;
            e.factories[e.factory_count++] = 0x10045;
        }
        break;
    case 42:
        Gravity(&f, 22528);
        if (in.contacts & 1U)
        {
            f.step = 40;
            if (f.substep)
                f.velocity_x = f.velocity_y = 0;
            else if (Direction(&f, in.pressed))
            {
                f.step = 41;
                f.velocity_y = 0;
                Speed(&f, 163840);
                Animate(&f, 0xcd);
            }
            else
            {
                f.velocity_x = f.velocity_y = 0;
                Animate(&f, 0xcf);
            }
            e.sounds[e.sound_count++] = 0x64b;
        }
        else
        {
            Decelerate(&f, 4096);
            if (Direction(&f, in.pressed))
                Speed(&f, 49152);
            if (!f.substep && (in.tapped & 0x80U))
                Attack(&f, &e);
            else if (f.substep && f.animation.remaining < 0)
            {
                f.substep = 0;
                Animate(&f, 0xd0);
            }
        }
        break;
    case 43:
        s.angle += 128U;
        f.draw_flags = 4;
        if (!f.substep)
        {
            f.rotation = f.knock_right == f.facing_left ? -512 : 512;
            f.pivot_y = 20;
            ++f.substep;
        }
        else if (f.substep == 1)
        {
            Gravity(&f, 11264);
            if (in.contacts & 2U)
                f.velocity_y = 65536;
            if (in.contacts & 1U)
            {
                f.rotation = f.knock_right == f.facing_left ? -896 : 896;
                f.substep = 2;
                f.velocity_y = 0;
                s.recovery = 48;
                e.sounds[e.sound_count++] = 0x64b;
                e.factories[e.factory_count++] = 0;
            }
        }
        else
        {
            SotnMathTrig trig = {0};
            if (!SotnMath_Trig(in.executable, (Sint32)(s.angle & 4095U), &trig))
                return false;
            // Original signed arithmetic shift before subtracting the opacity bias.
            const Sint32 sine = trig.sine;
            f.opacity = (Uint8)((sine >= 0 ? sine / 128 : -((-sine + 127) / 128)) - 64);
            f.draw_flags = 4U | 8U;
            if (!(in.contacts & 1U))
            {
                f.step = 42;
                f.substep = 0;
                f.velocity_y = 0;
                f.rotation = 0;
                f.draw_flags = 0;
                Animate(&f, 0xd0);
            }
            else
            {
                Decelerate(&f, 8192);
                if (s.recovery == 5)
                    f.rotation = f.knock_right == f.facing_left ? -512 : 512;
                const Uint32 bits = (Uint32)s.recovery - 1U;
                SDL_memcpy(&s.recovery, &bits, sizeof(bits));
                if (!s.recovery)
                {
                    f.step = 40;
                    f.substep = 0;
                    f.velocity_y = 0;
                    f.rotation = 0;
                    f.draw_flags = 0;
                    Animate(&f, 0xcf);
                }
            }
        }
        break;
    case 50:
        f.velocity_x = f.velocity_y = 0;
        if (in.simulated_mode != 2)
        {
            f.step = 40;
            f.substep = 0;
            Animate(&f, 0xcf);
        }
        break;
    }
    *state = s;
    *frame = f;
    *out = e;
    return true;
}
