#include <SDL3/SDL.h>
#include <sotn/player.h>

#define CHECK(condition)                                                                           \
    do                                                                                             \
    {                                                                                              \
        if (!(condition))                                                                          \
        {                                                                                          \
            SDL_LogError(SDL_LOG_CATEGORY_TEST, "Line %d: %s: %s", __LINE__, #condition,           \
                         SDL_GetError());                                                          \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

typedef struct Fixture
{
    Uint8 dra[0x11000], tiles[32 * 16 * 2], types[2];
} Fixture;

static void Put32(Uint8 *image, size_t offset, Uint32 value)
{
    for (unsigned int i = 0; i < 4; ++i)
        image[offset + i] = (Uint8)(value >> (i * 8));
}

static SotnCollisionMap MakeFixture(Fixture *fixture)
{
    SDL_memset(fixture, 0, sizeof(*fixture));
    for (size_t i = 0; i < 247; ++i)
        Put32(fixture->dra, 0x101b8 + i * 4, 0x800a0100);
    for (Uint32 pose = 0; pose < 6; ++pose)
    {
        Put32(fixture->dra, 0x100 + pose * 4, 2 | (pose + 1) << 16);
        Put32(fixture->dra, 0x200 + pose * 4, 1 | (pose + 1) << 16);
    }
    Put32(fixture->dra, 0x118, 0);
    Put32(fixture->dra, 0x218, 0x0000ffff);
    const Uint16 holds[] = {16, 19, 21, 35};
    for (size_t i = 0; i < SDL_arraysize(holds); ++i)
        Put32(fixture->dra, 0x101b8 + (size_t)holds[i] * 4, 0x800a0200);
    fixture->types[1] = 3;
    for (size_t y = 10; y < 16; ++y)
        for (size_t x = 0; x < 32; ++x)
            fixture->tiles[(y * 32 + x) * 2] = 1;
    return (SotnCollisionMap){
        {fixture->tiles, sizeof(fixture->tiles)}, {fixture->types, sizeof(fixture->types)}, 32, 16};
}

static bool State(SotnPlayer *player, SotnPlayerState *out)
{
    CHECK(SotnPlayer_GetState(player, out));
    return true;
}

static bool TestMovement(Fixture *fixture)
{
    const SotnCollisionMap map = MakeFixture(fixture);
    const SotnDataView dra = {fixture->dra, sizeof(fixture->dra)};
    SotnPlayer *player = NULL;
    CHECK(SotnPlayer_Create(dra, map, 64, 135, &player));
    SotnPlayerState state = {0};
    CHECK(SotnPlayer_Tick(player, 0) && State(player, &state));
    CHECK(state.y == 135 * 65536 && (state.contacts & 1) != 0);
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_RIGHT) && State(player, &state));
    CHECK(state.x == 64 * 65536 + 98304 && state.velocity_x == 98304);
    CHECK(state.step == SOTN_PLAYER_WALK && !state.facing_left);
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_LEFT | SOTN_BUTTON_RIGHT) && State(player, &state));
    CHECK(!state.facing_left && state.velocity_x == 98304);
    CHECK(SotnPlayer_Tick(player, 0) && State(player, &state));
    CHECK(state.step == SOTN_PLAYER_STAND && state.velocity_x == 0);
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_JUMP) && State(player, &state));
    CHECK(state.step == SOTN_PLAYER_JUMP && state.velocity_y == -319488 &&
          (state.status & 0x2000U));
    CHECK(state.y == 135 * 65536 - 319488);
    CHECK(SotnPlayer_Tick(player, 0) && State(player, &state));
    CHECK(state.velocity_y == -65536 + 11264);
    for (int i = 0; i < 100; ++i)
        CHECK(SotnPlayer_Tick(player, 0));
    CHECK(State(player, &state));
    CHECK(state.y == 135 * 65536 && state.step == SOTN_PLAYER_STAND);
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_DOWN) && State(player, &state));
    CHECK(state.step == SOTN_PLAYER_CROUCH && (state.status & 0x20U));
    for (int i = 0; i < 15; ++i)
        CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_DOWN));
    for (int i = 0; i < 20; ++i)
        CHECK(SotnPlayer_Tick(player, 0));
    CHECK(State(player, &state));
    CHECK(state.step == SOTN_PLAYER_STAND);
    const Sint32 before_x = state.x, before_y = state.y;
    CHECK(!SotnPlayer_Tick(player, 0x0010));
    CHECK(State(player, &state) && state.x == before_x && state.y == before_y);
    CHECK(SotnPlayer_ChangeRoom(player, map, 16, 0));
    CHECK(State(player, &state) && state.x == before_x + 16 * 65536);
    CHECK(!SotnPlayer_ChangeRoom(player, map, SDL_MIN_SINT32, 0));
    CHECK(State(player, &state) && state.x == before_x + 16 * 65536);
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_RIGHT | SOTN_BUTTON_JUMP));
    SotnPlayerState jumping = {0};
    CHECK(State(player, &jumping) && (jumping.y & 65535) != 0);
    CHECK(SotnPlayer_ChangeRoom(player, map, 0, -48));
    CHECK(State(player, &state) && state.y == jumping.y - 48 * 65536 && state.x == jumping.x &&
          state.velocity_x == jumping.velocity_x && state.velocity_y == jumping.velocity_y &&
          state.step == jumping.step);
    SotnPlayer_Close(player);
    return true;
}

