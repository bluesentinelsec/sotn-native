#include "actors_internal.h"
#include <sotn/stage_freeze.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    Uint8 dra[0xd000], nz0[0x3000], tiles[32 * 16 * 2], types[2];
} Fixture;

static void Put16(Uint8 *image, size_t offset, Uint16 value)
{
    image[offset] = (Uint8)value;
    image[offset + 1] = (Uint8)(value >> 8);
}

static void Put32(Uint8 *image, size_t offset, Uint32 value)
{
    for (unsigned int i = 0; i < 4; ++i)
        image[offset + i] = (Uint8)(value >> (i * 8));
}

static SotnCollisionMap MakeFixture(Fixture *fixture)
{
    SDL_memset(fixture, 0, sizeof(*fixture));
    Put32(fixture->nz0, 28, 0x80180040);
    for (size_t i = 0; i < 106; ++i)
        Put32(fixture->nz0, 0x40 + i * 4, 0x80180300);
    Put16(fixture->nz0, 0x300, 0xfffe);
    Put16(fixture->nz0, 0x302, 0xfffe);
    const Uint16 ids[] = {1, 0x23, 0x34, 0x2b, 0x13};
    for (size_t i = 0; i < SDL_arraysize(ids); ++i)
    {
        const size_t offset = 0x300 + (i + 1) * 10;
        Put16(fixture->nz0, offset, (Uint16)(40 + i * 40));
        Put16(fixture->nz0, offset + 2, 140);
        Put16(fixture->nz0, offset + 4, ids[i]);
        Put16(fixture->nz0, offset + 6, (Uint16)i);
    }
    Put16(fixture->nz0, 0x33c, 0xffff);
    Put16(fixture->nz0, 0x33e, 0xffff);
    Put32(fixture->dra, 0x98, 0x800a0200);
    Put32(fixture->dra, 0x224, 0xa8001000);
    const size_t initializers[] = {0xbc8, 0xc34, 0xcc4, 0xc7c};
    for (size_t i = 0; i < SDL_arraysize(initializers); ++i)
    {
        Put16(fixture->nz0, initializers[i], 0x8001);
        Put16(fixture->nz0, initializers[i] + 2, 1);
    }
    Put32(fixture->nz0, 0xe04, 0x80180f00);
    Put16(fixture->nz0, 0xe34, 3);
    fixture->nz0[0xe44] = 0x70;
    const Uint8 candle[] = {2, 1, 1, 2, 0, 0};
    const Uint8 globe[] = {1, 3, 0, 0};
    const Uint8 walk[] = {1, 1, 1, 2, 0, 0};
    const Uint8 attack[] = {1, 3, 1, 4, 255, 0};
    SDL_memcpy(fixture->nz0 + 0xf00, candle, sizeof(candle));
    SDL_memcpy(fixture->nz0 + 0xef0, globe, sizeof(globe));
    SDL_memcpy(fixture->nz0 + 0x22ec, walk, sizeof(walk));
    SDL_memcpy(fixture->nz0 + 0x22fc, attack, sizeof(attack));
    SDL_memcpy(fixture->nz0 + 0x237c, walk, sizeof(walk));
    Put16(fixture->nz0, 0x22d6, 20);
    Put16(fixture->nz0, 0x22da, 3);
    Put16(fixture->nz0, 0x22dc, 6);
    Put16(fixture->nz0, 0x22de, 65533);
    Put16(fixture->nz0, 0x22e0, 65524);
    Put16(fixture->nz0, 0x22e6, 20);
    Put16(fixture->nz0, 0x22e8, 6);
    fixture->types[1] = 3;
    for (size_t y = 10; y < 16; ++y)
        for (size_t x = 0; x < 32; ++x)
            fixture->tiles[(y * 32 + x) * 2] = 1;
    Put32(fixture->nz0, 40, 0x80182e00);
    Put32(fixture->nz0, 0x2e2c, 0x3c128018);
    Put32(fixture->nz0, 0x2e30, 0x26522f00);
    for (Uint16 i = 0; i < 22; ++i)
        Put16(fixture->nz0, 0x2f00 + i * 2U, (Uint16)(900U + i));
    return (SotnCollisionMap){
        {fixture->tiles, sizeof(fixture->tiles)}, {fixture->types, sizeof(fixture->types)}, 32, 16};
}

