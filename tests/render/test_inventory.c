#include "../core/character_fixture.h"
#include <sotn/alucard_sprite.h>
#include <sotn/inventory.h>
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
    Uint8 *dra = SDL_calloc(1, 0x3b0d4), *common = SDL_calloc(1, 0x42000),
          *rgba = SDL_calloc(1, SOTN_MENU_RGBA_BYTES);
    Uint16 *frame = SDL_calloc(SOTN_FRAME_PIXELS, sizeof(*frame));
    CHECK(dra && common && rgba && frame);
    TestCharacter_Fill(dra);
    TestCharacter_Progression(dra);
    SDL_memset(dra + 0x25324, 0x11, 128);
    dra[0x388d4 + 2] = 31;
    for (Uint32 i = 0; i < 4096; ++i)
    {
        common[0x40000 + i * 2] = 255;
        common[0x40000 + i * 2 + 1] = 127;
    }
    SotnCharacter *c = NULL;
    SotnInventoryRenderer *r = NULL;
    const SotnDataView image = {dra, 0x3b0d4};
    CHECK(SotnCharacter_Create(image, &c));
    CHECK(SotnInventoryRenderer_Create(image, (SotnDataView){common, 0x42000}, &r));
    SotnWorldState world = {0};
    SotnInventoryMenu m = {.phase = SOTN_MENU_MAIN};
    CHECK(SotnInventoryRenderer_Draw(r, m, c, 0, false, rgba, SOTN_MENU_RGBA_BYTES, &world));
    CHECK(rgba[(8 * 384) * 4] == 255 && rgba[3] == 255);
    m.phase = SOTN_MENU_EQUIPMENT;
    CHECK(SotnInventoryRenderer_Draw(r, m, c, 0, true, rgba, SOTN_MENU_RGBA_BYTES, &world));
    CHECK(!SotnInventoryRenderer_Draw(r, m, c, 0, true, rgba, 1, &world));
    m.slot = 7;
    CHECK(!SotnInventoryRenderer_Draw(r, m, c, 0, true, rgba, SOTN_MENU_RGBA_BYTES, &world));
    m.slot = 0;
    m.phase = SOTN_MENU_RELICS;
    TestCharacter_Put32(dra + 0xe0, 0x800a0200);
    dra[0x100] = 'R';
    dra[0x101] = 0;
    for (Uint32 i = 0; i < SOTN_RELICS; ++i)
    {
        TestCharacter_Put32(dra + 0x200 + i * 16, 0x800a0100);
        world.relics[i] = 1;
    }
    world.relics[12] = 3;
    for (Uint8 cursor = 0; cursor < 28; ++cursor)
    {
        m.relic_cursor = cursor;
        CHECK(SotnInventoryRenderer_Draw(r, m, c, 0, true, rgba, SOTN_MENU_RGBA_BYTES, &world));
    }
    m.relic_cursor = 28;
    CHECK(!SotnInventoryRenderer_Draw(r, m, c, 0, true, rgba, SOTN_MENU_RGBA_BYTES, &world));
    m.relic_cursor = 0;
    world.relics[12] = 2;
    rgba[0] = 123;
    CHECK(!SotnInventoryRenderer_Draw(r, m, c, 0, true, rgba, SOTN_MENU_RGBA_BYTES, &world) &&
          rgba[0] == 123);
    world.relics[12] = 3;
    TestCharacter_Put32(dra + 0x200, 0xfffffff0);
    CHECK(!SotnInventoryRenderer_Draw(r, m, c, 0, true, rgba, SOTN_MENU_RGBA_BYTES, &world) &&
          rgba[0] == 123);
    SotnItem item = {0};
    CHECK(SotnItem_Read(image, false, 0, &item));
    CHECK(SotnItemIcon_Draw(image, item, -8, -8, frame, SOTN_FRAME_PIXELS) && frame[0] == 31);
    item.icon = 320;
    frame[0] = 123;
    CHECK(!SotnItemIcon_Draw(image, item, 0, 0, frame, SOTN_FRAME_PIXELS) && frame[0] == 123);
    CHECK(!SotnItemIcon_Draw(image, item, SDL_MIN_SINT32, 0, frame, SOTN_FRAME_PIXELS));
    TestCharacter_Put32(dra + 0x2fbc, 48);
    TestCharacter_Put32(dra + 0x2fc0, 0x409);
    TestCharacter_Put32(dra + 0x3bb8 + 9 * 4, 0x800a3500);
    TestCharacter_Put32(dra + 0x3500, 1);
    TestCharacter_Put32(dra + 0x3504, 0x1006);
    TestCharacter_Put32(dra + 0x3508, 4);
    TestCharacter_Put32(dra + 0x350c, 0x800a3600);
    SDL_memset(dra + 0x3600, 0, 8);
    dra[0x3600] = 31;
    dra[0x3602] = 7;
    Uint16 colors[4] = {0};
    CHECK(SotnAlucard_CapePalette(image, 48, colors) && colors[0] == 31 && colors[1] == 7);
    TestCharacter_Put32(dra + 0x350c, 0xfffffffe);
    CHECK(!SotnAlucard_CapePalette(image, 48, colors) && colors[0] == 31);
    TestCharacter_Put32(dra + 0x3bb8 + 21 * 4, 0x800a3700);
    TestCharacter_Put32(dra + 0x3700, 0x20004);
    TestCharacter_Put32(dra + 0x3704, 0x1006);
    TestCharacter_Put32(dra + 0x3708, 2);
    TestCharacter_Put32(dra + 0x370c, 0x800a3800);
    TestCharacter_Put32(dra + 0x3710, 0x800a3810);
    TestCharacter_Put32(dra + 0x3714, SDL_MAX_UINT32);
    dra[0x3800] = 3;
    dra[0x3801] = 0;
    dra[0x3802] = 1;
    dra[0x3803] = 0;
    SDL_memset(dra + 0x3810, 0, 4);
    colors[0] = colors[1] = 0;
    Uint16 target = 0;
    Uint8 delay = 0;
    CHECK(SotnAlucard_TwilightTick(image, colors, &target, &delay) && colors[0] == 1 && delay == 1);
    CHECK(SotnAlucard_TwilightTick(image, colors, &target, &delay) && colors[0] == 1 && delay == 0);
    for (Uint32 i = 0; i < 5; ++i)
        CHECK(SotnAlucard_TwilightTick(image, colors, &target, &delay));
    CHECK(colors[0] == 3 && colors[1] == 1 && target == 1);
    CHECK(SotnAlucard_TwilightTick(image, colors, &target, &delay));
    CHECK(SotnAlucard_TwilightTick(image, colors, &target, &delay) && colors[0] == 2);
    TestCharacter_Put32(dra + 0x3710, 0xfffffff0);
    const Uint16 retained = colors[0];
    CHECK(!SotnAlucard_TwilightTick(image, colors, &target, &delay) && colors[0] == retained);
    SotnInventoryRenderer_Destroy(r);
    SotnCharacter_Destroy(c);
    SDL_free(dra);
    SDL_free(common);
    SDL_free(rgba);
    SDL_free(frame);
    SDL_Log("SOTN_INVENTORY_RENDER_TEST: PASS");
    return 0;
}