static bool TestWallsAndReplay(Fixture *fixture)
{
    const SotnCollisionMap map = MakeFixture(fixture);
    for (size_t y = 4; y < 10; ++y)
        fixture->tiles[(y * 32 + 12) * 2] = 1;
    const SotnDataView dra = {fixture->dra, sizeof(fixture->dra)};
    SotnPlayer *a = NULL, *b = NULL;
    CHECK(SotnPlayer_Create(dra, map, 64, 135, &a));
    CHECK(SotnPlayer_Create(dra, map, 64, 135, &b));
    for (int tick = 0; tick < 300; ++tick)
    {
        Uint16 buttons = SOTN_BUTTON_RIGHT;
        if (tick > 30 && tick < 50)
            buttons |= SOTN_BUTTON_JUMP;
        CHECK(SotnPlayer_Tick(a, buttons) && SotnPlayer_Tick(b, buttons));
        SotnPlayerState first = {0}, second = {0};
        CHECK(State(a, &first) && State(b, &second));
        CHECK(first.x == second.x && first.y == second.y && first.contacts == second.contacts);
        CHECK(first.animation.id == second.animation.id &&
              first.animation.pose == second.animation.pose);
        CHECK(first.x <= 185 * 65536);
    }
    SotnPlayerState last = {0};
    CHECK(State(a, &last) && (last.contacts & 4) != 0);
    SotnPlayer_Close(a);
    SotnPlayer_Close(b);
    return true;
}

static void AttackFixture(Fixture *fixture)
{
    Put32(fixture->dra, 0xc0, 0x800a0800);
    fixture->dra[0x811] = 0;
    fixture->dra[0x812] = 0x24;
    fixture->dra[0x815] = 5;
    fixture->dra[0x81a] = 8;
    fixture->dra[0x828] = 2;
    Put32(fixture->dra, 0xd53c, 0x800a0900);
    Put32(fixture->dra, 0xd540, 0x800a0a00);
    fixture->dra[0xd549] = 1;
    fixture->dra[0xa08] = 18;
    fixture->dra[0xa09] = 240;
    fixture->dra[0xa0a] = 10;
    fixture->dra[0xa0b] = 7;
    for (Uint32 i = 0; i < 7; ++i)
    {
        Put32(fixture->dra, 0x101b8 + (0x24 + i) * 4, 0x800a0300);
        Put32(fixture->dra, 0x900 + i * 4, 0x800a0b00);
    }
    for (Uint32 i = 0; i < 9; ++i)
    {
        Put32(fixture->dra, 0x300 + i * 4, 2 | (i + 1) << 16);
        fixture->dra[0xb00 + i * 2 + 1] = i >= 2 && i < 5 ? 4 : 0;
    }
    Put32(fixture->dra, 0x324, 0xffff);
}

static bool TestAttacks(Fixture *fixture)
{
    const SotnCollisionMap map = MakeFixture(fixture);
    const SotnDataView dra = {fixture->dra, sizeof(fixture->dra)};
    AttackFixture(fixture);
    SotnPlayer *player = NULL;
    SotnPlayerState state = {0};
    CHECK(SotnPlayer_Create(dra, map, 64, 135, &player));
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_LEFT_HAND) && State(player, &state));
    CHECK(state.animation.id == 0x24 && !state.attack.active && state.weapon_starting &&
          !state.subweapon_starting);
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_LEFT_HAND | SOTN_BUTTON_JUMP));
    CHECK(State(player, &state) && state.step == SOTN_PLAYER_STAND);
    for (Uint32 i = 0; i < 4; ++i)
    {
        CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_LEFT_HAND) && State(player, &state));
        CHECK(state.weapon_starting == (i < 2) && !state.subweapon_starting);
    }
    CHECK(State(player, &state) && state.attack.active);
    CHECK(state.attack.offset_x == 18 && state.attack.offset_y == -16);
    CHECK(state.attack.width == 10 && state.attack.height == 7 && !state.attack.right_hand);
    for (Uint32 i = 0; i < 30; ++i)
        CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_LEFT_HAND));
    CHECK(State(player, &state) && state.animation.pose == 7 && !state.attack.active);
    for (Uint32 i = 0; i < 8; ++i)
        CHECK(SotnPlayer_Tick(player, 0));
    CHECK(State(player, &state) && state.animation.id < 0x24);
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_DOWN));
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT_HAND));
    CHECK(State(player, &state) && state.animation.id == 0x26 && state.attack.right_hand);
    for (Uint32 i = 0; i < 35; ++i)
        CHECK(SotnPlayer_Tick(player, 0));
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_JUMP));
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_JUMP | SOTN_BUTTON_LEFT_HAND));
    CHECK(State(player, &state) && state.animation.id == 0x28);
    for (Uint32 i = 0; i < 100; ++i)
        CHECK(SotnPlayer_Tick(player, 0));
    CHECK(State(player, &state) && state.step == SOTN_PLAYER_STAND && !state.attack.active);
    const Sint32 x = state.x;
    Put32(fixture->dra, 0xc0, 0xfffffffc);
    CHECK(!SotnPlayer_Tick(player, SOTN_BUTTON_LEFT_HAND));
    CHECK(State(player, &state) && state.x == x && !state.attack.active);
    SotnPlayer_Close(player);
    return true;
}

