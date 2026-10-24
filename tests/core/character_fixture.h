#ifndef SOTN_TEST_CHARACTER_FIXTURE_H
#define SOTN_TEST_CHARACTER_FIXTURE_H
#include <SDL3/SDL.h>
/* Original synthetic records. Leaves the caller's unrelated collision/actor data intact. */
static void TestCharacter_Put32(Uint8 *p, Uint32 v)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8));
}
static void TestCharacter_Fill(Uint8 *dra)
{
    TestCharacter_Put32(dra + 0xc0, 0x800a0500);
    TestCharacter_Put32(dra + 0xc4, 0x800a2800);
    dra[0x180] = 'T' - 32;
    dra[0x181] = 'e' - 32;
    dra[0x182] = 's' - 32;
    dra[0x183] = 't' - 32;
    dra[0x184] = 255;
    dra[0x190] = 0;
    for (Uint32 body = 0; body < 2; ++body)
        for (Uint32 id = 0; id < (body ? 90U : 169U); ++id)
        {
            Uint8 *p = dra + (body ? 0x2800 + id * 32 : 0x500 + id * 52);
            SDL_memset(p, 0, body ? 32U : 52U);
            TestCharacter_Put32(p, 0x800a0180);
            TestCharacter_Put32(p + 4, 0x800a0190);
            if (body)
                p[28] = (Uint8)(id == 26 ? 0 : id == 48 ? 2 : id == 57 ? 3 : 1);
        }
    dra[0x2800 + 1 * 32 + 10] = 3;
    dra[0x2800 + 1 * 32 + 12] = 2;
    dra[0x2800 + 2 * 32 + 10] = 2;
}
#endif
