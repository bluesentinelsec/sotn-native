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
    SotnThunderHit t = {0};
    Uint32 rng = 17, expected = 17;
    SotnThunderHitInput in = {.dra = {dra, 0x11000},
                              .executable = {exe, 0x24000},
                              .source = {.x = 128 * 65536 + 123, .y = 140 * 65536 + 456},
                              .player_x = 135 * 65536 + 789,
                              .player_y = 129 * 65536 + 987,
                              .priority = 148,
                              .player_step = 10};
    CHECK(SotnThunderHit_Tick(&t, in, &rng) && t.removed && rng == 17 && !t.sound_count);
    t = (SotnThunderHit){0};
    in.allocated = true;
    in.player_velocity_y = -65536;
    CHECK(SotnThunderHit_Tick(&t, in, &rng) && t.phase == 1 && t.airborne && t.thickness == 16 &&
          t.sound_count == 2 && t.sounds[0] == 0x69d && t.sounds[1] == 0x665 &&
          t.x == in.source.x && t.y == in.source.y);
    for (Uint16 i = 0; i < 11; ++i)
        SotnCombat_Random(&expected);
    CHECK(rng == expected);
    const SotnPlayerEffectQuad history = t.quads[1];
    in.contacts = 8;
    CHECK(SotnThunderHit_Tick(&t, in, &rng) && t.phase == 2 && t.fade >= 16 && t.fade <= 31 &&
          !t.sound_count && t.thickness == 16 &&
          !SDL_memcmp(t.quads[0].x, history.x, sizeof(history.x)) &&
          !SDL_memcmp(t.quads[0].y, history.y, sizeof(history.y)) &&
          t.quads[0].palette == history.palette);
    const SotnThunderHit before = t;
    CHECK(SotnThunderHit_Rebase(&t, -17, 9, false) && t.x == before.x - 17 * 65536 &&
          t.quads[0].x[0] == before.quads[0].x[0] - 17 && t.wave == before.wave &&
          t.fade == before.fade);
    const Uint16 count = (Uint16)t.fade;
    Uint16 updates = 0;
    while (!t.removed && updates < 40)
    {
        CHECK(SotnThunderHit_Tick(&t, in, &rng));
        ++updates;
    }
    CHECK(t.removed && updates == count);
    t = (SotnThunderHit){0};
    in.player_step = 16;
    in.contacts = 0;
    CHECK(SotnThunderHit_Tick(&t, in, &rng) && !t.airborne);
    in.player_step = 10;
    in.contacts = 14;
    CHECK(SotnThunderHit_Tick(&t, in, &rng) && t.phase == 1);
    t = (SotnThunderHit){0};
    in.parameter = 0x100;
    in.player_step = 0;
    in.player_velocity_y = 0;
    in.contacts = 0;
    for (Uint16 age = 1; age <= 169; ++age)
    {
        CHECK(SotnThunderHit_Tick(&t, in, &rng) && t.age == (Sint16)age &&
              t.phase == (age == 169 ? 2 : 1));
    }
    const SotnThunderHit saved = t;
    Uint32 old = rng;
    t.quads[0].palette = 768;
    const SotnThunderHit malformed = t;
    CHECK(!SotnThunderHit_Tick(&t, in, &rng) && rng == old &&
          !SDL_memcmp(&t, &malformed, sizeof(t)));
    t = saved;
    return true;
}
static bool Shared(Uint8 *dra, Uint8 *exe)
{
    TestCharacter_Fill(dra);
    Uint8 *bp = dra + 0xd1d4 + 45 * 6;
    bp[0] = 30;
    bp[1] = 4;
    bp[2] = 0xc4;
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
    in.player.hurt.factories[0].blueprint = 45;
    in.player.hurt.factories[0].source =
        (SotnPlayerFactorySource){.x = in.player.x, .y = in.player.y, .priority = 148};
    CHECK(SotnActors_PrimitiveSlots(a, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(a, r) &&
          r->sound_count == 8);
    for (Uint16 child = 44; child < 48; ++child)
    {
        const Uint16 parameter = (Uint16)(47 - child);
        Uint32 first = SotnCombat_Random(&expected);
        for (Uint16 k = 1; k < 11; ++k)
            SotnCombat_Random(&expected);
        CHECK(r->children[child].kind == 30 && r->children[child].parameter == parameter &&
              r->children[child].primitive_count == 6 &&
              r->children[child].state.thunder.angle ==
                  (Sint16)((parameter << 9) + (first & 63U)) &&
              r->sounds[(child - 44) * 2] == 0x69d && r->sounds[(child - 44) * 2 + 1] == 0x665);
    }
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected &&
          after.hp == before.hp && after.mp == before.mp);
    in.player.hurt.count = 0;
    in.player.hurt.started = false;
    in.freeze = 3;
    *saved = *r;
    for (Uint16 k = 0; k < 3; ++k)
        CHECK(SotnBatRuntime_Tick(r, character, in) &&
              !SDL_memcmp(r->children, saved->children, sizeof(r->children)) && !r->sound_count);
    in.entering = true;
    in.screen_shift_x = in.room_shift_x = 11;
    in.player.x += 11 * 65536;
    CHECK(SotnBatRuntime_Tick(r, character, in) &&
          r->children[47].state.thunder.x == saved->children[47].state.thunder.x + 11 * 65536 &&
          r->children[47].state.thunder.quads[0].x[0] ==
              saved->children[47].state.thunder.quads[0].x[0] + 11 &&
          r->children[47].state.thunder.wave == saved->children[47].state.thunder.wave);
    in.entering = false;
    in.freeze = 0;
    in.screen_shift_x = in.room_shift_x = 0;
    // A later old child's invalid GT4 must undo an earlier four-child birth/RNG/row plan.
    in.player.hurt.count = 1;
    in.player.hurt.started = true;
    const Uint16 palette = r->children[47].state.thunder.quads[0].palette;
    r->children[47].state.thunder.quads[0].palette = 768;
    *saved = *r;
    CHECK(!SotnBatRuntime_Tick(r, character, in) && !SDL_memcmp(r, saved, sizeof(*r)) &&
          SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    r->children[47].state.thunder.quads[0].palette = palette;
    in.player.hurt.count = 0;
    in.player.hurt.started = false;
    in.player.step = SOTN_PLAYER_STAND;
    in.player.status = 0;
    for (Uint16 k = 0; k < 70; ++k)
        CHECK(SotnActors_PrimitiveSlots(a, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(a, r));
    CHECK(!SotnBatRuntime_Occupied(r) && a->primitives[0].owner == 77);
    // Exactly three six-row births fit; the final sibling retires before its eleven RNG calls.
    CHECK(SotnCharacter_Get(character, &before, NULL));
    expected = before.random;
    in.player.step = SOTN_PLAYER_HIT;
    in.player.status = 0x10000;
    in.player.hurt.count = 1;
    in.player.hurt.started = true;
    SDL_memset(&in.primitive_slots, 255, sizeof(in.primitive_slots));
    for (Uint16 row = 1; row <= 18; ++row)
        in.primitive_slots.occupied[row / 64U] &= ~((Uint64)1 << (row % 64U));
    CHECK(SotnBatRuntime_Tick(r, character, in) && !r->children[47].kind && r->sound_count == 6);
    for (Uint16 k = 0; k < 33; ++k)
        SotnCombat_Random(&expected);
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    for (Uint16 child = 44; child < 47; ++child)
        CHECK(r->children[child].kind == 30 && r->children[child].primitive_count == 6);
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
    Put16(dra + 0xdc7c, 17);
    Put16(dra + 0xdc7e, 18);
    for (Uint16 i = 0; i <= 1024; ++i)
        Put16(exe + 0x1dbcc + i * 2U, i * 4U);
    bool ok = Controller(dra, exe) && Shared(dra, exe);
    SDL_free(dra);
    SDL_free(exe);
    return ok ? 0 : 1;
}
