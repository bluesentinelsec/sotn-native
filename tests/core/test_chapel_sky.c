#include "actors_internal.h"
#include "boss_projection.h"
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    Uint8 dra[0x1000], stage[0x2600], tiles[32 * 32 * 2], types[2];
    SotnActorLayout *layout;
} Fixture;
static void Put16(Uint8 *p, Sint32 v)
{
    p[0] = (Uint8)(Uint16)v;
    p[1] = (Uint8)((Uint16)v >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    // Deliberately distinct model width, UVs, layer order/heights and grid pattern.
    for (Uint16 i = 0; i < 4; ++i)
    {
        Put16(f->stage + 0x2464 + i * 8U, i & 1U ? 96 : -96);
        Put16(f->stage + 0x2468 + i * 8U, i >= 2 ? 256 : 0);
    }
    for (Uint16 kind = 1; kind < 7; ++kind)
        for (Uint16 vertex = 0; vertex < 4; ++vertex)
            Put16(f->stage + 0x242c + kind * 8U + vertex * 2U,
                  (Sint32)(kind * 16U + (vertex & 1U) * 8U + (vertex >= 2 ? 24U : 0U) * 256U));
    for (Uint16 layer = 0; layer < 3; ++layer)
    {
        Put32(f->stage + 0x2544 + layer * 8U, 0x80182484U + (2U - layer) * 64U);
        Put16(f->stage + 0x2548 + layer * 8U, layer == 2 ? 120 : layer ? -288 : -192);
        Put16(f->stage + 0x254a + layer * 8U, 31 - layer);
        for (Uint16 i = 0; i < 64; ++i)
            f->stage[0x2484 + layer * 64U + i] = (Uint8)((i + layer) % 6U + 1U);
    }
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 6, out));
    CHECK(SotnActorLayout_OpenStage((SotnDataView){f->stage, sizeof(f->stage)}, 6, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *out, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 32}, 0,
        0));
    (*out)->slots[16] = (Actor){
        .slot = 16, .dynamic = true, .state = {.id = 0x1f, .generation = 1, .x = 200 * 65536}};
    return true;
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    f->layout = NULL;
}
static bool Projection(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    SotnProjectedPoint point[4] = {0}, saved[4] = {0};
    CHECK(SotnChapel_ProjectCloud(s->nz0, 0, 0, 256, point) && point[0].x == 32 &&
          point[1].x == 224 && point[2].x == 80 && point[3].x == 176 && point[3].y == 160);
    CHECK(SotnChapel_ProjectCloud(s->nz0, -16384, 512, 192, point) && point[0].x == -1024);
    SDL_memcpy(saved, point, sizeof(saved));
    CHECK(!SotnChapel_ProjectCloud(s->nz0, 0, 0, 191, point) &&
          !SDL_memcmp(saved, point, sizeof(saved)));
    CHECK(!SotnChapel_ProjectCloud(s->nz0, 0, 0, 2241, point));
    CHECK(!SotnChapel_ProjectCloud(s->nz0, 16385, 0, 256, point));
    CHECK(!SotnChapel_ProjectCloud(s->nz0, 0, -513, 256, point));
    CHECK(!SotnChapel_ProjectCloud(s->nz0, 0, 0, 256, NULL));
    CHECK(!SotnChapel_ProjectCloud((SotnDataView){f->stage, 0x255d}, 0, 0, 256, point));
    Put16(f->stage + 0x255c, 1);
    CHECK(!SotnChapel_ProjectCloud(s->nz0, 0, 0, 256, point));
    Put16(f->stage + 0x255c, 0);
    Put16(f->stage + 0x2466, 1);
    CHECK(!SotnChapel_ProjectCloud(s->nz0, 0, 0, 256, point));
    Put16(f->stage + 0x2466, 0);
    Put16(f->stage + 0x2468, -1);
    CHECK(!SotnChapel_ProjectCloud(s->nz0, 0, 0, 256, point));
    CHECK(!SDL_memcmp(saved, point, sizeof(saved)));
    Close(f, s);
    return true;
}
static bool Clouds(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[16];
    CHECK(ActorChapel(s, a) && a->state.step == 1 && a->state.primitive_count == 96 &&
          a->clouds.x[0] == 65536U && a->clouds.x[1] == 131072U && !a->clouds.x[2] &&
          a->clouds.y[0] == 327680U && a->clouds.y[1] == 196608U && a->clouds.y[2] == 327680U &&
          s->backdrop_color == (6U | (5U << 5) | (8U << 10)));
    Uint16 visible = 0;
    for (Uint16 i = 0; i < 96; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        CHECK(p != NULL);
        if (!p->state.active)
            continue;
        ++visible;
        CHECK(p->state.screen_space && p->state.quad && p->state.textured && p->state.gouraud &&
              p->state.modulate && !p->state.blend && p->state.page == 15 &&
              p->state.palette == 0xc0 && p->state.priority <= 31 && p->state.priority >= 29 &&
              p->state.u[0] >= 16 && p->state.u[0] <= 96 && p->state.v[3] == 24 &&
              p->state.offset_x[3] >= 0 && p->state.offset_x[2] <= 256 &&
              p->state.vertex_red[0] >= p->state.vertex_red[2]);
    }
    CHECK(visible > 0);
    const SotnActorPrimitive before = ActorPrimitiveGet(s, a, 0)->state;
    s->camera_x = 64;
    CHECK(ActorChapel(s, a) && ActorPrimitiveGet(s, a, 0)->state.offset_x[0] != before.offset_x[0]);
    a->clouds.x[0] = 0xffff0000U;
    a->clouds.x[1] = 0xfffe0000U;
    a->clouds.y[0] = a->clouds.y[2] = 0xfffb0000U;
    a->clouds.y[1] = 0xfffd0000U;
    CHECK(ActorChapel(s, a) && !a->clouds.x[0] && !a->clouds.x[1] && !a->clouds.y[0] &&
          !a->clouds.y[1] && !a->clouds.y[2]);
    // Sparse grids exercise zero skipping, direction reversal and hidden-tail cleanup.
    SDL_memset(f->stage + 0x2484, 0, 192);
    for (Uint16 row = 0; row < 24; ++row)
        f->stage[0x2484 + row * 8U + 7] = 6;
    CHECK(ActorChapel(s, a));
    bool hidden = false;
    for (Uint16 i = 0; i < 96; ++i)
        hidden = hidden || !ActorPrimitiveGet(s, a, i)->state.active;
    CHECK(hidden);
    CHECK(SotnActors_Enter(s, f->layout, s->map, 0, 0));
    Uint16 backdrop = 0xffff;
    CHECK(SotnActors_GetBackdrop(s, &backdrop) && !backdrop && !s->slots[16].state.generation &&
          !s->slots[16].clouds.x[0]);
    Close(f, s);
    return true;
}
static bool LandAndFailures(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[16];
    a->state.id = 0x20;
    CHECK(ActorChapel(s, a) && a->state.step == 1 && a->state.primitive_count == 5);
    const SotnActorPrimitive first = ActorPrimitiveGet(s, a, 0)->state;
    const SotnActorPrimitive last = ActorPrimitiveGet(s, a, 4)->state;
    CHECK(first.screen_space && first.offset_x[0] == 0 && last.offset_x[3] == 310 &&
          first.offset_y[0] == 192 && last.offset_y[3] == 228 && first.palette == 0xc5 &&
          first.priority == 16 && first.u[0] == 65 && first.u[1] == 127 && first.v[3] == 198);
    s->camera_x = 128;
    s->camera_y = 64;
    CHECK(ActorChapel(s, a) &&
          !SDL_memcmp(&first, &ActorPrimitiveGet(s, a, 0)->state, sizeof(first)));
    a->state.step = 2;
    CHECK(!ActorChapel(s, a));
    Close(f, s);
    CHECK(Open(f, &s));
    a = &s->slots[16];
    Put32(f->stage + 0x2544, 0x80182485);
    CHECK(!ActorChapel(s, a));
    Put32(f->stage + 0x2544, 0x80182484);
    Put16(f->stage + 0x254a, 512);
    CHECK(!ActorChapel(s, a));
    Put16(f->stage + 0x254a, 31);
    f->stage[0x2484] = 7;
    CHECK(!ActorChapel(s, a));
    SDL_memset(f->stage + 0x2484, 0, 64);
    CHECK(!ActorChapel(s, a));
    CHECK(!ActorChapelSky(NULL, a) && !ActorChapelSky(s, NULL));
    s->stage_id = 2;
    CHECK(!ActorChapelSky(s, a));
    s->stage_id = 6;
    a->state.id = 0x21;
    CHECK(!ActorChapelSky(s, a));
    Close(f, s);
    CHECK(Open(f, &s));
    a = &s->slots[16];
    for (Uint16 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 7;
    CHECK(ActorChapel(s, a) && a->remove && !a->state.primitive_count);
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    const bool ok = f && Projection(f) && Clouds(f) && LandAndFailures(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_CHAPEL_SKY: PASS");
    return 0;
}
