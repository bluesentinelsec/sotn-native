// SPDX-License-Identifier: AGPL-3.0-or-later
// Weapon func_ptr_80170004 secondaries: weapon/w_008.c, w_009.c, w_011.c, w_025.c, w_058.c,
// w_030.c, w_034.c, w_038.c, w_039.c, w_055.c and weapon/shared.h, plus dra/7879C.c UpdateAnim,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Descriptor, timeline and artwork tables
// remain private inputs.
#include "boss_projection.h"
#include <sotn/combat.h>
#include <sotn/game_math.h>
#include <sotn/weapon_secondary.h>

static Sint16 Word(Sint32 n)
{
    const Uint16 v = (Uint16)(Uint32)n;
    return v < 32768 ? (Sint16)v : (Sint16)((Sint32)v - 65536);
}
static Sint16 Pixel(Sint32 n)
{
    return Word((Sint32)((Uint32)n >> 16));
}
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static Uint32 Read32(const Uint8 *p)
{
    return Read16(p) | (Uint32)Read16(p + 2) << 16;
}
// Portable arithmetic right shift matching the MIPS sra the original compiles to.
static Sint32 Asr(Sint32 v, unsigned int n)
{
    return v >= 0 ? v >> n : -(Sint32)(((Uint32)(-(v + 1)) >> n) + 1U);
}

typedef struct Layout
{
    Uint32 frame_start; /**< w_055 D_8017B0B8: the descriptor table's frameStart byte. */
    Uint32 timeline;    /**< w_055 private AnimationFrame list. */
    Uint16 timeline_count, rows;
    Uint16 bank, palette, texture;
} Layout;
static bool Program(Uint16 family, Layout *out)
{
    if (family == 55)
        *out = (Layout){.frame_start = 0x10b8,
                        .timeline = 0x1174,
                        .timeline_count = 5,
                        .rows = 64,
                        .bank = 0x8011,
                        .palette = 0x117,
                        .texture = 0x65};
    else if (family == 38 || family == 39)
        *out = (Layout){.rows = family == 38 ? 24 : 32};
    else if (family == 53 || family == 56)
        *out = (Layout){.rows = 1};
    else if (family == 48)
        *out = (Layout){.rows = 10};
    else if (family == 1)
        *out = (Layout){.timeline = 0xfc8, .timeline_count = 9, .rows = 16};
    else if (family == 51)
        *out = (Layout){.rows = 1};
    else if (family == 49)
        *out = (Layout){.rows = 0};
    else if (family == 50)
        *out = (Layout){.rows = 80};
    else if (family == 6)
        *out = (Layout){.rows = 0};
    else if (family == 14)
        *out = (Layout){.rows = 48};
    else if (family == 13)
        *out = (Layout){.rows = 0};
    // Weapon7 (Shield rod): the orb loops D_8017AB44's four poses.
    else if (family == 7)
        *out = (Layout){.timeline = 0xb44, .timeline_count = 5, .rows = 0};
    else if (family == 12)
        *out = (Layout){.rows = 1};
    else if (family == 34)
        *out = (Layout){.rows = 0};
    else if (family == 30)
        *out = (Layout){.rows = 11};
    else if (SotnWeaponSecondary_ShieldSpell(family))
        *out = (Layout){.timeline = family == 10   ? 0xba0U
                                    : family == 23 ? 0xb58U
                                                   : 0U,
                        .timeline_count = family == 10   ? 7
                                          : family == 23 ? 6
                                                         : 0,
                        .rows = family == 9    ? 12
                                : family == 10 ? 33
                                : family == 23 ? 18
                                : family == 24 ? 18
                                : family == 27 ? 1
                                : family == 28 ? 13
                                : family == 29 ? 1
                                : family == 58 ? 2
                                : family == 11 ? 10
                                : family == 25 ? 17
                                : family == 26 ? 14
                                : family == 52 ? 23
                                               : 11};
    else
        return SDL_SetError("Weapon secondary requires a selected actual resident program");
    return true;
}

bool SotnWeaponSecondary_ShieldSpell(Uint16 family)
{
    return family == 8 || family == 9 || family == 10 || family == 23 || family == 24 ||
           family == 27 || family == 28 || family == 29 || family == 58 || family == 11 ||
           family == 25 || family == 26 || family == 52;
}
Uint16 SotnWeaponSecondary_ShieldSpellItem(Uint16 family)
{
    return family == 8    ? 203
           : family == 9  ? 204
           : family == 10 ? 205
           : family == 23 ? 207
           : family == 24 ? 208
           : family == 27 ? 211
           : family == 28 ? 212
           : family == 29 ? 213
           : family == 58 ? 215
           : family == 11 ? 206
           : family == 25 ? 209
           : family == 26 ? 210
           : family == 52 ? 214
                          : 0;
}
Uint16 SotnWeaponSecondary_ShieldSpellFamily(Uint16 item)
{
    return item == 203   ? 8
           : item == 204 ? 9
           : item == 205 ? 10
           : item == 207 ? 23
           : item == 208 ? 24
           : item == 211 ? 27
           : item == 212 ? 28
           : item == 213 ? 29
           : item == 215 ? 58
           : item == 206 ? 11
           : item == 209 ? 25
           : item == 210 ? 26
           : item == 214 ? 52
                         : 0;
}
bool SotnWeaponSecondary_Supported(Uint16 family)
{
    return family == 55 || family == 38 || family == 39 || family == 53 || family == 56 ||
           family == 48 || family == 1 || family == 51 || family == 49 || family == 50 ||
           family == 6 || family == 14 || family == 13 || family == 12 || family == 34 ||
           family == 30 || family == 7 || SotnWeaponSecondary_ShieldSpell(family);
}
bool SotnWeaponSecondary_SupportedRole(Uint16 family, Uint16 role)
{
    // Weapon51's role1 is the slot16 hand-weapon special; only its role2 fan is a secondary.
    if (!SotnWeaponSecondary_Supported(family))
        return false;
    if (family == 51)
        return role == 2;
    // Weapon49 owns sparks, ring, puffs, swirls and pillars (E1..E5).
    if (family == 49)
        return role >= 1 && role <= 5;
    // Weapon50 owns sparkles, its ring, frost streaks and crystals (E1..E4).
    if (family == 50)
        return role >= 1 && role <= 4;
    // Weapon6 owns Moon rod's crescents and their sparkles.
    if (family == 6)
        return role == 1 || role == 2;
    // Weapon14/13 blueprint50 roots run EntityWeaponAttack outside slot16; their E1 hitboxes or
    // afterimages follow them.
    // Weapon13's both-buttons combo202 adds its sweep (role2) and strike swords (role3).
    if (family == 14)
        return role <= 1;
    if (family == 13)
        return role <= 3;
    // Weapon7: Shield rod special173's orb (role1) and its sparkles (role2).
    if (family == 7)
        return role == 1 || role == 2;
    // Weapon12: role0 Vorpal/Crissaegrim slashes, role1 the Chakram, role2 its afterimages.
    if (family == 12)
        return role <= 2;
    // Weapon10 (Iron shield): its guard's thunder (role1), spell (role8) and swords (role9).
    if (family == 10)
        return role == 1 || role == 8 || role == 9;
    // Weapon23 (Herald shield): its spell (role8), swirl (role9) and rising sprites (role10).
    if (family == 23)
        return role == 8 || role == 9 || role == 10;
    // Weapon24 (Dark shield): its spell (role8) and rising rocks (role9).
    if (family == 24)
        return role == 8 || role == 9;
    // Weapon27 (Medusa shield): its spell (role8), lasers (role9) and hit points (role10).
    if (family == 27)
        return role == 8 || role == 9 || role == 10;
    // Weapon28 (Skull shield): its spell (role8) and beam (role9).
    if (family == 28)
        return role == 8 || role == 9;
    // Weapon29 (Fire shield): its guard's afterimages (role1), spell (role8), dragons (role9) and
    // hit points (role10).
    if (family == 29)
        return role == 1 || role == 8 || role == 9 || role == 10;
    // Shield programs (Leather8/Knight9/Alucart58): role8 is the both-buttons spell.
    // Axe Lord's spell (role8) also creates its vector9 walker/afterimages (role9); Goddess's
    // its vector9 clouds.
    if (family == 11 || family == 25 || family == 26)
        return role == 8 || role == 9;
    // Alucard shield's spell (role8) and its expiring armed shield's 0x3A letters (role1).
    if (family == 52)
        return role == 8 || role == 1;
    if (SotnWeaponSecondary_ShieldSpell(family))
        return role == 8;
    // Weapon30: role1 Sword of Dawn's circle, role2 its soldiers, role3 arrows/swords, role4
    // the bombs.
    if (family == 30)
        return role >= 1 && role <= 4;
    // Weapon34's role1 is Shotel's thrown special176.
    if (family == 34)
        return role == 1;
    // Weapon53's role3 is Yasutsuna's blueprint68 slash; its chain uses role1.
    if (family == 53)
        return role == 1 || role == 3;
    return role == 1 || (role == 2 && (family == 48 || family == 1));
}
Uint8 SotnWeaponSecondary_RoleRows(Uint16 family, Uint16 role)
{
    if (!SotnWeaponSecondary_SupportedRole(family, role))
        return 0;
    if (family == 49)
        return role == 2 ? 16 : 0;
    if (family == 50)
        return role == 1 ? 80 : role == 2 ? 16 : role == 3 ? 32 : 8;
    if (family == 6)
        return role == 2 ? 1 : 0;
    if (family == 14)
        return role == 0 ? 48 : 0;
    if (family == 13)
        return 0;
    // Weapon7's sparkles take one optional GT4 (failure retires them before their rand calls).
    if (family == 7)
        return role == 2 ? 1 : 0;
    if (family == 12)
        return role == 0 ? 1 : 0;
    if (family == 34)
        return 0;
    if (family == 30)
        return role == 1 ? 11 : role == 2 ? 14 : 0;
    if (SotnWeaponSecondary_ShieldSpell(family))
        return role == 8                      ? (family == 9    ? 12
                                                 : family == 10 ? 33
                                                 : family == 23 ? 18
                                                 : family == 24 ? 18
                                                 : family == 27 ? 1
                                                 : family == 28 ? 13
                                                 : family == 29 ? 1
                                                 : family == 58 ? 2
                                                 : family == 11 ? 10
                                                 : family == 25 ? 17
                                                 : family == 26 ? 14
                                                 : family == 52 ? 23
                                                                : 11)
               : family == 10                 ? (role == 9 ? 18 : 0)
               : family == 23                 ? (role == 9 ? 4 : 0)
               : family == 24                 ? (role == 9 ? 1 : 0)
               : family == 27                 ? (role == 9 ? 18 : 0)
               : family == 28                 ? (role == 9 ? 3 : 0)
               : family == 29                 ? (role == 9 ? 19 : 0)
               : family == 25 || family == 26 ? 1
               : family == 52                 ? 11
                                              : 0;
    if (role == 2)
        return family == 1 ? 0 : family == 51 ? 1 : 16;
    return SotnWeaponSecondary_Rows(family);
}
Uint8 SotnWeaponSecondary_ChildRows(Uint16 family, Uint16 role, Uint16 parameter)
{
    // Weapon26's vector9 stars take one GT4; their params1 trails four.
    if (family == 26 && role == 9)
        return (parameter & 0x7f00U) ? 4 : 1;
    return SotnWeaponSecondary_RoleRows(family, role);
}
Uint8 SotnWeaponSecondary_HeldRows(Uint16 family, Uint16 role, Uint16 parameter, Uint8 phase)
{
    return family == 13 && role == 2 && phase == 4
               ? 1
               : SotnWeaponSecondary_ChildRows(family, role, parameter);
}
bool SotnWeaponSecondary_ComboSpell(Uint16 item)
{
    return item == 202 || SotnWeaponSecondary_ShieldSpellFamily(item) != 0;
}
Uint8 SotnWeaponSecondary_Rows(Uint16 family)
{
    Layout layout = {0};
    return SotnWeaponSecondary_Supported(family) && Program(family, &layout) ? (Uint8)layout.rows
                                                                             : 0;
}

// Shared SetWeaponProperties with the raw parent's equip id; hit type4 keeps attack255.
static bool Properties(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                       SotnWeaponSecondaryEvents *e)
{
    if (!in.properties || !in.properties(in.context, rng, &s->properties))
        return SDL_SetError("Weapon secondary requires actual equipment property ownership");
    s->hitbox_state = s->properties.hit_type;
    if (s->hitbox_state == 4)
        s->properties.attack = 255;
    e->properties = true;
    return true;
}
static Uint16 Rand(Uint32 *rng)
{
    return (Uint16)(SotnCombat_Random(rng) & 0x7fffU);
}

// w_038/w_039: the lunge sparks follow the player's screen position while step_s stays 0x5A.
static bool Lunge(SotnWeaponSecondary *s, SotnHandWeaponProgram *program,
                  SotnWeaponSecondaryInput in, Uint32 *rng, SotnWeaponSecondaryEvents *e)
{
    const Uint16 rows = SotnWeaponSecondary_Rows(in.family);
    if (!in.attack_mode || in.substep != 0x5a)
    {
        s->removed = true;
        return true;
    }
    if (!s->phase)
    {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->allocated = true;
        for (Uint16 i = 0; i < rows; ++i)
        {
            SotnWeaponSecondaryRow *r = &s->rows[i];
            *r = (SotnWeaponSecondaryRow){.hidden = true};
            if (in.family == 38)
            {
                r->red[1] = (Uint8)((Rand(rng) & 0xfU) + 63U);
                r->green[1] = 255;
                r->blue[1] = 63;
                r->red[0] = 15;
                r->green[0] = 63;
                r->blue[0] = 15;
                r->timer = (Uint8)(i + 1U);
            }
            else
            {
                r->red[0] = r->red[1] = 0xbf;
                r->green[0] = r->green[1] = 0x3f;
                r->blue[0] = r->blue[1] = 0xff;
                r->timer = (Uint8)(i * 2U + 1U);
            }
        }
        s->row_priority = (Uint16)(in.priority + 4U);
        s->draw_mode = 0x331;
        s->flags = 0x860000U;
        if (!Properties(s, in, rng, e))
            return false;
        program->secondary_counter = 40;
        s->hitbox_width = 24;
        s->hitbox_height = 10;
        s->hitbox_x = 38;
        s->hitbox_y = -12;
        s->phase = 1;
    }
    else
    {
        Uint16 sound = 0;
        if ((program->secondary_counter & 7) == 7)
            e->sounds[sound++] = 0x640;
        program->secondary_counter = (Sint32)((Uint32)program->secondary_counter - 1U);
        if (!program->secondary_counter)
        {
            e->sounds[sound] = 0x6f0;
            s->removed = true;
            return true;
        }
    }
    const Sint16 x = Word(Word(Pixel(in.x) - in.camera_x) + (in.facing_left ? -34 : 34)),
                 y = Word(Word(Pixel(in.y) - in.camera_y) - 14 - ((in.draw_flags & 2U) ? 3 : 0));
    for (Uint16 i = 0; i < rows; ++i)
    {
        SotnWeaponSecondaryRow *r = &s->rows[i];
        if (r->phase == 0)
        {
            if (--r->timer)
                continue;
            const Sint16 px = Word(x + (Sint32)(Rand(rng) % 24U) - 12),
                         py = Word(y + (Sint32)(Rand(rng) % 18U) - 9);
            if (in.family == 38)
            {
                r->x[0] = Word(px + (in.facing_left ? 16 : -12));
                r->x[1] = Word(px + (in.facing_left ? -12 : 16));
                r->y[0] = r->y[1] = py;
                r->timer = 3;
            }
            else
            {
                // Row0 starts the chain; later rows continue from the previous retained end.
                const bool outward = i == 0 || (i & 1U) == 0;
                if (i == 0)
                {
                    r->x[0] = Word(px + (in.facing_left ? 16 : -16));
                    r->y[0] = py;
                }
                else
                {
                    r->x[0] = program->secondary_x[i - 1U];
                    r->y[0] = program->secondary_y[i - 1U];
                }
                r->x[1] = Word(px + ((in.facing_left != 0) == outward ? -16 : 16));
                r->y[1] = py;
                program->secondary_x[i] = r->x[1];
                program->secondary_y[i] = r->y[1];
                r->timer = 9;
            }
            r->hidden = false;
            ++r->phase;
        }
        else if (r->phase == 1 && !--r->timer)
        {
            if (in.family == 38)
            {
                r->timer = 2;
                r->phase = 0;
            }
            else
                ++r->phase;
            r->hidden = true;
        }
    }
    return true;
}

static bool View(SotnDataView o, Uint32 at, Uint32 size, const Uint8 **out)
{
    if (!o.bytes || at > o.size || o.size - at < size)
        return SDL_SetError("Weapon secondary private table is outside its program");
    *out = o.bytes + at;
    return true;
}
// w_053 (shared with w_056): one textured GT4 slash per child. Roots0/10/20/30/40 request a
// chained child every callback; nine callbacks complete it.
static bool Slash(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                  SotnWeaponSecondaryEvents *e)
{
    const Uint16 params = (s->parameter >> 8) & 0x7fU;
    // w_056 runs the same callback over its own tables, eight bytes later.
    const bool muramasa = in.family == 56;
    const Uint32 table = muramasa ? 0xfccU : 0xfc4U, hand_word = muramasa ? 0x105cU : 0x1268U;
    const Uint8 *source = NULL, *cluts = NULL;
    if (params >= 50 || !View(in.overlay, table + params * 14U, 14, &source) ||
        !View(in.overlay, muramasa ? 0xfa4U : 0xf9cU, 40, &cluts))
        return SDL_SetError("Weapon53/56 slash parameter exceeds its private table");
    // Root0x28's chain reaches rows48/49 past weapon53's48-row table; row48 overlaps its live
    // g_HandId word at0x1268, which reads as the resident hand. Weapon56's shorter table reaches
    // its own g_HandId at0x105C (row10) and then program words, read as the original does.
    Uint8 config[14];
    SDL_memcpy(config, source, sizeof(config));
    for (Uint32 b = 0; b < 4; ++b)
    {
        const Uint32 at = hand_word + b, base = table + params * 14U;
        if (at >= base && at < base + 14U)
            config[at - base] = (Uint8)((Uint32)in.hand >> (8U * b));
    }
    // w_053 func_ptr_8017000C (role3, blueprint68): Yasutsuna's slash stays at its factory
    // origin; only parameter0 rolls properties and chains, and it never follows the player.
    const bool fixed = in.role == 3;
    const bool root =
        fixed ? params == 0 : params == 0 || params == 10 || params == 20 || params == 30;
    if (!s->phase)
    {
        s->facing_left = in.facing_left;
        if (!fixed)
            s->x = (Sint32)((Uint32)s->x + (Uint32)(in.facing_left ? -20 : 20) * 65536U);
        s->y = (Sint32)((Uint32)s->y + (Uint32)(in.crouch ? 11 : -5) * 65536U);
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->allocated = true;
        const Uint8 v = in.hand ? 0x80 : 0;
        SotnPlayerEffectQuad q = {
            .page = 0x19, .priority = (Uint16)(in.priority + 2U), .draw_mode = 0x137};
        q.u[0] = q.u[2] = Read16(config) ? 0xc0 : 0x80;
        q.u[1] = q.u[3] = Read16(config) ? 0x80 : 0xc0;
        q.v[0] = q.v[1] = Read16(config + 2) ? (Uint8)(v | 0x40U) : v;
        q.v[2] = q.v[3] = Read16(config + 2) ? v : (Uint8)(v | 0x40U);
        for (Uint16 i = 0; i < 4; ++i)
        {
            q.red[i] = config[10];
            q.green[i] = config[11];
            q.blue[i] = config[12];
        }
        s->quad = q;
        s->flags = 0x08820000U;
        if (root || (!fixed && params > 0x27 && params <= 0x31))
        {
            if (!Properties(s, in, rng, e))
                return false;
            s->hitbox_width = 30;
            s->hitbox_height = 32;
        }
        if (!(params & 3U))
            e->sounds[0] = 0x60c;
        s->rotate = (Sint16)Read16(config + 8);
        s->lifetime = (Sint16)Read16(config + 6);
        s->age = 0;
        s->phase = 1;
    }
    if (root || (!fixed && params == 0x28))
        e->chain = 0x38U | ((in.hand + 1U) << 12) | ((Uint32)(s->age + params + 1U) << 16);
    s->quad.palette = (Uint16)((in.hand ? 0x18 : 0) +
                               Read16(cluts + (Read16(config + 4) ? 20U : 0U) + s->age * 2U));
    if (++s->age >= 9)
    {
        s->removed = true;
        return true;
    }
    if (fixed || (params >= 0x1e && params < 0x32))
        s->hitbox_x = 0x12;
    else
    {
        // posX.i.hi only: the child keeps its own fraction.
        s->x = (Sint32)((Uint32)(Pixel(in.x) + (s->facing_left ? -0x26 : 0x26)) * 65536U |
                        ((Uint32)s->x & 0xffffU));
        s->y = (Sint32)((Uint32)(Pixel(in.y) + (in.crouch ? 0xb : -5)) * 65536U |
                        ((Uint32)s->y & 0xffffU));
    }
    // Corners at rotate+0x600, -0x400, -0x800, +0x400, scaled by the private radius.
    const Sint16 steps[4] = {0x600, -0x400, -0x800, 0x400};
    Sint16 angle = s->rotate;
    for (Uint16 i = 0; i < 4; ++i)
    {
        angle = Word(angle + steps[i]);
        SotnMathTrig t = {0};
        if (!SotnMath_Trig(in.executable, angle, &t))
            return false;
        const Sint32 dx = Asr(Asr(t.cosine, 4) * s->lifetime, 8),
                     dy = Asr(Asr(t.sine, 4) * s->lifetime, 8);
        s->quad.x[i] = Word(s->facing_left ? -dx : dx);
        s->quad.y[i] = Word(-dy);
    }
    return true;
}

// w_048 func_ptr_8017B810: a per-animation anchor/extent row (whole or halved extent).
static bool ThunderAnchor(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, bool half,
                          Sint16 *px, Sint16 *py)
{
    const Uint8 *row = NULL;
    if (in.animation < 0x41 || in.animation >= 0x48 ||
        !View(in.overlay, 0x1000U + (in.animation - 0x41U) * 8U, 8, &row))
        return SDL_SetError("Weapon48 anchor requires its private animation row");
    const Sint16 ax = (Sint16)Read16(row), ay = (Sint16)Read16(row + 2),
                 dx = (Sint16)Read16(row + 4), dy = (Sint16)Read16(row + 6);
    const Sint16 x = Word(Pixel(in.x) - in.camera_x), y = Word(Pixel(in.y) - in.camera_y);
    const Sint16 hx = half ? (Sint16)(dx / 2) : dx;
    s->x = (Sint32)((Uint32)(Word(x + (in.facing_left ? -ax : ax)) + in.camera_x) * 65536U |
                    ((Uint32)s->x & 0xffffU));
    s->y = (Sint32)((Uint32)(Word(y + ay) + in.camera_y) * 65536U | ((Uint32)s->y & 0xffffU));
    *px = in.facing_left ? (Sint16)-hx : hx;
    *py = half ? (Sint16)(dy / 2) : dy;
    return true;
}
// w_048 func_ptr_80170004: ten LINE_G2 lightning segments from the anchor toward its target.
static bool Lightning(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                      SotnWeaponSecondaryEvents *e)
{
    if (in.animation < 65 || in.animation >= 72 || !in.attack_mode)
    {
        s->removed = true;
        return true;
    }
    const Uint16 params = (s->parameter >> 8) & 0x7fU;
    Sint16 x = 0, y = 0, tx = 0, ty = 0;
    if (params < 16)
    {
        x = Word(Word(Pixel(in.x) - in.camera_x) + (in.facing_left ? -12 : 12));
        y = Word(Word(Pixel(in.y) - in.camera_y) - 26);
        tx = x;
        ty = Word(y - 48);
    }
    else
    {
        Sint16 dx = 0, dy = 0;
        if (!ThunderAnchor(s, in, params < 32, &dx, &dy))
            return false;
        x = Word(Pixel(s->x) - in.camera_x);
        y = Word(Pixel(s->y) - in.camera_y);
        tx = Word(x + dx);
        ty = Word(y + dy);
    }
    s->facing_left = in.facing_left;
    if (in.draw_flags & 2U)
    {
        const Sint16 lift = (in.animation - 65 == 2 || in.animation - 65 == 3) ? 1 : 3;
        const Sint16 extra = params < 16 ? 3 : 0;
        y = Word(y - extra - lift);
        ty = Word(ty - extra - lift);
    }
    s->x = (Sint32)((Uint32)(x + in.camera_x) * 65536U | ((Uint32)s->x & 0xffffU));
    s->y = (Sint32)((Uint32)(y + in.camera_y) * 65536U | ((Uint32)s->y & 0xffffU));
    if (!s->phase)
    {
        const Uint8 *color = NULL;
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        if (!View(in.overlay, 0xfe0U + (params & 0xfU) * 8U, 6, &color))
            return false;
        s->allocated = true;
        s->flags = 0x860000U;
        const Uint32 red = Read16(color), blue = Read16(color + 2), green = Read16(color + 4);
        for (Uint16 i = 0; i < 10; ++i)
        {
            SotnWeaponSecondaryRow *r = &s->rows[i];
            *r = (SotnWeaponSecondaryRow){0};
            r->red[0] = (Uint8)(red * (10U - i) / 10U);
            r->blue[0] = (Uint8)(blue * (10U - i) / 10U);
            r->green[0] = (Uint8)(green * (10U - i) / 10U);
            r->red[1] = (Uint8)(red * (9U - i) / 10U);
            r->blue[1] = (Uint8)(blue * (9U - i) / 10U);
            r->green[1] = (Uint8)(green * (9U - i) / 10U);
            r->angle = (Sint16)SotnCombat_Random(rng);
        }
        s->row_priority = (Uint16)(in.priority + 4U);
        s->draw_mode = 0x335;
        if (!Properties(s, in, rng, e))
            return false;
        if (params < 16)
        {
            s->hitbox_x = 0;
            s->hitbox_y = -20;
            s->hitbox_width = 4;
            s->hitbox_height = 20;
            s->remaining = 40;
        }
        else
        {
            if (params >= 32)
            {
                if (!(params & 0xfU))
                {
                    s->hitbox_x = 32;
                    s->hitbox_y = 0;
                    s->hitbox_width = 32;
                    s->hitbox_height = 6;
                }
                else
                    s->hitbox_state = 0;
            }
            s->remaining = 6;
        }
        s->phase = 1;
    }
    else if (!(s->remaining = Word(s->remaining - 1)))
    {
        s->removed = true;
        return true;
    }
    const Sint32 dx = tx - x, dy = ty - y;
    Sint16 cx = x, cy = y;
    for (Uint16 i = 0; i < 10; ++i)
    {
        SotnWeaponSecondaryRow *r = &s->rows[i];
        r->x[0] = cx;
        r->y[0] = cy;
        const Sint16 angle = r->angle;
        r->angle = Word(r->angle + 768);
        SotnMathTrig t = {0};
        if (!SotnMath_Trig(in.executable, angle, &t))
            return false;
        r->x[1] = Word(Word(x + dx * (i + 1) / 10) + Asr(Asr(t.cosine, 4) * 3, 8));
        r->y[1] = Word(Word(y + dy * (i + 1) / 10) - Asr(Asr(t.sine, 4) * 3, 8));
        cx = r->x[1];
        cy = r->y[1];
    }
    return true;
}
// w_048 func_ptr_80170008: a sixteen-wedge GT4 ring that grows and fades its blue center.
static bool Ring(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, SotnWeaponSecondaryEvents *e)
{
    if (!(in.animation > 0x40 && in.animation < 0x48) || !in.attack_mode)
    {
        s->removed = true;
        return true;
    }
    const Sint16 x = Word(Word(Pixel(in.x) - in.camera_x) + (in.facing_left ? -12 : 12)),
                 y = Word(Word(Pixel(in.y) - in.camera_y) - 26 - ((in.draw_flags & 2U) ? 5 : 0));
    s->x = (Sint32)((Uint32)(x + in.camera_x) * 65536U | ((Uint32)s->x & 0xffffU));
    s->y = (Sint32)((Uint32)(y + in.camera_y) * 65536U | ((Uint32)s->y & 0xffffU));
    if (!s->phase)
    {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->allocated = true;
        s->flags = 0x860000U;
        for (Uint16 i = 0; i < 16; ++i)
        {
            SotnPlayerEffectQuad q = {.page = 0x1a,
                                      .palette = 0x15f,
                                      .priority = (Uint16)(in.priority + 2U),
                                      .draw_mode = 0x335};
            for (Uint16 k = 0; k < 2; ++k)
            {
                SotnMathTrig t = {0};
                if (!SotnMath_Trig(in.executable, (Sint32)(i + k) * 256, &t))
                    return false;
                q.u[k] = (Uint8)(Asr(t.cosine * 2, 8) + 32);
                q.v[k] = (Uint8)(-32 - Asr(t.sine * 2, 8));
                if (q.v[k] < 4)
                    q.v[k] = 0xff;
            }
            q.u[2] = q.u[3] = 0x20;
            q.v[2] = q.v[3] = 0xe0;
            q.blue[2] = q.blue[3] = 0x80;
            s->quads[i] = q;
        }
        s->remaining = 4;
        s->age = 0x28;
        e->sounds[0] = 0x665;
        s->phase = 1;
    }
    else
    {
        if (s->remaining < 24)
            s->remaining = (Sint16)(s->remaining + 4);
        if (!--s->age)
        {
            s->removed = true;
            return true;
        }
    }
    const Uint8 blue = s->age < 16 ? (Uint8)(s->age * 8U) : 0x80;
    for (Uint16 i = 0; i < 16; ++i)
    {
        SotnPlayerEffectQuad *q = &s->quads[i];
        q->x[2] = q->x[3] = x;
        q->y[2] = q->y[3] = y;
        q->blue[2] = q->blue[3] = blue;
        for (Uint16 k = 0; k < 2; ++k)
        {
            SotnMathTrig t = {0};
            if (!SotnMath_Trig(in.executable, (Sint32)(i + k) * 256, &t))
                return false;
            q->x[k] = Word(x + Asr(Asr(t.cosine, 4) * s->remaining, 8));
            q->y[k] = Word(y - Asr(Asr(t.sine, 4) * s->remaining, 8));
        }
    }
    return true;
}

// w_001 func_ptr_80170004: the slot16 punch rush (Iron Fist/Tulkas kind24) flashes sixteen
// textured GT4 fists while the player's attack window lasts.
static bool Punches(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                    SotnWeaponSecondaryEvents *e)
{
    const Uint8 *d = NULL, *uv = NULL;
    const Uint16 index = (Uint16)((s->parameter & 0x7fffU) >> 8);
    if (index > 4 || !View(in.overlay, 0xf44U + index * 16U, 16, &d) ||
        !View(in.overlay, 0xfb8U, 8, &uv))
        return SDL_SetError("Weapon1 punch rush requires its private descriptor and UVs");
    s->x = in.x;
    s->y = in.y;
    s->facing_left = in.facing_left;
    if (in.animation < d[12] || in.animation >= d[12] + 7U || !in.attack_mode)
    {
        s->removed = true;
        return true;
    }
    if (!s->phase)
    {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->allocated = true;
        const Uint8 v = in.hand ? 0x80 : 0;
        for (Uint16 i = 0; i < 16; ++i)
        {
            SotnPlayerEffectQuad q = {.page = 0x19,
                                      .palette = 0x100,
                                      .priority = (Uint16)(in.priority + 4U),
                                      .draw_mode = 0x379};
            for (Uint16 k = 0; k < 4; ++k)
            {
                q.u[k] = uv[k * 2U];
                q.v[k] = (Uint8)(uv[k * 2U + 1U] + v);
            }
            s->quads[i] = q;
            s->quad_timer[i] = (Uint8)(i + 1U);
            s->quad_phase[i] = 0;
        }
        s->flags = 0x860000U;
        if (!Properties(s, in, rng, e))
            return false;
        s->remaining = 128;
        s->hitbox_width = 18;
        s->hitbox_height = 10;
        s->hitbox_x = 28;
        s->hitbox_y = -12;
        s->phase = 1;
    }
    else
    {
        if ((s->remaining & 7) == 7)
            e->sounds[0] = 0x60c;
        if (!(s->remaining = Word(s->remaining - 1)))
        {
            e->sounds[e->sounds[0] ? 1 : 0] = 0x6f0;
            s->removed = true;
            return true;
        }
    }
    const Sint16 px = Word(Word(Pixel(in.x) - in.camera_x) + (in.facing_left ? -26 : 26)),
                 py = Word(Word(Pixel(in.y) - in.camera_y) - 6 - ((in.draw_flags & 2U) ? 3 : 0));
    for (Uint16 i = 0; i < 16; ++i)
    {
        SotnPlayerEffectQuad *q = &s->quads[i];
        Uint8 *timer = &s->quad_timer[i], *phase = &s->quad_phase[i];
        if (*phase == 0)
        {
            if (--*timer)
                continue;
            const Sint16 x = Word(px + (Sint32)(Rand(rng) % 24U) - 12);
            const Sint16 y = Word(py + (Sint32)(Rand(rng) % 18U) - 9);
            q->x[0] = q->x[2] = Word(x + (in.facing_left ? 8 : -8));
            q->x[1] = q->x[3] = Word(x + (in.facing_left ? -8 : 8));
            q->y[0] = q->y[1] = Word(y - 8);
            q->y[2] = q->y[3] = Word(y + 8);
            q->draw_mode = (Uint16)(q->draw_mode & ~8U);
            *timer = 2;
            *phase = 1;
        }
        else if (*phase <= 2 && !--*timer)
        {
            for (Uint16 k = 0; k < 4; ++k)
                q->u[k] = (Uint8)(q->u[k] + (*phase == 1 ? 48U : 208U));
            *timer = 2;
            if (*phase == 1)
                *phase = 2;
            else
            {
                *phase = 0;
                q->draw_mode |= 8U;
            }
        }
    }
    return true;
}
// w_001 func_ptr_80170008: Tulkas's growing fireball, retired the callback after any hit.
static bool Fireball(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                     SotnWeaponSecondaryEvents *e)
{
    if (!s->phase)
    {
        s->flags = 0x08100000U;
        s->bank = 9;
        s->pose = 0;
        s->remaining = 0;
        s->frame = 0;
        s->priority = (Uint16)(in.priority - 2U);
        s->facing_left = !in.facing_left;
        s->velocity_x = s->facing_left ? 0x48000 : -0x48000;
        s->palette = 0x81b0;
        s->blend = 0x30;
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x * 6U);
        s->y = (Sint32)((Uint32)s->y - (Uint32)(13 + ((in.draw_flags & 2U) ? 3 : 0)) * 65536U);
        s->draw_flags = 3;
        s->scale_x = s->scale_y = 0x60;
        s->allocated = true;
        if (!Properties(s, in, rng, e))
            return false;
        s->hitbox_width = 10;
        s->hitbox_height = 8;
        e->sounds[0] = 0x62c;
        s->phase = 1;
        return true;
    }
    if (s->hit_flags)
    {
        s->removed = true;
        return true;
    }
    s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
    s->scale_x = (Sint16)(s->scale_x + 0x10);
    s->scale_y = (Sint16)(s->scale_y + 0x10);
    if (s->scale_y >= 0x101)
        s->scale_y = 0x100;
    if (s->scale_x >= 0x181)
        s->scale_x = 0x180;
    return true;
}

static Sint32 Decelerate(Sint32 v, Sint32 amount);
// w_051 data words read through the live program: g_HandId (D_8017AED0) is the resident hand.
static bool FanWord(SotnDataView o, Uint16 hand, Uint32 at, Uint16 *out)
{
    const Uint8 *p = NULL;
    if (at == 0xed0 || at == 0xed2)
        *out = at == 0xed0 ? hand : 0;
    else if (View(o, at, 2, &p))
        *out = Read16(p);
    else
        return false;
    return true;
}
// w_051 func_ptr_80170008: the special's fan of GT4 blades. Param0 (the root) attacks and
// requests FACTORY(0x3E,n) children every callback; each blade slides out and retires at nine.
static bool Fan(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                SotnWeaponSecondaryEvents *e)
{
    const Uint16 sub = (Uint16)((s->parameter & 0x7f00U) >> 8),
                 color = (Uint16)((in.slot16_parameter & 0x7f00U) >> 8);
    const Uint8 *a = NULL;
    if (!View(in.overlay, 0xe74U + (sub % 8U) * 6U, 6, &a))
        return SDL_SetError("Weapon51 fan requires its private animation properties");
    if (!s->phase)
    {
        s->x = in.x;
        s->y = in.y;
        s->facing_left = in.facing_left;
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->allocated = true;
        const Uint8 v = in.hand ? 0x80 : 0;
        const bool flipped = Read16(a) != 0;
        SotnPlayerEffectQuad q = {
            .u = {128, 192, 128, 192},
            .v = {(Uint8)(flipped ? v | 0x40 : v), (Uint8)(flipped ? v | 0x40 : v),
                  (Uint8)(flipped ? v : v | 0x40), (Uint8)(flipped ? v : v | 0x40)},
            .palette = 0x15f,
            .page = 0x19,
            .priority = (Uint16)(in.priority + 4U),
            .draw_mode = (Uint16)(color == 3 ? 0x117 : 0x177)};
        Uint8 *channel[3][2] = {
            {&q.red[1], &q.red[3]}, {&q.green[1], &q.green[3]}, {&q.blue[1], &q.blue[3]}};
        for (Uint16 k = 0; k < 3; ++k)
        {
            Uint16 tint = 0;
            if (!FanWord(in.overlay, in.hand, 0xea4U + (color * 4U + k) * 2U, &tint))
                return SDL_SetError("Weapon51 fan colour index left its program");
            const Uint8 value = (Uint8)(tint + (Rand(rng) & 15U));
            *channel[k][0] = *channel[k][1] = value;
        }
        s->quad = q;
        s->flags = 0x08820000U;
        if (!sub)
        {
            if (!Properties(s, in, rng, e))
                return false;
            s->hitbox_width = 30;
            s->hitbox_height = 16;
            s->hitbox_x = 8;
        }
        if (!(sub & 3U))
            e->sounds[0] = 0x60c;
        const Sint32 dx = (color == 0 ? 20 : 30) * (s->facing_left ? -1 : 1);
        s->x = (Sint32)((Uint32)s->x + (Uint32)dx * 65536U);
        s->y = (Sint32)((Uint32)s->y - 18U * 65536U);
        s->rotate = (Sint16)Read16(a + 4);
        s->lifetime = (Sint16)Read16(a + 2);
        s->age = 0;
        const Sint32 speed = (Sint32)Rand(rng) * 2 + 0x60000;
        s->velocity_x = s->facing_left ? -speed : speed;
        s->phase = 1;
    }
    s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
    s->velocity_x = Decelerate(s->velocity_x, 0x2800);
    if (!sub)
        e->chain = 0x3eU | ((Uint32)(in.hand + 1U) << 12) | ((Uint32)(s->age + 1U) << 16);
    if (++s->age > 8)
    {
        s->removed = true;
        return true;
    }
    const Sint16 steps[4] = {0x600, -0x400, -0x800, 0x400};
    Sint16 angle = s->rotate;
    for (Uint16 k = 0; k < 4; ++k)
    {
        angle = Word(angle + steps[k]);
        SotnMathTrig t = {0};
        if (!SotnMath_Trig(in.executable, angle, &t))
            return false;
        const Sint32 dx = ((t.cosine >> 4) * s->lifetime) >> 8;
        s->quad.x[k] = Word(s->facing_left ? -dx : dx);
        s->quad.y[k] = Word(-(((t.sine >> 4) * s->lifetime) >> 9));
    }
    return true;
}

// Weapon49 children keep original screen positions; native positions are world (screen plus
// camera) with the original fraction retained.
static void SetScreen(Sint32 *position, Sint16 screen, Sint32 camera)
{
    *position = (Sint32)(((Uint32)(screen + camera) << 16) | ((Uint32)*position & 0xffffU));
}
static bool BrandOrbit(Sint16 angle, Sint16 magnitude, SotnDataView exe, Sint16 *dx, Sint16 *dy)
{
    SotnMathTrig t = {0};
    if (!SotnMath_Trig(exe, angle, &t))
        return false;
    *dx = Word(((t.cosine >> 4) * magnitude) >> 8);
    *dy = Word(((t.sine >> 4) * magnitude) >> 8);
    return true;
}
static bool BrandWindow(SotnWeaponSecondaryInput in)
{
    return in.animation >= 0x41 && in.animation <= 0x47 && in.attack_mode;
}
// w_049 func_ptr_80170004: eight sparks orbit the blade, copying its attack fields, then spiral
// in; spark0 requests the WFACTORY(0x40,0) ring at lifetime0x18.
static bool Sparks(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in,
                   SotnWeaponSecondaryEvents *e)
{
    if (!BrandWindow(in))
    {
        s->removed = true;
        return true;
    }
    const Uint8 params = (Uint8)s->parameter;
    const Sint16 bx = Word(Pixel(in.x) - in.camera_x + (in.facing_left ? -12 : 12)),
                 by = Word(Pixel(in.y) - in.camera_y - ((in.draw_flags & 2U) ? 0x1f : 0x1a));
    bool rotate = false;
    if (!s->phase)
    {
        s->role = 1;
        s->bank = 9;
        s->timeline = 0x1074;
        s->priority = (Uint16)(in.priority + 2U);
        s->facing_left = true;
        s->flags = 0x160000U;
        s->draw_flags = 4;
        s->angle = (Sint16)(params * 512);
        s->rotate = Word(s->angle - 0x400);
        s->properties = in.parent_properties;
        s->hitbox_state = in.parent_hitbox_state;
        e->group = true;
        s->hitbox_width = s->hitbox_height = 6;
        s->magnitude = 0x40;
        s->lifetime = 0x28;
        s->allocated = true;
        s->phase = 1;
    }
    else
    {
        rotate = true;
        if (s->magnitude >= 8)
            s->magnitude = (Sint16)(s->magnitude - 3);
        s->lifetime = (Sint16)(s->lifetime - 1);
        if (s->lifetime == 0x18 && params == 0)
            e->chain = 0x40U | ((Uint32)(in.hand + 1U) << 12);
        s->angle = Word(s->angle - 0x20);
        if (s->lifetime < 0x11)
        {
            s->removed = true;
            return true;
        }
    }
    Sint16 dx = 0, dy = 0;
    if (!BrandOrbit(s->angle, s->magnitude, in.executable, &dx, &dy))
        return false;
    const Sint16 x = Word(bx + dx), y = Word(by - dy);
    if (rotate)
    {
        Sint16 heading = 0;
        if (!SotnMath_Atan2(in.executable, -(Sint32)Word(y - s->prev_y), Word(x - s->prev_x),
                            &heading))
            return false;
        s->rotate = heading;
    }
    s->prev_x = x;
    s->prev_y = y;
    SetScreen(&s->x, x, in.camera_x);
    SetScreen(&s->y, y, in.camera_y);
    return true;
}
// w_049 func_ptr_80170008: a ring of sixteen white-fill wedges (LINE_G2 rows retyped GT4)
// around the blade, growing six per update to forty and fading out over its last eight.
static bool BrandRing(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in)
{
    if (!BrandWindow(in))
    {
        s->removed = true;
        return true;
    }
    const Sint16 x = Word(Pixel(in.x) - in.camera_x + (in.facing_left ? -12 : 12)),
                 y = Word(Pixel(in.y) - in.camera_y - 26 - ((in.draw_flags & 2U) ? 5 : 0));
    SetScreen(&s->x, x, in.camera_x);
    SetScreen(&s->y, y, in.camera_y);
    if (!s->phase)
    {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 2;
        s->allocated = true;
        for (Uint16 i = 0; i < 16; ++i)
        {
            SotnPlayerEffectQuad q = {.u = {0, 0, 0x20, 0x20},
                                      .v = {0, 0, 0xe0, 0xe0},
                                      .red = {0, 0, 0xff, 0xff},
                                      .green = {0, 0, 0x7f, 0x7f},
                                      .palette = 0x15f,
                                      .page = 0x1a,
                                      .priority = (Uint16)(in.priority + 2U),
                                      .draw_mode = 0x335};
            for (Uint16 k = 0; k < 2; ++k)
            {
                SotnMathTrig t = {0};
                if (!SotnMath_Trig(in.executable, (Sint16)((i + k) * 256U), &t))
                    return false;
                q.u[k] = (Uint8)(((t.cosine * 2) >> 8) + 32);
                q.v[k] = (Uint8)(-32 - ((t.sine * 2) >> 8));
                if (q.v[k] < 4)
                    q.v[k] = 0xff;
            }
            s->quads[i] = q;
        }
        s->flags = 0x860000U;
        s->lifetime = 4;
        s->age = (s->parameter & 0x7f00U) ? 40 : 0x19;
        s->phase = 1;
    }
    else
    {
        if (s->lifetime < 40)
            s->lifetime = (Sint16)(s->lifetime + 6);
        if (!--s->age)
        {
            s->removed = true;
            return true;
        }
    }
    const Uint8 red = s->age < 8 ? (Uint8)(s->age * 16U) : 0x80, green = (Uint8)(red >> 1);
    for (Uint16 i = 0; i < 16; ++i)
    {
        SotnPlayerEffectQuad *q = &s->quads[i];
        q->x[2] = q->x[3] = x;
        q->y[2] = q->y[3] = y;
        q->red[2] = q->red[3] = red;
        q->green[2] = q->green[3] = green;
        for (Uint16 k = 0; k < 2; ++k)
        {
            Sint16 dx = 0, dy = 0;
            if (!BrandOrbit((Sint16)((i + k) * 256U), s->lifetime, in.executable, &dx, &dy))
                return false;
            q->x[k] = Word(x + dx);
            q->y[k] = Word(y - dy);
        }
    }
    return true;
}
// w_049 func_ptr_8017000C: fire puffs placed along the blade's private anchor, rising with a
// random palette, scale, priority and blend; retired after their animation ends.
static bool Puffs(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                  SotnWeaponSecondaryEvents *e)
{
    if (s->phase)
    {
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->velocity_y);
        if (s->remaining < 0)
            s->removed = true;
        return true;
    }
    if (!BrandWindow(in))
    {
        s->removed = true;
        return true;
    }
    const Uint16 upper = (Uint16)((s->parameter >> 8) & 0x7fU),
                 lower = (Uint16)(s->parameter & 0xffU);
    const Uint8 *a = NULL;
    if (!View(in.overlay, 0x103cU + (Uint32)(in.animation - 0x41) * 8U, 8, &a))
        return SDL_SetError("Weapon49 puff requires its private blade anchors");
    const Sint32 ax = (Sint16)Read16(a), ay = (Sint16)Read16(a + 2), ex = (Sint16)Read16(a + 4),
                 ey = (Sint16)Read16(a + 6);
    const bool wide = upper != 1;
    s->facing_left = in.facing_left;
    Sint16 x = Word(Pixel(in.x) - in.camera_x + (in.facing_left ? -ax : ax)),
           y = Word(Pixel(in.y) - in.camera_y + ay);
    const Sint16 px = Word(wide ? ex * 3 / 4 : ex * 2 / 3),
                 py = Word(wide ? ey * 3 / 4 : ey * 2 / 3);
    const Sint16 tx = in.facing_left ? Word(-px) : px;
    x = Word(x + tx * (Sint32)lower / 8);
    y = Word(y + py * (Sint32)lower / 8);
    y = Word(y + (Sint32)(Rand(rng) % 16U));
    x = Word(x + (Sint32)(Rand(rng) % 4U));
    s->role = 3;
    s->bank = (Uint16)(0x8011U + in.hand * 2U);
    s->texture = (Uint16)(0x65U + in.hand * 2U);
    s->palette = (Uint16)((Rand(rng) & 7U) + (in.hand ? 0x12fU : 0x117U));
    s->timeline = 0x10d4;
    s->draw_flags = 3;
    s->scale_x = s->scale_y = (Sint16)((Rand(rng) & 0x3fU) | 0x80U);
    s->priority = (Uint16)(in.priority + (Rand(rng) & 1U) * 4U);
    s->flags = 0x08100000U;
    s->blend = 0x30;
    if (Rand(rng) % 3U)
        s->blend = 0x70;
    s->velocity_y = -0x8000;
    if (in.draw_flags & 2U)
        y = Word(y - (in.animation >= 0x43 && in.animation < 0x45 ? 1 : 3));
    SetScreen(&s->x, x, in.camera_x);
    SetScreen(&s->y, y, in.camera_y);
    s->allocated = true;
    if (!Properties(s, in, rng, e))
        return false;
    s->hitbox_width = s->hitbox_height = 6;
    s->phase = 1;
    return true;
}
// w_049 func_ptr_80170010: invisible swirls spiral out from the blade, requesting the
// WFACTORY(0x65,0) pillar whenever their counter reaches a multiple of seven.
static bool Swirl(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                  SotnWeaponSecondaryEvents *e)
{
    if (s->phase <= 1 && !BrandWindow(in))
    {
        s->removed = true;
        return true;
    }
    const Sint8 params = (Sint8)(Uint8)s->parameter;
    const Sint16 bx = Word(Pixel(in.x) - in.camera_x + (in.facing_left ? -12 : 12)),
                 by = Word(Pixel(in.y) - in.camera_y - ((in.draw_flags & 2U) ? 0x1f : 0x1a));
    if (!s->phase)
    {
        s->role = 4;
        s->facing_left = true;
        s->flags = 0x60000U;
        s->angle = (Sint16)(params * 512);
        s->allocated = true;
        if (!Properties(s, in, rng, e))
            return false;
        s->hitbox_width = s->hitbox_height = 0;
        s->magnitude = 0xc;
        s->lifetime = (Sint16)(Rand(rng) % 5U);
        s->phase = 1;
    }
    else if (s->phase == 1)
    {
        s->magnitude = (Sint16)(s->magnitude + 4);
        s->angle = Word(s->angle - 0x20);
        if (s->magnitude >= 0x40)
            s->phase = 2;
    }
    else
    {
        s->magnitude = (Sint16)(s->magnitude + 4);
        s->flags = 0x08000000U;
        if (s->magnitude >= 0x70)
        {
            s->removed = true;
            return true;
        }
    }
    Sint16 dx = 0, dy = 0;
    s->lifetime = (Sint16)(s->lifetime + 1);
    if (!BrandOrbit(s->angle, s->magnitude, in.executable, &dx, &dy))
        return false;
    SetScreen(&s->x, Word(bx + dx), in.camera_x);
    SetScreen(&s->y, Word(by - dy), in.camera_y);
    if (s->lifetime % 7 == 0)
        e->chain = 0x65U | ((Uint32)(in.hand + 1U) << 12);
    return true;
}
// w_049 func_ptr_80170014: a rising fire pillar with SFX_FM_EXPLODE_B, retired at its END.
static bool Pillar(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                   SotnWeaponSecondaryEvents *e)
{
    if (s->phase)
    {
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->velocity_y);
        if (s->remaining < 0)
            s->removed = true;
        return true;
    }
    s->role = 5;
    s->bank = (Uint16)(0x8011U + in.hand * 2U);
    s->texture = (Uint16)(0x65U + in.hand * 2U);
    s->palette = in.hand ? 0x12f : 0x117;
    s->timeline = 0x110c;
    s->priority = (Uint16)(in.priority + 2U);
    s->flags = 0x08100000U;
    s->draw_flags = 3;
    s->scale_x = s->scale_y = 0x100;
    s->blend = 0x30;
    s->allocated = true;
    if (!Properties(s, in, rng, e))
        return false;
    s->hitbox_width = s->hitbox_height = 12;
    s->velocity_y = -0x6000;
    e->sounds[0] = 0x65b;
    s->phase = 1;
    return true;
}

// w_050 func_8017B784: place one sparkle at its UV row (uv_coords D_8017B000), half-extent six
// for rows0..2 and four after; every second placement advances the row, and row6 ends it.
static bool IcePlace(SotnDataView o, Uint32 at, Uint8 *row, Uint8 *count, Uint8 uv[8], Uint8 *size,
                     bool *ended)
{
    const Uint8 *table = NULL;
    *ended = *row == 6;
    if (*ended)
        return true;
    if (!View(o, at + (Uint32)*row * 8U, 8, &table))
        return SDL_SetError("Weapon50 sparkle requires its private UV rows");
    *size = *row >= 3 ? 4 : 6;
    SDL_memcpy(uv, table, 8);
    ++*count;
    if (!(*count & 1U))
        ++*row;
    return true;
}
// w_050 func_8017B87C/func_8017BA38: the blade anchor for the player's animation, with three
// quarters of its extent or a smaller fraction (two thirds / one half).
static bool IceAnchor(SotnWeaponSecondaryInput in, bool wide, bool half, Sint16 *x, Sint16 *y,
                      Sint16 *px, Sint16 *py)
{
    const Uint8 *a = NULL;
    // Frost streaks outlive the attack window: other animations index the program's words
    // around D_8017B030 exactly as the original does.
    const Sint32 at = 0x1030 + ((Sint32)in.animation - 0x41) * 8;
    if (at < 0 || !View(in.overlay, (Uint32)at, 8, &a))
        return SDL_SetError("Weapon50 child requires its private blade anchor");
    const Sint32 ax = (Sint16)Read16(a), ay = (Sint16)Read16(a + 2), ex = (Sint16)Read16(a + 4),
                 ey = (Sint16)Read16(a + 6);
    *x = Word(Pixel(in.x) - in.camera_x + (in.facing_left ? -ax : ax));
    *y = Word(Pixel(in.y) - in.camera_y + ay);
    const Sint16 fx = Word(wide ? ex * 3 / 4 : half ? ex / 2 : ex * 2 / 3);
    *px = in.facing_left ? Word(-fx) : fx;
    *py = Word(wide ? ey * 3 / 4 : half ? ey / 2 : ey * 2 / 3);
    return true;
}
static bool IceWindow(SotnWeaponSecondaryInput in)
{
    return in.animation > 0x40 && in.animation < 0x48 && in.attack_mode;
}
// w_050 func_ptr_80170004: eighty sparkles, each waiting 1..24 updates, then drifting from a
// random point on a 32..63 radius into the blade over fifteen steps of its UV schedule.
static bool Sparkles(SotnWeaponSecondary *s, SotnHandWeaponProgram *p, SotnWeaponSecondaryInput in,
                     Uint32 *rng)
{
    if (!IceWindow(in))
    {
        s->removed = true;
        return true;
    }
    const Sint16 x = Word(Pixel(in.x) - in.camera_x + (in.facing_left ? -12 : 12)),
                 y = Word(Pixel(in.y) - in.camera_y - 26 - ((in.draw_flags & 2U) ? 5 : 0));
    if (!s->phase)
    {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 1;
        s->allocated = true;
        for (Uint16 i = 0; i < 80; ++i)
        {
            s->ice_b0[i] = s->ice_b1[i] = s->ice_g0[i] = s->ice_g2[i] = s->ice_size[i] = 0;
            s->ice_g1[i] = (Uint8)(Rand(rng) % 24U + 1U);
        }
        s->priority = (Uint16)(in.priority + 4U);
        s->draw_mode = 0x331;
        s->flags = 0x860000U;
        s->lifetime = 40;
        s->phase = 1;
    }
    else if (!(s->lifetime = (Sint16)(s->lifetime - 1)))
    {
        s->removed = true;
        return true;
    }
    for (Uint16 i = 0; i < 80; ++i)
    {
        if (s->ice_g0[i] == 0)
        {
            if (--s->ice_g1[i] == 0)
            {
                const Sint16 angle = (Sint16)Rand(rng);
                const Sint16 magnitude = (Sint16)((Rand(rng) & 0x1fU) + 32U);
                Sint16 dx = 0, dy = 0;
                if (!BrandOrbit(angle, magnitude, in.executable, &dx, &dy))
                    return false;
                p->ice_x[i] = Word(x + dx);
                p->ice_y[i] = Word(y - dy);
                s->ice_g0[i] = 1;
            }
        }
        else if (s->ice_g0[i] == 1)
        {
            const Sint32 g2 = s->ice_g2[i];
            const Sint16 px = Word(p->ice_x[i] - (p->ice_x[i] - x) * g2 / 15),
                         py = Word(p->ice_y[i] - (p->ice_y[i] - y) * g2 / 15);
            s->ice_g2[i] = (Uint8)(g2 + 1);
            bool ended = false;
            Uint8 size = 0;
            if (!IcePlace(in.overlay, 0x1000, &s->ice_b0[i], &s->ice_b1[i], s->ice_uv[i], &size,
                          &ended))
                return false;
            if (ended)
            {
                s->ice_size[i] = 0;
                s->ice_g0[i] = 2;
            }
            else
            {
                s->ice_cx[i] = px;
                s->ice_cy[i] = py;
                s->ice_size[i] = size;
            }
        }
    }
    return true;
}
// w_050 func_ptr_80170008: the frost ring, rolling properties from its raw parent with a box
// growing with the radius, white-fill wedges fading over the last sixteen of forty updates.
static bool IceRing(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                    SotnWeaponSecondaryEvents *e)
{
    if (!IceWindow(in))
    {
        s->removed = true;
        return true;
    }
    const Sint16 x = Word(Pixel(in.x) - in.camera_x + (in.facing_left ? -12 : 12)),
                 y = Word(Pixel(in.y) - in.camera_y - ((in.draw_flags & 2U) ? 0x1f : 0x1a));
    SetScreen(&s->x, x, in.camera_x);
    SetScreen(&s->y, y, in.camera_y);
    if (!s->phase)
    {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 2;
        s->allocated = true;
        for (Uint16 i = 0; i < 16; ++i)
        {
            SotnPlayerEffectQuad q = {.u = {0, 0, 0x20, 0x20},
                                      .v = {0, 0, 0xe0, 0xe0},
                                      .red = {0, 0, 0xff, 0xff},
                                      .green = {0, 0, 0xff, 0xff},
                                      .blue = {0, 0, 0xff, 0xff},
                                      .palette = 0x15f,
                                      .page = 0x1a,
                                      .priority = (Uint16)(in.priority + 2U),
                                      .draw_mode = 0x335};
            for (Uint16 k = 0; k < 2; ++k)
            {
                SotnMathTrig t = {0};
                if (!SotnMath_Trig(in.executable, (Sint16)((i + k) * 256U), &t))
                    return false;
                q.u[k] = (Uint8)(((t.cosine * 2) >> 8) + 32);
                q.v[k] = (Uint8)(-32 - ((t.sine * 2) >> 8));
                if (q.v[k] < 4)
                    q.v[k] = 0xff;
            }
            s->quads[i] = q;
        }
        s->flags = 0x860000U;
        s->lifetime = 4;
        s->age = 0x28;
        if (!Properties(s, in, rng, e))
            return false;
        s->hitbox_x = s->hitbox_y = 0;
        s->phase = 1;
    }
    else
    {
        if (s->lifetime < 0x28)
            s->lifetime = (Sint16)(s->lifetime + 6);
        if (!--s->age)
        {
            s->removed = true;
            return true;
        }
    }
    s->hitbox_width = s->hitbox_height = (Uint8)(s->lifetime - 4);
    const Uint8 color = s->age < 0x10 ? (Uint8)(s->age * 16U) : 0x80;
    for (Uint16 i = 0; i < 16; ++i)
    {
        SotnPlayerEffectQuad *q = &s->quads[i];
        q->x[2] = q->x[3] = x;
        q->y[2] = q->y[3] = y;
        q->red[2] = q->red[3] = q->green[2] = q->green[3] = q->blue[2] = q->blue[3] = color;
        for (Uint16 k = 0; k < 2; ++k)
        {
            Sint16 dx = 0, dy = 0;
            if (!BrandOrbit((Sint16)((i + k) * 256U), s->lifetime, in.executable, &dx, &dy))
                return false;
            q->x[k] = Word(x + dx);
            q->y[k] = Word(y - dy);
        }
    }
    return true;
}
// w_050 func_ptr_8017000C: thirty-two frost streaks along the blade, each shown for one update
// after a 1..5 delay; the child attacks for eight updates.
static bool Frost(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                  SotnWeaponSecondaryEvents *e)
{
    Sint16 x = 0, y = 0, px = 0, py = 0;
    if (!IceAnchor(in, true, false, &x, &y, &px, &py))
        return false;
    SetScreen(&s->x, x, in.camera_x);
    SetScreen(&s->y, y, in.camera_y);
    s->facing_left = in.facing_left;
    if (!s->phase)
    {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 3;
        s->allocated = true;
        s->flags = 0x860000U;
        for (Uint16 i = 0; i < 32; ++i)
        {
            SotnWeaponSecondaryRow *r = &s->rows[i];
            *r = (SotnWeaponSecondaryRow){.red = {0, 0x20}, .green = {0, 0x20}, .blue = {0, 0x20}};
            r->red[0] = (Uint8)((Rand(rng) & 0x1fU) + 0x80U);
            r->green[0] = (Uint8)((Rand(rng) & 0x1fU) + 0x60U);
            r->blue[0] = (Uint8)((Rand(rng) & 0x1fU) + 0xe0U);
            const Sint16 ry = (Sint16)((Sint32)(Rand(rng) % 24U) - 12),
                         rx = (Sint16)((Sint32)(Rand(rng) % 24U) - 12);
            r->x[0] = Word(x + rx);
            r->y[0] = Word(y + ry);
            r->x[1] = Word(px + r->x[0]);
            r->y[1] = Word(py + r->y[0]);
            r->phase = 0;
            r->timer = (Uint8)(Rand(rng) % 5U + 1U);
            r->hidden = true;
        }
        s->row_priority = (Uint16)(in.priority + 4U);
        s->draw_mode = 0x335;
        if (!Properties(s, in, rng, e))
            return false;
        s->hitbox_x = 0x18;
        s->hitbox_width = 0x18;
        s->hitbox_height = 8;
        s->hitbox_y = 0;
        s->lifetime = 8;
        s->phase = 1;
    }
    else if (!(s->lifetime = (Sint16)(s->lifetime - 1)))
    {
        s->removed = true;
        return true;
    }
    for (Uint16 i = 0; i < 32; ++i)
    {
        SotnWeaponSecondaryRow *r = &s->rows[i];
        if (r->phase == 0)
        {
            if (--r->timer == 0)
            {
                r->hidden = false;
                r->phase = 1;
            }
        }
        else if (r->phase == 1)
        {
            r->hidden = true;
            r->phase = 2;
        }
    }
    return true;
}
// w_050 func_ptr_80170010: eight crystals along the blade (sharing the sparkle points), each
// rising one pixel per placement after a staggered delay.
static bool Crystals(SotnWeaponSecondary *s, SotnHandWeaponProgram *p, SotnWeaponSecondaryInput in,
                     Uint32 *rng)
{
    if (!IceWindow(in))
    {
        s->removed = true;
        return true;
    }
    const Uint16 params = (Uint16)((s->parameter & 0x7f00U) >> 8);
    if (!s->phase)
    {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 4;
        s->allocated = true;
        s->flags = 0x860000U;
        Sint16 x = 0, y = 0, bx = 0, by = 0;
        if (!IceAnchor(in, params == 1, params != 1, &x, &y, &bx, &by))
            return false;
        if (in.draw_flags & 2U)
        {
            const Sint32 index = (Sint32)in.animation - 0x41;
            y = Word(y - (index == 2 || index == 3 ? 1 : 3));
        }
        SetScreen(&s->x, x, in.camera_x);
        SetScreen(&s->y, y, in.camera_y);
        for (Uint16 i = 0; i < 8; ++i)
        {
            Sint16 qx = Word(bx * (Sint32)i / 8 + x), qy = Word(by * (Sint32)i / 8 + y);
            qy = Word(qy + (Sint32)(Rand(rng) % 10U));
            qx = Word(qx + (Sint32)(Rand(rng) % 4U));
            p->ice_x[i] = qx;
            p->ice_y[i] = qy;
            s->quads[i] = (SotnPlayerEffectQuad){.palette = 0x1b0,
                                                 .page = 0x1a,
                                                 .priority = in.priority,
                                                 .draw_mode = (Uint16)(params == 0 ? 0x379 : 0x339),
                                                 .green = {0, (Uint8)(i * 2U + 1U), 0, 0}};
        }
        s->lifetime = 0x18;
        s->phase = 1;
    }
    else if (!(s->lifetime = (Sint16)(s->lifetime - 1)))
    {
        s->removed = true;
        return true;
    }
    for (Uint16 i = 0; i < 8; ++i)
    {
        // The original prim scratch bytes: g0 = green[0], g1 = green[1], b0/b1 = blue[0]/[1].
        SotnPlayerEffectQuad *q = &s->quads[i];
        if (q->green[0] == 0)
        {
            if (--q->green[1] == 0)
                q->green[0] = 1;
        }
        else if (q->green[0] == 1)
        {
            const Sint16 x = p->ice_x[i], y = p->ice_y[i];
            bool ended = false;
            Uint8 size = 0, uv[8] = {0};
            if (!IcePlace(in.overlay, 0x1000, &q->blue[0], &q->blue[1], uv, &size, &ended))
                return false;
            p->ice_y[i] = Word(p->ice_y[i] - 1);
            if (ended)
            {
                q->draw_mode |= 8U;
                q->green[0] = 2;
            }
            else
            {
                for (Uint16 k = 0; k < 4; ++k)
                {
                    q->u[k] = uv[k * 2U];
                    q->v[k] = uv[k * 2U + 1U];
                }
                q->x[0] = q->x[2] = Word(x - size);
                q->x[1] = q->x[3] = Word(x + size);
                q->y[0] = q->y[1] = Word(y - size);
                q->y[2] = q->y[3] = Word(y + size);
                q->draw_mode = (Uint16)(q->draw_mode & ~8U);
            }
        }
    }
    return true;
}

// Place one weapon6/50 sparkle GT4 at a whole screen centre (func_8017B784/func_8017B6A0).
static bool SparklePlace(SotnDataView o, Uint32 at, SotnPlayerEffectQuad *q, Sint16 x, Sint16 y,
                         bool *ended)
{
    Uint8 size = 0, uv[8] = {0};
    if (!IcePlace(o, at, &q->blue[0], &q->blue[1], uv, &size, ended))
        return false;
    if (*ended)
        return true;
    for (Uint16 k = 0; k < 4; ++k)
    {
        q->u[k] = uv[k * 2U];
        q->v[k] = uv[k * 2U + 1U];
    }
    q->x[0] = q->x[2] = Word(x - size);
    q->x[1] = q->x[3] = Word(x + size);
    q->y[0] = q->y[1] = Word(y - size);
    q->y[2] = q->y[3] = Word(y + size);
    return true;
}
// w_006 func_ptr_80170004: five crescents leave the player's hitbox, slow to a stop while
// growing, spin up for twenty-four updates, then accelerate away shedding sparkles. Any hit
// flashes white and retires them eight updates later.
static bool Crescent(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                     SotnWeaponSecondaryEvents *e)
{
    const bool first = !(s->parameter & ~0x8000U);
    if (!s->phase)
    {
        s->role = 1;
        s->allocated = true;
        s->y = (Sint32)((Uint32)s->y + (Uint32)in.parent_hitbox_y * 65536U);
        s->x =
            (Sint32)((Uint32)s->x +
                     (Uint32)(in.facing_left ? -in.parent_hitbox_x : in.parent_hitbox_x) * 65536U);
        s->bank = (Uint16)(0x8010U + in.hand * 2U);
        s->palette = (Uint16)(0x116U + in.hand * 0x18U);
        s->texture = (Uint16)(0x64U + in.hand * 2U);
        s->frame = 0x42;
        s->priority = (Uint16)(in.priority - 2U);
        s->draw_flags = 7;
        s->scale_x = s->scale_y = 0x40;
        s->spin = 0x40;
        const Sint32 speed = (Sint32)Rand(rng) - 0x40000;
        s->velocity_x = s->facing_left ? -speed : speed;
        s->accel_x = s->facing_left ? -0x3c00 : 0x3c00;
        const Uint8 *table = NULL;
        if (!View(in.overlay, 0xbf0U + (Uint32)(s->parameter & 0xffU) * 4U, 4, &table))
            return SDL_SetError("Weapon6 crescent requires its private launch speeds");
        s->velocity_y = (Sint32)(Rand(rng) & 0x1fffU) + (Sint32)Read32(table);
        s->accel_y = -(s->velocity_y >> 4);
        s->flags = 0x08020000U;
        s->rotate = (Sint16)Rand(rng);
        if (first)
            e->sounds[0] = 0x6ee;
        if (!Properties(s, in, rng, e))
            return false;
        e->clear_shield = true;
        e->use_ticks = 4;
        s->phase = 1;
    }
    else if (s->phase == 1)
    {
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->velocity_y);
        s->velocity_x = Decelerate(s->velocity_x, 0x2800);
        s->scale_x = (Sint16)(s->scale_x + 1);
        s->scale_y = s->scale_x;
        if (!s->velocity_x)
        {
            s->priority = (Uint16)(in.priority + 2U);
            s->lifetime = 0x18;
            s->phase = 2;
        }
    }
    else if (s->phase == 2)
    {
        s->spin = (Sint16)(s->spin + 8);
        if (!(s->lifetime = (Sint16)(s->lifetime - 1)))
        {
            e->sounds[0] = 0x668;
            s->phase = 3;
        }
    }
    else if (s->phase == 3)
    {
        s->scale_x = (Sint16)(s->scale_x + 2);
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->velocity_y);
        s->velocity_x = (Sint32)((Uint32)s->velocity_x + (Uint32)s->accel_x);
        s->velocity_y = (Sint32)((Uint32)s->velocity_y + (Uint32)s->accel_y);
        if (!(s->counter & 1))
            e->chain = 0x40U | ((Uint32)(in.hand + 1U) << 12);
    }
    else
    {
        if (!(s->lifetime = (Sint16)(s->lifetime - 1)))
            s->removed = true;
        s->hit_flags = 0;
        return true;
    }
    s->scale_y = s->scale_x;
    s->rotate = Word(s->rotate + s->spin);
    const Uint8 *cycle = NULL;
    if (!View(in.overlay, 0xbd4U + (Uint32)((s->counter >> 1) % 14) * 2U, 2, &cycle))
        return SDL_SetError("Weapon6 crescent requires its private palette cycle");
    s->palette = (Uint16)(Read16(cycle) + ((s->parameter & 0x8000U) ? 0x12eU : 0x116U));
    s->counter = (Sint16)(s->counter + 1);
    s->hitbox_width = s->hitbox_height = (Uint8)((s->scale_x * 3) >> 5);
    if (s->hit_flags)
    {
        s->palette = 0x815f;
        s->blend = 0x10;
        s->lifetime = 8;
        s->phase = 4;
    }
    if (s->counter % 10 == 0 && first)
        e->sounds[e->sounds[0] ? 1 : 0] = 0x60b;
    s->hit_flags = 0;
    return true;
}
// w_006 func_ptr_80170008: a falling sparkle shed by the crescent, retired after its six rows.
static bool MoonSparkle(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng)
{
    bool ended = false;
    if (!s->phase)
    {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 2;
        s->allocated = true;
        s->flags = 0x08800000U;
        s->velocity_x = 0x8000; // velocityY: the quad already holds the shared union words.
        const Sint16 dx = (Sint16)((Sint32)(Rand(rng) % 24U) - 12);
        s->x = (Sint32)((Uint32)s->x + (Uint32)dx * 65536U);
        const Sint16 dy = (Sint16)((Sint32)(Rand(rng) % 24U) - 12);
        s->y = (Sint32)((Uint32)s->y + (Uint32)dy * 65536U);
        s->quads[0] = (SotnPlayerEffectQuad){.palette = 0x1b0,
                                             .page = 0x1a,
                                             .priority = (Uint16)(s->priority + 4U),
                                             .draw_mode = 0x31};
        if (!SparklePlace(in.overlay, 0xc04, &s->quads[0], Word(Pixel(s->x) - in.camera_x),
                          Word(Pixel(s->y) - in.camera_y), &ended))
            return false;
        s->phase = 1;
        return true;
    }
    s->y = (Sint32)((Uint32)s->y + (Uint32)s->velocity_x);
    if (!SparklePlace(in.overlay, 0xc04, &s->quads[0], Word(Pixel(s->x) - in.camera_x),
                      Word(Pixel(s->y) - in.camera_y), &ended))
        return false;
    s->removed = ended;
    return true;
}

// w_014 blade line at the entity's rotation: its point 47 pixels out and its hilt 8 pixels out.
static bool RuneCorners(SotnDataView exe, Sint16 angle, Sint16 x, Sint16 y, bool left, Sint16 *x1,
                        Sint16 *y1, Sint16 *x3, Sint16 *y3)
{
    SotnMathTrig t = {0};
    if (!SotnMath_Trig(exe, angle, &t))
        return false;
    const Sint32 point = Asr(Asr(t.cosine, 4) * 47, 8), hilt = Asr(t.cosine, 9);
    *x1 = Word(left ? x - point : x + point);
    *y1 = Word(y + Asr(Asr(t.sine, 4) * 47, 8));
    *x3 = Word(left ? x - hilt : x + hilt);
    *y3 = Word(y + Asr(t.sine, 9));
    return true;
}
// w_014 EntityWeaponAttack: the thrown Runesword flies out spinning to level, stops, drifts back,
// then homes on the player's hitbox, retiring there; a 48-row trail follows its blade.
static bool Rune(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                 SotnWeaponSecondaryEvents *e)
{
    const Sint32 sx = Word(Pixel(s->x) - in.camera_x);
    switch (s->phase)
    {
    case 0: {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 0;
        s->allocated = true;
        s->bank = (Uint16)(0x8010U + in.hand * 2U);
        s->texture = (Uint16)(0x64U + in.hand * 2U);
        s->palette = in.hand ? 0x134 : 0x11c;
        s->frame = 8;
        s->facing_left = in.facing_left;
        s->priority = (Uint16)(in.priority - 2U);
        s->flags = 0x0c820000U;
        s->velocity_x = s->facing_left ? -0x60000 : 0x60000;
        s->rune_velocity_y = 0x10000;
        const Sint16 offset = Word(Word(Pixel(in.y) - in.camera_y) + in.parent_hitbox_y);
        SetScreen(&s->y, Word(offset - (in.player_crouch_step ? 8 : 16)), in.camera_y);
        const Sint16 px = Word(Pixel(in.x) - in.camera_x);
        SetScreen(&s->x, Word(s->facing_left ? px - in.parent_hitbox_x : px + in.parent_hitbox_x),
                  in.camera_x);
        s->velocity_x = (Sint32)((Uint32)s->velocity_x + (Uint32)in.player_velocity_x);
        s->lifetime = 0x16;
        if (!Properties(s, in, rng, e))
            return false;
        e->clear_shield = true;
        s->hitbox_width = s->hitbox_height = 4;
        s->rotate = -0x700;
        s->draw_flags |= 4;
        Sint16 x0 = 0, y0 = 0, x2 = 0, y2 = 0;
        if (!RuneCorners(in.executable, s->rotate, Word(Pixel(s->x) - in.camera_x),
                         Word(Pixel(s->y) - in.camera_y), s->facing_left, &x0, &y0, &x2, &y2))
            return false;
        s->rune_x0 = x0 + in.camera_x;
        s->rune_y0 = y0 + in.camera_y;
        s->rune_x2 = x2 + in.camera_x;
        s->rune_y2 = y2 + in.camera_y;
        for (Uint16 i = 0; i < 48; ++i)
            s->rune_hidden[i] = true;
        s->row_priority = (Uint16)(s->priority - 2U);
        s->draw_mode = 0x173;
        e->sounds[0] = 0x677;
        e->volume_sound = true;
        e->volume = 0x50;
        e->sounds[1] = 0x66c;
        e->use_ticks = 4;
        e->chain = 0x72U | ((Uint32)(in.hand + 1U) << 12);
        s->phase = 1;
        break;
    }
    case 1:
        s->rotate = Word(s->rotate + 0x70);
        if (s->rotate > 0)
            s->rotate = 0;
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->rune_velocity_y);
        s->lifetime = (Sint16)(s->lifetime - 1);
        if (s->lifetime < 7)
            s->velocity_x = Decelerate(s->velocity_x, 0xe0000);
        if (!s->lifetime)
        {
            s->lifetime = 8;
            s->velocity_x = s->facing_left ? 0x30000 : -0x30000;
            s->phase = 2;
        }
        break;
    case 2:
        s->draw_flags = (Uint8)(s->draw_flags & 0xfbU);
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->lifetime = (Sint16)(s->lifetime - 1);
        if (!s->lifetime)
            s->phase = 3;
        break;
    default: {
        const Sint32 tx = (Sint32)Word(Pixel(in.x) - in.camera_x) + in.parent_hitbox_x,
                     ty = (Sint32)Word(Pixel(in.y) - in.camera_y) + in.parent_hitbox_y,
                     sy = Word(Pixel(s->y) - in.camera_y);
        const Sint16 dx = Word(tx - sx < 0 ? sx - tx : tx - sx),
                     dy = Word(ty - sy < 0 ? sy - ty : ty - sy);
        if (dx < (in.player_hitbox_width + 16) / 2 && dy < (in.player_hitbox_height + 4) / 2)
        {
            s->removed = true;
            return true;
        }
        Sint16 angle = 0;
        if (!SotnMath_Atan2(in.executable, -(Sint32)Word(ty - sy), Word(tx - sx), &angle))
            return false;
        angle = (Sint16)(angle & 0xfff);
        SotnMathTrig t = {0};
        if (!SotnMath_Trig(in.executable, angle, &t))
            return false;
        s->x = (Sint32)((Uint32)s->x + (Uint32)t.cosine * 32U * 5U);
        s->y = (Sint32)((Uint32)s->y - (Uint32)t.sine * 32U * 5U);
        s->rotate = s->facing_left ? angle : Word(0x800 - angle);
        s->draw_flags |= 4;
        break;
    }
    }
    // Uncover the row for this callback, keep sixteen visible and spread the texture over them.
    const Sint16 count = s->rune_count;
    for (Sint32 i = 0; i < 48; ++i)
    {
        if (!s->rune_shown[i] && i == count - 1)
        {
            Sint16 x1 = 0, y1 = 0, x3 = 0, y3 = 0;
            if (!RuneCorners(in.executable, s->rotate, Word(Pixel(s->x) - in.camera_x),
                             Word(Pixel(s->y) - in.camera_y), s->facing_left, &x1, &y1, &x3, &y3))
                return false;
            s->rune_x1[i] = x1 + in.camera_x;
            s->rune_y1[i] = y1 + in.camera_y;
            s->rune_x3[i] = x3 + in.camera_x;
            s->rune_y3[i] = y3 + in.camera_y;
            s->rune_hidden[i] = false;
            s->rune_shown[i] = true;
        }
        if (count - 1 >= 0x11 && i < count - 0x11)
            s->rune_hidden[i] = true;
    }
    if (count)
    {
        const Sint16 end = count > 47 ? 47 : Word(count - 1);
        Sint16 start = Word(count - 16);
        if (start < 0)
            start = 0;
        if (start > 47)
            start = 47;
        Sint32 range = end - start + 1;
        if (!range)
            range = 1;
        const Sint16 step = Word(128 / range);
        for (Sint32 i = start; i <= end && i < 48; ++i)
        {
            s->rune_u0[i] = (Uint8)((i - start) * step);
            s->rune_u1[i] = (Uint8)((i - start + 1) * step);
        }
    }
    s->rune_count = Word(count + 1);
    return true;
}
// w_014 func_ptr_80170004: four 4x4 hitboxes along the parent's blade (paramsLo 0..3), retired
// once the raw parent slot is empty.
static bool RuneHitbox(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                       SotnWeaponSecondaryEvents *e)
{
    if (!in.parent_exists)
    {
        s->removed = true;
        return true;
    }
    const Uint16 index = (Uint16)(s->parameter & 0xffU);
    if (index > 3)
        return SDL_SetError("Weapon14 hitbox requires its actual paramsLo0..3");
    if (!s->phase)
    {
        s->role = 1;
        s->allocated = true;
        s->facing_left = in.parent_facing_left;
        s->flags = 0x0c020000U;
        if (!Properties(s, in, rng, e))
            return false;
        s->hitbox_width = s->hitbox_height = 4;
        s->phase = 1;
    }
    Sint16 angle = (in.parent_draw_flags & 4U) ? in.parent_rotate : 0;
    if (s->facing_left)
        angle = Word(0x800 - angle);
    static const Uint8 scale[4] = {5, 5, 15, 5}, shift[4] = {7, 6, 7, 5};
    SotnMathTrig t = {0};
    if (!SotnMath_Trig(in.executable, angle, &t))
        return false;
    const Sint16 bx = Word(Asr(Asr(t.cosine, 4) * scale[index], shift[index])),
                 by = Word(Asr(Asr(t.sine, 4) * scale[index], shift[index]));
    SetScreen(&s->x, Word(bx + Word(Pixel(in.parent_x) - in.camera_x)), in.camera_x);
    SetScreen(&s->y, Word(by + Word(Pixel(in.parent_y) - in.camera_y)), in.camera_y);
    return true;
}

// w_013 shared schedule of the root's flight steps2/3: an afterimage every other g_GameTimer,
// the childPalette cycle (D_8017A5E0) and a fading SWISH_A every eighth.
static bool HeavenTrail(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in,
                        SotnWeaponSecondaryEvents *e)
{
    if (!(in.game_timer & 1U))
        e->chain = 0x3aU | ((Uint32)(in.hand + 1U) << 12);
    const Uint8 *cycle = NULL;
    if (!View(in.overlay, 0x5e0U + (Uint32)((s->counter >> 1) % 14) * 2U, 2, &cycle))
        return SDL_SetError("Weapon13 root requires its private palette cycle");
    s->child_palette = Word(s->palette + (Sint16)Read16(cycle));
    if (!(in.game_timer & 7U))
    {
        e->sounds[0] = 0x60a;
        e->volume_sound = true;
        e->volume = (Uint16)s->volume;
        s->volume -= 4;
        if (s->volume < 0)
            s->volume = 0;
    }
    s->counter = Word(s->counter + 1);
    return true;
}
// w_013 EntityWeaponAttack: Heaven sword flies out, stops, rises on a parabola spinning, then
// steers toward the player's hitbox with a growing turn limit, shedding afterimages.
static bool Heaven(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                   SotnWeaponSecondaryEvents *e)
{
    switch (s->phase)
    {
    case 0: {
        s->role = 0;
        s->allocated = true;
        s->bank = (Uint16)(0x8010U + in.hand * 2U);
        s->texture = (Uint16)(0x64U + in.hand * 2U);
        s->palette = in.hand ? 0x128 : 0x110;
        s->frame = 6;
        s->facing_left = in.facing_left;
        s->priority = (Uint16)(in.priority - 2U);
        s->flags = 0x0c020000U;
        s->velocity_x = s->facing_left ? -0x60000 : 0x60000;
        Sint16 y = Word(Word(Pixel(in.y) - in.camera_y) + in.parent_hitbox_y - 8);
        if (!in.player_crouch_step)
            y = Word(y - 8);
        SetScreen(&s->y, y, in.camera_y);
        const Sint16 px = Word(Pixel(in.x) - in.camera_x);
        SetScreen(&s->x, Word(s->facing_left ? px - in.parent_hitbox_x : px + in.parent_hitbox_x),
                  in.camera_x);
        s->velocity_x = (Sint32)((Uint32)s->velocity_x + (Uint32)in.player_velocity_x);
        s->lifetime = 0x16;
        if (!Properties(s, in, rng, e))
            return false;
        e->clear_shield = true;
        s->hitbox_width = 0x12;
        s->hitbox_height = 4;
        e->sounds[0] = 0x60c;
        s->volume = 0x50;
        e->use_ticks = 4;
        s->phase = 1;
        return true;
    }
    case 1:
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->lifetime = (Sint16)(s->lifetime - 1);
        if (s->lifetime < 7)
            s->velocity_x = Decelerate(s->velocity_x, 0x10000);
        if (!s->lifetime)
        {
            s->velocity_y = -0x18000;
            s->lifetime = 0x18;
            s->velocity_x = s->facing_left ? 0x30000 : -0x30000;
            s->phase = 2;
        }
        return true;
    case 2: {
        s->hitbox_width = s->hitbox_height = 12;
        s->draw_flags |= 4;
        s->rotate = Word(s->rotate - 0x80);
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->velocity_y);
        s->velocity_y = (Sint32)((Uint32)s->velocity_y + 0x1000U);
        s->lifetime = (Sint16)(s->lifetime - 1);
        if (!s->lifetime)
        {
            s->magnitude = 0x80;
            Sint16 heading = 0;
            if (!SotnMath_Atan2Vector(in.executable, (Sint32)(0U - (Uint32)s->velocity_y),
                                      s->velocity_x, &heading))
                return false;
            s->angle = (Sint16)(heading & 0xfff);
            s->phase = 3;
        }
        return HeavenTrail(s, in, e);
    }
    default: {
        s->rotate = Word(s->rotate - 0x80);
        Sint16 limit = s->magnitude;
        s->magnitude = Word(s->magnitude + 0x20);
        const Sint32 sx = Word(Pixel(s->x) - in.camera_x), sy = Word(Pixel(s->y) - in.camera_y),
                     tx = (Sint32)Word(Pixel(in.x) - in.camera_x) + in.parent_hitbox_x,
                     ty = (Sint32)Word(Pixel(in.y) - in.camera_y) + in.parent_hitbox_y;
        const Sint16 dx = Word(tx - sx < 0 ? sx - tx : tx - sx),
                     dy = Word(ty - sy < 0 ? sy - ty : ty - sy);
        if (dx < (in.player_hitbox_width + s->hitbox_width) / 2 &&
            dy < (in.player_hitbox_height + s->hitbox_height) / 2)
        {
            s->removed = true;
            return true;
        }
        Sint16 target = 0;
        if (!SotnMath_Atan2(in.executable, -(Sint32)Word(ty - sy), Word(tx - sx), &target))
            return false;
        target = (Sint16)(target & 0xfff);
        Sint16 angle = (Sint16)(s->angle & 0xfff);
        const Sint16 diff = Word(angle - target < 0 ? target - angle : angle - target);
        if (limit > diff)
            limit = diff;
        if (angle < target)
            angle = Word(diff < 0x800 ? angle + limit : angle - limit);
        else
            angle = Word(diff < 0x800 ? angle - limit : angle + limit);
        s->angle = (Sint16)(angle & 0xfff);
        SotnMathTrig t = {0};
        if (!SotnMath_Trig(in.executable, s->angle, &t))
            return false;
        s->x = (Sint32)((Uint32)s->x + (Uint32)t.cosine * 64U);
        s->y = (Sint32)((Uint32)s->y - (Uint32)t.sine * 64U);
        return HeavenTrail(s, in, e);
    }
    }
}
// The combo sword's whole screen 16.16 position as world coordinates (no CAMERA_LOCKED).
static Sint32 ScreenWorld(Sint32 screen, Sint32 camera)
{
    return (Sint32)((Uint32)screen + ((Uint32)camera << 16));
}
// w_013 func_ptr_80170008: combo202's sword sweeps an arc out from the player's hitbox, shedding
// strike swords, then blinks at its target, flashes white and stretches into a screen-wide slash
// with a LINE_G2 row allocated only then (failure destroys it).
static bool HeavenSweep(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                        SotnWeaponSecondaryEvents *e)
{
    switch (s->phase)
    {
    case 0: {
        s->role = 2;
        s->allocated = true;
        s->bank = (Uint16)(0x8010U + in.hand * 2U);
        s->texture = (Uint16)(0x64U + in.hand * 2U);
        s->palette = in.hand ? 0x128 : 0x110;
        s->frame = 6;
        s->facing_left = in.facing_left;
        s->priority = (Uint16)(in.priority + 2U);
        s->flags = 0x04000000U;
        s->draw_flags = 4;
        Sint16 y = Word(Word(Pixel(in.y) - in.retained_camera_y) + in.parent_hitbox_y - 8);
        if (!in.player_crouch_step)
            y = Word(y - 8);
        const Sint16 px = Word(Pixel(in.x) - in.retained_camera_x),
                     x = Word(s->facing_left ? px - in.parent_hitbox_x : px + in.parent_hitbox_x);
        SetScreen(&s->x, x, in.retained_camera_x);
        SetScreen(&s->y, y, in.retained_camera_y);
        s->heaven_cx = (Sint32)((Uint32)(Uint16)x << 16);
        s->heaven_cy = (Sint32)((Uint32)(Uint16)y << 16);
        s->heaven_angle = -0x200;
        s->heaven_radius = 0x100;
        s->heaven_target_x = Word(s->facing_left ? x - 0x78 : x + 0x78);
        s->heaven_target_y = y;
        if (!Properties(s, in, rng, e))
            return false;
        e->clear_shield = true;
        s->hitbox_width = s->hitbox_height = 14;
        s->heaven_volume = 0x60;
        e->use_ticks = 4;
        s->phase = 1;
        return true;
    }
    case 1: {
        s->rotate = Word(s->rotate + 0x100);
        const Sint16 angle = s->heaven_angle;
        s->heaven_angle = Word(s->heaven_angle - 0x20);
        s->heaven_radius = (Sint32)((Uint32)s->heaven_radius + 0x10U);
        SotnMathTrig t = {0};
        if (!SotnMath_Trig(in.executable, angle, &t))
            return false;
        const Uint32 radius =
            s->facing_left ? 0U - (Uint32)s->heaven_radius : (Uint32)s->heaven_radius;
        s->x = ScreenWorld((Sint32)((Uint32)t.cosine * radius + (Uint32)s->heaven_cx),
                           in.retained_camera_x);
        s->y = ScreenWorld(
            (Sint32)((0U - (Uint32)s->heaven_radius) * (Uint32)t.sine + (Uint32)s->heaven_cy),
            in.retained_camera_y);
        if (!(in.game_timer & 1U) && s->heaven_angle < -0x400)
        {
            // CreateEntFactoryFromEntity fails silently without a free factory entity; the
            // counter advances either way.
            const Uint8 *cycle = NULL;
            if (!View(in.overlay, 0x5e0U + (Uint32)((s->heaven_count >> 1) % 14) * 2U, 2, &cycle))
                return SDL_SetError("Weapon13 sweep requires its private palette cycle");
            if (in.free_factories > e->factory_count &&
                e->factory_count < SDL_arraysize(e->factories))
            {
                e->factories[e->factory_count] =
                    (0x46U + ((in.hand + 1U) << 14)) + ((Uint32)Read16(cycle) << 16);
                e->factory_x[e->factory_count] = s->x;
                e->factory_y[e->factory_count++] = s->y;
            }
            s->heaven_count = Word(s->heaven_count + 1);
        }
        if (in.game_timer % 6U == 0)
        {
            e->sounds[0] = 0x60a;
            e->volume_sound = true;
            e->volume = (Uint16)s->heaven_volume;
            s->heaven_volume -= 4;
            if (s->heaven_volume < 0)
                s->heaven_volume = 0;
        }
        if (s->heaven_angle >= -0xb00)
            return true;
        s->heaven_timer = 8;
        s->phase = 2;
        return true;
    }
    case 2:
        s->heaven_timer = Word(s->heaven_timer - 1);
        if (!s->heaven_timer)
        {
            s->draw_flags = 0x80;
            s->phase = 3;
        }
        return true;
    case 3:
        s->heaven_timer = Word(s->heaven_timer + 1);
        if (s->heaven_timer < 0x20 && !(s->heaven_timer & 3))
            e->sounds[0] = 0x60b;
        if (s->heaven_timer > 0x10)
        {
            SetScreen(&s->x, s->heaven_target_x, in.retained_camera_x);
            SetScreen(&s->y, s->heaven_target_y, in.retained_camera_y);
            s->hitbox_width = 0x12;
            s->hitbox_height = 4;
        }
        if (s->heaven_timer > 0x30)
            s->draw_flags = 0;
        if (s->heaven_timer == 0x34)
        {
            e->sounds[0] = 0x66a;
            s->draw_flags = 3;
            s->scale_x = s->scale_y = 0x100;
            s->palette = 0x815f;
            // The caller reserved the one LINE_G2 row before this update.
            if (!in.allocated)
            {
                s->removed = true;
                return true;
            }
            const Sint16 x = Word(Pixel(s->x) - in.retained_camera_x),
                         y = Word(Pixel(s->y) - in.retained_camera_y);
            s->heaven_line = (SotnWeaponSecondaryRow){.x = {x, x},
                                                      .y = {y, y},
                                                      .red = {0xf0, 0xf0},
                                                      .green = {0x80, 0x80},
                                                      .blue = {0xf0, 0xf0}};
            s->flags |= 0x800000U; // FLAG_HAS_PRIMS
            s->row_priority = s->priority;
            s->draw_mode = 0x31;
            s->phase = 4;
        }
        return true;
    default: {
        SotnWeaponSecondaryRow *l = &s->heaven_line;
        if (l->x[0] > -0x110)
            l->x[0] = Word(l->x[0] - 0x10);
        if (l->x[1] < 0x110)
            l->x[1] = Word(l->x[1] + 0x10);
        if (l->x[0] < 0 && l->x[1] > 0x100)
        {
            if (l->blue[1] > 8)
                l->blue[1] = (Uint8)(l->blue[1] - 8);
            if (l->green[1] > 8)
                l->green[1] = (Uint8)(l->green[1] - 4);
            else
            {
                s->removed = true;
                return true;
            }
            l->red[0] = l->blue[0] = l->red[1] = l->blue[1];
            l->green[0] = l->green[1];
        }
        s->scale_x = Word(s->scale_x + 0x80);
        s->scale_y = 0x20;
        if (s->scale_x >= 0x700)
            s->frame = 0;
        const Sint16 px = Word(Pixel(s->x) - in.retained_camera_x);
        if (px >= 0x101)
            s->hitbox_x = Word(0x100 - px);
        if (px < 0)
            s->hitbox_x = Word(-px);
        if (s->facing_left)
            s->hitbox_x = Word(-s->hitbox_x);
        s->hitbox_width = 0xfe;
        s->hitbox_height = 2;
        return true;
    }
    }
}
// w_013 func_ptr_8017000C: a strike sword copies its raw parent sweep at birth, waits for the
// sweep's step3, counts down 1..32 updates, then flies at the sweep's target, retiring past it.
// Every update first retires it once the raw parent slot is empty.
static bool HeavenStrike(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                         SotnWeaponSecondaryEvents *e)
{
    if (!in.parent_exists)
    {
        s->removed = true;
        return true;
    }
    switch (s->phase)
    {
    case 0:
        s->role = 3;
        s->allocated = true;
        s->frame = in.parent_frame;
        s->bank = in.parent_bank;
        s->facing_left = in.parent_facing_left;
        s->texture = in.parent_texture;
        s->priority = in.parent_priority;
        s->flags = 0x04000000U;
        s->palette = (Uint16)(in.parent_palette + (s->parameter >> 8));
        s->draw_flags = (Uint8)(in.parent_draw_flags | 0x80U);
        s->rotate = in.parent_rotate;
        s->heaven_target_x = in.parent_target_x;
        s->heaven_target_y = in.parent_target_y;
        s->heaven_timer = (Sint16)((Rand(rng) & 0x1fU) + 1U);
        s->phase = 1;
        return true;
    case 1:
        if (in.parent_phase == 3)
            s->phase = 2;
        return true;
    case 2: {
        s->heaven_timer = Word(s->heaven_timer - 1);
        if (s->heaven_timer)
            return true;
        s->rotate = 0;
        const Sint32 sx = Word(Pixel(s->x) - in.retained_camera_x),
                     sy = Word(Pixel(s->y) - in.retained_camera_y);
        Sint16 angle = 0;
        if (!SotnMath_Atan2(in.executable, -(s->heaven_target_y - sy), s->heaven_target_x - sx,
                            &angle))
            return false;
        SotnMathTrig t = {0};
        if (!SotnMath_Trig(in.executable, angle, &t))
            return false;
        s->velocity_x = (Sint32)((Uint32)t.cosine * 0x100U);
        s->heaven_vy = (Sint32)((0U - (Uint32)t.sine) * 0x100U);
        s->phase = 3;
        if (!Properties(s, in, rng, e))
            return false;
        s->hitbox_width = 0x12;
        s->hitbox_height = 4;
        return true;
    }
    default: {
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->heaven_vy);
        const Sint16 px = Word(Pixel(s->x) - in.retained_camera_x);
        if (s->facing_left ? px < s->heaven_target_x : px > s->heaven_target_x)
            s->removed = true;
        return true;
    }
    }
}
// w_007 func_ptr_80170004: Shield rod special173's orb. At birth it copies its raw slot16 parent's
// facing, bank, texture and palette, offsets itself by D_8017AC48[params>>7] and launches at
// SetSpeedX(4.75) (3.75 with velocityY 2.75 for animations3/6) from 1/16 scale. It decelerates
// while growing, bobs on rsin while counting 0x200 updates down (a WFACTORY(0x40,0) sparkle every
// eighth), and shrinks out after the countdown or a hit. A hit while still growing leaves step4,
// which the original never handles.
static bool RodOrb(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in,
                   SotnWeaponSecondaryEvents *e)
{
    switch (s->phase)
    {
    case 0: {
        if (!in.parent_exists)
        {
            s->removed = true;
            return true;
        }
        const Uint8 *offset = NULL;
        if (!View(in.overlay, 0xc48U + (Uint32)(s->parameter >> 7) * 2U, 4, &offset))
            return SDL_SetError("Weapon7 orb requires its private origin table");
        s->role = 1;
        s->allocated = true;
        s->facing_left = in.parent_facing_left;
        Uint16 ox = Read16(offset);
        if (s->facing_left)
            ox = (Uint16)(0U - ox);
        s->x = (Sint32)((Uint32)s->x + ((Uint32)ox << 16));
        s->y = (Sint32)((Uint32)s->y + ((Uint32)Read16(offset + 2) << 16));
        s->bank = in.parent_bank;
        s->priority = 0xa8;
        s->texture = in.parent_texture;
        s->palette = in.parent_palette;
        const bool low = (s->parameter & 0xff00U) == 0x300 || (s->parameter & 0xff00U) == 0x600;
        const Sint32 speed = low ? 0x3c000 : 0x4c000;
        s->velocity_x = s->facing_left ? -speed : speed;
        s->velocity_y = low ? 0x2c000 : 0;
        s->draw_flags = 3;
        s->scale_x = s->scale_y = 0x10;
        s->flags = 0x08100000U;
        s->hitbox_width = s->hitbox_height = 9;
        // Fixed words, not SetWeaponProperties: hit type4 keeps attack1.
        s->properties = (SotnEquipment){
            .attack = 1, .hit_type = 4, .invincibility = 4, .stun = 4, .hit_effect = 1};
        s->hitbox_state = 4;
        e->group = true;
        e->sounds[0] = 0x6ad;
        s->phase = 1;
        return true;
    }
    case 1:
        if (s->hit_flags)
            s->phase = 3;
        s->velocity_x = Decelerate(s->velocity_x, 0x3800);
        s->velocity_y = Decelerate(s->velocity_y, 0x3800);
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->velocity_y);
        if (s->scale_x < 0x100)
        {
            s->scale_x = Word(s->scale_x + 0x10);
            s->scale_y = Word(s->scale_y + 0x10);
        }
        if (!s->velocity_x && s->scale_x >= 0x100)
        {
            s->orb_countdown = 0x200;
            ++s->phase;
        }
        return true;
    case 3:
        s->scale_x = Word(s->scale_x - 0xc);
        s->scale_y = Word(s->scale_y - 0xc);
        if (s->scale_x < 0x10)
        {
            s->removed = true;
            return true;
        }
        // fall through
    case 2: {
        SotnMathTrig t = {0};
        if (!SotnMath_Trig(in.executable, s->orb_wave, &t))
            return false;
        s->velocity_y = t.sine;
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->velocity_y);
        s->orb_wave = Word(s->orb_wave + 0x20);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->velocity_y);
        if (!(s->orb_countdown & 7))
            e->chain = 0x40U | ((in.hand + 1U) << 12);
        s->orb_countdown = Word(s->orb_countdown - 1);
        if (!s->orb_countdown || s->hit_flags)
            s->phase = 3;
        return true;
    }
    default:
        return true;
    }
}
// w_007 func_8017B604: the sparkle's 6x6 (from frame3, 4x4) GT4 around its screen point with
// D_8017AC64's UVs; b1 advances b0 every second call and frame6 retires it.
static bool RodSparkleQuad(SotnWeaponSecondary *s, SotnDataView overlay, Sint16 x, Sint16 y,
                           bool *done)
{
    SotnPlayerEffectQuad *q = &s->quads[0];
    const Uint8 frame = s->quad_phase[0];
    const Sint16 size = frame >= 3 ? 2 : 3;
    *done = frame == 6;
    if (*done)
        return true;
    const Uint8 *uv = NULL;
    if (!View(overlay, 0xc64U + frame * 8U, 8, &uv))
        return SDL_SetError("Weapon7 sparkle requires its private UV frames");
    const Sint16 left = Word(x - size), right = Word(x + size), top = Word(y - size),
                 bottom = Word(y + size);
    const Sint16 xs[4] = {left, right, left, right}, ys[4] = {top, top, bottom, bottom};
    for (Uint16 v = 0; v < 4; ++v)
    {
        q->x[v] = xs[v];
        q->y[v] = ys[v];
        q->u[v] = uv[v * 2U];
        q->v[v] = uv[v * 2U + 1U];
    }
    if (!(++s->quad_timer[0] & 1U))
        ++s->quad_phase[0];
    return true;
}
// w_007 func_ptr_80170008: a sparkle takes one GT4 (failure retires it before its rand calls),
// scatters by rand()%16-8 / rand()%16 and sinks at 0.75 pixels per update through six frames.
static bool RodSparkle(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng)
{
    bool done = false;
    if (!s->phase)
    {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 2;
        s->allocated = true;
        s->flags = 0x08800000U;
        s->quad_velocity_y = 0xc000;
        const Sint32 dx = (Sint32)(Rand(rng) % 16U) - 8, dy = (Sint32)(Rand(rng) % 16U);
        s->x = (Sint32)((Uint32)s->x + ((Uint32)dx << 16));
        s->y = (Sint32)((Uint32)s->y + ((Uint32)dy << 16));
        s->quads[0] = (SotnPlayerEffectQuad){.page = 0x1a,
                                             .palette = 0x1b0,
                                             .priority = (Uint16)(s->priority + 4U),
                                             .draw_mode = 0x31};
        s->quad_phase[0] = s->quad_timer[0] = 0;
        s->phase = 1;
        return RodSparkleQuad(s, in.overlay, Word(Pixel(s->x) - in.retained_camera_x),
                              Word(Pixel(s->y) - in.retained_camera_y), &done);
    }
    s->y = (Sint32)((Uint32)s->y + (Uint32)s->quad_velocity_y);
    if (!RodSparkleQuad(s, in.overlay, Word(Pixel(s->x) - in.retained_camera_x),
                        Word(Pixel(s->y) - in.retained_camera_y), &done))
        return false;
    s->removed = done;
    return true;
}
static void SpellColors(SotnPlayerEffectQuad *q, Uint8 v);
// ((rcos/rsin >> 4) * radius) >> 8 from a whole-pixel centre, as w_010's spokes compute it.
static Sint16 IronSpoke(Sint16 centre, Sint32 trig, Sint32 radius, bool subtract)
{
    const Sint32 d = Asr(Asr(trig, 4) * radius, 8);
    return Word(subtract ? centre - d : centre + d);
}
static Uint8 IronGlow(SotnDataView exe, Sint16 angle, Sint16 bright, bool *ok)
{
    SotnMathTrig t = {0};
    *ok = SotnMath_Trig(exe, angle, &t);
    return (Uint8)(((t.sine + 0x1000) >> 6) * bright / 256);
}
static bool IronAngles(SotnHandWeaponProgram *p, SotnDataView o)
{
    const Uint8 *a = NULL, *b = NULL;
    if (p->iron_loaded)
        return true;
    if (!View(o, 0xbbc, 32, &a) || !View(o, 0xbdc, 32, &b))
        return SDL_SetError("Weapon10 requires its private shieldAngles tables");
    for (Uint16 i = 0; i < 16; ++i)
    {
        p->iron_angles[i] = Word(Read16(a + i * 2U));
        p->iron_angles2[i] = Word(Read16(b + i * 2U));
    }
    p->iron_loaded = true;
    return true;
}
// w_010 func_ptr_80170004: the Iron shield guard's thunder. Every update it retires once its raw
// slot16 parent is gone or no longer holds item7; it follows the player's whole position, takes
// the parent's bank/unk5A, palette0x194 four priorities in front and plays anim_thunder once.
static bool IronThunder(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in,
                        SotnWeaponSecondaryEvents *e)
{
    if (!in.parent_exists)
    {
        s->removed = true;
        return true;
    }
    s->x = (Sint32)(((Uint32)in.x & 0xffff0000U) | ((Uint32)s->x & 0xffffU));
    s->y = (Sint32)(((Uint32)in.y & 0xffff0000U) | ((Uint32)s->y & 0xffffU));
    if (!s->phase)
    {
        s->role = 1;
        s->allocated = true;
        s->bank = in.parent_bank;
        s->palette = 0x8194;
        s->priority = (Uint16)(in.parent_priority + 4U);
        s->texture = in.parent_texture;
        s->flags = 0x160000U;
        e->sounds[0] = 0x664;
        s->phase = 1;
    }
    else if (s->remaining < 0)
        s->removed = true;
    return true;
}
// w_010 EntityWeaponShieldSpell: Iron shield's spell205. Thirty-three GT4s: the growing white glow
// and sixteen band/copy pairs on a disc. Like the card spells it retires the raised shield, freezes
// the scene and drifts back while the shield image grows and flashes out; the disc then opens,
// releases FACTORY(88,0/1)'s two swords, ends the freeze, dims, brightens, spreads and fades. Each
// update the copies re-colour from the program's retained shieldAngles. Rows are screen pixels.
static bool IronSpell(SotnWeaponSecondary *s, SotnHandWeaponProgram *p, SotnWeaponSecondaryInput in,
                      SotnWeaponSecondaryEvents *e)
{
    if (!IronAngles(p, in.overlay))
        return false;
    bool hide = false;
    switch (s->phase)
    {
    case 0: {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 8;
        s->allocated = true;
        s->bank = in.hand ? 0x8012 : 0x8010;
        s->palette = in.hand ? 0x128 : 0x110;
        s->spell_child_palette = in.hand ? 0x129 : 0x111;
        s->texture = in.hand ? 0x66 : 0x64;
        s->spell_v0 = in.hand ? 0 : 0x80;
        s->spell_v1 = in.hand ? 0x80 : 0;
        s->y = (Sint32)((Uint32)s->y - 0x80000U);
        s->flags = 0x04810000U;
        s->priority = (Uint16)(in.priority - 2U);
        s->facing_left = in.facing_left;
        s->frame = 0x3e;
        s->draw_flags = 3;
        s->scale_x = s->scale_y = 0;
        SotnPlayerEffectQuad *q = &s->spell_quads[0];
        *q = (SotnPlayerEffectQuad){.palette = 0x19f, .page = 0x19};
        q->u[0] = q->u[2] = 0x80;
        q->u[1] = q->u[3] = 0x80 + 0x4f;
        q->v[0] = q->v[1] = s->spell_v0;
        q->v[2] = q->v[3] = (Uint8)(s->spell_v0 + 0x4f);
        s->spell_glow = 0;
        SpellColors(q, 0x80);
        q->priority = (Uint16)(s->priority - 4U);
        q->draw_mode = 0x40 | 0x20 | 0x10 | 0x04 | 0x01;
        s->spell_half_h = 0;
        s->spell_half_w = 2;
        s->spell_wave_x = 0;
        for (Uint16 i = 0; i < 16; ++i)
        {
            SotnMathTrig a = {0}, b = {0};
            if (!SotnMath_Trig(in.executable, (Sint16)((i + 1U) * 0x100U), &a) ||
                !SotnMath_Trig(in.executable, (Sint16)(i * 0x100U), &b))
                return false;
            const Sint32 flip = s->facing_left ? -1 : 1;
            q = &s->spell_quads[1 + i * 2U];
            *q = (SotnPlayerEffectQuad){.palette = s->spell_child_palette, .page = 0x19};
            q->u[0] = q->u[2] = 0xc0;
            q->v[0] = q->v[2] = (Uint8)(s->spell_v1 + 0x40);
            q->u[1] = (Uint8)((Asr(Asr(a.cosine, 4) * 0x3e, 8) * flip) + 0xc0);
            q->v[1] = (Uint8)(s->spell_v1 + (0x40 - Asr(Asr(a.sine, 4) * 0x3e, 8)));
            q->u[3] = (Uint8)((Asr(Asr(b.cosine, 4) * 0x3e, 8) * flip) + 0xc0);
            q->v[3] = (Uint8)(s->spell_v1 + (0x40 - Asr(Asr(b.sine, 4) * 0x3e, 8)));
            SpellColors(q, 0x80);
            q->priority = 0x1b8;
            q->draw_mode = 0x08;
            SotnPlayerEffectQuad *c = &s->spell_quads[2 + i * 2U];
            *c = *q;
            c->palette = 0x15f;
            c->priority = 0x1ba;
            c->draw_mode |= 0x20 | 0x10 | 0x04 | 0x01;
        }
        s->spell_step = 0xc0;
        s->spell_bright = 0x100;
        e->clear_shield = true;
        s->velocity_x = s->facing_left ? 0x20000 : -0x20000;
        s->spell_velocity_y = 0;
        e->sounds[0] = 0x641;
        e->freeze_set = true;
        s->phase = 1;
        break;
    }
    case 1:
        s->velocity_x = s->velocity_x < 0 ? SDL_min(s->velocity_x + 0x1000, 0)
                                          : SDL_max(s->velocity_x - 0x1000, 0);
        s->spell_velocity_y -= 0x2800;
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->spell_velocity_y);
        s->scale_x = Word(s->scale_x + 12);
        s->scale_y = s->scale_x;
        s->spell_glow = (Sint16)(s->scale_x * 0x28 / 256);
        if (s->scale_x >= 0x100)
        {
            s->spell_glow = 0x28;
            s->scale_y = s->scale_x = 0x100;
            s->spell_timer = 8;
            s->phase = 2;
        }
        break;
    case 2:
        if (--s->spell_timer == 0)
        {
            s->spell_timer = 0x10;
            s->phase = 3;
        }
        break;
    case 3:
        s->spell_quads[0].priority = !(s->spell_timer & 1) ? 0x1bc : (Uint16)(s->priority - 2U);
        s->scale_x = Word(s->scale_x - 0x10);
        if (s->scale_x <= 0)
            s->scale_x = 0;
        s->scale_y = s->scale_x;
        if (--s->spell_timer == 0)
        {
            s->frame = 0;
            for (Uint16 i = 1; i < 33; ++i)
                s->spell_quads[i].draw_mode &= 0xfff7U;
            e->sounds[0] = 0x66a;
            s->phase = 4;
        }
        break;
    case 4: {
        s->spell_glow = Word(s->spell_glow + 8);
        SotnPlayerEffectQuad *q = &s->spell_quads[0];
        if (q->blue[3] > 8)
            q->blue[3] = (Uint8)(q->blue[3] - 4);
        SpellColors(q, q->blue[3]);
        s->spell_half_w = Word(s->spell_half_w + 2);
        s->spell_half_h = Word(s->spell_half_h + 2);
        if (s->spell_half_w >= 0x3e)
        {
            s->spell_half_w = 0x3e;
            q->draw_mode |= 8;
            s->phase = 5;
        }
        break;
    }
    case 5:
        s->spell_half_h = Word(s->spell_half_h - 2);
        if (s->spell_half_h <= 0)
        {
            s->spell_half_h = 0;
            // FACTORY(88,0) and (88,1): the two swords, with the combo's <<14 hand bits; each
            // CreateEntFactoryFromEntity fails silently without a free factory entity.
            for (Uint32 n = 0; n < 2 && in.free_factories > e->factory_count; ++n)
            {
                e->factories[e->factory_count] = (88U + ((in.hand + 1U) << 14)) | n << 16;
                e->factory_x[e->factory_count] = s->x;
                e->factory_y[e->factory_count++] = s->y;
            }
            e->freeze_clear = true;
            s->phase = 6;
        }
        break;
    case 6:
        s->spell_bright = Word(s->spell_bright - 8);
        if (s->spell_bright <= 0)
        {
            s->spell_bright = 0;
            s->phase = 7;
            s->spell_timer = 0x40;
        }
        break;
    case 7:
        if (--s->spell_timer == 0)
            s->phase = 8;
        break;
    case 8:
        s->spell_bright = Word(s->spell_bright + 12);
        if (s->spell_bright > 0x120)
            s->spell_bright = 0x120;
        s->spell_half_h = Word(s->spell_half_h + 3);
        if (s->spell_half_h >= 0x36)
        {
            s->spell_half_h = 0x36;
            hide = true;
            e->sounds[0] = 0x636;
            s->phase = 9;
        }
        break;
    default:
        s->spell_bright = Word(s->spell_bright - 8);
        if (s->spell_bright < 0)
        {
            s->removed = true;
            return true;
        }
        break;
    }
    const Sint16 x = Word(Pixel(s->x) - in.retained_camera_x),
                 y = Word(Pixel(s->y) - in.retained_camera_y);
    SotnPlayerEffectQuad *q = &s->spell_quads[0];
    if (q->blue[3] > 8)
    {
        if (in.engine_timer & 1U)
            q->draw_mode |= 8;
        else
            q->draw_mode &= 0xfff7U;
    }
    q->x[0] = q->x[2] = Word(x - s->spell_glow);
    q->x[1] = q->x[3] = Word(x + s->spell_glow - 1);
    q->y[0] = q->y[1] = Word(y - s->spell_glow);
    q->y[2] = q->y[3] = Word(y + s->spell_glow - 1);
    for (Uint16 i = 0; i < 16; ++i)
    {
        SotnMathTrig a = {0}, b = {0}, c = {0}, d = {0};
        const Sint16 offset = s->spell_wave_x;
        if (!SotnMath_Trig(in.executable, (Sint16)((i + 1U) * 0x100U + (Uint16)offset), &a) ||
            !SotnMath_Trig(in.executable, (Sint16)((i + 1U) * 0x100U), &b) ||
            !SotnMath_Trig(in.executable, (Sint16)(i * 0x100U + (Uint16)offset), &c) ||
            !SotnMath_Trig(in.executable, (Sint16)(i * 0x100U), &d))
            return false;
        q = &s->spell_quads[1 + i * 2U];
        q->x[0] = IronSpoke(x, a.cosine, s->spell_half_h, false);
        q->y[0] = IronSpoke(y, a.sine, s->spell_half_h, true);
        q->x[1] = IronSpoke(x, b.cosine, s->spell_half_w, false);
        q->y[1] = IronSpoke(y, b.sine, s->spell_half_w, true);
        q->x[2] = IronSpoke(x, c.cosine, s->spell_half_h, false);
        q->y[2] = IronSpoke(y, c.sine, s->spell_half_h, true);
        q->x[3] = IronSpoke(x, d.cosine, s->spell_half_w, false);
        q->y[3] = IronSpoke(y, d.sine, s->spell_half_w, true);
        if (hide)
            q->draw_mode |= 8;
        SotnPlayerEffectQuad *copy = &s->spell_quads[2 + i * 2U];
        *copy = *q;
        bool ok = true;
        copy->red[2] = copy->green[2] = copy->blue[2] = copy->blue[3] = 0;
        copy->green[3] = copy->red[3] =
            IronGlow(in.executable, p->iron_angles[i], s->spell_bright, &ok);
        if (!ok)
            return false;
        copy->red[0] = copy->green[0] = copy->blue[0] = copy->blue[1] = 0;
        copy->green[1] = copy->red[1] =
            IronGlow(in.executable, p->iron_angles[(i + 1) % 16], s->spell_bright, &ok);
        if (!ok)
            return false;
        p->iron_angles[i] = Word(p->iron_angles[i] + s->spell_step);
        copy->palette = 0x15f;
        copy->priority = 0x1ba;
        copy->draw_mode = 0x20 | 0x10 | 0x04 | 0x01;
    }
    return true;
}
// w_010 func_ptr_80170024: one of the spell's two swords (params bit8: the second). Its raw parent
// spell's facing, child palette and v band are copied at birth; it retires once that slot empties.
// Row0 is the original FakePrim centre (screen, drifting at 2.25 after a 0x20 delay that makes the
// sword CAMERA_LOCKED); the blade (row1) and sixteen white slices (rows2..17) circle that centre on
// an opening radius, recoloured from the program's shieldAngles2 as they fade.
static bool IronSword(SotnWeaponSecondary *s, SotnHandWeaponProgram *p, SotnWeaponSecondaryInput in,
                      Uint32 *rng, SotnWeaponSecondaryEvents *e)
{
    if (!IronAngles(p, in.overlay))
        return false;
    const Uint16 upper = (Uint16)((s->parameter & 0x7f00U) >> 8);
    if (!in.parent_exists)
    {
        s->removed = true;
        return true;
    }
    switch (s->phase)
    {
    case 0: {
        s->facing_left = in.parent_facing_left;
        s->spell_child_palette = (Uint16)(in.parent_child_palette + 1U + upper);
        s->spell_v1 = in.parent_band;
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 9;
        s->allocated = true;
        s->flags = 0x04800000U;
        s->iron_cx = (Sint32)((Uint32)s->x - ((Uint32)in.retained_camera_x << 16));
        s->iron_cy = (Sint32)((Uint32)s->y - ((Uint32)in.retained_camera_y << 16));
        s->iron_vx = s->facing_left ? -0x24000 : 0x24000;
        s->spell_quads[0] = (SotnPlayerEffectQuad){.draw_mode = 0x08};
        SotnPlayerEffectQuad *q = &s->spell_quads[1];
        *q = (SotnPlayerEffectQuad){.palette = s->spell_child_palette, .page = 0x19};
        q->u[0] = q->u[2] = 0;
        q->u[1] = q->u[3] = 0x7f;
        q->v[0] = q->v[1] = (Uint8)(s->spell_v1 + 0x30);
        q->v[2] = q->v[3] = (Uint8)(s->spell_v1 + 0x4f);
        SpellColors(q, 0x80);
        q->priority = 0x1bc;
        q->draw_mode = 0;
        for (Uint16 i = 0; i < 16; ++i)
        {
            SotnPlayerEffectQuad *c = &s->spell_quads[2 + i];
            *c = s->spell_quads[1 + i];
            c->u[0] = c->u[2] = (Uint8)(i * 128 / 16);
            c->u[1] = c->u[3] = (Uint8)((i + 1) * 128 / 16 - 1);
            c->palette = 0x15f;
            c->priority = 0x1be;
            c->draw_mode |= 0x20 | 0x10 | 0x04 | 0x01;
        }
        s->spell_step = 0x80;
        s->spell_bright = 0x100;
        if (!((s->facing_left ^ upper) & 1U))
        {
            s->iron_step = -0x100;
            s->iron_angle = 0x800;
            s->rotate = 0x200;
        }
        else
        {
            s->iron_step = 0x100;
            s->iron_angle = 0;
            s->rotate = 0x600;
        }
        if (!Properties(s, in, rng, e))
            return false;
        s->hitbox_width = s->hitbox_height = 0x10;
        s->iron_delay = 0x20;
        e->sounds[0] = 0x69d;
        s->phase = 1;
        break;
    }
    default:
        s->spell_bright = Word(s->spell_bright - 8);
        if (s->spell_bright < 0)
            s->spell_bright = 0;
        if (s->iron_delay)
        {
            if (--s->iron_delay == 0)
            {
                s->flags |= 0x08000000U;
                s->flags &= ~0x04000000U;
            }
        }
        else
        {
            if (s->iron_radius < 0x280)
                s->iron_radius = Word(s->iron_radius + 0x18);
            s->rotate = Word(s->rotate + s->iron_step * 7 / 4);
            const Sint16 previous = s->iron_angle;
            s->iron_angle = Word(s->iron_angle + s->iron_step);
            if (((previous ^ s->iron_angle) & 0x1000) && (s->parameter & 0x100U))
                e->sounds[0] = 0x689;
            s->iron_cx = (Sint32)((Uint32)s->iron_cx + (Uint32)s->iron_vx);
        }
        break;
    }
    // The FakePrim keeps its centre's whole pixels in x0/y0 (the original rewrites the high
    // words from them before each move).
    s->spell_quads[0].x[0] = Pixel(s->iron_cx);
    s->spell_quads[0].y[0] = Pixel(s->iron_cy);
    SotnMathTrig t = {0};
    if (!SotnMath_Trig(in.executable, s->iron_angle, &t))
        return false;
    const Sint32 sx = (Sint32)((Uint32)s->iron_cx + (Uint32)t.cosine * (Uint32)s->iron_radius),
                 sy = (Sint32)((Uint32)s->iron_cy - (Uint32)t.sine * (Uint32)s->iron_radius);
    s->x = (Sint32)((Uint32)sx + ((Uint32)in.retained_camera_x << 16));
    s->y = (Sint32)((Uint32)sy + ((Uint32)in.retained_camera_y << 16));
    s->iron_count = Word(s->iron_count + 1);
    const Sint16 x = Pixel(sx), y = Pixel(sy);
    Sint16 cx[4] = {0}, cy[4] = {0};
    const Uint16 corner[4] = {0x787, 0x79, 0x879, 0xf87};
    for (Uint16 k = 0; k < 4; ++k)
    {
        SotnMathTrig c = {0};
        if (!SotnMath_Trig(in.executable, (Sint16)((Uint16)s->rotate + corner[k]), &c))
            return false;
        cx[k] = IronSpoke(x, c.cosine, 0x42, false);
        cy[k] = IronSpoke(y, c.sine, 0x42, true);
    }
    SotnPlayerEffectQuad *q = &s->spell_quads[1];
    for (Uint16 k = 0; k < 4; ++k)
    {
        q->x[k] = cx[k];
        q->y[k] = cy[k];
    }
    for (Uint16 i = 0; i < 16; ++i)
    {
        q = &s->spell_quads[2 + i];
        q->x[0] = Word(cx[0] + (cx[1] - cx[0]) * i / 16);
        q->x[1] = Word(cx[0] + (cx[1] - cx[0]) * (i + 1) / 16);
        q->x[2] = Word(cx[2] + (cx[3] - cx[2]) * i / 16);
        q->x[3] = Word(cx[2] + (cx[3] - cx[2]) * (i + 1) / 16);
        q->y[0] = Word(cy[0] + (cy[1] - cy[0]) * i / 16);
        q->y[1] = Word(cy[0] + (cy[1] - cy[0]) * (i + 1) / 16);
        q->y[2] = Word(cy[2] + (cy[3] - cy[2]) * i / 16);
        q->y[3] = Word(cy[2] + (cy[3] - cy[2]) * (i + 1) / 16);
        bool ok = true;
        q->red[0] = q->green[0] = q->blue[0] = q->red[2] = q->green[2] = q->blue[2] =
            IronGlow(in.executable, p->iron_angles2[i], s->spell_bright, &ok);
        if (!ok)
            return false;
        q->red[1] = q->green[1] = q->blue[1] = q->red[3] = q->green[3] = q->blue[3] =
            IronGlow(in.executable, p->iron_angles2[(i + 1) % 16], s->spell_bright, &ok);
        if (!ok)
            return false;
        p->iron_angles2[i] = Word(p->iron_angles2[i] + s->spell_step);
    }
    return true;
}
// w_013 func_ptr_80170004: an afterimage copying the root's frame, palette cycle and rotation,
// fading by eight from 0x80 and retiring below48.
static bool HeavenImage(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in)
{
    if (!s->phase)
    {
        if (!in.parent_exists)
        {
            s->removed = true;
            return true;
        }
        s->role = (Uint8)in.role;
        s->allocated = true;
        s->frame = in.parent_frame;
        s->bank = in.parent_bank;
        s->facing_left = in.parent_facing_left;
        s->texture = in.parent_texture;
        s->priority = (Uint16)(in.priority - 4U);
        s->flags = 0x0c020000U;
        s->palette = in.parent_child_palette;
        s->draw_flags = (Uint8)(in.parent_draw_flags + 8U);
        s->rotate = in.parent_rotate;
        s->opacity = 0x80;
        s->phase = 1;
    }
    if (s->opacity >= 48)
        s->opacity = (Uint8)(s->opacity - 8);
    else
        s->removed = true;
    return true;
}

// w_012 EntityWeaponAttack (blueprint50/52 roots): Vorpal blade's and Crissaegrim's sliding GT4
// slash. The program's slash count (D_8017BB28) picks one of three shapes; the CLUT steps
// through ten fade palettes and retires the slash after the last.
static bool Vorpal(SotnWeaponSecondary *s, SotnHandWeaponProgram *p, SotnWeaponSecondaryInput in,
                   Uint32 *rng, SotnWeaponSecondaryEvents *e)
{
    const Uint16 masked = (Uint16)(s->parameter & 0x7f00U);
    const bool slide = masked == 0x100 || masked == 0x200;
    if (!s->phase)
    {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 0;
        s->allocated = true;
        s->x = in.x;
        s->y = in.y;
        s->facing_left = in.facing_left;
        s->slash_x[0] = s->slash_x[2] = 0xb;
        s->slash_x[1] = s->slash_x[3] = 0x4b;
        s->slash_y[0] = s->slash_y[1] = -0x27;
        s->slash_y[2] = s->slash_y[3] = 0x19;
        s->slash_drop = in.player_crouch_step && in.player_substep != 2 ? 0x18 : 0;
        s->slash_shape = (Sint16)(p->secondary_counter % 3);
        s->slash_spread[0] = s->slash_spread[1] = 0;
        if (s->slash_shape == 1)
        {
            s->slash_step[0] = -2;
            s->slash_step[1] = -1;
            for (Uint16 i = 0; i < 4; ++i)
                s->slash_y[i] = Word(-s->slash_y[i]);
            s->slash_x[2] = Word(s->slash_x[2] - 0x10);
            if (s->slash_drop)
                s->slash_spread[0] = -0x10;
        }
        if (s->slash_shape == 0)
        {
            s->slash_step[0] = 2;
            s->slash_step[1] = 1;
            s->slash_x[0] = Word(s->slash_x[0] - 0x10);
            if (s->slash_drop)
                s->slash_spread[1] = 0x10;
        }
        if (s->slash_shape == 2)
        {
            s->slash_step[0] = s->slash_step[1] = 1;
            s->slash_spread[0] = s->slash_spread[1] = 0x10;
        }
        if (s->facing_left)
            for (Uint16 i = 0; i < 4; ++i)
                s->slash_x[i] = Word(-s->slash_x[i]);
        if (slide)
        {
            s->velocity_x = s->facing_left ? -0x40000 : 0x40000;
            s->x = (Sint32)((Uint32)s->x - (Uint32)s->velocity_x * 4U);
        }
        SotnPlayerEffectQuad q = {.page = 0x19,
                                  .palette = (Uint16)((in.hand ? 0x18 : 0) + 0x110),
                                  .priority = (Uint16)(in.priority + 2U),
                                  .draw_mode = 0x33};
        if (masked == 0x200)
        {
            for (Uint16 i = 0; i < 4; ++i)
                q.red[i] = q.green[i] = q.blue[i] = 0x80;
            q.draw_mode |= 4;
        }
        s->flags = 0x08800000U;
        if (!Properties(s, in, rng, e))
            return false;
        s->hitbox_x = 0x2b;
        s->hitbox_width = 0x1e;
        s->hitbox_y = (Sint16)(s->slash_drop - 6);
        s->hitbox_height = (Uint8)(0x1c - s->slash_drop / 2);
        e->clear_shield = true;
        const Uint8 v = in.hand ? 0x80 : 0;
        q.u[0] = q.u[2] = 0;
        q.u[1] = q.u[3] = 0x40;
        q.v[0] = q.v[1] = v;
        q.v[2] = q.v[3] = (Uint8)(v + 0x40U);
        s->quad = q;
        p->secondary_counter = (Sint32)((Uint32)p->secondary_counter + 1U);
        e->sounds[0] = 0x60b;
        e->startup_ticks = 4;
        s->phase = 1;
    }
    else
    {
        if (masked == 0x100)
            s->velocity_x = Decelerate(s->velocity_x, 0x3000);
        if (masked == 0x200)
            s->velocity_x = Decelerate(s->velocity_x, 0x1000);
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->quad.palette = (Uint16)(s->quad.palette + 1U);
    }
    if (s->quad.palette >= ((s->parameter & 0x8000U) ? 0x132 : 0x11a))
    {
        s->removed = true;
        return true;
    }
    if (slide)
    {
        s->slash_spread[0] = Word(s->slash_spread[0] + s->slash_step[0]);
        s->slash_spread[1] = Word(s->slash_spread[1] + s->slash_step[1]);
        if (masked == 0x200)
        {
            const Sint16 d = s->velocity_x < 0 ? -7 : 7;
            s->slash_x[0] = Word(s->slash_x[0] + d);
            s->slash_x[2] = Word(s->slash_x[2] + d);
        }
    }
    // Corners are offsets from the entity's whole pixels (DRAW_UNK02 follows the camera).
    for (Uint16 i = 0; i < 4; ++i)
    {
        s->quad.x[i] = s->slash_x[i];
        s->quad.y[i] = Word(s->slash_y[i] + s->slash_drop +
                            (i < 2 ? s->slash_spread[0] : -s->slash_spread[1]));
    }
    return true;
}
// w_012 func_ptr_80170004 (blueprint56): the Chakram arcs out with random speeds for
// twenty-eight updates, then steers back with a growing turn limit like Heaven sword's root.
static bool Chakram(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                    SotnWeaponSecondaryEvents *e)
{
    if (!s->phase)
    {
        s->role = 1;
        s->allocated = true;
        s->bank = (Uint16)(0x8010U + in.hand * 2U);
        s->texture = (Uint16)(0x64U + in.hand * 2U);
        s->frame = 3;
        s->facing_left = in.facing_left;
        s->priority = (Uint16)(in.priority - 2U);
        s->palette = 0x100;
        s->flags = 0x0c020000U;
        const Sint32 speed = (Sint32)(((Rand(rng) & 0x3fffU) * 2U) | 0x48000U);
        s->velocity_x = s->facing_left ? -speed : speed;
        s->lifetime = 0x1c;
        const Sint16 offset = Word(Word(Pixel(in.y) - in.camera_y) + in.parent_hitbox_y);
        SetScreen(&s->y, Word(offset - (in.player_crouch_step ? 8 : 0x10)), in.camera_y);
        const Sint16 px = Word(Pixel(in.x) - in.camera_x);
        SetScreen(&s->x, Word(s->facing_left ? px - in.parent_hitbox_x : px + in.parent_hitbox_x),
                  in.camera_x);
        s->accel_x = s->facing_left ? 0x2800 : -0x2800;
        s->velocity_x = (Sint32)((Uint32)s->velocity_x + (Uint32)in.player_velocity_x);
        s->magnitude = 0x80;
        s->velocity_y = (Sint32)(Rand(rng) & 0x7fffU) * 2 + 0x20000;
        s->accel_y = -0x2000;
        if (!Properties(s, in, rng, e))
            return false;
        e->clear_shield = true;
        s->hitbox_width = s->hitbox_height = 4;
        e->use_ticks = 4;
        s->volume = 0x60;
        s->phase = 1;
    }
    else if (s->phase == 1)
    {
        if (!(s->counter & 7))
        {
            e->sounds[0] = 0x60a;
            e->volume_sound = true;
            e->volume = (Uint16)s->volume;
        }
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->velocity_y);
        s->velocity_x = (Sint32)((Uint32)s->velocity_x + (Uint32)(Sint32)Word(s->accel_x));
        s->velocity_y = (Sint32)((Uint32)s->velocity_y + (Uint32)(Sint32)Word(s->accel_y));
        s->accel_y = Word(s->accel_y - 0x1c0);
        if (s->lifetime == 4)
            ++s->frame;
        s->lifetime = (Sint16)(s->lifetime - 1);
        if (!s->lifetime)
        {
            Sint16 heading = 0;
            if (!SotnMath_Atan2Vector(in.executable, (Sint32)(0U - (Uint32)s->velocity_y),
                                      s->velocity_x, &heading))
                return false;
            s->angle = (Sint16)(heading & 0xfff);
            ++s->frame;
            s->phase = 2;
        }
    }
    else
    {
        Sint16 limit = s->magnitude;
        s->magnitude = Word(s->magnitude + 0x10);
        const Sint32 sx = Word(Pixel(s->x) - in.camera_x), sy = Word(Pixel(s->y) - in.camera_y),
                     tx = (Sint32)Word(Pixel(in.x) - in.camera_x) + in.parent_hitbox_x,
                     ty = (Sint32)Word(Pixel(in.y) - in.camera_y) + in.parent_hitbox_y;
        const Sint16 dx = Word(tx - sx < 0 ? sx - tx : tx - sx),
                     dy = Word(ty - sy < 0 ? sy - ty : ty - sy);
        if (dx < (in.player_hitbox_width + s->hitbox_width) >> 1 &&
            dy < (in.player_hitbox_height + s->hitbox_height) >> 1)
        {
            s->removed = true;
            return true;
        }
        Sint16 target = 0;
        if (!SotnMath_Atan2(in.executable, -(Sint32)Word(ty - sy), Word(tx - sx), &target))
            return false;
        target = (Sint16)(target & 0xfff);
        Sint16 angle = (Sint16)(s->angle & 0xfff);
        const Sint16 diff = Word(angle - target < 0 ? target - angle : angle - target);
        if (limit > diff)
            limit = diff;
        if (angle < target)
            angle = Word(diff < 0x800 ? angle + limit : angle - limit);
        else
            angle = Word(diff < 0x800 ? angle - limit : angle + limit);
        s->angle = (Sint16)(angle & 0xfff);
        s->facing_left = false;
        s->draw_flags |= 4;
        s->rotate = Word(0x800 - s->angle);
        SotnMathTrig t = {0};
        if (!SotnMath_Trig(in.executable, s->angle, &t))
            return false;
        s->x = (Sint32)((Uint32)s->x + (Uint32)t.cosine * 64U);
        s->y = (Sint32)((Uint32)s->y - (Uint32)t.sine * 64U);
        if (!(s->counter & 7))
        {
            e->sounds[0] = 0x60a;
            e->volume_sound = true;
            e->volume = (Uint16)s->volume;
            s->volume -= 0xc;
            if (s->volume < 0)
                s->volume = 0;
        }
    }
    if (!(in.game_timer & 3U))
        e->chain = 0x40U | ((Uint32)(in.hand + 1U) << 12);
    s->counter = Word(s->counter + 1);
    return true;
}

static bool Query(SotnWeaponSecondaryInput in, Sint16 x, Sint16 y, SotnCollider *out)
{
    // CheckCollision takes screen pixels and adds the current camera.
    if (in.query)
        return in.query(in.query_context, (Sint32)x + in.camera_x, (Sint32)y + in.camera_y, out);
    return SotnCollision_Query(&in.collision, (Sint32)x + in.camera_x, (Sint32)y + in.camera_y,
                               out);
}
// w_030 func_ptr_80170004: Sword of Dawn's QCF177 circle. One rand picks one or six soldiers
// before the eleven mandatory GT4s; a second picks the first soldier type. It rises for eight
// updates, scans fifteen ground points around the player's birth spot and creates its soldier
// factories there, then shows cosine bands (phases shared by the program), a fading tether to
// the last placement and drives the soldiers' emergence wave before flashing out.
static bool DawnCircle(SotnWeaponSecondary *s, SotnHandWeaponProgram *p,
                       SotnWeaponSecondaryInput in, Uint32 *rng, SotnWeaponSecondaryEvents *e)
{
    const Uint8 v = in.hand ? 0x80 : 0;
    const Uint16 clut = in.hand ? 0x128 : 0x110;
    bool flag = false;
    const Uint8 *order = NULL, *types = NULL;
    if (!View(in.overlay, 0x608, 16, &order) || !View(in.overlay, 0x618, 16, &types))
        return false;
    switch (s->phase)
    {
    case 0: {
        s->dawn_soldiers = (Rand(rng) & 7U) ? 1 : 6;
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 1;
        s->allocated = true;
        SotnPlayerEffectQuad *q = &s->dawn_quads[0];
        *q = (SotnPlayerEffectQuad){0};
        q->u[0] = q->u[2] = 0x70;
        q->u[1] = q->u[3] = 0x7f;
        q->v[0] = q->v[1] = (Uint8)(v + 0x38);
        q->v[2] = q->v[3] = (Uint8)(v + 0x7f);
        for (Uint16 k = 0; k < 4; ++k)
            q->red[k] = q->green[k] = q->blue[k] = 0x80;
        q->palette = clut;
        q->page = 0x19;
        q->priority = (Uint16)(in.priority + 2U);
        q->draw_mode = 0x40 | 0x20 | 0x10 | 0x04 | 0x02 | 0x01;
        for (Uint16 i = 0; i < 10; ++i)
        {
            q = &s->dawn_quads[1 + i];
            *q = (SotnPlayerEffectQuad){0};
            q->u[0] = q->u[2] = 0x70;
            q->u[1] = q->u[3] = 0x7f;
            q->v[0] = q->v[1] = (Uint8)(0x38U + v + i * 8U);
            q->v[2] = q->v[3] = (Uint8)(0x38U + v + (i + 1U) * 8U - 1U);
            q->draw_mode = 0x20 | 0x10 | 0x04 | 0x02 | 0x01;
            q->page = 0x19;
            q->priority = (Uint16)(in.priority + 4U);
            q->palette = 0x15f;
        }
        s->dawn_line = false;
        s->facing_left = in.facing_left;
        s->flags = 0x08800000U;
        s->velocity_x = s->facing_left ? -0x18000 : 0x18000;
        s->dawn_velocity_y = -0x52000;
        s->lifetime = 8;
        s->dawn_half_w = s->dawn_half_h = 0;
        s->dawn_type = types[Rand(rng) & 15U];
        s->dawn_birth_y = Word(Pixel(in.y) - in.camera_y);
        s->dawn_birth_x = Word(Pixel(in.x) - in.camera_x);
        e->use_ticks = 4;
        s->phase = 1;
        break;
    }
    case 1:
        if (++s->dawn_half_w > 8)
            s->dawn_half_w = 8;
        // The original clamps unk80 twice; unk82 grows unclamped.
        s->dawn_half_h = Word(s->dawn_half_h + 4);
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->dawn_velocity_y);
        s->lifetime = Word(s->lifetime - 1);
        if (!s->lifetime)
        {
            Sint32 buffer[16] = {0};
            // The original leaves unscanned points' stack words uninitialized; a sixth soldier
            // with no free point reads point0 (D_8017A608[15] is the zero pad).
            Sint16 xs[16] = {0}, ys[16] = {0};
            Sint16 x = s->dawn_birth_x, y = Word(s->dawn_birth_y + 0x19);
            SotnCollider c = {0};
            for (Uint16 i = 0; i < 15; ++i)
            {
                if (i == 8)
                {
                    x = s->dawn_birth_x;
                    y = Word(s->dawn_birth_y + 0x19);
                }
                if (i >= 8)
                    x = Word(x + (s->facing_left ? 7 : -7));
                if (!Query(in, x, Word(y + 7), &c))
                    return false;
                if (c.effects & SOTN_EFFECT_SOLID)
                {
                    y = Word(y + 7 + c.top);
                    if (!Query(in, x, Word(y - 1), &c))
                        return false;
                    if ((c.effects & (SOTN_EFFECT_SLOPE_FLOOR | SOTN_EFFECT_SOLID)) ==
                        SOTN_EFFECT_SOLID)
                        buffer[i] = 2;
                    else if (c.effects & SOTN_EFFECT_SOLID)
                        y = Word(y + c.top - 1);
                    xs[i] = x;
                    ys[i] = y;
                }
                else
                    buffer[i] = 1;
                if (i < 8)
                    x = Word(x + (s->facing_left ? -7 : 7));
            }
            const Uint16 id = in.hand ? 0xf2 : 0xe2;
            Uint8 free = in.free_factories;
            for (Sint16 j = 0; j < s->dawn_soldiers; ++j)
            {
                Uint16 i = 0;
                for (; i < 15; ++i)
                {
                    if (buffer[order[i]])
                        continue;
                    Uint16 k = 0;
                    for (; k < 15; ++k)
                    {
                        const Sint32 d = (Sint32)in.slot_x[k] - xs[order[i]];
                        if (in.slot_kind[k] == id && (d < 0 ? -d : d) < 3)
                            break;
                    }
                    if (k == 15)
                        break;
                }
                const Uint32 param = (Uint16)(s->dawn_type + (s->dawn_soldiers == 6 ? 0x10 : 0));
                if (!free)
                    continue;
                --free;
                const Uint8 at = order[i];
                if (e->factory_count >= SDL_arraysize(e->factories))
                    return SDL_SetError("Sword of Dawn exceeded its original soldier requests");
                e->factories[e->factory_count] = (0x3eU + ((in.hand + 1U) << 12)) | param << 16;
                s->dawn_target = 1;
                s->dawn_target_x = xs[at];
                s->dawn_target_y = Word(ys[at] - 0x19);
                e->factory_x[e->factory_count] =
                    (Sint32)(((Uint32)(Uint16)Word(xs[at] + in.camera_x) << 16) |
                             ((Uint32)s->x & 0xffffU));
                e->factory_y[e->factory_count] =
                    (Sint32)(((Uint32)(Uint16)Word(s->dawn_target_y + in.camera_y) << 16) |
                             ((Uint32)s->y & 0xffffU));
                ++e->factory_count;
                buffer[at] = 1;
                s->dawn_type = (Sint16)((s->dawn_type + 1) % 6);
            }
            s->lifetime = 8;
            e->sounds[0] = 0x635;
            e->volume_sound = true;
            e->volume = 0x58;
            s->phase = 2;
        }
        break;
    case 2:
        s->lifetime = Word(s->lifetime - 1);
        if (!s->lifetime)
        {
            for (Uint16 i = 0; i < 10; ++i)
                p->dawn_phase[i] = (Sint16)(0x199 * i);
            s->dawn_fade = 0xff;
            s->dawn_count = 0;
            s->phase = 3;
        }
        break;
    case 3:
        s->dawn_band = Word(s->dawn_band + 4);
        if (s->dawn_band > 0x120)
            s->dawn_band = 0x120;
        flag = true;
        s->dawn_count = Word(s->dawn_count + 1);
        s->dawn_fade = Word(s->dawn_fade - 8);
        if (s->dawn_fade < 0)
            s->dawn_fade = 0;
        if (s->dawn_count >= 10)
            s->dawn_wave = (Sint16)(1 + (s->dawn_count - 10) / 4);
        if (s->dawn_wave >= 7 && s->dawn_band == 0x120)
            s->phase = 4;
        break;
    default: {
        SotnPlayerEffectQuad *q = &s->dawn_quads[0];
        q->green[3] = (Uint8)(q->green[3] - 6);
        if (q->green[3] < 10)
            q->green[3] = 10;
        q->red[3] = (Uint8)(q->red[3] + 6);
        if (q->red[3] > 240)
            q->red[3] = 240;
        q->red[0] = q->red[1] = q->red[2] = q->red[3];
        q->blue[0] = q->blue[1] = q->blue[2] = q->blue[3] = q->green[0] = q->green[1] =
            q->green[2] = q->green[3];
        if (in.game_timer & 1U)
            q->draw_mode |= 8;
        else
            q->draw_mode = (Uint16)(q->draw_mode & 0xfff7U);
        s->dawn_band = Word(s->dawn_band - 8);
        if (s->dawn_band < 0)
        {
            s->removed = true;
            return true;
        }
        break;
    }
    }
    SotnPlayerEffectQuad *q = &s->dawn_quads[0];
    q->x[0] = q->x[2] = Word(-s->dawn_half_w);
    q->x[1] = q->x[3] = s->dawn_half_w;
    q->y[0] = q->y[1] = Word(-4 - s->dawn_half_h);
    q->y[2] = q->y[3] = Word(4 + s->dawn_half_h);
    for (Uint16 i = 0; i < 9; ++i)
    {
        q = &s->dawn_quads[1 + i];
        q->x[0] = q->x[2] = -8;
        q->x[1] = q->x[3] = 8;
        q->y[0] = q->y[1] = (Sint16)(i * 8 - 0x24);
        q->y[2] = q->y[3] = (Sint16)((i + 1) * 8 - 0x24);
        SotnMathTrig a = {0}, b = {0};
        if (!SotnMath_Trig(in.executable, p->dawn_phase[i], &a) ||
            !SotnMath_Trig(in.executable, p->dawn_phase[i + 1], &b))
            return false;
        q->red[0] = q->red[1] = (Uint8)((((a.cosine + 0x1000) >> 6) * s->dawn_band) / 0x100);
        q->red[2] = q->red[3] = (Uint8)((((b.cosine + 0x1000) >> 6) * s->dawn_band) / 0x100);
        p->dawn_phase[i] = Word(p->dawn_phase[i] + 0x80);
    }
    if (flag && s->dawn_target)
    {
        const Sint16 x = Word(Pixel(s->x) - in.camera_x), y = Word(Pixel(s->y) - in.camera_y);
        const Sint32 t = s->dawn_count > 10 ? 10 : s->dawn_count;
        q = &s->dawn_quads[10];
        q->x[0] = q->y[0] = 0;
        q->x[1] = Word(((Sint32)s->dawn_target_x - x) * t / 10);
        q->y[1] = Word((25 + (Sint32)s->dawn_target_y - y) * t / 10);
        q->red[0] = (Uint8)(s->dawn_fade / 2);
        q->red[1] = (Uint8)s->dawn_fade;
        q->priority = (Uint16)(in.priority + 6U);
        q->draw_mode = 0x20 | 0x10 | 0x02 | 0x01;
        s->dawn_line = true;
    }
    return true;
}

// dra/alu_anim.c UpdateAnim over a private AnimationFrame list with frame properties.
static bool DawnAnimate(SotnWeaponSecondary *s, SotnDataView overlay)
{
    const Uint8 *a = NULL, *props = NULL;
    if (s->remaining == -1)
        ;
    else if (s->remaining == 0)
    {
        if (!View(overlay, s->timeline + s->pose * 4U, 4, &a))
            return false;
        s->remaining = (Sint16)Read16(a);
    }
    else if (--s->remaining == 0)
    {
        ++s->pose;
        if (!View(overlay, s->timeline + s->pose * 4U, 4, &a))
            return false;
        const Uint16 duration = Read16(a);
        if (duration == 0)
        {
            s->pose = Read16(a + 2);
            if (!View(overlay, s->timeline + s->pose * 4U, 4, &a))
                return false;
            s->remaining = (Sint16)Read16(a);
        }
        else if (duration == 0xffff)
        {
            --s->pose;
            s->remaining = -1;
        }
        else if (duration == 0xfffe)
            return SDL_SetError("Sword of Dawn soldier animation has no anims table");
        else
            s->remaining = (Sint16)duration;
    }
    if (!View(overlay, s->timeline + s->pose * 4U, 4, &a))
        return false;
    const Uint16 pose = Read16(a + 2);
    if (!View(overlay, 0x554U + ((pose >> 9) & 0x7fU) * 4U, 4, &props))
        return false;
    s->hitbox_x = (Sint8)props[0];
    s->hitbox_y = (Sint8)props[1];
    s->hitbox_width = props[2];
    s->hitbox_height = props[3];
    s->frame = pose & 0x1ffU;
    return true;
}
static void DawnFactory(SotnWeaponSecondary *s, SotnWeaponSecondaryEvents *e, Uint8 *free,
                        Uint32 packed)
{
    // CreateEntFactoryFromEntity returns NULL beyond the free factory entities.
    if (!*free || e->factory_count >= SDL_arraysize(e->factories))
        return;
    --*free;
    e->factories[e->factory_count] = packed;
    e->factory_x[e->factory_count] = s->x;
    e->factory_y[e->factory_count] = s->y;
    ++e->factory_count;
}
static Sint32 DawnSlope(Sint32 v, Sint32 n)
{
    const Sint32 m = (Sint32)((Uint32)v * (Uint32)n);
    return m < 0 ? Asr(m + 0xf, 4) : Asr(m, 4);
}
// w_030 func_ptr_80170008: one summoned soldier (type0..5 from params bits8..11, bit12 the
// six-soldier palette). It waits for its circle's wave, rolls once, takes fourteen GT4s (rise
// mask and smoke), shakes the camera and rises; then it walks (types0/1), shoots arrows
// (2/3), throws homing swords (4) or a bomb (5) and blinks out. Hits blink it away (type1 has
// three lives). Rows are world pixels: the original DRAW_UNK02 screen vertices follow the camera.
static bool DawnSoldier(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                        SotnWeaponSecondaryEvents *e)
{
    const Uint16 type = (s->parameter & 0xf00U) >> 8;
    const bool alt = (s->parameter & 0x1000U) != 0;
    bool stop = false;
    Uint8 free = in.free_factories;
    const Sint16 sx = Word(Pixel(s->x) - in.camera_x);
    const Uint8 *table = NULL;
    if (type > 5 || !View(in.overlay, 0x628, 24, &table))
        return SDL_SetError("Sword of Dawn soldier requires its actual type table");
    if (s->phase < 7 && s->hit_flags)
    {
        s->hit_flags = 0;
        s->dawn_blink = 0x18;
        s->phase = 7;
        if ((s->dawn_lives = Word(s->dawn_lives - 1)) > 0)
            s->phase = 8;
    }
    switch (s->phase)
    {
    case 0: {
        if (in.parent_wave != type + 1)
            return true;
        const Uint16 random = Rand(rng) & 1U;
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        const Uint8 *add = NULL, *alts = NULL, *shots = NULL;
        if (!View(in.overlay, 0x640 + type, 1, &add) ||
            !View(in.overlay, 0x660 + type * 2U, 2, &alts) ||
            !View(in.overlay, 0x650 + type, 1, &shots))
            return false;
        s->role = 2;
        s->allocated = true;
        Uint8 left = 0;
        Uint16 clut = 0x110;
        s->palette = (Uint16)(add[0] + (random | 0x8110U));
        if (alt)
            s->palette = Read16(alts);
        s->texture = 0x64;
        if (in.hand)
        {
            left = 0x80;
            clut = 0x128;
            s->palette = (Uint16)(s->palette + 0x18U);
            s->texture = (Uint16)(s->texture + 2U);
        }
        SotnPlayerEffectQuad *q = &s->dawn_quads[0];
        *q = (SotnPlayerEffectQuad){0};
        q->u[0] = q->u[2] = 0xe0;
        q->u[1] = q->u[3] = 0xff;
        q->v[0] = q->v[1] = left;
        q->v[2] = q->v[3] = (Uint8)(left + 0x5f);
        q->x[0] = q->x[2] = Word(Pixel(s->x) - 0x10);
        q->x[1] = q->x[3] = Word(Pixel(s->x) + 0x10);
        q->page = 0x19;
        q->palette = (Uint16)(clut + 2U);
        q->priority = (Uint16)(in.priority + 10U);
        q->draw_mode = 0x02;
        for (Uint16 i = 0; i < 8; ++i)
        {
            q = &s->dawn_quads[1 + i];
            *q = (SotnPlayerEffectQuad){0};
            q->u[0] = q->u[2] = 0xe0;
            q->u[1] = q->u[3] = 0xff;
            q->v[0] = q->v[1] = (Uint8)(left + 0x60);
            q->v[2] = q->v[3] = (Uint8)(left + 0x7f);
            q->page = 0x19;
            q->palette = (Uint16)(clut + 2U);
            q->priority = (Uint16)(in.priority + 12U);
            q->draw_mode = 0x08;
            // Unplaced rows sit at screen zero, which then follows the camera like the world.
            for (Uint16 k = 0; k < 4; ++k)
            {
                q->x[k] = Word(in.camera_x);
                q->y[k] = Word(in.camera_y);
            }
        }
        for (Uint16 i = 9; i < 14; ++i)
            s->dawn_quads[i] = (SotnPlayerEffectQuad){.draw_mode = 0x08};
        s->flags = 0x08800000U;
        s->bank = 10;
        const Uint32 anim = Read32(table + type * 4U);
        if (anim < 0x8017a000U || anim >= 0x8017d000U)
            return SDL_SetError("Sword of Dawn soldier animation points outside its program");
        s->timeline = (Uint16)(anim - 0x8017a000U);
        s->pose = 0;
        s->remaining = 0;
        s->priority = (Uint16)(in.priority + 2U);
        s->facing_left = !in.parent_facing_left;
        s->dawn_shots = (Sint16)(shots[0] + random);
        s->dawn_rise1 = s->dawn_rise2 = -0x1000;
        s->dawn_rate1 = s->dawn_rate2 = -0x100;
        if (type == 1)
            s->dawn_lives = 3;
        e->shake = 4;
        e->sounds[0] = 0x654;
        s->phase = 1;
        break;
    }
    case 1: {
        stop = true;
        s->dawn_rate1 += 0x18;
        s->dawn_rate2 += 0x10;
        if (s->dawn_rate1 > 0x60)
            s->dawn_rate1 = 0x60;
        if (s->dawn_rate2 > 0x80)
            s->dawn_rate2 = 0x80;
        s->dawn_rise1 += s->dawn_rate1;
        s->dawn_rise2 += s->dawn_rate2;
        if (s->dawn_rise1 > 0)
            s->dawn_rise1 = 0;
        if (s->dawn_rise2 > 0)
            s->dawn_rise2 = 0;
        if (!(s->dawn_rise1 | s->dawn_rise2))
        {
            s->properties = (SotnEquipment){.attack = 4,
                                            .element = 0x20,
                                            .hit_type = 2,
                                            .invincibility = 4,
                                            .stun = 4,
                                            .hit_effect = 1};
            s->hitbox_state = 2;
            e->group = true;
            s->y = (Sint32)((Uint32)s->y + 0x10000U * (type == 5 ? 2U : 1U));
            const Uint8 *step = NULL, *speed = NULL;
            if (!View(in.overlay, 0x658 + type, 1, &step) ||
                !View(in.overlay, 0x648 + (type & 1U) * 4U, 4, &speed))
                return false;
            s->phase = step[0];
            if (type < 2)
            {
                const Sint32 v = (Sint32)Read32(speed);
                s->velocity_x = s->facing_left ? -v : v;
            }
        }
        break;
    }
    case 2:
        if (s->remaining == 1 && s->pose == 2)
            e->sounds[0] = 0x6c9;
        if (s->remaining == 1 && s->pose == 5)
        {
            e->sounds[0] = 0x627;
            DawnFactory(s, e, &free, 0x46U + ((in.hand + 1U) << 12) + ((Uint32)(type - 2U) << 16));
        }
        if (s->remaining < 0)
        {
            s->pose = 0;
            s->remaining = 0;
            if ((s->dawn_shots = Word(s->dawn_shots - 1)) == 0)
            {
                s->dawn_blink = 0x18;
                s->phase = 7;
            }
        }
        break;
    case 3:
    case 4: {
        SotnCollider c = {0};
        if (s->phase == 3)
        {
            const Sint32 world = (Sint32)sx + in.camera_x;
            if (world < in.room_left || in.room_right < world)
            {
                s->dawn_blink = 0x18;
                s->hit_flags = 0;
                s->phase = 7;
                return true;
            }
            s->y = (Sint32)((Uint32)s->y + 0x40000U);
            if (!Query(in, sx, Word(Pixel(s->y) - in.camera_y + 0x18), &c))
                return false;
            if (c.effects & SOTN_EFFECT_SOLID)
            {
                s->y = (Sint32)((Uint32)s->y + (Uint32)c.top * 65536U);
                Sint32 v = s->velocity_x;
                const Uint32 effect = c.effects & 0xf000U;
                if (v > 0)
                {
                    if (effect == 0x8000)
                        v = DawnSlope(v, 0xa);
                    if (effect == (0x8000 | 0x1000))
                        v = DawnSlope(v, 0xd);
                }
                else
                {
                    if (effect == (0x8000 | 0x4000))
                        v = DawnSlope(v, 0xa);
                    if (effect == (0x8000 | 0x4000 | 0x1000))
                        v = DawnSlope(v, 0xd);
                }
                s->x = (Sint32)((Uint32)s->x + (Uint32)v);
            }
            else
            {
                s->phase = 4;
                s->dawn_velocity_y = 0;
            }
        }
        else
        {
            s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
            s->y = (Sint32)((Uint32)s->y + (Uint32)s->dawn_velocity_y);
            s->dawn_velocity_y += 0x2800;
            if (s->dawn_velocity_y > 0x80000)
                s->dawn_velocity_y = 0x80000;
            if (!Query(in, Word(Pixel(s->x) - in.camera_x), Word(Pixel(s->y) - in.camera_y + 0x19),
                       &c))
                return false;
            if (c.effects & SOTN_EFFECT_SOLID)
            {
                s->dawn_blink = 0x18;
                s->phase = 3;
            }
        }
        const Sint16 px = Word(Pixel(s->x) - in.camera_x + (s->velocity_x < 0 ? -8 : 8)),
                     py = Word(Pixel(s->y) - in.camera_y);
        if (!Query(in, px, Word(py + 10), &c))
            return false;
        if (c.effects & SOTN_EFFECT_SIDE)
        {
            s->dawn_blink = 0x18;
            s->phase = 7;
        }
        if (!Query(in, px, Word(py - 6), &c))
            return false;
        if (c.effects & SOTN_EFFECT_SIDE)
        {
            s->dawn_blink = 0x18;
            s->phase = 7;
        }
        break;
    }
    case 5:
        if (!s->dawn_cooldown)
        {
            if ((s->dawn_shots = Word(s->dawn_shots - 1)) < 0)
            {
                s->dawn_blink = 0x18;
                s->phase = 7;
                break;
            }
            const Uint16 extra = Rand(rng) & 1U;
            for (Uint16 i = 0; i < extra + 2U; ++i)
                DawnFactory(s, e, &free, 0x46U | ((in.hand + 1U) << 12) | 2U << 16);
            s->dawn_cooldown = (Sint16)((Rand(rng) & 0x1fU) + 0x50);
            if (!s->dawn_shots)
                s->dawn_cooldown = 0x60;
        }
        s->dawn_cooldown = Word(s->dawn_cooldown - 1);
        break;
    case 6:
        if (s->pose == 4 && s->remaining == 1)
            DawnFactory(s, e, &free, 0x6aU | ((in.hand + 1U) << 12));
        if (s->remaining < 0)
        {
            s->dawn_blink = 0x18;
            s->phase = 7;
        }
        break;
    default:
        stop = true;
        s->draw_flags |= 0x80;
        if ((s->dawn_blink = Word(s->dawn_blink - 1)) == 0)
        {
            if (s->phase == 7)
            {
                s->removed = true;
                return true;
            }
            s->phase = 3;
            s->draw_flags &= 0x7f;
        }
        break;
    }
    // Rows: rise mask (row0) and eight smoke puffs, in world pixels.
    const Sint16 ex = Pixel(s->x), ey = Pixel(s->y);
    SotnPlayerEffectQuad *q = &s->dawn_quads[0];
    const Sint32 rise = (in.game_timer & 1U) ? s->dawn_rise1 : s->dawn_rise2;
    q->y[0] = q->y[1] = Word(ey + 0x19 + Asr(rise * 6, 8));
    q->y[2] = q->y[3] = Word(ey + 0x19);
    q->x[0] = Word(ex - 0x10 - (Sint32)(Rand(rng) & 0xfU));
    if (!(in.game_timer & 7U))
    {
        q->x[2] = Word(q->x[2] - 1);
        q->x[3] = Word(q->x[3] + 1);
    }
    for (Uint16 i = 0; i < 8; ++i)
    {
        q = &s->dawn_quads[1 + i];
        if ((in.game_timer & 7U) == i)
        {
            const Sint32 small = Rand(rng) & 7U, large = small + 0xa;
            const Sint32 rx = Rand(rng) % 32U, ry = Rand(rng) % 24U;
            q->x[0] = q->x[2] = Word(ex + rx - 0x1a - small);
            q->y[1] = q->y[0] = Word(ey - ry - large + 0x19);
            q->x[1] = q->x[3] = Word(q->x[0] + large * 2);
            q->y[2] = q->y[3] = Word(q->y[0] + large * 2);
            if (s->dawn_rise1 >= -0x480)
                q->y[0] = q->y[1] = q->y[2] = q->y[3] = Word(0x180 + in.camera_y);
            else
                q->draw_mode = 0x02;
        }
        if (in.game_timer & 1U)
            for (Uint16 k = 0; k < 4; ++k)
                q->y[k] = Word(q->y[k] - 1);
        q->draw_mode = ((i ^ in.game_timer) & 1U) ? 0x02 : 0x08;
    }
    if (!stop && !DawnAnimate(s, in.overlay))
        return false;
    return true;
}

static void DawnAttack(SotnWeaponSecondary *s, Sint16 attack, SotnWeaponSecondaryEvents *e)
{
    s->properties = (SotnEquipment){.attack = attack,
                                    .element = 0x20,
                                    .hit_type = 2,
                                    .invincibility = 4,
                                    .stun = 4,
                                    .hit_effect = 1};
    s->hitbox_state = 2;
    e->group = true;
}
// dra/7879C.c func_80118B18: the heading from a child's screen position to a retained target (or
// the screen-edge point on the side), offset negative when the target is gone, untouchable or
// flagged, or the child is off screen.
static bool TargetHeading(SotnWeaponSecondaryInput in, Sint16 sx, Sint16 sy, bool bound, bool side,
                          Sint16 *out)
{
    Sint16 tx = side ? -32 : 288, ty = 112;
    if (bound)
    {
        tx = in.target.x;
        ty = in.target.y;
    }
    Sint16 angle = 0;
    if (!SotnMath_Atan2(in.executable, -(Sint32)Word(ty - sy), Word(tx - sx), &angle))
        return false;
    angle = (Sint16)(angle & 0xfff);
    Sint32 offset = 0;
    if (bound)
    {
        if (!in.target.identity)
            offset = 0x1000;
        if (!in.target.hitbox)
            offset = 0x2000;
        if (in.target.flags & 0x200000U)
            offset = 0x3000;
    }
    if (sx > 288)
        offset = 0x4000;
    if (sx < -32)
        offset = 0x5000;
    if (sy > 240)
        offset = 0x6000;
    if (sy < 0)
        offset = 0x7000;
    *out = Word(angle - offset);
    return true;
}
static bool DawnHeading(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Sint16 *out)
{
    return TargetHeading(in, Word(Pixel(s->x) - in.camera_x), Word(Pixel(s->y) - in.camera_y),
                         s->dawn_bound, s->dawn_side != 0, out);
}
// w_030 func_ptr_8017000C: archers' arrows (params0/1) fly back at5.5; the sword thrower's
// swords (params2) appear around it, spin, then home on a func_80118970 target. Hits blink
// them out after ten updates.
static bool DawnArrow(SotnWeaponSecondary *s, SotnHandWeaponProgram *p, SotnWeaponSecondaryInput in,
                      Uint32 *rng, SotnWeaponSecondaryEvents *e)
{
    const Uint16 kind = (s->parameter >> 8) & 0x7fU;
    switch (s->phase)
    {
    case 0: {
        s->role = 3;
        s->allocated = true;
        s->palette = (Uint16)(0x110U + in.hand * 0x18U);
        s->texture = (Uint16)(0x64U + in.hand * 2U);
        s->flags = 0x08000000U;
        s->bank = 10;
        s->frame = 0x64;
        s->priority = (Uint16)(in.priority + 2U);
        s->facing_left = in.parent_facing_left;
        if (kind == 1)
            s->y = (Sint32)((Uint32)s->y - 0x40000U);
        if (kind == 0)
            s->y = (Sint32)((Uint32)s->y - 0x100000U);
        if (kind == 2)
        {
            const Uint8 *offsets = NULL;
            if (!View(in.overlay, 0x66c + (Uint32)(p->secondary_counter % 3) * 2U, 2, &offsets))
                return false;
            s->y = (Sint32)((Uint32)s->y - (Uint32)(28 + (Rand(rng) & 15U)) * 65536U);
            s->x = (Sint32)((Uint32)s->x + (Uint32)(Sint32)(Sint16)Read16(offsets) * 65536U);
            s->x = (Sint32)((Uint32)s->x + (Uint32)(-4 + (Sint32)(Rand(rng) & 7U)) * 65536U);
            s->dawn_side = s->facing_left ? 0 : 1;
            s->facing_left = false;
            s->draw_flags = 4;
            s->phase = 2;
            e->sounds[0] = 0x6ba;
            s->dawn_angle = (Sint16)Rand(rng);
            s->lifetime = (Sint16)((s->parameter & 0xffU) * 4U + 0x30U);
            p->secondary_counter = (Sint32)((Uint32)p->secondary_counter + 1U);
        }
        else
        {
            s->velocity_x = s->facing_left ? 0x58000 : -0x58000;
            s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x * 5U);
        }
        DawnAttack(s, 4, e);
        s->hitbox_width = 8;
        s->hitbox_height = 4;
        ++s->phase;
        break;
    }
    case 1:
        if (kind == 2)
        {
            Sint16 result = 0;
            if (!DawnHeading(s, in, &result))
                return false;
            if (result >= 0)
            {
                Sint16 angle = (Sint16)(s->dawn_angle & 0xfff);
                const Sint16 diff = Word(angle - result < 0 ? result - angle : angle - result);
                if (s->dawn_limit > diff)
                    s->dawn_limit = diff;
                if (angle < result)
                    angle = Word(diff < 0x800 ? angle + s->dawn_limit : angle - s->dawn_limit);
                else
                    angle = Word(diff < 0x800 ? angle - s->dawn_limit : angle + s->dawn_limit);
                s->dawn_angle = (Sint16)(angle & 0xfff);
            }
            SotnMathTrig t = {0};
            if (!SotnMath_Trig(in.executable, s->dawn_angle, &t))
                return false;
            s->velocity_x = Asr(t.cosine * s->dawn_speed, 5);
            s->dawn_limit = Word(s->dawn_limit + 0x18);
            s->rotate = Word(-s->dawn_angle + 0x800);
            s->dawn_velocity_y = -Asr(t.sine * s->dawn_speed, 5);
        }
        if (s->hit_flags)
        {
            s->lifetime = 10;
            ++s->phase;
        }
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->dawn_velocity_y);
        return true;
    case 2:
        s->draw_flags |= 0x80;
        if ((s->lifetime = Word(s->lifetime - 1)) == 0)
            s->removed = true;
        return true;
    default:
        s->dawn_angle = Word(s->dawn_angle + 0x100);
        s->rotate = Word(-s->dawn_angle + 0x800);
        if ((s->lifetime = Word(s->lifetime - 1)) == 0)
        {
            e->select_target = true;
            s->dawn_limit = 0x20;
            s->dawn_speed = 0x600;
            e->sounds[0] = 0x627;
            s->phase = 1;
        }
        break;
    }
    return true;
}
// w_030 func_ptr_80170010: the bomber's two blueprint0x6A bombs (DRA bank9) grow and spin with a
// rising EXPLODE_FAST_A until the bomber's pose0x20; index0 then bursts, index1 rolls away
// subtractively, scattering FACTORY(0x24,1) sparks, and shrinks after a hit. Each retires when
// its raw parent slot empties.
static bool DawnBomb(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                     SotnWeaponSecondaryEvents *e)
{
    if (!in.parent_exists)
    {
        s->removed = true;
        return true;
    }
    const bool roller = (s->parameter & 0xffU) != 0;
    switch (s->phase)
    {
    case 0:
        s->role = 4;
        s->allocated = true;
        s->bank = 9;
        s->flags = 0x08100000U;
        s->draw_flags = 7;
        s->scale_x = s->scale_y = 0x20;
        s->timeline = roller ? 0x548 : 0x514;
        s->pose = 0;
        s->remaining = 0;
        s->priority = (Uint16)(in.priority + 8U);
        s->facing_left = !in.parent_facing_left;
        s->velocity_x = s->facing_left ? -0x38000 : 0x38000;
        DawnAttack(s, 0x14, e);
        s->y = (Sint32)((Uint32)s->y - 0x270000U);
        s->dawn_volume = 0x30;
        s->phase = 1;
        break;
    case 1:
        if (!(in.game_timer & 7U))
        {
            e->sounds[0] = 0x652;
            e->volume_sound = true;
            e->volume = (Uint16)s->dawn_volume;
            s->dawn_volume = Word(s->dawn_volume + 8);
            if (s->dawn_volume > 0x7f)
                s->dawn_volume = 0x7f;
        }
        s->rotate = Word(s->rotate + 0x100);
        s->scale_x = Word(s->scale_x + 2);
        if (s->scale_x > 0x100)
            s->scale_x = 0x100;
        s->scale_y = s->scale_x;
        if (in.parent_pose == 0x20 && s->remaining == 1)
        {
            e->sounds[1] = 0x65e;
            if (roller)
            {
                s->blend = 0x50;
                s->phase = 2;
                break;
            }
            s->removed = true;
            return true;
        }
        break;
    case 2:
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        if (!(in.game_timer & 3U))
            s->rotate = Word(s->rotate + 0x400);
        if (!(in.game_timer & 1U) && (Rand(rng) & 1U))
        {
            Uint8 free = in.free_factories;
            DawnFactory(s, e, &free, 0x24U | 1U << 16);
        }
        if (s->hit_flags)
            s->phase = 3;
        break;
    default:
        if (s->phase == 3)
        {
            s->scale_x = Word(s->scale_x - 16);
            s->scale_y = s->scale_x;
            if (s->scale_x < 0x40)
            {
                s->hitbox_state = 0;
                s->bank = 0;
                s->lifetime = 0x80;
                s->phase = 4;
            }
        }
        if ((s->lifetime = Word(s->lifetime - 1)) == 0)
        {
            s->removed = true;
            return true;
        }
        break;
    }
    const Uint8 size = (Uint8)((s->scale_x >> 6) * 3);
    s->hitbox_width = s->hitbox_height = size;
    return true;
}

static void SpellColors(SotnPlayerEffectQuad *q, Uint8 v)
{
    for (Uint16 k = 0; k < 4; ++k)
        q->red[k] = q->green[k] = q->blue[k] = v;
}
// w_008/w_009 EntityWeaponShieldSpell: Leather (item203) and Knight (item204) shields'
// both-buttons spells (blueprint86 E8). Their mandatory GT4s (eleven/twelve) precede everything.
// Each retires a raised shield, plays ANIME_SWORD_C and enters the spell freeze (unk28=1); the
// shield image grows while drifting back and up, flashes out, and a card of wave bands opens on
// the program's retained angle table, releases the FACTORY(55,8|7) pink effect and fades before
// the freeze ends. Leather slices nine vertical bands; Knight ten horizontal ones with their own
// colour phases. Rows are screen pixels.
static bool ShieldCardSpell(SotnWeaponSecondary *s, SotnHandWeaponProgram *p,
                            SotnWeaponSecondaryInput in, SotnWeaponSecondaryEvents *e)
{
    const bool knight = in.family == 9;
    const Uint16 bands = knight ? 10 : 9;
    if (!p->shield_angles_loaded)
    {
        const Uint8 *a = NULL;
        if (!View(in.overlay, 0xa78, 64, &a))
            return false;
        for (Uint16 i = 0; i < 16; ++i)
            p->shield_angles[i] = (Sint32)Read32(a + i * 4U);
        p->shield_angles_loaded = true;
    }
    switch (s->phase)
    {
    case 0: {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 8;
        s->allocated = true;
        s->bank = in.hand ? 0x8012 : 0x8010;
        s->palette = in.hand ? 0x128 : 0x110;
        s->spell_child_palette = in.hand ? 0x129 : 0x111;
        s->texture = in.hand ? 0x66 : 0x64;
        s->spell_v0 = in.hand ? 0 : 0x80;
        s->spell_v1 = in.hand ? 0x80 : 0;
        s->y = (Sint32)((Uint32)s->y - 0x80000U);
        s->flags = 0x04810000U;
        s->priority = (Uint16)(in.priority - 2U);
        s->facing_left = in.facing_left;
        s->frame = 0x3e;
        s->draw_flags = 3;
        s->scale_x = s->scale_y = 0;
        SotnPlayerEffectQuad *q = &s->spell_quads[0];
        *q = (SotnPlayerEffectQuad){.palette = 0x19f, .page = 0x19};
        q->u[0] = q->u[2] = 0x80;
        q->u[1] = q->u[3] = 0x80 + 0x4f;
        q->v[0] = q->v[1] = s->spell_v0;
        q->v[2] = q->v[3] = (Uint8)(s->spell_v0 + 0x4f);
        s->spell_glow = 0;
        SpellColors(q, 0x80);
        q->priority = (Uint16)(s->priority - 4U);
        q->draw_mode = 0x40 | 0x20 | 0x10 | 0x04 | 0x01;
        q = &s->spell_quads[1];
        *q = (SotnPlayerEffectQuad){.palette = s->spell_child_palette, .page = 0x19};
        q->u[0] = q->u[2] = 0;
        q->u[1] = q->u[3] = knight ? 0x5f : 0x47;
        q->v[0] = q->v[1] = (Uint8)(s->spell_v1 + 0x30);
        q->v[2] = q->v[3] = (Uint8)(s->spell_v1 + (knight ? 0x7f : 0x67));
        s->spell_half_w = s->spell_half_h = 0;
        SpellColors(q, 0x80);
        q->priority = 0x1bc;
        q->draw_mode = 0x08;
        for (Uint16 i = 0; i < bands; ++i)
        {
            q = &s->spell_quads[2 + i];
            *q = s->spell_quads[1 + i];
            if (knight)
            {
                q->v[0] = q->v[1] = (Uint8)(s->spell_v1 + 0x30 + i * 80 / 10);
                q->v[2] = q->v[3] = (Uint8)(s->spell_v1 + 0x30 + (i + 1) * 80 / 10 - 1);
            }
            else
            {
                q->u[0] = q->u[2] = (Uint8)(i * 72 / 9);
                q->u[1] = q->u[3] = (Uint8)((i + 1) * 72 / 9 - 1);
            }
            q->palette = 0x15f;
            q->priority = 0x1be;
            q->draw_mode |= 0x20 | 0x10 | 0x04 | 0x01;
        }
        s->spell_wave_x = s->spell_wave_y = 0;
        s->spell_step = 0x40;
        s->spell_bright = 0x100;
        s->velocity_x = s->facing_left ? 0x20000 : -0x20000;
        s->spell_velocity_y = 0;
        e->clear_shield = true;
        e->sounds[0] = 0x641;
        e->freeze_set = true;
        s->phase = 1;
        break;
    }
    case 1:
        s->velocity_x = s->velocity_x < 0 ? SDL_min(s->velocity_x + 0x1000, 0)
                                          : SDL_max(s->velocity_x - 0x1000, 0);
        s->spell_velocity_y -= 0x2800;
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->spell_velocity_y);
        s->scale_x = Word(s->scale_x + 12);
        s->scale_y = s->scale_x;
        s->spell_glow = (Sint16)(s->scale_x * 0x28 / 256);
        if (s->scale_x >= 0x100)
        {
            s->spell_glow = 0x28;
            s->scale_y = s->scale_x = 0x100;
            s->spell_timer = 8;
            s->phase = 2;
        }
        break;
    case 2:
        if (--s->spell_timer == 0)
        {
            s->spell_timer = 0x10;
            s->phase = 3;
        }
        break;
    case 3:
        s->spell_quads[0].priority = !(s->spell_timer & 1) ? 0x1bc : (Uint16)(s->priority - 2U);
        s->scale_x = Word(s->scale_x - 0x10);
        if (s->scale_x <= 0)
            s->scale_x = 0;
        s->scale_y = s->scale_x;
        if (--s->spell_timer == 0)
        {
            s->frame = 0;
            for (Uint16 i = 1; i < 2U + bands; ++i)
                s->spell_quads[i].draw_mode &= 0xfff7U;
            s->spell_timer = 0x20;
            e->sounds[0] = 0x6b1;
            s->phase = 4;
        }
        break;
    case 4: {
        s->spell_glow = Word(s->spell_glow + 8);
        s->scale_x = Word(s->scale_x + 16);
        if (s->scale_x >= 0x100)
            s->scale_x = 0x100;
        s->spell_half_w = (Sint16)(s->scale_x * (knight ? 48 : 36) / 256);
        s->spell_half_h = (Sint16)(s->scale_x * (knight ? 40 : 28) / 256);
        SotnPlayerEffectQuad *q = &s->spell_quads[0];
        if (q->blue[3] > 8)
            q->blue[3] = (Uint8)(q->blue[3] - 4);
        SpellColors(q, q->blue[3]);
        if (--s->spell_timer == 0)
            s->phase = 5;
        break;
    }
    case 5:
        s->spell_bright = Word(s->spell_bright - 4);
        if (s->spell_bright < 0)
        {
            s->spell_bright = 0;
            s->phase = 6;
        }
        break;
    case 6:
        s->spell_bright = Word(s->spell_bright + 0x10);
        if (s->spell_bright > 0x120)
        {
            s->spell_quads[1].draw_mode |= 8;
            e->pink = knight ? 7 : 8;
            s->phase = 7;
        }
        break;
    default:
        if (knight)
        {
            s->spell_bright = Word(s->spell_bright - 8);
            if (s->spell_half_h > 2)
                s->spell_half_h = Word(s->spell_half_h - 1);
            s->spell_wave_x = Word(s->spell_wave_x + 6);
            s->spell_step = Word(s->spell_step + 4);
            if (s->spell_bright < 4)
            {
                e->freeze_clear = true;
                s->removed = true;
                return true;
            }
            break;
        }
        if (s->spell_half_w > 2)
            s->spell_half_h = Word(s->spell_half_h - 2);
        if (s->spell_bright > 8)
            s->spell_bright = Word(s->spell_bright - 8);
        s->spell_wave_y = Word(s->spell_wave_y + 6);
        if (s->spell_wave_y > 0xc0)
        {
            e->freeze_clear = true;
            s->removed = true;
            return true;
        }
        break;
    }
    const Sint16 x = Word(Pixel(s->x) - in.camera_x), y = Word(Pixel(s->y) - in.camera_y);
    SotnPlayerEffectQuad *q = &s->spell_quads[0];
    if (q->blue[3] > 8)
    {
        if (in.engine_timer & 1U)
            q->draw_mode |= 8;
        else
            q->draw_mode &= 0xfff7U;
    }
    q->x[0] = q->x[2] = Word(x - s->spell_glow);
    q->x[1] = q->x[3] = Word(x + s->spell_glow - 1);
    q->y[0] = q->y[1] = Word(y - s->spell_glow);
    q->y[2] = q->y[3] = Word(y + s->spell_glow - 1);
    q = &s->spell_quads[1];
    Sint16 top = q->y[0] = q->y[1] = Word(y - s->spell_half_h);
    Sint16 bottom = q->y[2] = q->y[3] = Word(y + s->spell_half_h - 1);
    Sint16 back = 0, front = 0;
    if (!s->facing_left)
    {
        back = q->x[0] = q->x[2] = Word(x - s->spell_half_w);
        front = q->x[1] = q->x[3] = Word(x + s->spell_half_w - 1);
    }
    else
    {
        front = q->x[1] = q->x[3] = Word(x - s->spell_half_w);
        back = q->x[0] = q->x[2] = Word(x + s->spell_half_w - 1);
    }
    front = Word(front + 1);
    bottom = Word(bottom + 1);
    for (Uint16 i = 0; knight && i < 10; ++i)
    {
        q = &s->spell_quads[2 + i];
        Sint32 sine[10] = {0};
        const Uint16 at[10] = {i,
                               (Uint16)((i + 1) % 10),
                               (Uint16)((i + 6) % 10),
                               (Uint16)((i + 7) % 10),
                               (Uint16)((i + 4) % 10),
                               (Uint16)((i + 8) % 10),
                               (Uint16)((i + 5) % 10),
                               (Uint16)((i + 9) % 10),
                               0,
                               0};
        for (Uint16 k = 0; k < 8; ++k)
        {
            SotnMathTrig t = {0};
            if (!SotnMath_Trig(in.executable, (Sint16)p->shield_angles[at[k]], &t))
                return false;
            sine[k] = t.sine;
        }
        Sint16 base = (Sint16)(Asr(sine[0], 5) * s->spell_wave_x / 256);
        q->x[0] = Word(back + base);
        q->x[1] = Word(front + base - 1);
        base = (Sint16)(Asr(sine[1], 5) * s->spell_wave_x / 256);
        q->x[2] = Word(back + base);
        q->x[3] = Word(front + base - 1);
        q->y[0] = q->y[1] = Word(top + ((bottom - top) * i) / 10);
        q->y[2] = q->y[3] = Word(top + ((bottom - top) * (i + 1)) / 10);
        base = (Sint16)(Asr(sine[2], 5) * s->spell_wave_y / 256);
        q->y[0] = Word(q->y[0] + base);
        q->y[2] = Word(q->y[2] + base);
        base = (Sint16)(Asr(sine[3], 5) * s->spell_wave_y / 256);
        q->y[1] = Word(q->y[1] + base);
        q->y[3] = Word(q->y[3] + base);
        q->red[0] = q->red[1] = (Uint8)(((sine[0] + 0x1000) >> 6) * s->spell_bright / 256);
        q->green[0] = q->green[1] = (Uint8)(((sine[4] + 0x1000) >> 6) * s->spell_bright / 256);
        q->blue[0] = q->blue[1] = (Uint8)(((sine[5] + 0x1000) >> 6) * s->spell_bright / 256);
        q->red[2] = q->red[3] = (Uint8)(((sine[1] + 0x1000) >> 6) * s->spell_bright / 256);
        q->green[2] = q->green[3] = (Uint8)(((sine[6] + 0x1000) >> 6) * s->spell_bright / 256);
        q->blue[2] = q->blue[3] = (Uint8)(((sine[7] + 0x1000) >> 6) * s->spell_bright / 256);
        p->shield_angles[i] = (Sint32)((Uint32)p->shield_angles[i] + (Uint32)s->spell_step);
    }
    for (Uint16 i = 0; !knight && i < 9; ++i)
    {
        q = &s->spell_quads[2 + i];
        SotnMathTrig t = {0};
        const Sint16 a[5] = {(Sint16)p->shield_angles[i], (Sint16)p->shield_angles[(i + 1) % 9],
                             (Sint16)p->shield_angles[(i + 6) % 9],
                             (Sint16)p->shield_angles[(i + 7) % 9], 0};
        Sint32 sine[4] = {0};
        for (Uint16 k = 0; k < 4; ++k)
        {
            if (!SotnMath_Trig(in.executable, a[k], &t))
                return false;
            sine[k] = t.sine;
        }
        Sint16 base = (Sint16)(Asr(sine[0], 5) * s->spell_wave_y / 256);
        q->y[0] = Word(top + base);
        q->y[2] = Word(bottom + base - 1);
        base = (Sint16)(Asr(sine[1], 5) * s->spell_wave_y / 256);
        q->y[1] = Word(top + base);
        q->y[3] = Word(bottom + base - 1);
        q->x[0] = q->x[2] = Word(back + ((front - back) * i) / 9);
        q->x[1] = q->x[3] = Word(back + ((front - back) * (i + 1)) / 9);
        base = (Sint16)(Asr(sine[2], 5) * s->spell_wave_x / 256);
        q->x[0] = Word(q->x[0] + base);
        q->x[1] = Word(q->x[1] + base);
        base = (Sint16)(Asr(sine[3], 5) * s->spell_wave_x / 256);
        q->x[2] = Word(q->x[2] + base);
        q->x[3] = Word(q->x[3] + base);
        const Uint8 c0 = (Uint8)(((sine[0] + 0x1000) >> 6) * s->spell_bright / 256),
                    c1 = (Uint8)(((sine[1] + 0x1000) >> 6) * s->spell_bright / 256);
        q->red[0] = q->green[0] = q->blue[0] = q->red[2] = q->green[2] = q->blue[2] = c0;
        q->red[1] = q->green[1] = q->blue[1] = q->red[3] = q->green[3] = q->blue[3] = c1;
        p->shield_angles[i] = (Sint32)((Uint32)p->shield_angles[i] + (Uint32)s->spell_step);
    }
    return true;
}

// w_058 EntityWeaponShieldSpell: Alucart shield's both-buttons spell (item215, two mandatory
// GT4s). It shares the shield's growth/flash (a longer 0x40 hold and SFX_TRANSFORM), keeps the
// original's unused accumulations (unk86 starts at the palette, unk92 sums it), then shows the
// page-0x1E question mark above the player for 0x28 updates and ends the freeze.
static bool AlucartSpell(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in,
                         SotnWeaponSecondaryEvents *e)
{
    bool question = false;
    switch (s->phase)
    {
    case 0: {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 8;
        s->allocated = true;
        s->bank = in.hand ? 0x8012 : 0x8010;
        s->palette = in.hand ? 0x128 : 0x110;
        s->spell_half_h = in.hand ? 0x129 : 0x111;
        s->texture = in.hand ? 0x66 : 0x64;
        s->spell_v0 = in.hand ? 0 : 0x80;
        s->spell_v1 = in.hand ? 0x80 : 0;
        s->y = (Sint32)((Uint32)s->y - 0x80000U);
        s->flags = 0x04810000U;
        s->priority = (Uint16)(in.priority - 2U);
        s->facing_left = in.facing_left;
        s->frame = 0x3e;
        s->draw_flags = 3;
        s->scale_x = s->scale_y = 0;
        SotnPlayerEffectQuad *q = &s->spell_quads[0];
        *q = (SotnPlayerEffectQuad){.palette = 0x19f, .page = 0x19};
        q->u[0] = q->u[2] = 0x80;
        q->u[1] = q->u[3] = 0x80 + 0x4f;
        q->v[0] = q->v[1] = s->spell_v0;
        q->v[2] = q->v[3] = (Uint8)(s->spell_v0 + 0x4f);
        s->spell_glow = 0;
        SpellColors(q, 0x80);
        q->priority = (Uint16)(s->priority - 4U);
        q->draw_mode = 0x40 | 0x20 | 0x10 | 0x04 | 0x01;
        q = &s->spell_quads[1];
        *q = (SotnPlayerEffectQuad){.palette = 0x190, .page = 0x1e};
        q->u[0] = q->u[2] = 0x78;
        q->u[1] = q->u[3] = 0x78 + 7;
        q->v[0] = q->v[1] = 8;
        q->v[2] = q->v[3] = 8 + 7;
        q->priority = (Uint16)(s->priority + 4U);
        q->draw_mode = 0x08;
        s->velocity_x = s->facing_left ? 0x20000 : -0x20000;
        s->spell_velocity_y = 0;
        e->clear_shield = true;
        e->sounds[0] = 0x641;
        e->freeze_set = true;
        s->phase = 1;
        break;
    }
    case 1:
        s->velocity_x = s->velocity_x < 0 ? SDL_min(s->velocity_x + 0x1000, 0)
                                          : SDL_max(s->velocity_x - 0x1000, 0);
        s->spell_velocity_y -= 0x2800;
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->spell_velocity_y);
        s->scale_x = Word(s->scale_x + 12);
        s->scale_y = s->scale_x;
        s->spell_glow = (Sint16)(s->scale_x * 0x28 / 256);
        if (s->scale_x >= 0x100)
        {
            s->spell_glow = 0x28;
            s->scale_y = s->scale_x = 0x100;
            s->spell_timer = 0x40;
            s->phase = 2;
        }
        break;
    case 2:
        if (--s->spell_timer == 0)
        {
            s->spell_timer = 0x10;
            e->sounds[0] = 0x668;
            s->phase = 3;
        }
        break;
    case 3:
        s->spell_quads[0].priority = !(s->spell_timer & 1) ? 0x1fc : (Uint16)(s->priority - 2U);
        s->scale_x = Word(s->scale_x - 0x10);
        if (s->scale_x <= 0)
            s->scale_x = 0;
        s->scale_y = s->scale_x;
        if (--s->spell_timer == 0)
        {
            s->frame = 0;
            s->spell_timer = 0x20;
            s->phase = 4;
        }
        break;
    case 4: {
        s->spell_glow = Word(s->spell_glow + 8);
        s->scale_x = Word(s->scale_x + 0x10);
        if (s->scale_x >= 0x100)
            s->scale_x = 0x100;
        s->spell_child_palette = (Uint16)(s->scale_x * 36 / 256);
        s->spell_step = (Sint16)(s->scale_x * 28 / 256);
        SotnPlayerEffectQuad *q = &s->spell_quads[0];
        if (q->blue[3] > 8)
            q->blue[3] = (Uint8)(q->blue[3] - 4);
        SpellColors(q, q->blue[3]);
        s->spell_half_h = Word(s->spell_half_h + 4);
        s->spell_wave_x = Word(s->spell_wave_x + s->spell_half_h);
        if (--s->spell_timer == 0)
        {
            s->spell_timer = 0x28;
            s->phase = 5;
        }
        break;
    }
    case 5:
        question = true;
        if (--s->spell_timer == 0)
            s->phase = 6;
        break;
    default:
        e->freeze_clear = true;
        s->removed = true;
        return true;
    }
    const Sint16 x = Word(Pixel(s->x) - in.camera_x), y = Word(Pixel(s->y) - in.camera_y);
    SotnPlayerEffectQuad *q = &s->spell_quads[0];
    if (q->blue[3] > 8)
    {
        if (in.engine_timer & 1U)
            q->draw_mode |= 8;
        else
            q->draw_mode &= 0xfff7U;
    }
    q->x[0] = q->x[2] = Word(x - s->spell_glow);
    q->x[1] = q->x[3] = Word(x + s->spell_glow - 1);
    q->y[0] = q->y[1] = Word(y - s->spell_glow);
    q->y[2] = q->y[3] = Word(y + s->spell_glow - 1);
    q = &s->spell_quads[1];
    const Sint16 px = Word(Pixel(in.x) - in.camera_x), py = Word(Pixel(in.y) - in.camera_y);
    q->x[0] = q->x[2] = Word(in.facing_left ? px + 2 : px - 6);
    q->x[1] = q->x[3] = Word(q->x[0] + 7);
    q->y[0] = q->y[1] = Word(py - 0x22);
    q->y[2] = q->y[3] = Word(q->y[0] + 7);
    q->draw_mode = question ? 0 : 0x08;
    return true;
}

// w_023 EntityWeaponShieldSpell: Herald shield's both-buttons spell (item207, eighteen mandatory
// GT4s). Like the card spells, it retires a raised shield, plays ANIME_SWORD_C, enters the spell
// freeze, grows and flashes out (TRANSFORM_3X); then its image (sideways UVs) opens over sixteen
// white slices, releases FACTORY(94)'s twenty-four rising sprites, dims, requests FACTORY(90)'s
// swirl with pink9 and later pink10, brightens, hides the image and waves the slices apart
// (program angles D_8017AB70) as they fade and the freeze ends. Rows are screen pixels.
static bool HeraldSpell(SotnWeaponSecondary *s, SotnHandWeaponProgram *p,
                        SotnWeaponSecondaryInput in, SotnWeaponSecondaryEvents *e)
{
    if (!p->shield_angles_loaded)
    {
        const Uint8 *a = NULL;
        if (!View(in.overlay, 0xb70, 64, &a))
            return false;
        for (Uint16 i = 0; i < 16; ++i)
            p->shield_angles[i] = (Sint32)Read32(a + i * 4U);
        p->shield_angles_loaded = true;
    }
    switch (s->phase)
    {
    case 0: {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 8;
        s->allocated = true;
        s->bank = in.hand ? 0x8012 : 0x8010;
        s->palette = in.hand ? 0x128 : 0x110;
        s->spell_child_palette = in.hand ? 0x129 : 0x111;
        s->texture = in.hand ? 0x66 : 0x64;
        s->spell_v0 = in.hand ? 0 : 0x80;
        s->spell_v1 = in.hand ? 0x80 : 0;
        s->y = (Sint32)((Uint32)s->y - 0x80000U);
        s->flags = 0x04810000U;
        s->priority = (Uint16)(in.priority - 2U);
        s->facing_left = in.facing_left;
        s->frame = 0x3e;
        s->draw_flags = 3;
        s->scale_x = s->scale_y = 0;
        SotnPlayerEffectQuad *q = &s->spell_quads[0];
        *q = (SotnPlayerEffectQuad){.palette = 0x19f, .page = 0x19};
        q->u[0] = q->u[2] = 0x80;
        q->u[1] = q->u[3] = 0x80 + 0x4f;
        q->v[0] = q->v[1] = s->spell_v0;
        q->v[2] = q->v[3] = (Uint8)(s->spell_v0 + 0x4f);
        s->spell_glow = 0;
        SpellColors(q, 0x80);
        q->priority = (Uint16)(s->priority - 10U);
        q->draw_mode = 0x40 | 0x20 | 0x10 | 0x04 | 0x01;
        q = &s->spell_quads[1];
        *q = (SotnPlayerEffectQuad){.palette = s->spell_child_palette, .page = 0x19};
        q->u[0] = q->u[1] = 0;
        q->u[2] = q->u[3] = 0x7f;
        q->v[3] = q->v[1] = (Uint8)(s->spell_v1 + 0x38);
        q->v[2] = q->v[0] = (Uint8)(s->spell_v1 + 0x7f);
        SpellColors(q, 0x80);
        q->priority = 0x1b8;
        q->draw_mode = 0x08;
        for (Uint16 i = 0; i < 16; ++i)
        {
            q = &s->spell_quads[2 + i];
            *q = s->spell_quads[1 + i];
            q->u[1] = q->u[0] = (Uint8)(i * 96 / 12);
            q->u[2] = q->u[3] = (Uint8)((i + 1) * 96 / 12 - 1);
            q->palette = 0x15f;
            q->priority = 0x1ba;
            q->draw_mode |= 0x20 | 0x10 | 0x04 | 0x01;
        }
        s->spell_step = 0x80;
        s->spell_bright = 0x100;
        s->spell_wave_x = s->spell_wave_y = 0;
        s->spell_half_w = s->spell_half_h = 0;
        s->velocity_x = s->facing_left ? 0x20000 : -0x20000;
        s->spell_velocity_y = 0;
        e->clear_shield = true;
        e->sounds[0] = 0x641;
        e->freeze_set = true;
        s->phase = 1;
        break;
    }
    case 1:
        s->velocity_x = s->velocity_x < 0 ? SDL_min(s->velocity_x + 0x1000, 0)
                                          : SDL_max(s->velocity_x - 0x1000, 0);
        s->spell_velocity_y -= 0x2800;
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->spell_velocity_y);
        s->scale_x = Word(s->scale_x + 12);
        s->scale_y = s->scale_x;
        s->spell_glow = (Sint16)(s->scale_x * 0x28 / 256);
        if (s->scale_x >= 0x100)
        {
            s->spell_glow = 0x28;
            s->scale_y = s->scale_x = 0x100;
            s->spell_timer = 8;
            s->phase = 2;
        }
        break;
    case 2:
        if (--s->spell_timer == 0)
        {
            s->spell_timer = 0x10;
            s->phase = 3;
        }
        break;
    case 3:
        s->spell_quads[0].priority = !(s->spell_timer & 1) ? 0x1b8 : (Uint16)(s->priority - 2U);
        s->scale_x = Word(s->scale_x - 0x10);
        if (s->scale_x <= 0)
            s->scale_x = 0;
        s->scale_y = s->scale_x;
        if (--s->spell_timer == 0)
        {
            s->frame = 0;
            for (Uint16 i = 1; i < 18; ++i)
                s->spell_quads[i].draw_mode &= 0xfff7U;
            s->spell_timer = 0;
            e->sounds[0] = 0x6b1;
            s->phase = 4;
        }
        break;
    case 4: {
        s->spell_glow = Word(s->spell_glow + 8);
        s->scale_x = Word(s->scale_x + 12);
        if (s->scale_x >= 0x100)
            s->scale_x = 0x100;
        s->spell_half_w = (Sint16)(s->scale_x * 36 / 256);
        s->spell_half_h = (Sint16)(s->scale_x * 64 / 256);
        SotnPlayerEffectQuad *q = &s->spell_quads[0];
        if (q->blue[3] > 8)
            q->blue[3] = (Uint8)(q->blue[3] - 4);
        SpellColors(q, q->blue[3]);
        if (s->scale_x == 0x100)
        {
            q->draw_mode |= 8;
            q->blue[3] = 4;
            s->spell_quads[1].draw_mode &= 0xfff7U;
            // FACTORY(94) (func_ptr_80170028's sprites) with the combo's <<14 hand bits; it
            // fails silently without a free factory entity.
            if (in.free_factories > e->factory_count)
            {
                e->factories[e->factory_count] = 94U + ((in.hand + 1U) << 14);
                e->factory_x[e->factory_count] = s->x;
                e->factory_y[e->factory_count++] = s->y;
            }
            s->phase = 5;
        }
        s->spell_timer = Word(s->spell_timer + 1);
        break;
    }
    case 5:
        s->spell_bright = Word(s->spell_bright - 10);
        if (s->spell_bright < 0)
        {
            s->spell_bright = 0;
            s->phase = 6;
            s->spell_timer = 0;
        }
        break;
    case 6:
        // FACTORY(90) (func_ptr_80170024's swirl) then FACTORY(55,9); later FACTORY(55,10).
        if (s->spell_timer == 0x30)
        {
            if (in.free_factories > e->factory_count)
            {
                e->factories[e->factory_count] = 90U + ((in.hand + 1U) << 14);
                e->factory_x[e->factory_count] = s->x;
                e->factory_y[e->factory_count++] = s->y;
            }
            e->pink = 9;
        }
        s->spell_timer = Word(s->spell_timer + 1);
        if (s->spell_timer == 0x70)
        {
            e->pink = 10;
            s->phase = 7;
        }
        break;
    case 7:
        s->spell_bright = Word(s->spell_bright + 6);
        if (s->spell_bright > 0x130)
        {
            s->spell_quads[1].draw_mode |= 8;
            s->phase = 8;
        }
        break;
    default:
        s->spell_bright = Word(s->spell_bright - 10);
        s->spell_wave_x = Word(s->spell_wave_x + 3);
        s->spell_wave_y = Word(s->spell_wave_y + 3);
        if (s->spell_bright < 0)
        {
            e->freeze_clear = true;
            s->removed = true;
            return true;
        }
        break;
    }
    // Screen words: the camera before this pass's retained-screen shift.
    const Sint16 x = Word(Pixel(s->x) - in.retained_camera_x),
                 y = Word(Pixel(s->y) - in.retained_camera_y);
    SotnPlayerEffectQuad *q = &s->spell_quads[0];
    if (q->blue[3] > 8)
    {
        if (in.engine_timer & 1U)
            q->draw_mode |= 8;
        else
            q->draw_mode &= 0xfff7U;
    }
    q->x[0] = q->x[2] = Word(x - s->spell_glow);
    q->x[1] = q->x[3] = Word(x + s->spell_glow - 1);
    q->y[0] = q->y[1] = Word(y - s->spell_glow);
    q->y[2] = q->y[3] = Word(y + s->spell_glow - 1);
    q = &s->spell_quads[1];
    const Sint16 top = q->y[0] = q->y[1] = Word(y - s->spell_half_h);
    Sint16 bottom = q->y[2] = q->y[3] = Word(y + s->spell_half_h - 1);
    Sint16 back = 0, front = 0;
    if (!s->facing_left)
    {
        back = q->x[0] = q->x[2] = Word(x - s->spell_half_w);
        front = q->x[1] = q->x[3] = Word(x + s->spell_half_w - 1);
    }
    else
    {
        front = q->x[1] = q->x[3] = Word(x - s->spell_half_w);
        back = q->x[0] = q->x[2] = Word(x + s->spell_half_w - 1);
    }
    front = Word(front + 1);
    bottom = Word(bottom + 1);
    for (Uint16 i = 0; i < 16; ++i)
    {
        q = &s->spell_quads[2 + i];
        SotnMathTrig at = {0}, next = {0}, wave = {0};
        if (!SotnMath_Trig(in.executable, (Sint16)p->shield_angles[i], &at) ||
            !SotnMath_Trig(in.executable, (Sint16)p->shield_angles[(i + 1) % 16], &next) ||
            !SotnMath_Trig(in.executable, (Sint16)p->shield_angles[(i + 6) % 16], &wave))
            return false;
        Sint16 base = (Sint16)(Asr(at.sine, 5) * s->spell_wave_x / 256);
        q->x[0] = Word(back + base);
        q->x[1] = Word(front + base - 1);
        base = (Sint16)(Asr(next.sine, 5) * s->spell_wave_x / 256);
        q->x[2] = Word(back + base);
        q->x[3] = Word(front + base - 1);
        base = (Sint16)(Asr(wave.sine, 5) * s->spell_wave_y / 256);
        q->y[0] = Word(top + ((bottom - top) * i) / 16 - base);
        q->y[2] = Word(top + ((bottom - top) * (i + 1)) / 16 - base);
        base = (Sint16)(Asr(next.sine, 5) * s->spell_wave_y / 256);
        q->y[1] = Word(top + (i * (bottom - top)) / 16 - base);
        q->y[3] = Word(top + ((bottom - top) * (i + 1)) / 16 - base);
        q->blue[0] = q->blue[1] = (Uint8)(((at.sine + 0x1000) >> 6) * s->spell_bright / 256);
        q->blue[2] = q->blue[3] = (Uint8)(((next.sine + 0x1000) >> 6) * s->spell_bright / 256);
        for (Uint16 k = 0; k < 4; ++k)
            q->red[k] = q->green[k] = 0;
        p->shield_angles[i] = (Sint32)((Uint32)p->shield_angles[i] + (Uint32)s->spell_step);
    }
    return true;
}
// w_023 func_ptr_80170024 (FACTORY(90), role9): a four-GT4 swirl of the shield's art. Birth
// rotates the program's sixteen model vectors (RotMatrix(0x80,0,0xC0)) into overlay BSS; it rises
// 0x48 below its factory, brightens, holds and dims, spinning (RotMatrix(0,unk7E,0), depth0x140,
// H0x200) about its screen position; a face shows only while 0 < otz <= 0x1BE. Screen words.
static bool HeraldSwirl(SotnWeaponSecondary *s, SotnHandWeaponProgram *p,
                        SotnWeaponSecondaryInput in)
{
    switch (s->phase)
    {
    case 0: {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        const Uint8 *uv = NULL, *vectors = NULL;
        if (!View(in.overlay, 0xbb0, 32, &uv) || !View(in.overlay, 0xc50, 64, &vectors))
            return false;
        s->role = 9;
        s->allocated = true;
        s->flags |= 0x04800000U;
        s->spell_timer = 0;
        s->herald_angle = 0;
        s->star_depth = 0x140;
        s->y = (Sint32)((Uint32)s->y + 0x480000U);
        s->spell_velocity_y = -0xe000;
        for (Uint16 i = 0; i < 4; ++i)
        {
            SotnPlayerEffectQuad *q = &s->spell_quads[i];
            *q = (SotnPlayerEffectQuad){.palette = in.hand ? 0x12b : 0x113, .page = 0x19};
            for (Uint16 k = 0; k < 4; ++k)
            {
                q->u[k] = uv[i * 8U + k * 2U];
                q->v[k] = (Uint8)(uv[i * 8U + k * 2U + 1U] + (in.hand ? 0x80 : 0));
            }
            q->priority = 0;
            q->draw_mode = 0x20 | 0x10 | 0x04 | 0x01;
        }
        s->spell_bright = 0;
        const Sint16 axis[3] = {0x80, 0, 0xc0};
        for (Uint16 i = 0; i < 16; ++i)
        {
            const Uint8 *v = NULL;
            if (!View(in.overlay, Read32(vectors + i * 4U) - 0x8017a000U, 6, &v))
                return false;
            const Sint16 model[3] = {(Sint16)Read16(v), (Sint16)Read16(v + 2),
                                     (Sint16)Read16(v + 4)};
            Sint32 out[3] = {0};
            if (!SotnShieldStar_Rotate(in.executable, axis, model, out))
                return false;
            p->herald_vectors[i][0] = Word(out[0]);
            p->herald_vectors[i][1] = Word(out[1] - 0x18);
            p->herald_vectors[i][2] = Word(out[2]);
        }
        s->phase = 1;
        break;
    }
    case 1:
        if (Word(Pixel(s->y) - in.retained_camera_y) < 0x10)
        {
            s->removed = true;
            return true;
        }
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->spell_velocity_y);
        s->spell_bright = Word(s->spell_bright + 10);
        if (s->spell_bright >= 0xf8)
        {
            s->spell_timer = 0x20;
            s->phase = 2;
        }
        break;
    case 2:
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->spell_velocity_y);
        s->spell_timer = Word(s->spell_timer - 1);
        if (!s->spell_timer)
            s->phase = 3;
        break;
    default:
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->spell_velocity_y);
        s->spell_bright = Word(s->spell_bright - 10);
        if (s->spell_bright < 9)
        {
            s->removed = true;
            return true;
        }
        break;
    }
    s->herald_angle = Word(s->herald_angle + 0x40);
    const Sint16 axis[3] = {0, s->herald_angle, 0};
    const Sint32 transfer[3] = {0, 0, s->star_depth};
    for (Uint16 i = 0; i < 4; ++i)
    {
        SotnPlayerEffectQuad *q = &s->spell_quads[i];
        const Sint16 vertices[4][3] = {
            {p->herald_vectors[i * 4U + 0U][0], p->herald_vectors[i * 4U + 0U][1],
             p->herald_vectors[i * 4U + 0U][2]},
            {p->herald_vectors[i * 4U + 1U][0], p->herald_vectors[i * 4U + 1U][1],
             p->herald_vectors[i * 4U + 1U][2]},
            {p->herald_vectors[i * 4U + 2U][0], p->herald_vectors[i * 4U + 2U][1],
             p->herald_vectors[i * 4U + 2U][2]},
            {p->herald_vectors[i * 4U + 3U][0], p->herald_vectors[i * 4U + 3U][1],
             p->herald_vectors[i * 4U + 3U][2]}};
        SotnProjectedPoint point[4] = {{0}};
        Sint32 otz = 0;
        if (!SotnShieldStar_Project(
                in.executable, axis, transfer, Word(Pixel(s->x) - in.retained_camera_x),
                Word(Pixel(s->y) - in.retained_camera_y), 0x200, vertices, point, &otz))
            return false;
        // RotTransPers4 writes vectors 0..3 to x0, x1, x3 and x2.
        const Uint16 to[4] = {0, 1, 3, 2};
        for (Uint16 k = 0; k < 4; ++k)
        {
            q->x[to[k]] = (Sint16)point[k].x;
            q->y[to[k]] = (Sint16)point[k].y;
        }
        q->draw_mode |= 8;
        if (otz > 0 && otz <= 0x1be)
        {
            SpellColors(q, (Uint8)s->spell_bright);
            q->priority = 0x1bc;
            q->draw_mode &= 0xfff7U;
        }
    }
    return true;
}
// w_023 func_ptr_80170028 (FACTORY(94), role10): one of twenty-four sprites of the spell's art.
// Birth copies its raw parent's palette and unk5A, starts D_8017AB58 at a random pose and waits
// params*4 updates (params0 wraps the s16 countdown: 65536 updates) with no animSet; then it
// copies the raw parent's animSet and for 0x40 updates rises, swaying about its birth screen X
// in front of and behind the spell while its scale pulses. Screen words.
static bool HeraldSprite(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng)
{
    switch (s->phase)
    {
    case 0:
        s->role = 10;
        s->allocated = true;
        s->palette = in.parent_palette;
        s->texture = in.parent_texture;
        s->flags |= 0x04100000U;
        s->draw_flags = 3;
        s->scale_x = s->scale_y = 0;
        s->y = (Sint32)((Uint32)s->y + 0x280000U);
        s->spell_velocity_y = -0x18000;
        s->herald_x = Word(Pixel(s->x) - in.retained_camera_x);
        s->pose = (Uint16)(Rand(rng) % 5U);
        s->remaining = 2;
        s->herald_angle = 0x400;
        s->spell_timer = (Sint16)((s->parameter & 0xffU) * 4U);
        s->phase = 1;
        return true;
    case 1:
        s->spell_timer = Word(s->spell_timer - 1);
        if (!s->spell_timer)
        {
            s->bank = in.parent_bank;
            s->spell_timer = 0x40;
            s->phase = 2;
        }
        return true;
    default: {
        s->spell_timer = Word(s->spell_timer - 1);
        if (!s->spell_timer)
        {
            s->removed = true;
            return true;
        }
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->spell_velocity_y);
        s->herald_angle = (Sint16)((s->herald_angle + 0x80) & 0xfff);
        SotnMathTrig t = {0};
        if (!SotnMath_Trig(in.executable, s->herald_angle, &t))
            return false;
        SetScreen(&s->x, Word(s->herald_x + Asr(t.cosine * 2, 8)), in.retained_camera_x);
        s->priority = s->herald_angle < 0x800 ? 0x1b4 : 0x1bc;
        s->scale_x = s->scale_y = Word(-Asr(t.sine, 5) + 0xc0);
        return true;
    }
    }
}
static void DarkTiles(SotnWeaponSecondary *s, Sint16 step, Uint8 limit, bool rise)
{
    for (Uint16 i = 0; i < 2; ++i)
    {
        SotnPlayerEffectQuad *t = &s->spell_quads[i];
        if (rise ? t->blue[0] < limit : t->blue[0] > limit)
            t->blue[0] = (Uint8)(t->blue[0] + step);
        t->red[0] = t->green[0] = t->blue[0];
    }
}
// w_024 EntityWeaponShieldSpell: Dark shield's both-buttons spell (item208, thirty MP, eighteen
// mandatory GT4 rows). Every update it holds the transform lockout (g_Player.unk20) at 8. Rows0/1
// are two screen-fixed subtractive TILEs (0x90x0xE8) that darken the screen; row2 the glow, row3
// the sideways shield image (fixed 0x1C/0x38 half extents) and rows4..17 fourteen white slices.
// It grows, shakes the camera (SHAKE_Y_MEDIUM), flashes out with WEAPON_APPEAR, then, while its
// image opens, requests FACTORY(96)'s rising rocks, ends the freeze and attacks for three updates
// with a 0x7F hitbox (SetWeaponProperties). The slices dim, hold, brighten, hide the image and
// fade as the darkness lifts. Slice colours come from the program's fourteen D_8017AD08 angles.
static bool DarkSpell(SotnWeaponSecondary *s, SotnHandWeaponProgram *p, SotnWeaponSecondaryInput in,
                      Uint32 *rng, SotnWeaponSecondaryEvents *e)
{
    if (!p->shield_angles_loaded)
    {
        const Uint8 *a = NULL;
        if (!View(in.overlay, 0xd08, 64, &a))
            return false;
        for (Uint16 i = 0; i < 16; ++i)
            p->shield_angles[i] = (Sint32)Read32(a + i * 4U);
        p->shield_angles_loaded = true;
    }
    e->lockout = 8;
    switch (s->phase)
    {
    case 0: {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 8;
        s->allocated = true;
        s->bank = in.hand ? 0x8012 : 0x8010;
        s->palette = in.hand ? 0x128 : 0x110;
        s->spell_child_palette = in.hand ? 0x137 : 0x11f;
        s->texture = in.hand ? 0x66 : 0x64;
        s->spell_v0 = in.hand ? 0 : 0x80;
        s->spell_v1 = in.hand ? 0x80 : 0;
        s->y = (Sint32)((Uint32)s->y - 0x80000U);
        s->flags = 0x04810000U;
        s->priority = (Uint16)(in.priority - 2U);
        s->facing_left = in.facing_left;
        s->frame = 0x3e;
        s->draw_flags = 3;
        s->scale_x = s->scale_y = 0;
        for (Uint16 i = 0; i < 2; ++i)
        {
            SotnPlayerEffectQuad *t = &s->spell_quads[i];
            *t = (SotnPlayerEffectQuad){.priority = 0x1b4, .draw_mode = 0x40 | 0x10 | 0x01};
            t->x[0] = (Sint16)(i * 0x90);
            t->u[0] = 0x90;
            t->v[0] = 0xe8;
        }
        SotnPlayerEffectQuad *q = &s->spell_quads[2];
        *q = (SotnPlayerEffectQuad){.palette = 0x19f, .page = 0x19};
        q->u[0] = q->u[2] = 0x80;
        q->u[1] = q->u[3] = 0x80 + 0x4f;
        q->v[0] = q->v[1] = s->spell_v0;
        q->v[2] = q->v[3] = (Uint8)(s->spell_v0 + 0x4f);
        s->spell_glow = 0;
        SpellColors(q, 0x80);
        q->priority = (Uint16)(s->priority - 10U);
        q->draw_mode = 0x40 | 0x20 | 0x10 | 0x04 | 0x01;
        q = &s->spell_quads[3];
        *q = (SotnPlayerEffectQuad){.palette = s->spell_child_palette, .page = 0x19};
        q->u[0] = q->u[1] = 0;
        q->u[2] = q->u[3] = 0x6f;
        q->v[3] = q->v[1] = (Uint8)(s->spell_v1 + 0x30);
        q->v[2] = q->v[0] = (Uint8)(s->spell_v1 + 0x67);
        SpellColors(q, 0x80);
        q->priority = 0x1b8;
        q->draw_mode = 0x08;
        for (Uint16 i = 0; i < 14; ++i)
        {
            q = &s->spell_quads[4 + i];
            *q = s->spell_quads[3 + i];
            q->u[1] = q->u[0] = (Uint8)(i * 8);
            q->u[2] = q->u[3] = (Uint8)((i + 1) * 8 - 1);
            q->palette = 0x15f;
            q->priority = 0x1ba;
            q->draw_mode |= 0x20 | 0x10 | 0x04 | 0x01;
        }
        s->spell_step = 0x80;
        s->spell_bright = 0x100;
        s->spell_half_w = 0x1c;
        s->spell_half_h = 0x38;
        s->spell_wave_x = 0x20;
        s->spell_wave_y = 0;
        s->velocity_x = s->facing_left ? 0x20000 : -0x20000;
        s->spell_velocity_y = 0;
        e->clear_shield = true;
        e->sounds[0] = 0x641;
        e->freeze_set = true;
        s->phase = 1;
        break;
    }
    case 1:
        s->velocity_x = s->velocity_x < 0 ? SDL_min(s->velocity_x + 0x1000, 0)
                                          : SDL_max(s->velocity_x - 0x1000, 0);
        s->spell_velocity_y -= 0x2800;
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->spell_velocity_y);
        s->scale_x = Word(s->scale_x + 12);
        s->scale_y = s->scale_x;
        s->spell_glow = (Sint16)(s->scale_x * 0x28 / 256);
        if (s->scale_x >= 0x100)
        {
            s->spell_glow = 0x28;
            s->scale_y = s->scale_x = 0x100;
            s->spell_timer = 8;
            s->phase = 2;
        }
        break;
    case 2:
        if (--s->spell_timer == 0)
        {
            s->spell_timer = 0x10;
            e->shake = 4;
            s->phase = 3;
        }
        break;
    case 3:
        s->spell_quads[2].priority = !(s->spell_timer & 1) ? 0x1b8 : (Uint16)(s->priority - 2U);
        s->scale_x = Word(s->scale_x - 0x10);
        if (s->scale_x <= 0)
            s->scale_x = 0;
        s->scale_y = s->scale_x;
        if (--s->spell_timer == 0)
        {
            s->frame = 0;
            for (Uint16 i = 4; i < 18; ++i)
                s->spell_quads[i].draw_mode &= 0xfff7U;
            s->spell_timer = 0;
            e->sounds[0] = 0x62f;
            s->phase = 4;
        }
        DarkTiles(s, 2, 0xf8, true);
        break;
    case 4: {
        s->spell_glow = Word(s->spell_glow + 8);
        s->scale_x = Word(s->scale_x + 6);
        if (s->scale_x >= 0x100)
            s->scale_x = 0x100;
        s->spell_wave_x = Word(s->spell_wave_x - 1);
        if (s->spell_wave_x < 0)
            s->spell_wave_x = 0;
        SotnPlayerEffectQuad *q = &s->spell_quads[2];
        if (q->blue[3] > 8)
            q->blue[3] = (Uint8)(q->blue[3] - 4);
        SpellColors(q, q->blue[3]);
        // From half scale the original's row pointer moves on to the image row, which the
        // full-scale write below then reaches.
        if (s->scale_x >= 0x80)
        {
            q = &s->spell_quads[3];
            q->draw_mode &= 0xfff7U;
        }
        if (s->spell_timer >= 35)
            s->hitbox_state = 0;
        if (s->spell_timer == 32)
        {
            // FACTORY(96)'s rocks with the combo's <<14 hand bits (dropped without a free
            // factory entity), the freeze's end and the spell's own attack.
            if (in.free_factories > e->factory_count)
            {
                e->factories[e->factory_count] = 96U + ((in.hand + 1U) << 14);
                e->factory_x[e->factory_count] = s->x;
                e->factory_y[e->factory_count++] = s->y;
            }
            e->freeze_clear = true;
            if (!Properties(s, in, rng, e))
                return false;
            s->hitbox_width = s->hitbox_height = 0x7f;
        }
        if (s->scale_x == 0x100)
        {
            q->blue[3] = 6;
            s->spell_quads[2].draw_mode |= 8;
            s->hitbox_state = 0;
            s->phase = 5;
        }
        s->spell_timer = Word(s->spell_timer + 1);
        DarkTiles(s, 3, 0xf8, true);
        break;
    }
    case 5:
        s->spell_bright = Word(s->spell_bright - 10);
        if (s->spell_bright < 0)
        {
            s->spell_bright = 0;
            s->spell_timer = 0;
            s->phase = 6;
        }
        break;
    case 6:
        s->spell_timer = Word(s->spell_timer + 1);
        if (s->spell_timer == 0x70)
            s->phase = 7;
        break;
    case 7:
        s->spell_bright = Word(s->spell_bright + 8);
        if (s->spell_bright > 0x120)
        {
            s->spell_quads[3].draw_mode |= 8;
            s->phase = 8;
        }
        break;
    default:
        DarkTiles(s, -8, 8, false);
        s->spell_bright = Word(s->spell_bright - 8);
        s->spell_wave_x = Word(s->spell_wave_x + 1);
        if (s->spell_bright < 0)
        {
            s->removed = true;
            return true;
        }
        break;
    }
    // Screen words: the camera before this pass's retained-screen shift.
    const Sint16 x = Word(Pixel(s->x) - in.retained_camera_x),
                 y = Word(Pixel(s->y) - in.retained_camera_y);
    SotnPlayerEffectQuad *q = &s->spell_quads[2];
    if (q->blue[3] > 8)
    {
        if (in.engine_timer & 1U)
            q->draw_mode |= 8;
        else
            q->draw_mode &= 0xfff7U;
    }
    q->x[0] = q->x[2] = Word(x - s->spell_glow);
    q->x[1] = q->x[3] = Word(x + s->spell_glow - 1);
    q->y[0] = q->y[1] = Word(y - s->spell_glow);
    q->y[2] = q->y[3] = Word(y + s->spell_glow - 1);
    q = &s->spell_quads[3];
    const Sint16 top = q->y[0] = q->y[1] = Word(y - s->spell_half_h);
    Sint16 bottom = q->y[2] = q->y[3] = Word(y + s->spell_half_h - 1);
    Sint16 back = 0, front = 0;
    if (!s->facing_left)
    {
        back = q->x[0] = q->x[2] = Word(x - s->spell_half_w);
        front = q->x[1] = q->x[3] = Word(x + s->spell_half_w - 1);
    }
    else
    {
        front = q->x[1] = q->x[3] = Word(x - s->spell_half_w);
        back = q->x[0] = q->x[2] = Word(x + s->spell_half_w - 1);
    }
    front = Word(front + 1);
    bottom = Word(bottom + 1);
    for (Uint16 i = 0; i < 14; ++i)
    {
        q = &s->spell_quads[4 + i];
        SotnMathTrig at = {0}, next = {0}, wave = {0};
        if (!SotnMath_Trig(in.executable, (Sint16)p->shield_angles[i], &at) ||
            !SotnMath_Trig(in.executable, (Sint16)p->shield_angles[(i + 1) % 14], &next) ||
            !SotnMath_Trig(in.executable, (Sint16)p->shield_angles[(i + 6) % 14], &wave))
            return false;
        Sint16 base = (Sint16)(Asr(at.sine, 5) * s->spell_wave_x / 256);
        q->x[0] = Word(back + base);
        q->x[1] = Word(front + base - 1);
        base = (Sint16)(Asr(next.sine, 5) * s->spell_wave_x / 256);
        q->x[2] = Word(back + base);
        q->x[3] = Word(front + base - 1);
        base = (Sint16)(Asr(wave.sine, 5) * s->spell_wave_y / 256);
        q->y[0] = Word(top + ((bottom - top) * i) / 14 - base);
        q->y[2] = Word(top + ((bottom - top) * (i + 1)) / 14 - base);
        base = (Sint16)(Asr(next.sine, 5) * s->spell_wave_y / 256);
        q->y[1] = Word(top + (i * (bottom - top)) / 14 - base);
        q->y[3] = Word(top + ((bottom - top) * (i + 1)) / 14 - base);
        const Uint8 near = (Uint8)(((at.sine + 0x1000) >> 6) * s->spell_bright / 256),
                    far = (Uint8)(((next.sine + 0x1000) >> 6) * s->spell_bright / 256);
        for (Uint16 k = 0; k < 4; ++k)
            q->red[k] = q->green[k] = q->blue[k] = k < 2 ? near : far;
        p->shield_angles[i] = (Sint32)((Uint32)p->shield_angles[i] + (Uint32)s->spell_step);
    }
    return true;
}
// w_024 func_ptr_80170024 (FACTORY(96), role9): one of thirty-two rocks. Unless its raw parent
// slot is empty it takes one GT4, copies the spell's v band and child palette (plus one), rolls
// its size, screen X/Y below the screen and facing (four rand calls), copies the parent's equip
// id through SetWeaponProperties (0x40 hitbox) and creeps up; once the spell passes step5 it
// rolls a 10..57-update tremble, plays START_SLAM_A and launches, stretching above screen Y 0x80
// and retiring above -0x40. Screen words.
static bool DarkRock(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                     SotnWeaponSecondaryEvents *e)
{
    SotnPlayerEffectQuad *q = &s->spell_quads[0];
    switch (s->phase)
    {
    case 0: {
        if (!in.parent_exists || !in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 9;
        s->allocated = true;
        s->spell_v1 = in.parent_band;
        s->spell_child_palette = (Uint16)(in.parent_child_palette + 1U);
        s->flags |= 0x04800000U;
        s->dark_size = (Sint16)(Rand(rng) & 0xfU);
        s->dark_anchor = (Sint16)(Rand(rng) % 248U + 4U);
        SetScreen(&s->x, s->dark_anchor, in.retained_camera_x);
        SetScreen(&s->y, (Sint16)((Rand(rng) & 0x3fU) + 0xd8U), in.retained_camera_y);
        s->facing_left = (Rand(rng) & 1U) != 0;
        *q = (SotnPlayerEffectQuad){.palette = s->spell_child_palette, .page = 0x19};
        Uint8 dark = 0;
        if (s->dark_size < 8)
        {
            q->u[0] = q->u[2] = 0x50;
            q->u[1] = q->u[3] = 0x50 + 15;
            q->v[0] = q->v[1] = s->spell_v1;
            q->v[2] = q->v[3] = (Uint8)(s->spell_v1 + 0x1f);
            s->spell_half_w = (Sint16)((s->dark_size + 8) * 8 / 10);
            s->spell_half_h = (Sint16)((s->dark_size + 8) * 0x10 / 10);
            dark = (Uint8)(-0x80 - (7 - s->dark_size) * 0xf);
            q->priority = 0x1b8;
        }
        else
        {
            q->u[0] = q->u[2] = 0x60;
            q->u[1] = q->u[3] = 0x60 + 31;
            q->v[0] = q->v[1] = s->spell_v1;
            q->v[2] = q->v[3] = (Uint8)(s->spell_v1 + 0x2f);
            s->spell_half_w = (Sint16)(s->dark_size * 0x10 / 10);
            s->spell_half_h = (Sint16)(s->dark_size * 0x18 / 10);
            q->priority = 0x1be;
            dark = (Uint8)(-0x80 - (0xf - s->dark_size) * 0xc);
        }
        for (Uint16 k = 0; k < 4; ++k)
            q->red[k] = q->green[k] = q->blue[k] = k < 2 ? 0x80 : dark;
        q->draw_mode = 0x04;
        s->spell_velocity_y = -(s->dark_size * 0x3000 + 0x30000);
        s->dark_accel = -((s->dark_size << 8) + 0x900);
        s->spell_wave_x = 0;
        if (!Properties(s, in, rng, e))
            return false;
        s->hitbox_width = s->hitbox_height = 0x40;
        s->phase = 1;
        break;
    }
    case 1:
        s->y = (Sint32)((Uint32)s->y + (Uint32)(s->spell_velocity_y / 16));
        if (in.parent_phase > 5)
        {
            s->spell_timer = (Sint16)(Rand(rng) % 48U + 10U);
            s->phase = 2;
        }
        break;
    case 2:
        SetScreen(&s->x, Word(s->dark_anchor + (s->spell_timer & 1)), in.retained_camera_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)(s->spell_velocity_y / 32));
        s->spell_timer = Word(s->spell_timer - 1);
        if (!s->spell_timer)
        {
            s->spell_velocity_y /= 32;
            e->sounds[0] = 0x63c;
            s->phase = 3;
        }
        break;
    default: {
        SetScreen(&s->x, Word(s->dark_anchor + (s->spell_timer & 1)), in.retained_camera_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->spell_velocity_y);
        s->spell_velocity_y = (Sint32)((Uint32)s->spell_velocity_y + (Uint32)s->dark_accel);
        const Sint16 screen = Word(Pixel(s->y) - in.retained_camera_y);
        if (screen < 0x80)
        {
            s->spell_wave_x = Word(s->spell_wave_x + 2);
            s->dark_accel = (Sint32)((Uint32)s->dark_accel - 0x2000U);
        }
        if (screen < -0x40)
        {
            s->removed = true;
            return true;
        }
        break;
    }
    }
    const Sint16 x = Word(Pixel(s->x) - in.retained_camera_x),
                 y = Word(Pixel(s->y) - in.retained_camera_y);
    q->y[0] = q->y[1] = Word(y - s->spell_half_h);
    q->y[2] = q->y[3] = Word(y + s->spell_half_h - 1 + s->spell_wave_x);
    if (!s->facing_left)
    {
        q->x[0] = q->x[2] = Word(x - s->spell_half_w);
        q->x[1] = q->x[3] = Word(x + s->spell_half_w - 1);
    }
    else
    {
        q->x[1] = q->x[3] = Word(x - s->spell_half_w);
        q->x[0] = q->x[2] = Word(x + s->spell_half_w - 1);
    }
    return true;
}
// w_027 EntityWeaponShieldSpell: Medusa shield's both-buttons spell (item211, one mandatory GT4,
// the glow). It retires a raised shield, plays ANIME_SWORD_C, freezes the scene and drifts back
// while the shield image grows (fixed priority0x1B6); at full size it plays D_8017AC8C (the
// Medusa head opening its eyes, FLAG_UNK_100000) for eight updates, then D_8017ACB4 while the
// glow spreads and fades. When the glow reaches 160 it requests FACTORY(98)'s two lasers, ends
// the freeze, holds eighty updates (D_8017AC8C again at the 64th left) and shrinks away.
static bool MedusaSpell(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in,
                        SotnWeaponSecondaryEvents *e)
{
    SotnPlayerEffectQuad *q = &s->spell_quads[0];
    switch (s->phase)
    {
    case 0:
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 8;
        s->allocated = true;
        s->bank = in.hand ? 0x8012 : 0x8010;
        s->palette = in.hand ? 0x128 : 0x110;
        s->spell_child_palette = in.hand ? 0x137 : 0x11f;
        s->texture = in.hand ? 0x66 : 0x64;
        s->spell_v0 = in.hand ? 0 : 0x80;
        s->spell_v1 = in.hand ? 0x80 : 0;
        s->flags = 0x04810000U;
        s->y = (Sint32)((Uint32)s->y - 0x80000U);
        s->priority = 0x1b6;
        s->facing_left = in.facing_left;
        s->frame = 0x3e;
        s->draw_flags = 3;
        s->scale_x = s->scale_y = 0;
        *q = (SotnPlayerEffectQuad){.palette = 0x19f, .page = 0x19};
        q->u[0] = q->u[2] = 0x80;
        q->u[1] = q->u[3] = 0x80 + 0x4f;
        q->v[0] = q->v[1] = s->spell_v0;
        q->v[2] = q->v[3] = (Uint8)(s->spell_v0 + 0x4f);
        s->spell_glow = 0;
        SpellColors(q, 0x80);
        q->priority = 0x1b4;
        q->draw_mode = 0x40 | 0x20 | 0x10 | 0x04 | 0x01;
        s->velocity_x = s->facing_left ? 0x20000 : -0x20000;
        s->spell_velocity_y = 0;
        e->clear_shield = true;
        e->sounds[0] = 0x641;
        e->freeze_set = true;
        s->phase = 1;
        break;
    case 1:
        s->velocity_x = s->velocity_x < 0 ? SDL_min(s->velocity_x + 0x1000, 0)
                                          : SDL_max(s->velocity_x - 0x1000, 0);
        s->spell_velocity_y -= 0x2800;
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->spell_velocity_y);
        s->scale_x = Word(s->scale_x + 12);
        s->scale_y = s->scale_x;
        s->spell_glow = (Sint16)(s->scale_x * 40 / 256);
        if (s->scale_x >= 0x100)
        {
            s->spell_glow = 0x28;
            s->scale_y = s->scale_x = 0x100;
            s->spell_timer = 8;
            s->timeline = 0xc8c;
            s->remaining = 0;
            s->pose = 0;
            s->flags |= 0x100000U;
            s->phase = 2;
        }
        break;
    case 2:
        if (--s->spell_timer == 0)
        {
            s->timeline = 0xcb4;
            s->remaining = 0;
            s->pose = 0;
            s->phase = 3;
        }
        break;
    case 3:
        s->spell_glow = Word(s->spell_glow + 8);
        if (q->blue[3] > 8)
            q->blue[3] = (Uint8)(q->blue[3] - 4);
        SpellColors(q, q->blue[3]);
        if (s->spell_glow >= 160)
        {
            q->blue[3] = 6;
            q->draw_mode |= 8;
            // FACTORY(98)'s two lasers with the combo's <<14 hand bits (dropped without a free
            // factory entity).
            if (in.free_factories > e->factory_count)
            {
                e->factories[e->factory_count] = 98U | ((in.hand + 1U) << 14);
                e->factory_x[e->factory_count] = s->x;
                e->factory_y[e->factory_count++] = s->y;
            }
            s->spell_timer = 80;
            e->freeze_clear = true;
            s->phase = 4;
        }
        break;
    case 4:
        if (s->spell_timer == 64)
        {
            s->timeline = 0xc8c;
            s->remaining = 0;
            s->pose = 0;
        }
        if (--s->spell_timer == 0)
            s->phase = 5;
        break;
    default:
        s->scale_x = Word(s->scale_x - 12);
        s->scale_y = s->scale_x;
        if (s->scale_x < 0)
        {
            s->removed = true;
            return true;
        }
        break;
    }
    const Sint16 x = Word(Pixel(s->x) - in.retained_camera_x),
                 y = Word(Pixel(s->y) - in.retained_camera_y);
    if (q->blue[3] > 8)
    {
        if (in.engine_timer & 1U)
            q->draw_mode |= 8;
        else
            q->draw_mode &= 0xfff7U;
    }
    q->x[0] = q->x[2] = Word(x - s->spell_glow);
    q->x[1] = q->x[3] = Word(x + s->spell_glow - 1);
    q->y[0] = q->y[1] = Word(y - s->spell_glow);
    q->y[2] = q->y[3] = Word(y + s->spell_glow - 1);
    return true;
}
static bool MedusaEdge(SotnDataView exe, Sint16 heading, Sint16 width, Sint16 *dx, Sint16 *dy)
{
    SotnMathTrig t = {0};
    if (!SotnMath_Trig(exe, heading, &t))
        return false;
    *dx = (Sint16)Asr(Asr(t.cosine, 4) * width, 8);
    *dy = (Sint16)Asr(Asr(t.sine, 4) * width, 8);
    return true;
}
// w_027 func_ptr_80170024 (FACTORY(98), role9): one of two lasers. It takes eighteen GT4s (failure
// retires it), steps 6 or 14 ahead, aims back-up (params0) or higher (params1), marks a target
// (func_80118970) and plays BIBLE_SCRAPE. Each update it turns toward its target (func_80118B18,
// limited by a turn step that tightens then relaxes), swings on an SDK square-root wave damped by
// a decaying unkA0, moves three units per step of that speed, plays BIBLE_SCRAPE once per wave
// period on screen, requests FACTORY(100)'s hit child on even g_Timer and lays a ribbon of up to
// eighteen segments whose width follows the swing. It retires far off screen. Screen words.
static bool MedusaLaser(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in,
                        SotnWeaponSecondaryEvents *e)
{
    Sint16 lift = 0;
    switch (s->phase)
    {
    case 0: {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 9;
        s->allocated = true;
        s->laser_palette = in.hand ? 0x129 : 0x111;
        s->laser_band = in.hand ? 0x80 : 0;
        s->flags |= 0x04800000U;
        const Sint16 step = (s->parameter & 0xffU) ? 14 : 6;
        SetScreen(&s->x, Word(Pixel(s->x) - in.retained_camera_x + (s->facing_left ? -step : step)),
                  in.retained_camera_x);
        s->laser_heading = s->facing_left ? (Sint16)(((s->parameter & 0xffU) << 9) + 0x100U)
                                          : (Sint16)(0x700 - (Sint16)((s->parameter & 0xffU) << 9));
        s->laser_turn = 0x50;
        s->laser_phase = 0x400;
        s->laser_step = -0x58;
        s->laser_swing = 0;
        e->select_target = true;
        s->laser_length = 1;
        const Sint16 x = Word(Pixel(s->x) - in.retained_camera_x),
                     y = Word(Pixel(s->y) - in.retained_camera_y);
        SotnMathTrig a = {0}, b = {0};
        if (!SotnMath_Trig(in.executable, (Sint16)(s->laser_heading + 0x400), &a) ||
            !SotnMath_Trig(in.executable, (Sint16)(s->laser_heading - 0x400), &b))
            return false;
        for (Uint16 i = 0; i < 18; ++i)
        {
            SotnPlayerEffectQuad *q = &s->laser_quads[i];
            *q = (SotnPlayerEffectQuad){.palette = s->laser_palette, .page = 0x19};
            SpellColors(q, 0x80);
            q->x[0] = Word(x + Asr(a.cosine, 9));
            q->y[0] = Word(y - Asr(a.sine, 9));
            q->x[2] = Word(x + Asr(b.cosine, 9));
            q->y[2] = Word(y - Asr(b.sine, 9));
            q->priority = 0x1bc;
            q->draw_mode = 0x20 | 0x10 | 0x08 | 0x01;
        }
        s->laser_damping = 0x300;
        s->laser_timer = 0;
        e->sounds[0] = 0x6b2;
        s->phase = 1;
        break;
    }
    default: {
        const Sint16 sx = Word(Pixel(s->x) - in.retained_camera_x),
                     sy = Word(Pixel(s->y) - in.retained_camera_y);
        if ((s->laser_phase & 0x800) && !(s->laser_previous & 0x800) && sx >= -0x120 &&
            sx <= 0x120 && sy >= 0 && sy <= 0x100)
            e->sounds[0] = 0x6b2;
        s->laser_previous = s->laser_phase;
        Sint16 aim = 0;
        if (!TargetHeading(in, sx, sy, s->laser_bound, s->facing_left, &aim))
            return false;
        if (aim >= 0)
        {
            const Sint32 heading = s->laser_heading & 0xfff;
            const Sint16 right = (Sint16)heading;
            const Sint16 gap = (Sint16)SDL_abs(right - aim);
            if (gap < s->laser_turn)
                s->laser_turn = gap;
            if ((right < aim && gap >= 0x800) || (right >= aim && gap < 0x800))
                s->laser_heading = (Sint16)((heading - s->laser_turn) & 0xfff);
            else
                s->laser_heading = (Sint16)((s->laser_turn + heading) & 0xfff);
        }
        s->laser_phase = Word(s->laser_phase + s->laser_step);
        SotnMathTrig w = {0}, h = {0};
        if (!SotnMath_Trig(in.executable, s->laser_phase, &w) ||
            !SotnMath_Trig(in.executable, s->laser_heading, &h))
            return false;
        Uint32 root = 0;
        if (!SotnMath_Sqrt0(in.executable, (Uint32)(0x01000000 - (Sint32)w.sine * (Sint32)w.sine),
                            &root))
            return false;
        Sint16 speed = (Sint16)root;
        s->laser_swing = (Sint32)((Uint32)s->laser_swing - (Uint32)Asr((Sint32)w.sine * 4096, 8));
        // MIPS mult keeps the product's low word.
        speed =
            (Sint16)(speed - (Sint32)((Uint32)s->laser_damping * (Uint32)s->laser_swing) / 0x10000);
        if (--s->laser_damping < 0)
            s->laser_damping = 0;
        const Sint32 dx = Asr((Sint32)h.cosine * speed, 7) * 3,
                     dy = -Asr((Sint32)h.sine * speed, 7) * 3;
        s->x = (Sint32)((Uint32)s->x + (Uint32)dx);
        s->y = (Sint32)((Uint32)s->y + (Uint32)dy);
        lift = (Sint16)-Asr(s->laser_swing, 15);
        s->laser_turn = (Sint16)SDL_min(s->laser_turn + 8, 0x100);
        if (++s->laser_length > 0x80)
            s->laser_length = 0x80;
        break;
    }
    }
    const Sint16 x = Word(Pixel(s->x) - in.retained_camera_x),
                 y = Word(Pixel(s->y) - in.retained_camera_y);
    if (x < -0x200 || x > 0x200 || y < -0xc0 || y > 0x1a0)
    {
        s->removed = true;
        return true;
    }
    if (!(in.engine_timer & 1U) && in.free_factories > e->factory_count)
    {
        e->factories[e->factory_count] = 100U | ((in.hand + 1U) << 14);
        e->factory_x[e->factory_count] = s->x;
        e->factory_y[e->factory_count++] = s->y;
    }
    Sint16 width = 0;
    if (s->laser_swing > 0)
    {
        width = (Sint16)(8 - (s->laser_swing >> 16));
        if (width < 0)
            width = 1;
    }
    else
    {
        width = (Sint16)(8 - Asr(s->laser_swing, 0x13));
        if (width > 16)
            width = 16;
    }
    Sint16 ax = 0, ay = 0, bx = 0, by = 0;
    if (!MedusaEdge(in.executable, (Sint16)(s->laser_heading + 0x400), width, &ax, &ay) ||
        !MedusaEdge(in.executable, (Sint16)(s->laser_heading - 0x400), width, &bx, &by))
        return false;
    SotnPlayerEffectQuad *q = &s->laser_quads[0];
    if (s->laser_length < 0x13)
    {
        Sint16 left = q->x[0], top = q->y[0], right = q->x[2], bottom = q->y[2];
        for (Sint16 i = 0; i < s->laser_length; ++i)
        {
            q = &s->laser_quads[i];
            if (i == s->laser_length - 1)
            {
                q->x[0] = left;
                q->y[0] = top;
                q->x[2] = right;
                q->y[2] = bottom;
                q->x[1] = Word(x + ax);
                q->y[1] = Word(lift + (y - ay));
                q->x[3] = Word(x + bx);
                q->y[3] = Word(lift + (y - by));
                q->draw_mode &= 0xfff7U;
            }
            left = q->x[1];
            top = q->y[1];
            right = q->x[3];
            bottom = q->y[3];
        }
    }
    else
    {
        for (Uint16 i = 0; i < 17; ++i)
            s->laser_quads[i] = s->laser_quads[i + 1];
        q = &s->laser_quads[17];
        q->x[0] = q->x[1];
        q->y[0] = q->y[1];
        q->x[2] = q->x[3];
        q->y[2] = q->y[3];
        q->x[1] = Word(x + ax);
        q->y[1] = Word(lift + (y - ay));
        q->x[3] = Word(x + bx);
        q->y[3] = Word(lift + (y - by));
    }
    const Sint16 count = s->laser_length >= 0x13 ? 0x12 : s->laser_length;
    for (Sint16 i = 0; i < count; ++i)
    {
        q = &s->laser_quads[i];
        q->u[0] = q->u[2] = (Uint8)(((i * 3) << 5) / count);
        q->u[1] = q->u[3] = (Uint8)(((i + 1) * 0x60) / count - 1);
        q->v[0] = q->v[1] = (Uint8)(s->laser_band + 0x30);
        q->v[2] = q->v[3] = (Uint8)(s->laser_band + 0x3f);
    }
    return true;
}
// w_027 func_ptr_80170028 (FACTORY(100), role10): a laser's six-update hit point. It retires once
// its raw parent laser's slot empties; at birth it rolls SetWeaponProperties for the laser's equip
// id (the factory argument, zero) with a 6x6 hitbox. Each hit counts; at six it clears the raw
// parent laser's target. Screen words.
static bool MedusaHit(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                      SotnWeaponSecondaryEvents *e)
{
    if (!in.parent_exists)
    {
        s->removed = true;
        return true;
    }
    if (!s->phase)
    {
        s->role = 10;
        s->allocated = true;
        if (!Properties(s, in, rng, e))
            return false;
        s->hitbox_width = s->hitbox_height = 6;
        s->laser_timer = 6;
        s->phase = 1;
    }
    else if (--s->laser_timer == 0)
    {
        s->removed = true;
        return true;
    }
    if (s->hit_flags)
    {
        if (++s->laser_hits >= 6)
            e->clear_parent_target = true;
        s->hit_flags = 0;
    }
    return true;
}
// w_028 func_C8000_8017B3D4: every Skull spell callback ping-pongs D_8017AB18 over 0..31
// (D_8017AB1A parity) and writes it into colour11's low five bits of palette0x113/0x12B, then
// uploads.
static void SkullRamp(SotnHandWeaponProgram *p, SotnWeaponSecondaryInput in,
                      SotnWeaponSecondaryEvents *e)
{
    if (p->skull_parity & 1)
    {
        p->skull_ramp = Word(p->skull_ramp + 1);
        if (p->skull_ramp >= 0x20)
        {
            p->skull_parity = Word(p->skull_parity + 1);
            p->skull_ramp = 0x1f;
        }
    }
    else
    {
        p->skull_ramp = Word(p->skull_ramp - 1);
        if (p->skull_ramp < 0)
        {
            p->skull_parity = Word(p->skull_parity + 1);
            p->skull_ramp = 0;
        }
    }
    const Uint32 index = 0x3bU + in.hand * 0x180U;
    e->clut = true;
    e->clut_palette = (Uint16)(0x110U + index / 16U);
    e->clut_color = (Uint16)(index % 16U);
    e->clut_mask = 0xffe0;
    e->clut_add = (Uint16)p->skull_ramp;
}
// w_028 EntityWeaponShieldSpell: Skull shield's both-buttons spell (item212, thirteen mandatory
// GT4s: the glow, the skull image, its palette+2 copy behind and ten white slices). It grows,
// flashes out with WEAPON_APPEAR, opens the image (scale/8 half extents, palette alternating each
// second g_Timer), dims, and at the hold's 64th update requests FACTORY(88)'s beam, ends the freeze
// and recoils 3.5 back, trembling for 96 updates; then it brightens, hides the image and slides the
// slices apart as they fade. Program angles D_8017AB1C (ten used) and the red ramp run each update.
static bool SkullSpell(SotnWeaponSecondary *s, SotnHandWeaponProgram *p,
                       SotnWeaponSecondaryInput in, SotnWeaponSecondaryEvents *e)
{
    if (!p->shield_angles_loaded)
    {
        const Uint8 *a = NULL, *ramp = NULL;
        if (!View(in.overlay, 0xb1c, 40, &a) || !View(in.overlay, 0xb18, 4, &ramp))
            return false;
        for (Uint16 i = 0; i < 10; ++i)
            p->shield_angles[i] = (Sint32)Read32(a + i * 4U);
        p->skull_ramp = (Sint16)Read16(ramp);
        p->skull_parity = (Sint16)Read16(ramp + 2);
        p->shield_angles_loaded = true;
    }
    SotnPlayerEffectQuad *q = &s->spell_quads[0];
    switch (s->phase)
    {
    case 0:
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 8;
        s->allocated = true;
        s->bank = in.hand ? 0x8012 : 0x8010;
        s->palette = in.hand ? 0x128 : 0x110;
        s->spell_child_palette = in.hand ? 0x129 : 0x111;
        s->texture = in.hand ? 0x66 : 0x64;
        s->spell_v0 = in.hand ? 0 : 0x80;
        s->spell_v1 = in.hand ? 0x80 : 0;
        s->flags = 0x04810000U;
        s->y = (Sint32)((Uint32)s->y - 0x80000U);
        s->priority = (Uint16)(in.priority - 2U);
        s->facing_left = in.facing_left;
        s->frame = 62;
        s->draw_flags = 3;
        s->scale_x = s->scale_y = 0;
        *q = (SotnPlayerEffectQuad){.palette = 0x19f, .page = 0x19};
        q->u[0] = q->u[2] = 128;
        q->u[1] = q->u[3] = 128 + 79;
        q->v[0] = q->v[1] = s->spell_v0;
        q->v[2] = q->v[3] = (Uint8)(s->spell_v0 + 79);
        s->spell_glow = 0;
        SpellColors(q, 0x80);
        q->priority = (Uint16)(s->priority - 4U);
        q->draw_mode = 0x40 | 0x20 | 0x10 | 0x04 | 0x01;
        q = &s->spell_quads[1];
        *q = (SotnPlayerEffectQuad){.palette = s->spell_child_palette, .page = 0x19};
        q->u[0] = q->u[2] = 0;
        q->u[1] = q->u[3] = 63;
        q->v[0] = q->v[1] = (Uint8)(s->spell_v1 + 0x30);
        q->v[2] = q->v[3] = (Uint8)(s->spell_v1 + 0x30 + 79);
        SpellColors(q, 0x80);
        q->priority = 0x1b6;
        q->draw_mode = 0x08;
        s->spell_quads[2] = *q;
        s->spell_quads[2].palette = (Uint16)(s->spell_child_palette + 2U);
        s->spell_quads[2].priority = 0x1b4;
        for (Uint16 i = 0; i < 10; ++i)
        {
            q = &s->spell_quads[3 + i];
            *q = s->spell_quads[2 + i];
            q->v[0] = q->v[1] = (Uint8)(s->spell_v1 + (i * 8 + 0x30));
            q->v[2] = q->v[3] = (Uint8)(s->spell_v1 + (i + 1) * 8 + 0x2f);
            q->palette = 0x15f;
            q->draw_mode |= 0x20 | 0x10 | 0x04 | 0x01;
            q->priority = 0x1b8;
        }
        s->spell_step = 0x80;
        s->spell_bright = 0x100;
        s->spell_half_w = s->spell_half_h = 0;
        s->spell_wave_x = s->spell_wave_y = 0;
        s->velocity_x = s->facing_left ? 0x20000 : -0x20000;
        s->spell_velocity_y = 0;
        e->clear_shield = true;
        e->sounds[0] = 0x641;
        e->freeze_set = true;
        s->phase = 1;
        break;
    case 1:
        s->velocity_x = s->velocity_x < 0 ? SDL_min(s->velocity_x + 0x1000, 0)
                                          : SDL_max(s->velocity_x - 0x1000, 0);
        s->spell_velocity_y -= 0x2800;
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->spell_velocity_y);
        s->scale_x = Word(s->scale_x + 12);
        s->scale_y = s->scale_x;
        s->spell_glow = (Sint16)(s->scale_x * 40 / 256);
        if (s->scale_x >= 0x100)
        {
            s->spell_glow = 0x28;
            s->scale_x = s->scale_y = 0x100;
            s->spell_timer = 8;
            s->phase = 2;
        }
        break;
    case 2:
        if (--s->spell_timer == 0)
        {
            s->spell_timer = 16;
            s->phase = 3;
        }
        break;
    case 3:
        q->priority = !(s->spell_timer & 1) ? 0x1b8 : (Uint16)(s->priority - 2U);
        s->scale_x = Word(s->scale_x - 16);
        if (s->scale_x <= 0)
            s->scale_x = 0;
        s->scale_y = s->scale_x;
        if (--s->spell_timer == 0)
        {
            s->frame = 0;
            for (Uint16 i = 1; i < 13; ++i)
                s->spell_quads[i].draw_mode &= 0xfff7U;
            s->spell_timer = 0;
            e->sounds[0] = 0x62f;
            s->phase = 4;
        }
        break;
    case 4:
        s->spell_glow = Word(s->spell_glow + 8);
        s->scale_x = Word(s->scale_x + 12);
        if (s->scale_x >= 0x100)
            s->scale_x = 0x100;
        s->spell_half_w = s->spell_half_h = (Sint16)(s->scale_x / 8);
        if (q->blue[3] > 8)
            q->blue[3] = (Uint8)(q->blue[3] - 4);
        SpellColors(q, q->blue[3]);
        if (!(in.engine_timer & 1U))
            s->spell_wave_x = Word(s->spell_wave_x - 1);
        if (s->spell_wave_x < 0)
            s->spell_wave_x = 0;
        if (s->scale_x == 0x100 && !s->spell_wave_x)
        {
            q->blue[3] = 4;
            q->draw_mode |= 8;
            s->phase = 5;
        }
        s->spell_timer = Word(s->spell_timer + 1);
        break;
    case 5:
        s->spell_bright = Word(s->spell_bright - 10);
        if (s->spell_bright < 0)
        {
            s->spell_bright = 0;
            s->spell_timer = 0;
            s->phase = 6;
        }
        break;
    case 6:
        if (s->spell_timer == 64)
        {
            // FACTORY(88)'s beam with the combo's <<14 hand bits (dropped without a free factory
            // entity), the freeze's end and the recoil.
            if (in.free_factories > e->factory_count)
            {
                e->factories[e->factory_count] = 88U | ((in.hand + 1U) << 14);
                e->factory_x[e->factory_count] = s->x;
                e->factory_y[e->factory_count++] = s->y;
            }
            e->freeze_clear = true;
            s->skull_y = Word(Pixel(s->y) - in.retained_camera_y);
            s->velocity_x = s->facing_left ? 0x38000 : -0x38000;
        }
        if (s->spell_timer > 64 && s->spell_timer < 160)
        {
            const Uint32 shake =
                s->spell_timer < 128 ? in.engine_timer & 1U : (in.engine_timer >> 1) & 1U;
            SetScreen(&s->y, Word(s->skull_y - (Sint16)shake), in.retained_camera_y);
            s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
            s->velocity_x = s->velocity_x < 0 ? SDL_min(s->velocity_x + 0x2c00, 0)
                                              : SDL_max(s->velocity_x - 0x2c00, 0);
        }
        s->spell_timer = Word(s->spell_timer + 1);
        if (s->spell_timer == 160)
            s->phase = 7;
        break;
    case 7:
        s->spell_bright = Word(s->spell_bright + 16);
        if (s->spell_bright > 0x120)
        {
            s->spell_quads[1].draw_mode |= 8;
            s->spell_quads[2].draw_mode |= 8;
            s->spell_timer = 0x18;
            s->spell_step = 0x20;
            s->phase = 8;
        }
        break;
    default:
        s->spell_wave_x = Word(s->spell_wave_x + 1);
        s->spell_half_h = Word(s->spell_half_h + 2);
        if (s->spell_half_h < 0)
            s->spell_half_h = 0;
        s->spell_bright = Word(s->spell_bright - 10);
        if (s->spell_bright < 0)
            s->spell_bright = 0;
        if (--s->spell_timer == 0)
        {
            s->removed = true;
            return true;
        }
        break;
    }
    const Sint16 x = Word(Pixel(s->x) - in.retained_camera_x),
                 y = Word(Pixel(s->y) - in.retained_camera_y);
    q = &s->spell_quads[0];
    if (q->blue[3] > 8)
    {
        if (in.engine_timer & 1U)
            q->draw_mode |= 8;
        else
            q->draw_mode &= 0xfff7U;
    }
    q->x[0] = q->x[2] = Word(x - s->spell_glow);
    q->x[1] = q->x[3] = Word(x + s->spell_glow - 1);
    q->y[0] = q->y[1] = Word(y - s->spell_glow);
    q->y[2] = q->y[3] = Word(y + s->spell_glow - 1);
    q = &s->spell_quads[1];
    q->palette = (Uint16)(s->spell_child_palette + ((in.engine_timer >> 1) & 1U));
    const Sint16 top = q->y[0] = q->y[1] = Word(y - s->spell_half_h);
    Sint16 bottom = Word(y + s->spell_half_h - 1);
    q->y[2] = q->y[3] = bottom;
    Sint16 left = 0, right = 0;
    if (!s->facing_left)
    {
        left = q->x[0] = q->x[2] = Word(x - s->spell_half_w);
        right = q->x[1] = q->x[3] = Word(x + s->spell_half_w - 1);
    }
    else
    {
        right = q->x[1] = q->x[3] = Word(x - s->spell_half_w);
        left = q->x[0] = q->x[2] = Word(x + s->spell_half_w - 1);
    }
    q = &s->spell_quads[2];
    q->x[0] = q->x[2] = left;
    q->x[1] = q->x[3] = right;
    q->y[0] = q->y[1] = top;
    q->y[2] = q->y[3] = bottom;
    right = Word(right + 1);
    bottom = Word(bottom + 1);
    for (Uint16 i = 0; i < 10; ++i)
    {
        q = &s->spell_quads[3 + i];
        SotnMathTrig at = {0}, next = {0}, wave = {0};
        if (!SotnMath_Trig(in.executable, (Sint16)p->shield_angles[i], &at) ||
            !SotnMath_Trig(in.executable, (Sint16)p->shield_angles[(i + 1) % 10], &next) ||
            !SotnMath_Trig(in.executable, (Sint16)p->shield_angles[(i + 6) % 10], &wave))
            return false;
        Sint16 sine = (Sint16)(Asr(at.sine, 5) * s->spell_wave_x / 256);
        q->x[0] = Word(left + sine);
        q->x[1] = Word(right + sine - 1);
        sine = (Sint16)(Asr(next.sine, 5) * s->spell_wave_x / 256);
        q->x[2] = Word(left + sine);
        q->x[3] = Word(right + sine - 1);
        sine = (Sint16)(Asr(wave.sine, 5) * s->spell_wave_y / 256);
        const Sint16 row_top = Word(top + ((bottom - top) * i) / 10),
                     row_bottom = Word(top + ((bottom - top) * (i + 1)) / 10);
        q->y[0] = Word(row_top - sine);
        q->y[2] = Word(row_bottom - sine);
        sine = (Sint16)(Asr(next.sine, 5) * s->spell_wave_y / 256);
        q->y[1] = Word(row_top - sine);
        q->y[3] = Word(row_bottom - sine);
        const Uint8 near = (Uint8)(((at.sine + 0x1000) >> 6) * s->spell_bright / 256),
                    far = (Uint8)(((next.sine + 0x1000) >> 6) * s->spell_bright / 256);
        for (Uint16 k = 0; k < 4; ++k)
            q->red[k] = q->green[k] = q->blue[k] = k < 2 ? near : far;
        p->shield_angles[i] = (Sint32)((Uint32)p->shield_angles[i] + (Uint32)s->spell_step);
    }
    SkullRamp(p, in, e);
    return true;
}
// w_028 func_ptr_80170024 (FACTORY(88), role9): the skull's beam. Unless its raw parent slot is
// empty it takes three GT4s (failure retires it), copies the spell's v band and child palette plus
// three, and sits 24 ahead of the raw parent's X every update. A black mask grows to 32 and gives
// way to the skull and a beam to the screen edge (FIREBALL_SHOT_A); for 0x40 updates it rolls
// SetWeaponProperties every update with a hitbox reaching the screen edge, then shrinks away.
static bool SkullBeam(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                      SotnWeaponSecondaryEvents *e)
{
    SotnPlayerEffectQuad *q = s->spell_quads;
    switch (s->phase)
    {
    case 0:
        if (!in.parent_exists || !in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 9;
        s->allocated = true;
        s->spell_v1 = in.parent_band;
        s->spell_child_palette = (Uint16)(in.parent_child_palette + 3U);
        s->flags |= 0x04800000U;
        SetScreen(&s->x, Word(Pixel(s->x) - in.retained_camera_x + (s->facing_left ? -24 : 24)),
                  in.retained_camera_x);
        q[0] = (SotnPlayerEffectQuad){.palette = 0x15e, .page = 0x1a, .priority = 0x1ba};
        q[0].u[0] = q[0].u[2] = 0;
        q[0].u[1] = q[0].u[3] = 63;
        q[0].v[0] = q[0].v[1] = 192;
        q[0].v[2] = q[0].v[3] = 192 + 63;
        s->spell_half_w = s->spell_half_h = 0;
        q[1] = (SotnPlayerEffectQuad){.page = 0x19, .priority = 0x1ba};
        q[1].u[0] = q[1].u[1] = 64;
        q[1].u[2] = q[1].u[3] = 64 + 63;
        q[1].v[3] = q[1].v[1] = (Uint8)(s->spell_v1 + 64);
        q[1].v[2] = q[1].v[0] = (Uint8)(s->spell_v1 + 64 + 63);
        q[1].draw_mode = 0x20 | 0x10 | 0x08 | 0x01;
        q[2] = (SotnPlayerEffectQuad){.page = 0x19, .priority = 0x1ba};
        q[2].u[0] = q[2].u[1] = 80;
        q[2].u[2] = q[2].u[3] = 80 + 31;
        q[2].v[3] = q[2].v[1] = (Uint8)(s->spell_v1 + 56);
        q[2].v[2] = q[2].v[0] = (Uint8)(s->spell_v1 + 56 + 7);
        q[2].draw_mode = 0x20 | 0x10 | 0x08 | 0x01;
        s->spell_timer = 0x40;
        s->phase = 1;
        break;
    case 1:
        s->spell_half_w = Word(s->spell_half_w + 2);
        if (s->spell_half_w >= 32)
        {
            s->spell_half_w = 32;
            q[0].draw_mode |= 8;
            q[1].draw_mode &= 0xfff7U;
            q[2].draw_mode &= 0xfff7U;
            e->sounds[0] = 0x660;
            s->phase = 2;
        }
        s->spell_half_h = s->spell_half_w;
        break;
    case 2: {
        if (!Properties(s, in, rng, e))
            return false;
        const Sint16 screen = Word(Pixel(s->x) - in.retained_camera_x);
        const Sint32 reach = s->facing_left ? (screen + 24) / 2 : (0x118 - screen) / 2;
        s->hitbox_width = (Uint8)reach;
        s->hitbox_x = (Sint16)reach;
        s->hitbox_height = 0x10;
        if (--s->spell_timer == 0)
        {
            s->hitbox_state = 0;
            s->phase = 3;
        }
        break;
    }
    default:
        s->spell_half_w = Word(s->spell_half_w - 8);
        s->spell_half_h = Word(s->spell_half_h - 8);
        if (s->spell_half_w < 0)
        {
            s->removed = true;
            return true;
        }
        break;
    }
    const Sint16 parent = Word(Pixel(in.parent_x) - in.retained_camera_x);
    SetScreen(&s->x, Word(parent + (s->facing_left ? -24 : 24)), in.retained_camera_x);
    const Sint16 x = Word(Pixel(s->x) - in.retained_camera_x),
                 y = Word(Pixel(s->y) - in.retained_camera_y);
    for (Uint16 k = 0; k < 2; ++k)
    {
        q[k].y[0] = q[k].y[1] = Word(y - s->spell_half_h);
        q[k].y[2] = q[k].y[3] = Word(y + s->spell_half_h - 1);
        if (!s->facing_left)
        {
            q[k].x[0] = q[k].x[2] = Word(x - s->spell_half_w);
            q[k].x[1] = q[k].x[3] = Word(x + s->spell_half_w - 1);
        }
        else
        {
            q[k].x[1] = q[k].x[3] = Word(x - s->spell_half_w);
            q[k].x[0] = q[k].x[2] = Word(x + s->spell_half_w - 1);
        }
    }
    const Uint16 palette = (Uint16)(s->spell_child_palette + ((in.engine_timer >> 1) & 1U));
    q[1].palette = palette;
    q[2].y[0] = q[2].y[1] = Word(y - s->spell_half_h / 2);
    q[2].y[2] = q[2].y[3] = Word(y + s->spell_half_h / 2 - 1);
    if (!s->facing_left)
    {
        q[2].x[0] = q[2].x[2] = Word(x + s->spell_half_w - 1);
        q[2].x[1] = q[2].x[3] = 288 - 16;
    }
    else
    {
        q[2].x[1] = q[2].x[3] = Word(x - s->spell_half_w);
        q[2].x[0] = q[2].x[2] = -16;
    }
    q[2].palette = palette;
    return true;
}
// w_029 func_ptr_80170004 (WFACTORY(56,0), role1): the Fire shield guard's afterimage. Every
// update it retires unless its raw slot16 parent holds item15; at birth it copies the parent's
// bank, unk5A, frame and unk80 palette base four priorities in front, is CAMERA_LOCKED and fades
// with ENTITY_OPACITY|MASK_R (red held at 128) for twenty updates through D_8017AD04's offsets.
static bool FireImage(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in)
{
    if (!in.parent_exists)
    {
        s->removed = true;
        return true;
    }
    if (!s->phase)
    {
        s->role = 1;
        s->allocated = true;
        s->bank = in.parent_bank;
        s->priority = (Uint16)(in.parent_priority + 4U);
        s->texture = in.parent_texture;
        s->spell_child_palette = in.parent_child_palette;
        s->frame = in.parent_frame;
        s->flags = 0x08000000U;
        s->draw_flags = 0x18;
        s->opacity = 0x80;
        s->lifetime = 0x14;
        s->phase = 1;
    }
    else
    {
        if (s->opacity >= 7)
            s->opacity = (Uint8)(s->opacity - 7U);
        s->lifetime = Word(s->lifetime - 1);
        if (!s->lifetime)
        {
            s->removed = true;
            return true;
        }
    }
    const Uint8 *offset = NULL;
    if (!View(in.overlay, 0xd04U + (in.game_timer / 2U % 5U) * 2U, 2, &offset))
        return SDL_SetError("Fire shield afterimage private palette offsets");
    s->palette = (Uint16)(s->spell_child_palette + Read16(offset));
    return true;
}
static bool FirePalette(SotnDataView o, Uint32 timer, Uint16 base, Uint16 *out)
{
    const Uint8 *offset = NULL;
    if (!View(o, 0xcf8U + (timer / 2U % 5U) * 2U, 2, &offset))
        return SDL_SetError("Fire shield spell private palette offsets");
    *out = (Uint16)(base + Read16(offset));
    return true;
}
// w_029 EntityWeaponShieldSpell: Fire shield's both-buttons spell (item213, one mandatory GT4,
// the glow). Like the card spells it retires a raised shield, plays ANIME_SWORD_C, freezes the
// scene and drifts back while the shield image grows (its palette cycling D_8017ACF8 on g_Timer,
// then g_GameTimer while it shrinks away). The flame (D_8017AD24, FLAG_UNK_100000, priority0x1B6)
// then grows with TRANSFORM while the glow spreads and fades; after 0x20 updates the glow hides,
// FIREBALL_SHOT_A plays, FACTORY(102) releases the three dragons, the freeze ends and, 0x18
// updates later, the flame shrinks away.
static bool FireSpell(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in,
                      SotnWeaponSecondaryEvents *e)
{
    SotnPlayerEffectQuad *q = &s->spell_quads[0];
    switch (s->phase)
    {
    case 0:
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 8;
        s->allocated = true;
        s->bank = in.hand ? 0x8012 : 0x8010;
        s->spell_child_palette = in.hand ? 0x128 : 0x110;
        s->texture = in.hand ? 0x66 : 0x64;
        s->spell_v0 = in.hand ? 0 : 0x80;
        s->spell_v1 = in.hand ? 0x80 : 0;
        s->y = (Sint32)((Uint32)s->y - 0x80000U);
        s->flags = 0x04810000U;
        s->priority = (Uint16)(in.priority - 2U);
        s->facing_left = in.facing_left;
        s->frame = 0x3e;
        s->draw_flags = 3;
        s->scale_x = s->scale_y = 0;
        *q = (SotnPlayerEffectQuad){.palette = 0x19f, .page = 0x19};
        q->u[0] = q->u[2] = 0x80;
        q->u[1] = q->u[3] = 0xcf;
        q->v[0] = q->v[1] = s->spell_v0;
        q->v[2] = q->v[3] = (Uint8)(s->spell_v0 + 0x4f);
        s->spell_glow = 0;
        SpellColors(q, 0x80);
        q->priority = (Uint16)(s->priority - 4U);
        q->draw_mode = 0x40 | 0x20 | 0x10 | 0x04 | 0x01;
        s->velocity_x = s->facing_left ? 0x20000 : -0x20000;
        s->spell_velocity_y = 0;
        e->clear_shield = true;
        e->sounds[0] = 0x641;
        e->freeze_set = true;
        s->phase = 1;
        break;
    case 1:
        s->velocity_x = s->velocity_x < 0 ? SDL_min(s->velocity_x + 0x1000, 0)
                                          : SDL_max(s->velocity_x - 0x1000, 0);
        s->spell_velocity_y -= 0x2800;
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->spell_velocity_y);
        s->scale_x = Word(s->scale_x + 12);
        s->scale_y = s->scale_x;
        s->spell_glow = (Sint16)(s->scale_x * 40 / 256);
        if (s->scale_x >= 0x100)
        {
            s->spell_glow = 0x28;
            s->scale_y = s->scale_x = 0x100;
            s->spell_timer = 8;
            s->phase = 2;
        }
        if (!FirePalette(in.overlay, in.engine_timer, s->spell_child_palette, &s->palette))
            return false;
        break;
    case 2:
        if (--s->spell_timer == 0)
            s->phase = 3;
        if (!FirePalette(in.overlay, in.engine_timer, s->spell_child_palette, &s->palette))
            return false;
        break;
    case 3:
        if (!FirePalette(in.overlay, in.game_timer, s->spell_child_palette, &s->palette))
            return false;
        s->scale_x = Word(s->scale_x - 0x10);
        s->scale_y = s->scale_x;
        if (s->scale_x <= 0)
        {
            s->scale_x = 0;
            s->timeline = 0xd24;
            s->pose = 0;
            s->remaining = 0;
            s->flags |= 0x100000U;
            s->spell_timer = 0x20;
            s->palette = s->spell_child_palette;
            s->priority = 0x1b6;
            e->sounds[0] = 0x668;
            s->phase = 4;
        }
        break;
    case 4:
        s->spell_glow = Word(s->spell_glow + 8);
        s->scale_x = Word(s->scale_x + 0x10);
        if (s->scale_x >= 0x100)
            s->scale_x = 0x100;
        s->scale_y = s->scale_x;
        if (q->blue[3] >= 9)
            q->blue[3] = (Uint8)(q->blue[3] - 4);
        SpellColors(q, q->blue[3]);
        if (--s->spell_timer == 0)
        {
            q->draw_mode |= 8;
            e->sounds[0] = 0x660;
            // FACTORY(102)'s three dragons with the combo's <<14 hand bits (dropped without a
            // free factory entity).
            if (in.free_factories > e->factory_count)
            {
                e->factories[e->factory_count] = 102U | ((in.hand + 1U) << 14);
                e->factory_x[e->factory_count] = s->x;
                e->factory_y[e->factory_count++] = s->y;
            }
            s->spell_timer = 0x18;
            e->freeze_clear = true;
            s->phase = 5;
        }
        break;
    case 5:
        if (--s->spell_timer == 0)
            s->phase = 6;
        break;
    default:
        s->scale_x = Word(s->scale_x - 0x10);
        s->scale_y = s->scale_x;
        if (s->scale_x <= 0)
        {
            s->removed = true;
            return true;
        }
        break;
    }
    const Sint16 x = Word(Pixel(s->x) - in.retained_camera_x),
                 y = Word(Pixel(s->y) - in.retained_camera_y);
    q->x[0] = q->x[2] = Word(x - s->spell_glow);
    q->x[1] = q->x[3] = Word(x + s->spell_glow - 1);
    q->y[0] = q->y[1] = Word(y - s->spell_glow);
    q->y[2] = q->y[3] = Word(y + s->spell_glow - 1);
    return true;
}
// One dragon edge: centre plus ((trig>>4)*size)>>8, Y subtracted.
static bool FireEdge(SotnDataView exe, Sint16 cx, Sint16 cy, Sint32 angle, Sint16 size, Sint16 *x,
                     Sint16 *y)
{
    SotnMathTrig t = {0};
    if (!SotnMath_Trig(exe, angle, &t))
        return false;
    *x = Word(cx + Asr(Asr(t.cosine, 4) * size, 8));
    *y = Word(cy - Asr(Asr(t.sine, 4) * size, 8));
    return true;
}
// w_029 func_ptr_80170024 (FACTORY(102), role9): one of three fire dragons (params0..2). It takes
// nineteen GT4s (failure retires it), resets its program-BSS row of twenty screen 16.16 segments
// at its position, heading 0x540 per dragon (0x800 more facing left), rolls properties from its
// raw parent's equipId and strikes with a 20x20 hitbox. Segments wake two updates apart, swing
// with their wave, curl, slow (dragon0's FIREBALL_SHOT_A as each turns), then speed off and stop
// turning off screen; the first to pass speed0x100 retires the dragon. Rows cycle D_8017AD10's
// palettes on g_Timer; the head rides segment1, whose screen position the entity takes before
// requesting FACTORY(100)'s hit point every update.
static bool FireDragon(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                       SotnWeaponSecondaryEvents *e)
{
    const Uint16 dragon = s->parameter & 0xffU;
    const bool left = s->facing_left;
    SotnPlayerEffectQuad *q = s->fire_quads;
    SotnWeaponFireSegment *g = s->fire_segments;
    switch (s->phase)
    {
    case 0: {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 9;
        s->allocated = true;
        s->fire_palette = in.hand ? 0x128 : 0x110;
        s->fire_band = in.hand ? 0x80 : 0;
        s->flags |= 0x04800000U;
        s->fire_head = 0x40;
        for (Uint16 i = 0; i < 19; ++i)
        {
            q[i] =
                (SotnPlayerEffectQuad){.page = 0x19, .palette = s->fire_palette, .priority = 0x1ba};
            q[i].u[0] = q[i].u[2] = 0x50;
            q[i].v[0] = q[i].v[1] = (Uint8)(s->fire_band + 8);
            q[i].u[1] = q[i].u[3] = 0x7f;
            q[i].v[2] = q[i].v[3] = (Uint8)(s->fire_band + 0x2f);
        }
        q[0].u[0] = q[0].u[1] = 0;
        q[0].u[2] = q[0].u[3] = 0x57;
        q[0].v[0] = q[0].v[2] = (Uint8)(s->fire_band + 0x30);
        q[0].v[1] = q[0].v[3] = (Uint8)(s->fire_band + 0x6f);
        const Sint32 sx = (Sint32)(((Uint32)Word(Pixel(s->x) - in.retained_camera_x) << 16) |
                                   ((Uint32)s->x & 0xffffU)),
                     sy = (Sint32)(((Uint32)Word(Pixel(s->y) - in.retained_camera_y) << 16) |
                                   ((Uint32)s->y & 0xffffU));
        for (Uint16 i = 0; i < 20; ++i)
            g[i] = (SotnWeaponFireSegment){.x = sx,
                                           .y = sy,
                                           .heading = Word((left ? 0x800 : 0) + dragon * 0x540),
                                           .turn = 0xc00,
                                           .speed = 0x70,
                                           .wave = 0xc00,
                                           .width = i == 19   ? 0
                                                    : i == 18 ? 10
                                                              : 20,
                                           .delay = (Sint16)(i * 2 + 1),
                                           .swell = 256};
        if (!Properties(s, in, rng, e))
            return false;
        s->hitbox_width = s->hitbox_height = 20;
        s->phase = 1;
        break;
    }
    default:
        for (Uint16 i = 0; i < 20; ++i)
        {
            SotnWeaponFireSegment *k = &g[i];
            if (k->state)
            {
                SotnMathTrig wave = {0}, head = {0};
                k->wave = Word(k->wave + 0x80);
                k->swell = Word(k->swell - 4);
                if (!SotnMath_Trig(in.executable, k->wave, &wave) ||
                    !SotnMath_Trig(in.executable, k->heading, &head))
                    return false;
                const Sint32 swing = Word(wave.sine / 2 + 0x1000);
                if (k->swell < 0)
                    k->swell = 0;
                k->x = (Sint32)((Uint32)k->x + (Uint32)(Asr(head.cosine * k->speed, 12) * swing));
                k->y = (Sint32)((Uint32)k->y + (Uint32)(-Asr(head.sine * k->speed, 12) * swing));
                k->heading = Word(k->heading + (left ? -1 : 1) * Asr((Sint16)k->turn, 4));
            }
            switch (k->state)
            {
            case 0:
                if (--k->delay == 0)
                    ++k->state;
                break;
            case 1:
                k->turn = (Uint16)(k->turn - 40U);
                if (k->wave > 0x1400)
                    ++k->state;
                break;
            case 2:
                k->turn = (Uint16)(k->turn - 80U);
                k->speed = Word(k->speed - 4);
                if (k->speed < 0x40)
                {
                    if (!(s->parameter & 0x7fU))
                        e->sounds[e->sounds[0] ? 1 : 0] = 0x660;
                    ++k->state;
                }
                break;
            case 3:
                k->wave = Word(k->wave - 0x60);
                k->turn = (Uint16)(k->turn - 48U);
                if (k->x < -288 * 0x10000 || k->x > 288 * 0x10000 || k->y < 0 ||
                    k->y > 256 * 0x10000)
                    k->turn = 0;
                k->speed = Word(k->speed + 2);
                if (k->speed > 0x100)
                {
                    s->removed = true;
                    return true;
                }
                break;
            }
        }
        s->fire_head = Word(s->fire_head + 2);
        if (s->fire_head > 0x100)
            s->fire_head = 0x100;
        break;
    }
    const Uint8 *offsets = NULL;
    if (!View(in.overlay, 0xd10U + (in.engine_timer / 2U % 10U) * 2U, 2, &offsets))
        return SDL_SetError("Fire shield dragon private palette offsets");
    Sint16 x2 = 0, y2 = 0, x3 = 0, y3 = 0;
    for (Uint16 i = 0; i < 19; ++i)
    {
        q[i].palette = (Uint16)(s->fire_palette + Read16(offsets));
        q[i].x[0] = x2;
        q[i].y[0] = y2;
        q[i].x[1] = x3;
        q[i].y[1] = y3;
        const SotnWeaponFireSegment *k = &g[i + 1];
        SotnMathTrig wave = {0};
        if (!SotnMath_Trig(in.executable, k->wave, &wave))
            return false;
        const Sint16 size = Word(k->width * (wave.sine * k->swell / 256 + 4096) / 4096);
        const Sint16 cx = Pixel(k->x), cy = Pixel(k->y);
        if (!FireEdge(in.executable, cx, cy, k->heading - 0x400, size, &x2, &y2) ||
            !FireEdge(in.executable, cx, cy, k->heading + 0x400, size, &x3, &y3))
            return false;
        q[i].x[2] = x2;
        q[i].y[2] = y2;
        q[i].x[3] = x3;
        q[i].y[3] = y3;
    }
    SotnMathTrig wave = {0};
    if (!SotnMath_Trig(in.executable, g[0].wave, &wave))
        return false;
    const Sint16 size =
        Word((s->fire_head * 54 / 256) * (wave.sine * g[0].swell / 256 + 4096) / 4096);
    const Sint16 nx = Pixel(g[1].x), ny = Pixel(g[1].y);
    if (!FireEdge(in.executable, nx, ny, g[0].heading - 0x199, size, &q[0].x[0], &q[0].y[0]) ||
        !FireEdge(in.executable, nx, ny, g[0].heading + 0x199, size, &q[0].x[1], &q[0].y[1]) ||
        !FireEdge(in.executable, nx, ny, g[0].heading + 0x999, size, &q[0].x[2], &q[0].y[2]) ||
        !FireEdge(in.executable, nx, ny, g[0].heading + 0x667, size, &q[0].x[3], &q[0].y[3]))
        return false;
    SetScreen(&s->x, nx, in.retained_camera_x);
    SetScreen(&s->y, ny, in.retained_camera_y);
    if (in.free_factories > e->factory_count)
    {
        e->factories[e->factory_count] = 100U | ((in.hand + 1U) << 14);
        e->factory_x[e->factory_count] = s->x;
        e->factory_y[e->factory_count++] = s->y;
    }
    return true;
}
// w_029 func_ptr_80170028 (FACTORY(100), role10): a dragon's hit point. It retires once its raw
// parent's slot empties; at birth it rolls properties from the parent's equipId with a 14x14
// hitbox, then lives ten more updates.
static bool FireHit(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                    SotnWeaponSecondaryEvents *e)
{
    if (!in.parent_exists)
    {
        s->removed = true;
        return true;
    }
    if (!s->phase)
    {
        s->role = 10;
        s->allocated = true;
        if (!Properties(s, in, rng, e))
            return false;
        s->hitbox_width = s->hitbox_height = 14;
        s->spell_timer = 10;
        s->phase = 1;
    }
    else if (--s->spell_timer == 0)
        s->removed = true;
    return true;
}
// w_011 EntityWeaponShieldSpell: Axe Lord shield's both-buttons spell (item206, ten mandatory
// GT4s). Birth resets the program's sixteen band angles to (i<<9)*2; the shield grows, flashes
// out with MAGIC_WEAPON_APPEAR_A, eight green/blue bands narrow onto a 16-wide card, then the
// spell hides, ends the freeze itself and creates the blueprint88 armor walker before fading.
static bool AxeLordSpell(SotnWeaponSecondary *s, SotnHandWeaponProgram *p,
                         SotnWeaponSecondaryInput in, SotnWeaponSecondaryEvents *e)
{
    switch (s->phase)
    {
    case 0: {
        for (Uint16 i = 0; i < 16; ++i)
            p->shield_angles[i] = (Sint32)((Uint32)i << 10);
        p->shield_angles_loaded = true;
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 8;
        s->allocated = true;
        s->bank = in.hand ? 0x8012 : 0x8010;
        s->palette = in.hand ? 0x128 : 0x110;
        s->spell_child_palette = in.hand ? 0x129 : 0x111;
        s->texture = in.hand ? 0x66 : 0x64;
        s->spell_v0 = in.hand ? 0 : 0x80;
        s->spell_v1 = in.hand ? 0x80 : 0;
        s->y = (Sint32)((Uint32)s->y - 0x80000U);
        s->flags = 0x04810000U;
        s->priority = (Uint16)(in.priority - 2U);
        s->facing_left = in.facing_left;
        s->frame = 0x3e;
        s->draw_flags = 3;
        s->scale_x = s->scale_y = 0;
        SotnPlayerEffectQuad *q = &s->spell_quads[0];
        *q = (SotnPlayerEffectQuad){.palette = 0x19f, .page = 0x19};
        q->u[0] = q->u[2] = 0x80;
        q->u[1] = q->u[3] = 0x80 + 0x4f;
        q->v[0] = q->v[1] = s->spell_v0;
        q->v[2] = q->v[3] = (Uint8)(s->spell_v0 + 0x4f);
        s->spell_glow = 0;
        SpellColors(q, 0x80);
        q->priority = (Uint16)(s->priority - 4U);
        q->draw_mode = 0x40 | 0x20 | 0x10 | 0x04 | 0x01;
        q = &s->spell_quads[1];
        *q = (SotnPlayerEffectQuad){.palette = s->spell_child_palette, .page = 0x19};
        q->u[0] = q->u[2] = 0;
        q->u[1] = q->u[3] = 0x1f;
        q->v[0] = q->v[1] = (Uint8)(s->spell_v1 + 0x30);
        q->v[2] = q->v[3] = (Uint8)(s->spell_v1 + 0x6f);
        SpellColors(q, 0x80);
        q->priority = 0x1bc;
        q->draw_mode = 0x08;
        for (Uint16 i = 0; i < 8; ++i)
        {
            q = &s->spell_quads[2 + i];
            *q = s->spell_quads[1 + i];
            q->v[0] = q->v[1] = (Uint8)(s->spell_v1 + 0x30 + i * 64 / 8);
            q->v[2] = q->v[3] = (Uint8)(s->spell_v1 + 0x30 + (i + 1) * 64 / 8 - 1);
            q->palette = 0x15f;
            q->priority = 0x1be;
            q->draw_mode |= 0x20 | 0x10 | 0x04 | 0x01;
        }
        s->spell_wave_x = 0x120;
        s->spell_wave_y = 0;
        s->spell_half_w = 0x10;
        s->spell_half_h = 0;
        s->spell_step = 0x40;
        s->spell_bright = 0x100;
        s->velocity_x = s->facing_left ? 0x20000 : -0x20000;
        s->spell_velocity_y = 0;
        e->clear_shield = true;
        e->sounds[0] = 0x641;
        e->freeze_set = true;
        s->phase = 1;
        break;
    }
    case 1:
        s->velocity_x = s->velocity_x < 0 ? SDL_min(s->velocity_x + 0x1000, 0)
                                          : SDL_max(s->velocity_x - 0x1000, 0);
        s->spell_velocity_y -= 0x2800;
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->spell_velocity_y);
        s->scale_x = Word(s->scale_x + 12);
        s->scale_y = s->scale_x;
        s->spell_glow = (Sint16)(s->scale_x * 0x28 / 256);
        if (s->scale_x >= 0x100)
        {
            s->spell_glow = 0x28;
            s->scale_y = s->scale_x = 0x100;
            s->spell_timer = 8;
            s->phase = 2;
        }
        break;
    case 2:
        if (--s->spell_timer == 0)
        {
            s->spell_timer = 0x10;
            s->phase = 3;
        }
        break;
    case 3:
        s->spell_quads[0].priority = !(s->spell_timer & 1) ? 0x1bc : (Uint16)(s->priority - 2U);
        s->scale_x = Word(s->scale_x - 0x10);
        if (s->scale_x <= 0)
            s->scale_x = 0;
        s->scale_y = s->scale_x;
        if (--s->spell_timer == 0)
        {
            s->frame = 0;
            for (Uint16 i = 2; i < 10; ++i)
                s->spell_quads[i].draw_mode &= 0xfff7U;
            s->spell_timer = 0x20;
            e->sounds[0] = 0x669;
            s->phase = 4;
        }
        break;
    case 4: {
        s->spell_glow = Word(s->spell_glow + 8);
        SotnPlayerEffectQuad *q = &s->spell_quads[0];
        if (q->blue[3] > 8)
            q->blue[3] = (Uint8)(q->blue[3] - 4);
        SpellColors(q, q->blue[3]);
        s->scale_x = Word(s->scale_x + 0x10);
        if (s->scale_x >= 0x100)
            s->scale_x = 0x100;
        s->spell_half_h = (Sint16)(s->scale_x * 32 / 256);
        s->spell_wave_x = Word(s->spell_wave_x - 12);
        if (s->spell_wave_x <= 0)
        {
            s->spell_wave_x = 0;
            s->spell_quads[1].draw_mode &= 0xfff7U;
            s->phase = 5;
        }
        break;
    }
    case 5:
        s->spell_bright = Word(s->spell_bright - 0x10);
        if (s->spell_bright < 0)
        {
            s->spell_quads[0].draw_mode |= 8;
            s->spell_bright = 0;
            s->spell_quads[1].draw_mode |= 8;
            // Blueprint88: the shield program's vector9 walker, factory-created with <<14 bits.
            if (in.free_factories && e->factory_count < SDL_arraysize(e->factories))
            {
                e->factories[e->factory_count] = 88U + ((in.hand + 1U) << 14);
                e->factory_x[e->factory_count] = s->x;
                e->factory_y[e->factory_count] = s->y;
                ++e->factory_count;
            }
            e->freeze_clear = true;
            s->phase = 6;
        }
        break;
    case 6:
        s->spell_bright = Word(s->spell_bright + 8);
        if (s->spell_bright >= 0x80)
        {
            s->spell_timer = 4;
            s->phase = 7;
        }
        break;
    default:
        if (--s->spell_timer == 0)
        {
            s->removed = true;
            return true;
        }
        break;
    }
    const Sint16 x = Word(Pixel(s->x) - in.camera_x), y = Word(Pixel(s->y) - in.camera_y);
    SotnPlayerEffectQuad *q = &s->spell_quads[0];
    if (q->blue[3] > 8)
    {
        if (in.engine_timer & 1U)
            q->draw_mode |= 8;
        else
            q->draw_mode &= 0xfff7U;
    }
    q->x[0] = q->x[2] = Word(x - s->spell_glow);
    q->x[1] = q->x[3] = Word(x + s->spell_glow - 1);
    q->y[0] = q->y[1] = Word(y - s->spell_glow);
    q->y[2] = q->y[3] = Word(y + s->spell_glow - 1);
    q = &s->spell_quads[1];
    Sint16 top = q->y[0] = q->y[1] = Word(y - s->spell_half_h);
    Sint16 bottom = q->y[2] = q->y[3] = Word(y + s->spell_half_h - 1);
    Sint16 back = 0, front = 0;
    if (!s->facing_left)
    {
        back = q->x[0] = q->x[2] = Word(x - s->spell_half_w);
        front = q->x[1] = q->x[3] = Word(x + s->spell_half_w - 1);
    }
    else
    {
        front = q->x[1] = q->x[3] = Word(x - s->spell_half_w);
        back = q->x[0] = q->x[2] = Word(x + s->spell_half_w - 1);
    }
    front = Word(front + 1);
    bottom = Word(bottom + 1);
    for (Uint16 i = 0; i < 8; ++i)
    {
        q = &s->spell_quads[2 + i];
        Sint32 sine[4] = {0};
        const Uint16 at[4] = {i, (Uint16)((i + 1) % 8), (Uint16)((i + 6) % 8),
                              (Uint16)((i + 7) % 8)};
        for (Uint16 k = 0; k < 4; ++k)
        {
            SotnMathTrig t = {0};
            if (!SotnMath_Trig(in.executable, (Sint16)p->shield_angles[at[k]], &t))
                return false;
            sine[k] = t.sine;
        }
        Sint16 base = (Sint16)(Asr(sine[0], 5) * s->spell_wave_x / 256);
        q->x[0] = Word(back + base);
        q->x[1] = Word(front + base - 1);
        base = (Sint16)(Asr(sine[1], 5) * s->spell_wave_x / 256);
        q->x[2] = Word(back + base);
        q->x[3] = Word(front + base - 1);
        q->y[0] = q->y[1] = Word(top + ((bottom - top) * i) / 8);
        q->y[2] = q->y[3] = Word(top + ((bottom - top) * (i + 1)) / 8);
        base = (Sint16)(Asr(sine[2], 5) * s->spell_wave_y / 256);
        q->y[0] = Word(q->y[0] + base);
        q->y[2] = Word(q->y[2] + base);
        base = (Sint16)(Asr(sine[3], 5) * s->spell_wave_y / 256);
        q->y[1] = Word(q->y[1] + base);
        q->y[3] = Word(q->y[3] + base);
        const Uint8 c0 = (Uint8)(((sine[0] + 0x1000) >> 6) * s->spell_bright / 256),
                    c1 = (Uint8)(((sine[1] + 0x1000) >> 6) * s->spell_bright / 256);
        q->green[0] = q->blue[0] = q->green[1] = q->blue[1] = c0;
        q->red[0] = q->red[1] = 0;
        // The original writes b3 twice; the second (next angle) value remains.
        q->green[2] = q->blue[2] = q->green[3] = q->blue[3] = c1;
        q->red[2] = q->red[3] = 0;
        p->shield_angles[i] = (Sint32)((Uint32)p->shield_angles[i] + (Uint32)s->spell_step);
    }
    return true;
}
// w_025 EntityWeaponShieldSpell: Goddess shield's both-buttons spell (item209, seventeen
// mandatory GT4s). Its fifteen band angles stay in the program (D_8017AA98, overlay data loaded
// once). The shield grows, holds and flashes out with TRANSFORM_3X while every tenth timer value
// requests a FACTORY(90,1) cloud; the card narrows to 16 wide as its wave decays, darkens, waits
// 0x40 updates, releases the FACTORY(55,15) pink effect, brightens, and the bands spread and fade
// before the spell ends the freeze and retires.
static bool GoddessSpell(SotnWeaponSecondary *s, SotnHandWeaponProgram *p,
                         SotnWeaponSecondaryInput in, SotnWeaponSecondaryEvents *e)
{
    if (!p->shield_angles_loaded)
    {
        const Uint8 *a = NULL;
        if (!View(in.overlay, 0xa98, 60, &a))
            return false;
        for (Uint16 i = 0; i < 15; ++i)
            p->shield_angles[i] = (Sint32)Read32(a + i * 4U);
        p->shield_angles_loaded = true;
    }
    bool cloud = false;
    switch (s->phase)
    {
    case 0: {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 8;
        s->allocated = true;
        s->bank = in.hand ? 0x8012 : 0x8010;
        s->palette = in.hand ? 0x128 : 0x110;
        s->spell_child_palette = in.hand ? 0x129 : 0x111;
        s->texture = in.hand ? 0x66 : 0x64;
        s->spell_v0 = in.hand ? 0 : 0x80;
        s->spell_v1 = in.hand ? 0x80 : 0;
        s->y = (Sint32)((Uint32)s->y - 0x80000U);
        s->flags = 0x04810000U;
        s->priority = (Uint16)(in.priority - 2U);
        s->facing_left = in.facing_left;
        s->frame = 0x3e;
        s->draw_flags = 3;
        s->scale_x = s->scale_y = 0;
        SotnPlayerEffectQuad *q = &s->spell_quads[0];
        *q = (SotnPlayerEffectQuad){.palette = 0x19f, .page = 0x19};
        q->u[0] = q->u[2] = 0x80;
        q->u[1] = q->u[3] = 0x80 + 0x4f;
        q->v[0] = q->v[1] = s->spell_v0;
        q->v[2] = q->v[3] = (Uint8)(s->spell_v0 + 0x4f);
        s->spell_glow = 0;
        SpellColors(q, 0x80);
        q->priority = (Uint16)(s->priority - 4U);
        q->draw_mode = 0x40 | 0x20 | 0x10 | 0x04 | 0x01;
        q = &s->spell_quads[1];
        *q = (SotnPlayerEffectQuad){.palette = s->spell_child_palette, .page = 0x19};
        q->u[0] = q->u[2] = 0x60;
        q->u[1] = q->u[3] = 0x60 + 0x1f;
        q->v[0] = q->v[1] = s->spell_v1;
        q->v[2] = q->v[3] = (Uint8)(s->spell_v1 + 0x77);
        SpellColors(q, 0x80);
        q->priority = 0x1b8;
        q->draw_mode = 0x08;
        for (Uint16 i = 0; i < 15; ++i)
        {
            q = &s->spell_quads[2 + i];
            *q = s->spell_quads[1 + i];
            q->v[0] = q->v[1] = (Uint8)(s->spell_v1 + i * 120 / 15);
            q->v[2] = q->v[3] = (Uint8)(s->spell_v1 + (i + 1) * 120 / 15 - 1);
            q->palette = 0x15f;
            q->priority = 0x1ba;
            q->draw_mode |= 0x20 | 0x10 | 0x04 | 0x01;
        }
        s->spell_step = 0x80;
        s->spell_bright = 0x100;
        s->spell_half_w = 0;
        s->spell_half_h = 0x78;
        s->spell_wave_x = 0x20;
        s->spell_wave_y = 0;
        s->velocity_x = s->facing_left ? 0x20000 : -0x20000;
        s->spell_velocity_y = 0;
        e->clear_shield = true;
        e->sounds[0] = 0x641;
        e->freeze_set = true;
        s->phase = 1;
        break;
    }
    case 1:
        s->velocity_x = s->velocity_x < 0 ? SDL_min(s->velocity_x + 0x1000, 0)
                                          : SDL_max(s->velocity_x - 0x1000, 0);
        s->spell_velocity_y -= 0x2800;
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->spell_velocity_y);
        s->scale_x = Word(s->scale_x + 12);
        s->scale_y = s->scale_x;
        s->spell_glow = (Sint16)(s->scale_x * 0x28 / 256);
        if (s->scale_x >= 0x100)
        {
            s->spell_glow = 0x28;
            s->scale_y = s->scale_x = 0x100;
            s->spell_timer = 8;
            s->phase = 2;
        }
        break;
    case 2:
        if (--s->spell_timer == 0)
        {
            s->spell_timer = 0x10;
            s->phase = 3;
        }
        cloud = !(s->spell_timer % 10);
        break;
    case 3:
        s->spell_quads[0].priority = !(s->spell_timer & 1) ? 0x1b8 : (Uint16)(s->priority - 2U);
        s->scale_x = Word(s->scale_x - 0x10);
        if (s->scale_x <= 0)
            s->scale_x = 0;
        s->scale_y = s->scale_x;
        if (--s->spell_timer == 0)
        {
            s->frame = 0;
            for (Uint16 i = 2; i < 17; ++i)
                s->spell_quads[i].draw_mode &= 0xfff7U;
            s->spell_timer = 0;
            e->sounds[0] = 0x6b1;
            s->phase = 4;
        }
        cloud = !(s->spell_timer % 10);
        break;
    case 4: {
        s->spell_glow = Word(s->spell_glow + 8);
        s->scale_x = Word(s->scale_x + 12);
        if (s->scale_x >= 0x100)
            s->scale_x = 0x100;
        s->spell_half_w = (Sint16)(s->scale_x * 16 / 256);
        s->spell_half_h = (Sint16)((0x100 - s->scale_x) * 60 / 256 + 60);
        if (!(in.engine_timer & 7U))
            s->spell_wave_x = Word(s->spell_wave_x - 1);
        if (s->spell_wave_x < 0)
            s->spell_wave_x = 0;
        SotnPlayerEffectQuad *q = &s->spell_quads[0];
        if (q->blue[3] > 8)
            q->blue[3] = (Uint8)(q->blue[3] - 4);
        SpellColors(q, q->blue[3]);
        if (!(in.engine_timer & 1U))
            s->spell_wave_x = Word(s->spell_wave_x - 1);
        if (s->spell_wave_x < 0)
            s->spell_wave_x = 0;
        if (s->scale_x == 0x100 && !s->spell_wave_x)
        {
            s->spell_quads[1].draw_mode &= 0xfff7U;
            s->phase = 5;
        }
        s->spell_timer = Word(s->spell_timer + 1);
        cloud = !(s->spell_timer % 10);
        break;
    }
    case 5:
        s->spell_bright = Word(s->spell_bright - 10);
        if (s->spell_bright < 0)
        {
            s->spell_bright = 0;
            s->spell_timer = 0;
            s->phase = 6;
        }
        break;
    case 6:
        s->spell_timer = Word(s->spell_timer + 1);
        if (s->spell_timer == 0x40)
        {
            s->phase = 7;
            e->pink = 15;
        }
        break;
    case 7:
        s->spell_bright = Word(s->spell_bright + 0x10);
        if (s->spell_bright > 0x120)
        {
            s->spell_quads[1].draw_mode |= 8;
            s->spell_timer = 0x10;
            s->spell_step = 0x20;
            s->phase = 8;
        }
        break;
    case 8:
        s->spell_wave_x = Word(s->spell_wave_x + 1);
        s->spell_half_h = Word(s->spell_half_h + 2);
        if (s->spell_half_h < 0)
            s->spell_half_h = 0;
        s->spell_bright = Word(s->spell_bright - 0x10);
        if (s->spell_bright < 0)
            s->spell_bright = 0;
        if (--s->spell_timer == 0)
        {
            s->spell_timer = 0x40;
            s->phase = 9;
        }
        break;
    default:
        s->spell_bright = Word(s->spell_bright - 0x10);
        if (s->spell_bright < 0)
            s->spell_bright = 0;
        if (--s->spell_timer == 0)
        {
            e->freeze_clear = true;
            s->removed = true;
            return true;
        }
        break;
    }
    // FACTORY(90,1) with the combo's <<14 hand bits: the vector9 cloud.
    if (cloud && in.free_factories && e->factory_count < SDL_arraysize(e->factories))
    {
        e->factories[e->factory_count] = (90U + ((in.hand + 1U) << 14)) | 1U << 16;
        e->factory_x[e->factory_count] = s->x;
        e->factory_y[e->factory_count] = s->y;
        ++e->factory_count;
    }
    const Sint16 x = Word(Pixel(s->x) - in.camera_x), y = Word(Pixel(s->y) - in.camera_y);
    SotnPlayerEffectQuad *q = &s->spell_quads[0];
    if (q->blue[3] > 8)
    {
        if (in.engine_timer & 1U)
            q->draw_mode |= 8;
        else
            q->draw_mode &= 0xfff7U;
    }
    q->x[0] = q->x[2] = Word(x - s->spell_glow);
    q->x[1] = q->x[3] = Word(x + s->spell_glow - 1);
    q->y[0] = q->y[1] = Word(y - s->spell_glow);
    q->y[2] = q->y[3] = Word(y + s->spell_glow - 1);
    q = &s->spell_quads[1];
    const Sint16 top = q->y[0] = q->y[1] = Word(y - s->spell_half_h);
    Sint16 bottom = q->y[2] = q->y[3] = Word(y + s->spell_half_h - 1);
    Sint16 back = 0, front = 0;
    if (!s->facing_left)
    {
        back = q->x[0] = q->x[2] = Word(x - s->spell_half_w);
        front = q->x[1] = q->x[3] = Word(x + s->spell_half_w - 1);
    }
    else
    {
        front = q->x[1] = q->x[3] = Word(x - s->spell_half_w);
        back = q->x[0] = q->x[2] = Word(x + s->spell_half_w - 1);
    }
    front = Word(front + 1);
    bottom = Word(bottom + 1);
    for (Uint16 i = 0; i < 15; ++i)
    {
        q = &s->spell_quads[2 + i];
        Sint32 sine[4] = {0};
        const Uint16 at[4] = {i, (Uint16)((i + 1) % 15), (Uint16)((i + 6) % 15),
                              (Uint16)((i + 1) % 15)};
        for (Uint16 k = 0; k < 4; ++k)
        {
            SotnMathTrig t = {0};
            if (!SotnMath_Trig(in.executable, (Sint16)p->shield_angles[at[k]], &t))
                return false;
            sine[k] = t.sine;
        }
        Sint16 base = (Sint16)(Asr(sine[0], 5) * s->spell_wave_x / 256);
        q->x[0] = Word(back + base);
        q->x[1] = Word(front + base - 1);
        base = (Sint16)(Asr(sine[1], 5) * s->spell_wave_x / 256);
        q->x[2] = Word(back + base);
        q->x[3] = Word(front + base - 1);
        base = (Sint16)(Asr(sine[2], 5) * s->spell_wave_y / 256);
        q->y[0] = Word(top + ((bottom - top) * i) / 15 - base);
        q->y[2] = Word(top + ((bottom - top) * (i + 1)) / 15 - base);
        base = (Sint16)(Asr(sine[3], 5) * s->spell_wave_y / 256);
        q->y[1] = Word(top + (i * (bottom - top)) / 15 - base);
        q->y[3] = Word(top + ((bottom - top) * (i + 1)) / 15 - base);
        const Uint8 c0 = (Uint8)(((sine[0] + 0x1000) >> 6) * s->spell_bright / 256),
                    c1 = (Uint8)(((sine[1] + 0x1000) >> 6) * s->spell_bright / 256);
        q->red[0] = q->green[0] = q->blue[0] = q->red[1] = q->green[1] = q->blue[1] = c0;
        q->red[2] = q->green[2] = q->blue[2] = q->red[3] = q->green[3] = q->blue[3] = c1;
        p->shield_angles[i] = (Sint32)((Uint32)p->shield_angles[i] + (Uint32)s->spell_step);
    }
    return true;
}
// w_025 func_ptr_80170024 (vector9): FACTORY(90,1)'s cloud (one mandatory GT4, slots32..63).
// Birth places it randomly around its raw parent (six rand calls, after the row), then it
// grows, orbits a point that circles 0x20 above the parent (re-aimed every fourth update by
// ratan2, speed 0.5), spins and, once the parent slot empties, fades out by six per update.
// It lacks CAMERA_LOCKED and runs during the freeze (its factory's 0x10000).
static bool GoddessCloud(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng)
{
    SotnPlayerEffectQuad *q = &s->spell_quads[0];
    switch (s->phase)
    {
    case 0: {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 9;
        s->allocated = true;
        s->spell_child_palette = in.hand ? 0x12a : 0x112;
        s->spell_v1 = in.hand ? 0x80 : 0;
        const Sint16 px = Word(Pixel(in.parent_x) - in.camera_x),
                     py = Word(Pixel(in.parent_y) - in.camera_y);
        SetScreen(&s->x, Word(px + (Sint32)(Rand(rng) % 160U) - 80), in.camera_x);
        SetScreen(&s->y, Word(py + (Sint32)(Rand(rng) % 192U) - 64), in.camera_y);
        s->flags |= 0x04800000U;
        s->facing_left = in.facing_left;
        s->scale_x = s->scale_y = 0;
        s->rotate = 0;
        *q = (SotnPlayerEffectQuad){.palette = s->spell_child_palette, .page = 0x19};
        q->u[0] = q->u[2] = 0;
        q->u[1] = q->u[3] = 0x5f;
        q->v[0] = q->v[1] = (Uint8)(s->spell_v1 + 0x28);
        q->v[2] = q->v[3] = (Uint8)(s->spell_v1 + 0x28 + 0x57);
        SpellColors(q, 0x80);
        q->priority = (Rand(rng) & 3U) ? 0x1bc : 0x1b6;
        q->draw_mode = 0x20 | 0x10 | 0x04 | 0x01;
        s->spell_timer = (Sint16)((Rand(rng) & 7U) + 8U);
        s->cloud_spin = (Sint16)((Sint32)(Rand(rng) % 16U) - 8);
        if (s->cloud_spin >= 0)
            ++s->cloud_spin;
        s->cloud_angle = (Sint16)Rand(rng);
        s->phase = 1;
        break;
    }
    case 1:
        s->scale_x = Word(s->scale_x + 8);
        s->scale_y = Word(s->scale_y + 8);
        if (!s->spell_timer)
        {
            s->spell_timer = (Sint16)((Rand(rng) & 0x3fU) + 0x60U);
            s->phase = 2;
        }
        break;
    case 2:
        s->scale_x = Word(s->scale_x + 2);
        s->scale_y = Word(s->scale_y + 2);
        if (!s->spell_timer)
            s->phase = 3;
        break;
    default:
        if (q->blue[3] >= 6)
            q->blue[3] = (Uint8)(q->blue[3] - 6);
        if (q->blue[3] < 7)
        {
            s->removed = true;
            return true;
        }
        SpellColors(q, q->blue[3]);
        break;
    }
    s->spell_timer = Word(s->spell_timer - 1);
    s->cloud_angle = Word(s->cloud_angle + 0x40);
    if (!(s->spell_timer & 3))
    {
        if (in.parent_exists)
        {
            SotnMathTrig t = {0};
            if (!SotnMath_Trig(in.executable, s->cloud_angle, &t))
                return false;
            Sint16 tx = Word(Pixel(in.parent_x) - in.camera_x),
                   ty = Word(Word(Pixel(in.parent_y) - in.camera_y) - 0x20);
            ty = Word(ty - Asr(Asr(t.sine, 4) * 64, 8));
            tx = Word(tx + Asr(Asr(t.cosine, 4) * 48, 8));
            const Sint16 sx = Word(Pixel(s->x) - in.camera_x), sy = Word(Pixel(s->y) - in.camera_y);
            Sint16 heading = 0;
            if (!SotnMath_Atan2Vector(in.executable, -(ty - sy), tx - sx, &heading) ||
                !SotnMath_Trig(in.executable, heading, &t))
                return false;
            s->velocity_x = t.cosine * 8;
            s->spell_velocity_y = -t.sine * 8;
        }
        else
        {
            s->velocity_x = s->spell_velocity_y = 0;
            s->phase = 3;
        }
    }
    s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
    s->y = (Sint32)((Uint32)s->y + (Uint32)s->spell_velocity_y);
    s->rotate = Word(s->rotate + s->cloud_spin);
    const Sint16 x = Word(Pixel(s->x) - in.camera_x), y = Word(Pixel(s->y) - in.camera_y);
    const Sint16 w = (Sint16)(s->scale_x * 0x43 / 256), h = (Sint16)(s->scale_y * 0x43 / 256);
    const Sint16 corner[4] = {0x639, 0x1c7, 0x9c7, 0xe39};
    for (Uint16 k = 0; k < 4; ++k)
    {
        SotnMathTrig t = {0};
        if (!SotnMath_Trig(in.executable, Word(s->rotate + corner[k]), &t))
            return false;
        q->x[k] = Word(x + Asr(Asr(t.cosine, 4) * w, 8));
        q->y[k] = Word(y - Asr(Asr(t.sine, 4) * h, 8));
    }
    return true;
}
// w_026 func_BA000_8017B37C: the program's green cycle (D_8017AA98 0..0x1F bouncing on
// D_8017AA9A's parity) rewrites colour13's green of the hand's palette 0x112 (g_Clut[1][0x12D +
// hand*0x180]) and uploads the three weapon CLUT rows; the edit persists until a palette reload.
static void ShamanGreen(SotnHandWeaponProgram *p, SotnWeaponSecondaryInput in,
                        SotnWeaponSecondaryEvents *e)
{
    if (p->shaman_direction & 1)
    {
        p->shaman_green = Word(p->shaman_green + 1);
        if (p->shaman_green >= 0x20)
        {
            p->shaman_direction = Word(p->shaman_direction + 1);
            p->shaman_green = 0x1f;
        }
    }
    else
    {
        p->shaman_green = Word(p->shaman_green - 1);
        if (p->shaman_green < 0)
        {
            p->shaman_direction = Word(p->shaman_direction + 1);
            p->shaman_green = 0;
        }
    }
    const Uint32 index = 0x2dU + in.hand * 0x180U;
    e->clut = true;
    e->clut_palette = (Uint16)(0x110U + index / 16U);
    e->clut_color = (Uint16)(index % 16U);
    e->clut_mask = 0xfc1f;
    e->clut_add = (Uint16)((Uint32)p->shaman_green << 5);
}
// w_026 EntityWeaponShieldSpell: Shaman shield's both-buttons spell (item210, fourteen mandatory
// GT4s). The shield grows, holds and flashes out; then the card and twelve vertical bands
// (red/green/blue phases a third of the circle apart) open, blueprint92 releases eight stars and
// GLASS_SHARDS repeats every 22 counts of the program counter. The card darkens, waits 0x30,
// brightens past 0x120 for the FACTORY(55,6) pink effect, and the bands spread and fade before
// the spell ends the freeze. Every callback advances the program's green palette cycle.
static bool ShamanSpell(SotnWeaponSecondary *s, SotnHandWeaponProgram *p,
                        SotnWeaponSecondaryInput in, SotnWeaponSecondaryEvents *e)
{
    if (!p->shield_angles_loaded)
    {
        const Uint8 *a = NULL;
        if (!View(in.overlay, 0xa98, 52, &a))
            return false;
        p->shaman_green = (Sint16)Read16(a);
        p->shaman_direction = (Sint16)Read16(a + 2);
        for (Uint16 i = 0; i < 12; ++i)
            p->shield_angles[i] = (Sint32)Read32(a + 4 + i * 4U);
        p->shaman_counter = 0;
        p->shield_angles_loaded = true;
    }
    switch (s->phase)
    {
    case 0: {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 8;
        s->allocated = true;
        s->bank = in.hand ? 0x8012 : 0x8010;
        s->palette = in.hand ? 0x128 : 0x110;
        s->spell_child_palette = in.hand ? 0x129 : 0x111;
        s->texture = in.hand ? 0x66 : 0x64;
        s->spell_v0 = in.hand ? 0 : 0x80;
        s->spell_v1 = in.hand ? 0x80 : 0;
        s->y = (Sint32)((Uint32)s->y - 0x80000U);
        s->flags = 0x04810000U;
        s->priority = (Uint16)(in.priority - 2U);
        s->facing_left = in.facing_left;
        s->frame = 0x3e;
        s->draw_flags = 3;
        s->scale_x = s->scale_y = 0;
        SotnPlayerEffectQuad *q = &s->spell_quads[0];
        *q = (SotnPlayerEffectQuad){.palette = 0x19f, .page = 0x19};
        q->u[0] = q->u[2] = 0x80;
        q->u[1] = q->u[3] = 0x80 + 0x4f;
        q->v[0] = q->v[1] = s->spell_v0;
        q->v[2] = q->v[3] = (Uint8)(s->spell_v0 + 0x4f);
        s->spell_glow = 0;
        SpellColors(q, 0x80);
        q->priority = (Uint16)(s->priority - 10U);
        q->draw_mode = 0x40 | 0x20 | 0x10 | 0x04 | 0x01;
        q = &s->spell_quads[1];
        *q = (SotnPlayerEffectQuad){.palette = s->spell_child_palette, .page = 0x19};
        q->u[0] = q->u[1] = 0;
        q->u[2] = q->u[3] = 0x5f;
        q->v[3] = q->v[1] = (Uint8)(s->spell_v1 + 0x48);
        q->v[2] = q->v[0] = (Uint8)(s->spell_v1 + 0x7f);
        SpellColors(q, 0x80);
        q->priority = 0x1b8;
        q->draw_mode = 0x08;
        for (Uint16 i = 0; i < 12; ++i)
        {
            q = &s->spell_quads[2 + i];
            *q = s->spell_quads[1 + i];
            q->u[1] = q->u[0] = (Uint8)(i * 96 / 12);
            q->u[2] = q->u[3] = (Uint8)((i + 1) * 96 / 12 - 1);
            q->palette = 0x15f;
            q->priority = 0x1ba;
            q->draw_mode |= 0x20 | 0x10 | 0x04 | 0x01;
        }
        s->spell_step = 0x80;
        s->spell_bright = 0x100;
        s->spell_wave_x = s->spell_wave_y = 0;
        s->spell_half_w = s->spell_half_h = 0;
        s->velocity_x = s->facing_left ? 0x20000 : -0x20000;
        s->spell_velocity_y = 0;
        e->clear_shield = true;
        e->sounds[0] = 0x641;
        e->freeze_set = true;
        s->phase = 1;
        break;
    }
    case 1:
        s->velocity_x = s->velocity_x < 0 ? SDL_min(s->velocity_x + 0x1000, 0)
                                          : SDL_max(s->velocity_x - 0x1000, 0);
        s->spell_velocity_y -= 0x2800;
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->spell_velocity_y);
        s->scale_x = Word(s->scale_x + 12);
        s->scale_y = s->scale_x;
        s->spell_glow = (Sint16)(s->scale_x * 0x28 / 256);
        if (s->scale_x >= 0x100)
        {
            s->spell_glow = 0x28;
            s->scale_y = s->scale_x = 0x100;
            s->spell_timer = 8;
            s->phase = 2;
        }
        break;
    case 2:
        if (--s->spell_timer == 0)
        {
            s->spell_timer = 0x10;
            s->phase = 3;
        }
        break;
    case 3:
        s->spell_quads[0].priority = !(s->spell_timer & 1) ? 0x1b8 : (Uint16)(s->priority - 2U);
        s->scale_x = Word(s->scale_x - 0x10);
        if (s->scale_x <= 0)
            s->scale_x = 0;
        s->scale_y = s->scale_x;
        if (--s->spell_timer == 0)
        {
            s->frame = 0;
            for (Uint16 i = 1; i < 14; ++i)
                s->spell_quads[i].draw_mode &= 0xfff7U;
            // Blueprint92 (<<14 hand bits): eight vector9 stars.
            if (in.free_factories && e->factory_count < SDL_arraysize(e->factories))
            {
                e->factories[e->factory_count] = 92U + ((in.hand + 1U) << 14);
                e->factory_x[e->factory_count] = s->x;
                e->factory_y[e->factory_count] = s->y;
                ++e->factory_count;
            }
            s->spell_timer = 0;
            e->sounds[0] = 0x6b0;
            p->shaman_counter = 0;
            s->phase = 4;
        }
        break;
    case 4: {
        if (!(p->shaman_counter % 22))
            e->sounds[0] = 0x6b0;
        p->shaman_counter = (Sint32)((Uint32)p->shaman_counter + 1U);
        s->spell_glow = Word(s->spell_glow + 8);
        s->scale_x = Word(s->scale_x + 12);
        if (s->scale_x >= 0x100)
            s->scale_x = 0x100;
        s->spell_half_w = (Sint16)(s->scale_x * 28 / 256);
        s->spell_half_h = (Sint16)(s->scale_x * 48 / 256);
        SotnPlayerEffectQuad *q = &s->spell_quads[0];
        if (q->blue[3] > 8)
            q->blue[3] = (Uint8)(q->blue[3] - 4);
        SpellColors(q, q->blue[3]);
        if (s->scale_x == 0x100)
        {
            q->draw_mode |= 8;
            q->blue[3] = 4;
            s->spell_quads[1].draw_mode &= 0xfff7U;
            s->phase = 5;
        }
        s->spell_timer = Word(s->spell_timer + 1);
        break;
    }
    case 5:
        if (!(p->shaman_counter % 22))
            e->sounds[0] = 0x6b0;
        p->shaman_counter = (Sint32)((Uint32)p->shaman_counter + 1U);
        s->spell_bright = Word(s->spell_bright - 10);
        if (s->spell_bright < 0)
        {
            s->spell_bright = 0;
            s->phase = 6;
            s->spell_timer = 0;
        }
        break;
    case 6:
        if (!(p->shaman_counter % 22))
            e->sounds[0] = 0x6b0;
        p->shaman_counter = (Sint32)((Uint32)p->shaman_counter + 1U);
        s->spell_timer = Word(s->spell_timer + 1);
        if (s->spell_timer == 0x30)
            s->phase = 7;
        break;
    case 7:
        if (!(p->shaman_counter % 22))
            e->sounds[0] = 0x6b0;
        p->shaman_counter = (Sint32)((Uint32)p->shaman_counter + 1U);
        s->spell_bright = Word(s->spell_bright + 4);
        if (s->spell_bright > 0x120)
        {
            s->spell_quads[1].draw_mode |= 8;
            e->pink = 6;
            s->phase = 8;
        }
        break;
    default:
        p->shaman_counter = (Sint32)((Uint32)p->shaman_counter + 1U);
        s->spell_bright = Word(s->spell_bright - 8);
        s->spell_wave_x = Word(s->spell_wave_x + 2);
        s->spell_wave_y = Word(s->spell_wave_y + 2);
        if (s->spell_bright < 0)
        {
            e->freeze_clear = true;
            s->removed = true;
            return true;
        }
        break;
    }
    const Sint16 x = Word(Pixel(s->x) - in.camera_x), y = Word(Pixel(s->y) - in.camera_y);
    SotnPlayerEffectQuad *q = &s->spell_quads[0];
    if (q->blue[3] > 8)
    {
        if (in.engine_timer & 1U)
            q->draw_mode |= 8;
        else
            q->draw_mode &= 0xfff7U;
    }
    q->x[0] = q->x[2] = Word(x - s->spell_glow);
    q->x[1] = q->x[3] = Word(x + s->spell_glow - 1);
    q->y[0] = q->y[1] = Word(y - s->spell_glow);
    q->y[2] = q->y[3] = Word(y + s->spell_glow - 1);
    q = &s->spell_quads[1];
    const Sint16 top = q->y[0] = q->y[1] = Word(y - s->spell_half_h);
    Sint16 bottom = q->y[2] = q->y[3] = Word(y + s->spell_half_h - 1);
    Sint16 back = 0, front = 0;
    if (!s->facing_left)
    {
        back = q->x[0] = q->x[2] = Word(x - s->spell_half_w);
        front = q->x[1] = q->x[3] = Word(x + s->spell_half_w - 1);
    }
    else
    {
        front = q->x[1] = q->x[3] = Word(x - s->spell_half_w);
        back = q->x[0] = q->x[2] = Word(x + s->spell_half_w - 1);
    }
    front = Word(front + 1);
    bottom = Word(bottom + 1);
    // Per band: x0/x1, x2/x3 (wave X), y0/y2, y1/y3 (wave Y), then the six colour phases.
    static const Uint16 offsets[10] = {0, 1, 6, 1, 0, 4, 8, 1, 5, 9};
    for (Uint16 i = 0; i < 12; ++i)
    {
        q = &s->spell_quads[2 + i];
        Sint32 sine[10] = {0};
        for (Uint16 k = 0; k < 10; ++k)
        {
            SotnMathTrig t = {0};
            if (!SotnMath_Trig(in.executable, (Sint16)p->shield_angles[(i + offsets[k]) % 12], &t))
                return false;
            sine[k] = t.sine;
        }
        Sint16 base = (Sint16)(Asr(sine[0], 5) * s->spell_wave_x / 256);
        q->x[0] = Word(back + base);
        q->x[1] = Word(front + base - 1);
        base = (Sint16)(Asr(sine[1], 5) * s->spell_wave_x / 256);
        q->x[2] = Word(back + base);
        q->x[3] = Word(front + base - 1);
        base = (Sint16)(Asr(sine[2], 5) * s->spell_wave_y / 256);
        q->y[0] = Word(top + ((bottom - top) * i) / 12 - base);
        q->y[2] = Word(top + ((bottom - top) * (i + 1)) / 12 - base);
        base = (Sint16)(Asr(sine[3], 5) * s->spell_wave_y / 256);
        q->y[1] = Word(top + (i * (bottom - top)) / 12 - base);
        q->y[3] = Word(top + ((bottom - top) * (i + 1)) / 12 - base);
        Uint8 c[6] = {0};
        for (Uint16 k = 0; k < 6; ++k)
            c[k] = (Uint8)(((sine[4 + k] + 0x1000) >> 6) * s->spell_bright / 256);
        q->red[0] = q->red[1] = c[0];
        q->green[0] = q->green[1] = c[1];
        q->blue[0] = q->blue[1] = c[2];
        q->red[2] = q->red[3] = c[3];
        q->green[2] = q->green[3] = c[4];
        q->blue[2] = q->blue[3] = c[5];
        p->shield_angles[i] = (Sint32)((Uint32)p->shield_angles[i] + (Uint32)s->spell_step);
    }
    ShamanGreen(p, in, e);
    return true;
}
// w_026 func_ptr_80170024 (vector9), params0..7: blueprint92's eight stars. After one GT4 and one
// rand (a 0..14 delay) each requests its FACTORY(90,1) trail, then circles the spell's birth
// point in 3D (RotMatrix about Y advancing by 0x28, RotTransPers4 at depth0x180 with screen0x280)
// while its offsets shrink by six to zero after the delay; once its raw parent spell reaches step8
// it waits rand&15 and flies off (X grows, Y shrinks) until Y passes -0x80. It shows only for
// 0 < OTZ <= 0x1BE, in front (0x1BE) for angles 0x901..0xEFF.
static bool ShamanStar(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                       SotnWeaponSecondaryEvents *e)
{
    SotnPlayerEffectQuad *q = &s->spell_quads[0];
    switch (s->phase)
    {
    case 0: {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        const Uint8 *uv = NULL;
        if (!View(in.overlay, 0xadc, 8, &uv))
            return false;
        s->role = 9;
        s->allocated = true;
        s->flags |= 0x04800000U;
        s->star_offset_x = 0xa0;
        s->star_offset_y = 0x60;
        s->spell_timer = (Sint16)(Rand(rng) % 15U);
        s->star_axis[0] = -0x100;
        s->star_axis[1] = (Sint16)((Uint32)(s->parameter & 0xffU) << 9);
        s->star_axis[2] = 0;
        s->star_spin = 0x28;
        s->star_depth = 0x180;
        const Uint8 offset = in.hand ? 0x80 : 0;
        *q = (SotnPlayerEffectQuad){.palette = in.hand ? 0x12a : 0x112, .page = 0x19};
        for (Uint16 k = 0; k < 4; ++k)
        {
            q->u[k] = uv[k * 2U];
            q->v[k] = (Uint8)(uv[k * 2U + 1U] + offset);
        }
        q->priority = 0;
        q->draw_mode = 0x40 | 0x20 | 0x10 | 0x01;
        if (in.free_factories && e->factory_count < SDL_arraysize(e->factories))
        {
            e->factories[e->factory_count] = (90U + ((in.hand + 1U) << 14)) | 1U << 16;
            e->factory_x[e->factory_count] = s->x;
            e->factory_y[e->factory_count] = s->y;
            ++e->factory_count;
        }
        s->phase = 1;
        break;
    }
    case 1:
        s->star_axis[1] = Word(s->star_axis[1] + s->star_spin);
        if (!s->spell_timer)
        {
            s->star_offset_x = (Sint16)SDL_max(s->star_offset_x - 6, 0);
            s->star_offset_y = (Sint16)SDL_max(s->star_offset_y - 6, 0);
        }
        else
            s->spell_timer = Word(s->spell_timer - 1);
        if (in.parent_phase == 8)
        {
            s->phase = 2;
            s->spell_timer = (Sint16)(Rand(rng) & 0xfU);
        }
        break;
    default:
        s->star_axis[1] = Word(s->star_axis[1] + s->star_spin);
        if (!s->spell_timer)
        {
            s->star_offset_x = Word(s->star_offset_x + 6);
            s->star_offset_y = Word(s->star_offset_y - 6);
            if (s->star_offset_y < -0x80)
            {
                s->removed = true;
                return true;
            }
            break;
        }
        s->spell_timer = Word(s->spell_timer - 1);
        break;
    }
    const Sint16 vertices[4][3] = {{Word(8 + s->star_offset_x), Word(-28 + s->star_offset_y), 0},
                                   {Word(56 + s->star_offset_x), Word(-28 + s->star_offset_y), 0},
                                   {Word(8 + s->star_offset_x), Word(28 + s->star_offset_y), 0},
                                   {Word(56 + s->star_offset_x), Word(28 + s->star_offset_y), 0}};
    const Sint32 transfer[3] = {0, 0x10, s->star_depth};
    SotnProjectedPoint point[4] = {{0}};
    Sint32 otz = 0;
    if (!SotnShieldStar_Project(in.executable, s->star_axis, transfer,
                                Word(Pixel(s->x) - in.camera_x), Word(Pixel(s->y) - in.camera_y),
                                0x280, vertices, point, &otz))
        return false;
    for (Uint16 k = 0; k < 4; ++k)
    {
        q->x[k] = (Sint16)point[k].x;
        q->y[k] = (Sint16)point[k].y;
    }
    q->draw_mode |= 8;
    if (otz > 0 && otz <= 0x1be)
    {
        const Uint16 angle = (Uint16)s->star_axis[1] & 0xfffU;
        q->priority = angle > 0x900 && angle < 0xf00 ? 0x1be : 0x1b4;
        q->draw_mode &= 0xfff7U;
    }
    return true;
}
// w_026 func_ptr_80170024, params1: a star's trail of four GT4s (all hidden at birth). Every
// update it retires once its raw parent slot empties; otherwise its counter selects row
// (count>>1)%4, which copies the parent star's current row while the star's X offset is
// positive, else hides.
static bool ShamanTrail(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in)
{
    if (!in.parent_exists)
    {
        s->removed = true;
        return true;
    }
    if (!s->phase)
    {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 9;
        s->allocated = true;
        s->flags |= 0x04800000U;
        for (Uint16 k = 0; k < 4; ++k)
            s->spell_quads[k] = (SotnPlayerEffectQuad){.draw_mode = 8};
        s->phase = 1;
        return true;
    }
    s->spell_timer = Word(s->spell_timer + 1);
    const Uint16 row = (Uint16)(((Uint16)s->spell_timer >> 1) % 4U);
    if (in.parent_offset_x > 0)
        s->spell_quads[row] = in.parent_quad;
    else
        s->spell_quads[row].draw_mode |= 8;
    return true;
}
// w_052 letters: one GT4 per letter (D_8017ABC4 UVs, palette base+i), states kept in its
// original g0 scratch byte (here the row's green[0]) with the r0 countdown (red[0]).
static bool AlucardLetterRows(SotnPlayerEffectQuad *rows, SotnWeaponSecondaryInput in, Uint16 base,
                              Uint16 priority, Uint32 *rng)
{
    const Uint8 *uv = NULL;
    if (!View(in.overlay, 0xbc4, 44, &uv))
        return false;
    const Uint8 offset = in.hand ? 0x80 : 0;
    for (Uint16 i = 0; i < 11; ++i)
    {
        SotnPlayerEffectQuad *q = &rows[i];
        *q = (SotnPlayerEffectQuad){.page = 0x19, .palette = (Uint16)(base + i)};
        q->u[0] = q->u[2] = uv[i * 4U];
        q->v[0] = q->v[1] = (Uint8)(uv[i * 4U + 1U] + offset);
        q->u[1] = q->u[3] = uv[i * 4U + 2U];
        q->v[2] = q->v[3] = (Uint8)(uv[i * 4U + 3U] + offset);
        q->priority = priority;
        q->draw_mode = 0;
        q->red[0] = (Uint8)((Rand(rng) & 0xfU) + 1U);
    }
    return true;
}
// w_052 EntityWeaponShieldSpell: Alucard shield's both-buttons spell (item214, twenty-three
// mandatory rows: the glow, eleven letters and eleven G4 ribbons; eleven birth rand calls). The
// letters circle the player (radius0x78 shrinking, angle steps from unk86) while the shield grows,
// holds 0x40 and flashes out with TRANSFORM; then they stretch away (TELEPORT_BANG_B), the
// ribbons between their last centres open by twelve per update, BlinkWhite FACTORY(44,1) follows,
// the ribbons fade, and the spell arms the shield (D_8017ABBC=1, D_8017ABC0=0x1000) as it ends
// the freeze.
static bool AlucardSpell(SotnWeaponSecondary *s, SotnHandWeaponProgram *p,
                         SotnWeaponSecondaryInput in, Uint32 *rng, SotnWeaponSecondaryEvents *e)
{
    Uint8 ribbons = 0;
    switch (s->phase)
    {
    case 0: {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 8;
        s->allocated = true;
        s->bank = in.hand ? 0x8012 : 0x8010;
        s->palette = in.hand ? 0x128 : 0x110;
        s->texture = in.hand ? 0x66 : 0x64;
        s->spell_v0 = in.hand ? 0 : 0x80;
        s->spell_v1 = in.hand ? 0x80 : 0;
        s->y = (Sint32)((Uint32)s->y - 0x80000U);
        s->flags = 0x04810000U;
        s->priority = (Uint16)(in.priority - 2U);
        s->facing_left = in.facing_left;
        s->frame = 0x3e;
        s->draw_flags = 3;
        s->scale_x = s->scale_y = 0;
        SotnPlayerEffectQuad *q = &s->spell_quads[0];
        *q = (SotnPlayerEffectQuad){.palette = 0x19f, .page = 0x19};
        q->u[0] = q->u[2] = 0x80;
        q->u[1] = q->u[3] = 0x80 + 0x4f;
        q->v[0] = q->v[1] = s->spell_v0;
        q->v[2] = q->v[3] = (Uint8)(s->spell_v0 + 0x4f);
        s->spell_glow = 0;
        SpellColors(q, 0x80);
        q->priority = (Uint16)(s->priority - 4U);
        q->draw_mode = 0x40 | 0x20 | 0x10 | 0x04 | 0x01;
        if (!AlucardLetterRows(&s->spell_quads[1], in, in.hand ? 0x129 : 0x111,
                               (Uint16)(s->priority - 4U), rng))
            return false;
        const Uint8 *colors = NULL;
        if (!View(in.overlay, 0xbf0, 44, &colors))
            return false;
        for (Uint16 i = 0; i < 11; ++i)
        {
            q = &s->spell_quads[12 + i];
            *q = (SotnPlayerEffectQuad){.draw_mode = 8};
            const Uint8 *a = colors + i * 4U, *b = colors + ((i + 1U) % 11U) * 4U;
            q->red[0] = q->red[2] = a[0];
            q->green[0] = q->green[2] = a[1];
            q->blue[0] = q->blue[2] = a[2];
            q->red[1] = q->red[3] = b[0];
            q->green[1] = q->green[3] = b[1];
            q->blue[1] = q->blue[3] = b[2];
        }
        s->spell_half_w = 0x78;  // unk90 radius
        s->spell_wave_x = 0;     // unk92 angle
        s->spell_half_h = 0;     // unk94 letter size
        s->spell_step = 0x40;    // unk86 angle step
        s->spell_bright = 0x100; // unk98 vertical scale
        s->spell_wave_y = 0;     // unk96 ribbon half height
        s->velocity_x = s->facing_left ? 0x20000 : -0x20000;
        s->spell_velocity_y = 0;
        e->clear_shield = true;
        e->sounds[0] = 0x641;
        e->freeze_set = true;
        s->phase = 1;
        break;
    }
    case 1:
        s->velocity_x = s->velocity_x < 0 ? SDL_min(s->velocity_x + 0x1000, 0)
                                          : SDL_max(s->velocity_x - 0x1000, 0);
        s->spell_velocity_y -= 0x2800;
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->spell_velocity_y);
        s->scale_x = Word(s->scale_x + 12);
        s->scale_y = s->scale_x;
        s->spell_glow = (Sint16)(s->scale_x * 0x28 / 256);
        if (s->scale_x >= 0x100)
        {
            s->spell_glow = 0x28;
            s->scale_y = s->scale_x = 0x100;
            s->spell_timer = 0x40;
            s->phase = 2;
        }
        s->spell_half_h = s->scale_x;
        s->spell_wave_x = Word(s->spell_wave_x + s->spell_step);
        s->spell_half_w = Word(s->spell_half_w - 3);
        break;
    case 2:
        if (--s->spell_timer == 0)
        {
            s->spell_timer = 0x10;
            e->sounds[0] = 0x668;
            s->phase = 3;
        }
        if (s->spell_step > 0)
            s->spell_step = Word(s->spell_step - 1);
        s->spell_wave_x = Word(s->spell_wave_x + s->spell_step);
        break;
    case 3:
        s->spell_quads[0].priority = !(s->spell_timer & 1) ? 0x1fc : (Uint16)(s->priority - 2U);
        s->scale_x = Word(s->scale_x - 0x10);
        if (s->scale_x <= 0)
            s->scale_x = 0;
        s->scale_y = s->scale_x;
        s->spell_half_h = Word(s->spell_half_h - 2);
        s->spell_bright = Word(s->spell_bright - 2);
        if (s->spell_bright < 0x80)
            s->spell_bright = 0x80;
        s->spell_step = Word(s->spell_step + 3);
        s->spell_wave_x = Word(s->spell_wave_x + s->spell_step);
        s->spell_half_w = Word(s->spell_half_w - 1);
        if (--s->spell_timer == 0)
        {
            s->frame = 0;
            s->spell_timer = 0x20;
            s->phase = 4;
        }
        break;
    case 4: {
        s->spell_half_h = Word(s->spell_half_h - 2);
        s->spell_bright = Word(s->spell_bright - 2);
        if (s->spell_bright < 0x80)
            s->spell_bright = 0x80;
        s->spell_step = Word(s->spell_step + 1);
        s->spell_wave_x = Word(s->spell_wave_x + s->spell_step);
        s->spell_glow = Word(s->spell_glow + 8);
        s->scale_x = Word(s->scale_x + 0x10);
        if (s->scale_x >= 0x100)
            s->scale_x = 0x100;
        SotnPlayerEffectQuad *q = &s->spell_quads[0];
        if (q->blue[3] > 8)
            q->blue[3] = (Uint8)(q->blue[3] - 4);
        SpellColors(q, q->blue[3]);
        s->spell_step = Word(s->spell_step + 4);
        s->spell_wave_x = Word(s->spell_wave_x + s->spell_step);
        if (--s->spell_timer == 0)
        {
            for (Uint16 i = 0; i < 11; ++i)
                s->spell_quads[1 + i].green[0] = 5;
            e->sounds[0] = 0x636;
            s->phase = 6;
        }
        break;
    }
    case 6:
        ribbons = 1;
        if (s->spell_wave_y > 0xe0)
        {
            ribbons = 2;
            s->phase = 7;
            s->spell_timer = 0x20;
            e->blink = 1;
            p->alucard_active = 1;
        }
        s->spell_wave_y = Word(s->spell_wave_y + 12);
        break;
    case 7:
        ribbons = 2;
        if (--s->spell_timer == 0)
        {
            s->phase = 8;
            p->alucard_active = 1;
            p->alucard_timer = 0x1000;
        }
        break;
    default:
        e->freeze_clear = true;
        s->removed = true;
        return true;
    }
    const Sint16 x = Word(Pixel(s->x) - in.camera_x), y = Word(Pixel(s->y) - in.camera_y);
    SotnPlayerEffectQuad *q = &s->spell_quads[0];
    if (q->blue[3] > 8)
    {
        if (in.engine_timer & 1U)
            q->draw_mode |= 8;
        else
            q->draw_mode &= 0xfff7U;
    }
    q->x[0] = q->x[2] = Word(x - s->spell_glow);
    q->x[1] = q->x[3] = Word(x + s->spell_glow - 1);
    q->y[0] = q->y[1] = Word(y - s->spell_glow);
    q->y[2] = q->y[3] = Word(y + s->spell_glow - 1);
    // Letters circle 0x1C ahead of and 0x30 above the player's whole screen position.
    const Sint16 cx = Word(Word(Pixel(in.x) - in.camera_x) - (in.facing_left ? -0x1c : 0x1c)),
                 cy = Word(Word(Pixel(in.y) - in.camera_y) - 0x30);
    const Sint16 rx = s->spell_half_w, ry = (Sint16)(s->spell_half_w * s->spell_bright / 256);
    for (Uint16 i = 0; i < 11; ++i)
    {
        q = &s->spell_quads[1 + i];
        if (q->green[0] == 0)
        {
            SotnMathTrig t = {0};
            if (!SotnMath_Trig(in.executable, Word(s->spell_wave_x + ((Sint32)i << 12) / 11), &t))
                return false;
            const Sint16 mx = Word(cx + Asr(Asr(t.cosine, 4) * rx, 8)),
                         my = Word(cy - Asr(Asr(t.sine, 4) * ry, 8));
            const Sint16 h = (Sint16)(s->spell_half_h * (i == 6 ? 12 : 16) / 256),
                         w = (Sint16)(s->spell_half_h * 12 / 256);
            q->x[0] = q->x[2] = Word(mx - w);
            q->x[1] = q->x[3] = Word(mx + w);
            q->y[0] = q->y[1] = Word(my - h);
            q->y[2] = q->y[3] = Word(my + h);
            p->alucard_priority[i] = (Sint16)q->priority;
            p->alucard_x[i] = mx;
            p->alucard_y[i] = my;
        }
        else if (q->green[0] == 1)
        {
            if (--q->red[0] == 0)
                ++q->green[0];
        }
        else if (q->green[0] == 5)
        {
            if (q->x[1] - q->x[0] > 2)
            {
                q->x[0] = Word(q->x[0] + 1);
                q->x[1] = Word(q->x[1] - 1);
            }
            else
                q->x[0] = q->x[1];
            q->y[0] = Word(q->y[0] - 12);
            q->y[2] = Word(q->y[2] + 12);
            q->y[1] = q->y[0];
            q->y[3] = q->y[2];
            q->x[2] = q->x[0];
            q->x[3] = q->x[1];
            if (q->y[0] < 0 && q->y[2] > 0xf0)
            {
                q->draw_mode |= 8;
                ++q->green[0];
            }
        }
    }
    for (Uint16 i = 0; ribbons && i < 11; ++i)
    {
        q = &s->spell_quads[12 + i];
        if (ribbons == 2)
        {
            Uint8 *channel[3][4] = {{&q->red[0], &q->red[1], &q->red[2], &q->red[3]},
                                    {&q->blue[0], &q->blue[1], &q->blue[2], &q->blue[3]},
                                    {&q->green[0], &q->green[1], &q->green[2], &q->green[3]}};
            for (Uint16 c = 0; c < 3; ++c)
                for (Uint16 k = 0; k < 4; ++k)
                    if (*channel[c][k] > 8)
                        *channel[c][k] = (Uint8)(*channel[c][k] - 4);
        }
        else
        {
            const Uint16 j = (Uint16)((i + 1U) % 11U);
            q->x[0] = q->x[2] = p->alucard_x[i];
            q->x[1] = q->x[3] = p->alucard_x[j];
            q->y[0] = Word(p->alucard_y[i] - s->spell_wave_y);
            q->y[2] = Word(p->alucard_y[i] + s->spell_wave_y);
            q->y[1] = Word(p->alucard_y[j] - s->spell_wave_y);
            q->y[3] = Word(p->alucard_y[j] + s->spell_wave_y);
            q->priority = (Uint16)p->alucard_priority[i];
        }
        q->draw_mode = 0x20 | 0x10 | 0x01;
    }
    return true;
}
// w_052 func_ptr_80170004 (E1, blueprint0x3A from the expiring armed shield): the eleven letters
// burst from the player (eleven GT4s, then per letter one rand countdown and two rand
// velocities), grow, fall, wait their countdowns, streak upward and hide; WEAPON_SWISH_C plays on
// every fourth update of the first 24 of its 0x60-update tail.
static bool AlucardLetters(SotnWeaponSecondary *s, SotnHandWeaponProgram *p,
                           SotnWeaponSecondaryInput in, Uint32 *rng, SotnWeaponSecondaryEvents *e)
{
    switch (s->phase)
    {
    case 0: {
        if (!in.allocated)
        {
            s->removed = true;
            return true;
        }
        s->role = 1;
        s->allocated = true;
        s->flags = 0x04810000U;
        const Uint16 px = (Uint16)Word(Pixel(in.x) - in.camera_x);
        const Sint16 py = Word(Word(Pixel(in.y) - in.camera_y) - 8);
        const Uint8 *uv = NULL;
        if (!View(in.overlay, 0xbc4, 44, &uv))
            return false;
        const Uint8 offset = in.hand ? 0x80 : 0;
        for (Uint16 i = 0; i < 11; ++i)
        {
            SotnPlayerEffectQuad *q = &s->spell_quads[i];
            *q = (SotnPlayerEffectQuad){.page = 0x19,
                                        .palette = (Uint16)((in.hand ? 0x129 : 0x111) + i)};
            q->u[0] = q->u[2] = uv[i * 4U];
            q->v[0] = q->v[1] = (Uint8)(uv[i * 4U + 1U] + offset);
            q->u[1] = q->u[3] = uv[i * 4U + 2U];
            q->v[2] = q->v[3] = (Uint8)(uv[i * 4U + 3U] + offset);
            q->priority = (Uint16)(s->priority - 2U);
            q->draw_mode = 0;
            q->red[0] = (Uint8)((Rand(rng) & 0xfU) + 1U);
            p->letter_x[i] = (Sint32)(((Uint32)px << 16) | ((Uint32)p->letter_x[i] & 0xffffU));
            p->letter_y[i] =
                (Sint32)(((Uint32)(Uint16)py << 16) | ((Uint32)p->letter_y[i] & 0xffffU));
            p->letter_vx[i] = (Sint32)(Rand(rng) * 6U) - 0x18000;
            p->letter_vy[i] = -0x20000 - (Sint32)(Rand(rng) * 4U);
        }
        s->spell_half_h = 0x20;
        s->spell_timer = 0x20;
        s->phase = 1;
        break;
    }
    case 1:
        s->spell_half_h = Word(s->spell_half_h + 0x10);
        if (s->spell_half_h > 0xc0)
            s->spell_half_h = 0xc0;
        for (Uint16 i = 0; i < 11; ++i)
        {
            p->letter_x[i] = (Sint32)((Uint32)p->letter_x[i] + (Uint32)p->letter_vx[i]);
            p->letter_y[i] = (Sint32)((Uint32)p->letter_y[i] + (Uint32)p->letter_vy[i]);
            p->letter_vy[i] = (Sint32)((Uint32)p->letter_vy[i] + 0x2000U);
        }
        if (--s->spell_timer == 0)
        {
            for (Uint16 i = 0; i < 11; ++i)
                s->spell_quads[i].green[0] = 1;
            s->spell_timer = 0x60;
            s->phase = 2;
        }
        break;
    default:
        if (s->spell_timer >= 0x48 && !(s->spell_timer % 4))
            e->sounds[0] = 0x60c;
        if (--s->spell_timer == 0)
        {
            s->removed = true;
            return true;
        }
        break;
    }
    for (Uint16 i = 0; i < 11; ++i)
    {
        SotnPlayerEffectQuad *q = &s->spell_quads[i];
        switch (q->green[0])
        {
        case 0: {
            const Sint16 mx = Pixel(p->letter_x[i]), my = Pixel(p->letter_y[i]);
            const Sint16 h = (Sint16)(s->spell_half_h * (i == 6 ? 12 : 16) / 256),
                         w = (Sint16)(s->spell_half_h * 12 / 256);
            q->x[0] = q->x[2] = Word(mx - w);
            q->x[1] = q->x[3] = Word(mx + w);
            q->y[0] = q->y[1] = Word(my - h);
            q->y[2] = q->y[3] = Word(my + h);
            break;
        }
        case 1:
            if (--q->red[0] == 0)
                ++q->green[0];
            break;
        case 2:
            if (q->x[1] - q->x[0] >= 5)
            {
                q->x[0] = Word(q->x[0] + 1);
                q->x[1] = Word(q->x[1] - 1);
            }
            else
                q->x[0] = Word(q->x[1] - 2);
            q->y[0] = Word(q->y[0] - 18);
            q->y[2] = Word(q->y[2] - 6);
            q->y[1] = q->y[0];
            q->y[3] = q->y[2];
            q->x[2] = q->x[0];
            q->x[3] = q->x[1];
            if (q->y[2] < 0)
                ++q->green[0];
            break;
        case 3:
            q->draw_mode = 8;
            break;
        default:
            break;
        }
    }
    return true;
}
// Blueprint90 (FACTORY(90,1) with <<14 hand bits): one fading afterimage of the walker.
static void Afterimage(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in,
                       SotnWeaponSecondaryEvents *e)
{
    if (e->factory_count >= SDL_arraysize(e->factories))
        return;
    e->factories[e->factory_count] = (90U + ((in.hand + 1U) << 14)) | 1U << 16;
    e->factory_x[e->factory_count] = s->x;
    e->factory_y[e->factory_count] = s->y;
    ++e->factory_count;
}
// w_011 func_ptr_80170024 (vector9): blueprint88's armor walker (params0) and blueprint90's
// fading afterimages (params1). Both retire when their raw parent slot has emptied at birth and
// copy its sprite bank, texture, palette and facing. The walker rolls SetWeaponProperties for the
// parent's item, falls and walks (CheckCollision probes, slope speed1.25, STOMP_SOFT_A every ten
// updates, an afterimage every fourth g_Timer/own count) and on a wall or hit plays CANDLE_HIT,
// its final animation and leaves one last afterimage; afterimages fade by two for0x18 updates.
static bool AxeLordWalker(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                          SotnWeaponSecondaryEvents *e)
{
    SotnCollider c = {0};
    bool other = false;
    switch (s->phase)
    {
    case 0:
        if (!in.parent_exists)
        {
            s->removed = true;
            return true;
        }
        s->role = 9;
        s->allocated = true;
        s->bank = in.parent_bank;
        s->texture = in.parent_texture;
        s->priority = (Uint16)(in.priority + 2U);
        s->palette = in.parent_child_palette;
        s->facing_left = in.parent_facing_left;
        if (s->parameter & 0x7f00U)
        {
            s->flags = 0x08000000U;
            s->frame = in.parent_frame;
            s->draw_flags = 8;
            s->blend = 0x10;
            s->opacity = 0x80;
            s->remaining = 0x18;
            s->priority = (Uint16)(s->priority - 2U);
            s->phase = 4;
        }
        else
        {
            s->timeline = 0xbcc;
            s->pose = 0;
            s->remaining = 0;
            s->flags = 0x08100000U;
            if (!Properties(s, in, rng, e))
                return false;
            s->hitbox_width = 8;
            s->hitbox_height = 20;
            s->velocity_x = s->facing_left ? -0x10000 : 0x10000;
            s->phase = 1;
        }
        break;
    case 1: {
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->dawn_velocity_y);
        s->dawn_velocity_y += 0x2000;
        if (s->dawn_velocity_y > 0x60000)
            s->dawn_velocity_y = 0x60000;
        const Sint16 sx = Word(Pixel(s->x) - in.camera_x), sy = Word(Pixel(s->y) - in.camera_y);
        const Sint16 side = s->velocity_x < 0 ? -8 : 8;
        if (!Query(in, sx, Word(sy + 25), &c))
            return false;
        if (c.effects & SOTN_EFFECT_SOLID)
        {
            s->y = (Sint32)((Uint32)s->y + (Uint32)c.top * 65536U);
            e->sounds[0] = 0x64b;
            s->phase = 2;
            break;
        }
        if (!(c.effects & 0x8000U))
        {
            if (!Query(in, Word(sx + side), Word(sy + 22), &c))
                return false;
            if ((c.effects & SOTN_EFFECT_SOLID) && !(c.effects & 0x8000U))
            {
                other = true;
                break;
            }
            if (!Query(in, Word(sx + side), Word(sy + 8), &c))
                return false;
            if ((c.effects & SOTN_EFFECT_SOLID) && !(c.effects & 0x8000U))
            {
                other = true;
                break;
            }
        }
        if (!Query(in, Word(sx + side), Word(sy - 8), &c))
            return false;
        if ((c.effects & SOTN_EFFECT_SOLID) || s->hit_flags)
        {
            other = true;
            break;
        }
        if (!(in.engine_timer % 4U) && in.free_factories)
            Afterimage(s, in, e);
        break;
    }
    case 2: {
        s->velocity_x = s->facing_left ? -0x28000 : 0x28000;
        if (!(s->dawn_count % 10))
            e->sounds[0] = 0x64b;
        Sint16 sx = Word(Pixel(s->x) - in.camera_x), sy = Word(Pixel(s->y) - in.camera_y);
        const Sint16 side = s->velocity_x < 0 ? -8 : 8;
        if (!Query(in, sx, Word(sy + 25), &c))
            return false;
        if (c.effects & SOTN_EFFECT_SOLID)
            s->y = (Sint32)((Uint32)s->y + (Uint32)c.top * 65536U);
        else
        {
            s->phase = 1;
            s->dawn_velocity_y = 0;
        }
        sy = Word(Pixel(s->y) - in.camera_y);
        if (!(c.effects & 0x8000U))
        {
            if (!Query(in, Word(sx + side), Word(sy + 22), &c))
                return false;
            if ((c.effects & SOTN_EFFECT_SOLID) && !(c.effects & 0x8000U))
            {
                other = true;
                break;
            }
            if (!Query(in, Word(sx + side), Word(sy + 8), &c))
                return false;
            if ((c.effects & SOTN_EFFECT_SOLID) && !(c.effects & 0x8000U))
            {
                other = true;
                break;
            }
        }
        if ((((c.effects & 0xc000U) == 0xc000U) && s->facing_left) ||
            (((c.effects & 0xc000U) == 0x8000U) && !s->facing_left))
        {
            if (!(c.effects & (SOTN_EFFECT_SOLID_FROM_ABOVE | SOTN_EFFECT_SOLID_FROM_BELOW)))
                s->velocity_x = s->facing_left ? -0x14000 : 0x14000;
            // The original's 0x100/0x200 speed tests never match the 0x40/0x80 mask.
        }
        if (!Query(in, Word(sx + side), Word(sy - 8), &c))
            return false;
        if (!(c.effects & SOTN_EFFECT_SOLID) && !s->hit_flags)
        {
            if (!(s->dawn_count % 4) && in.free_factories)
                Afterimage(s, in, e);
            s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        }
        else
            other = true;
        break;
    }
    case 3:
        if (s->remaining < 0)
        {
            s->removed = true;
            return true;
        }
        break;
    default:
        s->opacity = (Uint8)(s->opacity - 2);
        if (--s->remaining < 0)
        {
            s->removed = true;
            return true;
        }
        break;
    }
    if (other)
    {
        e->sounds[1] = 0x634;
        s->phase = 3;
        s->timeline = 0xbe8;
        s->remaining = 0;
        s->pose = 0;
        if (in.free_factories)
            Afterimage(s, in, e);
    }
    s->dawn_count = Word(s->dawn_count + 1);
    return true;
}

// w_034 func_ptr_80170004: Shotel's QCF176 boomerang (blueprint56, own item). It spins by
// ROT(45) every update, flies out decelerating for 0x1C updates, then homes on the player's
// hitbox with a turn limit growing by 0x10 from 0x200 and retires inside half the summed
// hitboxes. Every eighth g_GameTimer plays SWISH_B (not on the retiring update).
static bool Shotel(SotnWeaponSecondary *s, SotnWeaponSecondaryInput in, Uint32 *rng,
                   SotnWeaponSecondaryEvents *e)
{
    if (!s->phase)
    {
        s->role = 1;
        s->allocated = true;
        s->bank = (Uint16)(0x8010U + in.hand * 2U);
        s->texture = (Uint16)(0x64U + in.hand * 2U);
        s->palette = (Uint16)(0x110U + in.hand * 0x18U);
        s->frame = 0x3a;
        s->facing_left = in.facing_left;
        s->priority = (Uint16)(in.priority - 2U);
        s->flags = 0x0c020000U;
        s->blend = 0x10;
        s->velocity_x = s->facing_left ? -0x48000 : 0x48000;
        s->lifetime = 0x1c;
        const Sint16 offset = Word(Word(Pixel(in.y) - in.camera_y) + in.parent_hitbox_y);
        SetScreen(&s->y, Word(offset - (in.player_crouch_step ? 8 : 0x10)), in.camera_y);
        const Sint16 px = Word(Pixel(in.x) - in.camera_x);
        SetScreen(&s->x, Word(s->facing_left ? px - in.parent_hitbox_x : px + in.parent_hitbox_x),
                  in.camera_x);
        s->accel_x = s->facing_left ? 0x2000 : -0x2000;
        s->velocity_x = (Sint32)((Uint32)s->velocity_x + (Uint32)in.player_velocity_x);
        s->magnitude = 0x200;
        s->velocity_y = -0x4000;
        s->accel_y = 0x400;
        s->rotate = 0;
        s->draw_flags |= 4;
        if (!Properties(s, in, rng, e))
            return false;
        e->clear_shield = true;
        s->hitbox_width = s->hitbox_height = 9;
        e->use_ticks = 4;
        s->phase = 1;
    }
    else if (s->phase == 1)
    {
        s->rotate = Word(s->rotate + 0x200);
        s->x = (Sint32)((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = (Sint32)((Uint32)s->y + (Uint32)s->velocity_y);
        s->velocity_x = (Sint32)((Uint32)s->velocity_x + (Uint32)(Sint32)Word(s->accel_x));
        s->velocity_y = (Sint32)((Uint32)s->velocity_y + (Uint32)(Sint32)Word(s->accel_y));
        s->lifetime = Word(s->lifetime - 1);
        if (!s->lifetime)
        {
            Sint16 heading = 0;
            if (!SotnMath_Atan2Vector(in.executable, (Sint32)(0U - (Uint32)s->velocity_y),
                                      s->velocity_x, &heading))
                return false;
            s->angle = (Sint16)(heading & 0xfff);
            s->phase = 2;
        }
    }
    else
    {
        s->rotate = Word(s->rotate + 0x200);
        Sint16 limit = s->magnitude;
        s->magnitude = Word(s->magnitude + 0x10);
        const Sint32 sx = Word(Pixel(s->x) - in.camera_x), sy = Word(Pixel(s->y) - in.camera_y),
                     tx = (Sint32)Word(Pixel(in.x) - in.camera_x) + in.parent_hitbox_x,
                     ty = (Sint32)Word(Pixel(in.y) - in.camera_y) + in.parent_hitbox_y;
        const Sint16 dx = Word(tx - sx < 0 ? sx - tx : tx - sx),
                     dy = Word(ty - sy < 0 ? sy - ty : ty - sy);
        if (dx < (in.player_hitbox_width + s->hitbox_width) >> 1 &&
            dy < (in.player_hitbox_height + s->hitbox_height) >> 1)
        {
            s->removed = true;
            return true;
        }
        Sint16 target = 0;
        if (!SotnMath_Atan2(in.executable, -(Sint32)Word(ty - sy), Word(tx - sx), &target))
            return false;
        target = (Sint16)(target & 0xfff);
        Sint16 angle = (Sint16)(s->angle & 0xfff);
        const Sint16 diff = Word(angle - target < 0 ? target - angle : angle - target);
        if (limit > diff)
            limit = diff;
        if (angle < target)
            angle = Word(diff < 0x800 ? angle + limit : angle - limit);
        else
            angle = Word(diff < 0x800 ? angle - limit : angle + limit);
        s->angle = (Sint16)(angle & 0xfff);
        SotnMathTrig t = {0};
        if (!SotnMath_Trig(in.executable, s->angle, &t))
            return false;
        s->x = (Sint32)((Uint32)s->x + (Uint32)t.cosine * 64U);
        s->y = (Sint32)((Uint32)s->y - (Uint32)t.sine * 64U);
    }
    if (!(in.game_timer & 7U))
        e->sounds[0] = 0x60b;
    return true;
}

bool SotnWeaponSecondary_Tick(SotnWeaponSecondary *state, SotnHandWeaponProgram *program,
                              SotnWeaponSecondaryInput in, Uint32 *random,
                              SotnWeaponSecondaryEvents *events)
{
    Layout layout = {0};
    if (!state || !program || !random || !events || in.hand > 1 || in.role > 10 ||
        !Program(in.family, &layout) ||
        !SotnWeaponSecondary_SupportedRole(
            in.family,
            in.role || in.family == 14 || in.family == 13 || in.family == 12 ? in.role : 1) ||
        state->removed ||
        state->phase > (in.family == 49 && in.role == 4                        ? 2
                        : in.family == 6 && in.role == 1                       ? 4
                        : (in.family == 14 || in.family == 13) && in.role == 0 ? 3
                        : in.family == 13 && in.role == 2                      ? 4
                        : in.family == 13 && in.role == 3                      ? 3
                        : in.family == 7 && in.role == 1                       ? 4
                        : in.family == 10 && in.role == 8                      ? 9
                        : in.family == 23 && in.role == 8                      ? 8
                        : in.family == 23                                      ? 3
                        : in.family == 24 && in.role == 8                      ? 8
                        : in.family == 24                                      ? 3
                        : in.family == 27 && in.role == 8                      ? 5
                        : in.family == 27                                      ? 1
                        : in.family == 28 && in.role == 8                      ? 8
                        : in.family == 28                                      ? 3
                        : in.family == 29 && in.role == 8                      ? 6
                        : (in.family == 12 || in.family == 34) && in.role == 1 ? 2
                        : in.family == 30 && in.role == 1                      ? 4
                        : in.family == 30 && in.role == 2                      ? 8
                        : in.family == 30 && in.role >= 3                      ? 4
                        : in.family == 58                                      ? 6
                        : in.family == 11 && in.role == 9                      ? 4
                        : in.family == 25 && in.role == 9                      ? 3
                        : in.family == 26 && in.role == 9                      ? 2
                        : in.family == 26                                      ? 8
                        : in.family == 52 && in.role == 1                      ? 2
                        : in.family == 52                                      ? 8
                        : in.family == 25                                      ? 9
                        : in.family == 11                                      ? 7
                        : in.family == 8 || in.family == 9                     ? 7
                                                                               : 1) ||
        state->remaining < -1)
        return SDL_SetError("Weapon secondary requires its actual live bounded child");
    const size_t at = layout.frame_start + ((state->parameter >> 4) & 0x7f0U);
    if (in.family == 55 && (!in.overlay.bytes || at >= in.overlay.size))
        return SDL_SetError("Weapon secondary descriptor frameStart is outside its program");
    SotnWeaponSecondary s = *state;
    SotnHandWeaponProgram shared = *program;
    SotnWeaponSecondaryEvents e = {0};
    Uint32 rng = *random;
    if (in.family == 53 || in.family == 56)
    {
        if (!Slash(&s, in, &rng, &e))
            return false;
        goto commit;
    }
    if (in.family == 48)
    {
        if (!(in.role == 2 ? Ring(&s, in, &e) : Lightning(&s, in, &rng, &e)))
            return false;
        goto commit;
    }
    if (in.family == 51)
    {
        if (!Fan(&s, in, &rng, &e))
            return false;
        goto commit;
    }
    if (in.family == 6)
    {
        if (!(in.role == 1 ? Crescent(&s, in, &rng, &e) : MoonSparkle(&s, in, &rng)))
            return false;
        goto commit;
    }
    if (in.family == 14)
    {
        if (!(in.role == 0 ? Rune(&s, in, &rng, &e) : RuneHitbox(&s, in, &rng, &e)))
            return false;
        goto commit;
    }
    if (in.family == 10)
    {
        if (!(in.role == 1   ? IronThunder(&s, in, &e)
              : in.role == 8 ? IronSpell(&s, &shared, in, &e)
                             : IronSword(&s, &shared, in, &rng, &e)))
            return false;
        goto commit;
    }
    if (in.family == 23)
    {
        if (!(in.role == 8   ? HeraldSpell(&s, &shared, in, &e)
              : in.role == 9 ? HeraldSwirl(&s, &shared, in)
                             : HeraldSprite(&s, in, &rng)))
            return false;
        goto commit;
    }
    if (in.family == 28)
    {
        if (!(in.role == 8 ? SkullSpell(&s, &shared, in, &e) : SkullBeam(&s, in, &rng, &e)))
            return false;
        goto commit;
    }
    if (in.family == 29)
    {
        if (!(in.role == 1   ? FireImage(&s, in)
              : in.role == 8 ? FireSpell(&s, in, &e)
              : in.role == 9 ? FireDragon(&s, in, &rng, &e)
                             : FireHit(&s, in, &rng, &e)))
            return false;
        goto commit;
    }
    if (in.family == 27)
    {
        if (!(in.role == 8   ? MedusaSpell(&s, in, &e)
              : in.role == 9 ? MedusaLaser(&s, in, &e)
                             : MedusaHit(&s, in, &rng, &e)))
            return false;
        goto commit;
    }
    if (in.family == 24)
    {
        if (!(in.role == 8 ? DarkSpell(&s, &shared, in, &rng, &e) : DarkRock(&s, in, &rng, &e)))
            return false;
        goto commit;
    }
    if (in.family == 7)
    {
        if (!(in.role == 1 ? RodOrb(&s, in, &e) : RodSparkle(&s, in, &rng)))
            return false;
        goto commit;
    }
    if (in.family == 13)
    {
        if (!(in.role == 0   ? Heaven(&s, in, &rng, &e)
              : in.role == 2 ? HeavenSweep(&s, in, &rng, &e)
              : in.role == 3 ? HeavenStrike(&s, in, &rng, &e)
                             : HeavenImage(&s, in)))
            return false;
        goto commit;
    }
    // w_012's func_ptr_80170008 afterimage matches w_013's, copying the parent's own palette.
    if (in.family == 12)
    {
        if (!(in.role == 0   ? Vorpal(&s, &shared, in, &rng, &e)
              : in.role == 1 ? Chakram(&s, in, &rng, &e)
                             : HeavenImage(&s, in)))
            return false;
        goto commit;
    }
    if (in.family == 11)
    {
        if (!(in.role == 9 ? AxeLordWalker(&s, in, &rng, &e) : AxeLordSpell(&s, &shared, in, &e)))
            return false;
        goto commit;
    }
    if (in.family == 25)
    {
        if (!(in.role == 9 ? GoddessCloud(&s, in, &rng) : GoddessSpell(&s, &shared, in, &e)))
            return false;
        goto commit;
    }
    if (in.family == 52)
    {
        if (!(in.role == 8 ? AlucardSpell(&s, &shared, in, &rng, &e)
                           : AlucardLetters(&s, &shared, in, &rng, &e)))
            return false;
        goto commit;
    }
    if (in.family == 26)
    {
        if (!(in.role == 8              ? ShamanSpell(&s, &shared, in, &e)
              : (s.parameter & 0x7f00U) ? ShamanTrail(&s, in)
                                        : ShamanStar(&s, in, &rng, &e)))
            return false;
        goto commit;
    }
    if (in.family == 58)
    {
        if (!AlucartSpell(&s, in, &e))
            return false;
        goto commit;
    }
    if (in.family == 8 || in.family == 9)
    {
        if (!ShieldCardSpell(&s, &shared, in, &e))
            return false;
        goto commit;
    }
    if (in.family == 30)
    {
        if (!(in.role == 1   ? DawnCircle(&s, &shared, in, &rng, &e)
              : in.role == 2 ? DawnSoldier(&s, in, &rng, &e)
              : in.role == 3 ? DawnArrow(&s, &shared, in, &rng, &e)
                             : DawnBomb(&s, in, &rng, &e)))
            return false;
        goto commit;
    }
    if (in.family == 34)
    {
        if (!Shotel(&s, in, &rng, &e))
            return false;
        goto commit;
    }
    if (in.family == 50)
    {
        const bool ok = in.role == 1   ? Sparkles(&s, &shared, in, &rng)
                        : in.role == 2 ? IceRing(&s, in, &rng, &e)
                        : in.role == 3 ? Frost(&s, in, &rng, &e)
                                       : Crystals(&s, &shared, in, &rng);
        if (!ok)
            return false;
        goto commit;
    }
    if (in.family == 49)
    {
        const bool ok = in.role == 1   ? Sparks(&s, in, &e)
                        : in.role == 2 ? BrandRing(&s, in)
                        : in.role == 3 ? Puffs(&s, in, &rng, &e)
                        : in.role == 4 ? Swirl(&s, in, &rng, &e)
                                       : Pillar(&s, in, &rng, &e);
        if (!ok)
            return false;
        goto commit;
    }
    if (in.family == 1)
    {
        if (!(in.role == 2 ? Fireball(&s, in, &rng, &e) : Punches(&s, in, &rng, &e)))
            return false;
        s.hit_flags = 0;
        goto commit;
    }
    s.x = in.x;
    s.y = in.y;
    s.facing_left = in.facing_left;
    if (in.family != 55)
    {
        if (!Lunge(&s, &shared, in, &rng, &e))
            return false;
        goto commit;
    }
    s.draw_flags = in.draw_flags;
    s.scale_y = in.scale_y;
    s.pivot_y = in.pivot_y;
    const Uint8 start = in.overlay.bytes[at];
    if (in.animation < start || in.animation >= start + 7U || !in.attack_mode)
    {
        s.removed = true;
        goto commit;
    }
    const Sint32 x = Word(Pixel(s.x) - in.camera_x) + (in.facing_left ? -12 : 12),
                 y = Word(Pixel(s.y) - in.camera_y) - 28;
    if (!s.phase)
    {
        if (!in.allocated)
        {
            s.removed = true;
            goto commit;
        }
        s.allocated = true;
        for (Uint16 i = 0; i < SOTN_WEAPON_SECONDARY_ROWS; ++i)
        {
            SotnWeaponSecondaryRow *r = &s.rows[i];
            *r = (SotnWeaponSecondaryRow){.red = {0, 127}, .green = {0, 127}, .blue = {0, 255}};
            r->angle = (Sint16)(SotnCombat_Random(&rng) & 0xfffU);
            r->length = (Sint16)((SotnCombat_Random(&rng) & 0xffU) + 512U);
            r->growth = (Sint16)(r->length / 2);
        }
        s.row_priority = (Uint16)(in.priority + 2U);
        s.draw_mode = 0x331;
        s.bank = (Uint16)(layout.bank + in.hand * 2U);
        s.palette = (Uint16)(layout.palette + in.hand * 24U);
        s.texture = (Uint16)(layout.texture + in.hand * 2U);
        s.flags = 0x960000U;
        s.pose = 0;
        s.remaining = 0;
        s.frame = 0;
        s.priority = (Uint16)(in.priority + 4U);
        s.blend = 0x30;
        if (!Properties(&s, in, &rng, &e))
            return false;
        s.hitbox_width = s.hitbox_height = 32;
        s.hitbox_x = 12;
        s.hitbox_y = -28;
        s.phase = 1;
    }
    else if (s.frame == 1 && s.remaining == 1)
    {
        s.hitbox_width = s.hitbox_height = 12;
        s.hitbox_x = 13;
        s.hitbox_y = -28;
    }
    if (s.remaining < 0)
    {
        s.removed = true;
        goto commit;
    }
    for (Uint16 i = 0; i < SOTN_WEAPON_SECONDARY_ROWS; ++i)
    {
        SotnWeaponSecondaryRow *r = &s.rows[i];
        if (r->red[1] >= 17)
            r->red[1] = (Uint8)(r->red[1] - 12U);
        if (r->green[1] >= 17)
            r->green[1] = (Uint8)(r->green[1] - 12U);
        if (r->blue[1] >= 17)
            r->blue[1] = (Uint8)(r->blue[1] - 24U);
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(in.executable, r->angle, &trig))
            return false;
        const Sint32 scale = r->length;
        r->length = Word(r->length + r->growth);
        r->x[0] = Word(x);
        r->y[0] = Word(y);
        r->x[1] = Word(x + Asr(Asr(trig.cosine, 8) * scale, 8));
        r->y[1] = Word(y - Asr(Asr(trig.sine, 8) * scale, 8));
    }
commit:
    *state = s;
    *program = shared;
    *random = rng;
    *events = e;
    return true;
}

bool SotnWeaponSecondary_Animate(SotnWeaponSecondary *state, SotnDataView overlay, Uint16 family)
{
    Layout layout = {0};
    if (state && (family == 49 || family == 30 || family == 11))
        layout = (Layout){0};
    else if (!Program(family, &layout))
        return SDL_SetError("Weapon secondary animation requires its actual private timeline");
    // Weapon49 children choose their private timeline at birth (sparks9, puffs/pillar14 words).
    if (state && family == 49)
        layout = (Layout){.timeline = state->timeline, .timeline_count = state->role == 1 ? 9 : 14};
    // Weapon11's walker: D_8017ABCC (seven, looping) or its final D_8017ABE8 (three).
    if (state && family == 11 && state->role == 9)
        layout = (Layout){.timeline = state->timeline,
                          .timeline_count = state->timeline == 0xbe8 ? 3 : 7};
    // Weapon29's spell: D_8017AD24 (eight frames, looping).
    if (state && family == 29 && state->role == 8)
        layout = (Layout){.timeline = state->timeline, .timeline_count = 9};
    // Weapon27's spell: D_8017AC8C (nine frames, looping) or D_8017ACB4 (two, looping).
    if (state && family == 27 && state->role == 8)
        layout = (Layout){.timeline = state->timeline,
                          .timeline_count = state->timeline == 0xcb4 ? 3 : 10};
    // Weapon30's bombs: D_8017A514 (thirteen frames) or the roller's D_8017A548 (three).
    if (state && family == 30 && state->role == 4)
        layout = (Layout){.timeline = state->timeline,
                          .timeline_count = state->timeline == 0x548 ? 3 : 13};
    if (!state ||
        (family != 55 && family != 1 && (family != 7 || state->role != 1) &&
         (family != 10 || state->role != 1) && (family != 23 || state->role != 10) &&
         (family != 27 || state->role != 8 || !layout.timeline) &&
         (family != 29 || state->role != 8 || !layout.timeline) &&
         ((family != 49 && family != 30 && family != 11) || !layout.timeline)) ||
        state->removed || !state->phase || state->pose >= layout.timeline_count ||
        state->remaining < -1 || !overlay.bytes ||
        overlay.size < layout.timeline + layout.timeline_count * 4U)
        return SDL_SetError("Weapon secondary animation requires its actual private timeline");
    const Uint8 *t = overlay.bytes + layout.timeline;
    SotnWeaponSecondary s = *state;
    if (s.remaining == 0)
    {
        const Uint16 duration = Read16(t + s.pose * 4U);
        if (!duration || duration > 32767)
            return SDL_SetError("Weapon secondary animation has an invalid birth duration");
        s.remaining = (Sint16)duration;
    }
    else if (s.remaining != -1 && --s.remaining == 0)
    {
        if (++s.pose >= layout.timeline_count)
            return SDL_SetError("Weapon secondary animation escaped its private timeline");
        Uint16 duration = Read16(t + s.pose * 4U);
        if (duration == 0xffffU)
        {
            --s.pose;
            s.remaining = -1;
        }
        else
        {
            if (!duration)
            {
                s.pose = Read16(t + s.pose * 4U + 2U);
                if (s.pose >= layout.timeline_count)
                    return SDL_SetError("Weapon secondary animation has an invalid loop target");
                duration = Read16(t + s.pose * 4U);
            }
            if (!duration || duration > 32767)
                return SDL_SetError("Weapon secondary animation has an unsupported control");
            s.remaining = (Sint16)duration;
        }
    }
    s.frame = Read16(t + s.pose * 4U + 2U) & 511U;
    *state = s;
    return true;
}

// w_053 func_ptr_80170008: SetSpeedX/DecelerateX from the shared weapon helpers.
static Sint32 Decelerate(Sint32 v, Sint32 amount)
{
    if (v < 0)
        return v + amount > 0 ? 0 : v + amount;
    return v - amount < 0 ? 0 : v - amount;
}
bool SotnWeaponWarp_Tick(SotnWeaponWarp *state, SotnWeaponWarpInput in,
                         SotnWeaponWarpEvents *events)
{
    if (!state || !events || in.hand > 1 || state->removed || state->phase > 3)
        return SDL_SetError("Weapon53 warp requires its actual live bounded child");
    SotnWeaponWarp s = *state;
    SotnWeaponWarpEvents e = {0};
    const Uint32 hand = (in.hand + 1U) << 12;
    Uint16 n = 0;
    switch (s.phase)
    {
    case 0:
        s.blend = 0x10;
        s.facing_left = in.facing_left;
        s.flags = 0x04020000U;
        s.velocity_x = s.facing_left ? -0x80000 : 0x80000;
        s.velocity_y = -0x8000;
        if (s.parameter & 0x7f00U)
            e.factories[n++] = 0x38U | hand | 0x28U << 16;
        s.phase = 1;
        break;
    case 1:
        s.x = (Sint32)((Uint32)s.x + (Uint32)s.velocity_x);
        s.y = (Sint32)((Uint32)s.y + (Uint32)s.velocity_y);
        s.velocity_x = Decelerate(s.velocity_x, 0x7800);
        if (in.pose == 6)
        {
            e.factories[n++] = 0x38U | hand | 0x1eU << 16;
            s.phase = 2;
        }
        break;
    case 2:
        s.blend = 0;
        if (in.pose == 8 && in.remaining == 1)
            e.factories[n++] = 0x38U | hand | 0x1eU << 16;
        if (in.pose == 10)
        {
            s.velocity_x = s.facing_left ? 0x80000 : -0x80000;
            s.velocity_y = 0x8000;
            s.phase = 3;
        }
        break;
    default:
        s.blend = 0x10;
        s.x = (Sint32)((Uint32)s.x + (Uint32)s.velocity_x);
        s.y = (Sint32)((Uint32)s.y + (Uint32)s.velocity_y);
        s.velocity_x = Decelerate(s.velocity_x, 0x7800);
        if (in.pose == 15)
        {
            s.removed = true;
            goto commit;
        }
        break;
    }
    s.draw_flags = in.draw_flags;
    s.scale_y = in.scale_y;
    s.pivot_y = in.pivot_y;
commit:
    *state = s;
    *events = e;
    return true;
}
