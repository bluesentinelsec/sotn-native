#ifndef SOTN_SHOP_H
#define SOTN_SHOP_H
#include <sotn/character.h>
#include <sotn/world.h>
enum
{
    SOTN_SHOP_STOCK = 48,
    SOTN_SHOP_GEMS = 7
};
/** @brief One decoded original offer; text and artwork remain in private runtime resources. */
typedef struct SotnShopOffer
{
    Uint16 row, category, item, available;
    Uint32 price;
} SotnShopOffer;
/** @brief Fixed-capacity catalogue in original stock order, without owned allocations. */
typedef struct SotnShopCatalogue
{
    SotnShopOffer offers[SOTN_SHOP_STOCK];
    Uint16 count, unlock;
} SotnShopCatalogue;
/** @brief Decode the current original buy or seven-gem sell list.
 * @param library Borrowed verified LIB overlay. @param character Borrowed progress owner.
 * @param world Borrowed validated progress. @param sell Select gem sales.
 * @param out Receives catalogue only on success.
 * @return Success or SDL error for malformed private records/state. No state is changed. */
bool SotnShop_Catalogue(SotnDataView library, const SotnCharacter *character,
                        const SotnWorldState *world, bool sell, SotnShopCatalogue *out);
/** @brief Commit a selected original stock-row trade atomically across character and world.
 * @param library Borrowed verified LIB. @param dra Borrowed verified DRA, same image as character.
 * @param character Owned progress. @param world Owned progress. @param sell Select gem sale.
 * @param row Original buy-table row 0..47 or sell-table row 0..6, not filtered cursor index.
 * @param quantity Copies 1..99. @return Success or SDL error, leaving all state unchanged on error.
 * @note Implements ownership/learning, not missing equipment effects or spell casting. */
bool SotnShop_Trade(SotnDataView library, SotnDataView dra, SotnCharacter *character,
                    SotnWorldState *world, bool sell, Uint16 row, Uint16 quantity);
#endif
