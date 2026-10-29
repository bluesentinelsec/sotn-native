#include "character_fixture.h"
#include <sotn/bat_runtime.h>
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
    Uint8 dra[0x11000], exe[0x24000];
    SotnCharacter *character;
    SotnBatRuntime runtime;
    SotnSpiritTargetSnapshot targets;
    SotnBatRuntimeInput input;
} Fixture;
static void Put16(Uint8 *p, Uint16 n)
{
    p[0] = (Uint8)n;
    p[1] = (Uint8)(n >> 8);
}
static void Blueprint(Fixture *f, Uint16 id, Uint8 child, Uint8 amount, Uint8 per_cycle, Uint8 kind)
{
    Uint8 *p = f->dra + 0xd1d4 + id * 6U;
    p[0] = child;
    p[1] = amount;
    p[2] = (Uint8)(per_cycle | 64U);
    p[3] = 2;
    p[4] = kind;
    p[5] = 0;
}
static bool Create(Fixture *f)
{
    SotnCharacter_Destroy(f->character);
    SDL_memset(f, 0, sizeof(*f));
    TestCharacter_Fill(f->dra);
    for (Uint16 i = 0; i < 1025; ++i)
    {
        Put16(f->exe + 0x1dbcc + i * 2U, (Uint16)(i * 4U));
        Put16(f->exe + 0x22a10 + i * 2U, (Uint16)(i / 2U));
    }
    const Sint16 shape[18] = {15, 0,   15, -16, 0,  -16, -16, -16, -16,
                              0,  -16, 15, 0,   15, 15,  15,  15,  0};
    for (Uint16 i = 0; i < 18; ++i)
        Put16(f->dra + 0x10860 + i * 2U, (Uint16)shape[i]);
    for (Uint16 i = 0; i < 18; ++i)
        f->dra[0x10846 + i] = (Uint8)(17U + i * 16U);
    for (Uint16 i = 0; i < 4; ++i)
        Put16(f->dra + 0x10858 + i * 2U, (Uint16)(512U + i * 512U));
    for (Uint16 i = 0; i < 8; ++i)
    {
        Put16(f->dra + 0x10798 + i * 4U, 1);
        Put16(f->dra + 0x1079a + i * 4U, (Uint16)(i + 1U));
    }
    Put16(f->dra + 0x107b8, 0);
    Put16(f->dra + 0x107ba, 0);
    Uint8 *profile = f->dra + 0xd670 + 0x27U * 20U;
    Put16(profile, 80);
    Put16(profile + 2, 40);
    Put16(profile + 4, 20);
    Put16(profile + 6, 0x123);
    Put16(profile + 8, 256);
    Put16(profile + 10, 256);
    Put16(profile + 12, 256);
    Put16(profile + 14, 2);
    Put16(profile + 16, 0x31);
    Put16(profile + 18, 64);
    Uint8 *flash = f->dra + 0xcfb4 + 13U * 4U;
    flash[0] = 6;
    flash[1] = 20;
    flash[2] = 30;
    flash[3] = 40;
    /* Authored schedules use two-update cycles/two-child Tetra bursts, not game tables. */
    Blueprint(f, 117, 63, 1, 1, 0);
    Blueprint(f, 116, 61, 1, 1, 1);
    Blueprint(f, 118, 61, 4, 2, 1);
    Blueprint(f, 44, 11, 1, 1, 0);
    Blueprint(f, 81, 45, 1, 1, 2);
    f->dra[0xd4b8] = 32;
    f->dra[0xd4b9] = 63;
    f->dra[0xd4ba] = 17;
    f->dra[0xd4bb] = 47;
    f->dra[0xd4bc] = 32;
    f->dra[0xd4bd] = 47;
    for (Uint16 i = 1; i <= 3; i += 2)
    {
        Uint8 *spell = f->dra + 0x8410 + i * 28U;
        spell[13] = 3;
        spell[14] = 1;
        spell[16] = 2;
        spell[18] = 1;
        spell[20] = 0;
        spell[22] = 8;
        spell[24] = i == 1 ? 20 : 70;
    }
    CHECK(SotnCharacter_Create((SotnDataView){f->dra, sizeof(f->dra)}, &f->character));
    f->input = (SotnBatRuntimeInput){.dra = {f->dra, sizeof(f->dra)},
                                     .executable = {f->exe, sizeof(f->exe)},
                                     .player = {.x = 100 * 65536 + 123,
                                                .y = 140 * 65536 + 456,
                                                .priority = 148,
                                                .animation_bank = 1},
                                     .image = {16, 32, -8, -16},
                                     .spirit_targets = &f->targets};
    return true;
}
static bool Tick(Fixture *f)
{
    CHECK(SotnBatRuntime_Tick(&f->runtime, f->character, f->input));
    f->input.player.spirit.started = false;
    if (f->runtime.freeze_changed)
        f->input.freeze = f->runtime.freeze;
    return true;
}
static bool Lifecycle(Fixture *f)
{
    for (Uint16 tetra = 0; tetra < 2; ++tetra)
    {
        CHECK(Create(f));
        f->input.player.spirit = (SotnPlayerSpirit){
            .source = {.x = 100 * 65536 + 123, .y = 140 * 65536 + 456, .priority = 148},
            .started = true,
            .spell = tetra ? 3 : 1,
            .blueprint = tetra ? 0x10075U : 117U};
        CHECK(Tick(f) && f->runtime.freeze_changed && f->runtime.freeze == 3 &&
              f->runtime.children[47].kind == 63 && f->runtime.children[47].initialized &&
              !f->runtime.children[47].primitive_count && f->runtime.screen_flash &&
              f->runtime.screen_flash_ticks == 5);
        Uint16 captures = 0, births = 0, flashes = 1, retired = 0;
        Uint64 generations = 0;
        bool fan = false;
        for (Uint16 t = 1; t < 240; ++t)
        {
            CHECK(Tick(f));
            flashes += (Uint16)f->runtime.screen_flash;
            captures += f->runtime.capture_count;
            if (f->runtime.capture_count)
            {
                const SotnSpiritCapture c = f->runtime.captures[0];
                CHECK(c.slot == 63 && c.generation == f->runtime.children[47].generation &&
                      c.x == 96 && c.y == 60 && f->runtime.children[47].primitive_count == 9);
            }
            for (Uint16 i = 0; i < 48; ++i)
            {
                const SotnBatRuntimeChild *c = &f->runtime.children[i];
                if (c->kind == 61)
                {
                    CHECK(c->initialized && c->primitive_count == 16 && c->group >= 3 &&
                          c->group <= 6 && c->state.spirit.attack.width == 6 &&
                          c->state.spirit.attack.height == 6 &&
                          c->state.spirit.attack.power >= 20 && c->state.spirit.attack.power < 70);
                    if (!(generations & ((Uint64)1 << i)))
                    {
                        generations |= (Uint64)1 << i;
                        ++births;
                        CHECK(c->parameter == (Uint16)(births - 1U) && c->state.spirit.frame == 1 &&
                              c->state.spirit.motion.trails[0].quad.u[0] == 17);
                    }
                    const SotnPlayerEffectQuad *q = NULL;
                    Uint16 count = 0;
                    CHECK(SotnBatRuntime_Visual(&f->runtime, i, &q, &count) && count == 8 &&
                          q == c->state.spirit.quads && f->runtime.spirit_mask & ((Uint64)1 << i));
                }
                if (c->kind == 11 && c->parameter == 0x6700)
                    fan = true;
            }
            if (!f->runtime.children[47].kind && !f->input.freeze)
                retired = 1;
        }
        CHECK(captures == 1 && births == (tetra ? 4 : 1) && flashes == 3 && retired && fan);
        CHECK(!f->runtime.children[47].kind);
    }
    return true;
}
static bool TargetsAndCamera(Fixture *f)
{
    CHECK(Create(f));
    f->targets.slots[3] =
        (SotnSpellTarget){.identity = 22, .hitbox = 2, .hp = 50, .x = 200, .y = 100};
    f->targets.generations[3] = 17;
    f->targets.slots[4] = f->targets.slots[3];
    f->targets.slots[4].identity = 23;
    f->targets.generations[4] = 18;
    for (Uint16 i = 1; i <= 2; ++i)
        f->runtime.children[i] =
            (SotnBatRuntimeChild){.kind = 61,
                                  .generation = i,
                                  .source = {.x = 100 * 65536, .y = -33 * 65536},
                                  .state.spirit.motion = {.parameter = 0}};
    f->runtime.serial = 2;
    CHECK(Tick(f));
    CHECK(f->runtime.children[1].group == 3 && f->runtime.children[2].group == 4);
    CHECK(Tick(f));
    CHECK(f->runtime.target_marks[0] == ((Uint64)1 << 3 | (Uint64)1 << 4) &&
          f->runtime.target_identities[3] == 22 && f->runtime.target_generations[4] == 18 &&
          f->runtime.children[1].state.spirit.target_slot == 3 &&
          f->runtime.children[2].state.spirit.target_slot == 4);
    /* Original retained pointers follow a replacement with nonzero identity, not generation. */
    f->targets.slots[3].identity = 99;
    f->targets.generations[3] = 99;
    CHECK(Tick(f) && f->runtime.children[1].state.spirit.motion.phase == 2);
    f->targets.slots[3].identity = 0;
    CHECK(Tick(f) && f->runtime.children[1].state.spirit.motion.phase == 3);
    CHECK(Create(f));
    f->runtime.children[1] = (SotnBatRuntimeChild){.kind = 61,
                                                   .generation = 1,
                                                   .source = {.x = 100 * 65536, .y = -33 * 65536},
                                                   .state.spirit = {0}};
    f->runtime.serial = 1;
    CHECK(Tick(f) && Tick(f));
    SotnSpirit expected = f->runtime.children[1].state.spirit.motion;
    SotnSpiritEvents events = {0};
    CHECK(SotnSpirit_Tick(&expected,
                          (SotnSpiritInput){.dra = f->input.dra,
                                            .executable = f->input.executable,
                                            .frame = f->runtime.children[1].state.spirit.frame,
                                            .allocated = true},
                          &events));
    CHECK(SotnSpirit_Translate(&expected, -32, 16));
    f->input.screen_shift_x = -32;
    f->input.screen_shift_y = 16;
    CHECK(Tick(f) &&
          !SDL_memcmp(&expected, &f->runtime.children[1].state.spirit.motion, sizeof(expected)));
    const SotnSpirit before = f->runtime.children[1].state.spirit.motion;
    f->input.entering = true;
    f->input.screen_shift_x = 24;
    f->input.screen_shift_y = -8;
    CHECK(SotnSpirit_Translate(&expected, 24, -8) && Tick(f) &&
          !SDL_memcmp(&expected, &f->runtime.children[1].state.spirit.motion, sizeof(expected)) &&
          f->runtime.children[1].state.spirit.motion.age == before.age);
    return true;
}
static bool PoolAndFreeze(Fixture *f)
{
    CHECK(Create(f));
    SotnBatRuntimeChild *fire = &f->runtime.children[16];
    *fire = (SotnBatRuntimeChild){.kind = 45,
                                  .generation = 1,
                                  .initialized = true,
                                  .group = 7,
                                  .source = {.x = 100 * 65536, .y = 120 * 65536},
                                  .state.fire = {.motion = {.x = 100 * 65536,
                                                            .y = 120 * 65536,
                                                            .velocity_x = 65536,
                                                            .scale_x = 64,
                                                            .scale_y = 64,
                                                            .bank = 9,
                                                            .initialized = true},
                                                 .frame = 1}};
    f->runtime.serial = 1;
    CHECK(SotnPlayerFactory_Begin(&f->runtime.factories[0].cursor,
                                  (SotnPlayerFactorySource){.x = 100 * 65536, .y = 120 * 65536}, 81,
                                  0));
    CHECK(SotnPlayerFactory_Begin(
        &f->runtime.factories[1].cursor,
        (SotnPlayerFactorySource){.x = 100 * 65536, .y = -33 * 65536, .flags = 0x10000}, 116, 0));
    const SotnBatRuntimeChild held = *fire;
    const SotnPlayerFactory factory = f->runtime.factories[0].cursor;
    f->input.freeze = 3;
    CHECK(Tick(f) && !SDL_memcmp(fire, &held, sizeof(held)) &&
          !SDL_memcmp(&f->runtime.factories[0].cursor, &factory, sizeof(factory)) &&
          f->runtime.children[1].kind == 61 && f->runtime.children[1].initialized &&
          f->runtime.fire_mask == ((Uint64)1 << 16));
    CHECK(Create(f));
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_WORDS; ++i)
        f->input.primitive_slots.occupied[i] = SDL_MAX_UINT64;
    f->runtime.children[1] = (SotnBatRuntimeChild){
        .kind = 61, .generation = 1, .source = {.flags = 0x10000}, .state.spirit = {0}};
    f->runtime.serial = 1;
    SotnCharacterState before = {0}, after = {0};
    CHECK(SotnCharacter_Get(f->character, &before, NULL) && Tick(f) &&
          !f->runtime.children[1].kind && !f->runtime.spirit_mask &&
          SotnCharacter_Get(f->character, &after, NULL) && before.random == after.random);
    CHECK(Create(f));
    f->runtime.children[47] = (SotnBatRuntimeChild){.kind = 63,
                                                    .generation = 1,
                                                    .source = {.x = 100 * 65536, .y = 140 * 65536},
                                                    .state.portal = {0}};
    f->runtime.serial = 1;
    CHECK(Tick(f));
    for (Uint16 i = 0; i < 9; ++i)
        CHECK(Tick(f));
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_WORDS; ++i)
        f->input.primitive_slots.occupied[i] = SDL_MAX_UINT64;
    SotnBatRuntime *saved = SDL_malloc(sizeof(*saved));
    CHECK(saved != NULL);
    *saved = f->runtime;
    CHECK(SotnCharacter_Get(f->character, &before, NULL));
    CHECK(!SotnBatRuntime_Tick(&f->runtime, f->character, f->input) &&
          !SDL_memcmp(saved, &f->runtime, sizeof(*saved)) &&
          SotnCharacter_Get(f->character, &after, NULL) && before.random == after.random &&
          f->runtime.freeze == 3);
    SDL_free(saved);
    return true;
}