static bool FieldsAndReferences(Fixture *f)
{
    MakeFixture(f);
    SotnDataView palettes = {0};
    CHECK(SotnStageFreeze_Palettes((SotnDataView){f->nz0, sizeof(f->nz0)}, &palettes) &&
          palettes.bytes == f->nz0 + 0x2f00 && palettes.size == 44);
    const SotnDataView before_view = palettes;
    Put32(f->nz0, 40, 0x8018fff0);
    CHECK(!SotnStageFreeze_Palettes((SotnDataView){f->nz0, sizeof(f->nz0)}, &palettes) &&
          palettes.bytes == before_view.bytes && palettes.size == before_view.size);
    Put32(f->nz0, 40, 0x80182e00);
    Put32(f->nz0, 0x2e30, 0x26522fff);
    CHECK(!SotnStageFreeze_Palettes((SotnDataView){f->nz0, sizeof(f->nz0)}, &palettes));
    Put32(f->nz0, 0x2e30, 0x26523ff0);
    CHECK(!SotnStageFreeze_Palettes((SotnDataView){f->nz0, sizeof(f->nz0)}, &palettes));
    Put32(f->nz0, 0x2e30, 0x26522f00);
    Put32(f->nz0, 0x2e2c, 0);
    CHECK(!SotnStageFreeze_Palettes((SotnDataView){f->nz0, sizeof(f->nz0)}, &palettes));
    MakeFixture(f);
    CHECK(SotnStageFreeze_Palettes((SotnDataView){f->nz0, sizeof(f->nz0)}, &palettes));
    SotnStageFreezeActor a = {
        .flags = 0x10003, .step = 1, .palette = 3, .hit_palette = 345, .palette_effect = 4};
    bool advance = false;
    CHECK(SotnStageFreeze_Begin(&a, palettes, &advance) && advance && a.flags == 0x10002 &&
          a.palette == 909 && a.hit_palette == 345);
    CHECK(SotnStageFreeze_Begin(&a, palettes, &advance) && advance && a.flags == 0x10001 &&
          a.palette == 908);
    CHECK(SotnStageFreeze_Begin(&a, palettes, &advance) && advance && a.flags == 0x10000 &&
          a.palette == 345 && !a.hit_palette);
    a = (SotnStageFreezeActor){.flags = 15, .step = 1, .palette = 7, .palette_effect = 255};
    SotnStageFreezeActor before = a;
    CHECK(SotnStageFreeze_Begin(&a, (SotnDataView){0}, &advance) && !advance &&
          !SDL_memcmp(&a, &before, sizeof(a)));
    a.step = 0;
    before = a;
    CHECK(SotnStageFreeze_Begin(&a, (SotnDataView){0}, &advance) && advance &&
          !SDL_memcmp(&a, &before, sizeof(a)));
    a.step = 1;
    a.flags |= 0x10000;
    before = a;
    advance = false;
    CHECK(!SotnStageFreeze_Begin(&a, palettes, &advance) && !advance &&
          !SDL_memcmp(&a, &before, sizeof(a)));
    a.palette_effect = 10;
    a.flags = SDL_MAX_UINT32;
    CHECK(SotnStageFreeze_Begin(&a, palettes, &advance) && advance &&
          a.flags == SDL_MAX_UINT32 - 1U && a.palette == 921);
    CHECK(!SotnStageFreeze_Begin(NULL, palettes, &advance));
    CHECK(!SotnStageFreeze_Palettes((SotnDataView){0}, &palettes));
    return true;
}
static bool ActualStage(Fixture *f)
{
    const SotnCollisionMap map = MakeFixture(f);
    SotnActors *s = NULL;
    SotnActorLayout *layout = NULL;
    CHECK(SotnActors_Create((SotnDataView){f->dra, sizeof(f->dra)},
                            (SotnDataView){f->nz0, sizeof(f->nz0)}, &s));
    CHECK(SotnActorLayout_Open((SotnDataView){f->nz0, sizeof(f->nz0)}, 0, &layout));
    SotnPlayerState p = {.x = 10 * 65536, .y = 140 * 65536};
    CHECK(SotnActors_Enter(s, layout, map, 0, 0));
    for (Uint16 i = 0; i < 3; ++i)
        CHECK(SotnActors_Tick(s, p, 0, 0));
    Actor *hold = &s->slots[3], *effect = &s->slots[0];
    hold->state.flags &= 0xfffeffffU;
    hold->state.flags |= 0xa0000000U;
    hold->state.x = 600 * 65536;
    hold->stun = 50;
    hold->cooldown[0] = 17;
    hold->hit_flags = 2;
    hold->hit_params = 8;
    const Actor before = *hold;
    effect->state.flags |= 0x10003;
    effect->stun = 70;
    effect->cooldown[0] = 21;
    effect->hit_flags = 2;
    effect->hit_params = 0;
    effect->palette_effect = 4;
    effect->hit_palette = 345;
    s->control.timer = 18;
    s->combat.contact_damage = 15;
    s->combat.sound_count = 1;
    s->motion.x = 20;
    const Uint32 clock = s->ticks, rng = s->random, combat_rng = s->combat_random;
    s->slots[188] =
        (Actor){.state = {.id = 1, .generation = 12345, .x = 60 * 65536, .y = 140 * 65536},
                .slot = 188,
                .dynamic = true};
    p.x = hold->state.x;
    p.attack = (SotnPlayerAttack){.active = true,
                                  .hit_type = 2,
                                  .power = 100,
                                  .width = 30,
                                  .height = 30,
                                  .sound = true,
                                  .sound_id = 5};
    CHECK(SotnActors_TickFrozen(s, p, 0, 0));
    CHECK(!SDL_memcmp(hold, &before, sizeof(before)));
    CHECK(effect->state.palette == 909 && (effect->state.flags & 15U) == 2 && effect->stun == 70 &&
          effect->cooldown[0] == 21 && !effect->hit_flags && !effect->hit_params);
    CHECK(s->slots[188].state.step != 0 && s->slots[188].state.generation == 12345);
    const Actor initialized = s->slots[188];
    CHECK(s->control.timer == 18 && s->ticks == clock && s->random == rng &&
          s->combat_random == combat_rng && !s->combat.contact_damage && !s->motion.x &&
          !s->combat.sound_count);
    CHECK(SotnActors_TickFrozen(s, p, 0, 0) &&
          !SDL_memcmp(&s->slots[188], &initialized, sizeof(initialized)));
    CHECK(SotnActors_TickFrozen(s, p, 0, 0) && effect->state.palette == 345 &&
          !effect->hit_palette && !(effect->state.flags & 15U));
    const Actor final = *effect;
    CHECK(!SotnActors_TickFrozen(s, p, 1, 0) && !SDL_memcmp(effect, &final, sizeof(final)));
    CHECK(!SotnActors_TickFrozen(NULL, p, 0, 0));
    SotnActorLayout_Close(layout);
    SotnActors_Destroy(s);
    return true;
}

