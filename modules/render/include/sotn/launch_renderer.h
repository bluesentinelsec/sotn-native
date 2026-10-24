#ifndef SOTN_LAUNCH_RENDERER_H
#define SOTN_LAUNCH_RENDERER_H
#include <sotn/launch_menu.h>
#include <sotn/tilemap.h>
/** @brief Copied slot summary; no private assets, pointers, or transient gameplay state. */
typedef struct SotnLaunchSlot
{
    bool exists, valid;
    Uint16 level, hp;
    Uint32 room;
    Uint16 stage;
} SotnLaunchSlot;
/** @brief Draw the native checkpoint/Continue shell with the original common font.
 * @param common Borrowed verified F_GAME.BIN. @param menu Valid initialized controller.
 * @param slots Exactly 15 validated summaries. @param error Optional bounded ASCII error.
 * @param frame Full writable RGB555 output. @param count At least SOTN_FRAME_PIXELS.
 * @return Success or SDL error. Validate inputs before modifying frame.
 * @note This filesystem UI is not the original title/memory-card presentation. */
bool SotnLaunchRenderer_Draw(SotnDataView common, SotnLaunchMenu menu,
                             const SotnLaunchSlot slots[15], const char *error, Uint16 *frame,
                             size_t count);
#endif
