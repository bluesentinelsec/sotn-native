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
    const size_t inits[] = {0x6f0};
    for (Uint16 i = 0; i < SDL_arraysize(inits); ++i)
    {
        Put16(f->stage + inits[i], 0x8002);
        Put16(f->stage + inits[i] + 2, 3);
        Put16(f->stage + inits[i] + 6, 2);
    }
    const size_t streams[] = {0xd48, 0xd50};
    for (Uint16 i = 0; i < SDL_arraysize(streams); ++i)
    {
        const Uint16 count = i == 0 ? 2 : 4;
        for (Uint16 j = 0; j < count; ++j)
        {
            f->stage[streams[i] + j * 2U] = 2;
            f->stage[streams[i] + j * 2U + 1] = (Uint8)(3 + j);
        }
        f->stage[streams[i] + count * 2U] = i == 0 ? 255 : 0;
    }
    for (Uint16 i = 0; i < 4; ++i)
        Put16(f->stage + 0xbf8 + i * 2U, i * 4);
    for (Uint16 i = 0; i <= 1024; ++i)
        Put16(f->exe + 0x1dbcc + i * 2U, i * 4);
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
static bool Lifetime(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *root = &s->slots[32], *a = NULL;
    CHECK(OlroxReserved(s, root, 11, 0x31, 3, &a));
    a->state.x = 80 * 65536 + 123;
    a->state.y = 180 * 65536 + 456;
    CHECK(ActorOlroxColumn(s, a) && a->state.step == 1 && a->state.primitive_count == 24 &&
          a->state.hitbox_width == 16 && a->state.hitbox_height == 0 &&
          a->state.transform_flags == 3 && a->state.scale_y == 0 && a->state.scale_x == 256 &&
          a->olrox.timer == 12);
    SotnActorPrimitive first = ActorPrimitiveGet(s, a, 0)->state,
                       mirror = ActorPrimitiveGet(s, a, 1)->state;
    CHECK(first.active && first.page == 0x33 && first.palette == 0x214 && first.u[0] == 0 &&
          first.u[1] == 23 && mirror.u[0] == 47 && mirror.u[1] == 24);
    CHECK(!ActorPrimitiveGet(s, a, 12)->state.active &&
          ActorPrimitiveGet(s, a, 12)->state.framebuffer);
    Uint16 ticks = 0;
    while (a->state.step == 1 && ticks++ < 40)
        CHECK(ActorOlroxColumn(s, a));
    CHECK(a->state.step == 2 && ticks < 40);
    root->olrox.fire_skulls = 1;
    for (Uint16 i = 0; i < 12; ++i)
        CHECK(ActorOlroxColumn(s, a) && a->state.step == 2);
    CHECK(ActorOlroxColumn(s, a) && a->state.step == 3);
    for (Uint16 i = 0; i < 24; ++i)
        CHECK(ActorOlroxColumn(s, a));
    CHECK(a->state.step == 4 && a->olrox.column_width == 0 && a->olrox.column_spread == 48 &&
          a->olrox.column_radii[0] == 0 && a->olrox.column_radii[6] == -192 &&
          s->combat.sounds[s->combat.sound_count - 1] == 0x7ca);
    CHECK(ActorOlroxColumn(s, a) && a->olrox.column_radii[0] == 8 &&
          a->olrox.column_radii[1] == -25 && a->olrox.column_radii[6] == -190);
    first = ActorPrimitiveGet(s, a, 0)->state;
    CHECK(first.offset_x[1] == 80 && first.offset_y[0] == 172 && first.offset_y[2] == 180);
    CHECK(ActorPrimitiveGet(s, a, 12)->state.active && !ActorPrimitiveGet(s, a, 22)->state.active);
    ++root->state.generation; // Released geometry no longer dereferences its parent.
    ticks = 0;
    while (!a->remove && ticks++ < 400)
        CHECK(ActorOlroxColumn(s, a));
    CHECK(a->remove && ticks < 400 && a->state.scale_x == 0 && a->state.hitbox_height > 0 &&
          a->state.hitbox_y < 0 && a->olrox.column_radii[0] == 640);
    Close(f, s);
    return true;
}
static bool OwnershipAndFailures(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *root = &s->slots[32], *a = NULL;
    CHECK(OlroxReserved(s, root, 11, 0x31, 0, &a));
    ++root->state.generation;
    CHECK(ActorOlroxColumn(s, a) && a->remove && !a->state.primitive_count);
    CHECK(OlroxReserved(s, root, 12, 0x31, 4, &a));
    CHECK(!ActorOlroxColumn(s, a));
    a->state.parameters = 0;
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 99;
    CHECK(ActorOlroxColumn(s, a) && a->remove && !a->state.primitive_count);
    SDL_zero(s->primitives);
    CHECK(OlroxReserved(s, root, 13, 0x31, 1, &a));
    CHECK(ActorOlroxColumn(s, a));
    a->state.step = 2;
    a->olrox.timer = 1;
    root->olrox.fire_skulls = 1;
    s->olrox_flags = 1;
    CHECK(ActorOlroxColumn(s, a) && !a->remove && a->olrox.timer == 0);
    CHECK(ActorOlroxColumn(s, a) && a->remove);
    a->remove = false;
    a->state.step = 4;
    a->state.primitive_count = 0;
    CHECK(!ActorOlroxColumn(s, a));
    a->state.step = 9;
    CHECK(!ActorOlroxColumn(s, a));
    a->state.generation = 0;
    CHECK(!ActorOlroxColumn(s, a));
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool okay = Lifetime(f) && OwnershipAndFailures(f);
    SDL_free(f);
    return okay ? 0 : 1;
}
