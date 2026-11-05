// Weapon func_ptr_80170004 secondaries (w_055 Holy sword, w_038/w_039 lunges) with authored tables
// only: mandatory
// LINE_G2 rows before RNG, raw-parent properties/group, hitbox shrink, post-callback UpdateAnim,
// blueprint56 runtime ownership, shared rows, holds, rebases and failure atomicity.
#include "actors_internal.h"
#include "boss_projection.h"
#include "character_fixture.h"
#include <sotn/bat_runtime.h>
#include <sotn/combat.h>
#include <sotn/game_math.h>
#include <sotn/weapon_secondary.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 n)
{
    p[0] = (Uint8)n;
    p[1] = (Uint8)(n >> 8);
}
static void Put32(Uint8 *p, Uint32 n)
{
    Put16(p, (Uint16)n);
    Put16(p + 2, (Uint16)(n >> 16));
}
// Authored weapon55 layout: two attack descriptors, frame stream, hitbox and the secondary's
// five-entry timeline; linear SDK quarter wave. No original tables.
static Uint16 Read16Test(const Uint8 *p)
{
    return (Uint16)(p[0] | p[1] << 8);
}
static void Program(Uint8 *o)
{
    for (Uint16 v = 0; v < 2; ++v)
    {
        Uint8 *d = o + 0x10ac + v * 16U;
        Put32(d, 0x8017a100);
        Put32(d + 4, 0x8017a200);
        Put16(d + 10, 0x611);
        d[12] = (Uint8)(v ? 0x50 : 0x41);
        d[13] = 4;
    }
    for (Uint16 i = 0; i < 7; ++i)
        Put32(o + 0x100 + i * 4U, 0x8017a300);
    for (Uint16 i = 0; i < 32; ++i)
        Put16(o + 0x300 + i * 2U, 512U + 23U);
    o[0x204] = 252;
    o[0x205] = 250;
    o[0x206] = 7;
    o[0x207] = 9;
    for (Uint16 i = 0; i < 4; ++i)
    {
        Put16(o + 0x1174 + i * 4U, 2);
        Put16(o + 0x1176 + i * 4U, (Uint16)(i + 1U));
    }
    Put16(o + 0x1184, 0xffff);
}
// Linear authored quarter wave at the SDK rsin/rcos table offset.
static void Math(Uint8 *exe)
{
    for (Uint32 i = 0; i <= 1024; ++i)
        Put16(exe + 0x1dbcc + i * 2, (Uint16)(i * 4));
}
static bool Properties(void *context, Uint32 *random, SotnEquipment *out)
{
    const Uint16 *hit = context;
    (void)SotnCombat_Random(random);
    *out = (SotnEquipment){.id = 196, .attack = 35, .hit_type = *hit, .element = 64};
    return true;
}
static Sint32 Floor256(Sint32 v)
{
    return v >= 0 ? v / 256 : -((-v + 255) / 256);
}
static bool Module(void)
{
    static Uint8 o[0x1400], exe[0x24000];
    Program(o);
    Math(exe);
    SotnHandWeaponProgram program = {0};
    Uint16 hit = 2;
    SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                   .executable = {exe, sizeof(exe)},
                                   .properties = Properties,
                                   .context = &hit,
                                   .x = 150 * 65536 + 99,
                                   .y = 120 * 65536 + 7,
                                   .camera_x = 30,
                                   .camera_y = 10,
                                   .scale_y = 240,
                                   .pivot_y = -3,
                                   .family = 55,
                                   .priority = 148,
                                   .animation = 0x43,
                                   .attack_mode = 0x801d,
                                   .draw_flags = 2,
                                   .allocated = true};
    for (Uint16 hand = 0; hand < 2; ++hand)
        for (Uint16 face = 0; face < 2; ++face)
        {
            in.hand = hand;
            in.facing_left = face != 0;
            SotnWeaponSecondary s = {0};
            SotnWeaponSecondaryEvents e = {0};
            Uint32 rng = 77, expected = 77;
            Sint16 angle[64], length[64];
            for (Uint16 i = 0; i < 64; ++i)
            {
                angle[i] = (Sint16)(SotnCombat_Random(&expected) & 0xfffU);
                length[i] = (Sint16)((SotnCombat_Random(&expected) & 0xffU) + 512U);
            }
            (void)SotnCombat_Random(&expected);
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == expected &&
                  e.properties && s.phase == 1 && s.allocated && s.flags == 0x960000U &&
                  s.bank == 0x8011 + hand * 2U && s.palette == 0x117 + hand * 24U &&
                  s.texture == 0x65 + hand * 2U && s.priority == 152 && s.row_priority == 150 &&
                  s.blend == 0x30 && s.hitbox_width == 32 && s.hitbox_height == 32 &&
                  s.hitbox_x == 12 && s.hitbox_y == -28 && s.properties.attack == 35 &&
                  s.scale_y == 240 && s.pivot_y == -3 && s.draw_flags == 2);
            const Sint32 x = 150 - 30 + (face ? -12 : 12), y = 120 - 10 - 28;
            for (Uint16 i = 0; i < 64; ++i)
            {
                const SotnWeaponSecondaryRow *r = &s.rows[i];
                SotnMathTrig t = {0};
                CHECK(SotnMath_Trig(in.executable, angle[i], &t));
                CHECK(r->red[1] == 115 && r->green[1] == 115 && r->blue[1] == 231 && !r->red[0] &&
                      r->angle == angle[i] && r->growth == length[i] / 2 &&
                      r->length == length[i] + length[i] / 2 && r->x[0] == x && r->y[0] == y &&
                      r->x[1] == x + Floor256(Floor256(t.cosine) * length[i]) &&
                      r->y[1] == y - Floor256(Floor256(t.sine) * length[i]));
            }
            // Post-callback UpdateAnim: two updates per frame, shrink at frame1/timer1, then
            // END keeps frame4 with timer-1 and the next callback retires before row writes.
            Uint16 callbacks = 1, shrink = 0;
            CHECK(SotnWeaponSecondary_Animate(&s, in.overlay, 55) && s.frame == 1 &&
                  s.remaining == 2);
            while (!s.removed)
            {
                const SotnWeaponSecondary before = s;
                const Uint32 kept = rng;
                CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == kept &&
                      !e.properties);
                ++callbacks;
                if (before.frame == 1 && before.remaining == 1)
                    shrink = callbacks;
                if (s.removed)
                    CHECK(before.remaining == -1 &&
                          !SDL_memcmp(s.rows, before.rows, sizeof(s.rows)));
                else
                    CHECK(SotnWeaponSecondary_Animate(&s, in.overlay, 55) &&
                          s.rows[5].blue[1] == (Uint8)(before.rows[5].blue[1] >= 17
                                                           ? before.rows[5].blue[1] - 24U
                                                           : before.rows[5].blue[1]));
            }
            CHECK(callbacks == 10 && shrink == 3 && s.hitbox_width == 12 && s.hitbox_x == 13 &&
                  s.frame == 4);
        }
    // Exhausted mandatory rows retire before any row write, RNG or property roll.
    SotnWeaponSecondary s = {0};
    SotnWeaponSecondaryEvents e = {0};
    Uint32 rng = 5;
    in.allocated = false;
    CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.removed && rng == 5 &&
          !e.properties && !s.phase && !s.rows[0].blue[1]);
    in.allocated = true;
    // Animation range and unk46 gate use the parameter-selected frameStart.
    const Uint16 animations[] = {0x40, 0x48, 0x43};
    const Uint16 modes[] = {0x801d, 0x801d, 0};
    for (Uint16 k = 0; k < 3; ++k)
    {
        s = (SotnWeaponSecondary){0};
        in.animation = animations[k];
        in.attack_mode = modes[k];
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.removed && rng == 5);
    }
    in.attack_mode = 0x801d;
    s = (SotnWeaponSecondary){.parameter = 0x100};
    in.animation = 0x50;
    CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && !s.removed && s.phase == 1);
    // Hit type4 keeps the original attack255 override.
    hit = 4;
    s = (SotnWeaponSecondary){0};
    in.animation = 0x41;
    CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.properties.attack == 255 &&
          s.hitbox_state == 4);
    // Failures leave every word, the RNG cursor and events unchanged.
    const SotnWeaponSecondary before = s;
    const Uint32 kept = rng;
    in.family = 54;
    CHECK(!SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) &&
          !SDL_memcmp(&s, &before, sizeof(s)) && rng == kept &&
          !SotnWeaponSecondary_Animate(&s, in.overlay, 54));
    in.family = 55;
    in.overlay.size = 0x1000;
    CHECK(!SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) &&
          !SDL_memcmp(&s, &before, sizeof(s)) && !SotnWeaponSecondary_Animate(&s, in.overlay, 55) &&
          !SDL_memcmp(&s, &before, sizeof(s)));
    in.overlay.size = sizeof(o);
    in.executable.size = 0x1000;
    CHECK(!SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) &&
          !SDL_memcmp(&s, &before, sizeof(s)) && rng == kept);
    CHECK(SotnWeaponSecondary_Supported(55) && !SotnWeaponSecondary_Supported(54) &&
          SotnWeaponSecondary_Rows(55) == 64 && SotnWeaponSecondary_Rows(56) == 1 &&
          !SotnWeaponSecondary_Rows(57));
    return true;
}
// w_038/w_039 lunge sparks: mandatory rows before RNG, step_s0x5A/unk46 gates, program-BSS
// countdown with ANIME_SWORD_B/VO_ALU_ATTACK_C, Rapier's cycling rows and Were bane's chain.
static bool Lunge(void)
{
    Uint16 hit = 2;
    for (Uint16 family = 38; family <= 39; ++family)
    {
        const Uint16 rows = family == 38 ? 24 : 32;
        SotnWeaponSecondaryInput in = {.properties = Properties,
                                       .context = &hit,
                                       .x = 150 * 65536 + 99,
                                       .y = 120 * 65536 + 7,
                                       .camera_x = 30,
                                       .camera_y = 10,
                                       .family = family,
                                       .priority = 148,
                                       .attack_mode = 0x801b,
                                       .substep = 0x5a,
                                       .draw_flags = 2,
                                       .allocated = true};
        SotnWeaponSecondary s = {0};
        SotnHandWeaponProgram program = {.secondary_counter = 7, .secondary_x = {99}};
        SotnWeaponSecondaryEvents e = {0};
        Uint32 rng = 41, expected = 41;
        for (Uint16 i = 0; i < (family == 38 ? rows : 0); ++i)
            (void)SotnCombat_Random(&expected);
        (void)SotnCombat_Random(&expected);
        // Row0's countdown starts at one, so it is placed in the birth callback (two calls).
        const Sint32 dx = (Sint32)(SotnCombat_Random(&expected) % 24U) - 12,
                     dy = (Sint32)(SotnCombat_Random(&expected) % 18U) - 9;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == expected &&
              e.properties && !e.sounds[0] && s.phase == 1 && s.flags == 0x860000U && !s.bank &&
              !s.frame && s.draw_mode == 0x331 && s.row_priority == 152 && s.hitbox_width == 24 &&
              s.hitbox_height == 10 && s.hitbox_x == 38 && s.hitbox_y == -12 &&
              program.secondary_counter == 40);
        CHECK(!s.rows[0].hidden && s.rows[0].phase == 1 && s.rows[1].hidden &&
              s.rows[rows - 1U].hidden && s.rows[0].y[0] == s.rows[0].y[1]);
        const Sint32 x = 150 - 30 + 34, y = 120 - 10 - 14 - 3;
        CHECK(s.rows[0].y[0] == y + dy && s.rows[0].x[0] == x + dx + (family == 38 ? -12 : -16));
        if (family == 38)
            CHECK(s.rows[0].timer == 3 && s.rows[1].timer == 1 && s.rows[0].red[0] == 15 &&
                  s.rows[0].green[1] == 255 && s.rows[0].red[1] >= 63 && s.rows[0].red[1] < 79 &&
                  s.rows[0].x[1] - s.rows[0].x[0] == 28 && s.rows[0].x[0] >= x - 24 &&
                  s.rows[0].x[0] < x);
        else
            CHECK(s.rows[0].timer == 9 && s.rows[1].timer == 2 && s.rows[0].red[0] == 0xbf &&
                  s.rows[0].blue[1] == 0xff && s.rows[0].x[1] - s.rows[0].x[0] == 32 &&
                  program.secondary_x[0] == s.rows[0].x[1] &&
                  program.secondary_y[0] == s.rows[0].y[1]);
        // The countdown queues ANIME_SWORD_B at counter&7==7 and ends with VO_ALU_ATTACK_C.
        Uint16 swish = 0, callbacks = 1;
        while (!s.removed)
        {
            const Sint32 counter = program.secondary_counter;
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && !e.properties);
            ++callbacks;
            swish += e.sounds[0] == 0x640;
            CHECK(e.sounds[0] == ((counter & 7) == 7 ? 0x640 : counter == 1 ? 0x6f0 : 0));
            if (family == 39 && callbacks == 4)
                CHECK(!s.rows[1].hidden && s.rows[1].x[0] == s.rows[0].x[1] &&
                      s.rows[1].y[0] == s.rows[0].y[1] && program.secondary_x[1] == s.rows[1].x[1]);
            if (family == 38 && callbacks == 4)
                CHECK(s.rows[0].hidden && s.rows[0].phase == 0 && s.rows[0].timer == 2);
        }
        CHECK(callbacks == 41 && swish == 5 && !program.secondary_counter);
        // Leaving step_s0x5A or clearing unk46 retires before rows, RNG or the countdown.
        for (Uint16 k = 0; k < 2; ++k)
        {
            s = (SotnWeaponSecondary){0};
            program.secondary_counter = 12;
            in.substep = k ? 0x5a : 0x50;
            in.attack_mode = k ? 0 : 0x801b;
            const Uint32 kept = rng;
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.removed &&
                  rng == kept && program.secondary_counter == 12 && !e.properties);
        }
        in.substep = 0x5a;
        in.attack_mode = 0x801b;
        // Exhausted mandatory rows retire without RNG; failures commit no program BSS.
        s = (SotnWeaponSecondary){0};
        in.allocated = false;
        const Uint32 kept = rng;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.removed && rng == kept &&
              program.secondary_counter == 12);
        in.allocated = true;
        s = (SotnWeaponSecondary){0};
        in.properties = NULL;
        CHECK(!SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && !s.phase &&
              program.secondary_counter == 12 && rng == kept);
        in.properties = Properties;
        CHECK(SotnWeaponSecondary_Rows(family) == rows && SotnWeaponSecondary_Supported(family));
    }
    return true;
}
// Authored weapon53 slash tables: config rows at0xfc4 (u/v flip, CLUT order, radius, rotation,
// RGB) and the two nine-entry CLUT schedules at0xf9c/0xfb0.
static void SlashProgram(Uint8 *o)
{
    for (Uint16 i = 0; i < 48; ++i)
    {
        Uint8 *c = o + 0xfc4 + i * 14U;
        Put16(c, (Uint16)(i & 1U));
        Put16(c + 2, (Uint16)((i >> 1) & 1U));
        Put16(c + 4, (Uint16)((i >> 2) & 1U));
        Put16(c + 6, (Uint16)(24 + i));
        Put16(c + 8, (Uint16)(i * 300U));
        c[10] = (Uint8)(i + 1U);
        c[11] = 96;
        c[12] = (Uint8)(200U - i);
    }
    for (Uint16 i = 0; i < 9; ++i)
    {
        Put16(o + 0xf9c + i * 2U, (Uint16)(0x11a + i));
        Put16(o + 0xfb0 + i * 2U, (Uint16)(0x122 - i));
    }
}
static bool Slash(void)
{
    static Uint8 o[0x1400], exe[0x24000];
    SlashProgram(o);
    Math(exe);
    Uint16 hit = 2;
    SotnHandWeaponProgram program = {0};
    for (Uint16 k = 0; k < 4; ++k)
    {
        const Uint16 param = k == 0 ? 0 : k == 1 ? 1 : k == 2 ? 30 : 41;
        const bool root = param == 0 || param == 30, properties = root || param == 41;
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = {exe, sizeof(exe)},
                                       .properties = Properties,
                                       .context = &hit,
                                       .x = 150 * 65536 + 99,
                                       .y = 120 * 65536 + 7,
                                       .camera_x = 30,
                                       .camera_y = 10,
                                       .family = 53,
                                       .hand = (Uint16)(k & 1U),
                                       .priority = 148,
                                       .facing_left = k == 3,
                                       .crouch = k == 1 || k == 2,
                                       .allocated = true};
        SotnWeaponSecondary s = {
            .parameter = (Uint16)(param << 8), .x = 100 * 65536 + 0x1111, .y = 90 * 65536 + 0x2222};
        SotnWeaponSecondaryEvents e = {0};
        Uint32 rng = 3, expected = 3;
        if (properties)
            (void)SotnCombat_Random(&expected);
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == expected &&
              e.properties == properties && s.phase == 1 && s.age == 1 && s.flags == 0x08820000U &&
              s.quad.page == 0x19 && s.quad.priority == 150 && s.quad.draw_mode == 0x137 &&
              s.lifetime == 24 + param && s.rotate == (Sint16)(param * 300U) &&
              s.quad.red[2] == param + 1U && s.quad.blue[0] == 200U - param);
        CHECK(e.sounds[0] == (param % 4 == 0 ? 0x60c : 0) &&
              e.chain == (root ? 0x38U | ((k & 1U) + 1U) << 12 | (Uint32)(param + 1U) << 16 : 0U));
        const Uint8 v = (k & 1U) ? 0x80 : 0;
        CHECK(s.quad.u[0] == ((param & 1U) ? 0xc0 : 0x80) &&
              s.quad.v[0] == (((param >> 1) & 1U) ? (v | 0x40) : v) &&
              s.quad.palette == ((k & 1U) ? 0x18 : 0) + ((param >> 2) & 1U ? 0x122 : 0x11a));
        if (properties)
            CHECK(s.hitbox_width == 30 && s.hitbox_height == 32 &&
                  s.hitbox_x == (param >= 30 ? 0x12 : 0));
        else
            CHECK(!s.hitbox_width && !s.hitbox_state);
        // Params below0x1e follow the player at (+-38, crouch11/-5) with their own fraction;
        // others keep their birth offset (+-20, -5/11) from the factory position.
        if (param < 0x1e)
            CHECK(s.x == (150 + (k == 3 ? -38 : 38)) * 65536 + 0x1111 &&
                  s.y == (120 + (k == 1 ? 11 : -5)) * 65536 + 0x2222);
        else
            CHECK(s.x == (100 + (k == 3 ? -20 : 20)) * 65536 + 0x1111 &&
                  s.y == (90 + (k == 2 ? 11 : -5)) * 65536 + 0x2222);
        SotnMathTrig t = {0};
        CHECK(SotnMath_Trig(in.executable, (Sint16)(s.rotate + 0x600), &t));
        const Sint32 c16 = t.cosine >= 0 ? t.cosine / 16 : -((-t.cosine + 15) / 16),
                     s16 = t.sine >= 0 ? t.sine / 16 : -((-t.sine + 15) / 16);
        CHECK(c16 && s16);
        CHECK(s.quad.x[0] == (k == 3 ? -1 : 1) * Floor256(c16 * s.lifetime) &&
              s.quad.y[0] == -Floor256(s16 * s.lifetime));
        Uint16 callbacks = 1;
        while (!s.removed)
        {
            const Uint16 age = s.age;
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && !e.properties);
            ++callbacks;
            CHECK(e.chain ==
                  (root ? 0x38U | ((k & 1U) + 1U) << 12 | (Uint32)(age + param + 1U) << 16 : 0U));
        }
        CHECK(callbacks == 9 && s.age == 9);
    }
    // Parameters beyond the private table and exhausted GT4 rows.
    SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                   .executable = {exe, sizeof(exe)},
                                   .properties = Properties,
                                   .context = &hit,
                                   .family = 53,
                                   .allocated = false};
    SotnWeaponSecondary s = {.parameter = 50 << 8};
    SotnWeaponSecondaryEvents e = {0};
    Uint32 rng = 3;
    CHECK(!SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && !s.phase);
    // Row48 lies past the table: its CLUT-order word is the live g_HandId at0x1268, never the
    // image's stored bytes.
    o[0x1268] = 0x5a;
    in.allocated = true;
    for (Uint16 hand = 0; hand < 2; ++hand)
    {
        in.hand = hand;
        s = (SotnWeaponSecondary){.parameter = 48 << 8};
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 1 &&
              s.quad.palette == (hand ? 0x18 + 0x122 : 0x11a));
    }
    in.hand = 0;
    in.allocated = false;
    rng = 3;
    s = (SotnWeaponSecondary){0};
    CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.removed && rng == 3 &&
          !e.chain && SotnWeaponSecondary_Rows(53) == 1);
    return true;
}
// Authored weapon48 tables: four bolt colors at0xfe0 and seven per-animation anchor rows at0x1000.
static void ThunderProgram(Uint8 *o)
{
    const Uint16 colors[4][3] = {{128, 128, 128}, {64, 176, 16}, {32, 208, 16}, {0, 255, 64}};
    for (Uint16 i = 0; i < 4; ++i)
        for (Uint16 c = 0; c < 3; ++c)
            Put16(o + 0xfe0 + i * 8U + c * 2U, colors[i][c]);
    for (Uint16 a = 0; a < 7; ++a)
    {
        Put16(o + 0x1000 + a * 8U, (Uint16)(20 + a));
        Put16(o + 0x1002 + a * 8U, (Uint16)(Sint16)(-10 + a));
        Put16(o + 0x1004 + a * 8U, (Uint16)(60 - a));
        Put16(o + 0x1006 + a * 8U, (Uint16)(a * 4U));
    }
}
static bool Thunder(void)
{
    static Uint8 o[0x1400], exe[0x24000];
    ThunderProgram(o);
    Math(exe);
    Uint16 hit = 2;
    SotnHandWeaponProgram program = {0};
    const Uint16 params[] = {0, 3, 0x11, 0x20, 0x22};
    for (Uint16 k = 0; k < SDL_arraysize(params); ++k)
    {
        const Uint16 param = params[k];
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = {exe, sizeof(exe)},
                                       .properties = Properties,
                                       .context = &hit,
                                       .x = 150 * 65536 + 99,
                                       .y = 120 * 65536 + 7,
                                       .camera_x = 30,
                                       .camera_y = 10,
                                       .family = 48,
                                       .role = 1,
                                       .hand = (Uint16)(k & 1U),
                                       .priority = 148,
                                       .animation = 0x43,
                                       .attack_mode = 0x8014,
                                       .facing_left = k == 2,
                                       .allocated = true};
        SotnWeaponSecondary s = {.parameter = (Uint16)(param << 8)};
        SotnWeaponSecondaryEvents e = {0};
        Uint32 rng = 21, expected = 21;
        Sint16 angles[10];
        for (Uint16 i = 0; i < 10; ++i)
            angles[i] = (Sint16)SotnCombat_Random(&expected);
        (void)SotnCombat_Random(&expected);
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == expected &&
              e.properties && s.phase == 1 && s.flags == 0x860000U && s.draw_mode == 0x335 &&
              s.row_priority == 152 && s.remaining == (param < 16 ? 40 : 6));
        // Colors fade along the bolt; each row keeps its rand angle advanced by768.
        const Uint16 c = param & 0xfU, red = c == 0 ? 128 : c == 1 ? 64 : c == 2 ? 32 : 0;
        CHECK(s.rows[0].red[0] == red && s.rows[9].red[1] == 0 &&
              s.rows[4].red[1] == red * 5U / 10U && s.rows[3].angle == (Sint16)(angles[3] + 768));
        if (param < 16)
            CHECK(s.hitbox_width == 4 && s.hitbox_height == 20 && s.hitbox_y == -20 &&
                  s.rows[0].x[0] == 150 - 30 + 12 && s.rows[0].y[0] == 120 - 10 - 26);
        else if (param >= 32 && !(param & 0xfU))
            CHECK(s.hitbox_width == 32 && s.hitbox_height == 6 && s.hitbox_x == 32 &&
                  s.hitbox_state == 2);
        else
            CHECK(!s.hitbox_width && s.hitbox_state == (param >= 32 ? 0 : 2));
        if (param >= 16)
        {
            // Anchor row2 (animation0x43): +-22 then -8 vertically; extents halve below0x20.
            const Sint16 ax = k == 2 ? -22 : 22;
            CHECK(s.rows[0].x[0] == 150 - 30 + ax && s.rows[0].y[0] == 120 - 10 - 8);
            // The bolt ends within its three-pixel jitter of the (halved below0x20) extent58/8.
            const Sint32 reach = (param < 32 ? 29 : 58) * (k == 2 ? -1 : 1);
            CHECK(SDL_abs(s.rows[9].x[1] - (s.rows[0].x[0] + reach)) <= 3 &&
                  SDL_abs(s.rows[9].y[1] - (s.rows[0].y[0] + (param < 32 ? 4 : 8))) <= 3);
        }
        Uint16 callbacks = 1;
        while (!s.removed)
        {
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && !e.properties);
            ++callbacks;
        }
        CHECK(callbacks == (param < 16 ? 41 : 7));
    }
    // The Y-scale flag lifts bolts (three more below parameter16) and leaving0x41..0x47 retires.
    SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                   .executable = {exe, sizeof(exe)},
                                   .properties = Properties,
                                   .context = &hit,
                                   .x = 150 * 65536,
                                   .y = 120 * 65536,
                                   .family = 48,
                                   .role = 1,
                                   .animation = 0x43,
                                   .attack_mode = 1,
                                   .draw_flags = 2,
                                   .allocated = true};
    SotnWeaponSecondary s = {0};
    SotnWeaponSecondaryEvents e = {0};
    Uint32 rng = 1;
    CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.rows[0].y[0] == 120 - 26 - 4);
    in.animation = 0x48;
    CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.removed);
    // Role2 ring: sixteen GT4 wedges, radius4 growing by4 to24, blue fading over forty updates.
    in = (SotnWeaponSecondaryInput){.overlay = {o, sizeof(o)},
                                    .executable = {exe, sizeof(exe)},
                                    .x = 150 * 65536,
                                    .y = 120 * 65536,
                                    .camera_x = 30,
                                    .camera_y = 10,
                                    .family = 48,
                                    .role = 2,
                                    .priority = 148,
                                    .animation = 0x44,
                                    .attack_mode = 1,
                                    .allocated = true};
    s = (SotnWeaponSecondary){0};
    rng = 7;
    CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == 7 && !e.properties &&
          e.sounds[0] == 0x665 && s.remaining == 4 && s.age == 0x28 && !s.hitbox_width);
    const SotnPlayerEffectQuad *q = &s.quads[0];
    CHECK(q->x[2] == 150 - 30 + 12 && q->y[2] == 120 - 10 - 26 && q->x[0] == q->x[2] + 4 &&
          q->y[0] == q->y[2] && q->u[0] == 32 + 32 && q->v[0] == 0xe0 && s.quads[12].v[0] == 0xff &&
          s.quads[4].v[0] == 0xc0 && q->u[2] == 0x20 && q->v[2] == 0xe0 && q->palette == 0x15f &&
          q->page == 0x1a && q->blue[2] == 0x80 && !q->blue[0] && q->draw_mode == 0x335 &&
          q->priority == 150);
    Uint16 callbacks = 1;
    while (!s.removed)
    {
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
        ++callbacks;
        if (!s.removed)
            CHECK(s.remaining == SDL_min(4 + 4 * (callbacks - 1), 24) &&
                  s.quads[5].blue[2] == (s.age < 16 ? s.age * 8 : 0x80));
    }
    CHECK(callbacks == 0x29 && SotnWeaponSecondary_RoleRows(48, 2) == 16 &&
          SotnWeaponSecondary_RoleRows(48, 1) == 10 && !SotnWeaponSecondary_RoleRows(53, 2));
    in.role = 2;
    in.family = 55;
    s = (SotnWeaponSecondary){0};
    CHECK(!SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && !s.phase);
    return true;
}
// w_053 func_ptr_80170008: out at speed8 (-0.5 up), decelerating15/32 per update, root0x1E at
// pose6 and pose8's last update, back at pose10 and retired at pose15; variant1 adds root0x28.
static bool Warp(void)
{
    for (Uint16 k = 0; k < 4; ++k)
    {
        const bool masamune = k & 1U, left = k >> 1;
        SotnWeaponWarp w = {.x = 100 * 65536, .y = 80 * 65536, .parameter = masamune ? 0x100 : 0};
        SotnWeaponWarpEvents e = {0};
        SotnWeaponWarpInput in = {.scale_y = 240,
                                  .pivot_y = -3,
                                  .hand = 1,
                                  .draw_flags = 4,
                                  .facing_left = left,
                                  .remaining = 2};
        CHECK(SotnWeaponWarp_Tick(&w, in, &e) && w.phase == 1 && w.blend == 0x10 &&
              w.flags == 0x04020000U && w.velocity_x == (left ? -0x80000 : 0x80000) &&
              w.velocity_y == -0x8000 && w.x == 100 * 65536 && w.draw_flags == 4 &&
              w.scale_y == 240 && e.factories[0] == (masamune ? 0x282038U : 0U));
        CHECK(SotnWeaponWarp_Tick(&w, in, &e) && w.x == 100 * 65536 + (left ? -0x80000 : 0x80000) &&
              w.y == 80 * 65536 - 0x8000 && w.velocity_x == (left ? -0x78800 : 0x78800) &&
              !e.factories[0]);
        in.pose = 6;
        CHECK(SotnWeaponWarp_Tick(&w, in, &e) && w.phase == 2 && e.factories[0] == 0x1e2038U);
        in.pose = 8;
        in.remaining = 1;
        CHECK(SotnWeaponWarp_Tick(&w, in, &e) && !w.blend && e.factories[0] == 0x1e2038U);
        in.remaining = 2;
        CHECK(SotnWeaponWarp_Tick(&w, in, &e) && !e.factories[0]);
        in.pose = 10;
        CHECK(SotnWeaponWarp_Tick(&w, in, &e) && w.phase == 3 &&
              w.velocity_x == (left ? 0x80000 : -0x80000) && w.velocity_y == 0x8000);
        const Sint32 x = w.x;
        CHECK(SotnWeaponWarp_Tick(&w, in, &e) && w.blend == 0x10 &&
              w.x == x + (left ? 0x80000 : -0x80000));
        in.pose = 15;
        CHECK(SotnWeaponWarp_Tick(&w, in, &e) && w.removed);
        CHECK(!SotnWeaponWarp_Tick(&w, in, &e));
    }
    return true;
}
// Authored weapon1 layout: five descriptors (frameStart0x24), trail colors/offsets, the punch
// UVs and the fireball's eight-frame looping timeline.
static void FistProgram(Uint8 *o)
{
    Program(o);
    for (Uint16 v = 0; v < 5; ++v)
    {
        SDL_memcpy(o + 0xf44 + v * 16U, o + 0x10ac, 16);
        o[0xf44 + v * 16U + 12] = 0x24;
    }
    for (Uint16 i = 0; i < 8; ++i)
        o[0xfb0 + i] = (Uint8)(0x20 + i);
    const Uint8 uv[] = {24, 80, 40, 80, 24, 96, 40, 96};
    SDL_memcpy(o + 0xfb8, uv, sizeof(uv));
    for (Uint16 i = 0; i < 8; ++i)
    {
        Put16(o + 0xfc8 + i * 4U, 1);
        Put16(o + 0xfca + i * 4U, (Uint16)(i + 1U));
    }
    Put32(o + 0xfe8, 0);
}
// w_001 secondaries: the slot16 punch rush and Tulkas's fireball.
static bool Fist(void)
{
    static Uint8 o[0x3000];
    FistProgram(o);
    const Uint16 hit = 2;
    for (Uint16 k = 0; k < 4; ++k)
    {
        const bool left = k & 1U, hand = k >> 1;
        SotnWeaponSecondary s = {.parameter = (Uint16)(0x100 | (hand ? 0x8000 : 0))};
        SotnHandWeaponProgram program = {0};
        SotnWeaponSecondaryEvents e = {0};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .properties = Properties,
                                       .context = (void *)&hit,
                                       .x = 150 * 65536 + 77,
                                       .y = 120 * 65536,
                                       .camera_x = 10,
                                       .camera_y = 4,
                                       .hand = hand,
                                       .family = 1,
                                       .role = 1,
                                       .priority = 148,
                                       .animation = 0x26,
                                       .attack_mode = 0x8019,
                                       .facing_left = left,
                                       .allocated = true};
        Uint32 rng = 77, check = 77;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 1 && s.allocated &&
              s.flags == 0x860000U && s.remaining == 128 && s.hitbox_width == 18 &&
              s.hitbox_height == 10 && s.hitbox_x == 28 && s.hitbox_y == -12 && e.properties &&
              !e.sounds[0]);
        (void)SotnCombat_Random(&check);
        const Sint32 dx = (Sint32)((SotnCombat_Random(&check) & 0x7fffU) % 24U) - 12,
                     dy = (Sint32)((SotnCombat_Random(&check) & 0x7fffU) % 18U) - 9;
        const Sint32 x = 140 + (left ? -26 : 26) + dx, y = 116 - 6 + dy;
        const SotnPlayerEffectQuad q = s.quads[0];
        CHECK(rng == check && q.x[0] == x + (left ? 8 : -8) && q.x[1] == x + (left ? -8 : 8) &&
              q.x[2] == q.x[0] && q.y[0] == y - 8 && q.y[2] == y + 8 && !(q.draw_mode & 8U) &&
              q.draw_mode == 0x371 && q.page == 0x19 && q.palette == 0x100 && q.priority == 152 &&
              q.u[0] == 24 && q.u[3] == 40 && q.v[0] == (hand ? 208 : 80) &&
              q.v[3] == (hand ? 224 : 96) && s.quad_phase[0] == 1 && s.quad_timer[0] == 2);
        CHECK((s.quads[1].draw_mode & 8U) && s.quad_timer[1] == 1 && s.quad_timer[15] == 15);
        // Quad0 swaps to the second image, then back and hides; quad1 appears now.
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.quads[0].u[0] == 24 &&
              !(s.quads[1].draw_mode & 8U) && s.quad_timer[0] == 1);
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.quads[0].u[0] == 72 &&
              s.quad_phase[0] == 2);
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) &&
              SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.quads[0].u[0] == 24 &&
              (s.quads[0].draw_mode & 8U) && !s.quad_phase[0] && s.quad_timer[0] == 2);
        // Swish every eighth update; the last update plays SFX_VO_ALU_ATTACK_C and retires.
        Uint16 swish = 0, t = 0;
        for (; t < 200 && !s.removed; ++t)
        {
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
            swish += e.sounds[0] == 0x60c;
            if (s.removed)
                CHECK(e.sounds[e.sounds[0] == 0x60c ? 1 : 0] == 0x6f0);
        }
        // Remaining124..1 swish at119..7 (the127 swish came earlier).
        CHECK(s.removed && t == 124 && swish == 15);
    }
    // Leaving the descriptor's window, losing attack mode or the rows retires without RNG.
    for (Uint16 k = 0; k < 4; ++k)
    {
        SotnWeaponSecondary s = {0};
        SotnHandWeaponProgram program = {0};
        SotnWeaponSecondaryEvents e = {0};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .properties = Properties,
                                       .context = (void *)&hit,
                                       .family = 1,
                                       .role = 1,
                                       .animation = k == 0   ? 0x23
                                                    : k == 1 ? 0x2b
                                                             : 0x24,
                                       .attack_mode = k == 2 ? 0 : 0x8019,
                                       .allocated = k != 3};
        Uint32 rng = 5;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.removed && rng == 5 &&
              !s.allocated && !e.properties);
    }
    // The fireball: speed4.5 away from the player's facing, six steps ahead, 13 up (16 scaled).
    for (Uint16 k = 0; k < 2; ++k)
    {
        const bool left = k != 0;
        SotnWeaponSecondary s = {.x = 100 * 65536 + 5, .y = 90 * 65536 + 9};
        SotnHandWeaponProgram program = {0};
        SotnWeaponSecondaryEvents e = {0};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .properties = Properties,
                                       .context = (void *)&hit,
                                       .family = 1,
                                       .role = 2,
                                       .priority = 148,
                                       .draw_flags = (Uint8)(left ? 2 : 0),
                                       .facing_left = left};
        Uint32 rng = 9;
        const Sint32 vx = left ? 0x48000 : -0x48000;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 1 &&
              s.flags == 0x08100000U && s.bank == 9 && s.priority == 146 &&
              s.facing_left == !left && s.velocity_x == -vx && s.palette == 0x81b0 &&
              s.blend == 0x30 && s.x == 100 * 65536 + 5 - vx * 6 &&
              s.y == (90 - (left ? 16 : 13)) * 65536 + 9 && s.draw_flags == 3 &&
              s.scale_x == 0x60 && s.scale_y == 0x60 && s.hitbox_width == 10 &&
              s.hitbox_height == 8 && e.sounds[0] == 0x62c && e.properties && rng != 9);
        CHECK(SotnWeaponSecondary_Animate(&s, (SotnDataView){o, sizeof(o)}, 1) && s.frame == 1);
        for (Uint16 t = 1; t <= 24; ++t)
        {
            const Sint32 x = s.x;
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.x == x - vx &&
                  s.scale_x == SDL_min(0x60 + t * 0x10, 0x180) &&
                  s.scale_y == SDL_min(0x60 + t * 0x10, 0x100) && !e.sounds[0]);
            CHECK(SotnWeaponSecondary_Animate(&s, (SotnDataView){o, sizeof(o)}, 1) &&
                  s.frame == t % 8U + 1U);
        }
        s.hit_flags = 1;
        const Sint32 x = s.x;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.removed && s.x == x);
    }
    CHECK(SotnWeaponSecondary_SupportedRole(1, 1) && SotnWeaponSecondary_SupportedRole(1, 2) &&
          SotnWeaponSecondary_RoleRows(1, 1) == 16 && !SotnWeaponSecondary_RoleRows(1, 2));
    return true;
}
// Authored weapon51 layout: the special's descriptors at 0xd18 (frameStart0xD6), fan
// AnimProperties {flipped, timer, rotation} at 0xe74 and tints at 0xea4.
static void Weapon51Program(Uint8 *o)
{
    Program(o);
    for (Uint16 v = 0; v < 4; ++v)
    {
        SDL_memcpy(o + 0xcd8 + v * 16U, o + 0x10ac, 16);
        SDL_memcpy(o + 0xd18 + v * 16U, o + 0x10ac, 16);
        o[0xd18 + v * 16U + 12] = 0xd6;
    }
    for (Uint16 i = 0; i < 8; ++i)
    {
        Put16(o + 0xe74 + i * 6U, (Uint16)(i & 1U));
        Put16(o + 0xe76 + i * 6U, (Uint16)(37U - (i & 3U) * 4U));
        Put16(o + 0xe78 + i * 6U, (Uint16)(0x100U * i));
    }
    for (Uint16 i = 0; i < 16; ++i)
        Put16(o + 0xea4 + i * 2U, (Uint16)(0x40U + i * 8U));
}
// w_051 func_ptr_80170008: blades slide out from the player at speed6+rand/32768, the root
// attacks and chains FACTORY(0x3E,n) children each callback, and every blade retires at nine.
static bool Fan(void)
{
    static Uint8 o[0x3000], exe[0x24000];
    Weapon51Program(o);
    Math(exe);
    const Uint16 hit = 2;
    for (Uint16 k = 0; k < 4; ++k)
    {
        const bool left = k & 1U;
        const Uint16 hand = k >> 1, sub = k == 3 ? 4 : 0, color = k == 3 ? 3 : 0;
        SotnWeaponSecondary s = {.parameter = (Uint16)(sub << 8)};
        SotnHandWeaponProgram program = {0};
        SotnWeaponSecondaryEvents e = {0};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = {exe, sizeof(exe)},
                                       .properties = Properties,
                                       .context = (void *)&hit,
                                       .x = 100 * 65536 + 7,
                                       .y = 80 * 65536 + 9,
                                       .hand = hand,
                                       .family = 51,
                                       .role = 2,
                                       .priority = 148,
                                       .facing_left = left,
                                       .allocated = true,
                                       .slot16_parameter = (Uint16)(color << 8 | hand << 15)};
        Uint32 rng = 21, check = 21;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 1 && s.allocated &&
              s.flags == 0x08820000U && s.age == 1 && s.rotate == (Sint16)(0x100 * (sub % 8)) &&
              s.lifetime == 37 - (sub & 3) * 4);
        Uint8 tint[3] = {0};
        for (Uint16 c = 0; c < 3; ++c)
            tint[c] = (Uint8)(0x40U + (color * 4U + c) * 8U +
                              ((SotnCombat_Random(&check) & 0x7fffU) & 15U));
        if (!sub)
            (void)SotnCombat_Random(&check);
        const Sint32 speed = (Sint32)(SotnCombat_Random(&check) & 0x7fffU) * 2 + 0x60000;
        const Sint32 dx = color ? 30 : 20, velocity = left ? -speed : speed;
        const SotnPlayerEffectQuad q = s.quad;
        CHECK(rng == check && s.x == 100 * 65536 + 7 + (left ? -dx : dx) * 65536 + velocity &&
              s.y == 62 * 65536 + 9 &&
              s.velocity_x == (left ? velocity + 0x2800 : velocity - 0x2800) && q.u[0] == 128 &&
              q.u[1] == 192 && q.v[0] == (hand ? 0x80 : 0) && q.v[2] == (hand ? 0xc0 : 0x40) &&
              q.red[1] == tint[0] && q.red[3] == tint[0] && q.green[1] == tint[1] &&
              q.blue[3] == tint[2] && !q.red[0] && !q.blue[2] && q.palette == 0x15f &&
              q.page == 0x19 && q.priority == 152 && q.draw_mode == (color == 3 ? 0x117 : 0x177) &&
              e.sounds[0] == 0x60c);
        if (!sub)
            CHECK(e.properties && s.hitbox_width == 30 && s.hitbox_height == 16 &&
                  s.hitbox_x == 8 && e.chain == (0x3eU | (hand + 1U) << 12 | 1U << 16));
        else
            CHECK(!e.properties && !s.hitbox_width && !e.chain);
        // Mirrored facing negates the X corners only.
        SotnWeaponSecondary mirror = {.parameter = s.parameter};
        Uint32 mirror_rng = 21;
        SotnWeaponSecondaryInput other = in;
        other.facing_left = !left;
        CHECK(SotnWeaponSecondary_Tick(&mirror, &program, other, &mirror_rng, &e));
        for (Uint16 v = 0; v < 4; ++v)
            CHECK(mirror.quad.x[v] == -q.x[v] && mirror.quad.y[v] == q.y[v]);
        Uint16 t = 1;
        for (; t < 20 && !s.removed; ++t)
        {
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && !e.sounds[0]);
            if (!sub)
                CHECK(e.chain == (0x3eU | (hand + 1U) << 12 | (Uint32)(t + 1U) << 16));
        }
        CHECK(s.removed && t == 9 && s.age == 9);
    }
    // Losing the mandatory GT4 retires before any RNG.
    SotnWeaponSecondary s = {0};
    SotnHandWeaponProgram program = {0};
    SotnWeaponSecondaryEvents e = {0};
    Uint32 rng = 5;
    CHECK(SotnWeaponSecondary_Tick(
              &s, &program,
              (SotnWeaponSecondaryInput){
                  .overlay = {o, sizeof(o)}, .family = 51, .role = 2, .properties = Properties},
              &rng, &e) &&
          s.removed && rng == 5 && !e.properties);
    CHECK(SotnWeaponSecondary_SupportedRole(51, 2) && !SotnWeaponSecondary_SupportedRole(51, 1) &&
          SotnWeaponSecondary_RoleRows(51, 2) == 1);
    return true;
}
// Authored weapon49 layout: descriptors at 0xffc, blade anchors at 0x103c and the sparks/puffs/
// pillar timelines (eight looping frames, then thirteen frames ending in END).
static void Weapon49Program(Uint8 *o)
{
    Program(o);
    for (Uint16 v = 0; v < 4; ++v)
        SDL_memcpy(o + 0xffc + v * 16U, o + 0x10ac, 16);
    for (Uint16 i = 0; i < 7; ++i)
    {
        Put16(o + 0x103c + i * 8U, 24);
        Put16(o + 0x103e + i * 8U, (Uint16)-14);
        Put16(o + 0x1040 + i * 8U, 64);
        Put16(o + 0x1042 + i * 8U, (Uint16)(i * 6U));
    }
    for (Uint16 i = 0; i < 8; ++i)
    {
        Put16(o + 0x1074 + i * 4U, 1);
        Put16(o + 0x1076 + i * 4U, (Uint16)(i + 1U));
    }
    Put32(o + 0x1094, 0);
    for (Uint16 t = 0; t < 2; ++t)
    {
        const Uint32 at = t ? 0x110cU : 0x10d4U;
        for (Uint16 i = 0; i < 13; ++i)
        {
            Put16(o + at + i * 4U, 2);
            Put16(o + at + 2U + i * 4U, (Uint16)(i + 1U));
        }
        Put16(o + at + 52U, 0xffff);
    }
}
// w_049: sparks copy the parent's attack and take func_80118894's group; spark0 requests the
// ring at lifetime0x18; the ring grows and fades; puffs and pillars rise to their END; swirls
// spiral out and request pillars on multiples of seven.
static bool Brand(void)
{
    static Uint8 o[0x3000], exe[0x24000];
    Weapon49Program(o);
    Math(exe);
    const Uint16 hit = 2;
    const SotnEquipment parent = {
        .id = 179, .attack = 31, .element = 0x8040, .hit_type = 2, .stun = 5, .invincibility = 7};
    SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                   .executable = {exe, sizeof(exe)},
                                   .properties = Properties,
                                   .context = (void *)&hit,
                                   .x = 100 * 65536 + 7,
                                   .y = 80 * 65536 + 9,
                                   .camera_x = 10,
                                   .camera_y = 4,
                                   .family = 49,
                                   .priority = 148,
                                   .animation = 0x42,
                                   .attack_mode = 0x8015,
                                   .allocated = true,
                                   .parent_properties = parent,
                                   .parent_hitbox_state = 2};
    SotnHandWeaponProgram program = {0};
    // Sparks.
    for (Uint16 index = 0; index < 2; ++index)
    {
        SotnWeaponSecondary s = {.parameter = index};
        SotnWeaponSecondaryEvents e = {0};
        Uint32 rng = 3;
        in.role = 1;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == 3 && e.group &&
              !e.properties && s.properties.attack == 31 && s.properties.id == 179 &&
              s.hitbox_state == 2 && s.hitbox_width == 6 && s.bank == 9 && s.facing_left &&
              s.draw_flags == 4 && s.flags == 0x160000U && s.priority == 150 &&
              s.angle == index * 512 && s.rotate == index * 512 - 0x400 && s.magnitude == 0x40 &&
              s.lifetime == 0x28);
        Uint16 t = 0, ring = 0;
        for (; t < 40 && !s.removed; ++t)
        {
            e = (SotnWeaponSecondaryEvents){0};
            const Sint16 magnitude = s.magnitude;
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && !e.group);
            if (!s.removed)
                CHECK(s.magnitude == (magnitude >= 8 ? magnitude - 3 : magnitude));
            ring += e.chain == 0x1040U;
        }
        // Lifetime0x28 counts down; retirement once it falls below0x11.
        CHECK(s.removed && t == 0x28 - 0x10 && ring == (index ? 0 : 1));
    }
    // Ring: sixteen wedges, the param1 ring lasting forty updates and fading over its last eight.
    for (Uint16 param = 0; param < 2; ++param)
    {
        SotnWeaponSecondary s = {.parameter = (Uint16)(param << 8)};
        SotnWeaponSecondaryEvents e = {0};
        Uint32 rng = 3;
        in.role = 2;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == 3 &&
              s.flags == 0x860000U && s.lifetime == 4 && s.age == (param ? 40 : 0x19) &&
              s.quads[0].page == 0x1a && s.quads[0].palette == 0x15f &&
              s.quads[0].draw_mode == 0x335 && s.quads[3].u[2] == 0x20 && s.quads[3].v[3] == 0xe0 &&
              s.quads[5].red[2] == 0x80 && s.quads[5].green[3] == 0x40);
        Uint16 t = 0;
        for (; t < 60 && !s.removed; ++t)
        {
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
            if (!s.removed && s.age < 8)
                CHECK(s.quads[2].red[2] == s.age * 16 && s.quads[2].green[2] == s.age * 8);
        }
        CHECK(s.removed && t == (param ? 40 : 0x19) && s.lifetime == 40);
    }
    // Puffs: six libc calls before the property roll, the anchor/offset and the rise to END.
    {
        SotnWeaponSecondary s = {.parameter = 0x103};
        SotnWeaponSecondaryEvents e = {0};
        Uint32 rng = 7, check = 7;
        in.role = 3;
        in.hand = 1;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && e.properties &&
              s.bank == 0x8013 && s.texture == 0x67 && s.flags == 0x08100000U &&
              s.draw_flags == 3 && s.hitbox_width == 6 && s.velocity_y == -0x8000);
        Uint16 r[6] = {0};
        for (Uint16 i = 0; i < 6; ++i)
            r[i] = (Uint16)(SotnCombat_Random(&check) & 0x7fffU);
        (void)SotnCombat_Random(&check);
        // Param1 uses two thirds of the anchor extent (64,6) scaled by lower3/8.
        const Sint32 x = 90 + 24 + 42 * 3 / 8 + r[1] % 4, y = 76 - 14 + 4 * 3 / 8 + r[0] % 16;
        CHECK(rng == check && (s.x >> 16) == x + 10 && (s.y >> 16) == y + 4 &&
              s.palette == 0x12f + (r[2] & 7) && s.scale_x == ((r[3] & 0x3f) | 0x80) &&
              s.priority == 148 + (r[4] & 1) * 4 && s.blend == (r[5] % 3 ? 0x70 : 0x30));
        Uint16 t = 0;
        for (; t < 60 && !s.removed; ++t)
        {
            const Sint32 sy = s.y;
            CHECK(SotnWeaponSecondary_Animate(&s, (SotnDataView){o, sizeof(o)}, 49) &&
                  SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
            if (!s.removed)
                CHECK(s.y == sy - 0x8000);
        }
        // Thirteen two-update frames, then END sets the timer negative for the next callback.
        CHECK(s.removed && t == 27);
        in.hand = 0;
    }
    // Swirls: invisible spirals that request a pillar on each multiple of seven.
    {
        SotnWeaponSecondary s = {.parameter = 2};
        SotnWeaponSecondaryEvents e = {0};
        Uint32 rng = 11;
        in.role = 4;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && e.properties &&
              !s.hitbox_width && s.magnitude == 0xc && s.angle == 1024 && s.phase == 1);
        Uint16 t = 0, pillars = e.chain == 0x1065U;
        for (; t < 60 && !s.removed; ++t)
        {
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
            pillars += e.chain == 0x1065U;
            // A pillar request accompanies exactly the multiples of seven of the counter.
            if (!s.removed)
                CHECK((e.chain == 0x1065U) == (s.lifetime % 7 == 0));
            if (s.phase == 2 && !s.removed && s.magnitude > 0x44)
                CHECK(s.flags == 0x08000000U);
        }
        // 0xC rises by four: phase2 at0x40 (13 updates), retirement at0x70 (12 more).
        CHECK(s.removed && t == 25 && pillars >= 3);
    }
    // Pillar: SFX_FM_EXPLODE_B and a 12x12 box, rising until its END.
    {
        SotnWeaponSecondary s = {.x = 5 * 65536, .y = 70 * 65536};
        SotnWeaponSecondaryEvents e = {0};
        Uint32 rng = 13;
        in.role = 5;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && e.sounds[0] == 0x65b &&
              e.properties && s.hitbox_width == 12 && s.palette == 0x117 && s.scale_x == 0x100 &&
              s.y == 70 * 65536);
        CHECK(SotnWeaponSecondary_Animate(&s, (SotnDataView){o, sizeof(o)}, 49) &&
              SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.y == 70 * 65536 - 0x6000);
    }
    // Leaving the brand's animations retires sparks, rings and unborn puffs/swirls.
    for (Uint16 role = 1; role <= 4; ++role)
    {
        SotnWeaponSecondary s = {0};
        SotnWeaponSecondaryEvents e = {0};
        Uint32 rng = 3;
        in.role = role;
        in.animation = 0x48;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.removed && rng == 3);
        in.animation = 0x42;
    }
    CHECK(SotnWeaponSecondary_RoleRows(49, 2) == 16 && !SotnWeaponSecondary_RoleRows(49, 1) &&
          SotnWeaponSecondary_SupportedRole(49, 5) && !SotnWeaponSecondary_SupportedRole(49, 6));
    return true;
}
// Authored weapon50 layout: descriptors at 0xfc0, six UV rows at 0x1000 and blade anchors at
// 0x1030 (24,-14 with extent 64,6*i).
static void Weapon50Program(Uint8 *o)
{
    Program(o);
    for (Uint16 v = 0; v < 4; ++v)
        SDL_memcpy(o + 0xfc0 + v * 16U, o + 0x10ac, 16);
    for (Uint16 r = 0; r < 6; ++r)
        for (Uint16 k = 0; k < 8; ++k)
            o[0x1000 + r * 8U + k] = (Uint8)(r * 16U + k);
    for (Uint16 i = 0; i < 7; ++i)
    {
        Put16(o + 0x1030 + i * 8U, 24);
        Put16(o + 0x1032 + i * 8U, (Uint16)-14);
        Put16(o + 0x1034 + i * 8U, 64);
        Put16(o + 0x1036 + i * 8U, (Uint16)(i * 6U));
    }
}
// w_050: eighty sparkles wait, then drift through six UV rows (two placements each, size six then
// four) to the blade; the ring rolls properties with a radius-sized box; frost streaks flash once
// each; crystals rise from points shared with the sparkles.
static bool Ice(void)
{
    static Uint8 o[0x3000], exe[0x24000];
    Weapon50Program(o);
    Math(exe);
    const Uint16 hit = 2;
    SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                   .executable = {exe, sizeof(exe)},
                                   .properties = Properties,
                                   .context = (void *)&hit,
                                   .x = 100 * 65536 + 7,
                                   .y = 80 * 65536 + 9,
                                   .camera_x = 10,
                                   .camera_y = 4,
                                   .family = 50,
                                   .priority = 148,
                                   .animation = 0x42,
                                   .attack_mode = 0x8016,
                                   .allocated = true};
    SotnHandWeaponProgram program = {0};
    // Sparkles: 80 delays rolled at birth (no properties), retirement after forty updates.
    {
        SotnWeaponSecondary s = {.x = 5 * 65536};
        SotnWeaponSecondaryEvents e = {0};
        Uint32 rng = 3, check = 3;
        in.role = 1;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && !e.properties &&
              s.flags == 0x860000U && s.lifetime == 40 && s.priority == 152 && s.x == 5 * 65536);
        // The birth callback also runs the update: each 1..24 delay has ticked once, and a
        // delay of one has already fired.
        for (Uint16 i = 0; i < 80; ++i)
        {
            const Uint8 delay = (Uint8)((SotnCombat_Random(&check) & 0x7fffU) % 24U + 1U);
            CHECK(delay == 1 ? s.ice_g0[i] == 1 && !s.ice_g1[i]
                             : !s.ice_g0[i] && s.ice_g1[i] == delay - 1U);
        }
        Uint16 shown = 0, t = 1, ended = 0;
        for (; t < 60 && !s.removed; ++t)
        {
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
            for (Uint16 i = 0; i < 80 && !s.removed; ++i)
            {
                if (s.ice_size[i])
                {
                    CHECK((s.ice_size[i] == 6) ==
                          (s.ice_b0[i] < 3 || (s.ice_b0[i] == 3 && (s.ice_b1[i] & 1U) == 0)));
                    ++shown;
                }
                ended += s.ice_g0[i] == 2;
            }
        }
        // Forty updates after birth (the counter starts at forty and retires on zero).
        CHECK(s.removed && t == 41 && shown > 200 && ended > 0);
    }
    // Ring: properties with a box of radius minus four, fading over its last sixteen updates.
    {
        SotnWeaponSecondary s = {0};
        SotnWeaponSecondaryEvents e = {0};
        Uint32 rng = 3;
        in.role = 2;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && e.properties &&
              s.age == 0x28 && s.lifetime == 4 && !s.hitbox_width && s.quads[0].blue[2] == 0x80);
        Uint16 t = 0;
        for (; t < 60 && !s.removed; ++t)
        {
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && !e.properties);
            if (!s.removed)
                CHECK(s.hitbox_width == s.lifetime - 4 &&
                      s.quads[7].blue[3] == (s.age < 16 ? s.age * 16 : 0x80));
        }
        CHECK(s.removed && t == 0x28 && s.lifetime == 0x28);
    }
    // Frost: thirty-two streaks with six rand calls each, then the property roll; each shows for
    // exactly one update; the child retires after eight.
    {
        SotnWeaponSecondary s = {0};
        SotnWeaponSecondaryEvents e = {0};
        Uint32 rng = 5, check = 5;
        in.role = 3;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && e.properties &&
              s.hitbox_x == 0x18 && s.hitbox_width == 0x18 && s.hitbox_height == 8 &&
              s.draw_mode == 0x335 && s.row_priority == 152);
        for (Uint16 i = 0; i < 32 * 6 + 1; ++i)
            (void)SotnCombat_Random(&check);
        CHECK(rng == check);
        Uint16 flashes[32] = {0};
        for (Uint16 t = 0; t < 12 && !s.removed; ++t)
        {
            for (Uint16 i = 0; i < 32; ++i)
                flashes[i] += !s.rows[i].hidden;
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
        }
        for (Uint16 i = 0; i < 32; ++i)
            CHECK(flashes[i] <= 1 && s.rows[i].red[1] == 0x20 && s.rows[i].blue[0] >= 0xe0);
        CHECK(s.removed);
    }
    // Crystals: eight points written into the program's shared sparkle points, each rising a
    // pixel per placement after a 2i+1 delay.
    for (Uint16 param = 0; param < 2; ++param)
    {
        SotnWeaponSecondary s = {.parameter = (Uint16)(param << 8)};
        SotnWeaponSecondaryEvents e = {0};
        Uint32 rng = 9;
        in.role = 4;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && !e.properties &&
              s.quads[0].draw_mode == (param ? 0x339 : 0x379) && s.quads[3].green[1] == 6 &&
              s.lifetime == 0x18);
        const Sint16 y0 = program.ice_y[0];
        Uint16 t = 0;
        for (; t < 40 && !s.removed; ++t)
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
        // Crystal0 placed twelve times (six rows, two each) before row6 ends it.
        CHECK(s.removed && t == 0x18 && program.ice_y[0] == y0 - 13);
    }
    // Leaving the brand's animations retires sparkles, rings and crystals before any RNG.
    for (Uint16 role = 1; role <= 4; role += role == 2 ? 2 : 1)
    {
        SotnWeaponSecondary s = {0};
        SotnWeaponSecondaryEvents e = {0};
        Uint32 rng = 3;
        in.role = role;
        in.animation = 0x48;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.removed && rng == 3);
        in.animation = 0x42;
    }
    CHECK(SotnWeaponSecondary_RoleRows(50, 1) == 80 && SotnWeaponSecondary_RoleRows(50, 3) == 32 &&
          SotnWeaponSecondary_RoleRows(50, 4) == 8 && !SotnWeaponSecondary_SupportedRole(50, 5));
    return true;
}
// Authored weapon6 layout: crescent launch speeds at 0xbf0, the fourteen-step palette cycle at
// 0xbd4 and six sparkle UV rows at 0xc04.
static void Weapon6Program(Uint8 *o)
{
    Program(o);
    const Sint32 launch[5] = {0x14000, -0x1c000, 0x8000, -0x10000, -0x4000};
    for (Uint16 i = 0; i < 5; ++i)
        Put32(o + 0xbf0 + i * 4U, (Uint32)launch[i]);
    for (Uint16 i = 0; i < 14; ++i)
        Put16(o + 0xbd4 + i * 2U, (Uint16)(i < 8 ? i + 1U : 14U - i));
    for (Uint16 r = 0; r < 6; ++r)
        for (Uint16 k = 0; k < 8; ++k)
            o[0xc04 + r * 8U + k] = (Uint8)(r * 16U + k);
}
// w_006: crescents leave the player's hitbox, slow to a stop while growing, spin up, then fly
// shedding sparkles every other update; a hit flashes them white and retires them after eight.
static bool Moon(void)
{
    static Uint8 o[0x3000];
    Weapon6Program(o);
    const Uint16 hit = 2;
    SotnHandWeaponProgram program = {0};
    for (Uint16 k = 0; k < 4; ++k)
    {
        const Uint16 index = k & 1U ? 3 : 0, hand = k >> 1;
        SotnWeaponSecondary s = {.parameter = (Uint16)(index | hand << 15),
                                 .x = 100 * 65536,
                                 .y = 80 * 65536,
                                 .facing_left = (k & 1U) != 0};
        SotnWeaponSecondaryEvents e = {0};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .properties = Properties,
                                       .context = (void *)&hit,
                                       .hand = hand,
                                       .family = 6,
                                       .role = 1,
                                       .priority = 148,
                                       .facing_left = s.facing_left,
                                       .parent_hitbox_x = 5,
                                       .parent_hitbox_y = -20};
        Uint32 rng = 7, check = 7;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 1 &&
              e.properties && e.clear_shield && e.use_ticks == 4 &&
              e.sounds[0] == (index ? 0 : 0x6ee) && s.bank == 0x8010U + hand * 2U &&
              s.frame == 0x42 && s.draw_flags == 7 && s.flags == 0x08020000U && s.spin == 0x40);
        const Sint32 speed = (Sint32)(SotnCombat_Random(&check) & 0x7fffU) - 0x40000;
        const Sint32 vy =
            (Sint32)(SotnCombat_Random(&check) & 0x1fffU) + (index ? -0x10000 : 0x14000);
        CHECK(s.x == 100 * 65536 + (s.facing_left ? -5 : 5) * 65536 && s.y == 60 * 65536 &&
              s.velocity_x == (s.facing_left ? -speed : speed) && s.velocity_y == vy &&
              s.accel_y == -(vy >> 4) && s.accel_x == (s.facing_left ? -0x3c00 : 0x3c00) &&
              s.counter == 1 && s.palette == (hand ? 0x12e : 0x116) + 1);
        // Phase1 decelerates by0x2800 until stopped, then twenty-four spin-up updates.
        Uint16 t = 0, chains = 0, swishes = 0;
        for (; t < 400 && s.phase < 3; ++t)
        {
            e = (SotnWeaponSecondaryEvents){0};
            const Sint32 vx = s.velocity_x;
            const Uint8 before = s.phase;
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && !e.chain);
            if (before == 1)
                CHECK(s.velocity_x == (vx < 0 ? SDL_min(vx + 0x2800, 0) : SDL_max(vx - 0x2800, 0)));
            swishes += e.sounds[0] == 0x60b || e.sounds[1] == 0x60b;
        }
        CHECK(s.phase == 3 && !s.velocity_x && s.spin == 0x40 + 24 * 8);
        for (Uint16 i = 0; i < 6; ++i)
        {
            e = (SotnWeaponSecondaryEvents){0};
            const Sint16 counter = s.counter;
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
            chains += e.chain != 0;
            CHECK((e.chain != 0) == !(counter & 1));
        }
        CHECK(chains == 3 && (index ? !swishes : swishes > 0));
        // A hit flashes white, stops attacking motion and retires eight updates later.
        s.hit_flags = 1;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 4 &&
              s.palette == 0x815f && s.blend == 0x10 && s.lifetime == 8);
        for (t = 0; t < 20 && !s.removed; ++t)
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
        CHECK(s.removed && t == 8);
    }
    // Sparkles: two rand offsets, then half-pixel falls through six two-placement rows.
    {
        SotnWeaponSecondary s = {.x = 100 * 65536, .y = 80 * 65536, .priority = 150};
        SotnWeaponSecondaryEvents e = {0};
        Uint32 rng = 9, check = 9;
        SotnWeaponSecondaryInput in = {
            .overlay = {o, sizeof(o)}, .family = 6, .role = 2, .allocated = true};
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 1 &&
              s.flags == 0x08800000U && s.quads[0].priority == 154 &&
              s.quads[0].draw_mode == 0x31 && s.quads[0].palette == 0x1b0);
        const Sint32 dx = (Sint32)((SotnCombat_Random(&check) & 0x7fffU) % 24U) - 12,
                     dy = (Sint32)((SotnCombat_Random(&check) & 0x7fffU) % 24U) - 12;
        CHECK(rng == check && (s.x >> 16) == 100 + dx && (s.y >> 16) == 80 + dy &&
              s.quads[0].x[0] == 100 + dx - 6 && s.quads[0].u[0] == 0);
        Uint16 t = 0;
        for (; t < 20 && !s.removed; ++t)
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
        // Twelve placements (birth plus eleven); the twelfth update reaches row6.
        CHECK(s.removed && t == 12);
        SotnWeaponSecondary unallocated = {0};
        in.allocated = false;
        rng = 3;
        CHECK(SotnWeaponSecondary_Tick(&unallocated, &program, in, &rng, &e) &&
              unallocated.removed && rng == 3);
    }
    CHECK(SotnWeaponSecondary_SupportedRole(6, 1) && SotnWeaponSecondary_RoleRows(6, 2) == 1 &&
          !SotnWeaponSecondary_SupportedRole(6, 3));
    return true;
}
// w_014: the thrown Runesword flies out levelling its blade, stops, drifts back and homes on the
// player's hitbox; its trail uncovers one row per update and keeps the newest seventeen.
static bool Rune(void)
{
    static Uint8 o[0x3000], exe[0x24000];
    Math(exe);
    const Uint16 hit = 2;
    SotnHandWeaponProgram program = {0};
    for (Uint16 k = 0; k < 4; ++k)
    {
        const Uint16 hand = k >> 1;
        const bool left = (k & 1U) != 0, crouch = k == 3;
        SotnWeaponSecondary s = {.parameter = (Uint16)(hand << 15),
                                 .x = 100 * 65536 + 0x1234,
                                 .y = 80 * 65536 + 0x4321,
                                 .facing_left = !left};
        SotnWeaponSecondaryEvents e = {0};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = {exe, sizeof(exe)},
                                       .properties = Properties,
                                       .context = (void *)&hit,
                                       .x = 120 * 65536,
                                       .y = 100 * 65536,
                                       .camera_x = 30,
                                       .camera_y = -10,
                                       .hand = hand,
                                       .family = 14,
                                       .role = 0,
                                       .priority = 148,
                                       .facing_left = left,
                                       .allocated = true,
                                       .parent_hitbox_x = 5,
                                       .parent_hitbox_y = -6,
                                       .player_velocity_x = 0x8000,
                                       .player_hitbox_width = 12,
                                       .player_hitbox_height = 30,
                                       .player_crouch_step = crouch};
        Uint32 rng = 7, check = 7;
        (void)SotnCombat_Random(&check);
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 1 &&
              rng == check && e.properties && e.clear_shield && e.use_ticks == 4 &&
              e.sounds[0] == 0x677 && e.volume_sound && e.volume == 0x50 && e.sounds[1] == 0x66c &&
              e.chain == (0x72U | (hand + 1U) << 12) && s.bank == 0x8010U + hand * 2U &&
              s.texture == 0x64U + hand * 2U && s.palette == (hand ? 0x134 : 0x11c) &&
              s.frame == 8 && s.facing_left == left && s.priority == 146 && s.row_priority == 144 &&
              s.draw_mode == 0x173 && s.flags == 0x0c820000U && s.draw_flags == 4 &&
              s.rotate == -0x700 && s.hitbox_width == 4 && s.hitbox_height == 4 &&
              s.lifetime == 0x16 && s.velocity_x == (left ? -0x60000 : 0x60000) + 0x8000 &&
              s.rune_velocity_y == 0x10000 && s.rune_count == 1);
        // Origin: the player's hitbox, sixteen (crouching eight) pixels up; fractions kept.
        CHECK(s.x == (120 + (left ? -5 : 5)) * 65536 + 0x1234 &&
              s.y == (100 - 6 - (crouch ? 8 : 16)) * 65536 + 0x4321);
        for (Uint16 i = 0; i < 48; ++i)
            CHECK(s.rune_hidden[i] && !s.rune_shown[i]);
        // Flight: the blade turns 0x70 per update to level, stops below lifetime7.
        for (Uint16 t = 1; t <= 0x16; ++t)
        {
            const Sint32 x = s.x, y = s.y, vx = s.velocity_x;
            const Sint16 rotate = s.rotate;
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && !e.properties &&
                  !e.sounds[0] && !e.chain && rng == check);
            CHECK(s.x == x + vx && s.y == y + 0x10000 && s.rotate == SDL_min(rotate + 0x70, 0) &&
                  s.rune_count == t + 1);
            CHECK(s.rune_shown[t - 1U] && !s.rune_hidden[t - 1U] &&
                  (t == 0x16 || !s.rune_shown[t]));
            if (t < 0x16)
                CHECK(s.velocity_x == (0x16 - t < 7 ? 0 : vx));
        }
        CHECK(s.phase == 2 && s.lifetime == 8 && s.velocity_x == (left ? 0x30000 : -0x30000));
        // The newest seventeen rows stay visible; the texture spreads over sixteen of them.
        for (Uint16 i = 0; i < 0x16; ++i)
            CHECK(s.rune_hidden[i] == (i < 0x16 - 17));
        CHECK(s.rune_u0[0x15] == 120 && s.rune_u1[0x15] == 128 && s.rune_u0[6] == 0 &&
              s.rune_u1[6] == 8);
        // Row0 x0/x2 hold the birth corners; later rows chain the previous row's x1/x3.
        CHECK(s.rune_x1[0] != s.rune_x0 || s.rune_y1[0] != s.rune_y0);
        for (Uint16 t = 0; t < 8; ++t)
        {
            const Sint32 x = s.x, y = s.y;
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.x == x + s.velocity_x &&
                  s.y == y && !(s.draw_flags & 4U));
        }
        CHECK(s.phase == 3);
        // Homing: ten pixels per update toward the hitbox; retires inside its window.
        Uint16 t = 0;
        Sint32 distance = 1 << 30;
        for (; t < 200 && !s.removed; ++t)
        {
            const Sint16 count = s.rune_count;
            const Sint32 dx = 120 + 5 - (Sint32)(s.x >> 16), dy = 100 - 6 - (Sint32)(s.y >> 16);
            const bool inside = SDL_abs(dx) < (12 + 16) / 2 && SDL_abs(dy) < (30 + 4) / 2;
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.removed == inside);
            if (s.removed)
            {
                CHECK(SDL_abs(dx) < (12 + 16) / 2 && SDL_abs(dy) < (30 + 4) / 2 &&
                      s.rune_count == count);
                break;
            }
            CHECK((s.draw_flags & 4U) && dx * dx + dy * dy <= distance);
            distance = dx * dx + dy * dy;
        }
        CHECK(s.removed);
    }
    // Exhausted rows retire the root before properties or RNG.
    {
        SotnWeaponSecondary s = {.x = 100 * 65536, .y = 80 * 65536};
        SotnWeaponSecondaryEvents e = {0};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = {exe, sizeof(exe)},
                                       .properties = Properties,
                                       .context = (void *)&hit,
                                       .family = 14,
                                       .role = 0};
        Uint32 rng = 3;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.removed && rng == 3 &&
              !e.properties && !e.sounds[0] && !e.chain);
        SotnWeaponSecondary bad = {.phase = 4};
        CHECK(!SotnWeaponSecondary_Tick(&bad, &program, in, &rng, &e));
    }
    // Hitboxes: four blade offsets from the raw parent, retired once its slot is empty.
    for (Uint16 index = 0; index < 4; ++index)
    {
        static const Sint16 reach[4] = {10, 20, 30, 40};
        SotnWeaponSecondary s = {.parameter = index, .x = 1 * 65536 + 0x77, .y = 2 * 65536};
        SotnWeaponSecondaryEvents e = {0};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = {exe, sizeof(exe)},
                                       .properties = Properties,
                                       .context = (void *)&hit,
                                       .family = 14,
                                       .role = 1,
                                       .parent_exists = true,
                                       .parent_facing_left = index == 2,
                                       .parent_x = 90 * 65536,
                                       .parent_y = 70 * 65536,
                                       .parent_rotate = 0x400};
        Uint32 rng = 5;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && e.properties &&
              s.phase == 1 && s.flags == 0x0c020000U && s.facing_left == (index == 2) &&
              s.hitbox_width == 4 && s.hitbox_height == 4);
        CHECK(s.x == (90 + (index == 2 ? -reach[index] : reach[index])) * 65536 + 0x77 &&
              s.y == 70 * 65536);
        // The parent's rotation counts only while it draws rotated.
        in.parent_draw_flags = 4;
        in.parent_facing_left = !in.parent_facing_left;
        e = (SotnWeaponSecondaryEvents){0};
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && !e.properties &&
              s.x == 90 * 65536 + 0x77 && s.y == (70 + reach[index]) * 65536 &&
              s.facing_left == (index == 2));
        in.parent_exists = false;
        const Uint32 before = rng;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.removed && rng == before);
    }
    SotnWeaponSecondary forged = {.parameter = 4};
    SotnWeaponSecondaryEvents e = {0};
    Uint32 rng = 1;
    CHECK(!SotnWeaponSecondary_Tick(&forged, &program,
                                    (SotnWeaponSecondaryInput){.overlay = {o, sizeof(o)},
                                                               .executable = {exe, sizeof(exe)},
                                                               .family = 14,
                                                               .role = 1,
                                                               .parent_exists = true},
                                    &rng, &e));
    CHECK(SotnWeaponSecondary_SupportedRole(14, 0) && SotnWeaponSecondary_SupportedRole(14, 1) &&
          !SotnWeaponSecondary_SupportedRole(14, 2) && SotnWeaponSecondary_RoleRows(14, 0) == 48 &&
          !SotnWeaponSecondary_RoleRows(14, 1) && !SotnWeaponSecondary_SupportedRole(6, 0));
    return true;
}

// w_013: Heaven sword flies out and stops, rises on a spinning parabola, then steers toward the
// player with a growing turn limit; every other g_GameTimer it sheds a fading afterimage.
static bool Heaven(void)
{
    static Uint8 o[0x3000], exe[0x24000];
    Math(exe);
    for (Uint16 i = 0; i < 14; ++i)
        Put16(o + 0x5e0 + i * 2U, (Uint16)(i < 8 ? i + 1U : 14U - i));
    const Uint16 hit = 2;
    SotnHandWeaponProgram program = {0};
    for (Uint16 k = 0; k < 4; ++k)
    {
        const Uint16 hand = k >> 1;
        const bool left = (k & 1U) != 0, crouch = k == 3;
        SotnWeaponSecondary s = {
            .parameter = (Uint16)(hand << 15), .x = 100 * 65536 + 0x1234, .y = 80 * 65536 + 0x4321};
        SotnWeaponSecondaryEvents e = {0};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = {exe, sizeof(exe)},
                                       .properties = Properties,
                                       .context = (void *)&hit,
                                       .x = 120 * 65536,
                                       .y = 100 * 65536,
                                       .hand = hand,
                                       .family = 13,
                                       .role = 0,
                                       .priority = 148,
                                       .facing_left = left,
                                       .parent_hitbox_x = 5,
                                       .parent_hitbox_y = -6,
                                       .player_velocity_x = 0x8000,
                                       .player_hitbox_width = 12,
                                       .player_hitbox_height = 30,
                                       .player_crouch_step = crouch,
                                       .game_timer = 1};
        Uint32 rng = 7, check = 7;
        (void)SotnCombat_Random(&check);
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 1 &&
              rng == check && e.properties && e.clear_shield && e.use_ticks == 4 &&
              e.sounds[0] == 0x60c && !e.volume_sound && !e.chain &&
              s.bank == 0x8010U + hand * 2U && s.texture == 0x64U + hand * 2U &&
              s.palette == (hand ? 0x128 : 0x110) && s.frame == 6 && s.priority == 146 &&
              s.flags == 0x0c020000U && !s.draw_flags && s.hitbox_width == 0x12 &&
              s.hitbox_height == 4 && s.lifetime == 0x16 && s.volume == 0x50 &&
              s.velocity_x == (left ? -0x60000 : 0x60000) + 0x8000);
        CHECK(s.x == (120 + (left ? -5 : 5)) * 65536 + 0x1234 &&
              s.y == (100 - 6 - (crouch ? 8 : 16)) * 65536 + 0x4321);
        // Flight: DecelerateX(1) below lifetime7, then the backward rise.
        for (Uint16 t = 1; t <= 0x16; ++t)
        {
            const Sint32 x = s.x, vx = s.velocity_x;
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && !e.sounds[0] &&
                  !e.chain && s.x == x + vx);
            if (t < 0x16 && 0x16 - t < 7)
                CHECK(s.velocity_x ==
                      (vx < 0 ? SDL_min(vx + 0x10000, 0) : SDL_max(vx - 0x10000, 0)));
        }
        CHECK(s.phase == 2 && s.lifetime == 0x18 && s.velocity_y == -0x18000 &&
              s.velocity_x == (left ? 0x30000 : -0x30000));
        // Rise: 12x12 box, spinning, gravity1/16; even timers chain afterimages, every eighth a
        // swish fading by four.
        Uint16 images = 0, swishes = 0;
        for (Uint16 t = 0; t < 0x18; ++t)
        {
            in.game_timer = 8U + t;
            const Sint32 vy = s.velocity_y;
            const Sint16 rotate = s.rotate, counter = s.counter;
            const Sint32 volume = s.volume;
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.hitbox_width == 12 &&
                  (s.draw_flags & 4U) && s.rotate == (Sint16)(rotate - 0x80) &&
                  (t == 0x17 || s.velocity_y == vy + 0x1000) && s.counter == counter + 1);
            CHECK((e.chain != 0) == !(in.game_timer & 1U) &&
                  (!e.chain || e.chain == (0x3aU | (hand + 1U) << 12)));
            CHECK(s.child_palette == s.palette + (Sint16)((counter >> 1) % 14 < 8
                                                              ? (counter >> 1) % 14 + 1
                                                              : 14 - (counter >> 1) % 14));
            if (!(in.game_timer & 7U))
                CHECK(e.sounds[0] == 0x60a && e.volume_sound && e.volume == volume &&
                      s.volume == SDL_max(volume - 4, 0));
            images += e.chain != 0;
            swishes += e.sounds[0] == 0x60a;
        }
        CHECK(s.phase == 3 && s.magnitude == 0x80 && images == 12 && swishes == 3);
        // Homing: the turn limit grows by 0x20 but never overshoots a smaller heading change;
        // it retires inside the shared hitbox window.
        Uint16 t = 0;
        for (; t < 400 && !s.removed; ++t)
        {
            in.game_timer = 40U + t;
            const Sint16 limit = s.magnitude, angle = s.angle;
            const Sint32 dx = 120 + 5 - (Sint32)(s.x >> 16), dy = 100 - 6 - (Sint32)(s.y >> 16);
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
            if (s.removed)
            {
                CHECK(SDL_abs(dx) < (12 + 12) / 2 && SDL_abs(dy) < (30 + 12) / 2);
                break;
            }
            Sint32 turn = SDL_abs(s.angle - angle);
            turn = SDL_min(turn, 0x1000 - turn);
            CHECK(s.magnitude == limit + 0x20 && turn <= limit && s.angle >= 0 && s.angle < 0x1000);
        }
        CHECK(s.removed);
    }
    // A heading change smaller than the turn limit snaps exactly onto the target.
    {
        SotnWeaponSecondary s = {.x = 60 * 65536,
                                 .y = 50 * 65536,
                                 .phase = 3,
                                 .magnitude = 0x100,
                                 .hitbox_width = 12,
                                 .hitbox_height = 12,
                                 .palette = 0x110};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = {exe, sizeof(exe)},
                                       .x = 120 * 65536,
                                       .y = 90 * 65536,
                                       .family = 13,
                                       .role = 0,
                                       .player_hitbox_width = 12,
                                       .player_hitbox_height = 30,
                                       .game_timer = 1};
        Sint16 target = 0;
        CHECK(SotnMath_Atan2((SotnDataView){exe, sizeof(exe)}, -(90 - 50), 120 - 60, &target));
        target = (Sint16)(target & 0xfff);
        s.angle = (Sint16)((target + 0x10) & 0xfff);
        SotnWeaponSecondaryEvents e = {0};
        Uint32 rng = 1;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.angle == target &&
              s.magnitude == 0x120);
    }
    // Afterimages copy the raw parent and fade by eight from 0x80 until below48.
    {
        SotnWeaponSecondary s = {.x = 3 * 65536, .y = 4 * 65536};
        SotnWeaponSecondaryEvents e = {0};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = {exe, sizeof(exe)},
                                       .family = 13,
                                       .role = 1,
                                       .priority = 148,
                                       .parent_exists = true,
                                       .parent_facing_left = true,
                                       .parent_rotate = -0x300,
                                       .parent_draw_flags = 4,
                                       .parent_frame = 6,
                                       .parent_bank = 0x8012,
                                       .parent_texture = 0x66,
                                       .parent_child_palette = 0x12c};
        Uint32 rng = 9;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == 9 && !e.properties &&
              !e.chain && s.phase == 1 && s.frame == 6 && s.bank == 0x8012 && s.texture == 0x66 &&
              s.facing_left && s.priority == 144 && s.palette == 0x12c && s.draw_flags == 12 &&
              s.rotate == -0x300 && s.opacity == 0x78 && s.flags == 0x0c020000U &&
              s.x == 3 * 65536 && !s.hitbox_width);
        in.parent_exists = false;
        Uint16 t = 1;
        for (; t < 20 && !s.removed; ++t)
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
        CHECK(s.removed && t == 12 && s.opacity == 40);
        SotnWeaponSecondary orphan = {0};
        CHECK(SotnWeaponSecondary_Tick(&orphan, &program, in, &rng, &e) && orphan.removed &&
              !orphan.phase);
    }
    CHECK(SotnWeaponSecondary_SupportedRole(13, 0) && SotnWeaponSecondary_SupportedRole(13, 1) &&
          !SotnWeaponSecondary_RoleRows(13, 0) && !SotnWeaponSecondary_RoleRows(13, 1));
    return true;
}

// w_056 shares w_053's slash callback over its own tables eight bytes later (0xFCC rows,
// 0xFA4/0xFB8 CLUT schedules); row10 overlaps its live g_HandId word at0x105C.
static bool MuramasaSlash(void)
{
    static Uint8 o[0x1400], exe[0x24000];
    Math(exe);
    for (Uint16 i = 0; i < 11; ++i)
    {
        Uint8 *c = o + 0xfcc + i * 14U;
        Put16(c + 4, (Uint16)(i & 1U));
        Put16(c + 6, (Uint16)(40 + i));
        Put16(c + 8, (Uint16)(i * 100U));
        c[10] = (Uint8)(i + 7U);
        c[11] = 64;
        c[12] = 32;
    }
    for (Uint16 i = 0; i < 9; ++i)
    {
        Put16(o + 0xfa4 + i * 2U, (Uint16)(0x150 + i));
        Put16(o + 0xfb8 + i * 2U, (Uint16)(0x160 + i));
    }
    // Weapon53's schedule starts eight bytes earlier: Muramasa must not read those words.
    SDL_memset(o + 0xf9c, 0xee, 8);
    Put32(o + 0x105c, 0x12345678U); // g_HandId's static image, replaced by the live hand
    const Uint16 hit = 2;
    SotnHandWeaponProgram program = {0};
    for (Uint16 hand = 0; hand < 2; ++hand)
        for (Uint16 param = 0; param <= 10; param += 10)
        {
            SotnWeaponSecondary s = {
                .parameter = (Uint16)(param << 8), .x = 100 * 65536, .y = 90 * 65536};
            SotnWeaponSecondaryEvents e = {0};
            Uint32 rng = 5;
            const SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                                 .executable = {exe, sizeof(exe)},
                                                 .properties = Properties,
                                                 .context = (void *)&hit,
                                                 .x = 120 * 65536,
                                                 .y = 100 * 65536,
                                                 .hand = hand,
                                                 .family = 56,
                                                 .role = 1,
                                                 .priority = 148,
                                                 .allocated = true};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 1 &&
                  e.properties && s.hitbox_width == 30 &&
                  e.chain == (0x38U | (hand + 1U) << 12 | (Uint32)(param + 1U) << 16));
            // Row10's bytes4..7 (CLUT flag, lifetime) are the live hand word: the right hand
            // selects the second schedule and both read lifetime0.
            CHECK(param ? s.lifetime == 0 && s.rotate == 1000 &&
                              s.quad.palette == (hand ? 0x18 + 0x160 : 0x150)
                        : s.lifetime == 40 && s.rotate == 0 &&
                              s.quad.palette == (hand ? 0x18 : 0) + 0x150 && s.quad.red[0] == 7 &&
                              s.quad.green[0] == 64);
        }
    return true;
}

static Sint32 Decel(Sint32 v, Sint32 amount)
{
    if (v < 0)
        return v + amount > 0 ? 0 : v + amount;
    return v - amount < 0 ? 0 : v - amount;
}

// w_012 slashes: the program's count picks one of three shapes; Vorpal (descriptor1) slides
// from four updates back and decelerates, Crissaegrim (descriptor2) shades its GT4 and widens;
// the CLUT steps through ten fade palettes. The Chakram arcs out, then steers home.
static bool Twelve(void)
{
    static Uint8 o[0x3000], exe[0x24000];
    Math(exe);
    const Uint16 hit = 2;
    for (Uint16 k = 0; k < 7; ++k)
    {
        const Uint16 variant = k < 3 ? 1 : 2, hand = k & 1U;
        SotnHandWeaponProgram program = {.secondary_counter = (Sint32)k};
        SotnWeaponSecondary s = {.parameter = (Uint16)(variant << 8 | hand << 15)};
        SotnWeaponSecondaryEvents e = {0};
        // Crouching drops the slash only outside crouch substep2 (k6 crouches in substep2).
        const bool left = k == 2, crouch = k == 4, crouched = k == 4 || k == 6;
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = {exe, sizeof(exe)},
                                       .properties = Properties,
                                       .context = (void *)&hit,
                                       .x = 100 * 65536 + 0x55,
                                       .y = 80 * 65536,
                                       .hand = hand,
                                       .family = 12,
                                       .role = 0,
                                       .priority = 148,
                                       .facing_left = left,
                                       .allocated = true,
                                       .player_crouch_step = crouched,
                                       .player_substep = (Uint16)(k == 6 ? 2 : 1)};
        Uint32 rng = 3;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 1 &&
              e.properties && e.clear_shield && e.startup_ticks == 4 && e.sounds[0] == 0x60b &&
              program.secondary_counter == (Sint32)k + 1 && s.slash_shape == (Sint16)(k % 3U) &&
              s.slash_drop == (crouch ? 0x18 : 0));
        CHECK(s.velocity_x == (left ? -0x40000 : 0x40000) &&
              s.x == 100 * 65536 + 0x55 - 4 * s.velocity_x && s.flags == 0x08800000U &&
              s.hitbox_x == 0x2b && s.hitbox_width == 0x1e &&
              s.hitbox_y == (crouch ? 0x18 - 6 : -6) &&
              s.hitbox_height == (crouch ? 0x1c - 12 : 0x1c) &&
              s.quad.palette == (hand ? 0x128 : 0x110) && s.quad.priority == 150 &&
              s.quad.draw_mode == (variant == 2 ? 0x37 : 0x33) &&
              s.quad.red[0] == (variant == 2 ? 0x80 : 0) && s.quad.v[2] == (hand ? 0xc0 : 0x40));
        // Shape0 pulls x0 in by16, shape1 x2 and flips y, shape2 spreads both edges by16.
        const Sint16 sign = left ? -1 : 1, shape = s.slash_shape;
        CHECK(s.slash_x[1] == sign * 0x4b &&
              s.slash_x[0] ==
                  sign * (shape == 0 ? 0xb - 0x10 : 0xb) + (variant == 2 ? (left ? -7 : 7) : 0) &&
              s.slash_y[0] == (shape == 1 ? 0x27 : -0x27));
        Uint16 t = 1;
        for (; t < 20 && !s.removed; ++t)
        {
            const Sint32 vx = s.velocity_x, x = s.x;
            const Uint16 palette = s.quad.palette;
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
            const Sint32 slowed = Decel(vx, variant == 1 ? 0x3000 : 0x1000);
            CHECK(s.velocity_x == slowed && s.x == x + slowed);
            if (!s.removed)
                CHECK(s.quad.palette == palette + 1U);
        }
        CHECK(s.removed && t == 11);
    }
    // Exhausted rows retire before properties; the Chakram's two launch rolls precede properties.
    {
        SotnHandWeaponProgram program = {0};
        SotnWeaponSecondary s = {0};
        SotnWeaponSecondaryEvents e = {0};
        Uint32 rng = 3;
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = {exe, sizeof(exe)},
                                       .properties = Properties,
                                       .context = (void *)&hit,
                                       .family = 12,
                                       .role = 0};
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.removed && rng == 3 &&
              !program.secondary_counter);
    }
    for (Uint16 k = 0; k < 2; ++k)
    {
        SotnHandWeaponProgram program = {0};
        SotnWeaponSecondary s = {.x = 10 * 65536, .y = 10 * 65536};
        SotnWeaponSecondaryEvents e = {0};
        Uint32 rng = 11U + k * 1000U, check = rng;
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = {exe, sizeof(exe)},
                                       .properties = Properties,
                                       .context = (void *)&hit,
                                       .x = 120 * 65536,
                                       .y = 100 * 65536,
                                       .hand = k,
                                       .family = 12,
                                       .role = 1,
                                       .priority = 148,
                                       .facing_left = k != 0,
                                       .parent_hitbox_x = 5,
                                       .parent_hitbox_y = -6,
                                       .player_hitbox_width = 12,
                                       .player_hitbox_height = 30,
                                       .game_timer = 4};
        const Sint32 speed = (Sint32)(((SotnCombat_Random(&check) & 0x3fffU) * 2U) | 0x48000U);
        const Sint32 vy = (Sint32)(SotnCombat_Random(&check) & 0x7fffU) * 2 + 0x20000;
        (void)SotnCombat_Random(&check);
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == check &&
              s.phase == 1 && s.frame == 3 && s.palette == 0x100 && s.flags == 0x0c020000U &&
              s.velocity_x == (k ? -speed : speed) && s.velocity_y == vy &&
              s.accel_x == (k ? 0x2800 : -0x2800) && s.accel_y == -0x2000 && s.lifetime == 0x1c &&
              s.volume == 0x60 && e.use_ticks == 4 && e.clear_shield &&
              e.chain == (0x40U | (k + 1U) << 12) && s.counter == 1);
        Uint16 t = 0, chains = 0;
        for (; t < 0x1c; ++t)
        {
            in.game_timer = 5U + t;
            e = (SotnWeaponSecondaryEvents){0};
            const Sint32 vx = s.velocity_x, vy0 = s.velocity_y;
            const Sint16 ay = (Sint16)s.accel_y;
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) &&
                  s.velocity_x == vx + (k ? 0x2800 : -0x2800) && s.velocity_y == vy0 + ay &&
                  (Sint16)s.accel_y == ay - 0x1c0);
            chains += e.chain != 0;
            CHECK(!(e.sounds[0] == 0x60a) == !!((s.counter - 1) & 7));
        }
        CHECK(s.phase == 2 && s.frame == 5 && chains == 7);
        // Homing grows the turn limit by0x10; every eighth update swishes and fades by0xC.
        Uint16 fades = 0;
        for (t = 0; t < 400 && !s.removed; ++t)
        {
            const Sint16 limit = s.magnitude;
            const Sint32 volume = s.volume;
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
            if (s.removed)
                break;
            CHECK(!s.facing_left && (s.draw_flags & 4U) && s.magnitude == limit + 0x10);
            if (e.sounds[0] == 0x60a)
            {
                CHECK(e.volume == volume && s.volume == SDL_max(volume - 0xc, 0));
                ++fades;
            }
        }
        CHECK(s.removed && fades);
    }
    CHECK(SotnWeaponSecondary_SupportedRole(12, 2) && !SotnWeaponSecondary_SupportedRole(12, 3) &&
          SotnWeaponSecondary_RoleRows(12, 0) == 1 && !SotnWeaponSecondary_RoleRows(12, 1));
    return true;
}

// w_034 func_ptr_80170004: Shotel's176 boomerang launches at4.5 plus the player's speed from
// its hitbox (16 up, 8 when crouched), spins ROT(45) per update, decelerates for0x1C updates,
// then homes with a turn limit growing0x10 from0x200 and retires at the player's hitbox.
static bool ShotelBoomerang(void)
{
    static Uint8 o[0x1400], exe[0x24000];
    Math(exe);
    const Uint16 hit = 2;
    for (Uint16 k = 0; k < 2; ++k)
    {
        SotnHandWeaponProgram program = {0};
        SotnWeaponSecondary s = {.x = 10 * 65536 + 0x1234, .y = 10 * 65536 + 0x4321};
        SotnWeaponSecondaryEvents e = {0};
        Uint32 rng = 21U + k, check = rng;
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = {exe, sizeof(exe)},
                                       .properties = Properties,
                                       .context = (void *)&hit,
                                       .x = 120 * 65536,
                                       .y = 100 * 65536,
                                       .hand = k,
                                       .family = 34,
                                       .role = 1,
                                       .priority = 148,
                                       .facing_left = k != 0,
                                       .parent_hitbox_x = 5,
                                       .parent_hitbox_y = -6,
                                       .player_hitbox_width = 12,
                                       .player_hitbox_height = 30,
                                       .player_velocity_x = 0x18000,
                                       .player_crouch_step = k != 0,
                                       .game_timer = 8};
        (void)SotnCombat_Random(&check);
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == check &&
              s.phase == 1 && s.frame == 0x3a && s.palette == 0x110U + k * 0x18U &&
              s.bank == 0x8010U + k * 2U && s.texture == 0x64U + k * 2U && s.blend == 0x10 &&
              s.priority == 146 && s.flags == 0x0c020000U &&
              s.velocity_x == (k ? -0x48000 : 0x48000) + 0x18000 && s.velocity_y == -0x4000 &&
              s.accel_x == (k ? 0x2000 : -0x2000) && s.accel_y == 0x400 && s.lifetime == 0x1c &&
              s.magnitude == 0x200 && !s.rotate && (s.draw_flags & 4U) && s.hitbox_width == 9 &&
              s.hitbox_height == 9 && e.use_ticks == 4 && e.clear_shield && e.properties &&
              e.sounds[0] == 0x60b && !e.chain);
        CHECK((s.x >> 16) == (k ? 115 : 125) && (s.x & 0xffff) == 0x1234 &&
              (s.y >> 16) == 100 - 6 - (k ? 8 : 16) && (s.y & 0xffff) == 0x4321);
        Uint16 t = 0;
        for (; t < 0x1c; ++t)
        {
            in.game_timer = 9U + t;
            e = (SotnWeaponSecondaryEvents){0};
            const Sint32 vx = s.velocity_x, vy = s.velocity_y, x = s.x, y = s.y;
            const Sint16 rotate = s.rotate;
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.x == x + vx &&
                  s.y == y + vy && s.velocity_x == vx + (k ? 0x2000 : -0x2000) &&
                  s.velocity_y == vy + 0x400 && s.rotate == (Sint16)(rotate + 0x200) &&
                  rng == check && !e.properties && (e.sounds[0] == 0x60b) == !(in.game_timer & 7U));
        }
        CHECK(s.phase == 2 && !s.lifetime);
        Uint16 updates = 0;
        for (t = 0; t < 400 && !s.removed; ++t)
        {
            const Sint16 limit = s.magnitude, angle = s.angle, rotate = s.rotate;
            in.game_timer = 0x40U + t;
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
            CHECK(s.magnitude == limit + 0x10 && s.rotate == (Sint16)(rotate + 0x200));
            if (s.removed)
            {
                CHECK(!e.sounds[0]);
                break;
            }
            Sint16 turn = (Sint16)(s.angle - angle);
            if (turn > 0x800)
                turn = (Sint16)(turn - 0x1000);
            if (turn < -0x800)
                turn = (Sint16)(turn + 0x1000);
            CHECK(turn <= limit && turn >= -limit && s.facing_left == (k != 0) &&
                  (e.sounds[0] == 0x60b) == !(in.game_timer & 7U));
            ++updates;
        }
        CHECK(s.removed && updates);
    }
    CHECK(SotnWeaponSecondary_SupportedRole(34, 1) && !SotnWeaponSecondary_SupportedRole(34, 0) &&
          !SotnWeaponSecondary_SupportedRole(34, 2) && !SotnWeaponSecondary_RoleRows(34, 1));
    return true;
}

// w_053 func_ptr_8017000C: Yasutsuna's blueprint68 slash stays at its origin (only the crouch Y
// offset), rolls properties and chains WFACTORY(0x38,age+1) every callback for parameter0, and
// keeps its fixed hitbox offset while the player moves.
static bool Yasutsuna(void)
{
    static Uint8 o[0x1400], exe[0x24000];
    SlashProgram(o);
    Math(exe);
    const Uint16 hit = 2;
    SotnHandWeaponProgram program = {0};
    for (Uint16 hand = 0; hand < 2; ++hand)
    {
        SotnWeaponSecondary s = {
            .parameter = (Uint16)(hand << 15), .x = 100 * 65536 + 0x1111, .y = 90 * 65536 + 0x2222};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = {exe, sizeof(exe)},
                                       .properties = Properties,
                                       .context = (void *)&hit,
                                       .x = 140 * 65536,
                                       .y = 120 * 65536,
                                       .hand = hand,
                                       .family = 53,
                                       .role = 3,
                                       .priority = 148,
                                       .facing_left = hand != 0,
                                       .crouch = hand != 0,
                                       .allocated = true};
        Uint32 rng = 5;
        for (Uint16 t = 0; t < 9; ++t)
        {
            SotnWeaponSecondaryEvents e = {0};
            in.x = (140 + t * 7) * 65536;
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
            CHECK(e.chain == (0x38U | (hand + 1U) << 12 | (Uint32)(t + 1U) << 16) &&
                  e.properties == (t == 0) && s.x == 100 * 65536 + 0x1111 &&
                  s.y == (90 + (hand ? 11 : -5)) * 65536 + 0x2222);
            if (t < 8)
                CHECK(!s.removed && s.hitbox_x == 0x12 && s.hitbox_width == 30 &&
                      s.quad.palette == (hand ? 0x18 : 0) + 0x11a + t);
        }
        CHECK(s.removed);
    }
    // Other parameters (even Katana's later roots and 0x28..0x31) neither roll properties nor
    // chain.
    const Uint16 others[] = {1, 5, 0x28, 0x2a};
    for (Uint16 k = 0; k < 4; ++k)
    {
        SotnWeaponSecondary s = {.parameter = (Uint16)(others[k] << 8)};
        SotnWeaponSecondaryEvents e = {0};
        Uint32 rng = 5;
        CHECK(SotnWeaponSecondary_Tick(&s, &program,
                                       (SotnWeaponSecondaryInput){.overlay = {o, sizeof(o)},
                                                                  .executable = {exe, sizeof(exe)},
                                                                  .properties = Properties,
                                                                  .context = (void *)&hit,
                                                                  .family = 53,
                                                                  .role = 3,
                                                                  .allocated = true},
                                       &rng, &e) &&
              !e.properties && !e.chain && rng == 5 && !s.hitbox_width && s.hitbox_x == 0x12);
    }
    CHECK(SotnWeaponSecondary_SupportedRole(53, 3) && SotnWeaponSecondary_RoleRows(53, 3) == 1 &&
          !SotnWeaponSecondary_SupportedRole(53, 2));
    return true;
}

// Room entry translates DRAW_UNK_200 GT4s (all four vertices) without touching their UVs,
// schedules or modes, and never applies the LINE_G2 row translation to quad storage.
static bool QuadsRebased(const SotnWeaponSecondary *before, const SotnWeaponSecondary *after,
                         Uint16 count, Sint16 dx, Sint16 dy)
{
    for (Uint16 j = 0; j < count; ++j)
    {
        SotnPlayerEffectQuad expected = before->quads[j];
        for (Uint16 v = 0; v < 4; ++v)
        {
            expected.x[v] = (Sint16)(expected.x[v] + dx);
            expected.y[v] = (Sint16)(expected.y[v] + dy);
        }
        if (SDL_memcmp(&expected, &after->quads[j], sizeof(expected)) ||
            before->quad_timer[j] != after->quad_timer[j] ||
            before->quad_phase[j] != after->quad_phase[j])
            return false;
    }
    return true;
}
static void FamilyDefinition(Uint8 *dra, Uint16 item, Uint8 category, Uint8 family, Uint8 kind,
                             Uint8 variant, Uint16 mp, Uint8 special)
{
    Uint8 *p = dra + 0x500 + item * 52U;
    Put16(p + 8, 26);
    Put16(p + 12, 64);
    p[14] = category;
    p[15] = family;
    p[17] = kind;
    p[18] = 0x41;
    p[19] = 48;
    p[20] = variant;
    p[21] = 13;
    p[22] = 128;
    p[24] = special;
    p[26] = 4;
    Put16(p + 36, mp);
    Put16(p + 38, 4);
    Put16(p + 40, 2);
    Put16(p + 42, 129);
}
static void Definition(Uint8 *dra, Uint16 item, Uint8 kind, Uint8 variant, Uint16 mp, Uint8 special)
{
    FamilyDefinition(dra, item, 1, 55, kind, variant, mp, special);
}
static bool Runtime(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(56, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r)), *before = SDL_malloc(sizeof(*before));
    static Uint8 exe[0x24000];
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r && before);
    TestCharacter_Fill(dra);
    Math(exe);
    Definition(dra, 115, 1, 0, 0, 196);
    Definition(dra, 196, 28, 1, 5, 0);
    // Authored blueprint48 (slot16 replace) and56 (first free17..31, noncritical).
    Uint8 *p = dra + 0xd1d4 + 48U * 6U;
    p[1] = 1;
    p[2] = 0xc1;
    p[4] = 0x23;
    dra[0xd4b8 + 6] = dra[0xd4b9 + 6] = 16;
    p = dra + 0xd1d4 + 56U * 6U;
    p[0] = 1;
    p[1] = 1;
    p[2] = 0xc1;
    p[4] = 0x06;
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    Program(weapons + 55U * 0x7000 + 0x4000);
    weapons[55U * 0x7000 + 0x4000 + 0x10c8] = 0x41; // Both descriptors start at0x41 here.
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    actors->primitives[7].owner = 777;
    CHECK(SotnCharacter_Create(actors->dra, &character));
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = {exe, sizeof(exe)},
                              .weapons = {weapons, 56U * 0x7000},
                              .weapon_family = {55, 55},
                              .weapon_bound = {true, true},
                              .camera_x = 20,
                              .camera_y = 8,
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 128 * 65536 + 123,
                                         .y = 100 * 65536 + 456,
                                         .priority = 148,
                                         .attack_mode = 0x801d,
                                         .animation = {.id = 0x41, .pose = 0, .remaining = 2}}};
    const SotnPlayerFactorySource source = {.x = in.player.x, .y = in.player.y, .priority = 148};
    SotnCharacterState progress = {0};
    CHECK(SotnCharacter_Get(character, &progress, NULL));
    Uint32 expected = progress.random;
    (void)SotnCombat_Random(&expected);
    bool allocated = false;
    // Descriptor1 birth: SFX_MAGIC_SWITCH and WFACTORY(0x38,0) before its property roll.
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestHand(r, source, 0x11030, 196, &allocated) && allocated &&
          SotnBatRuntime_Tick(r, character, in) && r->children[0].hand_weapon &&
          r->children[0].group == 1 && r->sound_count == 1 && r->sounds[0] == 0x69d &&
          SotnCharacter_Get(character, &progress, NULL) && progress.random == expected);
    Uint16 pending = 0;
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        if (r->factories[i].secondary_factory)
        {
            ++pending;
            CHECK(r->factories[i].cursor.blueprint == 56 &&
                  r->factories[i].cursor.entity_modifier == 0xe0 &&
                  r->factories[i].cursor.source.x == r->children[0].state.hand.motion.x &&
                  r->factories[i].cursor.source.priority == 146);
        }
    // The slot16 hand weapon receives collision.h's attacker hitFlags for its own generation.
    CHECK(!SotnBatRuntime_Hit(r, 0, r->children[0].generation + 1U, 1) &&
          !r->children[0].state.hand.motion.hit_flags &&
          SotnBatRuntime_Hit(r, 0, r->children[0].generation, 3) &&
          r->children[0].state.hand.motion.hit_flags == 3);
    r->children[0].state.hand.motion.hit_flags = 0;
    CHECK(pending == 1 && !r->children[1].kind && SotnActors_CommitBatPrimitives(actors, r));
    // Next pass: the factory spawns E1 at slot17, which reserves its64 rows before RNG.
    for (Uint16 i = 0; i < 129; ++i)
        (void)SotnCombat_Random(&expected);
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && r->children[1].weapon_secondary &&
          r->children[1].kind == 0xe1 && r->children[1].initialized &&
          r->children[1].primitive_count == 64 && r->children[1].group == 1 &&
          r->children[1].state.secondary.item == 196 && (r->secondary_mask & 2U) &&
          r->groups[1] == 1 && !r->groups[3] && SotnBatRuntime_SecondaryValid(&r->children[1], 1) &&
          SotnCharacter_Get(character, &progress, NULL) && progress.random == expected &&
          SotnActors_CommitBatPrimitives(actors, r) && actors->primitives[7].owner == 777);
    Uint16 owned = 0;
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        owned += actors->primitives[i].owner == r->children[1].generation &&
                 actors->primitives[i].ability_kind == 0xe1;
    CHECK(owned == 64);
    // Frozen updates hold the child; room entry translates position and both row endpoints.
    *before = *r;
    in.freeze = 3;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) &&
          !SDL_memcmp(&before->children[1], &r->children[1], sizeof(r->children[1])) &&
          SotnCharacter_Get(character, &progress, NULL) && progress.random == expected);
    in.freeze = 0;
    in.entering = true;
    in.room_shift_x = 7;
    in.screen_shift_x = -5;
    in.player.animation.remaining = 1;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) &&
          r->children[1].state.secondary.motion.x ==
              before->children[1].state.secondary.motion.x + 7 * 65536 &&
          r->children[1].state.secondary.motion.rows[3].x[1] ==
              before->children[1].state.secondary.motion.rows[3].x[1] - 5 &&
          r->children[1].state.secondary.motion.remaining ==
              before->children[1].state.secondary.motion.remaining &&
          SotnActors_CommitBatPrimitives(actors, r));
    in.entering = false;
    in.room_shift_x = in.screen_shift_x = 0;
    const SotnWeaponSecondary moved = r->children[1].state.secondary.motion;
    CHECK(SotnBatRuntime_PlayerShove(r, 65536, -32768) &&
          r->children[1].state.secondary.motion.x == moved.x + 65536 &&
          r->children[1].state.secondary.motion.rows[0].x[0] == moved.rows[0].x[0]);
    // A failed pass (lost resident program) leaves the whole runtime and RNG unchanged.
    *before = *r;
    in.weapon_family[0] = 54;
    CHECK(!SotnBatRuntime_Tick(r, character, in) && !SDL_memcmp(r, before, sizeof(*r)) &&
          SotnCharacter_Get(character, &progress, NULL) && progress.random == expected);
    in.weapon_family[0] = 55;
    // Ordinary updates finish the private timeline; retirement releases all64 rows.
    for (Uint16 t = 0; t < 12 && r->children[1].kind; ++t)
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    CHECK(!r->children[1].kind && !(r->secondary_mask & 2U));
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        CHECK(actors->primitives[i].owner == (i == 7 ? 777U : 0U));
    // Mandatory-row exhaustion retires the new secondary before RNG/properties, without retry.
    in.player.animation.remaining = 2;
    CHECK(SotnBatRuntime_RetireHand(r, false));
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestHand(r, source, 0x11030, 196, &allocated) && allocated &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    CHECK(SotnCharacter_Get(character, &progress, NULL));
    expected = progress.random;
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        if (!actors->primitives[i].owner)
            actors->primitives[i].owner = 999;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && !r->children[1].kind &&
          SotnCharacter_Get(character, &progress, NULL) && progress.random == expected &&
          SotnActors_CommitBatPrimitives(actors, r));
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        CHECK(!r->factories[i].cursor.entity_id);
    SotnCharacter_Destroy(character);
    SDL_free(before);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}
// Katana189's root (raw parent slot16) chains nine slashes; each chained child's raw parent is
// the root's own slot, reads no properties and owns one shared GT4 row until it retires.
static bool KatanaRuntime(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(56, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    static Uint8 exe[0x24000];
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Math(exe);
    FamilyDefinition(dra, 100, 5, 53, 1, 0, 0, 189);
    FamilyDefinition(dra, 189, 5, 53, 1, 1, 0, 0);
    Uint8 *p = dra + 0xd1d4 + 48U * 6U;
    p[1] = 1;
    p[2] = 0xc1;
    p[4] = 0x23;
    dra[0xd4b8 + 6] = dra[0xd4b9 + 6] = 16;
    p = dra + 0xd1d4 + 56U * 6U;
    p[0] = 1;
    p[1] = 1;
    p[2] = 0xc1;
    p[4] = 0x06;
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    Uint8 *o = weapons + 53U * 0x7000 + 0x4000;
    Program(o);
    for (Uint16 v = 0; v < 5; ++v)
        SDL_memcpy(o + 0xf4c + v * 16U, o + 0x10ac, 16);
    SlashProgram(o);
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    CHECK(SotnCharacter_Create(actors->dra, &character));
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = {exe, sizeof(exe)},
                              .weapons = {weapons, 56U * 0x7000},
                              .weapon_family = {53, 53},
                              .weapon_bound = {true, true},
                              .camera_x = 20,
                              .camera_y = 8,
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 128 * 65536 + 123,
                                         .y = 100 * 65536 + 456,
                                         .priority = 148,
                                         .attack_mode = 0x8002,
                                         .animation = {.id = 0x41, .pose = 0, .remaining = 2}}};
    const SotnPlayerFactorySource source = {.x = in.player.x, .y = in.player.y, .priority = 148};
    bool allocated = false;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestHand(r, source, 0x11030, 189, &allocated) && allocated &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    Uint16 chained = 0, max_live = 0;
    bool root_seen = false;
    for (Uint16 t = 0; t < 30; ++t)
    {
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in));
        CHECK(SotnActors_CommitBatPrimitives(actors, r));
        Uint16 live = 0;
        for (Uint16 i = 1; i < 16; ++i)
        {
            const SotnBatRuntimeChild *c = &r->children[i];
            if (!c->weapon_secondary)
                continue;
            ++live;
            CHECK(c->primitive_count == 1 && SotnBatRuntime_SecondaryValid(c, i));
            if (!c->parameter)
            {
                root_seen = true;
                CHECK(c->state.secondary.parent_slot == 16 && c->group == 1 &&
                      c->state.secondary.item == 189 && c->state.secondary.attack.active);
            }
            else
            {
                // Chained children name the root's slot17, even when reborn into it.
                CHECK(c->state.secondary.parent_slot == 17 && !c->group &&
                      !c->state.secondary.item && !c->state.secondary.attack.active &&
                      (c->parameter >> 8) <= 9);
                chained |= (Uint16)(1U << (c->parameter >> 8));
            }
        }
        max_live = SDL_max(max_live, live);
    }
    // Nine chained slashes (params1..9) follow the root; every row is released afterwards.
    // The root retires in the update its eighth chained child is born, so at most eight overlap.
    CHECK(root_seen && chained == 0x3fe && max_live == 8);
    for (Uint16 i = 1; i < 16; ++i)
        CHECK(!r->children[i].weapon_secondary);
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        CHECK(!actors->primitives[i].owner || actors->primitives[i].ability_kind != 0xe1);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}
// Thunderbrand178's birth requests: the bp56 root in slot17, bp58 branches reverse-allocated in
// slots63/62/61 and the bp64 ring in slot60, each owning its rows and copying slot16's group.
static bool ThunderRuntime(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(56, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    static Uint8 exe[0x24000];
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Math(exe);
    FamilyDefinition(dra, 112, 1, 48, 1, 0, 0, 178);
    FamilyDefinition(dra, 178, 1, 48, 19, 1, 5, 0);
    const Uint8 blueprints[][6] = {{48, 0, 1, 0xc1, 0, 0x23},
                                   {56, 1, 1, 0xc1, 0, 0x06},
                                   {58, 1, 1, 0xc1, 0, 0x00},
                                   {64, 2, 1, 0xc1, 0, 0x00}};
    for (Uint16 i = 0; i < 4; ++i)
        SDL_memcpy(dra + 0xd1d4 + blueprints[i][0] * 6U, blueprints[i] + 1, 5);
    dra[0xd4b8 + 6] = dra[0xd4b9 + 6] = 16;
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    Uint8 *o = weapons + 48U * 0x7000 + 0x4000;
    Program(o);
    SDL_memcpy(o + 0xfc0, o + 0x10ac, 16);
    SDL_memcpy(o + 0xfd0, o + 0x10ac, 16);
    ThunderProgram(o);
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    CHECK(SotnCharacter_Create(actors->dra, &character));
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = {exe, sizeof(exe)},
                              .weapons = {weapons, 56U * 0x7000},
                              .weapon_family = {48, 48},
                              .weapon_bound = {true, true},
                              .camera_x = 20,
                              .camera_y = 8,
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 128 * 65536 + 123,
                                         .y = 100 * 65536 + 456,
                                         .priority = 148,
                                         .attack_mode = 0x8014,
                                         .animation = {.id = 0x41, .pose = 0, .remaining = 2}}};
    const SotnPlayerFactorySource source = {.x = in.player.x, .y = in.player.y, .priority = 148};
    bool allocated = false;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestHand(r, source, 0x11030, 178, &allocated) && allocated &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    Uint16 pending = 0;
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        pending += r->factories[i].cursor.entity_id && r->factories[i].secondary_factory &&
                   r->factories[i].secondary_parent_slot == 16;
    CHECK(pending == 5);
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    const struct
    {
        Uint16 index, kind, parameter, rows;
        Uint8 group;
    } expect[] = {{1, 0xe1, 0, 10, 1},
                  {47, 0xe1, 0x100, 10, 1},
                  {46, 0xe1, 0x200, 10, 1},
                  {45, 0xe1, 0x300, 10, 1},
                  {44, 0xe2, 0, 16, 0}};
    for (Uint16 k = 0; k < SDL_arraysize(expect); ++k)
    {
        const SotnBatRuntimeChild *c = &r->children[expect[k].index];
        CHECK(c->weapon_secondary && c->kind == expect[k].kind &&
              c->parameter == expect[k].parameter && c->primitive_count == expect[k].rows &&
              c->group == expect[k].group && c->initialized &&
              SotnBatRuntime_SecondaryValid(c, expect[k].index));
        CHECK(((r->secondary_mask >> expect[k].index) & 1U) == (expect[k].kind == 0xe1));
    }
    const SotnWeaponSecondary ring = r->children[44].state.secondary.motion;
    in.entering = true;
    in.screen_shift_x = 4;
    in.screen_shift_y = -2;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          QuadsRebased(&ring, &r->children[44].state.secondary.motion, 16, 4, -2));
    in.entering = false;
    in.screen_shift_x = in.screen_shift_y = 0;
    // Leaving the brand's animations retires every secondary and releases all rows.
    in.player.animation.id = 0x30;
    in.player.attack_mode = 0;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    for (Uint16 i = 1; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        CHECK(!r->children[i].weapon_secondary);
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        CHECK(!actors->primitives[i].owner);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}
// Osafune193's blueprint60 replaces slot16 with the warp; its pose6 root0x1E copies the warp's
// item193 and enemyId0 from that raw parent and keeps its birth position.
static bool WarpRuntime(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(56, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    static Uint8 exe[0x24000];
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Math(exe);
    FamilyDefinition(dra, 139, 5, 53, 1, 2, 0, 193);
    FamilyDefinition(dra, 193, 5, 53, 135, 0, 0, 0);
    dra[0x500 + 193 * 52 + 18] = 0xd8;
    dra[0x500 + 193 * 52 + 19] = 60;
    dra[0x500 + 193 * 52 + 21] = 0;
    const Uint8 blueprints[][6] = {{60, 2, 1, 0xc1, 0, 0x23}, {56, 1, 1, 0xc1, 0, 0x06}};
    for (Uint16 i = 0; i < 2; ++i)
        SDL_memcpy(dra + 0xd1d4 + blueprints[i][0] * 6U, blueprints[i] + 1, 5);
    dra[0xd4b8 + 6] = dra[0xd4b9 + 6] = 16;
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    SlashProgram(weapons + 53U * 0x7000 + 0x4000);
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    CHECK(SotnCharacter_Create(actors->dra, &character));
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = {exe, sizeof(exe)},
                              .weapons = {weapons, 56U * 0x7000},
                              .weapon_family = {53, 53},
                              .weapon_bound = {true, true},
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 128 * 65536,
                                         .y = 100 * 65536,
                                         .priority = 148,
                                         .animation = {.id = 0xd8, .pose = 0, .remaining = 2}}};
    const SotnPlayerFactorySource source = {.x = in.player.x, .y = in.player.y, .priority = 148};
    bool allocated = false;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestHand(r, source, 0x103c, 193, &allocated) && allocated &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          r->children[0].weapon_warp && r->children[0].kind == 0xe2 &&
          r->children[0].state.warp.item == 193 && SotnBatRuntime_WarpValid(&r->children[0], 0));
    in.player.animation.pose = 6;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    const Sint32 warp_x = r->children[0].state.warp.x;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    const SotnBatRuntimeChild *root = &r->children[1];
    CHECK(root->weapon_secondary && root->parameter == 0x1e00 && root->primitive_count == 1 &&
          root->state.secondary.item == 193 && !root->group &&
          root->state.secondary.parent_slot == 16 && root->state.secondary.attack.active &&
          root->state.secondary.motion.x == warp_x + 20 * 65536 && (r->secondary_mask & 2U));
    in.player.animation.pose = 10;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          r->children[0].state.warp.phase == 3);
    in.player.animation.pose = 15;
    for (Uint16 t = 0; t < 20; ++t)
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    CHECK(!r->children[0].kind);
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        CHECK(!actors->primitives[i].owner);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}
// Iron Fist186's blueprint54 replaces slot16 with the punch rush (its own item, hand group, 16
// rows); Tulkas188's descriptor4 requests the role2 fireball, which retires after a hit.
static bool FistRuntime(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(56, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    static Uint8 exe[0x24000];
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Math(exe);
    FamilyDefinition(dra, 102, 3, 1, 0, 2, 0, 186);
    FamilyDefinition(dra, 186, 3, 1, 24, 0, 0, 0);
    FamilyDefinition(dra, 188, 2, 1, 25, 4, 0, 0);
    const Uint16 items[] = {102, 186, 188};
    for (Uint16 i = 0; i < 3; ++i)
    {
        dra[0x500 + items[i] * 52 + 18] = 0x24;
        dra[0x500 + items[i] * 52 + 21] = i == 2 ? 7 : 5;
    }
    dra[0x500 + 186 * 52 + 19] = 54;
    const Uint8 blueprints[][6] = {
        {48, 0, 1, 0xc1, 0, 0x23}, {54, 1, 1, 0xc1, 0, 0x23}, {62, 2, 1, 0xc1, 0, 0x06}};
    for (Uint16 i = 0; i < 3; ++i)
        SDL_memcpy(dra + 0xd1d4 + blueprints[i][0] * 6U, blueprints[i] + 1, 5);
    dra[0xd4b8 + 6] = dra[0xd4b9 + 6] = 16;
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    FistProgram(weapons + 1U * 0x7000 + 0x4000);
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    CHECK(SotnCharacter_Create(actors->dra, &character));
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = {exe, sizeof(exe)},
                              .weapons = {weapons, 56U * 0x7000},
                              .weapon_family = {1, 1},
                              .weapon_bound = {true, true},
                              .camera_x = 20,
                              .camera_y = 8,
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 128 * 65536,
                                         .y = 100 * 65536,
                                         .priority = 148,
                                         .attack_mode = 0x8019,
                                         .animation = {.id = 0x24, .pose = 0, .remaining = 2}}};
    const SotnPlayerFactorySource source = {.x = in.player.x, .y = in.player.y, .priority = 148};
    bool allocated = false;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestHand(r, source, 0x1036, 186, &allocated) && allocated &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    const SotnBatRuntimeChild *rush = &r->children[0];
    CHECK(rush->weapon_secondary && !rush->hand_weapon && rush->kind == 0xe1 &&
          rush->state.secondary.family == 1 && rush->state.secondary.item == 186 &&
          rush->group == 1 && rush->primitive_count == 16 && rush->initialized &&
          rush->state.secondary.attack.active && rush->state.secondary.attack.equipment == 186 &&
          (r->secondary_mask & 1U) && SotnBatRuntime_SecondaryValid(rush, 0));
    // A rush identity outside slot16 or with another item is rejected.
    SotnBatRuntimeChild forged = *rush;
    forged.state.secondary.item = 188;
    CHECK(!SotnBatRuntime_SecondaryValid(&forged, 0) && !SotnBatRuntime_SecondaryValid(rush, 1));
    Uint16 rows = 0;
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        rows += actors->primitives[i].owner != 0;
    CHECK(rows == 16);
    const SotnWeaponSecondary punches = rush->state.secondary.motion;
    in.entering = true;
    in.room_shift_x = 7;
    in.screen_shift_x = -5;
    in.screen_shift_y = 3;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          rush->state.secondary.motion.remaining == punches.remaining &&
          QuadsRebased(&punches, &rush->state.secondary.motion, 16, -5, 3));
    in.entering = false;
    in.room_shift_x = in.screen_shift_x = in.screen_shift_y = 0;
    // Ending the player's attack retires the rush and releases its rows.
    in.player.attack_mode = 0;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          !r->children[0].kind && !(r->secondary_mask & 1U));
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        CHECK(!actors->primitives[i].owner);
    // Tulkas's descriptor4 hand weapon requests the fireball on pose2's last update.
    in.player.attack_mode = 0x801a;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestHand(r, source, 0x41030, 188, &allocated) && allocated &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          r->children[0].hand_weapon && r->children[0].state.hand.item == 188);
    in.player.animation.pose = 2;
    in.player.animation.remaining = 1;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    in.player.animation.remaining = 2;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    const SotnBatRuntimeChild *fire = &r->children[1];
    CHECK(fire->weapon_secondary && fire->kind == 0xe2 && fire->state.secondary.family == 1 &&
          fire->state.secondary.item == 188 && fire->group == 1 && !fire->primitive_count &&
          fire->initialized && fire->state.secondary.attack.active &&
          fire->state.secondary.motion.frame == 1 && (r->secondary_mask & 2U) &&
          SotnBatRuntime_SecondaryValid(fire, 1));
    const Sint32 x = fire->state.secondary.motion.x;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          fire->state.secondary.motion.x == x + 0x48000 && fire->state.secondary.motion.frame == 2);
    CHECK(!SotnBatRuntime_Hit(r, 1, fire->generation + 1U, 1) &&
          SotnBatRuntime_Hit(r, 1, fire->generation, 1) &&
          fire->state.secondary.motion.hit_flags == 1);
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          !r->children[1].kind && !(r->secondary_mask & 2U));
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}
// Flamberge183's blueprint54 replaces slot16 with the E1 special hand child (its own item, hand
// group, glow writes); pose1 requests the fan root, which copies 183 and the hand group from
// that raw parent and chains its blades from its own slot.
static bool FanRuntime(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(56, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    static Uint8 exe[0x24000];
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Math(exe);
    FamilyDefinition(dra, 101, 5, 51, 6, 1, 0, 0);
    FamilyDefinition(dra, 183, 1, 51, 23, 1, 5, 0);
    dra[0x500 + 101 * 52 + 18] = 0x68;
    dra[0x500 + 101 * 52 + 23] = 183;
    dra[0x500 + 183 * 52 + 18] = 0xd6;
    dra[0x500 + 183 * 52 + 19] = 54;
    const Uint8 blueprints[][6] = {{54, 1, 1, 0xc1, 0, 0x23}, {62, 2, 1, 0xc1, 0, 0x06}};
    for (Uint16 i = 0; i < 2; ++i)
        SDL_memcpy(dra + 0xd1d4 + blueprints[i][0] * 6U, blueprints[i] + 1, 5);
    dra[0xd4b8 + 6] = dra[0xd4b9 + 6] = 16;
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    Weapon51Program(weapons + 51U * 0x7000 + 0x4000);
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    CHECK(SotnCharacter_Create(actors->dra, &character));
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = {exe, sizeof(exe)},
                              .weapons = {weapons, 56U * 0x7000},
                              .weapon_family = {51, 51},
                              .weapon_bound = {true, true},
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 128 * 65536,
                                         .y = 100 * 65536,
                                         .priority = 148,
                                         .attack_mode = 0x8018,
                                         .animation = {.id = 0xd6, .pose = 0, .remaining = 2}}};
    const SotnPlayerFactorySource source = {.x = in.player.x, .y = in.player.y, .priority = 148};
    bool allocated = false;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestHand(r, source, 0x11036, 183, &allocated) && allocated &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    const SotnBatRuntimeChild *special = &r->children[0];
    CHECK(special->hand_weapon && special->kind == 0xe1 && special->state.hand.item == 183 &&
          special->state.hand.family == 51 && special->group == 1 && special->initialized &&
          special->state.hand.motion.palette == 0x112 && SotnBatRuntime_HandValid(special, 0) &&
          r->clut_count == 1 && r->clut[0].palette == 0x112 && r->clut[0].color == 11);
    SotnBatRuntimeChild forged = *special;
    forged.kind = 0xe0;
    CHECK(!SotnBatRuntime_HandValid(&forged, 0));
    // The ordinary blueprint48 identity cannot carry a special item.
    SotnBatRuntime *other = SDL_calloc(1, sizeof(*other));
    CHECK(other && !SotnBatRuntime_RequestHand(other, source, 0x11030, 183, &allocated));
    SDL_free(other);
    in.player.animation.pose = 1;
    in.player.animation.remaining = 1;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    in.player.animation.remaining = 2;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    const SotnBatRuntimeChild *root = &r->children[1];
    CHECK(root->weapon_secondary && root->kind == 0xe2 && root->state.secondary.family == 51 &&
          !root->parameter && root->state.secondary.item == 183 && root->group == 1 &&
          root->primitive_count == 1 && root->state.secondary.attack.active &&
          (r->secondary_mask & 2U) && SotnBatRuntime_SecondaryValid(root, 1));
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    Uint16 blades = 0;
    for (Uint16 i = 2; i < 16; ++i)
        if (r->children[i].weapon_secondary)
        {
            const SotnBatRuntimeChild *b = &r->children[i];
            CHECK(b->parameter == 0x100 && b->state.secondary.parent_slot == 17 &&
                  !b->state.secondary.attack.active && b->primitive_count == 1 &&
                  SotnBatRuntime_SecondaryValid(b, i));
            ++blades;
        }
    CHECK(blades == 1);
    // Leaving the special's animations retires it; the fan finishes on its own schedule.
    in.player.animation.id = 0x30;
    in.player.attack_mode = 0;
    for (Uint16 t = 0; t < 24; ++t)
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        CHECK(!r->children[i].kind);
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        CHECK(!actors->primitives[i].owner);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}
// Firebrand179's descriptor1 spawns eight sparks (the hand's attack, their own least-used
// groups, item179) and pose1 puffs; spark0 requests the ring. Marsil180's descriptor3 spawns
// swirls, whose pillars copy the swirl as their raw parent, and the param1 ring.
static bool BrandRuntime(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(56, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    static Uint8 exe[0x24000];
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Math(exe);
    FamilyDefinition(dra, 179, 1, 49, 20, 1, 5, 0);
    FamilyDefinition(dra, 180, 1, 49, 21, 3, 5, 0);
    const Uint8 blueprints[][6] = {{48, 0, 1, 0xc1, 0, 0x23}, {0x5b, 1, 8, 0xc8, 0, 6},
                                   {0x5d, 3, 8, 0xc1, 1, 6},  {0x5f, 4, 8, 0xc8, 0, 1},
                                   {0x40, 2, 1, 0xc1, 0, 0},  {0x65, 5, 1, 0xc1, 0, 1}};
    for (Uint16 i = 0; i < SDL_arraysize(blueprints); ++i)
        SDL_memcpy(dra + 0xd1d4 + blueprints[i][0] * 6U, blueprints[i] + 1, 5);
    const Uint8 ranges[][2] = {{32, 63}, {17, 47}, {0, 0}, {16, 16}, {0, 0}, {0, 0}, {17, 31}};
    for (Uint16 k = 0; k < SDL_arraysize(ranges); ++k)
        if (ranges[k][1])
        {
            dra[0xd4b8 + k * 2] = ranges[k][0];
            dra[0xd4b9 + k * 2] = ranges[k][1];
        }
    Weapon49Program(weapons + 49U * 0x7000 + 0x4000);
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    CHECK(SotnCharacter_Create(actors->dra, &character));
    for (Uint16 k = 0; k < 2; ++k)
    {
        const Uint16 item = k ? 180 : 179;
        SotnBatRuntimeInput in = {.dra = actors->dra,
                                  .executable = {exe, sizeof(exe)},
                                  .weapons = {weapons, 56U * 0x7000},
                                  .weapon_family = {49, 49},
                                  .weapon_bound = {true, true},
                                  .player = {.hp = 70,
                                             .hp_max = 70,
                                             .x = 128 * 65536,
                                             .y = 100 * 65536,
                                             .priority = 148,
                                             .attack_mode = 0x8015,
                                             .animation = {.id = 0x41, .pose = 0, .remaining = 2}}};
        const SotnPlayerFactorySource source = {
            .x = in.player.x, .y = in.player.y, .priority = 148};
        bool allocated = false;
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_RequestHand(r, source, k ? 0x31030U : 0x11030U, item, &allocated) &&
              allocated && SotnBatRuntime_Tick(r, character, in) &&
              SotnActors_CommitBatPrimitives(actors, r));
        Uint32 roles = 0, parents = 0;
        Uint16 sparks = 0, max_children = 0;
        for (Uint16 t = 0; t < 30; ++t)
        {
            in.player.animation.pose = (Uint16)(t < 3 ? 0 : 1);
            // Room entry never applies the LINE_G2 row translation to a spark's motion words.
            for (Uint16 i = 1; i < 48 && !k && t == 5; ++i)
                if (r->children[i].weapon_secondary && (r->children[i].kind & 15U) == 1)
                {
                    const SotnWeaponSecondary spark = r->children[i].state.secondary.motion;
                    in.entering = true;
                    in.screen_shift_x = -5;
                    in.screen_shift_y = 3;
                    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
                          SotnBatRuntime_Tick(r, character, in) &&
                          SotnActors_CommitBatPrimitives(actors, r));
                    const SotnWeaponSecondary *after = &r->children[i].state.secondary.motion;
                    CHECK(after->angle == spark.angle && after->magnitude == spark.magnitude &&
                          after->prev_x == spark.prev_x && after->prev_y == spark.prev_y);
                    in.entering = false;
                    in.screen_shift_x = in.screen_shift_y = 0;
                    break;
                }
            CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
                  SotnBatRuntime_Tick(r, character, in) &&
                  SotnActors_CommitBatPrimitives(actors, r));
            Uint16 children = 0;
            for (Uint16 i = 1; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
            {
                const SotnBatRuntimeChild *c = &r->children[i];
                if (!c->weapon_secondary)
                    continue;
                ++children;
                const Uint16 role = c->kind & 15U;
                CHECK(c->state.secondary.family == 49 && SotnBatRuntime_SecondaryValid(c, i) &&
                      c->primitive_count == (role == 2 ? 16 : 0));
                roles |= 1U << role;
                if (role == 1 && c->initialized)
                {
                    CHECK(c->state.secondary.item == 179 && c->group >= 3 && c->group <= 6 &&
                          c->state.secondary.attack.power ==
                              r->children[0].state.hand.motion.properties.attack);
                    ++sparks;
                }
                if (role == 5)
                    parents |= 1U << (c->state.secondary.parent_slot - 16U);
            }
            max_children = SDL_max(max_children, children);
        }
        CHECK(roles == (k ? 0x3cU : 0x0eU) && (k || sparks >= 8) && max_children >= 8);

        if (k)
            for (Uint16 i = 0; i < 32; ++i)
                if ((parents >> i) & 1U)
                    CHECK(i >= 1);
        in.player.animation.id = 0x30;
        in.player.attack_mode = 0;
        for (Uint16 t = 0; t < 60; ++t)
            CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
                  SotnBatRuntime_Tick(r, character, in) &&
                  SotnActors_CommitBatPrimitives(actors, r));
        for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
            CHECK(!r->children[i].kind);
        for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
            CHECK(!actors->primitives[i].owner);
    }
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}
// Icebrand181's descriptor1 spawns the eighty-row sparkles and the ring; pose1 adds frost and
// crystals. Each role leases its actual rows (80/16/32/8) and releases them on retirement.
static bool IceRuntime(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(56, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    static Uint8 exe[0x24000];
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Math(exe);
    FamilyDefinition(dra, 181, 1, 50, 22, 1, 5, 0);
    const Uint8 blueprints[][6] = {{48, 0, 1, 0xc1, 0, 0x23},
                                   {0x3a, 1, 1, 0xc1, 0, 0},
                                   {0x40, 2, 1, 0xc1, 0, 0},
                                   {0x44, 3, 1, 0xc1, 0, 6},
                                   {0x61, 4, 1, 0xc1, 0, 0}};
    for (Uint16 i = 0; i < SDL_arraysize(blueprints); ++i)
        SDL_memcpy(dra + 0xd1d4 + blueprints[i][0] * 6U, blueprints[i] + 1, 5);
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    dra[0xd4b8 + 6] = dra[0xd4b9 + 6] = 16;
    Weapon50Program(weapons + 50U * 0x7000 + 0x4000);
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    CHECK(SotnCharacter_Create(actors->dra, &character));
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = {exe, sizeof(exe)},
                              .weapons = {weapons, 56U * 0x7000},
                              .weapon_family = {50, 50},
                              .weapon_bound = {true, true},
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 128 * 65536,
                                         .y = 100 * 65536,
                                         .priority = 148,
                                         .attack_mode = 0x8016,
                                         .animation = {.id = 0x41, .pose = 0, .remaining = 2}}};
    const SotnPlayerFactorySource source = {.x = in.player.x, .y = in.player.y, .priority = 148};
    bool allocated = false;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestHand(r, source, 0x11030U, 181, &allocated) && allocated &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    Uint32 roles = 0;
    Uint16 max_rows = 0;
    for (Uint16 t = 0; t < 8; ++t)
    {
        in.player.animation.pose = (Uint16)(t < 2 ? 0 : 1);
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        Uint16 rows = 0;
        for (Uint16 i = 1; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        {
            const SotnBatRuntimeChild *c = &r->children[i];
            if (!c->weapon_secondary)
                continue;
            const Uint16 role = c->kind & 15U;
            CHECK(c->state.secondary.family == 50 && SotnBatRuntime_SecondaryValid(c, i) &&
                  c->primitive_count == SotnWeaponSecondary_RoleRows(50, role) &&
                  c->state.secondary.item == ((role == 2 || role == 3) ? 181 : 0));
            roles |= 1U << role;
        }
        for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
            rows += actors->primitives[i].owner != 0;
        max_rows = SDL_max(max_rows, rows);
    }
    CHECK(roles == 0x1eU && max_rows == 80 + 16 + 32 + 8);
    in.player.animation.id = 0x30;
    in.player.attack_mode = 0;
    for (Uint16 t = 0; t < 12; ++t)
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        CHECK(!r->children[i].kind);
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        CHECK(!actors->primitives[i].owner);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}
// Moon rod172's blueprint72 spawns five crescents (one per three updates) from the player: their
// own item, least-used groups, the shield clear and timer10 request; hits feed back and their
// sparkles chain from the crescent slot.
static bool MoonRuntime(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(56, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    static Uint8 exe[0x24000];
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Math(exe);
    FamilyDefinition(dra, 172, 4, 6, 5, 0, 5, 0);
    dra[0x500 + 172 * 52 + 18] = 0x66;
    dra[0x500 + 172 * 52 + 19] = 72;
    dra[0x500 + 172 * 52 + 22] = 1;
    const Uint8 blueprints[][6] = {{72, 1, 5, 0xc1, 3, 0x26}, {0x40, 2, 1, 0xc1, 0, 0}};
    for (Uint16 i = 0; i < 2; ++i)
        SDL_memcpy(dra + 0xd1d4 + blueprints[i][0] * 6U, blueprints[i] + 1, 5);
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    Weapon6Program(weapons + 6U * 0x7000 + 0x4000);
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    CHECK(SotnCharacter_Create(actors->dra, &character));
    SotnBatRuntimeInput in = {
        .dra = actors->dra,
        .executable = {exe, sizeof(exe)},
        .weapons = {weapons, 56U * 0x7000},
        .weapon_family = {6, 6},
        .weapon_bound = {true, true},
        .player = {.hp = 70,
                   .hp_max = 70,
                   .x = 128 * 65536,
                   .y = 100 * 65536,
                   .priority = 148,
                   .attack_mode = 0x8006,
                   .animation = {.id = 0x66, .hitbox_x = 4, .hitbox_y = -18}}};
    const SotnPlayerFactorySource source = {.x = in.player.x, .y = in.player.y, .priority = 148};
    bool allocated = false;
    CHECK(SotnBatRuntime_RequestHand(r, source, 0x1048U, 172, &allocated) && allocated);
    Uint16 crescents = 0, births = 0;
    for (Uint16 t = 0; t < 16; ++t)
    {
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        births += r->magic_clear_shield && r->use_ticks == 4;
        crescents = 0;
        for (Uint16 i = 1; i < 16; ++i)
        {
            const SotnBatRuntimeChild *c = &r->children[i];
            if (!c->weapon_secondary || (c->kind & 15U) != 1)
                continue;
            CHECK(c->state.secondary.family == 6 && c->state.secondary.own_item &&
                  c->state.secondary.item == 172 && c->group >= 3 && c->group <= 6 &&
                  SotnBatRuntime_SecondaryValid(c, i) && !c->primitive_count);
            ++crescents;
        }
    }
    CHECK(crescents == 5 && births == 5);
    // A collision hit feeds back to a crescent, which flashes on its next callback.
    Uint16 hit = 0;
    for (Uint16 i = 1; i < 16 && !hit; ++i)
        if (r->children[i].weapon_secondary && (r->children[i].kind & 15U) == 1)
            hit = i;
    CHECK(hit && SotnBatRuntime_Hit(r, hit, r->children[hit].generation, 1));
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          r->children[hit].state.secondary.motion.phase == 4);
    // A forged crescent without its own item is rejected.
    SotnBatRuntimeChild forged = r->children[hit];
    forged.state.secondary.own_item = false;
    CHECK(!SotnBatRuntime_SecondaryValid(&forged, hit));
    for (Uint16 t = 0; t < 200; ++t)
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    Uint16 sparkle_rows = 0;
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        sparkle_rows += r->children[i].weapon_secondary ? r->children[i].primitive_count : 0;
    CHECK(!sparkle_rows);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}
// Blueprint50 creates the Runesword root in slots17..31 (own item143, least-used group, 48 rows);
// its WFACTORY(0x72,0) hitboxes copy its equip id/group, and all retire once it homes back.
static bool RuneRuntime(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(56, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    static Uint8 exe[0x24000];
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Math(exe);
    FamilyDefinition(dra, 143, 2, 14, 129, 0, 0, 0);
    Uint8 *record = dra + 0x500 + 143 * 52;
    record[18] = 0x5d;
    record[19] = 50;
    record[21] = 0;
    record[22] = 1;
    const Uint8 blueprints[][6] = {{50, 0, 1, 0xc1, 0, 0x26}, {0x72, 1, 4, 0xc4, 0, 0x01}};
    for (Uint16 i = 0; i < 2; ++i)
        SDL_memcpy(dra + 0xd1d4 + blueprints[i][0] * 6U, blueprints[i] + 1, 5);
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    dra[0xd4b8 + 2] = 17;
    dra[0xd4b9 + 2] = 47;
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    SotnEquipment rune = {0};
    CHECK(SotnEquipment_Read(actors->dra, 143, &rune) && rune.kind == 129 && rune.family == 14 &&
          rune.blueprint == 50 && rune.chain_limit == 1 && !rune.consumable);
    record[25] = 1;
    CHECK(!SotnEquipment_Read(actors->dra, 143, &rune));
    record[25] = 0;
    CHECK(SotnCharacter_Create(actors->dra, &character));
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = {exe, sizeof(exe)},
                              .weapons = {weapons, 56U * 0x7000},
                              .weapon_family = {14, 14},
                              .weapon_bound = {true, true},
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 128 * 65536,
                                         .y = 100 * 65536,
                                         .priority = 148,
                                         .animation = {.id = 0x5d,
                                                       .hitbox_x = 4,
                                                       .hitbox_y = -18,
                                                       .hitbox_width = 12,
                                                       .hitbox_height = 30}}};
    const SotnPlayerFactorySource source = {.x = in.player.x, .y = in.player.y, .priority = 148};
    bool allocated = false;
    CHECK(!SotnBatRuntime_RequestThrown(r, source, 0x1033U, 143, &allocated));
    CHECK(SotnBatRuntime_RequestThrown(r, source, 0x1032U, 143, &allocated) && allocated);
    Uint16 root = 0, hitboxes = 0, volume = 0;
    for (Uint16 t = 0; t < 6; ++t)
    {
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        for (Uint16 k = 0; k < r->sound_count; ++k)
            if (r->sounds[k] == 0x677)
                volume += ((r->sound_volume_mask >> k) & 1U) && r->sound_volumes[k] == 0x50;
            else if (r->sounds[k] == 0x66c)
                CHECK(!((r->sound_volume_mask >> k) & 1U));
        hitboxes = 0;
        for (Uint16 i = 1; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        {
            const SotnBatRuntimeChild *c = &r->children[i];
            if (!c->weapon_secondary || c->state.secondary.family != 14)
                continue;
            CHECK(SotnBatRuntime_SecondaryValid(c, i));
            if (c->kind == 0xe0)
            {
                CHECK(i >= 1 && i <= 15 && c->state.secondary.own_item &&
                      c->state.secondary.item == 143 && c->primitive_count == 48 && c->group >= 3 &&
                      c->group <= 6 && c->state.secondary.attack.equipment == 143);
                root = i;
            }
            else if (c->initialized)
            {
                CHECK(c->kind == 0xe1 && c->state.secondary.parent_slot == root + 16U &&
                      c->state.secondary.item == 143 && c->group == r->children[root].group &&
                      !c->primitive_count && c->state.secondary.attack.width == 4);
                ++hitboxes;
            }
        }
    }
    CHECK(root && hitboxes == 4 && volume == 1);
    // A forged root without its own item143 is rejected.
    SotnBatRuntimeChild forged = r->children[root];
    forged.state.secondary.item = 172;
    CHECK(!SotnBatRuntime_SecondaryValid(&forged, root));
    // Room entry moves the world-space trail corners with the entity.
    const SotnWeaponSecondary trail = r->children[root].state.secondary.motion;
    in.entering = true;
    in.room_shift_x = 7;
    in.room_shift_y = -3;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    const SotnWeaponSecondary *moved = &r->children[root].state.secondary.motion;
    CHECK(moved->rune_x0 == trail.rune_x0 + 7 && moved->rune_y2 == trail.rune_y2 - 3);
    for (Uint16 j = 0; j < 48; ++j)
        if (trail.rune_shown[j])
            CHECK(moved->rune_x1[j] == trail.rune_x1[j] + 7 &&
                  moved->rune_y3[j] == trail.rune_y3[j] - 3);
    in.entering = false;
    in.room_shift_x = in.room_shift_y = 0;
    // The sword returns to the stationary player: root and hitboxes retire with every row.
    for (Uint16 t = 0; t < 200; ++t)
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        CHECK(!r->children[i].weapon_secondary && !r->children[i].primitive_count);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}

// Blueprint50 creates the Heaven sword root (own item119, least-used group, no rows); its
// WFACTORY(0x3A,0) afterimages land in slots32..63 naming it, and all retire once it returns.
// w_013 func_ptr_80170008/8017000C: combo202's sweep circles out from the player's hitbox with
// screen positions (no CAMERA_LOCKED), sheds strike swords on even timers past -0x400, blinks at
// its target, then a late LINE_G2 row stretches and fades; strike swords wait for its step3,
// count down 1..32, then fly at its target.
static bool HeavenCombo(void)
{
    static Uint8 o[0x3000], exe[0x24000];
    Math(exe);
    for (Uint16 i = 0; i < 14; ++i)
        Put16(o + 0x5e0 + i * 2U, (Uint16)(i < 8 ? i + 1U : 14U - i));
    const SotnDataView x_exe = {exe, sizeof(exe)};
    const Uint16 hit = 2;
    SotnHandWeaponProgram program = {0};
    CHECK(SotnWeaponSecondary_SupportedRole(13, 2) && SotnWeaponSecondary_SupportedRole(13, 3) &&
          !SotnWeaponSecondary_SupportedRole(13, 4) && !SotnWeaponSecondary_RoleRows(13, 2) &&
          SotnWeaponSecondary_HeldRows(13, 2, 0, 4) == 1 &&
          !SotnWeaponSecondary_HeldRows(13, 2, 0, 3) &&
          !SotnWeaponSecondary_HeldRows(13, 3, 0, 4) && SotnWeaponSecondary_ComboSpell(202) &&
          SotnWeaponSecondary_ComboSpell(203) && !SotnWeaponSecondary_ComboSpell(201) &&
          !SotnWeaponSecondary_ComboSpell(119));
    for (Uint16 k = 0; k < 5; ++k)
    {
        const Uint16 hand = k == 4 ? 1 : 0;
        const bool left = (k & 1U) != 0, crouch = k == 2, starved = k == 3;
        const Sint32 cx = 40, cy = 16;
        SotnWeaponSecondary s = {
            .parameter = (Uint16)(hand << 15), .x = 100 * 65536 + 0x1234, .y = 80 * 65536 + 0x4321};
        SotnWeaponSecondaryEvents e = {0};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = x_exe,
                                       .properties = Properties,
                                       .context = (void *)&hit,
                                       .x = (k == 4 ? 190 : 171) * 65536,
                                       .y = 120 * 65536,
                                       .camera_x = cx + 7,
                                       .camera_y = cy - 3,
                                       .retained_camera_x = cx,
                                       .retained_camera_y = cy,
                                       .hand = hand,
                                       .family = 13,
                                       .role = 2,
                                       .priority = 148,
                                       .facing_left = left,
                                       .parent_hitbox_x = 5,
                                       .parent_hitbox_y = -6,
                                       .player_crouch_step = crouch,
                                       .free_factories = 8,
                                       .game_timer = 1};
        Uint32 rng = 7, check = 7;
        (void)SotnCombat_Random(&check);
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 1 &&
              rng == check && e.properties && e.clear_shield && e.use_ticks == 4 && !e.sounds[0] &&
              !e.factory_count && s.bank == 0x8010U + hand * 2U && s.texture == 0x64U + hand * 2U &&
              s.palette == (hand ? 0x128 : 0x110) && s.frame == 6 && s.priority == 150 &&
              s.flags == 0x04000000U && s.draw_flags == 4 && s.hitbox_width == 14 &&
              s.hitbox_height == 14 && s.heaven_volume == 0x60 && s.heaven_angle == -0x200 &&
              s.heaven_radius == 0x100 && s.role == 2);
        const Sint16 sx = (Sint16)((k == 4 ? 190 : 171) - cx + (left ? -5 : 5)),
                     sy = (Sint16)(120 - cy - 6 - (crouch ? 8 : 16));
        CHECK(s.x == (sx + cx) * 65536 + 0x1234 && s.y == (sy + cy) * 65536 + 0x4321 &&
              s.heaven_cx == sx * 65536 && s.heaven_cy == sy * 65536 &&
              s.heaven_target_x == sx + (left ? -0x78 : 0x78) && s.heaven_target_y == sy);
        // The sweep: angle -0x20 and radius +0x10 each update, seventy-three updates.
        Sint16 angle = -0x200;
        Sint32 radius = 0x100, volume = 0x60;
        Uint16 count = 0, updates = 0;
        while (s.phase == 1)
        {
            in.game_timer = 1U + updates;
            in.free_factories = (Uint8)(updates % 5 == 3 ? 0 : 8);
            const Sint16 rotate = s.rotate;
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
            SotnMathTrig t = {0};
            CHECK(SotnMath_Trig(x_exe, angle, &t));
            angle = (Sint16)(angle - 0x20);
            radius += 0x10;
            const Sint32 px = t.cosine * (left ? -radius : radius) + sx * 65536,
                         py = -radius * t.sine + sy * 65536;
            CHECK(s.rotate == (Sint16)(rotate + 0x100) && s.x == px + cx * 65536 &&
                  s.y == py + cy * 65536 && s.heaven_angle == angle && s.heaven_radius == radius);
            const bool strike = !(in.game_timer & 1U) && angle < -0x400;
            CHECK(e.factory_count == (strike && in.free_factories ? 1 : 0));
            if (e.factory_count)
            {
                const Uint16 offset = (Uint16)((count >> 1) % 14 < 8 ? (count >> 1) % 14 + 1U
                                                                     : 14U - (count >> 1) % 14);
                CHECK(e.factories[0] == (0x46U | (hand + 1U) << 14 | (Uint32)offset << 16) &&
                      e.factory_x[0] == s.x && e.factory_y[0] == s.y);
            }
            count = (Uint16)(count + strike);
            CHECK(s.heaven_count == (Sint16)count);
            if (in.game_timer % 6U == 0)
            {
                CHECK(e.sounds[0] == 0x60a && e.volume_sound && e.volume == volume);
                volume = volume >= 4 ? volume - 4 : 0;
            }
            else
                CHECK(!e.sounds[0]);
            CHECK(s.heaven_volume == volume);
            ++updates;
        }
        // Strikes from updates17..71 on even timers: angle -0x400 itself (update15) is not below.
        CHECK(updates == 73 && s.phase == 2 && s.heaven_timer == 8 && count == 28);
        for (Uint16 t = 0; t < 8; ++t)
        {
            CHECK(s.draw_flags == 4 && SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
            CHECK(s.phase == (t == 7 ? 3 : 2));
        }
        CHECK(s.draw_flags == 0x80 && !s.heaven_timer);
        const Sint32 before_x = s.x;
        for (Sint16 t = 1; t <= 0x34; ++t)
        {
            in.allocated = t == 0x34 && !starved;
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.heaven_timer == t);
            CHECK(e.sounds[0] == (t == 0x34 ? 0x66a : t < 0x20 && !(t & 3) ? 0x60b : 0));
            if (t <= 0x10)
                CHECK(s.x == before_x && s.hitbox_width == 14);
            else
                CHECK(((Uint32)s.x >> 16) == (Uint16)(s.heaven_target_x + cx) &&
                      ((Uint32)s.y >> 16) == (Uint16)(sy + cy) &&
                      (s.x & 0xffff) == (before_x & 0xffff) && s.hitbox_width == 0x12 &&
                      s.hitbox_height == 4);
            if (t < 0x34)
                CHECK(s.draw_flags == (t > 0x30 ? 0 : 0x80) && s.phase == 3);
        }
        if (starved)
        {
            CHECK(s.removed && s.phase == 3);
            continue;
        }
        const Sint16 tx = s.heaven_target_x;
        CHECK(s.phase == 4 && s.draw_flags == 3 && s.scale_x == 0x100 && s.scale_y == 0x100 &&
              s.palette == 0x815f && s.draw_mode == 0x31 && s.row_priority == 150 &&
              s.flags == 0x04800000U && s.heaven_line.x[0] == tx && s.heaven_line.x[1] == tx &&
              s.heaven_line.y[0] == sy && s.heaven_line.y[1] == sy &&
              s.heaven_line.red[0] == 0xf0 && s.heaven_line.blue[1] == 0xf0 &&
              s.heaven_line.green[0] == 0x80 && s.heaven_line.green[1] == 0x80 &&
              !s.heaven_line.hidden);
        // The slash: ends grow by sixteen to +-0x110, then blue fades by eight and green by
        // four until green reaches eight.
        Sint16 x0 = tx, x1 = tx, scale = 0x100;
        Uint8 blue = 0xf0, green = 0x80;
        Uint16 slash = 0;
        while (!s.removed)
        {
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
            ++slash;
            if (x0 > -0x110)
                x0 = (Sint16)(x0 - 0x10);
            if (x1 < 0x110)
                x1 = (Sint16)(x1 + 0x10);
            if (x0 < 0 && x1 > 0x100)
            {
                if (blue > 8)
                    blue = (Uint8)(blue - 8);
                if (green <= 8)
                {
                    CHECK(s.removed);
                    break;
                }
                green = (Uint8)(green - 4);
            }
            scale = (Sint16)(scale + 0x80);
            const Sint16 hx = tx >= 0x101 ? (Sint16)(0x100 - tx) : tx < 0 ? (Sint16)-tx : 0;
            CHECK(!s.removed && s.heaven_line.x[0] == x0 && s.heaven_line.x[1] == x1 &&
                  s.heaven_line.blue[1] == (x0 < 0 && x1 > 0x100 ? blue : 0xf0) &&
                  s.heaven_line.green[1] == green && s.scale_x == scale && s.scale_y == 0x20 &&
                  s.frame == (scale >= 0x700 ? 0 : 6) && s.hitbox_width == 0xfe &&
                  s.hitbox_height == 2 && s.hitbox_x == (left ? -hx : hx));
            if (x0 < 0 && x1 > 0x100)
                CHECK(s.heaven_line.red[0] == blue && s.heaven_line.blue[0] == blue &&
                      s.heaven_line.red[1] == blue && s.heaven_line.green[0] == green);
        }
        CHECK(slash > 40 && green <= 8 && x0 <= -0x110);
        if (!left && hand == 0 && !crouch)
            CHECK(tx == 256 && x0 == -0x110);
        // Hand1's target lies past the right edge: the slash hitbox clamps back to 0x100.
        if (k == 4)
            CHECK(tx == 275 && s.hitbox_x == 0x100 - 275);
    }
    // Strike swords: an empty raw parent retires them, birth copies the sweep, one rand picks
    // the1..32 countdown that starts after the sweep reaches step3.
    {
        SotnWeaponSecondary orphan = {.parameter = 0x300};
        SotnWeaponSecondaryEvents e = {0};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = x_exe,
                                       .properties = Properties,
                                       .context = (void *)&hit,
                                       .family = 13,
                                       .role = 3};
        Uint32 rng = 5;
        CHECK(SotnWeaponSecondary_Tick(&orphan, &program, in, &rng, &e) && orphan.removed &&
              !orphan.phase && rng == 5);
    }
    Sint16 longest = 0;
    for (Uint16 k = 0; k < 6; ++k)
    {
        const bool left = (k & 1U) != 0, level = k >= 4;
        const Sint32 cx = 24, cy = 8;
        SotnWeaponSecondary s = {
            .parameter = 0x500, .x = (200 + cx) * 65536 + 0x8000, .y = (90 + cy) * 65536 + 0x4000};
        SotnWeaponSecondaryEvents e = {0};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = x_exe,
                                       .properties = Properties,
                                       .context = (void *)&hit,
                                       .camera_x = cx + 7,
                                       .camera_y = cy - 3,
                                       .retained_camera_x = cx,
                                       .retained_camera_y = cy,
                                       .family = 13,
                                       .role = 3,
                                       .parent_exists = true,
                                       .parent_facing_left = left,
                                       .parent_rotate = 0x700,
                                       .parent_draw_flags = 4,
                                       .parent_frame = 6,
                                       .parent_bank = 0x8010,
                                       .parent_texture = 0x64,
                                       .parent_palette = 0x110,
                                       .parent_priority = 150,
                                       .parent_target_x = (Sint16)(level  ? (left ? 104 : 296)
                                                                   : left ? 80
                                                                          : 300),
                                       .parent_target_y = (Sint16)(level ? 90 : 60),
                                       .parent_phase = 1};
        Uint32 rng = 11U + k * 7U, check = rng;
        const Sint16 delay = (Sint16)((SotnCombat_Random(&check) & 0x1fU) + 1U);
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == check &&
              s.phase == 1 && s.frame == 6 && s.bank == 0x8010 && s.texture == 0x64 &&
              s.facing_left == left && s.priority == 150 && s.flags == 0x04000000U &&
              s.palette == 0x115 && s.draw_flags == 0x84 && s.rotate == 0x700 &&
              s.heaven_target_x == in.parent_target_x && s.heaven_target_y == in.parent_target_y &&
              s.heaven_timer == delay && !e.properties && s.role == 3);
        longest = SDL_max(longest, delay);
        for (Uint16 t = 0; t < 5; ++t)
        {
            in.parent_phase = (Uint8)(t < 3 ? 1 : 2);
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 1);
        }
        in.parent_phase = 3;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 2 &&
              s.heaven_timer == delay);
        in.parent_phase = 4;
        for (Sint16 t = 1; t < delay; ++t)
        {
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 2 &&
                  !e.properties && s.rotate == 0x700);
        }
        e = (SotnWeaponSecondaryEvents){0};
        check = rng;
        (void)SotnCombat_Random(&check);
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 3 &&
              e.properties && rng == check && !s.rotate && s.hitbox_width == 0x12 &&
              s.hitbox_height == 4 && s.x == (200 + cx) * 65536 + 0x8000);
        Sint16 heading = 0;
        CHECK(
            SotnMath_Atan2(x_exe, -(in.parent_target_y - 90), in.parent_target_x - 200, &heading));
        SotnMathTrig t = {0};
        CHECK(SotnMath_Trig(x_exe, heading, &t) && s.velocity_x == t.cosine * 0x100 &&
              s.heaven_vy == -t.sine * 0x100);
        Uint16 flight = 0;
        Sint32 x = s.x, y = s.y;
        while (!s.removed)
        {
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
            x += s.velocity_x;
            y += s.heaven_vy;
            CHECK(s.x == x && s.y == y);
            ++flight;
            const Sint16 px = (Sint16)((x >> 16) - cx);
            CHECK(s.removed == (left ? px < in.parent_target_x : px > in.parent_target_x));
        }
        // Level flights move sixteen pixels per update: they pass over the target column
        // (6x16 = 96) and retire one update later.
        CHECK(flight > 3 && flight < 16 && (!level || flight == 7));
        // The empty slot retires a strike at any step.
        in.parent_exists = false;
        SotnWeaponSecondary late = {
            .parameter = 0x200, .phase = 2, .heaven_timer = 3, .allocated = true};
        CHECK(SotnWeaponSecondary_Tick(&late, &program, in, &rng, &e) && late.removed &&
              late.heaven_timer == 3);
    }
    CHECK(longest > 16);
    return true;
}
static bool HeavenRuntime(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(56, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    static Uint8 exe[0x24000];
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Math(exe);
    FamilyDefinition(dra, 119, 2, 13, 129, 0, 0, 0);
    Uint8 *record = dra + 0x500 + 119 * 52;
    record[18] = 0x5d;
    record[19] = 50;
    record[21] = 0;
    record[22] = 1;
    const Uint8 blueprints[][6] = {{50, 0, 1, 0xc1, 0, 0x26}, {0x3a, 1, 1, 0xc1, 0, 0}};
    for (Uint16 i = 0; i < 2; ++i)
        SDL_memcpy(dra + 0xd1d4 + blueprints[i][0] * 6U, blueprints[i] + 1, 5);
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    for (Uint16 i = 0; i < 14; ++i)
        Put16(weapons + 13U * 0x7000 + 0x4000 + 0x5e0 + i * 2U, (Uint16)(i % 7U));
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    SotnEquipment heaven = {0};
    CHECK(SotnEquipment_Read(actors->dra, 119, &heaven) && heaven.kind == 129 &&
          heaven.family == 13 && heaven.blueprint == 50 && !heaven.consumable);
    CHECK(SotnCharacter_Create(actors->dra, &character));
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = {exe, sizeof(exe)},
                              .weapons = {weapons, 56U * 0x7000},
                              .weapon_family = {13, 13},
                              .weapon_bound = {true, true},
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 128 * 65536,
                                         .y = 100 * 65536,
                                         .priority = 148,
                                         .animation = {.id = 0x5d,
                                                       .hitbox_x = 4,
                                                       .hitbox_y = -18,
                                                       .hitbox_width = 12,
                                                       .hitbox_height = 30}}};
    const SotnPlayerFactorySource source = {.x = in.player.x, .y = in.player.y, .priority = 148};
    bool allocated = false;
    CHECK(SotnBatRuntime_RequestThrown(r, source, 0x1032U, 119, &allocated) && allocated);
    Uint16 root = 0, images = 0, faded = 0, births = 0, ticks = 0;
    for (Uint16 t = 0; t < 300; ++t)
    {
        in.game_timer = t;
        ticks += r->children[root].weapon_secondary && root;
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        for (Uint16 i = 1; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        {
            const SotnBatRuntimeChild *c = &r->children[i];
            if (!c->weapon_secondary || c->state.secondary.family != 13)
                continue;
            CHECK(SotnBatRuntime_SecondaryValid(c, i) && !c->primitive_count);
            if (c->kind == 0xe0)
            {
                CHECK(i <= 15 && c->state.secondary.own_item && c->state.secondary.item == 119 &&
                      c->group >= 3 && c->group <= 6);
                root = i;
            }
            else if (c->initialized)
            {
                // Afterimages carry the root's childPalette (palette0x110 plus the cycle).
                CHECK(c->kind == 0xe1 && i >= 16 && c->state.secondary.parent_slot == root + 16U &&
                      !c->state.secondary.item && !c->state.secondary.attack.active &&
                      (c->state.secondary.motion.draw_flags & 8U) &&
                      c->state.secondary.motion.palette >= 0x110 &&
                      c->state.secondary.motion.palette <= 0x116);
                births += c->state.secondary.motion.opacity == 0x78;
                ++images;
                faded += c->state.secondary.motion.opacity < 0x80;
            }
        }
    }
    // One afterimage per even g_GameTimer during the rise and homing, never every update.
    CHECK(root && images > 20 && faded == images && births * 2 <= ticks + 2 && births > 10);
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        CHECK(!r->children[i].weapon_secondary && !r->children[i].primitive_count);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}

// w_007 func_ptr_80170004/80170008: Shield rod special173's orb launches from its D_8017AC48
// origin, decelerates while growing, bobs for 0x200 updates shedding a sparkle every eighth, and
// shrinks out; a hit on the growth's last update strands it in step4. Sparkles take one GT4
// before two rand calls, then sink through six UV frames.
static void RodTables(Uint8 *o)
{
    const Uint16 origins[14] = {0x28, 0xfff2, 0x28,   0xfff2, 0x28,   4,    0x24,
                                0x14, 0x28,   0xfff2, 0x28,   0xfff3, 0x28, 0x10};
    for (Uint16 i = 0; i < 14; ++i)
        Put16(o + 0xc48 + i * 2U, origins[i]);
    for (Uint16 i = 0; i < 48; ++i)
        o[0xc64 + i] = (Uint8)(0x10 + i);
    for (Uint16 i = 0; i < 4; ++i)
    {
        Put16(o + 0xb44 + i * 4U, 4);
        Put16(o + 0xb46 + i * 4U, (Uint16)(0x243d + i));
    }
    Put16(o + 0xb54, 0);
    Put16(o + 0xb56, 0);
}
static bool RodOrbTest(void)
{
    static Uint8 o[0x3000], exe[0x24000];
    Math(exe);
    RodTables(o);
    SotnHandWeaponProgram program = {0};
    CHECK(SotnWeaponSecondary_Supported(7) && SotnWeaponSecondary_SupportedRole(7, 1) &&
          SotnWeaponSecondary_SupportedRole(7, 2) && !SotnWeaponSecondary_SupportedRole(7, 0) &&
          !SotnWeaponSecondary_SupportedRole(7, 3) && !SotnWeaponSecondary_RoleRows(7, 1) &&
          SotnWeaponSecondary_RoleRows(7, 2) == 1);
    {
        SotnWeaponSecondary orphan = {.parameter = 0x100};
        SotnWeaponSecondaryEvents e = {0};
        Uint32 rng = 3;
        CHECK(SotnWeaponSecondary_Tick(&orphan, &program,
                                       (SotnWeaponSecondaryInput){.overlay = {o, sizeof(o)},
                                                                  .executable = {exe, sizeof(exe)},
                                                                  .family = 7,
                                                                  .role = 1},
                                       &rng, &e) &&
              orphan.removed && !e.group && !e.sounds[0] && rng == 3);
    }
    for (Uint16 k = 0; k < 6; ++k)
    {
        const Uint16 anim = k < 2 ? 0 : k < 4 ? 3 : 6, hand = k & 1U;
        const bool left = k == 1 || k == 2 || k == 5, low = anim == 3 || anim == 6, strand = k == 4;
        SotnWeaponSecondary s = {
            .parameter = (Uint16)(anim << 8), .x = 100 * 65536 + 0x1234, .y = 80 * 65536 + 0x4321};
        SotnWeaponSecondaryEvents e = {0};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = {exe, sizeof(exe)},
                                       .hand = hand,
                                       .family = 7,
                                       .role = 1,
                                       .parent_exists = true,
                                       .parent_facing_left = left,
                                       .parent_bank = (Uint16)(0x8010U + hand * 2U),
                                       .parent_texture = (Uint16)(0x64U + hand * 2U),
                                       .parent_palette = (Uint16)(0x11fU + hand * 24U)};
        Uint32 rng = 5;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == 5 && e.group &&
              e.sounds[0] == 0x6ad && !e.properties && s.phase == 1 && s.role == 1 &&
              s.facing_left == left && s.bank == 0x8010U + hand * 2U &&
              s.texture == 0x64U + hand * 2U && s.palette == 0x11fU + hand * 24U &&
              s.priority == 0xa8 && s.flags == 0x08100000U && s.draw_flags == 3 &&
              s.scale_x == 0x10 && s.scale_y == 0x10 && s.hitbox_width == 9 &&
              s.hitbox_height == 9 && s.hitbox_state == 4 && s.properties.attack == 1 &&
              s.properties.hit_type == 4 && s.properties.invincibility == 4 &&
              s.properties.stun == 4 && s.properties.hit_effect == 1 && !s.properties.id);
        const Sint16 ox = (Sint16)(anim == 0   ? 0x28
                                   : anim == 3 ? 0x24
                                               : 0x28),
                     oy = (Sint16)(anim == 0   ? -14
                                   : anim == 3 ? 0x14
                                               : 0x10);
        const Sint32 speed = low ? 0x3c000 : 0x4c000;
        CHECK(s.x == (100 + (left ? -ox : ox)) * 65536 + 0x1234 &&
              s.y == (80 + oy) * 65536 + 0x4321 && s.velocity_x == (left ? -speed : speed) &&
              s.velocity_y == (low ? 0x2c000 : 0));
        CHECK(SotnWeaponSecondary_Animate(&s, (SotnDataView){o, sizeof(o)}, 7) && s.frame == 0x3d &&
              s.remaining == 4);
        // Growth: both velocities decelerate by 7/32 toward zero, scale +0x10 up to 0x100.
        Uint16 updates = 0;
        while (s.phase == 1)
        {
            const Sint32 vx = s.velocity_x, vy = s.velocity_y, x = s.x, y = s.y;
            const Sint16 scale = s.scale_x;
            const Sint32 nvx = vx < 0 ? SDL_min(vx + 0x3800, 0) : SDL_max(vx - 0x3800, 0),
                         nvy = vy < 0 ? SDL_min(vy + 0x3800, 0) : SDL_max(vy - 0x3800, 0);
            s.hit_flags = (Uint8)(strand && !nvx && scale + 0x10 >= 0x100 ? 2 : 0);
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && !e.chain);
            ++updates;
            CHECK(s.velocity_x == nvx && s.velocity_y == nvy && s.x == x + nvx && s.y == y + nvy &&
                  s.scale_x == (scale < 0x100 ? scale + 0x10 : scale) && s.scale_y == s.scale_x);
        }
        CHECK(updates == (low ? 18 : 22) && s.orb_countdown == 0x200);
        if (strand)
        {
            // The hit set step3 before step++: the original's unhandled step4 holds it.
            s.hit_flags = 0;
            const SotnWeaponSecondary held = s;
            CHECK(s.phase == 4 && SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) &&
                  !SDL_memcmp(&s, &held, sizeof(s)));
            continue;
        }
        CHECK(s.phase == 2);
        // Bobbing: velocityY = rsin(wave) added twice, wave +0x20, a sparkle every eighth.
        Uint16 sparkles = 0, shrink = 0;
        const bool hit = k == 5;
        for (Uint16 t = 0; !s.removed && t < 0x300; ++t)
        {
            const Sint16 wave = s.orb_wave, count = s.orb_countdown, scale = s.scale_x;
            const Uint8 phase = s.phase;
            const Sint32 y = s.y;
            SotnMathTrig trig = {0};
            CHECK(SotnMath_Trig((SotnDataView){exe, sizeof(exe)}, wave, &trig));
            s.hit_flags = (Uint8)(hit && t == 40 ? 1 : 0);
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
            if (phase == 3 && scale - 0xc < 0x10)
            {
                CHECK(s.removed && !e.chain);
                break;
            }
            CHECK(s.velocity_y == trig.sine && s.y == y + 2 * trig.sine &&
                  s.orb_wave == (Sint16)(wave + 0x20) && s.orb_countdown == count - 1 &&
                  e.chain == (!(count & 7) ? 0x40U | (hand + 1U) << 12 : 0U));
            sparkles += e.chain != 0;
            if (phase == 3)
            {
                CHECK(s.scale_x == scale - 0xc);
                ++shrink;
            }
            CHECK(s.phase == (phase == 3 || count == 1 || (hit && t == 40) ? 3 : 2));
        }
        // 0x200..1 (64 sparkles) then 0..-19 while shrinking (0, -8, -16); a hit after update40
        // leaves six, then 0x1D7..0x1C4 (0x1D0, 0x1C8).
        CHECK(s.removed && shrink == 20 && sparkles == (hit ? 6 + 2 : 0x200 / 8 + 3));
    }
    return true;
}
static bool RodSparkleTest(void)
{
    static Uint8 o[0x3000], exe[0x24000];
    RodTables(o);
    SotnHandWeaponProgram program = {0};
    SotnWeaponSecondaryEvents e = {0};
    SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                   .executable = {exe, sizeof(exe)},
                                   .family = 7,
                                   .role = 2,
                                   .camera_x = 50,
                                   .camera_y = 9,
                                   .retained_camera_x = 40,
                                   .retained_camera_y = 4};
    {
        SotnWeaponSecondary s = {.x = 140 * 65536, .y = 90 * 65536, .priority = 0xa8};
        Uint32 rng = 21;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.removed && rng == 21);
    }
    in.allocated = true;
    for (Uint16 k = 0; k < 3; ++k)
    {
        SotnWeaponSecondary s = {.x = 140 * 65536 + 0x77, .y = 90 * 65536 + 0x55, .priority = 0xa8};
        Uint32 rng = 21U + k * 1000U, check = rng;
        const Sint32 dx = (Sint32)((SotnCombat_Random(&check) & 0x7fffU) % 16U) - 8,
                     dy = (Sint32)((SotnCombat_Random(&check) & 0x7fffU) % 16U);
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == check &&
              s.phase == 1 && s.flags == 0x08800000U && s.quad_velocity_y == 0xc000 &&
              s.x == (140 + dx) * 65536 + 0x77 && s.y == (90 + dy) * 65536 + 0x55 &&
              s.quads[0].page == 0x1a && s.quads[0].palette == 0x1b0 &&
              s.quads[0].priority == 0xac && s.quads[0].draw_mode == 0x31);
        Uint16 calls = 1;
        for (;;)
        {
            const Uint8 frame = (Uint8)((calls - 1U) / 2U);
            const Sint16 size = frame >= 3 ? 2 : 3, sx = (Sint16)((s.x >> 16) - 40),
                         sy = (Sint16)((s.y >> 16) - 4);
            CHECK(s.quads[0].x[0] == sx - size && s.quads[0].x[3] == sx + size &&
                  s.quads[0].y[1] == sy - size && s.quads[0].y[2] == sy + size &&
                  s.quads[0].u[0] == 0x10 + frame * 8 && s.quads[0].v[3] == 0x10 + frame * 8 + 7 &&
                  s.quad_timer[0] == calls && s.quad_phase[0] == calls / 2U);
            const Sint32 y = s.y;
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == check &&
                  s.y == y + 0xc000);
            ++calls;
            if (s.removed)
                break;
        }
        CHECK(calls == 13);
    }
    return true;
}
// Shield rod special173 through the shared runtime: slot16's descriptor1 requests the 0x4A orb
// (slots28..31, raw parent slot16, no equip id, least-used group, hit type4) whose 0x40 sparkles
// (slots32..63) each lease one row before their two rand calls; all retire with their rows.
static bool RodRuntime(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(56, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    static Uint8 exe[0x24000];
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Math(exe);
    FamilyDefinition(dra, 173, 4, 7, 2, 1, 0, 0);
    Uint8 *record = dra + 0x500 + 173 * 52;
    record[18] = 0x48;
    record[21] = 13;
    const Uint8 blueprints[][6] = {
        {48, 0, 1, 0xc1, 0, 0x23}, {0x4a, 1, 1, 0xc1, 0, 0x24}, {0x40, 2, 1, 0xc1, 0, 0}};
    for (Uint16 i = 0; i < 3; ++i)
        SDL_memcpy(dra + 0xd1d4 + blueprints[i][0] * 6U, blueprints[i] + 1, 5);
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    dra[0xd4b8 + 6] = dra[0xd4b9 + 6] = 16;
    dra[0xd4b8 + 8] = 28;
    dra[0xd4b9 + 8] = 31;
    Uint8 *o = weapons + 7U * 0x7000 + 0x4000;
    Program(o);
    SDL_memcpy(o + 0xc28, o + 0x10ac, 16);
    SDL_memcpy(o + 0xc38, o + 0x10ac, 16);
    o[0xc28 + 12] = o[0xc38 + 12] = 0x48;
    RodTables(o);
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    SotnEquipment rod = {0};
    CHECK(SotnEquipment_Read(actors->dra, 173, &rod) && rod.family == 7 && rod.kind == 2 &&
          rod.variant == 1);
    CHECK(SotnCharacter_Create(actors->dra, &character));
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = {exe, sizeof(exe)},
                              .weapons = {weapons, 56U * 0x7000},
                              .weapon_family = {7, 7},
                              .weapon_bound = {true, true},
                              .camera_x = 20,
                              .camera_y = 8,
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 128 * 65536,
                                         .y = 100 * 65536,
                                         .priority = 148,
                                         .attack_mode = 0x801d,
                                         .animation = {.id = 0x4e, .pose = 4, .remaining = 7}}};
    const SotnPlayerFactorySource source = {.x = in.player.x, .y = in.player.y, .priority = 148};
    bool allocated = false;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestHand(r, source, 0x11030, 173, &allocated) && allocated &&
          SotnBatRuntime_Tick(r, character, in) && r->children[0].hand_weapon &&
          SotnActors_CommitBatPrimitives(actors, r));
    Uint16 pending = 0;
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        if (r->factories[i].secondary_factory)
        {
            ++pending;
            CHECK(r->factories[i].cursor.blueprint == 0x4a &&
                  r->factories[i].cursor.entity_modifier == 0xe0 &&
                  r->factories[i].secondary_parent_slot == 16);
        }
    CHECK(pending == 1);
    in.player.animation.remaining = 6;
    Uint16 orb = 0, sparkles = 0, sparkle_births = 0, live_max = 0, ticks = 0;
    bool born[SOTN_BAT_RUNTIME_SLOTS] = {0};
    for (Uint16 t = 0; t < 900; ++t)
    {
        // The swing ends: slot16 retires, the orb keeps its birth copies.
        if (t == 3)
            in.player.attack_mode = 0;
        in.game_timer = t;
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        ++ticks;
        Uint16 live = 0;
        bool any = false;
        for (Uint16 i = 1; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        {
            const SotnBatRuntimeChild *c = &r->children[i];
            if (!c->weapon_secondary || c->state.secondary.family != 7 || !c->initialized)
            {
                born[i] = false;
                continue;
            }
            any = true;
            const SotnWeaponSecondary *w = &c->state.secondary.motion;
            CHECK(SotnBatRuntime_SecondaryValid(c, i) && !c->state.secondary.item);
            if (c->kind == 0xe1)
            {
                CHECK(i >= 12 && i <= 15 && c->state.secondary.parent_slot == 16 &&
                      c->parameter == 0x600 && c->group >= 3 && c->group <= 6 &&
                      !c->primitive_count && c->state.secondary.attack.active &&
                      c->state.secondary.attack.hit_type == 4 &&
                      c->state.secondary.attack.power == 1 && w->priority == 0xa8 &&
                      w->palette == 0x110U + Read16Test(o + 0xc38 + 8) && w->frame >= 0x3d &&
                      w->frame <= 0x40);
                orb = i;
            }
            else
            {
                CHECK(c->kind == 0xe2 && i >= 16 && c->state.secondary.parent_slot == orb + 16U &&
                      !c->parameter && c->primitive_count == 1 && !c->group &&
                      !c->state.secondary.attack.active && w->quads[0].priority == 0xac);
                sparkle_births += !born[i];
                born[i] = true;
                ++live;
                ++sparkles;
            }
        }
        live_max = SDL_max(live_max, live);
        if (!any && t > 10)
            break;
    }
    // One sparkle every eighth countdown, each living twelve updates: at most two at once.
    CHECK(orb && sparkle_births == 67 && live_max == 2 && sparkles == 67 * 12 && ticks < 900);
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        CHECK(!r->children[i].weapon_secondary && !r->children[i].primitive_count);
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        CHECK(!actors->primitives[i].owner);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}
// Combo202 through the shared runtime: blueprint62's E2 sweep (own item, least-used group, no
// rows) sheds blueprint0x46 strike swords whose raw parent is its slot; they launch with its
// item/enemyId, the sweep leases one real row only at step4, and both keep their screen
// positions while the camera scrolls.
static bool HeavenComboRuntime(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(56, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    static Uint8 exe[0x24000];
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Math(exe);
    FamilyDefinition(dra, 202, 2, 13, 5, 0, 5, 0);
    Uint8 *record = dra + 0x500 + 202 * 52;
    record[18] = 0x66;
    record[19] = 62;
    record[21] = 0;
    record[22] = 1;
    const Uint8 blueprints[][6] = {{62, 2, 1, 0xc1, 0, 6}, {0x46, 3, 1, 0xc1, 0, 1}};
    for (Uint16 i = 0; i < 2; ++i)
        SDL_memcpy(dra + 0xd1d4 + blueprints[i][0] * 6U, blueprints[i] + 1, 5);
    dra[0xd4b8 + 2] = 17;
    dra[0xd4b9 + 2] = 47;
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    for (Uint16 i = 0; i < 14; ++i)
        Put16(weapons + 13U * 0x7000 + 0x4000 + 0x5e0 + i * 2U, (Uint16)(i < 8 ? i + 1U : 14U - i));
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    SotnEquipment combo = {0};
    CHECK(SotnEquipment_Read(actors->dra, 202, &combo) && combo.kind == 5 && combo.family == 13 &&
          combo.blueprint == 62 && combo.chain_limit == 1);
    CHECK(SotnCharacter_Create(actors->dra, &character));
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = {exe, sizeof(exe)},
                              .weapons = {weapons, 56U * 0x7000},
                              .weapon_family = {13, 13},
                              .weapon_bound = {true, true},
                              .camera_x = 32,
                              .camera_y = 16,
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 160 * 65536,
                                         .y = 120 * 65536,
                                         .priority = 148,
                                         .animation = {.id = 0x66,
                                                       .hitbox_x = 4,
                                                       .hitbox_y = -18,
                                                       .hitbox_width = 12,
                                                       .hitbox_height = 30}}};
    const SotnPlayerFactorySource source = {.x = in.player.x, .y = in.player.y, .priority = 148};
    bool allocated = false;
    CHECK(!SotnBatRuntime_RequestHand(r, source, 0x103eU, 202, &allocated));
    CHECK(SotnBatRuntime_RequestHand(r, source, 0x403eU, 202, &allocated) && allocated);
    Uint16 sweep = 0, waiting = 0, launched = 0, lined = 0, scrolls = 0, checked = 0;
    Uint8 sweep_group = 0;
    bool born[SOTN_BAT_RUNTIME_SLOTS] = {0};
    Sint32 strike_x[SOTN_BAT_RUNTIME_SLOTS] = {0};
    Uint8 sweep_phase = 0;
    Uint16 retained = 0;
    for (Uint16 t = 0; t < 600; ++t)
    {
        in.game_timer = t;
        // Every eighth update the camera moves right by two after the callbacks.
        in.screen_shift_x = (Sint16)(t % 8 == 7 ? -2 : 0);
        in.camera_x += t % 8 == 7 ? 2 : 0;
        scrolls += t % 8 == 7;
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        bool live = false;
        for (Uint16 i = 1; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        {
            const SotnBatRuntimeChild *c = &r->children[i];
            if (!c->weapon_secondary || c->state.secondary.family != 13 || !c->initialized)
            {
                born[i] = false;
                continue;
            }
            live = true;
            const SotnWeaponSecondary *w = &c->state.secondary.motion;
            CHECK(SotnBatRuntime_SecondaryValid(c, i));
            const Sint32 screen_x = (Sint32)(Sint16)((Uint32)w->x >> 16) - in.camera_x;
            if (c->kind == 0xe2)
            {
                CHECK(i <= 15 && c->state.secondary.own_item && c->state.secondary.item == 202 &&
                      c->group >= 3 && c->group <= 6 && !c->parameter &&
                      c->primitive_count == (w->phase == 4 ? 1 : 0));
                if (!sweep)
                    CHECK(w->heaven_cx == (160 - 32 + 4) * 65536 &&
                          w->heaven_cy == (120 - 16 - 18 - 16) * 65536);
                sweep = i;
                sweep_group = c->group;
                sweep_phase = w->phase;
                SotnBatRuntimeChild altered = *c;
                altered.state.secondary.item = 119;
                altered.state.secondary.motion.properties.id = 119;
                CHECK(!SotnBatRuntime_SecondaryValid(&altered, i));
                lined += w->phase == 4;
                // The screen position follows the program's screen words, whatever the scroll.
                if (w->phase == 1 && w->heaven_angle < -0x220)
                {
                    SotnMathTrig trig = {0};
                    CHECK(SotnMath_Trig((SotnDataView){exe, sizeof(exe)},
                                        (Sint16)(w->heaven_angle + 0x20), &trig));
                    CHECK(w->x - in.camera_x * 65536 ==
                          trig.cosine * w->heaven_radius + w->heaven_cx);
                    ++checked;
                }
                if (w->phase == 3 && w->heaven_timer > 0x10)
                    CHECK(screen_x == w->heaven_target_x);
            }
            else
            {
                CHECK(c->kind == 0xe3 && i <= 31 && c->state.secondary.parent_slot == sweep + 16U &&
                      (c->parameter & 0x80ffU) == 0 && (c->parameter >> 8) >= 1 &&
                      (c->parameter >> 8) <= 8 && !c->primitive_count &&
                      w->palette == 0x110U + (c->parameter >> 8) && (w->draw_flags & 0x80U));
                // Strikes leave their wait only once the raw parent sweep reaches step3.
                CHECK(w->phase <= 1 || sweep_phase >= 3);
                // In flight they move by their velocity plus the retained-screen shift.
                if (w->phase == 3 && born[i] && strike_x[i])
                {
                    CHECK(w->x ==
                          strike_x[i] + w->velocity_x + (in.screen_shift_x ? 2 * 65536 : 0));
                    retained += in.screen_shift_x != 0;
                }
                strike_x[i] = w->x;
                if (w->phase == 3)
                {
                    CHECK(c->state.secondary.item == 202 && c->group == sweep_group &&
                          c->state.secondary.attack.active && w->hitbox_width == 0x12);
                    launched += !born[i];
                    born[i] = true;
                }
                else
                {
                    CHECK(!c->state.secondary.item && !c->group &&
                          !c->state.secondary.attack.active);
                    ++waiting;
                }
            }
        }
        if (!live && sweep && t > 10)
            break;
    }
    CHECK(sweep && lined > 20 && launched >= 20 && waiting > 100 && checked > 60 && scrolls > 20 &&
          retained > 0);
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        CHECK(!r->children[i].weapon_secondary && !r->children[i].primitive_count);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}

// Blueprint52 releases Crissaegrim's four slashes one every four updates (index0..3, one GT4
// each, own item164); blueprint56's Chakram (own item133) spawns blueprint0x40 afterimages in
// slots32..63 naming it; everything retires with its rows.
static bool TwelveRuntime(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(56, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    static Uint8 exe[0x24000];
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Math(exe);
    FamilyDefinition(dra, 164, 1, 12, 129, 2, 0, 0);
    Uint8 *record = dra + 0x500 + 164 * 52;
    record[18] = 0x5d;
    record[19] = 52;
    record[21] = 0;
    record[22] = 3;
    FamilyDefinition(dra, 133, 2, 12, 130, 0, 0, 0);
    record = dra + 0x500 + 133 * 52;
    record[18] = 0x60;
    record[19] = 56;
    record[21] = 0;
    record[22] = 2;
    const Uint8 blueprints[][6] = {
        {52, 0, 4, 0xc1, 4, 0x26}, {56, 1, 1, 0xc1, 0, 0x06}, {0x40, 2, 1, 0xc1, 0, 0}};
    for (Uint16 i = 0; i < 3; ++i)
        SDL_memcpy(dra + 0xd1d4 + blueprints[i][0] * 6U, blueprints[i] + 1, 5);
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    SotnEquipment e = {0};
    CHECK(SotnEquipment_Read(actors->dra, 164, &e) && e.blueprint == 52 && e.variant == 2 &&
          e.chain_limit == 3 && !e.consumable && SotnEquipment_Read(actors->dra, 133, &e) &&
          e.kind == 130 && e.blueprint == 56 && e.chain_limit == 2 && !e.consumable);
    CHECK(SotnCharacter_Create(actors->dra, &character));
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = {exe, sizeof(exe)},
                              .weapons = {weapons, 56U * 0x7000},
                              .weapon_family = {12, 12},
                              .weapon_bound = {true, true},
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 128 * 65536,
                                         .y = 100 * 65536,
                                         .priority = 148,
                                         .animation = {.id = 0x5d,
                                                       .hitbox_x = 4,
                                                       .hitbox_y = -18,
                                                       .hitbox_width = 12,
                                                       .hitbox_height = 30}}};
    const SotnPlayerFactorySource source = {.x = in.player.x, .y = in.player.y, .priority = 148};
    bool allocated = false;
    CHECK(!SotnBatRuntime_RequestThrown(r, source, 0x1032U | 2U << 16, 164, &allocated) &&
          SotnBatRuntime_RequestThrown(r, source, 0x1034U | 2U << 16, 164, &allocated) &&
          allocated);
    Uint16 seen = 0, startups = 0;
    for (Uint16 t = 0; t < 40; ++t)
    {
        in.game_timer = t;
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        startups += r->startup_ticks == 4;
        for (Uint16 i = 1; i < 16; ++i)
        {
            const SotnBatRuntimeChild *c = &r->children[i];
            if (!c->weapon_secondary || c->state.secondary.family != 12)
                continue;
            CHECK(SotnBatRuntime_SecondaryValid(c, i) && c->kind == 0xe0 &&
                  c->state.secondary.item == 164 && c->primitive_count == 1 &&
                  (c->parameter & 0x7f00U) == 0x200 && c->group >= 3 && c->group <= 6);
            seen |= (Uint16)(1U << (c->parameter & 3U));
        }
    }
    CHECK(seen == 15 && startups == 4 && r->hand_program[0].secondary_counter == 4);
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        CHECK(!r->children[i].weapon_secondary && !r->children[i].primitive_count);
    // The Chakram and its afterimages.
    CHECK(SotnBatRuntime_RequestThrown(r, source, 0x2038U | 0x80U << 16, 133, &allocated) &&
          allocated);
    Uint16 images = 0, chakram = 0;
    for (Uint16 t = 0; t < 300; ++t)
    {
        in.game_timer = t;
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        for (Uint16 i = 1; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        {
            const SotnBatRuntimeChild *c = &r->children[i];
            if (!c->weapon_secondary || c->state.secondary.family != 12 || !c->initialized)
                continue;
            CHECK(SotnBatRuntime_SecondaryValid(c, i));
            if (c->kind == 0xf1)
            {
                CHECK(i <= 15 && c->state.secondary.item == 133 && !c->primitive_count);
                chakram = i;
            }
            else
            {
                CHECK(c->kind == 0xf2 && i >= 16 &&
                      c->state.secondary.parent_slot == chakram + 16U &&
                      c->state.secondary.motion.palette == 0x100 && !c->state.secondary.item &&
                      (c->state.secondary.motion.draw_flags & 8U));
                ++images;
            }
        }
    }
    CHECK(chakram && images > 10);
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        CHECK(!r->children[i].weapon_secondary && !r->children[i].primitive_count);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}

// Blueprint68 creates Yasutsuna's E3 root (own item165, enemyId0 from the player entity, one
// GT4); it chains nine blueprint56 E1 slashes naming it, and all retire with their rows.
static bool YasutsunaRuntime(Uint16 hand)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(56, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    static Uint8 exe[0x24000];
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Math(exe);
    SlashProgram(weapons + 53U * 0x7000 + 0x4000);
    FamilyDefinition(dra, 165, 5, 53, 129, 0, 0, 0);
    Uint8 *record = dra + 0x500 + 165 * 52;
    record[18] = 0x5d;
    record[19] = 68;
    record[21] = 0;
    record[22] = 1;
    const Uint8 blueprints[][6] = {{68, 3, 1, 0xc1, 0, 0x06}, {56, 1, 1, 0xc1, 0, 0x06}};
    for (Uint16 i = 0; i < 2; ++i)
        SDL_memcpy(dra + 0xd1d4 + blueprints[i][0] * 6U, blueprints[i] + 1, 5);
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    SotnEquipment e = {0};
    CHECK(SotnEquipment_Read(actors->dra, 165, &e) && e.blueprint == 68 && e.family == 53 &&
          !e.consumable && SotnEquipment_IsTwoHand(165));
    CHECK(SotnCharacter_Create(actors->dra, &character));
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = {exe, sizeof(exe)},
                              .weapons = {weapons, 56U * 0x7000},
                              .weapon_family = {53, 53},
                              .weapon_bound = {true, true},
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 128 * 65536,
                                         .y = 100 * 65536,
                                         .priority = 148,
                                         .animation = {.id = 0x5d}}};
    const SotnPlayerFactorySource source = {.x = in.player.x, .y = in.player.y, .priority = 148};
    bool allocated = false;
    CHECK(SotnBatRuntime_RequestThrown(r, source, hand ? 0x802044U : 0x1044U, 165, &allocated) &&
          allocated);
    Uint16 root = 0, chained = 0, seen = 0;
    for (Uint16 t = 0; t < 30; ++t)
    {
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        chained = 0;
        for (Uint16 i = 1; i < 16; ++i)
        {
            const SotnBatRuntimeChild *c = &r->children[i];
            if (!c->weapon_secondary || c->state.secondary.family != 53 || !c->initialized)
                continue;
            CHECK(SotnBatRuntime_SecondaryValid(c, i) && c->primitive_count == 1);
            if (c->kind == (hand ? 0xf3 : 0xe3))
            {
                CHECK(c->state.secondary.own_item && c->state.secondary.item == 165 &&
                      c->parameter == (hand ? 0x8000U : 0U) && !c->group &&
                      c->state.secondary.attack.active &&
                      c->state.secondary.attack.equipment == 165);
                root = i;
            }
            else
            {
                CHECK(c->kind == (hand ? 0xf1 : 0xe1) &&
                      c->state.secondary.parent_slot == root + 16U &&
                      !c->state.secondary.attack.active);
                seen |= (Uint16)(1U << ((c->parameter >> 8) & 15U));
                ++chained;
            }
        }
    }
    CHECK(root && seen == 0x3fe && !chained);
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        CHECK(!r->children[i].weapon_secondary && !r->children[i].primitive_count);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}

// Blueprint56 creates Shotel's176 E1/F1 boomerang (own item, least-used group, no rows) in
// slots17..31; it flies out and back and retires at the player's hitbox.
static bool ShotelRuntime(Uint16 hand)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(56, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    static Uint8 exe[0x24000];
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Math(exe);
    FamilyDefinition(dra, 28, 0, 34, 12, 0, 0, 176);
    dra[0x500 + 28 * 52 + 18] = 0x92;
    FamilyDefinition(dra, 176, 1, 34, 129, 0, 0, 0);
    Uint8 *record = dra + 0x500 + 176 * 52;
    record[18] = 0x5d;
    record[19] = 56;
    record[21] = 0;
    record[22] = 1;
    const Uint8 blueprint[] = {1, 1, 0xc1, 0, 0x06};
    SDL_memcpy(dra + 0xd1d4 + 56U * 6U, blueprint, 5);
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    SotnEquipment e = {0};
    CHECK(SotnEquipment_Read(actors->dra, 28, &e) && e.kind == 12 && e.special_move == 176 &&
          e.family == 34 && SotnEquipment_Read(actors->dra, 176, &e) && e.kind == 129 &&
          e.blueprint == 56 && e.chain_limit == 1 && !e.consumable && e.animation == 0x5d);
    CHECK(SotnCharacter_Create(actors->dra, &character));
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = {exe, sizeof(exe)},
                              .weapons = {weapons, 56U * 0x7000},
                              .weapon_family = {34, 34},
                              .weapon_bound = {true, true},
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 128 * 65536,
                                         .y = 100 * 65536,
                                         .priority = 148,
                                         .animation = {.id = 0x5d,
                                                       .hitbox_x = 4,
                                                       .hitbox_y = -18,
                                                       .hitbox_width = 12,
                                                       .hitbox_height = 30}}};
    const SotnPlayerFactorySource source = {.x = in.player.x, .y = in.player.y, .priority = 148};
    bool allocated = false;
    const Uint32 packed = 56U | (hand + 1U) << 12 | (hand ? 0x80U << 16 : 0U);
    CHECK(!SotnBatRuntime_RequestThrown(r, source, packed ^ 0x10000U, 176, &allocated) &&
          SotnBatRuntime_RequestThrown(r, source, packed, 176, &allocated) && allocated);
    Uint16 boomerang = 0, live = 0;
    for (Uint16 t = 0; t < 300; ++t)
    {
        in.game_timer = t;
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        for (Uint16 i = 1; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        {
            const SotnBatRuntimeChild *c = &r->children[i];
            if (!c->weapon_secondary || c->state.secondary.family != 34 || !c->initialized)
                continue;
            CHECK(SotnBatRuntime_SecondaryValid(c, i) && i <= 15 &&
                  c->kind == (hand ? 0xf1 : 0xe1) && c->state.secondary.item == 176 &&
                  c->state.secondary.own_item && !c->primitive_count &&
                  c->parameter == (hand ? 0x8000U : 0U) && c->group >= 3 && c->group <= 6 &&
                  c->state.secondary.motion.properties.id == 176);
            boomerang = i;
            ++live;
        }
    }
    CHECK(boomerang && live > 0x1c);
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        CHECK(!r->children[i].weapon_secondary && !r->children[i].primitive_count);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}

// Authored weapon30 tables: scan order, soldier types, six private soldier timelines (two-update
// frames with frame property1, END after six), frame properties, palettes, speeds and steps.
static void DawnProgram(Uint8 *o)
{
    for (Uint16 i = 0; i < 15; ++i)
        o[0x608 + i] = (Uint8)i;
    for (Uint16 i = 0; i < 16; ++i)
        o[0x618 + i] = (Uint8)(i % 6);
    for (Uint16 t = 0; t < 6; ++t)
    {
        Put32(o + 0x628 + t * 4U, 0x8017a700U + t * 0x20U);
        for (Uint16 k = 0; k < 6; ++k)
        {
            Put16(o + 0x700 + t * 0x20U + k * 4U, 2);
            Put16(o + 0x700 + t * 0x20U + k * 4U + 2, (Uint16)(1U << 9 | (5U + k)));
        }
        Put16(o + 0x700 + t * 0x20U + 24, 0xffff);
        o[0x640 + t] = (Uint8)(2 * t);
        o[0x650 + t] = 2;
        Put16(o + 0x660 + t * 2U, (Uint16)(0x8120 + t));
    }
    const Uint8 steps[] = {3, 3, 2, 2, 5, 6};
    SDL_memcpy(o + 0x658, steps, 6);
    Put32(o + 0x648, 0xffff2000U);
    Put32(o + 0x64c, 0xffff4000U);
    const Sint8 props[] = {-3, 5, 12, 20};
    SDL_memcpy(o + 0x554 + 4, props, 4);
    Put16(o + 0x66c, 0xfff4);
    Put16(o + 0x66e, 12);
    for (Uint16 k = 0; k < 13; ++k)
        Put16(o + 0x514 + k * 4U, 3), Put16(o + 0x516 + k * 4U, (Uint16)(1 + k));
    Put16(o + 0x514 + 12 * 4U, 0xffff);
    for (Uint16 k = 0; k < 3; ++k)
        Put16(o + 0x548 + k * 4U, 2), Put16(o + 0x54a + k * 4U, (Uint16)(20 + k));
    Put16(o + 0x548 + 2 * 4U, 0);
    Put16(o + 0x54a + 2 * 4U, 0);
}
static Uint8 dawn_tiles[64 * 16 * 2], dawn_types[2] = {0, 3};
static SotnCollisionMap DawnFloor(void)
{
    for (Uint16 y = 10; y < 16; ++y)
        for (Uint16 x = 0; x < 64; ++x)
            dawn_tiles[(y * 64U + x) * 2] = 1;
    return (SotnCollisionMap){{dawn_tiles, sizeof(dawn_tiles)}, {dawn_types, 2}, 64, 16};
}
// w_030 func_ptr_80170004: one or six soldiers (first rand) before the rows, a second rand for
// the first type; an eight-update rise; a fifteen-point ground scan placing soldier factories
// (respecting free factories and live E2 spacing); bands, tether, waves and the flashing exit.
static bool DawnCircleTest(Uint16 hand)
{
    static Uint8 o[0x1400], exe[0x24000];
    DawnProgram(o);
    Math(exe);
    // Rolls: rand&7 neither0 nor4 (one soldier), 0 (six) and 4 (one; only &7 selects six).
    for (Uint16 want = 0; want < 3; ++want)
    {
        Uint32 seed = 1;
        for (;; ++seed)
        {
            Uint32 probe = seed;
            const Uint16 roll = (SotnCombat_Random(&probe) & 0x7fffU) & 7U;
            if (want == 0 ? roll != 0 && roll != 4 : roll == (want == 1 ? 0 : 4))
                break;
        }
        SotnHandWeaponProgram program = {0};
        SotnWeaponSecondary s = {.parameter = (Uint16)(hand << 15),
                                 .x = 100 * 65536 + 0x1234,
                                 .y = 128 * 65536 + 0x4321};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = {exe, sizeof(exe)},
                                       .x = 100 * 65536,
                                       .y = 128 * 65536,
                                       .hand = hand,
                                       .family = 30,
                                       .role = 1,
                                       .priority = 148,
                                       .facing_left = hand != 0,
                                       .allocated = true,
                                       .collision = DawnFloor(),
                                       .free_factories = 8};
        // A live E2 soldier of this hand within two pixels of point1 is skipped.
        in.slot_kind[3] = hand ? 0xf2 : 0xe2;
        in.slot_x[3] = (Sint16)(hand ? 95 : 109);
        Uint32 rng = seed, check = seed;
        const Sint16 six = ((SotnCombat_Random(&check) & 0x7fffU) & 7U) ? 1 : 6;
        const Uint16 first = (SotnCombat_Random(&check) & 0x7fffU) & 15U;
        SotnWeaponSecondaryEvents e = {0};
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == check &&
              s.phase == 1 && s.dawn_soldiers == six && six == (want == 1 ? 6 : 1) &&
              s.dawn_type == first % 6 && e.use_ticks == 4 &&
              s.dawn_quads[0].palette == (hand ? 0x128 : 0x110) &&
              s.dawn_quads[0].v[0] == (hand ? 0xb8 : 0x38) && s.dawn_quads[0].red[2] == 0x80 &&
              s.dawn_quads[0].draw_mode == 0x77 && s.dawn_quads[0].priority == 150 &&
              s.dawn_quads[10].palette == 0x15f && s.dawn_quads[10].priority == 152 &&
              s.dawn_quads[10].v[3] == (Uint8)((hand ? 0x80 : 0) + 0x38 + 79) &&
              s.velocity_x == (hand ? -0x18000 : 0x18000) && s.dawn_birth_x == 100 &&
              s.dawn_birth_y == 128 && s.dawn_quads[0].x[1] == 0 && s.dawn_quads[0].y[2] == 4);
        for (Uint16 t = 0; t < 8; ++t)
        {
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) &&
                  s.dawn_half_w == SDL_min(t + 1, 8) && s.dawn_half_h == 4 * (t + 1));
        }
        CHECK(s.phase == 2 && rng == check && (s.x >> 16) == (hand ? 88 : 112) &&
              Pixel(s.y) == 128 - 41 && e.sounds[0] == 0x635 && e.volume_sound &&
              e.volume == 0x58 && e.factory_count == six && s.dawn_target);
        for (Uint16 k = 0; k < e.factory_count; ++k)
        {
            // Point1 (one step forward) holds the live soldier and is skipped.
            const Uint16 point = k == 0 ? 0 : (Uint16)(k + 1);
            const Sint32 px = point < 8 ? (hand ? 100 - 7 * point : 100 + 7 * point)
                                        : (hand ? 100 + 7 * (point - 7) : 100 - 7 * (point - 7));
            CHECK(e.factories[k] == (0x3eU | (hand + 1U) << 12 |
                                     (Uint32)((first + k) % 6U + (six == 6 ? 0x10U : 0U)) << 16) &&
                  (e.factory_x[k] >> 16) == px && (e.factory_x[k] & 0xffff) == 0x1234 &&
                  (e.factory_y[k] >> 16) == 160 - 0x19 &&
                  (e.factory_y[k] & 0xffff) == (s.y & 0xffff));
        }
        CHECK(s.dawn_type == (Sint16)((first + six) % 6U) &&
              s.dawn_target_x == (Sint16)(e.factory_x[six - 1] >> 16) &&
              s.dawn_target_y == 160 - 0x19);
        for (Uint16 t = 0; t < 8; ++t)
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
        CHECK(s.phase == 3 && program.dawn_phase[9] == 0x199 * 9 &&
              program.dawn_phase[1] == 0x199 + 0x80 && s.dawn_fade == 0xff);
        Uint16 t = 0;
        for (; s.phase == 3 && t < 200; ++t)
        {
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.dawn_line &&
                  s.dawn_quads[10].red[1] == (Uint8)SDL_max(0xff - 8 * (t + 1), 0) &&
                  s.dawn_band == SDL_min(4 * (t + 1), 0x120) &&
                  s.dawn_wave == (t + 1 >= 10 ? 1 + (t + 1 - 10) / 4 : 0));
        }
        CHECK(s.phase == 4 && t == 72 && rng == check);
        for (t = 0; !s.removed && t < 60; ++t)
        {
            in.game_timer = t;
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
            if (!s.removed)
                CHECK(((s.dawn_quads[0].draw_mode & 8U) != 0) == ((t & 1U) != 0) &&
                      s.dawn_quads[0].red[0] == SDL_min(0x80 + 6 * (t + 1), 240) &&
                      s.dawn_quads[0].blue[3] == SDL_max(0x80 - 6 * (t + 1), 10));
        }
        CHECK(s.removed && t == 37);
    }
    // Exhausted rows retire after the first roll; no free factory places nothing.
    {
        SotnHandWeaponProgram program = {0};
        SotnWeaponSecondary s = {0};
        SotnWeaponSecondaryEvents e = {0};
        Uint32 rng = 7, check = 7;
        (void)SotnCombat_Random(&check);
        CHECK(SotnWeaponSecondary_Tick(&s, &program,
                                       (SotnWeaponSecondaryInput){.overlay = {o, sizeof(o)},
                                                                  .executable = {exe, sizeof(exe)},
                                                                  .family = 30,
                                                                  .role = 1},
                                       &rng, &e) &&
              s.removed && rng == check);
        s = (SotnWeaponSecondary){.y = 128 * 65536};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = {exe, sizeof(exe)},
                                       .y = 128 * 65536,
                                       .family = 30,
                                       .role = 1,
                                       .allocated = true,
                                       .collision = DawnFloor()};
        for (Uint16 t = 0; t < 9; ++t)
        {
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
        }
        CHECK(s.phase == 2 && !e.factory_count && !s.dawn_target && e.sounds[0] == 0x635);
    }
    CHECK(SotnWeaponSecondary_SupportedRole(30, 4) && !SotnWeaponSecondary_SupportedRole(30, 5) &&
          SotnWeaponSecondary_RoleRows(30, 1) == 11 && SotnWeaponSecondary_RoleRows(30, 2) == 14 &&
          !SotnWeaponSecondary_RoleRows(30, 3));
    return true;
}
// w_030 func_ptr_80170008: soldiers wait for their wave, roll once, take their rows and rise;
// walkers walk on the floor and leave the room bounds, archers shoot at pose5, the sword
// thrower requests two or three swords, the bomber its bomb at pose4; hits blink them out.
static bool DawnSoldierTest(Uint16 hand)
{
    static Uint8 o[0x1400], exe[0x24000];
    DawnProgram(o);
    Math(exe);
    for (Uint16 type = 0; type < 6; ++type)
    {
        SotnHandWeaponProgram program = {0};
        SotnWeaponSecondary s = {.parameter = (Uint16)((type | (type == 3 ? 0x10 : 0)) << 8),
                                 .x = 100 * 65536 + 0x77,
                                 .y = 135 * 65536};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = {exe, sizeof(exe)},
                                       .hand = hand,
                                       .family = 30,
                                       .role = 2,
                                       .priority = 148,
                                       .allocated = true,
                                       .collision = DawnFloor(),
                                       .free_factories = 8,
                                       .parent_wave = (Sint16)type,
                                       .parent_facing_left = hand == 0,
                                       .room_left = 0,
                                       .room_right = 1024};
        Uint32 rng = 41U + type, check = rng;
        SotnWeaponSecondaryEvents e = {0};
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && !s.phase && rng == check);
        in.parent_wave = (Sint16)(type + 1);
        const Uint16 random = (SotnCombat_Random(&check) & 0x7fffU) & 1U;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 1 &&
              e.shake == 4 && e.sounds[0] == 0x654 && s.bank == 10 &&
              s.palette == (type == 3 ? 0x8123U : (Uint16)(0x8110U + 2U * type + random)) +
                               (hand ? 0x18U : 0U) &&
              s.texture == 0x64U + hand * 2U && s.facing_left == (hand != 0) &&
              s.dawn_shots == 2 + random && s.dawn_lives == (type == 1 ? 3 : 0) &&
              s.dawn_quads[0].palette == (hand ? 0x12a : 0x112) && s.dawn_quads[0].x[1] == 116 &&
              s.dawn_quads[3].priority == 160 && s.frame == 5 && s.hitbox_width == 12 &&
              s.hitbox_x == -3);
        // The rise mask uses one row rand per update; smoke uses three on its slot's update.
        Uint16 t = 0;
        for (; s.phase == 1 && t < 100; ++t)
        {
            in.game_timer = 8U + t;
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
        }
        const Uint8 steps[] = {3, 3, 2, 2, 5, 6};
        CHECK(s.phase == steps[type] && e.group && s.hitbox_state == 2 &&
              s.properties.attack == 4 && s.properties.element == 0x20 &&
              Pixel(s.y) == 135 + (type == 5 ? 2 : 1) &&
              s.velocity_x == (type == 0   ? (hand ? 0xe000 : -0xe000)
                               : type == 1 ? (hand ? 0xc000 : -0xc000)
                                           : 0));
        if (type == 0)
        {
            // Walking: floor contact holds the feet at the floor; leaving the bounds blinks out.
            const Sint32 x = s.x;
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 3 &&
                  s.x == x + s.velocity_x);
            in.room_right = Pixel(s.x) - 1;
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 7 &&
                  s.dawn_blink == 0x18);
        }
        else if (type == 1)
        {
            // Three lives: a hit blinks back to walking, keeping the actual blink count.
            s.hit_flags = 1;
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 8 &&
                  s.dawn_lives == 2 && (s.draw_flags & 0x80U));
            for (t = 0; s.phase == 8 && t < 30; ++t)
                CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
            CHECK(s.phase == 3 && t == 0x17 && !(s.draw_flags & 0x80U));
        }
        else
        {
            Uint16 requests = 0, bombs = 0;
            for (t = 0; !s.removed && t < 600; ++t)
            {
                in.game_timer = 0x100U + t;
                e = (SotnWeaponSecondaryEvents){0};
                CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
                for (Uint16 k = 0; k < e.factory_count; ++k)
                {
                    const Uint32 f = e.factories[k];
                    CHECK(e.factory_x[k] == s.x && e.factory_y[k] == s.y);
                    if (type == 2 || type == 3)
                        CHECK(f == (0x46U | (hand + 1U) << 12 | (Uint32)(type - 2U) << 16) &&
                              e.sounds[0] == 0x627 && s.pose == 5);
                    else if (type == 4)
                        CHECK(f == (0x46U | (hand + 1U) << 12 | 2U << 16));
                    else
                    {
                        // The request is made at pose4/timer1; the post-callback UpdateAnim then
                        // advances.
                        CHECK(f == (0x6aU | (hand + 1U) << 12) && s.pose == 5);
                        ++bombs;
                    }
                    ++requests;
                }
            }
            CHECK(s.removed && (type == 5   ? bombs == 1
                                : type == 4 ? requests >= 4 * 2
                                            : requests == 2 + random));
        }
    }
    return true;
}
// w_030 func_ptr_8017000C: arrows fly back at5.5 (five updates rewound), swords appear around
// their thrower (D_8017A66C by the program's count), spin, then home on a selected target.
static bool DawnArrowTest(Uint16 hand)
{
    static Uint8 o[0x1400], exe[0x24000];
    DawnProgram(o);
    Math(exe);
    SotnHandWeaponProgram program = {.secondary_counter = 4};
    for (Uint16 kind = 0; kind < 3; ++kind)
    {
        SotnWeaponSecondary s = {
            // The right hand's swords carry factory index1: lifetime0x30 plus four per index.
            .parameter = (Uint16)(kind << 8 | hand),
            .x = 100 * 65536,
            .y = 120 * 65536};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = {exe, sizeof(exe)},
                                       .hand = hand,
                                       .family = 30,
                                       .role = 3,
                                       .priority = 148,
                                       .parent_facing_left = hand != 0,
                                       .target = {.identity = 7, .hitbox = 3, .x = 200, .y = 60}};
        Uint32 rng = 9, check = 9;
        SotnWeaponSecondaryEvents e = {0};
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && e.group &&
              s.hitbox_state == 2 && s.hitbox_width == 8 && s.hitbox_height == 4 &&
              s.frame == 0x64 && s.bank == 10);
        if (kind < 2)
        {
            const Sint32 v = hand ? 0x58000 : -0x58000;
            CHECK(rng == check && s.phase == 1 && s.velocity_x == v && s.x == 100 * 65536 + v * 5 &&
                  Pixel(s.y) == 120 - (kind ? 4 : 16));
            const Sint32 x = s.x;
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.x == x + v);
        }
        else
        {
            const Uint16 a = (SotnCombat_Random(&check) & 0x7fffU) & 15U,
                         b = (SotnCombat_Random(&check) & 0x7fffU) & 7U;
            const Sint16 angle = (Sint16)(SotnCombat_Random(&check) & 0x7fffU);
            CHECK(rng == check && s.phase == 3 && e.sounds[0] == 0x6ba &&
                  program.secondary_counter == 5 && Pixel(s.y) == 120 - 28 - a &&
                  Pixel(s.x) == 100 + 12 - 4 + b && !s.facing_left && s.dawn_side == !hand &&
                  s.dawn_angle == angle && s.lifetime == 0x30 + 4 * hand && s.draw_flags == 4);
            Uint16 t = 0;
            for (; s.phase == 3 && t < 100; ++t)
            {
                e = (SotnWeaponSecondaryEvents){0};
                CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
            }
            CHECK(t == 0x30 + 4 * hand && e.select_target && e.sounds[0] == 0x627 &&
                  s.dawn_limit == 0x20 && s.dawn_speed == 0x600 && s.phase == 1);
            s.dawn_bound = true;
            for (t = 0; t < 40; ++t)
            {
                const Sint16 limit = s.dawn_limit;
                CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) &&
                      s.rotate == (Sint16)(0x800 - s.dawn_angle));
                CHECK(s.dawn_limit <= limit + 0x18);
            }
            // Locked on: the heading points at the target (up-right).
            CHECK(s.dawn_angle > 0 && s.dawn_angle < 0x400);
            in.target.identity = 0; // A retired target stops the steering.
            const Sint16 locked = s.dawn_angle, grown = s.dawn_limit;
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.dawn_angle == locked &&
                  s.dawn_limit == grown + 0x18);
        }
        s.hit_flags = 1;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 2 &&
              s.lifetime == 10);
        Uint16 t = 0;
        for (; !s.removed && t < 20; ++t)
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && (s.draw_flags & 0x80U));
        CHECK(s.removed && t == 10);
    }
    return true;
}
// w_030 func_ptr_80170010: the bombs retire with their raw parent; they grow and spin with a
// rising volume, burst (index0) or roll (index1) at the bomber's pose0x20, and shrink after a hit.
static bool DawnBombTest(Uint16 hand)
{
    static Uint8 o[0x1400], exe[0x24000];
    DawnProgram(o);
    Math(exe);
    SotnHandWeaponProgram program = {0};
    for (Uint16 index = 0; index < 2; ++index)
    {
        SotnWeaponSecondary s = {.parameter = index, .x = 100 * 65536, .y = 140 * 65536};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = {exe, sizeof(exe)},
                                       .hand = hand,
                                       .family = 30,
                                       .role = 4,
                                       .priority = 148,
                                       .free_factories = 8,
                                       .parent_facing_left = hand == 0};
        Uint32 rng = 3;
        SotnWeaponSecondaryEvents e = {0};
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.removed);
        s = (SotnWeaponSecondary){.parameter = index, .x = 100 * 65536, .y = 140 * 65536};
        in.parent_exists = true;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 1 && e.group &&
              s.properties.attack == 0x14 && s.scale_x == 0x20 && s.bank == 9 &&
              s.timeline == (index ? 0x548 : 0x514) && Pixel(s.y) == 140 - 0x27 &&
              s.velocity_x == (hand ? -0x38000 : 0x38000) && s.hitbox_width == 0 &&
              s.priority == 156 && s.flags == 0x08100000U);
        Uint16 t = 0, sounds = 0;
        for (; t < 40; ++t)
        {
            in.game_timer = t;
            const Sint16 volume = s.dawn_volume;
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) &&
                  s.scale_x == SDL_min(0x20 + 2 * (t + 1), 0x100) &&
                  s.rotate == (Sint16)(0x100 * (t + 1)) &&
                  s.hitbox_width == (Uint8)((s.scale_x >> 6) * 3));
            if (!(t & 7U))
            {
                CHECK(e.sounds[0] == 0x652 && e.volume == volume);
                ++sounds;
            }
        }
        CHECK(sounds == 5 && s.dawn_volume == 0x58);
        in.parent_pose = 0x20;
        s.remaining = 1;
        e = (SotnWeaponSecondaryEvents){0};
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && e.sounds[1] == 0x65e);
        if (!index)
        {
            CHECK(s.removed);
            continue;
        }
        CHECK(s.phase == 2 && s.blend == 0x50);
        Uint16 sparks = 0;
        for (t = 0; t < 32; ++t)
        {
            in.game_timer = t;
            const Sint32 x = s.x;
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.x == x + s.velocity_x);
            for (Uint16 k = 0; k < e.factory_count; ++k)
            {
                CHECK(e.factories[k] == 0x10024U && !(t & 1U));
                ++sparks;
            }
        }
        CHECK(sparks > 0 && sparks < 16);
        s.hit_flags = 1;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 3);
        for (t = 0; !s.removed && t < 400; ++t)
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
        CHECK(s.removed && !s.hitbox_state && !s.bank);
    }
    return true;
}

// w_008 EntityWeaponShieldSpell: eleven mandatory rows first; birth retires a raised shield,
// plays ANIME_SWORD_C and enters freeze1; the image grows (12/update), holds, flashes out with
// TRANSFORM_3X, the wave card opens and darkens, the pink FACTORY(55,8) follows, and the spell
// ends the freeze and retires. shieldAngles load once from the program and keep stepping.
// w_009 Knight (knight): ten horizontal bands in twelve rows, a 48x40 card, pink7 and a fade that
// steps the angle increment.
static bool LeatherSpellTest(Uint16 hand, bool knight)
{
    static Uint8 o[0x1400], exe[0x24000];
    Math(exe);
    for (Uint16 i = 0; i < 16; ++i)
        Put32(o + 0xa78 + i * 4U, i * 0x200U);
    SotnHandWeaponProgram program = {0};
    SotnWeaponSecondary s = {
        .parameter = (Uint16)(hand << 15), .x = 128 * 65536 + 0x111, .y = 120 * 65536};
    SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                   .executable = {exe, sizeof(exe)},
                                   .hand = hand,
                                   .family = knight ? 9 : 8,
                                   .role = 8,
                                   .priority = 148,
                                   .facing_left = hand != 0,
                                   .allocated = true};
    Uint32 rng = 5;
    SotnWeaponSecondaryEvents e = {0};
    {
        SotnWeaponSecondary cold = {0};
        SotnWeaponSecondaryInput none = in;
        none.allocated = false;
        CHECK(SotnWeaponSecondary_Tick(&cold, &program, none, &rng, &e) && cold.removed &&
              !e.freeze_set && !e.sounds[0] && program.shield_angles_loaded &&
              program.shield_angles[3] == 0x600);
        program = (SotnHandWeaponProgram){0};
    }
    CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == 5 && s.phase == 1 &&
          e.clear_shield && e.sounds[0] == 0x641 && e.freeze_set && !e.freeze_clear &&
          s.bank == (hand ? 0x8012 : 0x8010) && s.palette == (hand ? 0x128 : 0x110) &&
          s.texture == (hand ? 0x66 : 0x64) && s.frame == 0x3e && s.draw_flags == 3 && !s.scale_x &&
          Pixel(s.y) == 112 && s.flags == 0x04810000U && s.priority == 146 &&
          s.velocity_x == (hand ? 0x20000 : -0x20000) && s.spell_bright == 0x100 &&
          s.spell_quads[0].palette == 0x19f && s.spell_quads[0].draw_mode == 0x75 &&
          s.spell_quads[0].v[0] == (hand ? 0 : 0x80) && s.spell_quads[0].priority == 142 &&
          s.spell_quads[1].palette == (hand ? 0x129 : 0x111) && s.spell_quads[1].draw_mode == 8 &&
          s.spell_quads[1].v[0] == (hand ? 0xb0 : 0x30) &&
          s.spell_quads[1].v[2] == (Uint8)((hand ? 0x80 : 0) + (knight ? 0x7f : 0x67)) &&
          s.spell_quads[2].v[0] ==
              (knight ? (Uint8)((hand ? 0xb0 : 0x30)) : (hand ? 0xb0 : 0x30)) &&
          s.spell_quads[3].v[0] == (Uint8)((hand ? 0xb0 : 0x30) + (knight ? 8 : 0)) &&
          s.spell_quads[10].u[1] == (knight ? 0x5f : 71) && s.spell_quads[10].palette == 0x15f &&
          s.spell_quads[10].draw_mode == 0x3d && s.spell_quads[10].priority == 0x1be &&
          s.spell_quads[knight ? 11 : 10].v[2] ==
              (Uint8)((hand ? 0xb0 : 0x30) + (knight ? 79 : 0x37)) &&
          program.shield_angles[0] == 0x40 && program.shield_angles[8] == 0x1040 &&
          program.shield_angles[9] == (knight ? 0x1240 : 0x1200));
    Uint16 t = 0, pink = 0, flashes = 0, brightening = 0, fading = 0;
    Sint32 vx = s.velocity_x;
    for (; !s.removed && t < 400; ++t)
    {
        in.engine_timer = t;
        const Uint8 phase = s.phase;
        e = (SotnWeaponSecondaryEvents){0};
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == 5);
        if (phase == 1)
        {
            vx = vx < 0 ? SDL_min(vx + 0x1000, 0) : SDL_max(vx - 0x1000, 0);
            CHECK(s.velocity_x == vx && (s.phase == 2 ? s.scale_x == 0x100 && s.spell_glow == 0x28
                                                      : s.scale_x == 12 * (t + 1)));
        }
        // The darkened card brightens 0x10 per update: nineteen updates past0x120.
        brightening += phase == 6;
        fading += phase == 7;
        // The card reaches its full half size (Knight48x40, Leather36x28) as phase4 ends.
        if (phase == 4 && s.phase == 5)
            CHECK(s.spell_half_w == (knight ? 48 : 36) && s.spell_half_h == (knight ? 40 : 28));
        if (e.sounds[0] == 0x6b1)
        {
            CHECK(phase == 3 && s.phase == 4 && !s.frame && !(s.spell_quads[1].draw_mode & 8U) &&
                  !(s.spell_quads[5].draw_mode & 8U) && s.spell_timer == 0x20);
            ++flashes;
        }
        if (e.pink)
        {
            CHECK(e.pink == (knight ? 7 : 8) && phase == 6 && s.phase == 7 &&
                  s.spell_bright > 0x120 && (s.spell_quads[1].draw_mode & 8U));
            ++pink;
        }
        if (s.removed)
            CHECK(e.freeze_clear && phase == 7 &&
                  (knight ? s.spell_bright < 4 && s.spell_step > 0x40 : s.spell_wave_y > 0xc0));
    }
    // Knight fades 0x130 by eight to below four: thirty-eight updates.
    CHECK(s.removed && pink == 1 && flashes == 1 && brightening == 19 &&
          (!knight || fading == 38) &&
          (knight || program.shield_angles[0] == (Sint32)(0x40 * (t + 0U))));
    CHECK(SotnWeaponSecondary_SupportedRole(8, 8) && !SotnWeaponSecondary_SupportedRole(8, 1) &&
          SotnWeaponSecondary_RoleRows(8, 8) == 11 && SotnWeaponSecondary_RoleRows(9, 8) == 12);
    return true;
}

// w_058 EntityWeaponShieldSpell: Alucart's two rows; the shield grows, holds 0x40 updates, plays
// TRANSFORM, flashes out, keeps the unused unk86/unk92 accumulations, then shows the page-0x1E
// question mark above the player for 0x28 updates and ends the freeze.
static bool AlucartSpellTest(Uint16 hand)
{
    static Uint8 o[0x1400], exe[0x24000];
    Math(exe);
    SotnHandWeaponProgram program = {0};
    SotnWeaponSecondary s = {.parameter = (Uint16)(hand << 15), .x = 128 * 65536, .y = 120 * 65536};
    SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                   .executable = {exe, sizeof(exe)},
                                   .x = 100 * 65536,
                                   .y = 150 * 65536,
                                   .hand = hand,
                                   .family = 58,
                                   .role = 8,
                                   .priority = 148,
                                   .facing_left = hand != 0,
                                   .allocated = true};
    Uint32 rng = 5;
    SotnWeaponSecondaryEvents e = {0};
    CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == 5 && e.freeze_set &&
          e.clear_shield && e.sounds[0] == 0x641 && s.spell_half_h == (hand ? 0x129 : 0x111) &&
          s.spell_quads[1].palette == 0x190 && s.spell_quads[1].page == 0x1e &&
          s.spell_quads[1].u[1] == 0x7f && s.spell_quads[1].v[2] == 15 &&
          s.spell_quads[1].priority == 150 && s.spell_quads[1].draw_mode == 8 &&
          s.spell_quads[1].x[0] == (hand ? 102 : 94) && s.spell_quads[1].y[0] == 150 - 0x22 &&
          !program.shield_angles_loaded);
    Uint16 t = 0, shown = 0, holding = 0, transform = 0;
    for (; !s.removed && t < 400; ++t)
    {
        in.engine_timer = t;
        const Uint8 phase = s.phase;
        const Sint16 sum = s.spell_wave_x, unk86 = s.spell_half_h;
        e = (SotnWeaponSecondaryEvents){0};
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == 5);
        holding += phase == 2;
        transform += e.sounds[0] == 0x668;
        // The flash alternates the glow behind everything (0x1FC) with priority-2.
        if (phase == 3 && s.phase == 3)
            CHECK(s.spell_quads[0].priority == (!((s.spell_timer + 1) & 1) ? 0x1fc : 144));
        if (phase == 4)
            CHECK(s.spell_half_h == unk86 + 4 && s.spell_wave_x == (Sint16)(sum + s.spell_half_h));
        if (phase == 5)
        {
            CHECK(!s.spell_quads[1].draw_mode);
            ++shown;
        }
        else if (!s.removed)
            CHECK(s.spell_quads[1].draw_mode == 8);
        if (s.removed)
            CHECK(e.freeze_clear && phase == 6 && !e.pink);
    }
    CHECK(s.removed && holding == 0x40 && transform == 1 && shown == 0x28 &&
          SotnWeaponSecondary_RoleRows(58, 8) == 2 && SotnWeaponSecondary_ShieldSpell(58) &&
          SotnWeaponSecondary_ShieldSpellItem(58) == 215 &&
          SotnWeaponSecondary_ShieldSpellFamily(215) == 58);
    return true;
}

// w_011 EntityWeaponShieldSpell: Axe Lord's ten rows. Birth resets the program's band angles to
// (i<<9)*2; the image grows, holds, flashes out with MAGIC_WEAPON_APPEAR_A, eight green bands
// narrow, then the spell hides, ends the freeze and creates the blueprint88 walker on the same
// update before its four-update tail.
static bool AxeLordSpellTest(Uint16 hand)
{
    static Uint8 o[0x1400], exe[0x24000];
    Math(exe);
    SotnHandWeaponProgram program = {0};
    for (Uint16 i = 0; i < 16; ++i)
        program.shield_angles[i] = 0x7777;
    SotnWeaponSecondary s = {.parameter = (Uint16)(hand << 15), .x = 128 * 65536, .y = 120 * 65536};
    SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                   .executable = {exe, sizeof(exe)},
                                   .hand = hand,
                                   .family = 11,
                                   .role = 8,
                                   .priority = 148,
                                   .facing_left = hand != 0,
                                   .allocated = true,
                                   .free_factories = 4};
    Uint32 rng = 5;
    SotnWeaponSecondaryEvents e = {0};
    {
        SotnWeaponSecondary cold = {0};
        SotnWeaponSecondaryInput none = in;
        none.allocated = false;
        CHECK(SotnWeaponSecondary_Tick(&cold, &program, none, &rng, &e) && cold.removed &&
              !e.freeze_set && !e.sounds[0] && program.shield_angles[15] == 15 << 10);
    }
    CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == 5 && s.phase == 1 &&
          e.clear_shield && e.sounds[0] == 0x641 && e.freeze_set && !e.factory_count &&
          s.bank == (hand ? 0x8012 : 0x8010) && s.palette == (hand ? 0x128 : 0x110) &&
          s.frame == 0x3e && Pixel(s.y) == 112 && s.flags == 0x04810000U && s.priority == 146 &&
          s.velocity_x == (hand ? 0x20000 : -0x20000) && s.spell_quads[0].palette == 0x19f &&
          s.spell_quads[0].draw_mode == 0x75 && s.spell_quads[0].v[0] == (hand ? 0 : 0x80) &&
          s.spell_quads[1].palette == (hand ? 0x129 : 0x111) && s.spell_quads[1].draw_mode == 8 &&
          s.spell_quads[1].v[0] == (hand ? 0xb0 : 0x30) &&
          s.spell_quads[9].v[0] == (Uint8)((hand ? 0xb0 : 0x30) + 56) &&
          s.spell_quads[9].v[2] == (Uint8)((hand ? 0xb0 : 0x30) + 63) &&
          s.spell_quads[9].palette == 0x15f && s.spell_quads[9].priority == 0x1be &&
          s.spell_quads[9].draw_mode == 0x3d && program.shield_angles[0] == 0x40 &&
          program.shield_angles[7] == (7 << 10) + 0x40 && program.shield_angles[8] == 8 << 10 &&
          SotnWeaponSecondary_RoleRows(11, 8) == 10 && SotnWeaponSecondary_Rows(11) == 10 &&
          SotnWeaponSecondary_ShieldSpellItem(11) == 206 &&
          SotnWeaponSecondary_ShieldSpellFamily(206) == 11 &&
          SotnWeaponSecondary_SupportedRole(11, 9) && !SotnWeaponSecondary_RoleRows(11, 9));
    Uint16 t = 0, counts[8] = {0}, appear = 0, walkers = 0;
    for (; !s.removed && t < 400; ++t)
    {
        in.engine_timer = t;
        const Uint8 phase = s.phase;
        e = (SotnWeaponSecondaryEvents){0};
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == 5 && !e.pink);
        ++counts[phase];
        if (e.sounds[0] == 0x669)
        {
            CHECK(phase == 3 && !s.frame && !(s.spell_quads[2].draw_mode & 8U) &&
                  !(s.spell_quads[9].draw_mode & 8U) && (s.spell_quads[1].draw_mode & 8U));
            ++appear;
        }
        // Eight bands slice the 64-row card evenly (wave_y stays zero).
        if (phase == 4 && s.phase == 5)
            CHECK(!s.spell_wave_x && !(s.spell_quads[1].draw_mode & 8U) && s.spell_half_h == 32 &&
                  s.spell_quads[1].y[2] - s.spell_quads[1].y[0] == 63 &&
                  s.spell_quads[5].y[0] - s.spell_quads[2].y[0] == 24 &&
                  s.spell_quads[9].y[2] - s.spell_quads[1].y[0] == 64 && !s.spell_wave_y);
        if (phase >= 4 && phase < 7)
            CHECK(!s.spell_quads[4].red[0] &&
                  s.spell_quads[4].green[0] == s.spell_quads[4].blue[0]);
        if (e.factory_count)
        {
            CHECK(phase == 5 && s.phase == 6 && e.freeze_clear && e.factory_count == 1 &&
                  e.factories[0] == 88U + ((hand + 1U) << 14) && e.factory_x[0] == s.x &&
                  e.factory_y[0] == s.y &&
                  // The flash glow still flickers with g_Timer (its blue3 stays above eight).
                  ((s.spell_quads[0].draw_mode & 8U) != 0) == ((t & 1U) != 0) &&
                  (s.spell_quads[1].draw_mode & 8U) && !s.spell_bright);
            ++walkers;
        }
        else
            CHECK(!e.freeze_clear);
    }
    // Growth22, hold8, flash16, narrowing24, darkening17, brightening16, tail4.
    CHECK(s.removed && appear == 1 && walkers == 1 && counts[1] == 22 && counts[2] == 8 &&
          counts[3] == 16 && counts[4] == 24 && counts[5] == 17 && counts[6] == 16 &&
          counts[7] == 4);
    return true;
}

static Uint8 axe_tiles[64 * 16 * 2], axe_types[2] = {0, 3};
// A floor at world y1000 (a fall long enough to reach the 6.0 velocity cap).
static bool DeepFloor(void *context, Sint32 x, Sint32 y, SotnCollider *out)
{
    (void)context;
    (void)x;
    *out = (SotnCollider){0};
    if (y >= 1000)
    {
        out->effects = SOTN_EFFECT_SOLID;
        out->top = 1000 - y;
    }
    return true;
}
static bool AxeAnimation(Uint8 *o)
{
    // D_8017ABCC loops its seven frames; D_8017ABE8 ends after two.
    for (Uint16 i = 0; i < 6; ++i)
        Put32(o + 0xbcc + i * 4U, 3U | (Uint32)(i + 1U) << 16);
    Put32(o + 0xbcc + 24, 0);
    Put32(o + 0xbe8, 2U | 9U << 16);
    Put32(o + 0xbec, 2U | 10U << 16);
    Put32(o + 0xbf0, 0xffffU);
    return true;
}
// w_011 func_ptr_80170024: the walker (params0) copies its raw parent's bank/texture/palette/
// facing, rolls properties, falls to the floor (STOMP_SOFT_A), walks 2.5 px per update with a
// stomp every ten and an afterimage every four counts, and on a wall plays CANDLE_HIT, its final
// animation and one last afterimage. Afterimages (params1) fade by two for0x18 updates.
static bool AxeLordWalkerTest(Uint16 hand)
{
    static Uint8 o[0x1400], exe[0x24000];
    Math(exe);
    AxeAnimation(o);
    for (Uint16 y = 0; y < 16; ++y)
        for (Uint16 x = 0; x < 64; ++x)
            axe_tiles[(y * 64U + x) * 2] = (Uint8)(y >= 10 || x == 4 || x == 40);
    const Uint16 hit = 1;
    SotnHandWeaponProgram program = {0};
    SotnWeaponSecondaryInput in = {
        .overlay = {o, sizeof(o)},
        .executable = {exe, sizeof(exe)},
        .hand = hand,
        .family = 11,
        .role = 9,
        .priority = 148,
        .context = (void *)&hit,
        .properties = Properties,
        .collision = {{axe_tiles, sizeof(axe_tiles)}, {axe_types, 2}, 64, 16},
        .free_factories = 4,
        .parent_bank = (Uint16)(hand ? 0x8012 : 0x8010),
        .parent_texture = (Uint16)(hand ? 0x66 : 0x64),
        .parent_child_palette = (Uint16)(hand ? 0x128 : 0x110),
        .parent_frame = 0x22,
        .parent_facing_left = hand != 0};
    Uint32 rng = 9, check = 9;
    SotnWeaponSecondaryEvents e = {0};
    {
        SotnWeaponSecondary orphan = {.parameter = (Uint16)(hand << 15)};
        CHECK(SotnWeaponSecondary_Tick(&orphan, &program, in, &rng, &e) && orphan.removed &&
              rng == check && !e.properties);
    }
    in.parent_exists = true;
    SotnWeaponSecondary s = {
        .parameter = (Uint16)(hand << 15), .x = 300 * 65536 + 0x55, .y = 100 * 65536};
    (void)SotnCombat_Random(&check);
    CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == check && s.phase == 1 &&
          e.properties && s.bank == in.parent_bank && s.texture == in.parent_texture &&
          s.palette == in.parent_child_palette && s.facing_left == (hand != 0) &&
          s.priority == 150 && s.flags == 0x08100000U && s.timeline == 0xbcc &&
          s.hitbox_width == 8 && s.hitbox_height == 20 && s.hitbox_state == 1 &&
          s.velocity_x == (hand ? -0x10000 : 0x10000) && !e.factory_count);
    Uint16 t = 0, stomps = 0, images = 0, landed = 0;
    // Walking afterimages are created before the step (walking moves after the wall probe).
    for (; s.phase != 3 && t < 400; ++t)
    {
        in.engine_timer = t;
        const Uint8 phase = s.phase;
        const Sint16 count = s.dawn_count;
        const Sint32 x0 = s.x;
        e = (SotnWeaponSecondaryEvents){0};
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == check &&
              SotnWeaponSecondary_Animate(&s, in.overlay, 11));
        stomps += e.sounds[0] == 0x64b;
        if (phase == 1 && s.phase == 2)
        {
            CHECK(Pixel(s.y) == 135 && e.sounds[0] == 0x64b && !landed);
            ++landed;
        }
        if (phase == 2 && s.phase == 2)
            CHECK(s.velocity_x == (hand ? -0x28000 : 0x28000) &&
                  (e.sounds[0] == 0x64b) == !(count % 10) &&
                  e.factory_count == (count % 4 ? 0 : 1));
        // Falling afterimages follow g_Timer; landing breaks before them.
        if (phase == 1 && s.phase == 1)
            CHECK(e.factory_count == (t % 4 ? 0 : 1));
        for (Uint8 f = 0; f < e.factory_count; ++f)
            CHECK(e.factories[f] == ((90U + ((hand + 1U) << 14)) | 1U << 16) &&
                  e.factory_x[f] == (phase == 2 ? x0 : s.x) && ++images);
    }
    // The wall column (x64..79 or x640..655) stops it eight pixels before.
    CHECK(s.phase == 3 && e.sounds[1] == 0x634 && s.timeline == 0xbe8 && !s.pose &&
          e.factory_count && landed == 1 && stomps > 4 && images > 20 &&
          (hand ? Pixel(s.x) - 8 <= 80 && Pixel(s.x) - 8 > 77
                : Pixel(s.x) + 8 >= 640 && Pixel(s.x) + 8 < 643));
    for (t = 0; !s.removed && t < 20; ++t)
    {
        e = (SotnWeaponSecondaryEvents){0};
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && !e.sounds[0] &&
              !e.factory_count && (s.removed || SotnWeaponSecondary_Animate(&s, in.overlay, 11)));
    }
    // Animation births2, then2, then the end marker; retirement follows the next callback.
    CHECK(s.removed && t == 5);
    // A hit also stops it (CANDLE_HIT, final animation, one last afterimage).
    {
        SotnWeaponSecondary w = {.parameter = 0, .x = 300 * 65536, .y = 135 * 65536};
        in.parent_facing_left = false;
        CHECK(SotnWeaponSecondary_Tick(&w, &program, in, &rng, &e));
        CHECK(SotnWeaponSecondary_Tick(&w, &program, in, &rng, &e) && w.phase == 2);
        w.hit_flags = 1;
        e = (SotnWeaponSecondaryEvents){0};
        const Sint32 x = w.x;
        CHECK(SotnWeaponSecondary_Tick(&w, &program, in, &rng, &e) && w.phase == 3 && w.x == x &&
              e.sounds[1] == 0x634 && e.factory_count == 1);
    }
    // A long fall: gravity 1/8 per update up to the 6.0 cap; a hit while falling also stops it.
    {
        SotnWeaponSecondaryInput deep = in;
        deep.collision = (SotnCollisionMap){0};
        deep.query = DeepFloor;
        SotnWeaponSecondary w = {.parameter = 0, .x = 300 * 65536, .y = 20 * 65536};
        CHECK(SotnWeaponSecondary_Tick(&w, &program, deep, &rng, &e));
        for (Sint32 k = 1; w.phase == 1 && k < 200; ++k)
        {
            CHECK(SotnWeaponSecondary_Tick(&w, &program, deep, &rng, &e));
            CHECK(w.phase != 1 || w.dawn_velocity_y == SDL_min(0x2000 * k, 0x60000));
        }
        CHECK(w.phase == 2 && Pixel(w.y) == 1000 - 25);
        SotnWeaponSecondary h = {.parameter = 0, .x = 300 * 65536, .y = 20 * 65536};
        CHECK(SotnWeaponSecondary_Tick(&h, &program, deep, &rng, &e) && h.phase == 1);
        h.hit_flags = 1;
        e = (SotnWeaponSecondaryEvents){0};
        CHECK(SotnWeaponSecondary_Tick(&h, &program, deep, &rng, &e) && h.phase == 3 &&
              e.sounds[1] == 0x634 && e.factory_count == 1);
    }
    // Afterimage: the raw parent's frame, transparent, fading by two until poseTimer passes zero.
    SotnWeaponSecondary a = {.parameter = (Uint16)((hand << 15) | 0x100), .x = 200 * 65536};
    check = rng;
    CHECK(SotnWeaponSecondary_Tick(&a, &program, in, &rng, &e) && rng == check && a.phase == 4 &&
          a.frame == 0x22 && a.flags == 0x08000000U && a.draw_flags == 8 && a.blend == 0x10 &&
          a.opacity == 0x80 && a.remaining == 0x18 && a.priority == 148 && !a.hitbox_state);
    for (t = 0; !a.removed && t < 40; ++t)
    {
        CHECK(SotnWeaponSecondary_Tick(&a, &program, in, &rng, &e));
        CHECK(a.removed || a.opacity == 0x80 - 2 * (t + 1));
    }
    CHECK(a.removed && t == 25);
    return true;
}

// w_025 EntityWeaponShieldSpell: Goddess's seventeen rows. Its fifteen band angles load once from
// the program and keep stepping (0x80, then 0x20); every tenth timer value in phases2..4
// requests a FACTORY(90,1) cloud; TRANSFORM_3X reveals the bands, the card narrows to 32x120,
// darkens, waits 0x40 updates for the pink15 effect, brightens and spreads before the spell ends
// the freeze itself.
static bool GoddessSpellTest(Uint16 hand)
{
    static Uint8 o[0x1400], exe[0x24000];
    Math(exe);
    for (Uint16 i = 0; i < 15; ++i)
        Put32(o + 0xa98 + i * 4U, i * 0x111U);
    SotnHandWeaponProgram program = {0};
    SotnWeaponSecondary s = {.parameter = (Uint16)(hand << 15), .x = 128 * 65536, .y = 120 * 65536};
    SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                   .executable = {exe, sizeof(exe)},
                                   .hand = hand,
                                   .family = 25,
                                   .role = 8,
                                   .priority = 148,
                                   .facing_left = hand != 0,
                                   .allocated = true,
                                   .free_factories = 4};
    Uint32 rng = 5;
    SotnWeaponSecondaryEvents e = {0};
    CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == 5 && s.phase == 1 &&
          e.clear_shield && e.sounds[0] == 0x641 && e.freeze_set && !e.factory_count &&
          s.spell_quads[1].u[0] == 0x60 && s.spell_quads[1].v[2] == (hand ? 0xf7 : 0x77) &&
          s.spell_quads[1].priority == 0x1b8 && s.spell_quads[1].draw_mode == 8 &&
          s.spell_quads[2].v[0] == (hand ? 0x80 : 0) &&
          s.spell_quads[16].v[0] == (hand ? 0xf0 : 0x70) &&
          s.spell_quads[16].v[2] == (hand ? 0xf7 : 0x77) && s.spell_quads[16].palette == 0x15f &&
          s.spell_quads[16].priority == 0x1ba && s.spell_quads[16].draw_mode == 0x3d &&
          s.spell_half_h == 0x78 && s.spell_wave_x == 0x20 && program.shield_angles_loaded &&
          program.shield_angles[14] == 14 * 0x111 + 0x80 && program.shield_angles[15] == 0 &&
          SotnWeaponSecondary_RoleRows(25, 8) == 17 && SotnWeaponSecondary_RoleRows(25, 9) == 1 &&
          SotnWeaponSecondary_Rows(25) == 17 && SotnWeaponSecondary_ShieldSpellItem(25) == 209 &&
          SotnWeaponSecondary_ShieldSpellFamily(209) == 25);
    Uint16 t = 0, counts[10] = {0}, clouds = 0, transform = 0, pink = 0;
    Sint32 angle = program.shield_angles[3];
    for (; !s.removed && t < 600; ++t)
    {
        in.engine_timer = t;
        const Uint8 phase = s.phase;
        const Sint16 wave = s.spell_wave_x, tall = s.spell_half_h;
        e = (SotnWeaponSecondaryEvents){0};
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == 5);
        ++counts[phase];
        // Phase4's wave loses one on every eighth and every second g_Timer value (floor zero);
        // phase8 grows it again by one and the card by two.
        if (phase == 4)
            CHECK(s.spell_wave_x == SDL_max(SDL_max(wave - !(t & 7U), 0) - !(t & 1U), 0));
        if (phase == 8)
            CHECK(s.spell_wave_x == wave + 1 && s.spell_half_h == tall + 2);
        // Adjacent bands share edges: band i's lower corners use the angle band i+1's upper ones
        // read before stepping.
        if (phase == 4 && s.spell_wave_x)
            for (Uint16 k = 2; k < 16; ++k)
                CHECK(s.spell_quads[k].x[2] == s.spell_quads[k + 1].x[0] &&
                      s.spell_quads[k].red[2] == s.spell_quads[k + 1].red[0]);
        // The band loop adds the step current after the phase work (phase7 lowers it to 0x20).
        CHECK(s.removed || program.shield_angles[3] == angle + s.spell_step);
        angle = program.shield_angles[3];
        // Clouds follow the timer in phases2..4 only.
        const bool cloud = phase >= 2 && phase <= 4 && !(s.spell_timer % 10);
        CHECK(e.factory_count == (cloud ? 1 : 0));
        if (cloud)
        {
            CHECK(e.factories[0] == ((90U + ((hand + 1U) << 14)) | 1U << 16) &&
                  e.factory_x[0] == s.x && e.factory_y[0] == s.y);
            ++clouds;
        }
        if (e.sounds[0] == 0x6b1)
        {
            CHECK(phase == 3 && !s.frame && !(s.spell_quads[16].draw_mode & 8U));
            ++transform;
        }
        if (phase == 4 && s.phase == 5)
            CHECK(s.spell_half_w == 16 && s.spell_half_h == 60 && !s.spell_wave_x &&
                  s.spell_quads[1].y[2] - s.spell_quads[1].y[0] == 119 &&
                  s.spell_quads[9].y[0] - s.spell_quads[2].y[0] == 56 &&
                  !(s.spell_quads[1].draw_mode & 8U));
        if (phase >= 4 && phase < 9)
            CHECK(s.spell_quads[5].red[0] == s.spell_quads[5].green[0] &&
                  s.spell_quads[5].green[2] == s.spell_quads[5].blue[3]);
        if (e.pink)
        {
            CHECK(e.pink == 15 && phase == 6 && s.phase == 7 && !s.spell_bright);
            ++pink;
        }
        if (phase == 7 && s.phase == 8)
            CHECK((s.spell_quads[1].draw_mode & 8U) && s.spell_step == 0x20 &&
                  s.spell_bright > 0x120);
        CHECK(e.freeze_clear == s.removed);
    }
    // Growth22, hold8, flash16, darkening26, wait0x40, brightening19, spread16, fade0x40.
    CHECK(s.removed && transform == 1 && pink == 1 && clouds >= 6 && counts[1] == 22 &&
          counts[2] == 8 && counts[3] == 16 && counts[4] > 40 && counts[5] == 26 &&
          counts[6] == 0x40 && counts[7] == 19 && counts[8] == 16 && counts[9] == 0x40);
    // A second cast continues the retained angles instead of reloading them.
    SotnWeaponSecondary again = {.parameter = (Uint16)(hand << 15), .x = 128 * 65536};
    const Sint32 kept = program.shield_angles[0];
    CHECK(SotnWeaponSecondary_Tick(&again, &program, in, &rng, &e) &&
          program.shield_angles[0] == kept + 0x80);
    return true;
}
// w_025 func_ptr_80170024: the cloud takes its row first, then six rand calls place it around
// the raw parent, choose its priority, first timer, spin and orbit angle; it grows by eight,
// then by two, re-aims every fourth update at a point circling 0x20 above the parent and fades
// by six once the parent slot is empty.
// Floor shift (MIPS sra) for the cloud checks.
static Sint32 Sra(Sint32 v, unsigned int n)
{
    return v >= 0 ? v >> n : -(Sint32)((((Uint32)-v) + (1U << n) - 1U) >> n);
}
// The original aim (every fourth timer value, with a parent) and corners, from its constants:
// orbit 48x64 around parent-0x20, speed 8, corners at rotate+0x639/0x1C7/0x9C7/0xE39 by 0x43.
static bool CloudExpected(const SotnWeaponSecondary *s, Uint8 *exe, Sint32 cx, Sint32 cy, Sint32 px,
                          Sint32 py, bool aimed)
{
    const SotnDataView x = {exe, 0x24000};
    SotnMathTrig t = {0};
    if (aimed)
    {
        // The aim reads the position before this update's step.
        const Sint32 bx = (Sint32)((Uint32)s->x - (Uint32)s->velocity_x),
                     by = (Sint32)((Uint32)s->y - (Uint32)s->spell_velocity_y);
        CHECK(SotnMath_Trig(x, s->cloud_angle, &t));
        const Sint32 tx = px - cx + Sra(Sra(t.cosine, 4) * 48, 8);
        const Sint32 ty = py - cy - 0x20 - Sra(Sra(t.sine, 4) * 64, 8);
        Sint16 heading = 0;
        CHECK(SotnMath_Atan2Vector(x, -(ty - (Pixel(by) - cy)), tx - (Pixel(bx) - cx), &heading) &&
              SotnMath_Trig(x, heading, &t));
        CHECK(s->velocity_x == t.cosine * 8 && s->spell_velocity_y == -t.sine * 8);
    }
    const Sint16 w = (Sint16)(s->scale_x * 0x43 / 256), h = (Sint16)(s->scale_y * 0x43 / 256);
    const Sint16 corner[4] = {0x639, 0x1c7, 0x9c7, 0xe39};
    const Sint32 sx = Pixel(s->x) - cx, sy = Pixel(s->y) - cy;
    for (Uint16 k = 0; k < 4; ++k)
    {
        CHECK(SotnMath_Trig(x, (Sint16)(s->rotate + corner[k]), &t));
        CHECK(s->spell_quads[0].x[k] == (Sint16)(sx + Sra(Sra(t.cosine, 4) * w, 8)) &&
              s->spell_quads[0].y[k] == (Sint16)(sy - Sra(Sra(t.sine, 4) * h, 8)));
    }
    return true;
}
static bool GoddessCloudTest(Uint16 hand)
{
    static Uint8 o[0x1400], exe[0x24000];
    Math(exe);
    for (Uint32 i = 0; i <= 1024; ++i)
    {
        Put16(exe + 0x1dbcc + i * 2, (Uint16)(i * 4));
        // An authored monotonic arctangent table so headings follow the aim point.
        Put16(exe + 0x22a10 + i * 2, (Uint16)(i / 2));
    }
    SotnHandWeaponProgram program = {0};
    SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                   .executable = {exe, sizeof(exe)},
                                   .hand = hand,
                                   .family = 25,
                                   .role = 9,
                                   .priority = 148,
                                   .facing_left = hand != 0,
                                   .camera_x = 30,
                                   .camera_y = 10,
                                   .parent_exists = true,
                                   .parent_x = 200 * 65536,
                                   .parent_y = 120 * 65536};
    Uint32 rng = 77, check = 77;
    SotnWeaponSecondaryEvents e = {0};
    {
        SotnWeaponSecondary cold = {.parameter = 0x100};
        CHECK(SotnWeaponSecondary_Tick(&cold, &program, in, &rng, &e) && cold.removed &&
              rng == check);
    }
    in.allocated = true;
    SotnWeaponSecondary s = {
        .parameter = 0x100, .x = 210 * 65536 + 0x1234, .y = 90 * 65536, .flags = 0x10000};
    Uint16 r[6] = {0};
    for (Uint16 k = 0; k < 6; ++k)
        r[k] = (Uint16)(SotnCombat_Random(&check) & 0x7fffU);
    CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == check && s.phase == 1 &&
          !e.factory_count && !e.sounds[0] && !e.properties && s.flags == 0x04810000U &&
          s.spell_quads[0].palette == (hand ? 0x12a : 0x112) &&
          s.spell_quads[0].v[0] == (hand ? 0xa8 : 0x28) &&
          s.spell_quads[0].v[2] == (hand ? 0xff : 0x7f) && s.spell_quads[0].u[1] == 0x5f &&
          s.spell_quads[0].priority == ((r[2] & 3U) ? 0x1bc : 0x1b6) &&
          s.spell_quads[0].draw_mode == 0x35 && s.spell_timer == (Sint16)((r[3] & 7U) + 7U) &&
          s.cloud_spin == (Sint16)((Sint32)(r[4] % 16U) - 8 + ((r[4] % 16U) >= 8 ? 1 : 0)) &&
          s.cloud_angle == (Sint16)(r[5] + 0x40) && s.cloud_spin && s.rotate == s.cloud_spin);
    // Birth placement: parent +rand%160-80, +rand%192-64 (before the first step).
    const Sint32 bx = 200 + (Sint32)(r[0] % 160U) - 80, by = 120 + (Sint32)(r[1] % 192U) - 64;
    CHECK(s.x == (Sint32)(((Uint32)bx << 16) | 0x1234U) + s.velocity_x &&
          s.y == (Sint32)((Uint32)by << 16) + s.spell_velocity_y);
    Uint16 t = 0, grow = 0, aims = 0;
    Sint32 nearest = 100000;
    for (; s.phase < 3 && t < 400; ++t)
    {
        const Uint8 phase = s.phase;
        const Sint16 scale = s.scale_x;
        const Sint32 vx = s.velocity_x;
        check = rng;
        Uint32 roll = rng;
        const Uint16 next = (Uint16)(SotnCombat_Random(&roll) & 0x7fffU);
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
        if (phase == 1 && s.phase == 2)
            CHECK(s.spell_timer == (Sint16)((next & 0x3fU) + 0x60U - 1U));
        CHECK(CloudExpected(&s, exe, 30, 10, 200, 120, !(s.spell_timer & 3) && s.phase < 3));
        CHECK(s.scale_x == scale + (phase == 1 ? 8 : 2) && s.scale_y == s.scale_x);
        grow += phase == 1;
        // The timer rolls again only when phase1 ends.
        CHECK(rng == (phase == 1 && s.phase == 2 ? (SotnCombat_Random(&check), check) : check));
        aims += vx != s.velocity_x;
        CHECK(SDL_abs(s.velocity_x) <= 0x8000 && SDL_abs(s.spell_velocity_y) <= 0x8000);
        const Sint32 dx = Pixel(s.x) - 200, dy = Pixel(s.y) - (120 - 0x20);
        nearest = SDL_min(nearest, dx * dx + dy * dy);
        // Corners rotate around the cloud by its scale (0x43 at full size).
        const Sint32 cx = Pixel(s.x) - 30, half = s.scale_x * 0x43 / 256 + 2;
        for (Uint16 k = 0; k < 4; ++k)
            CHECK(SDL_abs(s.spell_quads[0].x[k] - cx) <= half);
    }
    CHECK(s.phase == 3 && grow >= 8 && grow <= 16 && aims > 4 && nearest < 40 * 40);
    // The parent slot empties: the next aim stops it and it fades by six.
    in.parent_exists = false;
    for (t = 0; !s.removed && t < 60; ++t)
    {
        const Uint8 blue = s.spell_quads[0].blue[3];
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
        // An aim without a parent stops it.
        CHECK(s.removed ||
              (s.spell_quads[0].blue[3] == blue - 6 && ((s.spell_timer & 3) || !s.velocity_x) &&
               s.spell_quads[0].red[0] == s.spell_quads[0].blue[3]));
    }
    CHECK(s.removed && t == 21);
    // An absent parent at an aim also ends growth (phase1 to3).
    SotnWeaponSecondary g = {.parameter = 0x100, .x = 210 * 65536, .y = 90 * 65536};
    in.parent_exists = true;
    CHECK(SotnWeaponSecondary_Tick(&g, &program, in, &rng, &e) && g.phase == 1);
    in.parent_exists = false;
    for (t = 0; g.phase == 1 && t < 8; ++t)
        CHECK(SotnWeaponSecondary_Tick(&g, &program, in, &rng, &e));
    CHECK(g.phase == 3 && !(g.spell_timer & 3));
    // The spin skips zero: a roll of eight (rand%16-8 == 0) spins by one.
    for (Uint32 seed = 1;; ++seed)
    {
        Uint32 probe = seed;
        for (Uint16 k = 0; k < 4; ++k)
            (void)SotnCombat_Random(&probe);
        if ((SotnCombat_Random(&probe) & 0x7fffU) % 16U != 8U)
            continue;
        SotnWeaponSecondary z = {.parameter = 0x100, .x = 210 * 65536, .y = 90 * 65536};
        Uint32 seeded = seed;
        CHECK(SotnWeaponSecondary_Tick(&z, &program, in, &seeded, &e) && z.cloud_spin == 1);
        break;
    }
    return true;
}

// Runtime: Mablung/Axe Lord's blueprint86 E8/F8 spell (ten rows) creates, at its release, the
// blueprint88 E9/F9 walker in17..31 (the spell's item, func_80118894's least-used group, no
// parent copy, params without the hand bit); its blueprint90 afterimages (32..63) take no
// property roll and keep equipId0.
// The w_010 spoke: ((rcos/rsin >> 4) * radius) >> 8 from a whole-pixel centre.
static Sint16 IronModelSpoke(Sint16 centre, Sint32 trig, Sint32 radius, bool subtract)
{
    const Sint32 a = trig >= 0 ? trig >> 4 : -((-trig + 15) >> 4);
    const Sint32 m = a * radius, d = m >= 0 ? m >> 8 : -((-m + 255) >> 8);
    return (Sint16)(subtract ? centre - d : centre + d);
}
static Uint8 IronModelGlow(SotnDataView exe, Sint16 angle, Sint16 bright)
{
    SotnMathTrig t = {0};
    (void)SotnMath_Trig(exe, angle, &t);
    return (Uint8)(((t.sine + 0x1000) >> 6) * bright / 256);
}
static void IronTables(Uint8 *o)
{
    for (Uint16 i = 0; i < 16; ++i)
    {
        Put16(o + 0xbbc + i * 2U, (Uint16)(i * 0x300U));
        Put16(o + 0xbdc + i * 2U, (Uint16)(0x100U + i * 0x250U));
    }
    for (Uint16 i = 0; i < 6; ++i)
    {
        Put16(o + 0xba0 + i * 4U, 2);
        Put16(o + 0xba2 + i * 4U, (Uint16)(0xc3f + i));
    }
    Put16(o + 0xbb8, 0xffff);
}
// w_010 EntityWeaponShieldSpell against an exact model: birth rows, the step timeline (22/8/16
// growth-hold-flash, 30 opening, 30 closing with the two swords, 32 dimming, 0x40 hold, 18
// spreading, 28 fading), every update's spoke geometry and the copies' retained-angle colours.
static bool IronSpellTest(void)
{
    static Uint8 o[0x3000], exe[0x24000];
    Math(exe);
    IronTables(o);
    const SotnDataView x_exe = {exe, sizeof(exe)};
    for (Uint16 k = 0; k < 5; ++k)
    {
        const Uint16 hand = k & 1U;
        const bool left = k == 1 || k == 2, starved = k == 3, failed = k == 4;
        SotnHandWeaponProgram program = {0};
        SotnWeaponSecondary s = {.parameter = (Uint16)(hand << 15),
                                 .x = (120 + 30) * 65536 + 0x1357,
                                 .y = (100 + 10) * 65536 + 0x2468};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = x_exe,
                                       .camera_x = 99,
                                       .camera_y = 77,
                                       .retained_camera_x = 30,
                                       .retained_camera_y = 10,
                                       .hand = hand,
                                       .family = 10,
                                       .role = 8,
                                       .priority = 148,
                                       .facing_left = left,
                                       .allocated = !failed,
                                       .free_factories = (Uint8)(starved ? 1 : 8)};
        Sint16 angles[16] = {0};
        for (Uint16 i = 0; i < 16; ++i)
            angles[i] = (Sint16)(i * 0x300);
        Uint32 rng = 5;
        Uint16 tick = 0, transitions[10] = {0}, sounds = 0, heard[4] = {0};
        Uint8 phase = 0;
        bool born = false;
        while (!s.removed && tick < 400)
        {
            in.engine_timer = tick;
            SotnWeaponSecondaryEvents e = {0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == 5);
            if (failed)
            {
                CHECK(s.removed && !e.freeze_set && !program.iron_loaded == false);
                break;
            }
            if (!born)
            {
                born = true;
                CHECK(e.freeze_set && e.clear_shield && e.sounds[0] == 0x641 && s.phase == 1 &&
                      s.spell_half_w == 2 && !s.spell_half_h && s.spell_step == 0xc0 &&
                      s.spell_bright == 0x100 && s.spell_child_palette == (hand ? 0x129 : 0x111) &&
                      s.flags == 0x04810000U && s.priority == 146);
                for (Uint16 i = 0; i < 16; ++i)
                {
                    SotnMathTrig a = {0}, b = {0};
                    CHECK(SotnMath_Trig(x_exe, (Sint16)((i + 1) * 0x100), &a) &&
                          SotnMath_Trig(x_exe, (Sint16)(i * 0x100), &b));
                    const SotnPlayerEffectQuad *q = &s.spell_quads[1 + i * 2];
                    const Sint32 flip = left ? -1 : 1;
                    CHECK(q->u[0] == 0xc0 && q->u[2] == 0xc0 &&
                          q->u[1] ==
                              (Uint8)(IronModelSpoke(0, a.cosine, 0x3e, false) * flip + 0xc0) &&
                          q->u[3] ==
                              (Uint8)(IronModelSpoke(0, b.cosine, 0x3e, false) * flip + 0xc0) &&
                          q->v[1] ==
                              (Uint8)(s.spell_v1 + 0x40 - IronModelSpoke(0, a.sine, 0x3e, false)) &&
                          q->v[3] ==
                              (Uint8)(s.spell_v1 + 0x40 - IronModelSpoke(0, b.sine, 0x3e, false)) &&
                          q->priority == 0x1b8 && q->draw_mode == 0x08 &&
                          q->palette == s.spell_child_palette);
                    const SotnPlayerEffectQuad *c = &s.spell_quads[2 + i * 2];
                    CHECK(c->priority == 0x1ba && c->palette == 0x15f && c->draw_mode == 0x35 &&
                          c->u[1] == q->u[1] && c->v[3] == q->v[3]);
                }
            }
            if (s.phase != phase)
            {
                transitions[s.phase] = tick;
                phase = s.phase;
            }
            if (e.sounds[0] == 0x66a)
                CHECK(s.phase == 4 && transitions[4] == tick && !s.frame);
            if (e.sounds[0] == 0x636)
                CHECK(s.phase == 9 && transitions[9] == tick);
            if (e.sounds[0] && sounds < 4)
                heard[sounds] = e.sounds[0];
            sounds += e.sounds[0] != 0;
            if (e.factory_count || e.freeze_clear)
            {
                CHECK(e.freeze_clear && s.phase == 6 && e.factory_count == (starved ? 1 : 2) &&
                      e.factories[0] == (88U | (hand + 1U) << 14) &&
                      (starved || e.factories[1] == (88U | (hand + 1U) << 14 | 1U << 16)));
            }
            if (s.removed)
                break;
            // Geometry and the copies' colours from the model angles.
            const Sint16 x = (Sint16)((s.x >> 16) - 30), y = (Sint16)((s.y >> 16) - 10);
            for (Uint16 i = 0; i < 16; ++i)
            {
                SotnMathTrig a = {0}, b = {0};
                CHECK(SotnMath_Trig(x_exe, (Sint16)((i + 1) * 0x100), &a) &&
                      SotnMath_Trig(x_exe, (Sint16)(i * 0x100), &b));
                const SotnPlayerEffectQuad *q = &s.spell_quads[1 + i * 2],
                                           *c = &s.spell_quads[2 + i * 2];
                CHECK(q->x[0] == IronModelSpoke(x, a.cosine, s.spell_half_h, false) &&
                      q->y[0] == IronModelSpoke(y, a.sine, s.spell_half_h, true) &&
                      q->x[1] == IronModelSpoke(x, a.cosine, s.spell_half_w, false) &&
                      q->y[1] == IronModelSpoke(y, a.sine, s.spell_half_w, true) &&
                      q->x[2] == IronModelSpoke(x, b.cosine, s.spell_half_h, false) &&
                      q->x[3] == IronModelSpoke(x, b.cosine, s.spell_half_w, false) &&
                      q->y[3] == IronModelSpoke(y, b.sine, s.spell_half_w, true));
                CHECK(c->draw_mode == 0x35 && c->priority == 0x1ba && c->palette == 0x15f &&
                      !c->red[2] && !c->blue[3] && !c->red[0] && !c->blue[1] &&
                      c->red[3] == IronModelGlow(x_exe, angles[i], s.spell_bright) &&
                      c->green[3] == c->red[3] &&
                      c->red[1] == IronModelGlow(x_exe, angles[(i + 1) % 16], s.spell_bright) &&
                      c->x[0] == q->x[0] && c->u[1] == q->u[1]);
                angles[i] = (Sint16)(angles[i] + 0xc0);
            }
            for (Uint16 i = 0; i < 16; ++i)
                CHECK(program.iron_angles[i] == angles[i] &&
                      program.iron_angles2[i] == (Sint16)(0x100 + i * 0x250));
            // Bands stay hidden until the flash ends and again once the spread completes.
            CHECK(((s.spell_quads[1].draw_mode & 8U) != 0) == (s.phase < 4 || s.phase == 9));
            ++tick;
        }
        if (failed)
            continue;
        // Birth is update0; each phase's first update follows.
        const Uint16 expected[10] = {0, 0, 22, 30, 46, 76, 106, 138, 202, 220};
        for (Uint16 ph = 2; ph < 10; ++ph)
            CHECK(transitions[ph] == expected[ph]);
        CHECK(s.removed && tick == 220 + 28 && sounds == 3 && heard[0] == 0x641 &&
              heard[1] == 0x66a && heard[2] == 0x636);
    }
    return true;
}
// w_010 func_ptr_80170024 against an exact model: birth copies and rows, the 0x20 delay (then
// CAMERA_LOCKED without KEEP_ALIVE), the drifting centre, opening radius, spin, scrape sound,
// interpolated slices and shieldAngles2 colours.
static bool IronSwordTest(void)
{
    static Uint8 o[0x3000], exe[0x24000];
    Math(exe);
    IronTables(o);
    const SotnDataView x_exe = {exe, sizeof(exe)};
    const Uint16 hit = 2;
    for (Uint16 k = 0; k < 8; ++k)
    {
        const Uint16 hand = k & 1U, upper = (k >> 1) & 1U;
        const bool left = (k & 4U) != 0;
        SotnHandWeaponProgram program = {0};
        SotnWeaponSecondary s = {.parameter = (Uint16)(upper << 8),
                                 .x = (140 + 30) * 65536 + 0x4321,
                                 .y = (90 + 10) * 65536 + 0x1111};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = x_exe,
                                       .properties = Properties,
                                       .context = (void *)&hit,
                                       .camera_x = 70,
                                       .retained_camera_x = 30,
                                       .retained_camera_y = 10,
                                       .hand = hand,
                                       .family = 10,
                                       .role = 9,
                                       .allocated = true,
                                       .parent_exists = true,
                                       .parent_facing_left = left,
                                       .parent_child_palette = (Uint16)(hand ? 0x129 : 0x111),
                                       .parent_band = (Uint8)(hand ? 0x80 : 0)};
        Uint32 rng = 3, check = 3;
        (void)SotnCombat_Random(&check);
        SotnWeaponSecondaryEvents e = {0};
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == check &&
              e.properties && e.sounds[0] == 0x69d && s.facing_left == left &&
              s.spell_child_palette == (hand ? 0x129 : 0x111) + 1U + upper &&
              s.spell_v1 == (hand ? 0x80 : 0) && s.flags == 0x04800000U && s.hitbox_width == 0x10 &&
              s.hitbox_height == 0x10 && s.iron_delay == 0x20 && s.spell_step == 0x80 &&
              s.iron_vx == (left ? -0x24000 : 0x24000));
        const bool reverse = ((left ? 1U : 0U) ^ upper) & 1U;
        CHECK(s.iron_step == (reverse ? 0x100 : -0x100) && s.iron_angle == (reverse ? 0 : 0x800) &&
              s.rotate == (reverse ? 0x600 : 0x200));
        const SotnPlayerEffectQuad *blade = &s.spell_quads[1];
        CHECK(blade->u[0] == 0 && blade->u[1] == 0x7f && blade->v[0] == s.spell_v1 + 0x30 &&
              blade->v[2] == s.spell_v1 + 0x4f && blade->priority == 0x1bc && !blade->draw_mode &&
              blade->palette == s.spell_child_palette);
        for (Uint16 i = 0; i < 16; ++i)
        {
            const SotnPlayerEffectQuad *c = &s.spell_quads[2 + i];
            CHECK(c->u[0] == i * 8 && c->u[1] == (i + 1) * 8 - 1 && c->priority == 0x1be &&
                  c->palette == 0x15f && c->draw_mode == 0x35);
        }
        Sint32 cx = (140) * 65536 + 0x4321, cy = 90 * 65536 + 0x1111;
        Sint16 radius = 0, angle = s.iron_angle, rotate = s.rotate, bright = 0x100;
        Sint16 angles2[16] = {0};
        for (Uint16 i = 0; i < 16; ++i)
            angles2[i] = (Sint16)(0x100 + i * 0x250);
        Uint16 scrapes = 0;
        for (Uint16 t = 0; t < 120; ++t)
        {
            // Model this update (the birth update already coloured from the initial angles).
            if (t)
            {
                bright = (Sint16)SDL_max(bright - 8, 0);
                if (t <= 0x20)
                    ;
                else
                {
                    if (radius < 0x280)
                        radius = (Sint16)(radius + 0x18);
                    rotate = (Sint16)(rotate + s.iron_step * 7 / 4);
                    angle = (Sint16)(angle + s.iron_step);
                    cx += s.iron_vx;
                }
            }
            SotnMathTrig a = {0};
            CHECK(SotnMath_Trig(x_exe, angle, &a));
            const Sint32 sx = cx + a.cosine * radius, sy = cy - a.sine * radius;
            CHECK(s.x == sx + 30 * 65536 && s.y == sy + 10 * 65536 && s.rotate == rotate &&
                  s.iron_radius == radius && s.spell_bright == bright &&
                  s.flags == (t >= 0x20 ? 0x08800000U : 0x04800000U) &&
                  s.spell_quads[0].x[0] == (Sint16)(cx >> 16));
            Sint16 px[4] = {0}, py[4] = {0};
            const Uint16 corner[4] = {0x787, 0x79, 0x879, 0xf87};
            for (Uint16 c = 0; c < 4; ++c)
            {
                SotnMathTrig ct = {0};
                CHECK(SotnMath_Trig(x_exe, (Sint16)((Uint16)rotate + corner[c]), &ct));
                px[c] = IronModelSpoke((Sint16)(sx >> 16), ct.cosine, 0x42, false);
                py[c] = IronModelSpoke((Sint16)(sy >> 16), ct.sine, 0x42, true);
                CHECK(s.spell_quads[1].x[c] == px[c] && s.spell_quads[1].y[c] == py[c]);
            }
            for (Uint16 i = 0; i < 16; ++i)
            {
                const SotnPlayerEffectQuad *c = &s.spell_quads[2 + i];
                CHECK(c->x[0] == (Sint16)(px[0] + (px[1] - px[0]) * i / 16) &&
                      c->x[1] == (Sint16)(px[0] + (px[1] - px[0]) * (i + 1) / 16) &&
                      c->x[3] == (Sint16)(px[2] + (px[3] - px[2]) * (i + 1) / 16) &&
                      c->y[2] == (Sint16)(py[2] + (py[3] - py[2]) * i / 16) &&
                      c->red[0] == IronModelGlow(x_exe, angles2[i], bright) &&
                      c->red[1] == IronModelGlow(x_exe, angles2[(i + 1) % 16], bright));
                angles2[i] = (Sint16)(angles2[i] + 0x80);
                CHECK(program.iron_angles2[i] == angles2[i]);
            }
            const Sint16 previous = s.iron_angle;
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && !e.properties);
            const bool flip = t + 1 > 0x20 && ((previous ^ s.iron_angle) & 0x1000) && upper;
            CHECK(e.sounds[0] == (flip ? 0x689 : 0));
            scrapes += flip;
        }
        CHECK(scrapes == (upper ? 5 : 0));
        in.parent_exists = false;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.removed);
    }
    // Thunder: the high words follow the player, fractions stay; birth copies and THUNDER_A.
    {
        static Uint8 to[0x3000];
        IronTables(to);
        SotnHandWeaponProgram program = {0};
        SotnWeaponSecondary s = {.x = 5 * 65536 + 0x1234, .y = 6 * 65536 + 0x5678};
        SotnWeaponSecondaryInput in = {.overlay = {to, sizeof(to)},
                                       .x = 150 * 65536 + 0xaaaa,
                                       .y = 140 * 65536 + 0xbbbb,
                                       .family = 10,
                                       .role = 1,
                                       .parent_exists = true,
                                       .parent_bank = 0x8012,
                                       .parent_texture = 0x66,
                                       .parent_priority = 146};
        Uint32 rng = 1;
        SotnWeaponSecondaryEvents e = {0};
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && e.sounds[0] == 0x664 &&
              s.x == 150 * 65536 + 0x1234 && s.y == 140 * 65536 + 0x5678 && s.bank == 0x8012 &&
              s.texture == 0x66 && s.priority == 150 && s.palette == 0x8194 &&
              s.flags == 0x160000U && s.phase == 1);
        Uint16 updates = 1;
        while (!s.removed && updates < 40)
        {
            CHECK(SotnWeaponSecondary_Animate(&s, (SotnDataView){to, sizeof(to)}, 10));
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && !e.sounds[0]);
            ++updates;
        }
        // Birth, six frames of two updates, then the END's negative poseTimer retires it.
        CHECK(s.removed && updates == 14);
        SotnWeaponSecondary gone = {.phase = 1};
        in.parent_exists = false;
        CHECK(SotnWeaponSecondary_Tick(&gone, &program, in, &rng, &e) && gone.removed);
    }
    return true;
}
// Iron shield's spell205 through the shared runtime: blueprint86's E8 takes its 33 real rows,
// freezes the scene, releases blueprint88's two E9 swords (18 rows each, item205 copied from the
// raw parent, least-used group, no enemyId copy) as the freeze ends; the swords retire with
// their parent and every row is released.
static bool IronRuntime(Uint16 hand)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(56, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    static Uint8 exe[0x24000];
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Math(exe);
    Uint8 *o = weapons + 10U * 0x7000U + 0x4000U;
    for (Uint16 i = 0; i < 16; ++i)
    {
        Put16(o + 0xbbc + i * 2U, (Uint16)(i * 0x300U));
        Put16(o + 0xbdc + i * 2U, (Uint16)(i * 0x300U));
    }
    FamilyDefinition(dra, 7, 9, 10, 128, 0, 0, 0);
    FamilyDefinition(dra, 205, 1, 10, 5, 0, 20, 0);
    Uint8 *record = dra + 0x500 + 205 * 52;
    record[18] = 0xba;
    record[19] = 86;
    record[21] = 0;
    record[22] = 1;
    const Uint8 blueprints[2][6] = {{8, 1, 0xc1, 0, 6, 0}, {9, 1, 0xc1, 0, 6, 0}};
    for (Uint16 b = 0; b < 2; ++b)
        SDL_memcpy(dra + 0xd1d4 + (86U + b * 2U) * 6U, blueprints[b], 6);
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    const Uint8 blink[6] = {11, 1, 0x81, 0, 0, 0};
    SDL_memcpy(dra + 0xd1d4 + 44U * 6U, blink, 6);
    for (Uint16 i = 0; i < 2; ++i)
    {
        Uint8 *q = dra + 0xd670 + (i ? 0x47U : 0x40U) * 20U;
        Put16(q + 6, 0x100);
        for (Uint16 k = 8; k <= 12; k += 2)
            Put16(q + k, 256);
        Put16(q + 14, 0x7002);
        Put16(q + 16, 0x31);
        Put16(q + 18, 512);
    }
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    CHECK(SotnCharacter_Create(actors->dra, &character));
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = {exe, sizeof(exe)},
                              .weapons = {weapons, 56U * 0x7000},
                              .weapon_family = {hand ? 57 : 10, hand ? 10 : 57},
                              .weapon_bound = {true, true},
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 200 * 65536,
                                         .y = 135 * 65536,
                                         .priority = 148,
                                         .facing_left = hand == 0}};
    const SotnPlayerFactorySource source = {
        .x = in.player.x, .y = in.player.y, .priority = 148, .facing_left = hand == 0};
    bool allocated = false;
    CHECK(SotnBatRuntime_RequestHand(r, source, 86U | (hand + 1U) << 14 | (Uint32)hand << 23, 205,
                                     &allocated) &&
          allocated);
    Uint16 spell = 0, spell_slot = 0, swords = 0, freezes = 0, released = 0, sword_kinds = 0;
    Uint8 groups[2] = {0};
    Sint32 delay_screen[SOTN_BAT_RUNTIME_SLOTS] = {0};
    Uint16 retained = 0;
    for (Uint16 t = 0; t < 700; ++t)
    {
        in.game_timer = t;
        in.engine_timer = t;
        // Every eighth update the camera moves right by two after the callbacks.
        in.screen_shift_x = (Sint16)(t % 8 == 7 ? -2 : 0);
        in.camera_x += t % 8 == 7 ? 2 : 0;
        in.freeze = r->freeze;
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        freezes += r->freeze_changed && r->freeze == 1;
        released += r->freeze_changed && r->freeze == 0;
        bool live = false;
        for (Uint16 i = 1; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        {
            const SotnBatRuntimeChild *c = &r->children[i];
            if (!c->weapon_secondary || c->state.secondary.family != 10 || !c->initialized)
                continue;
            live = true;
            const SotnWeaponSecondary *w = &c->state.secondary.motion;
            CHECK(SotnBatRuntime_SecondaryValid(c, i));
            if ((c->kind & 15U) == 8)
            {
                CHECK(c->kind == (hand ? 0xf8 : 0xe8) && c->primitive_count == 33 &&
                      c->state.secondary.item == 205 && c->state.secondary.own_item);
                ++spell;
                spell_slot = i;
            }
            else
            {
                const Uint16 n = (c->parameter >> 8) & 1U;
                CHECK(c->kind == (hand ? 0xf9 : 0xe9) && c->primitive_count == 18 &&
                      c->state.secondary.parent_slot == spell_slot + 16U &&
                      c->state.secondary.item == 205 && !c->state.secondary.own_item &&
                      c->group >= 3 && c->group <= 6 && w->properties.id == 205 &&
                      c->state.secondary.attack.active &&
                      w->spell_child_palette == (hand ? 0x129U : 0x111U) + 1U + n);
                groups[n] = c->group;
                // Its v band is the raw parent spell's unk7D.
                CHECK(w->spell_v1 == r->children[spell_slot].state.secondary.motion.spell_v1);
                // Before its delay ends (no CAMERA_LOCKED) it keeps its screen position.
                const Sint32 screen = (w->x >> 16) - in.camera_x;
                if (w->iron_delay && w->iron_delay < 0x1f && delay_screen[i])
                {
                    CHECK(screen == delay_screen[i]);
                    retained += in.screen_shift_x != 0;
                }
                delay_screen[i] = w->iron_delay ? screen : 0;
                sword_kinds |= (Uint16)(1U << n);
                ++swords;
            }
        }
        if (!live && spell && t > 10)
            break;
    }
    // Both swords take distinct least-used groups. Once their delay drops KEEP_ALIVE, a sword
    // whose wide orbit leaves the screen is culled early (as in the original).
    CHECK(spell > 150 && swords > 100 && sword_kinds == 3 && groups[0] != groups[1] &&
          freezes == 1 && released == 1 && retained > 0);
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        CHECK(!r->children[i].weapon_secondary && !r->children[i].primitive_count);
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        CHECK(!actors->primitives[i].owner);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}
// Iron shield's raised hit: slot16 requests WFACTORY(56,0)'s thunder then func_8010E168(0,0x60)
// (BlinkWhite0x15 and protection0x60, no healing); the E1 thunder follows the player with the
// shield's bank and palette0x194, plays its six frames and retires (or retires at once when
// slot16 stops holding item7).
static bool IronThunderRuntime(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(56, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    static Uint8 exe[0x24000];
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Math(exe);
    Uint8 *o = weapons + 10U * 0x7000U + 0x4000U;
    Put16(o + 0x400, 2);
    Put16(o + 0x402, 0x3e);
    Put16(o + 0x404, 0);
    Put16(o + 0x406, 0);
    for (Uint16 i = 0; i < 14; ++i)
        Put32(o + 0xb68 + i * 4U, 0x8017a400);
    for (Uint16 i = 0; i < 6; ++i)
    {
        Put16(o + 0xba0 + i * 4U, 2);
        Put16(o + 0xba2 + i * 4U, (Uint16)(0x3f + i));
    }
    Put16(o + 0xbb8, 0xffff);
    FamilyDefinition(dra, 7, 9, 10, 128, 0, 0, 0);
    Uint8 *record = dra + 0x500 + 7 * 52;
    record[18] = 0x63;
    record[21] = 0;
    const Uint8 blueprints[3][6] = {
        {0, 1, 0xc1, 0, 0x23, 0}, {1, 1, 0xc1, 0, 6, 0}, {11, 1, 0x81, 0, 0, 0}};
    const Uint16 ids[3] = {48, 56, 44};
    for (Uint16 b = 0; b < 3; ++b)
        SDL_memcpy(dra + 0xd1d4 + ids[b] * 6U, blueprints[b], 6);
    dra[0xd4b8 + 6] = dra[0xd4b9 + 6] = 16;
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    Uint8 *q = dra + 0xd670 + 0x15U * 20U;
    Put16(q + 6, 0x100);
    for (Uint16 k = 8; k <= 12; k += 2)
        Put16(q + k, 256);
    Put16(q + 14, 0x7002);
    Put16(q + 16, 0x31);
    Put16(q + 18, 512);
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    CHECK(SotnCharacter_Create(actors->dra, &character));
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = {exe, sizeof(exe)},
                              .weapons = {weapons, 56U * 0x7000},
                              .weapon_family = {10, 10},
                              .weapon_bound = {true, true},
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 200 * 65536 + 0x1234,
                                         .y = 135 * 65536 + 0x5678,
                                         .priority = 148,
                                         .buttons = SOTN_BUTTON_LEFT_HAND}};
    const SotnPlayerFactorySource source = {.x = in.player.x, .y = in.player.y, .priority = 148};
    bool allocated = false;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestHand(r, source, 0x1030, 7, &allocated) && allocated &&
          SotnBatRuntime_Tick(r, character, in) && r->children[0].hand_weapon &&
          r->children[0].state.hand.family == 10 && r->children[0].state.hand.motion.phase == 1 &&
          !r->shield_protection && SotnActors_CommitBatPrimitives(actors, r));
    // A raised hit: the thunder factory precedes the BlinkWhite0x15 factory.
    r->children[0].state.hand.motion.hit_flags = 1;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && r->shield_protection == 0x60 &&
          !r->shield_heal && !r->children[0].state.hand.motion.hit_flags &&
          SotnActors_CommitBatPrimitives(actors, r));
    Uint16 order[2] = {0}, made = 0;
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        if (r->factories[i].cursor.entity_id && made < 2)
            order[made++] = r->factories[i].cursor.blueprint;
    CHECK(made == 2 && order[0] == 56 && order[1] == 44);
    Uint16 frames = 0, thunder = 0;
    for (Uint16 t = 0; t < 30; ++t)
    {
        in.player.x += 65536;
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        bool live = false;
        for (Uint16 i = 1; i < 16; ++i)
        {
            const SotnBatRuntimeChild *c = &r->children[i];
            if (!c->weapon_secondary || c->state.secondary.family != 10 || !c->initialized)
                continue;
            const SotnWeaponSecondary *w = &c->state.secondary.motion;
            CHECK(SotnBatRuntime_SecondaryValid(c, i) && c->kind == 0xe1 &&
                  c->state.secondary.parent_slot == 16 && !c->state.secondary.item &&
                  !c->primitive_count && !c->state.secondary.attack.active &&
                  w->palette == 0x8194 && w->bank == 0x8010 && w->texture == 0x64 &&
                  w->priority == r->children[0].state.hand.motion.priority + 4U &&
                  w->flags == 0x160000U && (w->x >> 16) == (in.player.x >> 16) &&
                  w->frame >= 0x3f && w->frame <= 0x44);
            frames |= (Uint16)(1U << (w->frame - 0x3f));
            live = true;
            ++thunder;
        }
        if (!live && thunder)
            break;
    }
    CHECK(frames == 0x3f && thunder >= 12 && thunder <= 14);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}
static bool AxeLordRuntime(Uint16 hand)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(56, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    static Uint8 exe[0x24000];
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Math(exe);
    AxeAnimation(weapons + 11U * 0x7000U + 0x4000U);
    for (Uint16 y = 0; y < 16; ++y)
        for (Uint16 x = 0; x < 64; ++x)
            axe_tiles[(y * 64U + x) * 2] = (Uint8)(y >= 10 || x == 40);
    FamilyDefinition(dra, 8, 9, 11, 128, 0, 0, 0);
    FamilyDefinition(dra, 206, 1, 11, 5, 0, 20, 0);
    // The other hand's Shotel (QCF176 boomerangs, blueprint56 in17..31) later takes the
    // retired walker's slot while its afterimages still fade.
    FamilyDefinition(dra, 28, 0, 34, 12, 0, 0, 176);
    FamilyDefinition(dra, 176, 1, 34, 129, 0, 0, 0);
    dra[0x500 + 176 * 52 + 18] = 0x5d;
    dra[0x500 + 176 * 52 + 19] = 56;
    dra[0x500 + 176 * 52 + 21] = 0;
    dra[0x500 + 176 * 52 + 22] = 1;
    const Uint8 boomerang[6] = {1, 1, 0xc1, 0, 0x06, 0};
    SDL_memcpy(dra + 0xd1d4 + 56U * 6U, boomerang, 6);
    Uint8 *record = dra + 0x500 + 206 * 52;
    record[18] = 0xba;
    record[19] = 86;
    record[21] = 0;
    record[22] = 1;
    const Uint8 blueprints[3][6] = {
        {8, 1, 0xc1, 0, 6, 0}, {9, 1, 0xc1, 0, 6, 0}, {9, 1, 0xc1, 0, 0, 0}};
    for (Uint16 b = 0; b < 3; ++b)
        SDL_memcpy(dra + 0xd1d4 + (86U + b * 2U) * 6U, blueprints[b], 6);
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    // Freeze1's BlinkWhite0x47/0x40 (blueprint44) use authored profiles.
    const Uint8 blink[6] = {11, 1, 0x81, 0, 0, 0};
    SDL_memcpy(dra + 0xd1d4 + 44U * 6U, blink, 6);
    for (Uint16 i = 0; i < 2; ++i)
    {
        Uint8 *q = dra + 0xd670 + (i ? 0x47U : 0x40U) * 20U;
        Put16(q + 6, 0x100);
        for (Uint16 k = 8; k <= 12; k += 2)
            Put16(q + k, 256);
        Put16(q + 14, 0x7002);
        Put16(q + 16, 0x31);
        Put16(q + 18, 512);
    }
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    CHECK(SotnCharacter_Create(actors->dra, &character));
    SotnBatRuntimeInput in = {
        .dra = actors->dra,
        .executable = {exe, sizeof(exe)},
        .weapons = {weapons, 56U * 0x7000},
        .weapon_family = {hand ? 34 : 11, hand ? 11 : 34},
        .weapon_bound = {true, true},
        .food_collision = {{axe_tiles, sizeof(axe_tiles)}, {axe_types, 2}, 64, 16},
        .player = {.hp = 70,
                   .hp_max = 70,
                   .x = 200 * 65536,
                   .y = 135 * 65536,
                   .priority = 148,
                   .facing_left = hand == 0}};
    const SotnPlayerFactorySource source = {
        .x = in.player.x, .y = in.player.y, .priority = 148, .facing_left = hand == 0};
    bool allocated = false;
    CHECK(SotnBatRuntime_RequestHand(r, source, 86U | (hand + 1U) << 14 | (Uint32)hand << 23, 206,
                                     &allocated) &&
          allocated);
    Uint16 spell = 0, walker = 0, images = 0, image_slot = 0, freezes = 0, walker_slot = 0,
           foreign = 0;
    bool thrown = false;
    Uint16 shifted = 0;
    for (Uint16 t = 0; t < 600; ++t)
    {
        in.engine_timer = t;
        // After its freeze the spell keeps its screen position while the camera moves.
        Uint16 spell_slot = 0;
        for (Uint16 i = 1; i < 16; ++i)
            if (r->children[i].weapon_secondary && (r->children[i].kind & 15U) == 8 &&
                r->children[i].state.secondary.motion.phase == 6)
                spell_slot = i;
        const Sint32 before = spell_slot ? r->children[spell_slot].state.secondary.motion.x : 0;
        in.screen_shift_x = (Sint16)(spell_slot ? -3 : 0);
        if (walker && !thrown && !r->children[walker_slot].kind)
        {
            const Uint16 other = (Uint16)!hand;
            const Uint32 packed = 56U | (other + 1U) << 12 | (other ? 0x80U << 16 : 0U);
            for (Uint16 k = 0; k < 2; ++k)
                CHECK(SotnBatRuntime_RequestThrown(r, source, packed, 176, &allocated) &&
                      allocated);
            thrown = true;
        }
        // Afterimages read their raw parent only at birth: a foreign occupant is no error.
        foreign += thrown && r->children[walker_slot].weapon_secondary &&
                   r->children[walker_slot].state.secondary.family == 34 && images &&
                   r->children[image_slot].kind;
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        freezes += r->freeze_changed;
        if (spell_slot && r->children[spell_slot].state.secondary.motion.phase == 6)
        {
            CHECK(r->children[spell_slot].state.secondary.motion.x == before + 3 * 65536);
            ++shifted;
        }
        for (Uint16 i = 1; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        {
            const SotnBatRuntimeChild *c = &r->children[i];
            if (!c->weapon_secondary || c->state.secondary.family != 11 || !c->initialized)
                continue;
            const SotnWeaponSecondary *w = &c->state.secondary.motion;
            // Afterimages keep the factory's equipId0 (CheckChainLimit never counts them).
            CHECK(SotnBatRuntime_SecondaryValid(c, i) &&
                  c->state.secondary.item ==
                      ((c->kind & 15U) == 9 && (c->parameter & 0x7f00U) ? 0 : 206));
            if ((c->kind & 15U) == 8)
            {
                CHECK(i <= 15 && c->kind == (hand ? 0xf8 : 0xe8) && c->primitive_count == 10 &&
                      c->state.secondary.own_item && !w->properties.id);
                ++spell;
            }
            else if (!(c->parameter & 0x7f00U))
            {
                CHECK(i <= 15 && c->kind == (hand ? 0xf9 : 0xe9) && !c->primitive_count &&
                      !c->parameter && !c->state.secondary.own_item && c->group >= 3 &&
                      c->group <= 6 && w->properties.id == 206 && w->hitbox_width == 8 &&
                      c->state.secondary.attack.active && w->facing_left == (hand == 0) &&
                      w->palette == (hand ? 0x128 : 0x110) && w->bank == (hand ? 0x8012 : 0x8010));
                // A delivered enemy contact stops the walker on its next callback (CANDLE_HIT).
                if (walker == 10)
                    CHECK(SotnBatRuntime_Hit(r, i, c->generation, 1) &&
                          r->children[i].state.secondary.motion.hit_flags == 1);
                if (walker == 11)
                    CHECK(w->phase == 3);
                ++walker;
                walker_slot = i;
            }
            else
            {
                CHECK(i >= 16 && c->kind == (hand ? 0xf9 : 0xe9) && c->parameter == 0x100 &&
                      !w->properties.id && !c->state.secondary.attack.active && w->draw_flags == 8);
                ++images;
                image_slot = i;
            }
        }
    }
    // The walker crosses to the wall column and retires after its final animation.
    CHECK(spell > 100 && walker >= 11 && images > walker && image_slot && freezes >= 2 &&
          foreign > 0 && shifted > 10);
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        CHECK(!r->children[i].weapon_secondary && !r->children[i].primitive_count);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}

// Runtime: Mablung/Goddess's blueprint86 E8/F8 spell (seventeen rows) requests FACTORY(90,1)
// E9/F9 clouds (one row each, slots32..63, flag0x10000, params1, no property roll or attack)
// that read the spell's live position and fade out. Clouds and spell
// keep their screen position when the camera moves.
static bool GoddessRuntime(Uint16 hand, bool orphan)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(56, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    static Uint8 exe[0x24000];
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Math(exe);
    for (Uint32 i = 0; i <= 1024; ++i)
        Put16(exe + 0x1dbcc + i * 2, (Uint16)(i * 4));
    FamilyDefinition(dra, 11, 9, 25, 128, 0, 0, 0);
    FamilyDefinition(dra, 209, 10, 25, 5, 0, 40, 0);
    Uint8 *record = dra + 0x500 + 209 * 52;
    record[18] = 0xba;
    record[19] = 86;
    record[21] = 0;
    record[22] = 1;
    const Uint8 blueprints[4][6] = {{8, 1, 0xc1, 0, 6, 0},
                                    {9, 1, 0xc1, 0, 0, 0},
                                    {11, 1, 0x81, 0, 0, 0},
                                    {39, 1, 0x81, 0, 0, 0}};
    const Uint16 ids[4] = {86, 90, 44, 55};
    for (Uint16 b = 0; b < 4; ++b)
        SDL_memcpy(dra + 0xd1d4 + ids[b] * 6U, blueprints[b], 6);
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    for (Uint16 i = 0; i < 2; ++i)
    {
        Uint8 *q = dra + 0xd670 + (i ? 0x47U : 0x40U) * 20U;
        Put16(q + 6, 0x100);
        for (Uint16 k = 8; k <= 12; k += 2)
            Put16(q + k, 256);
        Put16(q + 14, 0x7002);
        Put16(q + 16, 0x31);
        Put16(q + 18, 512);
    }
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    CHECK(SotnCharacter_Create(actors->dra, &character));
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = {exe, sizeof(exe)},
                              .weapons = {weapons, 56U * 0x7000},
                              .weapon_family = {25, 25},
                              .weapon_bound = {true, true},
                              .camera_x = 40,
                              .camera_y = 8,
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 200 * 65536,
                                         .y = 135 * 65536,
                                         .priority = 148,
                                         .facing_left = hand == 0}};
    const SotnPlayerFactorySource source = {
        .x = in.player.x, .y = in.player.y, .priority = 148, .facing_left = hand == 0};
    bool allocated = false;
    CHECK(SotnBatRuntime_RequestHand(r, source, 86U | (hand + 1U) << 14 | (Uint32)hand << 23, 209,
                                     &allocated) &&
          allocated);
    Uint16 spell = 0, clouds = 0, fading = 0, shifted = 0, most = 0, orphaned = 0;
    Uint32 spell_gone = 0;
    for (Uint16 t = 0; t < 900; ++t)
    {
        // Orphan scenario: the spell's slot empties while clouds still grow; their next aim
        // (within four updates) finds no parent and starts the fade.
        if (orphan && !orphaned)
            for (Uint16 i = 16; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
                if (r->children[i].weapon_secondary &&
                    r->children[i].state.secondary.motion.phase == 2)
                {
                    const Uint16 at = r->children[i].state.secondary.parent_slot;
                    r->children[at - 16U] = (SotnBatRuntimeChild){0};
                    orphaned = t;
                    break;
                }
        if (orphaned && t == orphaned + 4)
            for (Uint16 i = 16; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
                CHECK(!r->children[i].weapon_secondary ||
                      r->children[i].state.secondary.motion.phase == 3);
        in.engine_timer = t;
        // Every eighth update the camera moves right by two after the callbacks.
        in.screen_shift_x = (Sint16)(t % 8 == 7 ? -2 : 0);
        in.camera_x += t % 8 == 7 ? 2 : 0;
        Sint32 before[SOTN_BAT_RUNTIME_SLOTS] = {0};
        bool born[SOTN_BAT_RUNTIME_SLOTS] = {0};
        for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        {
            before[i] = r->children[i].state.secondary.motion.x;
            born[i] = r->children[i].initialized && r->children[i].weapon_secondary;
        }
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        Uint16 live = 0;
        for (Uint16 i = 1; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        {
            const SotnBatRuntimeChild *c = &r->children[i];
            if (!c->weapon_secondary || c->state.secondary.family != 25 || !c->initialized)
                continue;
            const SotnWeaponSecondary *w = &c->state.secondary.motion;
            CHECK(SotnBatRuntime_SecondaryValid(c, i));
            if ((c->kind & 15U) == 8)
            {
                CHECK(i <= 15 && c->kind == (hand ? 0xf8 : 0xe8) && c->primitive_count == 17 &&
                      c->state.secondary.item == 209 && (w->flags & 0x10000U));
                // A camera move retains the spell's screen position (it does not move itself
                // after phase1).
                if (w->phase >= 2 && in.screen_shift_x && before[i])
                {
                    CHECK(w->x == before[i] + 2 * 65536);
                    ++shifted;
                }
                ++spell;
                continue;
            }
            CHECK(i >= 16 && c->kind == (hand ? 0xf9 : 0xe9) && c->parameter == 0x100 &&
                  c->primitive_count == 1 && !c->state.secondary.item && !w->properties.id &&
                  !c->state.secondary.attack.active && (w->flags & 0x10000U) &&
                  w->flags == 0x04810000U);
            // A living cloud moves by its velocity plus the camera's retained-screen shift.
            if (born[i] && w->phase < 3)
                CHECK(w->x == before[i] + w->velocity_x + (in.screen_shift_x ? 2 * 65536 : 0));
            ++clouds;
            ++live;
            fading += w->phase == 3;
        }
        most = SDL_max(most, live);
        if (spell && !spell_gone)
        {
            bool alive = false;
            for (Uint16 i = 1; i < 16; ++i)
                alive |= r->children[i].weapon_secondary && (r->children[i].kind & 15U) == 8;
            spell_gone = alive ? 0 : t;
        }
    }
    // Clouds fade (phase3) before retiring, normally before the spell's own retirement.
    CHECK(orphan
              ? orphaned && fading
              : spell > 250 && clouds > 100 && most >= 4 && shifted > 20 && spell_gone && fading);
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        CHECK(!r->children[i].weapon_secondary && !r->children[i].primitive_count);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}

// w_026 EntityWeaponShieldSpell: Shaman's fourteen rows. Its twelve band angles, green cycle and
// glass-shard counter are program words loaded once. The shield grows, holds and flashes out;
// blueprint92 releases the stars with GLASS_SHARDS, repeated every 22 counts through phase7; the
// card darkens, waits 0x30, brightens past 0x120 for pink6 and the bands spread and fade before
// the spell ends the freeze. Every callback bounces the green cycle and rewrites palette 0x112's
// colour13 green.
// Shaman's band colours from the angles before the callback: band i reads angles i+4/8 (red/
// green/blue) and i+1/5/9 (lower corners) as the loop steps earlier bands by the step.
static bool ShamanBands(const SotnWeaponSecondary *s, const Sint32 *before, Uint8 *exe)
{
    Sint32 a[12] = {0};
    SDL_memcpy(a, before, sizeof(a));
    const Uint16 offsets[6] = {0, 4, 8, 1, 5, 9};
    for (Uint16 i = 0; i < 12; ++i)
    {
        Uint8 c[6] = {0};
        for (Uint16 k = 0; k < 6; ++k)
        {
            SotnMathTrig t = {0};
            CHECK(
                SotnMath_Trig((SotnDataView){exe, 0x24000}, (Sint16)a[(i + offsets[k]) % 12], &t));
            c[k] = (Uint8)(((t.sine + 0x1000) >> 6) * s->spell_bright / 256);
        }
        const SotnPlayerEffectQuad *q = &s->spell_quads[2 + i];
        CHECK(q->red[0] == c[0] && q->green[0] == c[1] && q->blue[0] == c[2] && q->red[3] == c[3] &&
              q->green[3] == c[4] && q->blue[3] == c[5]);
        a[i] += s->spell_step;
    }
    return true;
}
static bool ShamanSpellTest(Uint16 hand)
{
    static Uint8 o[0x1400], exe[0x24000];
    Math(exe);
    Put16(o + 0xa98, 3);
    Put16(o + 0xa9a, 1);
    for (Uint16 i = 0; i < 12; ++i)
        Put32(o + 0xa9c + i * 4U, i * 0x155U);
    SotnHandWeaponProgram program = {0};
    SotnWeaponSecondary s = {.parameter = (Uint16)(hand << 15), .x = 128 * 65536, .y = 120 * 65536};
    SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                   .executable = {exe, sizeof(exe)},
                                   .hand = hand,
                                   .family = 26,
                                   .role = 8,
                                   .priority = 148,
                                   .facing_left = hand != 0,
                                   .allocated = true,
                                   .free_factories = 4};
    Uint32 rng = 5;
    SotnWeaponSecondaryEvents e = {0};
    CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == 5 && s.phase == 1 &&
          e.clear_shield && e.sounds[0] == 0x641 && e.freeze_set && !e.factory_count &&
          s.spell_quads[0].priority == 136 && s.spell_quads[1].u[1] == 0 &&
          s.spell_quads[1].u[2] == 0x5f && s.spell_quads[1].v[0] == (hand ? 0xff : 0x7f) &&
          s.spell_quads[1].v[1] == (hand ? 0xc8 : 0x48) && s.spell_quads[13].u[0] == 88 &&
          s.spell_quads[13].u[3] == 95 && s.spell_quads[13].palette == 0x15f &&
          s.spell_quads[13].priority == 0x1ba && s.spell_quads[13].draw_mode == 0x3d &&
          program.shield_angles[11] == 11 * 0x155 + 0x80 && program.shaman_green == 4 &&
          program.shaman_direction == 1 && e.clut && e.clut_palette == (hand ? 0x12a : 0x112) &&
          e.clut_color == 13 && e.clut_mask == 0xfc1f && e.clut_add == 4 << 5 &&
          SotnWeaponSecondary_RoleRows(26, 8) == 14 && SotnWeaponSecondary_Rows(26) == 14 &&
          SotnWeaponSecondary_ChildRows(26, 9, 3) == 1 &&
          SotnWeaponSecondary_ChildRows(26, 9, 0x100) == 4 &&
          SotnWeaponSecondary_ChildRows(25, 9, 0x100) == 1 &&
          SotnWeaponSecondary_ShieldSpellItem(26) == 210 &&
          SotnWeaponSecondary_ShieldSpellFamily(210) == 26);
    Uint16 t = 0, counts[9] = {0}, stars = 0, shards = 0, pink = 0;
    Sint16 green = program.shaman_green, direction = program.shaman_direction;
    for (; !s.removed && t < 600; ++t)
    {
        in.engine_timer = t;
        const Uint8 phase = s.phase;
        const Sint32 counter = program.shaman_counter;
        const Sint16 wave = s.spell_wave_y, wave_x = s.spell_wave_x;
        Sint32 angles[12] = {0};
        SDL_memcpy(angles, program.shield_angles, sizeof(angles));
        e = (SotnWeaponSecondaryEvents){0};
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == 5);
        CHECK(s.removed || ShamanBands(&s, angles, exe));
        ++counts[phase];
        // The green cycle bounces 0..0x1F; even direction words descend.
        if (!s.removed)
        {
            if (direction & 1)
            {
                if (++green >= 0x20)
                    green = 0x1f, ++direction;
            }
            else if (--green < 0)
                green = 0, ++direction;
            CHECK(program.shaman_green == green && program.shaman_direction == direction &&
                  e.clut && e.clut_add == (Uint16)(green << 5));
        }
        // GLASS_SHARDS: at the stars' release, then on every 22nd count through phase7.
        const bool shard =
            (phase == 3 && s.phase == 4) || (phase >= 4 && phase <= 7 && !(counter % 22));
        CHECK((e.sounds[0] == 0x6b0) == shard);
        shards += shard;
        if (phase >= 4)
            CHECK(program.shaman_counter == (phase == 3 ? 0 : counter + 1));
        if (e.factory_count)
        {
            CHECK(phase == 3 && s.phase == 4 && e.factory_count == 1 &&
                  e.factories[0] == 92U + ((hand + 1U) << 14) && program.shaman_counter == 0 &&
                  !(s.spell_quads[1].draw_mode & 8U) && !(s.spell_quads[13].draw_mode & 8U));
            ++stars;
        }
        if (phase == 4 && s.phase == 5)
            CHECK(s.spell_half_w == 28 && s.spell_half_h == 48 &&
                  (s.spell_quads[0].draw_mode & 8U) && s.spell_quads[0].blue[3] == 4 &&
                  s.spell_quads[1].y[2] - s.spell_quads[1].y[0] == 95 &&
                  s.spell_quads[7].y[0] - s.spell_quads[2].y[0] == 40);
        // Bands carry independent red/green/blue phases a third of the circle apart.
        if (phase == 5 && s.spell_bright > 0x80)
            CHECK(s.spell_quads[2].red[0] != s.spell_quads[2].green[0] ||
                  s.spell_quads[2].green[0] != s.spell_quads[2].blue[0]);
        if (phase == 8)
            CHECK(s.removed || (s.spell_wave_y == wave + 2 && s.spell_wave_x == wave_x + 2));
        if (e.pink)
        {
            CHECK(e.pink == 6 && phase == 7 && s.spell_bright > 0x120 &&
                  (s.spell_quads[1].draw_mode & 8U));
            ++pink;
        }
        CHECK(e.freeze_clear == s.removed);
    }
    // Growth22, hold8, flash16, opening22, darkening26, wait0x30, brightening73, spread37.
    CHECK(s.removed && stars == 1 && pink == 1 && shards > 6 && counts[1] == 22 && counts[2] == 8 &&
          counts[3] == 16 && counts[4] == 22 && counts[5] == 26 && counts[6] == 0x30 &&
          counts[7] == 73 && counts[8] == 37);
    return true;
}
// Floating-point SDK matrix pairs for the star tests (sine, cosine at 4096 per turn).
static void MatrixPairs(Uint8 *exe)
{
    for (Uint32 i = 0; i < 4096; ++i)
    {
        const double a = (double)i * 2.0 * SDL_PI_D / 4096.0;
        Put16(exe + 0x1ea10 + i * 4, (Uint16)(Sint16)SDL_lround(SDL_sin(a) * 4096.0));
        Put16(exe + 0x1ea12 + i * 4, (Uint16)(Sint16)SDL_lround(SDL_cos(a) * 4096.0));
    }
}
static bool StarExpected(const SotnWeaponSecondary *s, Uint8 *exe, Sint16 sx, Sint16 sy)
{
    const Sint16 ox = s->star_offset_x, oy = s->star_offset_y;
    const Sint16 v[4][3] = {{(Sint16)(8 + ox), (Sint16)(-28 + oy), 0},
                            {(Sint16)(56 + ox), (Sint16)(-28 + oy), 0},
                            {(Sint16)(8 + ox), (Sint16)(28 + oy), 0},
                            {(Sint16)(56 + ox), (Sint16)(28 + oy), 0}};
    const Sint32 transfer[3] = {0, 0x10, 0x180};
    SotnProjectedPoint point[4] = {{0}};
    Sint32 otz = 0;
    CHECK(SotnShieldStar_Project((SotnDataView){exe, 0x24000}, s->star_axis, transfer, sx, sy,
                                 0x280, v, point, &otz));
    for (Uint16 k = 0; k < 4; ++k)
        CHECK(s->spell_quads[0].x[k] == point[k].x && s->spell_quads[0].y[k] == point[k].y);
    const Uint16 angle = (Uint16)s->star_axis[1] & 0xfffU;
    const bool shown = otz > 0 && otz <= 0x1be;
    CHECK(
        ((s->spell_quads[0].draw_mode & 8U) == 0) == shown &&
        (!shown || s->spell_quads[0].priority == (angle > 0x900 && angle < 0xf00 ? 0x1be : 0x1b4)));
    return true;
}
// w_026 func_ptr_80170024 stars (params0..7): one GT4 then a rand%15 delay, the overlay UVs,
// a FACTORY(90,1) trail, a Y spin of 0x28 per update from index<<9, offsets shrinking by six after
// the delay, release on the raw parent's step8 with a rand&15 delay, then flight until the Y
// offset passes -0x80. RotTransPers4 places it around its birth point.
static bool ShamanStarTest(Uint16 hand, Uint16 index)
{
    static Uint8 o[0x1400], exe[0x24000];
    Math(exe);
    MatrixPairs(exe);
    const Uint8 uv[8] = {80, 0, 127, 0, 80, 55, 127, 55};
    SDL_memcpy(o + 0xadc, uv, 8);
    SotnHandWeaponProgram program = {0};
    SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                   .executable = {exe, sizeof(exe)},
                                   .hand = hand,
                                   .family = 26,
                                   .role = 9,
                                   .priority = 148,
                                   .camera_x = 20,
                                   .camera_y = 4,
                                   .free_factories = 4,
                                   .parent_exists = true,
                                   .parent_phase = 4};
    // The projection itself: an identity rotation at depth0x200 with screen0x280 scales by 1.25
    // around the offset, and OTZ is the last vertex's depth over four.
    {
        const Sint16 axis[3] = {0, 0, 0},
                     v[4][3] = {{40, 30, 0}, {-40, 30, 0}, {40, -30, 0}, {0, 0, 0}};
        const Sint32 transfer[3] = {0, 0x10, 0x200};
        SotnProjectedPoint point[4] = {{0}};
        Sint32 otz = 0;
        CHECK(SotnShieldStar_Project((SotnDataView){exe, sizeof(exe)}, axis, transfer, 100, 50,
                                     0x280, v, point, &otz) &&
              otz == 0x80 && point[0].x == 150 && point[1].x == 50 && point[0].y == 107 &&
              point[2].y == 32 && point[3].x == 100 && point[3].y == 70);
    }
    Uint32 rng = 31, check = 31;
    SotnWeaponSecondaryEvents e = {0};
    {
        SotnWeaponSecondary cold = {.parameter = index};
        CHECK(SotnWeaponSecondary_Tick(&cold, &program, in, &rng, &e) && cold.removed &&
              rng == check);
    }
    in.allocated = true;
    SotnWeaponSecondary s = {
        .parameter = index, .x = 150 * 65536, .y = 100 * 65536, .flags = 0x10000};
    const Uint16 delay = (Uint16)((SotnCombat_Random(&check) & 0x7fffU) % 15U);
    CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == check && s.phase == 1 &&
          s.spell_timer == delay && s.star_axis[0] == -0x100 &&
          s.star_axis[1] == (Sint16)(index << 9) && s.star_offset_x == 0xa0 &&
          s.star_offset_y == 0x60 && s.flags == 0x04810000U &&
          s.spell_quads[0].palette == (hand ? 0x12a : 0x112) && s.spell_quads[0].u[1] == 127 &&
          s.spell_quads[0].v[2] == (hand ? 0x80 + 55 : 55) && s.spell_quads[0].draw_mode == 0x71 &&
          e.factory_count == 1 && e.factories[0] == ((90U + ((hand + 1U) << 14)) | 1U << 16) &&
          e.factory_x[0] == s.x);
    Uint16 t = 0, shown = 0, front = 0;
    for (; s.phase == 1 && t < 200; ++t)
    {
        const Sint16 axis = s.star_axis[1], ox = s.star_offset_x;
        const Sint16 timer = s.spell_timer;
        in.parent_phase = (Uint8)(t < 60 ? 4 : 8);
        check = rng;
        e = (SotnWeaponSecondaryEvents){0};
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && !e.factory_count);
        CHECK(s.star_axis[1] == (Sint16)(axis + 0x28));
        if (timer)
            CHECK(s.star_offset_x == ox && (s.phase == 2 || s.spell_timer == timer - 1));
        else
            CHECK(s.star_offset_x == SDL_max(ox - 6, 0));
        if (s.phase == 2)
            CHECK(s.spell_timer == (Sint16)((SotnCombat_Random(&check) & 0x7fffU) & 15U) &&
                  rng == check && t == 60);
        else
            CHECK(rng == check);
        shown += !(s.spell_quads[0].draw_mode & 8U);
        front += s.spell_quads[0].priority == 0x1be;
        // The original constants: vertices 8/56 x -28/28 plus the offsets, transfer
        // (0, 0x10, 0x180), screen 0x280 at the birth point; OTZ picks visibility and the angle
        // the front/back priority.
        CHECK(StarExpected(&s, exe, 130, 96));
    }
    CHECK(s.phase == 2 && shown > 50 && front > 5 && front < shown && !s.star_offset_x &&
          !s.star_offset_y);
    for (t = 0; !s.removed && t < 200; ++t)
    {
        const Sint16 ox = s.star_offset_x, oy = s.star_offset_y, timer = s.spell_timer;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
        // It retires exactly when Y passes below -0x80.
        if (s.removed)
            CHECK(!timer && oy - 6 < -0x80);
        else if (!timer)
            CHECK(s.star_offset_x == ox + 6 && s.star_offset_y == oy - 6 &&
                  s.star_offset_y >= -0x80 && StarExpected(&s, exe, 130, 96));
    }
    // Y falls from0 by six until below -0x80: twenty-two steps after the delay.
    CHECK(s.removed);
    return true;
}
// w_026 func_ptr_80170024 trails (params1): four hidden GT4s at birth; each update row
// (count>>1)%4 copies the raw parent star's row while its X offset is positive, else hides; an
// empty parent slot retires the trail.
static bool ShamanTrailTest(void)
{
    static Uint8 o[0x1400], exe[0x24000];
    SotnHandWeaponProgram program = {0};
    SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                   .executable = {exe, sizeof(exe)},
                                   .family = 26,
                                   .role = 9,
                                   .parent_exists = true,
                                   .parent_offset_x = 10};
    Uint32 rng = 3;
    SotnWeaponSecondaryEvents e = {0};
    SotnWeaponSecondary s = {.parameter = 0x100, .flags = 0x10000};
    {
        SotnWeaponSecondary cold = s;
        CHECK(SotnWeaponSecondary_Tick(&cold, &program, in, &rng, &e) && cold.removed);
        SotnWeaponSecondaryInput gone = in;
        gone.allocated = true;
        gone.parent_exists = false;
        cold = s;
        CHECK(SotnWeaponSecondary_Tick(&cold, &program, gone, &rng, &e) && cold.removed);
    }
    in.allocated = true;
    CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 1 &&
          s.flags == 0x04810000U && (s.spell_quads[3].draw_mode & 8U) && rng == 3);
    for (Uint16 t = 1; t < 20; ++t)
    {
        in.parent_quad = (SotnPlayerEffectQuad){.x = {(Sint16)t}, .priority = 0x1b4};
        in.parent_offset_x = (Sint16)(t < 12 ? 10 : 0);
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
        const Uint16 row = (Uint16)((t >> 1) % 4U);
        CHECK(t < 12 ? s.spell_quads[row].x[0] == (Sint16)t && !(s.spell_quads[row].draw_mode & 8U)
                     : (s.spell_quads[row].draw_mode & 8U) != 0);
    }
    in.parent_exists = false;
    CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.removed);
    return true;
}

// Runtime: Mablung/Shaman's blueprint86 spell (fourteen rows) releases blueprint92's eight E9/F9
// stars (one row, params0..7, slots17..31), each with a blueprint90 trail of four rows (32..63,
// params1). Every spell callback queues one read-modify palette word; stars leave after the
// spell's step8 and trails retire with their stars.
static bool ShamanRuntime(Uint16 hand)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(56, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    static Uint8 exe[0x24000];
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Math(exe);
    MatrixPairs(exe);
    FamilyDefinition(dra, 12, 9, 26, 128, 0, 0, 0);
    FamilyDefinition(dra, 210, 10, 26, 5, 0, 10, 0);
    Uint8 *record = dra + 0x500 + 210 * 52;
    record[18] = 0xba;
    record[19] = 86;
    record[21] = 0;
    record[22] = 1;
    const Uint8 blueprints[5][6] = {{8, 1, 0xc1, 0, 6, 0},
                                    {9, 8, 0xc8, 0, 6, 0},
                                    {9, 1, 0xc1, 0, 0, 0},
                                    {11, 1, 0x81, 0, 0, 0},
                                    {39, 1, 0x81, 0, 0, 0}};
    const Uint16 ids[5] = {86, 92, 90, 44, 55};
    for (Uint16 b = 0; b < 5; ++b)
        SDL_memcpy(dra + 0xd1d4 + ids[b] * 6U, blueprints[b], 6);
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    for (Uint16 i = 0; i < 2; ++i)
    {
        Uint8 *q = dra + 0xd670 + (i ? 0x47U : 0x40U) * 20U;
        Put16(q + 6, 0x100);
        for (Uint16 k = 8; k <= 12; k += 2)
            Put16(q + k, 256);
        Put16(q + 14, 0x7002);
        Put16(q + 16, 0x31);
        Put16(q + 18, 512);
    }
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    CHECK(SotnCharacter_Create(actors->dra, &character));
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = {exe, sizeof(exe)},
                              .weapons = {weapons, 56U * 0x7000},
                              .weapon_family = {26, 26},
                              .weapon_bound = {true, true},
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 200 * 65536,
                                         .y = 135 * 65536,
                                         .priority = 148,
                                         .facing_left = hand == 0}};
    const SotnPlayerFactorySource source = {
        .x = in.player.x, .y = in.player.y, .priority = 148, .facing_left = hand == 0};
    bool allocated = false;
    CHECK(SotnBatRuntime_RequestHand(r, source, 86U | (hand + 1U) << 14 | (Uint32)hand << 23, 210,
                                     &allocated) &&
          allocated);
    Uint16 most_stars = 0, most_trails = 0, copies = 0, cluts = 0, spell_ticks = 0;
    for (Uint16 t = 0; t < 900; ++t)
    {
        in.engine_timer = t;
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        Uint16 stars = 0, trails = 0;
        bool spell = false;
        for (Uint16 i = 1; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        {
            const SotnBatRuntimeChild *c = &r->children[i];
            if (!c->weapon_secondary || c->state.secondary.family != 26 || !c->initialized)
                continue;
            const SotnWeaponSecondary *w = &c->state.secondary.motion;
            CHECK(SotnBatRuntime_SecondaryValid(c, i) && (w->flags & 0x10000U));
            if ((c->kind & 15U) == 8)
            {
                spell = true;
                CHECK(c->primitive_count == 14 && c->state.secondary.item == 210);
                continue;
            }
            CHECK(c->kind == (hand ? 0xf9 : 0xe9) && !c->state.secondary.item &&
                  !c->state.secondary.attack.active);
            if (c->parameter & 0x7f00U)
            {
                CHECK(i >= 16 && c->parameter == 0x100 && c->primitive_count == 4);
                ++trails;
                const SotnBatRuntimeChild *star =
                    &r->children[c->state.secondary.parent_slot - 16U];
                for (Uint16 k = 0; k < 4; ++k)
                    copies += star->weapon_secondary && !(w->spell_quads[k].draw_mode & 8U) &&
                              !SDL_memcmp(&w->spell_quads[k],
                                          &star->state.secondary.motion.spell_quads[0],
                                          sizeof(SotnPlayerEffectQuad));
            }
            else
            {
                CHECK(i <= 15 && c->parameter <= 7 && c->primitive_count == 1);
                ++stars;
            }
        }
        if (spell)
        {
            ++spell_ticks;
            CHECK(r->clut_count == 1 && r->clut[0].palette == (hand ? 0x12a : 0x112) &&
                  r->clut[0].color == 13 && r->clut[0].mask == 0xfc1f);
            ++cluts;
        }
        else
            CHECK(!r->clut_count);
        most_stars = SDL_max(most_stars, stars);
        most_trails = SDL_max(most_trails, trails);
    }
    CHECK(most_stars == 8 && most_trails == 8 && copies > 100 && cluts == spell_ticks &&
          spell_ticks > 250);
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        CHECK(!r->children[i].weapon_secondary && !r->children[i].primitive_count);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}

// Alucard's orbit from the original constants: letters at angle+i*4096/11 around 0x1C ahead and
// 0x30 above the player, radius (unk90) by vertical scale (unk98), size12/16 (letter6 twelve).
static bool AlucardOrbit(const SotnWeaponSecondary *s, Uint8 *exe, Sint16 px, Sint16 py, bool left)
{
    const Sint16 cx = (Sint16)(px - (left ? -0x1c : 0x1c)), cy = (Sint16)(py - 0x30);
    const Sint16 rx = s->spell_half_w, ry = (Sint16)(s->spell_half_w * s->spell_bright / 256);
    for (Uint16 i = 0; i < 11; ++i)
    {
        const SotnPlayerEffectQuad *q = &s->spell_quads[1 + i];
        if (q->green[0])
            continue;
        SotnMathTrig t = {0};
        CHECK(SotnMath_Trig((SotnDataView){exe, 0x24000},
                            (Sint16)(s->spell_wave_x + ((Sint32)i << 12) / 11), &t));
        const Sint32 mx = cx + Sra(Sra(t.cosine, 4) * rx, 8), my = cy - Sra(Sra(t.sine, 4) * ry, 8);
        const Sint16 h = (Sint16)(s->spell_half_h * (i == 6 ? 12 : 16) / 256),
                     w = (Sint16)(s->spell_half_h * 12 / 256);
        CHECK(q->x[0] == (Sint16)(mx - w) && q->x[3] == (Sint16)(mx + w) &&
              q->y[0] == (Sint16)(my - h) && q->y[3] == (Sint16)(my + h));
    }
    return true;
}
// w_052 EntityWeaponShieldSpell: Alucard's twenty-three rows and eleven birth rands. Letters
// circle 0x1C ahead of and 0x30 above the player as the shield grows (22), holds (0x40, then
// TRANSFORM) and flashes (16); they keep circling while the glow fades (0x20), then streak away
// with TELEPORT_BANG_B; the G4 ribbons between their retained centres open by twelve (20 updates)
// before BlinkWhite FACTORY(44,1) and the armed flag, fade for 0x20 updates, arm the timer
// (0x1000) and end the freeze.
static bool AlucardSpellTest(Uint16 hand)
{
    static Uint8 o[0x1400], exe[0x24000];
    Math(exe);
    for (Uint16 i = 0; i < 11; ++i)
    {
        o[0xbc4 + i * 4U] = (Uint8)(i * 8);
        o[0xbc5 + i * 4U] = 0x30;
        o[0xbc6 + i * 4U] = (Uint8)(i * 8 + 7);
        o[0xbc7 + i * 4U] = 0x4f;
        o[0xbf0 + i * 4U] = (Uint8)(0x80 - i * 8);
        o[0xbf1 + i * 4U] = (Uint8)(i * 8);
        o[0xbf2 + i * 4U] = 0x40;
    }
    SotnHandWeaponProgram program = {0};
    SotnWeaponSecondary s = {.parameter = (Uint16)(hand << 15), .x = 128 * 65536, .y = 120 * 65536};
    SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                   .executable = {exe, sizeof(exe)},
                                   .x = 100 * 65536,
                                   .y = 150 * 65536,
                                   .hand = hand,
                                   .family = 52,
                                   .role = 8,
                                   .priority = 148,
                                   .facing_left = hand != 0,
                                   .allocated = true};
    Uint32 rng = 5, check = 5;
    Uint16 roll[11] = {0};
    for (Uint16 i = 0; i < 11; ++i)
        roll[i] = (Uint16)(SotnCombat_Random(&check) & 0x7fffU);
    SotnWeaponSecondaryEvents e = {0};
    CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == check && s.phase == 1 &&
          e.freeze_set && e.clear_shield && e.sounds[0] == 0x641 &&
          s.spell_quads[1].palette == (hand ? 0x129 : 0x111) &&
          s.spell_quads[11].palette == (hand ? 0x133 : 0x11b) && s.spell_quads[11].u[1] == 87 &&
          s.spell_quads[11].v[0] == (hand ? 0xb0 : 0x30) && s.spell_quads[11].draw_mode == 0 &&
          s.spell_quads[4].red[0] == (Uint8)((roll[3] & 15U) + 1U) &&
          (s.spell_quads[12].draw_mode & 8U) && s.spell_quads[12].red[0] == 0x80 &&
          s.spell_quads[12].red[1] == 0x78 && s.spell_quads[22].green[1] == 0 &&
          SotnWeaponSecondary_RoleRows(52, 8) == 23 && SotnWeaponSecondary_RoleRows(52, 1) == 11 &&
          SotnWeaponSecondary_ShieldSpellItem(52) == 214 &&
          SotnWeaponSecondary_ShieldSpellFamily(214) == 52);
    // Letters circle (0x1C ahead, 0x30 above) with a radius shrinking from0x78.
    const Sint16 cx = (Sint16)(100 - (hand ? -0x1c : 0x1c)), cy = 150 - 0x30;
    for (Uint16 i = 0; i < 11; ++i)
        CHECK(SDL_abs(program.alucard_x[i] - cx) <= 0x78 &&
              SDL_abs(program.alucard_y[i] - cy) <= 0x78);
    Uint16 t = 0, counts[9] = {0}, transform = 0, bang = 0, blink = 0;
    // The original scalars: radius unk90, angle unk92, size unk94, step unk86, scale unk98.
    Sint16 radius = 0x78, angle = 0, size = 0, step = 0x40, yscale = 0x100, scale = 0;
    CHECK(s.spell_half_w == radius && s.spell_wave_x == angle && s.spell_half_h == size &&
          s.spell_step == step && s.spell_bright == yscale &&
          AlucardOrbit(&s, exe, 100, 150, hand));
    for (; !s.removed && t < 600; ++t)
    {
        in.engine_timer = t;
        const Uint8 phase = s.phase;
        const Sint16 ribbon = s.spell_wave_y, timer = s.spell_timer;
        SotnPlayerEffectQuad letters[11];
        SDL_memcpy(letters, &s.spell_quads[1], sizeof(letters));
        e = (SotnWeaponSecondaryEvents){0};
        check = rng;
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == check);
        ++counts[phase];
        switch (phase)
        {
        case 1:
            scale = (Sint16)SDL_min(scale + 12, 0x100);
            size = scale;
            angle = (Sint16)(angle + step);
            radius = (Sint16)(radius - 3);
            break;
        case 2:
            if (step > 0)
                --step;
            angle = (Sint16)(angle + step);
            break;
        case 3:
            size = (Sint16)(size - 2);
            yscale = (Sint16)SDL_max(yscale - 2, 0x80);
            step = (Sint16)(step + 3);
            angle = (Sint16)(angle + step);
            radius = (Sint16)(radius - 1);
            CHECK(s.spell_quads[0].priority == (!(timer & 1) ? 0x1fc : 144));
            break;
        case 4:
            size = (Sint16)(size - 2);
            yscale = (Sint16)SDL_max(yscale - 2, 0x80);
            step = (Sint16)(step + 1);
            angle = (Sint16)(angle + step);
            step = (Sint16)(step + 4);
            angle = (Sint16)(angle + step);
            break;
        default:
            break;
        }
        if (!s.removed)
        {
            CHECK(s.spell_half_w == radius && s.spell_wave_x == angle && s.spell_half_h == size &&
                  s.spell_step == step && s.spell_bright == yscale &&
                  AlucardOrbit(&s, exe, 100, 150, hand));
            // Streaking letters (state5) narrow by one per side (to a line), open by twelve per
            // update both ways and hide once beyond the 0..0xF0 screen rows.
            for (Uint16 i = 0; i < 11; ++i)
            {
                const SotnPlayerEffectQuad *a = &letters[i], *b = &s.spell_quads[1 + i];
                if (a->green[0] != 5)
                    continue;
                const bool narrow = a->x[1] - a->x[0] > 2;
                const Sint16 y0 = (Sint16)(a->y[0] - 12), y2 = (Sint16)(a->y[2] + 12);
                CHECK(b->x[0] == (narrow ? a->x[0] + 1 : a->x[1]) &&
                      b->x[1] == (narrow ? a->x[1] - 1 : a->x[1]) && b->x[2] == b->x[0] &&
                      b->x[3] == b->x[1] && b->y[0] == y0 && b->y[1] == y0 && b->y[2] == y2 &&
                      b->y[3] == y2 && b->green[0] == (y0 < 0 && y2 > 0xf0 ? 6 : 5) &&
                      ((b->draw_mode & 8U) != 0) == (y0 < 0 && y2 > 0xf0));
            }
        }
        transform += e.sounds[0] == 0x668;
        if (e.sounds[0] == 0x636)
        {
            CHECK(phase == 4 && s.phase == 6 && s.spell_quads[1].green[0] == 5 &&
                  s.spell_quads[11].green[0] == 5);
            ++bang;
        }
        if (phase == 6 && !e.blink)
        {
            // Ribbons span neighbouring letter centres, opened by the current height.
            const SotnPlayerEffectQuad *q = &s.spell_quads[13];
            CHECK(q->x[0] == program.alucard_x[1] && q->x[1] == program.alucard_x[2] &&
                  q->y[0] == program.alucard_y[1] - s.spell_wave_y &&
                  q->y[3] == program.alucard_y[2] + s.spell_wave_y &&
                  s.spell_wave_y == ribbon + 12 && q->draw_mode == 0x31 &&
                  q->priority == (Uint16)program.alucard_priority[1]);
        }
        if (e.blink)
        {
            CHECK(e.blink == 1 && phase == 6 && s.phase == 7 && program.alucard_active == 1 &&
                  !program.alucard_timer);
            ++blink;
        }
        if (phase == 7 && !s.removed)
            CHECK(s.spell_quads[12].red[0] <= SDL_max(8, 0x80 - 4 * (counts[7])));
        CHECK(e.freeze_clear == s.removed);
    }
    // A streaking letter reaching exactly row0xF0 stays shown; beyond it, it hides.
    {
        SotnWeaponSecondary edge = s;
        edge.removed = false;
        edge.phase = 6;
        edge.spell_wave_y = 0;
        edge.allocated = true;
        SotnPlayerEffectQuad *q = &edge.spell_quads[1];
        *q = (SotnPlayerEffectQuad){.x = {10, 30, 10, 30}, .y = {-50, -50, 228, 228}};
        q->green[0] = 5;
        SotnWeaponSecondaryEvents edge_events = {0};
        SotnHandWeaponProgram edge_program = program;
        CHECK(SotnWeaponSecondary_Tick(&edge, &edge_program, in, &rng, &edge_events) &&
              q->y[2] == 0xf0 && q->green[0] == 5 && !(q->draw_mode & 8U));
        CHECK(SotnWeaponSecondary_Tick(&edge, &edge_program, in, &rng, &edge_events) &&
              q->y[2] == 0xfc && q->green[0] == 6 && (q->draw_mode & 8U));
    }
    CHECK(s.removed && transform == 1 && bang == 1 && blink == 1 && counts[1] == 22 &&
          counts[2] == 0x40 && counts[3] == 16 && counts[4] == 0x20 && counts[6] == 20 &&
          counts[7] == 0x20 && counts[8] == 1 && program.alucard_active == 1 &&
          program.alucard_timer == 0x1000);
    return true;
}
// w_052 func_ptr_80170004 (blueprint0x3A from the expiring armed shield): eleven letters from the
// player (8 above), one countdown and two velocity rands each; they grow and fall for0x20
// updates, count down, streak upward and hide; WEAPON_SWISH_C on every fourth update from0x60 to
// 0x48 of the 0x60-update tail.
static bool AlucardLettersTest(Uint16 hand)
{
    static Uint8 o[0x1400], exe[0x24000];
    SotnHandWeaponProgram program = {0};
    for (Uint16 i = 0; i < 11; ++i)
        program.letter_x[i] = 0x1234;
    SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                   .executable = {exe, sizeof(exe)},
                                   .camera_x = 10,
                                   .camera_y = 4,
                                   .x = 130 * 65536,
                                   .y = 140 * 65536,
                                   .hand = hand,
                                   .family = 52,
                                   .role = 1,
                                   .priority = 148};
    Uint32 rng = 21, check = 21;
    SotnWeaponSecondaryEvents e = {0};
    {
        SotnWeaponSecondary cold = {.flags = 0x10000};
        CHECK(SotnWeaponSecondary_Tick(&cold, &program, in, &rng, &e) && cold.removed &&
              rng == check);
    }
    in.allocated = true;
    SotnWeaponSecondary s = {.flags = 0x10000, .priority = 150};
    Uint16 r[33] = {0};
    for (Uint16 i = 0; i < 33; ++i)
        r[i] = (Uint16)(SotnCombat_Random(&check) & 0x7fffU);
    CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == check && s.phase == 1 &&
          s.flags == 0x04810000U && s.spell_timer == 0x20 && s.spell_half_h == 0x20 &&
          s.spell_quads[0].palette == (hand ? 0x129 : 0x111) && s.spell_quads[0].priority == 148 &&
          s.spell_quads[5].red[0] == (Uint8)((r[15] & 15U) + 1U) &&
          program.letter_x[5] == (Sint32)((120U << 16) | 0x1234U) &&
          (program.letter_y[5] >> 16) == 140 - 4 - 8 &&
          program.letter_vx[5] == (Sint32)(r[16] * 6U) - 0x18000 &&
          program.letter_vy[5] == -0x20000 - (Sint32)(r[17] * 4U) &&
          SotnWeaponSecondary_SupportedRole(52, 1) && !SotnWeaponSecondary_SupportedRole(52, 9));
    Uint16 t = 0, swish = 0, streaking = 0;
    for (; !s.removed && t < 300; ++t)
    {
        const Sint32 vy = program.letter_vy[3];
        const Uint8 phase = s.phase;
        SotnPlayerEffectQuad before[11];
        SDL_memcpy(before, s.spell_quads, sizeof(before));
        e = (SotnWeaponSecondaryEvents){0};
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == check);
        // Streaks (state2) narrow while at least five wide (else snap to two), rise eighteen at
        // the top and six at the bottom, and hide (state3, drawMode8) once the bottom is above0.
        for (Uint16 i = 0; !s.removed && i < 11; ++i)
        {
            const SotnPlayerEffectQuad *a = &before[i], *b = &s.spell_quads[i];
            if (a->green[0] == 2)
            {
                const bool narrow = a->x[1] - a->x[0] >= 5;
                CHECK(b->x[0] == (narrow ? a->x[0] + 1 : a->x[1] - 2) &&
                      b->x[1] == (narrow ? a->x[1] - 1 : a->x[1]) && b->y[0] == a->y[0] - 18 &&
                      b->y[2] == a->y[2] - 6 && b->y[1] == b->y[0] && b->x[2] == b->x[0] &&
                      b->green[0] == (b->y[2] < 0 ? 3 : 2));
            }
            if (a->green[0] == 3)
                CHECK(b->draw_mode == 8);
        }
        if (phase == 1)
            CHECK(program.letter_vy[3] == vy + 0x2000 &&
                  s.spell_half_h == SDL_min(0x20 + 0x10 * (t + 1), 0xc0));
        swish += e.sounds[0] == 0x60c;
        streaking += s.spell_quads[3].green[0] == 2;
    }
    CHECK(s.removed && swish == 7 && streaking > 0 && t == 0x20 + 0x60);
    return true;
}

// Authored w_023 tables: the spell's s32 angles D_8017AB70, the swirl UVs D_8017ABB0, its model
// vector pointers D_8017AC50 (permuted as the original lists them) and SVECTORs, and the sprites'
// D_8017AB58 timeline.
static const Uint8 HeraldOrder[4] = {0, 1, 3, 2};
static void HeraldTables(Uint8 *o)
{
    for (Uint16 i = 0; i < 16; ++i)
        Put32(o + 0xb70 + i * 4U, (Uint32)(i * 0x1c0U + 0x30U));
    for (Uint16 i = 0; i < 32; ++i)
        o[0xbb0 + i] = (Uint8)(i * 7U + 3U);
    for (Uint16 i = 0; i < 16; ++i)
    {
        const Uint16 j = (Uint16)((i & ~3U) | HeraldOrder[i & 3U]);
        Put32(o + 0xc50 + i * 4U, 0x8017abd0U + j * 8U);
        Put16(o + 0xbd0 + i * 8U, (Uint16)(Sint16)(i * 7 - 50));
        Put16(o + 0xbd2 + i * 8U, (Uint16)(Sint16)(i * 3 - 20));
        Put16(o + 0xbd4 + i * 8U, (Uint16)(Sint16)(40 - i * 5));
    }
    for (Uint16 i = 0; i < 5; ++i)
    {
        Put16(o + 0xb58 + i * 4U, (Uint16)(i == 4 ? 3 : 2));
        Put16(o + 0xb5a + i * 4U, (Uint16)(0xc3f + i));
    }
}
static Sint32 HeraldShift(Sint32 v, Uint16 n)
{
    return v >= 0 ? v >> n : -((-v + (1 << n) - 1) >> n);
}
static Sint16 HeraldWave(SotnDataView exe, Sint32 angle, Sint16 amplitude)
{
    SotnMathTrig t = {0};
    if (!SotnMath_Trig(exe, (Sint16)angle, &t))
        return 0x7fff;
    return (Sint16)(HeraldShift(t.sine, 5) * amplitude / 256);
}
static Uint8 HeraldBlue(SotnDataView exe, Sint32 angle, Sint16 bright)
{
    SotnMathTrig t = {0};
    if (!SotnMath_Trig(exe, (Sint16)angle, &t))
        return 0;
    return (Uint8)(((t.sine + 0x1000) >> 6) * bright / 256);
}
// w_023 EntityWeaponShieldSpell against an exact model: birth rows (sideways image UVs, sixteen
// white slices), the step timeline (22/8/16 growth-hold-flash, 22 opening, 26 dimming, 0x70 hold
// with FACTORY(90) and pink9 at 0x30 and pink10 at its end, 51 brightening, 31 spreading), the
// FACTORY(94) sprites, and every update's glow/image/slice geometry and retained-angle colours.
static bool HeraldSpellTest(void)
{
    static Uint8 o[0x3000], exe[0x24000];
    Math(exe);
    HeraldTables(o);
    const SotnDataView x_exe = {exe, sizeof(exe)};
    for (Uint16 k = 0; k < 5; ++k)
    {
        const Uint16 hand = k & 1U;
        const bool left = k == 1 || k == 2, starved = k == 3, failed = k == 4;
        SotnHandWeaponProgram program = {0};
        SotnWeaponSecondary s = {.parameter = (Uint16)(hand << 15),
                                 .x = (120 + 30) * 65536 + 0x1357,
                                 .y = (100 + 10) * 65536 + 0x2468};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = x_exe,
                                       .camera_x = 99,
                                       .camera_y = 77,
                                       .retained_camera_x = 30,
                                       .retained_camera_y = 10,
                                       .hand = hand,
                                       .family = 23,
                                       .role = 8,
                                       .priority = 148,
                                       .facing_left = left,
                                       .allocated = !failed,
                                       .free_factories = (Uint8)(starved ? 0 : 8)};
        Sint32 angles[16] = {0};
        for (Uint16 i = 0; i < 16; ++i)
            angles[i] = (Sint32)(i * 0x1c0 + 0x30);
        Uint32 rng = 5;
        // The g_Timer phase varies, so the opening's final flicker state differs per case.
        const Uint32 phase_offset = k;
        Uint16 tick = 0, transitions[9] = {0}, sounds = 0, heard[3] = {0}, swirl = 0, pinks = 0;
        Uint8 phase = 0;
        Sint16 bright = 0x100;
        while (!s.removed && tick < 400)
        {
            in.engine_timer = tick + phase_offset;
            const Sint16 before_glow = s.spell_glow;
            SotnWeaponSecondaryEvents e = {0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == 5);
            if (failed)
            {
                CHECK(s.removed && !e.freeze_set && !e.sounds[0]);
                break;
            }
            CHECK(program.shield_angles_loaded);
            if (!tick)
            {
                const Uint8 v1 = hand ? 0x80 : 0;
                CHECK(e.freeze_set && e.clear_shield && e.sounds[0] == 0x641 && s.phase == 1 &&
                      s.spell_step == 0x80 && s.spell_bright == 0x100 && !s.spell_wave_x &&
                      !s.spell_wave_y && s.flags == 0x04810000U && s.priority == 146 &&
                      s.frame == 0x3e && s.draw_flags == 3 && !s.scale_x &&
                      s.palette == (hand ? 0x128 : 0x110) && s.bank == (hand ? 0x8012 : 0x8010) &&
                      s.texture == (hand ? 0x66 : 0x64) &&
                      s.velocity_x == (left ? 0x20000 : -0x20000) &&
                      s.y == (100 + 10 - 8) * 65536 + 0x2468);
                const SotnPlayerEffectQuad *g = &s.spell_quads[0], *q = &s.spell_quads[1];
                CHECK(g->palette == 0x19f && g->page == 0x19 && g->u[0] == 0x80 &&
                      g->u[3] == 0xcf && g->v[0] == (hand ? 0 : 0x80) &&
                      g->v[3] == (Uint8)((hand ? 0 : 0x80) + 0x4f) && g->priority == 136 &&
                      (g->draw_mode & 0xfff7U) == 0x75 && g->red[1] == 0x80 && g->blue[3] == 0x80);
                CHECK(q->palette == (hand ? 0x129 : 0x111) && q->page == 0x19 && !q->u[0] &&
                      !q->u[1] && q->u[2] == 0x7f && q->u[3] == 0x7f &&
                      q->v[1] == (Uint8)(v1 + 0x38) && q->v[3] == q->v[1] &&
                      q->v[0] == (Uint8)(v1 + 0x7f) && q->v[2] == q->v[0] && q->priority == 0x1b8 &&
                      q->draw_mode == 0x08 && q->red[0] == 0x80);
                for (Uint16 i = 0; i < 16; ++i)
                {
                    const SotnPlayerEffectQuad *c = &s.spell_quads[2 + i];
                    CHECK(c->palette == 0x15f && c->priority == 0x1ba && c->draw_mode == 0x3d &&
                          c->u[0] == i * 8 && c->u[1] == i * 8 && c->u[2] == i * 8 + 7 &&
                          c->u[3] == i * 8 + 7 && c->v[0] == q->v[0] && c->v[1] == q->v[1] &&
                          c->page == 0x19);
                }
            }
            if (s.phase != phase)
            {
                transitions[s.phase] = tick;
                phase = s.phase;
            }
            if (e.sounds[0] && sounds < 3)
                heard[sounds] = e.sounds[0];
            sounds += e.sounds[0] != 0;
            if (e.sounds[0] == 0x6b1)
                CHECK(s.phase == 4 && transitions[4] == tick && !s.frame && !s.spell_timer);
            // FACTORY(94) when the opening completes; FACTORY(90) with pink9 at the hold's 0x30.
            if (e.factory_count)
            {
                CHECK(!starved && e.factory_count == 1 && e.factory_x[0] == s.x &&
                      e.factory_y[0] == s.y);
                if (s.phase == 5)
                    CHECK(e.factories[0] == (94U | (hand + 1U) << 14) && tick == 68 && !e.pink);
                else
                    CHECK(e.factories[0] == (90U | (hand + 1U) << 14) && s.phase == 6 &&
                          tick == 143 && e.pink == 9 && s.spell_timer == 0x31);
                swirl += s.phase == 6;
            }
            if (e.pink)
            {
                CHECK((e.pink == 9 && tick == 143) ||
                      (e.pink == 10 && tick == 206 && s.phase == 7));
                ++pinks;
            }
            CHECK(!e.freeze_clear || s.removed);
            if (s.removed)
            {
                CHECK(e.freeze_clear && tick == 288 && s.phase == 8);
                break;
            }
            // Scalars: phase 4 opens the image; phases 5..8 dim, hold, brighten and fade it.
            if ((s.phase == 4 && transitions[4] != tick) ||
                (s.phase == 5 && transitions[5] == tick))
                CHECK(s.spell_glow == before_glow + 8 && s.spell_half_w == s.scale_x * 36 / 256 &&
                      s.spell_half_h == s.scale_x * 64 / 256);
            if (s.phase >= 5)
                CHECK(s.spell_half_w == 36 && s.spell_half_h == 64);
            const bool entered = transitions[s.phase] == tick;
            if ((s.phase == 5 && !entered) || (s.phase == 6 && entered))
                bright = (Sint16)SDL_max(bright - 10, 0);
            else if ((s.phase == 7 && !entered) || (s.phase == 8 && entered))
                bright = (Sint16)(bright + 6);
            else if (s.phase == 8)
                bright = (Sint16)(bright - 10);
            CHECK(s.spell_bright == bright);
            CHECK(s.spell_wave_x == s.spell_wave_y &&
                  s.spell_wave_x == (s.phase == 8 ? (tick - 257) * 3 : 0));
            // Geometry from the model angles (screen words use the retained camera).
            const Sint16 x = (Sint16)((s.x >> 16) - 30), y = (Sint16)((s.y >> 16) - 10);
            const SotnPlayerEffectQuad *g = &s.spell_quads[0], *q = &s.spell_quads[1];
            CHECK(g->x[0] == x - s.spell_glow && g->x[2] == g->x[0] &&
                  g->x[1] == x + s.spell_glow - 1 && g->x[3] == g->x[1] &&
                  g->y[0] == y - s.spell_glow && g->y[1] == g->y[0] &&
                  g->y[2] == y + s.spell_glow - 1 && g->y[3] == g->y[2]);
            // The glow flickers on g_Timer while its blue word is above 8; the opening hides it.
            if (s.phase < 5 && g->blue[3] > 8)
                CHECK(((g->draw_mode & 8U) != 0) == (((tick + phase_offset) & 1U) != 0));
            // Phase3 flickers the glow's priority between 0x1B8 and the spell's less two.
            if (s.phase == 3 && transitions[3] != tick)
                CHECK(g->priority == ((s.spell_timer + 1) & 1 ? 146 - 2 : 0x1b8));
            if (s.phase >= 5)
                CHECK((g->draw_mode & 8U) && g->blue[3] == 4 && g->red[0] == 0x28);
            const Sint16 top = (Sint16)(y - s.spell_half_h), bottom = (Sint16)(y + s.spell_half_h);
            const Sint16 back = (Sint16)(left ? x + s.spell_half_w - 1 : x - s.spell_half_w),
                         front = (Sint16)(left ? x - s.spell_half_w + 1 : x + s.spell_half_w);
            CHECK(q->y[0] == top && q->y[1] == top && q->y[2] == bottom - 1 &&
                  q->y[3] == bottom - 1 && q->x[0] == back && q->x[2] == back &&
                  q->x[1] == front - 1 && q->x[3] == front - 1);
            CHECK(((q->draw_mode & 8U) != 0) == (s.phase < 4 || s.phase == 8));
            for (Uint16 i = 0; i < 16; ++i)
            {
                const SotnPlayerEffectQuad *c = &s.spell_quads[2 + i];
                const Sint32 a = angles[i], n = angles[(i + 1) % 16], w = angles[(i + 6) % 16];
                const Sint16 bx0 = HeraldWave(x_exe, a, s.spell_wave_x),
                             bx1 = HeraldWave(x_exe, n, s.spell_wave_x),
                             by0 = HeraldWave(x_exe, w, s.spell_wave_y),
                             by1 = HeraldWave(x_exe, n, s.spell_wave_y);
                const Sint16 span = (Sint16)(bottom - top);
                CHECK(c->x[0] == back + bx0 && c->x[1] == front + bx0 - 1 &&
                      c->x[2] == back + bx1 && c->x[3] == front + bx1 - 1 &&
                      c->y[0] == top + span * i / 16 - by0 &&
                      c->y[2] == top + span * (i + 1) / 16 - by0 &&
                      c->y[1] == top + span * i / 16 - by1 &&
                      c->y[3] == top + span * (i + 1) / 16 - by1);
                CHECK(c->blue[0] == HeraldBlue(x_exe, a, s.spell_bright) &&
                      c->blue[1] == c->blue[0] &&
                      c->blue[2] == HeraldBlue(x_exe, n, s.spell_bright) &&
                      c->blue[3] == c->blue[2] && !c->red[0] && !c->green[1] && !c->red[3] &&
                      !c->green[2]);
                CHECK(((c->draw_mode & 8U) != 0) == (s.phase < 4));
                angles[i] += 0x80;
            }
            for (Uint16 i = 0; i < 16; ++i)
                CHECK(program.shield_angles[i] == angles[i]);
            ++tick;
        }
        if (failed)
            continue;
        const Uint16 expected[9] = {0, 0, 22, 30, 46, 68, 94, 206, 257};
        for (Uint16 ph = 2; ph < 9; ++ph)
            CHECK(transitions[ph] == expected[ph]);
        CHECK(s.removed && sounds == 2 && heard[0] == 0x641 && heard[1] == 0x6b1 &&
              swirl == (starved ? 0 : 1) && pinks == 2);
    }
    return true;
}
// w_023 func_ptr_80170024 against an exact model: birth (four rows, hand palette/UV band, the
// program's sixteen vectors rotated by RotMatrix(0x80,0,0xC0), 0x48 lower), brighten/hold/dim with
// its rise, the spin and per-face RotTransPers4 (x0,x1,x3,x2) visibility, and the top-of-screen
// retirement while brightening.
static bool HeraldSwirlTest(void)
{
    static Uint8 o[0x3000], exe[0x24000];
    Math(exe);
    MatrixPairs(exe);
    HeraldTables(o);
    const SotnDataView x_exe = {exe, sizeof(exe)};
    {
        // A zero RotMatrix is the identity (cosine 4096, sine 0 in the authored table).
        const Sint16 zero[3] = {0}, model[3] = {5, -7, 9};
        Sint32 out[3] = {0};
        CHECK(SotnShieldStar_Rotate(x_exe, zero, model, out) && out[0] == 5 && out[1] == -7 &&
              out[2] == 9);
    }
    for (Uint16 k = 0; k < 4; ++k)
    {
        const Uint16 hand = k & 1U;
        const bool high = k == 2, failed = k == 3;
        SotnHandWeaponProgram program = {0};
        SotnWeaponSecondary s = {.parameter = (Uint16)(hand << 15),
                                 .flags = 0x10000,
                                 .x = (120 + 30) * 65536 + 0x1357,
                                 .y = ((high ? -60 : 100) + 10) * 65536 + 0x2468};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = x_exe,
                                       .camera_x = 99,
                                       .camera_y = 77,
                                       .retained_camera_x = 30,
                                       .retained_camera_y = 10,
                                       .hand = hand,
                                       .family = 23,
                                       .role = 9,
                                       .priority = 148,
                                       .allocated = !failed};
        Uint32 rng = 5;
        Uint16 tick = 0, shown = 0, transitions[4] = {0};
        Sint16 angle = 0, bright = 0;
        Uint8 phase = 0;
        while (!s.removed && tick < 200)
        {
            const Sint32 y = s.y;
            SotnWeaponSecondaryEvents e = {0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && rng == 5 &&
                  !e.factory_count && !e.sounds[0]);
            if (failed)
            {
                CHECK(s.removed);
                break;
            }
            if (!tick)
            {
                CHECK(s.phase == 1 && s.flags == 0x04810000U && s.star_depth == 0x140 &&
                      s.spell_velocity_y == -0xe000 && s.y == y + 0x480000 && !s.spell_bright);
                for (Uint16 i = 0; i < 4; ++i)
                {
                    const SotnPlayerEffectQuad *q = &s.spell_quads[i];
                    CHECK(q->palette == (hand ? 0x12b : 0x113) && q->page == 0x19 &&
                          q->priority == 0x1bc * ((q->draw_mode & 8U) == 0) &&
                          (q->draw_mode & 0xfff7U) == 0x35);
                    for (Uint16 v = 0; v < 4; ++v)
                        CHECK(q->u[v] == (Uint8)(i * 56 + v * 14 + 3) &&
                              q->v[v] == (Uint8)(i * 56 + v * 14 + 10 + (hand ? 0x80 : 0)));
                }
                const Sint16 axis[3] = {0x80, 0, 0xc0};
                for (Uint16 i = 0; i < 16; ++i)
                {
                    const Uint16 j = (Uint16)((i & ~3U) | HeraldOrder[i & 3U]);
                    const Sint16 model[3] = {(Sint16)(j * 7 - 50), (Sint16)(j * 3 - 20),
                                             (Sint16)(40 - j * 5)};
                    Sint32 out[3] = {0};
                    CHECK(SotnShieldStar_Rotate(x_exe, axis, model, out) &&
                          program.herald_vectors[i][0] == (Sint16)out[0] &&
                          program.herald_vectors[i][1] == (Sint16)(out[1] - 0x18) &&
                          program.herald_vectors[i][2] == (Sint16)out[2]);
                }
            }
            if (s.phase != phase)
            {
                transitions[s.phase] = tick;
                phase = s.phase;
            }
            if (high && tick)
            {
                // Its screen Y starts above 0x10: the first brightening update retires it.
                CHECK(s.removed && tick == 1 && s.y == y);
                break;
            }
            if (s.removed)
                break;
            if (tick)
            {
                CHECK(s.y == (Sint32)((Uint32)y - 0xe000U));
                bright = (Sint16)(s.phase == 3 && transitions[3] != tick   ? bright - 10
                                  : s.phase <= 1 || transitions[2] == tick ? bright + 10
                                                                           : bright);
            }
            CHECK(s.spell_bright == bright);
            angle = (Sint16)(angle + 0x40);
            CHECK(s.herald_angle == angle);
            const Sint16 axis[3] = {0, angle, 0};
            const Sint32 transfer[3] = {0, 0, 0x140};
            for (Uint16 i = 0; i < 4; ++i)
            {
                Sint16 vertices[4][3] = {{0}};
                for (Uint16 v = 0; v < 4; ++v)
                    for (Uint16 c = 0; c < 3; ++c)
                        vertices[v][c] = program.herald_vectors[i * 4 + v][c];
                SotnProjectedPoint point[4] = {{0}};
                Sint32 otz = 0;
                CHECK(SotnShieldStar_Project(x_exe, axis, transfer, (Sint16)((s.x >> 16) - 30),
                                             (Sint16)((s.y >> 16) - 10), 0x200, vertices, point,
                                             &otz));
                const SotnPlayerEffectQuad *q = &s.spell_quads[i];
                CHECK(q->x[0] == point[0].x && q->y[1] == point[1].y && q->x[3] == point[2].x &&
                      q->y[3] == point[2].y && q->x[2] == point[3].x && q->y[2] == point[3].y);
                const bool show = otz > 0 && otz <= 0x1be;
                CHECK(((q->draw_mode & 8U) == 0) == show);
                if (show)
                    CHECK(q->priority == 0x1bc && q->red[0] == (Uint8)bright &&
                          q->blue[3] == (Uint8)bright);
                shown += show;
            }
            ++tick;
        }
        if (failed || high)
            continue;
        // 25 brightening updates (to 0xFA), 0x20 holding, 25 dimming (below 9).
        CHECK(transitions[2] == 25 && transitions[3] == 57 && tick == 82 && shown > 50);
    }
    return true;
}
// w_023 func_ptr_80170028 against an exact model: birth copies the raw parent's palette and unk5A,
// FLAG_UNK_100000/KEEP_ALIVE, one rand for its pose (poseTimer2), and waits params*4 with no
// animSet (params0: 65536 updates); then it copies the parent's animSet and for 0x40 updates
// rises and sways about its cached screen X with priority and scale from its angle.
static bool HeraldSpriteTest(void)
{
    static Uint8 o[0x3000], exe[0x24000];
    Math(exe);
    HeraldTables(o);
    const SotnDataView x_exe = {exe, sizeof(exe)};
    for (Uint16 k = 0; k < 3; ++k)
    {
        const Uint16 index = k == 0 ? 3 : k == 1 ? 23 : 0;
        SotnHandWeaponProgram program = {0};
        SotnWeaponSecondary s = {.parameter = (Uint16)(0x8000U | index),
                                 .flags = 0x10000,
                                 .x = (120 + 30) * 65536 + 0x1357,
                                 .y = (100 + 10) * 65536 + 0x2468};
        SotnWeaponSecondaryInput in = {.overlay = {o, sizeof(o)},
                                       .executable = x_exe,
                                       .camera_x = 99,
                                       .camera_y = 77,
                                       .retained_camera_x = 30,
                                       .retained_camera_y = 10,
                                       .hand = 1,
                                       .family = 23,
                                       .role = 10,
                                       .priority = 148,
                                       .parent_palette = 0x128,
                                       .parent_texture = 0x66,
                                       .parent_bank = 0x8012};
        Uint32 rng = 77, check = 77;
        SotnWeaponSecondaryEvents e = {0};
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
        const Uint16 pose = (Uint16)((SotnCombat_Random(&check) & 0x7fffU) % 5U);
        CHECK(rng == check && s.phase == 1 && s.pose == pose && s.remaining == 2 &&
              s.palette == 0x128 && s.texture == 0x66 && !s.bank && s.flags == 0x04110000U &&
              s.draw_flags == 3 && !s.scale_x && !s.scale_y &&
              s.y == (100 + 10 + 40) * 65536 + 0x2468 && s.spell_velocity_y == -0x18000 &&
              s.herald_x == 120 && s.herald_angle == 0x400 && s.spell_timer == (Sint16)(index * 4));
        CHECK(SotnWeaponSecondary_Animate(&s, (SotnDataView){o, sizeof(o)}, 23) &&
              s.remaining == 1 && s.frame == 0x3f + pose);
        const Uint32 wait = index ? index * 4U : 65536U;
        for (Uint32 n = 1; n < wait; ++n)
        {
            in.parent_bank = (Uint16)(0x8000U + n);
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 1 && !s.bank &&
                  rng == check);
        }
        in.parent_bank = 0x8012;
        in.parent_palette = 0x999;
        e = (SotnWeaponSecondaryEvents){0};
        CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e) && s.phase == 2 &&
              s.bank == 0x8012 && s.spell_timer == 0x40 && s.palette == 0x128);
        Sint16 angle = 0x400;
        Sint32 y = s.y;
        for (Uint16 n = 0; n < 0x40; ++n)
        {
            in.retained_camera_x = (Sint32)(30 + n);
            const Sint32 x = s.x;
            e = (SotnWeaponSecondaryEvents){0};
            CHECK(SotnWeaponSecondary_Tick(&s, &program, in, &rng, &e));
            if (n == 0x3f)
            {
                CHECK(s.removed && s.y == y && s.x == x);
                break;
            }
            angle = (Sint16)((angle + 0x80) & 0xfff);
            y = (Sint32)((Uint32)y - 0x18000U);
            SotnMathTrig t = {0};
            CHECK(SotnMath_Trig(x_exe, angle, &t));
            CHECK(!s.removed && s.y == y && s.herald_angle == angle &&
                  (s.x >> 16) == 120 + HeraldShift(t.cosine * 2, 8) + 30 + n &&
                  (s.x & 0xffff) == (x & 0xffff) && s.priority == (angle < 0x800 ? 0x1b4 : 0x1bc) &&
                  s.scale_x == (Sint16)(-HeraldShift(t.sine, 5) + 0xc0) && s.scale_y == s.scale_x);
        }
    }
    return true;
}
// Herald shield's spell207 in the runtime: E8/F8 spell (eighteen rows), FACTORY(94)'s twenty-four
// E10/F10 sprites (spawn index params, the spell's palette/unk5A at birth and animSet at the end
// of each wait, slots63 down), FACTORY(90)'s E9/F9 swirl (four rows) created before pink9's
// factory, the freeze set and released once. The params0 sprite outlives the spell, waiting.
static bool HeraldRuntime(Uint16 hand)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(56, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    static Uint8 exe[0x24000];
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Math(exe);
    MatrixPairs(exe);
    HeraldTables(weapons + 23U * 0x7000U + 0x4000U);
    FamilyDefinition(dra, 9, 9, 23, 128, 0, 0, 0);
    FamilyDefinition(dra, 207, 10, 23, 5, 0, 10, 0);
    Uint8 *record = dra + 0x500 + 207 * 52;
    record[18] = 0xba;
    record[19] = 86;
    record[21] = 0;
    record[22] = 1;
    const Uint8 blueprints[5][6] = {{8, 1, 0xc1, 0, 6, 0},
                                    {10, 24, 0xd8, 0, 0, 0},
                                    {9, 1, 0xc1, 0, 0, 0},
                                    {11, 1, 0x81, 0, 0, 0},
                                    {39, 1, 0x81, 0, 0, 0}};
    const Uint16 ids[5] = {86, 94, 90, 44, 55};
    for (Uint16 b = 0; b < 5; ++b)
        SDL_memcpy(dra + 0xd1d4 + ids[b] * 6U, blueprints[b], 6);
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    for (Uint16 i = 0; i < 2; ++i)
    {
        Uint8 *q = dra + 0xd670 + (i ? 0x47U : 0x40U) * 20U;
        Put16(q + 6, 0x100);
        for (Uint16 k = 8; k <= 12; k += 2)
            Put16(q + k, 256);
        Put16(q + 14, 0x7002);
        Put16(q + 16, 0x31);
        Put16(q + 18, 512);
    }
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    CHECK(SotnCharacter_Create(actors->dra, &character));
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = {exe, sizeof(exe)},
                              .weapons = {weapons, 56U * 0x7000},
                              .weapon_family = {hand ? 57 : 23, hand ? 23 : 57},
                              .weapon_bound = {true, true},
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 200 * 65536,
                                         .y = 135 * 65536,
                                         .priority = 148,
                                         .facing_left = hand == 0}};
    const SotnPlayerFactorySource source = {
        .x = in.player.x, .y = in.player.y, .priority = 148, .facing_left = hand == 0};
    bool allocated = false;
    CHECK(SotnBatRuntime_RequestHand(r, source, 86U | (hand + 1U) << 14 | (Uint32)hand << 23, 207,
                                     &allocated) &&
          allocated);
    Uint16 spell_slot = 0, spell_ticks = 0, freezes = 0, released = 0, most_sprites = 0,
           swirl_ticks = 0, shown = 0, ordered = 0, waiting = 0;
    Uint32 indices = 0;
    Uint16 t = 0;
    Sint32 wait_screen[SOTN_BAT_RUNTIME_SLOTS] = {0};
    Uint16 retained = 0;
    for (; t < 700; ++t)
    {
        in.game_timer = t;
        in.engine_timer = t;
        // Every eighth update the camera moves right by two after the callbacks.
        in.screen_shift_x = (Sint16)(t % 8 == 7 ? -2 : 0);
        in.camera_x += t % 8 == 7 ? 2 : 0;
        in.freeze = r->freeze;
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        freezes += r->freeze_changed && r->freeze == 1;
        released += r->freeze_changed && r->freeze == 0;
        // The swirl's factory precedes pink9's in the factory table.
        Sint16 swirl_factory = -1, pink_factory = -1;
        for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        {
            const SotnPlayerFactory *f = &r->factories[i].cursor;
            if (f->entity_id && f->blueprint == 90 && swirl_factory < 0)
                swirl_factory = (Sint16)i;
            if (f->entity_id && f->blueprint == 55 && pink_factory < 0)
                pink_factory = (Sint16)i;
        }
        if (swirl_factory >= 0 && pink_factory >= 0)
            ordered += swirl_factory < pink_factory;
        Uint16 sprites = 0;
        bool spell = false;
        for (Uint16 i = 1; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        {
            const SotnBatRuntimeChild *c = &r->children[i];
            if (!c->weapon_secondary || c->state.secondary.family != 23 || !c->initialized)
                continue;
            const SotnWeaponSecondary *w = &c->state.secondary.motion;
            CHECK(SotnBatRuntime_SecondaryValid(c, i) && (w->flags & 0x10000U));
            if ((c->kind & 15U) == 8)
            {
                CHECK(c->kind == (hand ? 0xf8 : 0xe8) && c->primitive_count == 18 &&
                      c->state.secondary.item == 207 && c->state.secondary.own_item);
                spell = true;
                spell_slot = i;
                ++spell_ticks;
                continue;
            }
            CHECK(c->state.secondary.parent_slot == spell_slot + 16U && !c->state.secondary.item &&
                  !c->state.secondary.attack.active && !c->group);
            if ((c->kind & 15U) == 9)
            {
                CHECK(c->kind == (hand ? 0xf9 : 0xe9) && c->primitive_count == 4 &&
                      w->spell_quads[0].palette == (hand ? 0x12b : 0x113));
                ++swirl_ticks;
                for (Uint16 q = 0; q < 4; ++q)
                    shown += !(w->spell_quads[q].draw_mode & 8U);
                continue;
            }
            CHECK(c->kind == (hand ? 0xfa : 0xea) && !c->primitive_count &&
                  w->palette == (hand ? 0x128 : 0x110) && w->texture == (hand ? 0x66 : 0x64) &&
                  i >= 16 && (c->parameter & 0xffU) <= 23 &&
                  w->bank == (w->phase == 2 ? (hand ? 0x8012 : 0x8010) : 0));
            indices |= 1U << (c->parameter & 0xffU);
            ++sprites;
            // Waiting sprites keep their screen position across camera moves.
            const Sint32 screen = (w->x >> 16) - in.camera_x;
            if (w->phase == 1 && wait_screen[i])
            {
                CHECK(screen == wait_screen[i]);
                retained += in.screen_shift_x != 0;
            }
            wait_screen[i] = w->phase == 1 ? screen : 0;
        }
        most_sprites = (Uint16)SDL_max(most_sprites, sprites);
        if (!spell && spell_ticks && sprites <= 1 && t > 300)
            break;
    }
    CHECK(spell_ticks > 250 && most_sprites == 24 && indices == 0xffffffU && swirl_ticks > 70 &&
          shown > 50 && ordered > 0 && freezes == 1 && released == 1 && retained > 10);
    // Only the params0 sprite remains, still waiting: its countdown wrapped below zero.
    for (Uint16 i = 1; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
    {
        SotnBatRuntimeChild *c = &r->children[i];
        if (!c->weapon_secondary)
            continue;
        CHECK((c->kind & 15U) == 10 && !(c->parameter & 0xffU) &&
              c->state.secondary.motion.phase == 1 && c->state.secondary.motion.spell_timer < 0 &&
              !c->primitive_count);
        ++waiting;
        // Fast-forward its wait: the empty raw parent slot gives animSet0, then 0x40 updates.
        c->state.secondary.motion.spell_timer = 1;
    }
    CHECK(waiting == 1);
    for (Uint16 n = 0; n <= 0x40; ++n)
    {
        in.freeze = r->freeze;
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        for (Uint16 i = 1; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
            if (r->children[i].weapon_secondary)
                CHECK(n < 0x40 && !r->children[i].state.secondary.motion.bank &&
                      r->children[i].state.secondary.motion.phase == 2);
    }
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        CHECK(!r->children[i].weapon_secondary && !r->children[i].primitive_count);
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        CHECK(!actors->primitives[i].owner);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}

int main(void)
{
    return Module() && Lunge() && Slash() && Thunder() && Warp() && Fist() && Fan() && Brand() &&
                   Ice() && Moon() && Runtime() && KatanaRuntime() && ThunderRuntime() &&
                   WarpRuntime() && FistRuntime() && FanRuntime() && BrandRuntime() &&
                   IceRuntime() && MoonRuntime() && Rune() && RuneRuntime() && Heaven() &&
                   HeavenCombo() && HeavenRuntime() && HeavenComboRuntime() && RodOrbTest() &&
                   RodSparkleTest() && RodRuntime() && IronRuntime(0) && IronRuntime(1) &&
                   IronThunderRuntime() && IronSpellTest() && IronSwordTest() &&
                   HeraldSpellTest() && HeraldSwirlTest() && HeraldSpriteTest() &&
                   HeraldRuntime(0) && HeraldRuntime(1) && MuramasaSlash() && Twelve() &&
                   TwelveRuntime() && Yasutsuna() && YasutsunaRuntime(0) && YasutsunaRuntime(1) &&
                   ShotelBoomerang() && ShotelRuntime(0) && ShotelRuntime(1) && DawnCircleTest(0) &&
                   DawnCircleTest(1) && DawnSoldierTest(0) && DawnSoldierTest(1) &&
                   DawnArrowTest(0) && DawnArrowTest(1) && DawnBombTest(0) && DawnBombTest(1) &&
                   LeatherSpellTest(0, false) && LeatherSpellTest(1, false) &&
                   LeatherSpellTest(0, true) && LeatherSpellTest(1, true) && AlucartSpellTest(0) &&
                   AlucartSpellTest(1) && AxeLordSpellTest(0) && AxeLordSpellTest(1) &&
                   AxeLordWalkerTest(0) && AxeLordWalkerTest(1) && AxeLordRuntime(0) &&
                   AxeLordRuntime(1) && GoddessSpellTest(0) && GoddessSpellTest(1) &&
                   GoddessCloudTest(0) && GoddessCloudTest(1) && GoddessRuntime(0, false) &&
                   GoddessRuntime(1, false) && GoddessRuntime(0, true) && GoddessRuntime(1, true) &&
                   ShamanSpellTest(0) && ShamanSpellTest(1) && ShamanStarTest(0, 3) &&
                   ShamanStarTest(1, 3) && ShamanStarTest(0, 2) && ShamanTrailTest() &&
                   ShamanRuntime(0) && ShamanRuntime(1) && AlucardSpellTest(0) &&
                   AlucardSpellTest(1) && AlucardLettersTest(0) && AlucardLettersTest(1)
               ? 0
               : 1;
}
