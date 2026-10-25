// SPDX-License-Identifier: MPL-2.0
// Selected rcossin interpretation from PSY-Z b52003e55c0d1b20719e984dd19bd4d760eeb14a,
// psyz/src/psyz/libgte.c. The full original pair table stays in the private boot image.
#include <SDL3/SDL.h>
#include <sotn/game_math.h>
static Sint32 SignedWord(const Uint8 *bytes)
{
    const Uint16 value = (Uint16)(bytes[0] | (Uint16)bytes[1] << 8);
    return value < 32768 ? value : (Sint32)value - 65536;
}
bool SotnMath_MatrixTrig(SotnDataView executable, Sint16 angle, SotnMathTrig *out)
{
    if (executable.bytes == NULL || executable.size < 0x1ea10 + 4096 * 4 || out == NULL)
        return SDL_SetError("SDK matrix trigonometry requires its complete private pair table");
    const Sint32 signed_angle = angle;
    const Uint32 magnitude = (Uint32)(signed_angle < 0 ? -signed_angle : signed_angle);
    const size_t offset = 0x1ea10 + (magnitude & 4095U) * 4U;
    const Sint32 sine = SignedWord(executable.bytes + offset);
    const Sint32 cosine = SignedWord(executable.bytes + offset + 2);
    if (sine < -4096 || sine > 4096 || cosine < -4096 || cosine > 4096)
        return SDL_SetError("Private SDK matrix pair exceeds its original amplitude");
    *out = (SotnMathTrig){(Sint16)(signed_angle < 0 ? -sine : sine), (Sint16)cosine};
    return true;
}
