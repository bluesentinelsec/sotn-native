#include "actors_internal.h"
#include "character_fixture.h"
#include <sotn/bat_runtime.h>
#include <sotn/combat.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static void Animation(Uint8 *dra)
{
    for (Uint16 i = 0; i < 13; ++i)
    {
        Put16(dra + 0xdc44 + i * 4U, 2);
        Put16(dra + 0xdc46 + i * 4U, (Uint16)(i + 1));
    }
    Put16(dra + 0xdc78, 0xffff);
}
static bool Controller(Uint8 *dra)
{
    SotnDarkHit d = {0};
    Uint32 rng = 17, expected = rng, phase = SDL_MAX_UINT32;
    SotnPlayerFactorySource source = {.x = 32760 * 65536 + 123, .y = -32760 * 65536 + 456};
    const Sint16 dy = (Sint16)((Sint32)(SotnCombat_Random(&expected) % 35U) - 15);
    const Sint16 dx = (Sint16)((Sint32)(SotnCombat_Random(&expected) % 20U) - 10);
    const Sint32 vy = -24576 - (Sint32)(SotnCombat_Random(&expected) & 0x3fffU);
    CHECK(SotnDarkHit_Tick(&d, source, 148, &phase, &rng) && rng == expected && !phase &&
          d.blend_mode == 0x70 && d.scale_x == 64 && d.opacity == 255 && d.velocity_y == vy &&
          (Uint32)d.x == (Uint32)source.x + (Uint32)(Sint32)dx * 65536U &&
          (Uint32)d.y == (Uint32)source.y + (Uint32)(Sint32)dy * 65536U);
    CHECK(SotnDarkHit_Animate(&d, (SotnDataView){dra, 0x11000}) && d.frame == 1 &&
          d.remaining == 2);
    const SotnDarkHit born = d;
    CHECK(SotnDarkHit_Rebase(&d, -17, 9) && d.pose == born.pose && d.opacity == born.opacity &&
          (Uint32)d.x == (Uint32)born.x - 17U * 65536U);
    for (Uint16 t = 1; t <= 26; ++t)
    {
        CHECK(SotnDarkHit_Tick(&d, source, 148, &phase, &rng) && !d.removed &&
              d.scale_x == 64 + t * 8 && d.opacity == 255 - t * 8 && rng == expected);
        CHECK(SotnDarkHit_Animate(&d, (SotnDataView){dra, 0x11000}));
    }
    CHECK(d.remaining == -1 && d.pose == 12 && d.frame == 13);
    CHECK(SotnDarkHit_Tick(&d, source, 148, &phase, &rng) && d.removed && d.scale_x == 280 &&
          rng == expected);
    d = (SotnDarkHit){0};
    CHECK(SotnDarkHit_Tick(&d, source, 148, &phase, &rng) && phase == 1 && d.blend_mode == 0x10);
    const SotnDarkHit before = d;
    Put16(dra + 0xdc44, 0);
    CHECK(!SotnDarkHit_Animate(&d, (SotnDataView){dra, 0x11000}) &&
          !SDL_memcmp(&d, &before, sizeof(d)));
    Animation(dra);
    return true;
}
static bool Shared(Uint8 *dra)
{
    TestCharacter_Fill(dra);
    Animation(dra);
    // Authored three-child schedule, distinct from the private original blueprint.
    Uint8 *bp = dra + 0xd1d4 + 113 * 6;
    bp[0] = 54;
    bp[1] = 3;
    bp[2] = 0xc1;
    bp[3] = 4;
    bp[4] = 0x70;
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    SotnCharacter *character = NULL;
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r)), *saved = SDL_malloc(sizeof(*saved));
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    CHECK(r && saved && actors && SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    actors->primitives[0].owner = 77;
    SotnCharacterState before = {0}, after = {0};
    CHECK(SotnCharacter_Get(character, &before, NULL));
    Uint32 expected = before.random;
    SotnBatRuntimeInput in = {.dra = {dra, 0x11000},
                              .camera_x = 20,
                              .screen_shift_x = -3,
                              .player = {.x = 100 * 65536 + 123,
                                         .y = 140 * 65536 + 456,
                                         .hp = 70,
                                         .hp_max = 70,
                                         .mp = 20,
                                         .priority = 148,
                                         .step = SOTN_PLAYER_HIT,
                                         .status = 0x10000,
                                         .animation = {.sprite = 1},
                                         .hurt = {.count = 1, .started = true}}};
    in.player.hurt.factories[0].blueprint = 113;
    in.player.hurt.factories[0].source =
        (SotnPlayerFactorySource){.x = in.player.x, .y = in.player.y, .priority = 148};
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          r->children[47].kind == 54 && r->dark_blend_phase == 1 &&
          r->children[47].state.dark.frame == 1 && !r->children[47].primitive_count);
    const Sint16 dy = (Sint16)((Sint32)(SotnCombat_Random(&expected) % 35U) - 15);
    const Sint16 dx = (Sint16)((Sint32)(SotnCombat_Random(&expected) % 20U) - 10);
    SotnCombat_Random(&expected);
    CHECK(r->children[47].state.dark.x == (80 + dx) * 65536 + 123 &&
          r->children[47].state.dark.y == (140 + dy) * 65536 + 456);
    in.player.hurt.count = 0;
    in.player.hurt.started = false;
    in.screen_shift_x = 0;
    in.player.x += 7 * 65536;
    const SotnDarkHit born = r->children[47].state.dark;
    CHECK(SotnBatRuntime_Tick(r, character, in) && r->children[47].state.dark.x == born.x &&
          r->children[47].state.dark.y == born.y + born.velocity_y &&
          r->children[47].state.dark.scale_x == 72);
    in.freeze = 3;
    const SotnDarkHit held = r->children[47].state.dark;
    *saved = *r;
    for (Uint16 i = 0; i < 5; ++i)
        CHECK(SotnBatRuntime_Tick(r, character, in) &&
              !SDL_memcmp(&r->children[47].state.dark, &held, sizeof(held)) &&
              !SDL_memcmp(r->factories, saved->factories, sizeof(r->factories)) &&
              r->dark_blend_phase == 1);
    in.entering = true;
    in.room_shift_x = in.screen_shift_x = 11;
    CHECK(SotnBatRuntime_Tick(r, character, in) &&
          r->children[47].state.dark.x == held.x + 11 * 65536 &&
          r->children[47].state.dark.remaining == held.remaining && r->dark_blend_phase == 1);
    in.entering = false;
    in.freeze = 0;
    in.room_shift_x = in.screen_shift_x = 0;
    const Sint16 valid = r->children[47].state.dark.remaining;
    r->children[47].state.dark.remaining = -2;
    *saved = *r;
    CHECK(!SotnBatRuntime_Tick(r, character, in) && !SDL_memcmp(r, saved, sizeof(*r)) &&
          SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    r->children[47].state.dark.remaining = valid;
    // Later old-child animation failure must undo a lower-slot sibling birth/global/RNG.
    CHECK(SotnBatRuntime_Tick(r, character, in) && SotnBatRuntime_Tick(r, character, in));
    CHECK(r->children[47].state.dark.pose == 1 && r->children[47].state.dark.remaining == 1);
    Put16(dra + 0xdc4e, 0);
    *saved = *r;
    CHECK(!SotnBatRuntime_Tick(r, character, in) && !SDL_memcmp(r, saved, sizeof(*r)) &&
          SotnCharacter_Get(character, &after, NULL) && after.random == expected &&
          r->dark_blend_phase == 1);
    Put16(dra + 0xdc4e, 3);
    for (Uint16 t = 0; t < 60; ++t)
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    for (Uint16 i = 0; i < 6; ++i)
        SotnCombat_Random(&expected);
    CHECK(r->dark_blend_phase == 3 && !SotnBatRuntime_Occupied(r) &&
          SotnCharacter_Get(character, &after, NULL) && after.random == expected &&
          after.hp == before.hp && after.mp == before.mp && actors->primitives[0].owner == 77);
    // Optional real-slot exhaustion retires its noncritical root before RNG/phase.
    in.other_occupied = 0xffffffff00000000ULL;
    in.player.hurt.count = 1;
    in.player.hurt.started = true;
    CHECK(SotnBatRuntime_Tick(r, character, in) && r->dark_blend_phase == 3 &&
          !SotnBatRuntime_Occupied(r) && SotnCharacter_Get(character, &after, NULL) &&
          after.random == expected);
    in.other_occupied = 0;
    bp[1] = 1;
    in.player.x = -100 * 65536;
    CHECK(SotnBatRuntime_Tick(r, character, in) && !SotnBatRuntime_Occupied(r) &&
          r->dark_blend_phase == 4);
    for (Uint16 i = 0; i < 3; ++i)
        SotnCombat_Random(&expected);
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(saved);
    SDL_free(actors);
    return true;
}
int main(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    if (!dra)
        return 1;
    Animation(dra);
    bool ok = Controller(dra) && Shared(dra);
    SDL_free(dra);
    return ok ? 0 : 1;
}
