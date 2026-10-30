#ifndef SOTN_WATER_DROPS_H
#define SOTN_WATER_DROPS_H
#include <sotn/game_math.h>

/** @brief Actual water tile packet and its separately retained packed control words. */
typedef struct SotnWaterDropTile
{
    Sint32 fixed_x, fixed_y, velocity_y;
    Sint16 x, y, delay;
    Uint16 priority, draw_mode;
    Uint8 red, green, blue, width, height;
} SotnWaterDropTile;
/** @brief One small drop or original32-particle owner with its actual trailing sentinel. */
typedef struct SotnWaterDrops
{
    SotnWaterDropTile tiles[33];
    Sint32 x, y, velocity_x, velocity_y, gravity;
    Uint16 parameter, timer;
    Uint8 phase;
    bool small, removed;
} SotnWaterDrops;
/** @brief Private ordinary188-byte water block and actual tile lease/context. */
typedef struct SotnWaterDropsInput
{
    SotnDataView tables;
    Uint16 allocated_rows, priority;
} SotnWaterDropsInput;
/** @brief Advance original small-drop or32-particle behavior once.
 * @param drops Copied actual owner with birth origin, parameter and actual role.
 * @param input Real lease: one small row or33 contiguous large rows, including sentinel.
 * @param random Actual stage Random state for small drops; actual libc rand state for large.
 * @return Success or SDL error preserving state/RNG on invalid ownership/data. Allocation
 * failure retires before private reads/RNG. Small birth consumes two stage values and large
 * birth consumes128 libc values; draws never tick. The caller owns actual creation, flags,
 * primitive reservations and rendering; this interface creates no pool or player feedback.
 */
bool SotnWaterDrops_Tick(SotnWaterDrops *drops, SotnWaterDropsInput input, Uint32 *random);
/** @brief Apply actual screen-space camera shifts without changing packed control fractions.
 * @param drops Actual owner. @param dx Signed-word X delta. @param dy Signed-word Y delta.
 * @return Success or SDL error without mutation. All real rows, including sentinel, shift;
 * the next particle update restores sentinel coordinates through its original behavior.
 */
bool SotnWaterDrops_Translate(SotnWaterDrops *drops, Sint32 dx, Sint32 dy);
#endif
