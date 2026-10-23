// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted from sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5,
// src/dra/4AEA4.c:DecompressData. See docs/game-core.md for modifications.
#include <SDL3/SDL_error.h>
#include <sotn/decompress.h>

typedef struct Decoder
{
    SotnDataView source;
    size_t read;
    size_t count;
    size_t capacity;
    Uint8 *pixels;
    bool low_nibble;
} Decoder;

static bool ReadNibble(Decoder *decoder, Uint8 *value)
{
    if (decoder->read >= decoder->source.size)
        return SDL_SetError("Compressed graphics ended before the terminating opcode");
    const Uint8 byte = decoder->source.bytes[decoder->read];
    *value = decoder->low_nibble ? byte & 15 : byte >> 4;
    if (decoder->low_nibble)
        ++decoder->read;
    decoder->low_nibble = !decoder->low_nibble;
    return true;
}

static bool Repeat(Decoder *decoder, Uint8 value, size_t count)
{
    if (count > decoder->capacity * 2 - decoder->count)
        return SDL_SetError("Decoded graphics exceed the %zu-byte output buffer",
                            decoder->capacity);
    if (decoder->pixels != NULL)
    {
        for (size_t i = 0; i < count; ++i)
        {
            const size_t nibble = decoder->count + i;
            if ((nibble & 1) == 0)
                decoder->pixels[nibble / 2] = value;
            else
                decoder->pixels[nibble / 2] |= (Uint8)(value << 4);
        }
    }
    decoder->count += count;
    return true;
}

static bool Decode(Decoder *decoder)
{
    for (;;)
    {
        Uint8 op = 0, value = 0, extra = 0;
        if (!ReadNibble(decoder, &op))
            return false;
        switch (op)
        {
        case 0:
            if (!ReadNibble(decoder, &value) || !ReadNibble(decoder, &extra) ||
                !Repeat(decoder, 0, (size_t)value * 16 + extra + 19))
                return false;
            break;
        case 1:
        case 2:
            if (!ReadNibble(decoder, &value) || !Repeat(decoder, value, op))
                return false;
            break;
        case 3:
        case 4:
            for (Uint8 i = 0; i < op - 1; ++i)
            {
                if (!ReadNibble(decoder, &value) || !Repeat(decoder, value, 1))
                    return false;
            }
            break;
        case 5:
            if (!ReadNibble(decoder, &value) || !ReadNibble(decoder, &extra) ||
                !Repeat(decoder, value, (size_t)extra + 3))
                return false;
            break;
        case 6:
            if (!ReadNibble(decoder, &extra) || !Repeat(decoder, 0, (size_t)extra + 3))
                return false;
            break;
        case 15:
            return true;
        default:
            value = decoder->source.bytes[op - 7];
            if ((value & 0xf0) == 0x10 || (value & 0xf0) == 0x20)
            {
                if (!Repeat(decoder, value & 15, value >> 4))
                    return false;
            }
            else if ((value & 0xf0) == 0x60 && !Repeat(decoder, 0, (size_t)(value & 15) + 3))
                return false;
            break;
        }
    }
}

bool SotnGraphics_Decompress(SotnDataView source, Uint8 *pixels, size_t capacity, size_t *written)
{
    if (written == NULL)
        return SDL_SetError("Decoded graphics require an output byte count");
    *written = 0;
    if (source.bytes == NULL || source.size < 9 || pixels == NULL || capacity == 0 ||
        capacity > 64 * 1024 * 1024)
        return SDL_SetError("Compressed graphics require a dictionary, stream, and bounded output");
    const uintptr_t input_address = (uintptr_t)source.bytes;
    const uintptr_t output_address = (uintptr_t)pixels;
    if ((input_address <= output_address && output_address - input_address < source.size) ||
        (output_address < input_address && input_address - output_address < capacity))
        return SDL_SetError("Compressed graphics input and output must not overlap");
    Decoder validation = {source, 8, 0, capacity, NULL, false};
    if (!Decode(&validation))
        return false;
    Decoder decoder = {source, 8, 0, capacity, pixels, false};
    if (!Decode(&decoder))
        return false;
    *written = (decoder.count + 1) / 2;
    return true;
}
