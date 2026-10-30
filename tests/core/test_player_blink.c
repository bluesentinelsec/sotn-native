#include <SDL3/SDL.h>
#include <sotn/player_blink.h>

#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)

static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
int main(void)
{
    Uint8 *dra = SDL_calloc(1, 0xe000), *exe = SDL_calloc(1, 0x24000);
    CHECK(dra && exe);
    for (Uint16 i = 0; i < 1025; ++i)
        Put16(exe + 0x1dbcc + i * 2U, (Uint16)(i * 4U));
    const Uint16 ids[] = {0x20, 0x21, 0x1b, 5, 0x1c, 4, 8};
    for (Uint16 i = 0; i < SDL_arraysize(ids); ++i)
    {
        Uint8 *p = dra + 0xd670 + ids[i] * 20U;
        Put16(p + 6, 0x123);
        Put16(p + 8, 256);
        Put16(p + 10, 256);
        Put16(p + 12, 256);
        Put16(p + 14, 5);
        Put16(p + 16, 0x31);
        Put16(p + 18, 512);
    }
    SotnPlayerBlinkShared shared = {0};
    SotnPlayerBlink effect = {.parameter = 0x20};
    SotnPlayerBlinkInput input = {.dra = {dra, 0xe000},
                                  .executable = {exe, 0x24000},
                                  .image = {20, 32, -10, -20},
                                  .x = 100,
                                  .y = 120,
                                  .priority = 144,
                                  .image_present = true};
    SotnPlayerBlinkEvents events = {0};
    input.allocation_failed = true;
    SotnPlayerBlink blocked = {.parameter = 5};
    SotnPlayerBlinkShared locked = {.bat_lock = true};
    CHECK(SotnPlayerBlink_Tick(&blocked, &locked, input, &events));
    CHECK(blocked.removed && locked.bat_lock && !events.signal && !events.reset_afterimages);
    blocked = (SotnPlayerBlink){.parameter = 0x20};
    input.status = 2;
    CHECK(SotnPlayerBlink_Tick(&blocked, &locked, input, &events));
    CHECK(blocked.removed && !locked.bat_lock && !events.signal);
    input.status = 0;
    for (Uint16 i = 0; i < 25; ++i)
    {
        CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events));
        CHECK(!effect.phase && !effect.removed && !events.signal && !events.reset_afterimages &&
              !shared.bat_lock && !effect.quads[0].priority);
    }
    input.allocation_failed = false;
    CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events));
    CHECK(effect.phase == 8 && !events.signal && events.reset_afterimages);
    CHECK(effect.quads[0].x[0] == 90 && effect.quads[0].x[1] == 110 &&
          effect.quads[0].y[0] == 100 && effect.quads[0].y[2] == 104 &&
          effect.quads[0].palette == 0x123 && effect.quads[0].priority == 148 &&
          effect.quads[0].draw_mode == 0x335 && effect.quads[0].red[0] == 128);
    for (Uint16 i = 0; i < 16; ++i)
    {
        CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events));
        CHECK(!events.signal && shared.bat_lock);
    }
    CHECK(effect.amplitude == 128 && effect.phase == 8);
    CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events));
    CHECK(events.signal == 2 && effect.phase == 9);
    for (Uint16 i = 0; i < 16; ++i)
        CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && !events.signal);
    CHECK(effect.amplitude == 0);
    CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events));
    CHECK(events.signal == 3 && effect.parameter == 0x1b && effect.phase == 1 && !shared.bat_lock &&
          !events.reset_afterimages && effect.quads[0].palette == 0x15f);
    Uint16 tail_ticks = 0;
    while (!effect.removed && tail_ticks < 100)
    {
        CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events));
        CHECK(!events.signal);
        ++tail_ticks;
    }
    CHECK(effect.removed && tail_ticks == 57);
    Put16(dra + 0xd670 + 0x1b * 20U + 14, 0x7008);
    effect = (SotnPlayerBlink){.parameter = 0x1b, .phase = 1};
    input.status = 0x400000;
    for (Uint16 i = 0; i < 4; ++i)
        CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events));
    CHECK(effect.phase == 2 && effect.brightness == 256);
    for (Uint16 i = 0; i < 20; ++i)
        CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && effect.phase == 2);
    input.status = 0;
    CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && effect.phase == 3);
    effect = (SotnPlayerBlink){.parameter = 0x21};
    CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && effect.phase == 10);
    for (Uint16 i = 0; i < 30; ++i)
        CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && !events.signal);
    CHECK(effect.phase == 10 && effect.amplitude == 128);
    input.signal = 1;
    CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && effect.phase == 11);
    for (Uint16 i = 0; i < 17; ++i)
        CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events));
    CHECK(events.signal == 3 && effect.parameter == 0x1b && !shared.bat_lock);
    shared.bat_lock = true;
    CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && effect.removed &&
          shared.bat_lock);
    effect = (SotnPlayerBlink){.parameter = 0x20};
    const SotnPlayerBlink before = effect;
    const SotnPlayerBlinkShared phases = shared;
    Put16(dra + 0xd670 + 0x20 * 20U + 8, 0);
    CHECK(!SotnPlayerBlink_Tick(&effect, &shared, input, &events));
    CHECK(!SDL_memcmp(&effect, &before, sizeof(effect)) &&
          !SDL_memcmp(&shared, &phases, sizeof(phases)));
    input.image_present = false;
    CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && effect.removed &&
          !shared.bat_lock);
    effect = (SotnPlayerBlink){.parameter = 0x20};
    input.image_present = true;
    input.status = 2;
    shared.bat_lock = true;
    CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && effect.removed &&
          !shared.bat_lock);
    effect = (SotnPlayerBlink){.parameter = 0x21};
    input.status = 0x1000000;
    CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && effect.removed);
    shared = (SotnPlayerBlinkShared){0};
    input.status = 0;
    input.signal = 0;
    input.player_step = 5;
    input.player_substep = 3;
    effect = (SotnPlayerBlink){.parameter = 0x5c};
    Put16(dra + 0xd670 + 0x1c * 20U + 14, 0x7009);
    CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events));
    CHECK(effect.phase == 1 && effect.quads[0].priority == 146 && effect.quads[0].x[0] == 90 &&
          effect.quads[0].x[1] == 100 && effect.quads[2].x[0] == 110 &&
          effect.quads[2].y[1] == 116 && effect.quads[0].u[2] == 14 && effect.quads[0].v[2] == 17);
    CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events));
    CHECK(effect.quads[0].red[0] > 0 && effect.quads[0].red[2] == 0 &&
          effect.quads[0].green[3] == 0 && !events.signal);
    effect.phase = 2;
    effect.hold = 8;
    effect.brightness = 256;
    input.status = 0x40000000U;
    CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && effect.phase == 2);
    input.player_substep = 1;
    input.status = 0;
    CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && effect.phase == 4);
    for (Uint16 i = 0; i < 20; ++i)
        CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && effect.phase == 4 &&
              effect.brightness == 256 && !effect.removed);
    shared.bat_lock = true;
    CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && effect.removed &&
          shared.bat_lock);
    shared.bat_lock = false;
    effect = (SotnPlayerBlink){.parameter = 5};
    for (Uint16 i = 0; i < 100 && !effect.removed; ++i)
        CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && !events.signal);
    CHECK(effect.removed);
    shared = (SotnPlayerBlinkShared){0};
    for (Uint16 kind = 0; kind < 2; ++kind)
    {
        effect = (SotnPlayerBlink){.parameter = kind ? 0x48 : 0x44};
        input.allocation_failed = true;
        CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && !effect.phase &&
              !effect.removed && !events.reset_afterimages);
        input.allocation_failed = false;
        Uint16 ticks = 0;
        while (!effect.removed && ticks < 100)
        {
            CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && !events.signal &&
                  !shared.bat_lock && !shared.wolf_lock);
            if (!effect.removed)
                CHECK(effect.quads[0].priority == 146 && effect.quads[0].palette == 0x123);
            ++ticks;
        }
        CHECK(effect.removed && ticks > 50 && ticks < 64);
    }
    for (Uint16 parameter = 0x16; parameter <= 0x17; ++parameter)
    {
        Uint8 *p = dra + 0xd670 + parameter * 20U;
        Put16(p + 6, 0x123);
        Put16(p + 8, 256);
        Put16(p + 10, 256);
        Put16(p + 12, 256);
        Put16(p + 14, parameter == 0x16 ? 0x7002 : 0x7000);
        Put16(p + 16, 0x31);
        Put16(p + 18, 512);
        effect = (SotnPlayerBlink){.parameter = parameter};
        SDL_zero(shared);
        input.status = 0;
        input.allocation_failed = false;
        input.poison_ticks = input.curse_ticks = 4095;
        CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events));
        for (Uint16 t = 0; t < 26; ++t)
            CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events));
        CHECK(effect.phase == 2 && effect.brightness == 256);
        CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && effect.phase == 2 &&
              effect.hold == 7 && effect.brightness == (parameter == 0x16 ? 208 : 256));
        input.poison_ticks = 2048;
        CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) &&
              effect.brightness == (parameter == 0x16 ? 112 : 256));
        if (parameter == 0x16)
            input.poison_ticks = 0;
        else
            input.curse_ticks = 0;
        CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && effect.phase == 3 &&
              effect.hold == 7);
        for (Uint16 t = 0; t < 32 && !effect.removed; ++t)
            CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events));
        CHECK(effect.removed);
    }
    for (Uint16 kind = 0; kind < 2; ++kind)
    {
        const Uint16 parameter = kind ? 0x59 : 0x4e;
        Uint8 *p = dra + 0xd670 + (parameter & 0x3fU) * 20U;
        Put16(p + 6, 0x123);
        Put16(p + 8, 256);
        Put16(p + 10, 256);
        Put16(p + 12, 256);
        Put16(p + 14, kind ? 0x7005 : 0x7004);
        Put16(p + 16, 0x31);
        Put16(p + 18, 512);
        effect = (SotnPlayerBlink){.parameter = parameter};
        SDL_zero(shared);
        input.water_waist = 1;
        input.holy_symbol = false;
        input.player_animation = 0xc0;
        input.allocation_failed = true;
        CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && !effect.phase &&
              !effect.removed && !events.reset_afterimages);
        input.allocation_failed = false;
        for (Uint16 t = 0; t < 27; ++t)
            CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events));
        CHECK(effect.phase == 2 && effect.brightness == 256);
        for (Uint16 t = 0; t < 100; ++t)
            CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && effect.phase == 2 &&
                  effect.hold == 7 && !events.signal);
        if (kind)
            input.player_animation = 0xc1;
        else
            input.holy_symbol = true;
        CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && effect.phase == 3 &&
              effect.hold == 7);
        for (Uint16 t = 0; t < 30 && !effect.removed; ++t)
            CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events));
        CHECK(effect.removed);
        if (!kind)
        {
            input.holy_symbol = false;
            input.water_waist = 0;
            effect = (SotnPlayerBlink){.parameter = parameter, .phase = 2, .brightness = 256};
            CHECK(SotnPlayerBlink_Tick(&effect, &shared, input, &events) && effect.phase == 3);
        }
    }
    effect = (SotnPlayerBlink){.parameter = 0x4e};
    input.water_waist = 32768;
    const SotnPlayerBlink invalid_depth = effect;
    const SotnPlayerBlinkShared invalid_shared = shared;
    const SotnPlayerBlinkEvents invalid_events = events;
    CHECK(!SotnPlayerBlink_Tick(&effect, &shared, input, &events) &&
          !SDL_memcmp(&effect, &invalid_depth, sizeof(effect)) &&
          !SDL_memcmp(&shared, &invalid_shared, sizeof(shared)) &&
          !SDL_memcmp(&events, &invalid_events, sizeof(events)));
    SDL_free(dra);
    SDL_free(exe);
    return 0;
}
