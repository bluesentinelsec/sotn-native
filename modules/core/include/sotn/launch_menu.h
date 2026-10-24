#ifndef SOTN_LAUNCH_MENU_H
#define SOTN_LAUNCH_MENU_H
#include <SDL3/SDL.h>
/** @brief Native filesystem launch/continue UI states; original title/file menus are separate. */
typedef enum SotnLaunchPhase
{
    SOTN_LAUNCH_HOME,
    SOTN_LAUNCH_SLOTS,
    SOTN_LAUNCH_ERROR
} SotnLaunchPhase;
typedef enum SotnLaunchAction
{
    SOTN_LAUNCH_NONE,
    SOTN_LAUNCH_CHECKPOINT,
    SOTN_LAUNCH_LOAD
} SotnLaunchAction;
/** @brief Initialized controller; no file access, game state, or pointers. */
typedef struct SotnLaunchMenu
{
    SotnLaunchPhase phase;
    Uint16 previous;
    Uint8 choice, slot;
} SotnLaunchMenu;
/** @brief Reset menu and suppress held input from the previous screen.
 * @param menu Writable controller. @param buttons Current original button bits.
 * @param slot One-based selected slot, 1..15. @return Success or SDL error. */
bool SotnLaunchMenu_Reset(SotnLaunchMenu *menu, Uint16 buttons, Uint32 slot);
/** @brief Advance on tapped controls; request a checked load rather than doing I/O.
 * @param menu Initialized controller. @param buttons Held original buttons.
 * @param available Bitmask of validated ordinary slots (bit 0 is slot 1).
 * @param action Receives one action, otherwise NONE. @return Success or SDL error.
 * A disabled slot produces an error state; cancel returns to the preceding menu. */
bool SotnLaunchMenu_Tick(SotnLaunchMenu *menu, Uint16 buttons, Uint16 available,
                         SotnLaunchAction *action);
#endif
