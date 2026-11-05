// Weapon func_ptr_80170004 secondaries (w_055 Holy sword, w_038/w_039 lunges) with authored tables
// only: mandatory
// LINE_G2 rows before RNG, raw-parent properties/group, hitbox shrink, post-callback UpdateAnim,
// blueprint56 runtime ownership, shared rows, holds, rebases and failure atomicity.
#include "actors_internal.h"
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
          !SotnWeaponSecondary_SupportedRole(13, 2) && !SotnWeaponSecondary_RoleRows(13, 0));
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
static bool LeatherSpellTest(Uint16 hand)
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
                                   .family = 8,
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
          s.spell_quads[1].v[0] == (hand ? 0xb0 : 0x30) && s.spell_quads[10].u[1] == 71 &&
          s.spell_quads[10].palette == 0x15f && s.spell_quads[10].draw_mode == 0x3d &&
          s.spell_quads[10].priority == 0x1be && program.shield_angles[0] == 0x40 &&
          program.shield_angles[8] == 0x1040 && program.shield_angles[9] == 0x1200);
    Uint16 t = 0, pink = 0, flashes = 0, brightening = 0;
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
        if (e.sounds[0] == 0x6b1)
        {
            CHECK(phase == 3 && s.phase == 4 && !s.frame && !(s.spell_quads[1].draw_mode & 8U) &&
                  !(s.spell_quads[5].draw_mode & 8U) && s.spell_timer == 0x20);
            ++flashes;
        }
        if (e.pink)
        {
            CHECK(e.pink == 8 && phase == 6 && s.phase == 7 && s.spell_bright > 0x120 &&
                  (s.spell_quads[1].draw_mode & 8U));
            ++pink;
        }
        if (s.removed)
            CHECK(e.freeze_clear && phase == 7 && s.spell_wave_y > 0xc0);
    }
    CHECK(s.removed && pink == 1 && flashes == 1 && brightening == 19 &&
          program.shield_angles[0] == (Sint32)(0x40 * (t + 0U)));
    CHECK(SotnWeaponSecondary_SupportedRole(8, 8) && !SotnWeaponSecondary_SupportedRole(8, 1) &&
          SotnWeaponSecondary_RoleRows(8, 8) == 11);
    return true;
}

int main(void)
{
    return Module() && Lunge() && Slash() && Thunder() && Warp() && Fist() && Fan() && Brand() &&
                   Ice() && Moon() && Runtime() && KatanaRuntime() && ThunderRuntime() &&
                   WarpRuntime() && FistRuntime() && FanRuntime() && BrandRuntime() &&
                   IceRuntime() && MoonRuntime() && Rune() && RuneRuntime() && Heaven() &&
                   HeavenRuntime() && MuramasaSlash() && Twelve() && TwelveRuntime() &&
                   Yasutsuna() && YasutsunaRuntime(0) && YasutsunaRuntime(1) && ShotelBoomerang() &&
                   ShotelRuntime(0) && ShotelRuntime(1) && DawnCircleTest(0) && DawnCircleTest(1) &&
                   DawnSoldierTest(0) && DawnSoldierTest(1) && DawnArrowTest(0) &&
                   DawnArrowTest(1) && DawnBombTest(0) && DawnBombTest(1) && LeatherSpellTest(0) &&
                   LeatherSpellTest(1)
               ? 0
               : 1;
}
