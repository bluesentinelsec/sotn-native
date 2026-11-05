#include "actors_internal.h"
#include "character_fixture.h"
#include <sotn/bat_runtime.h>
#include <sotn/combat.h>
#include <sotn/hand_weapon.h>
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
static bool Properties(void *context, Uint32 *random, SotnEquipment *out)
{
    (void)context;
    (void)SotnCombat_Random(random);
    *out = (SotnEquipment){.id = 86, .attack = 19, .hit_type = 2, .element = 64};
    return true;
}
static void Resources(Uint8 *o)
{
    // Authored indirection, frame stream and signed properties; no original tables.
    const Uint16 descriptors[] = {0x106c, 0xbc4, 0x8ec, 0x6a4, 0x1014};
    for (Uint16 i = 0; i < SDL_arraysize(descriptors); ++i)
    {
        Uint8 *d = o + descriptors[i];
        Put32(d, 0x8017a100);
        Put32(d + 4, 0x8017a200);
        Put16(d + 10, 0x611);
        d[12] = 0x41;
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
    for (Uint16 i = 0; i < 14; ++i)
        Put32(o + 0xb84 + i * 4U, 0x8017a400);
    Put16(o + 0x400, 1);
    Put16(o + 0x402, 512U + 61U);
    Put16(o + 0x404, 65535);
    o[0xb40] = 249;
    o[0xb41] = 254;
    o[0xb42] = 8;
    o[0xb43] = 16;
}
static bool Run(void)
{
    Uint8 o[0x1200] = {0};
    Resources(o);
    SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                              .properties = Properties,
                              .x = 0x7fff1234,
                              .y = -78123,
                              .priority = 1,
                              .animation = 0x41,
                              .pose = 4,
                              .remaining = 1,
                              .attack_mode = 0x8002,
                              .scale_y = -123,
                              .pivot_y = -4,
                              .draw_flags = 2,
                              .facing_left = true};
    SotnHandWeapon w = {0};
    SotnHandWeaponProgram program = {0};
    SotnHandWeaponEvents e = {0};
    Uint32 rng = 7, expected = rng;
    (void)SotnCombat_Random(&expected);
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e));
    CHECK(rng == expected && e.properties && e.sound == 0x611 && !w.removed && w.phase == 1 &&
          w.x == in.x && w.y == in.y && w.facing_left && w.priority == 65535 && w.bank == 0x8010 &&
          w.texture == 100 && w.flags == 0x60000 && w.blend == 0x30 && w.hitbox_x == -4 &&
          w.hitbox_y == -6 && w.frame == 23 && w.scale_y == -123 && w.pivot_y == -4 &&
          w.properties.attack == 19);
    const SotnHandWeapon before = w;
    const SotnHandWeaponProgram before_program = program;
    const SotnHandWeaponEvents before_events = e;
    in.overlay.size = 0x205;
    CHECK(!SotnHandWeapon_Tick(&w, &program, in, &rng, &e));
    CHECK(!SDL_memcmp(&w, &before, sizeof(w)) && rng == expected &&
          !SDL_memcmp(&program, &before_program, sizeof(program)) &&
          !SDL_memcmp(&e, &before_events, sizeof(e)));
    in.overlay.size = sizeof(o);
    in.remaining = -1;
    w.hitbox_x = 88;
    w.draw_flags = 77;
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e));
    CHECK(w.removed && w.hitbox_x == -4 && w.draw_flags == 77 && rng == expected);
    w = (SotnHandWeapon){.hitbox_x = 88, .draw_flags = 77};
    Put16(o + 0x308, 65535);
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e));
    CHECK(w.removed && e.properties && w.hitbox_x == 88 && w.draw_flags == 77 && e.sound == 0 &&
          w.phase == 1);
    w = (SotnHandWeapon){0};
    in.attack_mode = 0;
    expected = rng;
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e));
    CHECK(w.removed && !e.properties && rng == expected);
    // Alucard sword allocates optional real tiles before properties, then consumes three
    // random values per tile only when the original player pose/timer opens their birth.
    Put16(o + 0x308, 512U + 23U);
    in.attack_mode = 0x8002;
    in.family = 43;
    in.pose = 3;
    in.remaining = 1;
    w = (SotnHandWeapon){0};
    expected = rng;
    (void)SotnCombat_Random(&expected);
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && rng == expected && !w.allocated);
    w = (SotnHandWeapon){0};
    in.allocated = true;
    expected = rng;
    for (Uint16 i = 0; i < 49; ++i)
        (void)SotnCombat_Random(&expected);
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e));
    CHECK(rng == expected && w.allocated && w.flags == 0x860000 && w.tile_started[15] &&
          w.tile_age[15] == 9 && w.tiles[15].draw_mode == 0x239);
    in.pose = 4;
    for (Uint16 i = 0; i < 13; ++i)
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && rng == expected);
    CHECK(w.tiles[15].draw_mode & 8U);
    // Shield knock-away uses full fractions, signed wrap and TWO timer decrements.
    // Its final original drawFlags assignment overwrites the provisional rotate/Blink bits.
    in.family = 52;
    in.status = 0;
    in.held = true;
    in.hand = 1;
    w = (SotnHandWeapon){0};
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e));
    CHECK(w.phase == 1 && e.shield == 1 && e.properties && w.properties.attack == 1 &&
          program.shield_attack == 19 && w.bank == 0x8012 && w.frame == 61);
    in.status = 0x10000;
    in.contacts = 0;
    in.velocity_x = 0x40000001;
    const Sint32 origin_x = in.x;
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e));
    CHECK(w.phase == 4 && w.timer == 126 && w.flags == 0x8000000 && w.hitbox_state == 0 &&
          e.shield == 2 && w.velocity_x == -2147483646 && w.x == origin_x - 2147483646 &&
          w.draw_flags == in.draw_flags && w.rotation == 128);
    for (Uint16 i = 0; i < 62; ++i)
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && !w.removed);
    CHECK(w.timer == 2);
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && w.removed && !w.timer);
    return true;
}
static bool BasicShieldProperties(void *context, Uint32 *random, SotnEquipment *out)
{
    if (!Properties(context, random, out))
        return false;
    out->hit_type = 4;
    return true;
}
static bool BasicShields(void)
{
    // Leather8 shares Knight9's layout, Goddess25 Shaman26's and Alucart58 Alucard52's;
    // only weapon58 (like 52) caches the property attack and overrides it to one.
    const Uint16 families[] = {9, 11, 26, 8, 25, 58},
                 tables[] = {0xa40, 0xb94, 0xa60, 0xa40, 0xa60, 0xb84};
    CHECK(SotnHandWeapon_IsShieldFamily(10) && SotnHandWeapon_IsShieldFamily(23) &&
          SotnHandWeapon_IsShieldFamily(24) && SotnHandWeapon_IsShieldFamily(27) &&
          SotnHandWeapon_IsShieldFamily(28) && !SotnHandWeapon_IsShieldFamily(29) &&
          !SotnHandWeapon_IsShieldFamily(255));
    for (Uint16 i = 0; i < SDL_arraysize(families); ++i)
        for (Uint16 hand = 0; hand < 2; ++hand)
        {
            Uint8 overlay[0x1200] = {0};
            Resources(overlay);
            const Uint16 table = tables[i], props = (Uint16)(table - 0x48U + 4U);
            for (Uint16 animation = 0; animation < 14; ++animation)
                Put32(overlay + table + animation * 4U, 0x8017a400);
            overlay[props] = 249;
            overlay[props + 1] = 254;
            overlay[props + 2] = 8;
            overlay[props + 3] = 16;
            SotnHandWeaponInput in = {.overlay = {overlay, table},
                                      .family = families[i],
                                      .hand = hand,
                                      .properties = BasicShieldProperties,
                                      .x = 0x123456,
                                      .y = -345678,
                                      .priority = 148,
                                      .held = true};
            SotnHandWeapon w = {0};
            SotnHandWeaponProgram program = {.shield_attack = 123};
            SotnHandWeaponEvents events = {.sound = 777};
            Uint32 random = 7, expected = random;
            // Failed private table reads cannot publish birth properties/RNG or events.
            const SotnHandWeapon before = w;
            CHECK(!SotnHandWeapon_Tick(&w, &program, in, &random, &events) && random == 7 &&
                  !SDL_memcmp(&w, &before, sizeof(w)) && program.shield_attack == 123 &&
                  events.sound == 777);
            in.overlay.size = sizeof(overlay);
            (void)SotnCombat_Random(&expected);
            CHECK(SotnHandWeapon_IsShieldFamily(in.family) &&
                  SotnHandWeapon_Tick(&w, &program, in, &random, &events) && random == expected &&
                  events.shield == 1 && events.properties && w.hitbox_state == 4 &&
                  w.properties.attack == (families[i] == 58 ? 1 : 255) &&
                  program.shield_attack == (families[i] == 58 ? 255 : 123) &&
                  w.bank == 0x8010U + hand * 2U && w.frame == 61 && w.hitbox_x == -7 &&
                  w.hitbox_y == -2 && w.hitbox_width == 8 && w.hitbox_height == 16);
            in.held = false;
            for (Uint16 frame = 0; frame < 4 && !w.removed; ++frame)
                CHECK(SotnHandWeapon_Tick(&w, &program, in, &random, &events) &&
                      events.shield == 2 && !events.properties && random == expected);
            CHECK(w.removed && program.shield_attack == (families[i] == 58 ? 255 : 123));
        }
    return true;
}
static Uint16 Read16Test(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
// Weapon3/33/34/35/36/57 run w_002's callback from their own descriptors; blends differ.
static bool PlainWeapons(void)
{
    static Uint8 o[0x1400];
    Resources(o);
    SDL_memcpy(o + 0xc28, o + 0x106c, 16);
    SDL_memcpy(o + 0x540, o + 0x106c, 16);
    SDL_memcpy(o + 0x109c, o + 0x106c, 16);
    SDL_memcpy(o + 0xc3c, o + 0x106c, 16);
    SDL_memcpy(o + 0x530, o + 0x106c, 16);
    SDL_memcpy(o + 0x934, o + 0x106c, 16);
    Put16(o + 0x934 + 8, 2);
    SDL_memcpy(o + 0x5e4, o + 0x106c, 16);
    Put16(o + 0xc28 + 8, 5);
    SDL_memcpy(o + 0x12f4, o + 0x106c, 16);
    SDL_memcpy(o + 0x1304, o + 0x106c, 16);
    const Uint16 families[] = {3, 36, 57, 2, 33, 35, 4, 5, 34, 30},
                 variants[] = {0, 0, 3, 0, 0, 0, 1, 1, 0, 0};
    for (Uint16 k = 0; k < SDL_arraysize(families); ++k)
        for (Uint16 hand = 0; hand < 2; ++hand)
        {
            if (families[k] == 2)
                SDL_memcpy(o + 0xbc4, o + 0x106c, 16);
            SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                                      .properties = Properties,
                                      .family = families[k],
                                      .hand = hand,
                                      .priority = 100,
                                      .animation = 0x41,
                                      .pose = 4,
                                      .remaining = 1,
                                      .attack_mode = 0x8002};
            SotnHandWeapon w = {.parameter = (Uint16)(variants[k] * 256U + hand * 32768U)};
            SotnHandWeaponProgram program = {.short_pose = 4, .shield_attack = 9};
            SotnHandWeaponEvents e = {0};
            Uint32 rng = 21, expected = 21;
            (void)SotnCombat_Random(&expected);
            CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && rng == expected &&
                  e.properties &&
                  w.blend == (families[k] == 2 || families[k] == 33    ? 0
                              : families[k] == 35 || families[k] == 34 ? 0x10
                              : families[k] == 30                      ? 0x70
                                                                       : 0x30) &&
                  e.sound == Read16Test(o + 0x106c + 10) &&
                  e.extra_sounds[0] == (families[k] == 4 || families[k] == 5 ? 0x640 : 0) &&
                  !e.extra_sounds[1] && w.bank == (families[k] == 30 ? 10U : 0x8010U + hand * 2U) &&
                  w.priority == 98 &&
                  w.palette == 0x110U + hand * 24U +
                                   (families[k] == 3    ? 5U
                                    : families[k] == 34 ? 2U
                                                        : 0U) &&
                  program.short_pose == 4 && program.shield_attack == 9);
            // Nunchaku repeats its descriptor swing and adds SWISH_B at pose12 (timer1).
            in.pose = 12;
            SotnHandWeaponEvents swing = {0};
            CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &swing) &&
                  swing.extra_sounds[0] == (families[k] == 33 ? Read16Test(o + 0x106c + 10) : 0) &&
                  swing.extra_sounds[1] == (families[k] == 33 ? 0x60b : 0));
            // Star flail widens its reach by seven on poses6 and7 only.
            in.pose = 6;
            SotnHandWeaponEvents reach = {0};
            const Uint8 width = w.hitbox_width;
            const Sint16 x = w.hitbox_x;
            CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &reach) &&
                  w.hitbox_width == width + (families[k] == 5 ? 7 : 0) &&
                  w.hitbox_x == x + (families[k] == 5 ? 7 : 0));
            in.pose = 4;
            // Single-descriptor programs reject any other variant before mutation.
            if (families[k] != 57 && families[k] != 4 && families[k] != 5)
            {
                SotnHandWeapon other = {.parameter = 0x100};
                Uint32 kept = 21;
                CHECK(!SotnHandWeapon_Tick(&other, &program, in, &kept, &e) && kept == 21 &&
                      !other.phase);
            }
        }
    return true;
}
// Weapon32 Combat knife lingers four updates (no sprite/hitbox) instead of retiring; weapon51
// Great Sword draws from DRA bank10 without a hand offset. Authored tables only.
static bool KnifeAndGreatSword(void)
{
    static Uint8 o[0x1400];
    Resources(o);
    SDL_memcpy(o + 0x8f8, o + 0x106c, 16);
    SDL_memcpy(o + 0x908, o + 0x106c, 16);
    Put16(o + 0x908 + 10, 0x60b); // special descriptor: SWISH_B adds the attack voice
    SDL_memcpy(o + 0xcd8, o + 0x106c, 16);
    for (Uint16 hand = 0; hand < 2; ++hand)
    {
        SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                                  .properties = Properties,
                                  .family = 51,
                                  .hand = hand,
                                  .priority = 100,
                                  .animation = 0x41,
                                  .pose = 4,
                                  .remaining = 1,
                                  .attack_mode = 0x8002};
        SotnHandWeapon w = {.parameter = (Uint16)(hand << 15)};
        SotnHandWeaponProgram program = {0};
        SotnHandWeaponEvents e = {0};
        Uint32 rng = 3;
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && w.bank == 10 && !w.blend &&
              w.texture == 100U + hand * 2U && w.palette == 0x110U + hand * 24U);
        // Weapon51 owns descriptors0..3 and Estoc's variant8; others stay out of bounds.
        SotnHandWeapon other = {.parameter = 0x400};
        CHECK(!SotnHandWeapon_Tick(&other, &program, in, &rng, &e));
        other = (SotnHandWeapon){.parameter = 0x900};
        CHECK(!SotnHandWeapon_Tick(&other, &program, in, &rng, &e));
        // Knife: variant0 draws in front of the player, the special behind it.
        for (Uint16 variant = 0; variant < 2; ++variant)
        {
            in.family = 32;
            in.pose = 4;
            in.remaining = 1;
            in.attack_mode = 0x8002;
            SotnHandWeapon k = {.parameter = (Uint16)((variant << 8) | (hand << 15))};
            SotnHandWeaponEvents ke = {0};
            CHECK(SotnHandWeapon_Tick(&k, &program, in, &rng, &ke) && k.phase == 1 &&
                  k.priority == (variant ? 98 : 102) && !k.blend &&
                  ke.extra_sounds[0] == (variant ? 0x6ef : 0));
            // Pose6 within the first four animations becomes a hit-state4 parry.
            in.pose = 6;
            CHECK(SotnHandWeapon_Tick(&k, &program, in, &rng, &ke) && k.hitbox_state == 4);
            // The attack ending lingers four updates without a sprite or hitbox.
            in.attack_mode = 0;
            CHECK(SotnHandWeapon_Tick(&k, &program, in, &rng, &ke) && k.phase == 2 &&
                  k.timer == 3 && !k.frame && !k.hitbox_state && !k.removed);
            for (Uint16 i = 0; i < 2; ++i)
                CHECK(SotnHandWeapon_Tick(&k, &program, in, &rng, &ke) && !k.removed);
            CHECK(SotnHandWeapon_Tick(&k, &program, in, &rng, &ke) && k.removed);
        }
    }
    return true;
}
// Weapon54: optional LINE_G2 rows with their birth rand calls, Dark descriptor2/attack doubling,
// timer11 floor and the descriptor1 outline request. Authored tables only.
static bool Gurthang(void)
{
    static Uint8 o[0x1400];
    Resources(o);
    for (Uint16 v = 0; v < 3; ++v)
        SDL_memcpy(o + 0xec4 + v * 16U, o + 0x106c, 16);
    Put16(o + 0x300 + 2U * 2U, 512U + 13U); // pose2 shows trigger frame0xD
    Put16(o + 0xef4, (Uint16)-9);
    Put16(o + 0xefc, (Uint16)-8);
    struct
    {
        Uint16 variant, dark, allocated;
    } cases[] = {{0, 0, 1}, {0, 0, 0}, {1, 0, 1}, {0, 0x40, 1}, {0, 0x10, 1}};
    for (Uint16 k = 0; k < SDL_arraysize(cases); ++k)
    {
        SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                                  .properties = Properties,
                                  .family = 54,
                                  .priority = 148,
                                  .x = 100 * 65536,
                                  .y = 80 * 65536,
                                  .camera_x = 20,
                                  .camera_y = 10,
                                  .animation = 0x41,
                                  .pose = 2,
                                  .remaining = 1,
                                  .attack_mode = 0x8002,
                                  .dark_ticks = cases[k].dark,
                                  .allocated = cases[k].allocated != 0};
        SotnHandWeapon w = {.parameter = (Uint16)(cases[k].variant << 8)};
        SotnHandWeaponProgram program = {0};
        SotnHandWeaponEvents e = {0};
        Uint32 rng = 33, expected = 33;
        for (Uint16 i = 0; i < (cases[k].allocated ? 28 : 0) + 1; ++i)
            (void)SotnCombat_Random(&expected);
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && rng == expected &&
              w.allocated == (cases[k].allocated != 0) &&
              w.flags == (cases[k].allocated ? 0x860000U : 0x60000U));
        const bool alternate = cases[k].variant || cases[k].dark;
        CHECK(w.properties.attack == (cases[k].dark && alternate ? 38 : 19) &&
              e.dark_minimum == (alternate && cases[k].dark < 0x38 ? 0x38 : 0) &&
              e.outline == (cases[k].variant == 1 && !cases[k].dark ? 0x11003dU : 0U));
        if (cases[k].allocated)
        {
            // Frame0xD places every row from screen coordinates; row0's delta starts at -1.
            const SotnPlayerEffectQuad *q = &w.tiles[0];
            CHECK(w.frame == 13 && !(q->draw_mode & 8U) && q->x[2] == -1 &&
                  q->x[0] == 100 - 20 - 9 && q->x[1] == q->x[0] - 1 && q->y[0] == 80 - 10 - 8 &&
                  q->red[0] == 255 && q->priority == 148 && w.tiles[11].red[2] == 0x18);
        }
    }
    return true;
}
static bool PowerProperties(void *context, Uint32 *random, SotnEquipment *out)
{
    (void)SotnCombat_Random(random);
    *out = (SotnEquipment){.id = 141, .attack = *(const Sint16 *)context, .hit_type = 2};
    return true;
}
// w_056: Gurthang's callback over Muramasa's own tables (descriptors0xF64, rows0xF94/0xF9C);
// timer11 selects descriptor2 without doubling, and attack0x1E or more requests WFACTORY(56,0).
static bool Muramasa(void)
{
    static Uint8 o[0x1400];
    Resources(o);
    for (Uint16 v = 0; v < 3; ++v)
        SDL_memcpy(o + 0xf64 + v * 16U, o + 0x106c, 16);
    o[0xf64 + 2U * 16U + 12] = 0x50; // descriptor2's frameStart differs
    Put16(o + 0x300 + 2U * 2U, 512U + 13U);
    Put16(o + 0xf94, (Uint16)-7);
    Put16(o + 0xf9c, (Uint16)-6);
    const struct
    {
        Sint16 attack;
        Uint16 dark, hand;
    } cases[] = {{29, 0, 0}, {30, 0, 1}, {45, 0x20, 0}, {12, 0x20, 1}};
    for (Uint16 k = 0; k < SDL_arraysize(cases); ++k)
    {
        SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                                  .properties = PowerProperties,
                                  .context = (void *)&cases[k].attack,
                                  .family = 56,
                                  .hand = cases[k].hand,
                                  .priority = 148,
                                  .x = 100 * 65536,
                                  .y = 80 * 65536,
                                  .camera_x = 20,
                                  .camera_y = 10,
                                  .animation = (Uint16)(cases[k].dark ? 0x50 : 0x41),
                                  .pose = 2,
                                  .remaining = 1,
                                  .attack_mode = 0x8002,
                                  .dark_ticks = cases[k].dark,
                                  .allocated = true};
        SotnHandWeapon w = {.parameter = (Uint16)(cases[k].hand << 15)};
        SotnHandWeaponProgram program = {0};
        SotnHandWeaponEvents e = {0};
        Uint32 rng = 33, expected = 33;
        for (Uint16 i = 0; i < 29; ++i)
            (void)SotnCombat_Random(&expected);
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && rng == expected && !w.removed &&
              w.allocated && w.blend == 0x30 && w.properties.attack == cases[k].attack &&
              !e.dark_minimum && !e.outline &&
              e.factory == (cases[k].attack >= 0x1e ? 0x38U | (cases[k].hand + 1U) << 12 : 0U));
        const SotnPlayerEffectQuad *q = &w.tiles[0];
        CHECK(w.frame == 13 && q->x[0] == 100 - 20 - 7 && q->y[0] == 80 - 10 - 6 &&
              q->red[0] == 255 && w.tiles[11].red[2] == 0x18);
    }
    // Outside descriptor2's window while timer11 runs, the attack retires.
    SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                              .properties = PowerProperties,
                              .context = (void *)&cases[0].attack,
                              .family = 56,
                              .animation = 0x41,
                              .attack_mode = 0x8002,
                              .dark_ticks = 1,
                              .allocated = true};
    SotnHandWeapon w = {0};
    SotnHandWeaponProgram program = {0};
    SotnHandWeaponEvents e = {0};
    Uint32 rng = 1;
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && w.removed && rng == 1);
    return true;
}
// w_055 descriptor1 birth: SFX_MAGIC_SWITCH and WFACTORY(0x38,0) before properties; a
// same-update descriptor sound follows the birth sound.
static bool HolySword(void)
{
    static Uint8 o[0x1400];
    Resources(o);
    SDL_memcpy(o + 0x10ac, o + 0x106c, 16);
    SDL_memcpy(o + 0x10bc, o + 0x106c, 16);
    for (Uint16 k = 0; k < 8; ++k)
    {
        const Uint16 variant = k & 1U, hand = (k >> 1) & 1U, sound_pose = k >> 2;
        SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                                  .properties = Properties,
                                  .family = 55,
                                  .hand = hand,
                                  .priority = 148,
                                  .x = 100 * 65536,
                                  .y = 80 * 65536,
                                  .animation = 0x41,
                                  .pose = (Uint16)(sound_pose ? 4 : 0),
                                  .remaining = 1,
                                  .attack_mode = 0x8002};
        SotnHandWeapon w = {.parameter = (Uint16)(variant << 8 | hand << 15)};
        SotnHandWeaponProgram program = {0};
        SotnHandWeaponEvents e = {0};
        Uint32 rng = 9, expected = 9;
        (void)SotnCombat_Random(&expected);
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && rng == expected && e.properties &&
              w.phase == 1 && w.blend == 0x30 && w.priority == 146 &&
              w.bank == 0x8010 + hand * 2U && w.palette == 0x110 + hand * 24U &&
              w.texture == 100 + hand * 2U);
        CHECK(e.factory == (variant ? 0x38U | (hand + 1U) << 12 : 0U));
        const Uint16 first = variant ? 0x69d : sound_pose ? 0x611 : 0;
        CHECK(e.sound == first && e.extra_sounds[0] == (variant && sound_pose ? 0x611 : 0) &&
              !e.extra_sounds[1]);
    }
    return true;
}
// w_038/w_039 descriptor1 requests WFACTORY(0x38,0) on pose4's final update only; no blend.
static bool Lunges(void)
{
    static Uint8 o[0x1400];
    Resources(o);
    SDL_memcpy(o + 0xdc0, o + 0x106c, 16);
    SDL_memcpy(o + 0xdd0, o + 0x106c, 16);
    for (Uint16 k = 0; k < 16; ++k)
    {
        const Uint16 family = k & 1U ? 39 : 38, variant = (k >> 1) & 1U, hand = (k >> 2) & 1U,
                     timer = (k >> 3) & 1U ? 2 : 1;
        SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                                  .properties = Properties,
                                  .family = family,
                                  .hand = hand,
                                  .priority = 148,
                                  .x = 100 * 65536,
                                  .y = 80 * 65536,
                                  .animation = 0x41,
                                  .pose = 3,
                                  .remaining = 1,
                                  .attack_mode = 0x801b};
        SotnHandWeapon w = {.parameter = (Uint16)(variant << 8 | hand << 15)};
        SotnHandWeaponProgram program = {0};
        SotnHandWeaponEvents e = {0};
        Uint32 rng = 9;
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && !e.factory && !w.blend &&
              w.priority == 146 && w.phase == 1);
        in.pose = 4;
        in.remaining = (Sint16)timer;
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) &&
              e.factory == (variant && timer == 1 ? 0x38U | (hand + 1U) << 12 : 0U));
    }
    return true;
}
// w_007 (Shield rod): descriptors at 0xC28 add like w_003; descriptor1 (special173) requests the
// 0x4A orb with the weapon's animation index on pose4 while the player's poseTimer is7.
static bool ShieldRod(void)
{
    static Uint8 o[0x1400];
    Resources(o);
    SDL_memcpy(o + 0xc28, o + 0x106c, 16);
    SDL_memcpy(o + 0xc38, o + 0x106c, 16);
    CHECK(o[0xc28 + 12] == 0x41);
    for (Uint16 k = 0; k < 12; ++k)
    {
        const Uint16 variant = k & 1U, hand = (k >> 1) & 1U,
                     timer = (Uint16)(k >> 2) == 0 ? 7
                             : (k >> 2) == 1       ? 6
                                                   : 8;
        SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                                  .properties = Properties,
                                  .family = 7,
                                  .hand = hand,
                                  .priority = 148,
                                  .x = 100 * 65536,
                                  .y = 80 * 65536,
                                  .animation = 0x43,
                                  .pose = 3,
                                  .remaining = 7,
                                  .attack_mode = 0x801b};
        SotnHandWeapon w = {.parameter = (Uint16)(variant << 8 | hand << 15)};
        SotnHandWeaponProgram program = {0};
        SotnHandWeaponEvents e = {0};
        Uint32 rng = 9;
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && !e.factory && w.blend == 0x30 &&
              w.priority == 146 && w.phase == 1 && w.bank == 0x8010U + hand * 2U &&
              w.palette == 0x110U + hand * 24U + Read16Test(o + 0xc28 + 8) && w.flags == 0x60000);
        in.pose = 4;
        in.remaining = (Sint16)timer;
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) &&
              e.factory == (variant && timer == 7 ? 0x4aU | (hand + 1U) << 12 | 2U << 16 : 0U));
    }
    return true;
}
// w_053 descriptors1/3 blend a quarter; descriptors1/3/4 request slash roots0/10/0x14 at birth.
static bool Katana(void)
{
    static Uint8 o[0x1400];
    Resources(o);
    for (Uint16 v = 0; v < 5; ++v)
        SDL_memcpy(o + 0xf4c + v * 16U, o + 0x106c, 16);
    for (Uint16 k = 0; k < 10; ++k)
    {
        const Uint16 variant = k % 5U, hand = k / 5U;
        SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                                  .properties = Properties,
                                  .family = 53,
                                  .hand = hand,
                                  .priority = 148,
                                  .x = 100 * 65536,
                                  .y = 80 * 65536,
                                  .animation = 0x41,
                                  .pose = 0,
                                  .remaining = 2,
                                  .attack_mode = 0x8002};
        SotnHandWeapon w = {.parameter = (Uint16)(variant << 8 | hand << 15)};
        SotnHandWeaponProgram program = {0};
        SotnHandWeaponEvents e = {0};
        Uint32 rng = 9;
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && e.properties &&
              w.blend == (variant == 1 || variant == 3 ? 0x70 : 0x30));
        const Uint32 root = variant == 1 ? 0 : variant == 3 ? 10 : 0x14;
        CHECK(e.factory == (variant == 1 || variant == 3 || variant == 4
                                ? 0x38U | (hand + 1U) << 12 | root << 16
                                : 0U));
    }
    return true;
}
// w_048: descriptor1 birth requests the bolt root, three branches and the ring; the first update
// pose1 follows pose0 adds a root and three branches (0x10 ordinary, 0x20 special) once.
static bool Thunderbrand(void)
{
    static Uint8 o[0x1400];
    Resources(o);
    SDL_memcpy(o + 0xfc0, o + 0x106c, 16);
    SDL_memcpy(o + 0xfd0, o + 0x106c, 16);
    for (Uint16 k = 0; k < 4; ++k)
    {
        const Uint16 variant = k & 1U, hand = k >> 1;
        const Uint32 h = (hand + 1U) << 12, base = variant ? 0x20U : 0x10U;
        SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                                  .properties = Properties,
                                  .family = 48,
                                  .hand = hand,
                                  .priority = 148,
                                  .x = 100 * 65536,
                                  .y = 80 * 65536,
                                  .animation = 0x41,
                                  .pose = 0,
                                  .remaining = 2,
                                  .attack_mode = 0x8002};
        SotnHandWeapon w = {.parameter = (Uint16)(variant << 8 | hand << 15)};
        SotnHandWeaponProgram program = {.previous_pose = 9};
        SotnHandWeaponEvents e = {0};
        Uint32 rng = 9;
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && w.blend == 0x30 &&
              !program.previous_pose);
        if (variant)
            CHECK(e.factory == (0x38U | h) && e.more_factories[0] == (0x3aU | h | 1U << 16) &&
                  e.more_factories[2] == (0x3aU | h | 3U << 16) &&
                  e.more_factories[3] == (0x40U | h) && !e.more_factories[4]);
        else
            CHECK(!e.factory && !e.more_factories[0]);
        in.pose = 1;
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) &&
              e.factory == (0x38U | h | base << 16) &&
              e.more_factories[0] == (0x3aU | h | (base + 1U) << 16) &&
              e.more_factories[2] == (0x3aU | h | (base + 3U) << 16) && !e.more_factories[3] &&
              program.previous_pose == 1);
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && !e.factory);
    }
    return true;
}
// Authored weapon1 overlay: descriptors0..3, trail offsets and colors; no original tables.
// Authored weapon51 layout: attack descriptors0..3 at 0xcd8 (frameStart0x41, palettes 0/2/4/6),
// special descriptors at 0xd18 (frameStart0xD6), Estoc's past-table words at 0xd58 and the
// glow colour_table at 0xd74 (row0 rises to 16/15/0, row1 to 0/16/16).
static void Weapon51Resources(Uint8 *o)
{
    Resources(o);
    for (Uint16 v = 0; v < 4; ++v)
    {
        SDL_memcpy(o + 0xcd8 + v * 16U, o + 0x106c, 16);
        Put16(o + 0xcd8 + v * 16U + 8, (Uint16)(v * 2U));
        SDL_memcpy(o + 0xd18 + v * 16U, o + 0x106c, 16);
        o[0xd18 + v * 16U + 12] = 0xd6;
    }
    SDL_memcpy(o + 0xd58, o + 0x106c, 16);
    o[0xd58 + 12] = 0xd6;
    const Sint32 table[16] = {0, 0, 0, 0, 16, 15, 0, 1, 0, 0, 0, 0, 0, 16, 16, 1};
    for (Uint16 i = 0; i < 16; ++i)
        Put32(o + 0xd74 + i * 4U, (Uint32)table[i]);
}
// w_051: descriptors1/2 add, 3 is transparent; func_8017B1DC pulses colour11 of the descriptor's
// palette up then down (two per update), writing and uploading g_Clut until both phases end.
static bool Weapon51(void)
{
    static Uint8 o[0x1400];
    Weapon51Resources(o);
    for (Uint16 k = 0; k < 4; ++k)
    {
        const Uint16 hand = k >> 1, variant = (Uint16)(1U + (k & 1U));
        SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                                  .properties = Properties,
                                  .family = 51,
                                  .hand = hand,
                                  .priority = 148,
                                  .animation = 0x41,
                                  .pose = 0,
                                  .remaining = 2,
                                  .attack_mode = 0x8002};
        SotnHandWeapon w = {.parameter = (Uint16)(variant << 8 | hand << 15)};
        SotnHandWeaponProgram program = {.glow_red = 9, .glow_step = 2};
        SotnHandWeaponEvents e = {0};
        Uint32 rng = 9;
        const Uint16 palette = (Uint16)(0x110U + hand * 24U + variant * 2U);
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && w.blend == 0x30 &&
              w.palette == palette && e.clut && e.clut_palette == palette && e.clut_color == 11 &&
              program.glow_step == 0 && program.glow_green == 2 &&
              e.clut_value == (variant == 1 ? 0x8042U : 0x8840U));
        Uint16 writes = 1, t = 0;
        for (; t < 40 && program.glow_step != 2; ++t)
        {
            e = (SotnHandWeaponEvents){0};
            CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && e.clut);
            ++writes;
        }
        // Row0 rises 2..16 (8 writes) then falls to0 (8 writes); row1 rises to16 (8 + 8).
        CHECK(program.glow_step == 2 && !program.glow_red && !program.glow_green &&
              !program.glow_blue && !program.glow_alpha && writes == 16);
        e = (SotnHandWeaponEvents){0};
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && !e.clut);
    }
    // Descriptor3 is BLEND_TRANSP; descriptor0 (Great Sword/Claymore) never glows.
    for (Uint16 variant = 0; variant < 4; variant += 3)
    {
        SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                                  .properties = Properties,
                                  .family = 51,
                                  .animation = 0x41,
                                  .remaining = 2,
                                  .attack_mode = 0x8002};
        SotnHandWeapon w = {.parameter = (Uint16)(variant << 8)};
        SotnHandWeaponProgram program = {0};
        SotnHandWeaponEvents e = {0};
        Uint32 rng = 1;
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && w.blend == (variant ? 0x10 : 0) &&
              e.clut == (variant != 0));
    }
    // Estoc's variant8 uses the words at D_8017AD58 as its descriptor (frameStart0xD6) and
    // glows colour11 of palette0x120 with row7 (all zero): two writes of zero, then nothing.
    {
        SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                                  .properties = Properties,
                                  .family = 51,
                                  .hand = 1,
                                  .animation = 0xd6,
                                  .remaining = 2,
                                  .attack_mode = 0x8002};
        SotnHandWeapon w = {.parameter = 0x8800};
        SotnHandWeaponProgram program = {0};
        SotnHandWeaponEvents e = {0};
        Uint32 rng = 1;
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && !w.removed && !w.blend &&
              w.palette == 0x110 + 24 && e.clut && e.clut_palette == 0x120 + 24 &&
              e.clut_color == 11 && !e.clut_value && program.glow_step == 1);
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && e.clut && !e.clut_value &&
              program.glow_step == 2);
        e = (SotnHandWeaponEvents){0};
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && !e.clut);
        in.animation = 0xdd;
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && w.removed);
    }
    // The special (func_ptr_80170004): palette0x110+subType*2, no descriptor sound or attack-
    // mode check, WFACTORY(0x3E,0) on pose1's last update, glow held on pose4 once started and
    // retirement outside animations0xD6..0xD7.
    for (Uint16 hand = 0; hand < 2; ++hand)
    {
        SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                                  .properties = Properties,
                                  .family = 51,
                                  .hand = hand,
                                  .animation = 0xd7,
                                  .pose = 4,
                                  .remaining = 1,
                                  .special = true};
        SotnHandWeapon w = {.parameter = (Uint16)(0x200 | hand << 15)};
        SotnHandWeaponProgram program = {0};
        SotnHandWeaponEvents e = {0};
        Uint32 rng = 1;
        // Pose4 at birth still glows (colour_step is zero) and plays no descriptor sound.
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && !w.removed &&
              w.palette == 0x110U + hand * 24U + 4U && w.blend == 0x30 && e.clut && !e.sound &&
              !e.factory && program.glow_green == 2);
        e = (SotnHandWeaponEvents){0};
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && e.clut && program.glow_green == 4);
        for (Uint16 t = 0; t < 8; ++t)
            CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e));
        CHECK(program.glow_step == 1);
        e = (SotnHandWeaponEvents){0};
        const Sint16 green = program.glow_green;
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && !e.clut &&
              program.glow_green == green);
        in.pose = 1;
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && e.clut &&
              e.factory == (0x3eU | (hand + 1U) << 12));
        in.remaining = 2;
        e = (SotnHandWeaponEvents){0};
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && !e.factory);
        in.animation = 0xd8;
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && w.removed);
    }
    return true;
}
// w_049: descriptor1 requests the sparks and descriptor3 the swirls plus the param1 ring at birth;
// the first pose1 after pose0 requests WFACTORY(0x5D) puffs (param1 for descriptor2) with
// SFX_FM_EXPLODE_B. Descriptor0 (Firebrand's swing) requests nothing.
static bool Weapon49(void)
{
    static Uint8 o[0x1400];
    Resources(o);
    for (Uint16 v = 0; v < 4; ++v)
        SDL_memcpy(o + 0xffc + v * 16U, o + 0x106c, 16);
    for (Uint16 k = 0; k < 8; ++k)
    {
        const Uint16 variant = k & 3U, hand = k >> 2;
        const Uint32 h = (hand + 1U) << 12;
        SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                                  .properties = Properties,
                                  .family = 49,
                                  .hand = hand,
                                  .priority = 148,
                                  .animation = 0x41,
                                  .pose = 0,
                                  .remaining = 2,
                                  .attack_mode = 0x8002};
        SotnHandWeapon w = {.parameter = (Uint16)(variant << 8 | hand << 15)};
        SotnHandWeaponProgram program = {.previous_pose = 9};
        SotnHandWeaponEvents e = {0};
        Uint32 rng = 9;
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && w.blend == 0x30 &&
              w.bank == 0x8010U + hand * 2U && !program.previous_pose && e.properties);
        CHECK(e.factory == (variant == 1   ? (0x5bU | h)
                            : variant == 3 ? (0x5fU | h)
                                           : 0U) &&
              e.more_factories[0] == (variant == 3 ? (0x40U | h | 1U << 16) : 0U));
        in.pose = 1;
        e = (SotnHandWeaponEvents){0};
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && program.previous_pose == 1);
        if (variant)
            CHECK(e.factory == (0x5dU | h | (variant == 2 ? 1U << 16 : 0U)) &&
                  (e.sound == 0x65b || e.extra_sounds[0] == 0x65b));
        else
            CHECK(!e.factory && e.sound != 0x65b && e.extra_sounds[0] != 0x65b);
        e = (SotnHandWeaponEvents){0};
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && !e.factory);
    }
    return true;
}
// w_050: descriptor1 plays SFX_MAGIC_WEAPON_APPEAR_A and requests the sparkles and ring at birth;
// the first pose1 after pose0 requests (descriptor1) the frost streaks with SFX_FM_EXPLODE_B and
// SFX_MAGIC_SWITCH, then FACTORY(0x61, descriptor) crystals for every descriptor.
static bool Weapon50(void)
{
    static Uint8 o[0x1400];
    Resources(o);
    for (Uint16 v = 0; v < 4; ++v)
        SDL_memcpy(o + 0xfc0 + v * 16U, o + 0x106c, 16);
    for (Uint16 k = 0; k < 8; ++k)
    {
        const Uint16 variant = k & 3U, hand = k >> 2;
        const Uint32 h = (hand + 1U) << 12;
        SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                                  .properties = Properties,
                                  .family = 50,
                                  .hand = hand,
                                  .priority = 148,
                                  .animation = 0x41,
                                  .pose = 0,
                                  .remaining = 2,
                                  .attack_mode = 0x8002};
        SotnHandWeapon w = {.parameter = (Uint16)(variant << 8 | hand << 15)};
        SotnHandWeaponProgram program = {.previous_pose = 9};
        SotnHandWeaponEvents e = {0};
        Uint32 rng = 9;
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && w.blend == 0x30 &&
              !program.previous_pose && e.properties);
        if (variant == 1)
            CHECK(e.sound == 0x669 && e.factory == (0x3aU | h) &&
                  e.more_factories[0] == (0x40U | h) && !e.more_factories[1]);
        else
            CHECK(!e.factory && e.sound != 0x669);
        in.pose = 1;
        e = (SotnHandWeaponEvents){0};
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && program.previous_pose == 1);
        const Uint32 crystals = 0x61U | h | (Uint32)variant << 16;
        if (variant == 1)
            CHECK(e.factory == (0x44U | h) && e.more_factories[0] == crystals && e.sound == 0x65b &&
                  e.extra_sounds[0] == 0x69d);
        else
            CHECK(e.factory == crystals && !e.more_factories[0] && e.sound != 0x65b);
        e = (SotnHandWeaponEvents){0};
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && !e.factory);
    }
    return true;
}
static void FistResources(Uint8 *o)
{
    Resources(o);
    for (Uint16 v = 0; v < 5; ++v)
    {
        Uint8 *d = o + 0xf44 + v * 16U;
        Put32(d, 0x8017a100);
        Put32(d + 4, 0x8017a200);
        Put16(d + 8, (Uint16)(0x19e + v));
        Put16(d + 10, 0x60c);
        d[12] = 0x24;
        d[13] = 1;
    }
    for (Uint16 a = 0; a < 7; ++a)
    {
        Put16(o + 0xf94 + a * 4U, (Uint16)(18 + a));
        Put16(o + 0xf96 + a * 4U, (Uint16)(Sint16)(-13 + a));
    }
    const Uint8 colors[] = {0, 32, 32, 80, 80, 32, 32, 0};
    SDL_memcpy(o + 0xfb0, colors, sizeof(colors));
}
// Linear authored SDK tables at the original executable offsets.
static void FistMath(Uint8 *exe)
{
    for (Uint32 i = 0; i <= 1024; ++i)
    {
        Put16(exe + 0x22a10 + i * 2, (Uint16)(i / 2));
        Put16(exe + 0x1dbcc + i * 2, (Uint16)(i * 4));
    }
    for (Uint32 i = 0; i < 192; ++i)
        Put16(exe + 0x1e3e0 + i * 2, 4096);
}
static bool BasicFists(void)
{
    static Uint8 o[0x1200], exe[0x24000];
    FistResources(o);
    FistMath(exe);
    SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                              .executable = {exe, sizeof(exe)},
                              .camera_x = 30,
                              .camera_y = 10,
                              .properties = Properties,
                              .x = 100 * 65536 + 77,
                              .y = 50 * 65536 + 5,
                              .family = 1,
                              .priority = 148,
                              .animation = 0x24,
                              .pose = 2,
                              .remaining = 2,
                              .attack_mode = 0x8002,
                              .draw_flags = 2,
                              .allocated = false};
    SotnHandWeaponProgram program = {.short_pose = 9, .shield_attack = -3};
    SotnHandWeaponEvents e = {.sound = 77};
    // Exhausted mandatory rows destroy the entity before flags, properties or RNG.
    SotnHandWeapon w = {.parameter = 0};
    Uint32 rng = 5;
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && w.removed && !w.flags &&
          !w.allocated && !e.properties && !e.sound && rng == 5 && !w.phase &&
          program.short_pose == 9 && program.shield_attack == -3);
    // Birth binds four GT4 rows, no sprite bank, then the property roll and per-row rand.
    in.allocated = true;
    SDL_zero(w);
    Uint32 expected = rng;
    (void)SotnCombat_Random(&expected);
    Uint32 spreads[4] = {0};
    for (Uint16 i = 0; i < 4; ++i)
        spreads[i] = (SotnCombat_Random(&expected) & 15U) + 16U;
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && !w.removed && w.allocated &&
          w.flags == 0x860000U && e.properties && w.phase == 1 && !w.bank && !w.palette &&
          !w.texture && !w.blend && !w.priority && rng == expected && w.frame == 23 &&
          w.hitbox_x == -4 && w.hitbox_y == -6);
    const Sint16 x = (Sint16)(100 - 30 + 18), y = (Sint16)(50 - 10 - 13 - 3);
    for (Uint16 i = 0; i < 4; ++i)
    {
        const SotnPlayerEffectQuad *q = &w.tiles[i];
        CHECK(q->page == 0x1a && q->palette == 0x19e && q->priority == 152 &&
              q->draw_mode == 0x335 && q->u[0] == 0x80 && q->u[1] == 0xa0 && q->u[2] == 0x80 &&
              q->v[0] == 0x80 + i * 4 && q->v[2] == 0x84 + i * 4 && q->red[1] == 0 &&
              q->red[0] == (Uint8)(i ? (i == 1   ? 32
                                        : i == 2 ? 80
                                                 : 32)
                                     : 0) &&
              q->x[0] == x + 16 && q->x[2] == x + 16 && q->x[1] == x - (Sint16)spreads[i] &&
              q->x[3] == q->x[1] && q->y[0] == -8 + y + i * 4 && q->y[2] == q->y[0] + 4);
    }
    // Variant0 hides animation0 pose4; variant1 keeps it, using its own descriptor palette.
    in.pose = 4;
    const SotnHandWeapon visible = w;
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && rng == expected &&
          (w.tiles[0].draw_mode & 8U) && w.tiles[0].x[0] == visible.tiles[0].x[0]);
    SotnHandWeapon second = {.parameter = 0x100};
    Uint32 second_rng = 11;
    CHECK(SotnHandWeapon_Tick(&second, &program, in, &second_rng, &e) &&
          second.tiles[0].palette == 0x19f && !(second.tiles[0].draw_mode & 8U));
    // Facing left mirrors the origin and spread; scale flag absent leaves Y unchanged.
    in.pose = 2;
    in.facing_left = true;
    in.draw_flags = 0;
    expected = rng;
    const Uint32 left_spread = (SotnCombat_Random(&expected) & 15U) + 16U;
    for (Uint16 i = 1; i < 4; ++i)
        (void)SotnCombat_Random(&expected);
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && rng == expected &&
          w.tiles[0].x[0] == 100 - 30 - 18 - 16 &&
          w.tiles[0].x[1] == 100 - 30 - 18 + (Sint16)left_spread &&
          w.tiles[0].y[0] == 50 - 10 - 13 - 8);
    // Animation5 follow-through rotates every vertex with SDK sqrt/atan2/trig; table failure
    // leaves the entity, rows and RNG untouched.
    in.animation = 0x29;
    in.pose = 1;
    const SotnHandWeapon before = w;
    const Uint32 before_rng = rng;
    in.executable.size = 0x1000;
    CHECK(!SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && rng == before_rng &&
          !SDL_memcmp(&w, &before, sizeof(w)));
    in.executable.size = sizeof(exe);
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && w.animation == 5 &&
          !(w.tiles[0].draw_mode & 8U));
    SotnHandWeapon flat = before;
    Uint32 flat_rng = before_rng;
    in.animation = 0x28;
    CHECK(SotnHandWeapon_Tick(&flat, &program, in, &flat_rng, &e) && flat_rng == rng &&
          (flat.tiles[0].x[0] != w.tiles[0].x[0] || flat.tiles[0].y[0] != w.tiles[0].y[0]));
    // Descriptor4 is Fist of Tulkas's QCF special188: SFX_VO_ALU_ATTACK_A after its rows, no
    // trails, and WFACTORY(0x3E,0) on pose2's last update. Descriptor5 is outside the table.
    SotnHandWeapon special = {.parameter = 0x400};
    Uint32 special_rng = 3;
    e = (SotnHandWeaponEvents){0};
    CHECK(SotnHandWeapon_Tick(&special, &program, in, &special_rng, &e) && special.phase &&
          e.sound == 0x6ee && !e.factory && (special.tiles[0].draw_mode & 8U));
    in.pose = 2;
    in.remaining = 1;
    e = (SotnHandWeaponEvents){0};
    CHECK(SotnHandWeapon_Tick(&special, &program, in, &special_rng, &e) && e.factory == 0x103e &&
          !e.sound && (special.tiles[3].draw_mode & 8U));
    in.remaining = 2;
    e = (SotnHandWeaponEvents){0};
    CHECK(SotnHandWeapon_Tick(&special, &program, in, &special_rng, &e) && !e.factory);
    SotnHandWeapon outside = {.parameter = 0x500};
    special_rng = 3;
    CHECK(!SotnHandWeapon_Tick(&outside, &program, in, &special_rng, &e) && special_rng == 3 &&
          !outside.phase);
    SotnHandWeapon truncated = {0};
    in.overlay.size = 0xfb4;
    in.animation = 0x24;
    CHECK(!SotnHandWeapon_Tick(&truncated, &program, in, &special_rng, &e) && special_rng == 3 &&
          !truncated.phase && !truncated.allocated);
    return true;
}
static void Definition(Uint8 *dra, Uint16 item, Uint16 family, Uint16 attack)
{
    Uint8 *p = dra + 0x500 + item * 52U;
    Put16(p + 8, attack);
    Put16(p + 12, 64);
    p[14] = 1;
    p[15] = (Uint8)family;
    p[17] = 1;
    p[18] = 0x41;
    p[19] = 48;
    p[21] = 13;
    p[22] = 128;
    p[26] = 4;
    Put16(p + 38, 4);
    Put16(p + 40, 2);
    Put16(p + 42, 129);
}
static bool Ownership(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(53, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r)), *before = SDL_malloc(sizeof(*before));
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r && before);
    TestCharacter_Fill(dra);
    Definition(dra, 123, 43, 42);
    Definition(dra, 86, 0, 3);
    Definition(dra, 16, 52, 0);
    Uint8 *shield = dra + 0x500 + 16U * 52U;
    shield[14] = 9;
    shield[17] = 128;
    shield[18] = 0x63;
    shield[21] = 0;
    Put16(shield + 40, 4);
    Uint8 *p = dra + 0xd1d4 + 48U * 6U;
    p[1] = 1;
    p[2] = 0xc1;
    p[4] = 0x23;
    dra[0xd4b8 + 6] = dra[0xd4b9 + 6] = 16;
    Resources(weapons + 43U * 0x7000 + 0x4000);
    Resources(weapons + 0x4000);
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    actors->primitives[7].owner = 777;
    CHECK(SotnCharacter_Create(actors->dra, &character));
    CHECK(SotnCharacter_Collect(character, false, 16) &&
          SotnCharacter_Equip(character, SOTN_EQUIP_RIGHT, 16));
    SotnCharacterStats shield_stats = {0};
    CHECK(SotnCharacter_Get(character, NULL, &shield_stats) && !shield_stats.attack[1]);
    Uint32 shield_random = 7, shield_expected = 7;
    (void)SotnCombat_Random(&shield_expected);
    SotnEquipment shield_properties = {0};
    CHECK(SotnCharacter_HandProperties(character, 16, false, &shield_random, &shield_properties) &&
          !shield_properties.attack && shield_properties.hit_type == 4 &&
          shield_random == shield_expected);
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .weapons = {weapons, 53U * 0x7000},
                              .weapon_family = {43, 43},
                              .weapon_bound = {true, true},
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 128 * 65536 + 123,
                                         .y = 100 * 65536 + 456,
                                         .priority = 148,
                                         .attack_mode = 0x8002,
                                         .animation = {.id = 0x41, .pose = 3, .remaining = 1}}};
    const SotnPlayerFactorySource source = {.x = in.player.x, .y = in.player.y, .priority = 148};
    SotnCharacterState progress = {0};
    CHECK(SotnCharacter_Get(character, &progress, NULL));
    Uint32 expected = progress.random;
    for (Uint16 i = 0; i < 49; ++i)
        (void)SotnCombat_Random(&expected);
    bool allocated = false;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestHand(r, source, 0x1030, 123, &allocated) && allocated &&
          SotnBatRuntime_Tick(r, character, in) && r->children[0].hand_weapon &&
          r->children[0].group == 1 && r->hand_attack && r->children[0].primitive_count == 16 &&
          r->children[0].state.hand.motion.tile_started[15] && r->groups[1] == 1 &&
          SotnBatRuntime_HandValid(&r->children[0], 0) &&
          SotnCharacter_Get(character, &progress, NULL) && progress.random == expected &&
          SotnActors_CommitBatPrimitives(actors, r) && actors->primitives[7].owner == 777);
    *before = *r;
    in.freeze = 3;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) &&
          !SDL_memcmp(before->children, r->children, sizeof(r->children)) && r->hand_attack &&
          SotnCharacter_Get(character, &progress, NULL) && progress.random == expected);
    in.freeze = 0;
    in.entering = true;
    in.room_shift_x = 7;
    in.screen_shift_x = -5;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) &&
          r->children[0].state.hand.motion.x ==
              before->children[0].state.hand.motion.x + 7 * 65536 &&
          r->children[0].state.hand.motion.tiles[0].x[0] ==
              before->children[0].state.hand.motion.tiles[0].x[0] - 5 &&
          r->children[0].state.hand.motion.tile_age[0] ==
              before->children[0].state.hand.motion.tile_age[0] &&
          SotnCharacter_Get(character, &progress, NULL) && progress.random == expected &&
          SotnActors_CommitBatPrimitives(actors, r));
    in.entering = false;
    in.room_shift_x = in.screen_shift_x = 0;
    const SotnHandWeapon moved = r->children[0].state.hand.motion;
    CHECK(SotnBatRuntime_PlayerShove(r, 65536 + 23, -32768) &&
          r->children[0].source.x == moved.x + 65536 + 23 &&
          r->children[0].state.hand.motion.y == moved.y - 32768 &&
          r->children[0].state.hand.motion.tiles[0].x[0] == moved.tiles[0].x[0] &&
          r->children[0].state.hand.motion.tile_age[0] == moved.tile_age[0] &&
          SotnCharacter_Get(character, &progress, NULL) && progress.random == expected);
    // The actual forced slot16 replacement retires the old sixteen-row owner in the
    // same stage-pool transaction; normal hand group two does not underflow to spell groups.
    in.freeze = 0;
    in.weapon_family[1] = 0;
    in.player.animation.pose = 0;
    in.player.animation.remaining = 2;
    (void)SotnCombat_Random(&expected);
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestHand(r, source, 0x802030, 86, &allocated) && allocated &&
          SotnBatRuntime_Tick(r, character, in) && r->children[0].hand_weapon &&
          r->children[0].kind == 0xf0 && r->children[0].group == 2 && r->groups[2] == 1 &&
          !r->children[0].primitive_count && SotnActors_CommitBatPrimitives(actors, r) &&
          actors->primitives[7].owner == 777 && SotnCharacter_Get(character, &progress, NULL) &&
          progress.random == expected);
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        CHECK(actors->primitives[i].owner == (i == 7 ? 777U : 0U));
    *before = *r;
    in.weapons.size = 1;
    CHECK(!SotnBatRuntime_Tick(r, character, in) && !SDL_memcmp(r, before, sizeof(*r)) &&
          SotnCharacter_Get(character, &progress, NULL) && progress.random == expected);
    in.weapons.size = 53U * 0x7000;
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        actors->primitives[i].owner = 777;
    in.player.animation.pose = 3;
    in.player.animation.remaining = 1;
    (void)SotnCombat_Random(&expected);
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestHand(r, source, 0x1030, 123, &allocated) && allocated &&
          SotnBatRuntime_Tick(r, character, in) && r->hand_attack &&
          !r->children[0].primitive_count && !r->children[0].state.hand.motion.allocated &&
          SotnCharacter_Get(character, &progress, NULL) && progress.random == expected &&
          SotnActors_CommitBatPrimitives(actors, r));
    // Optional exhaustion omits only the visuals and their RNG; it never retries later.
    for (Uint16 i = 32; i < 48; ++i)
        actors->primitives[i].owner = 0;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && r->hand_attack &&
          !r->children[0].primitive_count && SotnCharacter_Get(character, &progress, NULL) &&
          progress.random == expected && SotnActors_CommitBatPrimitives(actors, r));
    r->hand_program[0].short_pose = 123;
    CHECK(SotnBatRuntime_PruneWeapons(r, 1) && !r->children[0].kind && !r->hand_attack &&
          r->hand_program[0].short_pose == 123 && SotnActors_CommitBatPrimitives(actors, r));
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(SotnBatRuntime_RequestHand(r, source, 0x1030, 123, &allocated) && allocated);
    *before = *r;
    CHECK(SotnBatRuntime_RequestHand(r, source, 0x1030, 123, &allocated) && !allocated &&
          !SDL_memcmp(r, before, sizeof(*r)));
    // New weapon0 variants use their own descriptor and real packed factory parameters.
    Definition(dra, 116, 0, 18);
    Uint8 *term = dra + 0x500 + 116U * 52U, *ordinary = weapons + 0x4000;
    term[16] = 2;
    term[20] = 2;
    SDL_memcpy(ordinary + 0x108c, ordinary + 0x106c, 16);
    Put32(ordinary + 0x1090, 0x8017a220);
    ordinary[0x224] = 13;
    ordinary[0x225] = 251;
    ordinary[0x226] = 11;
    ordinary[0x227] = 7;
    in.weapon_family[0] = in.weapon_family[1] = 0;
    for (Uint16 hand = 0; hand < 2; ++hand)
    {
        SDL_zero(*r);
        CHECK(SotnCharacter_Collect(character, false, 116) &&
              SotnCharacter_Equip(character, hand, 116));
        *before = *r;
        CHECK(!SotnBatRuntime_RequestHand(r, source, 0x1030, 116, &allocated) &&
              !SDL_memcmp(r, before, sizeof(*r)));
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_RequestHand(
                  r, source, 0x20030U | ((Uint32)(hand + 1U) << 12) | ((Uint32)hand << 23), 116,
                  &allocated) &&
              allocated && SotnBatRuntime_Tick(r, character, in) && r->hand_attack &&
              r->children[0].parameter == (0x200U | (hand ? 0x8000U : 0U)) &&
              SotnBatRuntime_HandValid(&r->children[0], 0) &&
              r->children[0].state.hand.motion.hitbox_x == 13 &&
              r->children[0].state.hand.motion.hitbox_y == -5 &&
              r->children[0].group == hand + 1U && !r->children[0].primitive_count &&
              SotnActors_CommitBatPrimitives(actors, r));
    }
    Definition(dra, 83, 0, (Uint16)-30);
    dra[0x500 + 83U * 52U + 21] = 18;
    CHECK(SotnCharacter_Collect(character, false, 83) && SotnCharacter_Equip(character, 0, 83));
    SotnCharacterStats negative = {0};
    SotnEquipment properties = {0};
    CHECK(SotnCharacter_Get(character, NULL, &negative) && !negative.attack[0] &&
          SotnCharacter_HandProperties(character, 83, false, &shield_random, &properties) &&
          !properties.attack);
    CHECK(!SotnCharacter_Equip(character, 0, 84) &&
          !SotnCharacter_HandProperties(character, 125, false, &shield_random, &properties));
    // Knight6/AxeLord8/Shaman12 bind their own resident families through the same slot16
    // owner. GetEquipProperties zeroes base-ATK1 shields even with Attack buffs or poison;
    // the callback's later hit-state4 SetWeaponProperties override alone produces attack255.
    const Uint16 shield_ids[] = {6, 8, 12}, shield_families[] = {9, 11, 26};
    const Uint16 shield_tables[] = {0xa40, 0xb94, 0xa60};
    CHECK(SotnCharacter_SetStatBuffMask(character, 1U << SOTN_BUFF_ATTACK));
    for (Uint16 k = 0; k < SDL_arraysize(shield_ids); ++k)
    {
        const Uint16 id = shield_ids[k], family = shield_families[k];
        Definition(dra, id, family, 1);
        Uint8 *record = dra + 0x500 + id * 52U;
        record[14] = 9;
        record[17] = 128;
        record[18] = 0x63;
        record[21] = 0;
        Put16(record + 40, 4);
        SotnEquipment definition = {0};
        record[23] = 1;
        CHECK(!SotnEquipment_Read(actors->dra, id, &definition));
        record[23] = 0;
        CHECK(SotnEquipment_Read(actors->dra, id, &definition) && definition.family == family &&
              definition.kind == 128 && definition.attack == 1);
        Uint8 *overlay = weapons + family * 0x7000U + 0x4000U;
        Resources(overlay);
        for (Uint16 animation = 0; animation < 14; ++animation)
            Put32(overlay + shield_tables[k] + animation * 4U, 0x8017a400);
        const Uint16 props = (Uint16)(shield_tables[k] - 0x48U + 4U);
        overlay[props] = 249;
        overlay[props + 1] = 254;
        overlay[props + 2] = 8;
        overlay[props + 3] = 16;
        CHECK(SotnCharacter_Collect(character, false, id) &&
              SotnCharacter_Equip(character, SOTN_EQUIP_LEFT, id));
        SotnCharacterStats shield_attack = {0};
        Uint32 roll = 11, rolled = 11;
        (void)SotnCombat_Random(&rolled);
        CHECK(SotnCharacter_Get(character, NULL, &shield_attack) && !shield_attack.attack[0] &&
              SotnCharacter_HandProperties(character, id, true, &roll, &properties) &&
              !properties.attack && properties.hit_type == 4 && roll == rolled);
        SDL_zero(*r);
        in.weapon_family[0] = in.weapon_family[1] = (Uint8)family;
        in.player.buttons = SOTN_BUTTON_LEFT_HAND;
        in.player.attack_mode = 0;
        CHECK(SotnCharacter_Get(character, &progress, NULL));
        expected = progress.random;
        (void)SotnCombat_Random(&expected);
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_RequestHand(r, source, 0x1030, id, &allocated) && allocated);
        // A different resident program cannot run this item's callback.
        in.weapon_family[0] = 52;
        *before = *r;
        CHECK(!SotnBatRuntime_Tick(r, character, in) && !SDL_memcmp(r, before, sizeof(*r)) &&
              SotnCharacter_Get(character, &progress, NULL) && progress.random != expected);
        in.weapon_family[0] = (Uint8)family;
        const SotnBatRuntimeChild *c = &r->children[0];
        CHECK(SotnBatRuntime_Tick(r, character, in) && c->hand_weapon &&
              c->state.hand.family == family && c->group == 1 && r->groups[1] == 1 &&
              !c->primitive_count && SotnBatRuntime_HandValid(c, 0) && r->hand_shield &&
              c->state.hand.motion.hitbox_state == 4 &&
              c->state.hand.motion.properties.attack == 255 && c->state.hand.attack.hit_type == 4 &&
              c->state.hand.attack.power == 255 && c->state.hand.attack.equipment == id &&
              r->hand_program[0].shield_attack == 0 &&
              SotnCharacter_Get(character, &progress, NULL) && progress.random == expected);
        if (k)
        {
            CHECK(SotnBatRuntime_RetireHand(r, true) && !r->children[0].hand_weapon &&
                  !r->hand_shield);
            continue;
        }
        // Runesword's root runs DestroyEntityWeapon(true) at birth: the raised shield in slot16
        // retires through its actual owner, with unk48 cleared and the timer10 use request.
        static Uint8 rune_exe[0x1dbcc + 2050];
        Definition(dra, 143, 14, 40);
        Uint8 *rune = dra + 0x500 + 143U * 52U;
        rune[14] = 2;
        rune[17] = 129;
        rune[18] = 0x5d;
        rune[19] = 50;
        rune[21] = 0;
        rune[22] = 1;
        const Uint8 bp50[5] = {0, 1, 0xc1, 0, 0x26}, bp72[5] = {1, 4, 0xc4, 0, 1};
        SDL_memcpy(dra + 0xd1d4 + 50U * 6U, bp50, 5);
        SDL_memcpy(dra + 0xd1d4 + 0x72U * 6U, bp72, 5);
        dra[0xd4b8 + 2] = dra[0xd4b8 + 12] = 17;
        dra[0xd4b9 + 2] = 47;
        dra[0xd4b9 + 12] = 31;
        const SotnDataView executable = in.executable;
        in.executable = (SotnDataView){rune_exe, sizeof(rune_exe)};
        in.weapon_family[1] = 14;
        CHECK(SotnBatRuntime_RequestThrown(r, source, 0x802032U, 143, &allocated) && allocated);
        // Free the earlier exhaustion filler so the root's mandatory 48 rows are available.
        static bool filler[SOTN_PRIMITIVE_SLOTS];
        for (Uint16 q = 0; q < SOTN_PRIMITIVE_SLOTS; ++q)
            if ((filler[q] = q != 7 && actors->primitives[q].owner == 777))
                actors->primitives[q].owner = 0;
        bool retired = false;
        for (Uint16 t = 0; t < 4 && !retired; ++t)
        {
            CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
                  SotnBatRuntime_Tick(r, character, in) &&
                  SotnActors_CommitBatPrimitives(actors, r));
            retired = !r->children[0].kind;
            CHECK(retired == (r->magic_clear_shield && r->use_ticks == 4));
        }
        CHECK(retired && !r->hand_shield);
        SDL_zero(*r);
        CHECK(SotnActors_CommitBatPrimitives(actors, r));
        for (Uint16 q = 0; q < SOTN_PRIMITIVE_SLOTS; ++q)
            if (filler[q])
                actors->primitives[q].owner = 777;
        in.executable = executable;
        in.weapon_family[1] = (Uint8)family;
    }
    CHECK(SotnCharacter_SetStatBuffMask(character, 0));
    // Knuckle duster85/Jewel knuckles97 own four mandatory shared rows in slot16; exhaustion
    // omits the attack before properties, and retirement releases every actual row.
    static Uint8 fist_exe[0x24000];
    FistMath(fist_exe);
    FistResources(weapons + 0x7000U + 0x4000U);
    in.executable = (SotnDataView){fist_exe, sizeof(fist_exe)};
    for (Uint16 k = 0; k < 2; ++k)
    {
        const Uint16 id = k ? 97 : 85;
        Definition(dra, id, 1, k ? 21 : 1);
        Uint8 *record = dra + 0x500 + id * 52U;
        record[14] = 3;
        record[17] = 0;
        record[18] = 0x24;
        record[20] = (Uint8)k;
        record[21] = 5;
        SotnEquipment definition = {0};
        record[24] = 186;
        CHECK(!SotnEquipment_Read(actors->dra, id, &definition));
        record[24] = 0;
        CHECK(SotnEquipment_Read(actors->dra, id, &definition) && definition.family == 1 &&
              definition.variant == k && definition.animation == 0x24);
        CHECK(SotnCharacter_Collect(character, false, id) &&
              SotnCharacter_Equip(character, SOTN_EQUIP_LEFT, id));
        for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
            actors->primitives[i].owner = i == 7 ? 777U : 0U;
        SDL_zero(*r);
        in.weapon_family[0] = in.weapon_family[1] = 1;
        in.player.animation.id = 0x24;
        in.player.animation.pose = 2;
        in.player.animation.remaining = 2;
        in.player.attack_mode = 0x8002;
        CHECK(SotnCharacter_Get(character, &progress, NULL));
        expected = progress.random;
        for (Uint16 i = 0; i < 5; ++i)
            (void)SotnCombat_Random(&expected);
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_RequestHand(r, source, 0x1030 | ((Uint32)k << 16), id, &allocated) &&
              allocated && SotnBatRuntime_Tick(r, character, in));
        const SotnBatRuntimeChild *c = &r->children[0];
        CHECK(c->hand_weapon && c->state.hand.family == 1 && c->group == 1 &&
              c->primitive_count == SOTN_HAND_WEAPON_FIST_ROWS && SotnBatRuntime_HandValid(c, 0) &&
              r->hand_attack && c->parameter == (Uint16)(k << 8) &&
              !(c->state.hand.motion.tiles[0].draw_mode & 8U) &&
              SotnCharacter_Get(character, &progress, NULL) && progress.random == expected &&
              SotnActors_CommitBatPrimitives(actors, r) && actors->primitives[7].owner == 777);
        // Room rebases translate all four trail vertices without a callback or RNG.
        const SotnHandWeapon placed = c->state.hand.motion;
        in.entering = true;
        in.room_shift_x = 7;
        in.screen_shift_x = -5;
        in.screen_shift_y = 3;
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) &&
              SotnCharacter_Get(character, &progress, NULL) && progress.random == expected &&
              c->state.hand.motion.x == placed.x + 7 * 65536 &&
              SotnActors_CommitBatPrimitives(actors, r));
        for (Uint16 i = 0; i < SOTN_HAND_WEAPON_FIST_ROWS; ++i)
            for (Uint16 v = 0; v < 4; ++v)
                CHECK(c->state.hand.motion.tiles[i].x[v] == placed.tiles[i].x[v] - 5 &&
                      c->state.hand.motion.tiles[i].y[v] == placed.tiles[i].y[v] + 3);
        in.entering = false;
        in.room_shift_x = in.screen_shift_x = in.screen_shift_y = 0;
        Uint16 leased = 0;
        for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
            leased += actors->primitives[i].owner && i != 7;
        CHECK(leased == SOTN_HAND_WEAPON_FIST_ROWS && SotnBatRuntime_RetireHand(r, false) &&
              SotnActors_CommitBatPrimitives(actors, r));
        for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
            CHECK(actors->primitives[i].owner == (i == 7 ? 777U : 0U));
        // Three free rows cannot satisfy the mandatory four: no attack, group or property roll.
        for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
            actors->primitives[i].owner = i < 3 ? 0U : 777U;
        SDL_zero(*r);
        CHECK(SotnCharacter_Get(character, &progress, NULL));
        expected = progress.random;
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_RequestHand(r, source, 0x1030 | ((Uint32)k << 16), id, &allocated) &&
              allocated && SotnBatRuntime_Tick(r, character, in) && !r->children[0].kind &&
              !r->hand_attack && !r->groups[1] && SotnCharacter_Get(character, &progress, NULL) &&
              progress.random == expected && SotnActors_CommitBatPrimitives(actors, r));
        for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
            CHECK(actors->primitives[i].owner == (i < 3 ? 0U : 777U));
    }
    in.executable = (SotnDataView){0};
    SotnCharacter_Destroy(character);
    SDL_free(before);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}
