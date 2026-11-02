#ifndef SOTN_CHARACTER_H
#define SOTN_CHARACTER_H
#include <sotn/equipment.h>
#include <sotn/familiar_stats.h>
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
    Uint8 category; /**< Original hand itemCategory; zero for body records. */
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
/** @brief Copy the actual seven familiar records and selected identity; either output may be NULL.
 * Selection0 means none. This read never grants a card or advances progression. */
bool SotnCharacter_GetFamiliars(const SotnCharacter *character, SotnFamiliarProgress *progress,
                                Uint16 *selected);
/** @brief Restore all seven validated records, retaining current selection and ordinary character.
 * This is a separate progression boundary; existing v9 save records remain unchanged. */
bool SotnCharacter_RestoreFamiliars(SotnCharacter *character, const SotnFamiliarProgress *progress);
/** @brief Restore actual familiar records and selection atomically at an engine/session boundary.
 * @param character Actual destination owner. @param progress Actual validated seven records.
 * @param selected Original identity0..7, bound separately to an owned card and native runtime.
 * @return Success or SDL error preserving destination on failure. Does not increment loads,
 * grant a card, change ordinary character/resources/RNG or initialize an entity.
 * Menu changes use SelectFamiliar instead. Existing v9 saves remain unchanged. */
bool SotnCharacter_RestoreFamiliarState(SotnCharacter *character,
                                        const SotnFamiliarProgress *progress, Uint16 selected);
/** @brief Commit an actual successful native menu selection/load, after resource binding.
 * @param character Actual owner. @param selected Original identity0..7.
 * @param reloaded True only when actual overlay loading completed, including a reload of the
 * same identity. Changed nonzero selections and reloads increment original load count once.
 * Ordinary rooms and unchanged menu selection do not increment. Grants no card/resources. */
bool SotnCharacter_SelectFamiliar(SotnCharacter *character, Uint16 selected, bool reloaded);
/** @brief Resolve actual virtual197 strike properties using live original stats and RNG.
 * @param character Borrowed character at the actual retained strike birth.
 * @param poison Actual player status.
 * @param random Owned copied libc RNG, committed by the shared runtime after success.
 * @param out Receives virtual equipment with final attack/critical values.
 * @return Success or SDL error preserving RNG/output. The caller validates the actual parent
 * generation and its retained virtual197; changing hands does not replace that identity.
 * Applies signed-word body/buff bonuses before caps, then poison, without granting an item. */
bool SotnCharacter_SwordProperties(const SotnCharacter *character, bool poison, Uint32 *random,
                                   SotnEquipment *out);
/** @brief Retained owned71..74 properties at their actual activation callbacks. */
bool SotnCharacter_ThrowingProperties(const SotnCharacter *, Uint16 item, bool poison,
                                      Uint32 *random, SotnEquipment *out);
/** @brief Retained72 properties at birth and each successful actual burst reservation. */
bool SotnCharacter_SireProperties(const SotnCharacter *, bool poison, Uint32 *random,
                                  SotnEquipment *out);
/** @brief Resolve retained Neutron71 ATK/critical properties at the real activation callback.
 * Uses live stats/body buffs and copied libc RNG, even after the last hand copy is consumed. */
bool SotnCharacter_NeutronProperties(const SotnCharacter *character, bool poison, Uint32 *random,
                                     SotnEquipment *out);
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
/** @brief Apply actual acquisition ordering to a staged progress transaction.
 * The supplied progress and live character remain unchanged on failure. */
bool SotnCharacter_PlanCollect(const SotnCharacter *character, SotnCharacterState *state, bool body,
                               Uint16 id);
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
/** @brief Consume actual equipped166 once; spare counts exclude equipped copies.
 * Original Duplicator84 in either accessory preserves it. A last copy becomes empty and
 * recalculates actual stats. Both character and output remain unchanged on failure. */
bool SotnCharacter_UseLibraryCard(SotnCharacter *character, Uint16 hand, Uint16 *equipped);
/** @brief Consume one actual equipped medicine144..162 (excluding146/147) or Card166 after player
 * success. Spare counts exclude the equipped copy. Either actual Duplicator84 suppresses
 * consumption. No default grants or inventory/hand selection are performed. Output commits
 * atomically. */
bool SotnCharacter_UseMedicine(SotnCharacter *character, Uint16 hand, Uint16 item,
                               Uint16 *equipped);
/** @brief Consume actual weapon21 food29..70 after successful player control.
 * Spare stock, last-copy removal and actual Duplicator use the same original ReduceWeapon. */
bool SotnCharacter_UseFood(SotnCharacter *character, Uint16 hand, Uint16 item, Uint16 *equipped);
/** @brief Consume owned71..74 with actual stock/Duplicator semantics, after player success. */
bool SotnCharacter_UseThrowing(SotnCharacter *, Uint16 hand, Uint16 item, Uint16 *equipped);
/** @brief Consume equipped Neutron Bomb71 after player success, preserving Duplicator rules. */
bool SotnCharacter_UseNeutron(SotnCharacter *character, Uint16 hand, Uint16 *equipped);
/** @brief Commit one original Fairy bag-byte decrement for actual mapped medicine144..162.
 * Call after the real callback/player/native operations succeed, even if optional effects
 * cannot allocate. Leaves equipment, acquisition order, stats, resources and RNG unchanged;
 * Duplicator does not suppress this write. Preserves original0-to255 byte wrap when stock
 * changes between the earlier ability check and actual use. Enables no familiar by itself.
 * @return Success or SDL error without mutation for an invalid owner/item. */
bool SotnCharacter_UseFamiliarItem(SotnCharacter *character, Uint16 item);
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
/** @brief Advance selected MP regeneration for a caller with an already-approved status gate.
 * @param character Borrowed progress; HP does not gate MP. @param ticks Actual gameplay clock.
 * @return Success or SDL error. The native HUD uses SotnHudResources_Tick with actual status
 * instead; this compatibility helper owns no walking-heal counter or player-status gate. */
bool SotnCharacter_TickResources(SotnCharacter *character, Uint64 ticks);
/** @brief Spend an already-approved ability cost from ordinary character MP.
 * @param character Borrowed owner. @param amount MP cost, 0..9999.
 * @return Success or SDL error without mutation when insufficient or invalid.
 * Ability-specific thresholds are checked by the simulation before this transfer. */
bool SotnCharacter_SpendMP(SotnCharacter *character, Uint16 amount);
/** @brief Commit the resource part of an actual original AddHearts request after player success.
 * @param character Owned progress. @param amount Original nonnegative HP loss, 0..9999.
 * @return Success or SDL error without mutation. Caps at hearts_max; no RNG or save-format change.
 * The producer gates its real shared factory99/pickup sound on the pre-add heart count.
 * This operation alone does not create an effect or implement another item. */
bool SotnCharacter_AddHearts(SotnCharacter *character, Uint16 amount);
/** @brief Release character, retaining borrowed private bytes. @param character Owned or NULL. */
void SotnCharacter_Destroy(SotnCharacter *character);
#endif
