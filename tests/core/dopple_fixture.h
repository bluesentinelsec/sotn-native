#ifndef SOTN_TEST_DOPPLE_FIXTURE_H
#define SOTN_TEST_DOPPLE_FIXTURE_H
#include <SDL3/SDL.h>
static void Put16(Uint8 *image, size_t at, Uint16 value)
{
    image[at] = (Uint8)value;
    image[at + 1] = (Uint8)(value >> 8);
}
static void Put32(Uint8 *image, size_t at, Uint32 value)
{
    Put16(image, at, (Uint16)value);
    Put16(image, at + 2, (Uint16)(value >> 16));
}
static void Fixture(Uint8 *image)
{
    /* Synthetic geometry, transition profiles and repeated animation poses. */
    for (size_t i = 0; i < 4; ++i)
    {
        const Sint16 x = i == 2 ? 2 : i == 3 ? -2 : 0;
        Put16(image, 0x126c + i * 2, (Uint16)-9);
        Put16(image, 0x1274 + i * 2, i == 0 ? 12 : 8);
        Put16(image, 0x128c + i * 4, (Uint16)x);
        Put16(image, 0x128e + i * 4, (Uint16)-9);
        Put16(image, 0x129c + i * 4, (Uint16)x);
        Put16(image, 0x129e + i * 4, i == 0 ? 12 : 8);
        image[0x1318 + i * 2] = i == 0 ? 0 : i == 1 ? 4 : i == 2 ? 1 : 3;
        image[0x1319 + i * 2] = (Uint8)(10 + i);
    }
    for (size_t i = 0; i < 14; ++i)
    {
        const Sint16 y = (Sint16)(7 - (Sint32)(i % 7) * 2);
        Put16(image, 0x12ac + i * 4, i < 7 ? 3 : (Uint16)-3);
        Put16(image, 0x12ae + i * 4, (Uint16)y);
        if (i < 7)
            Put16(image, 0x127c + i * 2, (Uint16)y);
    }
    for (size_t i = 0; i < 10; ++i)
        image[0x1320 + i] = (Uint8)(i + 2);
    Put16(image, 0x1334, 1);
    Put16(image, 0x1336, 15);
    Put16(image, 0x1338, 0);
    Put16(image, 0x133a, 16);
    for (size_t i = 0; i < 220; ++i)
        Put32(image, 0x3cb4 + i * 4, 0x80180100);
    for (size_t i = 0; i < 32; ++i)
    {
        Put16(image, 0x100 + i * 4, 1);
        Put16(image, 0x102 + i * 4, 1);
    }
    Put16(image, 0x180, 0xffff);
}
#endif
