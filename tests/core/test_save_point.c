#include <SDL3/SDL.h>
#include <sotn/save_point.h>

#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
int main(void)
{
    SotnSavePoint point = {0};
    SotnSavePointAction action = {0};
    SotnPlayerState player = {.x = 128 * 65536, .hp = 40, .step = SOTN_PLAYER_STAND};
    CHECK(!SotnSavePoint_Reset(NULL, 15, 1, 0));
    CHECK(!SotnSavePoint_Reset(&point, 16, 1, 0));
    CHECK(!SotnSavePoint_Tick(&point, player, 0, &action));
    CHECK(!SotnSavePoint_Complete(&point, true));
    CHECK(SotnSavePoint_Reset(&point, 15, 1, 0));
    CHECK(SotnSavePoint_Tick(&point, player, SOTN_BUTTON_UP, &action));
    CHECK(point.phase == SOTN_SAVE_POINT_IDLE && !action.heal);
    player.looking_up = true;
    player.x = 95 * 65536;
    CHECK(SotnSavePoint_Tick(&point, player, SOTN_BUTTON_UP, &action));
    CHECK(point.phase == SOTN_SAVE_POINT_IDLE);
    player.x = 100 * 65536;
    CHECK(SotnSavePoint_Tick(&point, player, SOTN_BUTTON_UP, &action));
    CHECK(point.phase == SOTN_SAVE_POINT_CENTER && action.heal && !action.write);
    CHECK(SotnSavePoint_Buttons(&point, player, 0) == SOTN_BUTTON_RIGHT);
    player.x = 150 * 65536;
    CHECK(SotnSavePoint_Buttons(&point, player, 0) == SOTN_BUTTON_LEFT);
    player.x = 128 * 65536;
    CHECK(SotnSavePoint_Tick(&point, player, 0, &action));
    CHECK(point.phase == SOTN_SAVE_POINT_SLOT && SotnSavePoint_Menu(&point));
    CHECK(SotnSavePoint_Tick(&point, player, SOTN_BUTTON_UP, &action));
    CHECK(point.slot == 15);
    CHECK(SotnSavePoint_Tick(&point, player, SOTN_BUTTON_UP, &action) && point.slot == 15);
    CHECK(SotnSavePoint_Tick(&point, player, SOTN_BUTTON_DOWN, &action) && point.slot == 1);
    CHECK(SotnSavePoint_Tick(&point, player, SOTN_BUTTON_JUMP, &action));
    CHECK(point.phase == SOTN_SAVE_POINT_CONFIRM && !action.write);
    CHECK(SotnSavePoint_Tick(&point, player, SOTN_BUTTON_RIGHT, &action) && !point.yes);
    CHECK(SotnSavePoint_Tick(&point, player, SOTN_BUTTON_JUMP, &action));
    CHECK(point.phase == SOTN_SAVE_POINT_IDLE && !action.write);
    CHECK(SotnSavePoint_Tick(&point, player, SOTN_BUTTON_UP, &action));
    CHECK(point.phase == SOTN_SAVE_POINT_IDLE); /* Release/repress required after cancellation. */
    CHECK(SotnSavePoint_Tick(&point, player, 0, &action));
    CHECK(SotnSavePoint_Tick(&point, player, SOTN_BUTTON_UP, &action));
    CHECK(SotnSavePoint_Tick(&point, player, 0, &action));
    CHECK(SotnSavePoint_Tick(&point, player, SOTN_BUTTON_START, &action));
    CHECK(SotnSavePoint_Tick(&point, player, 0, &action));
    CHECK(SotnSavePoint_Tick(&point, player, SOTN_BUTTON_START, &action));
    CHECK(point.phase == SOTN_SAVE_POINT_GROW);
    Uint32 writes = 0;
    for (Uint32 i = 0; i < 1600; ++i)
    {
        CHECK(SotnSavePoint_Tick(&point, player, SOTN_BUTTON_START, &action));
        if (action.write)
            ++writes;
    }
    CHECK(point.phase == SOTN_SAVE_POINT_WRITE && writes == 1);
    CHECK(SotnSavePoint_Complete(&point, false));
    CHECK(point.phase == SOTN_SAVE_POINT_ERROR && point.occupied == 0);
    CHECK(SotnSavePoint_Tick(&point, player, SOTN_BUTTON_START, &action) && !action.write);
    CHECK(SotnSavePoint_Tick(&point, player, 0, &action));
    CHECK(SotnSavePoint_Tick(&point, player, SOTN_BUTTON_START, &action) && action.write);
    CHECK(SotnSavePoint_Complete(&point, true));
    CHECK(point.phase == SOTN_SAVE_POINT_SAVED && point.occupied == 1);
    CHECK(SotnSavePoint_Tick(&point, player, SOTN_BUTTON_CANCEL, &action));
    CHECK(point.phase == SOTN_SAVE_POINT_EXIT);
    for (Uint32 i = 0; i < 128; ++i)
        CHECK(SotnSavePoint_Tick(&point, player, SOTN_BUTTON_CANCEL, &action));
    CHECK(point.phase == SOTN_SAVE_POINT_IDLE && !action.write);
    CHECK(SotnSavePoint_Buttons(&point, player, SOTN_BUTTON_LEFT) == SOTN_BUTTON_LEFT);
    SDL_Log("SOTN_SAVE_POINT_TEST: PASS");
    return 0;
}
