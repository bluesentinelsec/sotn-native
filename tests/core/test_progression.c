#include "character_fixture.h"
#include <sotn/character.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
int main(void)
{
    Uint8 *bytes = SDL_calloc(1, 0xc910);
    CHECK(bytes != NULL);
    TestCharacter_Fill(bytes);
    TestCharacter_Progression(bytes);
    SotnCharacter *c = NULL;
    CHECK(SotnCharacter_Create((SotnDataView){bytes, 0xc910}, &c));
    Uint16 levels = 0;
    Uint32 next = 0;
    CHECK(SotnCharacter_NextLevel(c, &next) && next == 10);
    CHECK(SotnCharacter_GainExperience(c, 10, 1, &levels) && levels == 1);
    SotnCharacterState state = {0};
    CHECK(SotnCharacter_Get(c, &state, NULL));
    CHECK(state.level == 2 && state.experience == 10 && state.hp == 71 && state.hp_max == 71 &&
          state.mp == 20 && state.mp_max == 24 && state.hearts_max == 52);
    CHECK(SotnCharacter_NextLevel(c, &next) && next == 10);
    CHECK(SotnCharacter_GainExperience(c, 0, 1, &levels) && levels == 0);
    CHECK(SotnCharacter_TickResources(c, 63));
    CHECK(SotnCharacter_Get(c, &state, NULL) && state.mp == 20);
    CHECK(SotnCharacter_TickResources(c, 64));
    CHECK(SotnCharacter_Get(c, &state, NULL) && state.mp == 21);
    CHECK(SotnCharacter_CollectVessel(c, true));
    CHECK(SotnCharacter_CollectVessel(c, false));
    CHECK(SotnCharacter_Get(c, &state, NULL) && state.hp == 76 && state.hp_max == 76 &&
          state.hearts == 15 && state.hearts_max == 57);
    CHECK(SotnCharacter_GainExperience(c, 1, 0, &levels));
    CHECK(SotnCharacter_Get(c, &state, NULL) && state.experience == 11);
    const Uint32 random = state.random;
    TestCharacter_Put32(bytes + 0xc780 + 3 * 4, 0);
    CHECK(!SotnCharacter_GainExperience(c, 100, 1, &levels));
    CHECK(SotnCharacter_Get(c, &state, NULL) && state.random == random && state.experience == 11);
    TestCharacter_Progression(bytes);
    CHECK(SotnCharacter_GainExperience(c, 100, 2, &levels) && levels == 10);
    CHECK(SotnCharacter_Get(c, &state, NULL) && state.level == 12 && state.experience == 111 &&
          state.hp_max == 86 && state.hearts_max == 77);
    CHECK(SotnCharacter_GainExperience(c, 65535, 99, &levels) && levels == 87);
    CHECK(SotnCharacter_NextLevel(c, &next) && next == 0);
    CHECK(SotnCharacter_Get(c, &state, NULL) && state.level == 99 && state.experience == 980);
    CHECK(SotnCharacter_GainExperience(c, 65535, 99, &levels) && levels == 0);
    SotnCharacter_Destroy(c);
    SDL_free(bytes);
    SDL_Log("SOTN_PROGRESSION_TEST: PASS");
    return 0;
}
