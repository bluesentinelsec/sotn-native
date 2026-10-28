#include <SDL3/SDL.h>
#include <sotn/player.h>
#include <sotn/wolf_motion.h>

#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

static bool Bounds(void)
{
    SotnWolfBodyPositions positions = {.player = {100, 100}};
    for (Uint16 i = 0; i < 14; ++i)
        positions.parts[i] = positions.player;
    SotnWolfCaptureBounds out = {0};
    CHECK(SotnWolf_CaptureBounds(&positions, &out));
    CHECK(out.right == 8 && out.bottom == 28 && out.left == -8 && out.top == -12);
    positions.parts[1] = (SotnWolfPoint){SDL_MIN_SINT16, SDL_MAX_SINT16};
    positions.parts[2] = (SotnWolfPoint){SDL_MAX_SINT16, SDL_MIN_SINT16};
    CHECK(SotnWolf_CaptureBounds(&positions, &out) && out.right == 8 && out.top == -12);
    positions.parts[4] = (SotnWolfPoint){200, 50};
    positions.parts[5] = (SotnWolfPoint){-200, 200};
    positions.parts[0].y = 130;
    CHECK(SotnWolf_CaptureBounds(&positions, &out));
    CHECK(out.right == 20 && out.bottom == 39 && out.left == -44 && out.top == -24);
    positions.facing_left = true;
    CHECK(SotnWolf_CaptureBounds(&positions, &out) && out.right == 44 && out.left == -20);
    // Original one-sided clamps do not normalize an arbitrary inverted supplied body.
    positions.player.x = SDL_MAX_SINT16;
    for (Uint16 i = 0; i < 14; ++i)
        positions.parts[i].x = SDL_MIN_SINT16;
    CHECK(SotnWolf_CaptureBounds(&positions, &out) && out.right == -65527 && out.left == -20);
    const SotnWolfCaptureBounds before = out;
    CHECK(!SotnWolf_CaptureBounds(NULL, &out) && !SDL_memcmp(&out, &before, sizeof(out)));
    CHECK(!SotnWolf_CaptureBounds(&positions, NULL));
    return true;
}
static bool Jump(void)
{
    for (Uint16 face = 0; face < 2; ++face)
        for (Uint16 flags = 0; flags < 16; ++flags)
        {
            SotnWolfJumpFrame f = {.velocity_x = 4 * 65536,
                                   .body_angle = 2048,
                                   .contacts = (Uint32)flags << 12,
                                   .substep = 2,
                                   .mode = 2,
                                   .buttons = SOTN_BUTTON_UP | SOTN_BUTTON_LEFT,
                                   .facing_left = face != 0};
            CHECK(SotnWolf_Jump(&f));
            const Sint32 expected = flags == (face ? 12 : 8)   ? -491520
                                    : flags == (face ? 8 : 12) ? -32768
                                                               : -229376;
            CHECK(f.velocity_y == expected && f.velocity_x == 4 * 65536 && f.substep == 4 &&
                  f.mode == 2 && f.animation.id == 0xe7 && !f.animation.pose &&
                  !f.animation.remaining && f.body_angle == 2176);
        }
    SotnWolfJumpFrame f = {.substep = 2, .mode = 0, .buttons = SOTN_BUTTON_RIGHT};
    CHECK(SotnWolf_Jump(&f) && f.mode == 0 && f.animation.id == 0xe6 && f.velocity_y == -229376);
    f = (SotnWolfJumpFrame){.substep = 1, .buttons = SOTN_BUTTON_RIGHT | SOTN_BUTTON_UP};
    CHECK(SotnWolf_Jump(&f) && f.mode == 1 && f.animation.id == 0xe7 && f.velocity_y == -319488);
    f = (SotnWolfJumpFrame){.substep = 9,
                            .mode = 2,
                            .buttons = SOTN_BUTTON_RIGHT | SOTN_BUTTON_UP,
                            .body_angle = SDL_MAX_SINT32 - 63};
    CHECK(SotnWolf_Jump(&f) && f.mode == 0 && f.animation.id == 0xe6 && f.velocity_y == -319488 &&
          f.body_angle == SDL_MIN_SINT32 + 64);
    f.mode = 5;
    const SotnWolfJumpFrame before = f;
    CHECK(!SotnWolf_Jump(&f) && !SDL_memcmp(&before, &f, sizeof(f)));
    CHECK(!SotnWolf_Jump(NULL));
    return true;
}
static bool Cadence(void)
{
    const Sint32 speeds[] = {0, 262143, 262144, 327679, 327680, 393215, 393216, 589824};
    const Uint16 durations[] = {4, 4, 3, 3, 2, 2, 1, 1};
    for (Uint16 i = 0; i < SDL_arraysize(speeds); ++i)
        for (Sint32 sign = -1; sign <= 1; sign += 2)
        {
            SotnWolfCadence out = {0};
            CHECK(SotnWolf_Cadence(speeds[i] * sign, 2048, &out) && out.body_angle == 2048);
            for (Uint16 j = 0; j < 8; ++j)
                CHECK(out.durations[j] == durations[i]);
        }
    SotnWolfCadence out = {0};
    CHECK(SotnWolf_Cadence(0, SDL_MIN_SINT32, &out) && out.body_angle == 1536);
    CHECK(SotnWolf_Cadence(0, SDL_MAX_SINT32, &out) && out.body_angle == 2560);
    const SotnWolfCadence before = out;
    CHECK(!SotnWolf_Cadence(SDL_MIN_SINT32, 0, &out) && !SDL_memcmp(&before, &out, sizeof(out)));
    CHECK(!SotnWolf_Cadence(0, 0, NULL));
    return true;
}
int main(void)
{
    return Bounds() && Jump() && Cadence() ? 0 : 1;
}
