#include <SDL3/SDL.h>
#include <sotn/shield.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8));
}
int main(void)
{
    const size_t size = 53 * 0x7000;
    Uint8 *bytes = SDL_calloc(1, size);
    CHECK(bytes != NULL);
    Uint8 *overlay = bytes + 52 * 0x7000 + 0x4000;
    for (Uint32 i = 0; i < 14; ++i)
    {
        Put32(overlay + 0xb84 + i * 4, 0x8017b000 + i * 64);
        for (Uint32 j = 0; j < 3; ++j)
        {
            Put16(overlay + 0x1000 + i * 64 + j * 4, 2);
            Put16(overlay + 0x1002 + i * 64 + j * 4, (Uint16)(0x200 | (i * 3 + j + 1)));
        }
        Put16(overlay + 0x100c + i * 64, 0xffff);
    }
    overlay[0xb40] = 14;
    overlay[0xb41] = 249;
    overlay[0xb42] = 4;
    overlay[0xb43] = 12;
    const SotnDataView weapons = {bytes, size};
    SotnShield shield = {0};
    CHECK(SotnShield_Tick(weapons, 99, 0, false, true, false, &shield));
    CHECK(shield.phase == 1 && shield.frame.sprite == 31 && shield.frame.x == 14 &&
          shield.frame.y == -7);
    for (Uint32 i = 0; i < 7; ++i)
        CHECK(SotnShield_Tick(weapons, 99, 0, false, true, false, &shield));
    CHECK(shield.phase == 2 && shield.frame.sprite == 1);
    CHECK(SotnShield_Tick(weapons, 7, 2, false, true, false, &shield));
    CHECK(shield.animation == 8 && shield.frame.sprite == 27);
    CHECK(SotnShield_Tick(weapons, 101, 0, true, true, false, &shield));
    CHECK(shield.animation == 1 && shield.frame.sprite == 4);
    CHECK(SotnShield_Tick(weapons, 17, 0, true, false, false, &shield));
    CHECK(shield.phase == 3 && shield.animation == 13);
    for (Uint32 i = 0; i < 7; ++i)
        CHECK(SotnShield_Tick(weapons, 17, 0, true, false, false, &shield));
    CHECK(shield.phase == 0 && shield.frame.sprite == 0);
    CHECK(SotnShield_Tick(weapons, 99, 0, false, true, false, &shield));
    CHECK(SotnShield_Tick(weapons, 99, 0, false, true, true, &shield) && shield.phase == 0);
    Put32(overlay + 0xb84 + 10 * 4, 0xfffffff0);
    CHECK(!SotnShield_Tick(weapons, 99, 0, false, true, false, &shield) && shield.phase == 0);
    CHECK(!SotnShield_Tick((SotnDataView){0}, 99, 0, false, true, false, &shield));
    CHECK(!SotnShield_Tick(weapons, 99, 0, false, true, false, NULL));
    SDL_free(bytes);
    SDL_Log("ORDINARY_SHIELD: PASS");
    return 0;
}
