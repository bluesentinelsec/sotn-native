#include <SDL3/SDL.h>
#include <sotn/collision.h>
#include <sotn/decompress.h>

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

static bool TestCollision(void)
{
    Uint8 tiles[18] = {0};
    Uint8 types[1] = {3};
    const SotnCollisionMap map = {{tiles, sizeof(tiles)}, {types, sizeof(types)}, 3, 3};
    SotnCollider result = {0};
    CHECK(SotnCollision_Query(&map, 19, 21, &result));
    CHECK(result.effects == SOTN_EFFECT_BLOCK);
    CHECK(result.left == -3 && result.top == -5 && result.right == 12 && result.bottom == 10);
    CHECK(SotnCollision_Query(&map, -1, 0, &result));
    CHECK(result.effects == 0 && result.left == -15 && result.right == 0);
    CHECK(SotnCollision_Query(&map, SDL_MIN_SINT32, SDL_MAX_SINT32, &result));
    CHECK(result.effects == 0);
    for (int code = 0; code <= 255; ++code)
    {
        types[0] = (Uint8)code;
        for (int y = 0; y < 16; ++y)
        {
            for (int x = 0; x < 16; ++x)
            {
                CHECK(SotnCollision_Query(&map, 16 + x, 16 + y, &result));
                CHECK(result.tile_left == -x && result.tile_top == -y);
                if (code < 128)
                    CHECK(result.effects == ((Uint32)code & SOTN_EFFECT_BLOCK));
                if (code == 0x80)
                    CHECK(((result.effects & SOTN_EFFECT_SOLID) != 0) == (x + y >= 15));
                if (code == 0x83)
                    CHECK(((result.effects & SOTN_EFFECT_SOLID) != 0) == (y >= x));
                if (code == 0xe7 && (result.effects & SOTN_EFFECT_SOLID) != 0)
                    CHECK((result.effects & SOTN_EFFECT_SOLID_FROM_ABOVE) != 0);
            }
        }
    }
    result.effects = 0x1234;
    tiles[8] = 1;
    CHECK(!SotnCollision_Query(&map, 16, 16, &result));
    CHECK(result.effects == 0x1234);
    tiles[8] = 0;
    types[0] = 0x92;
    CHECK(SotnCollision_Query(&map, 18, 16, &result));
    CHECK(result.bottom == SDL_MIN_SINT32 + 14);
    result.effects = 0x1234;
    SotnCollisionMap invalid = map;
    invalid.width = SDL_MAX_UINT32;
    CHECK(!SotnCollision_Query(&invalid, 0, 0, &result));
    CHECK(!SotnCollision_Query(NULL, 0, 0, &result));
    CHECK(!SotnCollision_Query(&map, 0, 0, NULL));
    /* A slope filler probes the tile above and must reject its invalid ID too. */
    types[0] = 0x81;
    tiles[2] = 1;
    CHECK(!SotnCollision_Query(&map, 16, 16, &result));
    CHECK(result.effects == 0x1234);
    return true;
}

static bool TestDecompression(void)
{
    /* Synthetic stream: literal, pair, repeated nibble, zero run, dictionary, end. */
    Uint8 source[] = {0x1a, 0x2b, 0x60, 0, 0, 0, 0, 0, 0x11, 0x32, 0x35, 0x40, 0x60, 0x78, 0x9f};
    Uint8 pixels[64] = {0};
    size_t written = 0;
    CHECK(SotnGraphics_Decompress((SotnDataView){source, sizeof(source)}, pixels, sizeof(pixels),
                                  &written));
    static const Uint8 expected[] = {0x21, 0x43, 0x44, 0x00, 0xa0, 0xbb, 0x00, 0x00};
    CHECK(written == sizeof(expected));
    CHECK(SDL_memcmp(pixels, expected, written) == 0);
    for (size_t size = 0; size < sizeof(source); ++size)
    {
        SDL_memset(pixels, 0xaa, sizeof(pixels));
        CHECK(!SotnGraphics_Decompress((SotnDataView){source, size}, pixels, sizeof(pixels),
                                       &written));
        CHECK(written == 0);
        for (size_t i = 0; i < sizeof(pixels); ++i)
            CHECK(pixels[i] == 0xaa);
    }
    CHECK(!SotnGraphics_Decompress((SotnDataView){source, sizeof(source)}, pixels, 7, &written));
    CHECK(pixels[0] == 0xaa && written == 0);
    CHECK(!SotnGraphics_Decompress((SotnDataView){source, sizeof(source)}, source, sizeof(source),
                                   &written));
    CHECK(!SotnGraphics_Decompress((SotnDataView){source, sizeof(source)}, pixels, sizeof(pixels),
                                   NULL));
    /* A legal odd-nibble stream clears the high half of its final byte. */
    source[8] = 0x1c;
    source[9] = 0xf0;
    CHECK(SotnGraphics_Decompress((SotnDataView){source, 10}, pixels, sizeof(pixels), &written));
    CHECK(written == 1 && pixels[0] == 0x0c);
    return true;
}

static bool TestGraphicsOpcodes(void)
{
    static const struct
    {
        Uint8 stream[3];
        size_t stream_size;
        size_t bytes;
        Uint8 first, last;
    } cases[] = {
        {{0x00, 0x0f}, 2, 10, 0, 0},
        {{0x0f, 0xff}, 2, 137, 0, 0},
        {{0x2b, 0xf0}, 2, 1, 0xbb, 0xbb},
        {{0x41, 0x23, 0xf0}, 3, 2, 0x21, 0x03},
        {{0x5a, 0xff}, 2, 9, 0xaa, 0xaa},
        {{0x6f, 0xf0}, 2, 9, 0, 0},
        {{0xff}, 1, 0, 0, 0},
    };
    for (size_t i = 0; i < SDL_arraysize(cases); ++i)
    {
        Uint8 source[11] = {0};
        Uint8 pixels[140] = {0};
        size_t written = 0;
        SDL_memcpy(source + 8, cases[i].stream, cases[i].stream_size);
        CHECK(SotnGraphics_Decompress((SotnDataView){source, 8 + cases[i].stream_size}, pixels,
                                      sizeof(pixels), &written));
        CHECK(written == cases[i].bytes);
        if (written != 0)
            CHECK(pixels[0] == cases[i].first && pixels[written - 1] == cases[i].last);
    }
    for (Uint8 op = 7; op < 15; ++op)
    {
        Uint8 source[9] = {0};
        Uint8 pixels[4] = {0};
        size_t written = 0;
        source[op - 7] = 0x2d;
        source[8] = (Uint8)((op << 4) | 15);
        CHECK(SotnGraphics_Decompress((SotnDataView){source, sizeof(source)}, pixels,
                                      sizeof(pixels), &written));
        CHECK(written == 1 && pixels[0] == 0xdd);
    }
    return true;
}

int main(void)
{
    if (!TestCollision() || !TestDecompression() || !TestGraphicsOpcodes())
        return 1;
    SDL_Log("SOTN_CORE_TEST: PASS");
    return 0;
}
