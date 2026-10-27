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
    const Uint16 ids[3] = {0x20, 0x21, 0x1b};
    for (Uint16 i = 0; i < 3; ++i)
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
    SDL_free(dra);
    SDL_free(exe);
    return 0;
}
