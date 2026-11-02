#ifndef SOTN_FOOD_POPUP_H
#define SOTN_FOOD_POPUP_H
#include <sotn/actor_primitive.h>
#include <sotn/game_data.h>
#include <sotn/primitive_slots.h>
enum
{
    SOTN_FOOD_POPUP_ROWS = 68,
    SOTN_FOOD_TEXT_TAG = 0x1003
};
/** @brief Original weapon21 lower-left pickup text, before actual shared row allocation.
 * Every packet comes from bounded private name bytes; no glyph/artwork is embedded. */
typedef struct SotnFoodPopup
{
    SotnActorPrimitive rows[SOTN_FOOD_POPUP_ROWS];
    Uint16 count;
} SotnFoodPopup;
/** @brief Reconstruct original paired-glyph parsing, gradient, border and font packets.
 * This supplies no completed allocation, item collection, audio or rendering. */
bool SotnFoodPopup_Build(SotnDataView dra, Uint16 item, SotnFoodPopup *out);
/** @brief The original global text's actual row references and lifetime; no row storage. */
typedef struct SotnFoodTextLease
{
    Uint32 generation;
    Uint16 rows[SOTN_FOOD_POPUP_ROWS], count, timer;
} SotnFoodTextLease;
/** @brief One actual-stage transaction, including the expected departing lease.
 * Packet copies are intents for the existing1024 rows, never another allocation pool. */
typedef struct SotnFoodTextPlan
{
    const void *owner;
    SotnFoodTextLease lease, previous;
    SotnFoodPopup popup;
    Uint32 serial, previous_serial;
    bool changed;
} SotnFoodTextPlan;
/** @brief Replace the copied current lease at the exact shared-child allocation point.
 * The stage owner must validate and commit this plan with all later player row requests. */
bool SotnFoodTextPlan_Replace(SotnFoodTextPlan *plan, SotnDataView dra, Uint16 item,
                              SotnPrimitiveSlots *slots);
#endif
