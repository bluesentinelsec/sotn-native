// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/704D0.c ordinary spell commands and 5D5BC.c:CastSpell,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Spell data stays private.
#include <sotn/player.h>
#include <sotn/spell_commands.h>

static bool Expire(SotnSpellCommand *c)
{
    const Uint16 bits = (Uint16)((Uint16)c->timer - 1U);
    c->timer = bits <= 32767U ? (Sint16)bits : (Sint16)((Sint32)bits - 65536);
    if (!c->timer)
        c->phase = 0;
    return !c->timer;
}
static void Advance(SotnSpellCommand *c, Sint16 timer)
{
    ++c->phase;
    c->timer = timer;
}
static bool Ground(const SotnSpellCommandFrame *f)
{
    return f->step == SOTN_PLAYER_STAND || f->step == SOTN_PLAYER_WALK;
}
static bool Attack(const SotnSpellCommandFrame *f)
{
    return (f->tapped & (SOTN_BUTTON_LEFT_HAND | SOTN_BUTTON_RIGHT_HAND)) &&
           !(f->attack_mode & 0x8000U);
}
static bool Cast(SotnSpellCommand *c, SotnDataView dra, Uint16 id, SotnSpellCommandFrame *f,
                 SotnSpellCommandEvents *e)
{
    if (!(f->available & (1U << id)))
        return true;
    const size_t at = 0x8410U + id * 28U;
    if (!dra.bytes || at > dra.size || dra.size - at < 28)
        return SDL_SetError("Spell command requires complete private definition for spell%u", id);
    const Uint16 cost = dra.bytes[at + 12];
    if (f->mp < cost)
        return true;
    f->mp = (Uint16)(f->mp - cost);
    *e = (SotnSpellCommandEvents){
        .cast = true, .spell = id, .mp_spent = cost, .sword_notify = e->sword_notify};
    c->phase = 0;
    if (id == SOTN_SPELL_COMMAND_SWORD)
    {
        c->phase = 255;
        c->timer = 20;
        e->sword_notify = true;
    }
    return true;
}
static bool Dark(SotnSpellCommand *c, SotnDataView dra, SotnSpellCommandFrame *f,
                 SotnSpellCommandEvents *e)
{
    const Uint32 directions = f->pressed & 0xf000U;
    const Uint32 forward = c->facing_left ? SOTN_BUTTON_LEFT : SOTN_BUTTON_RIGHT,
                 backward = c->facing_left ? SOTN_BUTTON_RIGHT : SOTN_BUTTON_LEFT;
    switch (c->phase)
    {
    case 0:
        if (f->tapped & (f->facing_left ? SOTN_BUTTON_RIGHT : SOTN_BUTTON_LEFT))
        {
            Advance(c, 20);
            c->facing_left = f->facing_left;
        }
        break;
    case 1:
        if ((f->pressed & (SOTN_BUTTON_UP | backward)) == (SOTN_BUTTON_UP | backward))
            Advance(c, 20);
        else
            Expire(c);
        break;
    case 2:
        if (f->pressed == SOTN_BUTTON_UP)
            Advance(c, 20);
        else
            Expire(c);
        break;
    case 3:
        if ((directions & (SOTN_BUTTON_UP | forward)) == (SOTN_BUTTON_UP | forward))
            Advance(c, 20);
        else
            Expire(c);
        break;
    case 4:
        if (directions == forward)
            Advance(c, 20);
        else
            Expire(c);
        break;
    case 5:
        if (!Expire(c) && Attack(f) && Ground(f))
            return Cast(c, dra, SOTN_SPELL_COMMAND_DARK, f, e);
        break;
    }
    return true;
}
static bool Summon(SotnSpellCommand *c, SotnDataView dra, SotnSpellCommandFrame *f,
                   SotnSpellCommandEvents *e)
{
    const Uint32 directions = f->pressed & 0xf000U;
    const Uint32 forward = directions & (c->facing_left ? SOTN_BUTTON_LEFT : SOTN_BUTTON_RIGHT);
    switch (c->phase)
    {
    case 0:
        if (f->tapped == (Uint32)(f->facing_left ? SOTN_BUTTON_RIGHT : SOTN_BUTTON_LEFT))
        {
            Advance(c, 20);
            c->facing_left = f->facing_left;
        }
        break;
    case 1:
        // Original forward is the masked bit, so neutral also satisfies this equality.
        if (directions == forward)
            Advance(c, 20);
        else
            Expire(c);
        break;
    case 2:
        if (directions == SOTN_BUTTON_UP)
            Advance(c, 20);
        else
            Expire(c);
        break;
    case 3:
        if (directions == SOTN_BUTTON_DOWN)
            Advance(c, 20);
        else
            Expire(c);
        break;
    case 4:
        Expire(c); // Original still attempts a cast on the expiration frame.
        if (Attack(f) && f->step == SOTN_PLAYER_CROUCH)
            return Cast(c, dra, SOTN_SPELL_COMMAND_SUMMON, f, e);
        break;
    }
    return true;
}
static bool Hellfire(SotnSpellCommand *c, SotnDataView dra, SotnSpellCommandFrame *f,
                     SotnSpellCommandEvents *e)
{
    const Uint32 directions = f->pressed & 0xf000U,
                 forward = c->facing_left ? SOTN_BUTTON_LEFT : SOTN_BUTTON_RIGHT;
    switch (c->phase)
    {
    case 0:
        if (f->tapped == SOTN_BUTTON_UP)
            Advance(c, 20);
        break;
    case 1:
        if (directions == SOTN_BUTTON_DOWN)
        {
            Advance(c, 21);
            c->facing_left = f->facing_left;
        }
        else
            Expire(c);
        break;
    case 2:
        if ((directions & (SOTN_BUTTON_DOWN | forward)) == (SOTN_BUTTON_DOWN | forward))
            Advance(c, 20);
        else
            Expire(c);
        break;
    case 3:
        if (directions == forward)
            Advance(c, 20);
        else
            Expire(c);
        break;
    case 4:
        if (c->timer && Expire(c))
            break;
        if (Attack(f) && (Ground(f) || f->step == SOTN_PLAYER_CROUCH))
        {
            if (f->cramped)
                c->phase = 0;
            else
                return Cast(c, dra, SOTN_SPELL_COMMAND_HELLFIRE, f, e);
        }
        break;
    }
    return true;
}
static bool Tetra(SotnSpellCommand *c, SotnDataView dra, SotnSpellCommandFrame *f,
                  SotnSpellCommandEvents *e)
{
    const Uint32 directions = f->pressed & 0xf000U,
                 forward = c->facing_left ? SOTN_BUTTON_LEFT : SOTN_BUTTON_RIGHT;
    switch (c->phase)
    {
    case 0:
        if (f->tapped == SOTN_BUTTON_UP)
            Advance(c, 32);
        break;
    case 1:
        if (directions != SOTN_BUTTON_UP)
            c->phase = 0;
        else if (Expire(c))
        {
            c->phase = 2;
            c->facing_left = f->facing_left;
        }
        break;
    case 2:
        if (directions != SOTN_BUTTON_UP)
            Advance(c, 32);
        if ((directions & (SOTN_BUTTON_UP | forward)) == (SOTN_BUTTON_UP | forward))
        {
            c->phase = 4;
            c->timer = 20;
        }
        break;
    case 3:
        if ((directions & (SOTN_BUTTON_UP | forward)) == (SOTN_BUTTON_UP | forward))
            Advance(c, 20);
        else
            Expire(c);
        break;
    case 4:
        if (directions == forward)
            Advance(c, 20);
        else
            Expire(c);
        break;
    case 5:
        if ((directions & (SOTN_BUTTON_DOWN | forward)) == (SOTN_BUTTON_DOWN | forward))
            Advance(c, 20);
        else
            Expire(c);
        break;
    case 6:
        if (directions == SOTN_BUTTON_DOWN)
            Advance(c, 20);
        else
            Expire(c);
        break;
    case 7:
        Expire(c);
        if (Attack(f) && f->step == SOTN_PLAYER_CROUCH)
            return Cast(c, dra, SOTN_SPELL_COMMAND_TETRA, f, e);
        break;
    }
    return true;
}
static bool Soul(SotnSpellCommand *c, SotnDataView dra, SotnSpellCommandFrame *f,
                 SotnSpellCommandEvents *e)
{
    const Uint32 directions = f->pressed & 0xf000U,
                 forward = c->facing_left ? SOTN_BUTTON_LEFT : SOTN_BUTTON_RIGHT,
                 backward = c->facing_left ? SOTN_BUTTON_RIGHT : SOTN_BUTTON_LEFT;
    bool matched = false;
    switch (c->phase)
    {
    case 0:
        if (f->tapped == (Uint32)(f->facing_left ? SOTN_BUTTON_RIGHT : SOTN_BUTTON_LEFT))
        {
            Advance(c, f->facing_left ? 20 : 24);
            c->facing_left = f->facing_left;
        }
        return true;
    case 1:
    case 6:
        matched = directions == forward;
        break;
    case 2:
        matched = (directions & (SOTN_BUTTON_DOWN | forward)) == (SOTN_BUTTON_DOWN | forward);
        break;
    case 3:
        matched = directions == SOTN_BUTTON_DOWN;
        break;
    case 4:
        matched = (directions & (SOTN_BUTTON_DOWN | backward)) == (SOTN_BUTTON_DOWN | backward);
        break;
    case 5:
        matched = directions == backward;
        break;
    case 7:
        if (!Expire(c) && Attack(f) && Ground(f))
            return Cast(c, dra, SOTN_SPELL_COMMAND_SOUL, f, e);
        return true;
    }
    if (matched)
        Advance(c, 24);
    else
        Expire(c);
    return true;
}
static bool Sword(SotnSpellCommand *c, SotnDataView dra, SotnSpellCommandFrame *f,
                  SotnSpellCommandEvents *e)
{
    const Uint32 directions = f->pressed & 0xf000U,
                 forward = f->facing_left ? SOTN_BUTTON_LEFT : SOTN_BUTTON_RIGHT;
    if (!f->sword_active)
        c->phase = 0;
    switch (c->phase)
    {
    case 0:
        if (directions == SOTN_BUTTON_DOWN)
            Advance(c, 20);
        break;
    case 1:
        if (directions == (SOTN_BUTTON_DOWN | forward))
            Advance(c, 20);
        else
            Expire(c);
        break;
    case 2:
        if (directions == forward)
            Advance(c, 20);
        else
            Expire(c);
        break;
    case 3:
        if (directions == (SOTN_BUTTON_UP | forward))
            Advance(c, 20);
        else
            Expire(c);
        break;
    case 4:
        if (directions == SOTN_BUTTON_UP)
            Advance(c, 64);
        else
            Expire(c);
        break;
    case 5:
        if (Expire(c))
            c->phase = 6;
        else if (!(directions & SOTN_BUTTON_UP))
            c->phase = 0;
        break;
    case 6:
        if (!(directions & SOTN_BUTTON_UP))
            Advance(c, 20);
        break;
    case 7:
        if (directions == SOTN_BUTTON_DOWN)
            Advance(c, 20);
        else
            Expire(c);
        break;
    case 8:
        Expire(c);
        if (Attack(f) && f->step == SOTN_PLAYER_CROUCH)
            return Cast(c, dra, SOTN_SPELL_COMMAND_SWORD, f, e);
        break;
    case 255:
        if (Expire(c))
            e->sword_notify = true;
        break;
    }
    return true;
}
bool SotnSpellCommands_Tick(SotnSpellCommands *commands, SotnDataView dra,
                            SotnSpellCommandFrame *frame, SotnSpellCommandEvents *out)
{
    const Uint8 valid = 0xaf;
    if (!commands || !frame || !out || frame->mp > 9999 || (frame->available & ~valid))
        return SDL_SetError("Spell commands require original state and actual selected owners");
    const Sint16 limits[8] = {5, 4, 4, 7, 0, 7, 0, 8};
    for (Uint16 i = 0; i < 8; ++i)
        if (commands->spells[i].phase < 0 ||
            (commands->spells[i].phase > limits[i] &&
             !(i == SOTN_SPELL_COMMAND_SWORD && commands->spells[i].phase == 255)) ||
            ((i == 4 || i == 6) && (commands->spells[i].timer || commands->spells[i].facing_left)))
            return SDL_SetError("Spell command%u has an invalid original phase", i);
    SotnSpellCommands next = *commands;
    SotnSpellCommandFrame f = *frame;
    SotnSpellCommandEvents e = {0};
    typedef bool (*Command)(SotnSpellCommand *, SotnDataView, SotnSpellCommandFrame *,
                            SotnSpellCommandEvents *);
    const Command handlers[8] = {Dark, Summon, Hellfire, Tetra, NULL, Soul, NULL, Sword};
    const Uint8 order[6] = {2, 3, 5, 7, 1, 0};
    for (Uint16 i = 0; i < 6; ++i)
    {
        const Uint16 id = order[i];
        if (!handlers[id](&next.spells[id], dra, &f, &e))
            return false;
        if (e.cast)
            break;
    }
    *commands = next;
    *frame = f;
    *out = e;
    return true;
}
