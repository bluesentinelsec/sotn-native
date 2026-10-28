#include <SDL3/SDL.h>
#include <sotn/bat_echo.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
int main(void)
{
    Uint8 *dra = SDL_calloc(1, 0x10900), *exe = SDL_calloc(1, 0x20000);
    CHECK(dra && exe);
    for (Uint16 i = 0; i < 17; ++i)
    {
        Put16(dra + 0x10884 + i * 2, (Uint16)(i % 6));
        Put16(dra + 0x108a8 + i * 2, (Uint16)(i * 8));
    }
    for (Uint16 i = 0; i <= 1024; ++i)
        Put16(exe + 0x1dbcc + i * 2, (Uint16)(i * 4));
    SotnBatEchoInput in = {.dra = {dra, 0x10900},
                           .executable = {exe, 0x20000},
                           .x = 100,
                           .y = 120,
                           .priority = 144,
                           .primitives_allocated = true};
    SotnBatEchoEvents out = {0};
    for (Uint16 force = 0; force < 2; ++force)
    {
        SotnBatEcho e = {.facing_left = force != 0};
        in.force_enabled = force != 0;
        CHECK(SotnBatEcho_Tick(&e, in, &out));
        CHECK(e.phase == 1 && !e.intensity && e.limit == (force ? 256 : 160) && e.priority == 142 &&
              out.sound == 0x61f && out.initialize_attack == (force != 0) && !out.disable_attack);
        CHECK(e.quads[0].x[0] == (force ? 84 : 116) && e.quads[0].x[1] == (force ? 76 : 124));
        const Uint16 priority = e.priority;
        Uint16 frames = 1, disabled = 0, sounds = 1;
        // Toggling Force after birth cannot change the retained lifetime or create an attack.
        in.force_enabled = !force;
        in.priority = 200;
        while (!e.removed && frames < 100)
        {
            CHECK(SotnBatEcho_Tick(&e, in, &out));
            ++frames;
            disabled += out.disable_attack;
            sounds += (out.sound != 0);
            CHECK(!out.initialize_attack && e.priority == priority);
        }
        CHECK(e.removed && frames == (force ? 53 : 35) && disabled == 1 && sounds == 1);
        CHECK(!SotnBatEcho_Tick(&e, in, &out));
        in.priority = 144;
    }
    SotnBatEcho e = {0};
    in.primitives_allocated = false;
    CHECK(SotnBatEcho_Tick(&e, in, &out) && e.removed && out.sound == 0x61f &&
          !out.initialize_attack);
    e = (SotnBatEcho){0};
    in.primitives_allocated = true;
    in.force_enabled = true;
    CHECK(SotnBatEcho_Tick(&e, in, &out));
    e.retire_ticks = 4;
    const Sint16 angle = e.drivers[0].angle;
    for (Uint16 i = 0; i < 4; ++i)
    {
        CHECK(SotnBatEcho_Tick(&e, in, &out));
        CHECK(e.hidden && e.drivers[0].angle == angle);
    }
    CHECK(e.removed);
    e = (SotnBatEcho){0};
    in.priority = 1;
    out = (SotnBatEchoEvents){.sound = 123};
    CHECK(!SotnBatEcho_Tick(&e, in, &out) && !e.phase && out.sound == 123);
    in.priority = 144;
    in.dra.size = 100;
    CHECK(!SotnBatEcho_Tick(&e, in, &out) && !e.age && out.sound == 123);
    in.dra.size = 0x10900;
    Put16(dra + 0x10884, 6);
    CHECK(!SotnBatEcho_Tick(&e, in, &out) && !e.age && out.sound == 123);
    SDL_free(dra);
    SDL_free(exe);
    return 0;
}
