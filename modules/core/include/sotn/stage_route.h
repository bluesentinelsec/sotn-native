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
#endif
