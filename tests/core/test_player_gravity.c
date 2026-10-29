#include <SDL3/SDL.h>
#include <sotn/player.h>
#define CHECK(x)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(x))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #x, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    Uint8 dra[0x11000], tiles[64 * 64 * 2], types[2];
    SotnPlayer *p;
} Fixture;
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8));
}
static bool Start(Fixture *f, Uint8 relic, Uint16 mp, Uint16 roof)
{
    SotnPlayer_Close(f->p);
    SDL_memset(f, 0, sizeof(*f));
    for (Uint32 i = 0; i < 247; ++i)
        Put32(f->dra + 0x101b8 + i * 4, 0x800a0100);
    for (Uint32 i = 0; i < 6; ++i)
        Put32(f->dra + 0x100 + i * 4, 2 | (i + 1) << 16);
    Put32(f->dra + 0x101b8 + 45 * 4, 0x800a0200);
    for (Uint32 i = 0; i < 4; ++i)
        Put32(f->dra + 0x200 + i * 4, 2 | (i + 1) << 16);
    Put32(f->dra + 0x210, 0xffff);
    f->types[1] = 3;
    for (Uint32 y = 50; y < 64; ++y)
        for (Uint32 x = 0; x < 64; ++x)
            f->tiles[(y * 64 + x) * 2] = 1;
    if (roof)
        for (Uint32 x = 0; x < 64; ++x)
            f->tiles[(roof * 64 + x) * 2] = 1;
    const SotnCollisionMap map = {{f->tiles, sizeof(f->tiles)}, {f->types, 2}, 64, 64};
    SotnWorldState world = {0};
    world.relics[SOTN_RELIC_GRAVITY] = relic;
    world.relics[SOTN_RELIC_LEAP] = 3;
    CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)}, map, 256, 775, &f->p));
    CHECK(SotnPlayer_SetRelics(f->p, &world));
    CHECK(SotnPlayer_SetCharacter(
        f->p, &(SotnCharacterState){.hp = 70, .hp_max = 70, .mp = mp, .mp_max = 20},
        (SotnCharacterStats){.total = {6, 6, 6, 6}, .attack = {6, 6}, .unarmed_attack = 6}));
    CHECK(SotnPlayer_Tick(f->p, 0));
    return true;
}
static bool Command(Fixture *f)
{
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_DOWN));
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_UP));
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_JUMP));
    return true;
}
static bool TestLaunch(Fixture *f)
{
    const Uint16 costs[] = {0, 4, 5, 20};
    for (Uint8 relic = 0; relic <= 3; ++relic)
    {
        if (relic == 2)
            continue;
        for (Uint32 i = 0; i < SDL_arraysize(costs); ++i)
        {
            SotnPlayerState p = {0};
            CHECK(Start(f, relic, costs[i], 0) && Command(f) && SotnPlayer_GetState(f->p, &p));
            bool active = relic == 3 && costs[i] > 4;
            CHECK((p.step == SOTN_PLAYER_HIGH_JUMP) == active);
            CHECK(p.mp == (active ? costs[i] - 4 : costs[i]) && p.mp_spent == (active ? 4 : 0));
            if (active)
            {
                CHECK(p.velocity_y == -12 * 65536 && p.velocity_x == 0 && p.animation.id == 33 &&
                      p.gravity.timer == 1);
                CHECK(p.y == 763 * 65536 && p.gravity.beams[0].active &&
                      p.gravity.beams[0].y == 800 * 65536 && p.gravity.beams[0].width == 1536);
                CHECK(SotnPlayer_Tick(f->p, 0) && SotnPlayer_GetState(f->p, &p) &&
                      p.mp_spent == 0 && p.mp == costs[i] - 4);
                Uint32 n = 0;
                while (p.step == SOTN_PLAYER_HIGH_JUMP && n++ < 90)
                {
                    CHECK(SotnPlayer_Tick(f->p, 0) && SotnPlayer_GetState(f->p, &p));
                }
                CHECK(p.step == SOTN_PLAYER_JUMP && p.substep == 1 && p.velocity_y == 49152 &&
                      !p.gravity.upside_down);
                for (Uint32 t = 0; t < 60; ++t)
                    CHECK(SotnPlayer_Tick(f->p, 0));
                CHECK(SotnPlayer_GetState(f->p, &p) && !p.gravity.beams[0].active &&
                      p.mp == costs[i] - 4);
            }
        }
    }
    return true;
}
static bool TestCommands(Fixture *f)
{
    SotnPlayerState p = {0};
    CHECK(Start(f, 3, 20, 0));
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_DOWN));
    for (Uint32 i = 0; i < 16; ++i)
        CHECK(SotnPlayer_Tick(f->p, 0));
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_UP) && SotnPlayer_Tick(f->p, SOTN_BUTTON_JUMP));
    CHECK(SotnPlayer_GetState(f->p, &p) && p.step != SOTN_PLAYER_HIGH_JUMP && p.mp == 20);
    CHECK(Start(f, 3, 20, 0) && SotnPlayer_Tick(f->p, SOTN_BUTTON_RIGHT));
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_RIGHT | SOTN_BUTTON_DOWN));
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_UP) && SotnPlayer_Tick(f->p, SOTN_BUTTON_JUMP));
    CHECK(SotnPlayer_GetState(f->p, &p) && p.step != SOTN_PLAYER_HIGH_JUMP && p.mp == 20);
    CHECK(Start(f, 3, 20, 0) && SotnPlayer_Tick(f->p, SOTN_BUTTON_DOWN) &&
          SotnPlayer_Tick(f->p, SOTN_BUTTON_UP));
    for (Uint32 x = 0; x < 64; ++x)
        f->tiles[(47 * 64 + x) * 2] = 1;
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_JUMP) && SotnPlayer_GetState(f->p, &p) && p.mp == 20 &&
          p.step != SOTN_PLAYER_HIGH_JUMP);
    CHECK(Start(f, 3, 20, 0) && SotnPlayer_Tick(f->p, SOTN_BUTTON_JUMP) &&
          SotnPlayer_Tick(f->p, 0) && SotnPlayer_Tick(f->p, SOTN_BUTTON_JUMP));
    // Original command requires a neutral previous word before the initial Down.
    CHECK(SotnPlayer_Tick(f->p, 0) && Command(f) && SotnPlayer_GetState(f->p, &p) &&
          p.step == SOTN_PLAYER_HIGH_JUMP && p.mp == 16);
    CHECK(Start(f, 3, 20, 0) && SotnPlayer_Tick(f->p, SOTN_BUTTON_DOWN) &&
          SotnPlayer_Tick(f->p, SOTN_BUTTON_UP));
    Put32(f->dra + 0x101b8 + 33 * 4, 0x800b1000);
    CHECK(!SotnPlayer_Tick(f->p, SOTN_BUTTON_JUMP) && SotnPlayer_GetState(f->p, &p) && p.mp == 20 &&
          !p.mp_spent && !p.gravity.beams[0].active);
    Put32(f->dra + 0x101b8 + 33 * 4, 0x800a0100);
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_JUMP) && SotnPlayer_GetState(f->p, &p) && p.mp == 16 &&
          p.mp_spent == 4);
    return true;
}
static bool TestCeilings(Fixture *f)
{
    const Uint16 roofs[] = {46, 30};
    for (Uint32 i = 0; i < 2; ++i)
    {
        SotnPlayerState p = {0}, origin = {0};
        CHECK(Start(f, 3, 20, roofs[i]) && Command(f));
        Uint32 n = 0;
        do
        {
            CHECK(SotnPlayer_GetState(f->p, &origin));
            CHECK(SotnPlayer_Tick(f->p, 0) && SotnPlayer_GetState(f->p, &p));
        } while (p.substep == 0 && n++ < 50);
        CHECK(p.step == SOTN_PLAYER_HIGH_JUMP && p.substep == (i ? 2 : 3) && p.velocity_y == 0 &&
              p.velocity_x == 0 && p.sound == 0x644);
        CHECK(p.gravity.upside_down == (i != 0) && p.facing_left == (i != 0) &&
              p.draw_pivot_x == 0 && p.draw_pivot_y == (i ? 2 : 0));
        CHECK(p.gravity.ceiling_impact && p.gravity.impact_source.x == origin.x + 3 * 65536 &&
              p.gravity.impact_source.y == origin.y - 22 * 65536 &&
              !p.gravity.impact_source.facing_left && p.gravity.impact_source.priority == 148);
        const SotnCollisionMap map = {{f->tiles, sizeof(f->tiles)}, {f->types, 2}, 64, 64};
        const SotnPlayerState captured = p;
        CHECK(SotnPlayer_ChangeRoom(f->p, map, 16, -8) && SotnPlayer_GetState(f->p, &p));
        CHECK(p.gravity.ceiling_impact && p.gravity.timer == captured.gravity.timer &&
              p.gravity.impact_source.x == captured.gravity.impact_source.x + 16 * 65536 &&
              p.gravity.impact_source.y == captured.gravity.impact_source.y - 8 * 65536);
        CHECK(SotnPlayer_ChangeRoom(f->p, map, -16, 8) && SotnPlayer_GetState(f->p, &p));
        CHECK(!SDL_memcmp(&p.gravity.impact_source, &captured.gravity.impact_source,
                          sizeof(p.gravity.impact_source)));
        if (i)
        {
            CHECK(p.animation.id == 43);
            while (p.gravity.timer < 56)
                CHECK(SotnPlayer_Tick(f->p, 0) && SotnPlayer_GetState(f->p, &p));
            CHECK(SotnPlayer_Tick(f->p, 0) && SotnPlayer_GetState(f->p, &p) &&
                  p.gravity.timer == 57 && p.substep == 4 && p.animation.id == 45 &&
                  !p.gravity.upside_down && !p.facing_left);
        }
        n = 0;
        while (p.step == SOTN_PLAYER_HIGH_JUMP && n++ < 80)
            CHECK(SotnPlayer_Tick(f->p, 0) && SotnPlayer_GetState(f->p, &p));
        CHECK(p.step == SOTN_PLAYER_JUMP && p.substep == 1);
        CHECK(!p.gravity.ceiling_impact && p.draw_pivot_y == (i ? 2 : 0));
    }
    CHECK(Start(f, 3, 20, 30) && Command(f));
    SotnPlayerState p = {0};
    for (Uint32 i = 0; i < 30; ++i)
        CHECK(SotnPlayer_Tick(f->p, 0));
    CHECK(SotnPlayer_GetState(f->p, &p) && p.gravity.upside_down);
    CHECK(SotnPlayer_Damage(f->p, 1, 200, 0) && SotnPlayer_GetState(f->p, &p));
    CHECK(p.step == SOTN_PLAYER_HIT && !p.gravity.upside_down && !p.facing_left &&
          p.velocity_y == -2 * 65536);
    return true;
}

