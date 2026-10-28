// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/624DC.c:ShakeCamera/StepShakeRequest/UpdateCameraShake, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Patterns remain private data.
#include <SDL3/SDL.h>
#include <sotn/camera_shake.h>
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static Sint16 Signed16(const Uint8 *p)
{
    const Uint16 n = Read16(p);
    return (Sint16)(n < 32768 ? n : (Sint32)n - 65536);
}
bool SotnCameraShake_Request(SotnCameraShake *s, SotnDataView dra, Uint16 type)
{
    const size_t table = 0x3194;
    if (!s || type > 6 || !dra.bytes || dra.size < table + 7U * 4U)
        return SDL_SetError("Camera shake requires its original request and private pattern table");
    const Uint8 *p = dra.bytes + table + type * 4U;
    const Uint32 address = p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
    if (address < 0x800a0000U || (address & 1U) || address - 0x800a0000U > dra.size ||
        dra.size - (address - 0x800a0000U) < 4)
        return SDL_SetError("Camera shake pattern pointer exceeds its private main image");
    s->type = type;
    s->index = 0;
    s->pattern = address - 0x800a0000U;
    return true;
}
bool SotnCameraShake_Tick(SotnCameraShake *s, SotnDataView dra)
{
    if (!s || s->type > 6)
        return SDL_SetError("Camera shake update requires its bounded original cursor");
    if (s->type == 0 || s->type == 5)
        return true;
    if (!dra.bytes || s->index >= 256 || (s->pattern & 1U) || s->pattern > dra.size ||
        dra.size - s->pattern < (size_t)s->index * 2U + 4U)
        return SDL_SetError("Camera shake update exceeds its bounded private pattern");
    const size_t offset = s->pattern + (size_t)s->index * 2U;
    if (Read16(dra.bytes + offset) == 0x7fff)
        return SDL_SetError("Camera shake cursor unexpectedly points at its terminal word");
    SotnCameraShake next = *s;
    next.amount = Signed16(dra.bytes + offset);
    ++next.index;
    if (Read16(dra.bytes + offset + 2) == 0x7fff)
    {
        next.type = 0;
        next.amount = 0;
    }
    if (s->type == 2)
        next.x = next.amount;
    else
        next.y = next.amount;
    *s = next;
    return true;
}
