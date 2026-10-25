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
int main(void)
{
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
