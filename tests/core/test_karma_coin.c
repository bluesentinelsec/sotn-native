#include "actors_internal.h"
#include "character_fixture.h"
#include <sotn/bat_runtime.h>
#include <sotn/combat.h>
#include <sotn/karma_coin.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    Put16(p, (Uint16)v);
    Put16(p + 2, (Uint16)(v >> 16));
}
static void Resources(Uint8 *o, Uint8 *exe)
{
    // Authored timelines, UV mesh and linear quarter wave; no private original arrays.
    const Uint16 offsets[] = {0x850, 0x864, 0x8c8, 0x8d8, 0x8e8, 0x954, 0x9d8};
    const Uint16 counts[] = {5, 25, 4, 4, 13, 33, 2};
    for (Uint16 j = 0; j < 7; ++j)
        for (Uint16 i = 0; i < counts[j]; ++i)
        {
            Put16(o + offsets[j] + i * 4U, i == counts[j] - 1U ? 0xffff : 1);
            Put16(o + offsets[j] + i * 4U + 2, (Uint16)(55U + (j == 4 ? 512U : 0)));
        }
    o[0x9e4] = 252;
    o[0x9e5] = 5;
    o[0x9e6] = 6;
    o[0x9e7] = 7;
    for (Uint16 i = 0; i < 3; ++i)
    {
        Put16(o + 0xa10 + i * 2U, (Uint16)(0x8140U + i));
        Put32(o + 0xb44 + i * 4U, 70U - i * 10U);
    }
    for (Uint16 i = 0; i < 24; ++i)
    {
        const Uint8 xy[] = {0, 0, 0, 4, 4, 0, 4, 4};
        SDL_memcpy(o + 0xa1c + i * 8U, xy, sizeof(xy));
        Put32(o + 0xadc + i * 4U, 0);
    }
    for (Uint16 i = 0; i <= 1024; ++i)
        Put16(exe + 0x1dbcc + i * 2U, (Uint16)(i * 4U));
}
static bool Run(void)
{
    Uint8 overlay[0xb50] = {0};
    Uint8 *exe = SDL_calloc(1, 0x1dbcc + 2050);
    CHECK(exe);
    Resources(overlay, exe);
    SotnDataView data = {overlay, sizeof(overlay)};
    SotnKarmaShared shared = {0};
    CHECK(SotnKarmaCoin_LoadProgram(&shared, data) && shared.loaded && !shared.child_counter);
    SotnKarmaShared before_shared = shared;
    overlay[0xadc] = 7;
    CHECK(SotnKarmaCoin_LoadProgram(&shared, data) && shared.angles[0] == 7);
    shared.child_counter = 99;
    CHECK(SotnKarmaCoin_LoadProgram(&shared, data) && !shared.child_counter);
    overlay[0xadc] = 0;
    CHECK(SotnKarmaCoin_LoadProgram(&shared, data));
    before_shared = shared;
    CHECK(!SotnKarmaCoin_LoadProgram(&shared, (SotnDataView){overlay, 0xadc + 95}) &&
          SDL_memcmp(&shared, &before_shared, sizeof(shared)) == 0);
    SotnKarmaCoin n = {.x = 128 * 65536 + 123, .y = 100 * 65536 + 456};
    Uint32 random = 99, expected = random;
    SotnKarmaEvents e = {0};
    SotnKarmaInput in = {.overlay = data,
                         .executable = {exe, 0x1dbcc + 2050},
                         .priority = 148,
                         .hand = 1,
                         .facing_left = true};
    CHECK(SotnKarmaCoin_Tick(&n, &shared, in, &random, &e) && n.removed && random == 99 &&
          !e.clear_shield && !e.properties && !e.use_ticks && !e.protect_ticks);
    n = (SotnKarmaCoin){.x = 128 * 65536 + 123, .y = 100 * 65536 + 456};
    in.allocated = true;
    Sint32 speed = (Sint32)(SotnCombat_Random(&expected) & 16383U) + 57344;
    CHECK(SotnKarmaCoin_Tick(&n, &shared, in, &random, &e) && random == expected && n.phase == 1 &&
          n.velocity_x == -speed && n.velocity_y == -262144 && !n.facing_left && n.bank == 0x8012 &&
          n.palette == 0x128 && n.texture == 102 && n.priority == 146 && e.clear_shield &&
          !e.use_ticks && !e.protect_ticks && !e.properties && n.flags == 0x08900000);
    CHECK(SotnKarmaCoin_Animate(&n, data) && n.frame == 55 && n.remaining == 1);
    Uint8 tiles[16 * 16 * 2] = {0}, types[] = {3};
    in.collision = (SotnCollisionMap){.tiles = {tiles, sizeof(tiles)},
                                      .types = {types, sizeof(types)},
                                      .width = 16,
                                      .height = 16};
    CHECK(SotnKarmaCoin_Tick(&n, &shared, in, &random, &e) && n.phase == 2 &&
          n.animation == 0x864 && n.pose == 0 && n.remaining == 0 && e.sound == 0x6a9 &&
          n.x == 128 * 65536 + 123 - speed);
    n.pose = 13;
    n.remaining = 1;
    n.volume_index = 1;
    CHECK(SotnKarmaCoin_Tick(&n, &shared, in, &random, &e) && e.volume_sound && e.volume == 60 &&
          n.volume_index == 2);
    n.remaining = -1;
    expected = random;
    const bool heads = (SotnCombat_Random(&expected) & 1U) != 0;
    CHECK(SotnKarmaCoin_Tick(&n, &shared, in, &random, &e) && random == expected && n.phase == 3 &&
          n.animation == (heads ? 0x8c8 : 0x8d8) && !n.pose && !n.remaining);
    // Tail uses its internal frame-property animation after clearing the outer flag.
    n = (SotnKarmaCoin){.phase = 3, .animation = 0x8d8, .remaining = -1, .flags = 0x08900000};
    expected = random;
    CHECK(SotnKarmaCoin_Tick(&n, &shared, in, &random, &e) && n.phase == 4 && e.use_ticks == 4 &&
          !e.protect_ticks && e.properties && !e.fixed_properties && e.flash == 7 &&
          e.sound == 0x665 && n.priority == 438 && !(n.flags & 0x100000U) && random == expected);
    CHECK(SotnKarmaCoin_Tick(&n, &shared, in, &random, &e) && n.frame == 55 && n.hitbox_x == -4 &&
          n.hitbox_y == 5 && n.hitbox_width == 6 && n.hitbox_height == 7 && n.remaining == 1);
    for (Uint16 i = 0; i < 12; ++i)
        CHECK(SotnKarmaCoin_Tick(&n, &shared, in, &random, &e));
    CHECK(n.remaining == -1 && !n.removed && SotnKarmaCoin_Tick(&n, &shared, in, &random, &e) &&
          n.removed);
    // Head birth does not move its mesh or consume a property RNG call.
    n = (SotnKarmaCoin){
        .phase = 3, .animation = 0x8c8, .remaining = -1, .x = 123, .y = 456, .flags = 0x08900000};
    expected = random;
    in.game_timer = 1;
    CHECK(SotnKarmaCoin_Tick(&n, &shared, in, &random, &e) && n.phase == 5 &&
          n.x == 128 * 65536 + 123 && n.y == 456 && e.fixed_properties && !e.properties &&
          e.protect_ticks == 4 && !e.use_ticks && e.flash == 8 && e.sound == 0x6b1 &&
          random == expected && n.timer == 224 && n.opacity == 128 && n.frame == 29 &&
          n.draw_flags == 128 && n.blend == 48 && n.flags == 0x800000 && shared.angles[0] == 0 &&
          n.quads[1].draw_mode == 8 && n.quads[25].draw_mode == 61 && n.quads[1].palette == 0x159);
    before_shared = shared;
    CHECK(SotnKarmaCoin_Tick(&n, &shared, in, &random, &e) && n.scale_x == 252 &&
          n.scale_y == 252 && n.brightness == 8 && n.quads[0].draw_mode == 0x435 &&
          shared.angles[0] == 64 && shared.angles[23] == 64 && before_shared.angles[0] == 0);
    CHECK(n.quads[2].x[0] == 210 &&
          n.quads[25].x[1] == 206); // Last seam reads already advanced angle zero.
    while (n.phase == 5)
        CHECK(SotnKarmaCoin_Tick(&n, &shared, in, &random, &e));
    CHECK(n.phase == 6 && e.factory == 0x2054 && n.scale_x == 40);
    for (Uint16 i = 0; i < 41; ++i)
        CHECK(SotnKarmaCoin_Tick(&n, &shared, in, &random, &e) && !e.factory);
    CHECK(n.phase == 7 && n.scale_x == 0 && !(n.quads[1].draw_mode & 8));
    for (Uint16 i = 0; i < 224; ++i)
        CHECK(SotnKarmaCoin_Tick(&n, &shared, in, &random, &e) && n.phase == 7);
    CHECK(n.timer == 0 && SotnKarmaCoin_Tick(&n, &shared, in, &random, &e) && n.phase == 8 &&
          n.timer == -1);
    Uint16 ticks = 0;
    while (!n.removed && ticks++ < 100)
        CHECK(SotnKarmaCoin_Tick(&n, &shared, in, &random, &e));
    CHECK(n.removed && ticks < 100);
    // Raw parent and signed counter choose children before birth RNG/movement.
    in.child = true;
    in.parent_exists = true;
    in.bank = 15;
    in.texture = 126;
    shared.child_counter = 1;
    n = (SotnKarmaCoin){.x = 123, .y = 456};
    expected = random;
    CHECK(SotnKarmaCoin_Tick(&n, &shared, in, &random, &e) && n.phase == 1 &&
          n.animation == 0x9d8 && n.x == 140 * 65536 + 123 + 4096 &&
          n.y == 100 * 65536 + 456 + 17408 && n.priority == 144 && random == expected &&
          shared.child_counter == 2 && n.palette == 0x8159 && n.bank == 15 && n.texture == 126);
    shared.child_counter = SDL_MAX_UINT32;
    n = (SotnKarmaCoin){.x = 123, .y = 456};
    expected = random;
    Uint16 pose = SotnCombat_Random(&expected) & 31U;
    Sint32 vy = (Sint32)(SotnCombat_Random(&expected) & 8191U) + 16384;
    CHECK(SotnKarmaCoin_Tick(&n, &shared, in, &random, &e) && shared.child_counter == 0 &&
          n.x == 138 * 65536 + 123 + 4864 && n.y == 66 * 65536 + 456 + vy && n.pose == pose &&
          random == expected);
    in.parent_exists = false;
    expected = random;
    before_shared = shared;
    CHECK(SotnKarmaCoin_Tick(&n, &shared, in, &random, &e) && n.removed && random == expected &&
          SDL_memcmp(&shared, &before_shared, sizeof(shared)) == 0);
    // Failed table/math/timeline work commits no state, writable data, RNG or events.
    n = (SotnKarmaCoin){.phase = 5, .scale_x = 256, .scale_y = 256, .angular_speed = 64};
    SotnKarmaCoin before = n;
    before_shared = shared;
    SotnKarmaEvents before_e = e;
    expected = random;
    in.child = false;
    in.executable = (SotnDataView){0};
    CHECK(!SotnKarmaCoin_Tick(&n, &shared, in, &random, &e) &&
          SDL_memcmp(&n, &before, sizeof(n)) == 0 &&
          SDL_memcmp(&shared, &before_shared, sizeof(shared)) == 0 &&
          SDL_memcmp(&e, &before_e, sizeof(e)) == 0 && random == expected);
    n = (SotnKarmaCoin){.animation = 0x850, .pose = 3, .remaining = 1};
    before = n;
    Put16(overlay + 0x860, 0xfffe);
    CHECK(!SotnKarmaCoin_Animate(&n, data) && SDL_memcmp(&n, &before, sizeof(n)) == 0);
    SDL_free(exe);
    return true;
}
static bool Ownership(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(21U, 0x7000);
    Uint8 *exe = SDL_calloc(1, 0x1dbcc + 2050);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r)), *before = SDL_malloc(sizeof(*before));
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && exe && actors && r && before);
    TestCharacter_Fill(dra);
    Uint8 *p = dra + 0x500 + 24U * 52U;
    Put16(p + 8, 50);
    Put16(p + 12, 0x20);
    p[14] = 7;
    p[15] = 20;
    p[17] = 130;
    p[18] = 0x60;
    p[19] = 50;
    p[22] = 1;
    p[25] = 1;
    p[26] = 10;
    Put16(p + 38, 4);
    Put16(p + 40, 2);
    Put16(p + 42, 1);
    // Authored scheduling records bind the real shared factory/pool APIs.
    p = dra + 0xd1d4 + 50U * 6U;
    p[1] = 1;
    p[2] = 0xc1;
    p[4] = 6;
    p = dra + 0xd1d4 + 84U * 6U;
    p[0] = 2;
    p[1] = 4;
    p[2] = 0xc1;
    p[3] = 2;
    p[4] = 2;
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    dra[0xd4b8 + 4] = 32;
    dra[0xd4b9 + 4] = 47;
    dra[0xcfb4 + 8U * 4U] = 4;
    Uint8 *overlay = weapons + 20U * 0x7000 + 0x4000;
    Resources(overlay, exe);
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    actors->primitives[7].owner = 777;
    CHECK(SotnCharacter_Create(actors->dra, &character) &&
          SotnKarmaCoin_LoadProgram(&r->karma[0], (SotnDataView){overlay, 0x3000}));
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .weapons = {weapons, 21U * 0x7000},
                              .executable = {exe, 0x1dbcc + 2050},
                              .weapon_family = {20, 20},
                              .weapon_bound = {true, true},
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 128 * 65536 + 123,
                                         .y = 100 * 65536 + 456,
                                         .priority = 148}};
    SotnCharacterState a = {0}, b = {0};
    bool allocated = false;
    CHECK(SotnCharacter_Get(character, &a, NULL));
    Uint32 expected = a.random;
    (void)SotnCombat_Random(&expected);
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestKarma(
              r, (SotnPlayerFactorySource){.x = in.player.x, .y = in.player.y, .priority = 148},
              0x1032, 24, &allocated) &&
          allocated && SotnBatRuntime_Tick(r, character, in) && r->children[1].karma &&
          r->children[1].primitive_count == 26 && !r->children[1].group &&
          !r->children[1].state.karma.attack.active && r->magic_clear_shield && !r->use_ticks &&
          !r->spell_use_ticks && SotnCharacter_Get(character, &a, NULL) && a.random == expected &&
          SotnActors_CommitBatPrimitives(actors, r) && actors->primitives[7].owner == 777);
    // Supplied head-result context tests the real effect/property/group owner.
    SotnKarmaCoin *n = &r->children[1].state.karma.motion;
    n->phase = 3;
    n->animation = 0x8c8;
    n->pose = 2;
    n->remaining = -1;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && n->phase == 5 && r->children[1].group == 3 &&
          r->children[1].state.karma.attack.power == 4 &&
          r->children[1].state.karma.attack.hit_type == 0x82 &&
          r->children[1].state.karma.attack.hit_effect == 2 && r->spell_use_ticks == 4 &&
          !r->use_ticks && SotnCharacter_Get(character, &b, NULL) && b.random == expected &&
          SotnActors_CommitBatPrimitives(actors, r));
    n->scale_x = 40;
    n->scale_y = 40;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && n->phase == 6 && r->factories[0].karma_factory &&
          r->factories[0].cursor.blueprint == 84 && !r->children[16].kind &&
          SotnActors_CommitBatPrimitives(actors, r));
    // An explicit external-owner mask exhausts the actual child range, not its row pool.
    // The noncritical factory retires before child RNG/BSS while the real root continues.
    *before = *r;
    in.other_occupied = ((Uint64)0xffffU << 32);
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && !r->children[16].kind &&
          !r->karma[0].child_counter && r->children[1].state.karma.attack.active &&
          r->children[1].primitive_count == 26 && SotnCharacter_Get(character, &b, NULL) &&
          b.random == expected);
    *r = *before;
    in.other_occupied = 0;
    (void)SotnCombat_Random(&expected);
    (void)SotnCombat_Random(&expected);
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && r->children[16].karma &&
          r->children[16].kind == 0xe2 && !r->children[16].primitive_count &&
          r->children[16].state.karma.parent_slot == 17 &&
          r->children[16].state.karma.motion.bank == n->bank && !r->children[16].group &&
          !r->children[16].state.karma.attack.active && r->karma[0].child_counter == 1 &&
          SotnCharacter_Get(character, &a, NULL) && a.random == expected &&
          SotnActors_CommitBatPrimitives(actors, r));
    *before = *r;
    in.freeze = 3;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) &&
          !SDL_memcmp(before->children, r->children, sizeof(r->children)) &&
          !SDL_memcmp(before->factories, r->factories, sizeof(r->factories)) &&
          !SDL_memcmp(before->karma, r->karma, sizeof(r->karma)));
    in.freeze = 0;
    in.weapons.size = 1;
    *before = *r;
    CHECK(!SotnBatRuntime_Tick(r, character, in) && !SDL_memcmp(before, r, sizeof(*r)) &&
          SotnCharacter_Get(character, &b, NULL) && b.random == expected);
    in.weapons.size = 21U * 0x7000;
    SotnKarmaShared retained = r->karma[0];
    CHECK(SotnBatRuntime_PruneKarma(r) && SotnBatRuntime_PruneRoomFactories(r) &&
          SotnActors_CommitBatPrimitives(actors, r) && !r->karma_mask && !r->children[1].kind &&
          !r->children[16].kind && !SDL_memcmp(&retained, &r->karma[0], sizeof(retained)) &&
          actors->primitives[7].owner == 777);
    SDL_zero(*r);
    CHECK(SotnKarmaCoin_LoadProgram(&r->karma[0], (SotnDataView){overlay, 0x3000}));
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        actors->primitives[i].owner = 777;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestKarma(
              r, (SotnPlayerFactorySource){.x = in.player.x, .y = in.player.y, .priority = 148},
              0x1032, 24, &allocated) &&
          allocated && SotnBatRuntime_Tick(r, character, in) && !r->children[1].kind &&
          !r->karma_mask && !r->magic_clear_shield && SotnCharacter_Get(character, &b, NULL) &&
          b.random == expected && SotnActors_CommitBatPrimitives(actors, r));
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        CHECK(actors->primitives[i].owner == 777);
    SotnCharacter_Destroy(character);
    SDL_free(before);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(exe);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}

int main(void)
{
    return Run() && Ownership() ? 0 : 1;
}
