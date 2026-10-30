#include "actors_internal.h"
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
    Uint8 dra[0x500], stage[0x4000];
    SotnActors *actors;
} Fixture;
static void P16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static void P32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
static bool Open(Fixture *f, Uint16 stage)
{
    SDL_zero(*f);
    P32(f->dra + 0x98, 0x800a0200);
    P32(f->dra + 0x200 + 3 * 40 + 36, 0x08000000U);
    const size_t init = stage == 65 ? 0xb00 : 0xa90, spawner = stage == 65 ? 0xad0 : 0xa60,
                 table = stage == 65 ? 0x3858 : 0x2168, volume = stage == 65 ? 0x13ac : 0x1230;
    P16(f->stage + init + 8, 3);
    P16(f->stage + spawner + 8, 4);
    P16(f->stage + volume + 2, 200);
    P16(f->stage + volume + 6, 150);
    P16(f->stage + volume + 8, 300);
    P32(f->stage + table + 32, (Uint32)-131072);
    P32(f->stage + table + 36, 32768);
    P32(f->stage + table + 156, (Uint32)-65536);
    P32(f->stage + table + 160, (Uint32)-131072);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, stage, &f->actors));
    f->actors->entered = true;
    f->actors->player =
        (SotnPlayerState){.x = 100 * 65536 + 123, .y = 90 * 65536 + 456, .priority = 148};
    return true;
}
static bool Controller(Fixture *f, Uint16 stage)
{
    CHECK(Open(f, stage));
    SotnActors *s = f->actors;
    const Uint16 base = stage == 65 ? 0x34 : 0x2c;
    Actor root = {.state = {.id = base, .parameters = 0x100, .generation = 1}};
    CHECK(ActorEntrance(s, &root) && root.state.step == 1 && !root.state.unsupported &&
          !s->water.feet);
    s->player.y = 130 * 65536 + 456;
    s->player.velocity_y = 131072;
    CHECK(ActorEntrance(s, &root) && s->water.feet == 5 && !s->water.waist && !s->water.head);
    Actor *splash = &s->slots[160];
    CHECK(splash->state.id == base + 1U && splash->state.x == 100 * 65536 &&
          splash->state.y == 150 * 65536 && splash->state.priority == 148 && splash->dynamic);
    CHECK(ActorEntrance(s, splash) && splash->state.step == 1 &&
          splash->state.primitive_count == 2 && s->combat.sound_count == 1 &&
          s->slots[161].state.id == base + 5U && s->slots[161].vy == -131072 &&
          !s->slots[161].state.priority);
    CHECK(ActorEntrance(s, &s->slots[161]) && s->slots[161].state.primitive_count == 33 &&
          s->slots[161].state.priority == 136 && s->slots[161].water.drops.timer == 47);
    const ActorPrimitive *q = ActorPrimitiveGet(s, splash, 0);
    CHECK(q && q->state.active && q->state.textured && q->state.gouraud && q->state.screen_space &&
          q->state.page == 0x7a && q->state.priority == 150);
    const Sint16 old_x = q->state.offset_x[0];
    s->camera_x = 8;
    CHECK(ActorEntrance(s, splash) &&
          ActorPrimitiveGet(s, splash, 0)->state.offset_x[0] == old_x - 8 &&
          splash->water.effect.world_y == 150);
    CHECK(!ActorPrimitiveGet(s, &s->slots[161], 32)->state.active);
    SotnWaterVolume water = {0};
    s->player.y = 160 * 65536 + 456;
    CHECK(ActorEntrance(s, &root) && SotnActors_GetWater(s, &water) && water.feet == 35 &&
          water.waist == 19 && !water.head);
    SotnActors_Destroy(s);
    f->actors = NULL;
    return true;
}
static bool Capacity(Fixture *f)
{
    CHECK(Open(f, 65));
    SotnActors *s = f->actors;
    Actor root = {.state = {.id = 0x34, .parameters = 0x100, .generation = 1}};
    CHECK(ActorWater(s, &root));
    s->player.y = 130 * 65536;
    s->player.velocity_y = 131072;
    s->effect_serial = 0x7fffffffU;
    const SotnWaterActor before = root.water.controller;
    CHECK(!ActorWater(s, &root) && !SDL_memcmp(&before, &root.water.controller, sizeof(before)) &&
          !s->water.feet && !s->slots[160].state.generation);
    s->effect_serial = 0;
    for (Uint16 i = 160; i < 192; ++i)
        s->slots[i].state.generation = 77;
    CHECK(ActorWater(s, &root) && s->water.feet == 5 && !s->effect_serial &&
          s->slots[160].state.generation == 77);
    Actor splash = {.state = {.id = 0x35, .generation = 100, .x = 100 * 65536, .y = 150 * 65536}};
    for (Uint16 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 999;
    const Uint32 rng = s->combat_random;
    CHECK(ActorWater(s, &splash) && splash.remove && !splash.state.primitive_count &&
          s->combat_random == rng && !s->combat.sound_count);
    for (Uint16 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        CHECK(s->primitives[i].owner == 999);
    s->primitives[0] = s->primitives[1] = (ActorPrimitive){0};
    splash = (Actor){.state = {.id = 0x35, .generation = 100, .parameters = 7 * 256}};
    CHECK(!ActorWater(s, &splash) && !splash.state.primitive_count && !s->primitives[0].owner &&
          !s->primitives[1].owner && s->combat_random == rng && !s->combat.sound_count);
    for (Uint16 i = 2; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        CHECK(s->primitives[i].owner == 999);
    SotnActors_Destroy(s);
    f->actors = NULL;
    return true;
}
static bool Drops(Fixture *f)
{
    CHECK(Open(f, 65));
    SotnActors *s = f->actors;
    Actor a = {
        .state = {.id = 0x38, .generation = 1, .x = 100 * 65536 + 123, .y = 150 * 65536 + 456}};
    const Uint32 rng = s->combat_random;
    CHECK(ActorWater(s, &a) && a.state.primitive_count == 1 && a.water.drops.small &&
          a.water.drops.phase == 1 && s->random && s->combat_random == rng &&
          !s->combat.sound_count);
    CHECK(ActorPrimitiveGet(s, &a, 0)->state.blend &&
          ActorPrimitiveGet(s, &a, 0)->state.page == 0x20);
    const Uint32 stage_rng = s->random;
    for (Uint16 i = 0; i < 20 && !a.remove; ++i)
        CHECK(ActorWater(s, &a));
    CHECK(a.remove && s->random == stage_rng && s->combat_random == rng);
    ActorPrimitivesClear(s, &a);
    /* Fragmented free rows do not satisfy the original33-contiguous-row allocation. */
    for (Uint16 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = i % 16 == 0 ? 999 : 0;
    a = (Actor){.state = {.id = 0x39, .generation = 2}};
    CHECK(ActorWater(s, &a) && a.remove && !a.state.primitive_count && s->combat_random == rng);
    SotnActors_Destroy(s);
    f->actors = NULL;
    return true;
}
static bool Currents(Fixture *f)
{
    CHECK(Open(f, 65));
    SotnActors *s = f->actors;
    P16(f->stage + 0x13ac + 14, 8192);
    Actor root = {.state = {.id = 0x34, .parameters = 0x100, .generation = 1}};
    CHECK(ActorWater(s, &root));
    s->player.y = 160 * 65536 + 456;
    const Sint32 before = s->player.x;
    CHECK(ActorWater(s, &root) && s->motion.x == 57344 && s->player.x == before + 57344 &&
          root.water.controller.x == 100 && root.water.controller.cooldown == 7 &&
          s->slots[160].state.id == 0x36 && s->slots[160].water.effect.current == 8192);
    const Uint32 random = s->combat_random;
    CHECK(ActorWater(s, &s->slots[160]) && s->slots[160].state.primitive_count == 2 &&
          s->slots[160].water.effect.velocity_x == 131072 && s->combat_random != random);
    s->motion = (SotnActorMotion){0};
    s->player.contacts = 4;
    const Sint32 held = s->player.x;
    CHECK(ActorWater(s, &root) && !s->motion.x && s->player.x == held);
    SotnActors_Destroy(s);
    f->actors = NULL;
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Controller(f, 65) && Controller(f, 7) && Capacity(f) && Drops(f) && Currents(f);
    SotnActors_Destroy(f->actors);
    SDL_free(f);
    if (ok)
        SDL_Log("NATIVE_WATER: PASS authored "
                "stage/real-slot/row/current/packet/camera/rollback/exhaustion owners");
    return ok ? 0 : 1;
}
