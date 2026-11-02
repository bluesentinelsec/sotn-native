#include "actors_internal.h"
#include "character_fixture.h"
#include <sotn/combat.h>
#include <sotn/sire.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static bool Shared(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(1, 20U * 0x7000U),
          *executable = SDL_calloc(1, 0x20000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r)), *before = SDL_malloc(sizeof(*before));
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && executable && actors && r && before);
    TestCharacter_Fill(dra);
    Uint8 *record = dra + 0x500 + 72U * 52U;
    Put16(record + 8, 43);
    record[14] = 7;
    record[15] = 18;
    record[17] = 130;
    record[18] = 0x60;
    record[19] = 50;
    record[22] = record[25] = 1;
    Put16(record + 38, 4);
    Put16(record + 40, 0x82);
    Put16(record + 42, 1);
    Put16(record + 48, 9);
    // Authored timelines, blueprints and triangular SDK samples, never extracted tables.
    Uint8 *overlay = weapons + 18U * 0x7000U + 0x4000;
    for (Uint16 i = 0; i < 15; ++i)
    {
        Put16(overlay + 0x6a8 + i * 4U, i == 14 ? 0xffff : 2);
        Put16(overlay + 0x6aa + i * 4U, (Uint16)(i + 1));
    }
    Put16(overlay + 0x6e4, 2);
    Put16(overlay + 0x6e6, 7);
    Put16(overlay + 0x6e8, 0);
    Put16(overlay + 0x6ea, 0);
    Put16(overlay + 0x724, 2);
    Put16(overlay + 0x726, 9);
    Put16(overlay + 0x728, 0);
    Put16(overlay + 0x72a, 0);
    for (Uint16 i = 0; i <= 1024; ++i)
        Put16(executable + 0x1dbcc + i * 2U, (Uint16)(i * 4U));
    for (Uint16 i = 0; i < 2; ++i)
    {
        Uint8 *bp = dra + 0xd1d4 + (i ? 56U : 50U) * 6U;
        bp[0] = (Uint8)i;
        bp[1] = bp[2] = 1;
        bp[4] = i ? 6 : 0x26;
    }
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b8 + 13] = 31;
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    SotnBatRuntimeInput in = {
        .dra = actors->dra,
        .weapons = {weapons, 20U * 0x7000U},
        .executable = {executable, 0x20000},
        .player = {.hp = 70, .x = 128 * 65536, .y = 140 * 65536, .priority = 148}};
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots));
    bool allocated = false;
    CHECK(SotnBatRuntime_RequestSire(r, (SotnPlayerFactorySource){.priority = 148}, 0x1032, 72,
                                     &allocated) &&
          allocated);
    SotnCharacterState initial = {0}, after = {0};
    CHECK(SotnCharacter_Get(character, &initial, NULL));
    in.freeze = 3;
    CHECK(SotnBatRuntime_Tick(r, character, in) && !r->children[1].kind);
    in.freeze = 0;
    // Real factory birth, properties, private animation and global attack share one transaction.
    Put16(overlay + 0x6a8, 0xffff);
    *before = *r;
    CHECK(!SotnBatRuntime_Tick(r, character, in) && !SDL_memcmp(before, r, sizeof(*r)));
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == initial.random);
    Put16(overlay + 0x6a8, 2);
    CHECK(SotnBatRuntime_Tick(r, character, in) && r->sire_mask == 2 && r->children[1].sire &&
          r->spell_use_ticks == 4 && !r->use_ticks && !r->children[1].sire_child &&
          r->children[1].group == 3 && r->children[1].state.sire.attack.equipment == 72 &&
          r->children[1].state.sire.animation.frame == 1 &&
          SotnActors_CommitBatPrimitives(actors, r));
    Uint32 expected = initial.random;
    (void)SotnCombat_Random(&expected);
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    *before = *r;
    in.freeze = 3;
    CHECK(SotnBatRuntime_Tick(r, character, in) && r->sire_mask == 2 &&
          !SDL_memcmp(&before->children[1], &r->children[1], sizeof(r->children[1])));
    in.freeze = 0;
    // Successful burst reservation refreshes properties; its child starts next root pass.
    r->children[1].state.sire.parent.phase = 2;
    r->children[1].state.sire.parent.animation = 2;
    r->children[1].state.sire.parent.lifetime = 64;
    r->children[1].state.sire.animation = (SotnSireAnimation){0};
    CHECK(SotnBatRuntime_Tick(r, character, in) && r->sire_mask == 2 && r->children[1].group == 4 &&
          r->factories[0].sire_factory && r->factories[0].sire_parent_slot == 17 &&
          !r->children[2].kind);
    (void)SotnCombat_Random(&expected);
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    // A late descendant animation failure preserves the root, both actors and libc RNG.
    Put16(overlay + 0x724, 0xffff);
    *before = *r;
    CHECK(!SotnBatRuntime_Tick(r, character, in) && !SDL_memcmp(before, r, sizeof(*r)));
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    Put16(overlay + 0x724, 2);
    CHECK(SotnBatRuntime_Tick(r, character, in) && !r->sire_mask && r->children[2].sire_child &&
          r->children[2].state.sire.parent_slot == 17 &&
          r->children[2].state.sire.animation.frame == 9 &&
          SotnActors_CommitBatPrimitives(actors, r));
    (void)SotnCombat_Random(&expected);
    (void)SotnCombat_Random(&expected);
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    // Original raw-slot semantics accept a new cold item72 parent at the same slot.
    r->children[1] = (SotnBatRuntimeChild){.kind = 0xe0, .sire = true, .generation = 50};
    CHECK(SotnBatRuntime_Tick(r, character, in) && r->children[2].sire_child &&
          r->children[2].state.sire.child.lifetime == 5);
    (void)SotnCombat_Random(&expected);
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    // Eight occupied actual roots omit only this burst and its property/RNG/flash refresh.
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        r->factories[i].cursor = (SotnPlayerFactory){.entity_id = 1,
                                                     .blueprint = 50,
                                                     .initialized = true,
                                                     .amount = 1,
                                                     .per_cycle = 1,
                                                     .kind = 6,
                                                     .origin = 2,
                                                     .delay = 20,
                                                     .source.priority = 148};
    r->children[1].state.sire.parent.phase = 2;
    r->children[1].state.sire.parent.lifetime = 64;
    CHECK(SotnBatRuntime_Tick(r, character, in) && !r->sire_mask &&
          r->children[1].state.sire.parent.lifetime == 63);
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    SDL_zero(r->factories);
    // Real parent retirement is visible to later raw-slot children during the same callback pass.
    r->children[1].state.sire.parent.phase = 3;
    r->children[1].state.sire.parent.scale_x = 6;
    r->children[1].state.sire.parent.scale_y = 1024;
    r->children[1].state.sire.parent.lifetime = 1;
    CHECK(SotnBatRuntime_Tick(r, character, in) && !r->children[1].kind && !r->children[2].kind &&
          !r->groups[3] && !r->groups[4] && SotnActors_CommitBatPrimitives(actors, r));
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    // A particle can outlive its portrait. Room cleanup still owns its real rows.
    r->children[32] = (SotnBatRuntimeChild){
        .kind = 5,
        .parameter = 0xc00,
        .generation = ++r->serial,
        .initialized = true,
        .primitive_count = 2,
        .primitives = {0, 1},
        .state.particles = {
            .phase = 1, .parameter = 0xc00, .count = 1, .lifetime = 32, .flags = 0x04800000}};
    CHECK(SotnActors_CommitBatPrimitives(actors, r) && actors->primitives[0].owner &&
          actors->primitives[1].owner);
    CHECK(SotnBatRuntime_PruneSire(r) && !r->children[32].kind &&
          SotnActors_CommitBatPrimitives(actors, r) && !actors->primitives[0].owner &&
          !actors->primitives[1].owner);
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    SotnCharacter_Destroy(character);
    SDL_free(before);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(executable);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}
int main(void)
{
    return Shared() ? 0 : 1;
}
