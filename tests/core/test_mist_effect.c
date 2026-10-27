#include <SDL3/SDL.h>
#include <sotn/mist_effect.h>

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
    // Authored linear/constant lookup fixtures exercise ownership and lifecycle, not SDK parity.
    Uint8 *dra = SDL_calloc(1, 0xe270), *exe = SDL_calloc(1, 0x24000);
    CHECK(dra && exe);
    for (Uint16 i = 0; i < 1025; ++i)
    {
        Put16(exe + 0x1dbcc + i * 2U, (Uint16)(i * 4U));
        Put16(exe + 0x22a10 + i * 2U, (Uint16)(i / 2U));
    }
    for (Uint16 i = 0; i < 192; ++i)
        Put16(exe + 0x1e3e0 + i * 2U, 4096);
    for (Uint16 i = 0; i < 32; ++i)
        Put16(dra + 0xe1b0 + i * 2U, 64);
    for (Uint16 i = 0; i < 8; ++i)
        dra[0xe230 + i * 4U] = dra[0xe250 + i * 4U] = (Uint8)i;
    SotnMistWedge capture[8] = {0};
    for (Uint16 i = 0; i < 8; ++i)
        capture[i] = (SotnMistWedge){
            .x = {80, 96, 88}, .y = {72, 72, 88}, .u = {4, 20, 12}, .v = {1, 1, 17}};
    SotnMistEffectInput in = {.dra = {dra, 0xe270},
                              .executable = {exe, 0x24000},
                              .capture = capture,
                              .x = 88,
                              .y = 88,
                              .priority = 128,
                              .random = {100, 200, 300, 400}};
    SotnMistEffect effect = {0};
    effect.x = 1;
    effect.y = 2;
    effect.points[0].x = 123;
    effect.points[0].y = 456;
    SotnMistEffectEvents events = {0};
    CHECK(SotnMistEffect_Tick(&effect, in, &events) && events.random_count == 4);
    CHECK(effect.phase == 1 && effect.timer == 0 && effect.opacity == 0);
    CHECK(((Uint32)effect.x & 65535U) == 1 && ((Uint32)effect.y & 65535U) == 2);
    CHECK(((Uint32)effect.points[0].x & 65535U) == 123 &&
          ((Uint32)effect.points[0].y & 65535U) == 456);
    CHECK(effect.quads[0].u[0] == 20 && effect.quads[0].u[1] == 12 && effect.quads[1].u[0] == 12 &&
          effect.quads[1].u[1] == 4);
    CHECK(effect.quads[0].page == 0x18 && effect.quads[0].palette == 0x10f &&
          effect.quads[0].priority == 130 && effect.quads[16].priority == 132);
    CHECK(effect.quads[0].grey[0] == 0 && effect.quads[0].grey[2] == 255);
    in.capture = NULL;
    for (Uint32 tick = 1; tick <= 79; ++tick)
    {
        in.game_timer = tick;
        CHECK(SotnMistEffect_Tick(&effect, in, &events));
        CHECK(events.cancel_palette == (tick == 79) && !events.restore_palette &&
              !events.random_count);
    }
    CHECK(effect.phase == 2 && effect.opacity == 255 && effect.quads[0].hidden);
    CHECK(effect.color_base[0] == 208 && effect.color_base[1] == 160 &&
          effect.color_base[2] == 112 && effect.color_base[3] == 48);
    effect.phase = 3;
    SotnMistEffect before = effect;
    events = (SotnMistEffectEvents){.random_count = 23};
    CHECK(!SotnMistEffect_Tick(&effect, in, &events));
    CHECK(!SDL_memcmp(&effect, &before, sizeof(effect)) && events.random_count == 23);
    in.capture = capture;
    CHECK(SotnMistEffect_Tick(&effect, in, &events) && effect.phase == 4 && effect.timer == 0 &&
          !events.random_count);
    in.capture = NULL;
    for (Uint32 tick = 1; tick <= 25; ++tick)
    {
        CHECK(SotnMistEffect_Tick(&effect, in, &events));
        CHECK(events.cancel_palette == (tick == 12) && events.restore_palette == (tick == 12));
    }
    CHECK(effect.phase == 5 && effect.timer == 16 && effect.opacity == 0 && !effect.removed);
    before = effect;
    CHECK(SotnMistEffect_Tick(&effect, in, &events) && effect.timer == 17 && !effect.removed);
    CHECK(!SDL_memcmp(effect.quads, before.quads, sizeof(effect.quads)));
    effect.timer = 32767;
    CHECK(SotnMistEffect_Tick(&effect, in, &events) && effect.timer == -32768 && effect.removed);
    CHECK(!SotnMistEffect_Tick(&effect, in, &events));
    effect = (SotnMistEffect){0};
    in.capture = capture;
    in.facing_left = true;
    CHECK(SotnMistEffect_Tick(&effect, in, &events) && effect.quads[0].u[0] == 4 &&
          effect.quads[1].u[1] == 20);
    effect = (SotnMistEffect){0};
    in.alternate_sheet = true;
    CHECK(SotnMistEffect_Tick(&effect, in, &events) && effect.quads[0].page == 0x118 &&
          effect.quads[0].u[0] == 20);
    effect = (SotnMistEffect){0};
    before = effect;
    dra[0xe250] = 8;
    CHECK(!SotnMistEffect_Tick(&effect, in, &events) &&
          !SDL_memcmp(&effect, &before, sizeof(effect)));
    dra[0xe250] = 0;
    Put16(dra + 0xe1b0, 257);
    CHECK(!SotnMistEffect_Tick(&effect, in, &events) &&
          !SDL_memcmp(&effect, &before, sizeof(effect)));
    Put16(dra + 0xe1b0, 64);
    Put16(exe + 0x1dbcc, 4097);
    CHECK(!SotnMistEffect_Tick(&effect, in, &events) &&
          !SDL_memcmp(&effect, &before, sizeof(effect)));
    CHECK(!SotnMistEffect_Tick(NULL, in, &events));
    CHECK(!SotnMistEffect_Tick(&effect, in, NULL));
    SDL_free(dra);
    SDL_free(exe);
    return 0;
}