static bool TestDamage(Fixture *fixture)
{
    const SotnCollisionMap map = MakeFixture(fixture);
    for (Uint32 id = 0x31; id <= 0x34; ++id)
        Put32(fixture->dra, 0x101b8 + id * 4, 0x800a0200);
    Put32(fixture->dra, 0x101b8 + 0xc0 * 4, 0x800a0200);
    SotnPlayer *player = NULL;
    CHECK(SotnPlayer_Create((SotnDataView){fixture->dra, sizeof(fixture->dra)}, map, 64, 135,
                            &player));
    SotnPlayerState state = {0};
    CHECK(State(player, &state) && state.hp == 70 && !state.invulnerable);
    const Sint32 start_x = state.x, start_y = state.y;
    CHECK(SotnPlayer_Displace(player, 65536, -65536, 0x41));
    CHECK(State(player, &state) && state.x == start_x + 65536 && state.y == start_y - 65536 &&
          (state.contacts & 0x41) == 0x41);
    CHECK(!SotnPlayer_Displace(player, 65 * 65536, 0, 0));
    CHECK(!SotnPlayer_Displace(player, 0, 0, 0x8000));
    CHECK(State(player, &state) && state.x == start_x + 65536);
    CHECK(SotnPlayer_Displace(player, -65536, 65536, 0));
    CHECK(SotnPlayer_StageGuard(player, true));
    CHECK(State(player, &state) && state.invulnerable && state.velocity_y == 0);
    CHECK(SotnPlayer_Damage(player, 3, 80, 0));
    CHECK(State(player, &state) && state.hp == 70);
    for (Uint32 i = 0; i < 32; ++i)
        CHECK(SotnPlayer_Tick(player, 0));
    CHECK(State(player, &state) && !state.invulnerable);
    CHECK(SotnPlayer_Damage(player, 3, 80, 0));
    CHECK(State(player, &state) && state.hp == 67 && state.invulnerable);
    CHECK(state.step == SOTN_PLAYER_HIT && state.velocity_x == -163840);
    CHECK(SotnPlayer_Damage(player, 3, 80, 0));
    CHECK(State(player, &state) && state.hp == 67);
    for (Uint32 i = 0; i < 30; ++i)
        CHECK(SotnPlayer_Tick(player, 0));
    CHECK(State(player, &state) && state.step == SOTN_PLAYER_STAND && !state.invulnerable);
    CHECK(!SotnPlayer_Damage(player, 10, SDL_MIN_SINT32, 0));
    CHECK(State(player, &state) && state.hp == 67);
    CHECK(SotnPlayer_Restore(player, 67, 12345));
    CHECK(State(player, &state) && state.hp == 67 && state.random == 12345);
    CHECK(!SotnPlayer_Restore(player, 71, 0));
    CHECK(!SotnPlayer_Restore(player, 0, 0));
    CHECK(State(player, &state) && state.hp == 67 && state.random == 12345);
    Put32(fixture->dra, 0x101b8 + 0xc0 * 4, 0xfffffffc);
    CHECK(!SotnPlayer_Damage(player, 67, 80, 0));
    CHECK(State(player, &state) && state.hp == 67);
    Put32(fixture->dra, 0x101b8 + 0xc0 * 4, 0x800a0200);
    CHECK(SotnPlayer_Damage(player, 67, 80, 0));
    CHECK(State(player, &state) && state.hp == 0 && state.step == SOTN_PLAYER_KILL &&
          (state.status & 0x40000U) && state.invulnerable);
    CHECK(!SotnPlayer_SetScriptHold(player, true));
    CHECK(SotnPlayer_SetScriptHold(player, false));
    for (Uint32 tick = 0; tick < 300; ++tick)
        CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_JUMP | SOTN_BUTTON_RIGHT));
    CHECK(State(player, &state) && state.death.phase == 128);
    for (Uint32 i = 0; i < 32; ++i)
        CHECK(state.death.mask[i] == 255);
    CHECK(SotnPlayer_Damage(player, 10, 80, 0));
    CHECK(!SotnPlayer_Restore(player, 70, 0));
    SotnPlayer_Close(player);
    return true;
}

