#ifndef SOTN_BAT_ECHO_H
#define SOTN_BAT_ECHO_H

#include <sotn/game_math.h>

enum
{
    SOTN_BAT_ECHO_DRIVERS = 17,
    SOTN_BAT_ECHO_QUADS = 32,
    SOTN_BAT_ECHO_PRIMITIVES = 49
};
/** @brief One original untextured Echo strip; inner vertices have color, outer vertices fade. */
typedef struct SotnBatEchoQuad
{
    Sint16 x[4], y[4];
    Uint8 red[4], green[4], blue[4];
} SotnBatEchoQuad;
/** @brief Actual Echo primitive metadata and lifetime, bound to one owned child.
 * Zero-initialize and copy the factory's facing before first Tick. The caller must
 * reserve all49 actual primitive slots; this component owns no second allocator. */
typedef struct SotnBatEcho
{
    struct
    {
        Sint16 angle, color_angle, shape, color;
    } drivers[17];
    SotnBatEchoQuad quads[32];
    Sint16 intensity, limit, retire_ticks, age;
    Uint16 phase, priority;
    bool facing_left, hidden, removed;
} SotnBatEcho;
/** @brief Actual player anchor/rotation and private tables at this child's update point. */
typedef struct SotnBatEchoInput
{
    SotnDataView dra, executable;
    Sint16 x, y, rotation;
    Uint16 priority;
    bool primitives_allocated; /**< Actual49-slot allocation result, read only on birth. */
    bool force_enabled;        /**< Owned Force of Echo snapshot, read only on birth. */
} SotnBatEchoInput;
/** @brief Original one-shot requests; no successful attack or allocation is manufactured. */
typedef struct SotnBatEchoEvents
{
    Uint16 sound;
    bool initialize_attack, disable_attack;
} SotnBatEchoEvents;
/** @brief Advance original Echo controller and all32 colored strip packets once.
 * @param echo Owned actual child, not ordinary saved progress.
 * @param input Private shape/color/SDK tables and actual player context.
 * @param events Receives this tick's requests, unchanged on failure.
 * @return Success or SDL error, retaining both outputs on malformed inputs/state.
 * Actual49-primitive exhaustion removes the child after its original sound request.
 * Force is sampled only at birth. The owner supplies spell-ten combat/group,
 * original blend/render ordering and room translation. Live Echo is not enabled. */
bool SotnBatEcho_Tick(SotnBatEcho *echo, SotnBatEchoInput input, SotnBatEchoEvents *events);
#endif
