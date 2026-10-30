#ifndef SOTN_CHARACTER_H
#define SOTN_CHARACTER_H
#include <sotn/game_data.h>
#include <sotn/stat_buffs.h>

enum
{
    SOTN_HAND_ITEMS = 169,
    SOTN_BODY_ITEMS = 90,
    SOTN_EQUIP_SLOTS = 7,
    SOTN_SPELLS = 8
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
    Uint16 hp, hp_max, mp, mp_max, hearts, hearts_max, level, base[4], equipment[7], subweapon;
    Uint32 experience, gold, kills, random;
    Uint8 hand_count[SOTN_HAND_ITEMS], body_count[SOTN_BODY_ITEMS];
    Uint8 hand_order[SOTN_HAND_ITEMS], body_order[SOTN_BODY_ITEMS];
    Uint32 spells_learned; /**< Original known-spell mask; casting remains a separate service. */
    Uint8 spells[SOTN_SPELLS]; /**< Acquisition order, original 0x80-known encoding. */
    Uint8 laboratory_items[2]; /**< Original NZ0 persistent item indices, not transient candles. */
} SotnCharacterState;
/** @brief Decoded private equipment record; text views expire with the source DRA image. */
typedef struct SotnItem
{
    SotnDataView name, description;
    Sint16 attack, defense, bonus[4];
    Uint16 id, type, weakness, resistance, immunity, absorption, icon, palette;
    bool body, supported;
    bool
        name_ascii; /**< Relic names use bounded ASCII; equipment uses the private font encoding. */
} SotnItem;
/** @brief Derived original attributes with selected actual stat-buff mask; other conditional
 * effects remain rejected at equip. */
typedef struct SotnCharacterStats
{
    Uint16 total[4], attack[2], defense, weakness, resistance, immunity, absorption;
    Sint16 bonus[4];
    Uint16 unarmed_attack; /**< Original empty-hand power including body/selected buff bonuses. */
} SotnCharacterStats;
/** @brief Owned character borrowing immutable DRA bytes until Destroy. */
typedef struct SotnCharacter SotnCharacter;
/** @brief Recalculate derived stats from an actual stat-buff owner's active-word snapshot.
 * @param character Actual character. @param mask Derived sixteen-word mask; words12..15 have no
 * selected stat effect. The caller retains actual timers separately, never in ordinary saves.
 * @return Success or SDL error preserving character/stats. Applies bonuses before original
 * clamps and US stone immunity. Enables no item/ability or replacement timer pool. */
bool SotnCharacter_SetStatBuffMask(SotnCharacter *character, Uint16 mask);
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
/** @brief Exchange gold and unequipped equipment atomically using the shop's original limits.
 * @param character Owned progress. @param body Select item table. @param id Original item ID.
 * @param quantity Copies, 1..99. @param price Unit price, 0..999999.
 * @param sell Sell unequipped copies; otherwise buy with a 99-copy limit including equipment.
 * @return Success or SDL error with unchanged ownership, gold and acquisition order. */
bool SotnCharacter_Trade(SotnCharacter *character, bool body, Uint16 id, Uint16 quantity,
                         Uint32 price, bool sell);
/** @brief Learn an original spell once and append its encoded acquisition order.
 * @param state Writable initialized progress. @param id Original spell 0..7.
 * @return Success or SDL error with unchanged state on invalid input. Does not implement casting.
 */
bool SotnCharacter_LearnSpell(SotnCharacterState *state, Uint16 id);
/** @brief Exchange an inventory copy with one equipment slot; virtual empty items cost nothing.
 * @param character Borrowed character. @param slot Slot 0..6. @param id Original item ID.
 * @return Success or false with SDL_GetError(); incompatible/unowned/unsupported items do not
 * mutate state. */
bool SotnCharacter_Equip(SotnCharacter *character, Uint32 slot, Uint16 id);
/** @brief Install the original first-Entrance equipment on a fresh character.
 * @param character Fresh owner before play; replaces the seven equipment slots atomically.
 * @return Success or SDL error. Resources and prologue-derived stats remain caller-owned.
 * @note This is equipment initialization, not a complete title/prologue/new-game flow. */
bool SotnCharacter_OpeningGear(SotnCharacter *character);
/** @brief Apply Death's original six-item theft, preferring an equipped copy over inventory.
 * @param character Borrowed owner. @return Success or SDL error, with unchanged state on failure.
 * Does not remove unrelated equipment, resources, or additional inventory copies.
 */
bool SotnCharacter_DeathTheft(SotnCharacter *character);
/** @brief Read private item metadata without exposing the character's borrowed image.
 * @param character Borrowed character. @param body Item table. @param id Item ID.
 * @param out Decoded metadata, unchanged on failure. @return Success or false with SDL_GetError().
 */
bool SotnCharacter_Item(const SotnCharacter *character, bool body, Uint16 id, SotnItem *out);
/** @brief Award original level-adjusted enemy EXP and process all resulting level gains.
 * @param character Borrowed writable character. @param experience Enemy's original EXP reward.
 * @param enemy_level Original enemy level, 0..99. @param levels Receives levels gained.
 * @return Success or false with SDL_GetError(), leaving progress and RNG unchanged on failure.
 * @note Uses private thresholds/HP growth and preserves recursive MP/stat RNG consumption order. */
bool SotnCharacter_GainExperience(SotnCharacter *character, Uint16 experience, Uint16 enemy_level,
                                  Uint16 *levels);
/** @brief Read EXP needed for the next level; level 99 reports zero.
 * @param character Borrowed character. @param remaining Output unchanged on failure.
 * @return Success or false with SDL_GetError() for invalid/truncated progression tables. */
bool SotnCharacter_NextLevel(const SotnCharacter *character, Uint32 *remaining);
/** @brief Collect an original Heart/Life Max Up and update its resources.
 * @param character Borrowed character. @param life True for Life Max Up, false for Heart Max Up.
 * @return Success or false with SDL_GetError(); caps and full-heal behavior follow the selected
 * original path. */
bool SotnCharacter_CollectVessel(SotnCharacter *character, bool life);
/** @brief Advance selected original MP regeneration at a gameplay tick, never a render call.
 * @param character Borrowed living progress. @param ticks Gameplay clock.
 * @return Success or false with SDL_GetError(); pause/menu callers do not invoke this function. */
bool SotnCharacter_TickResources(SotnCharacter *character, Uint64 ticks);
/** @brief Spend an already-approved ability cost from ordinary character MP.
 * @param character Borrowed owner. @param amount MP cost, 0..9999.
 * @return Success or SDL error without mutation when insufficient or invalid.
 * Ability-specific thresholds are checked by the simulation before this transfer. */
bool SotnCharacter_SpendMP(SotnCharacter *character, Uint16 amount);
/** @brief Release character, retaining borrowed private bytes. @param character Owned or NULL. */
void SotnCharacter_Destroy(SotnCharacter *character);
#endif