static bool TestStageStop(Fixture *fixture)
{
    const SotnCollisionMap map = MakeFixture(fixture);
    SotnPlayer *player = NULL;
    CHECK(SotnPlayer_Create((SotnDataView){fixture->dra, sizeof(fixture->dra)}, map, 64, 135,
                            &player));
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_RIGHT));
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_RIGHT | SOTN_BUTTON_JUMP));
    SotnPlayerState before = {0}, after = {0};
    CHECK(State(player, &before) && before.velocity_x != 0 && before.velocity_y != 0);
    CHECK(SotnPlayer_StageStopHorizontal(player) && State(player, &after));
    CHECK(after.velocity_x == 0 && after.velocity_y == before.velocity_y && before.x == after.x &&
          before.y == after.y && before.step == after.step &&
          before.animation.sprite == after.animation.sprite && before.hp == after.hp);
    before = after;
    CHECK(SotnPlayer_StageStop(player) && State(player, &after));
    CHECK(after.velocity_x == 0 && after.velocity_y == 0 && before.x == after.x &&
          before.y == after.y && before.step == after.step &&
          before.animation.sprite == after.animation.sprite && before.hp == after.hp &&
          before.invulnerable == after.invulnerable);
    CHECK(!SotnPlayer_StageStop(NULL));
    CHECK(SotnPlayer_Damage(player, 70, 80, 0) && State(player, &before) &&
          before.step == SOTN_PLAYER_KILL && before.velocity_x != 0);
    CHECK(SotnPlayer_StageStopHorizontal(player) && State(player, &after));
    CHECK(!after.velocity_x && after.velocity_y == before.velocity_y && after.step == before.step &&
          after.death.phase == before.death.phase && after.x == before.x && after.y == before.y &&
          !after.hp);
    CHECK(!SotnPlayer_StageStopHorizontal(NULL));
    SotnPlayer_Close(player);
    return true;
}

static bool TestStone(Fixture *fixture)
{
    const SotnCollisionMap map = MakeFixture(fixture);
    const SotnDataView dra = {fixture->dra, sizeof(fixture->dra)};
    for (Uint16 id = 0x38; id <= 0x3e; ++id)
        Put32(fixture->dra, 0x101b8 + (size_t)id * 4, 0x800a0200);
    SotnPlayer *player = NULL;
    SotnPlayerState state = {0};
    CHECK(SotnPlayer_Create(dra, map, 256, 135, &player));
    CHECK(State(player, &state));
    const Uint32 initial_random = state.random;
    for (Uint16 id = 0x2e; id <= 0x30; ++id)
        Put32(fixture->dra, 0x101b8 + (size_t)id * 4, 0);
    CHECK(!SotnPlayer_Petrify(player, 5, 128, 0));
    CHECK(State(player, &state) && state.hp == 70 && state.random == initial_random &&
          state.step == SOTN_PLAYER_STAND && !state.stone.active);
    for (Uint16 id = 0x2e; id <= 0x30; ++id)
        Put32(fixture->dra, 0x101b8 + (size_t)id * 4, 0x800a0100);
    CHECK(!SotnPlayer_Petrify(NULL, 0, 128, 0));
    CHECK(SotnPlayer_Petrify(player, 0, 128, 0) && State(player, &state));
    CHECK(state.step == SOTN_PLAYER_STONE && state.hp == 70 && state.stone.active &&
          (state.status & 0x80U) && state.stone.shakes >= 4 && state.stone.shakes <= 8 &&
          !state.can_pause && !state.attack.active && !state.shield.active);
    CHECK(SotnPlayer_Petrify(player, 10, 128, 0) && State(player, &state) && state.hp == 70);
    for (Uint32 i = 0; i < 100; ++i)
        CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_LEFT_HAND | SOTN_BUTTON_JUMP));
    CHECK(State(player, &state) && state.step == SOTN_PLAYER_STONE && (state.contacts & 1) &&
          !state.invulnerable && state.stone.palette >= 0x19e);
    const Uint16 before_impact = state.stone.shakes;
    CHECK(SotnPlayer_Damage(player, 8, 128, 0));
    for (Uint32 i = 0; i < 64; ++i)
        CHECK(SotnPlayer_Tick(player, 0));
    CHECK(State(player, &state) && state.hp == 62 && state.stone.shakes == before_impact - 1);
    const Uint16 shakes = state.stone.shakes;
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_UP));
    for (Uint32 i = 0; i < 64; ++i)
        CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_UP));
    CHECK(State(player, &state) && state.step == SOTN_PLAYER_STONE &&
          state.stone.shakes == shakes - 1);
    // A held direction cannot free Alucard; distinct taps complete the selected escape.
    for (Uint32 i = 0; i < 300; ++i)
    {
        const Uint16 buttons = (i % 24) == 0 ? SOTN_BUTTON_LEFT : 0;
        CHECK(SotnPlayer_Tick(player, buttons));
    }
    CHECK(State(player, &state) && state.step == SOTN_PLAYER_STAND && !state.stone.active &&
          state.hp == 62);
    const Sint32 x = state.x;
    CHECK(!SotnPlayer_Petrify(player, 10000, 128, 0));
    CHECK(State(player, &state) && state.x == x && state.hp == 62 && !state.stone.active);
    // A lethal newly petrifying hit lands, waits and reaches the existing game-over signal.
    CHECK(SotnPlayer_Petrify(player, 70, 128, 0));
    bool grounded_stone = false, stone_death = false;
    for (Uint32 i = 0; i < 500; ++i)
    {
        CHECK(SotnPlayer_Tick(player, 0) && State(player, &state));
        grounded_stone =
            grounded_stone || (state.step == SOTN_PLAYER_STONE && (state.contacts & 1));
        stone_death = stone_death || state.death.phase == 17;
        if (state.death.phase == 128)
            break;
    }
    CHECK(grounded_stone && stone_death && state.hp == 0 && state.stone.hidden &&
          state.death.phase == 128 && state.step == SOTN_PLAYER_KILL);
    SotnPlayer_Close(player);
    return true;
}

