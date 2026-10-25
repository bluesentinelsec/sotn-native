#include "character_fixture.h"
#include <SDL3/SDL.h>
#include <sotn/actors.h>

#define CHECK(condition)                                                                           \
    do                                                                                             \
    {                                                                                              \
        if (!(condition))                                                                          \
        {                                                                                          \
            SDL_LogError(SDL_LOG_CATEGORY_TEST, "Line %d: %s: %s", __LINE__, #condition,           \
                         SDL_GetError());                                                          \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

typedef struct Fixture
{
    Uint8 dra[0xd000], nz0[0x3000], tiles[32 * 16 * 2], types[2];
} Fixture;

static void Put16(Uint8 *image, size_t offset, Uint16 value)
{
    image[offset] = (Uint8)value;
    image[offset + 1] = (Uint8)(value >> 8);
}

static void Put32(Uint8 *image, size_t offset, Uint32 value)
{
    for (unsigned int i = 0; i < 4; ++i)
        image[offset + i] = (Uint8)(value >> (i * 8));
}

static SotnCollisionMap MakeFixture(Fixture *fixture)
{
    SDL_memset(fixture, 0, sizeof(*fixture));
    Put32(fixture->nz0, 28, 0x80180040);
    for (size_t i = 0; i < 106; ++i)
        Put32(fixture->nz0, 0x40 + i * 4, 0x80180300);
    Put16(fixture->nz0, 0x300, 0xfffe);
    Put16(fixture->nz0, 0x302, 0xfffe);
    const Uint16 ids[] = {1, 0x23, 0x34, 0x2b, 0x13};
    for (size_t i = 0; i < SDL_arraysize(ids); ++i)
    {
        const size_t offset = 0x300 + (i + 1) * 10;
        Put16(fixture->nz0, offset, (Uint16)(40 + i * 40));
        Put16(fixture->nz0, offset + 2, 140);
        Put16(fixture->nz0, offset + 4, ids[i]);
        Put16(fixture->nz0, offset + 6, (Uint16)i);
    }
    Put16(fixture->nz0, 0x33c, 0xffff);
    Put16(fixture->nz0, 0x33e, 0xffff);
    Put32(fixture->dra, 0x98, 0x800a0200);
    Put32(fixture->dra, 0x224, 0xa8001000);
    const size_t initializers[] = {0xbc8, 0xc34, 0xcc4, 0xc7c};
    for (size_t i = 0; i < SDL_arraysize(initializers); ++i)
    {
        Put16(fixture->nz0, initializers[i], 0x8001);
        Put16(fixture->nz0, initializers[i] + 2, 1);
    }
    Put32(fixture->nz0, 0xe04, 0x80180f00);
    Put16(fixture->nz0, 0xe34, 3);
    fixture->nz0[0xe44] = 0x70;
    const Uint8 candle[] = {2, 1, 1, 2, 0, 0};
    const Uint8 globe[] = {1, 3, 0, 0};
    const Uint8 walk[] = {1, 1, 1, 2, 0, 0};
    const Uint8 attack[] = {1, 3, 1, 4, 255, 0};
    SDL_memcpy(fixture->nz0 + 0xf00, candle, sizeof(candle));
    SDL_memcpy(fixture->nz0 + 0xef0, globe, sizeof(globe));
    SDL_memcpy(fixture->nz0 + 0x22ec, walk, sizeof(walk));
    SDL_memcpy(fixture->nz0 + 0x22fc, attack, sizeof(attack));
    SDL_memcpy(fixture->nz0 + 0x237c, walk, sizeof(walk));
    Put16(fixture->nz0, 0x22d6, 20);
    Put16(fixture->nz0, 0x22da, 3);
    Put16(fixture->nz0, 0x22dc, 6);
    Put16(fixture->nz0, 0x22de, 65533);
    Put16(fixture->nz0, 0x22e0, 65524);
    Put16(fixture->nz0, 0x22e6, 20);
    Put16(fixture->nz0, 0x22e8, 6);
    fixture->types[1] = 3;
    for (size_t y = 10; y < 16; ++y)
        for (size_t x = 0; x < 32; ++x)
            fixture->tiles[(y * 32 + x) * 2] = 1;
    return (SotnCollisionMap){
        {fixture->tiles, sizeof(fixture->tiles)}, {fixture->types, sizeof(fixture->types)}, 32, 16};
}

static bool TestBehaviors(Fixture *fixture)
{
    const SotnCollisionMap map = MakeFixture(fixture);
    const SotnDataView dra = {fixture->dra, sizeof(fixture->dra)},
                       nz0 = {fixture->nz0, sizeof(fixture->nz0)};
    SotnActors *actors = NULL;
    SotnActorLayout *layout = NULL;
    CHECK(SotnActors_Create(dra, nz0, &actors));
    CHECK(SotnActorLayout_Open(nz0, 0, &layout));
    SotnPlayerState player = {0};
    player.x = 10 * 65536;
    player.y = 140 * 65536;
    player.facing_left = true;
    CHECK(!SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Enter(actors, layout, map, 0, 0));
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    SotnActorState state = {0};
    CHECK(SotnActors_Get(actors, 0, &state) && state.animation_set == 3 &&
          state.blend_mode == 0x70);
    CHECK(SotnActors_Get(actors, 1, &state) && state.frame == 7 && state.priority == 0x70);
    CHECK(SotnActors_Get(actors, 2, &state) && state.frame == 3);
    CHECK(SotnActors_Get(actors, 3, &state) && state.step == 2);
    CHECK(SotnActors_Get(actors, 4, &state) && state.unsupported);
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Get(actors, 0, &state) && state.frame == 1);
    CHECK(SotnActors_Get(actors, 3, &state) && state.step == 1 && state.y == 140 * 65536);
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Get(actors, 3, &state));
    CHECK(state.x == 160 * 65536 - 24576 && state.y == 140 * 65536 && state.frame == 2);
    player.x = 150 * 65536;
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Get(actors, 3, &state) && state.step == 3);
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Get(actors, 3, &state) && state.step == 4);
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Get(actors, 3, &state) && state.frame == 3);
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Get(actors, 3, &state) && state.frame == 4);
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Get(actors, 3, &state) && state.step == 1);
    CHECK(SotnActors_Get(actors, 0, &state));
    const Uint32 generation = state.generation;
    CHECK(SotnActors_Tick(actors, player, 500, 0));
    CHECK(SotnActors_Get(actors, 0, &state) && state.generation == 0);
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Get(actors, 0, &state) && state.generation != 0 &&
          state.generation != generation);
    CHECK(!SotnActors_Tick(actors, player, -1, 0));
    player.x = SDL_MAX_SINT32;
    CHECK(!SotnActors_Tick(actors, player, 0, 0));
    CHECK(!SotnActors_Get(actors, 192, &state));
    SotnActors_Destroy(actors);
    SotnActorLayout_Close(layout);
    return true;
}

