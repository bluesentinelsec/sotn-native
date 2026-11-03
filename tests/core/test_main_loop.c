#include "character_fixture.h"
#include <sotn/character.h>
#include <sotn/combat.h>
#include <sotn/main_loop.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
int main(void)
{
    for (Uint16 flags = 0; flags < 4; ++flags)
    {
        SotnMainLoopTail tail = {{0, 59, 59, 59}, 0xffffffffU, 7};
        const SotnPlaytime held = tail.playtime;
        Uint32 expected = tail.random;
        (void)SotnCombat_Random(&expected);
        CHECK(SotnMainLoop_Advance(&tail, 256, (flags & 1U) != 0, (flags & 2U) != 0) &&
              !tail.map_cursor && tail.random == expected);
        if (flags)
            CHECK(!SDL_memcmp(&tail.playtime, &held, sizeof(held)));
        else
            CHECK(tail.playtime.hours == 1 && !tail.playtime.minutes && !tail.playtime.seconds &&
                  !tail.playtime.frames);
        // Even a zero normalized count still executes the original one trailing rand call.
        const SotnMainLoopTail zero = tail;
        (void)SotnCombat_Random(&expected);
        CHECK(SotnMainLoop_Advance(&tail, 0, false, false) &&
              !SDL_memcmp(&tail.playtime, &zero.playtime, sizeof(zero.playtime)) &&
              tail.map_cursor == zero.map_cursor && tail.random == expected);
    }
    SotnMainLoopTail negative = {{0}, 0, 0};
    CHECK(SotnPlaytime_FrameDelta(-257) == -1 &&
          SotnPlaytime_FrameDelta(SDL_MAX_SINT32) == -8388608 &&
          SotnMainLoop_Advance(&negative, -257, false, false) &&
          negative.map_cursor == 0xffffffffU && negative.playtime.frames == -1);
    CHECK(!SotnMainLoop_Advance(NULL, 256, false, false));
    Uint8 *dra = SDL_calloc(1, 0x11000);
    CHECK(dra);
    TestCharacter_Fill(dra);
    SotnCharacter *character = NULL;
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    SotnCharacterState before = {0}, after = {0};
    SotnCharacterStats old_stats = {0}, stats = {0};
    CHECK(SotnCharacter_Get(character, &before, &old_stats));
    const SotnPlaytime clock = {37, 1, 2, 3};
    CHECK(SotnCharacter_CommitMainLoop(character, clock, 0xfedcba98U) &&
          SotnCharacter_Get(character, &after, &stats) && after.random == 0xfedcba98U &&
          !SDL_memcmp(&old_stats, &stats, sizeof(stats)));
    after.random = before.random;
    CHECK(!SDL_memcmp(&before, &after, sizeof(before)) &&
          !SotnCharacter_CommitMainLoop(NULL, clock, 0));
    // Reject a later-hour derived-stat failure without publishing either clock or RNG.
    // Repeating the same failed hour catches a leaked clock that would skip recalculation.
    SotnCharacterState committed = {0}, rejected = {0};
    CHECK(SotnCharacter_Get(character, &committed, &old_stats) && committed.equipment[2] == 26);
    dra[0x3400 + 26U * 32U + 28] = 1;
    const SotnPlaytime later = {38, 1, 2, 3};
    CHECK(!SotnCharacter_CommitMainLoop(character, later, 0x12345678U) &&
          !SotnCharacter_CommitMainLoop(character, later, 0x87654321U) &&
          SotnCharacter_Get(character, &rejected, &stats) &&
          !SDL_memcmp(&committed, &rejected, sizeof(committed)) &&
          !SDL_memcmp(&old_stats, &stats, sizeof(stats)));
    dra[0x3400 + 26U * 32U + 28] = 0;
    CHECK(SotnCharacter_CommitMainLoop(character, later, 0x12345678U));
    SotnCharacter_Destroy(character);
    SDL_free(dra);
    return 0;
}