static bool TestScriptHold(Fixture *fixture)
{
    const SotnCollisionMap map = MakeFixture(fixture);
    SotnPlayer *player = NULL;
    CHECK(SotnPlayer_Create((SotnDataView){fixture->dra, sizeof(fixture->dra)}, map, 64, 135,
                            &player));
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_RIGHT));
    SotnPlayerState before = {0}, held = {0};
    CHECK(State(player, &before) && before.step == SOTN_PLAYER_WALK);
    CHECK(SotnPlayer_SetScriptHold(player, true));
    for (Uint16 tick = 0; tick < 30; ++tick)
    {
        CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_LEFT | SOTN_BUTTON_JUMP | SOTN_BUTTON_LEFT_HAND));
        CHECK(State(player, &held) && held.step == SOTN_PLAYER_SCRIPT && held.x == before.x &&
              held.y == before.y);
        CHECK(!held.velocity_x && !held.velocity_y && held.facing_left == before.facing_left);
        CHECK(held.animation.id == before.animation.id && held.invulnerable && !held.can_pause &&
              !held.attack.active);
        CHECK((held.status & 0x38110000U) == 0x38110000U);
    }
    CHECK(SotnPlayer_SetScriptHold(player, false) && SotnPlayer_Tick(player, 0));
    CHECK(State(player, &held) && held.step == SOTN_PLAYER_STAND && held.can_pause &&
          held.animation.id == 4);
    for (Uint16 tick = 0; tick < 12; ++tick)
        CHECK(SotnPlayer_Tick(player, 0));
    CHECK(State(player, &held) && !held.invulnerable);
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_JUMP));
    CHECK(State(player, &before) && before.step == SOTN_PLAYER_JUMP);
    CHECK(SotnPlayer_SetScriptHold(player, true));
    for (Uint16 tick = 0; tick < 20; ++tick)
        CHECK(SotnPlayer_Tick(player, 0));
    CHECK(State(player, &held) && held.y == before.y && !held.velocity_y);
    CHECK(SotnPlayer_SetScriptHold(player, false) && SotnPlayer_Tick(player, 0) &&
          SotnPlayer_Tick(player, 0));
    CHECK(State(player, &held) && held.step == SOTN_PLAYER_FALL);
    CHECK(!SotnPlayer_SetScriptHold(NULL, true));
    SotnPlayer_Close(player);
    return true;
}

static bool TestFailures(Fixture *fixture)
{
    SotnCollisionMap map = MakeFixture(fixture);
    const SotnDataView dra = {fixture->dra, sizeof(fixture->dra)};
    SotnPlayer *player = NULL;
    CHECK(!SotnPlayer_Create(dra, map, SDL_MAX_SINT32, 0, &player) && player == NULL);
    CHECK(!SotnPlayer_Create((SotnDataView){0}, map, 64, 135, &player) && player == NULL);
    map.width = SDL_MAX_UINT32;
    CHECK(!SotnPlayer_Create(dra, map, 0, 0, &player) && player == NULL);
    CHECK(!SotnPlayer_Tick(NULL, 0));
    CHECK(!SotnPlayer_GetState(NULL, NULL));
    SotnPlayer_Close(NULL);
    return true;
}

static bool TestBellInside(Fixture *fixture)
{
    const SotnCollisionMap map = MakeFixture(fixture);
    AttackFixture(fixture);
    SotnPlayer *p = NULL;
    CHECK(SotnPlayer_Create((SotnDataView){fixture->dra, sizeof(fixture->dra)}, map, 64, 135, &p));
    CHECK(SotnPlayer_Tick(p, 0));
    CHECK(SotnPlayer_BellInside(p, true) && SotnPlayer_StageGuard(p, false));
    SotnPlayerState state = {0};
    CHECK(State(p, &state) && (state.status & 0x100008U) == 0x100008U && state.invulnerable);
    CHECK(SotnPlayer_Tick(p, SOTN_BUTTON_RIGHT | SOTN_BUTTON_LEFT_HAND));
    CHECK(State(p, &state) && !state.attack.active && !state.weapon_starting &&
          state.velocity_x > 0 && !(state.status & 0x100008U));
    CHECK(SotnPlayer_BellInside(p, true) && SotnPlayer_BellInside(p, false));
    CHECK(SotnPlayer_Tick(p, 0) && SotnPlayer_Tick(p, SOTN_BUTTON_LEFT_HAND));
    CHECK(State(p, &state) && state.weapon_starting);
    for (Uint16 i = 0; i < 5; ++i)
        CHECK(SotnPlayer_Tick(p, SOTN_BUTTON_LEFT_HAND));
    CHECK(State(p, &state) && state.attack.active);
    CHECK(!SotnPlayer_BellInside(NULL, true));
    SotnPlayer_Close(p);
    return true;
}

