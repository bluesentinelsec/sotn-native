#include "character_fixture.h"
#include <sotn/bat_runtime.h>
#include <sotn/combat.h>
#include <sotn/sword_familiar.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static Uint8 dra[0x11000];
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
// g_SpellDefs[19/20] as US DRA holds them: invincibility20, stun4, hitbox2, effect0x81,
// cut element, attack5/10.
static void Spells(void)
{
    for (Uint16 id = 19; id <= 20; ++id)
    {
        Uint8 *s = dra + 0x8410 + id * 28U;
        s[13] = 0x14;
        Put16(s + 14, 4);
        Put16(s + 16, 2);
        Put16(s + 18, 0x81);
        Put16(s + 22, 0x40);
        Put16(s + 24, id == 19 ? 5 : 10);
    }
}
static SotnBatRuntimeInput Input(void)
{
    return (SotnBatRuntimeInput){.dra = {dra, sizeof(dra)},
                                 .image = {12, 24, -6, -12},
                                 .player = {.x = 100 * 65536,
                                            .y = 140 * 65536,
                                            .hp = 70,
                                            .hp_max = 70,
                                            .priority = 148,
                                            .contacts = 1,
                                            .animation = {.sprite = 1}}};
}
static bool HitboxTest(void)
{
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r)), *saved = SDL_malloc(sizeof(*saved));
    SotnCharacter *character = NULL;
    CHECK(r && saved && SotnCharacter_Create((SotnDataView){dra, sizeof(dra)}, &character));
    SotnCharacterState c = {0};
    SotnCharacterStats stats = {0};
    CHECK(SotnCharacter_Get(character, &c, &stats));
    c.random = 0x13579bdU;
    CHECK(SotnCharacter_Restore(character, &c));
    const SotnPlayerFactorySource tip = {.x = 140 * 65536, .y = 90 * 65536, .priority = 0x93};
    bool allocated = false;
    // GetFreeEntity(0x11, 0x2F): the first free slot17..46, after other owners.
    CHECK(SotnBatRuntime_SpawnFamiliarHitbox(r, (Uint64)1 << 17, tip, 0, 19, 50, &allocated) &&
          allocated && r->children[2].familiar_hitbox && r->children[2].kind == 0xde &&
          !r->children[2].initialized);
    CHECK(SotnBatRuntime_SpawnFamiliarHitbox(r, 0, tip, 1, 19, 50, &allocated) && allocated &&
          r->children[1].familiar_hitbox && r->children[1].parameter == 1);
    *saved = *r;
    CHECK(
        SotnBatRuntime_SpawnFamiliarHitbox(r, 0x00007ffffffe0000ULL, tip, 0, 19, 50, &allocated) &&
        !allocated && !SDL_memcmp(saved, r, sizeof(*r)));
    CHECK(!SotnBatRuntime_SpawnFamiliarHitbox(r, 0, tip, 2, 19, 50, &allocated) &&
          !SotnBatRuntime_SpawnFamiliarHitbox(r, 0, tip, 0, 18, 50, &allocated) &&
          !SotnBatRuntime_SpawnFamiliarHitbox(r, 0, tip, 0, 19, 0, &allocated) &&
          !SotnBatRuntime_SpawnFamiliarHitbox(r, 0, tip, 0, 19, 100, &allocated));
    // A corrupted DE stops the pass before any change.
    r->children[1].state.familiar.level = 0;
    *saved = *r;
    CHECK(!SotnBatRuntime_Tick(r, character, Input()) && !SDL_memcmp(saved, r, sizeof(*r)));
    r->children[1].state.familiar.level = 50;
    // First update, in slot order: GetSpellDef's rand per entity, level scale 50*4/95+1 = 3,
    // a 4x4 hitbox and the least-used groups 3 then 4.
    Uint32 random = 0x13579bdU;
    const Uint16 first = SotnCombat_Random(&random), second = SotnCombat_Random(&random);
    const Uint32 intelligence = stats.total[2];
    CHECK(SotnBatRuntime_Tick(r, character, Input()));
    const SotnPlayerAttack a = r->children[1].state.familiar.attack,
                           b = r->children[2].state.familiar.attack;
    CHECK(r->children[1].initialized && r->children[2].initialized &&
          a.power == (5 + (intelligence * 2 + first % 12) / 10) * 3 &&
          b.power == (5 + (intelligence * 2 + second % 12) / 10) * 3 && a.width == 4 &&
          a.height == 4 && !a.offset_x && a.hit_type == 2 && a.hit_effect == 0x81 &&
          a.element == 0x40 && a.stun_frames == 4 && a.invincibility_frames == 20 && a.active &&
          r->children[1].group == 3 && r->children[2].group == 4 && r->groups[3] == 1 &&
          r->groups[4] == 1 && r->familiar_mask == 6);
    CHECK(SotnCharacter_Get(character, &c, NULL) && c.random == random);
    // A frozen pass holds them; the next update destroys them and frees their groups.
    SotnBatRuntimeInput frozen = Input();
    frozen.freeze = 3;
    CHECK(SotnBatRuntime_Tick(r, character, frozen) && r->children[1].familiar_hitbox &&
          r->familiar_mask == 6);
    CHECK(SotnBatRuntime_Tick(r, character, Input()) && !r->children[1].kind &&
          !r->children[2].kind && !r->familiar_mask && !r->groups[3] && !r->groups[4]);
    // Mojo Mail adds half; the dive's spell20 at level99 scales by five; slot32+ uses 7..10.
    CHECK(SotnCharacter_Get(character, &c, NULL));
    c.equipment[SOTN_EQUIP_ARMOR] = 21;
    random = c.random = 0x2468aU;
    CHECK(SotnCharacter_Restore(character, &c) && SotnCharacter_Get(character, &c, &stats));
    CHECK(SotnBatRuntime_SpawnFamiliarHitbox(r, 0xfffe0000ULL, tip, 0, 20, 99, &allocated) &&
          allocated && r->children[32 - 16].familiar_hitbox);
    const Uint16 roll = SotnCombat_Random(&random);
    Uint32 power = 10 + (stats.total[2] * 2U + roll % 12U) / 10U;
    power += power / 2U;
    CHECK(SotnBatRuntime_Tick(r, character, Input()) &&
          r->children[16].state.familiar.attack.power == power * 5 && r->children[16].group == 7);
    // Death clears the live hitbox's attack.
    SotnBatRuntime dead = *r;
    SDL_zero(dead.children[16]);
    CHECK(SotnBatRuntime_SpawnFamiliarHitbox(&dead, 0, tip, 0, 19, 1, &allocated) && allocated);
    SotnBatRuntimeInput dying = Input();
    dying.player.step = SOTN_PLAYER_DEATH_HOLD;
    CHECK(SotnBatRuntime_Tick(&dead, character, dying) && !dead.familiar_mask &&
          !dead.children[1].state.familiar.attack.active);
    SotnCharacter_Destroy(character);
    SDL_free(saved);
    SDL_free(r);
    return true;
}
int main(void)
{
    TestCharacter_Fill(dra);
    Spells();
    return HitboxTest() ? 0 : 1;
}
