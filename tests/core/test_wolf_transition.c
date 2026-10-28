#include <SDL3/SDL.h>
#include <sotn/wolf_transition.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static SotnWolfTransition Frame(void)
{
    return (SotnWolfTransition){.player = {.motion = {.animation = {.id = 22, .sprite = 7},
                                                      .body_angle = 2048,
                                                      .velocity_x = 65536,
                                                      .velocity_y = -131072},
                                           .priority = 144,
                                           .run_window = 24,
                                           .direction_locked = true},
                                .floor_y = {8, 8, 8, 8},
                                .ceiling_y = {-8, -8, -8, -8},
                                .bank = 1,
                                .palette = 0x8100,
                                .rotation = 384,
                                .movement = {2, 0x100, 8}};
}
static bool Entry(void)
{
    SotnWolfTransition t = Frame(), before = t;
    SotnWolfTransitionEvents e = {0};
    SotnWolfTransitionInput in = {.center_priority = 144};
    for (Uint16 i = 0; i < 20; ++i)
    {
        CHECK(SotnWolfTransition_Tick(&t, in, &e));
        CHECK(e.entry_flash && !e.replace_body && !SDL_memcmp(&t, &before, sizeof(t)));
    }
    in.entry_factory_created = true;
    t.timer14 = 20;
    CHECK(SotnWolfTransition_Tick(&t, in, &e) && e.entry_flash && e.clear_weapon &&
          e.replace_body && !e.finish_flash);
    CHECK(t.saved_bank == 1 && t.saved_sprite == 7 && t.bank == 15 && t.texture == 126 &&
          t.palette == 0x810d && t.signal == 1 && t.timer14 == 20);
    CHECK(t.player.motion.animation.id == 0xe8 && t.player.motion.animation.pose == 4 &&
          t.player.motion.animation.remaining == 4 && t.player.motion.substep == 8 &&
          !t.player.motion.velocity_x && !t.player.motion.velocity_y &&
          t.player.motion.body_angle == 2048 && t.player.priority == 142 &&
          t.player.coyote_ticks == 8 && !t.player.run_window && !t.player.direction_locked &&
          !t.rotation && !t.movement[0] && !t.movement[1] && !t.movement[2]);
    for (Uint16 i = 0; i < 30; ++i)
    {
        CHECK(SotnWolfTransition_Tick(&t, in, &e));
        CHECK(t.player.motion.substep == 8 && t.signal == 1 && !e.entry_flash && !e.finish_flash &&
              !e.replace_body);
    }
    CHECK(t.floor_y[0] == 29 && t.floor_y[1] == 25 && t.floor_y[2] == 25 && t.floor_y[3] == 25);
    t.signal = 3;
    CHECK(SotnWolfTransition_Tick(&t, in, &e) && t.player.motion.substep == 5 &&
          t.player.motion.velocity_y == -98304 && t.palette == 0x104);
    t = Frame();
    t.bank = 13;
    t.signal = 65535;
    CHECK(SotnWolfTransition_Tick(&t, in, &e) && t.signal == 0 && t.saved_bank == 13);
    t = Frame();
    in.mist_child = true;
    in.entry_factory_created = false;
    CHECK(SotnWolfTransition_Tick(&t, in, &e) && e.replace_body && !e.entry_flash &&
          !e.clear_weapon && t.signal == 0);
    CHECK(SotnWolfTransition_Tick(&t, in, &e) && t.player.motion.substep == 8 && !e.finish_flash);
    in.mist_ready = true;
    CHECK(SotnWolfTransition_Tick(&t, in, &e) && e.finish_flash && t.player.motion.substep == 5 &&
          t.palette == 0x104);
    return true;
}
static bool Return(void)
{
    SotnWolfTransition t = Frame();
    t.returning = true;
    t.bank = 15;
    t.texture = 126;
    t.draw_flags = 4;
    t.signal = 0;
    t.timer14 = 2;
    SotnWolfTransitionEvents e = {0};
    const SotnWolfTransitionInput in = {.center_priority = 144};
    for (Uint16 i = 0; i < 14; ++i)
    {
        CHECK(SotnWolfTransition_Tick(&t, in, &e));
        CHECK(!t.player.motion.substep && !e.normal_return && !e.stuck_return);
    }
    CHECK(t.ceiling_y[0] == -22);
    CHECK(SotnWolfTransition_Tick(&t, in, &e) && t.player.motion.substep == 1 && t.signal == 1 &&
          t.bank == 1 && !t.texture && !t.rotation && !t.draw_flags && t.form_pose == 0x5f &&
          t.player.motion.animation.id == 0xca);
    for (Uint16 i = 0; i < 25; ++i)
    {
        CHECK(SotnWolfTransition_Tick(&t, in, &e) && !e.normal_return && !e.stuck_return);
    }
    t.signal = 3;
    CHECK(SotnWolfTransition_Tick(&t, in, &e) && e.normal_return && !e.stuck_return);
    t = Frame();
    t.returning = true;
    t.player.motion.contacts = 3;
    CHECK(SotnWolfTransition_Tick(&t, in, &e) && t.cramped && !t.player.motion.velocity_x &&
          !t.player.motion.velocity_y);
    for (Uint16 i = 0; i < 14; ++i)
        CHECK(SotnWolfTransition_Tick(&t, in, &e));
    CHECK(t.player.motion.substep == 2 && t.form_pose == 0xc7);
    t.signal = 3;
    CHECK(SotnWolfTransition_Tick(&t, in, &e) && e.stuck_return && !e.normal_return &&
          t.player.priority == 144);
    t = Frame();
    t.returning = true;
    t.player.motion.contacts = 1;
    t.player.motion.velocity_x = -8193;
    t.player.motion.velocity_y = -100;
    CHECK(SotnWolfTransition_Tick(&t, in, &e) && t.player.motion.velocity_x == -1 &&
          !t.player.motion.velocity_y);
    return true;
}
static bool Invalid(void)
{
    SotnWolfTransition t = Frame();
    t.player.motion.substep = 7;
    const SotnWolfTransition before = t;
    SotnWolfTransitionEvents e = {.clear_weapon = true}, prior = e;
    SotnWolfTransitionInput in = {.center_priority = 144};
    CHECK(!SotnWolfTransition_Tick(&t, in, &e) && !SDL_memcmp(&t, &before, sizeof(t)) &&
          !SDL_memcmp(&e, &prior, sizeof(e)));
    t = Frame();
    t.ceiling_y[3] = 65;
    CHECK(!SotnWolfTransition_Tick(&t, in, &e));
    t = Frame();
    in.center_priority = 1;
    CHECK(!SotnWolfTransition_Tick(&t, in, &e));
    CHECK(!SotnWolfTransition_Tick(NULL, in, &e) && !SotnWolfTransition_Tick(&t, in, NULL));
    return true;
}
int main(void)
{
    return Entry() && Return() && Invalid() ? 0 : 1;
}
