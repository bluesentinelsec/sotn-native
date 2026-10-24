#include <sotn/combat.h>

#define CHECK(condition)                                                                           \
    do                                                                                             \
    {                                                                                              \
        if (!(condition))                                                                          \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #condition, SDL_GetError());                      \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

static bool Boxes(void)
{
    const SotnCombatBox attack = {100, 100, 5, 6};
    CHECK(SotnCombat_Overlap(attack, (SotnCombatBox){109, 110, 5, 5}));
    CHECK(!SotnCombat_Overlap(attack, (SotnCombatBox){110, 110, 5, 5}));
    CHECK(!SotnCombat_Overlap(attack, (SotnCombatBox){109, 111, 5, 5}));
    CHECK(!SotnCombat_Overlap((SotnCombatBox){100, 100, 0, 5}, attack));
    CHECK(SotnCombat_Overlap((SotnCombatBox){-32768, -32768, 1, 1},
                             (SotnCombatBox){32767, 32767, 1, 1}));
    CHECK(!SotnCombat_Overlap((SotnCombatBox){0, 0, 256, 1}, attack));
    return true;
}

static bool Damage(void)
{
    const Uint16 stats[4] = {6, 6, 6, 6}, strength[4] = {99, 1, 1, 1};
    SotnCombatEnemy enemy = {0};
    Uint32 random = 0;
    Uint16 result = 0;
    CHECK(SotnCombat_Damage(enemy, 6, 0, 0, stats, 1, &random, &result) && result == 6);
    CHECK(random == 12345);
    enemy.defense = 10;
    CHECK(SotnCombat_Damage(enemy, 6, 0, 0, stats, 1, &random, &result) && result == 1);
    enemy.defense = 1;
    enemy.weaknesses = 0x20;
    CHECK(SotnCombat_Damage(enemy, 6, 0, 0, stats, 1, &random, &result) && result == 11);
    enemy.weaknesses = 0;
    enemy.strengths = 0x20;
    CHECK(SotnCombat_Damage(enemy, 6, 0, 0, stats, 1, &random, &result) && result == 2);
    enemy.immunes = 0x20;
    const Uint32 before = random;
    CHECK(SotnCombat_Damage(enemy, 6, 0, 0, stats, 1, &random, &result) && result == 0xc000 &&
          random == before);
    enemy.immunes = 0;
    enemy.absorbs = 0x20;
    CHECK(SotnCombat_Damage(enemy, 6, 0, 0, stats, 1, &random, &result) && result == 0x8006 &&
          random == before);
    enemy = (SotnCombatEnemy){0};
    random = 0;
    CHECK(SotnCombat_Damage(enemy, 6, 0, 255, strength, 1, &random, &result) && result == 0x400c);
    const Uint32 saved = random;
    const Uint16 saved_result = result;
    CHECK(!SotnCombat_Damage(enemy, 1000, 0, 0, stats, 1, &random, &result));
    CHECK(random == saved && result == saved_result);
    CHECK(!SotnCombat_Damage(enemy, 6, 0, 0, NULL, 1, &random, &result));
    return true;
}

int main(void)
{
    if (!Boxes() || !Damage())
        return 1;
    SDL_Log("SOTN_COMBAT_TEST: PASS");
    return 0;
}
