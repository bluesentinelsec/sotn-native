#ifndef SOTN_STAGE_ROUTE_H
#define SOTN_STAGE_ROUTE_H
#include <sotn/game_data.h>
#include <sotn/world.h>
/** @brief Original US CD-room arrival, decoded from private DRA rather than embedded tables. */
typedef struct SotnStageRoute
{
    Uint16 source, stage, room, x, y;
} SotnStageRoute;
/** @brief Decode a bounded original US transition record and verify its source identity.
 * @param dra Borrowed verified DRA. @param source Current original stage ID.
 * @param index Transition room's layout_id (0..130). @param out Unchanged on failure.
 * @return Success or SDL error. Destination gameplay support and room dimensions must be
 * validated by the session before installing an arrival. No host/code pointers are returned.
 */
bool SotnStageRoute_Read(SotnDataView dra, Uint16 source, Uint16 index, SotnStageRoute *out);

/** @brief Select an original boss override before ordinary room lookup.
 * @param dra Borrowed verified US DRA with its sentinel-terminated boss table.
 * @param source Original current stage. @param x Target castle chunk, -1..64.
 * @param y Target castle chunk, -1..64. @param world Borrowed initialized progress.
 * @param found Receives whether an undefeated/always-active transfer matched.
 * @param out Receives the validated arrival only on a match; unchanged otherwise.
 * @return Success or SDL error; both outputs remain unchanged on failure.
 * @note Destination gameplay support remains the session's responsibility. */
bool SotnStageRoute_FindBoss(SotnDataView dra, Uint16 source, Sint32 x, Sint32 y,
                             const SotnWorldState *world, bool *found, SotnStageRoute *out);

/** @brief Copied neighbor probe and original vertical arrival adjustment, in room-local pixels. */
typedef struct SotnRoomCrossing
{
    Sint32 x, y, arrival_y;
    bool active;
} SotnRoomCrossing;
/** @brief Transient original horizontal room limits, separate from the owned collision map.
 * Limits use the reference's moving room origin. Cell indices remain relative to the initial
 * room header; changing origin_cell never physically displaces the player or room artwork. */
typedef struct SotnRoomWindow
{
    Sint32 start_x, end_x, origin_cell, last_cell;
} SotnRoomWindow;
/** @brief Resolve an actor-adjusted room window before the original vertical edges.
 * @param window Copied reference limits and relative castle-cell endpoints.
 * @param x Whole-pixel X relative to the initial room origin. @param y Whole-pixel Y.
 * @param width Full collision-map width in pixels. @param height Full height in pixels.
 * @param grounded Original ground contact. @param out Neighbor probe, unchanged on failure.
 * @return Success or SDL error for invalid dimensions, positions or window bounds.
 * Returned probes identify neighboring castle cells; they are not physical arrival positions.
 * The caller preserves world position when rebasing into the selected destination. */
bool SotnStageRoute_WindowCrossing(SotnRoomWindow window, Sint32 x, Sint32 y, Uint32 width,
                                   Uint32 height, bool grounded, SotnRoomCrossing *out);
/** @brief Resolve selected normal-Alucard room edges in original horizontal-first order.
 * @param x Room-local whole-pixel X. @param y Room-local whole-pixel Y.
 * @param width Room width in pixels, 256..16384. @param height Room height in pixels.
 * @param grounded Current original ground-contact bit; selects the downward threshold/offset.
 * @param out Receives a copied neighbor probe and arrival offset; unchanged on failure.
 * @return Success or SDL error. An inactive result stays in the current room. The caller must
 * verify a neighbor exists before applying the offset; no position or velocity is mutated.
 * @note Transformations and locked camera-edge rules need separate integration.
 */
bool SotnStageRoute_Crossing(Sint32 x, Sint32 y, Uint32 width, Uint32 height, bool grounded,
                             SotnRoomCrossing *out);
#endif