// w_052 armed by its spell (D_8017ABBC): it holds MP regeneration, drains one MP on every
// sixteenth game update and restores its cached attack; raised (phase1/2) it attacks (hitbox |2),
// a hit requests func_8010E168/healing/a heart, and the timer or MP end it with a fresh property
// roll, attack1 and the ((hand+1)<<14)|0x3A letters.
static bool GuardProperties(void *context, Uint32 *random, SotnEquipment *out)
{
    (void)context;
    (void)SotnCombat_Random(random);
    *out = (SotnEquipment){.id = 16, .attack = 20, .hit_type = 4, .element = 64};
    return true;
}
// w_010 (Iron shield): a raised hit (step1/2) requests WFACTORY(56,0)'s thunder and
// func_8010E168(0,0x60), clearing hitFlags; lowering or birth ignores it.
static bool IronGuard(void)
{
    Uint8 o[0x1200] = {0};
    Resources(o);
    // The fixture authors family52's table at0xB84; w_010's sits at0xB68 (hitboxes 0x48 below).
    SDL_memmove(o + 0xb68 - 0x48, o + 0xb84 - 0x48, 0x48);
    SDL_memmove(o + 0xb68, o + 0xb84, 14 * 4);
    for (Uint16 hand = 0; hand < 2; ++hand)
    {
        SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                                  .properties = Properties,
                                  .priority = 148,
                                  .animation = 0x41,
                                  .remaining = 1,
                                  .attack_mode = 0x8002,
                                  .scale_y = 256,
                                  .family = 10,
                                  .hand = hand,
                                  .held = true};
        SotnHandWeapon w = {.hit_flags = 1};
        SotnHandWeaponProgram program = {0};
        SotnHandWeaponEvents e = {0};
        Uint32 rng = 9;
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && w.phase == 1 && !e.factory &&
              !e.guard_blink && w.hit_flags == 1);
        w.hit_flags = 3;
        e = (SotnHandWeaponEvents){0};
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) &&
              e.factory == (56U | (hand + 1U) << 12) && e.guard_blink && !w.hit_flags);
        e = (SotnHandWeaponEvents){0};
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && !e.factory && !e.guard_blink);
        // A hit while following the player's animation (step2) counts too.
        w.phase = 2;
        w.hit_flags = 1;
        e = (SotnHandWeaponEvents){0};
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && e.guard_blink &&
              e.factory == (56U | (hand + 1U) << 12) && !w.hit_flags);
        in.held = false;
        w.hit_flags = 1;
        e = (SotnHandWeaponEvents){0};
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && w.phase == 3 && !e.factory &&
              !e.guard_blink && w.hit_flags == 1);
    }
    return true;
}
// w_023 (Herald shield): a raised (step1/2) hitFlags3 with no cooldown adds a heart and starts
// unk84's 0x20 cooldown, decremented on the same update and every later one; hitFlags1 stays
// set and grants nothing; a hitFlags3 during the cooldown waits for it to expire.
static bool HeraldGuard(void)
{
    Uint8 o[0x1200] = {0};
    Resources(o);
    SDL_memmove(o + 0xb20 - 0x48, o + 0xb84 - 0x48, 0x48 + 14 * 4);
    // Family52's table must not be read for family23.
    SDL_memset(o + 0xb84, 0xff, 14 * 4);
    for (Uint16 hand = 0; hand < 2; ++hand)
    {
        SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                                  .properties = Properties,
                                  .priority = 148,
                                  .animation = 0x41,
                                  .remaining = 1,
                                  .attack_mode = 0x8002,
                                  .scale_y = 256,
                                  .family = 23,
                                  .hand = hand,
                                  .held = true};
        SotnHandWeapon w = {.hit_flags = 3};
        SotnHandWeaponProgram program = {0};
        SotnHandWeaponEvents e = {0};
        Uint32 rng = 9;
        // Birth (step0) runs before the check falls through to step1: no heart yet.
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && w.phase == 1 && !e.hearts &&
              w.hit_flags == 3 && !w.guard_cooldown && !e.factory);
        e = (SotnHandWeaponEvents){0};
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && e.hearts && !w.hit_flags &&
              w.guard_cooldown == 0x1f && !e.guard_blink && !e.shield_hit);
        w.hit_flags = 1;
        e = (SotnHandWeaponEvents){0};
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && !e.hearts && w.hit_flags == 1 &&
              w.guard_cooldown == 0x1e);
        w.hit_flags = 3;
        for (Uint16 n = 0; n < 0x1e; ++n)
        {
            e = (SotnHandWeaponEvents){0};
            CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && !e.hearts &&
                  w.hit_flags == 3 && w.guard_cooldown == 0x1d - n);
        }
        e = (SotnHandWeaponEvents){0};
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && e.hearts && !w.hit_flags &&
              w.guard_cooldown == 0x1f);
        // A plain hit (hitFlags1) with no cooldown adds nothing and stays recorded.
        while (w.guard_cooldown)
        {
            e = (SotnHandWeaponEvents){0};
            CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e));
        }
        w.hit_flags = 1;
        e = (SotnHandWeaponEvents){0};
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && !e.hearts && w.hit_flags == 1 &&
              !w.guard_cooldown);
        // Following the player's animation (step2) counts; lowering (step3) does not, but the
        // cooldown still counts down.
        w.phase = 2;
        w.guard_cooldown = 0;
        w.hit_flags = 3;
        e = (SotnHandWeaponEvents){0};
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && e.hearts &&
              w.guard_cooldown == 0x1f);
        in.held = false;
        w.guard_cooldown = 0;
        w.hit_flags = 3;
        e = (SotnHandWeaponEvents){0};
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && w.phase == 3 && !e.hearts &&
              w.hit_flags == 3);
        w.guard_cooldown = 5;
        e = (SotnHandWeaponEvents){0};
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && w.guard_cooldown == 4);
    }
    return true;
}
// w_024 (Dark shield): birth rolls rand()&3 for one of D_8017ACF8's four palette sequences
// before SetWeaponProperties; every later update not ending the shield steps (duration,
// offset) entries from base palette0x110/0x128, and an 0xFFFF end applies its offset, restarts
// at entry0 the next update and rolls again. Knock-away keeps cycling.
static bool DarkGuard(void)
{
    Uint8 o[0x1200] = {0};
    Resources(o);
    SDL_memmove(o + 0xc20 - 0x48, o + 0xb84 - 0x48, 0x48 + 14 * 4);
    SDL_memset(o + 0xb84, 0xff, 14 * 4);
    // Authored sequences (duration, signed offset) ending in 0xFFFF.
    const Sint16 sequences[4][8] = {{3, 1, 2, 5, -1, 9, 0, 0},
                                    {1, 2, 2, -3, -1, 0, 0, 0},
                                    {4, 3, -1, 6, 0, 0, 0, 0},
                                    {2, 7, 1, 4, 3, 2, -1, 11}};
    for (Uint16 t = 0; t < 4; ++t)
    {
        Put32(o + 0xcf8 + t * 4U, 0x8017ad10U + t * 0x20U);
        for (Uint16 k = 0; k < 8; ++k)
            Put16(o + 0xd10 + t * 0x20U + k * 2U, (Uint16)sequences[t][k]);
    }
    for (Uint16 hand = 0; hand < 2; ++hand)
    {
        SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                                  .properties = Properties,
                                  .priority = 148,
                                  .animation = 0x41,
                                  .remaining = 1,
                                  .attack_mode = 0x8002,
                                  .scale_y = 256,
                                  .family = 24,
                                  .hand = hand,
                                  .held = true};
        SotnHandWeapon w = {0};
        SotnHandWeaponProgram program = {0};
        Uint32 rng = 9U + hand * 4U;
        Uint32 model = rng;
        Uint8 table = (Uint8)(SotnCombat_Random(&model) & 3U);
        (void)SotnCombat_Random(&model);
        Uint16 index = 0, timer = 0, rerolls = 0;
        const Uint16 base = (Uint16)(0x110U + hand * 24U);
        for (Uint16 tick = 0; tick < 120; ++tick)
        {
            if (tick == 100)
                in.status = 0x10000; // knocked away: phase4 keeps the cycle
            SotnHandWeaponEvents e = {0};
            CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e));
            if (!tick)
                CHECK(w.dark_table == table);
            Sint16 offset = 0;
            if (!timer)
            {
                timer = (Uint16)sequences[table][index * 2];
                offset = sequences[table][index * 2 + 1];
                CHECK(w.palette == (Uint16)(base + offset));
            }
            else if (!--timer)
            {
                ++index;
                timer = (Uint16)sequences[table][index * 2];
                offset = sequences[table][index * 2 + 1];
                CHECK(w.palette == (Uint16)(base + offset));
                if (timer == 0xffffU)
                {
                    index = 0;
                    timer = 0;
                    table = (Uint8)(SotnCombat_Random(&model) & 3U);
                    ++rerolls;
                }
            }
            CHECK(w.dark_timer == timer && w.dark_index == index && w.dark_table == table &&
                  rng == model);
            if (w.removed)
                break;
        }
        CHECK(rerolls > 4 && w.phase == 4);
    }
    return true;
}
// w_027 (Medusa shield) runs the plain template from its own table at 0xC54.
static bool MedusaGuard(void)
{
    Uint8 o[0x1200] = {0};
    Resources(o);
    SDL_memmove(o + 0xc54 - 0x48, o + 0xb84 - 0x48, 0x48 + 14 * 4);
    SDL_memset(o + 0xb84, 0xff, 14 * 4);
    SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                              .properties = Properties,
                              .priority = 148,
                              .animation = 0x41,
                              .remaining = 1,
                              .attack_mode = 0x8002,
                              .scale_y = 256,
                              .family = 27,
                              .hand = 1,
                              .held = true};
    SotnHandWeapon w = {.hit_flags = 3};
    SotnHandWeaponProgram program = {0};
    Uint32 rng = 9;
    for (Uint16 tick = 0; tick < 20; ++tick)
    {
        SotnHandWeaponEvents e = {0};
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && !e.hearts && !e.factory &&
              !e.guard_blink && w.palette == 0x128 && w.frame);
    }
    return true;
}
// w_028 (Skull shield) runs the plain template from its own table at 0xAE0.
static bool SkullGuard(void)
{
    Uint8 o[0x1200] = {0};
    Resources(o);
    SDL_memmove(o + 0xae0 - 0x48, o + 0xb84 - 0x48, 0x48 + 14 * 4);
    SDL_memset(o + 0xb84, 0xff, 14 * 4);
    SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                              .properties = Properties,
                              .priority = 148,
                              .animation = 0x41,
                              .remaining = 1,
                              .attack_mode = 0x8002,
                              .scale_y = 256,
                              .family = 28,
                              .hand = 0,
                              .held = true};
    SotnHandWeapon w = {.hit_flags = 3};
    SotnHandWeaponProgram program = {0};
    Uint32 rng = 9;
    for (Uint16 tick = 0; tick < 20; ++tick)
    {
        SotnHandWeaponEvents e = {0};
        CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && !e.hearts && !e.factory &&
              w.palette == 0x110 && w.frame);
    }
    return true;
}
static bool AlucardArmed(void)
{
    Uint8 o[0x1200] = {0};
    Resources(o);
    SotnHandWeaponInput in = {.overlay = {o, sizeof(o)},
                              .properties = Properties,
                              .priority = 148,
                              .animation = 0x41,
                              .remaining = 1,
                              .attack_mode = 0x8002,
                              .scale_y = 256,
                              .family = 52,
                              .hand = 1,
                              .held = true,
                              .mp = 40,
                              .game_timer = 1};
    SotnHandWeapon w = {0};
    SotnHandWeaponProgram program = {0};
    SotnHandWeaponEvents e = {0};
    Uint32 rng = 9;
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && w.phase == 1 &&
          w.properties.attack == 1 && program.shield_attack == 19 && !e.mp_hold && !e.mp_drain);
    // Unarmed hits are ignored (no heal/heart requests).
    w.hit_flags = 1;
    e = (SotnHandWeaponEvents){0};
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && !e.shield_hit && w.hit_flags == 1);
    program.alucard_active = 1;
    program.alucard_timer = 3;
    in.game_timer = 32;
    e = (SotnHandWeaponEvents){0};
    Uint32 check = rng;
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && rng == check && e.mp_hold &&
          e.mp_drain && e.shield_hit && !w.hit_flags && w.properties.attack == 19 &&
          (w.hitbox_state & 2U) && program.alucard_timer == 2 && !e.factory);
    in.game_timer = 33;
    e = (SotnHandWeaponEvents){0};
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && e.mp_hold && !e.mp_drain &&
          !e.shield_hit && program.alucard_timer == 1);
    // The timer reaches zero: a fresh roll (one rand), attack1 and the letters.
    e = (SotnHandWeaponEvents){0};
    (void)SotnCombat_Random(&check);
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && rng == check && e.properties &&
          !program.alucard_active && w.properties.attack == 1 && w.hitbox_state == 2 &&
          e.factory == ((2U << 14) | 0x3aU));
    // Disarmed: no hold, no drain.
    in.game_timer = 48;
    e = (SotnHandWeaponEvents){0};
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && !e.mp_hold && !e.mp_drain &&
          w.properties.attack == 1);
    // MP running out ends it too (the drained word reaches zero), even with time left.
    program.alucard_active = 1;
    program.alucard_timer = 100;
    in.mp = 1;
    e = (SotnHandWeaponEvents){0};
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && e.mp_drain &&
          !program.alucard_active && program.alucard_timer == 99 &&
          e.factory == ((2U << 14) | 0x3aU));
    // Lowering (phase3): the hold and drain continue, but no attack bit, timer or expiry.
    program.alucard_active = 1;
    program.alucard_timer = 1;
    in.mp = 40;
    in.held = false;
    in.game_timer = 64;
    e = (SotnHandWeaponEvents){0};
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && w.phase == 3 && e.mp_hold &&
          e.mp_drain && program.alucard_active && program.alucard_timer == 1 && !e.factory);
    // A guard roll (hit type4, attack255 cached) gains the attack bit only while armed and raised.
    in.properties = GuardProperties;
    in.held = true;
    in.mp = 40;
    program = (SotnHandWeaponProgram){0};
    w = (SotnHandWeapon){0};
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && w.hitbox_state == 4 &&
          program.shield_attack == 255 && w.properties.attack == 1);
    program.alucard_active = 1;
    program.alucard_timer = 100;
    e = (SotnHandWeaponEvents){0};
    CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &e) && w.hitbox_state == 6 &&
          w.properties.attack == 255);
    // A params-carrying (0x7F00) shield entity is never the armed one.
    SotnHandWeapon other = {.parameter = 0x100};
    SotnHandWeaponProgram armed = {.alucard_active = 1, .alucard_timer = 100};
    e = (SotnHandWeaponEvents){0};
    CHECK(SotnHandWeapon_Tick(&other, &armed, in, &rng, &e) && !e.mp_hold && !e.mp_drain);
    e = (SotnHandWeaponEvents){0};
    other.hit_flags = 1;
    CHECK(SotnHandWeapon_Tick(&other, &armed, in, &rng, &e) && !e.shield_hit &&
          !(other.hitbox_state & 2U) && armed.alucard_timer == 100);
    return true;
}

