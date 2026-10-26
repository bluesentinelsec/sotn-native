#include <SDL3/SDL.h>
#include <sotn/player.h>

#define CHECK(x)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(x))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #x, SDL_GetError());                              \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

typedef struct Fixture
{
    Uint8 dra[0x11000], tiles[64 * 64 * 2], types[2];
    SotnPlayer *player;
} Fixture;

static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8));
}

static bool Begin(Fixture *f, bool relic)
{
    SotnPlayer_Close(f->player);
    SDL_memset(f, 0, sizeof(*f));
    for (Uint32 i = 0; i < 247; ++i)
        Put32(f->dra + 0x101b8 + i * 4, 0x800a0100);
    for (Uint32 i = 0; i < 6; ++i)
        Put32(f->dra + 0x100 + i * 4, 2 | (i + 1) << 16);
    Put32(f->dra + 0xc0, 0x800a0300);
    f->dra[0x312] = 0x24;
    f->dra[0x31a] = 6;
    f->dra[0x326] = 3;
    f->dra[0x328] = 2;
    f->dra[0x32a] = 1;
    f->dra[0x330] = 5;
    f->types[1] = 3;
    for (Uint32 y = 50; y < 64; ++y)
        for (Uint32 x = 0; x < 64; ++x)
            f->tiles[(y * 64 + x) * 2] = 1;
    const SotnCollisionMap map = {{f->tiles, sizeof(f->tiles)}, {f->types, 2}, 64, 64};
    SotnWorldState world = {0};
    world.relics[SOTN_RELIC_LEAP] = relic ? 3 : 0;
    CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)}, map, 256, 775, &f->player));
    CHECK(SotnPlayer_SetRelics(f->player, &world));
    CHECK(SotnPlayer_Tick(f->player, 0));
    for (Uint32 i = 0; i < 12; ++i)
        CHECK(SotnPlayer_Tick(f->player, SOTN_BUTTON_JUMP));
    CHECK(SotnPlayer_Tick(f->player, 0));
    for (Uint32 i = 0; i < 8; ++i)
        CHECK(SotnPlayer_Tick(f->player, SOTN_BUTTON_JUMP));
    CHECK(SotnPlayer_Tick(f->player, 0));
    return true;
}

static bool Kick(Fixture *f, Uint16 direction, SotnPlayerState *state)
{
    CHECK(SotnPlayer_Tick(f->player, SOTN_BUTTON_DOWN | SOTN_BUTTON_JUMP | direction));
    CHECK(SotnPlayer_GetState(f->player, state));
    return true;
}

static bool TestStart(Fixture *f)
{
    SotnPlayerState state = {0};
    CHECK(Begin(f, false) && Kick(f, 0, &state));
    CHECK(state.substep != 0x70 && state.kick.generation == 0 && !state.kick.attack.active);
    const Uint16 directions[] = {0, SOTN_BUTTON_RIGHT, SOTN_BUTTON_LEFT};
    for (Uint32 i = 0; i < SDL_arraysize(directions); ++i)
    {
        CHECK(Begin(f, true) && Kick(f, directions[i], &state));
        CHECK(state.step == SOTN_PLAYER_JUMP && state.substep == 0x70 && state.animation.id == 34);
        CHECK(state.velocity_y == 393216 && state.sound == 0x6f0);
        CHECK(state.velocity_x == (i == 0 ? 0 : i == 1 ? 294912 : -294912));
        CHECK(state.kick.generation && state.kick.attack.active && !state.attack.active);
        CHECK(state.kick.attack.offset_x == 9 && state.kick.attack.offset_y == 21 &&
              state.kick.attack.width == 4 && state.kick.attack.height == 5 &&
              state.kick.attack.power == 6 && state.kick.attack.hit_type == 2 &&
              state.kick.attack.stun_frames == 3 && state.kick.attack.invincibility_frames == 6);
        const Uint32 generation = state.kick.generation, random = state.random;
        CHECK(SotnPlayer_Tick(f->player, SOTN_BUTTON_DOWN | SOTN_BUTTON_JUMP | directions[i]));
        CHECK(SotnPlayer_GetState(f->player, &state));
        CHECK(state.kick.generation == generation && state.random == random && state.sound == 0);
        CHECK(state.velocity_y == 393216 + 11264);
    }
    return true;
}

