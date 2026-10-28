#include <SDL3/SDL.h>
#include <sotn/collision.h>
#include <sotn/player.h>
#include <sotn/wolf_controller.h>

#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static SotnWolfControllerFrame Frame(Uint16 step, Uint16 mode)
{
    return (SotnWolfControllerFrame){.motion = {.animation = {.id = 0xe0},
                                                .substep = step,
                                                .mode = mode,
                                                .body_angle = 2048,
                                                .contacts = 1},
                                     .x = 10 * 65536 + 13,
                                     .y = 135 * 65536,
                                     .root_y = 135,
                                     .priority = 146,
                                     .mp = 20,
                                     .game_timer = 1};
}
static bool Sprint(SotnDataView dra)
{
    SotnWolfControllerEvents e = {0};
    SotnWolfControllerFrame f = Frame(2, 2);
    f.motion.facing_left = true;
    f.motion.buttons = SOTN_BUTTON_LEFT;
    f.motion.velocity_x = -2 * 65536;
    CHECK(SotnWolfController_Tick(&f, dra, &e) && f.motion.velocity_x == -3 * 65536);
    f = Frame(2, 2);
    f.motion.buttons = SOTN_BUTTON_RIGHT;
    f.motion.velocity_x = 2 * 65536;
    CHECK(SotnWolfController_Tick(&f, dra, &e) && f.motion.velocity_x == 132608);
    f = Frame(2, 2);
    f.motion.facing_left = true;
    f.motion.buttons = SOTN_BUTTON_LEFT;
    f.motion.velocity_x = -2 * 65536;
    f.power_enabled = true;
    CHECK(SotnWolfController_Tick(&f, dra, &e) && f.motion.velocity_x == -132608);
    f = Frame(2, 0);
    f.motion.animation.pose = 2;
    f.motion.animation.remaining = 1;
    f.motion.buttons = SOTN_BUTTON_LEFT;
    CHECK(SotnWolfController_Tick(&f, dra, &e) && f.motion.facing_left &&
          f.motion.velocity_x == 65536 && !e.turned);
    f = Frame(1, 0);
    f.idle_timer = 1;
    CHECK(SotnWolfController_Tick(&f, dra, &e) && f.motion.animation.id == 0xe9 &&
          f.motion.mode == 1 && !f.idle_timer);
    f = Frame(1, 0);
    f.idle_timer = 1;
    f.root_y = 131;
    CHECK(SotnWolfController_Tick(&f, dra, &e) && f.idle_timer == 1);
    return true;
}
static bool Charge(SotnDataView dra)
{
    SotnWolfControllerFrame f = Frame(1, 1);
    f.tapped = SOTN_BUTTON_LEFT_HAND;
    f.motion.buttons = SOTN_BUTTON_LEFT;
    f.quarter_circle_complete = f.skill_enabled = true;
    f.mp = 5;
    SotnWolfControllerEvents e = {0};
    CHECK(SotnWolfController_Tick(&f, dra, &e));
    CHECK(e.attack_requested && e.learn_charge && e.mp_spent == 5 && e.turned && f.mp == 0 &&
          f.motion.substep == 2 && f.motion.mode == 4 && f.motion.animation.id == 0xed &&
          f.motion.velocity_x == -5 * 65536);
    f = Frame(1, 1);
    f.tapped = SOTN_BUTTON_LEFT_HAND;
    f.quarter_circle_complete = f.skill_enabled = true;
    f.mp = 4;
    CHECK(SotnWolfController_Tick(&f, dra, &e) && e.attack_requested && !e.learn_charge &&
          !e.mp_spent && f.mp == 4 && f.motion.substep == 1);
    f = Frame(1, 1);
    f.tapped = SOTN_BUTTON_RIGHT_HAND;
    f.quarter_circle_complete = f.skill_enabled = true;
    CHECK(SotnWolfController_Tick(&f, (SotnDataView){0}, &e) && !e.attack_requested);
    f.tapped = SOTN_BUTTON_LEFT_HAND;
    const SotnWolfControllerFrame before = f;
    e = (SotnWolfControllerEvents){.count = 7};
    const SotnWolfControllerEvents before_events = e;
    CHECK(!SotnWolfController_Tick(&f, (SotnDataView){0}, &e));
    CHECK(!SDL_memcmp(&before, &f, sizeof(f)) && !SDL_memcmp(&before_events, &e, sizeof(e)));
    return true;
}
static bool CollisionTransitions(SotnDataView dra)
{
    SotnWolfControllerFrame f = Frame(4, 0);
    f.motion.contacts = 2;
    f.motion.velocity_x = 2 * 65536;
    f.motion.velocity_y = -6 * 65536;
    f.motion.buttons = SOTN_BUTTON_RIGHT;
    SotnWolfControllerEvents e = {0};
    CHECK(SotnWolfController_Tick(&f, dra, &e));
    CHECK(f.motion.substep == 5 && f.motion.velocity_x == 65536 && !f.motion.velocity_y &&
          f.coyote_ticks == 8 && e.count == 3 && e.requests[0].kind == SOTN_WOLF_FACTORY &&
          e.requests[0].parameter == 0x10004 && e.requests[0].source.x == 13 * 65536 + 13 &&
          f.x == 10 * 65536 + 13 && e.requests[1].kind == SOTN_WOLF_SHAKE &&
          e.requests[2].kind == SOTN_WOLF_SOUND);
    f = Frame(2, 2);
    f.power_enabled = true;
    f.motion.contacts = 5;
    f.motion.velocity_x = 5 * 65536;
    f.motion.buttons = SOTN_BUTTON_RIGHT;
    CHECK(SotnWolfController_Tick(&f, dra, &e) && e.input_hold == 32 && !e.return_requested &&
          f.motion.substep == 5 && f.motion.velocity_y == -229376 && e.count == 3);
    f = Frame(2, 2);
    f.power_enabled = true;
    f.motion.contacts = 5;
    f.motion.velocity_x = 6 * 65536;
    f.motion.buttons = SOTN_BUTTON_RIGHT;
    CHECK(SotnWolfController_Tick(&f, dra, &e) && e.return_requested && !f.motion.velocity_x &&
          e.input_hold == 32 && e.count == 2);
    f = Frame(2, 2);
    f.power_enabled = true;
    f.motion.contacts = 9;
    f.motion.velocity_x = -5 * 65536;
    f.motion.facing_left = true;
    f.motion.buttons = SOTN_BUTTON_LEFT;
    CHECK(SotnWolfController_Tick(&f, dra, &e) && !e.count && f.motion.mode == 3 &&
          f.turn_timer == 24);
    f = Frame(3, 0);
    f.motion.buttons = SOTN_BUTTON_DOWN;
    f.tapped = SOTN_BUTTON_JUMP;
    f.floor_effects[1] = SOTN_EFFECT_SOLID_FROM_ABOVE;
    CHECK(SotnWolfController_Tick(&f, dra, &e) && f.motion.substep == 5 && f.drop_ticks == 8 &&
          f.motion.velocity_y == 2 * 65536 && f.motion.animation.pose == 4 &&
          f.motion.animation.remaining == 1);
    // Falling tests landing before gravity; recoil tests it after gravity.
    f = Frame(5, 0);
    f.motion.velocity_y = 445000;
    CHECK(SotnWolfController_Tick(&f, dra, &e) && f.motion.substep == 1 && e.count == 1);
    f = Frame(6, 0);
    f.motion.velocity_y = 445000;
    CHECK(SotnWolfController_Tick(&f, dra, &e) && f.motion.substep == 3 && f.motion.mode == 3 &&
          e.count == 2 && e.requests[0].kind == SOTN_WOLF_SOUND &&
          e.requests[1].kind == SOTN_WOLF_FACTORY);
    return true;
}
static bool Swim(SotnDataView dra)
{
    SotnWolfControllerFrame f = Frame(5, 0);
    f.motion.contacts = 0;
    f.water_level = 13;
    f.skill_enabled = f.holy_symbol_enabled = true;
    f.motion.buttons = SOTN_BUTTON_CANCEL | SOTN_BUTTON_UP | SOTN_BUTTON_RIGHT | SOTN_BUTTON_LEFT;
    f.swim_lock = 1;
    SotnWolfControllerEvents e = {0};
    CHECK(SotnWolfController_Tick(&f, dra, &e) && f.motion.substep == 9 && f.motion.facing_left &&
          f.motion.velocity_x == -32768 && f.motion.velocity_y == -32768 &&
          f.motion.animation.id == 0xec && !e.turned && !f.swim_lock);
    f.tapped = SOTN_BUTTON_JUMP;
    CHECK(SotnWolfController_Tick(&f, dra, &e) && f.motion.substep == 4 && f.swim_lock == 16 &&
          f.motion.animation.id == 0xe6 && f.motion.velocity_y == -319488);
    f = Frame(5, 0);
    f.motion.contacts = 0;
    f.water_level = 13;
    f.skill_enabled = f.holy_symbol_enabled = true;
    f.motion.buttons = SOTN_BUTTON_CANCEL;
    f.swim_lock = 2;
    CHECK(SotnWolfController_Tick(&f, dra, &e) && f.motion.substep == 5 && f.swim_lock == 1);
    const SotnWolfControllerFrame before = f;
    f.motion.substep = 8;
    const SotnWolfControllerFrame entry = f;
    CHECK(!SotnWolfController_Tick(&f, dra, &e) && !SDL_memcmp(&entry, &f, sizeof(f)));
    f = before;
    f.motion.velocity_x = SDL_MIN_SINT32;
    CHECK(!SotnWolfController_Tick(&f, dra, &e));
    return true;
}
static bool OrderingAndWrap(SotnDataView dra)
{
    SotnWolfControllerEvents e = {0};
    SotnWolfControllerFrame f = Frame(1, 0);
    f.idle_timer = 0;
    f.run_window = f.swim_lock = SDL_MAX_UINT32;
    CHECK(SotnWolfController_Tick(&f, dra, &e) && f.idle_timer == SDL_MAX_UINT32 &&
          f.run_window == SDL_MAX_UINT32 - 1 && f.swim_lock == SDL_MAX_UINT32 - 1);
    // Stand can select mode one; the separate original Walk test then runs.
    f = Frame(4, 0);
    f.motion.contacts = 0x41;
    f.motion.buttons = SOTN_BUTTON_RIGHT;
    CHECK(SotnWolfController_Tick(&f, dra, &e) && f.motion.substep == 2 && f.motion.mode == 1 &&
          f.motion.animation.id == 0xe0 && f.run_window == 12);
    // The original airborne sprint checks the left wall for either velocity sign.
    f = Frame(4, 2);
    f.motion.contacts = 8;
    f.motion.buttons = SOTN_BUTTON_RIGHT;
    f.motion.velocity_x = 2 * 65536;
    CHECK(SotnWolfController_Tick(&f, dra, &e) && f.motion.mode == 1);
    f = Frame(4, 2);
    f.motion.contacts = 4;
    f.motion.buttons = SOTN_BUTTON_RIGHT;
    f.motion.velocity_x = 2 * 65536;
    CHECK(SotnWolfController_Tick(&f, dra, &e) && f.motion.mode == 2);
    // Ceiling request position wraps only its whole word, preserving the fraction.
    f = Frame(4, 0);
    f.motion.contacts = 2;
    f.motion.velocity_y = -6 * 65536;
    f.x = 32767 * 65536 + 13;
    CHECK(SotnWolfController_Tick(&f, dra, &e) && e.requests[0].source.x == -32766 * 65536 + 13 &&
          f.x == 32767 * 65536 + 13);
    f = Frame(1, 0);
    f.tapped = SOTN_BUTTON_LEFT_HAND;
    f.skill_enabled = f.quarter_circle_complete = true;
    const SotnWolfControllerFrame before = f;
    const SotnWolfControllerEvents before_events = e;
    const SotnDataView truncated = {dra.bytes, 0x8410 + 4 * 28 + 27};
    CHECK(!SotnWolfController_Tick(&f, truncated, &e) && !SDL_memcmp(&f, &before, sizeof(f)) &&
          !SDL_memcmp(&e, &before_events, sizeof(e)));
    return true;
}
int main(void)
{
    Uint8 *bytes = SDL_calloc(1, 0x8500);
    if (!bytes)
        return 1;
    bytes[0x8410 + 4 * 28 + 12] = 5;
    const SotnDataView dra = {bytes, 0x8500};
    const bool ok = Sprint(dra) && Charge(dra) && CollisionTransitions(dra) && Swim(dra) &&
                    OrderingAndWrap(dra);
    SDL_free(bytes);
    return ok ? 0 : 1;
}
