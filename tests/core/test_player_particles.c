#include <SDL3/SDL.h>
#include <sotn/collision.h>
#include <sotn/combat.h>
#include <sotn/player_particles.h>
#include <sotn/player_smoke.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 n)
{
    p[0] = (Uint8)n;
    p[1] = (Uint8)(n >> 8);
}
static void Put32(Uint8 *p, Uint32 n)
{
    Put16(p, (Uint16)n);
    Put16(p + 2, (Uint16)(n >> 16));
}
static bool Run(void)
{
    Uint8 dra[0xdbd4] = {0};
    const Uint16 kinds[] = {1, 9, 13};
    for (Uint16 i = 0; i < 3; ++i)
    {
        Put32(dra + 0xdb98 + kinds[i] * 4U, 0x800a0100U + i * 16U);
        Uint8 *p = dra + 0x100 + i * 16U;
        p[0] = 3;
        p[1] = 40;
        p[2] = 30;
        p[3] = 20;
        p[4] = p[5] = 2;
        Put16(p + 6, 2);
        Put16(p + 8, i == 2 ? 8 : 0);
        Put16(p + 10, kinds[i]);
        Put32(p + 12, 0x12345678);
    }
    const SotnDataView view = {dra, sizeof(dra)};
    for (Uint16 i = 0; i < 3; ++i)
    {
        SotnPlayerParticles effect = {0};
        Uint32 random = 10;
        Uint16 rows = 0;
        SotnPlayerParticlesInput input = {.parameter = (Uint16)(kinds[i] << 8),
                                          .source = {.x = -65536 + 123, .y = 100 * 65536 + 456},
                                          .player_y = 100,
                                          .player_priority = 148,
                                          .water_level = 40};
        CHECK(SotnPlayerParticles_Rows(view, input.parameter, &rows) && rows == 4);
        CHECK(SotnPlayerParticles_Tick(&effect, view, input, &random) && effect.removed &&
              random == 10 && !effect.phase && !effect.count);
        effect = (SotnPlayerParticles){0};
        input.allocated = true;
        CHECK(SotnPlayerParticles_Tick(&effect, view, input, &random) && !effect.removed &&
              effect.phase == 1 && effect.count == 3 && effect.priority == 150 &&
              effect.flags == 0x12345678 && effect.tiles[0].x == -65536 &&
              effect.tiles[0].y == 100 * 65536 && random != 10);
        const Uint32 retained = random;
        const SotnPlayerParticles before = effect;
        CHECK(SotnPlayerParticles_Rebase(&effect, -17, 9) &&
              effect.tiles[0].x == before.tiles[0].x - 17 * 65536 &&
              effect.tiles[0].y == before.tiles[0].y + 9 * 65536 &&
              effect.lifetime == before.lifetime);
        CHECK(SotnPlayerParticles_Rebase(&effect, 17, -9) &&
              !SDL_memcmp(&before, &effect, sizeof(effect)));
        input.parameter = 0;
        CHECK(!SotnPlayerParticles_Tick(&effect, view, input, &random) &&
              !SDL_memcmp(&before, &effect, sizeof(effect)) && random == retained);
        input.parameter = (Uint16)(kinds[i] << 8);
        CHECK(SotnPlayerParticles_Tick(&effect, view, input, &random) && random == retained);
        if (kinds[i] == 13)
        {
            CHECK(effect.lifetime == 254 && effect.tiles[0].red == 39 &&
                  effect.tiles[1].red == 40 && effect.tiles[1].timer == 3 &&
                  !(effect.tiles[0].draw_mode & 8));
            input.water_level = 0;
            CHECK(SotnPlayerParticles_Tick(&effect, view, input, &random) &&
                  (effect.tiles[0].draw_mode & 8));
            for (Uint16 tick = 0; tick < 126; ++tick)
                CHECK(SotnPlayerParticles_Tick(&effect, view, input, &random));
            CHECK(effect.removed && effect.lifetime == 0);
        }
        else
        {
            CHECK(effect.tiles[0].red == 37 &&
                  effect.tiles[0].velocity_y == before.tiles[0].velocity_y + 10240);
            for (Uint16 tick = 0; tick < 10; ++tick)
                CHECK(SotnPlayerParticles_Tick(&effect, view, input, &random));
            CHECK(effect.removed);
        }
    }
    Uint16 unchanged = 90;
    dra[0x100] = 31;
    CHECK(!SotnPlayerParticles_Rows(view, 0x100, &unchanged) && unchanged == 90);
    Put32(dra + 0xdb98 + 4, 0x800adbd0);
    CHECK(!SotnPlayerParticles_Rows(view, 0x100, &unchanged) && unchanged == 90);
    return true;
}
static bool VectorParticles(Uint16 kind)
{
    Uint8 dra[0xdbd4] = {0};
    Put32(dra + 0xdb98 + kind * 4U, 0x800a0100);
    Uint8 *profile = dra + 0x100;
    profile[0] = 2;
    profile[1] = 63;
    profile[2] = 64;
    profile[3] = 65;
    profile[4] = profile[5] = 3;
    Put16(profile + 6, 2);
    Put16(profile + 8, 49);
    Put16(profile + 10, 4);
    SotnPlayerParticles p = {0};
    SotnPlayerParticleVectors vectors = {.initialized = true};
    for (Uint16 i = 0; i < 32; ++i)
    {
        vectors.x[i] = 65;
        vectors.y[i] = -33;
    }
    const SotnDataView view = {dra, sizeof(dra)};
    SotnPlayerParticlesInput in = {.parameter = (Uint16)(kind << 8 | 0x37U),
                                   .source = {.x = -65536 + 123, .y = 100 * 65536 + 456},
                                   .player_priority = 148};
    Uint32 rng = 17;
    Uint16 rows = 0;
    CHECK(SotnPlayerParticles_Rows(view, in.parameter, &rows) && rows == 3);
    CHECK(SotnPlayerParticles_Tick(&p, view, in, &rng) && p.removed && rng == 17);
    p = (SotnPlayerParticles){0};
    in.allocated = true;
    CHECK(!SotnPlayerParticles_Tick(&p, view, in, &rng) && !p.phase && !p.count && rng == 17);
    in.vectors = &vectors;
    CHECK(SotnPlayerParticles_Tick(&p, view, in, &rng) && p.phase == 1 && p.lifetime == 47 &&
          p.count == 2 && p.tiles[0].velocity_x == 65 && p.tiles[0].velocity_y == -33 &&
          !(p.tiles[0].x & 65535) && !(p.tiles[0].y & 65535));
    Uint32 expected = 17;
    for (Uint16 i = 0; i < 6; ++i)
        SotnCombat_Random(&expected);
    CHECK(rng == expected);
    const SotnPlayerParticles live = p;
    p.lifetime = 48;
    const SotnPlayerParticles invalid = p;
    CHECK(!SotnPlayerParticles_Tick(&p, view, in, &rng) && !SDL_memcmp(&p, &invalid, sizeof(p)) &&
          rng == expected);
    p = live;
    const Sint32 x = p.tiles[0].x, y = p.tiles[0].y;
    in.vectors = NULL;
    CHECK(SotnPlayerParticles_Tick(&p, view, in, &rng) && p.tiles[0].x == x + 65 &&
          p.tiles[0].y == y - 33 && p.tiles[0].velocity_y == -31 && p.tiles[0].red == 63);
    while (p.lifetime > 40)
        CHECK(SotnPlayerParticles_Tick(&p, view, in, &rng));
    CHECK(p.tiles[0].velocity_x == 32 && p.tiles[0].velocity_y == -7 && p.tiles[0].width == 3);
    while (p.lifetime > 24)
        CHECK(SotnPlayerParticles_Tick(&p, view, in, &rng));
    CHECK(p.tiles[0].draw_mode == 2 && p.tiles[0].width == 3);
    while (p.lifetime > 8)
        CHECK(SotnPlayerParticles_Tick(&p, view, in, &rng));
    CHECK(p.tiles[0].width == 2 && p.tiles[0].height == 2);
    while (p.lifetime > 3)
        CHECK(SotnPlayerParticles_Tick(&p, view, in, &rng));
    CHECK(p.tiles[0].width == 1 && p.tiles[0].height == 1 && p.tiles[0].red == 63);
    while (!p.removed)
        CHECK(SotnPlayerParticles_Tick(&p, view, in, &rng));
    CHECK(!p.lifetime && rng == expected);
    return true;
}
static bool StoneParticles(void)
{
    Uint8 dra[0xdbd4] = {0};
    const SotnDataView view = {dra, sizeof(dra)};
    for (Uint16 kind = 3; kind <= 8; kind += 5)
    {
        Put32(dra + 0xdb98 + kind * 4U, 0x800a0100);
        Uint8 *profile = dra + 0x100;
        profile[0] = 1;
        profile[1] = profile[2] = profile[3] = 63;
        profile[4] = profile[5] = 3;
        Put16(profile + 6, 2);
        Put16(profile + 8, 49);
        Put16(profile + 10, kind == 3 ? 2 : 8);
        SotnPlayerParticles p = {0};
        SotnPlayerParticlesInput in = {.parameter = (Uint16)(kind << 8),
                                       .source = {.x = 100 * 65536 + 123, .y = 140 * 65536 + 456},
                                       .player_priority = 148};
        Uint32 rng = 17, expected = 17;
        CHECK(SotnPlayerParticles_Tick(&p, view, in, &rng) && p.removed && rng == 17);
        p = (SotnPlayerParticles){0};
        in.allocated = true;
        CHECK(SotnPlayerParticles_Tick(&p, view, in, &rng));
        for (Uint16 i = 0; i < 4; ++i)
            SotnCombat_Random(&expected);
        CHECK(rng == expected && p.lifetime == (kind == 3 ? 47 : 63) &&
              p.tiles[0].x == (kind == 3 ? 6700337 : 6848388) &&
              p.tiles[0].y == (kind == 3 ? 142 : 111) * 65536 && p.tiles[0].velocity_x == 7723 &&
              p.tiles[0].velocity_y == -168 && p.tiles[0].timer == (kind == 3 ? 19 : 35));
        p.tiles[0].timer = 1;
        CHECK(SotnPlayerParticles_Tick(&p, view, in, &rng) && p.lifetime == (kind == 3 ? 45 : 61) &&
              !p.tiles[0].timer && (p.tiles[0].draw_mode & 8U) && p.tiles[0].velocity_y == 4952);
        const Uint16 updates = kind == 3 ? 23 : 31;
        for (Uint16 i = 1; i < updates; ++i)
            CHECK(SotnPlayerParticles_Tick(&p, view, in, &rng) && !p.removed);
        CHECK(p.lifetime == 1 && p.tiles[0].timer == 65537U - updates && p.tiles[0].red == 63 &&
              SotnPlayerParticles_Tick(&p, view, in, &rng) && p.removed && !p.lifetime &&
              rng == expected);
    }
    return true;
}
static bool Vectors(void)
{
    Uint8 *exe = SDL_calloc(1, 0x24000);
    CHECK(exe);
    for (Uint16 i = 0; i < 1025; ++i)
        Put16(exe + 0x1dbcc + i * 2U, (Uint16)(i * 4U));
    SotnPlayerParticleVectors vectors = {0};
    Uint32 rng = 17, expected = 17;
    for (Uint16 i = 0; i < 64; ++i)
        SotnCombat_Random(&expected);
    CHECK(SotnPlayerParticleVectors_Init(&vectors, (SotnDataView){exe, 0x24000}, &rng) &&
          vectors.initialized && rng == expected);
    for (Uint16 i = 0; i < 32; ++i)
        CHECK(vectors.x[i] >= -131072 && vectors.x[i] <= 131072 && vectors.y[i] < 0 &&
              vectors.y[i] >= -262144);
    const SotnPlayerParticleVectors before = vectors;
    CHECK(!SotnPlayerParticleVectors_Init(&vectors, (SotnDataView){0}, &rng) &&
          !SDL_memcmp(&vectors, &before, sizeof(vectors)) && rng == expected);
    SDL_free(exe);
    return true;
}
static bool Smoke(void)
{
    Uint8 dra[0xd5e0] = {0};
    for (Uint16 i = 0; i < 6; ++i)
    {
        Put16(dra + 0xd54c + i * 2U, (Uint16)(0U - i * 3U));
        Put32(dra + 0xd558 + i * 4U, (Uint32)(0U - (i + 1U) * 4096U));
        Put16(dra + 0xd570 + i * 2U, (Uint16)(80 + i * 16U));
    }
    for (Uint16 i = 0; i < 24; ++i)
    {
        Put16(dra + 0xd57c + i * 4U, 1);
        Put16(dra + 0xd57e + i * 4U, (Uint16)(i + 1U));
    }
    Put16(dra + 0xd5dc, 65535);
    const SotnDataView view = {dra, sizeof(dra)};
    const SotnPlayerFactorySource source = {
        .x = 64 * 65536 + 123, .y = 100 * 65536 + 456, .facing_left = true};
    SotnPlayerSmoke puff = {0};
    CHECK(SotnPlayerSmoke_Tick(&puff, view, source, 0x102, 148, 0x8000, 0));
    CHECK(puff.x == 71 * 65536 + 123 && puff.y == 124 * 65536 + 456 && puff.velocity_x == 12288 &&
          puff.velocity_y == -16384 && puff.scale == 160 && puff.sprite == 1 &&
          puff.remaining == 1 && puff.priority == 150);
    const SotnPlayerSmoke before = puff;
    CHECK(SotnPlayerSmoke_Rebase(&puff, -17, 9) && puff.x == before.x - 17 * 65536 &&
          puff.y == before.y + 9 * 65536 && puff.remaining == before.remaining);
    CHECK(SotnPlayerSmoke_Rebase(&puff, 17, -9) && !SDL_memcmp(&before, &puff, sizeof(puff)));
    CHECK(!SotnPlayerSmoke_Tick(&puff, view, source, 0x101, 148, 0, 0) &&
          !SDL_memcmp(&before, &puff, sizeof(puff)));
    CHECK(SotnPlayerSmoke_Tick(&puff, view, source, 0x102, 148, 0, 0) &&
          puff.x == before.x + 12288 && puff.y == before.y - 16384 && puff.sprite == 2);
    for (Uint16 i = 0; i < 23; ++i)
        CHECK(SotnPlayerSmoke_Tick(&puff, view, source, 0x102, 148, 0, 0));
    CHECK(!puff.removed && puff.sprite == 24 && puff.remaining == -1);
    CHECK(SotnPlayerSmoke_Tick(&puff, view, source, 0x102, 148, 0, 0) && puff.removed);
    puff = (SotnPlayerSmoke){0};
    CHECK(!SotnPlayerSmoke_Tick(&puff, view, source, 0, 148, 0, 0x20000) && !puff.phase);
    for (Uint16 i = 0; i < 3; ++i)
    {
        puff = (SotnPlayerSmoke){0};
        CHECK(SotnPlayerSmoke_Tick(&puff, view, source, (Uint16)(0x500U + i), 148, 0, 0));
        CHECK(puff.x == 70 * 65536 + 123 && puff.y == 124 * 65536 + 456 && !puff.velocity_x &&
              puff.velocity_y == -(Sint32)(5U - i * 2U) * 4096 && puff.scale == 144 + i * 16U);
    }
    puff = (SotnPlayerSmoke){0};
    CHECK(!SotnPlayerSmoke_Tick(&puff, view, source, 0x503, 148, 0, 0) && !puff.phase);
    Put16(dra + 0xd57c, 0);
    CHECK(!SotnPlayerSmoke_Tick(&puff, view, source, 0, 148, 0, 0) && !puff.phase);
    return true;
}
static bool ScatteredSmoke(void)
{
    Uint8 dra[0xd5e0] = {0};
    for (Uint16 i = 0; i < 6; ++i)
    {
        Put32(dra + 0xd558 + i * 4U, (Uint32)(0U - (i + 1U) * 4096U));
        Put16(dra + 0xd570 + i * 2U, (Uint16)(80 + i * 16U));
    }
    Put16(dra + 0xd57c, 2);
    Put16(dra + 0xd57e, 1);
    const SotnDataView view = {dra, sizeof(dra)};
    for (Uint16 high = 3; high <= 7; high += 4)
        for (Uint16 face = 0; face < 2; ++face)
        {
            const SotnPlayerFactorySource source = {
                .x = 64 * 65536 + 123, .y = 100 * 65536 + 456, .facing_left = face != 0};
            const Uint16 parameter = (Uint16)(high << 8 | 23U);
            SotnPlayerSmoke puff = {0};
            Uint32 rng = 17, expected = 17;
            CHECK(!SotnPlayerSmoke_TickOwned(&puff, view, source, parameter, 148, 0, 0, NULL));
            CHECK(!SotnPlayerSmoke_TickShared(&puff, view, source, parameter, 148, 0, 0x20000, NULL,
                                              &rng) &&
                  rng == 17 && !puff.phase);
            CHECK(
                SotnPlayerSmoke_TickShared(&puff, view, source, parameter, 148, 0, 0, NULL, &rng));
            const Sint32 offset = high == 3 ? 3 : 17;
            for (Uint16 i = 0; i < 3; ++i)
                SotnCombat_Random(&expected);
            CHECK(rng == expected && puff.x == (64 + (face ? -offset : offset)) * 65536 + 123 &&
                  puff.y == 123 * 65536 + 456 && puff.velocity_y == -12288 && puff.scale == 176 &&
                  puff.parameter == parameter && puff.remaining == 2);
            CHECK(
                SotnPlayerSmoke_TickShared(&puff, view, source, parameter, 148, 0, 0, NULL, &rng) &&
                rng == expected && puff.remaining == 1);
            puff = (SotnPlayerSmoke){0};
            Put16(dra + 0xd57c, 0);
            rng = 17;
            CHECK(!SotnPlayerSmoke_TickShared(&puff, view, source, parameter, 148, 0, 0, NULL,
                                              &rng) &&
                  rng == 17 && !puff.phase);
            Put16(dra + 0xd57c, 2);
        }
    return true;
}
static bool HurtSmoke(void)
{
    Uint8 dra[0xd5fa] = {0};
    for (Uint16 i = 0; i < 6; ++i)
    {
        Put16(dra + 0xd54c + i * 2U, (Uint16)(i * 3U));
        Put32(dra + 0xd558 + i * 4U, (Uint32)(0U - (i + 1U) * 4096U));
        Put16(dra + 0xd570 + i * 2U, (Uint16)(80 + i * 16U));
    }
    Put16(dra + 0xd57c, 2);
    Put16(dra + 0xd57e, 1);
    for (Uint16 i = 0; i < 14; ++i)
        dra[0xd5e0 + i] = (Uint8)(13U - i);
    for (Uint16 i = 0; i < 10; ++i)
        dra[0xd5f0 + i] = (Uint8)i;
    const SotnDataView view = {dra, sizeof(dra)};
    const SotnPlayerFactorySource source = {
        .x = 64 * 65536 + 123, .y = 100 * 65536 + 456, .facing_left = true};
    SotnPlayerSmokeEnvironment live = {
        .x = 32767 * 65536 + 999, .y = -32768 * 65536 + 888, .wall_owned = true};
    for (Sint32 speed = -1; speed <= 1; ++speed)
    {
        live.velocity_x = speed;
        SotnPlayerSmoke puff = {0};
        CHECK(SotnPlayerSmoke_TickOwned(&puff, view, source, 0x602, 148, 0, 0, &live));
        CHECK(puff.x == (speed > 0 ? 88 : 40) * 65536 + 123 && puff.y == 124 * 65536 + 456 &&
              puff.velocity_y == -12288 && puff.scale == 176 && !puff.velocity_x);
    }
    SotnPlayerSmoke puff = {0};
    CHECK(!SotnPlayerSmoke_Tick(&puff, view, source, 0x600, 148, 0, 0) && !puff.phase);
    CHECK(SotnPlayerSmoke_Tick(&puff, view, source, 0xa02, 148, 0, 0) &&
          puff.x == 60 * 65536 + 123 && puff.y == 118 * 65536 + 456);
    for (Uint16 profile = 4; profile <= 8; profile += 4)
    {
        const Uint16 parameter = (Uint16)(profile * 256U + 1U);
        live.wall[9].effects = SOTN_EFFECT_SIDE;
        live.wall[9].x = 7;
        live.wall[9].y = -13;
        live.wall[0].effects = SOTN_EFFECT_SOLID; // Profile8 starts scanning at index2.
        puff = (SotnPlayerSmoke){0};
        live.wall_owned = false;
        CHECK(!SotnPlayerSmoke_TickOwned(&puff, view, source, parameter, 148, 0, 0, &live) &&
              !puff.phase);
        live.wall_owned = true;
        CHECK(SotnPlayerSmoke_TickOwned(&puff, view, source, parameter, 148, 0, 0, &live) &&
              puff.x == -32762 * 65536 + 123 && puff.y == 32755 * 65536 + 456 &&
              puff.velocity_y == (profile == 4 ? -16384 : -8192) &&
              puff.scale == (profile == 4 ? 160 : 128) && puff.remaining == 2);
        const SotnPlayerSmoke before = puff;
        CHECK(SotnPlayerSmoke_TickOwned(&puff, view, source, parameter, 148, 0, 0, NULL) &&
              puff.x == before.x && puff.y == before.y + before.velocity_y && puff.remaining == 1);
        live.wall[9].effects = 0;
        puff = (SotnPlayerSmoke){0};
        if (profile == 4)
            live.wall[0].effects = 0;
        CHECK(SotnPlayerSmoke_TickOwned(&puff, view, source, parameter, 148, 0, 0, &live) &&
              puff.removed && !puff.sprite);
        puff = (SotnPlayerSmoke){0};
        const Uint32 table = profile == 4 ? 0xd5e0U : 0xd5f0U;
        const Uint8 saved = dra[table];
        dra[table] = 14;
        CHECK(!SotnPlayerSmoke_TickOwned(&puff, view, source, parameter, 148, 0, 0, &live) &&
              !puff.phase && !puff.removed);
        dra[table] = saved;
        CHECK(!SotnPlayerSmoke_TickOwned(&puff, (SotnDataView){dra, table}, source, parameter, 148,
                                         0, 0, &live) &&
              !puff.phase);
    }
    return true;
}
static bool SireParticles(void)
{
    Uint8 dra[0xdbd4] = {0};
    Put32(dra + 0xdb98 + 12U * 4U, 0x800a0100);
    Uint8 *profile = dra + 0x100;
    profile[0] = 22; // Authored count exercises both sides of the original index10 split.
    profile[1] = 64;
    profile[2] = 96;
    profile[3] = 128;
    profile[4] = 3;
    profile[5] = 4;
    Put16(profile + 6, 2);
    Put16(profile + 10, 12);
    Put32(profile + 12, 0x08000000);
    const SotnDataView view = {dra, sizeof(dra)};
    SotnPlayerParticles effect = {0};
    SotnPlayerParticlesInput input = {.parameter = 12U << 8,
                                      .source = {.x = -17 * 65536 + 123, .y = 40 * 65536 + 456},
                                      .player_x = 250,
                                      .player_y = -70,
                                      .player_priority = 148};
    Uint32 random = 0x12345678;
    Uint16 rows = 0;
    CHECK(SotnPlayerParticles_Rows(view, input.parameter, &rows) && rows == 23);
    CHECK(SotnPlayerParticles_Tick(&effect, view, input, &random) && effect.removed &&
          !effect.count && random == 0x12345678);
    effect = (SotnPlayerParticles){0};
    input.allocated = true;
    CHECK(SotnPlayerParticles_Tick(&effect, view, input, &random) && effect.count == 22 &&
          effect.lifetime == 63 && effect.tiles[0].x == -17 * 65536 &&
          effect.tiles[0].y == 40 * 65536 && effect.tiles[0].velocity_y == -8192 &&
          effect.tiles[9].velocity_y == -505856 && effect.tiles[10].velocity_y == 8192 &&
          effect.tiles[21].velocity_y == 751616 && random == 0x12345678);
    CHECK(SotnPlayerParticles_Tick(&effect, view, input, &random) && effect.lifetime == 61 &&
          effect.tiles[0].timer == 62 && effect.tiles[0].y == 40 * 65536 - 8192);
    const SotnPlayerParticles before = effect;
    CHECK(SotnPlayerParticles_Rebase(&effect, 3, -5) && effect.lifetime == before.lifetime &&
          random == 0x12345678 && SotnPlayerParticles_Rebase(&effect, -3, 5) &&
          !SDL_memcmp(&effect, &before, sizeof(effect)));
    for (Uint16 tick = 1; tick < 31; ++tick)
        CHECK(SotnPlayerParticles_Tick(&effect, view, input, &random) && !effect.removed &&
              effect.tiles[0].red == 64 && effect.tiles[0].green == 96 &&
              effect.tiles[0].blue == 128 && effect.tiles[0].x == -17 * 65536 &&
              random == 0x12345678);
    CHECK(effect.lifetime == 1 && effect.tiles[0].timer == 32);
    const Sint32 last = effect.tiles[0].y;
    CHECK(SotnPlayerParticles_Tick(&effect, view, input, &random) && effect.removed &&
          !effect.lifetime && effect.tiles[0].y == last && random == 0x12345678);
    return true;
}
// Profile7 (holy water flame smoke, behaviour3 like profiles4/11): three rand calls per tile
// (spread 0..15-7, rise 0..31, speed), staggered timers, Y-only motion, double lifetime decrement.
static bool HolySmoke(void)
{
    Uint8 dra[0xdbd4] = {0};
    Put32(dra + 0xdb98 + 7 * 4U, 0x800a0100U);
    Uint8 *p = dra + 0x100;
    p[0] = 8;
    p[1] = 63;
    p[2] = 240;
    p[3] = 63;
    p[4] = p[5] = 1;
    Put16(p + 6, 4);
    Put16(p + 8, 0x73);
    Put16(p + 10, 3);
    Put32(p + 12, 0x8800000);
    const SotnDataView view = {dra, sizeof(dra)};
    Uint16 rows = 0;
    CHECK(SotnPlayerParticles_Rows(view, 0x700, &rows) && rows == 9);
    Put16(p + 10, 4);
    CHECK(!SotnPlayerParticles_Rows(view, 0x700, &rows));
    Put16(p + 10, 3);
    SotnPlayerParticles effect = {0};
    Uint32 random = 77, expected = 77;
    const SotnPlayerParticlesInput input = {.parameter = 0x700,
                                            .source = {.x = 100 * 65536, .y = 80 * 65536},
                                            .player_priority = 148,
                                            .allocated = true};
    CHECK(SotnPlayerParticles_Tick(&effect, view, input, &random) && effect.phase == 1 &&
          effect.count == 8 && effect.priority == 152 && effect.lifetime == 31);
    for (Uint16 i = 0; i < 8; ++i)
    {
        const SotnPlayerParticle *t = &effect.tiles[i];
        const Sint32 x = 100 + (Sint32)(SotnCombat_Random(&expected) & 15U) - 7,
                     y = 80 - (Sint32)(SotnCombat_Random(&expected) & 31U);
        const Sint32 vy = -49152 - (Sint32)(SotnCombat_Random(&expected) & 32767U);
        CHECK(t->x == x * 65536 && t->y == y * 65536 && t->velocity_y == vy && !t->velocity_x &&
              t->timer == i * 2U + 15U && t->green == 240 && t->draw_mode == 0x73);
    }
    CHECK(random == expected);
    Uint16 updates = 0;
    while (!effect.removed && updates < 40)
    {
        const SotnPlayerParticles before = effect;
        CHECK(SotnPlayerParticles_Tick(&effect, view, input, &random) && random == expected);
        ++updates;
        if (!effect.removed)
            CHECK(effect.tiles[3].y == before.tiles[3].y + before.tiles[3].velocity_y &&
                  effect.tiles[3].x == before.tiles[3].x);
    }
    CHECK(updates == 16);
    return true;
}
int main(void)
{
    return Run() && VectorParticles(5) && VectorParticles(6) && StoneParticles() && Vectors() &&
                   Smoke() && ScatteredSmoke() && HurtSmoke() && SireParticles() && HolySmoke()
               ? 0
               : 1;
}
