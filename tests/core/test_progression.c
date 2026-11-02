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
    SotnFamiliarProgress familiar = {0}, held = {0};
    Uint16 selected = 99;
    CHECK(SotnCharacter_GetFamiliars(c, &familiar, &selected) && !selected);
    for (Uint16 i = 0; i < SOTN_FAMILIAR_COUNT; ++i)
        CHECK(familiar.records[i].level == 1 && !familiar.records[i].experience &&
              !familiar.records[i].loads);
    CHECK(SotnCharacter_SelectFamiliar(c, 3, false) && SotnCharacter_SelectFamiliar(c, 3, false) &&
          SotnCharacter_GetFamiliars(c, &familiar, &selected) && selected == 3 &&
          familiar.records[2].loads == 1);
    held = familiar;
    CHECK(!SotnCharacter_SelectFamiliar(c, 8, true) && !SotnCharacter_SelectFamiliar(c, 0, true) &&
          SotnCharacter_GetFamiliars(c, &familiar, &selected) && selected == 3 &&
          !SDL_memcmp(&held, &familiar, sizeof(held)));
    Uint16 levels = 0;
    Uint32 next = 0;
    CHECK(SotnCharacter_NextLevel(c, &next) && next == 10);
    CHECK(SotnCharacter_GainExperience(c, 10, 1, &levels) && levels == 1);
    CHECK(SotnCharacter_GetFamiliars(c, &familiar, NULL) && familiar.records[2].experience == 4);
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
    CHECK(SotnCharacter_GetFamiliars(c, &familiar, NULL) && familiar.records[2].experience == 5);
    CHECK(SotnCharacter_Get(c, &state, NULL) && state.random == random && state.experience == 11);
    TestCharacter_Progression(bytes);
    CHECK(SotnCharacter_GetFamiliars(c, &held, NULL));
    TestCharacter_Put32(bytes + 0x2ec0, 9999);
    CHECK(!SotnCharacter_GainExperience(c, 100, 2, &levels) &&
          SotnCharacter_GetFamiliars(c, &familiar, NULL) &&
          !SDL_memcmp(&held, &familiar, sizeof(held)));
    CHECK(SotnCharacter_Get(c, &state, NULL) && state.random == random && state.experience == 11);
    TestCharacter_Progression(bytes);
    CHECK(SotnCharacter_GainExperience(c, 100, 2, &levels) && levels == 10);
    CHECK(SotnCharacter_Get(c, &state, NULL) && state.level == 12 && state.experience == 111 &&
          state.hp_max == 86 && state.hearts_max == 77);
    CHECK(SotnCharacter_GainExperience(c, 65535, 99, &levels) && levels == 87);
    CHECK(SotnCharacter_GetFamiliars(c, &familiar, NULL) && familiar.records[2].experience == 28);
    held = familiar;
    CHECK(SotnCharacter_NextLevel(c, &next) && next == 0);
    CHECK(SotnCharacter_Get(c, &state, NULL) && state.level == 99 && state.experience == 980);
    CHECK(SotnCharacter_GainExperience(c, 65535, 99, &levels) && levels == 0);
    CHECK(SotnCharacter_GetFamiliars(c, &familiar, NULL) &&
          !SDL_memcmp(&held, &familiar, sizeof(held)));
    CHECK(SotnCharacter_SelectFamiliar(c, 3, true) &&
          SotnCharacter_GetFamiliars(c, &familiar, NULL) && familiar.records[2].loads == 2);
    familiar.records[2] = (SotnFamiliarStats){.level = 99, .experience = 9898, .loads = 9999};
    CHECK(SotnCharacter_RestoreFamiliars(c, &familiar) &&
          SotnCharacter_SelectFamiliar(c, 3, true) &&
          SotnCharacter_GetFamiliars(c, &familiar, NULL) && familiar.records[2].loads == 9999);
    CHECK(SotnFamiliarProgress_Reward(&familiar, 3, 65535, 1) &&
          familiar.records[2].experience == 9899 && familiar.records[2].level == 99);
    held = familiar;
    familiar.records[6].level = 0;
    CHECK(!SotnCharacter_RestoreFamiliars(c, &familiar) &&
          !SotnFamiliarProgress_Reward(&familiar, 3, 100, 1) &&
          familiar.records[2].experience == held.records[2].experience);
    CHECK(SotnCharacter_GetFamiliars(c, &familiar, &selected) && selected == 3 &&
          familiar.records[2].experience == 9898);
    SotnCharacter *destination = NULL;
    SotnCharacterState before_restore = {0}, after_restore = {0};
    CHECK(SotnCharacter_Create((SotnDataView){bytes, 0xc910}, &destination) &&
          SotnCharacter_Get(destination, &before_restore, NULL) &&
          SotnCharacter_RestoreFamiliarState(destination, &familiar, 3) &&
          SotnCharacter_GetFamiliars(destination, &held, &selected) && selected == 3 &&
          !SDL_memcmp(&held, &familiar, sizeof(held)) &&
          SotnCharacter_Get(destination, &after_restore, NULL) &&
          !SDL_memcmp(&before_restore, &after_restore, sizeof(before_restore)));
    // A restored selection is already current: its next unchanged menu exit does not load again.
    CHECK(SotnCharacter_SelectFamiliar(destination, 3, false) &&
          SotnCharacter_GetFamiliars(destination, &held, NULL) &&
          !SDL_memcmp(&held, &familiar, sizeof(held)));
    CHECK(!SotnCharacter_RestoreFamiliarState(destination, &familiar, 8));
    familiar.records[6].loads = -1;
    CHECK(!SotnCharacter_RestoreFamiliarState(destination, &familiar, 0) &&
          SotnCharacter_GetFamiliars(destination, &familiar, &selected) && selected == 3 &&
          !SDL_memcmp(&held, &familiar, sizeof(held)));
    CHECK(SotnCharacter_RestoreFamiliarState(destination, &familiar, 0) &&
          SotnCharacter_GetFamiliars(destination, &held, &selected) && !selected &&
          !SDL_memcmp(&held, &familiar, sizeof(held)) &&
          SotnCharacter_Get(destination, &after_restore, NULL) &&
          !SDL_memcmp(&before_restore, &after_restore, sizeof(before_restore)));
    SotnCharacter_Destroy(destination);
    CHECK(SotnCharacter_SelectFamiliar(c, 0, false) &&
          SotnCharacter_GetFamiliars(c, &familiar, &selected) && !selected);
    SotnCharacter_Destroy(c);
    SDL_free(bytes);
    SDL_Log("SOTN_PROGRESSION_TEST: PASS");
    return 0;
}
