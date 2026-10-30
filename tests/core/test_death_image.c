#include <SDL3/SDL.h>
#include <sotn/death_image.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
int main(void)
{
    Uint8 source[SOTN_DEATH_IMAGE_BYTES] = {0};
    SDL_memset(source, 0xab, sizeof(source));
    SotnDeathImage image = {0};
    CHECK(!SotnDeathImage_Tick(&image) && !image.captured && !image.remaining);
    CHECK(SotnDeathImage_Begin(&image, (SotnDataView){source, sizeof(source)}) && image.captured &&
          image.remaining == 64 && image.cursor == 0 &&
          !SDL_memcmp(image.pixels, source, sizeof(source)));
    const SotnDeathImage original = image;
    CHECK(!SotnDeathImage_Begin(&image, (SotnDataView){source, sizeof(source) - 1U}) &&
          !SDL_memcmp(&image, &original, sizeof(image)));
    for (Uint16 update = 0; update < 64; ++update)
    {
        CHECK(SotnDeathImage_Tick(&image) && image.remaining == 63U - update);
        size_t live = 0;
        for (size_t i = 0; i < sizeof(image.pixels); ++i)
            live += (image.pixels[i] & 15U ? 1U : 0U) + (image.pixels[i] & 240U ? 1U : 0U);
        CHECK(live == 10240U - (update + 1U) * 160U);
        if (!update)
            CHECK(image.cursor == 140 && image.pixels[0] == 0x0b && image.pixels[1] == 0xab);
    }
    CHECK(!image.cursor && !SDL_memcmp(source, original.pixels, sizeof(source)));
    const SotnDeathImage finished = image;
    CHECK(!SotnDeathImage_Tick(&image) && !SDL_memcmp(&image, &finished, sizeof(image)));
    image = original;
    image.remaining = 65;
    const SotnDeathImage invalid = image;
    CHECK(!SotnDeathImage_Tick(&image) && !SDL_memcmp(&image, &invalid, sizeof(image)));
    image = original;
    CHECK(SotnDeathImage_Begin(&image, (SotnDataView){image.pixels, sizeof(image.pixels)}) &&
          !SDL_memcmp(&image, &original, sizeof(image)));
    SDL_Log("DEATH_IMAGE: PASS packed capture/alias/immutability, exact nibble coverage, cursor "
            "wrap,64-update completion and invalid-bound rollback");
    return 0;
}
