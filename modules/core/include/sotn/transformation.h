#ifndef SOTN_TRANSFORMATION_H
#define SOTN_TRANSFORMATION_H

#include <sotn/world.h>

/** @brief Original transformation resource families; not serialized state identities. */
typedef enum SotnTransformationForm
{
    SOTN_FORM_BAT = 0,
    SOTN_FORM_MIST,
    SOTN_FORM_WOLF
} SotnTransformationForm;

/** @brief One resource decision, separate from movement, presentation and ownership changes. */
typedef struct SotnTransformationCharge
{
    bool allowed; /**< Found/enabled relic and strictly more MP than its original cost. */
    Uint16 spent; /**< Amount owed this simulation tick; zero for a check or blocked form. */
} SotnTransformationCharge;

/** @brief Evaluate the original relic/MP gate and simulation-clock resource cadence.
 * @param world Borrowed validated progress; never modified or granted relics.
 * @param form Bat, mist or wolf. @param mp Current MP snapshot.
 * @param game_timer Original wrapping 32-bit simulation timer, independent of render cadence.
 * @param reduce Whether this is an active-form reduction rather than an entry eligibility check.
 * @param out Initialized copied decision on success; unchanged on malformed input.
 * @return Success, including a blocked form, or SDL error for invalid pointers/state/form.
 * @note The caller applies spent once to ordinary progress. This function neither enters a
 * form nor implements its collision, effect, attack or input behavior. */
bool SotnTransformation_MP(const SotnWorldState *world, SotnTransformationForm form, Uint16 mp,
                           Uint32 game_timer, bool reduce, SotnTransformationCharge *out);

#endif
