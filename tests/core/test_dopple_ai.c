#include <SDL3/SDL.h>
#include <sotn/dopple_ai.h>

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

static bool Tests(void)
{
    SotnDoppleAI ai = {0};
    SotnDoppleObservation at = {.boss_x = 200, .world_x = 200, .step = 4};
    Uint32 random = 0;
    CHECK(SotnDoppleAI_Init(&ai) && ai.step == 18);
    CHECK(SotnDoppleAI_Tick(&at, &random, &ai) && ai.step == 18 && ai.buttons == 0);
    at.step = 1;
    CHECK(SotnDoppleAI_Tick(&at, &random, &ai) && ai.step == 19 && ai.buttons == 0x10 &&
          ai.attack_timer == 64);
    for (size_t i = 0; i < 64; ++i)
        CHECK(SotnDoppleAI_Tick(&at, &random, &ai));
    CHECK(ai.step == 0 && random == 0 && ai.buttons == 0);
    CHECK(SotnDoppleAI_Tick(&at, &random, &ai) && ai.buttons == 0x8000);
    at.boss_x = 64;
    at.facing_left = true;
    CHECK(SotnDoppleAI_Tick(&at, &random, &ai) && ai.step == 1 && ai.timer == 16);
    for (size_t i = 0; i < 16; ++i)
        CHECK(SotnDoppleAI_Tick(&at, &random, &ai));
    CHECK(ai.buttons == 0x1080 && ai.timer == 112 && ai.attack_timer == 32 && random == 0);
    at.boss_x = 32;
    for (size_t i = 0; i < 32; ++i)
    {
        CHECK(SotnDoppleAI_Tick(&at, &random, &ai));
        CHECK(ai.buttons == 0);
    }
    CHECK(SotnDoppleAI_Tick(&at, &random, &ai) && ai.buttons == 0x80 && ai.attack_timer == 32);
    at.world_x = 80;
    at.facing_left = false;
    CHECK(SotnDoppleAI_Tick(&at, &random, &ai) && ai.step == 3 && ai.buttons == 0x8008);
    at.substep = 1;
    CHECK(SotnDoppleAI_Tick(&at, &random, &ai) && ai.step == 4 && ai.buttons == 0x80 &&
          ai.timer == 24);
    for (Uint32 i = 0; i < 24; ++i)
    {
        at.tick = i;
        CHECK(SotnDoppleAI_Tick(&at, &random, &ai));
        CHECK(ai.buttons == (i == 23 ? 8 : i % 2 == 0 ? 0x1000 : 0));
    }
    CHECK(ai.step == 0);
    at = (SotnDoppleObservation){
        .boss_x = 64, .world_x = 200, .facing_left = true, .player_subweapon_starting = true};
    ai = (SotnDoppleAI){0};
    CHECK(SotnDoppleAI_Tick(&at, &random, &ai));
    CHECK(ai.step == 5 && ai.buttons == 0x4000 && ai.timer == 72 && random == 0x3039);
    ai = (SotnDoppleAI){.step = 1};
    random = 0;
    at.player_subweapon_starting = false;
    at.player_weapon_starting = true;
    CHECK(SotnDoppleAI_Tick(&at, &random, &ai));
    CHECK(ai.step == 6 && ai.buttons == 0x40 && random == 0x3039);
    at.contact = 0;
    at.tick = 32;
    CHECK(SotnDoppleAI_Tick(&at, &random, &ai) && ai.buttons == 0x1060);
    at.contact = 1;
    CHECK(SotnDoppleAI_Tick(&at, &random, &ai) && ai.step == 0);
    at.player_weapon_starting = false;
    at.player_casting = true;
    at.tick = 33;
    CHECK(SotnDoppleAI_Tick(&at, &random, &ai) && ai.step == 14 && ai.buttons == 2 &&
          ai.timer == 63);
    at.player_casting = false;
    at.boss_status = 2;
    for (Uint32 i = 0; i < 63; ++i)
    {
        at.tick = i;
        CHECK(SotnDoppleAI_Tick(&at, &random, &ai));
    }
    CHECK(ai.step == 0 && ai.buttons == 2);
    at.player_status = 0x40000;
    at.boss_status = 1;
    CHECK(SotnDoppleAI_Tick(&at, &random, &ai) && ai.step == 16 && ai.buttons == 8);
    at.boss_status = 2;
    CHECK(SotnDoppleAI_Tick(&at, &random, &ai) && ai.buttons == 2);
    at.player_status = 0;
    at.boss_status = 0x80;
    at.tick = 16;
    CHECK(SotnDoppleAI_Tick(&at, &random, &ai) && ai.step == 15 && ai.buttons == 0x1000);
    at.boss_status = 0;
    CHECK(SotnDoppleAI_Tick(&at, &random, &ai) && ai.step == 0);
    /* Fixed RNG seeds exercise every original immediate weapon reaction. */
    const Uint32 seeds[8] = {0, 4, 8, 3, 2, 11, 1, 5};
    const Uint16 steps[8] = {6, 5, 2, 2, 2, 2, 6, 6};
    const Uint16 buttons[8] = {0x40, 0x4000, 0x10, 0x10, 0x10, 0x10, 0x40, 0x40};
    at = (SotnDoppleObservation){
        .boss_x = 64, .world_x = 200, .facing_left = true, .player_weapon_starting = true};
    for (size_t i = 0; i < 8; ++i)
    {
        ai = (SotnDoppleAI){.step = 1};
        random = seeds[i];
        CHECK(SotnDoppleAI_Tick(&at, &random, &ai));
        CHECK(ai.step == steps[i] && ai.buttons == buttons[i]);
    }
    at.player_status = 0x20;
    ai = (SotnDoppleAI){.step = 1};
    random = 8;
    CHECK(SotnDoppleAI_Tick(&at, &random, &ai) && ai.step == 5 && ai.buttons == 0x4000);
    const SotnDoppleAI before = ai;
    const Uint32 rng = random;
    CHECK(!SotnDoppleAI_Tick(NULL, &random, &ai));
    CHECK(!SotnDoppleAI_Tick(&at, NULL, &ai));
    CHECK(!SotnDoppleAI_Tick(&at, &random, NULL));
    CHECK(!SotnDoppleAI_Init(NULL));
    at.world_x = 32768;
    CHECK(!SotnDoppleAI_Tick(&at, &random, &ai));
    CHECK(ai.step == before.step && ai.buttons == before.buttons && ai.timer == before.timer &&
          ai.attack_timer == before.attack_timer && random == rng);
    at.world_x = 200;
    ai.step = 8;
    CHECK(!SotnDoppleAI_Tick(&at, &random, &ai) && ai.step == 8 && random == rng);
    return true;
}
int main(void)
{
    if (!Tests())
        return 1;
    SDL_Log("SOTN_DOPPLE_AI_TEST: PASS");
    return 0;
}
