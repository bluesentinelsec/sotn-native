#include <sotn/launch_menu.h>
#include <sotn/player.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s", __LINE__, #v);                                                  \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
int main(void)
{
    SotnLaunchMenu m = {0};
    SotnLaunchAction a = SOTN_LAUNCH_NONE;
    CHECK(SotnLaunchMenu_Reset(&m, SOTN_BUTTON_START, 15));
    CHECK(SotnLaunchMenu_Tick(&m, SOTN_BUTTON_START, 0x4000, &a) && m.phase == SOTN_LAUNCH_HOME &&
          a == SOTN_LAUNCH_NONE);
    CHECK(SotnLaunchMenu_Tick(&m, 0, 0x4000, &a));
    CHECK(SotnLaunchMenu_Tick(&m, SOTN_BUTTON_JUMP, 0x4000, &a) && m.phase == SOTN_LAUNCH_SLOTS);
    CHECK(SotnLaunchMenu_Tick(&m, 0, 0x4000, &a));
    CHECK(SotnLaunchMenu_Tick(&m, SOTN_BUTTON_JUMP, 0x4000, &a) && a == SOTN_LAUNCH_LOAD &&
          m.slot == 15);
    CHECK(SotnLaunchMenu_Tick(&m, SOTN_BUTTON_JUMP, 0x4000, &a) && a == SOTN_LAUNCH_NONE);
    CHECK(SotnLaunchMenu_Tick(&m, SOTN_BUTTON_DOWN, 0, &a) && m.slot == 1);
    CHECK(SotnLaunchMenu_Tick(&m, SOTN_BUTTON_UP, 0, &a) && m.slot == 15);
    CHECK(SotnLaunchMenu_Tick(&m, SOTN_BUTTON_JUMP, 0, &a) && m.phase == SOTN_LAUNCH_ERROR &&
          a == SOTN_LAUNCH_NONE);
    CHECK(SotnLaunchMenu_Tick(&m, SOTN_BUTTON_CANCEL, 0, &a) && m.phase == SOTN_LAUNCH_SLOTS);
    CHECK(SotnLaunchMenu_Tick(&m, 0, 0, &a));
    CHECK(SotnLaunchMenu_Tick(&m, SOTN_BUTTON_CANCEL, 0, &a) && m.phase == SOTN_LAUNCH_HOME);
    CHECK(SotnLaunchMenu_Tick(&m, SOTN_BUTTON_DOWN, 0, &a) && m.choice == 1);
    CHECK(SotnLaunchMenu_Tick(&m, SOTN_BUTTON_JUMP, 0, &a) && a == SOTN_LAUNCH_CHECKPOINT);
    CHECK(!SotnLaunchMenu_Reset(&m, 0, 16));
    CHECK(!SotnLaunchMenu_Tick(&m, 0, 0x8000, &a));
    m.slot = 0;
    CHECK(!SotnLaunchMenu_Tick(&m, 0, 0, &a));
    SDL_Log("LAUNCH_MENU: PASS");
    return 0;
}
