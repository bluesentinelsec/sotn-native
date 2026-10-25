#include "character_fixture.h"
#include <sotn/subweapon.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
static void Definition(Uint8 *bytes, Uint16 id, Uint8 cost, Uint8 chain)
{
    Uint8 *p = bytes + 0x4a00 + id * 20;
    p[0] = 7;
    p[2] = cost;
    p[4] = 0x40;
    p[6] = chain;
    p[7] = 4;
    p[8] = 5;
    p[10] = 0x5d;
    p[12] = 2;
    p[14] = 3;
}
static bool Select(SotnCharacter *c, Uint16 id, Uint16 hearts)
{
    SotnCharacterState state = {0};
    if (!SotnCharacter_Get(c, &state, NULL))
        return false;
    state.subweapon = id;
    state.hearts = hearts;
    state.hearts_max = 200;
    return SotnCharacter_Restore(c, &state);
}
int main(void)
{
    Uint8 *bytes = SDL_calloc(1, 0x11000);
    CHECK(bytes != NULL);
    TestCharacter_Fill(bytes);
    TestCharacter_Progression(bytes);
    Definition(bytes, 1, 3, 2);
    Definition(bytes, 2, 7, 2);
    Definition(bytes, 3, 5, 2);
    Definition(bytes, 6, 20, 1);
    Definition(bytes, 11, 1, 8);
    Uint8 tiles[32 * 16 * 2] = {0}, types[2] = {0, 3};
    for (Uint32 y = 10; y < 16; ++y)
        for (Uint32 x = 0; x < 32; ++x)
            tiles[(y * 32 + x) * 2] = 1;
    SotnCollisionMap map = {{tiles, sizeof(tiles)}, {types, sizeof(types)}, 32, 16};
    SotnCharacter *character = NULL;
    SotnSubweapons *s = NULL;
    CHECK(SotnCharacter_Create((SotnDataView){bytes, 0x11000}, &character));
    CHECK(SotnSubweapons_Create((SotnDataView){bytes, 0x11000}, character, map, &s));
    SotnPlayerState player = {
        .x = 64 * 65536, .y = 135 * 65536, .hp = 70, .hp_max = 70, .step = SOTN_PLAYER_STAND};
    bool used = false;
    Uint16 animation = 0;
    SotnCharacterState state = {0};
    SotnSubweaponState p = {0};
    CHECK(Select(character, 1, 2));
    CHECK(SotnSubweapons_Use(s, player, &used, &animation) && !used);
    CHECK(SotnCharacter_Get(character, &state, NULL) && state.hearts == 2 && state.random == 0);
    CHECK(Select(character, 1, 20));
    CHECK(SotnSubweapons_Use(s, player, &used, &animation) && used && animation == 0x5d);
    CHECK(SotnSubweapons_Get(s, 0, &p) && p.attack.power >= 8 && p.y == 126 * 65536);
    CHECK(SotnSubweapons_Tick(s, player, 0, 0) && SotnSubweapons_Get(s, 0, &p) &&
          p.x == 64 * 65536);
    CHECK(SotnSubweapons_Tick(s, player, 0, 0) && SotnSubweapons_Get(s, 0, &p) &&
          p.x == 72 * 65536);
    CHECK(SotnSubweapons_Use(s, player, &used, &animation) && used);
    CHECK(SotnCharacter_Get(character, &state, NULL));
    const Uint32 random = state.random;
    CHECK(SotnSubweapons_Use(s, player, &used, &animation) && !used);
    CHECK(SotnCharacter_Get(character, &state, NULL) && state.hearts == 14 &&
          state.random == random);
    CHECK(SotnSubweapons_Hit(s, 0, 1));
    CHECK(SotnSubweapons_Tick(s, player, 0, 0) && SotnSubweapons_Get(s, 0, &p) && p.phase == 3 &&
          !p.attack.active);
    for (Uint32 i = 0; i < 4; ++i)
        CHECK(SotnSubweapons_Tick(s, player, 0, 0));
    CHECK(SotnSubweapons_Get(s, 0, &p) && p.generation == 0);
    CHECK(SotnSubweapons_Enter(s, map) && Select(character, 2, 20));
    player.facing_left = true;
    CHECK(SotnSubweapons_Use(s, player, &used, &animation) && used);
    CHECK(SotnSubweapons_Tick(s, player, 0, 0) && SotnSubweapons_Tick(s, player, 0, 0));
    CHECK(SotnSubweapons_Get(s, 0, &p) && p.x == 62 * 65536 && p.velocity_y == -6 * 65536 + 17408);
    CHECK(SotnSubweapons_Hit(s, 0, 2) && SotnSubweapons_Tick(s, player, 0, 0));
    CHECK(SotnSubweapons_Get(s, 0, &p) && p.phase == 2 && !p.attack.active &&
          p.velocity_x == 65536);
    CHECK(SotnSubweapons_Enter(s, map) && Select(character, 3, 20));
    player.facing_left = false;
    CHECK(SotnSubweapons_Use(s, player, &used, &animation) && used);
    bool flame = false, active = false;
    for (Uint32 t = 0; t < 150; ++t)
    {
        CHECK(SotnSubweapons_Tick(s, player, 0, 0));
        for (Uint32 i = 0; i < 16; ++i)
        {
            CHECK(SotnSubweapons_Get(s, i, &p));
            if (p.generation && p.id == 11)
            {
                flame = true;
                active |= p.attack.active;
            }
        }
    }
    CHECK(flame && active);
    CHECK(SotnSubweapons_Enter(s, map) && Select(character, 6, 50));
    CHECK(SotnSubweapons_Use(s, player, &used, &animation) && used && SotnSubweapons_Frozen(s));
    CHECK(SotnSubweapons_Use(s, player, &used, &animation) && !used);
    Uint32 duration = 0;
    while (SotnSubweapons_Frozen(s) && duration < 600)
    {
        CHECK(SotnSubweapons_Tick(s, player, 0, 0));
        ++duration;
        Uint16 sounds[64] = {0}, count = 0;
        CHECK(SotnSubweapons_Sounds(s, sounds, 64, &count));
    }
    CHECK(duration == 533 && !SotnSubweapons_Frozen(s));
    CHECK(SotnCharacter_Get(character, &state, NULL) && state.hearts == 30);
    CHECK(SotnSubweapons_Enter(s, map) && !SotnSubweapons_Frozen(s));
    CHECK(Select(character, 4, 150));
    CHECK(SotnSubweapons_Use(s, player, &used, &animation) && !used);
    CHECK(SotnCharacter_Get(character, &state, NULL) && state.hearts == 150);
    // Heart broach discounts both rings, with the original minimum-one cost.
    bytes[0x3400 + 77 * 32 + 28] = 3;
    state.subweapon = 1;
    state.equipment[5] = 77;
    state.equipment[6] = 77;
    CHECK(SotnCharacter_Restore(character, &state));
    CHECK(SotnSubweapons_Use(s, player, &used, &animation) && used);
    CHECK(SotnCharacter_Get(character, &state, NULL) && state.hearts == 149);
    CHECK(!SotnSubweapons_Get(s, 16, &p));
    CHECK(!SotnSubweapons_Hit(s, 0, 4));
    // A throw animation can transition back to an idle animation without locking controls.
    CHECK(SotnSubweapons_Enter(s, map) && Select(character, 1, 50));
    for (Uint32 i = 0; i < 247; ++i)
        TestCharacter_Put32(bytes + 0x101b8 + i * 4, 0x800a9000);
    TestCharacter_Put32(bytes + 0x9000, 2 | (1U << 16));
    TestCharacter_Put32(bytes + 0x9004, 0xfffe | (3U << 16));
    TestCharacter_Put32(bytes + 0x101b8 + 3 * 4, 0x800a9100);
    TestCharacter_Put32(bytes + 0x9100, 255 | (1U << 16));
    TestCharacter_Put32(bytes + 0x9104, 0);
    SotnPlayer *alucard = NULL;
    CHECK(SotnPlayer_Create((SotnDataView){bytes, 0x11000}, map, 64, 135, &alucard));
    CHECK(SotnPlayer_SetSubweapons(alucard, s));
    for (Uint32 t = 0; t < 60; ++t)
    {
        CHECK(SotnPlayer_Tick(
            alucard, (Uint16)(SOTN_BUTTON_UP | (t % 20 == 0 ? SOTN_BUTTON_LEFT_HAND : 0))));
        CHECK(SotnPlayer_GetState(alucard, &player));
        if (t == 0)
            CHECK(player.subweapon_starting);
        if (t == 4)
            CHECK(!player.subweapon_starting);
        CHECK(SotnSubweapons_Tick(s, player, 0, 0));
    }
    CHECK(SotnCharacter_Get(character, &state, NULL) && state.hearts <= 47);
    CHECK(SotnPlayer_Tick(alucard, SOTN_BUTTON_RIGHT));
    CHECK(SotnPlayer_GetState(alucard, &player) && player.step == SOTN_PLAYER_WALK);
    SotnPlayer_Close(alucard);
    SotnSubweapons_Destroy(s);
    SotnCharacter_Destroy(character);
    SDL_free(bytes);
    SDL_Log("SOTN_SUBWEAPON_TEST: PASS");
    return 0;
}
