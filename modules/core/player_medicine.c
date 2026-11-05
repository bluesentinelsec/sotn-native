// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/692E8.c:EntityAlucard, 7E4BC.c:EntityPlayerPinkEffect and
// 71830.c:PlayerStepStone, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
#include "player_internal.h"
#include <sotn/weapon_secondary.h>

bool SotnPlayer_SetThrowingContext(SotnPlayer *p, SotnPlayerMedicineFactory factory,
                                   SotnPlayerMedicineConsume consume,
                                   SotnPlayerThrowingRandom random, void *context, Uint8 left_count,
                                   Uint8 right_count)
{
    if (!p || (factory != NULL) != (consume != NULL) || (factory != NULL) != (random != NULL) ||
        left_count > 48 || right_count > 48)
        return SDL_SetError(
            "Throwing context requires actual paired factory/consumption/RNG owners");
    p->throwing_factory = factory;
    p->throwing_consume = consume;
    p->throwing_random = random;
    p->throwing_context = context;
    p->throwing_count[0] = left_count;
    p->throwing_count[1] = right_count;
    return true;
}

bool SotnPlayer_SetSpecialChain(SotnPlayer *p, Uint8 left_count, Uint8 right_count)
{
    if (!p || left_count > 48 || right_count > 48)
        return SDL_SetError("Special chain counts require actual shared player slots");
    p->special_count[0] = left_count;
    p->special_count[1] = right_count;
    return true;
}
bool SotnPlayer_SetComboChain(SotnPlayer *p, Uint8 count)
{
    if (!p || count > 48)
        return SDL_SetError("Combo chain count requires actual shared player slots");
    p->combo_count = count;
    return true;
}
Uint32 SotnPlayer_Combo(const SotnPlayer *p)
{
    return p ? p->combo : 0;
}
bool SotnPlayer_ComboSupported(Uint16 item)
{
    // Combo spells with an actual native owner (Leather203, Knight204, Alucart215).
    return SotnWeaponSecondary_ShieldSpellFamily(item) != 0;
}
bool SotnPlayer_SetSpecialCross(SotnPlayer *p, Uint8 left_count, Uint8 right_count)
{
    if (!p || left_count > 48 || right_count > 48)
        return SDL_SetError("Special chain counts require actual shared player slots");
    p->special_cross[0] = left_count;
    p->special_cross[1] = right_count;
    return true;
}
bool SotnPlayer_SetNeutronContext(SotnPlayer *p, SotnPlayerMedicineFactory factory,
                                  SotnPlayerMedicineConsume consume,
                                  SotnPlayerThrowingRandom random, void *context, Uint8 left_count,
                                  Uint8 right_count)
{
    return SotnPlayer_SetThrowingContext(p, factory, consume, random, context, left_count,
                                         right_count);
}

bool SotnPlayer_SetFoodContext(SotnPlayer *p, SotnPlayerMedicineFactory factory,
                               SotnPlayerMedicineConsume consume, void *context, Uint8 left_count,
                               Uint8 right_count)
{
    if (!p || (factory != NULL) != (consume != NULL) || left_count > 48 || right_count > 48)
        return SDL_SetError("Food context requires paired actual factories and hand counts");
    p->food_factory = factory;
    p->food_consume = consume;
    p->food_context = context;
    p->food_count[0] = left_count;
    p->food_count[1] = right_count;
    return true;
}
bool SotnPlayer_CommitFood(SotnPlayer *p, const SotnFoodEvents *events, Uint16 count)
{
    if (!p || (!events && count) || count > 15)
        return SDL_SetError("Food commit requires actual slot-ordered callbacks");
    SotnPlayer n = *p;
    for (Uint16 i = 0; i < count; ++i)
    {
        const SotnFoodEvents e = events[i];
        if ((e.heal && e.heal_amount > 9999) || (e.simulated_ticks && e.simulated_ticks != 20) ||
            (e.crouch && !e.simulated_ticks))
            return SDL_SetError("Food commit lost its original healing or simulated input");
        if (e.clear_weapon && n.shield_cursor.phase)
        {
            n.shield_cursor = (SotnShield){0};
            n.shield = (SotnPlayerAttack){0};
            n.shield_button = 0;
        }
        if (e.peanut_pose)
        {
            n.animation.pose = 3;
            n.animation.remaining = 8;
        }
        if (e.heal && !n.healing.pending_kind)
        {
            n.healing.pending_kind = 1;
            n.healing.pending = e.heal_amount;
        }
        if (e.simulated_ticks)
        {
            n.simulated_pad.buttons = e.crouch ? SOTN_BUTTON_DOWN : 0;
            n.simulated_pad.remaining = e.simulated_ticks;
        }
        if (e.poison)
        {
            n.queued_contact.parameters = 0x81;
            n.queued_contact.attack = 0;
        }
    }
    *p = n;
    return true;
}

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
