#ifndef SOTN_MEDICINE_EFFECT_H
#define SOTN_MEDICINE_EFFECT_H
#include <sotn/game_data.h>

/** @brief Original rowless entity39 cursor; profiles remain in private DRA data. */
typedef struct SotnMedicineEffect
{
    Sint16 timer, index;
    Uint16 parameter, phase;
    bool removed;
} SotnMedicineEffect;
/** @brief Actual player/equipment inputs read at this child callback, including dead states. */
typedef struct SotnMedicineEffectInput
{
    SotnDataView dra;
    Uint16 player_step, hp_max;
    bool enhanced_potion; /**< Actual accessory80 count is nonzero; two copies still add once. */
} SotnMedicineEffectInput;
/** @brief Ordered original child requests and resource writes, committed by actual owners.
 * Healing replaces pending kind/amount unconditionally; it is not blood's first-pending API.
 * A request alone does not complete protection, buffs, healing, MP, cures, Hammer or factories. */
typedef struct SotnMedicineEffectEvents
{
    Uint32 factories[2];
    Uint16 sounds[2], buff_mask, protection, heal_amount;
    Uint8 factory_count, sound_count, cure_mask;
    bool heal, mp_full, hammer;
} SotnMedicineEffectEvents;
/** @brief Advance actual original medicine effect once, with signed-word timer/index semantics.
 * This component enables no hand use, equipment, familiar, pool or resource availability.
 * Caller owns real rowless child/factories and commits outputs only after native success.
 * @param effect Initialized cursor, or a zeroed birth cursor with actual parameter.
 * @param input Borrowed private profile and current player/equipment snapshot.
 * @param out Receives this callback's requests; does not retain input pointers.
 * @return Success, or SDL error with cursor and output unchanged. */
bool SotnMedicineEffect_Tick(SotnMedicineEffect *effect, SotnMedicineEffectInput input,
                             SotnMedicineEffectEvents *out);
#endif