// Runtime: Alucard shield16 (family52) armed by its spell. The raised shield holds MP regeneration
// and drains the character's MP on every sixteenth game update; a delivered hit becomes one
// BlinkWhite0x15 factory, the timer13 floor, a kind1 heal request and one heart; at expiry it
// releases blueprint0x3A's E1/F1 letters (eleven rows in32..63), which retire on their own.
// Herald shield in the runtime: a raised block (hitFlags3) adds a heart with the rising heart
// (child51) and HEART_PICKUP; a block during the cooldown keeps its hitFlags, so its heart arrives
// when the cooldown ends (as in the original); at the maximum AddHearts does nothing at all.
static bool HeraldRuntime(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(53, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Definition(dra, 9, 23, 0);
    Uint8 *shield = dra + 0x500 + 9U * 52U;
    shield[14] = 9;
    shield[17] = 128;
    shield[18] = 0x63;
    shield[21] = 0;
    Put16(shield + 40, 4);
    const Uint8 blueprints[2][6] = {{0, 1, 0xc1, 0, 0x23, 0}, {51, 1, 0xc1, 0, 0, 0}};
    const Uint16 ids[2] = {48, 99};
    for (Uint16 b = 0; b < 2; ++b)
        SDL_memcpy(dra + 0xd1d4 + ids[b] * 6U, blueprints[b], 6);
    dra[0xd4b8 + 6] = dra[0xd4b9 + 6] = 16;
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    Uint8 *o = weapons + 23U * 0x7000 + 0x4000;
    Resources(o);
    SDL_memmove(o + 0xb20 - 0x48, o + 0xb84 - 0x48, 0x48 + 14 * 4);
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    CHECK(SotnCharacter_Create(actors->dra, &character));
    CHECK(SotnCharacter_Collect(character, false, 9) &&
          SotnCharacter_Equip(character, SOTN_EQUIP_RIGHT, 9));
    SotnCharacterState progress = {0};
    CHECK(SotnCharacter_Get(character, &progress, NULL));
    progress.hearts = 5;
    CHECK(SotnCharacter_Restore(character, &progress));
    static Uint8 exe[0x24000];
    FistMath(exe);
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = {exe, sizeof(exe)},
                              .weapons = {weapons, 53U * 0x7000},
                              .weapon_family = {23, 23},
                              .weapon_bound = {true, true},
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 128 * 65536,
                                         .y = 100 * 65536,
                                         .priority = 148,
                                         .attack_mode = 0x8002,
                                         .buttons = SOTN_BUTTON_RIGHT_HAND,
                                         .animation = {.id = 0x41, .pose = 3, .remaining = 1}}};
    const SotnPlayerFactorySource source = {.x = in.player.x, .y = in.player.y, .priority = 148};
    bool allocated = false;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestHand(r, source, 0x802030, 9, &allocated) && allocated &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          r->children[0].hand_weapon && r->children[0].state.hand.family == 23);
    Uint16 gains = 0, sounds = 0, risen = 0, hearts_live = 0;
    for (Uint16 t = 1; t < 200; ++t)
    {
        if (t == 150)
        {
            CHECK(SotnCharacter_Get(character, &progress, NULL));
            progress.hearts = progress.hearts_max;
            CHECK(SotnCharacter_Restore(character, &progress));
        }
        CHECK(SotnCharacter_Get(character, &progress, NULL));
        const Uint16 hearts = progress.hearts;
        if (t == 5 || t == 10 || t == 160)
            CHECK(SotnBatRuntime_Hit(r, 0, r->children[0].generation, 3));
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        CHECK(SotnCharacter_Get(character, &progress, NULL));
        gains += (Uint16)(progress.hearts - hearts);
        CHECK(progress.hearts == hearts + (t == 5 || t == 37 ? 1 : 0));
        for (Uint16 k = 0; k < r->sound_count; ++k)
            sounds += r->sounds[k] == 0x67a;
        // Rising-heart births (each lives up to 128 updates).
        Uint16 live = 0;
        for (Uint16 i = 1; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
            live += r->children[i].kind == 51 && r->children[i].initialized;
        risen += live > hearts_live ? (Uint16)(live - hearts_live) : 0;
        hearts_live = live;
    }
    CHECK(gains == 2 && sounds == 2 && risen == 2);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}
