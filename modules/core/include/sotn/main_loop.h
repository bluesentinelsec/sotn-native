#ifndef SOTN_MAIN_LOOP_H
#define SOTN_MAIN_LOOP_H
#include <sotn/playtime.h>

/** @brief Copied MainGame tail inputs/outputs, never a second live RNG or timer pool. */
typedef struct SotnMainLoopTail
{
    SotnPlaytime playtime;
    Uint32 map_cursor, random;
} SotnMainLoopTail;
/** @brief Execute the original post-render status/map/RNG tail once.
 * @param tail Borrowed working snapshot; all fields commit together on success.
 * @param vcount Raw signed SDK vertical count. Native fixed60 supplies256.
 * @param menu Post-update inventory pause flag; holds only status time.
 * @param map_pause Post-update map/status pause flag; holds only status time.
 * @return Success or SDL error for NULL. The map word wraps by normalized count;
 * the existing native libc adapter advances once even for zero count/paused time.
 * The caller must bind outputs to its actual owners after a successful frame.
 * This does not supply BIOS initial state, SDK raster timing or other RNG owners. */
bool SotnMainLoop_Advance(SotnMainLoopTail *tail, Sint32 vcount, bool menu, bool map_pause);
#endif
