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
    const Uint16 families[] = {9, 11, 26}, tables[] = {0xa40, 0xb94, 0xa60};
    CHECK(!SotnHandWeapon_IsShieldFamily(10) && !SotnHandWeapon_IsShieldFamily(255));
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
                  w.properties.attack == 255 && program.shield_attack == 123 &&
                  w.bank == 0x8010U + hand * 2U && w.frame == 61 && w.hitbox_x == -7 &&
                  w.hitbox_y == -2 && w.hitbox_width == 8 && w.hitbox_height == 16);
            in.held = false;
            for (Uint16 frame = 0; frame < 4 && !w.removed; ++frame)
                CHECK(SotnHandWeapon_Tick(&w, &program, in, &random, &events) &&
                      events.shield == 2 && !events.properties && random == expected);
            CHECK(w.removed && program.shield_attack == 123);
        }
    return true;
}
static Uint16 Read16Test(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
// Weapon3/33/35/36/57 run w_002's callback from their own descriptors; blends differ.
static bool PlainWeapons(void)
{
    static Uint8 o[0x1200];
    Resources(o);
    SDL_memcpy(o + 0xc28, o + 0x106c, 16);
    SDL_memcpy(o + 0x540, o + 0x106c, 16);
    SDL_memcpy(o + 0x109c, o + 0x106c, 16);
    SDL_memcpy(o + 0xc3c, o + 0x106c, 16);
    SDL_memcpy(o + 0x530, o + 0x106c, 16);
    Put16(o + 0xc28 + 8, 5);
    const Uint16 families[] = {3, 36, 57, 2, 33, 35}, variants[] = {0, 0, 3, 0, 0, 0};
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
                  w.blend == (families[k] == 2 || families[k] == 33 ? 0
                              : families[k] == 35                   ? 0x10
                                                                    : 0x30) &&
                  !e.extra_sounds[0] && !e.extra_sounds[1] && w.bank == 0x8010U + hand * 2U &&
                  w.priority == 98 &&
                  w.palette == 0x110U + hand * 24U + (families[k] == 3 ? 5U : 0U) &&
                  program.short_pose == 4 && program.shield_attack == 9);
            // Nunchaku repeats its descriptor swing and adds SWISH_B at pose12 (timer1).
            in.pose = 12;
            SotnHandWeaponEvents swing = {0};
            CHECK(SotnHandWeapon_Tick(&w, &program, in, &rng, &swing) &&
                  swing.extra_sounds[0] == (families[k] == 33 ? Read16Test(o + 0x106c + 10) : 0) &&
                  swing.extra_sounds[1] == (families[k] == 33 ? 0x60b : 0));
            in.pose = 4;
            // Single-descriptor programs reject any other variant before mutation.
            if (families[k] != 57)
            {
                SotnHandWeapon other = {.parameter = 0x100};
                Uint32 kept = 21;
                CHECK(!SotnHandWeapon_Tick(&other, &program, in, &kept, &e) && kept == 21 &&
                      !other.phase);
            }
        }
    return true;
}
// Authored weapon1 overlay: descriptors0..3, trail offsets and colors; no original tables.
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
    // Descriptor4 is Iron Fist's special; malformed trail colors fail before publication.
    SotnHandWeapon special = {.parameter = 0x400};
    Uint32 special_rng = 3;
    CHECK(!SotnHandWeapon_Tick(&special, &program, in, &special_rng, &e) && special_rng == 3 &&
          !special.phase);
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
        CHECK(SotnBatRuntime_RetireHand(r, true) && !r->children[0].hand_weapon && !r->hand_shield);
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
int main(void)
{
    return Run() && BasicShields() && BasicFists() && PlainWeapons() && Ownership() ? 0 : 1;
}
