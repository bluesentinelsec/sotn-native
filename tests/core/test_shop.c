#include "character_fixture.h"
#include <sotn/shop.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    Uint8 dra[0x8000], lib[0x1600];
    SotnCharacter *character;
    SotnWorldState world;
} Fixture;
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static void Stock(Fixture *f, Uint16 row, Uint8 category, Uint8 level, Uint16 id, Uint32 price)
{
    Uint8 *p = f->lib + 0x134c + row * 8U;
    p[0] = category;
    p[1] = level;
    Put16(p + 2, id);
    TestCharacter_Put32(p + 4, price);
}
static bool Open(Fixture *f)
{
    SDL_zero(*f);
    TestCharacter_Fill(f->dra);
    for (Uint16 i = 0; i < SOTN_SHOP_STOCK; ++i)
        Stock(f, i, 0, 9, 86, 3);
    Stock(f, 0, 5, 255, 0, 7);
    Stock(f, 1, 0, 0, 86, 3);
    Stock(f, 2, 6, 0x80, 0, 5);
    Stock(f, 3, 6, 0x81, 1, 11);
    Stock(f, 4, 3, 1, 49, 13);
    f->dra[0x3400 + 49 * 32 + 28] = 2;
    Put16(f->lib + 0x14d4, SOTN_RELIC_JEWEL);
    for (Uint16 i = 0; i < 5; ++i)
    {
        Put16(f->lib + 0x1510 + i * 2U, (Uint16)(7U - i));
        Put16(f->lib + 0x151c + i * 2U, i);
    }
    for (Uint16 i = 0; i < SOTN_SHOP_GEMS; ++i)
    {
        Uint8 *p = f->lib + 0x14d8 + i * 8U;
        Put16(p, 4);
        Put16(p + 2, (Uint16)(63U + i));
        TestCharacter_Put32(p + 4, 17U + i);
        f->dra[0x3400 + (63U + i) * 32 + 28] = 3;
    }
    TestCharacter_Put32(f->dra + 0xe0, 0x800a6000);
    Uint8 *relic = f->dra + 0x6000 + SOTN_RELIC_JEWEL * 16;
    TestCharacter_Put32(relic, 0x800a6300);
    SDL_memcpy(f->dra + 0x6300, "Synthetic", 10);
    TestCharacter_Put32(f->dra + 0x2d44, 0x800a6200);
    const Uint8 map[] = {0, 1, 1, 1, 0x81, 255};
    SDL_memcpy(f->dra + 0x6200, map, sizeof(map));
    CHECK(SotnCharacter_Create((SotnDataView){f->dra, sizeof(f->dra)}, &f->character));
    SotnCharacterState state = {0};
    CHECK(SotnCharacter_Get(f->character, &state, NULL));
    state.gold = 999;
    return SotnCharacter_Restore(f->character, &state);
}
static bool Catalogue(Fixture *f)
{
    CHECK(Open(f));
    const SotnDataView lib = {f->lib, sizeof(f->lib)};
    SotnShopCatalogue cat = {0};
    CHECK(SotnShop_Catalogue(lib, f->character, &f->world, false, &cat));
    CHECK(cat.count == 4 && cat.unlock == 0 && cat.offers[0].row == 0 &&
          cat.offers[0].available == 1 && cat.offers[1].available == 99);
    f->world.flags[98] = 1;
    CHECK(SotnShop_Catalogue(lib, f->character, &f->world, false, &cat));
    CHECK(cat.count == 5 && cat.unlock == 1 && cat.offers[4].item == 49);
    f->world.flags[180] = f->world.flags[216] = f->world.flags[150] = f->world.flags[220] = 1;
    f->world.relics[0] = 1;
    f->world.time_attack[21] = 101;
    CHECK(SotnShop_Catalogue(lib, f->character, &f->world, false, &cat) && cat.unlock == 7);
    f->world.game_clear = 1;
    CHECK(SotnShop_Catalogue(lib, f->character, &f->world, false, &cat) && cat.unlock == 8);
    const SotnShopCatalogue before = cat;
    f->lib[0x134c] = 7;
    CHECK(!SotnShop_Catalogue(lib, f->character, &f->world, false, &cat));
    CHECK(SDL_memcmp(&cat, &before, sizeof(cat)) == 0);
    f->lib[0x134c] = 5;
    CHECK(
        !SotnShop_Catalogue((SotnDataView){f->lib, 0x1350}, f->character, &f->world, false, &cat));
    CHECK(!SotnShop_Catalogue(lib, NULL, &f->world, false, &cat));
    CHECK(!SotnShop_Catalogue(lib, f->character, NULL, false, &cat));
    SotnCharacter_Destroy(f->character);
    return true;
}
static bool Trades(Fixture *f)
{
    CHECK(Open(f));
    const SotnDataView lib = {f->lib, sizeof(f->lib)}, dra = {f->dra, sizeof(f->dra)};
    SotnCharacterState state = {0}, before = {0};
    SotnShopCatalogue cat = {0};
    CHECK(SotnShop_Trade(lib, dra, f->character, &f->world, false, 1, 2));
    CHECK(SotnCharacter_Get(f->character, &state, NULL) && state.gold == 993 &&
          state.hand_count[86] == 2);
    CHECK(SotnCharacter_Equip(f->character, SOTN_EQUIP_LEFT, 86));
    CHECK(SotnCharacter_Equip(f->character, SOTN_EQUIP_RIGHT, 86));
    CHECK(SotnShop_Trade(lib, dra, f->character, &f->world, false, 1, 97));
    CHECK(SotnCharacter_Get(f->character, &state, NULL) && state.hand_count[86] == 97);
    before = state;
    CHECK(!SotnShop_Trade(lib, dra, f->character, &f->world, false, 1, 1));
    CHECK(!SotnShop_Trade(lib, dra, f->character, &f->world, false, 4, 1));
    CHECK(!SotnShop_Trade(lib, dra, f->character, &f->world, false, 48, 1));
    CHECK(!SotnShop_Trade(lib, dra, f->character, &f->world, false, 1, 0));
    CHECK(SotnCharacter_Get(f->character, &state, NULL) &&
          SDL_memcmp(&state, &before, sizeof(state)) == 0);
    CHECK(SotnShop_Catalogue(lib, f->character, &f->world, false, &cat) &&
          !cat.offers[1].available);
    // An equipped gem is excluded from sales, while both accessories count toward buy limits.
    CHECK(SotnCharacter_Collect(f->character, true, 63));
    CHECK(SotnCharacter_Collect(f->character, true, 63));
    CHECK(SotnCharacter_Equip(f->character, SOTN_EQUIP_RING1, 63));
    CHECK(SotnShop_Catalogue(lib, f->character, &f->world, true, &cat));
    CHECK(cat.count == 7 && cat.offers[0].available == 1);
    CHECK(!SotnShop_Trade(lib, dra, f->character, &f->world, true, 0, 2));
    CHECK(SotnShop_Trade(lib, dra, f->character, &f->world, true, 0, 1));
    CHECK(SotnCharacter_Get(f->character, &state, NULL) && !state.body_count[63] &&
          state.equipment[5] == 63);
    state.gold = 999998;
    state.body_count[63] = 1;
    CHECK(SotnCharacter_Restore(f->character, &state));
    CHECK(SotnShop_Trade(lib, dra, f->character, &f->world, true, 0, 1));
    CHECK(SotnCharacter_Get(f->character, &state, NULL) && state.gold == 999999);
    // Relic and spell one-shots disappear on rebuilding the catalogue.
    CHECK(SotnShop_Trade(lib, dra, f->character, &f->world, false, 0, 1));
    CHECK(f->world.relics[SOTN_RELIC_JEWEL] == 3);
    CHECK(!SotnShop_Trade(lib, dra, f->character, &f->world, false, 0, 1));
    CHECK(SotnShop_Trade(lib, dra, f->character, &f->world, false, 3, 1));
    CHECK(SotnCharacter_Get(f->character, &state, NULL) && state.spells_learned == 128 &&
          state.spells[0] == 0x87);
    CHECK(!SotnShop_Trade(lib, dra, f->character, &f->world, false, 3, 1));
    CHECK(SotnShop_Trade(lib, dra, f->character, &f->world, false, 2, 1));
    CHECK(f->world.flags[SOTN_LIBRARY_MAP] && f->world.map[16] == 128 && f->world.map[17] == 2 &&
          !f->world.rooms);
    CHECK(SotnShop_Catalogue(lib, f->character, &f->world, false, &cat) && cat.count == 1 &&
          cat.offers[0].row == 1);
    // Malformed map and missing relic metadata must never debit gold or partially change world.
    f->world.flags[SOTN_LIBRARY_MAP] = 0;
    f->dra[0x6200 + 4] = 1;
    f->dra[0x6200 + 5] = 64;
    CHECK(SotnCharacter_Get(f->character, &before, NULL));
    const SotnWorldState world = f->world;
    CHECK(!SotnShop_Trade(lib, dra, f->character, &f->world, false, 2, 1));
    CHECK(SotnCharacter_Get(f->character, &state, NULL) &&
          SDL_memcmp(&state, &before, sizeof(state)) == 0);
    CHECK(SDL_memcmp(&f->world, &world, sizeof(world)) == 0);
    f->world.relics[SOTN_RELIC_JEWEL] = 0;
    CHECK(!SotnShop_Trade(lib, (SotnDataView){0}, f->character, &f->world, false, 0, 1));
    CHECK(SotnCharacter_Get(f->character, &state, NULL) && state.gold == before.gold);
    state.gold = 0;
    state.hand_count[86] = 0;
    CHECK(SotnCharacter_Restore(f->character, &state));
    CHECK(!SotnShop_Trade(lib, dra, f->character, &f->world, false, 1, 1));
    SotnCharacter_Destroy(f->character);
    return true;
}
static bool Reveal(void)
{
    SotnWorldState w = {0};
    CHECK(SotnWorld_Visit(&w, 0, 0, false));
    const Uint8 stream[] = {0, 0, 1, 1, 0x81, 255};
    CHECK(SotnWorld_Reveal(&w, stream, sizeof(stream), false));
    CHECK(w.rooms == 1 && w.map[0] == 192 && w.map[1] == 2 && SotnWorld_Validate(&w));
    CHECK(SotnWorld_Reveal(&w, stream, sizeof(stream), true));
    CHECK(w.map[1024] == 128 && w.map[1025] == 2 && w.rooms == 1);
    const SotnWorldState before = w;
    CHECK(!SotnWorld_Reveal(&w, stream, sizeof(stream) - 1, false));
    CHECK(SDL_memcmp(&w, &before, sizeof(w)) == 0);
    const Uint8 invalid[] = {63, 0, 1, 1, 255, 255};
    CHECK(!SotnWorld_Reveal(&w, invalid, sizeof(invalid), false));
    CHECK(!SotnWorld_Reveal(NULL, stream, sizeof(stream), false));
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Catalogue(f) && Trades(f) && Reveal();
    SDL_free(f);
    if (ok)
        SDL_Log("SOTN_SHOP_TEST: PASS");
    return ok ? 0 : 1;
}
