#include "character_fixture.h"
#include <SDL3/SDL.h>
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
static bool Motion(SotnDataView exe)
{
    SotnRisingHeart h = {0};
    const SotnPlayerFactorySource source = {.x = SDL_MAX_SINT32, .y = SDL_MIN_SINT32 + 456};
    CHECK(SotnRisingHeart_Tick(&h, (SotnDataView){0}, source, 148, 0) && h.x == SDL_MAX_SINT32 &&
          h.y == SDL_MAX_SINT32 - 16 * 65536 + 457 && h.timer == 128 && h.speed == 64 &&
          h.priority == 146 && !h.blink);
    CHECK(SotnRisingHeart_Tick(&h, exe, source, 148, 1) && h.x == SDL_MIN_SINT32 + 32767 &&
          h.timer == 127 && h.angle == 64 && h.speed == 64);
    const SotnRisingHeart held = h;
    CHECK(!SotnRisingHeart_Tick(&h, (SotnDataView){0}, source, 148, 4) &&
          !SDL_memcmp(&h, &held, sizeof(h)));
    CHECK(SotnRisingHeart_Rebase(&h, 10, -20) && h.timer == 127 && h.angle == 64 &&
          h.x == held.x + 10 * 65536 && h.y == held.y - 20 * 65536);
    h.angle = 32760;
    h.speed = 64;
    CHECK(SotnRisingHeart_Tick(&h, exe, source, 148, 4) && h.angle == -32712 && h.speed == 63);
    while (h.timer > 32)
        CHECK(SotnRisingHeart_Tick(&h, exe, source, 148, 0));
    CHECK(!h.blink && SotnRisingHeart_Tick(&h, exe, source, 148, 0) && h.timer == 31 && !h.blink);
    CHECK(SotnRisingHeart_Tick(&h, exe, source, 148, 0) && h.timer == 30 && h.blink);
    while (h.timer)
        CHECK(SotnRisingHeart_Tick(&h, exe, source, 148, SDL_MAX_UINT32));
    CHECK(h.removed && !SotnRisingHeart_Tick(&h, exe, source, 148, 0));
    return true;
}
static bool Runtime(SotnDataView exe)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    CHECK(dra && r);
    TestCharacter_Fill(dra);
    dra[0x3400 + 54 * 32 + 28] = 2;
    Uint8 *blueprint = dra + 0xd1d4 + 99 * 6;
    blueprint[0] = 51;
    blueprint[1] = 1;
    blueprint[2] = 0x81;
    dra[0xd4b8] = 16;
    dra[0xd4b9] = 63;
    SotnCharacter *c = NULL;
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &c));
    CHECK(SotnCharacter_Collect(c, true, 54) && SotnCharacter_Equip(c, SOTN_EQUIP_CAPE, 54));
    SotnBatRuntimeInput in = {
        .dra = {dra, 0x11000},
        .executable = exe,
        .player = {.priority = 148,
                   .heart_gain = {.requested = true,
                                  .amount = 3,
                                  .source = {.x = 100 * 65536 + 123, .y = 90 * 65536 + 456}}}};
    SotnCharacterState s = {0};
    CHECK(SotnCharacter_AddHearts(c, 3));
    CHECK(SotnCharacter_Get(c, &s, NULL));
    const SotnCharacterState resource_before = s;
    CHECK(!SotnCharacter_AddHearts(c, 10000) && SotnCharacter_Get(c, &s, NULL) &&
          !SDL_memcmp(&resource_before, &s, sizeof(s)));
    CHECK(SotnBatRuntime_Tick(r, c, in) && SotnCharacter_Get(c, &s, NULL) && s.hearts == 13 &&
          r->sound_count == 1 && r->sounds[0] == 0x67a && r->children[47].kind == 51 &&
          r->children[47].initialized && !r->children[47].primitive_count &&
          r->children[47].state.heart.timer == 128 &&
          r->children[47].state.heart.x == 100 * 65536 + 123);
    in.player.heart_gain.requested = false;
    in.freeze = 3;
    const SotnRisingHeart before = r->children[47].state.heart;
    CHECK(SotnBatRuntime_Tick(r, c, in) &&
          !SDL_memcmp(&before, &r->children[47].state.heart, sizeof(before)) && !r->sound_count);
    in.entering = true;
    in.screen_shift_x = -10;
    CHECK(SotnBatRuntime_Tick(r, c, in) && r->children[47].state.heart.x == before.x - 10 * 65536 &&
          r->children[47].state.heart.timer == 128);
    in.entering = false;
    in.screen_shift_x = 0;
    in.freeze = 0;
    CHECK(SotnBatRuntime_Tick(r, c, in) && r->children[47].state.heart.timer == 127 &&
          SotnCharacter_Get(c, &s, NULL) && s.hearts == 13);
    SDL_memset(r, 0, sizeof(*r));
    s.hearts = s.hearts_max - 1;
    CHECK(SotnCharacter_Restore(c, &s));
    in.player.heart_gain.requested = true;
    in.dra.size = 0xd1d4;
    const SotnBatRuntime empty = *r;
    CHECK(!SotnBatRuntime_Tick(r, c, in) && !SDL_memcmp(r, &empty, sizeof(*r)) &&
          SotnCharacter_Get(c, &s, NULL) && s.hearts == s.hearts_max - 1);
    in.dra.size = 0x11000;
    in.other_occupied = SDL_MAX_UINT64 & ~0xffffULL;
    CHECK(SotnCharacter_AddHearts(c, 3));
    CHECK(SotnBatRuntime_Tick(r, c, in) && !SotnBatRuntime_Occupied(r) && r->sound_count == 1 &&
          SotnCharacter_Get(c, &s, NULL) && s.hearts == s.hearts_max);
    in.player.heart_gain.requested = false;
    CHECK(SotnBatRuntime_Tick(r, c, in) && !r->sound_count && !SotnBatRuntime_Occupied(r));
    s.hearts = 10;
    CHECK(SotnCharacter_Restore(c, &s));
    in.other_occupied = 0;
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(
            SotnPlayerFactory_Begin(&r->factories[i].cursor, (SotnPlayerFactorySource){0}, 99, 0));
    in.player.heart_gain.amount = 0;
    in.player.heart_gain.requested = true;
    CHECK(SotnCharacter_AddHearts(c, 0));
    CHECK(SotnBatRuntime_Tick(r, c, in) && r->sound_count == 1 && SotnCharacter_Get(c, &s, NULL) &&
          s.hearts == 10);
    Uint16 count = 0;
    for (Uint16 i = 0; i < 48; ++i)
        count += r->children[i].kind == 51;
    CHECK(count == 8);
    in.player.heart_gain.requested = false;
    in.executable = (SotnDataView){0};
    const SotnBatRuntime before_failure = *r;
    CHECK(!SotnBatRuntime_Tick(r, c, in) && !SDL_memcmp(&before_failure, r, sizeof(*r)) &&
          SotnCharacter_Get(c, &s, NULL) && s.hearts == 10);
    in.executable = exe;
    const Sint16 positions[][2] = {{-32, 0}, {-33, 0}, {288, 0}, {289, 0},
                                   {0, 0},   {0, -1},  {0, 272}, {0, 273}};
    for (Uint16 i = 0; i < SDL_arraysize(positions); ++i)
    {
        SDL_memset(r, 0, sizeof(*r));
        in.player.heart_gain.requested = true;
        in.player.heart_gain.source.x = (Sint32)positions[i][0] * 65536;
        in.player.heart_gain.source.y = (Sint32)positions[i][1] * 65536;
        CHECK(SotnBatRuntime_Tick(r, c, in) && (r->children[47].kind == 51) == ((i & 1U) == 0) &&
              r->sound_count == 1 && SotnCharacter_Get(c, &s, NULL) && s.hearts == 10);
    }
    SotnCharacter_Destroy(c);
    SDL_free(r);
    SDL_free(dra);
    return true;
}
int main(void)
{
    const size_t size = 0x1dbcc + 2050;
    Uint8 *exe = SDL_calloc(1, size);
    if (!exe)
        return 1;
    /* Authored flat waveform deliberately differs from private SDK data. */
    for (size_t i = 0x1dbcc; i < size; i += 2)
    {
        exe[i] = 0;
        exe[i + 1] = 16;
    }
    const bool ok = Motion((SotnDataView){exe, size}) && Runtime((SotnDataView){exe, size});
    SDL_free(exe);
    return ok ? 0 : 1;
}