static bool TestChairs(Fixture *f)
{
    const SotnCollisionMap map = MakeFixture(f);
    AttackFixture(f);
    Put32(f->dra, 0x101b8 + 0xf2 * 4, 0x800a0200);
    Put32(f->dra, 0x101b8 + 0xf3 * 4, 0x800a0200);
    // Synthetic sleep sprites, not a copy of the game's animation records.
    for (Uint32 i = 0; i < 4; ++i)
        Put32(f->dra, 0xe294 + i * 4, 3U | ((i + 9U) << 16));
    Put32(f->dra, 0xe2a4, 0xffff);
    for (Uint16 kind = 1; kind <= 4; ++kind)
    {
        SotnPlayer *p = NULL;
        SotnPlayerState state = {0};
        CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)}, map, 64, 135, &p));
        CHECK(SotnPlayer_Tick(p, 0) && SotnPlayer_SetChairKind(p, kind));
        CHECK(SotnPlayer_Tick(p, SOTN_BUTTON_UP) && State(p, &state));
        CHECK(state.step == SOTN_PLAYER_STAND && state.substep == 4 &&
              state.animation.id == (kind <= 2 ? 0xf2 : 0xf3) &&
              state.facing_left == ((kind & 1U) == 0));
        for (Uint16 i = 0; i < 12; ++i)
            CHECK(SotnPlayer_Tick(p, 0));
        CHECK(State(p, &state) && state.substep == 4 && state.animation.remaining == -1);
        for (Uint16 i = 12; i < 0x1800; ++i)
            CHECK(SotnPlayer_Tick(p, 0));
        CHECK(State(p, &state) && state.sleep_event == (kind <= 2) &&
              state.sleep.active == (kind <= 2));
        if (kind <= 2)
        {
            CHECK(state.sleep.frame == 9 &&
                  state.sleep.x == state.x + (kind == 2 ? -8 : 8) * 65536 &&
                  state.sleep.y == state.y - 16 * 65536);
            const Sint32 letter_x = state.sleep.x;
            CHECK(SotnPlayer_Tick(p, 0) && State(p, &state) && !state.sleep_event &&
                  state.sleep.x == letter_x + (kind == 2 ? -16384 : 16384));
            for (Uint16 i = 1; i < 0x200; ++i)
                CHECK(SotnPlayer_Tick(p, 0));
            CHECK(State(p, &state) && state.sleep_event && state.sleep.active);
        }
        CHECK(SotnPlayer_Tick(p, SOTN_BUTTON_JUMP) && State(p, &state));
        CHECK(state.step == SOTN_PLAYER_JUMP && !state.sleep.active);
        for (Uint16 i = 0; i < 90; ++i)
            CHECK(SotnPlayer_Tick(p, 0));
        // The actor hint is consumed once; looking up elsewhere does not reseat the player.
        CHECK(SotnPlayer_Tick(p, SOTN_BUTTON_UP) && State(p, &state) && state.substep == 1);
        CHECK(SotnPlayer_SetChairKind(p, kind) && SotnPlayer_Tick(p, SOTN_BUTTON_UP));
        CHECK(SotnPlayer_Tick(p, SOTN_BUTTON_RIGHT) && State(p, &state) &&
              state.step == SOTN_PLAYER_WALK);
        CHECK(!SotnPlayer_SetChairKind(p, 5) && !SotnPlayer_SetChairKind(NULL, 1));
        SotnPlayer_Close(p);
    }
    return true;
}
static bool TestScriptAlignment(Fixture *f)
{
    const SotnCollisionMap map = MakeFixture(f);
    SotnPlayer *p = NULL;
    SotnPlayerState before = {0}, after = {0};
    CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)}, map, 64, 135, &p));
    CHECK(SotnPlayer_Tick(p, 0) && SotnPlayer_Tick(p, SOTN_BUTTON_RIGHT) && State(p, &before));
    CHECK(SotnPlayer_StageAlignX(p, 368) && State(p, &after));
    CHECK(after.x == 368 * 65536 + (Sint32)((Uint32)before.x & 65535U) &&
          after.velocity_x == before.velocity_x && after.step == before.step &&
          after.hp == before.hp);
    CHECK(!SotnPlayer_StageAlignX(p, SDL_MAX_SINT32) && State(p, &before) && before.x == after.x);
    CHECK(!SotnPlayer_StageAlignX(NULL, 368));
    SotnPlayer_Close(p);
    return true;
}

static bool Grab(SotnPlayer *p, SotnPlayerGrabState state, Uint16 damage)
{
    return SotnPlayer_RequestGrab(
        p, (SotnPlayerGrab){
               .state = state, .damage = damage, .source_x = 64, .source_velocity = -65536});
}

