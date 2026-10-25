#include "actors_internal.h"
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    Uint8 dra[0x400], stage[0x2000], exe[0x23220], tiles[512], types[0x600];
    SotnActorLayout *layout;
} Fixture;
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static void Put32(Uint8 *p, Uint32 value)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(value >> (i * 8));
}
static Uint16 Tile(SotnActors *s, Uint32 index)
{
    return Read16(s->map.tiles.bytes + index * 2);
}
static bool Step(SotnActors *s, Fixture *f, Uint64 tick)
{
    CHECK(SotnActors_SetServices(s, (SotnDataView){f->exe, sizeof(f->exe)}, tick, true));
    CHECK(
        SotnActors_Tick(s, (SotnPlayerState){.x = 128 * 65536, .y = 160 * 65536, .hp = 70}, 0, 0));
    return true;
}
static bool Clock(Fixture *f)
{
    Put32(f->dra + 0x98, 0x800a0200);
    Put32(f->dra + 0x224, 0x04002000);
    Put32(f->stage + 28, 0x80180040);
    for (Uint32 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    Put16(f->stage + 0x1c3c, 80);
    Put16(f->stage + 0x1c3e, (Uint16)-80);
    Put16(f->stage + 0x1c40, 128);
    Put16(f->stage + 0x1c42, (Uint16)-128);
    Put32(f->stage + 0x1c54, 32768);
    Put32(f->stage + 0x1c58, (Uint32)-32768);
    Put16(f->stage + 0x1c5c, 41);
    Put16(f->stage + 0x1c5e, 42);
    f->stage[0x1c78] = f->stage[0x1c84] = 1;
    f->stage[0x1c79] = f->stage[0x1c85] = 17;
    Put16(f->stage + 0x1c90, 5);
    SotnActors *s = NULL;
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 0, &s));
    CHECK(SotnActorLayout_OpenStage(s->nz0, 0, 0, &f->layout));
    CHECK(SotnActors_Enter(
        s, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 16, 16}, 0,
        0));
    s->slots[16] =
        (Actor){.state = {.id = 0x19, .generation = 100, .x = 128 * 65536, .y = 104 * 65536},
                .slot = 16,
                .dynamic = true};
    CHECK(Step(s, f, 0));
    CHECK(s->combat.music_change && !s->combat.music);
    CHECK(s->clock_statues[1] && !s->clock_statues[0]);
    CHECK(Tile(s, 2) == 0 && Tile(s, 12) == 0x597 && Tile(s, 0xc4) == 0x597);
    CHECK(s->slots[21].state.id == 0x1a && s->slots[26].state.id == 0x20);
    CHECK(s->slots[26].state.opacity_set && s->slots[26].state.opacity == 0 &&
          s->slots[26].state.palette == 0);
    CHECK(s->slots[25].state.palette == 0x804b);
    CHECK(Step(s, f, 3600));
    CHECK(s->slots[18].state.step == 2 && s->slots[21].state.rotation == 68);
    for (Uint32 i = 0; i < 96; ++i)
        CHECK(Step(s, f, 3601 + i));
    CHECK(s->slots[18].state.step == 1 && Tile(s, 2) == 0x597);
    CHECK(Step(s, f, 36000));
    CHECK(s->slots[23].clock.state == 1 && s->slots[23].state.step == 2);
    for (Uint32 i = 0; i < 256; ++i)
        CHECK(Step(s, f, 36001 + i));
    CHECK(s->slots[23].state.step == 1 && s->slots[23].state.frame == 42 &&
          s->slots[23].state.opacity == 128);
    CHECK(s->slots[23].state.scale_x == 256);
    s->world->flags[SOTN_CLOCK_FLOOR] = 1;
    CHECK(Step(s, f, 36260));
    for (Uint32 i = 0; i < 97; ++i)
        CHECK(Step(s, f, 36261 + i));
    CHECK(Tile(s, 0xc4) == 5 && Tile(s, 0xc5) == 0);
    CHECK(!SotnActors_SetServices(s, (SotnDataView){f->exe, sizeof(f->exe)},
                                  99999ULL * 3600 * 60 + 1, true));
    s->world->flags[SOTN_CLOCK_FLOOR] = 2;
    CHECK(!SotnWorld_Validate(s->world));
    s->world->flags[SOTN_CLOCK_FLOOR] = 1;
    Actor bad = {.state = {.id = 0x1c, .parameters = 2, .generation = 200}};
    CHECK(!ActorClock(s, &bad));
    Actor tick = {.state = {.id = 0x42, .parameters = 1, .generation = 201}};
    s->ticks = 60;
    s->player.x = 600 * 65536;
    CHECK(ActorClock(s, &tick));
    CHECK(s->combat.pans[s->combat.sound_count - 1] == 8 &&
          s->combat.volumes[s->combat.sound_count - 1] == 60);
    Actor pendulum = {.state = {.id = 0x2b, .generation = 202}};
    s->ticks = 30;
    CHECK(ActorClock(s, &pendulum) && pendulum.state.rotation == 96 && pendulum.state.frame == 33);
    CHECK(pendulum.state.opacity_set && pendulum.state.opacity == 96);
    s->ticks = 60;
    const Uint16 sounds = s->combat.sound_count;
    CHECK(ActorClock(s, &pendulum) && pendulum.state.rotation == 0 &&
          s->combat.sound_count == sounds + 1);
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (f == NULL)
        return 1;
    const bool ok = Clock(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
