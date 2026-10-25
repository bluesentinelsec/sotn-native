#include <sotn/launch_menu.h>
#include <sotn/player.h>
bool SotnLaunchMenu_Reset(SotnLaunchMenu *menu, Uint16 buttons, Uint32 slot)
{
    if (menu == NULL || slot < 1 || slot > 15)
        return SDL_SetError("Launch menu requires a valid native slot");
    *menu = (SotnLaunchMenu){.previous = buttons, .slot = (Uint8)slot};
    return true;
}
bool SotnLaunchMenu_Tick(SotnLaunchMenu *menu, Uint16 buttons, Uint16 available,
                         SotnLaunchAction *action)
{
    if (menu == NULL || action == NULL || menu->phase > SOTN_LAUNCH_ERROR ||
        menu->phase < SOTN_LAUNCH_HOME || menu->choice > 1 || menu->slot < 1 || menu->slot > 15 ||
        (available & 0x8000))
        return SDL_SetError("Launch menu state is invalid");
    *action = SOTN_LAUNCH_NONE;
    const Uint16 tapped = (Uint16)(buttons & ~menu->previous);
    menu->previous = buttons;
    const bool confirm = (tapped & (SOTN_BUTTON_START | SOTN_BUTTON_JUMP)) != 0;
    if (menu->phase == SOTN_LAUNCH_ERROR)
    {
        if (confirm || (tapped & SOTN_BUTTON_CANCEL))
            menu->phase = SOTN_LAUNCH_SLOTS;
    }
    else if (menu->phase == SOTN_LAUNCH_HOME)
    {
        if (tapped & (SOTN_BUTTON_UP | SOTN_BUTTON_DOWN))
            menu->choice ^= 1;
        if (confirm)
        {
            if (menu->choice == 1)
                *action = SOTN_LAUNCH_CHECKPOINT;
            else
                menu->phase = SOTN_LAUNCH_SLOTS;
        }
    }
    else
    {
        if (tapped & SOTN_BUTTON_CANCEL)
            menu->phase = SOTN_LAUNCH_HOME;
        else
        {
            if (tapped & SOTN_BUTTON_UP)
                menu->slot = (Uint8)(menu->slot == 1 ? 15 : menu->slot - 1);
            if (tapped & SOTN_BUTTON_DOWN)
                menu->slot = (Uint8)(menu->slot == 15 ? 1 : menu->slot + 1);
            if (confirm)
            {
                if (available & (1U << (menu->slot - 1)))
                    *action = SOTN_LAUNCH_LOAD;
                else
                    menu->phase = SOTN_LAUNCH_ERROR;
            }
        }
    }
    return true;
}
