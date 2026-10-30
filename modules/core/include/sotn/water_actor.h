#ifndef SOTN_WATER_ACTOR_H
#define SOTN_WATER_ACTOR_H
#include <sotn/water_volume.h>

/** @brief Original placed water controller's retained, defined cache words. */
typedef struct SotnWaterActor
{
    Uint16 raw, row;
    Sint16 cooldown, x, y, left, right;
    Uint8 phase;
} SotnWaterActor;
/** @brief Original water-child roles, resolved to actual IDs by the stage owner. */
typedef enum SotnWaterChildKind
{
    SOTN_WATER_SPLASH,
    SOTN_WATER_SURFACING,
    SOTN_WATER_SIDE_SPLASH
} SotnWaterChildKind;
/** @brief Planned real auxiliary reservation, not a completed native factory. */
typedef struct SotnWaterChildPlan
{
    Sint32 x, y;
    Uint16 slot, parameters, priority, row;
    Sint16 current;
    SotnWaterChildKind kind;
} SotnWaterChildPlan;
/** @brief Actual pre-stage player and shared auxiliary occupancy snapshot. */
typedef struct SotnWaterActorInput
{
    SotnWaterVolumeInput volume;
    Sint32 x, y, velocity_x, velocity_y;
    Uint32 contacts, occupied; /**< Collision words and actual slots224..255, bit0=224. */
    Uint16 priority;
} SotnWaterActorInput;
/** @brief One copied controller transaction requiring actual native reservations before commit. */
typedef struct SotnWaterActorEvents
{
    SotnWaterVolume volume;
    SotnWaterChildPlan children[8];
    Sint32 current_x;
    Uint32 reserved;
    Uint8 count, attempts;
    bool depths_changed;
} SotnWaterActorEvents;
/** @brief Prepare the original ordinary water controller's update and ordered child plans.
 * @param actor Copied actual controller, initially zeroed. @param input Actual owner snapshot.
 * @param out Receives selected depths, current displacement and bounded auxiliary plans.
 * @return Success or SDL error preserving controller/output on invalid data or origin mismatch.
 * Birth caches the query but publishes no depth change. The surface cooldown starts only in a
 * transaction that can reserve its actual slot. The native owner must validate unchanged shared
 * availability and commit every planned real child before committing this copied controller or
 * depth/current feedback. Plans confer no successful factory, readiness or visual ownership.
 * No engine clock, RNG, native player, actual pool or save state is advanced by this interface.
 */
bool SotnWaterActor_Tick(SotnWaterActor *actor, SotnWaterActorInput input,
                         SotnWaterActorEvents *out);
#endif
