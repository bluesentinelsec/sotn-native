#ifndef SOTN_STAGE_PROFILE_H
#define SOTN_STAGE_PROFILE_H
#include <SDL3/SDL.h>
/** @brief Bounded native stage capabilities and original table dimensions; no runtime assets. */
typedef struct SotnStageProfile
{
    Uint16 id, actor_limit, sprite_banks, prize_count, prize_bit, door_types, camera_count;
    Uint16 save_rooms[3]; /**< Unused entries are UINT16_MAX; room zero is valid. */
    bool connected; /**< Allow ordinary checkpoint travel; does not imply complete area parity. */
} SotnStageProfile;
/** @brief Look up immutable metadata for native stage adapters.
 * @param stage Original stage ID. @return Borrowed static metadata, NULL if unsupported.
 * A profile does not imply every actor or event in an area is implemented. */
const SotnStageProfile *SotnStageProfile_Get(Uint16 stage);
/** @brief Test a stage-local ordinary save room against its explicit profile. */
bool SotnStageProfile_SaveRoom(Uint16 stage, Uint32 room);
/** @brief Identify explicit NO3 development entries on the far side of Death's room.
 * @param room Original room identity. @return Whether a fresh diagnostic entry seeds theft.
 * Forest/bridge and pre-Death side rooms retain opening gear. Normal travel always retains
 * the actual saved event state; this policy applies only to fresh development starts. */
bool SotnStageProfile_EntranceAfterDeath(Uint32 room);
#endif
