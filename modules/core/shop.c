// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected LIB e_shop.c catalogue/transaction rules and dra/5D5BC.c spell learning,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Stock, prices and maps stay private.
#include <sotn/relic.h>
#include <sotn/shop.h>
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static Uint32 Read32(const Uint8 *p)
{
    return p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
}
static bool View(SotnDataView image, size_t offset, size_t size, const Uint8 **out)
{
    if (!image.bytes || offset > image.size || size > image.size - offset)
        return SDL_SetError("Shop metadata exceeds its private image bounds");
    *out = image.bytes + offset;
    return true;
}
static Uint16 Unlock(const SotnWorldState *w)
{
    const Uint16 level =
        (Uint16)((w->flags[SOTN_MARIA_HIPPOGRYPH] != 0) +
                 (w->flags[SOTN_RICHTER_AFTER_MINOTAUR] != 0) +
                 ((w->relics[SOTN_RELIC_BAT] & 1U) != 0) + (w->flags[SOTN_MARIA_CENTER] != 0) +
                 (w->flags[SOTN_INVERTED_UNLOCKED] != 0) + (w->flags[SOTN_DEATH_FIGHT] != 0) +
                 (w->time_attack[SOTN_TIME_GALAMOTH] != 0));
    return w->game_clear ? 8 : level;
}
static bool Offer(SotnDataView lib, const SotnCharacter *c, const SotnCharacterState *s,
                  const SotnWorldState *w, bool sell, Uint16 row, SotnShopOffer *out,
                  bool *eligible)
{
    const Uint8 *p = NULL;
    if (row >= (sell ? SOTN_SHOP_GEMS : SOTN_SHOP_STOCK) ||
        !View(lib, (sell ? 0x14d8U : 0x134cU) + (size_t)row * 8, 8, &p))
        return SDL_SetError("Shop row exceeds the original stock table");
    SotnShopOffer result = {.row = row,
                            .category = sell ? Read16(p) : p[0],
                            .item = Read16(p + 2),
                            .price = Read32(p + 4)};
    Uint16 level = p[1];
    if (result.category > 6 || result.price > 999999 || (sell && result.category != 4))
        return SDL_SetError("Shop stock has an invalid category or unit price");
    if (result.category < 5)
    {
        const bool body = result.category != 0;
        SotnItem item = {0};
        if (!SotnCharacter_Item(c, body, result.item, &item))
            return false;
        if (body && item.type != result.category - 1U)
            return SDL_SetError("Shop equipment category disagrees with its private definition");
        Uint16 owned = body ? s->body_count[result.item] : s->hand_count[result.item];
        if (sell)
            result.available = owned;
        else
        {
            const Uint16 first = body ? 2 : 0, end = body ? 7 : 2;
            for (Uint16 i = first; i < end; ++i)
                if (s->equipment[i] == result.item)
                    ++owned;
            result.available = owned >= 99 ? 0 : (Uint16)(99U - owned);
        }
    }
    else if (result.category == 5)
    {
        const Uint8 *mapping = NULL;
        if (result.item != 0 || !View(lib, 0x14d4, 2, &mapping) || Read16(mapping) >= SOTN_RELICS)
            return SDL_SetError("Shop relic mapping is invalid");
        result.available = (w->relics[Read16(mapping)] & 1U) ? 0 : 1;
        if (level == 255 && result.available)
            level = 0;
    }
    else
    {
        result.available = 1;
        if (result.item == 0)
        {
            if (w->flags[SOTN_LIBRARY_MAP])
                result.available = 0;
            if (level == 0x80 && result.available)
                level = 0;
        }
        else
        {
            const Uint8 *mapping = NULL, *levels = NULL;
            if (result.item > 5 || !View(lib, 0x1510, 10, &mapping) ||
                !View(lib, 0x151c, 10, &levels) ||
                Read16(mapping + (result.item - 1U) * 2U) >= SOTN_SPELLS)
                return SDL_SetError("Shop spell document mapping is invalid");
            if (s->spells_learned & (1U << Read16(mapping + (result.item - 1U) * 2U)))
                result.available = 0;
            if (level >= 0x81 && level <= 0x85)
            {
                if (level - 0x80U != result.item)
                    return SDL_SetError("Shop spell unlock selector disagrees with document ID");
                level = result.available ? Read16(levels + (result.item - 1U) * 2U) : 255;
            }
        }
    }
    *eligible = sell || level <= Unlock(w);
    *out = result;
    return true;
}
bool SotnShop_Catalogue(SotnDataView lib, const SotnCharacter *c, const SotnWorldState *w,
                        bool sell, SotnShopCatalogue *out)
{
    SotnCharacterState state = {0};
    if (!out)
        return SDL_InvalidParamError("out");
    if (!SotnWorld_Validate(w) || !SotnCharacter_Get(c, &state, NULL))
        return false;
    SotnShopCatalogue result = {.unlock = Unlock(w)};
    for (Uint16 row = 0; row < (sell ? SOTN_SHOP_GEMS : SOTN_SHOP_STOCK); ++row)
    {
        SotnShopOffer offer = {0};
        bool eligible = false;
        if (!Offer(lib, c, &state, w, sell, row, &offer, &eligible))
            return false;
        if (eligible)
            result.offers[result.count++] = offer;
    }
    *out = result;
    return true;
}
bool SotnShop_Trade(SotnDataView lib, SotnDataView dra, SotnCharacter *c, SotnWorldState *w,
                    bool sell, Uint16 row, Uint16 quantity)
{
    SotnCharacterState state = {0};
    SotnShopOffer offer = {0};
    bool eligible = false;
    if (!quantity || quantity > 99)
        return SDL_SetError("Shop trade quantity must be 1..99");
    if (!SotnWorld_Validate(w) || !SotnCharacter_Get(c, &state, NULL) ||
        !Offer(lib, c, &state, w, sell, row, &offer, &eligible))
        return false;
    if (!eligible || quantity > offer.available)
        return SDL_SetError("Shop offer is unavailable or quantity exceeds owned stock");
    if (offer.category < 5)
        return SotnCharacter_Trade(c, offer.category != 0, offer.item, quantity, offer.price, sell);
    if (sell || quantity != 1 || state.gold < offer.price)
        return SDL_SetError("Shop document or relic purchase exceeds available gold or quantity");
    SotnWorldState next = *w;
    const Uint8 *mapping = NULL;
    if (offer.category == 5)
    {
        if (!View(lib, 0x14d4, 2, &mapping) || !SotnRelic_Collect(&next, dra, Read16(mapping)))
            return false;
    }
    else if (offer.item)
    {
        if (!View(lib, 0x1510, 10, &mapping) ||
            !SotnCharacter_LearnSpell(&state, Read16(mapping + (offer.item - 1U) * 2U)))
            return false;
    }
    else
    {
        const Uint8 *pointer = NULL, *stream = NULL;
        if (!View(dra, 0x2d44, 4, &pointer))
            return false;
        const Uint32 address = Read32(pointer);
        if (address < 0x800a0000 || !View(dra, address - 0x800a0000U, 1, &stream) ||
            !SotnWorld_Reveal(&next, stream, dra.size - (size_t)(stream - dra.bytes), false))
            return SDL_SetError("Shop castle-map purchase references invalid private rectangles");
        next.flags[SOTN_LIBRARY_MAP] = 1;
    }
    state.gold -= offer.price;
    if (!SotnCharacter_Restore(c, &state))
        return false;
    *w = next;
    return true;
}