static bool TestCombat(Fixture *fixture)
{
    const SotnCollisionMap map = MakeFixture(fixture);
    Put16(fixture->nz0, 0x310, 0x100);
    Put16(fixture->dra, 0x20c, 2);
    fixture->dra[0x222] = 8;
    fixture->dra[0x223] = 8;
    fixture->nz0[0xe24] = 8;
    Put16(fixture->nz0, 0xbd4, 3);
    Put16(fixture->nz0, 0xbdc, 2);
    Put16(fixture->dra, 0x25c, 1);
    fixture->dra[0x272] = 6;
    fixture->dra[0x273] = 7;
    Put32(fixture->nz0, 0x1d3c, 0x80180f10);
    const Uint8 prize_animation[] = {1, 14, 0, 0};
    SDL_memcpy(fixture->nz0 + 0xf10, prize_animation, sizeof(prize_animation));
    fixture->nz0[0x1da4] = 1;
    Put32(fixture->nz0, 0x1e54, 0x80180f10);
    Put32(fixture->nz0, 0x1e5c, 0x80180f10);
    Put32(fixture->nz0, 0x1da8, 0xffff0000);
    Put32(fixture->nz0, 0x1db0, 0xfffe8000);
    Put16(fixture->nz0, 0xc84, 1);
    Put32(fixture->dra, 0x24c, 0x1000);
    Put16(fixture->dra, 0x22c, 12);
    Put16(fixture->dra, 0x234, 3);
    fixture->dra[0x24a] = 4;
    fixture->dra[0x24b] = 22;
    const Uint8 end[] = {1, 20, 255, 0};
    SDL_memcpy(fixture->nz0 + 0x2320, end, sizeof(end));
    SDL_memcpy(fixture->nz0 + 0x2334, end, sizeof(end));
    SotnActors *actors = NULL;
    SotnActorLayout *layout = NULL;
    CHECK(SotnActors_Create((SotnDataView){fixture->dra, sizeof(fixture->dra)},
                            (SotnDataView){fixture->nz0, sizeof(fixture->nz0)}, &actors));
    CHECK(SotnActorLayout_Open((SotnDataView){fixture->nz0, sizeof(fixture->nz0)}, 0, &layout));
    CHECK(SotnActors_Enter(actors, layout, map, 0, 0));
    SotnPlayerState player = {0};
    player.x = 22 * 65536;
    player.y = 140 * 65536;
    player.animation.hitbox_width = 4;
    player.animation.hitbox_height = 20;
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    player.attack = (SotnPlayerAttack){.offset_x = 18,
                                       .power = 6,
                                       .width = 10,
                                       .height = 8,
                                       .hit_type = 2,
                                       .hit_effect = 1,
                                       .invincibility_frames = 8,
                                       .active = true};
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    SotnActorState state = {0};
    CHECK(SotnActors_Get(actors, 0, &state) && state.id == 3);
    player.attack.active = false;
    for (Uint32 i = 0; i < 30; ++i)
        CHECK(SotnActors_Tick(actors, player, 0, 0));
    SotnActorCombat combat = {0};
    CHECK(SotnActors_GetCombat(actors, &combat) && combat.hearts == 11);
    CHECK(SotnActors_Get(actors, 0, &state) && state.generation == 0);
    CHECK(SotnActors_Enter(actors, layout, map, 0, 0));
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Get(actors, 0, &state) && state.generation != 0 && state.id == 1);
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    player.x = 142 * 65536;
    player.attack.active = true;
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Get(actors, 3, &state) && state.hit_points == 6);
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Get(actors, 3, &state) && state.hit_points == 6);
    for (Uint32 i = 0; i < 8; ++i)
        CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Get(actors, 3, &state) && state.hit_points == 0 && state.step >= 8);
    player.attack.active = false;
    for (Uint32 i = 0; i < 70; ++i)
        CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Get(actors, 3, &state) && state.generation == 0);
    CHECK(SotnActors_GetCombat(actors, &combat) && combat.kills == 1 && combat.hearts == 11);
    const SotnActorProgress saved = {23, 3, 777, 9, 123, 456};
    SotnActorProgress progress = {0};
    CHECK(SotnActors_Restore(actors, saved));
    CHECK(SotnActors_GetProgress(actors, &progress));
    CHECK(progress.hearts == 23 && progress.subweapon == 3 && progress.gold == 777 &&
          progress.kills == 9 && progress.random == 123 && progress.combat_random == 456);
    CHECK(!SotnActors_Restore(actors, (SotnActorProgress){.hearts = 51}));
    CHECK(SotnActors_GetProgress(actors, &progress) && progress.gold == 777);
    SotnActors_Destroy(actors);
    SotnActorLayout_Close(layout);
    return true;
}

