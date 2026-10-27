#ifndef SOTN_GAS_CLOUD_H
#define SOTN_GAS_CLOUD_H

#include <sotn/player.h>

enum
{
    SOTN_GAS_CLOUD_SLOTS = 16
};

/** @brief One original invisible Gas Cloud child, with whole room coordinates. */
typedef struct SotnGasCloudChild
{
    Sint32 x, y;
    SotnPlayerAttack attack;
    Uint16 slot; /**< Actual original DRA slot 32..47 selected by the shared available mask. */
} SotnGasCloudChild;

/** @brief Owned transient children for one generation of the real Mist effect.
 * Zero-initialize; retain across rooms, discard on full stage initialization.
 * All children share original enemy-invulnerability group four. */
typedef struct SotnGasCloud
{
    Uint32 generation;
    Uint16 mask;
    SotnGasCloudChild children[SOTN_GAS_CLOUD_SLOTS];
} SotnGasCloud;

/** @brief Run the selected original blueprint-83 allocation and spell initialization.
 * @param cloud Owned output, unchanged on failure. @param dra Borrowed private definitions.
 * @param character Borrowed owner of derived INT, armor and spell RNG; consumes one RNG
 * value per successfully allocated child. @param generation Actual nonzero Mist generation.
 * @param available Original slots 32..47 available for allocation, least index first.
 * @return Success or SDL error, preserving character/RNG/output on malformed metadata.
 * Original blueprint 83 requests seventeen children in a sixteen-slot range: the failed
 * seventeenth allocation ends its noncritical factory. This never invents another slot.
 * The caller must account for shared allocation contention when providing available. */
bool SotnGasCloud_Begin(SotnGasCloud *cloud, SotnDataView dra, SotnCharacter *character,
                        Uint32 generation, Uint16 available);

/** @brief Read the actual occupied DRA slots, including disabled but retained children.
 * @param cloud Borrowed owner. @param out Receives a validated slot mask, unchanged on failure.
 * @return Success or SDL error for malformed generations, duplicate or out-of-range slots. */
bool SotnGasCloud_Occupied(const SotnGasCloud *cloud, Uint64 *out);

/** @brief Update actual cloud hitboxes after the generation-owned visual has advanced.
 * @param cloud Owned cursor. @param player Copied actual player; leaving Mist retires children.
 * @param effect Borrowed actual live visual, required while player Mist status is set.
 * @param generation Actual visual generation. @param camera_x Whole room camera position.
 * @param camera_y Whole room camera position. @param enabled Gas Cloud's current active bit.
 * @return Success or SDL error without mutation for stale/missing effects or invalid origins.
 * This does not create children when the relic is enabled midway through a transformation. */
bool SotnGasCloud_Tick(SotnGasCloud *cloud, SotnPlayerState player, const SotnMistEffect *effect,
                       Uint32 generation, Sint32 camera_x, Sint32 camera_y, bool enabled);

#endif
