#include <sotn/hellfire_cast.h>
#include <sotn/player.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
static bool Is(SotnHellfireCastEvents e, Uint16 i, SotnHellfireCastActionKind kind, Uint16 value)
{
    return i < e.count && e.actions[i].kind == kind && e.actions[i].value == value;
}
int main(void)
{
    SotnHellfireCast c = {0};
    SotnHellfireCastInput in = {.root_created = true};
    SotnHellfireCastEvents e = {0};
    CHECK(SotnHellfireCast_Tick(&c, in, &e) && c.phase == 1 && !c.finished && e.count == 2 &&
          Is(e, 0, SOTN_HELLFIRE_FACTORY, 33) && Is(e, 1, SOTN_HELLFIRE_ANIMATION, 1));
    CHECK(SotnHellfireCast_Tick(&c, in, &e) && c.phase == 1 && !e.count);
    in.signal = 1;
    CHECK(SotnHellfireCast_Tick(&c, in, &e) && c.phase == 2 && !e.count);
    in.signal = 0;
    in.buttons = SOTN_BUTTON_RIGHT;
    in.x = 248;
    CHECK(SotnHellfireCast_Tick(&c, in, &e) && !c.velocity_x);
    in.x++;
    CHECK(SotnHellfireCast_Tick(&c, in, &e) && c.velocity_x == 196608);
    in.x = 100;
    in.floor_right = 1;
    c.facing_left = true;
    CHECK(SotnHellfireCast_Tick(&c, in, &e) && c.velocity_x == 196608 && e.turned &&
          !c.facing_left);
    in.movement_flags = 2;
    CHECK(SotnHellfireCast_Tick(&c, in, &e) && !c.velocity_x && !e.turned);
    in.movement_flags = 0;
    in.buttons = SOTN_BUTTON_LEFT;
    in.floor_right = 0;
    in.floor_left = 0x8000;
    CHECK(SotnHellfireCast_Tick(&c, in, &e) && c.velocity_x == -196608 && e.turned &&
          c.facing_left);
    in.floor_left = 0;
    in.x = 8;
    CHECK(SotnHellfireCast_Tick(&c, in, &e) && !c.velocity_x);
    in.x = 7;
    CHECK(SotnHellfireCast_Tick(&c, in, &e) && c.velocity_x == -196608);
    in.floor_center = 0x8000;
    in.x = 100;
    in.buttons = SOTN_BUTTON_RIGHT | SOTN_BUTTON_LEFT;
    CHECK(SotnHellfireCast_Tick(&c, in, &e) && c.velocity_x == 196608 && !c.facing_left);
    in.signal = 2;
    CHECK(SotnHellfireCast_Tick(&c, in, &e) && c.phase == 3 && !c.velocity_x && e.count == 1 &&
          Is(e, 0, SOTN_HELLFIRE_ANIMATION, 0x3d));
    in.signal = 3;
    CHECK(SotnHellfireCast_Tick(&c, in, &e) && c.phase == 4 && e.count == 1 &&
          Is(e, 0, SOTN_HELLFIRE_ANIMATION, 0x3c));
    in.pose = 10;
    in.remaining = 1;
    CHECK(SotnHellfireCast_Tick(&c, in, &e) && c.phase == 4 && e.count == 2 &&
          Is(e, 0, SOTN_HELLFIRE_USE_TIMER, 4) && Is(e, 1, SOTN_HELLFIRE_FACTORY, 37));
    in.remaining = -1;
    CHECK(SotnHellfireCast_Tick(&c, in, &e) && c.finished && !c.phase && e.count == 3 &&
          Is(e, 0, SOTN_HELLFIRE_NORMAL, 0) && Is(e, 1, SOTN_HELLFIRE_ANIMATION, 0x3d) &&
          Is(e, 2, SOTN_HELLFIRE_RESTORE_FLASHES, 0));
    c = (SotnHellfireCast){.phase = 3};
    in = (SotnHellfireCastInput){.buttons = SOTN_BUTTON_DOWN, .signal = 2};
    CHECK(SotnHellfireCast_Tick(&c, in, &e) && !c.finished && !e.count);
    in.signal = 3;
    in.normal_phase = 2;
    CHECK(SotnHellfireCast_Tick(&c, in, &e) && c.finished && c.phase == 2 && e.count == 4 &&
          Is(e, 0, SOTN_HELLFIRE_ANIMATION, 0x3c) && Is(e, 1, SOTN_HELLFIRE_NORMAL, 0));
    c = (SotnHellfireCast){0};
    in = (SotnHellfireCastInput){.normal_phase = 2};
    CHECK(SotnHellfireCast_Tick(&c, in, &e) && c.finished && c.phase == 3 && e.count == 3 &&
          Is(e, 0, SOTN_HELLFIRE_FACTORY, 33) && Is(e, 1, SOTN_HELLFIRE_NORMAL, 0) &&
          Is(e, 2, SOTN_HELLFIRE_ANIMATION, 1));
    c = (SotnHellfireCast){0};
    in.signal = 0xffff;
    CHECK(SotnHellfireCast_Tick(&c, in, &e) && c.finished && c.phase == 2 && e.count == 6 &&
          Is(e, 3, SOTN_HELLFIRE_NORMAL, 0) && Is(e, 4, SOTN_HELLFIRE_ANIMATION, 0x3d) &&
          Is(e, 5, SOTN_HELLFIRE_RESTORE_FLASHES, 0));
    const SotnHellfireCast before = c;
    const SotnHellfireCastEvents previous = e;
    CHECK(!SotnHellfireCast_Tick(&c, in, &e) && !SDL_memcmp(&c, &before, sizeof(c)) &&
          !SDL_memcmp(&e, &previous, sizeof(e)));
    c = (SotnHellfireCast){.phase = 5};
    CHECK(!SotnHellfireCast_Tick(&c, in, &e));
    c.phase = 0;
    in.normal_phase = 1;
    CHECK(!SotnHellfireCast_Tick(&c, in, &e));
    SDL_Log("SOTN_HELLFIRE_CAST_TEST: PASS");
    return 0;
}
