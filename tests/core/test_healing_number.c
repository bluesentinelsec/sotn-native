#include "actors_internal.h"
#include "character_fixture.h"
#include <sotn/bat_runtime.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static bool Motion(SotnDataView exe)
{
    const Uint16 amounts[] = {1, 8, 10, 100, 1000, 9999};
    const Uint8 counts[] = {0, 0, 1, 2, 3, 3};
    for (Uint16 i = 0; i < SDL_arraysize(amounts); ++i)
    {
        SotnHealingNumber n = {.number = amounts[i]};
        CHECK(SotnHealingNumber_Tick(
            &n, (SotnDataView){0},
            (SotnPlayerFactorySource){.x = 100 * 65536 + 123, .y = 140 * 65536 + 456}, true));
        CHECK(n.extra_digits == counts[i] && n.phase == 1 && n.timer == 8 && n.half_width == 0 &&
              n.half_height == 2 && n.digits[0].palette == 0x1b8 && n.digits[0].priority == 0x1b8 &&
              n.digits[0].page == 0x1a);
        CHECK(n.digits[0].u[0] == 32 + (amounts[i] % 10 ? amounts[i] % 10 - 1 : 9) * 8);
        for (Uint16 d = 0; d < 4; ++d)
            CHECK(((n.digits[d].draw_mode & 8U) != 0) == (d > counts[i]));
        Uint16 ticks = 0, births = 0;
        while (!n.removed && ticks < 160)
        {
            const Uint8 phase = n.phase;
            const Sint16 timer = n.timer;
            const SotnHealingNumber before = n;
            if (phase == 5 && timer == 1)
                CHECK(!SotnHealingNumber_Tick(&n, (SotnDataView){0}, (SotnPlayerFactorySource){0},
                                              true) &&
                      !SDL_memcmp(&n, &before, sizeof(n)));
            CHECK(SotnHealingNumber_Tick(&n, exe, (SotnPlayerFactorySource){0}, true));
            CHECK(((Uint32)n.x & 65535U) == 123 && ((Uint32)n.y & 65535U) == 456);
            for (Uint16 t = 0; t < 16; ++t)
            {
                births += !before.tiles[t].started && n.tiles[t].started;
                if (!before.tiles[t].started && n.tiles[t].started)
                    CHECK(phase == 6 && timer == (Sint16)(t + 1) && n.tiles[t].timer == 12 &&
                          !n.tiles[t].hidden);
                if (before.tiles[t].started && before.tiles[t].timer == 1 && !n.removed)
                    CHECK(n.tiles[t].hidden && !n.tiles[t].timer);
            }
            if (phase == 1 && timer == 1)
                CHECK(n.phase == 2 && n.half_width == 8 && n.half_height == 10 &&
                      n.y == 132 * 65536 + 456);
            if (phase == 6 && timer == 1)
                CHECK(n.phase == 7 && n.x == 21 * 65536 + 123 && n.y == 70 * 65536 + 456);
            ++ticks;
        }
        CHECK(n.removed && ticks == 108 && births == 16 && n.phase == 11 && !n.half_height);
        CHECK(!SotnHealingNumber_Tick(&n, exe, (SotnPlayerFactorySource){0}, true));
    }
    SotnHealingNumber n = {.number = 8};
    CHECK(SotnHealingNumber_Tick(&n, (SotnDataView){0}, (SotnPlayerFactorySource){0}, false) &&
          n.removed && !n.phase);
    n = (SotnHealingNumber){.number = 10000};
    CHECK(!SotnHealingNumber_Tick(&n, exe, (SotnPlayerFactorySource){0}, true) && !n.phase);
    n = (SotnHealingNumber){.number = 16};
    CHECK(SotnHealingNumber_Tick(
        &n, exe, (SotnPlayerFactorySource){.x = SDL_MAX_SINT32, .y = SDL_MIN_SINT32 + 456}, true));
    const SotnHealingNumber before = n;
    CHECK(SotnHealingNumber_Rebase(&n, 1, -1) && n.x == SDL_MIN_SINT32 + 65535 &&
          n.y == SDL_MAX_SINT32 - 65535 + 456 && n.phase == before.phase &&
          n.timer == before.timer && n.digits[0].x[0] == SDL_MIN_SINT16 &&
          n.digits[0].y[0] == before.digits[0].y[0] - 1);
    return true;
}
static bool Runtime(SotnDataView exe)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    CHECK(dra && r && actors);
    TestCharacter_Fill(dra);
    SotnCharacter *c = NULL;
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &c));
    SotnBatRuntimeInput in = {
        .dra = {dra, 0x11000},
        .executable = exe,
        .player = {.priority = 148,
                   .healing = {.applied = true,
                               .applied_kind = 2,
                               .applied_amount = 1,
                               .source = {.x = 150 * 65536 + 123, .y = 140 * 65536 + 456}}}};
    in.other_occupied = (Uint64)1 << 56;
    CHECK(SotnBatRuntime_Tick(r, c, in) && r->children[41].kind == 19 &&
          r->children[41].initialized && r->children[41].primitive_count == 20 &&
          r->children[41].state.healing_number.number == 1 && !r->sound_count);
    CHECK(SotnActors_CommitBatPrimitives(actors, r));
    for (Uint16 i = 0; i < 20; ++i)
        CHECK(actors->primitives[r->children[41].primitives[i]].ability_kind == 19 &&
              actors->primitives[r->children[41].primitives[i]].owner ==
                  r->children[41].generation);
    // The same real slots are visible to later requests; independent heal kinds share this visual.
    in.player.healing.applied_kind = 1;
    in.player.healing.applied_amount = 16;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) && SotnBatRuntime_Tick(r, c, in) &&
          r->children[42].kind == 19 && r->children[42].state.healing_number.number == 16 &&
          r->sound_count == 1 && r->sounds[0] == 0x68e);
    CHECK(SotnActors_CommitBatPrimitives(actors, r));
    in.player.healing.applied = false;
    in.freeze = 3;
    const Sint16 held_timer = r->children[42].state.healing_number.timer;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) && SotnBatRuntime_Tick(r, c, in) &&
          r->children[42].state.healing_number.timer == held_timer - 1 && !r->sound_count);
    CHECK(SotnActors_CommitBatPrimitives(actors, r));
    const SotnHealingNumber before = r->children[42].state.healing_number;
    in.entering = true;
    in.screen_shift_x = -16;
    in.screen_shift_y = 2;
    CHECK(SotnBatRuntime_Tick(r, c, in) &&
          r->children[42].state.healing_number.x == before.x - 16 * 65536 &&
          r->children[42].state.healing_number.y == before.y + 2 * 65536 &&
          r->children[42].state.healing_number.timer == before.timer);
    in.entering = false;
    in.screen_shift_x = in.screen_shift_y = 0;
    for (Uint16 tick = 0; tick < 160 && r->children[42].kind; ++tick)
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, c, in) && SotnActors_CommitBatPrimitives(actors, r));
    CHECK(!SotnBatRuntime_Occupied(r));
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        CHECK(!actors->primitives[i].owner);
    in.freeze = 0;
    in.player.healing.applied = true;
    in.player.healing.applied_kind = 2;
    in.other_occupied = 0xff00000000000000ULL;
    CHECK(SotnBatRuntime_Tick(r, c, in) && !SotnBatRuntime_Occupied(r));
    in.other_occupied = 0;
    SDL_memset(&in.primitive_slots, 255, sizeof(in.primitive_slots));
    CHECK(SotnBatRuntime_Tick(r, c, in) && !SotnBatRuntime_Occupied(r));
    in.primitive_slots = (SotnPrimitiveSlots){0};
    CHECK(SotnBatRuntime_Tick(r, c, in));
    in.player.healing.applied = false;
    while (r->children[40].state.healing_number.phase != 5 ||
           r->children[40].state.healing_number.timer != 1)
        CHECK(SotnBatRuntime_Tick(r, c, in));
    const SotnBatRuntime saved = *r;
    in.executable = (SotnDataView){0};
    CHECK(!SotnBatRuntime_Tick(r, c, in) && !SDL_memcmp(&saved, r, sizeof(*r)));
    r->children[40].primitive_count = 19;
    CHECK(!SotnActors_CommitBatPrimitives(actors, r));
    *r = saved;
    r->children[40].primitives[1] = r->children[40].primitives[0];
    CHECK(!SotnActors_CommitBatPrimitives(actors, r));
    SotnCharacter_Destroy(c);
    SDL_free(dra);
    SDL_free(r);
    SDL_free(actors);
    return true;
}
int main(void)
{
    const size_t size = 0x22a10 + 2050;
    Uint8 *exe = SDL_calloc(1, size);
    if (!exe)
        return 1;
    for (Uint16 i = 0; i <= 1024; ++i)
    {
        Put16(exe + 0x1dbcc + i * 2U, i * 4U);
        Put16(exe + 0x22a10 + i * 2U, i / 2U);
    }
    for (Uint16 i = 0; i < 192; ++i)
        Put16(exe + 0x1e3e0 + i * 2U, 4096);
    const bool ok = Motion((SotnDataView){exe, size}) && Runtime((SotnDataView){exe, size});
    SDL_free(exe);
    return ok ? 0 : 1;
}
