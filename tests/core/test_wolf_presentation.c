#include <SDL3/SDL.h>
#include <sotn/wolf_presentation.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8));
}
static SotnWolfDrawInput Input(void)
{
    return (SotnWolfDrawInput){.part = {.x = 100 * 65536 + 123,
                                        .y = 120 * 65536 + 456,
                                        .bank = 15,
                                        .sprite = 1,
                                        .texture_offset = 126,
                                        .palette = 0x104,
                                        .priority = 144},
                               .slot = 18,
                               .player_x = 100,
                               .player_y = 120,
                               .player_priority = 144,
                               .available = 768};
}
static bool Geometry(SotnDataView dra, SotnDataView exe)
{
    SotnWolfDrawPacket packets[4] = {0};
    size_t count = 99;
    SotnWolfDrawInput in = Input();
    CHECK(SotnWolfPresentation_Packets(dra, exe, in, packets, 4, &count) && count == 4);
    SotnPlayerEffectQuad q = packets[0].quad;
    CHECK(!packets[0].capture && packets[1].capture && !packets[2].capture && packets[3].capture);
    CHECK(q.x[0] == 98 && q.x[1] == 102 && q.y[0] == 117 && q.y[2] == 121 && q.u[0] == 0 &&
          q.u[1] == 4 && q.v[0] == 128 && q.v[2] == 132 && q.palette == 0x105 &&
          q.priority == 144 && q.page == 31 && !q.draw_mode && q.red[0] == 128);
    q = packets[1].quad;
    CHECK(q.x[0] == 42 && q.y[0] == 21 && q.palette == 0x104 && q.priority == 416);
    CHECK(packets[2].quad.priority == 148 && packets[3].quad.priority == 420);
    in.part.facing_left = true;
    CHECK(SotnWolfPresentation_Packets(dra, exe, in, packets, 4, &count) &&
          packets[0].quad.x[0] == 99 && packets[0].quad.u[0] == 3 && packets[0].quad.u[1] == 255 &&
          packets[1].quad.x[0] == 19);
    in = Input();
    in.part.draw_flags = 8 | 16;
    in.part.opacity = 73;
    in.part.blend = 0x70;
    in.part.palette = 0x810d;
    CHECK(SotnWolfPresentation_Packets(dra, exe, in, packets, 4, &count));
    CHECK(packets[0].quad.red[0] == 128 && packets[0].quad.green[0] == 73 &&
          packets[0].quad.blue[0] == 73 && packets[0].quad.draw_mode == 5 &&
          packets[0].quad.page == 127 && packets[0].quad.palette == 0x10d &&
          packets[1].quad.palette == 0x104 && packets[1].quad.page == 127);
    in = Input();
    in.part.draw_flags = 4;
    in.part.pivot_x = 1;
    in.part.pivot_y = 9;
    in.part.rotation = 384;
    CHECK(SotnWolfPresentation_Packets(dra, exe, in, packets, 4, &count) &&
          packets[0].quad.x[0] == 101 && packets[0].quad.x[3] == 101 &&
          packets[0].quad.y[0] == 129 && packets[1].quad.x[0] == 45 && packets[1].quad.y[0] == 33);
    in.part.facing_left = true;
    CHECK(SotnWolfPresentation_Packets(dra, exe, in, packets, 4, &count) &&
          packets[0].quad.x[0] == 99);
    return true;
}
static bool CapacityAndFailures(SotnDataView dra, SotnDataView exe)
{
    SotnWolfDrawPacket packets[4] = {0};
    size_t count = 99;
    SotnWolfDrawInput in = Input();
    for (Uint16 rows = 0; rows < 5; ++rows)
    {
        in.available = rows;
        CHECK(SotnWolfPresentation_Packets(dra, exe, in, packets, 4, &count) && count == rows);
    }
    CHECK(!packets[0].capture && packets[1].capture && !packets[2].capture);
    in = Input();
    const SotnWolfDrawPacket before[4] = {packets[0], packets[1], packets[2], packets[3]};
    count = 99;
    CHECK(!SotnWolfPresentation_Packets(dra, exe, in, packets, 3, &count) && count == 99 &&
          !SDL_memcmp(packets, before, sizeof(packets)));
    in.part.draw_flags = 128;
    in.game_timer = 1;
    CHECK(SotnWolfPresentation_Packets(dra, exe, in, NULL, 0, &count) && !count);
    in = Input();
    in.part.draw_flags = 4;
    count = 99;
    CHECK(!SotnWolfPresentation_Packets(dra, (SotnDataView){0}, in, packets, 4, &count) &&
          count == 99 && !SDL_memcmp(packets, before, sizeof(packets)));
    in = Input();
    CHECK(!SotnWolfPresentation_Packets((SotnDataView){dra.bytes, 0x3baf}, exe, in, packets, 4,
                                        &count));
    Uint8 *bad = SDL_malloc(dra.size);
    CHECK(bad != NULL);
    SDL_memcpy(bad, dra.bytes, dra.size);
    Put16(bad + 0x3c24 + 6, 0);
    const bool rejected =
        !SotnWolfPresentation_Packets((SotnDataView){bad, dra.size}, exe, in, packets, 4, &count);
    SDL_free(bad);
    CHECK(rejected && count == 99 && !SDL_memcmp(packets, before, sizeof(packets)));
    return true;
}
int main(void)
{
    Uint8 *data = SDL_calloc(1, 0x5000), *sdk = SDL_calloc(1, 0x23220);
    if (!data || !sdk)
    {
        SDL_free(data);
        SDL_free(sdk);
        return 1;
    }
    Put32(data + 0x3bac, 0x800a3c00U);
    Put32(data + 0x3c04, 0x800a3c0cU);
    Put32(data + 0x3c08, 0x800a3c3aU);
    Put16(data + 0x3c0c, 2);
    Put16(data + 0x3c3a, 1);
    const Sint16 parts[2][11] = {{4, -2, -3, 5, 4, 1, 0, 0, 0, 5, 4},
                                 {0x110, 1, -2, 4, 5, 2, 1, 6, 1, 10, 6}};
    for (Uint16 j = 0; j < 2; ++j)
        for (Uint16 i = 0; i < 11; ++i)
            Put16(data + 0x3c0e + j * 22U + i * 2U, (Uint16)parts[j][i]);
    for (Uint32 i = 0; i < 192; ++i)
        Put16(sdk + 0x1e3e0 + i * 2U, 4096);
    const SotnDataView dra = {data, 0x5000}, exe = {sdk, 0x23220};
    const bool ok = Geometry(dra, exe) && CapacityAndFailures(dra, exe);
    SDL_free(data);
    SDL_free(sdk);
    return ok ? 0 : 1;
}
