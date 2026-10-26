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
    Uint8 stage[0x2000], dra[0x400], exe[0x23220], tiles[96 * 64 * 2], types[1];
    SotnActorLayout *layout;
} Fixture;
static void Put16(Uint8 *p, Sint32 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)((Uint32)v >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
static bool Open(Fixture *f, SotnActors **out, Uint16 id, Uint16 params)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, -2);
    Put16(f->stage + 0x302, -2);
    Put16(f->stage + 0x30a, -1);
    Put16(f->stage + 0x30c, -1);
    for (Uint16 i = 0; i < 4096; ++i)
    {
        const Uint16 phase = i & 1023U;
        const Sint32 sine = i < 1024   ? phase * 4
                            : i < 2048 ? 4096 - phase * 4
                            : i < 3072 ? -(Sint32)phase * 4
                                       : -4096 + phase * 4;
        Put16(f->exe + 0x1ea10 + i * 4U, sine);
        const Uint16 c = (i + 1024) & 4095U, cp = c & 1023U;
        const Sint32 cosine = c < 1024   ? cp * 4
                              : c < 2048 ? 4096 - cp * 4
                              : c < 3072 ? -(Sint32)cp * 4
                                         : -4096 + cp * 4;
        Put16(f->exe + 0x1ea12 + i * 4U, cosine);
    }
    for (Uint16 i = 0; i < 19; ++i)
    {
        Put32(f->stage + 0xc50 + i * 4U, 0x80180bb8 + i * 8U);
        Put16(f->stage + 0xbb8 + i * 8U, (i % 2) * 24 - 12);
        Put16(f->stage + 0xbba + i * 8U, (i / 2) * 16 - 80);
        Put16(f->stage + 0xbbc + i * 8U, 0);
    }
    for (Uint16 kind = 0; kind < 3; ++kind)
        for (Uint16 side = 0; side < 2; ++side)
        {
            size_t offset = kind == 2 ? 0xcb0 : kind == 1 ? 0xca8 : 0xc9c;
            size_t count = kind == 2 ? 20 : kind == 1 ? 4 : 5;
            for (size_t i = 0; i < count; ++i)
                f->stage[offset + side * count + i] = (Uint8)(i % 4);
        }
    for (Uint16 variant = 0; variant < 2; ++variant)
    {
        Uint8 *p = f->stage + 0xcd8 + variant * 14U;
        Put16(p, 0);
        Put16(p + 2, 256 + variant * 128);
        Put16(p + 4, 20 + variant);
        Put16(p + 6, 80);
        Put16(p + 8, 96);
        Put16(p + 10, 24);
        Put16(p + 12, 112);
    }
    for (Uint16 i = 0; i < 11; ++i)
    {
        Put16(f->stage + 0xcf4 + i * 6U, 80 + i * 32);
        Put16(f->stage + 0xcf6 + i * 6U, 700);
        Put16(f->stage + 0xcf8 + i * 6U, i & 1U);
    }
    Put16(f->stage + 0xd36, -1);
    for (Uint16 i = 0; i < 7; ++i)
    {
        Put16(f->stage + 0xb7c + i * 2U, 0x8040 + i);
        Put32(f->stage + 0xb8c + i * 4U, 1 + i);
    }
    f->stage[0xba8] = 2;
    f->stage[0xba9] = 8;
    f->stage[0xbb4] = 2;
    f->stage[0xbb5] = 10;
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 4, out));
    CHECK(SotnActorLayout_OpenStage((SotnDataView){f->stage, sizeof(f->stage)}, 4, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *out, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 96, 64}, 0,
        0));
    SotnActors *s = *out;
    s->executable = (SotnDataView){f->exe, sizeof(f->exe)};
    s->player = (SotnPlayerState){.x = 128 * 65536, .y = 128 * 65536, .priority = 0x80};
    s->slots[16] = (Actor){.slot = 16,
                           .dynamic = true,
                           .state = {.id = id,
                                     .parameters = params,
                                     .generation = 1,
                                     .x = 128 * 65536 + 123,
                                     .y = 192 * 65536 + 456}};
    return true;
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
}
static bool Houses(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    SotnActors *s = NULL;
    CHECK(f && Open(f, &s, 0x1e, 0));
    Actor *a = &s->slots[16];
    CHECK(ActorNo2Scene(s, a) && a->state.primitive_count == 16 && !a->state.frame);
    CHECK(ActorNo2Scene(s, a));
    Uint16 active = 0;
    for (Uint16 i = 0; i < 16; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        CHECK(p);
        active += p->state.active;
        CHECK(p->state.priority == 88 && p->state.page == 15 && p->state.screen_space);
    }
    CHECK(active == 16);
    a->state.x = -600 * 65536;
    CHECK(ActorNo2Scene(s, a));
    for (Uint16 i = 0; i < 16; ++i)
        CHECK(!ActorPrimitiveGet(s, a, i)->state.active);
    a->state.x = 128 * 65536;
    CHECK(ActorNo2Scene(s, a));
    SotnProjectedPoint points[19] = {0}, before[19] = {0};
    CHECK(SotnNo2_ProjectHouse(s->executable, s->nz0, 0, 128, 192, points));
    SDL_memcpy(before, points, sizeof(before));
    Put32(f->stage + 0xc50 + 18 * 4U, 0xfffffff0);
    CHECK(!SotnNo2_ProjectHouse(s->executable, s->nz0, 0, 128, 192, points) &&
          SDL_memcmp(points, before, sizeof(points)) == 0);
    CHECK(!SotnNo2_ProjectHouse(s->executable, s->nz0, 2, 128, 192, points));
    Close(f, s);
    CHECK(Open(f, &s, 0x1f, 0));
    a = &s->slots[16];
    CHECK(ActorNo2Scene(s, a) && a->state.step == 1);
    for (Uint16 i = 0; i < 11; ++i)
    {
        Actor *child = &s->slots[160 + i];
        CHECK(child->state.id == 0x1e && child->state.parameters == (i & 1U) &&
              Pixel(child->state.x) == 80 + i * 32 && (child->state.x & 65535) == 123);
        CHECK(ActorNo2Scene(s, child));
        CHECK(ActorNo2Scene(s, child));
    }
    CHECK(ActorNo2Scene(s, a) && !s->slots[171].state.generation);
    Close(f, s);
    CHECK(Open(f, &s, 0x1f, 0));
    a = &s->slots[16];
    for (Uint16 i = 162; i < 192; ++i)
        s->slots[i].state.generation = 99;
    CHECK(ActorNo2Scene(s, a) && s->slots[160].state.id == 0x1e && s->slots[161].state.id == 0x1e &&
          s->slots[162].state.id == 0);
    Close(f, s);
    CHECK(Open(f, &s, 0x1f, 0));
    Put16(f->stage + 0xd36, 0);
    CHECK(!ActorNo2Scene(s, &s->slots[16]) && !s->slots[160].state.generation);
    Close(f, s);
    SDL_free(f);
    return true;
}
static bool Scenery(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    SotnActors *s = NULL;
    CHECK(f);
    for (Uint16 variant = 0; variant < 2; ++variant)
    {
        CHECK(Open(f, &s, 0x19, variant));
        Actor *a = &s->slots[16];
        CHECK(ActorNo2Scene(s, a) && a->state.primitive_count == (variant ? 32 : 8));
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
        CHECK(p && p->state.offset_y[0] == (variant ? -64 : 88) &&
              p->state.offset_y[3] == (variant ? 44 : 196) && p->state.priority == 16);
        Close(f, s);
    }
    CHECK(Open(f, &s, 0x1a, 0));
    Actor *a = &s->slots[16];
    CHECK(ActorNo2Scene(s, a) && a->state.palette == 0x8040);
    CHECK(ActorNo2Scene(s, a) && a->state.palette == 0x8041);
    s->camera_y = 0x304;
    CHECK(ActorNo2Scene(s, a) && a->state.step == 2);
    for (Uint16 i = 0; i < 15; ++i)
        CHECK(ActorNo2Scene(s, a) && !s->palette_red_shift.count);
    CHECK(ActorNo2Scene(s, a) && s->palette_red_shift.count == 7 &&
          s->palette_red_shift.palettes[0] == 65 && s->palette_red_shift.palettes[6] == 71);
    SotnPaletteRedShift shift = {0};
    CHECK(SotnActors_GetPaletteRedShift(s, &shift) && shift.count == 7);
    CHECK(!SotnActors_GetPaletteRedShift(NULL, &shift) && !SotnActors_GetPaletteRedShift(s, NULL));
    CHECK(SotnActors_Enter(s, f->layout, s->map, 0, 0));
    CHECK(SotnActors_GetPaletteRedShift(s, &shift) && !shift.count);
    Close(f, s);
    CHECK(Open(f, &s, 0x1d, 0));
    a = &s->slots[16];
    s->slots[0].state = (SotnActorState){.generation = 2, .id = 1};
    CHECK(ActorNo2Scene(s, a) && a->state.frame == 8);
    s->slots[0] = (Actor){0};
    CHECK(ActorNo2Scene(s, a) && a->state.frame == 10);
    for (Uint16 i = 160; i < 165; ++i)
        CHECK(s->slots[i].state.id == 6 && s->slots[i].state.parameters == 16);
    CHECK(ActorNo2Scene(s, a) && !s->slots[165].state.generation);
    Close(f, s);
    CHECK(Open(f, &s, 0x1c, 0));
    CHECK(ActorNo2Scene(s, &s->slots[16]) && s->backdrop_color == (4 | (3 << 5) | (5 << 10)));
    Close(f, s);
    CHECK(Open(f, &s, 0x1e, 0));
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 42;
    CHECK(ActorNo2Scene(s, &s->slots[16]) && s->slots[16].remove);
    CHECK(!ActorNo2Scene(NULL, &s->slots[16]) && !ActorNo2Scene(s, NULL));
    Close(f, s);
    SDL_free(f);
    return true;
}
int main(void)
{
    if (!Houses() || !Scenery())
        return 1;
    SDL_Log("NO2_COURTYARD_SCENE: PASS");
    return 0;
}
