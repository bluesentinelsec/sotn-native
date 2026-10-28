#ifndef SOTN_PRIMITIVE_SLOTS_H
#define SOTN_PRIMITIVE_SLOTS_H
#include <SDL3/SDL.h>

enum
{
    SOTN_PRIMITIVE_SLOTS = 1024,
    SOTN_PRIMITIVE_WORDS = SOTN_PRIMITIVE_SLOTS / 64
};
/** @brief Copied availability of the actual original primitive pool, not a second pool. */
typedef struct SotnPrimitiveSlots
{
    Uint64 occupied[SOTN_PRIMITIVE_WORDS];
} SotnPrimitiveSlots;
/** @brief Plan an original first-free linked allocation against a copied pool snapshot.
 * @param slots In/out availability, committed only on complete allocation.
 * @param count Requested rows, 1..1024. @param indices Caller-owned space for count indices.
 * @param allocated Receives false for ordinary exhaustion; indices/snapshot stay unchanged.
 * @return Success or SDL error for invalid arguments. The actual pool owner must validate and
 * commit the resulting reservation before other owners advance. No primitive storage is owned. */
bool SotnPrimitiveSlots_Reserve(SotnPrimitiveSlots *slots, Uint16 count, Uint16 *indices,
                                bool *allocated);
/** @brief Release a checked reservation from a copied actual-pool snapshot.
 * @param slots In/out availability. @param count Rows, 1..1024.
 * @param indices Borrowed unique occupied row indices, count entries.
 * @return Success or SDL error without mutation for duplicate/free/out-of-range rows.
 * The actual owner still validates generation and commits the resulting pool transaction. */
bool SotnPrimitiveSlots_Release(SotnPrimitiveSlots *slots, Uint16 count, const Uint16 *indices);
#endif
