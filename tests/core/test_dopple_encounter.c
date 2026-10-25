#include "actors_internal.h"
#include "boss_projection.h"
#include "dopple_fixture.h"
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static bool Tests(SotnActors *s, Uint8 *image, Uint8 *dra, Uint8 *executable)
{
    Fixture(image);
    Put32(dra, 0x98, 0x800a0200);
    Put16(image, 0x418, 1);
    Put16(dra, 0x22c, 200);
    Put16(dra, 0x22e, 8);
    Put16(dra, 0x230, 32);
    Put16(dra, 0x234, 3);
    dra[0x24a] = 8;
    dra[0x24b] = 16;
    for (Uint32 i = 0; i <= 1024; ++i)
        Put16(executable, 0x1dbcc + i * 2U, (Uint16)(i * 4U));
    Put16(executable, 0x1ea12, 4096);
    Put16(executable, 0x1ea10 + 640 * 4, 2560);
    Put16(executable, 0x1ea12 + 640 * 4, 1536);
    Put16(executable, 0x1ea10 + 1024 * 4, 4096);
    Put16(executable, 0x1ea12 + 2048 * 4, (Uint16)-4096);
    Put32(image, 0x120c, 32);
    for (Uint32 i = 0; i < 7; ++i)
        Put16(image, 0x133c + i * 2U, (Uint16)(100 + i));
    s->stage_id = 26;
    s->nz0 = (SotnDataView){image, 0x4400};
    s->dra = (SotnDataView){dra, 0x5000};
    s->executable = (SotnDataView){executable, 0x23220};
    s->player.x = 32 * 65536;
    s->player.y = 100 * 65536;
    Uint8 tiles[16 * 16 * 2] = {0}, types[2] = {0, 3};
    for (size_t i = 16 * 10; i < 16 * 16; ++i)
        tiles[i * 2] = 1;
    s->map = (SotnCollisionMap){{tiles, sizeof(tiles)}, {types, sizeof(types)}, 16, 16};
    CHECK(SotnRoomActors_Create(&s->spawns));
    s->slots[0] = (Actor){.state = {.id = 0x16, .generation = 1, .x = 96 * 65536, .y = 132 * 65536},
                          .slot = 0};
    Actor *root = &s->slots[0];
    Uint32 expected = 0;
    for (Uint32 i = 0; i < 64; ++i)
        SotnCombat_Random(&expected);
    CHECK(ActorDoppleEncounter(s, root));
    CHECK(s->dopple.active && root->state.hit_points == 200 && root->state.priority == 0x9c &&
          root->state.flags == 0x18400000U && root->state.step == 4 &&
          root->state.hitbox_width == 8 && s->combat_random == expected);
    CHECK(s->dopple.ai.step == 18 && s->dopple.previous_hp == 200);
    root->state.hit_points = 190;
    root->hit_flags = 1;
    CHECK(ActorDoppleEncounter(s, root));
    CHECK(s->dopple.body.step == 11 && s->dopple.body.substep == 1 &&
          root->state.hitbox_state == 0 && s->dopple.previous_hp == 190);
    root->hit_flags = 0;
    root->state.flags |= 0x100;
    root->state.hit_points = 0;
    CHECK(ActorDoppleEncounter(s, root) && (s->boss_flags & 2U));
    for (Uint32 tick = 0; tick < 160 && !s->dopple.body.death.complete; ++tick)
        CHECK(ActorDoppleEncounter(s, root));
    CHECK(s->dopple.body.death.complete && s->boss_flags == 6 && root->state.hitbox_state == 0);
    Uint8 selector = 9;
    CHECK(ActorDoppleHitEffect(s, 32, &selector) && selector == 0);
    Put16(image, 0xa10, 0x4000);
    Put16(image, 0xa12, 0x8000);
    image[0xa24] = 4;
    image[0xa25] = 2;
    CHECK(ActorDoppleHitEffect(s, 0xc000, &selector) && selector == 4);
    image[0xa24] = 16;
    CHECK(!ActorDoppleHitEffect(s, 0x4000, &selector) && selector == 4);
    CHECK(!ActorDoppleHitEffect(NULL, 0, &selector));
    CHECK(!ActorDoppleHitEffect(s, 0, NULL));
    CHECK(!ActorDoppleEncounter(NULL, root));
    CHECK(!ActorDoppleEncounter(s, NULL));
    root->state.id = 0x138;
    CHECK(ActorDoppleEncounter(s, root) && root->state.unsupported);
    root->state.id = 0x122;
    CHECK(!ActorDoppleEncounter(s, root));
    return true;
}
static bool Effects(SotnActors *s, Uint8 *image)
{
    s->slots[0].state.priority = 156;
    s->dopple.body = (SotnDoppleBody){.step = 6,
                                      .substep = 3,
                                      .animation_set = 0x8002,
                                      .animation = {.sprite = 1},
                                      .motion = {.x = 100 * 65536, .y = 100 * 65536, .vx = 65536},
                                      .rotated = true,
                                      .rotation = 128};
    Actor effect = {.state = {.id = 0x123, .generation = 2}};
    CHECK(ActorDoppleTrail(s, &effect) && effect.state.priority == 154 &&
          effect.state.scale_x == 256 && effect.state.opacity == 128 &&
          effect.state.frame == 0x8001 && effect.state.rotation == 128);
    CHECK(ActorDoppleTrail(s, &effect) && effect.state.scale_x == 248 &&
          effect.state.opacity == 123);
    for (Uint32 i = 0; i < 25; ++i)
        CHECK(ActorDoppleTrail(s, &effect));
    CHECK(effect.remove);
    effect = (Actor){.state = {.id = 0x124, .generation = 3}};
    Put16(image, 0x43c, 1);
    s->dopple.body.motion.status = 0x100;
    CHECK(ActorDoppleWingHitbox(s, &effect) && effect.attack == 4 && effect.state.hitbox_x == 4 &&
          effect.state.hitbox_width == 8 && effect.state.x == 100 * 65536);
    s->dopple.body.step = 10;
    CHECK(ActorDoppleWingHitbox(s, &effect) && effect.remove);
    for (Uint32 i = 0; i < 6; ++i)
    {
        Put16(image, 0x1778 + i * 2U, (Uint16)(-(Sint32)i * 2));
        Put32(image, 0x1784 + i * 4U, (Uint32)(-(Sint32)(i + 1) * 1024));
        Put16(image, 0x179c + i * 2U, (Uint16)(64 + i * 16));
    }
    for (Uint32 i = 0; i < 24; ++i)
    {
        Put16(image, 0x17a8 + i * 4U, 1);
        Put16(image, 0x17aa + i * 4U, 1);
    }
    Put16(image, 0x1808, 0xffff);
    effect = (Actor){
        .state = {
            .id = 0x102, .generation = 4, .parameters = 0x600, .x = 100 * 65536, .y = 100 * 65536}};
    CHECK(ActorDoppleSmoke(s, &effect) && effect.state.x == 124 * 65536 &&
          effect.state.y == 124 * 65536 && effect.state.scale_x == 128 && effect.vy == -1024 &&
          effect.state.frame == 1);
    CHECK(ActorDoppleSmoke(s, &effect) && effect.state.y == 124 * 65536 - 1024);
    for (Uint32 i = 0; i < 25 && !effect.remove; ++i)
        CHECK(ActorDoppleSmoke(s, &effect));
    CHECK(effect.remove);
    for (Uint32 i = 0; i < 14; ++i)
        image[0x180c + i] = (Uint8)i;
    s->dopple.body.motion.wall[0].effects = SOTN_EFFECT_SIDE;
    s->dopple.body.motion.wall_sensors[0] = (SotnDoppleSensor){3, 7};
    effect = (Actor){.state = {.id = 0x102,
                               .generation = 5,
                               .parameters = 0x400,
                               .x = 55 * 65536 + 16384,
                               .y = 88 * 65536 + 32768}};
    CHECK(ActorDoppleSmoke(s, &effect) && effect.state.x == 103 * 65536 + 16384 &&
          effect.state.y == 107 * 65536 + 32768 && effect.vy == -16384 &&
          effect.state.scale_x == 144);
    effect.state.parameters = 0xb00;
    CHECK(!ActorDoppleSmoke(s, &effect));
    CHECK(!ActorDoppleSmoke(NULL, &effect) && !ActorDoppleSmoke(s, NULL));
    CHECK(!ActorDoppleTrail(NULL, &effect) && !ActorDoppleTrail(s, NULL));
    CHECK(!ActorDoppleWingHitbox(NULL, &effect) && !ActorDoppleWingHitbox(s, NULL));
    const Uint16 parameters[] = {0, 3, 5};
    for (Uint32 i = 0; i < SDL_arraysize(parameters); ++i)
        Put32(image, 0x1d0c + parameters[i] * 4U, 0x80183000);
    image[0x3000] = 2;
    image[0x3001] = 24;
    image[0x3002] = 48;
    image[0x3003] = 72;
    image[0x3004] = image[0x3005] = 2;
    Put16(image, 0x3006, 2);
    Put16(image, 0x3008, 0x31);
    effect = (Actor){.state = {.id = 0x105, .generation = 6, .x = 100 * 65536, .y = 100 * 65536}};
    CHECK(ActorDoppleParticles(s, &effect) && effect.state.primitive_count == 3 &&
          effect.state.priority == 158);
    ActorPrimitive *particle = ActorPrimitiveGet(s, &effect, 0);
    CHECK(particle && particle->state.active && particle->state.blend &&
          particle->state.page == 0x20);
    for (Uint32 i = 0; i < 3; ++i)
        CHECK(ActorDoppleParticles(s, &effect));
    CHECK(effect.remove);
    ActorPrimitivesClear(s, &effect);
    Put16(image, 0x300a, 4);
    for (Uint32 i = 0; i < 32; ++i)
    {
        s->dopple.debris_x[i] = -65536;
        s->dopple.debris_y[i] = -131072;
    }
    effect = (Actor){.state = {.id = 0x105, .generation = 7, .parameters = 0x500}};
    CHECK(ActorDoppleParticles(s, &effect) && effect.timer == 47);
    for (Uint32 i = 0; i < 46; ++i)
        CHECK(ActorDoppleParticles(s, &effect) && !effect.remove);
    CHECK(effect.timer == 1 && ActorPrimitiveGet(s, &effect, 0)->state.width == 1);
    CHECK(ActorDoppleParticles(s, &effect) && effect.remove);
    ActorPrimitivesClear(s, &effect);
    Put16(image, 0x300a, 3);
    s->slots[0].vx = 2468;
    effect = (Actor){.state = {.id = 0x105, .generation = 8, .parameters = 0x300},
                     .enemy_state = {.owner_generation = 1}};
    CHECK(ActorDoppleParticles(s, &effect) && effect.timer == 47 &&
          ActorPrimitiveGet(s, &effect, 0)->vx == 2468);
    for (Uint32 i = 0; i < 23; ++i)
        CHECK(ActorDoppleParticles(s, &effect) && !effect.remove);
    CHECK(effect.timer == 1 && ActorDoppleParticles(s, &effect) && effect.remove);
    ActorPrimitivesClear(s, &effect);
    image[0x3000] = 33;
    effect = (Actor){.state = {.id = 0x105, .generation = 9}};
    CHECK(!ActorDoppleParticles(s, &effect) && !effect.state.primitive_count);
    CHECK(!ActorDoppleParticles(NULL, &effect) && !ActorDoppleParticles(s, NULL));
    return true;
}
static bool Projection(SotnActors *s)
{
    SotnProjectedPoint point = {0};
    CHECK(SotnBoss_ProjectIntro(s->executable, 128, 100, 0, 0, 0, &point) && point.x == 128 &&
          point.y == 100);
    CHECK(SotnBoss_ProjectIntro(s->executable, 128, 100, 0, 0, 16, &point) && point.x == 128 &&
          point.y == 112);
    CHECK(SotnBoss_ProjectIntro(s->executable, 128, 100, 1024, 0, 16, &point) && point.x == 115 &&
          point.y == 100);
    CHECK(SotnBoss_ProjectIntro(s->executable, 128, 100, 2048, 0, 16, &point) && point.x == 128 &&
          point.y == 87);
    CHECK(SotnBoss_ProjectIntro(s->executable, 0, 0, 0, 224, 32, &point));
    const SotnProjectedPoint saved = point;
    CHECK(!SotnBoss_ProjectIntro(s->executable, 0, 0, 0, 225, 0, &point) && point.x == saved.x &&
          point.y == saved.y);
    CHECK(!SotnBoss_ProjectIntro((SotnDataView){0}, 0, 0, 0, 0, 0, &point));
    CHECK(!SotnBoss_ProjectIntro(s->executable, 0, 0, 0, 0, 0, NULL));
    return true;
}
static bool Allocation(SotnActors *s)
{
    Actor *effect = &s->slots[127];
    *effect = (Actor){.state = {.generation = 10}};
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; i += 2)
        s->primitives[i].owner = 1;
    CHECK(!ActorPrimitivesContiguous(s, effect, 2) && !effect->state.primitive_count);
    CHECK(ActorPrimitives(s, effect, 2) && effect->primitive[0] == 1 && effect->primitive[1] == 3);
    ActorPrimitivesClear(s, effect);
    SDL_zero(s->primitives);
    *effect = (Actor){.state = {.generation = 11}};
    CHECK(ActorPrimitivesContiguous(s, effect, 384) && effect->state.primitive_count == 384);
    CHECK(ActorPrimitiveGet(s, effect, 383) != NULL);
    SotnActorPrimitive last = {0};
    CHECK(SotnActors_GetPrimitive(s, 127, 383, &last));
    ActorPrimitivesClear(s, effect);
    *effect = (Actor){.state = {.generation = 12}};
    CHECK(!ActorPrimitives(s, effect, 385) && !effect->state.primitive_count);
    return true;
}
static bool Entry(SotnActors *s, Uint8 *image)
{
    SDL_zero(s->slots);
    SDL_zero(s->primitives);
    SDL_zero(s->combat);
    s->boss_flags = 0;
    s->world = &s->fallback_world;
    s->camera_x = s->camera_y = 0;
    Actor *manager = &s->slots[128];
    *manager = (Actor){.state = {.id = 0x18, .generation = 100, .x = 256 * 65536, .y = 128 * 65536},
                       .slot = 128};
    CHECK(ActorDoppleEntry(s, manager) && manager->state.step == 1);
    CHECK(s->slots[129].state.id == 0x17 && s->slots[129].state.parameters == 0x101 &&
          s->slots[138].state.parameters == 5 && s->slots[139].state.id == 0x19 &&
          s->slots[140].state.id == 0x1a);
    s->player.x = 192 * 65536;
    CHECK(ActorDoppleEntry(s, manager) && manager->state.step == 1);
    s->player.x += 65536;
    CHECK(ActorDoppleEntry(s, manager) && manager->state.step == 3 && s->control.timer == 576 &&
          s->control.buttons == 0);
    CHECK(ActorDoppleEntry(s, manager) && manager->state.step == 4 &&
          s->slots[129].arena.triggered && s->slots[130].arena.triggered);
    s->boss_flags = 2;
    s->game_seconds = 3661;
    CHECK(ActorDoppleEntry(s, manager) && manager->state.step == 5 &&
          s->world->time_attack[SOTN_TIME_DOPPLEGANGER] == 10101);
    s->boss_flags = 0;
    Actor *plate = &s->slots[129];
    Put16(image, 0x5c2, 256);
    CHECK(ActorDoppleEntry(s, plate) && plate->state.frame == 0x64 && plate->state.opacity == 128 &&
          plate->state.primitive_count == 1);
    plate->state.step = 5;
    plate->arena.offset = -65 * 65536;
    plate->arena.speed = -64;
    Put16(image, 0x5aa, 32);
    Put16(image, 0x5b6, 8);
    Put16(image, 0x5e4, 0x0101);
    CHECK(ActorDoppleEntry(s, plate) && !plate->remove && plate->arena.offset < -65 * 65536);
    // Preserve the original signed threshold: only positive plates disappear at +64.
    plate->state.parameters = 1;
    plate->arena.offset = 65 * 65536;
    Put16(image, 0x5dc, 0x0101);
    CHECK(ActorDoppleEntry(s, plate) && plate->remove);
    ActorPrimitivesClear(s, plate);
    Uint8 tiles[32 * 16 * 2] = {0}, types[2] = {0};
    s->map = (SotnCollisionMap){{tiles, sizeof(tiles)}, {types, sizeof(types)}, 32, 16};
    s->mutable_tiles = tiles;
    Actor *block = &s->slots[160];
    *block =
        (Actor){.state = {.id = 0x1b, .generation = 110, .step = 3, .y = 145 * 65536}, .slot = 160};
    Put16(image, 0x670 + 7 * 4, 1);
    Put16(image, 0x672 + 7 * 4, 1);
    CHECK(ActorDoppleEntry(s, block) && block->state.step == 4 &&
          Read16(tiles + (0xc0 + 96) * 2) == 1);
    s->boss_flags = 4;
    CHECK(ActorDoppleEntry(s, block) && block->state.step == 5);
    CHECK(ActorDoppleEntry(s, block) && block->state.step == 6 &&
          Read16(tiles + (0xc0 + 96) * 2) == 0);
    block->state.step = 5;
    Put16(image, 0x672 + 3 * 4, 2);
    CHECK(!ActorDoppleEntry(s, block) && Read16(tiles + (0xc0 + 96) * 2) == 0);
    s->mutable_tiles = NULL;
    Actor *light = &s->slots[139];
    light->state.step = 2;
    CHECK(ActorDoppleLight(s, light) && light->state.primitive_count == 48);
    light->arena.triggered = true;
    ActorPrimitive *last = ActorPrimitiveGet(s, light, 47);
    CHECK(last != NULL);
    last->state.red = 248;
    CHECK(ActorDoppleLight(s, light) && light->state.step == 4 && s->slots[140].arena.triggered);
    CHECK(ActorDoppleLight(s, light) && light->state.step == 5 && s->slots[0].state.id == 0x16 &&
          (s->world->time_attack_visited & (1U << SOTN_TIME_DOPPLEGANGER)));
    ActorPrimitive *first = ActorPrimitiveGet(s, light, 0);
    CHECK(first && !first->state.blend && first->state.priority == 0x40 &&
          first->state.screen_space);
    last->state.red = 8;
    CHECK(ActorDoppleLight(s, light) && light->state.step == 6 &&
          light->state.primitive_count == 16);
    CHECK(ActorPrimitiveGet(s, light, 0)->state.gouraud);
    Actor *shock = &s->slots[140];
    shock->state.step = 2;
    CHECK(ActorDoppleShock(s, shock) && shock->state.primitive_count == 384);
    s->control.timer = 1;
    CHECK(ActorDoppleShock(s, shock));
    CHECK(ActorPrimitiveGet(s, shock, 368)->state.active &&
          !ActorPrimitiveGet(s, shock, 369)->state.active);
    s->control.timer = 0;
    CHECK(ActorDoppleShock(s, shock) && shock->state.step == 4 &&
          shock->state.primitive_count == 0 && s->combat.music_change && s->combat.music == 0);
    s->music_playing = true;
    CHECK(ActorDoppleShock(s, shock) && !shock->remove);
    s->music_playing = false;
    CHECK(ActorDoppleShock(s, shock) && shock->remove && s->combat.music == 0x31d);
    Actor reward = {.state = {.id = 0x1d, .generation = 120, .parameters = 4, .step = 6}};
    Put16(image, 0x42b8, 23);
    CHECK(ActorDoppleEntry(s, &reward) && reward.state.id == 3 &&
          reward.state.parameters == 0x8017 && reward.state.step == 0);
    CHECK(!ActorDoppleTrigger(s, SOTN_STAGE_ACTOR_SLOTS));
    CHECK(!ActorDoppleEntry(NULL, manager) && !ActorDoppleEntry(s, NULL));
    s->map = (SotnCollisionMap){0};
    return true;
}
int main(void)
{
    SotnActors *s = SDL_calloc(1, sizeof(*s));
    Uint8 *image = SDL_calloc(1, 0x4400), *dra = SDL_calloc(1, 0x5000),
          *executable = SDL_calloc(1, 0x23220);
    const bool passed = s && image && dra && executable && Tests(s, image, dra, executable) &&
                        Effects(s, image) && Allocation(s) && Projection(s) && Entry(s, image);
    if (s)
        SotnRoomActors_Destroy(s->spawns);
    SDL_free(executable);
    SDL_free(dra);
    SDL_free(image);
    SDL_free(s);
    if (!passed)
        return 1;
    SDL_Log("SOTN_DOPPLE_ENCOUNTER_TEST: PASS");
    return 0;
}