static bool TestBossGrab(Fixture *f)
{
    const SotnCollisionMap map = MakeFixture(f);
    AttackFixture(f);
    Put32(f->dra, 0x101b8 + 0xc0 * 4, 0x800a0200);
    SotnPlayer *p = NULL;
    SotnPlayerState before = {0}, held = {0};
    CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)}, map, 64, 135, &p));
    CHECK(SotnPlayer_Tick(p, 0) && SotnPlayer_Tick(p, SOTN_BUTTON_JUMP) &&
          SotnPlayer_Tick(p, SOTN_BUTTON_LEFT_HAND));
    CHECK(State(p, &before) && before.weapon_starting && before.step == SOTN_PLAYER_JUMP);
    CHECK(SotnPlayer_StageGuard(p, false));
    CHECK(Grab(p, SOTN_GRAB_START, 16));
    CHECK(State(p, &held) && held.grab.state == SOTN_GRAB_START && !held.can_pause);
    CHECK(!Grab(p, SOTN_GRAB_START, 16) && !Grab(p, SOTN_GRAB_DAMAGE, 16));
    CHECK(SotnPlayer_Tick(p, SOTN_BUTTON_LEFT | SOTN_BUTTON_JUMP | SOTN_BUTTON_LEFT_HAND));
    CHECK(State(p, &held) && held.step == SOTN_PLAYER_BOSS_GRAB && held.substep == 1 &&
          held.grab.state == SOTN_GRAB_HELD && held.animation.id == 0x37 && held.sound == 0x6eb &&
          held.x == before.x && held.y == before.y && !held.velocity_x && !held.velocity_y &&
          !held.attack.active && !held.shield.active && !held.can_pause && held.invulnerable &&
          held.random == before.random && held.hp == 70 && !held.facing_left &&
          (held.status & 0x38110040U) == 0x38110040U);
    CHECK(!SotnPlayer_SetScriptHold(p, true));
    // The boss owns whole-pixel position while held. Ordinary collision must not eject Alucard
    // from this synthetic solid floor, and stage alignment must retain each fractional part.
    CHECK(SotnPlayer_StageAlignX(p, 200) && SotnPlayer_StageAlignY(p, 180));
    CHECK(State(p, &before));
    CHECK(before.x == 200 * 65536 && before.y == 180 * 65536 + (Sint32)((Uint32)held.y & 65535U));
    for (Uint16 tick = 0; tick < 48; ++tick)
    {
        CHECK(SotnPlayer_Tick(p, SOTN_BUTTON_LEFT | SOTN_BUTTON_JUMP | SOTN_BUTTON_RIGHT_HAND));
        CHECK(State(p, &held) && held.x == before.x && held.y == before.y && held.hp == 70 &&
              held.grab.state == SOTN_GRAB_HELD && !held.attack.active && !held.shield.active &&
              !held.can_pause && held.invulnerable && !held.velocity_x && !held.velocity_y);
    }
    CHECK(SotnPlayer_Damage(p, 70, 64, 0) && SotnPlayer_Petrify(p, 70, 64, 0));
    CHECK(State(p, &held) && held.hp == 70 && held.step == SOTN_PLAYER_BOSS_GRAB);
    const SotnCharacterState profile = {.hp = 70, .hp_max = 70};
    CHECK(SotnPlayer_SetCharacter(p, &profile, (SotnCharacterStats){.defense = 3}));
    CHECK(Grab(p, SOTN_GRAB_DAMAGE, 16) && !Grab(p, SOTN_GRAB_DAMAGE, 16));
    CHECK(SotnPlayer_Tick(p, 0) && State(p, &held));
    CHECK(held.hp == 57 && held.sound == 0x6e7 && held.grab.state == SOTN_GRAB_HELD &&
          held.random == before.random && held.animation.id == 0x37);
    CHECK(SotnPlayer_RestorePoison(p, 100));
    CHECK(Grab(p, SOTN_GRAB_DAMAGE, 16) && SotnPlayer_Tick(p, 0) && State(p, &held));
    CHECK(held.hp == 31 && held.poison_ticks == 99);
    CHECK(Grab(p, SOTN_GRAB_DAMAGE, 0) && SotnPlayer_Tick(p, 0) && State(p, &held));
    CHECK(held.hp == 30 && held.grab.state == SOTN_GRAB_HELD);
    CHECK(SotnPlayer_StageAlignY(p, 90) && State(p, &before));
    CHECK(Grab(p, SOTN_GRAB_NONE, 0) && SotnPlayer_Tick(p, 0) && State(p, &held));
    CHECK(held.step == SOTN_PLAYER_HIT && held.substep == 1 && held.y == before.y &&
          held.grab.state == SOTN_GRAB_NONE && held.hp == 30 && !held.can_pause);
    for (Uint16 tick = 0; tick < 160; ++tick)
        CHECK(SotnPlayer_Tick(p, 0));
    CHECK(State(p, &held) && held.step == SOTN_PLAYER_STAND && held.can_pause &&
          held.y == 135 * 65536 && !held.invulnerable && held.hp == 30);
    // A second capture can kill while held and must reach the existing physical game-over signal.
    CHECK(Grab(p, SOTN_GRAB_START, 16) && SotnPlayer_Tick(p, 0));
    CHECK(Grab(p, SOTN_GRAB_DAMAGE, 9999) && SotnPlayer_Tick(p, 0) && State(p, &held));
    CHECK(!held.hp && held.step == SOTN_PLAYER_KILL && held.death.phase == 1 &&
          held.sound == 0x6f6 && held.velocity_x > 0 && !held.can_pause);
    CHECK(!Grab(p, SOTN_GRAB_START, 16) && Grab(p, SOTN_GRAB_NONE, 0));
    for (Uint16 tick = 0; tick < 500 && held.death.phase != 128; ++tick)
        CHECK(SotnPlayer_Tick(p, 0) && State(p, &held));
    CHECK(held.death.phase == 128 && held.grab.state == SOTN_GRAB_NONE);
    SotnPlayer_Close(p);
    return true;
}

