// SPDX-License-Identifier: MPL-2.0
// Adapted from PSY-Z b52003e55c0d1b20719e984dd19bd4d760eeb14a,
// psyz/src/psyz/libgte_sw_sqrt.c:SquareRoot12_impl. Lookup bytes remain private.
#include <SDL3/SDL.h>
#include <sotn/game_math.h>

bool SotnMath_Sqrt12(SotnDataView image, Uint32 value, Uint32 *out)
{
    if (out == NULL || image.bytes == NULL || image.size < 0x1e3e0 + 192 * 2 ||
        value > SDL_MAX_SINT32)
        return SDL_SetError("SDK square root requires nonnegative input and its private table");
    if (value == 0)
    {
        *out = 0;
        return true;
    }
    Uint32 leading = 0, probe = value;
    while (!(probe & 0x80000000U))
    {
        probe <<= 1;
        ++leading;
    }
    const Uint32 even = leading & 0xfffffffeU;
    const Uint32 normalized = even >= 24 ? value << (even - 24) : value >> (24 - even);
    if (normalized < 64 || normalized >= 256)
        return SDL_SetError("SDK square root normalization exceeds its private table");
    const size_t offset = 0x1e3e0 + (normalized - 64U) * 2U;
    const Uint16 coefficient = (Uint16)(image.bytes[offset] | (Uint16)image.bytes[offset + 1] << 8);
    if (coefficient < 4096 || coefficient > 8192)
        return SDL_SetError("Private SDK square root coefficient is outside its original range");
    const Uint32 shift = (31 - even) / 2;
    *out = (Uint32)(((Uint64)coefficient << shift) >> 6);
    return true;
}
bool SotnMath_Sqrt0(SotnDataView image, Uint32 value, Uint32 *out)
{
    Uint32 root = 0;
    if (!out || !SotnMath_Sqrt12(image, value, &root))
        return SDL_SetError("SDK whole root requires an output and valid private normalization");
    *out = root >> 6;
    return true;
}
