#include "actors_internal.h"
#include "character_fixture.h"
#include <sotn/bat_runtime.h>
#include <sotn/player_circle.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Sint32 v)
{
    const Uint16 bits = (Uint16)(Uint32)v;
    p[0] = (Uint8)bits;
    p[1] = (Uint8)(bits >> 8);
}
static bool Controllers(Uint8 *dra, Uint8 *exe)
{
    SotnPlayerCircleInput in = {.dra = {dra, 0x11000},
                                .executable = {exe, 0x24000},
                                .player_x = 100 * 65536 + 900,
                                .player_y = -10 * 65536 + 800,
                                .priority = 148,
                                .allocated = true};
    for (Uint16 mode = 0; mode < 2; ++mode)
    {
        SotnPlayerCircle n = {.parameter = (Uint16)(0x8000U | mode * 0x4000U), .x = 123, .y = 456};
        SotnPlayerCircleEvents e = {0};
        CHECK(SotnPlayerCircle_Tick(&n, in, &e) && n.phase == 1 && !n.removed &&
              e.reset_afterimages && n.x == 100 * 65536 + 123 && n.y == -10 * 65536 + 456 &&
              n.alpha == 192 && n.quads[31].priority == 150 && n.quads[31].draw_mode == 0x235 &&
              n.quads[31].page == 0x1a && n.quads[31].palette == 0x15f);
        CHECK(n.outer_x == (mode ? 64 : 640) && n.inner_x == (mode ? 0 : 576));
        const SotnPlayerCircle birth = n;
        CHECK(SotnPlayerCircle_Rebase(&n, 17, -3, false) && !SDL_memcmp(&birth, &n, sizeof(n)));
        CHECK(SotnPlayerCircle_Rebase(&n, 17, -3, true) && n.x == birth.x + 17 * 65536 &&
              n.y == birth.y - 3 * 65536 && !SDL_memcmp(n.quads, birth.quads, sizeof(n.quads)) &&
              n.angle == birth.angle && n.alpha == birth.alpha);
        CHECK(SotnPlayerCircle_Displace(&n, 123, -456) && n.x == birth.x + 17 * 65536 + 123 &&
              n.y == birth.y - 3 * 65536 - 456 &&
              !SDL_memcmp(n.quads, birth.quads, sizeof(n.quads)));
        Uint16 ticks = 1;
        while (!n.removed && ticks < 120)
        {
            const SotnPlayerCircle before = n;
            CHECK(SotnPlayerCircle_Tick(&n, in, &e) && e.reset_afterimages);
            ++ticks;
            if (n.removed)
                CHECK(!SDL_memcmp(n.quads, before.quads, sizeof(n.quads)) && n.alpha < 0);
        }
        CHECK(n.removed && ticks == (mode ? 107 : 108));
    }
    SotnPlayerCircle n = {0};
    SotnPlayerCircleEvents e = {0};
    in.allocated = false;
    in.executable = (SotnDataView){0};
    CHECK(SotnPlayerCircle_Tick(&n, in, &e) && n.removed && e.reset_afterimages && !n.phase);
    n = (SotnPlayerCircle){0};
    const SotnPlayerCircle fresh = n;
    const SotnPlayerCircleEvents events = e;
    in.allocated = true;
    CHECK(!SotnPlayerCircle_Tick(&n, in, &e) && !SDL_memcmp(&n, &fresh, sizeof(n)) &&
          !SDL_memcmp(&events, &e, sizeof(e)));
    in.executable = (SotnDataView){exe, 0x24000};
    CHECK(SotnPlayerCircle_Tick(&n, in, &e));
    n.angle = 32760;
    CHECK(SotnPlayerCircle_Tick(&n, in, &e) && n.angle == -32712);
    const SotnPlayerCircle held = n;
    Put16(exe + 0x1dbcc + 256 * 2, 4097);
    CHECK(!SotnPlayerCircle_Tick(&n, in, &e) && !SDL_memcmp(&held, &n, sizeof(n)));
    Put16(exe + 0x1dbcc + 256 * 2, 1024);
    return true;
}
static bool Shared(Uint8 *dra, Uint8 *exe)
{
    CHECK(!SotnBatRuntime_CircleValid(NULL) && !SotnBatRuntime_MedicineValid(NULL));
    TestCharacter_Fill(dra);
    Uint8 *bp = dra + 0xd1d4 + 53 * 6;
    bp[0] = 38;
    bp[1] = 1;
    bp[2] = 0xc1;
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r)), *saved = SDL_malloc(sizeof(*saved));
    SotnActors *a = SDL_calloc(1, sizeof(*a));
    SotnCharacter *character = NULL;
    CHECK(r && saved && a && SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    SotnCharacterState before = {0}, after = {0};
    CHECK(SotnCharacter_Get(character, &before, NULL));
    SotnBatRuntimeInput in = {.dra = {dra, 0x11000},
                              .executable = {exe, 0x24000},
                              .player = {.x = 100 * 65536,
                                         .y = 140 * 65536,
                                         .hp = 70,
                                         .hp_max = 70,
                                         .mp = 20,
                                         .priority = 148,
                                         .animation = {.id = 1, .sprite = 1}}};
    const SotnPlayerFactorySource source = {.x = 123, .y = 456, .priority = 148};
    CHECK(SotnPlayerFactory_Begin(&r->factories[0].cursor, source, 53, 0));
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        a->primitives[i].owner = 99;
    CHECK(SotnActors_PrimitiveSlots(a, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(a, r) &&
          !r->children[47].kind && r->afterimage_reset &&
          SotnCharacter_Get(character, &after, NULL) && after.random == before.random);
    for (Uint16 i = 1; i < SOTN_PRIMITIVE_SLOTS; ++i)
        a->primitives[i].owner = 0;
    CHECK(SotnPlayerFactory_Begin(&r->factories[0].cursor, source, 53, 0) &&
          SotnActors_PrimitiveSlots(a, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(a, r) &&
          r->children[47].kind == 38 && r->children[47].primitive_count == 32 &&
          r->children[47].state.circle.phase == 1 && r->afterimage_reset);
    for (Uint16 i = 0; i < 32; ++i)
        CHECK(a->primitives[r->children[47].primitives[i]].ability_kind == 38 &&
              r->children[47].primitives[i] != 0);
    in.freeze = 3;
    CHECK(SotnActors_PrimitiveSlots(a, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && r->children[47].state.circle.angle == 64);
    const SotnPlayerCircle held = r->children[47].state.circle;
    in.entering = true;
    in.screen_shift_x = 17;
    in.screen_shift_y = -3;
    CHECK(SotnActors_PrimitiveSlots(a, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(a, r) &&
          r->children[47].state.circle.angle == held.angle &&
          r->children[47].state.circle.x == held.x + 17 * 65536 &&
          !SDL_memcmp(r->children[47].state.circle.quads, held.quads, sizeof(held.quads)));
    CHECK(SotnBatRuntime_PlayerShove(r, 123, -456) &&
          r->children[47].state.circle.x == held.x + 17 * 65536 + 123);
    in.entering = false;
    in.screen_shift_x = in.screen_shift_y = 0;
    r->serial = 3;
    r->children[46] = (SotnBatRuntimeChild){.kind = 38, .generation = 3};
    const Uint32 bad_profile = 0xdde8 + 12;
    r->children[47].parameter = r->children[47].state.circle.parameter = 0x100;
    Put16(dra + bad_profile + 4, 0);
    *saved = *r;
    CHECK(SotnActors_PrimitiveSlots(a, &in.primitive_slots) &&
          !SotnBatRuntime_Tick(r, character, in) && !SDL_memcmp(saved, r, sizeof(*r)) &&
          SotnCharacter_Get(character, &after, NULL) && after.random == before.random);
    Put16(dra + bad_profile + 4, 256);
    SDL_zero(r->children[46]);
    Uint16 ticks = 0;
    while (r->children[47].kind && ticks++ < 120)
        CHECK(SotnActors_PrimitiveSlots(a, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(a, r));
    CHECK(ticks < 120 && a->primitives[0].owner == 99 &&
          SotnCharacter_Get(character, &after, NULL) && after.random == before.random);
    for (Uint16 i = 1; i < SOTN_PRIMITIVE_SLOTS; ++i)
        CHECK(!a->primitives[i].owner);
    SotnCharacter_Destroy(character);
    SDL_free(a);
    SDL_free(saved);
    SDL_free(r);
    return true;
}
int main(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *exe = SDL_calloc(1, 0x24000);
    if (!dra || !exe)
        return 1;
    for (Uint16 i = 0; i <= 1024; ++i)
        Put16(exe + 0x1dbcc + i * 2U, i * 4U);
    for (Uint16 i = 0; i < 22; ++i)
    {
        Uint8 *p = dra + 0xdde8 + i * 12U;
        Put16(p, 17);
        Put16(p + 2, -75);
        Put16(p + 4, 256);
        Put16(p + 6, 512);
        Put16(p + 8, 1024);
        Put16(p + 10, 768);
    }
    const bool ok = Controllers(dra, exe) && Shared(dra, exe);
    SDL_free(exe);
    SDL_free(dra);
    return ok ? 0 : 1;
}
