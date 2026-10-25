#include "actors_internal.h"
#include "boss_projection.h"
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
    Uint8 dra[0x400], lib[0x1000], exe[0x23220];
    SotnWorldState world;
} Fixture;
static void Put16(Uint8 *p, Sint32 v)
{
    p[0] = (Uint8)(Uint16)v;
    p[1] = (Uint8)((Uint16)v >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (8U * i));
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    // Synthetic cube and lighting: no original model or lookup table fixture.
    const Uint16 faces[16] = {0, 1, 2, 3, 5, 4, 7, 6, 1, 5, 3, 7, 4, 0, 6, 2};
    for (Uint16 i = 0; i < 8; ++i)
    {
        const Uint32 vertex = 0xc48U + i * 8U, normal = 0xca8U + i * 16U;
        Put32(f->lib + 0xc88 + i * 4U, 0x80180000U + vertex);
        Put32(f->lib + 0xd28 + i * 4U, 0x80180000U + normal);
        Put16(f->lib + vertex, i & 1U ? 7 : -7);
        Put16(f->lib + vertex + 2, i & 2U ? 0 : -64);
        Put16(f->lib + vertex + 4, i & 4U ? 19 : -19);
        Put32(f->lib + normal, (Uint32)(i & 1U ? 4096 : -4096));
        Put32(f->lib + normal + 8, (Uint32)(i & 4U ? 4096 : -4096));
    }
    for (Uint16 i = 0; i < 16; ++i)
        Put32(f->lib + 0xd48 + i * 4U, faces[i]);
    for (Uint16 i = 0; i < 3; ++i)
        Put16(f->lib + 0xd88 + i * 8U, 2048);
    Put16(f->lib + 0xdaa, 768);
    Put16(f->lib + 0xdb2, 48);
    Put16(f->exe + 0x1e5f4, 2800);
    for (Uint32 i = 0; i < 4096; ++i)
    {
        const double angle = i * SDL_PI_D / 2048.0;
        Put16(f->exe + 0x1ea10 + i * 4U, (Sint32)SDL_round(SDL_sin(angle) * 4096));
        Put16(f->exe + 0x1ea12 + i * 4U, (Sint32)SDL_round(SDL_cos(angle) * 4096));
    }
    for (Uint32 i = 0; i <= 1024; ++i)
        Put16(f->exe + 0x1dbcc + i * 2U, (Sint32)SDL_round(SDL_sin(i * SDL_PI_D / 2048.0) * 4096));
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->lib, sizeof(f->lib)}, 2, out));
    CHECK(SotnActors_SetServices(*out, (SotnDataView){f->exe, sizeof(f->exe)}, 0, false));
    CHECK(SotnActors_SetWorld(*out, &f->world));
    return true;
}
static bool Geometry(Fixture *f, SotnActors *s)
{
    SotnShelfProjection p = {0};
    CHECK(SotnLibrary_ProjectShelf(s->executable, s->nz0, 0, 0, 128, 160, &p));
    CHECK(p.point[0].x == 121 && p.point[0].y == 98 && p.point[0].x < p.point[1].x);
    const SotnShelfProjection before = p;
    CHECK(!SotnLibrary_ProjectShelf(s->executable, s->nz0, 0, -1, 0, 0, &p));
    CHECK(!SotnLibrary_ProjectShelf(s->executable, s->nz0, 0, 129, 0, 0, &p));
    CHECK(!SotnLibrary_ProjectShelf(s->executable, s->nz0, 0, 0, 16385, 0, &p));
    CHECK(!SotnLibrary_ProjectShelf(s->executable, s->nz0, 0, 0, 0, 0, NULL));
    CHECK(!SotnLibrary_ProjectShelf((SotnDataView){0}, s->nz0, 0, 0, 0, 0, &p));
    Put32(f->lib + 0xc88, 0x80180c49);
    CHECK(!SotnLibrary_ProjectShelf(s->executable, s->nz0, 0, 0, 0, 0, &p));
    Put32(f->lib + 0xc88, 0x80180c48);
    Put32(f->lib + 0xcac, 1);
    CHECK(!SotnLibrary_ProjectShelf(s->executable, s->nz0, 0, 0, 0, 0, &p));
    Put32(f->lib + 0xcac, 0);
    Put16(f->exe + 0x1e5f4, 0);
    CHECK(!SotnLibrary_ProjectShelf(s->executable, s->nz0, 0, 0, 0, 0, &p));
    Put16(f->exe + 0x1e5f4, 2800);
    CHECK(!SDL_memcmp(&p, &before, sizeof(p)));
    return true;
}
static bool Push(Fixture *f, SotnActors *s)
{
    Actor a = {.slot = 64,
               .state = {.id = 0x1a, .generation = 1, .x = 128 * 65536, .y = 160 * 65536}};
    CHECK(ActorLibrary(s, &a) && a.state.step == 1 && a.state.primitive_count == 37 &&
          a.shelf.angle == -192 && a.shelf.delay == 96 && a.shelf.damping == 64);
    ActorPrimitive *face = ActorPrimitiveGet(s, &a, 0), *shadow = ActorPrimitiveGet(s, &a, 4);
    CHECK(face && shadow && !face->state.active && shadow->state.active && shadow->state.blend);
    Uint16 visible = 0;
    for (Uint16 i = 5; i < 37; ++i)
    {
        const ActorPrimitive *p = ActorPrimitiveGet(s, &a, i);
        CHECK(p);
        visible += p->state.active ? 1U : 0U;
        if (p->state.active)
            CHECK(p->state.modulate && p->state.gouraud && p->state.priority == 0x70);
    }
    CHECK(visible == 32);
    for (Uint16 i = 0; i < 96; ++i)
    {
        const Sint32 edge = face->state.offset_x[0];
        s->player.x = (edge - 8) * 65536;
        s->player.velocity_x = 2 * 65536;
        s->motion = (SotnActorMotion){0};
        CHECK(ActorLibrary(s, &a) && s->motion.x == -2 * 65536 && s->motion.contacts == 0x48);
        if (i < 95)
            CHECK(!f->world.flags[SOTN_LIBRARY_SHELF] && !a.shelf.velocity && !a.shelf.force);
    }
    CHECK(f->world.flags[SOTN_LIBRARY_SHELF] == 1 && a.shelf.velocity == -1153 &&
          a.shelf.force == -1536 && !a.shelf.delay);
    for (Uint16 i = 0; i < 20; ++i)
    {
        s->player.x = (face->state.offset_x[0] - 8) * 65536;
        s->motion = (SotnActorMotion){0};
        CHECK(ActorLibrary(s, &a));
    }
    CHECK(a.shelf.angle < -192 && a.shelf.damping == 2);
    // Side-on contact is absent. No new force, but momentum continues.
    a.shelf.angle = 1024;
    const Sint32 velocity = a.shelf.velocity;
    CHECK(ActorLibrary(s, &a) && !a.shelf.force && a.shelf.velocity > velocity);
    a.shelf.angle = 2048;
    a.shelf.velocity = 0;
    ActorPrimitive *back = ActorPrimitiveGet(s, &a, 1);
    CHECK(back);
    back->state.offset_x[0] = 120;
    back->state.offset_x[1] = 130;
    s->player.x = 138 * 65536;
    s->motion = (SotnActorMotion){0};
    CHECK(ActorLibrary(s, &a) && a.shelf.force == 1536 && s->motion.x == 2 * 65536 &&
          s->motion.contacts == 0x44);
    ActorPrimitivesClear(s, &a);
    // A fresh actor retains discovery but starts at the original angle, not a saved pose.
    a = (Actor){.slot = 64,
                .state = {.id = 0x1a, .generation = 2, .x = 128 * 65536, .y = 160 * 65536}};
    CHECK(ActorLibrary(s, &a) && a.shelf.angle == -192 && a.shelf.delay == 96);
    face = ActorPrimitiveGet(s, &a, 0);
    CHECK(face);
    s->player.x = (face->state.offset_x[0] - 8) * 65536;
    s->motion = (SotnActorMotion){0};
    CHECK(ActorLibrary(s, &a) && a.shelf.force == -1536 && a.shelf.delay == 96);
    ActorPrimitivesClear(s, &a);
    a = (Actor){.slot = 64, .state = {.id = 0x1a, .generation = 3, .parameters = 1}};
    CHECK(ActorLibrary(s, &a) && a.state.step == 4 && a.shelf.angle == 768 && a.shelf.depth == 48);
    shadow = ActorPrimitiveGet(s, &a, 4);
    CHECK(shadow && !shadow->state.active);
    const Sint32 old_x = s->player.x;
    CHECK(ActorLibrary(s, &a) && s->player.x == old_x && !a.shelf.force);
    ActorPrimitivesClear(s, &a);
    a = (Actor){.slot = 64, .state = {.id = 0x1a, .generation = 4, .parameters = 4}};
    CHECK(!ActorLibrary(s, &a));
    f->world.flags[SOTN_LIBRARY_SHELF] = 2;
    CHECK(!SotnWorld_Validate(&f->world));
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    SotnActors *s = NULL;
    const bool ok = f && Open(f, &s) && Geometry(f, s) && Push(f, s);
    SotnActors_Destroy(s);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_LIBRARY_SHELF_TEST: PASS");
    return 0;
}
