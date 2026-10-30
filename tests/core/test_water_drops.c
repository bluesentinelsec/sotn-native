#include <SDL3/SDL.h>
#include <sotn/water_drops.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put32(Uint8 *p, Sint32 value)
{
    const Uint32 b = (Uint32)value;
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(b >> (i * 8U));
}
static bool Small(void)
{
    Uint8 tables[188] = {0};
    Put32(tables + 156, -65536);
    Put32(tables + 160, -131072);
    SotnWaterDropsInput in = {
        .tables = {tables, sizeof(tables)}, .allocated_rows = 1, .priority = 148};
    SotnWaterDrops s = {.small = true, .x = 100 * 65536 + 123, .y = 140 * 65536 + 456};
    Uint32 rng = 17, expected = rng;
    expected = expected * 0x01010101U + 1U;
    const Sint32 y = 140 - (Sint32)((expected >> 24) & 3U);
    expected = expected * 0x01010101U + 1U;
    const Sint32 x = 100 - (Sint32)((expected >> 24) & 3U);
    CHECK(SotnWaterDrops_Tick(&s, in, &rng) && rng == expected && s.phase == 1 &&
          s.x == x * 65536 + 123 && s.y == y * 65536 + 456 && s.velocity_x == -65536 &&
          s.velocity_y == -131072 && s.tiles[0].x == x && s.tiles[0].blue == 120 &&
          s.tiles[0].green == 90 && s.tiles[0].draw_mode == 0x33);
    CHECK(SotnWaterDrops_Tick(&s, in, &rng) && rng == expected && s.x == (x - 1) * 65536 + 123 &&
          s.y == (y - 2) * 65536 + 456 && s.velocity_y == -114688);
    const SotnWaterDrops saved = s;
    CHECK(SotnWaterDrops_Translate(&s, -17, 9) && s.tiles[0].x == saved.tiles[0].x - 17);
    CHECK(SotnWaterDrops_Translate(&s, 17, -9) && !SDL_memcmp(&s, &saved, sizeof(s)));
    for (Uint16 i = 0; i < 30 && !s.removed; ++i)
        CHECK(SotnWaterDrops_Tick(&s, in, &rng));
    CHECK(s.removed && rng == expected);
    s = (SotnWaterDrops){.small = true, .parameter = 0x100};
    CHECK(SotnWaterDrops_Tick(&s, in, &rng) && s.velocity_x == 65536 + 4096);
    s = (SotnWaterDrops){.small = true, .parameter = 4};
    const SotnWaterDrops bad = s;
    expected = rng;
    CHECK(!SotnWaterDrops_Tick(&s, in, &rng) && rng == expected &&
          !SDL_memcmp(&s, &bad, sizeof(s)));
    return true;
}
static bool Large(void)
{
    SotnWaterDrops s = {.x = 100 * 65536, .y = 140 * 65536, .velocity_y = -131072};
    SotnWaterDropsInput in = {.allocated_rows = 33, .priority = 148};
    Uint32 rng = 17, expected = rng;
    for (Uint16 i = 0; i < 128; ++i)
        expected = expected * 0x41c64e6dU + 0x3039U;
    CHECK(SotnWaterDrops_Tick(&s, in, &rng) && rng == expected && s.phase == 1 && s.timer == 47);
    for (Uint16 i = 0; i < 32; ++i)
        CHECK(s.tiles[i].width == 2 && s.tiles[i].height == 2 && s.tiles[i].delay >= 32 &&
              s.tiles[i].delay <= 47 && s.tiles[i].blue == 255 && s.tiles[i].priority == 150 &&
              s.tiles[i].draw_mode == 0x73);
    CHECK(!s.tiles[32].width && !s.tiles[32].height && !s.tiles[32].x &&
          s.tiles[32].draw_mode == 0x73);
    const Sint32 packed = s.tiles[0].fixed_y;
    CHECK(SotnWaterDrops_Translate(&s, 2, -3) && s.tiles[0].fixed_y == packed &&
          s.tiles[32].x == 2);
    CHECK(SotnWaterDrops_Tick(&s, in, &rng) && s.timer == 46 && !s.tiles[32].x && !s.tiles[32].y &&
          rng == expected);
    for (Uint16 i = 0; i < 45; ++i)
        CHECK(SotnWaterDrops_Tick(&s, in, &rng));
    CHECK(!s.removed && s.timer == 1);
    CHECK(SotnWaterDrops_Tick(&s, in, &rng) && s.removed && rng == expected);
    s = (SotnWaterDrops){0};
    const SotnWaterDrops saved = s;
    in.allocated_rows = 32;
    CHECK(!SotnWaterDrops_Tick(&s, in, &rng) && rng == expected &&
          !SDL_memcmp(&s, &saved, sizeof(s)));
    in.allocated_rows = 0;
    CHECK(SotnWaterDrops_Tick(&s, in, &rng) && s.removed && rng == expected);
    return true;
}
int main(void)
{
    const bool ok = Small() && Large();
    if (ok)
        SDL_Log("WATER_DROPS: PASS authored small/large/sentinel/RNG/fraction/camera/capacity "
                "contexts");
    return ok ? 0 : 1;
}
