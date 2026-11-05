#include "actors_internal.h"
#include "character_fixture.h"
#include <sotn/combat.h>
#include <sotn/neutron.h>
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
static bool Callback(void)
{
    SotnNeutron n = {.x = 123, .y = 456};
    SotnNeutronEvents e = {0};
    SotnNeutronInput in = {.player_x = -30 * 65536 + 789,
                           .player_y = 45 * 65536 + 987,
                           .priority = 148,
                           .allocated = true};
    CHECK(SotnNeutron_Tick(&n, in, &e) && n.phase == 1 && n.extent == 127 &&
          n.x == -30 * 65536 + 123 && n.y == 45 * 65536 + 456 && n.halo.blue[3] == 16 &&
          e.use_ticks == 4 && e.sounds[0] && e.sounds[1] && !e.properties);
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(SotnNeutron_Tick(&n, in, &e) && n.extent == 127 - (i + 1) * 16 && !e.properties);
    CHECK(n.extent == -1 && n.phase == 1 && n.halo.x[0] == -29 && n.halo.x[1] == -31);
    CHECK(SotnNeutron_Tick(&n, in, &e) && n.phase == 2 && e.properties && e.begin_animation &&
          n.halo.draw_mode == 8);
    in.pose = 3;
    in.remaining = 1;
    CHECK(SotnNeutron_Tick(&n, in, &e) && e.disable_attack && n.blend == 0x30);
    in.pose = 5;
    CHECK(SotnNeutron_Tick(&n, in, &e) && n.phase == 3 && n.opacity == 128 &&
          !(n.flags & 0x40000U));
    n.scale_x = 32760;
    n.rotation = 32736;
    in.remaining = 1;
    CHECK(SotnNeutron_Tick(&n, in, &e) && n.scale_x == -32760 && n.rotation == -32736 &&
          n.opacity == 126);
    in.remaining = -1;
    const Uint8 color = n.halo.blue[3];
    CHECK(SotnNeutron_Tick(&n, in, &e) && n.removed && n.halo.blue[3] == color);
    n = (SotnNeutron){0};
    in.allocated = false;
    CHECK(SotnNeutron_Tick(&n, in, &e) && n.removed && !e.properties && !e.use_ticks &&
          !e.sounds[0]);
    n = (SotnNeutron){.phase = 4};
    SotnNeutron before = n;
    e.use_ticks = 37;
    CHECK(!SotnNeutron_Tick(&n, in, &e) && !SDL_memcmp(&before, &n, sizeof(n)) &&
          e.use_ticks == 37);
    return true;
}
static bool Shared(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(1, 20U * 0x7000U);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r)), *before = SDL_malloc(sizeof(*before));
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && actors && r && before);
    TestCharacter_Fill(dra);
    Uint8 *record = dra + 0x500 + 71U * 52U;
    Put16(record + 8, 43);
    record[14] = 7;
    record[15] = 19;
    record[17] = 130;
    record[18] = 0x60;
    record[19] = 50;
    record[22] = 1;
    record[25] = 1;
    Put16(record + 38, 4);
    Put16(record + 40, 0x82);
    Put16(record + 42, 1);
    Put16(record + 48, 9);
    Uint8 *overlay = weapons + 19U * 0x7000U + 0x4000;
    // Authored durations/frames; these are not an original extracted timeline.
    for (Uint16 i = 0; i < 6; ++i)
    {
        Put16(overlay + 0x52c + i * 4, (Uint16)(2 + i % 2));
        Put16(overlay + 0x52e + i * 4, (Uint16)(7 + i));
    }
    Put16(overlay + 0x52c + 6 * 4, 0xffff);
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    r->children[1] =
        (SotnBatRuntimeChild){.kind = 0xe0,
                              .neutron = true,
                              .generation = 1,
                              .source = {.x = 40 * 65536 + 123, .y = 80 * 65536 + 456}};
    r->serial = 1;
    SotnBatRuntimeInput input = {
        .dra = actors->dra,
        .weapons = {weapons, 20U * 0x7000U},
        .player = {.hp = 70, .x = 40 * 65536, .y = 80 * 65536, .priority = 148}};
    CHECK(SotnActors_PrimitiveSlots(actors, &input.primitive_slots));
    SotnCharacterState initial = {0}, after = {0};
    CHECK(SotnCharacter_Get(character, &initial, NULL));
    // Frozen cold children do not allocate, call RNG or manufacture readiness.
    input.freeze = 3;
    CHECK(SotnBatRuntime_Tick(r, character, input) && !r->children[1].initialized &&
          !r->children[1].primitive_count);
    input.freeze = 0;
    for (Uint16 i = 0; i < 9; ++i)
    {
        CHECK(SotnBatRuntime_Tick(r, character, input) &&
              SotnActors_CommitBatPrimitives(actors, r));
        CHECK(r->children[1].primitive_count == 1 && !r->neutron_mask);
        CHECK(SotnActors_PrimitiveSlots(actors, &input.primitive_slots));
    }
    CHECK(r->children[1].state.neutron.motion.extent == -1);
    r->groups[3] = 7;
    r->groups[4] = r->groups[5] = 1;
    r->groups[6] = 3;
    r->children[24] = (SotnBatRuntimeChild){
        .kind = 11, .generation = 2, .parameter = 0x4400, .state.blink.parameter = 0x44};
    input.image.width = 1;
    *before = *r;
    CHECK(!SotnBatRuntime_Tick(r, character, input) && !SDL_memcmp(before, r, sizeof(*r)));
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == initial.random);
    r->children[24] = (SotnBatRuntimeChild){0};
    input.image.width = 0;
    CHECK(SotnBatRuntime_Tick(r, character, input) && r->neutron_mask == 2 &&
          r->children[1].group == 4 && r->groups[4] == 1);
    CHECK(r->children[1].state.neutron.attack.equipment == 71 &&
          r->children[1].state.neutron.attack.hit_type == 0x82 &&
          !r->children[1].state.neutron.attack.width &&
          r->children[1].state.neutron.animation.frame == 7);
    Uint32 expected = initial.random;
    (void)SotnCombat_Random(&expected);
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected &&
          SotnActors_CommitBatPrimitives(actors, r));
    *before = *r;
    input.freeze = 3;
    CHECK(SotnBatRuntime_Tick(r, character, input) && r->neutron_mask == 2 &&
          !SDL_memcmp(&before->children[1], &r->children[1], sizeof(r->children[1])));
    input.freeze = 0;
    CHECK(SotnBatRuntime_Tick(r, character, input) && !r->neutron_mask &&
          !r->children[1].state.neutron.attack.active);
    CHECK(SotnActors_CommitBatPrimitives(actors, r));
    *before = *r;
    CHECK(SotnBatRuntime_ReplaceWolfSlots(before) && !before->children[1].kind &&
          !before->neutron_mask && !before->groups[4]);
    // Actual room retirement releases the lease without consuming another callback/RNG.
    CHECK(SotnBatRuntime_PruneNeutron(r) && SotnActors_CommitBatPrimitives(actors, r) &&
          !r->children[1].kind);
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        CHECK(!actors->primitives[i].owner);
    // Optional real pool exhaustion retires before properties/RNG/sounds.
    r->children[1] = (SotnBatRuntimeChild){.kind = 0xe0, .neutron = true, .generation = 3};
    SDL_memset(&input.primitive_slots, 255, sizeof(input.primitive_slots));
    CHECK(SotnBatRuntime_Tick(r, character, input) && !r->children[1].kind && !r->sound_count);
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    SotnCharacter_Destroy(character);
    SDL_free(before);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}
int main(void)
{
    return Callback() && Shared() ? 0 : 1;
}