static bool TestEquipmentPickup(Fixture *f)
{
    const SotnCollisionMap map = MakeFixture(f);
    TestCharacter_Fill(f->dra);
    Put16(f->nz0, 0x30e, 0xc);
    Put16(f->nz0, 0x310, 0);
    Put16(f->nz0, 0x314, 0xffff);
    Put16(f->nz0, 0x316, 0xffff);
    Put16(f->nz0, 0x13b0, 128 + SOTN_HAND_ITEMS + 2);
    Put16(f->dra, 0x20c, 1);
    SotnCharacter *c = NULL;
    SotnActors *actors = NULL;
    SotnActorLayout *layout = NULL;
    const SotnDataView dra = {f->dra, sizeof(f->dra)}, nz0 = {f->nz0, sizeof(f->nz0)};
    CHECK(SotnCharacter_Create(dra, &c));
    CHECK(SotnActors_Create(dra, nz0, &actors));
    CHECK(SotnActors_SetCharacter(actors, c));
    CHECK(SotnActorLayout_Open(nz0, 0, &layout));
    CHECK(SotnActors_Enter(actors, layout, map, 0, 0));
    const SotnPlayerState player = {.x = 40 * 65536,
                                    .y = 135 * 65536,
                                    .hp = 70,
                                    .animation = {.hitbox_width = 8, .hitbox_height = 20}};
    for (Uint32 i = 0; i < 30; ++i)
        CHECK(SotnActors_Tick(actors, player, 0, 0));
    SotnCharacterState state = {0};
    CHECK(SotnCharacter_Get(c, &state, NULL) && state.body_count[2] == 1 &&
          (state.laboratory_items[0] & 1));
    CHECK(SotnActors_Enter(actors, layout, map, 0, 0));
    for (Uint32 i = 0; i < 30; ++i)
        CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnCharacter_Get(c, &state, NULL) && state.body_count[2] == 1);
    SotnActors_Destroy(actors);
    SotnActorLayout_Close(layout);
    SotnCharacter_Destroy(c);
    return true;
}

