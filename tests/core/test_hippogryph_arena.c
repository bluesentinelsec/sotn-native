#include "actors_internal.h"
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
    Uint8 dra[0x1000], stage[0x2000], tiles[32 * 16 * 2], types[2048];
} Fixture;
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static void Put32(Uint8 *p, Uint32 value)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(value >> (i * 8U));
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    Put32(f->stage + 0xb74, 1);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->stage + 0x4cc, 0x8001);
    f->types[0x4b3] = 3;
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 25, out));
    SotnActorLayout *layout = NULL;
    CHECK(SotnActorLayout_OpenStage((*out)->nz0, 25, 0, &layout));
    const bool entered = SotnActors_Enter(
        *out, layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16}, 0,
        0);
    SotnActorLayout_Close(layout);
    CHECK(entered && (*out)->hippogryph_scene == 1);
    return true;
}
static Actor Door(Uint16 side)
{
    return (Actor){.slot = (Uint16)(64 + side * 2U),
                   .state = {.id = 0x21,
                             .generation = 10U + side,
                             .parameters = side,
                             .x = 123 * 65536 + 7,
                             .y = 45 * 65536 + 9}};
}
static bool DoorCycle(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor doors[2] = {Door(0), Door(1)};
    for (Uint16 side = 0; side < 2; ++side)
    {
        Actor *a = &doors[side], *child = &s->slots[a->slot + 1U];
        CHECK(ActorHippogryphArena(s, a) && a->state.step == 1 && a->state.priority == 92 &&
              a->state.frame == 12 + side && a->state.x == (side ? 464 : 16) * 65536 + 7 &&
              a->state.y == 32 * 65536 + 9);
        CHECK(child->state.id == 0x21 && child->state.parameters == 2 &&
              child->state.x == (side ? 480 : 16) * 65536 && child->state.y == 160 * 65536 &&
              !child->state.facing_left &&
              child->enemy_state.owner_generation == a->state.generation);
        CHECK(ActorHippogryphArena(s, child) && child->state.frame == 14);
        CHECK(ActorHippogryphArena(s, a) && a->state.step == 1);
    }
    s->hippogryph_encounter = 1;
    for (Uint16 side = 0; side < 2; ++side)
    {
        Actor *a = &doors[side], *child = &s->slots[a->slot + 1U];
        CHECK(ActorHippogryphArena(s, a) && a->state.step == 2);
        CHECK(ActorHippogryphArena(s, child) && child->state.step == 8);
        for (Uint16 tick = 0; tick < 34; ++tick)
        {
            CHECK(ActorHippogryphArena(s, a));
            CHECK(ActorHippogryphArena(s, child));
        }
        CHECK(a->state.step == 3 && a->state.y == 70 * 65536 && child->state.step == 9 &&
              child->state.y == 129 * 65536);
    }
    for (Uint16 side = 0; side < 2; ++side)
        for (Uint16 row = 0; row < 4; ++row)
            CHECK(Read16(s->mutable_tiles + ((side ? 222U : 193U) + row * 32U) * 2U) == 0x4b3);
    s->hippogryph_encounter = 0;
    CHECK(ActorHippogryphArena(s, &doors[0]) && doors[0].state.step == 4);
    CHECK(Read16(s->mutable_tiles + 193U * 2U) == 0 &&
          Read16(s->mutable_tiles + 222U * 2U) == 0x4b3);
    CHECK(ActorHippogryphArena(s, &doors[1]) && doors[1].state.step == 4);
    for (Uint16 side = 0; side < 2; ++side)
    {
        Actor *a = &doors[side], *child = &s->slots[a->slot + 1U];
        CHECK(ActorHippogryphArena(s, child) && child->state.step == 10);
        for (Uint16 tick = 0; tick < 52; ++tick)
        {
            CHECK(ActorHippogryphArena(s, a));
            CHECK(ActorHippogryphArena(s, child));
        }
        CHECK(a->state.step == 5 && a->state.y == 32 * 65536 + 49152 && child->state.step == 11 &&
              child->state.y == 160 * 65536 + 16384);
        for (Uint16 row = 0; row < 4; ++row)
            CHECK(!Read16(s->mutable_tiles + ((side ? 222U : 193U) + row * 32U) * 2U));
    }
    SotnActors_Destroy(s);
    return true;
}
static bool ScrolledDoors(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    s->camera_x = 37;
    s->camera_y = 4;
    Actor a = Door(1);
    CHECK(ActorHippogryphArena(s, &a) && a.state.x == 464 * 65536 + 7 &&
          a.state.y == 32 * 65536 + 9);
    Actor *child = &s->slots[a.slot + 1U];
    CHECK(child->state.x == 480 * 65536 && child->state.y == 164 * 65536);
    CHECK(ActorHippogryphArena(s, child));
    s->hippogryph_encounter = 1;
    for (Uint16 i = 0; i < 50; ++i)
    {
        CHECK(ActorHippogryphArena(s, &a));
        CHECK(ActorHippogryphArena(s, child));
    }
    CHECK(a.state.step == 3 && a.state.y == 74 * 65536 && child->state.step == 9 &&
          child->state.y == 133 * 65536);
    s->hippogryph_encounter = 0;
    for (Uint16 i = 0; i < 65; ++i)
    {
        CHECK(ActorHippogryphArena(s, &a));
        CHECK(ActorHippogryphArena(s, child));
    }
    CHECK(a.state.step == 5 && Pixel(a.state.y) == 32 && child->state.step == 11 &&
          Pixel(child->state.y) == 160);
    SotnActors_Destroy(s);
    return true;
}
static bool Music(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = {.state = {.id = 0x29, .generation = 1}};
    CHECK(ActorHippogryphArena(s, &a) && a.state.step == 1 && !s->combat.music_change);
    s->hippogryph_encounter = 1;
    CHECK(ActorHippogryphArena(s, &a) && a.state.step == 2 && s->hippogryph_stop_music &&
          s->hippogryph_music == 0x334 && !s->combat.music_change);
    s->music_playing = true;
    CHECK(ActorHippogryphArena(s, &a) && a.state.step == 2 && !s->combat.music_change);
    s->music_playing = false;
    s->death_event.cd_busy = true;
    CHECK(ActorHippogryphArena(s, &a) && a.state.step == 2);
    s->death_event.cd_busy = false;
    CHECK(ActorHippogryphArena(s, &a) && a.state.step == 3 && !s->hippogryph_stop_music &&
          s->combat.music_change && s->combat.music == 0x334);
    s->combat = (SotnActorCombat){0};
    CHECK(ActorHippogryphArena(s, &a) && a.state.step == 3 && !s->combat.cd_command);
    s->hippogryph_scene = 0;
    CHECK(ActorHippogryphArena(s, &a) && a.state.step == 4 && s->combat.cd_command == 0x91);
    s->combat = (SotnActorCombat){0};
    CHECK(ActorHippogryphArena(s, &a) && a.state.step == 4 && !s->combat.cd_command);
    a = (Actor){.state = {.id = 0x29, .generation = 2}};
    CHECK(ActorHippogryphArena(s, &a) && a.state.step == 2 && s->hippogryph_stop_music);
    SotnActors_Destroy(s);
    return true;
}
static bool Failures(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = Door(0);
    CHECK(!ActorHippogryphArena(NULL, &a) && !ActorHippogryphArena(s, NULL));
    a.state.parameters = 3;
    CHECK(!ActorHippogryphArena(s, &a));
    a = Door(0);
    s->slots[65].state = (SotnActorState){.id = 3, .generation = 88};
    CHECK(!ActorHippogryphArena(s, &a) && s->slots[65].state.generation == 88 &&
          s->slots[65].state.id == 3);
    s->slots[65] = (Actor){0};
    a = Door(0);
    a.slot = 191;
    CHECK(!ActorHippogryphArena(s, &a));
    a = Door(0);
    a.state.generation = 0;
    CHECK(!ActorHippogryphArena(s, &a));
    a = Door(0);
    a.state.step = 1;
    s->hippogryph_encounter = 1;
    s->map.width = 16;
    CHECK(!ActorHippogryphArena(s, &a) && a.state.step == 1 && !Read16(s->mutable_tiles + 386));
    s->map.width = 32;
    s->map.types.size = 1;
    CHECK(!ActorHippogryphArena(s, &a) && a.state.step == 1);
    a.state.step = 2;
    a.state.y = SDL_MAX_SINT32;
    CHECK(!ActorHippogryphArena(s, &a) && a.state.y == SDL_MAX_SINT32);
    a.state.step = 4;
    a.state.y = SDL_MIN_SINT32;
    CHECK(!ActorHippogryphArena(s, &a) && a.state.y == SDL_MIN_SINT32);
    a.state.id = 0x22;
    CHECK(!ActorHippogryphArena(s, &a));
    s->stage_id = 6;
    CHECK(!ActorHippogryphArena(s, &a));
    s->stage_id = 25;
    Put32(f->stage + 0xb74, 2);
    CHECK(!SotnActors_Enter(s, NULL, s->map, 0, 0) && s->hippogryph_scene == 1);
    SotnActors_Destroy(s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = DoorCycle(f) && ScrolledDoors(f) && Music(f) && Failures(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_HIPPOGRYPH_ARENA: PASS");
    return 0;
}