static bool TestPreviousButtons(Fixture *f)
{
    const Uint16 previous[] = {SOTN_BUTTON_LEFT, SOTN_BUTTON_RIGHT, SOTN_BUTTON_UP};
    for (Uint16 i = 0; i < SDL_arraysize(previous); ++i)
    {
        SotnPlayerState p = {0};
        CHECK(Start(f, 3, 20, 0) && SotnPlayer_Tick(f->p, previous[i]) && Command(f) &&
              SotnPlayer_GetState(f->p, &p));
        CHECK(p.step != SOTN_PLAYER_HIGH_JUMP && p.mp == 20 && !p.mp_spent);
    }
    CHECK(Start(f, 3, 20, 0) && SotnPlayer_Tick(f->p, SOTN_BUTTON_LEFT) &&
          SotnPlayer_Tick(f->p, 0) && Command(f));
    SotnPlayerState p = {0};
    CHECK(SotnPlayer_GetState(f->p, &p) && p.step == SOTN_PLAYER_HIGH_JUMP && p.mp == 16);
    return true;
}
static bool TestLateCeilingAndRoom(Fixture *f)
{
    SotnPlayerState p = {0}, before = {0};
    CHECK(Start(f, 3, 20, 0) && Command(f) && SotnPlayer_GetState(f->p, &p));
    while (p.gravity.timer < 30)
        CHECK(SotnPlayer_Tick(f->p, 0) && SotnPlayer_GetState(f->p, &p));
    const Uint32 row = (Uint32)((p.y / 65536 - 22) / 16);
    for (Uint32 x = 0; x < 64; ++x)
        f->tiles[(row * 64 + x) * 2] = 1;
    for (Uint32 i = 0; i < 4 && p.substep != 2; ++i)
        CHECK(SotnPlayer_Tick(f->p, 0) && SotnPlayer_GetState(f->p, &p));
    CHECK(p.step == SOTN_PLAYER_HIGH_JUMP && p.substep == 2 && !p.gravity.upside_down &&
          !p.facing_left && p.velocity_y == 0);
    while (p.gravity.timer < 57)
        CHECK(SotnPlayer_Tick(f->p, 0) && SotnPlayer_GetState(f->p, &p));
    CHECK(p.substep == 4 && !p.gravity.upside_down && p.facing_left); // Original late-hit flip.

    CHECK(Start(f, 3, 20, 0) && Command(f) && SotnPlayer_GetState(f->p, &before));
    const SotnCollisionMap map = {{f->tiles, sizeof(f->tiles)}, {f->types, 2}, 64, 64};
    CHECK(SotnPlayer_ChangeRoom(f->p, map, 16, 0) && SotnPlayer_GetState(f->p, &p));
    CHECK(p.x == before.x + 16 * 65536 &&
          p.gravity.beams[0].x == before.gravity.beams[0].x + 16 * 65536 &&
          p.gravity.beams[0].y == before.gravity.beams[0].y && p.mp == before.mp);
    before = p;
    CHECK(!SotnPlayer_ChangeRoom(f->p, map, SDL_MAX_SINT32, 0));
    CHECK(SotnPlayer_GetState(f->p, &p) && p.x == before.x &&
          p.gravity.beams[0].x == before.gravity.beams[0].x && p.mp == before.mp);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    bool ok = TestLaunch(f) && TestCommands(f) && TestCeilings(f) && TestLateCeilingAndRoom(f) &&
              TestPreviousButtons(f);
    SotnPlayer_Close(f->p);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_PLAYER_GRAVITY_TEST: PASS");
    return 0;
}
