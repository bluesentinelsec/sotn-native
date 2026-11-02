// SPDX-License-Identifier: MPL-2.0
// Reverb processor adapted from PSY-Z b52003e55c0d1b20719e984dd19bd4d760eeb14a,
// psyz/src/psyz/psyz_spu.c. Native bounds and borrowed RAM replace global SDK registers.
// Coefficients remain caller-supplied private SDK data, never compiled game tables.
#include "reverb.h"

static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static Sint16 Word(Sint32 n)
{
    const Uint16 bits = (Uint16)(Uint32)n;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
bool SoundReverb_Configure(SotnReverb *r, Uint32 base, SotnDataView coefficients)
{
    if (!r || !coefficients.bytes || coefficients.size != 64 || base % 8 ||
        base >= SOTN_SPU_RAM_SIZE)
        return SDL_SetError(
            "SPU reverb requires bounded coefficients and its real aligned work area");
    SotnReverb next = {.base = base, .cursor = base, .configured = true, .enabled = true};
    for (Uint32 i = 0; i < 32; ++i)
        next.parameters[i] = Read16(coefficients.bytes + i * 2);
    // Selected SDK profiles must never alias voice/sample RAM outside their real work area.
    // Nonnegative APF differences also retain the original16-bit feedback addressing.
    for (Uint32 i = 10; i < 30; ++i)
        if ((Uint32)next.parameters[i] * 8 >= SOTN_SPU_RAM_SIZE - base)
            return SDL_SetError("SPU reverb delay exceeds its actual work area");
    const Uint16 previous[] = {10, 11, 18, 19};
    for (Uint32 i = 0; i < SDL_arraysize(previous); ++i)
        if (base && !next.parameters[previous[i]])
            return SDL_SetError("SPU reverb previous-sample read would alias unrelated RAM");
    for (Uint32 i = 26; i < 30; ++i)
        if (next.parameters[i] < next.parameters[i < 28 ? 0 : 1])
            return SDL_SetError("SPU reverb feedback would alias outside its work area");
    *r = next;
    return true;
}
bool SoundReverb_SetDepth(SotnReverb *r, Sint16 left, Sint16 right)
{
    if (!r || !r->configured)
        return SDL_SetError("SPU reverb depth requires actual configured ownership");
    r->depth[0] = Word((Sint32)left * 32767 / 127);
    r->depth[1] = Word((Sint32)right * 32767 / 127);
    return true;
}
static Uint32 Address(const SotnReverb *r, Uint16 offset, Sint32 extra)
{
    Uint32 address = (r->cursor + (Uint32)offset * 8 + (Uint32)extra) & 0x7ffffU;
    if (address < r->base)
        address += r->base;
    return address & 0x7ffffU;
}
static Sint32 Read(const SotnReverb *r, const Uint8 *ram, Uint16 offset, Sint32 extra)
{
    const Uint32 address = Address(r, offset, extra);
    return Word(ram[address] | (Sint32)ram[(address + 1) & 0x7ffffU] << 8);
}
static void Write(const SotnReverb *r, Uint8 *ram, Uint16 offset, Sint32 value)
{
    if (!r->enabled)
        return;
    const Uint32 address = Address(r, offset, 0);
    const Uint16 bits = (Uint16)clamp16(value);
    ram[address] = (Uint8)bits;
    ram[(address + 1) & 0x7ffffU] = (Uint8)(bits >> 8);
}
static Sint32 Multiply(Sint32 sample, Uint16 coefficient)
{
    return (sample * Word(coefficient)) >> 15;
}
static void Process(SotnReverb *r, Uint8 *ram, Sint32 left, Sint32 right, Sint16 output[2])
{
    const Uint16 *p = r->parameters;
    const Sint32 input[2] = {left, right};
    for (Uint32 c = 0; c < 2; ++c)
    {
        const Sint32 iir_a =
            clamp16(Multiply(Read(r, ram, p[16 + c], 0), p[7]) + Multiply(input[c], p[30 + c]));
        const Sint32 iir_b = clamp16(Multiply(Read(r, ram, p[24 + (1 - c)], 0), p[7]) +
                                     Multiply(input[c], p[30 + c]));
        const Sint32 inverse = 32768 - Word(p[2]);
        Write(r, ram, p[10 + c],
              clamp16(Multiply(iir_a, p[2]) + (Read(r, ram, p[10 + c], -2) * inverse >> 15)));
        Write(r, ram, p[18 + c],
              clamp16(Multiply(iir_b, p[2]) + (Read(r, ram, p[18 + c], -2) * inverse >> 15)));
        const Sint32 accumulator = Multiply(Read(r, ram, p[12 + c], 0), p[3]) +
                                   Multiply(Read(r, ram, p[14 + c], 0), p[4]) +
                                   Multiply(Read(r, ram, p[20 + c], 0), p[5]) +
                                   Multiply(Read(r, ram, p[22 + c], 0), p[6]);
        const Sint32 feedback_a = Read(r, ram, (Uint16)(p[26 + c] - p[0]), 0);
        const Sint32 feedback_b = Read(r, ram, (Uint16)(p[28 + c] - p[1]), 0);
        const Sint32 mix_a = clamp16(accumulator - Multiply(feedback_a, p[8]));
        const Sint32 mix_b =
            clamp16(feedback_a + Multiply(mix_a, p[8]) - Multiply(feedback_b, p[9]));
        output[c] = clamp16(feedback_b + Multiply(mix_b, p[9]));
        Write(r, ram, p[26 + c], mix_a);
        Write(r, ram, p[28 + c], mix_b);
    }
    r->cursor = (r->cursor + 2) & 0x7ffffU;
    if (r->cursor < r->base)
        r->cursor = r->base;
}
void SoundReverb_Tick(SotnReverb *r, Uint8 ram[SOTN_SPU_RAM_SIZE], Sint32 left, Sint32 right,
                      Sint32 output[2])
{
    Sint32 wet[2];
    if (!r->phase)
    {
        r->input[0] = clamp16(left);
        r->input[1] = clamp16(right);
        wet[0] = r->previous[0];
        wet[1] = r->previous[1];
    }
    else
    {
        Sint16 next[2];
        Process(r, ram, (r->input[0] + left) / 2, (r->input[1] + right) / 2, next);
        wet[0] = (r->previous[0] + next[0]) / 2;
        wet[1] = (r->previous[1] + next[1]) / 2;
        r->previous[0] = next[0];
        r->previous[1] = next[1];
    }
    r->phase = !r->phase;
    output[0] = Multiply(wet[0], (Uint16)r->depth[0]);
    output[1] = Multiply(wet[1], (Uint16)r->depth[1]);
}
