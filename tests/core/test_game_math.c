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
    CHECK(SotnMath_Atan2Vector(image, 65536, 131072, &result) && result == 256);
    // The SDK deliberately normalizes large ratios before integer division.
    CHECK(SotnMath_Atan2Vector(image, 0x200000, 0x200001, &result) && result == 512);
    CHECK(SotnMath_Atan2Vector(image, SDL_MAX_SINT32, SDL_MAX_SINT32, &result) && result == 512);
    CHECK(!SotnMath_Atan2Vector(image, SDL_MIN_SINT32, 1, &result) && result == 512);
    CHECK(!SotnMath_Atan2(image, SDL_MIN_SINT32, 0, &result) && result == 512);
    CHECK(!SotnMath_Atan2((SotnDataView){bytes, size - 1}, 1, 1, &result) && result == 512);
    bytes[0x22a10 + 1024] = 255;
    bytes[0x22a11 + 1024] = 255;
    CHECK(!SotnMath_Atan2(image, 1, 2, &result) && result == 512);
    CHECK(!SotnMath_Atan2(image, 0, 1, NULL));
    SDL_free(bytes);
    return true;
}
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static bool TrigAndRoot(void)
{
    const size_t size = 0x1e3e0 + 384;
    Uint8 *bytes = SDL_calloc(1, size);
    CHECK(bytes != NULL);
    for (Uint32 i = 0; i <= 1024; ++i)
        Put16(bytes + 0x1dbcc + i * 2, (Uint16)(i * 4));
    for (Uint32 i = 0; i < 192; ++i)
        Put16(bytes + 0x1e3e0 + i * 2, 4096);
    const SotnDataView image = {bytes, size};
    SotnMathTrig trig = {0};
    Uint32 root = 999;
    CHECK(SotnMath_Trig(image, 0, &trig) && trig.sine == 0 && trig.cosine == 4096);
    CHECK(SotnMath_Trig(image, 512, &trig) && trig.sine == 2048 && trig.cosine == 2048);
    CHECK(SotnMath_Trig(image, -512, &trig) && trig.sine == -2048 && trig.cosine == 2048);
    CHECK(SotnMath_Trig(image, 3072, &trig) && trig.sine == -4096 && trig.cosine == 0);
    CHECK(SotnMath_Trig(image, SDL_MIN_SINT32, &trig) && trig.sine == 0 && trig.cosine == 4096);
    CHECK(SotnMath_Sqrt12(image, 0, &root) && root == 0);
    CHECK(SotnMath_Sqrt12(image, 1, &root) && root == 64);
    CHECK(SotnMath_Sqrt12(image, 4, &root) && root == 128);
    CHECK(SotnMath_Sqrt12(image, 4096, &root) && root == 4096);
    Put16(bytes + 0x1e3e0 + 128 * 2, 6000);
    CHECK(SotnMath_Sqrt12(image, 3, &root) && root == 93);
    CHECK(!SotnMath_Sqrt12(image, 0x80000000U, &root) && root == 93);
    CHECK(!SotnMath_Sqrt12((SotnDataView){bytes, size - 1}, 3, &root) && root == 93);
    Put16(bytes + 0x1e3e0, 0);
    CHECK(!SotnMath_Sqrt12(image, 1, &root) && root == 93);
    Put16(bytes + 0x1dbcc + 512 * 2, 5000);
    CHECK(!SotnMath_Trig(image, 512, &trig) && trig.sine == 0 && trig.cosine == 4096);
    CHECK(!SotnMath_Trig((SotnDataView){bytes, 0x1dbcc + 2049}, 0, &trig));
    CHECK(!SotnMath_Trig(image, 0, NULL));
    CHECK(!SotnMath_Sqrt12(image, 0, NULL));
    SDL_free(bytes);
    return true;
}
static bool MatrixTrig(void)
{
    const size_t size = 0x1ea10 + 4096 * 4;
    Uint8 *bytes = SDL_calloc(1, size);
    CHECK(bytes != NULL);
    Put16(bytes + 0x1ea12, 4096);
    Put16(bytes + 0x1ea10 + 1024 * 4, 4096);
    Put16(bytes + 0x1ea10 + 512 * 4, 2000);
    Put16(bytes + 0x1ea12 + 512 * 4, 3000);
    const SotnDataView image = {bytes, size};
    SotnMathTrig pair = {0};
    CHECK(SotnMath_MatrixTrig(image, 512, &pair) && pair.sine == 2000 && pair.cosine == 3000);
    CHECK(SotnMath_MatrixTrig(image, -512, &pair) && pair.sine == -2000 && pair.cosine == 3000);
    CHECK(SotnMath_MatrixTrig(image, -1024, &pair) && pair.sine == -4096 && pair.cosine == 0);
    CHECK(SotnMath_MatrixTrig(image, -32768, &pair) && pair.sine == 0 && pair.cosine == 4096);
    Put16(bytes + 0x1ea10 + 512 * 4, 5000);
    CHECK(!SotnMath_MatrixTrig(image, 512, &pair) && pair.sine == 0 && pair.cosine == 4096);
    CHECK(!SotnMath_MatrixTrig((SotnDataView){bytes, size - 1}, 0, &pair));
    CHECK(!SotnMath_MatrixTrig(image, 0, NULL));
    SDL_free(bytes);
    return true;
}
int main(void)
{
    return Angles() && TrigAndRoot() && MatrixTrig() ? 0 : 1;
}
