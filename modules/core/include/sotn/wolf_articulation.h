#ifndef SOTN_WOLF_ARTICULATION_H
#define SOTN_WOLF_ARTICULATION_H
#include <sotn/wolf_controller.h>

/** @brief Copied original articulated part; lifetime/generation remains with its entity owner. */
typedef struct SotnWolfPart
{
    Sint32 x, y;
    Uint32 flags;
    Sint16 rotation, pivot_x, pivot_y, hitbox_x, hitbox_y, tail_rate;
    Uint16 step, sprite, bank, texture_offset, palette, priority, enemy_id, hitbox_state;
    Uint8 draw_flags, blend, opacity, width, height, hit_flags;
    bool facing_left;
} SotnWolfPart;
/** @brief Actual body snapshots in original slots 17..30 order.
 * The owner supplies root 17/head 20 and invokes followers 18/19/21..30 in original
 * slot order, interleaving actual head update 20. The service allocates no body or attack. */
typedef struct SotnWolfArticulation
{
    SotnWolfPart parts[14];
    Uint32 wave;
    Sint32 target_angle;
    SotnWolfCaptureBounds capture;
} SotnWolfArticulation;
/** @brief Original rush attack request and once-only MP spending from actual hit feedback. */
typedef struct SotnWolfArticulationEvents
{
    Uint16 mp_spent;
    bool rush_attack;
} SotnWolfArticulationEvents;
/** @brief Reconstruct selected linked Wolf body, spine and tail updates.
 * @param body Copied owned parts; other parts are retained. Native owner validates generations.
 * @param player Copied actual controller/player frame; rush feedback may select skid and spend MP.
 * @param slot Actual selected follower slot (18/19/21..30).
 * @param palette Current original player palette. @param dra Borrowed private pose/spell tables.
 * @param executable Borrowed original SDK trigonometry tables. @param out Current requests.
 * @return Success or SDL error, leaving all outputs unchanged on invalid state/data.
 * Caller invokes only while Wolf status is active and owns actual retirement, root/head,
 * hitbox/attack initialization, animation advancement and rendering. No live form is enabled. */
bool SotnWolfArticulation_Tick(SotnWolfArticulation *body, SotnWolfControllerFrame *player,
                               Uint16 slot, Uint16 palette, SotnDataView dra,
                               SotnDataView executable, SotnWolfArticulationEvents *out);
#endif
