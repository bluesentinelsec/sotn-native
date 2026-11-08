#ifndef SOTN_SUBWEAPON_H
#define SOTN_SUBWEAPON_H
#include <sotn/player.h>

/** @brief Copied g_SubwpnDefs record (DRA 0x4A00 + id * 20); private data stays in the image. */
typedef struct SotnSubweaponDefinition
{
    SotnPlayerAttack attack; /**< Definition attack before func_800FE3C4's per-birth roll. */
    Uint16 hearts;
    Uint8 chain, animation, blueprint, crash;
} SotnSubweaponDefinition;
/** @brief Read and validate one original subweapon definition.
 * @param dra Borrowed private DRA. @param id Subweapon 0..12. @param out Receives the record.
 * @return Success or SDL error without mutation for missing/malformed data. */
bool SotnSubweapon_Read(SotnDataView dra, Uint16 id, SotnSubweaponDefinition *out);
/** @brief func_800FE3C4's heart cost: one Heart broach77 halves it, two divide by three,
 * minimum one. @param profile Equipment words. @param cost Definition cost. @return Cost. */
Uint16 SotnSubweapon_Cost(SotnCharacterState profile, Uint16 cost);
/** @brief func_800FE3C4's attack for an explicit subweapon: Brilliant mail20 adds ten, each
 * Staurolite61 accessory multiplies the cross (4) and subweapon12 (US), then
 * (INT * 2 + rand() % 12) / 10 is added.
 * @param id Subweapon 1..12. @param base Definition attack. @param profile Equipment words.
 * @param intelligence Total INT. @param random Character libc cursor (advanced once).
 * @param power Receives the 16-bit attack. @return Success or SDL error without mutation. */
bool SotnSubweapons_Power(Uint16 id, Uint16 base, const SotnCharacterState *profile,
                          Uint16 intelligence, Uint32 *random, Uint16 *power);
#endif
