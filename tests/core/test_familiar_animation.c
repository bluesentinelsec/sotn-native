#include <SDL3/SDL.h>
#include <sotn/familiar_animation.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static void Put32(Uint8 *p, Uint32 value)
{
    Put16(p, (Uint16)value);
    Put16(p + 2, (Uint16)(value >> 16));
}
static bool Run(void)
{
    Uint8 overlay[128] = {0}, properties[8] = {0, 0, 1, 2, 253, 128, 250, 251};
    Put32(overlay, 0x80170020);
    Put32(overlay + 4, 0x80170040);
    Put32(overlay + 8, 0x80170020);
    Put16(overlay + 32, 2);
    Put16(overlay + 34, 3);
    Put16(overlay + 36, 1);
    Put16(overlay + 38, 4);
    Put16(overlay + 40, 0xfffe);
    Put16(overlay + 42, 1);
    Put16(overlay + 44, 1);
    Put16(overlay + 46, 5);
    Put16(overlay + 50, 5);
    Put16(overlay + 54, 6);
    Put16(overlay + 56, 0xffff);
    Put16(overlay + 64, 2);
    Put16(overlay + 66, 7 | 512);
    Put16(overlay + 68, 0xffff);
    const SotnFamiliarAnimationSource source = {.overlay = {overlay, sizeof(overlay)},
                                                .properties = {properties, sizeof(properties)},
                                                .table = 0x80170000,
                                                .count = 3};
    SotnFamiliarAnimation n = {0};
    Sint32 status = 123;
    CHECK(SotnFamiliarAnimation_Set(&n, source, 0, false) &&
          SotnFamiliarAnimation_Tick(&n, source, &status) && !status && n.remaining == 2 &&
          !n.pose && n.sprite == 3 && n.hitbox_width == 1);
    const SotnFamiliarAnimation held = n;
    CHECK(SotnFamiliarAnimation_Set(&n, source, 2, false) && !SDL_memcmp(&n, &held, sizeof(n)));
    CHECK(SotnFamiliarAnimation_Tick(&n, source, &status) && n.remaining == 1 && !n.pose &&
          SotnFamiliarAnimation_Tick(&n, source, &status) && n.remaining == 1 && n.pose == 1 &&
          n.sprite == 4 && SotnFamiliarAnimation_Tick(&n, source, &status) && status == -2 &&
          n.address == 0x80170040 && !n.pose && n.remaining == 2 && n.sprite == 7 &&
          n.hitbox_x == -3 && n.hitbox_y == -128 && n.hitbox_width == 250 &&
          n.hitbox_height == 251);
    CHECK(SotnFamiliarAnimation_Tick(&n, source, &status) && !status && n.remaining == 1 &&
          SotnFamiliarAnimation_Tick(&n, source, &status) && status == -1 && !n.pose &&
          n.remaining == -1 && n.sprite == 7);
    const SotnFamiliarAnimation terminal = n;
    CHECK(SotnFamiliarAnimation_Tick(&n, source, &status) && status == -1 &&
          !SDL_memcmp(&n, &terminal, sizeof(n)) && SotnFamiliarAnimation_Set(&n, source, 1, true) &&
          !n.pose && !n.remaining && n.sprite == terminal.sprite);
    n = (SotnFamiliarAnimation){.address = 0x80170020, .pose = 3, .remaining = 1};
    CHECK(SotnFamiliarAnimation_Tick(&n, source, &status) && !status && n.pose == 5 &&
          !n.remaining && n.sprite == 6);
    n = (SotnFamiliarAnimation){.address = 0x80170020, .remaining = SDL_MIN_SINT16};
    CHECK(SotnFamiliarAnimation_Tick(&n, source, &status) && n.remaining == SDL_MAX_SINT16);
    n = (SotnFamiliarAnimation){.address = 0x80170020, .pose = 1, .remaining = 1};
    const SotnFamiliarAnimation before = n;
    Put16(overlay + 42, 3);
    status = 123;
    CHECK(!SotnFamiliarAnimation_Tick(&n, source, &status) && status == 123 &&
          !SDL_memcmp(&n, &before, sizeof(n)));
    Put16(overlay + 42, 1);
    SotnFamiliarAnimationSource truncated = source;
    truncated.properties.size = 7;
    CHECK(!SotnFamiliarAnimation_Tick(&n, truncated, &status) && status == 123 &&
          !SDL_memcmp(&n, &before, sizeof(n)));
    CHECK(!SotnFamiliarAnimation_Set(&n, source, 3, false) && !SDL_memcmp(&n, &before, sizeof(n)) &&
          !SotnFamiliarAnimation_Tick(&n, (SotnFamiliarAnimationSource){0}, &status) &&
          !SDL_memcmp(&n, &before, sizeof(n)));
    SotnFamiliarAnimationSource no_properties = source;
    no_properties.properties = (SotnDataView){0};
    n.remaining = -2;
    n.hitbox_x = 700;
    CHECK(SotnFamiliarAnimation_Tick(&n, no_properties, NULL) && n.remaining == -3 &&
          n.hitbox_x == 700 && n.sprite == 4);
    return true;
}
int main(void)
{
    return Run() ? 0 : 1;
}
