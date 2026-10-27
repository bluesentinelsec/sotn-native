#include <SDL3/SDL.h>
#include <sotn/mist_motion.h>
#include <sotn/player.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)

static int Duration(SotnWorldState *world)
{
    SotnMistMotion motion = {.vx = 65536, .vy = -200000, .settle_y = 2, .facing_left = true};
    SotnMistMotionEvents event = {0};
    SotnMistInput input = {.world = world,
                           .mp = 99,
                           .directions = SOTN_BUTTON_RIGHT,
                           .cancel = true,
                           .underwater = true};
    // Entry initialization precedes active-form cancellation and MP reduction.
    CHECK(SotnMistMotion_Tick(&motion, input, &event) && event.started && !event.ended &&
          !event.spent && event.turned && !motion.facing_left && motion.vx == 32768 &&
          motion.vy == -32768 && motion.phase == 1 && motion.remaining == 16 &&
          motion.settle_y == 1 && event.shift_y == 1);
    input.cancel = input.underwater = false;
    Uint16 total = 0;
    for (Uint32 tick = 1; tick <= 16; ++tick)
    {
        input.game_timer = tick;
        CHECK(SotnMistMotion_Tick(&motion, input, &event) && !event.started);
        total = (Uint16)(total + event.spent);
        CHECK(event.ended == (tick == 16) && motion.remaining == 16 - tick);
    }
    CHECK(total == 20 && motion.ended && !motion.settle_y && !event.shift_y);
    const Uint32 remaining = motion.remaining;
    CHECK(!SotnMistMotion_Tick(&motion, input, &event) && motion.remaining == remaining);
    return 0;
}
static int Steering(SotnWorldState *world)
{
    SotnMistInput input = {.world = world,
                           .mp = 99,
                           .game_timer = 1,
                           .directions = SOTN_BUTTON_UP | SOTN_BUTTON_RIGHT};
    SotnMistMotion motion = {
        .vx = 65536, .vy = -65536, .phase = 1, .remaining = 10, .facing_left = true};
    SotnMistMotionEvents event = {0};
    CHECK(SotnMistMotion_Tick(&motion, input, &event) && motion.vx == 65024 &&
          motion.vy == -65024 && !motion.facing_left && !event.turned);
    motion.vx = motion.vy = 0;
    CHECK(SotnMistMotion_Tick(&motion, input, &event) && motion.vx == 20480 && motion.vy == -20480);
    input.directions = SOTN_BUTTON_LEFT;
    CHECK(SotnMistMotion_Tick(&motion, input, &event) && motion.vx == -32768 &&
          motion.vy == -19712 && motion.facing_left);
    motion.vx = 512;
    motion.vy = -512;
    input.directions = SOTN_BUTTON_LEFT | SOTN_BUTTON_RIGHT;
    CHECK(SotnMistMotion_Tick(&motion, input, &event) && !motion.vx && !motion.vy &&
          motion.facing_left);
    input.cancel = true;
    input.game_timer = 0;
    CHECK(SotnMistMotion_Tick(&motion, input, &event) && motion.ended && event.ended &&
          event.turned && !motion.facing_left && !event.spent);
    return 0;
}
static int PowerAndFailures(SotnWorldState *world)
{
    world->relics[SOTN_RELIC_POWER_MIST] = 3;
    SotnMistMotion motion = {.vx = -1, .vy = 1, .settle_y = -2};
    SotnMistInput input = {.world = world, .mp = 3, .game_timer = 0};
    SotnMistMotionEvents event = {0};
    CHECK(SotnMistMotion_Tick(&motion, input, &event) && event.started && !event.spent &&
          motion.remaining == 100000 && motion.vx == -65536 && motion.vy == 65536 &&
          event.shift_y == -1 && motion.settle_y == -1);
    input.game_timer = 30;
    input.directions = SOTN_BUTTON_DOWN | SOTN_BUTTON_LEFT;
    CHECK(SotnMistMotion_Tick(&motion, input, &event) && event.spent == 2 &&
          motion.remaining == 100000 && motion.vx == -65024 && motion.vy == 65024);
    input.mp = (Uint16)(input.mp - event.spent);
    input.game_timer = 31;
    CHECK(SotnMistMotion_Tick(&motion, input, &event) && motion.ended && !event.spent);
    // Disabling Power of Mist retains the original long duration instead of resetting it.
    motion = (SotnMistMotion){.phase = 1, .remaining = 100000};
    world->relics[SOTN_RELIC_POWER_MIST] = 1;
    input.mp = 99;
    CHECK(SotnMistMotion_Tick(&motion, input, &event) && motion.remaining == 99999);
    world->relics[SOTN_RELIC_MIST] = 1;
    CHECK(SotnMistMotion_Tick(&motion, input, &event) && event.ended && !event.spent);
    motion = (SotnMistMotion){.phase = 1, .remaining = 7, .facing_left = true};
    input.direction_locked = input.underwater = true;
    input.directions = SOTN_BUTTON_RIGHT;
    CHECK(SotnMistMotion_Tick(&motion, input, &event) && motion.facing_left && !event.turned &&
          motion.remaining == 7 && !event.spent);
    motion = (SotnMistMotion){.phase = 2};
    event = (SotnMistMotionEvents){.spent = 777};
    CHECK(!SotnMistMotion_Tick(&motion, input, &event) && motion.phase == 2 && event.spent == 777);
    motion.phase = 0;
    input.directions = SOTN_BUTTON_JUMP;
    CHECK(!SotnMistMotion_Tick(&motion, input, &event) && !motion.phase && event.spent == 777);
    CHECK(!SotnMistMotion_Tick(NULL, input, &event));
    CHECK(!SotnMistMotion_Tick(&motion, input, NULL));
    return 0;
}
int main(void)
{
    SotnWorldState world = {0};
    world.relics[SOTN_RELIC_MIST] = 3;
    CHECK(Duration(&world) == 0 && Steering(&world) == 0 && PowerAndFailures(&world) == 0);
    return 0;
}
