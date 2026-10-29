#include <SDL3/SDL.h>
#include <sotn/hellfire_beam.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static bool Lifecycle(void)
{
    for (Uint16 up = 0; up < 2; ++up)
    {
        SotnHellfireBeam b = {.x = 31 * 65536 + 123, .y = 45 * 65536 + 456};
        Uint16 uploads = 0, sounds = 0, signals = 0, factories = 0;
        for (Uint16 t = 0; t < 155; ++t)
        {
            SotnHellfireBeamEvents e = {0};
            CHECK(SotnHellfireBeam_Tick(&b,
                                        (SotnHellfireBeamInput){.casting = true,
                                                                .allocated = true,
                                                                .up = up != 0,
                                                                .player_x = 100,
                                                                .game_timer = SDL_MAX_UINT32 + t},
                                        &e));
            CHECK((Uint16)(Uint32)b.x == 123 && (Uint16)(Uint32)b.y == 456 &&
                  b.x == 100 * 65536 + 123);
            uploads += (Uint16)e.upload;
            sounds += (Uint16)(e.sound != 0);
            signals += (Uint16)e.signal_changed;
            if (e.sound)
                CHECK(e.sound == 0x636 && !t);
            if (e.upload)
                CHECK(!t && b.flags == 0x04860000U && b.quad.page == 0x11c &&
                      b.quad.priority == 448 && b.quad.draw_mode == 0x331 &&
                      b.quad.blue[3] == 128 && b.quad.red[0] == 0);
            if (e.signal_changed)
                CHECK((!t && e.signal == 0) || (t == 50 && e.signal == 1) ||
                      (t == 90 && e.signal == 2) || (t == 115 && e.signal == 3));
            if (e.factory)
            {
                ++factories;
                CHECK((t == 77 && e.factory == 38) || (t == 115 && e.factory == 0xa0004U) ||
                      (t == 147 && e.factory == (up ? 35U : 34U)));
            }
            CHECK(!b.removed || t == 154);
            if (t == 14)
                CHECK(b.phase == 3 && b.width == 24 && b.height == 128 && b.quad.blue[3] == 132);
            if (t == 46)
                CHECK(b.phase == 4 && e.palette == 0x810d);
            if (t == 50)
                CHECK(b.phase == 5 && b.width == -8 && (b.quad.draw_mode & 8U));
            if (t == 90)
                CHECK(b.timer == 2 && (b.quad.draw_mode & 8U));
            if (t == 92)
                CHECK(b.phase == 6 && b.width == 0 && !(b.quad.draw_mode & 8U));
            if (t == 98)
                CHECK(b.phase == 7 && b.timer == 96 && b.quad.blue[3] == 216 &&
                      e.palette == 0x8100);
            if (t == 104)
                CHECK(b.quad.blue[3] == 24);
            if (!b.removed)
            {
                CHECK(b.y == 120 * 65536 + 456 && b.quad.x[0] == 100 - b.width &&
                      b.quad.x[1] == 100 + b.width && b.quad.y[0] == 120 - b.height &&
                      b.quad.y[2] == 120 + b.height);
                for (Uint16 i = 0; i < 4; ++i)
                    CHECK(b.quad.v[i] == ((t & 1U) ? 252 : 248));
            }
        }
        CHECK(b.removed && uploads == 1 && sounds == 1 && signals == 4 && factories == 3);
    }
    return true;
}
static bool Boundaries(void)
{
    SotnHellfireBeam b = {.x = 123, .y = 456};
    SotnHellfireBeamEvents e = {0};
    CHECK(SotnHellfireBeam_Tick(&b, (SotnHellfireBeamInput){.casting = true, .player_x = -1}, &e) &&
          b.removed && e.signal_changed && e.signal == 0xffff && !e.upload && !e.sound &&
          !e.factory && b.x == -65536 + 123 && b.y == 456);
    b = (SotnHellfireBeam){.x = 123, .y = 456};
    CHECK(SotnHellfireBeam_Tick(&b, (SotnHellfireBeamInput){.allocated = true, .player_x = 123},
                                &e) &&
          b.removed && !e.signal_changed && !e.upload && b.x == 123 && b.y == 456);
    b = (SotnHellfireBeam){.x = 65535, .y = 65535, .phase = 1, .height = 32760, .width = 32760};
    CHECK(SotnHellfireBeam_Tick(&b, (SotnHellfireBeamInput){.casting = true, .player_x = 32767},
                                &e) &&
          b.phase == 1 && b.height == -32760 && b.x == SDL_MAX_SINT32 &&
          b.y == 120 * 65536 + 65535 && b.quad.x[1] == -9 && b.quad.x[0] == 7 &&
          b.quad.y[0] == -32656);
    b = (SotnHellfireBeam){.phase = 4, .width = SDL_MIN_SINT16};
    CHECK(SotnHellfireBeam_Tick(
              &b, (SotnHellfireBeamInput){.casting = true, .player_x = SDL_MIN_SINT16}, &e) &&
          b.width == 32760 && b.phase == 4 && !e.signal_changed && e.palette == 0x810d);
    b = (SotnHellfireBeam){.phase = 8};
    const SotnHellfireBeam saved = b;
    const SotnHellfireBeamEvents held = {.factory = 123};
    e = held;
    CHECK(!SotnHellfireBeam_Tick(&b, (SotnHellfireBeamInput){0}, &e) &&
          !SDL_memcmp(&b, &saved, sizeof(b)) && !SDL_memcmp(&e, &held, sizeof(e)));
    CHECK(!SotnHellfireBeam_Tick(NULL, (SotnHellfireBeamInput){0}, &e));
    b = (SotnHellfireBeam){.removed = true};
    CHECK(!SotnHellfireBeam_Tick(&b, (SotnHellfireBeamInput){0}, &e));
    return true;
}
int main(void)
{
    const bool ok = Lifecycle() && Boundaries();
    if (ok)
        SDL_Log("SOTN_HELLFIRE_BEAM_TEST: PASS");
    return ok ? 0 : 1;
}
