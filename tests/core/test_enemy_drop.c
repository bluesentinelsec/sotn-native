#include "actors_internal.h"
#include "character_fixture.h"
#include <sotn/enemy_drop.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    Uint8 dra[0x10000], stage[0x3000];
    SotnWorldState world;
    SotnCharacter *character;
    SotnActors *actors;
} Fixture;
enum
{
    ENEMY = 3,         /* Authored enemy definition. */
    LUCK = 0x18c0,     /* NZ0's g_testCollLuckCutoff. */
    DEFINITION = 0x200 /* g_api.enemyDefs (0x800a0200). */
};
static void Put16(Uint8 *p, Uint32 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static void Tables(Uint8 *stage)
{
    static const Uint8 pattern[20] = {0x00, 0x40, 0x20, 0x10, 0x80, 0x80, 0xa0, 0xa0, 0xc0, 0xc0,
                                      0x00, 0x00, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x00, 0x00};
    SDL_memcpy(stage + LUCK, pattern, sizeof(pattern));
    Put16(stage + LUCK - 0x320 + ENEMY * 2, 7); // enemy-list flag 6 (+1)
    for (Uint16 i = 0; i < 32; ++i)
        Put16(stage + LUCK + 0x34 + i * 2U, 0x10U + i);
    const Uint16 jewel[14] = {0xc00, 0x168, 0xf00, 0x169, 0xfd0, 0x16a, 0xff0,
                              0x16b, 0xff8, 0x16c, 0xffd, 0x16d, 0xfff, 0x16e};
    for (Uint16 i = 0; i < 14; ++i)
        Put16(stage + LUCK + 0x74 + i * 2U, jewel[i]);
}
static void Rates(Fixture *f, Uint16 rare, Uint16 uncommon, Uint16 rare_rate, Uint16 uncommon_rate)
{
    Uint8 *d = f->dra + DEFINITION + ENEMY * 40 + 0x1a;
    Put16(d, rare);
    Put16(d + 2, uncommon);
    Put16(d + 4, rare_rate);
    Put16(d + 6, uncommon_rate);
}
static bool Open(Fixture *f)
{
    SotnActors_Destroy(f->actors);
    SotnCharacter_Destroy(f->character);
    SDL_zero(*f);
    TestCharacter_Fill(f->dra);
    TestCharacter_Progression(f->dra);
    f->dra[0x3400 + 75 * 32 + 28] = 3; // Ring of Arcana: an accessory.
    f->dra[0x98] = 0x00;
    f->dra[0x99] = 0x02;
    f->dra[0x9a] = 0x0a;
    f->dra[0x9b] = 0x80;
    Tables(f->stage);
    CHECK(SotnCharacter_Create((SotnDataView){f->dra, sizeof(f->dra)}, &f->character));
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 12, &f->actors));
    CHECK(SotnActors_SetCharacter(f->actors, f->character) &&
          SotnActors_SetWorld(f->actors, &f->world));
    return true;
}
// A libc seed whose first roll passes (or fails) the scaled cutoff.
static Uint32 Seed(Sint32 cutoff, bool pass)
{
    for (Uint32 seed = 1;; ++seed)
    {
        Uint32 r = seed;
        if (((Sint32)(SotnCombat_Random(&r) & 0xffU) < cutoff) == pass)
            return seed;
    }
}
static bool Kind(void)
{
    CHECK(SotnEnemyDrop_LuckCutoff(0, 50) == 0 && SotnEnemyDrop_LuckCutoff(0x20, 0) == 0x20 &&
          SotnEnemyDrop_LuckCutoff(0x20, 64) == 0x30 && SotnEnemyDrop_LuckCutoff(0x40, 7) == 0x43);
    Uint32 r = 0x1234;
    Sint32 kind = 0;
    // A rare rate above any roll always drops the rare item, after two rand calls.
    CHECK(SotnEnemyDrop_Kind((SotnEnemyDropRates){.rare_rate = 0x100}, 6, 0, &r, &kind) &&
          kind == 0x40);
    Uint32 expected = 0x1234;
    SotnCombat_Random(&expected);
    SotnCombat_Random(&expected);
    CHECK(r == expected);
    // Without rates the prize index is rand() % 28, plus one per zero rate and Arcana copy.
    for (Uint16 arcana = 0; arcana < 3; ++arcana)
    {
        r = expected = 0x77 + arcana;
        for (Uint16 i = 0; i < 3; ++i)
            SotnCombat_Random(&expected);
        const Sint32 index = (Sint32)(SotnCombat_Random(&expected) % 28) + 2 + arcana;
        CHECK(SotnEnemyDrop_Kind((SotnEnemyDropRates){0}, 6, arcana, &r, &kind) && kind == index &&
              r == expected);
    }
    // The uncommon threshold sees the rare rate removed first.
    r = 9;
    CHECK(SotnEnemyDrop_Kind((SotnEnemyDropRates){.uncommon_rate = 0x200}, 6, 1, &r, &kind) &&
          kind == 0x20);
    const Uint32 kept = r;
    kind = -1;
    CHECK(!SotnEnemyDrop_Kind((SotnEnemyDropRates){0}, 6, 3, &r, &kind) && r == kept && kind == -1);
    CHECK(!SotnEnemyDrop_Kind((SotnEnemyDropRates){0}, 6, 0, NULL, &kind));
    return true;
}
static bool Read(Fixture *f)
{
    SotnEnemyDropTables t = {0};
    CHECK(SotnEnemyDrop_ReadTables(f->stage, sizeof(f->stage), LUCK, &t) &&
          t.luck_cutoff[2] == 0x20 && t.enemy_lookup[ENEMY] == 7 && t.prize[31] == 0x2f &&
          t.jewel_sword[13] == 0x16e);
    Uint32 r = 5, expected = 5;
    const Uint16 roll = (Uint16)(SotnCombat_Random(&expected) & 0xfff);
    const Uint16 gem = SotnEnemyDrop_Jewel(&t, &r);
    CHECK(r == expected && gem == (roll <= 0xc00   ? 0x168
                                   : roll <= 0xf00 ? 0x169
                                   : roll <= 0xfd0 ? 0x16a
                                   : roll <= 0xff0 ? 0x16b
                                   : roll <= 0xff8 ? 0x16c
                                   : roll <= 0xffd ? 0x16d
                                                   : 0x16e));
    const SotnEnemyDropTables before = t;
    CHECK(!SotnEnemyDrop_ReadTables(f->stage, sizeof(f->stage), LUCK + 1, &t) &&
          !SDL_memcmp(&t, &before, sizeof(t)));
    CHECK(!SotnEnemyDrop_ReadTables(f->stage, LUCK + 0x40, LUCK, &t));
    f->stage[LUCK + 0x74 + 24] = 0;
    CHECK(!SotnEnemyDrop_ReadTables(f->stage, sizeof(f->stage), LUCK, &t));
    return true;
}
static bool Drop(Fixture *f)
{
    CHECK(Open(f));
    SotnCharacterStats stats = {0};
    SotnCharacterState c = {0};
    CHECK(SotnCharacter_Get(f->character, &c, &stats));
    const Sint32 cutoff = SotnEnemyDrop_LuckCutoff(0x20, stats.total[3]);
    Actor root = {.slot = 70,
                  .enemy_definition = ENEMY + 1,
                  .state = {.generation = 5, .flags = 0x800, .x = 90 * 65536 + 3}};
    Actor part = root;
    part.slot = 71;
    part.state.x = 100 * 65536 + 5;
    part.state.y = 60 * 65536 + 7;
    part.state.facing_left = true;
    // The rare Ring of Varda becomes a Turquoise before the game is cleared (no record).
    Rates(f, 0x173, 0x05, 0x100, 0);
    f->actors->combat_random = Seed(cutoff, true);
    CHECK(ActorEnemyDrop(f->actors, &part, &root, 1));
    Actor *d = &f->actors->slots[160];
    CHECK(d->state.id == 0xa && d->state.parameters == 0x16a - 0x80 && d->vy == -0x38000 &&
          d->state.x == part.state.x && d->state.y == part.state.y && !d->state.facing_left &&
          !d->drop_castle_flag && d->dynamic);
    // After the game is cleared it keeps the Varda and its enemy-list record.
    SDL_zero(*d);
    f->world.game_clear = 1;
    f->actors->combat_random = Seed(cutoff, true);
    CHECK(ActorEnemyDrop(f->actors, &part, &root, 1) && d->state.id == 0xa &&
          d->state.parameters == 0x173 - 0x80 && d->drop_castle_flag == 7);
    ActorDropRecord(f->actors, d);
    CHECK(f->world.flags[0x1b0] == 0x40);
    // An uncommon prize, then a table prize (index rand() % 28 + 2), each from slot 160 on.
    SDL_zero(*d);
    Rates(f, 0x173, 0x05, 0, 0x200);
    f->actors->combat_random = Seed(cutoff, true);
    CHECK(ActorEnemyDrop(f->actors, &part, &root, 1) && d->state.id == 3 &&
          d->state.parameters == 5 && !d->drop_castle_flag);
    SDL_zero(*d);
    Rates(f, 0x173, 0x05, 0, 0);
    Uint32 seed = Seed(cutoff, true), expected = seed;
    for (Uint16 i = 0; i < 4; ++i)
        SotnCombat_Random(&expected);
    const Uint16 index = (Uint16)(SotnCombat_Random(&expected) % 28 + 2);
    f->actors->combat_random = seed;
    CHECK(ActorEnemyDrop(f->actors, &part, &root, 1) && d->state.id == 3 &&
          d->state.parameters == 0x10 + index && f->actors->combat_random == expected);
    // The Jewel sword (hitEffect 5) takes a gem from its own table.
    SDL_zero(*d);
    f->actors->combat_random = Seed(cutoff, true);
    CHECK(ActorEnemyDrop(f->actors, &part, &root, 0x85) && d->state.id == 0xa &&
          d->state.parameters >= 0x168 - 0x80 && d->state.parameters <= 0x16e - 0x80);
    // A failed cutoff roll or no free slot spends one roll and creates nothing.
    SDL_zero(*d);
    seed = Seed(cutoff, false);
    expected = seed;
    SotnCombat_Random(&expected);
    f->actors->combat_random = seed;
    CHECK(ActorEnemyDrop(f->actors, &part, &root, 1) && !d->state.id &&
          f->actors->combat_random == expected);
    for (Uint16 i = 160; i < 192; ++i)
        f->actors->slots[i].state.generation = 99;
    seed = Seed(cutoff, true);
    expected = seed;
    SotnCombat_Random(&expected);
    f->actors->combat_random = seed;
    CHECK(ActorEnemyDrop(f->actors, &part, &root, 1) && f->actors->combat_random == expected);
    for (Uint16 i = 160; i < 192; ++i)
        f->actors->slots[i] = (Actor){0};
    // Without flags 0x400/0x800 nothing is rolled; a flagged root needs its enemy definition.
    root.state.flags = 0x1000;
    f->actors->combat_random = 11;
    CHECK(ActorEnemyDrop(f->actors, &part, &root, 1) && f->actors->combat_random == 11 &&
          !d->state.id);
    root.state.flags = 0x400;
    root.enemy_definition = 0;
    f->actors->combat_random = Seed(SotnEnemyDrop_LuckCutoff(0x40, stats.total[3]), true);
    CHECK(!ActorEnemyDrop(f->actors, &part, &root, 1));
    // Ring of Arcana in an accessory slot shifts the prize index by one.
    root.enemy_definition = ENEMY + 1;
    for (Uint16 i = 160; i < 192; ++i)
        f->actors->slots[i] = (Actor){0};
    c.equipment[5] = 75;
    CHECK(SotnCharacter_Restore(f->character, &c));
    seed = Seed(SotnEnemyDrop_LuckCutoff(0x40, stats.total[3]), true);
    expected = seed;
    for (Uint16 i = 0; i < 4; ++i)
        SotnCombat_Random(&expected);
    const Uint16 arcana = (Uint16)(SotnCombat_Random(&expected) % 28 + 3);
    f->actors->combat_random = seed;
    CHECK(ActorEnemyDrop(f->actors, &part, &root, 1) && d->state.id == 3 &&
          d->state.parameters == 0x10 + arcana);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    SDL_zero(*f);
    Tables(f->stage);
    bool ok = Kind() && Read(f) && Drop(f);
    SotnActors_Destroy(f->actors);
    SotnCharacter_Destroy(f->character);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_ENEMY_DROP_TEST: PASS");
    return 0;
}
