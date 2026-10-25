#include <SDL3/SDL.h>
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
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8));
}
int main(void)
{
    Uint8 *bytes = SDL_calloc(1, 0x5000);
    CHECK(bytes != NULL);
    Put32(bytes + 0xc0, 0x800a1000);
    Put32(bytes + 0xc4, 0x800a4000);
    bytes[0x100] = 'T' - 32;
    bytes[0x101] = 'e' - 32;
    bytes[0x102] = 's' - 32;
    bytes[0x103] = 't' - 32;
    bytes[0x104] = 255;
    for (Uint32 body = 0; body < 2; ++body)
        for (Uint32 id = 0; id < (body ? 90U : 169U); ++id)
        {
            Uint8 *p = bytes + (body ? 0x4000 + id * 32 : 0x1000 + id * 52);
            Put32(p, 0x800a0100);
            Put32(p + 4, 0x800a0180);
            if (body)
                p[28] = (Uint8)(id == 26 ? 0 : id == 48 ? 2 : id == 57 ? 3 : 1);
        }
    bytes[0x1000 + 86 * 52 + 8] = 4;
    bytes[0x1000 + 128 * 52 + 8] = 12;
    bytes[0x4000 + 1 * 32 + 10] = 3;
    bytes[0x4000 + 1 * 32 + 12] = 2;
    bytes[0x4000 + 1 * 32 + 13] = 1;
    const SotnDataView dra = {bytes, 0x5000};
    SotnCharacter *c = NULL;
    SotnCharacterState s = {0};
    SotnCharacterStats stats = {0};
    CHECK(SotnCharacter_Create(dra, &c));
    CHECK(SotnCharacter_Get(c, &s, &stats));
    CHECK(s.hp == 70 && s.level == 1 && stats.attack[0] == 6 && stats.defense == 0 &&
          s.equipment[5] == 57);
    CHECK(!SotnCharacter_Equip(c, 0, 86));
    CHECK(SotnCharacter_Collect(c, false, 86));
    CHECK(SotnCharacter_Equip(c, 0, 86));
    CHECK(!SotnCharacter_Equip(c, 1, 86));
    CHECK(SotnCharacter_Collect(c, false, 86));
    CHECK(SotnCharacter_Equip(c, 1, 86));
    CHECK(SotnCharacter_Collect(c, true, 1));
    CHECK(!SotnCharacter_Equip(c, 2, 1));
    CHECK(SotnCharacter_Equip(c, 3, 1));
    CHECK(SotnCharacter_Get(c, &s, &stats));
    CHECK(stats.total[0] == 8 && stats.total[1] == 14 && stats.attack[0] == 12 &&
          stats.defense == 4);
    CHECK(SotnCharacter_Equip(c, 0, 0));
    CHECK(SotnCharacter_Get(c, &s, NULL) && s.hand_count[86] == 1);
    CHECK(SotnCharacter_Collect(c, false, 1));
    CHECK(!SotnCharacter_Equip(c, 0, 1));
    for (Uint32 i = 0; i < 120; ++i)
        CHECK(SotnCharacter_Collect(c, false, 128));
    CHECK(SotnCharacter_Get(c, &s, NULL) && s.hand_count[128] == 99);
    SotnCharacterState invalid = s;
    invalid.hand_order[1] = invalid.hand_order[0];
    CHECK(!SotnCharacter_Restore(c, &invalid));
    invalid = s;
    invalid.equipment[0] = 169;
    CHECK(!SotnCharacter_Restore(c, &invalid));
    invalid = s;
    invalid.equipment[2] = 1;
    CHECK(!SotnCharacter_Restore(c, &invalid));
    CHECK(SotnCharacter_Get(c, &invalid, NULL) && invalid.equipment[0] == s.equipment[0]);
    SotnItem item = {0};
    char name[16] = {0};
    CHECK(SotnItem_Read(dra, false, 86, &item));
    CHECK(SotnItem_Name(item, name, sizeof(name)) && SDL_strcmp(name, "Test") == 0);
    CHECK(!SotnItem_Name(item, name, 2));
    CHECK(!SotnItem_Read(dra, true, 90, &item));
    const Uint16 stolen_hand[] = {123, 16};
    const Uint16 stolen_body[] = {15, 45, 56, 78};
    for (Uint32 i = 0; i < 2; ++i)
    {
        CHECK(SotnCharacter_Collect(c, false, stolen_hand[i]));
        CHECK(SotnCharacter_Collect(c, false, stolen_hand[i]));
    }
    for (Uint32 i = 0; i < 4; ++i)
        CHECK(SotnCharacter_Collect(c, true, stolen_body[i]));
    CHECK(SotnCharacter_Get(c, &s, NULL));
    const Uint16 left_before = s.equipment[0], right_before = s.equipment[1];
    const Uint16 hp_before = s.hp;
    CHECK(SotnCharacter_DeathTheft(c));
    CHECK(SotnCharacter_Get(c, &s, NULL));
    CHECK(s.equipment[0] == left_before && s.equipment[1] == right_before && s.hp == hp_before);
    for (Uint32 i = 0; i < 2; ++i)
        CHECK(s.hand_count[stolen_hand[i]] == 1);
    for (Uint32 i = 0; i < 4; ++i)
        CHECK(s.body_count[stolen_body[i]] == 0);
    CHECK(!SotnCharacter_DeathTheft(NULL));
    bytes[0x4000 + 45 * 32 + 28] = 0;
    bytes[0x4000 + 56 * 32 + 28] = 2;
    bytes[0x4000 + 78 * 32 + 28] = 3;
    CHECK(SotnCharacter_OpeningGear(c));
    CHECK(SotnCharacter_Get(c, &s, NULL));
    CHECK(s.equipment[0] == 123 && s.equipment[1] == 16 && s.equipment[2] == 45 &&
          s.equipment[3] == 15 && s.equipment[4] == 56 && s.equipment[5] == 78);
    CHECK(SotnCharacter_DeathTheft(c));
    CHECK(SotnCharacter_Get(c, &s, NULL));
    CHECK(s.equipment[0] == 0 && s.equipment[1] == 0 && s.equipment[2] == 26 &&
          s.equipment[3] == 0 && s.equipment[4] == 48 && s.equipment[5] == 57);
    CHECK(s.hand_count[123] == 1 && s.hand_count[16] == 1);
    CHECK(!SotnCharacter_OpeningGear(NULL));
    bytes[0x104] = 0;
    CHECK(!SotnItem_Read(dra, false, 86, &item));
    SotnCharacter_Destroy(c);
    SDL_free(bytes);
    SDL_Log("SOTN_CHARACTER_TEST: PASS");
    return 0;
}
