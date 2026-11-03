#include "actors_internal.h"
#include "character_fixture.h"
#include <SDL3/SDL.h>
#include <sotn/combat.h>
#include <sotn/thrown_weapon.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static bool Callbacks(SotnDataView executable)
{
    SotnThrownInput in = {.executable = executable,
                          .priority = 148,
                          .hand = 1,
                          .player_x = 144 * 65536 + 12,
                          .player_y = 132 * 65536 + 34,
                          .offset_x = -3,
                          .offset_y = -5,
                          .facing_left = true};
    Uint32 random = 99, counter = SDL_MAX_UINT32;
    SotnThrownEvents e = {0};
    SotnThrownWeapon s = {.x = 128 * 65536 + 123, .y = 100 * 65536 + 456};
    CHECK(SotnThrownWeapon_Tick(&s, in, &random, &counter, &e) && s.removed && !s.phase &&
          !e.properties && !e.clear_shield && !e.sound_count && !e.use_ticks && random == 99);
    s = (SotnThrownWeapon){.x = 128 * 65536 + 123, .y = 100 * 65536 + 456};
    in.primitive_allocated = true;
    CHECK(SotnThrownWeapon_Tick(&s, in, &random, &counter, &e) && !s.removed && s.phase == 1 &&
          s.flags == 0x08800000 && s.x == 128 * 65536 + 123 && s.y == 100 * 65536 + 456 &&
          s.lifetime == 1 && s.frame == 14 && e.use_ticks == 4 && e.properties && e.clear_shield &&
          e.sounds[0] == 0x69e && !e.factory_count && random == 99);
    CHECK(SotnThrownWeapon_Tick(&s, in, &random, &counter, &e) && s.radius == 8 &&
          s.rotation == 256 && s.lifetime == 2 && e.factory_count == 1 &&
          e.factories[0].packed == 0x203a && e.factories[0].source.x == s.x &&
          e.factories[0].source.y == s.y && !e.properties);
    in.role = SOTN_THROWN_TRAIL;
    in.parent = (SotnThrownParent){.bank = 15,
                                   .texture = 126,
                                   .palette = 0x12a,
                                   .frame = 40,
                                   .rotation = -300,
                                   .facing_left = true};
    s = (SotnThrownWeapon){.x = 123, .y = 456};
    CHECK(SotnThrownWeapon_Tick(&s, in, &random, &counter, &e) && s.bank == 15 && s.frame == 40 &&
          s.rotation == -300 && s.opacity == 120 && s.lifetime == 21 && !e.properties &&
          s.x == 123 && s.y == 456);
    for (Uint16 i = 0; i < 20; ++i)
        CHECK(SotnThrownWeapon_Tick(&s, in, &random, &counter, &e) && !s.removed);
    CHECK(s.opacity == 40 && SotnThrownWeapon_Tick(&s, in, &random, &counter, &e) && s.removed);
    in.role = SOTN_THROWN_BOOMERANG;
    s = (SotnThrownWeapon){.x = 123, .y = 456, .parameter = 0x8100};
    CHECK(SotnThrownWeapon_Tick(&s, in, &random, &counter, &e) && e.sound_count == 2 &&
          e.sounds[0] == 0x69e && e.sounds[1] == 0x65b && s.flags == 0x0c000000 &&
          s.x == 127 * 65536 + 123 && s.y == 119 * 65536 + 456 && !s.timer);
    CHECK(SotnThrownWeapon_Tick(&s, in, &random, &counter, &e) && s.timer == 1 &&
          e.factory_count == 2 && e.factories[0].packed == 0x203a &&
          e.factories[1].packed == 0x206e);
    s.timer = 64;
    CHECK(SotnThrownWeapon_Tick(&s, in, &random, &counter, &e) && s.flags == 0x08000000 &&
          s.timer == 65);
    in.role = SOTN_THROWN_JAVELIN;
    s = (SotnThrownWeapon){.x = 123, .y = 456};
    CHECK(SotnThrownWeapon_Tick(&s, in, &random, &counter, &e) && s.velocity_x == -393216 &&
          s.velocity_y == -73728 && s.x == 137 * 65536 + 123 && s.y == 113 * 65536 + 456);
    const Sint32 x = s.x;
    CHECK(SotnThrownWeapon_Tick(&s, in, &random, &counter, &e) && s.x == x - 393216 &&
          s.velocity_y == -70656);
    in.role = SOTN_THROWN_FIRE;
    s = (SotnThrownWeapon){.x = 128 * 65536 + 123, .y = 100 * 65536 + 456};
    Uint32 expected = random;
    for (Uint16 i = 0; i < 3; ++i)
        (void)SotnCombat_Random(&expected);
    CHECK(SotnThrownWeapon_Tick(&s, in, &random, &counter, &e) && random == expected && !counter &&
          !e.factory_count && e.fire_animation && s.flags == 0x08100000);
    s = (SotnThrownWeapon){.x = 128 * 65536 + 123, .y = 100 * 65536 + 456};
    CHECK(SotnThrownWeapon_Tick(&s, in, &random, &counter, &e) && counter == 1 &&
          e.factory_count == 1 && e.factories[0].packed == 0xb0004 &&
          e.factories[0].weapon_dispatch && e.factories[0].source.x == 128 * 65536 + 123 &&
          e.factories[0].source.y == 100 * 65536 + 456);
    SotnThrownWeapon before;
    SotnThrownEvents events;
    SDL_memcpy(&before, &s, sizeof(s));
    SDL_memcpy(&events, &e, sizeof(e));
    expected = random;
    CHECK(!SotnThrownWeapon_Tick(&s, in, &random, NULL, &e) && random == expected &&
          !SDL_memcmp(&before, &s, sizeof(s)) && !SDL_memcmp(&events, &e, sizeof(e)));
    return true;
}
static bool Collision(void)
{
    Uint8 tiles[32 * 8 * 2] = {0}, types[2] = {0, 3};
    for (Uint16 y = 6; y < 8; ++y)
        for (Uint16 x = 0; x < 32; ++x)
            Put16(tiles + ((size_t)y * 32 + x) * 2U, 1);
    SotnThrownInput in = {.role = SOTN_THROWN_IRON,
                          .priority = 148,
                          .player_y = 60 * 65536,
                          .offset_y = -5,
                          .collision = {{tiles, sizeof(tiles)}, {types, sizeof(types)}, 32, 8}};
    SotnThrownWeapon s = {.x = 128 * 65536 + 123, .y = 60 * 65536 + 456};
    Uint32 random = 99, expected = 99;
    (void)SotnCombat_Random(&expected);
    (void)SotnCombat_Random(&expected);
    SotnThrownEvents e = {0};
    CHECK(SotnThrownWeapon_Tick(&s, in, &random, NULL, &e) && s.phase == 1 && random == expected &&
          s.y == 55 * 65536 + 456 && e.properties && e.sounds[0] == 0x6ee && e.use_ticks == 4);
    Uint16 bounces = 0, ticks = 0;
    while (!s.removed && ticks++ < 400)
    {
        CHECK(SotnThrownWeapon_Tick(&s, in, &random, NULL, &e) && random == expected);
        if (e.factory_count)
        {
            ++bounces;
            CHECK(e.factories[0].packed == 0x9001f && e.sound_count && e.sounds[0] == 0x655 &&
                  e.shakes[0] == 4);
        }
    }
    CHECK(s.removed && bounces == 2 && !s.width && s.draw_flags == 0x80 && ticks > 96 &&
          ticks < 400);
    return true;
}
static bool Animation(void)
{
    Uint8 bytes[0x5ec] = {0};
    // Authored duration/frame pairs, independent of original private timeline contents.
    for (Uint16 i = 0; i < 15; ++i)
    {
        Put16(bytes + 0x5b0 + i * 4U, i == 14 ? 0xffff : 2);
        Put16(bytes + 0x5b2 + i * 4U, (Uint16)(i + 40));
    }
    SotnThrownAnimation a = {0};
    for (Uint16 i = 0; i < 35; ++i)
        CHECK(SotnThrownWeapon_AnimateFire(&a, (SotnDataView){bytes, sizeof(bytes)}));
    CHECK(a.pose == 13 && a.remaining == -1 && a.frame == 53);
    SotnThrownAnimation before;
    SDL_memcpy(&before, &a, sizeof(a));
    CHECK(!SotnThrownWeapon_AnimateFire(&a, (SotnDataView){bytes, sizeof(bytes) - 1}) &&
          !SDL_memcmp(&before, &a, sizeof(a)));
    return true;
}
static bool Ownership(SotnDataView executable)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(1, 17U * 0x7000U);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r)), *before = SDL_malloc(sizeof(*before));
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r && before);
    TestCharacter_Fill(dra);
    dra[0x500 + 18] = 0x24;
    Put16(dra + 0x500 + 40, 2);
    dra[0x3400 + 84U * 32U + 28] = 3;
    const Uint16 items[] = {80, 81, 82, 134, 135};
    const Uint8 blueprints[] = {50, 62, 68, 62, 63}, chains[] = {3, 3, 2, 3, 4};
    for (Uint16 i = 0; i < 5; ++i)
    {
        Uint8 *p = dra + 0x500 + items[i] * 52U;
        Put16(p + 8, (Uint16)(10 + i));
        p[14] = 8;
        p[15] = 16;
        p[17] = i ? 129 : 130;
        p[18] = 0x5d;
        p[19] = blueprints[i];
        p[20] = i == 3;
        p[22] = chains[i];
        p[25] = 1;
        p[26] = 7;
        Put16(p + 40, 2);
    }
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    for (Uint16 i = 0; i < 5; ++i)
    {
        SotnEquipment e = {0};
        SotnItem item = {0};
        CHECK(SotnEquipment_Read((SotnDataView){dra, 0x11000}, items[i], &e) && e.family == 16 &&
              e.blueprint == blueprints[i] && e.chain_limit == chains[i] &&
              e.kind == (i ? 129 : 130) && SotnCharacter_Item(character, false, items[i], &item) &&
              item.supported && SotnCharacter_Collect(character, false, items[i]) &&
              SotnCharacter_Collect(character, false, items[i]) &&
              SotnCharacter_Equip(character, 0, items[i]));
        Uint16 equipped = 999;
        CHECK(SotnCharacter_UseThrowing(character, 0, items[i], &equipped) &&
              equipped == items[i] &&
              SotnCharacter_UseThrowing(character, 0, items[i], &equipped) && !equipped);
    }
    CHECK(SotnCharacter_Collect(character, true, 84) && SotnCharacter_Equip(character, 5, 84) &&
          SotnCharacter_Collect(character, false, 80) && SotnCharacter_Equip(character, 0, 80));
    Uint16 equipped = 999;
    CHECK(SotnCharacter_UseThrowing(character, 0, 80, &equipped) && equipped == 80);
    const struct
    {
        Uint8 blueprint, child, range, origin;
    } plans[] = {{50, 0, 6, 2}, {58, 1, 0, 0},  {62, 2, 6, 0}, {63, 5, 6, 2},
                 {68, 3, 6, 0}, {110, 4, 5, 0}, {4, 5, 0, 0}};
    for (Uint16 i = 0; i < SDL_arraysize(plans); ++i)
    {
        Uint8 *p = dra + 0xd1d4 + plans[i].blueprint * 6U;
        p[0] = plans[i].child;
        p[1] = 1;
        p[2] = 0xc1;
        p[4] = (Uint8)(plans[i].range | plans[i].origin << 4);
    }
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    dra[0xd4b8 + 10] = 25;
    dra[0xd4b9 + 10] = 31;
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b9 + 12] = 31;
    Uint8 *overlay = weapons + 16U * 0x7000U + 0x4000;
    for (Uint16 i = 0; i < 15; ++i)
    {
        Put16(overlay + 0x5b0 + i * 4U, i == 14 ? 0xffff : 2);
        Put16(overlay + 0x5b2 + i * 4U, (Uint16)(i + 40));
    }
    TestCharacter_Put32(dra + 0xdb98 + 11U * 4U, 0x800a0200);
    dra[0x200] = 2;
    dra[0x201] = 100;
    dra[0x202] = 80;
    dra[0x203] = 30;
    dra[0x204] = dra[0x205] = 2;
    Put16(dra + 0x206, 1);
    Put16(dra + 0x20a, 3);
    TestCharacter_Put32(dra + 0x20c, 0x08800000);
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    actors->primitives[7].owner = 777;
    SotnBatRuntimeInput in = {
        .dra = actors->dra,
        .executable = executable,
        .weapons = {weapons, 17U * 0x7000U},
        .weapon_family = {16, 16},
        .weapon_bound = {true, true},
        .player = {.hp = 70, .x = 128 * 65536 + 123, .y = 140 * 65536 + 456, .priority = 148}};
    bool allocated = false;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestThrown(r, (SotnPlayerFactorySource){.priority = 148}, 0x1032, 80,
                                       &allocated) &&
          allocated && SotnBatRuntime_Tick(r, character, in) && r->children[1].thrown &&
          r->thrown_mask == 2 && r->children[1].primitive_count == 1 && r->use_ticks == 4 &&
          !r->spell_use_ticks && r->magic_clear_shield &&
          SotnActors_CommitBatPrimitives(actors, r) && actors->primitives[7].owner == 777);
    SDL_memcpy(before, r, sizeof(*r));
    in.freeze = 3;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && !r->use_ticks && !r->magic_clear_shield &&
          !SDL_memcmp(&before->children[1], &r->children[1], sizeof(r->children[1])));
    in.freeze = 0;
    CHECK(SotnBatRuntime_Tick(r, character, in) && r->factories[0].thrown_factory &&
          r->factories[0].thrown_parent_slot == 17 && !r->children[47].kind);
    CHECK(SotnBatRuntime_Tick(r, character, in) && r->children[47].kind == 0xe1 &&
          r->children[47].thrown && r->children[47].state.thrown.parent_slot == 17 &&
          r->children[47].state.thrown.motion.lifetime == 21 &&
          r->children[47].state.thrown.motion.rotation ==
              r->children[1].state.thrown.motion.rotation &&
          !r->children[47].primitive_count && !r->children[47].group);
    CHECK(SotnBatRuntime_PruneThrown(r) && SotnActors_CommitBatPrimitives(actors, r) &&
          actors->primitives[7].owner == 777 && !r->thrown_mask);
    SDL_zero(*r);
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestThrown(r, (SotnPlayerFactorySource){.priority = 148}, 0x81203e, 134,
                                       &allocated) &&
          allocated && SotnBatRuntime_Tick(r, character, in) && r->children[1].kind == 0xf2 &&
          SotnBatRuntime_Tick(r, character, in) && SotnBatRuntime_Tick(r, character, in) &&
          r->children[9].kind == 0xf4 && r->thrown_fire_counter[1] == 1);
    SDL_memcpy(before, r, sizeof(*r));
    SotnCharacterState character_before = {0}, after = {0};
    CHECK(SotnCharacter_Get(character, &character_before, NULL));
    in.freeze = 3;
    CHECK(SotnBatRuntime_Tick(r, character, in) &&
          !SDL_memcmp(before->children, r->children, sizeof(r->children)) &&
          !SDL_memcmp(before->factories, r->factories, sizeof(r->factories)) &&
          r->thrown_fire_counter[1] == 1 && SotnCharacter_Get(character, &after, NULL) &&
          after.random == character_before.random);
    in.freeze = 0;
    Uint32 expected = character_before.random;
    for (Uint16 i = 0; i < 9; ++i)
        (void)SotnCombat_Random(&expected);
    CHECK(SotnBatRuntime_Tick(r, character, in) && r->children[46].thrown_particle_parent &&
          r->children[46].kind == 5 && r->children[46].primitive_count == 3 &&
          r->children[9].state.thrown.motion.y ==
              before->children[9].state.thrown.motion.y - 65536 &&
          r->thrown_fire_counter[1] == 2 && SotnCharacter_Get(character, &after, NULL) &&
          after.random == expected && SotnActors_CommitBatPrimitives(actors, r));
    SDL_memcpy(before, r, sizeof(*r));
    CHECK(SotnCharacter_Get(character, &character_before, NULL));
    in.freeze = 3;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) &&
          !SDL_memcmp(before->children, r->children, sizeof(r->children)) &&
          !SDL_memcmp(before->factories, r->factories, sizeof(r->factories)) &&
          SotnCharacter_Get(character, &after, NULL) && after.random == character_before.random);
    r->children[46].particle_parent_slot = 99;
    SDL_memcpy(before, r, sizeof(*r));
    CHECK(!SotnBatRuntime_Tick(r, character, in) && !SDL_memcmp(before, r, sizeof(*r)) &&
          SotnCharacter_Get(character, &after, NULL) && after.random == character_before.random);
    r->children[46].particle_parent_slot = 25;
    const Uint32 retained_counter = r->thrown_fire_counter[1];
    CHECK(SotnBatRuntime_PruneThrown(r) && SotnActors_CommitBatPrimitives(actors, r) &&
          !r->thrown_mask && !r->children[46].kind && !r->children[9].kind &&
          r->thrown_fire_counter[1] == retained_counter && actors->primitives[7].owner == 777);
    SDL_zero(*r);
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        actors->primitives[i].owner = 777;
    in.freeze = 0;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnCharacter_Get(character, &character_before, NULL) &&
          SotnBatRuntime_RequestThrown(r, (SotnPlayerFactorySource){.priority = 148}, 0x1032, 80,
                                       &allocated) &&
          allocated && SotnBatRuntime_Tick(r, character, in) && !r->children[1].kind &&
          !r->thrown_mask && !r->use_ticks && !r->magic_clear_shield && !r->sound_count &&
          SotnCharacter_Get(character, &after, NULL) && after.random == character_before.random &&
          SotnActors_CommitBatPrimitives(actors, r));
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        CHECK(actors->primitives[i].owner == 777);
    SDL_zero(*r);
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        CHECK(SotnBatRuntime_RequestThrown(r, (SotnPlayerFactorySource){.priority = 148}, 0x1032,
                                           80, &allocated) &&
              allocated);
    SDL_memcpy(before, r, sizeof(*r));
    CHECK(SotnBatRuntime_RequestThrown(r, (SotnPlayerFactorySource){.priority = 148}, 0x1032, 80,
                                       &allocated) &&
          !allocated && !SDL_memcmp(before, r, sizeof(*r)) &&
          SotnCharacter_Get(character, &after, NULL) && after.random == character_before.random);
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
    Uint8 *bytes = SDL_calloc(1, 0x23212);
    if (!bytes)
        return 1;
    for (Uint16 i = 0; i <= 1024; ++i)
    {
        Put16(bytes + 0x1dbcc + i * 2U, (Uint16)(i * 4U));
        Put16(bytes + 0x22a10 + i * 2U, (Uint16)(i / 2U));
    }
    const bool ok = Callbacks((SotnDataView){bytes, 0x23212}) && Collision() && Animation() &&
                    Ownership((SotnDataView){bytes, 0x23212});
    SDL_free(bytes);
    if (!ok)
        return 1;
    SDL_Log(
        "THROWN_WEAPON: PASS authored callbacks, hidden-row failure, raw trail, factory order, "
        "RNG/BSS, actual map collision/bounce/rest, animation and atomicity; no native readiness");
    return 0;
}
