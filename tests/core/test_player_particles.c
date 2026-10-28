#include <SDL3/SDL.h>
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
    dra[0x100] = 17;
    CHECK(!SotnPlayerParticles_Rows(view, 0x100, &unchanged) && unchanged == 90);
    Put32(dra + 0xdb98 + 4, 0x800adbd0);
    CHECK(!SotnPlayerParticles_Rows(view, 0x100, &unchanged) && unchanged == 90);
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
    Put16(dra + 0xd57c, 0);
    CHECK(!SotnPlayerSmoke_Tick(&puff, view, source, 0, 148, 0, 0) && !puff.phase);
    return true;
}
int main(void)
{
    return Run() && Smoke() ? 0 : 1;
}
