#include <SDL3/SDL.h>
#include <sotn/stage_route.h>
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
static void Put32(Uint8 *p, Uint32 value)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(value >> (i * 8U));
}
static int Boss(void)
{
    Uint8 bytes[0x297c + 64 * 20] = {0};
    const SotnDataView dra = {bytes, sizeof(bytes)};
    Uint8 *boss = bytes + 0x297c, *arrival = bytes + 0x245c + 130 * 10;
    Put16(arrival, 64);
    Put16(arrival + 2, 132);
    Put16(arrival + 6, 1);
    Put16(arrival + 8, 26);
    Put32(boss, 2);
    Put32(boss + 4, 3);
    Put32(boss + 8, 1);
    Put32(boss + 12, 2);
    Put32(boss + 16, 130);
    SDL_memcpy(boss + 20, boss, 20);
    Put32(boss + 20, 4);
    Put32(boss + 32, 255);
    Put32(boss + 40, 128);
    SotnWorldState world = {0};
    SotnStageRoute route = {.room = 999};
    bool found = false;
    CHECK(SotnStageRoute_FindBoss(dra, 1, 2, 3, &world, &found, &route) && found &&
          route.source == 1 && route.stage == 26 && route.room == 0 && route.x == 64);
    CHECK(SotnWorld_VisitEvent(&world, 2));
    CHECK(SotnStageRoute_FindBoss(dra, 1, 2, 3, &world, &found, &route) && found);
    CHECK(SotnWorld_RecordTime(&world, 2, 1));
    route.room = 999;
    CHECK(SotnStageRoute_FindBoss(dra, 1, 2, 3, &world, &found, &route) && !found &&
          route.room == 999);
    CHECK(SotnStageRoute_FindBoss(dra, 1, 4, 3, &world, &found, &route) && found);
    CHECK(SotnStageRoute_FindBoss(dra, 1, -1, 64, &world, &found, &route) && !found);
    found = true;
    route.room = 999;
    CHECK(!SotnStageRoute_FindBoss((SotnDataView){bytes, 0x297c + 40}, 1, 0, 0, &world, &found,
                                   &route) &&
          found && route.room == 999);
    Put32(boss + 4, 64);
    CHECK(!SotnStageRoute_FindBoss(dra, 1, 0, 0, &world, &found, &route) && found &&
          route.room == 999);
    Put32(boss + 4, 3);
    Put32(boss + 12, 32);
    CHECK(!SotnStageRoute_FindBoss(dra, 1, 0, 0, &world, &found, &route));
    Put32(boss + 12, 255);
    Put16(arrival + 6, 7);
    CHECK(!SotnStageRoute_FindBoss(dra, 1, 2, 3, &world, &found, &route));
    CHECK(!SotnStageRoute_FindBoss(dra, 1, 2, 3, NULL, &found, &route));
    CHECK(!SotnStageRoute_FindBoss(dra, 1, 2, 3, &world, NULL, &route));
    CHECK(!SotnStageRoute_FindBoss(dra, 1, 2, 3, &world, &found, NULL));
    CHECK(!SotnStageRoute_FindBoss(dra, 1, 65, 3, &world, &found, &route));
    SDL_memset(boss, 0, 64 * 20);
    CHECK(!SotnStageRoute_FindBoss(dra, 1, 2, 3, &world, &found, &route));
    return 0;
}
static int ChapelBoss(void)
{
    Uint8 bytes[0x297c + 3 * 20] = {0};
    const SotnDataView dra = {bytes, sizeof(bytes)};
    for (Uint32 side = 0; side < 2; ++side)
    {
        // Synthetic geometry/indices exercise both directions without copying game records.
        Uint8 *record = bytes + 0x297c + side * 20;
        Put32(record, 2 + side);
        Put32(record + 4, 3);
        Put32(record + 8, 6);
        Put32(record + 12, SOTN_TIME_HIPPOGRYPH);
        Put32(record + 16, 129 + side);
        Uint8 *arrival = bytes + 0x245c + (129 + side) * 10;
        Put16(arrival, (Uint16)(32 + side * 64));
        Put16(arrival + 2, 100);
        Put16(arrival + 6, 6);
        Put16(arrival + 8, 25);
    }
    Put32(bytes + 0x297c + 40, 128);
    SotnWorldState world = {0};
    for (Uint32 side = 0; side < 2; ++side)
    {
        SotnStageRoute route = {0};
        bool found = false;
        CHECK(SotnStageRoute_FindBoss(dra, 6, (Sint32)(2 + side), 3, &world, &found, &route) &&
              found && route.source == 6 && route.stage == 25 && route.room == 0 &&
              route.x == 32 + side * 64);
    }
    // Visited/completion settings are distinct from the defeat record used by routing.
    CHECK(SotnWorld_VisitEvent(&world, SOTN_TIME_HIPPOGRYPH));
    world.cutscenes_seen = 0x300;
    SotnStageRoute route = {.room = 99};
    bool found = false;
    CHECK(SotnStageRoute_FindBoss(dra, 6, 2, 3, &world, &found, &route) && found);
    CHECK(SotnWorld_RecordTime(&world, SOTN_TIME_HIPPOGRYPH, 123));
    route.room = 99;
    for (Sint32 x = 2; x <= 3; ++x)
        CHECK(SotnStageRoute_FindBoss(dra, 6, x, 3, &world, &found, &route) && !found &&
              route.room == 99);
    CHECK(SotnStageRoute_FindBoss(dra, 25, 2, 3, &world, &found, &route) && !found &&
          route.room == 99);
    return 0;
}

