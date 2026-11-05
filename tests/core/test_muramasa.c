// Muramasa's g_Status.D_80097C40 blood counter with authored tables only: menu.c CalcAttack's
// SquareRoot0 bonus (table-free below 0x80000000, the bound SDK table above), saturation and the
// signed-range binding requirement.
#include "character_fixture.h"
#include <sotn/character.h>
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
static bool Attack(SotnCharacter *c, Uint32 blood, Uint16 *attack)
{
    SotnCharacterState state = {0};
    SotnCharacterStats stats = {0};
    if (!SotnCharacter_Get(c, &state, NULL))
        return false;
    state.muramasa_blood = blood;
    if (!SotnCharacter_Restore(c, &state) || !SotnCharacter_Get(c, NULL, &stats))
        return false;
    *attack = stats.attack[0];
    return stats.attack[1] == stats.attack[0];
}
int main(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *exe = SDL_calloc(1, 0x24000);
    CHECK(dra && exe);
    TestCharacter_Fill(dra);
    Uint8 *item = dra + 0x500 + 141U * 52U;
    Put16(item + 8, 1);
    item[14] = 5;
    item[15] = 56;
    item[17] = 29;
    item[18] = 0x41;
    item[19] = 48;
    item[21] = 13;
    item[22] = 128;
    Put16(item + 40, 2);
    SotnCharacter *c = NULL;
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &c) &&
          SotnCharacter_Collect(c, false, 141) && SotnCharacter_Equip(c, 0, 141));
    SotnCharacterState state = {0};
    SotnCharacterStats stats = {0};
    CHECK(SotnCharacter_Get(c, &state, &stats) && state.equipment[0] == 141 &&
          state.equipment[1] == 141 && !state.muramasa_blood);
    const Uint16 base = stats.attack[0];
    // SquareRoot0 of the counter adds to Muramasa's attack (100 -> 10, 4e8 -> about20000,
    // clamped to999). CalcAttack sums in a signed word: 0x7FFFFFFF's root46340 wraps it
    // negative, clamped to0.
    Uint16 attack = 0;
    CHECK(Attack(c, 100, &attack) && attack == base + 10);
    CHECK(Attack(c, 99, &attack) && attack == base + 9);
    CHECK(Attack(c, 400000000U, &attack) && attack == 999);
    CHECK(Attack(c, 0x7fffffffU, &attack) && attack == 0);
    Uint32 random = 3;
    SotnEquipment props = {0};
    CHECK(Attack(c, 144, &attack) && SotnCharacter_HandProperties(c, 141, false, &random, &props) &&
          props.attack == base + 12 &&
          SotnCharacter_HandProperties(c, 141, true, &random, &props) &&
          props.attack == (base + 12) / 2);
    // Each committed update adds one, saturating at0xFFFFFFFF.
    CHECK(Attack(c, 120, &attack) && SotnCharacter_AddMuramasaBlood(c) &&
          SotnCharacter_Get(c, &state, &stats) && state.muramasa_blood == 121 &&
          stats.attack[0] == base + 11);
    // From0x80000000 the signed SDK input indexes before the table: it needs the bound image.
    CHECK(Attack(c, 0x7fffffffU, &attack) && !SotnCharacter_AddMuramasaBlood(c) &&
          SotnCharacter_Get(c, &state, NULL) && state.muramasa_blood == 0x7fffffffU);
    CHECK(!SotnCharacter_BindExecutable(c, (SotnDataView){exe, 0x1e3e0}));
    // 0x80000000: one leading sign bit, even0, srav24 -> -128, entry -192 (0x1E3E0-384).
    Put16(exe + 0x1e3e0 - 384, 0x0010);
    // 0xFFFFFF00: twenty-four leading ones, even24, sllv0 -> -256, entry -320.
    Put16(exe + 0x1e3e0 - 640, 0x8000);
    CHECK(SotnCharacter_BindExecutable(c, (SotnDataView){exe, 0x24000}) &&
          SotnCharacter_AddMuramasaBlood(c) && SotnCharacter_Get(c, &state, &stats) &&
          state.muramasa_blood == 0x80000000U && stats.attack[0] == base + ((0x10U << 15) >> 12));
    CHECK(Attack(c, 0xffffff00U, &attack) &&
          attack == (Uint16)SDL_max(0, (Sint32)base - 64)); // root0xFFFC0 is word -64
    CHECK(Attack(c, SDL_MAX_UINT32, &attack) && attack == base &&
          SotnCharacter_AddMuramasaBlood(c) && SotnCharacter_Get(c, &state, NULL) &&
          state.muramasa_blood == SDL_MAX_UINT32);
    SotnCharacter_Destroy(c);
    SDL_free(exe);
    SDL_free(dra);
    SDL_Log("SOTN_MURAMASA_TEST: PASS");
    return 0;
}
