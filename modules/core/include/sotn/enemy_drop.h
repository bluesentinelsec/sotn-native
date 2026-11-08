#ifndef SOTN_ENEMY_DROP_H
#define SOTN_ENEMY_DROP_H
#include <SDL3/SDL.h>

/** @brief An enemy definition's item fields (g_api.enemyDefs[enemyId], offsets 0x1A..0x21). */
typedef struct SotnEnemyDropRates
{
    Uint16 rare_item, uncommon_item, rare_rate, uncommon_rate;
} SotnEnemyDropRates;

/** @brief func_800FF460: a luck cutoff scaled by total LCK, arg + arg * LCK / 0x80 (unsigned).
 * @param cutoff g_testCollLuckCutoff entry (zero stays zero). @param luck g_Status.statsTotal[3].
 * @return The scaled cutoff compared with rand() & 0xFF. */
Sint32 SotnEnemyDrop_LuckCutoff(Uint32 cutoff, Sint32 luck);

/** @brief func_800FF494: choose an enemy drop's kind. Returns 0x40 (rare item), 0x20 (uncommon
 * item) or a g_testCollPrizeTable index 0..31. Each Ring of Arcana75 accessory copy lowers both
 * thresholds by the rate again and adds one to the prize index.
 * @param rates Copied enemy definition fields. @param luck Total LCK. @param arcana Ring of Arcana
 * count (0..2). @param random libc cursor, advanced by three to five rand() calls.
 * @param kind Receives the kind. @return Success or SDL error without mutation for bad inputs. */
bool SotnEnemyDrop_Kind(SotnEnemyDropRates rates, Sint32 luck, Uint16 arcana, Uint32 *random,
                        Sint32 *kind);

/** @brief The collision.h drop tables a stage overlay keeps beside its g_testCollLuckCutoff. */
typedef struct SotnEnemyDropTables
{
    Uint8 luck_cutoff[4];     /**< g_testCollLuckCutoff, indexed by (flags & 0xC00) >> 10. */
    Uint16 enemy_lookup[400]; /**< g_testCollEnemyLookup: rare-drop enemy-list flag + 1. */
    Uint16 prize[32];         /**< g_testCollPrizeTable ITEMDROP identities. */
    Uint16 jewel_sword[14];   /**< g_jewelSwordDropTable threshold/ITEMDROP pairs. */
} SotnEnemyDropTables;

/** @brief Read and validate a stage overlay's drop tables at its luck-cutoff offset.
 * @param overlay Private stage overlay image. @param size Its byte size.
 * @param luck_offset Its g_testCollLuckCutoff offset.
 * @param out Receives copies. @return Success or SDL error without mutation when the original
 * luck/coordinate pattern, bounds or final jewel threshold do not match. */
bool SotnEnemyDrop_ReadTables(const Uint8 *overlay, size_t size, size_t luck_offset,
                              SotnEnemyDropTables *out);

/** @brief Pick the Jewel sword's gem (hitEffect 5): the first pair whose threshold reaches
 * rand() & 0xFFF. @param tables Validated tables. @param random libc cursor (one call).
 * @return The ITEMDROP identity. */
Uint16 SotnEnemyDrop_Jewel(const SotnEnemyDropTables *tables, Uint32 *random);
#endif
