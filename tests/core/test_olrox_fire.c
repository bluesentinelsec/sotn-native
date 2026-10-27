#include "olrox_internal.h"
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
    Uint8 dra[0x1000], stage[0x4000], exe[0x24000], tiles[32 * 32 * 2], types[2];
    SotnActorLayout *layout;
} Fixture;
static void Put16(Uint8 *p, Sint32 n)
{
    p[0] = (Uint8)n;
    p[1] = (Uint8)((Uint32)n >> 8);
}
static void Put32(Uint8 *p, Uint32 n)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(n >> (i * 8));
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 12);
    Put16(f->dra + 0x20c, 3);
    const size_t inits[] = {0x60c, 0x6fc};
    for (Uint16 i = 0; i < SDL_arraysize(inits); ++i)
    {
        Put16(f->stage + inits[i], 0x8002);
        Put16(f->stage + inits[i] + 2, 3);
        Put16(f->stage + inits[i] + 6, 2);
    }
    const size_t streams[] = {0x1400, 0x1420, 0x1440};
    for (Uint16 i = 0; i < SDL_arraysize(streams); ++i)
    {
        for (Uint16 j = 0; j < 4; ++j)
        {
            f->stage[streams[i] + j * 2U] = 2;
            f->stage[streams[i] + j * 2U + 1] = (Uint8)(3 + j);
        }
        f->stage[streams[i] + 8] = 0;
    }
    for (Uint16 i = 0; i < 3; ++i)
    {
        Put16(f->stage + 0x1350 + i * 12U, 0x8002 + i);
        Put16(f->stage + 0x1352 + i * 12U, 0x20 + i);
        Put16(f->stage + 0x1354 + i * 12U, i);
        Put16(f->stage + 0x1356 + i * 12U, i == 1 ? 0x30 : 0);
        Put32(f->stage + 0x1358 + i * 12U, 0x80181400 + i * 32U);
    }
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 30, out));
    CHECK(SotnActorLayout_OpenStage((SotnDataView){f->stage, sizeof(f->stage)}, 30, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *out, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 32}, 0,
        0));
    (*out)->executable = (SotnDataView){f->exe, sizeof(f->exe)};
    (*out)->slots[32] = (Actor){
        .slot = 32,
        .state = {
            .id = 0x2b, .generation = 3, .x = 128 * 65536, .y = 128 * 65536, .priority = 136}};
    (*out)->player = (SotnPlayerState){.x = 160 * 65536, .y = 128 * 65536};
    return true;
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    f->layout = NULL;
}
static bool Fireballs(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    for (Uint16 facing = 0; facing < 2; ++facing)
    {
        Actor *a = &s->slots[96];
        *a = (Actor){.slot = 96,
                     .state = {.id = 0x30,
                               .generation = (Uint32)(20 + facing),
                               .x = 200 * 65536 + 123,
                               .y = 100 * 65536 + 456,
                               .facing_left = facing != 0,
                               .priority = 136}};
        CHECK(ActorOlroxFire(s, a) && a->state.step == 1 && a->state.primitive_count == 1);
        const Sint32 direction = facing ? -1 : 1;
        const Sint32 start = (200 - direction * 28) * 65536 + 123;
        CHECK(a->state.x == start && a->state.y == 88 * 65536 + 456 && a->vx == direction * 32768 &&
              a->state.hitbox_width == 0 && a->state.hitbox_height == 5 &&
              a->olrox.fire_length == direction * 48 * 65536);
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
        CHECK(p && p->state.active && p->state.quad && p->state.modulate && p->state.gouraud &&
              p->state.blend && p->state.page == 0x32 && p->state.palette == 0x212 &&
              p->state.priority == 140 && p->state.offset_x[1] == 0 &&
              p->state.vertex_red[0] == 0 && p->state.vertex_red[1] == 160);
        CHECK(ActorOlroxFire(s, a) && a->state.x == start + direction * 32768 &&
              a->vx == direction * 40960 && a->state.hitbox_width == 13 &&
              a->state.hitbox_x == 36 && p->state.offset_x[1] == direction * 53 &&
              p->state.offset_y[0] == -8 && p->state.offset_y[3] == 8);
        a->vx = 0;
        s->camera_x = 50;
        a->state.x = (50 + direction * 640) * 65536;
        CHECK(ActorOlroxFire(s, a) && !a->remove);
        a->vx = 0;
        a->state.x = (50 + direction * 641) * 65536;
        CHECK(ActorOlroxFire(s, a) && a->remove);
        ActorPrimitivesClear(s, a);
    }
    s->slots[96] = (Actor){.slot = 96, .state = {.id = 0x30, .generation = 30}};
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 99;
    CHECK(ActorOlroxFire(s, &s->slots[96]) && s->slots[96].remove &&
          !s->slots[96].state.primitive_count);
    Close(f, s);
    return true;
}
static bool Effects(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    for (Uint16 kind = 0; kind < 3; ++kind)
    {
        Actor *a = &s->slots[160];
        *a = (Actor){.slot = 160,
                     .state = {.id = 0x47,
                               .generation = 2,
                               .parameters = (Uint16)(0x9400U | kind),
                               .y = 100 * 65536}};
        CHECK(ActorOlroxFire(s, a) && a->state.step == kind + 1 && a->state.priority == 0x94 &&
              a->state.palette == 0x226U + kind && a->state.texture_offset == 0x20U + kind);
        Uint32 ticks = 0;
        while (!a->remove && ticks++ < 150)
            CHECK(ActorOlroxFire(s, a));
        CHECK(a->remove && ticks < 150);
        if (kind == 1)
            CHECK(a->state.transform_flags == 11 && a->state.scale_y == 288 &&
                  a->state.opacity == 0);
        if (kind < 2)
            CHECK(a->state.y < 100 * 65536);
    }
    Actor *a = &s->slots[160];
    *a = (Actor){.state = {.id = 0x47, .generation = 2, .parameters = 0x12}};
    CHECK(ActorOlroxFire(s, a) && a->state.palette == 0x819f && a->state.blend_mode == 0x10 &&
          a->state.facing_left);
    CHECK(ActorOlroxFire(s, a) && a->state.transform_flags == 8 && a->state.opacity == 128);
    a->pose = 5;
    a->remaining = 2;
    CHECK(ActorOlroxFire(s, a) && a->substep == 2);
    CHECK(ActorOlroxFire(s, a) && a->state.opacity == 124);
    *a = (Actor){.state = {.id = 0x47, .generation = 2, .parameters = 3}};
    CHECK(!ActorOlroxFire(s, a));
    a->state.parameters = 0;
    Put32(f->stage + 0x1358, 0x80190000);
    CHECK(!ActorOlroxFire(s, a));
    a->state.step = 9;
    CHECK(!ActorOlroxFire(s, a));
    a->state.id = 0x30;
    CHECK(!ActorOlroxFire(s, a));
    a->state.step = 1;
    CHECK(!ActorOlroxFire(s, a)); // No owned primitive.
    a->state.generation = 0;
    CHECK(!ActorOlroxFire(s, a));
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool okay = Fireballs(f) && Effects(f);
    SDL_free(f);
    return okay ? 0 : 1;
}
