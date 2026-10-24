#ifndef SOTN_INVENTORY_MENU_H
#define SOTN_INVENTORY_MENU_H
#include <sotn/character.h>
#include <sotn/player.h>
/** @brief Supported original pause/equipment navigation states. */
typedef enum SotnInventoryMenuPhase
{
    SOTN_MENU_CLOSED,
    SOTN_MENU_MAIN,
    SOTN_MENU_EQUIPMENT,
    SOTN_MENU_ITEMS
} SotnInventoryMenuPhase;
/** @brief POD cursor; no renderer, input-device, or filesystem ownership. */
typedef struct SotnInventoryMenu
{
    SotnInventoryMenuPhase phase;
    Uint16 previous, cursor, item_cursor, rows[170], count, repeat, ticks;
    Uint8 slot;
    char message[96];
} SotnInventoryMenu;
/** @brief Advance original Start/confirm/back and selected equipment navigation.
 * @param menu Initialized zeroed cursor. @param character Borrowed writable ownership.
 * @param buttons Held original button bits. @param sound One-tick original UI sound request.
 * @param changed True after a successful equipment exchange.
 * @return Success or false with SDL_GetError(); unsupported selections remain visible in menu. */
bool SotnInventoryMenu_Tick(SotnInventoryMenu *menu, SotnCharacter *character, Uint16 buttons,
                            Uint16 *sound, bool *changed);
#endif