static bool TestExperienceReward(Fixture *f)
{
    const SotnCollisionMap map = MakeFixture(f);
    TestCharacter_Fill(f->dra);
    TestCharacter_Progression(f->dra);
    Put16(f->nz0, 0x30e, 0x2b);
    Put16(f->nz0, 0x310, 0);
    Put16(f->nz0, 0x314, 0xffff);
    Put16(f->nz0, 0x316, 0xffff);
    Put16(f->dra, 0x204, 1);
    Put16(f->dra, 0x20c, 3);
    Put16(f->dra, 0x216, 1);
    Put16(f->dra, 0x218, 10);
    f->dra[0x222] = 8;
    f->dra[0x223] = 12;
    const Uint8 end_animation[] = {1, 20, 255, 0};
    SDL_memcpy(f->nz0 + 0x2334, end_animation, sizeof(end_animation));
    SDL_memcpy(f->nz0 + 0xf20, end_animation, sizeof(end_animation));
    Put32(f->nz0, 0x1e5c, 0x80180f20);
    SotnCharacter *c = NULL;
    SotnActors *actors = NULL;
    SotnActorLayout *layout = NULL;
    const SotnDataView dra = {f->dra, sizeof(f->dra)}, nz0 = {f->nz0, sizeof(f->nz0)};
    CHECK(SotnCharacter_Create(dra, &c));
    CHECK(SotnActors_Create(dra, nz0, &actors));
    CHECK(SotnActors_SetCharacter(actors, c));
    CHECK(SotnActorLayout_Open(nz0, 0, &layout));
    CHECK(SotnActors_Enter(actors, layout, map, 0, 0));
    const SotnPlayerState player = {.x = 40 * 65536,
                                    .y = 135 * 65536,
                                    .hp = 70,
                                    .attack = {.active = true,
                                               .power = 6,
                                               .hit_type = 2,
                                               .hit_effect = 1,
                                               .element = 0x20,
                                               .width = 32,
                                               .height = 32},
                                    .animation = {.hitbox_width = 8, .hitbox_height = 20}};
    for (Uint32 i = 0; i < 20; ++i)
        CHECK(SotnActors_Tick(actors, player, 0, 0));
    SotnCharacterState state = {0};
    CHECK(SotnCharacter_Get(c, &state, NULL));
    CHECK(state.experience == 10 && state.level == 2 && state.hp_max == 71 && state.mp_max == 24 &&
          state.kills == 1);
    SotnActors_Destroy(actors);
    SotnActorLayout_Close(layout);
    SotnCharacter_Destroy(c);
    return true;
}

static bool TestFailures(Fixture *fixture)
{
    const SotnCollisionMap map = MakeFixture(fixture);
    Put32(fixture->nz0, 0xe04, 0x8018ffff);
    const SotnDataView dra = {fixture->dra, sizeof(fixture->dra)},
                       nz0 = {fixture->nz0, sizeof(fixture->nz0)};
    SotnActors *actors = NULL;
    SotnActorLayout *layout = NULL;
    CHECK(SotnActors_Create(dra, nz0, &actors));
    CHECK(SotnActorLayout_Open(nz0, 0, &layout));
    CHECK(SotnActors_Enter(actors, layout, map, 0, 0));
    CHECK(SotnActors_Tick(actors, (SotnPlayerState){0}, 0, 0));
    CHECK(!SotnActors_Tick(actors, (SotnPlayerState){0}, 0, 0));
    SotnActors_Destroy(actors);
    SotnActorLayout_Close(layout);
    CHECK(!SotnActors_Create((SotnDataView){0}, nz0, &actors) && actors == NULL);
    SotnActors_Destroy(NULL);
    return true;
}

int main(void)
{
    Fixture *fixture = SDL_calloc(1, sizeof(*fixture));
    if (fixture == NULL)
        return 1;
    const bool passed = TestBehaviors(fixture) && TestCombat(fixture) &&
                        TestEquipmentPickup(fixture) && TestExperienceReward(fixture) &&
                        TestFailures(fixture);
    SDL_free(fixture);
    if (!passed)
        return 1;
    SDL_Log("SOTN_ACTORS_TEST: PASS");
    return 0;
}
