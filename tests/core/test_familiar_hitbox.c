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
    // g_SpellDefs[7], Sword Brothers: invincibility20, stun8, global hitbox0x82, attack20.
    Uint8 *b = dra + 0x8410 + 7 * 28U;
    b[13] = 0x14;
    Put16(b + 14, 8);
    Put16(b + 16, 0x82);
    Put16(b + 18, 0x81);
    Put16(b + 22, 0x40);
    Put16(b + 24, 20);
    // g_SpellDefs[17/18], the Ghost's: invincibility2, stun4, hitbox2, effect1/2 (soul steal).
    for (Uint16 id = 17; id <= 18; ++id)
    {
        Uint8 *g = dra + 0x8410 + id * 28U;
        g[13] = 2;
        Put16(g + 14, 4);
        Put16(g + 16, 2);
        Put16(g + 18, id == 17 ? 1 : 2);
        Put16(g + 24, 5);
    }
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
    // Sword Brothers' DF: spell7's global attack, no hitbox size, alive nine updates.
    SDL_zerop(r);
    CHECK(!SotnBatRuntime_SpawnFamiliarHitbox(r, 0, tip, 1, 7, 30, &allocated) &&
          SotnBatRuntime_SpawnFamiliarHitbox(r, 0, tip, 0, 7, 30, &allocated) && allocated &&
          r->children[1].kind == 0xdf && r->children[1].familiar_hitbox);
    CHECK(SotnCharacter_Get(character, &c, &stats));
    random = c.random;
    const Uint16 brothers_roll = SotnCombat_Random(&random);
    Uint32 brothers = 20 + (stats.total[2] * 2U + brothers_roll % 12U) / 10U;
    brothers += brothers / 2U; // Mojo Mail is still equipped
    Uint16 alive = 0;
    for (Uint16 t = 0; t < 12; ++t)
    {
        CHECK(SotnBatRuntime_Tick(r, character, Input()));
        if (r->children[1].kind)
        {
            const SotnPlayerAttack global = r->children[1].state.familiar.attack;
            CHECK(global.hit_type == 0x82 && !global.width && !global.height && global.active &&
                  global.power == brothers * 2 && global.stun_frames == 8 && r->familiar_mask == 2);
            ++alive;
        }
    }
    CHECK(alive == 9 && !r->familiar_mask);
    SotnCharacter_Destroy(character);
    SDL_free(saved);
    SDL_free(r);
    return true;
}
// GetServantStats for the Ghost's own slot4: one rand, level scale, least-used group3..6
// counted until the pass's slot17..47 recount; death clears the retained box.
static bool ServantTest(void)
{
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r)), *saved = SDL_malloc(sizeof(*saved));
    SotnCharacter *character = NULL;
    CHECK(r && saved && SotnCharacter_Create((SotnDataView){dra, sizeof(dra)}, &character));
    SotnCharacterState c = {0};
    SotnCharacterStats stats = {0};
    CHECK(SotnCharacter_Get(character, &c, &stats));
    c.random = 0x2468aceU;
    CHECK(SotnCharacter_Restore(character, &c));
    Uint32 random = c.random;
    const Uint16 roll = SotnCombat_Random(&random);
    r->groups[3] = 2;
    r->groups[4] = 1;
    r->groups[6] = 1;
    const SotnDataView view = {dra, sizeof(dra)};
    CHECK(SotnBatRuntime_ServantAttack(r, character, view, 18, 70, 120, -8, true, 8, 8));
    const SotnPlayerAttack a = r->servant.attack;
    CHECK(a.active && a.power == (5 + (stats.total[2] * 2U + roll % 12U) / 10U) * 3 &&
          a.width == 8 && a.height == 8 && !a.offset_x && a.hit_type == 2 && a.hit_effect == 2 &&
          a.stun_frames == 4 && a.invincibility_frames == 2 && r->servant.x == 120 &&
          r->servant.y == -8 && r->servant.facing_left && r->servant.group == 5 &&
          r->groups[5] == 1);
    CHECK(SotnCharacter_Get(character, &c, NULL) && c.random == random);
    // Invalid requests change nothing, including the character's rand cursor.
    *saved = *r;
    CHECK(
        !SotnBatRuntime_ServantAttack(r, character, view, 19, 70, 0, 0, false, 8, 8) &&
        !SotnBatRuntime_ServantAttack(r, character, view, 17, 0, 0, 0, false, 8, 8) &&
        !SotnBatRuntime_ServantAttack(r, character, view, 17, 100, 0, 0, false, 8, 8) &&
        !SotnBatRuntime_ServantAttack(r, character, view, 17, 1, 0, 0, false, 0, 8) &&
        !SotnBatRuntime_ServantAttack(r, character, view, 17, 1, 40000, 0, false, 8, 8) &&
        !SotnBatRuntime_ServantAttack(r, character, (SotnDataView){0}, 17, 1, 0, 0, false, 8, 8) &&
        !SDL_memcmp(saved, r, sizeof(*r)));
    CHECK(SotnCharacter_Get(character, &c, NULL) && c.random == random);
    // The pass's recount drops the slot4 count; the box itself stays until death clears it.
    CHECK(SotnBatRuntime_Tick(r, character, Input()) && !r->groups[5] && r->servant.attack.active);
    SotnBatRuntimeInput dying = Input();
    dying.player.step = SOTN_PLAYER_DEATH_HOLD;
    CHECK(SotnBatRuntime_Tick(r, character, dying) && !r->servant.attack.active);
    SotnCharacter_Destroy(character);
    SDL_free(saved);
    SDL_free(r);
    return true;
}
int main(void)
{
    TestCharacter_Fill(dra);
    Spells();
    return HitboxTest() && ServantTest() ? 0 : 1;
}
