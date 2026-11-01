#include <SDL3/SDL.h>
#include <sotn/combat.h>
#include <sotn/fairy_effects.h>
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
static void Profile(Uint8 *data, Uint16 index, Uint16 count, Uint16 batch, Uint16 style)
{
    Uint8 *p = data + 0x24cc + index * 28U;
    Put16(p, count);
    Put16(p + 2, batch);
    Put16(p + 4, 1);
    Put16(p + 6, style);
    Put16(p + 8, 1);
    Put16(p + 10, 2);
    Put16(p + 12, 97);
    Put16(p + 14, 53);
    Put16(p + 16, 7);
    Put16(p + 18, 2);
    Put16(p + 20, 0x33);
    Put32(p + 24, 0x0c800000);
}
static bool Items(void)
{
    Uint8 data[0x2600] = {0};
    Profile(data, 0, 4, 1, 0);
    Profile(data, 1, 2, 2, 1);
    Profile(data, 2, 3, 3, 2);
    Profile(data, 3, 128, 1, 0);
    SotnFairyItemEffectInput in = {
        .overlay = {data, sizeof(data)},
        .source = {.x = 100 * 65536 + 0xaaaa, .y = 150 * 65536 + 0xbbbb, .facing_left = true},
        .player_priority = 65535};
    Uint16 rows = 123;
    CHECK(SotnFairyItemEffect_Rows(in.overlay, 0, &rows) && rows == 5 &&
          SotnFairyItemEffect_Rows(in.overlay, 3, &rows) && rows == 129);
    rows = 123;
    CHECK(!SotnFairyItemEffect_Rows(in.overlay, 11, &rows) && rows == 123);
    SotnFairyItemEffect e = {0};
    Uint32 rng = 5;
    CHECK(SotnFairyItemEffect_Tick(&e, in, &rng) && e.removed && !e.count && !e.phase && rng == 5);
    e = (SotnFairyItemEffect){0};
    in.allocated = true;
    Uint32 expected_rng = rng;
    for (Uint16 i = 0; i < 4; ++i)
        (void)SotnCombat_Random(&expected_rng);
    CHECK(SotnFairyItemEffect_Tick(&e, in, &rng) && rng == expected_rng && !e.removed &&
          e.count == 4 && e.phase == 1 && e.active == 1 && e.activation_timer == 1 &&
          e.flags == 0x0c800000 && e.priority == 1 && e.x == in.source.x && e.y == in.source.y &&
          !(e.tiles[0].draw_mode & 8U) && (e.tiles[1].draw_mode & 8U) &&
          e.tiles[0].velocity_y == -8192 && e.tiles[0].y == 124 * 65536 &&
          e.tiles[1].x == 96 * 65536 && e.tiles[1].y == 124 * 65536 && !e.tiles[4].width &&
          !e.tiles[4].screen_x && !e.tiles[4].screen_y && !(e.tiles[4].draw_mode & 8U));
    const SotnFairyItemEffect held = e;
    CHECK(SotnFairyItemEffect_Rebase(&e, 7, -3, false) && rng == expected_rng &&
          e.active == held.active && e.activation_timer == held.activation_timer &&
          e.tiles[0].x == held.tiles[0].x && e.tiles[0].y == held.tiles[0].y &&
          e.tiles[0].screen_x == held.tiles[0].screen_x + 7 &&
          e.tiles[0].screen_y == held.tiles[0].screen_y - 3 && e.tiles[4].screen_x == 7 &&
          e.tiles[4].screen_y == -3);
    CHECK(SotnFairyItemEffect_Tick(&e, in, &rng) && rng == expected_rng && e.active == 2 &&
          (Uint32)e.tiles[0].x ==
              (Uint32)held.tiles[0].x + 7U * 65536U + (Uint32)held.tiles[0].velocity_x &&
          !e.tiles[4].screen_x && !e.tiles[4].screen_y && !e.tiles[4].width);
    const SotnFairyItemEffect before_room = e;
    CHECK(SotnFairyItemEffect_Rebase(&e, 10, 20, true) &&
          e.tiles[0].screen_x == before_room.tiles[0].screen_x &&
          e.x == before_room.x + 10 * 65536 && e.y == before_room.y + 20 * 65536);
    for (Uint16 i = 0; i < 80 && !e.removed; ++i)
        CHECK(SotnFairyItemEffect_Tick(&e, in, &rng));
    CHECK(e.removed && !e.alive && rng == expected_rng);
    for (Uint16 parameter = 1; parameter <= 2; ++parameter)
    {
        e = (SotnFairyItemEffect){.parameter = parameter};
        expected_rng = rng;
        for (Uint16 i = 0; i < 6; ++i)
            (void)SotnCombat_Random(&expected_rng);
        Uint16 updates = 0;
        while (!e.removed && updates < 100)
        {
            CHECK(SotnFairyItemEffect_Tick(&e, in, &rng));
            ++updates;
            if (updates == 1)
                CHECK(rng == expected_rng && e.phase == 2 &&
                      e.tiles[0].timer == (parameter == 1 ? 44 : 39) &&
                      e.tiles[0].red == (parameter == 1 ? 97 : 96));
        }
        CHECK(e.removed && rng == expected_rng && updates == (parameter == 1 ? 46 : 41));
    }
    e = (SotnFairyItemEffect){.phase = 1, .count = 4, .active = 4};
    const SotnFairyItemEffect invalid = e;
    const Uint32 saved_rng = rng;
    CHECK(!SotnFairyItemEffect_Tick(&e, in, &rng) && rng == saved_rng &&
          !SDL_memcmp(&e, &invalid, sizeof(e)));
    e = (SotnFairyItemEffect){0};
    in.overlay.size = 1;
    const SotnFairyItemEffect untouched = e;
    CHECK(!SotnFairyItemEffect_Tick(&e, in, &rng) && rng == saved_rng &&
          !SDL_memcmp(&e, &untouched, sizeof(e)));
    e = (SotnFairyItemEffect){.phase = 1};
    const SotnFairyItemEffect no_rows = e;
    CHECK(!SotnFairyItemEffect_Rebase(&e, 1, 1, false) && !SDL_memcmp(&e, &no_rows, sizeof(e)));
    return true;
}
static bool Apple(void)
{
    Uint8 *boot = SDL_calloc(1, 0x1e400);
    CHECK(boot);
    Put16(boot + 0x1dbcc + 1024 * 2, 4096);
    SotnFairyAppleEffectInput in = {
        .source = {.x = 128 * 65536 + 0xaaaa, .y = 160 * 65536 + 0xbbbb}};
    SotnFairyAppleEffect e = {0};
    CHECK(SotnFairyAppleEffect_Tick(&e, in) && e.removed && !e.phase && !e.flags);
    e = (SotnFairyAppleEffect){.x = 123};
    in.allocated = true;
    const SotnFairyAppleEffect before = e;
    CHECK(!SotnFairyAppleEffect_Tick(&e, in) && !SDL_memcmp(&e, &before, sizeof(e)));
    in.executable = (SotnDataView){boot, 0x1e400};
    e = (SotnFairyAppleEffect){0};
    CHECK(SotnFairyAppleEffect_Tick(&e, in) && e.phase == 1 && e.width == 16 && !e.height &&
          e.x == in.source.x + 4 * 65536 && e.y == in.source.y && e.quads[0].x[0] == 116 &&
          e.quads[0].x[1] == 132 && e.quads[0].red[0] == 64 && e.quads[0].red[1] == 255 &&
          e.quads[2].x[2] == 228 && e.quads[6].y[2] == 96 && e.quads[0].draw_mode == 0x435 &&
          e.quads[9].priority == 448);
    const SotnFairyAppleEffect held = e;
    CHECK(SotnFairyAppleEffect_Rebase(&e, 7, -3, false) &&
          !SDL_memcmp(held.quads, e.quads, sizeof(e.quads)) && e.x == held.x + 7 * 65536 &&
          e.y == held.y - 3 * 65536 && e.width == held.width);
    CHECK(SotnFairyAppleEffect_Tick(&e, in) && e.quads[0].x[1] == 139 && e.height == 16 &&
          !SDL_memcmp(&e.quads[2].x, &held.quads[2].x, sizeof(e.quads[2].x)));
    Uint16 updates = 2;
    SotnPlayerEffectQuad final_quads[SOTN_FAIRY_APPLE_ROWS] = {0};
    while (!e.removed && updates < 300)
    {
        SDL_memcpy(final_quads, e.quads, sizeof(final_quads));
        CHECK(SotnFairyAppleEffect_Tick(&e, in));
        ++updates;
    }
    CHECK(updates == 186 && e.removed && !e.width && e.fan_opacity == 32 && e.opacity == 127 &&
          !SDL_memcmp(final_quads, e.quads, sizeof(final_quads)));
    e = (SotnFairyAppleEffect){0};
    in.source.x = SDL_MIN_SINT32 + 0x1234;
    in.source.facing_left = true;
    CHECK(SotnFairyAppleEffect_Tick(&e, in) && (Uint32)e.x == 0x7ffc1234U &&
          e.quads[0].x[1] == 32764 && e.quads[1].x[0] == 256);
    e = (SotnFairyAppleEffect){0};
    in.source.x = SDL_MAX_SINT32;
    in.source.facing_left = false;
    CHECK(SotnFairyAppleEffect_Tick(&e, in) && (Uint32)e.x == 0x8003ffffU && e.quads[0].x[0] == 0 &&
          e.quads[0].x[1] == -32765);
    SDL_free(boot);
    return true;
}
int main(void)
{
    return Items() && Apple() ? 0 : 1;
}
