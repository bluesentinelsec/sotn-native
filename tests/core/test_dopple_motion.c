#include <SDL3/SDL.h>
#include <sotn/dopple_motion.h>

#define CHECK(condition)                                                                           \
    do                                                                                             \
    {                                                                                              \
        if (!(condition))                                                                          \
        {                                                                                          \
            SDL_LogError(SDL_LOG_CATEGORY_TEST, "Line %d: %s: %s", __LINE__, #condition,           \
                         SDL_GetError());                                                          \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

static void Put16(Uint8 *image, size_t offset, Sint16 value)
{
    const Uint16 bits = (Uint16)value;
    image[offset] = (Uint8)bits;
    image[offset + 1] = (Uint8)(bits >> 8);
}
static void Sensors(Uint8 *image)
{
    /* Original-sized records with deliberately synthetic geometry. */
    for (size_t i = 0; i < 4; ++i)
    {
        const Sint16 x = i == 2 ? 2 : i == 3 ? -2 : 0;
        Put16(image, 0x124c + i * 2, -3);
        Put16(image, 0x1254 + i * 2, 3);
        Put16(image, 0x126c + i * 2, -9);
        Put16(image, 0x1274 + i * 2, i == 0 ? 10 : 8);
        Put16(image, 0x128c + i * 4, x);
        Put16(image, 0x128e + i * 4, -9);
        Put16(image, 0x129c + i * 4, x);
        Put16(image, 0x129e + i * 4, i == 0 ? 10 : 8);
    }
    const Sint16 ys[7] = {7, 4, 1, -2, -5, -7, -9};
    for (size_t i = 0; i < 7; ++i)
    {
        Put16(image, 0x125c + i * 2, i == 0 ? 2 : i == 6 ? -2 : 0);
        Put16(image, 0x127c + i * 2, ys[i]);
    }
    for (size_t i = 0; i < 14; ++i)
    {
        Put16(image, 0x12ac + i * 4, i < 7 ? 3 : -3);
        Put16(image, 0x12ae + i * 4, ys[i % 7]);
    }
}
static bool Tests(void)
{
    Uint8 image[0x12e4] = {0}, tiles[16 * 16 * 2] = {0};
    Uint8 types[2] = {0, 3};
    Sensors(image);
    const SotnDataView overlay = {image, sizeof(image)};
    const SotnCollisionMap map = {{tiles, sizeof(tiles)}, {types, sizeof(types)}, 16, 16};
    for (size_t y = 0; y < 16; ++y)
        for (size_t x = 0; x < 16; ++x)
            if (y < 2 || y >= 10 || x >= 12 || x < 2)
                tiles[(y * 16 + x) * 2] = 1;
    SotnDoppleMotion motion = {0};
    CHECK(SotnDoppleMotion_Init(overlay, &motion));
    CHECK(motion.initialized && motion.wall_sensors[0].x == 3 && motion.wall_sensors[13].x == -3);
    motion.x = 128 * 65536;
    motion.y = 120 * 65536;
    motion.vy = 2 * 65536;
    for (size_t i = 0; i < 20; ++i)
        CHECK(SotnDoppleMotion_Advance(overlay, &map, false, &motion));
    CHECK((motion.contact & 1U) && motion.y == 152 * 65536);
    motion.vx = 3 * 65536;
    motion.vy = 0;
    for (size_t i = 0; i < 30; ++i)
        CHECK(SotnDoppleMotion_Advance(overlay, &map, false, &motion));
    CHECK((motion.contact & 4U) && motion.x == 189 * 65536);
    motion.vx = -3 * 65536;
    for (size_t i = 0; i < 60; ++i)
        CHECK(SotnDoppleMotion_Advance(overlay, &map, false, &motion));
    CHECK((motion.contact & 8U) && motion.x == 34 * 65536);
    motion.vx = 0;
    motion.vy = -2 * 65536;
    for (size_t i = 0; i < 60; ++i)
        CHECK(SotnDoppleMotion_Advance(overlay, &map, false, &motion));
    CHECK((motion.contact & 2U) && motion.y == 40 * 65536);

    CHECK(SotnDoppleMotion_Init(overlay, &motion));
    motion.x = 128 * 65536;
    motion.y = 100 * 65536;
    motion.status = 1;
    motion.vx = -65537;
    CHECK(SotnDoppleMotion_Advance(overlay, &map, false, &motion));
    CHECK(motion.vx == -65540 && motion.x == 128 * 65536 - 65540);
    CHECK(motion.floor_sensors[0].y == 3 && motion.ceiling_sensors[0].y == -3);
    motion.status = 0x800000U;
    motion.vx = 0;
    CHECK(SotnDoppleMotion_Advance(overlay, &map, false, &motion));
    CHECK(motion.floor_sensors[0].y == 3);
    motion.status = 0x20U;
    CHECK(SotnDoppleMotion_Advance(overlay, &map, false, &motion));
    CHECK(motion.floor_sensors[0].y == 10 && motion.wall_sensors[0].y == 7);
    motion.y = 65536;
    motion.vy = -8 * 65536;
    CHECK(SotnDoppleMotion_Advance(overlay, &map, true, &motion));
    CHECK(motion.y == -65536);

    const Sint32 x = motion.x, y = motion.y;
    motion.vx = SDL_MIN_SINT32;
    CHECK(!SotnDoppleMotion_Advance(overlay, &map, false, &motion));
    CHECK(motion.x == x && motion.y == y && motion.vx == SDL_MIN_SINT32);
    motion.vx = 0;
    CHECK(!SotnDoppleMotion_Advance((SotnDataView){0}, &map, false, &motion));
    CHECK(!SotnDoppleMotion_Advance(overlay, NULL, false, &motion));
    CHECK(!SotnDoppleMotion_Advance(overlay, &map, false, NULL));
    CHECK(!SotnDoppleMotion_Init(overlay, NULL));
    CHECK(!SotnDoppleMotion_Init((SotnDataView){image, sizeof(image) - 1}, &motion));
    CHECK(motion.x == x && motion.y == y);
    Put16(image, 0x128c, 65);
    CHECK(!SotnDoppleMotion_Init(overlay, &motion));
    CHECK(motion.x == x && motion.y == y);
    Sensors(image);
    motion.y = 100 * 65536;
    const SotnCollisionMap broken = {{0}, {types, sizeof(types)}, 16, 16};
    CHECK(!SotnDoppleMotion_Advance(overlay, &broken, false, &motion));
    CHECK(motion.y == 100 * 65536 && motion.x == x);
    Put16(image, 0x1274, 65);
    CHECK(!SotnDoppleMotion_Advance(overlay, &map, false, &motion));
    CHECK(motion.floor_sensors[0].y == 10);
    /* Exercise every encoded slope/one-way/mist geometry with both directions
     * and form profiles. Sanitizers check the original wide collision offsets. */
    Sensors(image);
    SDL_memset(tiles, 0, sizeof(tiles));
    for (Uint32 code = 128; code <= 255; ++code)
        for (Uint32 form = 0; form < 3; ++form)
            for (Sint32 offset = 0; offset < 16; ++offset)
            {
                types[0] = (Uint8)code;
                CHECK(SotnDoppleMotion_Init(overlay, &motion));
                motion.x = (128 + offset) * 65536;
                motion.y = (128 + offset) * 65536;
                motion.vx = offset & 1 ? 3 * 65536 : -3 * 65536;
                motion.vy = offset & 2 ? 2 * 65536 : -2 * 65536;
                motion.status = form;
                CHECK(SotnDoppleMotion_Advance(overlay, &map, false, &motion));
            }
    types[0] = 0;
    CHECK(SotnDoppleMotion_Init(overlay, &motion));
    motion.x = SDL_MAX_SINT32 - 1000;
    motion.y = 100 * 65536;
    motion.vx = 65536;
    CHECK(SotnDoppleMotion_Advance(overlay, &map, false, &motion));
    CHECK(motion.x == SDL_MIN_SINT32 + 64535);
    return true;
}
int main(void)
{
    if (!Tests())
        return 1;
    SDL_Log("SOTN_DOPPLE_MOTION_TEST: PASS");
    return 0;
}
