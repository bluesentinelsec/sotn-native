// Weapon func_ptr_80170004 secondaries (w_055 Holy sword) with authored tables only: mandatory
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
static void Math(Uint8 *exe)
{
    for (Uint32 i = 0; i <= 1024; ++i)
        Put16(exe + 0x22a10 + i * 2, (Uint16)(i * 4));
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
            CHECK(SotnWeaponSecondary_Tick(&s, in, &rng, &e) && rng == expected && e.properties &&
                  s.phase == 1 && s.allocated && s.flags == 0x960000U &&
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
                CHECK(r->red == 115 && r->green == 115 && r->blue == 231 && r->angle == angle[i] &&
                      r->growth == length[i] / 2 && r->length == length[i] + length[i] / 2 &&
                      r->x[0] == x && r->y[0] == y &&
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
                CHECK(SotnWeaponSecondary_Tick(&s, in, &rng, &e) && rng == kept && !e.properties);
                ++callbacks;
                if (before.frame == 1 && before.remaining == 1)
                    shrink = callbacks;
                if (s.removed)
                    CHECK(before.remaining == -1 &&
                          !SDL_memcmp(s.rows, before.rows, sizeof(s.rows)));
                else
                    CHECK(SotnWeaponSecondary_Animate(&s, in.overlay, 55) &&
                          s.rows[5].blue == (Uint8)(before.rows[5].blue >= 17
                                                        ? before.rows[5].blue - 24U
                                                        : before.rows[5].blue));
            }
            CHECK(callbacks == 10 && shrink == 3 && s.hitbox_width == 12 && s.hitbox_x == 13 &&
                  s.frame == 4);
        }
    // Exhausted mandatory rows retire before any row write, RNG or property roll.
    SotnWeaponSecondary s = {0};
    SotnWeaponSecondaryEvents e = {0};
    Uint32 rng = 5;
    in.allocated = false;
    CHECK(SotnWeaponSecondary_Tick(&s, in, &rng, &e) && s.removed && rng == 5 && !e.properties &&
          !s.phase && !s.rows[0].blue);
    in.allocated = true;
    // Animation range and unk46 gate use the parameter-selected frameStart.
    const Uint16 animations[] = {0x40, 0x48, 0x43};
    const Uint16 modes[] = {0x801d, 0x801d, 0};
    for (Uint16 k = 0; k < 3; ++k)
    {
        s = (SotnWeaponSecondary){0};
        in.animation = animations[k];
        in.attack_mode = modes[k];
        CHECK(SotnWeaponSecondary_Tick(&s, in, &rng, &e) && s.removed && rng == 5);
    }
    in.attack_mode = 0x801d;
    s = (SotnWeaponSecondary){.parameter = 0x100};
    in.animation = 0x50;
    CHECK(SotnWeaponSecondary_Tick(&s, in, &rng, &e) && !s.removed && s.phase == 1);
    // Hit type4 keeps the original attack255 override.
    hit = 4;
    s = (SotnWeaponSecondary){0};
    in.animation = 0x41;
    CHECK(SotnWeaponSecondary_Tick(&s, in, &rng, &e) && s.properties.attack == 255 &&
          s.hitbox_state == 4);
    // Failures leave every word, the RNG cursor and events unchanged.
    const SotnWeaponSecondary before = s;
    const Uint32 kept = rng;
    in.family = 54;
    CHECK(!SotnWeaponSecondary_Tick(&s, in, &rng, &e) && !SDL_memcmp(&s, &before, sizeof(s)) &&
          rng == kept && !SotnWeaponSecondary_Animate(&s, in.overlay, 54));
    in.family = 55;
    in.overlay.size = 0x1000;
    CHECK(!SotnWeaponSecondary_Tick(&s, in, &rng, &e) && !SDL_memcmp(&s, &before, sizeof(s)) &&
          !SotnWeaponSecondary_Animate(&s, in.overlay, 55) && !SDL_memcmp(&s, &before, sizeof(s)));
    in.overlay.size = sizeof(o);
    in.executable.size = 0x1000;
    CHECK(!SotnWeaponSecondary_Tick(&s, in, &rng, &e) && !SDL_memcmp(&s, &before, sizeof(s)) &&
          rng == kept);
    CHECK(SotnWeaponSecondary_Supported(55) && !SotnWeaponSecondary_Supported(54) &&
          SotnWeaponSecondary_Rows(55) == 64 && !SotnWeaponSecondary_Rows(56));
    return true;
}
static void Definition(Uint8 *dra, Uint16 item, Uint8 kind, Uint8 variant, Uint16 mp, Uint8 special)
{
    Uint8 *p = dra + 0x500 + item * 52U;
    Put16(p + 8, 26);
    Put16(p + 12, 64);
    p[14] = 1;
    p[15] = 55;
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
int main(void)
{
    return Module() && Runtime() ? 0 : 1;
}