static bool WolfReplacement(Fixture *f)
{
    CHECK(Create(f));
    f->input.other_occupied = ((Uint64)1 << 31) - ((Uint64)1 << 17);
    CHECK(SotnPlayerFactory_Begin(&f->runtime.factories[0].cursor,
                                  (SotnPlayerFactorySource){.x = 100 * 65536, .y = 100 * 65536},
                                  116, 0));
    CHECK(Tick(f) && f->runtime.children[15].kind == 61 && f->runtime.children[15].group == 3);
    f->input.other_occupied = 0;
    CHECK(SotnPlayerFactory_Begin(&f->runtime.factories[0].cursor,
                                  (SotnPlayerFactorySource){.x = 100 * 65536, .y = 100 * 65536},
                                  116, 0));
    CHECK(Tick(f) && f->runtime.children[1].kind == 61 && f->runtime.children[1].group == 4);
    CHECK(SotnBatRuntime_ReplaceWolfSlots(&f->runtime) && !f->runtime.children[1].kind &&
          f->runtime.children[15].kind == 61 && f->runtime.groups[3] == 1 &&
          !f->runtime.groups[4] && f->runtime.spirit_mask == ((Uint64)1 << 15));
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Lifecycle(f) && TargetsAndCamera(f) && PoolAndFreeze(f) && WolfReplacement(f);
    SotnCharacter_Destroy(f->character);
    SDL_free(f);
    if (ok)
        SDL_Log("SOTN_SHARED_SPIRIT_TEST: PASS");
    return ok ? 0 : 1;
}