static bool SpiritOwnership(Fixture *f)
{
    const SotnCollisionMap map = MakeFixture(f);
    SotnActors *s = NULL;
    SotnActorLayout *layout = NULL;
    CHECK(SotnActors_Create((SotnDataView){f->dra, sizeof(f->dra)},
                            (SotnDataView){f->nz0, sizeof(f->nz0)}, &s));
    CHECK(SotnActorLayout_Open((SotnDataView){f->nz0, sizeof(f->nz0)}, 0, &layout));
    SotnPlayerState p = {
        .x = 10 * 65536, .y = 140 * 65536, .hp = 70, .hp_max = 70, .invulnerable = true};
    CHECK(SotnActors_Enter(s, layout, map, 0, 0));
    for (Uint16 i = 0; i < 3; ++i)
        CHECK(SotnActors_Tick(s, p, 0, 0));
    Actor *target = &s->slots[3];
    target->state.hit_points = 50;
    target->state.hitbox_state = 2;
    target->state.hitbox_x = 7;
    target->state.hitbox_width = 10;
    target->state.hitbox_height = 12;
    SotnSpiritTargetSnapshot snapshot = {0};
    CHECK(SotnActors_SpiritTargets(s, &snapshot) && snapshot.slots[3].x == Pixel(target->state.x) &&
          snapshot.slots[3].identity == target->state.id && snapshot.slots[3].hp == 50 &&
          snapshot.generations[3] == target->state.generation);
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    CHECK(r != NULL);
    r->target_marks[0] = (Uint64)1 << 3;
    r->target_generations[3] = snapshot.generations[3];
    r->target_identities[3] = snapshot.slots[3].identity;
    CHECK(SotnActors_CommitBatPrimitives(s, r) && (target->state.flags & 0x80000U));
    target->state.flags &= 0xfff7ffffU;
    r->target_marks[0] |= (Uint64)1 << 4;
    r->target_generations[4] = SDL_MAX_UINT32;
    r->target_identities[4] = 99;
    CHECK(!SotnActors_CommitBatPrimitives(s, r) && !(target->state.flags & 0x80000U));
    r->target_marks[0] = (Uint64)1 << 3;
    CHECK(ActorPrimitives(s, target, 1));
    const Uint16 occupied = target->primitive[0];
    SotnBatRuntimeChild *spirit = &r->children[1];
    *spirit = (SotnBatRuntimeChild){.kind = 61,
                                    .generation = 1000,
                                    .initialized = true,
                                    .group = 3,
                                    .primitive_count = 16,
                                    .state.spirit.motion = {.phase = 1}};
    for (Uint16 i = 0; i < 16; ++i)
        spirit->primitives[i] = (Uint16)(occupied + i);
    CHECK(!SotnActors_CommitBatPrimitives(s, r) && !(target->state.flags & 0x80000U) &&
          s->primitives[occupied].owner == target->state.generation);
    ActorPrimitivesClear(s, target);
    target->state.primitive_count = 0;
    SotnPrimitiveSlots slots = {0};
    bool allocated = false;
    CHECK(SotnActors_PrimitiveSlots(s, &slots) &&
          SotnPrimitiveSlots_Reserve(&slots, 16, spirit->primitives, &allocated) && allocated);
    r->target_marks[0] = 0;
    spirit->source = (SotnPlayerFactorySource){
        .x = target->state.x + (target->state.facing_left ? -7 : 7) * 65536, .y = target->state.y};
    spirit->state.spirit.motion.x = spirit->source.x;
    spirit->state.spirit.motion.y = spirit->source.y;
    spirit->state.spirit.attack = (SotnPlayerAttack){.active = true,
                                                     .hit_type = 2,
                                                     .width = 6,
                                                     .height = 6,
                                                     .power = 10,
                                                     .element = 8,
                                                     .invincibility_frames = 4};
    r->spirit_mask = (Uint64)1 << 1;
    CHECK(SotnActors_CommitBatPrimitives(s, r) && SotnActors_SetBatRuntime(s, r));
    CHECK(SotnActors_Tick(s, p, 0, 0));
    const Sint16 first_hp = target->state.hit_points;
    CHECK(first_hp < 50 && first_hp > 0 && target->cooldown[ACTOR_WING_CHANNEL] == 4);
    spirit->generation = 1001;
    CHECK(SotnActors_CommitBatPrimitives(s, r) && SotnActors_Tick(s, p, 0, 0) &&
          target->state.hit_points == first_hp && target->cooldown[ACTOR_WING_CHANNEL] == 3);
    for (Uint16 i = 0; i < 3; ++i)
        CHECK(SotnActors_Tick(s, p, 0, 0));
    CHECK(target->state.hit_points < first_hp);
    const Uint8 death[] = {2, 1, 255, 0};
    SDL_memcpy(f->nz0 + 0x2334, death, sizeof(death));
    target->state.hit_points = 1;
    target->cooldown[ACTOR_WING_CHANNEL] = 0;
    spirit->state.spirit.attack.power = 100;
    CHECK(SotnActors_Tick(s, p, 0, 0) && target->state.hit_points <= 0 &&
          (target->state.flags & 0x100U) && s->combat.kills == 1 && spirit->kind == 61);
    CHECK(!SotnActors_SpiritTargets(NULL, &snapshot));
    SotnActorLayout_Close(layout);
    SotnActors_Destroy(s);
    SDL_free(r);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = FieldsAndReferences(f) && ActualStage(f) && SpiritOwnership(f);
    SDL_free(f);
    if (ok)
        SDL_Log("SOTN_STAGE_FREEZE_TEST: PASS");
    return ok ? 0 : 1;
}
