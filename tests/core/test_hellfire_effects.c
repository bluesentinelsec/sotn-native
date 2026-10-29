#include "character_fixture.h"
#include <sotn/bat_runtime.h>
#include <sotn/hellfire_effects.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static bool Visuals(void)
{
    for (Uint16 face = 0; face < 2; ++face)
    {
        SotnHellfireEffect e = {.kind = 28, .x = 123, .y = 456};
        SotnHellfireEffectEvents out = {0};
        SotnHellfireEffectInput in = {.player_x = 128,
                                      .player_y = 100,
                                      .player_priority = 148,
                                      .facing_left = face != 0,
                                      .allocated = true};
        CHECK(SotnHellfireEffect_Tick(&e, in, &out) && e.phase == 1 && !e.removed &&
              !out.signal_changed && e.width == 22 && e.height == 26 &&
              e.x == (face ? 138 : 118) * 65536 + 123 && e.y == 102 * 65536 + 456 &&
              e.flags == 0x04840000U && e.quad.page == 0x1a && e.quad.palette == 0x15f &&
              e.quad.priority == 149 && e.quad.draw_mode == 0x35);
        in.player_priority = 200;
        for (Uint16 t = 1; t <= 17; ++t)
        {
            in.facing_left = !in.facing_left;
            in.player_x++;
            CHECK(SotnHellfireEffect_Tick(&e, in, &out) && !e.removed && e.width == 22 + t * 2 &&
                  e.height == 26 + t * 2 && e.quad.priority == 149 &&
                  e.x == (in.player_x + (in.facing_left ? 10 : -10)) * 65536 + 123);
            if (t == 9)
                CHECK(e.quad.red[3] == 128 && e.quad.blue[3] == 64);
            if (t == 10)
                CHECK(e.quad.red[3] == 116 && e.quad.blue[3] == 58);
        }
        CHECK(e.quad.red[3] == 32 && e.quad.blue[3] == 16);
        const SotnPlayerEffectQuad previous = e.quad;
        CHECK(SotnHellfireEffect_Tick(&e, in, &out) && e.removed && e.width == 58 &&
              !SDL_memcmp(&previous, &e.quad, sizeof(previous)));
    }
    SotnHellfireEffect e = {.kind = 29, .x = 123, .y = 456};
    SotnHellfireEffectEvents out = {0};
    SotnHellfireEffectInput in = {.player_x = 128, .casting = true, .allocated = true};
    CHECK(SotnHellfireEffect_Tick(&e, in, &out) && !out.signal_changed && e.flags == 0x04860000U &&
          e.quad.draw_mode == 0x315 && e.quad.red[0] == 192 && e.quad.green[3] == 64 &&
          e.x == 128 * 65536 + 123 && e.y == 456 && e.quad.x[0] == 125 && e.quad.x[1] == 131 &&
          e.quad.y[0] == 0 && e.quad.y[2] == 240);
    for (Uint16 t = 1; t <= 14; ++t)
    {
        in.game_timer = t;
        CHECK(SotnHellfireEffect_Tick(&e, in, &out) && !e.removed && e.timer == t &&
              ((e.quad.draw_mode & 8U) != 0) == ((t & 1U) != 0));
    }
    const SotnPlayerEffectQuad previous = e.quad;
    CHECK(SotnHellfireEffect_Tick(&e, in, &out) && e.removed && e.timer == 15 &&
          !SDL_memcmp(&previous, &e.quad, sizeof(previous)));
    e = (SotnHellfireEffect){.kind = 29};
    in.allocated = false;
    CHECK(SotnHellfireEffect_Tick(&e, in, &out) && e.removed && out.signal_changed &&
          out.signal == 0xffff);
    e = (SotnHellfireEffect){.kind = 28};
    CHECK(SotnHellfireEffect_Tick(&e, in, &out) && e.removed && !out.signal_changed);
    e = (SotnHellfireEffect){.kind = 29, .x = 123};
    in.casting = false;
    in.allocated = true;
    CHECK(SotnHellfireEffect_Tick(&e, in, &out) && e.removed && !out.signal_changed && e.x == 123);
    e = (SotnHellfireEffect){.kind = 28, .phase = 1, .width = 32767, .height = 32767};
    in.player_x = in.player_y = 32767;
    in.facing_left = false;
    CHECK(SotnHellfireEffect_Tick(&e, in, &out) && !e.removed && e.width == -32767 &&
          e.quad.x[0] == -12 && e.quad.x[1] == -10 && e.quad.y[0] == 0 && e.quad.y[2] == 2);
    e = (SotnHellfireEffect){.kind = 28, .phase = 2};
    const SotnHellfireEffect held = e;
    const SotnHellfireEffectEvents saved = {.signal = 17};
    out = saved;
    CHECK(!SotnHellfireEffect_Tick(&e, in, &out) && !SDL_memcmp(&e, &held, sizeof(e)) &&
          !SDL_memcmp(&out, &saved, sizeof(out)));
    return true;
}
static bool Trails(void)
{
    const Uint16 values[] = {0, 23, 32767};
    for (Uint16 i = 0; i < 3; ++i)
    {
        SotnHellfireTrail t = {.parameter = 0x1ff,
                               .x = 100 * 65536 + 123,
                               .y = 140 * 65536 + 456,
                               .velocity_x = 12345,
                               .facing_left = true,
                               .blend_mode = 0x70,
                               .draw_flags = 0xff};
        SotnHellfireTrailInput in = {
            .player_priority = 148, .random = values[i], .random_ready = true};
        SotnHellfireTrailEvents out = {0};
        CHECK(SotnHellfireTrail_Tick(&t, in, &out) && out.initialize && out.random_used &&
              out.animation_offset == 0xd5fc && t.bank == 2 && t.priority == 150 &&
              t.palette == 0x8170 && t.flags == 0x08100000U && t.scale_x == 128 &&
              t.scale_y == 128 && t.draw_flags == 3 && t.blend_mode == 0x70 && t.facing_left &&
              t.x == 100 * 65536 + 123 &&
              t.y == (140 + (Sint32)(values[i] % 24U) - 12) * 65536 + 456 &&
              t.velocity_y == -65536);
        const Sint32 x = t.x, y = t.y;
        in.random_ready = false;
        in.random = 65535;
        CHECK(SotnHellfireTrail_Tick(&t, in, &out) && !out.initialize && !out.random_used &&
              t.x == x + 12345 && t.y == y - 65536);
        const Sint32 last_x = t.x, last_y = t.y;
        in.remaining = -1;
        CHECK(SotnHellfireTrail_Tick(&t, in, &out) && t.removed && t.x == last_x && t.y == last_y);
    }
    SotnHellfireTrail t = {.parameter = 0x100};
    const SotnHellfireTrail held = t;
    SotnHellfireTrailEvents out = {.animation_offset = 42}, saved = out;
    CHECK(!SotnHellfireTrail_Tick(&t, (SotnHellfireTrailInput){0}, &out) &&
          !SDL_memcmp(&t, &held, sizeof(t)) && !SDL_memcmp(&out, &saved, sizeof(out)));
    t.parameter = 0;
    CHECK(!SotnHellfireTrail_Tick(&t, (SotnHellfireTrailInput){.random_ready = true}, &out));
    return true;
}
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
static void Profile(Uint8 *d, Uint16 kind, Uint32 flags)
{
    Put32(d + 0xdb98 + kind * 4U, 0x800a0100);
    Uint8 *p = d + 0x100;
    p[0] = 3;
    p[1] = 60;
    p[2] = 50;
    p[3] = 40;
    p[4] = p[5] = 2;
    Put16(p + 6, 2);
    Put16(p + 8, 0x33);
    Put16(p + 10, kind);
    Put32(p + 12, flags);
}
static bool Particles(Uint8 *d)
{
    Profile(d, 10, 0x08800000U);
    const SotnDataView view = {d, 0x11000};
    Uint16 rows = 0;
    CHECK(SotnPlayerParticles_Rows(view, 0xa00, &rows) && rows == 4);
    SotnPlayerParticles e = {0};
    Uint32 rng = 0x12345678;
    SotnPlayerParticlesInput in = {.parameter = 0xa00,
                                   .source = {.x = -99 * 65536, .y = -99 * 65536},
                                   .player_x = 128,
                                   .player_y = 100,
                                   .player_priority = 148,
                                   .player_step = 34,
                                   .allocated = true};
    CHECK(SotnPlayerParticles_Tick(&e, view, in, &rng) && rng == 0x12345678 && e.lifetime == 63 &&
          e.tiles[0].x == 128 * 65536 && e.tiles[0].y == 100 * 65536 &&
          e.tiles[0].velocity_y == -16384 && e.tiles[2].velocity_y == -32768 &&
          e.tiles[2].timer == 63);
    in.player_x = 200;
    in.player_y = 200;
    for (Uint16 t = 1; t <= 31; ++t)
        CHECK(SotnPlayerParticles_Tick(&e, view, in, &rng) && !e.removed && rng == 0x12345678 &&
              e.lifetime == 63 - t * 2 && e.tiles[0].x == 128 * 65536 && e.tiles[0].red == 60 &&
              e.tiles[0].y == 100 * 65536 - (Sint32)t * 16384);
    CHECK(SotnPlayerParticles_Tick(&e, view, in, &rng) && e.removed && !e.lifetime);
    e = (SotnPlayerParticles){0};
    in.player_step = 0;
    CHECK(SotnPlayerParticles_Tick(&e, view, in, &rng) && !e.removed && e.lifetime == 63);
    CHECK(SotnPlayerParticles_Tick(&e, view, in, &rng) && e.removed && e.lifetime == 63);
    e = (SotnPlayerParticles){0};
    in.player_step = 34;
    CHECK(SotnPlayerParticles_Tick(&e, view, in, &rng));
    e.tiles[0].timer = 0;
    CHECK(SotnPlayerParticles_Tick(&e, view, in, &rng) && e.tiles[0].timer == 65535 &&
          (e.tiles[0].draw_mode & 8U));
    e = (SotnPlayerParticles){0};
    in.allocated = false;
    CHECK(SotnPlayerParticles_Tick(&e, view, in, &rng) && e.removed && rng == 0x12345678);
    return true;
}
static bool Culling(Uint8 *d)
{
    TestCharacter_Fill(d);
    Profile(d, 1, 0x0c800000U);
    SotnCharacter *c = NULL;
    CHECK(SotnCharacter_Create((SotnDataView){d, 0x11000}, &c));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    CHECK(r != NULL);
    r->children[47] = (SotnBatRuntimeChild){.kind = 5,
                                            .generation = 1,
                                            .parameter = 0x100,
                                            .source = {.x = 400 * 65536, .y = 100 * 65536},
                                            .state.particles = {.phase = 0}};
    r->serial = 1;
    SotnBatRuntimeInput in = {.dra = {d, 0x11000}, .player = {.priority = 148}};
    CHECK(SotnBatRuntime_Tick(r, c, in) && r->children[47].kind == 5 &&
          r->children[47].initialized && r->children[47].primitive_count == 4);
    CHECK(SotnBatRuntime_Tick(r, c, in) && r->children[47].kind == 5);
    SDL_zero(*r);
    Profile(d, 1, 0x08800000U);
    r->children[47] = (SotnBatRuntimeChild){.kind = 5,
                                            .generation = 1,
                                            .parameter = 0x100,
                                            .source = {.x = 400 * 65536, .y = 100 * 65536},
                                            .state.particles = {.phase = 0}};
    r->serial = 1;
    SotnCharacterState before = {0}, after = {0};
    CHECK(SotnCharacter_Get(c, &before, NULL));
    CHECK(SotnBatRuntime_Tick(r, c, in) && !r->children[47].kind &&
          SotnCharacter_Get(c, &after, NULL) && after.random != before.random);
    SDL_free(r);
    SotnCharacter_Destroy(c);
    return true;
}
int main(void)
{
    Uint8 *d = SDL_calloc(1, 0x11000);
    if (!d)
        return 1;
    const bool ok = Visuals() && Trails() && Particles(d) && Culling(d);
    SDL_free(d);
    if (ok)
        SDL_Log("SOTN_HELLFIRE_EFFECTS_TEST: PASS");
    return ok ? 0 : 1;
}
