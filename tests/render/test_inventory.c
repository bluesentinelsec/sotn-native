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
    SotnInventoryMenu m = {.phase = SOTN_MENU_MAIN};
    CHECK(SotnInventoryRenderer_Draw(r, m, c, 0, false, rgba, SOTN_MENU_RGBA_BYTES));
    CHECK(rgba[(8 * 384) * 4] == 255 && rgba[3] == 255);
    m.phase = SOTN_MENU_EQUIPMENT;
    CHECK(SotnInventoryRenderer_Draw(r, m, c, 0, true, rgba, SOTN_MENU_RGBA_BYTES));
    CHECK(!SotnInventoryRenderer_Draw(r, m, c, 0, true, rgba, 1));
    m.slot = 7;
    CHECK(!SotnInventoryRenderer_Draw(r, m, c, 0, true, rgba, SOTN_MENU_RGBA_BYTES));
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
    dra[0x3600] = 31;
    dra[0x3602] = 7;
    Uint16 colors[4] = {0};
    CHECK(SotnAlucard_CapePalette(image, 48, colors) && colors[0] == 31 && colors[1] == 7);
    TestCharacter_Put32(dra + 0x350c, 0xfffffffe);
    CHECK(!SotnAlucard_CapePalette(image, 48, colors) && colors[0] == 31);
    SotnInventoryRenderer_Destroy(r);
    SotnCharacter_Destroy(c);
    SDL_free(dra);
    SDL_free(common);
    SDL_free(rgba);
    SDL_free(frame);
    SDL_Log("SOTN_INVENTORY_RENDER_TEST: PASS");
    return 0;
}
