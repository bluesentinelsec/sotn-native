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
    Uint8 dra[0x1000], stage[0x2800], exe[0x23220], tiles[32 * 32 * 2], types[2];
    SotnActorLayout *layout;
} Fixture;
static void Put16(Uint8 *p, Sint32 v)
{
    p[0] = (Uint8)(Uint16)v;
    p[1] = (Uint8)((Uint16)v >> 8);
}
static void Put32(Uint8 *p, Sint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)((Uint32)v >> (i * 8U));
}
static Sint32 Sine(Uint32 phase)
{
    phase &= 4095U;
    return phase <= 1024   ? (Sint32)phase * 4
           : phase <= 3072 ? 8192 - (Sint32)phase * 4
                           : (Sint32)phase * 4 - 16384;
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
    for (Uint32 i = 0; i < 4096; ++i)
    {
        Put16(f->exe + 0x1ea10 + i * 4U, Sine(i));
        Put16(f->exe + 0x1ea12 + i * 4U, Sine(i + 1024U));
    }
    // Alternate subdivision traversal and deliberately different private models/poses/colors.
    const Uint16 indices[] = {3, 5, 6, 8, 0, 2, 3, 5};
    for (Uint16 i = 0; i < 8; ++i)
        Put16(f->stage + 0x2564 + i * 2U, indices[i]);
    for (Uint16 i = 0; i < 4; ++i)
    {
        Put16(f->stage + 0x2574 + i * 8U, i & 1U ? 160 : -160);
        Put16(f->stage + 0x2576 + i * 8U, i >= 2 ? 120 : -120);
        Put16(f->stage + 0x2594 + i * 8U, i & 1U ? 160 : -160);
        Put16(f->stage + 0x2596 + i * 8U, i >= 2 ? 72 : -72);
        Put16(f->stage + 0x26c4 + i * 8U, i & 1U ? 144 : -144);
        Put16(f->stage + 0x26c6 + i * 8U, i >= 2 ? 48 : -48);
    }
    for (Uint16 i = 0; i < 12; ++i)
    {
        Put32(f->stage + 0x25b4 + i * 16U, i & 1U ? -192 : 192);
        Put32(f->stage + 0x25b8 + i * 16U, 32);
        Put32(f->stage + 0x25bc + i * 16U, i < 2 ? 1024 : 2560);
        Put32(f->stage + 0x25c0 + i * 16U, i < 2 ? 2 : 0);
        Put16(f->stage + 0x2674 + i * 2U, i & 1U ? -1024 : 1024);
        f->stage[0x268c + i * 4U] = (Uint8)(100 + i);
        f->stage[0x268d + i * 4U] = (Uint8)(80 + i);
        f->stage[0x268e + i * 4U] = (Uint8)(60 + i);
        f->stage[0x268f + i * 4U] = 24;
    }
    for (Uint16 i = 0; i < 9; ++i)
    {
        Put32(f->stage + 0x26f0 + i * 16U, i ? -96 : 0);
        Put32(f->stage + 0x26f4 + i * 16U, i ? i * 256 : 2048);
    }
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 6, out));
    CHECK(SotnActors_SetServices(*out, (SotnDataView){f->exe, sizeof(f->exe)}, 1, false));
    CHECK(SotnActorLayout_OpenStage((SotnDataView){f->stage, sizeof(f->stage)}, 6, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *out, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 32}, 0,
        0));
    (*out)->slots[16] =
        (Actor){.slot = 16,
                .dynamic = true,
                .state = {.id = 0x17, .generation = 1, .x = 128 * 65536, .y = 160 * 65536}};
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
    const SotnGlassVertex model[4] = {{-32, -16, 0}, {32, -16, 0}, {-32, 16, 0}, {32, 16, 0}};
    SotnGlassTransform pose = {.depth = 1024};
    SotnProjectedPoint p[4] = {0}, saved[4] = {0};
    CHECK(SotnChapel_ProjectGlass(s->executable, pose, model, 4, p) && p[0].x == 96 &&
          p[0].y == 144 && p[3].x == 160 && p[3].y == 176);
    pose.y_angle = 1024;
    pose.z_angle = -1024;
    CHECK(SotnChapel_ProjectGlass(s->executable, pose, model, 4, p) && p[0].y == 160 &&
          p[1].y == 160 && p[0].x < p[2].x && p[0].x > p[1].x);
    SDL_memcpy(saved, p, sizeof(saved));
    pose.depth = 1023;
    CHECK(!SotnChapel_ProjectGlass(s->executable, pose, model, 4, p));
    pose.depth = 1024;
    CHECK(!SotnChapel_ProjectGlass((SotnDataView){0}, pose, model, 4, p));
    CHECK(!SotnChapel_ProjectGlass(s->executable, pose, model, 2, p));
    CHECK(!SotnChapel_ProjectGlass(s->executable, pose, NULL, 4, p));
    CHECK(!SotnChapel_ProjectGlass(s->executable, pose, model, 4, NULL));
    const SotnGlassVertex invalid = {.z = 1};
    CHECK(!SotnChapel_ProjectGlass(s->executable, pose, &invalid, 1, p) &&
          !SDL_memcmp(p, saved, sizeof(p)));
    Close(f, s);
    return true;
}
static bool Panels(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[16];
    CHECK(ActorChapel(s, a) && a->state.step == 1 && a->state.primitive_count == 60 &&
          !ActorPrimitiveGet(s, a, 12)->state.active);
    CHECK(ActorChapel(s, a) && !ActorPrimitiveGet(s, a, 0)->state.active &&
          ActorPrimitiveGet(s, a, 2)->state.active);
    const SotnActorPrimitive first = ActorPrimitiveGet(s, a, 12)->state;
    CHECK(first.active && first.v[0] == 131 && first.v[3] == 174 && first.page == 0x2f &&
          first.palette == 14 && first.vertex_red[0] == 100 && first.vertex_green[0] == 80);
    bool fill = false, ray = false, glow = false;
    for (Uint16 i = 12; i < 60; ++i)
    {
        const SotnActorPrimitive p = ActorPrimitiveGet(s, a, i)->state;
        if (!p.active)
            continue;
        fill = fill || (p.palette == 0 && p.priority == 85 && !p.blend && p.vertex_red[0] == 24 &&
                        p.vertex_red[2] == 12);
        ray = ray || (p.palette == 0x204 && p.page == 0x6f && p.blend);
        glow = glow || (p.palette == 0 && p.priority == 87 && p.page == 0x2f && p.blend &&
                        p.vertex_blue[0] == 32 && p.vertex_red[3] == 0);
    }
    CHECK(fill && ray && glow);
    s->camera_x = 32;
    CHECK(ActorChapel(s, a) && ActorPrimitiveGet(s, a, 2)->state.offset_x[0] !=
                                   ActorPrimitiveGet(s, a, 3)->state.offset_x[0]);
    Put32(f->stage + 0x25c0, 4);
    CHECK(!ActorChapel(s, a));
    Put32(f->stage + 0x25c0, 2);
    Put16(f->stage + 0x2564, 9);
    CHECK(!ActorChapel(s, a));
    Put16(f->stage + 0x2564, 3);
    Put16(f->stage + 0x26bc, 1);
    CHECK(!ActorChapel(s, a));
    CHECK(!ActorChapelGlass(NULL, a) && !ActorChapelGlass(s, NULL));
    Put16(f->stage + 0x26bc, 0);
    for (Uint16 i = 0; i < 12; ++i)
    {
        Put32(f->stage + 0x25b4 + i * 16U, 0);
        Put32(f->stage + 0x25c0 + i * 16U, 3);
        Put16(f->stage + 0x2674 + i * 2U, 0);
    }
    CHECK(!ActorChapel(s, a) && SDL_strstr(SDL_GetError(), "capacity") != NULL);
    Close(f, s);
    return true;
}
static bool Background(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[16];
    a->state.id = 0x18;
    CHECK(ActorChapel(s, a) && a->state.primitive_count == 19 && s->palette_blend.active &&
          s->palette_blend.source == 15 && s->palette_blend.destination == 0x205 &&
          s->palette_blend.steps == 16 && s->backdrop_color == (3U | (3U << 5) | (3U << 10)));
    CHECK(ActorChapel(s, a));
    const SotnActorPrimitive p = ActorPrimitiveGet(s, a, 0)->state,
                             floor = ActorPrimitiveGet(s, a, 4)->state;
    CHECK(p.active && p.offset_x[0] == 83 && p.offset_x[1] == 128 && p.offset_y[0] == 98 &&
          p.palette == 5 && p.priority == 80 && floor.vertex_red[2] == 128 && !floor.textured);
    CHECK(ActorPrimitiveGet(s, a, 5)->state.palette == 0x207 &&
          ActorPrimitiveGet(s, a, 18)->state.palette == 0x213);
    s->camera_x = 256;
    CHECK(ActorChapel(s, a) && ActorPrimitiveGet(s, a, 5)->state.offset_y[1] == -1 &&
          ActorPrimitiveGet(s, a, 5)->state.offset_y[3] == -1);
    s->camera_x = 0;
    const SotnPlayerState player = {.x = 128 * 65536, .y = 160 * 65536, .hp = 70, .hp_max = 70};
    SotnPaletteBlend blend = {0};
    CHECK(SotnActors_Tick(s, player, 0, 0) && SotnActors_GetPaletteBlend(s, &blend) &&
          !blend.active);
    CHECK(!SotnActors_GetPaletteBlend(NULL, &blend) && !SotnActors_GetPaletteBlend(s, NULL));
    Put16(f->stage + 0x277c, 1);
    CHECK(!ActorChapel(s, a));
    Put16(f->stage + 0x277c, 0);
    Put32(f->stage + 0x26f4, -1);
    CHECK(!ActorChapel(s, a));
    CHECK(SotnActors_Enter(s, f->layout, s->map, 0, 0) && SotnActors_GetPaletteBlend(s, &blend) &&
          !blend.active);
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
    const bool ok = f && Projection(f) && Panels(f) && Background(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_CHAPEL_GLASS: PASS");
    return 0;
}
