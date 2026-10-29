#include <SDL3/SDL.h>
#include <sotn/hellfire_fireball.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put32(Uint8 *p, Sint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)((Uint32)v >> (i * 8U));
}
static bool Normal(SotnDataView dra)
{
    for (Uint16 face = 0; face < 2; ++face)
        for (Uint16 parameter = 0; parameter < 3; ++parameter)
        {
            const Sint32 vx[3] = {-123456, 78901, -200003}, vy[3] = {16000, -20000, 30000};
            SotnHellfireFireball b = {
                .x = 100 * 65536 + 123, .y = 140 * 65536 + 456, .parameter = parameter};
            SotnHellfireFireballInput in = {
                .dra = dra, .player_priority = 148, .player_facing_left = face != 0};
            SotnHellfireFireballEvents e = {0};
            CHECK(SotnHellfireFireball_Tick(&b, in, &e));
            CHECK(e.initialize && e.spell == 2 && e.animation_offset == 0x10798 &&
                  e.sound == (parameter ? 0 : 0x660) && b.bank == 9 && b.priority == 150 &&
                  b.flags == 0x08100000U && b.width == 4 && b.height == 4 &&
                  b.facing_left != (face != 0) &&
                  b.velocity_x == (face ? vx[parameter] : -vx[parameter]) &&
                  b.velocity_y == vy[parameter] && b.x == 100 * 65536 + 123 &&
                  b.y == 140 * 65536 + 456 && b.timer == 20);
            for (Uint16 t = 1; t <= 20; ++t)
            {
                CHECK(SotnHellfireFireball_Tick(&b, in, &e) && !e.initialize && !e.spell &&
                      !e.sound &&
                      b.x ==
                          100 * 65536 + 123 + (Sint32)t * (face ? vx[parameter] : -vx[parameter]) &&
                      b.y == 140 * 65536 + 456 + (Sint32)t * vy[parameter]);
            }
            CHECK(b.phase == 2 && b.timer == 0 && b.velocity_y == vy[parameter]);
            const Sint32 y = b.y;
            CHECK(SotnHellfireFireball_Tick(&b, in, &e) && b.y == y);
            const Sint32 x = b.x;
            in.hit_flags = 0x80;
            CHECK(SotnHellfireFireball_Tick(&b, in, &e) && b.removed && b.x == x && b.y == y);
            b = (SotnHellfireFireball){
                .phase = 1, .timer = 20, .velocity_x = 100, .velocity_y = 200};
            CHECK(SotnHellfireFireball_Tick(&b, in, &e) && b.removed && b.timer == 20 && !b.x &&
                  !b.y);
        }
    return true;
}
static bool Big(void)
{
    for (Uint16 face = 0; face < 2; ++face)
        for (Uint16 parameter = 0; parameter < 2; ++parameter)
        {
            SotnHellfireFireball b = {.x = 100 * 65536 + 123,
                                      .y = 140 * 65536 + 456,
                                      .parameter = parameter,
                                      .big = true,
                                      .draw_flags = 0x10,
                                      .rotation = 1234};
            SotnHellfireFireballInput in = {.player_priority = 148,
                                            .player_facing_left = face != 0};
            SotnHellfireFireballEvents e = {0};
            CHECK(SotnHellfireFireball_Tick(&b, in, &e));
            CHECK(e.initialize && e.spell == 2 && e.animation_offset == 0x107c8 &&
                  e.sound == (parameter ? 0x683 : 0) &&
                  b.y == (parameter ? 156 : 136) * 65536 + 456 && b.x == 100 * 65536 + 123 &&
                  b.draw_flags == 0x14 && !b.rotation && b.velocity_x == (face ? -16 : 16) &&
                  b.width == 8 && b.height == 8);
            const Sint32 x = b.x, y = b.y;
            in.pose = 22;
            in.random = 65535;
            in.hit_flags = 255;
            CHECK(SotnHellfireFireball_Tick(&b, in, &e) && b.x == x && b.y == y && !e.random_used &&
                  !b.removed);
            in.pose = 23;
            in.random = 1;
            in.random_ready = true;
            CHECK(SotnHellfireFireball_Tick(&b, in, &e) && e.random_used && e.factory == 0x10024 &&
                  b.velocity_x == (face ? -6160 : 6160) && b.x == x + b.velocity_x &&
                  b.rotation == 1024 && !b.removed);
            in.game_timer = 1;
            in.random_ready = false;
            CHECK(SotnHellfireFireball_Tick(&b, in, &e) && !e.random_used && !e.factory &&
                  b.rotation == 1024);
            const SotnHellfireFireball saved = b;
            const SotnHellfireFireballEvents held = {.spell = 17};
            e = held;
            in.game_timer = 2;
            CHECK(!SotnHellfireFireball_Tick(&b, in, &e) && !SDL_memcmp(&b, &saved, sizeof(b)) &&
                  !SDL_memcmp(&e, &held, sizeof(e)));
            in.random_ready = true;
            in.random = 0;
            CHECK(SotnHellfireFireball_Tick(&b, in, &e) && e.random_used && !e.factory);
            b.rotation = 32760;
            in.game_timer = 4;
            CHECK(SotnHellfireFireball_Tick(&b, in, &e) && b.rotation == -31752);
        }
    SotnHellfireFireball b = {.phase = 1,
                              .big = true,
                              .x = SDL_MAX_SINT32,
                              .velocity_x = SDL_MAX_SINT32,
                              .velocity_y = 1,
                              .y = SDL_MAX_SINT32};
    SotnHellfireFireballEvents e = {0};
    CHECK(SotnHellfireFireball_Tick(&b, (SotnHellfireFireballInput){.pose = 23, .game_timer = 1},
                                    &e) &&
          b.velocity_x == SDL_MIN_SINT32 + 6143 && b.x == 6142 && b.y == SDL_MIN_SINT32);
    return true;
}
static bool Invalid(SotnDataView dra)
{
    SotnHellfireFireball b = {.parameter = 3};
    const SotnHellfireFireball saved = b;
    const SotnHellfireFireballEvents held = {.factory = 42};
    SotnHellfireFireballEvents e = held;
    CHECK(!SotnHellfireFireball_Tick(&b, (SotnHellfireFireballInput){.dra = dra}, &e) &&
          !SDL_memcmp(&b, &saved, sizeof(b)) && !SDL_memcmp(&e, &held, sizeof(e)));
    b = (SotnHellfireFireball){0};
    CHECK(!SotnHellfireFireball_Tick(&b, (SotnHellfireFireballInput){0}, &e));
    CHECK(!SotnHellfireFireball_Tick(
        &b, (SotnHellfireFireballInput){.dra = {dra.bytes, dra.size - 1}}, &e));
    CHECK(!SotnHellfireFireball_Tick(
        &b, (SotnHellfireFireballInput){.dra = dra, .player_priority = 510}, &e));
    b = (SotnHellfireFireball){.big = true, .phase = 1};
    CHECK(!SotnHellfireFireball_Tick(
        &b, (SotnHellfireFireballInput){.pose = 23, .random_ready = true, .random = 32768}, &e));
    CHECK(!SotnHellfireFireball_Tick(NULL, (SotnHellfireFireballInput){0}, &e));
    CHECK(!SotnHellfireFireball_Tick(&b, (SotnHellfireFireballInput){0}, NULL));
    return true;
}
int main(void)
{
    Uint8 *d = SDL_calloc(1, 0x10848);
    if (!d)
        return 1;
    const Sint32 vx[3] = {-123456, 78901, -200003}, vy[3] = {16000, -20000, 30000};
    for (Uint16 i = 0; i < 3; ++i)
    {
        Put32(d + 0x10830 + i * 4U, vx[i]);
        Put32(d + 0x1083c + i * 4U, vy[i]);
    }
    const SotnDataView dra = {d, 0x10848};
    const bool ok = Normal(dra) && Big() && Invalid(dra);
    SDL_free(d);
    if (ok)
        SDL_Log("SOTN_HELLFIRE_FIREBALL_TEST: PASS");
    return ok ? 0 : 1;
}
