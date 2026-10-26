#ifndef SOTN_RELIC_H
#define SOTN_RELIC_H
#include <sotn/character.h>
#include <sotn/world.h>
/** @brief Read a bounded private original relic definition as icon/name metadata.
 * @param dra Borrowed DRA. @param id Original relic index 0..29.
 * @param out Receives icon/name and familiar category (type); unchanged on failure.
 * @return Success or SDL error. No private text or artwork is embedded. */
bool SotnRelic_Read(SotnDataView dra, Uint16 id, SotnItem *out);
/** @brief Apply the original found/active rule to owned relic progress.
 * @param world Owned progress. @param dra Borrowed DRA. @param id Original relic index.
 * @return Success or SDL error, unchanged on malformed data. Does not grant missing abilities. */
bool SotnRelic_Collect(SotnWorldState *world, SotnDataView dra, Uint16 id);
/** @brief Whether this native checkpoint implements an ordinary relic's active behavior.
 * @param id Original relic index. @return False for invalid or unintegrated identities. */
bool SotnRelic_CanToggle(Uint16 id);
/** @brief Toggle a found, supported relic using the original active-bit XOR.
 * @param world Writable ordinary progress. @param dra Borrowed private definitions.
 * @param id Original relic index. @return Success or SDL error, unchanged on failure.
 * Familiar activation remains unavailable until its runtime and equipment side effects exist. */
bool SotnRelic_Toggle(SotnWorldState *world, SotnDataView dra, Uint16 id);
#endif
