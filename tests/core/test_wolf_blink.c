#include <SDL3/SDL.h>
#include <sotn/player_blink.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static SotnPlayerBlinkInput Input(SotnDataView dra, SotnDataView executable)
{
    return (SotnPlayerBlinkInput){
        .dra = dra,
        .executable = executable,
        .image = {32, 48, -10, -22},
        .x = 100,
        .y = 120,
        .priority = 144,
        .signal = 1,
        .wolf_form = true,
        .image_present = true,
        .wolf_bounds = {.left = -44, .right = 20, .top = -24, .bottom = 39}};
}
static bool Entry(SotnDataView dra, SotnDataView executable)
{
    SotnPlayerBlink e = {.parameter = 0x23};
    SotnPlayerBlinkShared s = {0};
    SotnPlayerBlinkInput in = Input(dra, executable);
    SotnPlayerBlinkEvents out = {0};
    CHECK(SotnPlayerBlink_Tick(&e, &s, in, &out) && e.phase == 12 && e.quads[0].priority == 148 &&
          e.quads[0].page == 0x18);
    for (Uint16 i = 0; i < 16; ++i)
        CHECK(SotnPlayerBlink_Tick(&e, &s, in, &out) && !out.signal && s.wolf_lock && !s.bat_lock);
    CHECK(e.amplitude == 256 && e.phase == 12);
    CHECK(SotnPlayerBlink_Tick(&e, &s, in, &out) && out.signal == 2 && e.phase == 13);
    in.signal = 2;
    in.x = 200;
    in.y = 210;
    in.rotation = 384;
    in.rotated = true;
    in.image = (SotnPlayerImage){0};
    CHECK(SotnPlayerBlink_Tick(&e, &s, in, &out));
    CHECK(e.quads[0].page == 0x118 && e.quads[0].u[0] == 0 && e.quads[0].u[1] == 64 &&
          e.quads[0].v[0] == 192 && e.quads[0].v[2] == 199 && e.quads[0].x[0] == 156 &&
          e.quads[0].x[1] == 220 && e.quads[0].y[0] == 186 && e.quads[0].y[2] == 193);
    for (Uint16 i = 0; i < 32; ++i)
        CHECK(SotnPlayerBlink_Tick(&e, &s, in, &out));
    CHECK(out.signal == 3 && e.parameter == 0x25 && e.phase == 1 && !s.wolf_lock &&
          e.quads[0].palette == 335);
    in.signal = 3;
    in.status = 0x40000000U;
    for (Uint16 i = 0; i < 26; ++i)
        CHECK(SotnPlayerBlink_Tick(&e, &s, in, &out));
    CHECK(e.phase == 2 && e.brightness == 256);
    CHECK(SotnPlayerBlink_Tick(&e, &s, in, &out) && e.phase == 2 && e.hold == 7);
    in.status = 0;
    CHECK(SotnPlayerBlink_Tick(&e, &s, in, &out) && e.phase == 3);
    for (Uint16 i = 0; i < 30 && !e.removed; ++i)
        CHECK(SotnPlayerBlink_Tick(&e, &s, in, &out));
    CHECK(e.removed);
    return true;
}
static bool ReturnAndCrossForm(SotnDataView dra, SotnDataView executable)
{
    SotnPlayerBlink e = {.parameter = 0x24};
    SotnPlayerBlinkShared s = {0};
    SotnPlayerBlinkInput in = Input(dra, executable);
    in.signal = 0;
    in.image = (SotnPlayerImage){0};
    SotnPlayerBlinkEvents out = {0};
    CHECK(SotnPlayerBlink_Tick(&e, &s, in, &out) && e.phase == 14 && e.quads[0].page == 0x118 &&
          e.quads[0].x[0] == 56);
    for (Uint16 i = 0; i < 20; ++i)
        CHECK(SotnPlayerBlink_Tick(&e, &s, in, &out) && !out.signal);
    CHECK(e.phase == 14 && e.amplitude == 256 && s.wolf_lock);
    in.wolf_form = false;
    in.signal = 1;
    in.image = (SotnPlayerImage){32, 48, -10, -22};
    CHECK(SotnPlayerBlink_Tick(&e, &s, in, &out) && e.phase == 15 && e.quads[0].page == 0x18);
    for (Uint16 i = 0; i < 33; ++i)
        CHECK(SotnPlayerBlink_Tick(&e, &s, in, &out));
    CHECK(out.signal == 3 && e.parameter == 0x1b && !s.wolf_lock && e.quads[0].palette == 0x15f);
    e = (SotnPlayerBlink){.parameter = 0x20};
    s = (SotnPlayerBlinkShared){0};
    in = Input(dra, executable);
    CHECK(SotnPlayerBlink_Tick(&e, &s, in, &out));
    for (Uint16 i = 0; i < 16; ++i)
        CHECK(SotnPlayerBlink_Tick(&e, &s, in, &out) && !out.signal);
    CHECK(e.amplitude == 256);
    CHECK(SotnPlayerBlink_Tick(&e, &s, in, &out) && out.signal == 2 && e.phase == 9);
    return true;
}
static bool LocksAndRawBounds(SotnDataView dra, SotnDataView executable)
{
    SotnPlayerBlink e = {.parameter = 0x23};
    SotnPlayerBlinkShared s = {.bat_lock = true, .wolf_lock = true};
    SotnPlayerBlinkInput in = Input(dra, executable);
    in.image_present = false;
    SotnPlayerBlinkEvents out = {0};
    CHECK(SotnPlayerBlink_Tick(&e, &s, in, &out) && e.removed && s.bat_lock && !s.wolf_lock);
    e = (SotnPlayerBlink){.parameter = 0x20};
    s = (SotnPlayerBlinkShared){.wolf_lock = true};
    in.image_present = true;
    CHECK(SotnPlayerBlink_Tick(&e, &s, in, &out) && e.removed && s.wolf_lock && !s.bat_lock);
    e = (SotnPlayerBlink){.parameter = 0x24};
    s = (SotnPlayerBlinkShared){0};
    in.wolf_bounds = (SotnWolfCaptureBounds){.left = 10, .right = -11, .top = 3, .bottom = -6};
    CHECK(SotnPlayerBlink_Tick(&e, &s, in, &out) && e.quads[0].u[2] == 54 &&
          e.quads[0].v[2] == 218);
    const SotnPlayerBlink before = e;
    const SotnPlayerBlinkShared shared = s;
    in.wolf_bounds.left = SDL_MIN_SINT32;
    CHECK(!SotnPlayerBlink_Tick(&e, &s, in, &out) && !SDL_memcmp(&e, &before, sizeof(e)) &&
          !SDL_memcmp(&s, &shared, sizeof(s)));
    return true;
}
int main(void)
{
    Uint8 *bytes = SDL_calloc(1, 0xe000), *sdk = SDL_calloc(1, 0x1e3ce);
    if (!bytes || !sdk)
    {
        SDL_free(bytes);
        SDL_free(sdk);
        return 1;
    }
    const Uint16 parameters[] = {0x20, 0x21, 0x23, 0x24, 0x25, 0x1b};
    for (Uint16 j = 0; j < SDL_arraysize(parameters); ++j)
    {
        Uint16 p = parameters[j];
        Uint8 *profile = bytes + 0xd670 + p * 20U;
        Put16(profile + 6, (Uint16)(300 + p));
        Put16(profile + 8, 256);
        Put16(profile + 10, 256);
        Put16(profile + 12, 256);
        Put16(profile + 14, p == 0x25 ? 0x700b : p == 0x1b ? 0x7008 : 16);
        Put16(profile + 16, 49);
        Put16(profile + 18, 512);
    }
    const SotnDataView dra = {bytes, 0xe000}, exe = {sdk, 0x1e3ce};
    const bool ok = Entry(dra, exe) && ReturnAndCrossForm(dra, exe) && LocksAndRawBounds(dra, exe);
    SDL_free(bytes);
    SDL_free(sdk);
    return ok ? 0 : 1;
}
