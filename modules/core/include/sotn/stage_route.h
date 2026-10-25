#ifndef SOTN_STAGE_ROUTE_H
#define SOTN_STAGE_ROUTE_H
#include <sotn/game_data.h>
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

/** @brief Copied neighbor probe and original vertical arrival adjustment, in room-local pixels. */
typedef struct SotnRoomCrossing
{
    Sint32 x, y, arrival_y;
    bool active;
} SotnRoomCrossing;
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
