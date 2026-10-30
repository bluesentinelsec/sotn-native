#include <SDL3/SDL.h>
#include <sotn/water_effect.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Sint16 v)
{
    Uint16 b = (Uint16)v;
    p[0] = (Uint8)b;
    p[1] = (Uint8)(b >> 8);
}
static void Put32(Uint8 *p, Sint32 v)
{
    Uint32 b = (Uint32)v;
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(b >> (i * 8U));
}
static void Tables(Uint8 *p)
{
    Put16(p + 2, 3);
    Put16(p + 8, -3);
    for (Uint16 i = 0; i < 4; ++i)
    {
        Put32(p + 32 + i * 8U, -131072);
        Put32(p + 36 + i * 8U, 32768);
    }
    for (Uint16 i = 0; i < 6; ++i)
        Put16(p + 64 + i * 2U, (Sint16)(i * 2U));
    Put32(p + 76, 2);
    Put32(p + 80, -3);
}
static bool Splash(SotnWaterEffectInput in)
{
    Uint32 rng = 17;
    SotnWaterEffect s = {.x = 128 * 65536 + 123, .y = 140 * 65536 + 456};
    SotnWaterEffectEvents e = {0};
    in.allocated_rows = 2;
    CHECK(SotnWaterEffect_Tick(&s, in, &rng, &e) && s.phase == 1 && s.count == 2 &&
          s.y == 140 * 65536 + 456 && s.velocity_y == -131072 && e.sound && e.positional &&
          e.sound_index == 0 && e.volume == 127 && e.pan == 0 && e.drop &&
          e.drop_x == 128 * 65536 && e.drop_y == 140 * 65536 && e.drop_velocity_y == -131072 &&
          rng == 17);
    CHECK(s.quads[0].y[0] == 140 && s.quads[0].y[2] == 140 && s.quads[0].blue[3] == 124 &&
          s.quads[0].red[0] == 255 && s.quads[1].palette == 0x15f && s.quads[1].blue[3] == 92 &&
          s.quads[1].red[0] == 0);
    CHECK(SotnWaterEffect_Tick(&s, in, &rng, &e) && s.y == 138 * 65536 + 456 &&
          s.velocity_y == -98304 && !e.sound && !e.drop && rng == 17 && s.quads[0].y[0] == 138 &&
          s.quads[0].y[2] == 140);
    const SotnWaterEffect saved = s;
    CHECK(SotnWaterEffect_Translate(&s, -32, -16) && s.x == saved.x - 32 * 65536 &&
          s.world_y == saved.world_y && s.quads[0].y[2] == saved.quads[0].y[2] - 16);
    CHECK(SotnWaterEffect_Translate(&s, 32, 16) && !SDL_memcmp(&s, &saved, sizeof(s)));
    CHECK(!SotnWaterEffect_Translate(&s, 32768, 0) && !SDL_memcmp(&s, &saved, sizeof(s)));
    for (Uint16 i = 0; i < 100 && !s.removed; ++i)
        CHECK(SotnWaterEffect_Tick(&s, in, &rng, &e));
    CHECK(s.removed);
    s = (SotnWaterEffect){.x = 128 * 65536, .y = 140 * 65536, .parameter = 8 * 2048};
    in.allocated_rows = 4;
    CHECK(SotnWaterEffect_Tick(&s, in, &rng, &e) && s.count == 4 && s.quads[0].x[0] == 122 &&
          s.quads[0].u[0] == 9 && s.quads[2].x[1] == 122 && s.quads[2].x[0] == 114);
    Uint16 rows = 99;
    CHECK(SotnWaterEffect_Rows(SOTN_WATER_SPLASH, (Uint16)(8 * 2048 + 7 * 32), &rows) && rows == 2);
    s = (SotnWaterEffect){.parameter = 7 * 256};
    in.allocated_rows = 2;
    const SotnWaterEffect bad = s;
    e.drop = true;
    CHECK(!SotnWaterEffect_Tick(&s, in, &rng, &e) && !SDL_memcmp(&s, &bad, sizeof(s)) && e.drop &&
          rng == 17);
    s = (SotnWaterEffect){0};
    CHECK(SotnWaterEffect_Tick(&s, (SotnWaterEffectInput){0}, &rng, &e) && s.removed && !e.drop &&
          !e.sound && rng == 17);
    return true;
}
static bool Surface(SotnWaterEffectInput in)
{
    Uint32 rng = 17, expected = 17;
    expected = expected * 0x41c64e6dU + 0x3039U;
    SotnWaterEffect s = {.kind = SOTN_WATER_SURFACING,
                         .x = 128 * 65536 + 123,
                         .y = 140 * 65536 + 456,
                         .parameter = 2};
    SotnWaterEffectEvents e = {0};
    in.allocated_rows = 2;
    CHECK(SotnWaterEffect_Tick(&s, in, &rng, &e) && s.phase == 1 && e.sound && e.sound_index == 1 &&
          !e.drop && rng == expected && s.quads[0].red[0] == 247 && s.quads[0].blue[3] == 124 &&
          s.quads[1].palette == 0x161);
    Sint32 top = s.top;
    Uint32 before = rng;
    CHECK(SotnWaterEffect_Tick(&s, in, &rng, &e) && s.top == top - 16384 && rng == before &&
          !e.sound);
    for (Uint16 i = 0; i < 40 && !s.removed; ++i)
        CHECK(SotnWaterEffect_Tick(&s, in, &rng, &e));
    CHECK(s.removed);
    s = (SotnWaterEffect){
        .kind = SOTN_WATER_SURFACING, .x = 128 * 65536, .y = 140 * 65536, .parameter = 0x8000};
    CHECK(SotnWaterEffect_Tick(&s, in, &rng, &e) && !e.sound && rng != before);
    Uint8 volumes[16] = {0};
    Put16(volumes + 2, 200);
    in.volumes = (SotnDataView){volumes, sizeof(volumes)};
    s = (SotnWaterEffect){
        .kind = SOTN_WATER_SURFACING, .x = 128 * 65536, .y = 140 * 65536, .current = 8192};
    CHECK(SotnWaterEffect_Tick(&s, in, &rng, &e) && s.x == 130 * 65536 && s.velocity_x == 131072 &&
          !s.removed);
    s = (SotnWaterEffect){
        .kind = SOTN_WATER_SURFACING, .x = 193 * 65536, .y = 140 * 65536, .current = 8192};
    before = rng;
    CHECK(SotnWaterEffect_Tick(&s, in, &rng, &e) && s.removed && e.sound && rng != before &&
          s.quads[0].red[0] == 255);
    return true;
}
static bool Side(SotnWaterEffectInput in)
{
    Uint32 rng = 17;
    SotnWaterEffect s = {.kind = SOTN_WATER_SIDE_SPLASH,
                         .x = 128 * 65536 + 123,
                         .y = 140 * 65536 + 456,
                         .parameter = 0x100};
    SotnWaterEffectEvents e = {0};
    in.allocated_rows = 1;
    in.priority = 509;
    CHECK(SotnWaterEffect_Tick(&s, in, &rng, &e) && s.count == 1 && s.age == 1 &&
          s.x == 128 * 65536 + 123 && s.velocity_x == 9216 && s.velocity_y == -12288 && e.sound &&
          !e.positional && rng == 17 && s.quads[0].priority == 511 && s.quads[0].x[0] == 124 &&
          s.quads[0].y[0] == 136 && s.quads[0].red[0] == 125 && s.quads[0].blue[3] == 124);
    CHECK(SotnWaterEffect_Tick(&s, in, &rng, &e) && s.x == 128 * 65536 + 123 + 9216 &&
          s.velocity_y == -1024 && !e.sound);
    for (Uint16 i = 0; i < 50 && !s.removed; ++i)
        CHECK(SotnWaterEffect_Tick(&s, in, &rng, &e));
    CHECK(s.removed);
    Uint16 rows = 99;
    CHECK(!SotnWaterEffect_Rows(SOTN_WATER_SIDE_SPLASH, 8, &rows) && rows == 99);
    return true;
}
int main(void)
{
    Uint8 tables[188] = {0};
    Tables(tables);
    Uint8 *exe = SDL_calloc(1, 0x24000);
    if (!exe)
        return 1;
    for (Uint16 i = 0; i < 1025; ++i)
        Put16(exe + 0x1dbcc + i * 2U, (Sint16)(i * 4U));
    SotnWaterEffectInput in = {.tables = {tables, sizeof(tables)},
                               .executable = {exe, 0x24000},
                               .priority = 148,
                               .scroll_y = 16};
    bool ok = Splash(in) && Surface(in) && Side(in);
    SDL_free(exe);
    if (ok)
        SDL_Log("WATER_EFFECT: PASS authored packet/birth/motion/fade/RNG/exhaustion/current/sound "
                "contexts");
    return ok ? 0 : 1;
}
