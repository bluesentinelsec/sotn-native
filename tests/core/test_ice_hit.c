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
static bool Controller(Uint8 *dra, Uint8 *exe)
{
    SotnIceHit n = {0};
    Uint32 rng = 17, expected = 17;
    SotnIceHitInput in = {.dra = {dra, 0x11000},
                          .executable = {exe, 0x24000},
                          .source = {.x = 12 * 65536 + 123, .y = 34 * 65536 + 456},
                          .player_x = 128 * 65536 + 999,
                          .player_y = 140 * 65536 + 888,
                          .priority = 148,
                          .status = 0x10000,
                          .player_step = 10};
    CHECK(SotnIceHit_Tick(&n, in, &rng) && n.removed && rng == 17 && !n.sound);
    n = (SotnIceHit){0};
    in.allocated = true;
    CHECK(SotnIceHit_Tick(&n, in, &rng) && n.phase == 1 && n.sound == 0x69d &&
          n.flags == 0x00860000U && n.x == 128 * 65536 + 123 && n.y == 140 * 65536 + 456 &&
          n.rotation == 3968);
    for (Uint16 i = 0; i < 96; ++i)
        SotnCombat_Random(&expected);
    CHECK(rng == expected);
    const SotnIceHit before = n;
    CHECK(SotnIceHit_Rebase(&n, -17, 9, false) && !SDL_memcmp(&n, &before, sizeof(n)));
    CHECK(SotnIceHit_Displace(&n, 65536 + 33, -65536 - 22) && n.x == before.x + 65536 + 33 &&
          n.quads[0].x[0] == before.quads[0].x[0]);
    CHECK(SotnIceHit_Tick(&n, in, &rng) && n.x == before.x + 33 && n.y == before.y - 22 &&
          !n.sound && rng == expected);
    const SotnIceHit held = n;
    CHECK(SotnIceHit_Rebase(&n, -17, 9, true) && n.x == held.x - 17 * 65536 &&
          n.quads[0].x[0] == held.quads[0].x[0] - 17 && n.timer == held.timer);
    in.status = 0;
    CHECK(SotnIceHit_Tick(&n, in, &rng) && n.phase == 2 && n.timer == 64 && n.sound == 0x61a);
    for (Uint16 i = 0; i < 24; ++i)
        SotnCombat_Random(&expected);
    CHECK(rng == expected);
    for (Uint16 i = 0; i < 16; ++i)
        CHECK(SotnIceHit_Tick(&n, in, &rng));
    CHECK(n.flags == 0x08800000U && n.quads[0].u[0] == 2 && rng == expected);
    const Sint32 x = n.x;
    CHECK(SotnIceHit_Displace(&n, 65536, 65536) && n.x == x);
    // Hidden rows still update their byte counters; the original inverted color gate wraps.
    n.quads[0].v[0] = 1;
    n.quads[0].red[3] = 2;
    CHECK(SotnIceHit_Tick(&n, in, &rng) && (n.quads[0].draw_mode & 8U) &&
          n.quads[0].red[0] == 254 && n.quads[0].v[0] == 0);
    CHECK(SotnIceHit_Tick(&n, in, &rng) && n.quads[0].v[0] == 255);
    const Sint16 remaining = n.timer;
    for (Sint16 i = 0; i < remaining; ++i)
        CHECK(SotnIceHit_Tick(&n, in, &rng));
    CHECK(n.removed && rng == expected);
    n = (SotnIceHit){0};
    in.status = 0x10000;
    in.velocity_y = -65536;
    in.facing_left = true;
    CHECK(SotnIceHit_Tick(&n, in, &rng) && n.airborne && n.rotation == 256);
    in.player_substep = 5;
    CHECK(SotnIceHit_Tick(&n, in, &rng) && n.phase == 2);
    n = (SotnIceHit){0};
    in.player_step = 16;
    in.contacts = 12;
    CHECK(SotnIceHit_Tick(&n, in, &rng) && n.death && !n.airborne && n.death_timer == 20);
    for (Uint16 i = 0; i < 19; ++i)
        CHECK(SotnIceHit_Tick(&n, in, &rng) && n.phase == 1);
    CHECK(SotnIceHit_Tick(&n, in, &rng) && n.phase == 2 && n.death_timer == 0);
    n = (SotnIceHit){0};
    in.player_step = 10;
    in.velocity_y = 0;
    CHECK(SotnIceHit_Tick(&n, in, &rng));
    TestCharacter_Put32(dra + 0xdcc8 + 71 * 4, 0x800b1000);
    const SotnIceHit saved = n;
    const Uint32 old = rng;
    in.status = 0;
    CHECK(!SotnIceHit_Tick(&n, in, &rng) && rng == old && !SDL_memcmp(&n, &saved, sizeof(n)));
    TestCharacter_Put32(dra + 0xdcc8 + 71 * 4, 0x800b0000);
    return true;
}
static bool Shared(Uint8 *dra, Uint8 *exe)
{
    TestCharacter_Fill(dra);
    Uint8 *bp = dra + 0xd1d4 + 46 * 6;
    bp[0] = 33;
    bp[1] = 1;
    bp[2] = 0xc1;
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r)), *saved = SDL_malloc(sizeof(*saved));
    SotnActors *a = SDL_calloc(1, sizeof(*a));
    SotnCharacter *character = NULL;
    CHECK(r && saved && a && SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    a->primitives[0].owner = 77;
    SotnCharacterState before = {0}, after = {0};
    CHECK(SotnCharacter_Get(character, &before, NULL));
    Uint32 expected = before.random;
    SotnBatRuntimeInput in = {.dra = {dra, 0x11000},
                              .executable = {exe, 0x24000},
                              .player = {.x = 128 * 65536 + 123,
                                         .y = 140 * 65536 + 456,
                                         .hp = 70,
                                         .hp_max = 70,
                                         .mp = 20,
                                         .priority = 148,
                                         .step = SOTN_PLAYER_HIT,
                                         .status = 0x10000,
                                         .hurt = {.count = 1, .started = true}}};
    in.player.hurt.factories[0].blueprint = 46;
    in.player.hurt.factories[0].source =
        (SotnPlayerFactorySource){.x = in.player.x, .y = in.player.y, .priority = 148};
    CHECK(SotnActors_PrimitiveSlots(a, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(a, r) &&
          r->sound_count == 1 && r->sounds[0] == 0x69d && r->children[47].kind == 33 &&
          r->children[47].primitive_count == 24);
    for (Uint16 i = 0; i < 96; ++i)
        SotnCombat_Random(&expected);
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    in.player.hurt.count = 0;
    in.player.hurt.started = false;
    in.freeze = 3;
    *saved = *r;
    CHECK(SotnBatRuntime_Tick(r, character, in) &&
          !SDL_memcmp(r->children, saved->children, sizeof(r->children)));
    CHECK(SotnBatRuntime_PruneIce(r) &&
          !SDL_memcmp(r->children, saved->children, sizeof(r->children)));
    in.entering = true;
    in.screen_shift_x = in.room_shift_x = 11;
    CHECK(SotnActors_PrimitiveSlots(a, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(a, r) &&
          r->children[47].state.ice.x == saved->children[47].state.ice.x + 11 * 65536 &&
          r->children[47].state.ice.quads[0].x[0] ==
              saved->children[47].state.ice.quads[0].x[0] + 11);
    in.entering = false;
    in.freeze = 0;
    in.screen_shift_x = in.room_shift_x = 0;
    // Malformed retained ownership fails without changing runtime or character RNG.
    r->children[47].state.ice.quads[0].u[0] = 3;
    *saved = *r;
    in.player.hurt.count = 1;
    in.player.hurt.started = true;
    CHECK(!SotnBatRuntime_Tick(r, character, in) && !SDL_memcmp(r, saved, sizeof(*r)) &&
          SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    r->children[47].state.ice.quads[0].u[0] = 0;
    in.player.hurt.count = 0;
    in.player.hurt.started = false;
    in.player.status = 0;
    for (Uint16 i = 0; i < 16; ++i)
        CHECK(SotnActors_PrimitiveSlots(a, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(a, r));
    CHECK(r->children[47].state.ice.flags == 0x08800000U);
    CHECK(SotnCharacter_Get(character, &before, NULL));
    in.entering = true;
    CHECK(SotnActors_PrimitiveSlots(a, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && !r->children[47].kind &&
          SotnCharacter_Get(character, &after, NULL) && after.random == before.random &&
          SotnActors_CommitBatPrimitives(a, r) && a->primitives[0].owner == 77 &&
          !a->primitives[1].owner);
    in.entering = false;
    // An actual later thunder sibling fails after the new ice birth consumed local RNG/rows.
    Uint8 *thunder = dra + 0xd1d4 + 45 * 6;
    thunder[0] = 30;
    thunder[1] = 4;
    thunder[2] = 0xc4;
    in.player.status = 0x10000;
    in.player.hurt.count = 1;
    in.player.hurt.started = true;
    in.player.hurt.factories[0].blueprint = 45;
    CHECK(SotnActors_PrimitiveSlots(a, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(a, r));
    CHECK(r->children[47].kind == 30 && r->children[44].kind == 30);
    r->children[47].state.thunder.quads[0].palette = 768;
    *saved = *r;
    CHECK(SotnCharacter_Get(character, &before, NULL));
    in.player.hurt.factories[0].blueprint = 46;
    CHECK(SotnActors_PrimitiveSlots(a, &in.primitive_slots) &&
          !SotnBatRuntime_Tick(r, character, in) && !SDL_memcmp(r, saved, sizeof(*r)) &&
          SotnCharacter_Get(character, &after, NULL) && after.random == before.random);
    SDL_zero(*r);
    CHECK(SotnActors_CommitBatPrimitives(a, r) && a->primitives[0].owner == 77);
    // Actual twenty-three-row exhaustion consumes no character RNG or sound.
    in.player.status = 0x10000;
    in.player.hurt.count = 1;
    in.player.hurt.started = true;
    CHECK(SotnCharacter_Get(character, &before, NULL));
    SDL_memset(&in.primitive_slots, 255, sizeof(in.primitive_slots));
    for (Uint16 row = 1; row <= 23; ++row)
        in.primitive_slots.occupied[row / 64U] &= ~((Uint64)1 << (row % 64U));
    CHECK(SotnBatRuntime_Tick(r, character, in) && !r->children[47].kind && !r->sound_count &&
          SotnCharacter_Get(character, &after, NULL) && after.random == before.random);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(saved);
    SDL_free(a);
    return true;
}
int main(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *exe = SDL_calloc(1, 0x24000);
    if (!dra || !exe)
        return 1;
    // Authored point and SDK fixtures; original private geometry is compared separately.
    for (Uint16 i = 0; i < 72; ++i)
        TestCharacter_Put32(dra + 0xdcc8 + i * 4, 0x800b0000);
    for (Uint16 i = 0; i <= 1024; ++i)
        Put16(exe + 0x1dbcc + i * 2U, i * 4U);
    const bool ok = Controller(dra, exe) && Shared(dra, exe);
    SDL_free(dra);
    SDL_free(exe);
    return ok ? 0 : 1;
}
