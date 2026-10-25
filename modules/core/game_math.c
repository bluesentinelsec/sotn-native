// SPDX-License-Identifier: MIT
// Original quantized ratio/quadrant interpretation follows reconstructed SDK
// ratan2 in sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5,
// src/main/psxsdk/libgte/ratan.c. The SDK lookup remains private runtime data.
#include <SDL3/SDL.h>
#include <sotn/game_math.h>
static bool Angle(SotnDataView image, Sint32 y, Sint32 x, Sint16 *out)
{
    if (out == NULL || x == SDL_MIN_SINT32 || y == SDL_MIN_SINT32 || image.bytes == NULL ||
        image.size < 0x22a10 + 2050)
        return SDL_SetError(
            "SDK angle requires bounded pixel differences and the private US boot table");
    if (x == 0 && y == 0)
    {
        *out = 0;
        return true;
    }
    const Uint32 ax = (Uint32)(x < 0 ? -x : x), ay = (Uint32)(y < 0 ? -y : y);
    const Uint32 minor = SDL_min(ax, ay), major = SDL_max(ax, ay);
    const Uint32 index = (minor & 0x7fe00000U) ? minor / (major >> 10) : (minor * 1024U) / major;
    if (index > 1024)
        return SDL_SetError("SDK angle ratio exceeds its private lookup");
    const size_t offset = 0x22a10 + (size_t)index * 2;
    const Uint16 value = (Uint16)(image.bytes[offset] | (Uint16)image.bytes[offset + 1] << 8);
    if (value > 512)
        return SDL_SetError("Private SDK angle table value exceeds its first octant");
    Sint32 angle = ay < ax ? value : 1024 - value;
    if (x < 0)
        angle = 2048 - angle;
    if (y < 0)
        angle = -angle;
    *out = (Sint16)angle;
    return true;
}

bool SotnMath_Atan2(SotnDataView image, Sint32 y, Sint32 x, Sint16 *out)
{
    if (x < -32767 || x > 32767 || y < -32767 || y > 32767)
        return SDL_SetError("SDK pixel direction exceeds its bounded room differences");
    return Angle(image, y, x, out);
}
bool SotnMath_Atan2Vector(SotnDataView image, Sint32 y, Sint32 x, Sint16 *out)
{
    return Angle(image, y, x, out);
}
