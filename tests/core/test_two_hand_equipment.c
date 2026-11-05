#include "character_fixture.h"
#include <sotn/character.h>
#include <sotn/inventory_menu.h>
#include <sotn/world.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static void Definition(Uint8 *dra, Uint16 id, Uint8 category, Sint16 attack)
{
    Uint8 *p = dra + 0x500 + id * 52U;
    Put16(p + 8, (Uint16)attack);
    p[14] = category;
    p[17] = 1;
    p[18] = 0x41;
    p[19] = 48;
    p[21] = id == 84 ? 18 : 13;
    p[22] = 128;
    Put16(p + 40, 2);
}
int main(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    CHECK(dra);
    TestCharacter_Fill(dra);
    Definition(dra, 84, 5, -5);
    Definition(dra, 86, 1, 3);
    Definition(dra, 122, 1, 19);
    Uint8 *shield = dra + 0x500 + 16U * 52U;
    shield[14] = 9;
    SotnCharacter *c = NULL, *restored = NULL;
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &c));
    CHECK(SotnCharacter_Collect(c, false, 86) && SotnCharacter_Equip(c, 0, 86) &&
          SotnCharacter_Collect(c, false, 16) && SotnCharacter_Equip(c, 1, 16) &&
          SotnCharacter_Collect(c, false, 84) && SotnCharacter_ConfirmEquipment(c, 0, 84));
    SotnCharacterState p = {0}, before = {0};
    SotnCharacterStats stats = {0};
    CHECK(SotnCharacter_Get(c, &p, &stats) && p.equipment[0] == 84 && p.equipment[1] == 84 &&
          !p.hand_count[84] && !p.hand_count[0] && p.hand_order[0] == 0 && p.hand_count[86] == 1 &&
          p.hand_count[16] == 1 && stats.attack[0] == 1 && stats.attack[1] == 1);
    before = p;
    CHECK(!SotnCharacter_ConfirmEquipment(c, 1, 84) && SotnCharacter_Get(c, &p, NULL) &&
          !SDL_memcmp(&p, &before, sizeof(p)) && SotnCharacter_Equip(c, 1, 84));
    CHECK(SotnCharacter_Collect(c, false, 122) && SotnCharacter_ConfirmEquipment(c, 1, 122) &&
          SotnCharacter_Get(c, &p, NULL) && !p.equipment[0] && p.equipment[1] == 122 &&
          p.hand_count[84] == 1 && !p.hand_count[122]);
    CHECK(SotnCharacter_ConfirmEquipment(c, 1, 84) && SotnCharacter_Get(c, &p, NULL) &&
          p.equipment[0] == 84 && p.equipment[1] == 84 && !p.hand_count[84] &&
          p.hand_count[122] == 1);
    before = p;
    before.equipment[1] = 0;
    CHECK(!SotnCharacter_Validate(&before) && !SotnCharacter_Restore(c, &before) &&
          SotnCharacter_Get(c, &before, NULL) && !SDL_memcmp(&p, &before, sizeof(p)));
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &restored) &&
          SotnCharacter_Restore(restored, &p) && SotnCharacter_Get(restored, &before, &stats) &&
          !SDL_memcmp(&p, &before, sizeof(p)) && stats.attack[0] == 1 && stats.attack[1] == 1);
    // Real confirmations consume then return; an unchecked byte above99 must not be clamped.
    p.hand_count[84] = 255;
    CHECK(SotnCharacter_Restore(c, &p) && SotnCharacter_Equip(c, 0, 84) &&
          SotnCharacter_Get(c, &before, NULL) && before.hand_count[84] == 255);
    SotnInventoryMenu menu = {.phase = SOTN_MENU_ITEMS, .count = 1, .rows = {84}};
    SotnWorldState world = {0};
    Uint16 sound = 0;
    bool changed = false;
    CHECK(SotnInventoryMenu_Tick(&menu, c, SOTN_BUTTON_JUMP, &sound, &changed, &world,
                                 (SotnDataView){dra, 0x11000}) &&
          changed && sound && menu.phase == SOTN_MENU_EQUIPMENT && SotnCharacter_Get(c, &p, NULL) &&
          p.equipment[0] == 84 && p.equipment[1] == 84 && p.hand_count[84] == 254);
    CHECK(SotnCharacter_ConfirmEquipment(c, 1, 0) && SotnCharacter_Get(c, &p, NULL) &&
          !p.equipment[0] && !p.equipment[1] && p.hand_count[84] == 254);
    // Original shop capacity deliberately counts the two actual equipped words.
    p.equipment[0] = p.equipment[1] = 84;
    p.hand_count[84] = 97;
    p.gold = 100;
    CHECK(SotnCharacter_Restore(c, &p) && !SotnCharacter_Trade(c, false, 84, 1, 1, false));
    p.hand_count[84] = 96;
    CHECK(SotnCharacter_Restore(c, &p) && SotnCharacter_Trade(c, false, 84, 1, 1, false) &&
          SotnCharacter_Get(c, &p, NULL) && p.hand_count[84] == 97 && p.gold == 99 &&
          p.equipment[0] == 84 && p.equipment[1] == 84);
    CHECK(SotnCharacter_Trade(c, false, 84, 1, 1, true) && SotnCharacter_Get(c, &p, NULL) &&
          p.hand_count[84] == 96 && p.equipment[0] == 84 && p.equipment[1] == 84);
    // A late metadata failure must preserve both slots, stock, ordering and derived stats.
    CHECK(SotnCharacter_ConfirmEquipment(c, 0, 86) && SotnCharacter_Get(c, &before, NULL));
    TestCharacter_Put32(dra + 0x3400, 0xdeadbeef);
    CHECK(!SotnCharacter_ConfirmEquipment(c, 1, 84) && SotnCharacter_Get(c, &p, NULL) &&
          !SDL_memcmp(&p, &before, sizeof(p)));
    // Takemitsu27 shares the same one-copy paired reservation and restore validation.
    TestCharacter_Fill(dra);
    Definition(dra, 27, 5, 22);
    Definition(dra, 86, 1, 3);
    SotnCharacter *takemitsu = NULL;
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &takemitsu) &&
          SotnCharacter_Collect(takemitsu, false, 86) && SotnCharacter_Equip(takemitsu, 0, 86) &&
          SotnCharacter_Collect(takemitsu, false, 27) &&
          SotnCharacter_ConfirmEquipment(takemitsu, 1, 27) &&
          SotnCharacter_Get(takemitsu, &p, NULL) && p.equipment[0] == 27 && p.equipment[1] == 27 &&
          !p.hand_count[27] && p.hand_count[86] == 1);
    before = p;
    before.equipment[0] = 86;
    CHECK(!SotnCharacter_Validate(&before) && !SotnCharacter_Restore(takemitsu, &before));
    SotnCharacter_Destroy(takemitsu);
    SotnCharacter_Destroy(restored);
    SotnCharacter_Destroy(c);
    SDL_free(dra);
    return 0;
}