static int OlroxBoss(void)
{
    Uint8 bytes[0x297c + 2 * 20] = {0};
    SotnDataView dra = {bytes, sizeof(bytes)};
    Uint8 *boss = bytes + 0x297c, *arrival = bytes + 0x245c + 125 * 10;
    // Authored coordinates differ from the private US record; only routing rules are tested.
    Put32(boss, 7);
    Put32(boss + 4, 8);
    Put32(boss + 8, 4);
    Put32(boss + 12, SOTN_TIME_OLROX);
    Put32(boss + 16, 125);
    Put32(boss + 20, 128);
    Put16(arrival, 480);
    Put16(arrival + 2, 100);
    Put16(arrival + 6, 4);
    Put16(arrival + 8, 30);
    SotnWorldState world = {0};
    SotnStageRoute route = {0};
    bool found = false;
    CHECK(SotnStageRoute_FindBoss(dra, 4, 7, 8, &world, &found, &route) && found &&
          route.stage == 30 && route.room == 0 && route.source == 4 && route.x == 480);
    CHECK(SotnWorld_VisitEvent(&world, SOTN_TIME_OLROX));
    world.cutscenes_seen = 0x300;
    CHECK(SotnStageRoute_FindBoss(dra, 4, 7, 8, &world, &found, &route) && found);
    CHECK(SotnStageRoute_FindBoss(dra, 4, 6, 8, &world, &found, &route) && !found);
    CHECK(SotnWorld_RecordTime(&world, SOTN_TIME_OLROX, 321));
    route.stage = 99;
    CHECK(SotnStageRoute_FindBoss(dra, 4, 7, 8, &world, &found, &route) && !found &&
          route.stage == 99);
    CHECK(SotnStageRoute_FindBoss(dra, 30, 7, 8, &world, &found, &route) && !found &&
          route.stage == 99);
    return 0;
}
static int Window(void)
{
    SotnRoomWindow window = {.end_x = 1024, .last_cell = 3};
    SotnRoomCrossing crossing = {0}, full = {0};
    const Sint32 xs[] = {-1, 0, 128, 1023, 1024};
    const Sint32 ys[] = {-1, 3, 4, 227, 228, 251, 252, 256};
    for (Uint16 ground = 0; ground < 2; ++ground)
        for (size_t x = 0; x < SDL_arraysize(xs); ++x)
            for (size_t y = 0; y < SDL_arraysize(ys); ++y)
            {
                CHECK(SotnStageRoute_Crossing(xs[x], ys[y], 1024, 256, ground != 0, &full));
                CHECK(SotnStageRoute_WindowCrossing(window, xs[x], ys[y], 1024, 256, ground != 0,
                                                    &crossing));
                // Horizontal probes select cell origins; arrival still uses the real player.
                CHECK(crossing.active == full.active && crossing.y == full.y &&
                      crossing.arrival_y == full.arrival_y &&
                      crossing.x == (xs[x] < 0 ? -256 : full.x));
            }
    window.end_x = 640;
    --window.last_cell;
    CHECK(SotnStageRoute_WindowCrossing(window, 641, 640, 1024, 2560, false, &crossing) &&
          crossing.active && crossing.x == 768 && crossing.y == 640 && !crossing.arrival_y);
    window = (SotnRoomWindow){.start_x = 384, .end_x = 1024, .origin_cell = 1, .last_cell = 3};
    CHECK(SotnStageRoute_WindowCrossing(window, 383, 896, 1024, 2560, false, &crossing) &&
          crossing.active && crossing.x == 0 && crossing.y == 896);
    CHECK(SotnStageRoute_WindowCrossing(window, 641, 896, 1024, 2560, false, &crossing) &&
          !crossing.active);
    CHECK(SotnStageRoute_WindowCrossing(window, 1024, 896, 1024, 2560, false, &crossing) &&
          !crossing.active);
    CHECK(SotnStageRoute_WindowCrossing(window, 1024, 3, 1024, 2560, false, &crossing) &&
          crossing.active && crossing.x == 1024 && crossing.arrival_y == -48);
    CHECK(SotnStageRoute_WindowCrossing(window, 383, 3, 1024, 2560, false, &crossing) &&
          crossing.active && crossing.x == 0 && !crossing.arrival_y);
    // Stable physical world position: original left++ and room-X-=256 cancel.
    const Sint32 physical_x = 383, initial_cell = 13;
    CHECK(initial_cell * 256 + physical_x ==
          (initial_cell + window.origin_cell) * 256 + physical_x - window.origin_cell * 256);
    crossing.arrival_y = 123;
    const SotnRoomWindow invalid[] = {
        {.end_x = 1025, .last_cell = 3},
        {.start_x = -1, .end_x = 1024, .last_cell = 3},
        {.start_x = 1024, .end_x = 1024, .last_cell = 3},
        {.end_x = 1024, .origin_cell = -1, .last_cell = 3},
        {.end_x = 1024, .origin_cell = 65, .last_cell = 3},
        {.end_x = 1024, .last_cell = -2},
        {.end_x = 1024, .last_cell = 4},
    };
    for (size_t i = 0; i < SDL_arraysize(invalid); ++i)
        CHECK(!SotnStageRoute_WindowCrossing(invalid[i], 128, 128, 1024, 256, false, &crossing) &&
              crossing.arrival_y == 123);
    CHECK(!SotnStageRoute_WindowCrossing(window, 128, 128, 1025, 256, false, &crossing));
    CHECK(!SotnStageRoute_WindowCrossing(window, -65, 128, 1024, 256, false, &crossing));
    CHECK(!SotnStageRoute_WindowCrossing(window, 128, 128, 1024, 256, false, NULL));
    CHECK(crossing.arrival_y == 123);
    return 0;
}
int main(void)
{
    CHECK(Boss() == 0);
    CHECK(ChapelBoss() == 0);
    CHECK(OlroxBoss() == 0);
    CHECK(Window() == 0);
    Uint8 bytes[0x245c + 131 * 10] = {0};
    SotnDataView dra = {bytes, sizeof(bytes)};
    Uint8 *record = bytes + 0x245c + 130 * 10;
    // Synthetic final record uses deliberately distinct source, target, and room.
    const Uint16 values[] = {752, 132, 80, 65, 12};
    for (Uint32 i = 0; i < SDL_arraysize(values); ++i)
        Put16(record + i * 2, values[i]);
    SotnStageRoute route = {0};
    CHECK(SotnStageRoute_Read(dra, 65, 130, &route));
    CHECK(route.source == 65 && route.stage == 12 && route.room == 10 && route.x == 752 &&
          route.y == 132);
    route.room = 999;
    CHECK(!SotnStageRoute_Read(dra, 12, 130, &route) && route.room == 999);
    CHECK(!SotnStageRoute_Read(dra, 65, 131, &route) && route.room == 999);
    CHECK(!SotnStageRoute_Read((SotnDataView){bytes, sizeof(bytes) - 1}, 65, 130, &route));
    CHECK(!SotnStageRoute_Read((SotnDataView){0}, 65, 130, &route));
    CHECK(!SotnStageRoute_Read(dra, 65, 130, NULL));
    const Uint16 offsets[] = {0, 2, 4, 4, 6, 8};
    const Uint16 invalid[] = {16385, 16385, 81, 2048, 66, 66};
    for (Uint32 i = 0; i < SDL_arraysize(offsets); ++i)
    {
        Put16(record + offsets[i], invalid[i]);
        CHECK(!SotnStageRoute_Read(dra, 65, 130, &route) && route.room == 999);
        Put16(record + offsets[i], values[offsets[i] / 2]);
    }
    SotnRoomCrossing crossing = {0};
    CHECK(SotnStageRoute_Crossing(128, 4, 768, 256, false, &crossing) && !crossing.active);
    CHECK(SotnStageRoute_Crossing(128, 3, 768, 256, false, &crossing));
    CHECK(crossing.active && crossing.x == 128 && crossing.y == -1 && crossing.arrival_y == -48);
    CHECK(SotnStageRoute_Crossing(128, 251, 768, 256, false, &crossing) && !crossing.active);
    CHECK(SotnStageRoute_Crossing(128, 252, 768, 256, false, &crossing));
    CHECK(crossing.active && crossing.y == 256 && crossing.arrival_y == 24);
    CHECK(SotnStageRoute_Crossing(128, 227, 768, 256, true, &crossing) && !crossing.active);
    CHECK(SotnStageRoute_Crossing(128, 228, 768, 256, true, &crossing));
    CHECK(crossing.active && crossing.y == 256 && crossing.arrival_y == 48);
    CHECK(SotnStageRoute_Crossing(768, -2, 768, 512, false, &crossing));
    CHECK(crossing.active && crossing.x == 768 && crossing.y == -2 && !crossing.arrival_y);
    CHECK(SotnStageRoute_Crossing(-1, 510, 768, 512, true, &crossing));
    CHECK(crossing.active && crossing.x == -1 && crossing.y == 510 && !crossing.arrival_y);
    crossing.arrival_y = 123;
    CHECK(!SotnStageRoute_Crossing(128, 128, 0, 512, false, &crossing));
    CHECK(!SotnStageRoute_Crossing(128, 128, 768, 257, false, &crossing));
    CHECK(!SotnStageRoute_Crossing(-65, 128, 768, 512, false, &crossing));
    CHECK(!SotnStageRoute_Crossing(128, 577, 768, 512, false, &crossing));
    CHECK(!SotnStageRoute_Crossing(128, 128, 768, 512, false, NULL));
    CHECK(crossing.arrival_y == 123);
    SDL_Log("STAGE_ROUTE: PASS");
    return 0;
}
