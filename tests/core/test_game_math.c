#include <SDL3/SDL.h>
#include <sotn/game_math.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static bool Angles(void)
{
    const size_t size = 0x22a10 + 2050;
    Uint8 *bytes = SDL_calloc(1, size);
    CHECK(bytes != NULL);
    for (Uint32 i = 0; i <= 1024; ++i)
    {
        Uint16 v = (Uint16)(i / 2);
        bytes[0x22a10 + i * 2] = (Uint8)v;
        bytes[0x22a11 + i * 2] = (Uint8)(v >> 8);
    }
    const SotnDataView image = {bytes, size};
    Sint16 result = 123;
    CHECK(SotnMath_Atan2(image, 0, 0, &result) && result == 0);
    CHECK(SotnMath_Atan2(image, 0, 1, &result) && result == 0);
    CHECK(SotnMath_Atan2(image, 1, 0, &result) && result == 1024);
    CHECK(SotnMath_Atan2(image, 0, -1, &result) && result == 2048);
    CHECK(SotnMath_Atan2(image, -1, 0, &result) && result == -1024);
    CHECK(SotnMath_Atan2(image, 1, 1, &result) && result == 512);
    CHECK(SotnMath_Atan2(image, 1, -1, &result) && result == 1536);
    CHECK(SotnMath_Atan2(image, -1, -1, &result) && result == -1536);
    CHECK(SotnMath_Atan2(image, -1, 1, &result) && result == -512);
    CHECK(SotnMath_Atan2(image, 1, 2, &result) && result == 256);
    CHECK(SotnMath_Atan2(image, 2, 1, &result) && result == 768);
    CHECK(SotnMath_Atan2(image, 32767, 32767, &result) && result == 512);
    CHECK(!SotnMath_Atan2(image, SDL_MIN_SINT32, 0, &result) && result == 512);
    CHECK(!SotnMath_Atan2((SotnDataView){bytes, size - 1}, 1, 1, &result) && result == 512);
    bytes[0x22a10 + 1024] = 255;
    bytes[0x22a11 + 1024] = 255;
    CHECK(!SotnMath_Atan2(image, 1, 2, &result) && result == 512);
    CHECK(!SotnMath_Atan2(image, 0, 1, NULL));
    SDL_free(bytes);
    return true;
}
int main(void)
{
    return Angles() ? 0 : 1;
}
