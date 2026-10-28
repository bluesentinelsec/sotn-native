#include <SDL3/SDL.h>
#include <sotn/player_afterimage.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static Uint8 image[0xcf4c];
static SotnDataView Image(void)
{
    // Authored timings/fades intentionally differ from private original profiles.
    SDL_memset(image + 0xcf18, 3, 20);
    SDL_memset(image + 0xcf2c, 5, 16);
    SDL_memset(image + 0xcf3c, 28, 16);
    return (SotnDataView){image, sizeof(image)};
}
static bool Capture(void)
{
    SotnPlayerAfterimage s = {0};
    CHECK(SotnPlayerAfterimage_Init(&s));
    CHECK(s.displays[0].bank == 1 && s.displays[0].texture_slot == 1 &&
          s.displays[2].texture_slot == 3 && s.displays[0].palette == 0x8100);
    const SotnAfterimageInput input = {.x = -32768,
                                       .y = 25 * 65536 + 123,
                                       .pressed = 0x2000,
                                       .tapped = 0x2000,
                                       .sprite = 0x8021,
                                       .palette = 0x8100,
                                       .facing_left = true};
    CHECK(SotnPlayerAfterimage_Capture(&s, (SotnDataView){0}, input));
    CHECK(s.index == 0 && s.timer == 0 && s.pose == 2 && s.write_index == 1 &&
          s.samples[0].x == -1 && s.samples[0].y == 25 && s.samples[0].sprite == 0x8021 &&
          s.samples[0].palette == 0x8100 && s.samples[0].facing == 1 && s.samples[0].red == 128 &&
          s.samples[0].green == 128 && s.samples[0].blue == 128);
    const SotnDataView dra = Image();
    for (Uint16 i = 0; i < 17; ++i)
        CHECK(SotnPlayerAfterimage_Capture(&s, dra, input));
    CHECK(s.write_index == 0 && !s.pose);
    for (Uint16 i = 0; i < 6; ++i)
        CHECK(s.samples[i].sprite == 0x8021);
    s.index = 8;
    s.timer = 2;
    s.pose = 0;
    CHECK(SotnPlayerAfterimage_Capture(&s, dra, (SotnAfterimageInput){.held = 0x2000}) &&
          !s.index && !s.timer &&
          s.pose == 2); // Release of the complete previous word restarts activity.
    s.index = 8;
    s.timer = 2;
    CHECK(SotnPlayerAfterimage_Capture(&s, dra,
                                       (SotnAfterimageInput){.tapped = 0x900, .pressed = 0x900}) &&
          s.index == 8 && s.timer == 1); // Original Start/Select words are excluded.
    CHECK(SotnPlayerAfterimage_Capture(&s, dra, (SotnAfterimageInput){.velocity_y = 32769}) &&
          !s.index && !s.timer);
    CHECK(SotnPlayerAfterimage_Init(&s));
    for (Uint16 i = 0; i < 30; ++i)
        CHECK(SotnPlayerAfterimage_Capture(&s, dra, (SotnAfterimageInput){0}));
    CHECK(s.index == 10 && s.timer == 3);
    const SotnPlayerAfterimage ended = s;
    CHECK(SotnPlayerAfterimage_Capture(&s, (SotnDataView){0}, input) && s.index == 0);
    s = ended;
    CHECK(SotnPlayerAfterimage_Capture(&s, (SotnDataView){0}, (SotnAfterimageInput){0}) &&
          !SDL_memcmp(&s, &ended, sizeof(s)));
    CHECK(SotnPlayerAfterimage_Init(&s));
    image[0xcf18] = 0;
    CHECK(SotnPlayerAfterimage_Capture(&s, dra, (SotnAfterimageInput){0}) && !s.index &&
          s.timer == 255);
    const Uint16 excluded[] = {0x5d, 0x5e, 0x60, 0x61, 0x62};
    for (Uint16 i = 0; i < SDL_arraysize(excluded); ++i)
    {
        s.index = 0;
        const Uint8 timer = s.timer;
        CHECK(SotnPlayerAfterimage_Capture(&s, (SotnDataView){0},
                                           (SotnAfterimageInput){.animation = excluded[i]}) &&
              s.index == 10 && s.timer == timer);
    }
    s.disabled = true;
    const SotnPlayerAfterimage disabled = s;
    CHECK(SotnPlayerAfterimage_Capture(&s, (SotnDataView){0}, input) &&
          !SDL_memcmp(&disabled, &s, sizeof(s)));
    return true;
}
static bool PresentAndReset(void)
{
    const SotnDataView dra = Image();
    SotnPlayerAfterimage s = {0};
    CHECK(SotnPlayerAfterimage_Init(&s));
    for (Uint16 i = 0; i < 6; ++i)
        s.samples[i] = (SotnAfterimageSample){.x = (Sint16)(i + 10),
                                              .y = (Sint16)(i + 20),
                                              .sprite = (Uint16)(i + 30),
                                              .palette = 0x8100,
                                              .facing = 1,
                                              .red = 128,
                                              .green = 128,
                                              .blue = 128};
    s.samples[0].red = 115;
    s.samples[0].blue = 236;
    s.samples[1].red = 90;
    s.samples[1].blue = 239;
    s.samples[2].red = 31;
    s.displays[0].x = 0x1234;
    s.displays[0].y = 0x4567;
    CHECK(SotnPlayerAfterimage_Present(&s, dra, 0, 148));
    CHECK(s.samples[0].red == 110 && s.samples[0].blue == 242 && s.samples[0].green == 128 &&
          !s.samples[0].blend && s.samples[1].red == 85 && s.samples[1].blue == 245 &&
          s.samples[1].blend == 16 && !s.samples[2].sprite);
    CHECK(s.displays[0].sprite == 31 && s.displays[0].x == 11 * 65536 + 0x1234 &&
          s.displays[0].y == 21 * 65536 + 0x4567 && s.displays[0].priority == 146 &&
          s.displays[0].red == 85 && s.displays[0].green == 85 && s.displays[0].blue == 245 &&
          s.displays[0].blend == 16 && s.displays[0].facing_left && s.displays[0].color_enabled);
    CHECK(SotnPlayerAfterimage_Present(&s, dra, 1, 148) && s.displays[0].sprite == 30 &&
          s.displays[0].blue == 242 && s.displays[0].x == 10 * 65536 + 0x1234);
    s.pose = 2;
    s.write_index = 4;
    s.timer = 99;
    Uint16 timer15 = 77;
    CHECK(SotnPlayerAfterimage_Reset(&s, false, 0, &timer15) && timer15 == 77 && s.disabled &&
          !s.reset && s.index == 10 && s.displays[0].sprite == 30);
    CHECK(SotnPlayerAfterimage_Reset(&s, true, 1, &timer15) && timer15 == 4 && s.reset &&
          s.pose == 2 && s.write_index == 4 && s.timer == 99 && s.samples[0].red == 105);
    for (Uint16 i = 0; i < 6; ++i)
        CHECK(!s.samples[i].sprite);
    for (Uint16 i = 0; i < 3; ++i)
        CHECK(!s.displays[i].sprite);
    const Sint32 durations[] = {-1, 3, 4, 17, 65536, SDL_MAX_SINT32};
    const Uint16 expected[] = {4, 4, 4, 17, 0, 65535};
    for (Uint16 i = 0; i < SDL_arraysize(durations); ++i)
        CHECK(SotnPlayerAfterimage_Reset(&s, false, durations[i], &timer15) &&
              timer15 == expected[i]);
    CHECK(SotnPlayerAfterimage_Force(&s) && !s.index && s.timer == 99 && s.disabled);
    CHECK(SotnPlayerAfterimage_Enable(&s) && !s.disabled && !s.reset);
    s.reset = true;
    for (Uint16 i = 0; i < 6; ++i)
        s.samples[i].sprite = 45;
    CHECK(SotnPlayerAfterimage_Present(&s, dra, 0, 148));
    CHECK(s.samples[0].sprite == 45 && !s.samples[1].sprite);
    CHECK(SotnPlayerAfterimage_Present(&s, dra, 1, 148) && !s.samples[0].sprite);
    return true;
}
static bool RebaseAndFailure(void)
{
    const SotnDataView dra = Image();
    SotnPlayerAfterimage s = {0};
    CHECK(SotnPlayerAfterimage_Init(&s));
    s.samples[0].x = -32768;
    s.samples[0].y = 32767;
    s.displays[0].x = 2147422772;  // 0x7fff1234.
    s.displays[0].y = -2147461512; // 0x80005678.
    const SotnPlayerAfterimage original = s;
    CHECK(SotnPlayerAfterimage_Rebase(&s, 1, -1));
    CHECK(
        ((Uint32)s.displays[0].x >> 16) == 0x8000 && ((Uint32)s.displays[0].x & 65535U) == 0x1234 &&
        ((Uint32)s.displays[0].y >> 16) == 0x7fff && ((Uint32)s.displays[0].y & 65535U) == 0x5678);
    CHECK(SotnPlayerAfterimage_Rebase(&s, -1, 1) && !SDL_memcmp(&s, &original, sizeof(s)));
    CHECK(!SotnPlayerAfterimage_Rebase(&s, 32768, 0) && !SDL_memcmp(&s, &original, sizeof(s)));
    CHECK(!SotnPlayerAfterimage_Capture(&s, (SotnDataView){0}, (SotnAfterimageInput){0}) &&
          !SDL_memcmp(&s, &original, sizeof(s)));
    CHECK(!SotnPlayerAfterimage_Present(&s, (SotnDataView){0}, 0, 148) &&
          !SDL_memcmp(&s, &original, sizeof(s)));
    CHECK(!SotnPlayerAfterimage_Present(&s, dra, 0, 1) && !SDL_memcmp(&s, &original, sizeof(s)));
    CHECK(!SotnPlayerAfterimage_Init(NULL) && !SotnPlayerAfterimage_Enable(NULL) &&
          !SotnPlayerAfterimage_Force(NULL));
    CHECK(!SotnPlayerAfterimage_Reset(&s, false, 1, NULL) && !SDL_memcmp(&s, &original, sizeof(s)));
    s.index = 11;
    const SotnPlayerAfterimage invalid = s;
    CHECK(!SotnPlayerAfterimage_Present(&s, dra, 0, 148) && !SDL_memcmp(&s, &invalid, sizeof(s)));
    return true;
}
int main(void)
{
    return Capture() && PresentAndReset() && RebaseAndFailure() ? 0 : 1;
}
