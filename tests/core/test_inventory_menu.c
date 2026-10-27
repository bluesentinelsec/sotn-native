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
    SotnWorldState world = {0};
    const SotnDataView dra = {bytes, 0x4000};
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_START, &sound, &changed, &world, dra) &&
          m.phase == SOTN_MENU_MAIN);
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_START, &sound, &changed, &world, dra) &&
          m.phase == SOTN_MENU_MAIN);
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_JUMP, &sound, &changed, &world, dra) &&
          m.phase == SOTN_MENU_EQUIPMENT);
    for (Uint32 i = 0; i < 3; ++i)
    {
        CHECK(SotnInventoryMenu_Tick(&m, c, 0, &sound, &changed, &world, dra));
        CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_DOWN, &sound, &changed, &world, dra));
    }
    CHECK(m.slot == 3);
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_JUMP, &sound, &changed, &world, dra));
    CHECK(m.phase == SOTN_MENU_ITEMS && m.count == 2 && m.rows[1] == 1);
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_DOWN, &sound, &changed, &world, dra));
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_JUMP, &sound, &changed, &world, dra));
    CHECK(changed && m.phase == SOTN_MENU_EQUIPMENT);
    SotnCharacterState s = {0};
    SotnCharacterStats stats = {0};
    CHECK(SotnCharacter_Get(c, &s, &stats) && s.equipment[3] == 1 && s.body_count[1] == 0 &&
          stats.defense == 3);
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_CANCEL, &sound, &changed, &world, dra) &&
          m.phase == SOTN_MENU_MAIN);
    CHECK(SotnInventoryMenu_Tick(&m, c, 0, &sound, &changed, &world, dra));
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_DOWN, &sound, &changed, &world, dra));
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_JUMP, &sound, &changed, &world, dra) &&
          m.phase == SOTN_MENU_MAIN && m.message[0] != '\0');
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_START, &sound, &changed, &world, dra) &&
          m.phase == SOTN_MENU_CLOSED);
    m.slot = 7;
    CHECK(!SotnInventoryMenu_Tick(&m, c, 0, &sound, &changed, &world, dra));
    /* Synthetic private relic definitions; no game text or records. */
    TestCharacter_Put32(bytes + 0xe0, 0x800a0200);
    bytes[0x100] = 'R';
    bytes[0x101] = 0;
    for (Uint32 i = 0; i < SOTN_RELICS; ++i)
        TestCharacter_Put32(bytes + 0x200 + i * 16, 0x800a0100);
    m = (SotnInventoryMenu){.phase = SOTN_MENU_MAIN, .cursor = 2};
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_JUMP, &sound, &changed, &world, dra) &&
          m.phase == SOTN_MENU_RELICS && !changed);
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_UP, &sound, &changed, &world, dra) &&
          m.relic_cursor == 0);
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_RIGHT, &sound, &changed, &world, dra) &&
          m.relic_cursor == 1);
    for (Uint32 i = 0; i < 13; ++i)
    {
        CHECK(SotnInventoryMenu_Tick(&m, c, 0, &sound, &changed, &world, dra));
        CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_DOWN, &sound, &changed, &world, dra));
    }
    CHECK(m.relic_cursor == 27);
    CHECK(SotnInventoryMenu_Tick(&m, c, 0, &sound, &changed, &world, dra));
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_DOWN, &sound, &changed, &world, dra) &&
          m.relic_cursor == 27);
    /* Every US cursor addresses its own identity, skipping the Japanese cards. */
    for (Uint8 cursor = 0; cursor < 28; ++cursor)
    {
        m.relic_cursor = cursor;
        m.previous = 0;
        m.message[0] = '\0';
        const Uint16 id = cursor < 23 ? cursor : (Uint16)(cursor + 2);
        world.relics[id] = 1;
        CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_JUMP, &sound, &changed, &world, dra));
        CHECK(changed == (id == 0 || id == 7 || id == 8 || id == 9 || id == 10 || id == 12 ||
                          id == 13 || id == 16));
        CHECK(world.relics[id] == (changed ? 3 : 1));
        CHECK((m.message[0] == '\0') == changed);
        const SotnWorldState before = world;
        CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_JUMP, &sound, &changed, &world, dra) &&
              !changed && SDL_memcmp(&world, &before, sizeof(world)) == 0);
    }
    CHECK(world.relics[23] == 0 && world.relics[24] == 0);
    m.relic_cursor = 12;
    m.previous = 0;
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_JUMP, &sound, &changed, &world, dra) &&
          changed && world.relics[12] == 1);
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_CANCEL, &sound, &changed, &world, dra) &&
          m.phase == SOTN_MENU_MAIN && m.relic_cursor == 12);
    CHECK(SotnInventoryMenu_Tick(&m, c, SOTN_BUTTON_JUMP, &sound, &changed, &world, dra) &&
          m.phase == SOTN_MENU_RELICS && m.relic_cursor == 12);
    m.relic_cursor = 28;
    CHECK(!SotnInventoryMenu_Tick(&m, c, 0, &sound, &changed, &world, dra));
    m.relic_cursor = 0;
    CHECK(!SotnInventoryMenu_Tick(&m, c, 0, &sound, &changed, NULL, dra));
    SotnCharacter_Destroy(c);
    SDL_free(bytes);
    SDL_Log("SOTN_INVENTORY_MENU_TEST: PASS");
    return 0;
}
