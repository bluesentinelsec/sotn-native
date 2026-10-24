#ifndef SOTN_CHARACTER_H
#define SOTN_CHARACTER_H
#include <sotn/game_data.h>

enum
{
    SOTN_HAND_ITEMS = 169,
    SOTN_BODY_ITEMS = 90,
    SOTN_EQUIP_SLOTS = 7
};
/** @brief Original equipment slots; inventory counts exclude currently equipped copies. */
typedef enum SotnEquipSlot
{
    SOTN_EQUIP_LEFT,
    SOTN_EQUIP_RIGHT,
    SOTN_EQUIP_HEAD,
    SOTN_EQUIP_ARMOR,
    SOTN_EQUIP_CAPE,
    SOTN_EQUIP_RING1,
    SOTN_EQUIP_RING2
} SotnEquipSlot;
/** @brief Ordinary character progress, independently serializable without pointers. */
typedef struct SotnCharacterState
{
    Uint16 hp, hp_max, mp, mp_max, hearts, hearts_max, level, base[4], equipment[7];
    Uint32 experience, gold, kills, random;
    Uint8 hand_count[SOTN_HAND_ITEMS], body_count[SOTN_BODY_ITEMS];
    Uint8 hand_order[SOTN_HAND_ITEMS], body_order[SOTN_BODY_ITEMS];
    Uint8 laboratory_items[2]; /**< Original NZ0 persistent item indices, not transient candles. */
} SotnCharacterState;
/** @brief Decoded private equipment record; text views expire with the source DRA image. */
typedef struct SotnItem
{
    SotnDataView name, description;
    Sint16 attack, defense, bonus[4];
    Uint16 id, type, weakness, resistance, immunity, absorption, icon, palette;
    bool body, supported;
} SotnItem;
/** @brief Derived unbuffed attributes; conditional effects not integrated are rejected at equip. */
typedef struct SotnCharacterStats
{
    Uint16 total[4], attack[2], defense, weakness, resistance, immunity, absorption;
    Sint16 bonus[4];
} SotnCharacterStats;
/** @brief Owned character borrowing immutable DRA bytes until Destroy. */
typedef struct SotnCharacter SotnCharacter;
/** @brief Read a bounded original item definition and encoded name/description.
 * @param dra Borrowed image. @param body Select body versus hand table.
 * @param id Original item ID within that table. @param out Unchanged on failure.
 * @return Success or false with SDL_GetError(). Unsupported behaviors are marked explicitly. */
bool SotnItem_Read(SotnDataView dra, bool body, Uint16 id, SotnItem *out);
/** @brief Copy a private encoded name to native UTF-8/ASCII for logs and menus.
 * @param item Borrowed decoded item. @param out Writable buffer. @param capacity Buffer size.
 * @return Success or false with SDL_GetError(); no unchecked private string scans. */
bool SotnItem_Name(SotnItem item, char *out, size_t capacity);
/** @brief Allocate the explicit Laboratory profile: level 1, 70 HP, 20 MP, stats 6.
 * @param dra Borrowed image. @param out Owned result, NULL on failure.
 * @return Success or false with SDL_GetError(). No original new-game entry is implied. */
bool SotnCharacter_Create(SotnDataView dra, SotnCharacter **out);
/** @brief Copy progress and derived stats. Either output can be NULL, but not both.
 * @param character Borrowed character. @param state Progress output. @param stats Derived output.
 * @return Success or false with SDL_GetError(), leaving outputs unchanged on failure. */
bool SotnCharacter_Get(const SotnCharacter *character, SotnCharacterState *state,
                       SotnCharacterStats *stats);
/** @brief Validate structural bounds independently of private item metadata.
 * @param state Borrowed progress. @return Success or false with SDL_GetError(). */
bool SotnCharacter_Validate(const SotnCharacterState *state);
/** @brief Restore validated supported progress and recalculate attributes atomically.
 * @param character Borrowed character. @param state Saved progress.
 * @return Success or false with SDL_GetError() and unchanged character on invalid input. */
bool SotnCharacter_Restore(SotnCharacter *character, const SotnCharacterState *state);
/** @brief Add an acquired original item, cap stock at 99, and retain acquisition order.
 * @param character Borrowed character. @param body Item table. @param id Bounded item ID.
 * @return Success or false with SDL_GetError(); unsupported items can be owned, never misused. */
bool SotnCharacter_Collect(SotnCharacter *character, bool body, Uint16 id);
/** @brief Exchange an inventory copy with one equipment slot; virtual empty items cost nothing.
 * @param character Borrowed character. @param slot Slot 0..6. @param id Original item ID.
 * @return Success or false with SDL_GetError(); incompatible/unowned/unsupported items do not
 * mutate state. */
bool SotnCharacter_Equip(SotnCharacter *character, Uint32 slot, Uint16 id);
/** @brief Release character, retaining borrowed private bytes. @param character Owned or NULL. */
void SotnCharacter_Destroy(SotnCharacter *character);
#endif
