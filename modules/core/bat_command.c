// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/71830.c:CheckWingSmashInput/ControlBatForm and 5D5BC.c:CastSpell,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Spell tables remain private.
#include <sotn/bat_command.h>
#include <sotn/player.h>

static Sint32 Decelerate(Sint32 value, Sint32 amount)
{
    return value > 0 ? SDL_max(0, value - amount) : value < 0 ? SDL_min(0, value + amount) : 0;
}
static void Rotate(SotnBatCommandFrame *f, Sint16 target)
{
    if (f->rotation < target)
        f->rotation = (Sint16)SDL_min(target, f->rotation + 16);
    if (f->rotation > target)
        f->rotation = (Sint16)SDL_max(target, f->rotation - 16);
}
static bool Combo(SotnBatCommand *c, Uint16 buttons, bool left)
{
    const Uint16 directions = buttons & 0xf000U;
    const Uint16 backward = left ? SOTN_BUTTON_RIGHT : SOTN_BUTTON_LEFT,
                 forward = left ? SOTN_BUTTON_LEFT : SOTN_BUTTON_RIGHT;
    Uint16 expected = 0;
    switch (c->counter)
    {
    case 0:
        expected = SOTN_BUTTON_UP;
        break;
    case 1:
        expected = backward | SOTN_BUTTON_UP;
        break;
    case 2:
        expected = backward;
        break;
    case 3:
        expected = backward | SOTN_BUTTON_DOWN;
        break;
    case 4:
        expected = SOTN_BUTTON_DOWN;
        break;
    case 5:
        expected = forward | SOTN_BUTTON_DOWN;
        break;
    case 6:
        expected = forward;
        break;
    default:
        break;
    }
    if (c->counter < 7 && directions == expected)
    {
        ++c->counter;
        c->timer = 20;
    }
    else if (--c->timer == 0)
        c->counter = 0;
    if (!(buttons & SOTN_BUTTON_JUMP) && c->counter != 7)
        c->counter = 0;
    return c->counter == 7;
}
static bool Cast(SotnDataView dra, Uint16 id, SotnBatCommandFrame *f, SotnBatCommandEvents *events,
                 bool *cast)
{
    const size_t at = 0x8410U + id * 28U;
    if (!dra.bytes || at > dra.size || dra.size - at < 28)
        return SDL_SetError("Bat command requires its complete private spell definition");
    const Uint16 cost = dra.bytes[at + 12];
    *cast = f->mp >= cost;
    if (*cast)
    {
        f->mp = (Uint16)(f->mp - cost);
        events->mp_spent = cost;
    }
    return true;
}
bool SotnBatCommand_Tick(SotnBatCommand *command, SotnDataView dra, SotnBatCommandFrame *frame,
                         SotnBatCommandEvents *out)
{
    if (!command || !frame || !out || command->counter > 7 || frame->substep > 5 ||
        frame->rotation < -4096 || frame->rotation > 4096 || frame->mp > 9999 ||
        frame->velocity_x < -16 * 65536 || frame->velocity_x > 16 * 65536 ||
        frame->velocity_y < -16 * 65536 || frame->velocity_y > 16 * 65536)
        return SDL_SetError("Bat command requires bounded original controller state");
    SotnBatCommand c = *command;
    SotnBatCommandFrame f = *frame;
    SotnBatCommandEvents e = {0};
    const bool complete = Combo(&c, f.buttons, f.facing_left);
    const bool jump = (f.buttons & SOTN_BUTTON_JUMP) != 0;
    const Uint16 directions = f.buttons & 0xf000U;
    bool cast = false;
    if (complete && !jump && f.substep)
    {
        if (!Cast(dra, 6, &f, &e, &cast))
            return false;
    }
    if (cast)
    {
        if (!SotnAnimation_Set(&f.animation, 0xc6))
            return false;
        f.velocity_x = f.facing_left ? -6 * 65536 : 6 * 65536;
        f.substep = 3;
        c.duration = 64;
        c.counter = 0;
        e.learn_wing_smash = e.wing_flash = e.wing_attack = true;
    }
    else if ((f.tapped & SOTN_BUTTON_CANCEL) && (f.substep == 1 || f.substep == 2) &&
             f.echo_enabled)
        e.echo = true;
    else if ((f.tapped & (SOTN_BUTTON_LEFT_HAND | SOTN_BUTTON_RIGHT_HAND)) &&
             (f.substep == 1 || f.substep == 2) && f.fire_enabled)
    {
        if (!Cast(dra, 9, &f, &e, &cast))
            return false;
        if (cast)
        {
            if (!SotnAnimation_Set(&f.animation, 0xc9))
                return false;
            f.substep = 4;
            e.fire_flash = true;
        }
    }
    switch (f.substep)
    {
    case 0:
        c.counter = 0;
        break;
    case 1:
    case 2:
        break;
    case 3:
        e.handled = true;
        if (f.contacts & (f.facing_left ? 8U : 4U))
        {
            e.return_requested = e.wall_impact = true;
            f.velocity_x = 0;
            break;
        }
        if (--c.duration == 0)
        {
            e.return_requested = true;
            break;
        }
        if (directions & SOTN_BUTTON_UP)
        {
            f.velocity_y -= 8192;
            Rotate(&f, 128);
        }
        if (directions & SOTN_BUTTON_DOWN)
            f.velocity_y += 8192;
        if (!(directions & SOTN_BUTTON_UP))
            Rotate(&f, 384);
        if (!(directions & (SOTN_BUTTON_UP | SOTN_BUTTON_DOWN)))
            f.velocity_y = Decelerate(f.velocity_y, 8192);
        if ((f.contacts & 0x800U) &&
            (f.facing_left ? (f.contacts & 0x400U) != 0 : !(f.contacts & 0x400U)))
            f.velocity_y = 6 * 65536;
        if ((f.contacts & 0x8000U) &&
            (f.facing_left ? (f.contacts & 0x4000U) != 0 : !(f.contacts & 0x4000U)))
            f.velocity_y = -6 * 65536;
        f.velocity_y = SDL_clamp(f.velocity_y, -6 * 65536, 6 * 65536);
        if (f.game_timer % 3U == 0)
        {
            e.trail = true;
            e.floor_dust = (f.contacts & 1U) != 0;
            e.ceiling_dust = (f.contacts & 2U) != 0;
        }
        break;
    case 4:
        e.handled = true;
        f.velocity_x = Decelerate(f.velocity_x, 6144);
        f.velocity_y = Decelerate(f.velocity_y, 14336);
        Rotate(&f, 384);
        if (f.animation.remaining < 0)
        {
            e.fireball = true;
            f.velocity_x = f.facing_left ? 98304 : -98304;
            Rotate(&f, 0);
            if (!SotnAnimation_Set(&f.animation, 0xc3))
                return false;
            ++f.substep;
        }
        break;
    case 5:
        e.handled = true;
        f.velocity_x = Decelerate(f.velocity_x, 6144);
        f.velocity_y = Decelerate(f.velocity_y, 14336);
        Rotate(&f, 0);
        if (f.animation.id == 0xc4 && f.animation.pose == 8)
            f.substep = 1;
        break;
    }
    *command = c;
    *frame = f;
    *out = e;
    return true;
}
