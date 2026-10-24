#include "data_internal.h"

static Uint32 RotateLeft(Uint32 value, unsigned int count)
{
    return (value << count) | (value >> (32U - count));
}

/* FIPS 180-4 sections 5 and 6.1; revision fingerprints only, never authentication.
 * https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf */
static void HashBlock(Uint32 state[5], const Uint8 block[64])
{
    Uint32 words[80] = {0};
    for (size_t i = 0; i < 16; ++i)
    {
        const Uint8 *word = block + i * 4;
        words[i] = ((Uint32)word[0] << 24) | ((Uint32)word[1] << 16) | ((Uint32)word[2] << 8) |
                   (Uint32)word[3];
    }
    for (size_t i = 16; i < 80; ++i)
    {
        words[i] = RotateLeft(words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16], 1);
    }
    Uint32 a = state[0], b = state[1], c = state[2], d = state[3], e = state[4];
    for (size_t i = 0; i < 80; ++i)
    {
        Uint32 function = 0, constant = 0;
        if (i < 20)
        {
            function = (b & c) | (~b & d);
            constant = 0x5a827999U;
        }
        else if (i < 40)
        {
            function = b ^ c ^ d;
            constant = 0x6ed9eba1U;
        }
        else if (i < 60)
        {
            function = (b & c) | (b & d) | (c & d);
            constant = 0x8f1bbcdcU;
        }
        else
        {
            function = b ^ c ^ d;
            constant = 0xca62c1d6U;
        }
        const Uint32 next = RotateLeft(a, 5) + function + e + constant + words[i];
        e = d;
        d = c;
        c = RotateLeft(b, 30);
        b = a;
        a = next;
    }
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

static void Finish(Uint32 state[5], const Uint8 *bytes, size_t size, Uint64 bit_length,
                   char output[41])
{
    Uint8 tail[128] = {0};
    if (size != 0)
    {
        SDL_memcpy(tail, bytes, size);
    }
    tail[size] = 0x80;
    const size_t padded_size = size < 56 ? 64 : 128;
    for (size_t i = 0; i < 8; ++i)
    {
        tail[padded_size - 1 - i] = (Uint8)(bit_length >> (i * 8));
    }
    HashBlock(state, tail);
    if (padded_size == 128)
    {
        HashBlock(state, tail + 64);
    }
    static const char hex[] = "0123456789abcdef";
    for (size_t i = 0; i < 20; ++i)
    {
        const Uint8 byte = (Uint8)(state[i / 4] >> ((3 - i % 4) * 8));
        output[i * 2] = hex[byte >> 4];
        output[i * 2 + 1] = hex[byte & 15];
    }
    output[40] = '\0';
}

void SotnData_Sha1(const Uint8 *bytes, size_t size, char output[41])
{
    Uint32 state[5] = {0x67452301U, 0xefcdab89U, 0x98badcfeU, 0x10325476U, 0xc3d2e1f0U};
    const Uint64 bits = (Uint64)size * 8U;
    while (size >= 64)
    {
        HashBlock(state, bytes);
        bytes += 64;
        size -= 64;
    }
    Finish(state, bytes, size, bits, output);
}

static bool ReadExact(SDL_IOStream *stream, Uint8 *buffer, size_t size)
{
    size_t read = 0;
    while (read < size)
    {
        const size_t count = SDL_ReadIO(stream, buffer + read, size - read);
        if (count == 0)
            return SDL_SetError("Short read while fingerprinting private music");
        read += count;
    }
    return true;
}

bool SotnData_VerifyStream(SDL_IOStream *stream, const SotnDataSpec *spec)
{
    if (stream == NULL || spec == NULL || spec->sha1 == NULL ||
        spec->size > 1024U * 1024U * 1024U || SDL_GetIOSize(stream) != (Sint64)spec->size ||
        SDL_SeekIO(stream, 0, SDL_IO_SEEK_SET) != 0)
        return SDL_SetError("Music stream has an invalid reference size or cannot seek");
    Uint32 state[5] = {0x67452301U, 0xefcdab89U, 0x98badcfeU, 0x10325476U, 0xc3d2e1f0U};
    Uint8 scratch[32768] = {0};
    size_t remaining = spec->size;
    while (remaining >= 64)
    {
        const size_t size = SDL_min(sizeof(scratch), remaining - remaining % 64);
        if (!ReadExact(stream, scratch, size))
            return false;
        for (size_t offset = 0; offset < size; offset += 64)
            HashBlock(state, scratch + offset);
        remaining -= size;
    }
    if (!ReadExact(stream, scratch, remaining))
        return false;
    char digest[41] = {0};
    Finish(state, scratch, remaining, (Uint64)spec->size * 8, digest);
    Uint8 extra = 0;
    if (SDL_ReadIO(stream, &extra, 1) != 0 || SDL_GetIOStatus(stream) != SDL_IO_STATUS_EOF)
        return SDL_SetError("Music changed size while checking its reference");
    if (SDL_strcmp(digest, spec->sha1) != 0)
        return SDL_SetError("Music reference checksum mismatch; supply the unmodified US file");
    return SDL_SeekIO(stream, 0, SDL_IO_SEEK_SET) == 0;
}
