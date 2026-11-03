#include "actors_internal.h"
#include "character_fixture.h"
#include <SDL3/SDL.h>
#include <sotn/combat.h>
#include <sotn/star_weapon.h>
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
static bool Run(void)
{
    Uint8 overlay[0x800] = {0}, tiles[64 * 16 * 2] = {0}, types[] = {3};
    for (Uint16 i = 0; i < 4; ++i)
        Put16(overlay + 0x6b8 + i * 2U, (Uint16)(31 + i));
    for (Uint16 i = 0; i < 16; ++i)
    {
        Put16(overlay + 0x6fc + i * 2U, (Uint16)(i * 3U));
        Put16(overlay + 0x71c + i * 2U, (Uint16)(i * 2U));
    }
    for (Uint16 i = 0; i < 15; ++i)
    {
        Put16(overlay + 0x6c0 + i * 4U, i == 14 ? 0xffff : 3);
        Put16(overlay + 0x6c2 + i * 4U, (Uint16)(40 + i));
        Put16(overlay + 0x770 + i * 4U, i == 14 ? 0xffff : 2);
        Put16(overlay + 0x772 + i * 4U, (Uint16)(60 + i));
    }
    for (Uint16 i = 16; i < 24; ++i)
        Put16(overlay + 0x78c + i * 2U, (Uint16)(i * 19U));
    SotnStarInput in = {.overlay = {overlay, sizeof(overlay)},
                        .role = SOTN_STAR_PROJECTILE,
                        .priority = 148,
                        .hand = 1,
                        .player_x = 144 * 65536 + 12,
                        .player_y = 132 * 65536 + 34,
                        .offset_x = -3,
                        .offset_y = -5,
                        .facing_left = true};
    SotnStarWeapon n = {
        .motion = {.x = 128 * 65536 + 123, .y = 100 * 65536 + 456, .parameter = 0x8200}};
    Uint32 random = 99, counter = 0;
    SotnStarEvents e = {0};
    CHECK(SotnStarWeapon_Tick(&n, in, &random, &counter, &e) && n.motion.phase == 1 &&
          n.motion.frame == 33 && n.motion.x == 147 * 65536 + 123 &&
          n.motion.y == 118 * 65536 + 456 && n.motion.velocity_x == -524288 &&
          e.effects.properties && e.effects.clear_shield && e.effects.use_ticks == 4 &&
          random == 99);
    CHECK(SotnStarWeapon_Tick(&n, in, &random, &counter, &e) && e.effects.factory_count == 1 &&
          e.effects.factories[0].packed == 0x203a && e.effects.factories[0].source.x == n.motion.x);
    n.hit_flags = 2;
    CHECK(SotnStarWeapon_Tick(&n, in, &random, &counter, &e) && n.motion.phase == 2 &&
          n.motion.lifetime == 24 && n.attack_cleared && (n.motion.draw_flags & 0x80));
    Sint32 x = n.motion.x;
    for (Uint16 i = 0; i < 23; ++i)
        CHECK(SotnStarWeapon_Tick(&n, in, &random, &counter, &e) && !n.motion.removed &&
              n.motion.x == x);
    CHECK(SotnStarWeapon_Tick(&n, in, &random, &counter, &e) && n.motion.removed);
    in.role = SOTN_STAR_TRAIL;
    in.parent = (SotnStarParent){.image = {.bank = 15,
                                           .frame = 40,
                                           .texture = 126,
                                           .palette = 0x12a,
                                           .rotation = -300,
                                           .facing_left = true},
                                 .priority = 144};
    n = (SotnStarWeapon){.motion = {.x = 123, .y = 456}};
    CHECK(SotnStarWeapon_Tick(&n, in, &random, &counter, &e) && n.motion.bank == 15 &&
          n.motion.frame == 40 && n.motion.rotation == 0 && n.motion.priority == 142 &&
          n.motion.opacity == 120 && n.motion.lifetime == 9 && n.motion.x == 123 &&
          n.motion.y == 456);
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(SotnStarWeapon_Tick(&n, in, &random, &counter, &e) && !n.motion.removed);
    CHECK(n.motion.opacity == 56 && SotnStarWeapon_Tick(&n, in, &random, &counter, &e) &&
          n.motion.removed);
    in.parent = (SotnStarParent){0};
    n = (SotnStarWeapon){.motion = {.x = 123, .y = 456}};
    CHECK(SotnStarWeapon_Tick(&n, in, &random, &counter, &e) && !n.motion.bank && !n.motion.frame &&
          n.motion.priority == 65534U && n.motion.lifetime == 9 && random == 99);
    in.role = SOTN_STAR_TNT;
    in.primitive_allocated = false;
    n = (SotnStarWeapon){.motion = {.x = 128 * 65536 + 123, .y = 100 * 65536 + 456}};
    CHECK(SotnStarWeapon_Tick(&n, in, &random, &counter, &e) && random == 99 &&
          n.motion.flags == 0x08000000 && n.motion.phase == 1 && n.motion.frame == 0 &&
          e.fixed_tnt_properties && !e.effects.properties && e.effects.clear_shield &&
          e.effects.sounds[0] == 0x60c);
    in.primitive_allocated = true;
    n = (SotnStarWeapon){.motion = {.x = 128 * 65536 + 123, .y = 100 * 65536 + 456}};
    Uint32 expected = random;
    for (Uint16 i = 0; i < 64; ++i)
        (void)SotnCombat_Random(&expected);
    CHECK(SotnStarWeapon_Tick(&n, in, &random, &counter, &e) && random == expected &&
          n.motion.flags == 0x08800000);
    for (Uint16 i = 0; i < 32; ++i)
        CHECK(n.sparks[i].direction >= 0 && n.sparks[i].direction < 16 && n.sparks[i].delay >= 0 &&
              n.sparks[i].delay < 16 && n.sparks[i].draw_mode == 10);
    n.hit_flags = 3;
    const Sint32 vx = n.motion.velocity_x;
    in.collision = (SotnCollisionMap){0};
    CHECK(SotnStarWeapon_Tick(&n, in, &random, &counter, &e) && !n.hit_flags &&
          n.motion.velocity_x == vx * 2 / 3 && n.motion.phase == 1 && !e.effects.factory_count);
    in.collision = (SotnCollisionMap){.tiles = {tiles, sizeof(tiles)},
                                      .types = {types, sizeof(types)},
                                      .width = 64,
                                      .height = 16};
    in.game_timer = 8;
    CHECK(SotnStarWeapon_Tick(&n, in, &random, &counter, &e) && n.motion.phase == 2 &&
          n.motion.lifetime == 16 && e.effects.factory_count == 2 &&
          e.effects.factories[0].packed == 0x204c && e.effects.factories[1].packed == 0x10204e &&
          e.effects.shake_count == 1 && e.effects.shakes[0] == 3 && e.effects.sounds[0] == 0x6a3);
    CHECK(SotnStarWeapon_Tick(&n, in, &random, &counter, &e) && n.attack_cleared &&
          n.motion.frame == 0 && n.motion.lifetime == 15 &&
          e.effects.factories[0].packed == 0x10204e && n.motion.radius == 1);
    in.role = SOTN_STAR_EXPLOSION;
    in.parent.exists = false;
    n = (SotnStarWeapon){0};
    expected = random;
    CHECK(SotnStarWeapon_Tick(&n, in, &random, &counter, &e) && n.motion.removed &&
          !e.effects.properties && !e.animation_offset && random == expected);
    in.parent.exists = true;
    in.parent.item = 79;
    n = (SotnStarWeapon){.motion = {.parameter = 0x1700}};
    CHECK(SotnStarWeapon_Tick(&n, in, &random, &counter, &e) && n.motion.phase == 1 &&
          n.scale_x == 23 * 19 && n.scale_y == 23 * 19 && e.inherit_group && e.effects.properties &&
          e.animation_offset == 0x770 && n.item == 79 && random == expected);
    SotnThrownAnimation a = {0};
    CHECK(SotnStarWeapon_Animate(&a, in.overlay, 0x770) && a.pose == 0 && a.remaining == 2 &&
          a.frame == 60);
    for (Uint16 i = 0; i < 27; ++i)
        CHECK(SotnStarWeapon_Animate(&a, in.overlay, 0x770));
    CHECK(a.pose == 13 && a.remaining == 1 && a.frame == 73 &&
          SotnStarWeapon_Animate(&a, in.overlay, 0x770) && a.pose == 13 && a.remaining == -1 &&
          a.frame == 73);
    SotnStarWeapon before = n;
    SotnStarEvents events = e;
    in.overlay.size = 1;
    n.motion.phase = 0;
    before = n;
    CHECK(!SotnStarWeapon_Tick(&n, in, &random, &counter, &e) &&
          !SDL_memcmp(&n, &before, sizeof(n)) && !SDL_memcmp(&e, &events, sizeof(e)) &&
          random == expected);
    return true;
}
static bool Ownership(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(16U, 0x7000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r)), *before = SDL_malloc(sizeof(*before));
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r && before);
    TestCharacter_Fill(dra);
    Uint8 *p = dra + 0x500 + 79U * 52U;
    Put16(p + 8, 12);
    Put16(p + 12, 0x20);
    p[14] = 8;
    p[15] = 15;
    p[17] = 130;
    p[18] = 0x60;
    p[19] = 68;
    p[22] = 1;
    p[25] = 1;
    p[26] = 20;
    Put16(p + 38, 4);
    Put16(p + 40, 2);
    Put16(p + 42, 1);
    const struct
    {
        Uint8 bp, child, amount, cycle, delay, range;
    } plans[] = {{68, 3, 1, 0xc1, 0, 6}, {76, 4, 14, 0xc2, 3, 8}, {78, 4, 1, 0xc1, 0, 6}};
    for (Uint16 i = 0; i < SDL_arraysize(plans); ++i)
    {
        p = dra + 0xd1d4 + plans[i].bp * 6U;
        p[0] = plans[i].child;
        p[1] = plans[i].amount;
        p[2] = plans[i].cycle;
        p[3] = plans[i].delay;
        p[4] = plans[i].range;
    }
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    dra[0xd4b8 + 16] = 17;
    dra[0xd4b9 + 16] = 63;
    Uint8 *o = weapons + 15U * 0x7000 + 0x4000;
    for (Uint16 i = 0; i < 15; ++i)
    {
        Put16(o + 0x770 + i * 4U, i == 14 ? 0xffff : 2);
        Put16(o + 0x772 + i * 4U, (Uint16)(40 + i));
    }
    for (Uint16 i = 16; i < 24; ++i)
        Put16(o + 0x78c + i * 2U, (Uint16)(i * 11U));
    Uint8 tiles[32 * 16 * 2] = {0}, types[] = {3};
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    actors->primitives[7].owner = 777;
    CHECK(SotnCharacter_Create(actors->dra, &character));
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .weapons = {weapons, 16U * 0x7000},
                              .weapon_family = {15, 15},
                              .weapon_bound = {true, true},
                              .food_collision = {.tiles = {tiles, sizeof(tiles)},
                                                 .types = {types, sizeof(types)},
                                                 .width = 32,
                                                 .height = 16},
                              .player = {.hp = 70,
                                         .hp_max = 70,
                                         .x = 128 * 65536 + 123,
                                         .y = 140 * 65536 + 456,
                                         .priority = 148},
                              .game_timer = 8};
    SotnCharacterState a = {0}, b = {0};
    bool allocated = false;
    CHECK(SotnCharacter_Get(character, &a, NULL));
    Uint32 expected = a.random;
    for (Uint16 i = 0; i < 64; ++i)
        (void)SotnCombat_Random(&expected);
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestStar(
              r, (SotnPlayerFactorySource){.x = in.player.x, .y = in.player.y, .priority = 148},
              0x1044, 79, &allocated) &&
          allocated && SotnBatRuntime_Tick(r, character, in) && r->children[1].star &&
          r->children[1].primitive_count == 32 && r->children[1].group == 3 &&
          r->children[1].state.star.attack.power == 1 && r->use_ticks == 4 &&
          r->magic_clear_shield && SotnCharacter_Get(character, &b, NULL) && b.random == expected &&
          SotnActors_CommitBatPrimitives(actors, r) && actors->primitives[7].owner == 777);
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) &&
          r->children[1].state.star.motion.motion.phase == 2 && r->factories[0].star_factory &&
          r->factories[0].cursor.blueprint == 76 && r->factories[1].cursor.blueprint == 78 &&
          r->wolf_shake_count == 1 && r->wolf_shakes[0] == 3 &&
          SotnActors_CommitBatPrimitives(actors, r));
    expected = b.random;
    for (Uint16 i = 0; i < 7; ++i)
        (void)SotnCombat_Random(&expected);
    SDL_ClearError();
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots));
    const bool third = SotnBatRuntime_Tick(r, character, in);
    CHECK(SotnCharacter_Get(character, &a, NULL));

    CHECK(third && r->factories[0].cursor.spawn_index == 2 && r->children[2].star &&
          r->children[16].star && r->children[3].star && r->children[3].parameter == 0x1000 &&
          r->children[2].state.star.motion.item == 79 && r->children[2].group == 3 &&
          r->children[16].group == 3 && r->children[2].state.star.parent_slot == 17 &&
          !r->children[1].state.star.attack.active && r->children[2].state.star.attack.active &&
          SotnCharacter_Get(character, &a, NULL) && a.random == expected &&
          SotnActors_CommitBatPrimitives(actors, r));
    *before = *r;
    in.freeze = 3;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) &&
          !SDL_memcmp(before->children, r->children, sizeof(r->children)) &&
          !SDL_memcmp(before->factories, r->factories, sizeof(r->factories)) &&
          SotnCharacter_Get(character, &b, NULL) && b.random == a.random);
    in.freeze = 0;
    const size_t size = in.weapons.size;
    in.weapons.size = 1;
    *before = *r;
    CHECK(!SotnBatRuntime_Tick(r, character, in) && !SDL_memcmp(before, r, sizeof(*r)) &&
          SotnCharacter_Get(character, &b, NULL) && b.random == a.random);
    in.weapons.size = size;
    CHECK(SotnBatRuntime_PruneStar(r) && SotnBatRuntime_PruneRoomFactories(r) &&
          SotnActors_CommitBatPrimitives(actors, r) && !r->star_mask && !r->children[1].kind &&
          !r->children[16].kind && actors->primitives[7].owner == 777);
    SDL_zero(*r);
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        actors->primitives[i].owner = 777;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnCharacter_Get(character, &a, NULL) &&
          SotnBatRuntime_RequestStar(
              r, (SotnPlayerFactorySource){.x = in.player.x, .y = in.player.y, .priority = 148},
              0x1044, 79, &allocated) &&
          allocated && SotnBatRuntime_Tick(r, character, in) && r->children[1].star &&
          !r->children[1].primitive_count &&
          r->children[1].state.star.motion.motion.flags == 0x08000000 &&
          r->children[1].state.star.attack.active && SotnCharacter_Get(character, &b, NULL) &&
          b.random == a.random && SotnActors_CommitBatPrimitives(actors, r));
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
    {
        CHECK(actors->primitives[i].owner == 777);
        if (i != 7)
            actors->primitives[i] = (ActorPrimitive){0};
    }
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && !r->children[1].primitive_count &&
          SotnCharacter_Get(character, &b, NULL) && b.random == a.random);
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
    return Run() && Ownership() ? 0 : 1;
}
