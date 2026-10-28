#include <SDL3/SDL.h>
#include <sotn/camera_shake.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 n)
{
    p[0] = (Uint8)n;
    p[1] = (Uint8)(n >> 8);
}
static void Put32(Uint8 *p, Uint32 n)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(n >> (i * 8U));
}
int main(void)
{
    Uint8 image[0x3200] = {0};
    const SotnDataView dra = {image, sizeof(image)};
    // Authored patterns intentionally differ from the original private game tables.
    for (Uint16 i = 0; i < 7; ++i)
        Put32(image + 0x3194 + i * 4U, 0x800a0100);
    Put16(image + 0x100, 9);
    Put16(image + 0x102, 65529);
    Put16(image + 0x104, 5);
    Put16(image + 0x106, 0x7fff);
    SotnCameraShake shake = {0};
    CHECK(SotnCameraShake_Tick(&shake, (SotnDataView){0}));
    for (Uint16 type = 0; type < 7; ++type)
    {
        shake = (SotnCameraShake){.x = 12, .y = 13, .amount = 14};
        CHECK(SotnCameraShake_Request(&shake, dra, type) && shake.type == type &&
              shake.pattern == 0x100 && !shake.index && shake.amount == 14 && shake.x == 12 &&
              shake.y == 13);
        CHECK(SotnCameraShake_Tick(&shake, dra));
        if (type == 0 || type == 5)
        {
            CHECK(!shake.index && shake.type == type && shake.amount == 14 && shake.x == 12 &&
                  shake.y == 13);
            continue;
        }
        CHECK(shake.index == 1 && shake.amount == 9 && shake.x == (type == 2 ? 9 : 12) &&
              shake.y == (type == 2 ? 13 : 9));
        CHECK(SotnCameraShake_Tick(&shake, dra) && shake.amount == -7);
        CHECK(SotnCameraShake_Tick(&shake, dra) && !shake.type && !shake.amount &&
              shake.index == 3 && shake.x == (type == 2 ? 0 : 12) &&
              shake.y == (type == 2 ? 13 : 0));
        const SotnCameraShake ended = shake;
        CHECK(SotnCameraShake_Tick(&shake, (SotnDataView){0}) &&
              !SDL_memcmp(&shake, &ended, sizeof(shake)));
    }
    shake = (SotnCameraShake){0};
    CHECK(SotnCameraShake_Request(&shake, dra, 2) && SotnCameraShake_Tick(&shake, dra));
    CHECK(SotnCameraShake_Request(&shake, dra, 3) && SotnCameraShake_Tick(&shake, dra) &&
          shake.x == 9 && shake.y == 9);
    CHECK(SotnCameraShake_Tick(&shake, dra) && SotnCameraShake_Tick(&shake, dra) && shake.x == 9 &&
          !shake.y);
    const SotnCameraShake before = shake;
    CHECK(!SotnCameraShake_Request(&shake, dra, 7) && !SDL_memcmp(&shake, &before, sizeof(shake)));
    Put32(image + 0x3194, 0x800a3200);
    CHECK(!SotnCameraShake_Request(&shake, dra, 0) && !SDL_memcmp(&shake, &before, sizeof(shake)));
    CHECK(!SotnCameraShake_Request(NULL, dra, 0));
    shake.type = 2;
    shake.pattern = SDL_MAX_UINT32 - 1;
    const SotnCameraShake bad = shake;
    CHECK(!SotnCameraShake_Tick(&shake, dra) && !SDL_memcmp(&shake, &bad, sizeof(shake)));
    shake.pattern = 0x100;
    shake.index = 3;
    CHECK(!SotnCameraShake_Tick(&shake, dra));
    shake.index = 256;
    CHECK(!SotnCameraShake_Tick(&shake, dra));
    CHECK(!SotnCameraShake_Tick(NULL, dra));
    return 0;
}
