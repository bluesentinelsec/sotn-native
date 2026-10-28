// SPDX-License-Identifier: AGPL-3.0-or-later
// Original first-free linked allocation: dra/4DA70.c:AllocPrimitives, pinned sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Storage remains with the actual pool owner.
#include <sotn/primitive_slots.h>

bool SotnPrimitiveSlots_Reserve(SotnPrimitiveSlots *slots, Uint16 count, Uint16 *indices,
                                bool *allocated)
{
    if (!slots || !indices || !allocated || !count || count > SOTN_PRIMITIVE_SLOTS)
        return SDL_SetError("Primitive allocation requires a snapshot and bounded output storage");
    Uint16 free_count = 0;
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        if (!(slots->occupied[i / 64U] & ((Uint64)1 << (i % 64U))))
            ++free_count;
    *allocated = free_count >= count;
    if (!*allocated)
        return true;
    Uint16 found = 0;
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS && found < count; ++i)
        if (!(slots->occupied[i / 64U] & ((Uint64)1 << (i % 64U))))
        {
            indices[found++] = i;
            slots->occupied[i / 64U] |= (Uint64)1 << (i % 64U);
        }
    return true;
}
