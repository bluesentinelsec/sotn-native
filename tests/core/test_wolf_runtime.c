#include "actors_internal.h"
#include <sotn/wolf_runtime.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8));
}
static bool Query(void *ctx, Sint32 x, Sint32 y, SotnCollider *out)
{
    (void)ctx;
    (void)x;
    (void)y;
    *out = (SotnCollider){0};
    return true;
}
static SotnWolfControllerFrame Player(void)
{
    return (SotnWolfControllerFrame){
        .x = 128 * 65536 + 123,
        .y = 120 * 65536 + 456,
        .priority = 144,
        .joint_distance = 14,
        .mp = 8,
        .motion = {.body_angle = 2048, .substep = 1, .animation = {.id = 0xde, .sprite = 7}}};
}
static bool Lifetime(SotnDataView dra, SotnDataView executable)
{
    SotnWolfRuntime r = {0};
    SotnWolfControllerFrame p = Player();
    SotnWolfRuntimeEvents e = {0};
    SotnWolfRuntimeInput in = {
        .dra = dra, .executable = executable, .query = Query, .palette = 0x104, .active = true};
    CHECK(!SotnWolfRuntime_Begin(&r, 0, 0) && !SotnWolfRuntime_Begin(&r, 1, (Uint64)1 << 18) &&
          !SotnWolfRuntime_Occupied(&r));
    CHECK(!SotnWolfRuntime_Tick(&r, &p, in, &e));
    CHECK(SotnWolfRuntime_Begin(&r, 111, 0) &&
          SotnWolfRuntime_Occupied(&r) == ((Uint64)0x3fff << 17));
    for (Uint16 i = 0; i < 14; ++i)
        CHECK(r.children[i].slot == 17U + i && r.children[i].lineage == 111 &&
              r.children[i].generation == i + 1U);
    CHECK(!SotnWolfRuntime_Begin(&r, 112, 0));
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_WORDS; ++i)
        in.primitive_slots.occupied[i] = SDL_MAX_UINT64;
    CHECK(SotnWolfRuntime_Tick(&r, &p, in, &e) && !r.root_reserved && !r.environment_reserved &&
          !r.body.parts[0].step && r.body.parts[1].step && r.body.parts[3].step);
    in.primitive_slots.occupied[0] &= ~(Uint64)255;
    CHECK(SotnWolfRuntime_Tick(&r, &p, in, &e) && r.root_reserved && r.environment_reserved &&
          r.body.parts[0].step);
    CHECK(r.environment_rows[0] == 0 && r.environment_rows[1] == 1 && r.root_rows[0] == 2 &&
          r.root_rows[5] == 7 && r.root.player_palette == 0x104);
    const SotnWolfRuntime before = r;
    const SotnWolfControllerFrame previous = p;
    in.entering = true;
    in.screen_shift_x = -20;
    in.screen_shift_y = 10;
    CHECK(SotnWolfRuntime_Tick(&r, &p, in, &e) && !SDL_memcmp(&p, &previous, sizeof(p)) &&
          r.head.pose == before.head.pose && r.head.remaining == before.head.remaining &&
          r.body.wave == before.body.wave &&
          r.body.parts[1].x == before.body.parts[1].x - 20 * 65536 &&
          r.body.parts[1].y == before.body.parts[1].y + 10 * 65536);
    for (Uint16 i = 0; i < 6; ++i)
        CHECK(r.effect.images[i].state == before.effect.images[i].state &&
              r.effect.images[i].timer == before.effect.images[i].timer &&
              r.effect.quads[i].x[0] == before.effect.quads[i].x[0] - 20);
    in.entering = false;
    in.screen_shift_x = in.screen_shift_y = 0;
    CHECK(!SotnWolfRuntime_Hit(&r, r.children[1].generation + 1U, 1));
    CHECK(SotnWolfRuntime_Hit(&r, r.children[1].generation, 1));
    p.motion.velocity_x = 4 * 65536;
    CHECK(SotnWolfRuntime_Tick(&r, &p, in, &e) && e.spell_count >= 1 && e.spells[0].slot == 18 &&
          e.spells[0].spell == 13 && e.spells[0].generation == r.children[1].generation &&
          e.mp_spent == 3 && p.mp == 5 && !r.body.parts[1].hit_flags);
    CHECK(SotnWolfRuntime_Tick(&r, &p, in, &e) && !e.mp_spent && p.mp == 5);
    const Uint32 front = r.children[1].generation, env = r.environment_generation,
                 serial = r.serial;
    in.active = false;
    CHECK(SotnWolfRuntime_Tick(&r, &p, in, &e) && !r.generation && !SotnWolfRuntime_Occupied(&r) &&
          !r.root_reserved && r.environment_reserved && r.environment_generation == env);
    CHECK(!SotnWolfRuntime_Hit(&r, front, 1));
    CHECK(SotnWolfRuntime_Begin(&r, 112, 0) && r.children[1].generation > serial &&
          r.environment_generation == env);
    CHECK(!SotnWolfRuntime_Hit(&r, front, 1));
    r.children[2].generation = r.children[1].generation;
    SotnWolfRuntime corrupt = r;
    CHECK(!SotnWolfRuntime_Tick(&r, &p, in, &e) && !SDL_memcmp(&r, &corrupt, sizeof(r)));
    return true;
}
static bool Pool(SotnDataView dra, SotnDataView executable)
{
    SotnActors *s = SDL_calloc(1, sizeof(*s));
    CHECK(s);
    SotnWolfRuntime r = {0};
    SotnWolfControllerFrame p = Player();
    SotnWolfRuntimeEvents e = {0};
    Actor a = {.state = {.generation = 1}};
    CHECK(ActorPrimitives(s, &a, 20));
    CHECK(SotnWolfRuntime_Begin(&r, 1, 0));
    SotnWolfRuntimeInput in = {
        .dra = dra, .executable = executable, .query = Query, .palette = 0x104, .active = true};
    CHECK(SotnActors_PrimitiveSlots(s, &in.primitive_slots));
    CHECK(SotnWolfRuntime_Tick(&r, &p, in, &e) && SotnActors_CommitWolfPrimitives(s, &r));
    CHECK(r.environment_rows[0] == 20 && r.root_rows[0] == 22 &&
          s->primitives[22].owner == r.children[0].generation &&
          s->primitives[22].ability_kind == 56 &&
          s->primitives[20].ability_kind == SOTN_WOLF_ENVIRONMENT_TAG);
    SotnPrimitiveSlots before = {0}, after = {0};
    CHECK(SotnActors_PrimitiveSlots(s, &before));
    const Uint16 old = r.root_rows[0];
    r.root_rows[0] = a.primitive[0];
    CHECK(!SotnActors_CommitWolfPrimitives(s, &r) && ActorPrimitiveGet(s, &a, 0));
    CHECK(SotnActors_PrimitiveSlots(s, &after) && !SDL_memcmp(&before, &after, sizeof(after)));
    r.root_rows[0] = old;
    r.root_rows[0] = r.environment_rows[0];
    CHECK(!SotnActors_CommitWolfPrimitives(s, &r));
    r.root_rows[0] = old;
    ++r.children[0].generation;
    CHECK(!SotnActors_CommitWolfPrimitives(s, &r));
    --r.children[0].generation;
    SotnBatRuntime *bat = SDL_calloc(1, sizeof(*bat));
    CHECK(bat);
    bat->children[0] = (SotnBatRuntimeChild){
        .kind = 11, .generation = 1, .initialized = true, .primitive_count = 8};
    for (Uint16 i = 0; i < 8; ++i)
        bat->children[0].primitives[i] = (Uint16)(28U + i);
    CHECK(SotnActors_CommitBatPrimitives(s, bat) && s->primitives[22].ability_kind == 56);
    bat->children[0].primitives[0] = 22;
    CHECK(!SotnActors_CommitBatPrimitives(s, bat));
    bat->children[0].primitives[0] = 28;
    in.active = false;
    CHECK(SotnWolfRuntime_Tick(&r, &p, in, &e) && SotnActors_CommitWolfPrimitives(s, &r));
    CHECK(!s->primitives[22].owner && s->primitives[20].ability_kind == SOTN_WOLF_ENVIRONMENT_TAG &&
          s->primitives[28].ability_kind == 11 && ActorPrimitiveGet(s, &a, 0));
    r = (SotnWolfRuntime){0};
    CHECK(SotnActors_CommitWolfPrimitives(s, &r) && !s->primitives[20].owner &&
          s->primitives[28].ability_kind == 11);
    SDL_free(bat);
    SDL_free(s);
    return true;
}
int main(void)
{
    Uint8 *d = SDL_calloc(1, 0x11200), *ex = SDL_calloc(1, 0x23220);
    if (!d || !ex)
    {
        SDL_free(d);
        SDL_free(ex);
        return 1;
    }
    for (Uint16 i = 0; i < 17; ++i)
    {
        Put32(d + 0x109f8 + i * 4U, 0x800b1000U + i * 16U);
        for (Uint16 j = 0; j < 8; ++j)
            Put16(d + 0x11000 + i * 16U + j * 2U, 1);
    }
    for (Uint16 i = 0; i < 6; ++i)
    {
        Put16(d + 0x10abc + i * 4U, 1);
        Put16(d + 0x10abe + i * 4U, 72);
    }
    Put16(d + 0x10abc + 5 * 4U, 65535);
    d[0x8410 + 13 * 28 + 12] = 3;
    const bool ok = Lifetime((SotnDataView){d, 0x11200}, (SotnDataView){ex, 0x23220}) &&
                    Pool((SotnDataView){d, 0x11200}, (SotnDataView){ex, 0x23220});
    SDL_free(d);
    SDL_free(ex);
    return ok ? 0 : 1;
}
