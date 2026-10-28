#ifndef SOTN_WOLF_ROOT_H
#define SOTN_WOLF_ROOT_H
#include <sotn/collision.h>
#include <sotn/wolf_articulation.h>
/** @brief Actual room collision adapter; report failure with SDL error, never fabricate a floor. */
typedef bool (*SotnWolfCollisionQuery)(void *context, Sint32 x, Sint32 y, SotnCollider *out);
/** @brief Retained original root and player presentation words; no primitive ownership. */
typedef struct SotnWolfRoot
{
    Sint32 image_cycle;
    Uint16 player_palette;
    Uint8 player_draw_flags, player_blend, player_opacity;
} SotnWolfRoot;
/** @brief Original six-primitive effect requests for the actual presentation owner. */
typedef struct SotnWolfRootEvents
{
    Sint32 image_index; /**< Negative means no selected retained image this update. */
    bool rush_image;
} SotnWolfRootEvents;
/** @brief Update original articulated Wolf root position, angle, private frame and ledge slide.
 * @param root Retained copied cycle/player words. Supply the actual current player palette.
 * @param body Actual copied body; only root slot17 changes. Native owner checks generations.
 * @param player Actual copied controller, after controller/animation/physical update.
 * @param primitives_allocated Actual successful six-row reservation, retained while initialized.
 * @param dra Borrowed private pose tables. @param executable Borrowed SDK tables.
 * @param query Actual room collision adapter. @param context Borrowed adapter state.
 * @param out Current effects requests, unchanged on failure. @return Success or SDL error,
 * atomically. Run before slot18. Original allocation exhaustion retains a pending child without
 * readiness. The owner must allocate/retire six real primitives, apply the requested image effects
 * and draw the actual artwork. This component enables no live Wolf controls. */
bool SotnWolfRoot_Tick(SotnWolfRoot *root, SotnWolfArticulation *body,
                       SotnWolfControllerFrame *player, bool primitives_allocated, SotnDataView dra,
                       SotnDataView executable, SotnWolfCollisionQuery query, void *context,
                       SotnWolfRootEvents *out);
#endif
