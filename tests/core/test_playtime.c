#include "actors_internal.h"
#include "character_fixture.h"
#include <sotn/character.h>
#include <sotn/playtime.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 n)
{
    p[0] = (Uint8)n;
    p[1] = (Uint8)(n >> 8);
}
int main(void)
{
    SotnPlaytime clock = {.hours = 1, .minutes = 59, .seconds = 59, .frames = 59};
    CHECK(SotnPlaytime_Advance(&clock, 256, false, false) && clock.hours == 2 && !clock.minutes &&
          !clock.seconds && !clock.frames);
    clock = (SotnPlaytime){.hours = 99, .minutes = 59, .seconds = 59, .frames = 59};
    CHECK(SotnPlaytime_Advance(&clock, 256, false, false) && clock.hours == 99 &&
          clock.minutes == 59 && clock.seconds == 59 && !clock.frames);
    const SotnPlaytime held = clock;
    CHECK(SotnPlaytime_Advance(&clock, 512, true, false) &&
          SotnPlaytime_Advance(&clock, 512, false, true) &&
          !SDL_memcmp(&clock, &held, sizeof(clock)));
    clock = (SotnPlaytime){0};
    CHECK(SotnPlaytime_Advance(&clock, 0, false, false) && !clock.frames &&
          SotnPlaytime_Advance(&clock, 1, false, false) && clock.frames == 1 &&
          SotnPlaytime_Advance(&clock, 255, false, false) && clock.frames == 2 &&
          SotnPlaytime_Advance(&clock, 257, false, false) && clock.frames == 4);
    // The original performs one carry per word, rather than normalizing an arbitrary hitch.
    CHECK(SotnPlaytime_Advance(&clock, 256 * 180, false, false) && clock.frames == 124 &&
          clock.seconds == 1);
    clock = (SotnPlaytime){.frames = SDL_MAX_SINT32};
    CHECK(SotnPlaytime_Advance(&clock, 256, false, false) && clock.frames == SDL_MIN_SINT32);
    Uint64 ticks = 123;
    CHECK(!SotnPlaytime_Ticks(clock, &ticks) && ticks == 123);
    CHECK(SotnPlaytime_FromTicks(99999ULL * 3600 * 60, &clock) && clock.hours == 99999 &&
          SotnPlaytime_Advance(&clock, 256, false, false) && clock.hours == 99999 &&
          clock.frames == 1 && SotnPlaytime_Ticks(clock, &ticks) && ticks == 99999ULL * 3600 * 60);
    const SotnPlaytime retained = clock;
    CHECK(!SotnPlaytime_FromTicks(99999ULL * 3600 * 60 + 1, &clock) &&
          !SDL_memcmp(&retained, &clock, sizeof(clock)));

    Uint8 *dra = SDL_calloc(1, 0x11000), *executable = SDL_calloc(1, 0x24000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    CHECK(dra && executable && actors);
    TestCharacter_Fill(dra);
    Uint8 *item = dra + 0x500 + 125U * 52U;
    Put16(item + 8, 3);
    item[14] = 1;
    item[17] = 1;
    item[18] = 0x41;
    item[19] = 48;
    item[20] = 3;
    item[21] = 13;
    item[22] = 128;
    Put16(item + 40, 2);
    SotnCharacter *c = NULL;
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &c) &&
          SotnCharacter_Collect(c, false, 125));
    SotnItem meta = {0};
    SotnEquipment props = {0};
    Uint32 random = 7;
    CHECK(SotnCharacter_Item(c, false, 125, &meta) && !meta.supported &&
          !SotnCharacter_Equip(c, 0, 125) &&
          !SotnCharacter_HandProperties(c, 125, 0, false, &random, &props) && random == 7);
    clock = (SotnPlaytime){.hours = 1, .minutes = 2, .seconds = 3, .frames = 4};
    CHECK(SotnCharacter_SetPlaytime(c, clock) && SotnCharacter_Item(c, false, 125, &meta) &&
          meta.supported && SotnCharacter_Equip(c, 0, 125));
    SotnCharacterStats stats = {0};
    CHECK(SotnCharacter_Get(c, NULL, &stats) && stats.attack[0] == 10 &&
          SotnCharacter_HandProperties(c, 125, 0, true, &random, &props) && props.attack == 5);
    clock.hours = 32767;
    CHECK(SotnCharacter_SetPlaytime(c, clock) && SotnCharacter_Get(c, NULL, &stats) &&
          !stats.attack[0]);
    // Stage HMS, engine modulo phase and gameplay resources have separate owners.
    clock = (SotnPlaytime){.hours = 1, .minutes = 2, .seconds = 3, .frames = 4};
    CHECK(SotnActors_SetEngineServices(actors, (SotnDataView){executable, 0x24000}, clock,
                                       0xffffffffU, true) &&
          actors->game_seconds == 3723 && actors->game_frame == 4 &&
          actors->service_ticks == 0xffffffffU && actors->music_playing);
    clock.frames = 60;
    CHECK(!SotnActors_SetEngineServices(actors, (SotnDataView){executable, 0x24000}, clock, 0,
                                        false) &&
          actors->game_seconds == 3723 && actors->game_frame == 4 &&
          actors->service_ticks == 0xffffffffU && actors->music_playing);
    SotnCharacter_Destroy(c);
    SDL_free(actors);
    SDL_free(executable);
    SDL_free(dra);
    return 0;
}
