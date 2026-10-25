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
#endif
