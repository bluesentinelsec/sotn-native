#include "character_fixture.h"
#include <sotn/inventory_menu.h>
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
    Uint8 *bytes = SDL_calloc(1, 0x4000);
    CHECK(bytes != NULL);
    TestCharacter_Fill(bytes);
    SotnCharacter *c = NULL;
    CHECK(SotnCharacter_Create((SotnDataView){bytes, 0x4000}, &c));
    CHECK(SotnCharacter_Collect(c, true, 1));
    SotnInventoryMenu m = {0};
    Uint16 sound = 0;
    bool changed = false;
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_START, &sound, &changed) &&
          m.phase == SOTN_MENU_MAIN);
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_START, &sound, &changed) &&
          m.phase == SOTN_MENU_MAIN);
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_JUMP, &sound, &changed) &&
          m.phase == SOTN_MENU_EQUIPMENT);
    for (Uint32 i = 0; i < 3; ++i)
    {
        CHECK(SotnInventoryMenu_Tick(&m, c, 0, &sound, &changed));
        CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_DOWN, &sound, &changed));
    }
    CHECK(m.slot == 3);
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_JUMP, &sound, &changed));
    CHECK(m.phase == SOTN_MENU_ITEMS && m.count == 2 && m.rows[1] == 1);
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_DOWN, &sound, &changed));
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_JUMP, &sound, &changed));
    CHECK(changed && m.phase == SOTN_MENU_EQUIPMENT);
    SotnCharacterState s = {0};
    SotnCharacterStats stats = {0};
    CHECK(SotnCharacter_Get(c, &s, &stats) && s.equipment[3] == 1 && s.body_count[1] == 0 &&
          stats.defense == 3);
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_CANCEL, &sound, &changed) &&
          m.phase == SOTN_MENU_MAIN);
    CHECK(SotnInventoryMenu_Tick(&m, c, 0, &sound, &changed));
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_DOWN, &sound, &changed));
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_JUMP, &sound, &changed) &&
          m.phase == SOTN_MENU_MAIN && m.message[0] != '\0');
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_START, &sound, &changed) &&
          m.phase == SOTN_MENU_CLOSED);
    m.slot = 7;
    CHECK(!SotnInventoryMenu_Tick(&m, c, 0, &sound, &changed));
    SotnCharacter_Destroy(c);
    SDL_free(bytes);
    SDL_Log("SOTN_INVENTORY_MENU_TEST: PASS");
    return 0;
}