static bool AlucardRuntime(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(53, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r);
    TestCharacter_Fill(dra);
    Definition(dra, 16, 52, 20);
    Uint8 *shield = dra + 0x500 + 16U * 52U;
    shield[14] = 9;
    shield[17] = 128;
    shield[18] = 0x63;
    shield[21] = 0;
    Put16(shield + 40, 4);
    // Blueprint99 is AddHearts' rising heart (child51, slots32..63).
    const Uint8 blueprints[4][6] = {{0, 1, 0xc1, 0, 0x23, 0},
                                    {11, 1, 0x81, 0, 0, 0},
                                    {1, 1, 0xc1, 0, 0, 0},
                                    {51, 1, 0xc1, 0, 0, 0}};
    const Uint16 ids[4] = {48, 44, 58, 99};
    for (Uint16 b = 0; b < 4; ++b)
        SDL_memcpy(dra + 0xd1d4 + ids[b] * 6U, blueprints[b], 6);
    dra[0xd4b8 + 6] = dra[0xd4b9 + 6] = 16;
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    Uint8 *blink = dra + 0xd670 + 0x15U * 20U;
    Put16(blink + 6, 0x100);
    for (Uint16 k = 8; k <= 12; k += 2)
        Put16(blink + k, 256);
    Put16(blink + 14, 0x7002);
    Put16(blink + 16, 0x31);
    Put16(blink + 18, 512);
    Resources(weapons + 52U * 0x7000 + 0x4000);
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    CHECK(SotnCharacter_Create(actors->dra, &character));
    CHECK(SotnCharacter_Collect(character, false, 16) &&
          SotnCharacter_Equip(character, SOTN_EQUIP_RIGHT, 16));
    SotnCharacterState progress = {0};
    CHECK(SotnCharacter_Get(character, &progress, NULL));
    progress.mp = progress.mp_max = 30;
    progress.hearts = 5;
    CHECK(SotnCharacter_Restore(character, &progress));
    // The rising heart's private sway uses the SDK tables.
    static Uint8 exe[0x24000];
    FistMath(exe);
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = {exe, sizeof(exe)},
                              .weapons = {weapons, 53U * 0x7000},
                              .weapon_family = {52, 52},
                              .weapon_bound = {true, true},
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 128 * 65536,
                                         .y = 100 * 65536,
                                         .priority = 148,
                                         .attack_mode = 0x8002,
                                         .buttons = SOTN_BUTTON_RIGHT_HAND,
                                         .animation = {.id = 0x41, .pose = 3, .remaining = 1}}};
    const SotnPlayerFactorySource source = {.x = in.player.x, .y = in.player.y, .priority = 148};
    bool allocated = false;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestHand(r, source, 0x802030, 16, &allocated) && allocated &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          r->children[0].hand_weapon && !r->shield_mp_hold);
    r->hand_program[1].alucard_active = 1;
    r->hand_program[1].alucard_timer = 0x30;
    Uint16 holds = 0, drained = 0, letters = 0, letter_rows = 0, heals = 0, blinks = 0,
           hearts_risen = 0;
    for (Uint16 t = 1; t < 400; ++t)
    {
        in.game_timer = t;
        CHECK(SotnCharacter_Get(character, &progress, NULL));
        const Uint16 mp = progress.mp, hearts = progress.hearts;
        if (t == 5)
            CHECK(SotnBatRuntime_Hit(r, 0, r->children[0].generation, 1));
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        CHECK(SotnCharacter_Get(character, &progress, NULL));
        holds += r->shield_mp_hold;
        drained += (Uint16)(mp - progress.mp);
        if (r->shield_heal)
        {
            CHECK(t == 5 && r->shield_protection == 0x60 && progress.hearts == hearts + 1);
            ++heals;
        }
        else
            CHECK(progress.hearts == hearts && !r->shield_protection);
        for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
            blinks += r->factories[i].cursor.entity_id && r->factories[i].cursor.blueprint == 44 &&
                      r->factories[i].cursor.parameter == 0x1500 && t == 5;
        // AddHearts(1) below the maximum: the rising heart (child51) and SFX_HEART_PICKUP.
        if (t == 5)
        {
            bool sound = false;
            for (Uint16 k = 0; k < r->sound_count; ++k)
                sound |= r->sounds[k] == 0x67a;
            CHECK(sound);
        }
        for (Uint16 i = 1; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
            hearts_risen += r->children[i].kind == 51 && r->children[i].initialized && t == 7;
        for (Uint16 i = 16; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        {
            const SotnBatRuntimeChild *c = &r->children[i];
            if (!c->weapon_secondary || !c->initialized)
                continue;
            CHECK(c->state.secondary.family == 52 && c->kind == 0xf1 && c->primitive_count == 11 &&
                  SotnBatRuntime_SecondaryValid(c, i));
            ++letters;
            letter_rows += !(c->state.secondary.motion.spell_quads[3].draw_mode & 8U);
        }
    }
    // Re-armed with one MP: the drain takes it to zero and ends the armed state at once.
    CHECK(SotnCharacter_Get(character, &progress, NULL));
    progress.mp = 1;
    CHECK(SotnCharacter_Restore(character, &progress));
    CHECK(r->children[0].hand_weapon);
    {
        r->hand_program[1].alucard_active = 1;
        r->hand_program[1].alucard_timer = 0x30;
        in.game_timer = 0x400;
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
              SotnCharacter_Get(character, &progress, NULL) && !progress.mp &&
              !r->hand_program[1].alucard_active);
    }
    // Thirty-one raised updates hold regeneration and drain on every sixteenth game update.
    CHECK(heals == 1 && blinks == 1 && hearts_risen == 1 && holds >= 0x30 && drained >= 2 &&
          drained <= 4 && !r->hand_program[1].alucard_active && letters > 0x60 && letter_rows > 20);
    for (Uint16 i = 16; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        CHECK(!r->children[i].weapon_secondary && !r->children[i].primitive_count);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}

int main(void)
{
    return Run() && BasicShields() && BasicFists() && PlainWeapons() && Gurthang() && Muramasa() &&
                   HolySword() && Lunges() && ShieldRod() && Katana() && Thunderbrand() &&
                   Weapon49() && Weapon50() && Weapon51() && KnifeAndGreatSword() && Ownership() &&
                   IronGuard() && HeraldGuard() && HeraldRuntime() && DarkGuard() &&
                   MedusaGuard() && SkullGuard() && AlucardArmed() && AlucardRuntime()
               ? 0
               : 1;
}