static bool TestBossGrabFailures(Fixture *f)
{
    const SotnCollisionMap map = MakeFixture(f);
    SotnPlayer *p = NULL;
    SotnPlayerState before = {0}, after = {0};
    CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)}, map, 64, 135, &p));
    CHECK(!SotnPlayer_RequestGrab(NULL, (SotnPlayerGrab){0}));
    CHECK(!Grab(p, SOTN_GRAB_HELD, 16) && !Grab(p, (SotnPlayerGrabState)4, 16) &&
          !Grab(p, SOTN_GRAB_DAMAGE, 16) && !Grab(p, SOTN_GRAB_START, 10000));
    CHECK(!SotnPlayer_RequestGrab(
        p, (SotnPlayerGrab){.state = SOTN_GRAB_START, .source_velocity = SDL_MIN_SINT32}));
    CHECK(!SotnPlayer_RequestGrab(
        p, (SotnPlayerGrab){.state = SOTN_GRAB_START, .source_x = SDL_MAX_SINT32}));
    CHECK(SotnPlayer_SetScriptHold(p, true) && !Grab(p, SOTN_GRAB_START, 16));
    CHECK(SotnPlayer_SetScriptHold(p, false));
    CHECK(Grab(p, SOTN_GRAB_START, 16) && Grab(p, SOTN_GRAB_NONE, 0));
    CHECK(SotnPlayer_Tick(p, 0) && State(p, &after) && after.step == SOTN_PLAYER_STAND);
    CHECK(Grab(p, SOTN_GRAB_START, 16) && State(p, &before));
    CHECK(SotnPlayer_Damage(p, 12, 64, 0) && SotnPlayer_Petrify(p, 12, 64, 0) &&
          SotnPlayer_CurseDamage(p, 12, 64, 0) && SotnPlayer_PoisonDamage(p, 12, 64, 0));
    CHECK(State(p, &after) && after.hp == 70 && !after.curse_ticks && !after.poison_ticks &&
          after.step == before.step && after.grab.state == SOTN_GRAB_START);
    Put32(f->dra, 0x101b8 + 0x37 * 4, 0x800b1000);
    CHECK(!SotnPlayer_Tick(p, 0) && State(p, &after));
    CHECK(after.grab.state == SOTN_GRAB_START && after.step == before.step && after.x == before.x &&
          after.y == before.y && after.hp == before.hp && after.random == before.random &&
          after.animation.id == before.animation.id);
    Put32(f->dra, 0x101b8 + 0x37 * 4, 0x800a0100);
    CHECK(SotnPlayer_Tick(p, 0) && State(p, &before));
    CHECK(Grab(p, SOTN_GRAB_DAMAGE, 16));
    Put32(f->dra, 0x101b8 + 0x37 * 4, 0x800b1000);
    CHECK(!SotnPlayer_Tick(p, 0) && State(p, &after));
    CHECK(after.hp == before.hp && after.grab.state == SOTN_GRAB_DAMAGE &&
          after.animation.pose == before.animation.pose && after.sound == before.sound &&
          after.random == before.random);
    Put32(f->dra, 0x101b8 + 0x37 * 4, 0x800a0100);
    CHECK(SotnPlayer_Tick(p, 0) && State(p, &before) && before.hp == 54 &&
          before.grab.state == SOTN_GRAB_HELD);
    CHECK(!SotnPlayer_StageAlignY(p, SDL_MIN_SINT32) &&
          !SotnPlayer_StageAlignY(p, SDL_MAX_SINT32) && !SotnPlayer_StageAlignY(NULL, 0));
    CHECK(State(p, &after) && after.x == before.x && after.y == before.y);
    SotnPlayer_Close(p);
    return true;
}

int main(void)
{
    Fixture *fixture = SDL_calloc(1, sizeof(*fixture));
    if (fixture == NULL)
        return 1;
    const bool passed = TestMovement(fixture) && TestWallsAndReplay(fixture) &&
                        TestAttacks(fixture) && TestDamage(fixture) && TestStageStop(fixture) &&
                        TestStone(fixture) && TestScriptHold(fixture) && TestBellInside(fixture) &&
                        TestChairs(fixture) && TestScriptAlignment(fixture) &&
                        TestBossGrab(fixture) && TestBossGrabFailures(fixture) &&
                        TestFailures(fixture);
    SDL_free(fixture);
    if (!passed)
        return 1;
    SDL_Log("SOTN_PLAYER_TEST: PASS");
    return 0;
}
