#ifndef SOTN_DOPPLE_MOTION_H
#define SOTN_DOPPLE_MOTION_H
#include <sotn/collision.h>

/** @brief Original signed pixel sensor, loaded from private BO4 data. */
typedef struct SotnDoppleSensor
{
    Sint16 x, y;
} SotnDoppleSensor;
/** @brief Owned boss collision state, independent of Alucard's sensor geometry.
 * Positions and velocities use original signed 16.16 coordinates. The owner may
 * update these and status before advancing. Sensor/collider history belongs to
 * this state; initialize it before use. No borrowed storage or allocation. */
typedef struct SotnDoppleMotion
{
    Sint32 x, y, vx, vy;
    Uint32 status, contact, previous_contact;
    SotnDoppleSensor floor_sensors[4], ceiling_sensors[4], wall_sensors[14];
    SotnCollider floor[4], ceiling[4], wall[14];
    bool initialized;
} SotnDoppleMotion;

/** @brief Initialize the boss's original sensor layout from its verified overlay.
 * @param overlay Borrowed immutable BO4 image, no embedded tables.
 * @param out Receives a zeroed motion state with sensors; unchanged on failure.
 * @return Success or SDL error on invalid/truncated sensor metadata. */
bool SotnDoppleMotion_Init(SotnDataView overlay, SotnDoppleMotion *out);
/** @brief Resolve one original boss movement tick, including four-step fast motion.
 * @param overlay Borrowed BO4 sensor data. @param map Borrowed room-local collision map.
 * @param high_jump Whether the original high-jump step is active.
 * @param motion Owned initialized state, unchanged on failure.
 * @return Success or SDL error for invalid input or failed collision probes.
 * @note Status uses the original bits. Bat/mist select the short sensor geometry;
 * unmorph status retains previous geometry. Speeds above 32 px/tick are rejected. */
bool SotnDoppleMotion_Advance(SotnDataView overlay, const SotnCollisionMap *map, bool high_jump,
                              SotnDoppleMotion *motion);
#endif
