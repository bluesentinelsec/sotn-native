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
    random = 0;
    CHECK(SotnCombat_DamageAttributes(enemy, 10, 0, 255, stats, 50, 1, &random, &result));
    CHECK(result == 0x4026 && random == 0xc21f1c8aU);
    const Uint32 previous = random;
    const Uint16 previous_result = result;
    CHECK(!SotnCombat_DamageAttributes(enemy, 10, 0, 255, stats, 0, 1, &random, &result));
    CHECK(random == previous && result == previous_result);
    return true;
}

static bool ContactPower(void)
{
    SotnContactPower result = {0};
    SotnCombatEnemy equipment = {0};
    CHECK(SotnCombat_ContactPower(30, 0x8001, equipment, &result));
    CHECK(result.attack == 30 && result.effects == 0x8000 && !result.immune && !result.absorb);
    equipment.weaknesses = 0x8000;
    CHECK(SotnCombat_ContactPower(31, 0x8001, equipment, &result) && result.attack == 62);
    equipment.strengths = 0x8000;
    CHECK(SotnCombat_ContactPower(31, 0x8001, equipment, &result) && result.attack == 31);
    equipment.weaknesses = 0;
    CHECK(SotnCombat_ContactPower(31, 0x8001, equipment, &result) && result.attack == 15);
    equipment.absorbs = equipment.immunes = 0x8000;
    CHECK(SotnCombat_ContactPower(31, 0x8001, equipment, &result) && result.immune &&
          !result.absorb);
    equipment.immunes = 0;
    CHECK(SotnCombat_ContactPower(0, 0x8001, equipment, &result) && result.absorb &&
          result.attack == 1);
    equipment = (SotnCombatEnemy){.immunes = 0x8200, .absorbs = 0x8000};
    CHECK(SotnCombat_ContactPower(31, 0x8201, equipment, &result) && !result.immune &&
          result.absorb && result.effects == 0x8000);
    equipment = (SotnCombatEnemy){.weaknesses = 0x20};
    CHECK(SotnCombat_ContactPower(9999, 0x21, equipment, &result) && result.attack == 19998);
    const SotnContactPower saved = result;
    CHECK(!SotnCombat_ContactPower(10000, 0x21, equipment, &result));
    CHECK(result.attack == saved.attack && result.effects == saved.effects);
    CHECK(!SotnCombat_ContactPower(1, 0, equipment, NULL));
    return true;
}

static bool NormalContact(void)
{
    Uint16 damage = 123;
    SotnContactPower power = {.attack = 15};
    CHECK(SotnCombat_NormalContact(power, 6, false, &damage) && damage == 9);
    CHECK(SotnCombat_NormalContact(power, 6, true, &damage) && damage == 18);
    CHECK(SotnCombat_NormalContact(power, 16, true, &damage) && damage == 1);
    power.attack = 0;
    CHECK(SotnCombat_NormalContact(power, 0, true, &damage) && damage == 1);
    power.immune = true;
    CHECK(SotnCombat_NormalContact(power, 0, true, &damage) && damage == 0);
    power = (SotnContactPower){.attack = 19998};
    CHECK(SotnCombat_NormalContact(power, 0, true, &damage) && damage == 9999);
    const Uint16 before = damage;
    power.absorb = true;
    CHECK(!SotnCombat_NormalContact(power, 0, false, &damage) && damage == before);
    power.absorb = false;
    power.attack = 19999;
    CHECK(!SotnCombat_NormalContact(power, 0, false, &damage) && damage == before);
    power.attack = 1;
    CHECK(!SotnCombat_NormalContact(power, 10000, false, &damage) && damage == before);
    CHECK(!SotnCombat_NormalContact(power, 0, false, NULL));
    CHECK(SotnCombat_ContactPower(10, 0x81, (SotnCombatEnemy){.immunes = 0x80}, &power) &&
          power.immune && SotnCombat_NormalContact(power, 0, true, &damage) && damage == 0);
    return true;
}

static bool EnvironmentalContact(void)
{
    Uint16 damage = 0;
    SotnContactPower power = {.attack = 15};
    CHECK(SotnCombat_EnvironmentalContact(power, 70, false, &damage) && damage == 8);
    CHECK(SotnCombat_EnvironmentalContact(power, 70, true, &damage) && damage == 16);
    CHECK(SotnCombat_EnvironmentalContact(power, 7, true, &damage) && damage == 1);
    power.attack = 19998;
    CHECK(SotnCombat_EnvironmentalContact(power, 9999, true, &damage) && damage == 2498);
    power.immune = true;
    CHECK(SotnCombat_EnvironmentalContact(power, 70, true, &damage) && damage == 0);
    power.absorb = true;
    damage = 123;
    CHECK(!SotnCombat_EnvironmentalContact(power, 70, false, &damage) && damage == 123);
    power.absorb = false;
    CHECK(!SotnCombat_EnvironmentalContact(power, 0, false, &damage) && damage == 123);
    CHECK(!SotnCombat_EnvironmentalContact(power, 10000, false, &damage) && damage == 123);
    power.attack = 19999;
    CHECK(!SotnCombat_EnvironmentalContact(power, 70, false, &damage) && damage == 123);
    CHECK(!SotnCombat_EnvironmentalContact(power, 70, false, NULL));
    return true;
}
int main(void)
{
    if (!Boxes() || !Damage() || !ContactPower() || !NormalContact() || !EnvironmentalContact())
        return 1;
    SDL_Log("SOTN_COMBAT_TEST: PASS");
    return 0;
}