static bool TestFeedback(Fixture *f)
{
    for (Uint16 hit = 1; hit <= 3; ++hit)
        for (Uint16 held = 0; held <= 1; ++held)
        {
            SotnPlayerState state = {0};
            CHECK(Begin(f, true) && Kick(f, 0, &state));
            const Uint32 generation = state.kick.generation;
            const Uint16 buttons = held ? SOTN_BUTTON_JUMP : 0;
            CHECK(!SotnPlayer_KickHit(NULL, generation, hit));
            CHECK(!SotnPlayer_KickHit(f->player, 0, hit) &&
                  !SotnPlayer_KickHit(f->player, generation + 1, hit) &&
                  !SotnPlayer_KickHit(f->player, generation, 0) &&
                  !SotnPlayer_KickHit(f->player, generation, 4));
            CHECK(SotnPlayer_KickHit(f->player, generation, hit));
            CHECK(SotnPlayer_Tick(f->player, buttons) && SotnPlayer_GetState(f->player, &state));
            CHECK(state.substep == 0x70 && state.velocity_y > 0); // Child publishes after player.
            CHECK(SotnPlayer_Tick(f->player, buttons) && SotnPlayer_GetState(f->player, &state));
            if (hit == 1)
            {
                const Sint32 speed = held ? -319488 : -278528;
                CHECK(state.substep == 0 && state.velocity_y == speed && !state.kick.generation);
                CHECK(!SotnPlayer_KickHit(f->player, generation, 1));
                CHECK(SotnPlayer_Tick(f->player, 0) && SotnPlayer_GetState(f->player, &state));
                CHECK(state.velocity_y == speed + 11264); // Rebound ignores early-release cap.
                CHECK(SotnPlayer_Tick(f->player,
                                      SOTN_BUTTON_JUMP)); // A rebound permits another leap.
                CHECK(SotnPlayer_GetState(f->player, &state) && state.animation.id == 32);
                CHECK(SotnPlayer_Tick(f->player, 0) && Kick(f, 0, &state));
                CHECK(state.substep == 0x70 && state.kick.generation != generation);
            }
            else
                CHECK(state.substep == 0x70 && state.velocity_y > 0 &&
                      state.kick.generation == generation);
        }
    return true;
}

static bool TestLifecycle(Fixture *f)
{
    SotnPlayerState before = {0}, after = {0};
    CHECK(Begin(f, true) && SotnPlayer_GetState(f->player, &before));
    Put32(f->dra + 0xc0, 0x800b1000);
    CHECK(!SotnPlayer_Tick(f->player, SOTN_BUTTON_DOWN | SOTN_BUTTON_JUMP));
    CHECK(SotnPlayer_GetState(f->player, &after));
    CHECK(after.x == before.x && after.y == before.y && after.velocity_y == before.velocity_y &&
          after.random == before.random && after.kick.generation == 0 &&
          after.animation.id == before.animation.id);
    Put32(f->dra + 0xc0, 0x800a0300);
    CHECK(Kick(f, SOTN_BUTTON_RIGHT, &after));
    CHECK(SotnPlayer_KickHit(f->player, after.kick.generation, 1));
    CHECK(SotnPlayer_Damage(f->player, 10, 256, 0));
    CHECK(SotnPlayer_Tick(f->player, 0) && SotnPlayer_GetState(f->player, &after));
    CHECK(after.step == SOTN_PLAYER_HIT && !after.kick.generation && !after.kick.attack.active);
    CHECK(Begin(f, true) && Kick(f, SOTN_BUTTON_RIGHT, &after));
    for (Uint16 i = 0; i < 40 && after.step == SOTN_PLAYER_JUMP; ++i)
    {
        CHECK(SotnPlayer_Tick(f->player, 0) && SotnPlayer_GetState(f->player, &after));
    }
    CHECK(after.step == SOTN_PLAYER_CROUCH && after.substep == 3 && !after.kick.generation &&
          after.velocity_x > 0 && after.velocity_y == 0);
    CHECK(Begin(f, true) && Kick(f, 0, &after));
    CHECK(SotnPlayer_SetScriptHold(f->player, true) && SotnPlayer_Tick(f->player, 0));
    CHECK(SotnPlayer_GetState(f->player, &after) && after.step == SOTN_PLAYER_SCRIPT &&
          !after.kick.generation);
    return true;
}

int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = TestStart(f) && TestFeedback(f) && TestLifecycle(f);
    SotnPlayer_Close(f->player);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_PLAYER_KICK_TEST: PASS");
    return 0;
}
