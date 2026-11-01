// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/692E8.c:EntityAlucard, 7E4BC.c:EntityPlayerPinkEffect and
// 71830.c:PlayerStepStone, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
#include "player_internal.h"

bool SotnPlayer_SetMedicineContext(SotnPlayer *p, SotnPlayerMedicineFactory factory,
                                   SotnPlayerMedicineConsume consume, void *context, bool present)
{
    if (!p || (factory != NULL) != (consume != NULL))
        return SDL_SetError("Medicine context requires paired actual reservation and consumption");
    p->medicine_factory = factory;
    p->medicine_consume = consume;
    p->medicine_context = context;
    p->medicine_present = present;
    return true;
}
bool SotnPlayer_CommitMedicine(SotnPlayer *p, const SotnMedicineEffectEvents *e, Uint16 mp_max)
{
    if (!p || !e || !mp_max || mp_max > 9999 || e->cure_mask > 3 ||
        (e->protection && e->protection != 64) ||
        (e->heal ? !e->heal_amount || e->heal_amount > 9999 : e->heal_amount != 0) ||
        (e->hammer && p->step != SOTN_PLAYER_STONE))
        return SDL_SetError("Medicine commit lost its actual resource request");
    SotnPlayer n = *p;
    if (e->protection)
        SotnPlayer_Protect(&n, (Sint16)e->protection);
    for (Uint16 i = 0; i < 2; ++i)
        if (e->cure_mask & (1U << i))
            n.timers[i] = 2;
    if (e->heal)
    {
        n.healing.pending = e->heal_amount;
        n.healing.pending_kind = 1;
    }
    if (e->mp_full)
        n.mp = mp_max;
    if (e->hammer)
    {
        n.stone.shakes = 1;
        n.hammer_timer = 64;
    }
    *p = n;
    return true;
}
bool SotnPlayer_RestoreHammer(SotnPlayer *p, Sint32 timer)
{
    if (!p || timer < 0 || timer > 64)
        return SDL_SetError("Hammer restore requires the actual separate0..64 DRA word");
    p->hammer_timer = timer;
    return true;
}
