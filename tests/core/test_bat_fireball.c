#include <sotn/bat_fireball.h>

#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
int main(void)
{
    for (Uint16 facing = 0; facing < 2; ++facing)
    {
        SotnBatFireball fireball = {.x = 100 * 65536 + 123, .y = -2 * 65536 + 456};
        SotnBatFireballInput in = {
            .player_priority = 144, .player_facing_left = facing != 0, .hit_flags = 255};
        SotnBatFireballEvents out = {0};
        CHECK(SotnBatFireball_Tick(&fireball, in, &out));
        const Sint32 speed = facing ? -229376 : 229376;
        CHECK(fireball.initialized && !fireball.removed && fireball.facing_left == !facing &&
              fireball.x == 100 * 65536 + 123 + speed * 2 && fireball.y == -6 * 65536 + 456 &&
              fireball.velocity_x == speed && fireball.scale_x == 64 && fireball.scale_y == 64 &&
              fireball.priority == 142 && fireball.bank == 9 && fireball.width == 4 &&
              fireball.height == 8 && out.initialize && out.spell == 9 && out.sound == 0x680 &&
              out.player_use_ticks == 4);
        // Only birth observes the player. Later turns do not redirect or reprioritize a shot.
        in = (SotnBatFireballInput){.player_priority = 511, .player_facing_left = !facing};
        for (Uint16 i = 1; i <= 30; ++i)
        {
            CHECK(SotnBatFireball_Tick(&fireball, in, &out));
            CHECK(fireball.x == 100 * 65536 + 123 + speed * (2 + i) &&
                  fireball.y == -6 * 65536 + 456 && fireball.priority == 142 &&
                  fireball.scale_x == SDL_min(384, 64 + i * 16) &&
                  fireball.scale_y == SDL_min(256, 64 + i * 16) && !out.initialize && !out.sound &&
                  !out.spell && !out.player_use_ticks);
        }
        const Sint32 x = fireball.x;
        in.hit_flags = 0x80;
        CHECK(SotnBatFireball_Tick(&fireball, in, &out) && fireball.removed && fireball.x == x);
        CHECK(!SotnBatFireball_Tick(&fireball, in, &out));
    }
    // Original word/fixed-point wrap retains fractions at signed coordinate boundaries.
    SotnBatFireball fireball = {.x = SDL_MAX_SINT32, .y = SDL_MIN_SINT32 + 123};
    SotnBatFireballEvents out = {0};
    CHECK(SotnBatFireball_Tick(&fireball, (SotnBatFireballInput){.player_priority = 2}, &out));
    CHECK(fireball.x == SDL_MIN_SINT32 + 7 * 65536 - 1 &&
          fireball.y == SDL_MAX_SINT32 - 4 * 65536 + 124 && !fireball.priority);
    fireball.scale_x = 385;
    const SotnBatFireball before = fireball;
    const SotnBatFireballEvents saved = out;
    CHECK(!SotnBatFireball_Tick(&fireball, (SotnBatFireballInput){0}, &out));
    CHECK(!SDL_memcmp(&before, &fireball, sizeof(before)) &&
          !SDL_memcmp(&saved, &out, sizeof(out)));
    fireball = (SotnBatFireball){0};
    CHECK(!SotnBatFireball_Tick(&fireball, (SotnBatFireballInput){.player_priority = 1}, &out));
    CHECK(!fireball.initialized && !SDL_memcmp(&saved, &out, sizeof(out)));
    CHECK(!SotnBatFireball_Tick(NULL, (SotnBatFireballInput){0}, &out));
    CHECK(!SotnBatFireball_Tick(&fireball, (SotnBatFireballInput){0}, NULL));
    return 0;
}
