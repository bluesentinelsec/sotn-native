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

void SotnData_Sha1(const Uint8 *bytes, size_t size, char output[41])
{
    Uint32 state[5] = {0x67452301U, 0xefcdab89U, 0x98badcfeU, 0x10325476U, 0xc3d2e1f0U};
    const Uint64 bit_length = (Uint64)size * 8U;
    while (size >= 64)
    {
        HashBlock(state, bytes);
        bytes += 64;
        size -= 64;
    }
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
