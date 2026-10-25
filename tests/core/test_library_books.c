#include "actors_internal.h"
#include "boss_projection.h"
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
    Uint8 dra[0x1000], stage[0x4000], exe[0x23220];
} Fixture;
static void Put16(Uint8 *p, Sint32 value)
{
    const Uint16 v = (Uint16)value;
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (8U * i));
}
static bool Open(Fixture *f, SotnActors *s, Uint16 id)
{
    SDL_zero(*f);
    SDL_zero(*s);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 40);
    Put16(f->dra + 0x206, 12);
    Put16(f->dra + 0x20c, 3);
    f->dra[0x222] = 8;
    f->dra[0x223] = 12;
    for (Uint32 i = 0; i < 4096; ++i)
    {
        const double angle = i * SDL_PI_D / 2048.0;
        Put16(f->exe + 0x1ea10 + i * 4, (Sint32)SDL_round(SDL_sin(angle) * 4096));
        Put16(f->exe + 0x1ea12 + i * 4, (Sint32)SDL_round(SDL_cos(angle) * 4096));
    }
    for (Uint32 i = 0; i <= 1024; ++i)
    {
        Put16(f->exe + 0x1dbcc + i * 2, (Sint32)SDL_round(SDL_sin(i * SDL_PI_D / 2048.0) * 4096));
        Put16(f->exe + 0x22a10 + i * 2, (Sint32)SDL_round(SDL_atan(i / 1024.0) * 2048 / SDL_PI_D));
    }
    for (Uint32 i = 0; i < 4; ++i)
    {
        Put16(f->stage + 0x2f44 + i * 8, i & 1U ? 8 : -8);
        Put16(f->stage + 0x2f46 + i * 8, i & 2U ? 8 : -8);
    }
    Put16(f->stage + 0x2fc6, 8);
    for (Uint32 face = 0; face < 12; ++face)
    {
        for (Uint32 i = 0; i < 4; ++i)
            Put32(f->stage + 0x2fcc + face * 16 + i * 4, 0x80182f44 + i * 8);
        Put32(f->stage + 0x30dc + face * 4, 0x80183400);
        Put32(f->stage + 0x310c + face * 4, 0x80183400);
    }
    f->stage[0x3400] = 4;
    f->stage[0x3401] = 8;
    f->stage[0x3402] = 20;
    f->stage[0x3403] = 24;
    for (Uint32 i = 0; i < 3; ++i)
    {
        Put32(f->stage + 0x316c + i * 16, 0x80180920);
        Put16(f->stage + 0x3174 + i * 16, 16);
        Put16(f->stage + 0x3176 + i * 16, 8);
        f->stage[0x3178 + i * 16] = 21;
    }
    s->stage_id = 2;
    s->nz0 = (SotnDataView){f->stage, sizeof(f->stage)};
    s->dra = (SotnDataView){f->dra, sizeof(f->dra)};
    s->executable = (SotnDataView){f->exe, sizeof(f->exe)};
    s->player = (SotnPlayerState){.x = 128 * 65536, .y = 128 * 65536, .hp = 70};
    s->slots[64] = (Actor){
        .slot = 64,
        .state = {.id = id, .generation = 1, .x = 128 * 65536 + 123, .y = 128 * 65536 + 456}};
    return SotnRoomActors_Create(&s->spawns);
}
static bool Projection(Fixture *f, SotnActors *s)
{
    CHECK(Open(f, s, 0x30));
    SotnBookTransform t = {.anchor_x = 128, .anchor_y = 100};
    SotnBookVertex v[4] = {{-8, -8, 0}, {8, -8, 0}, {-8, 8, 0}, {8, 8, 0}};
    SotnBookProjection p = {0};
    CHECK(SotnBook_Project(s->executable, t, v, 4, &p) && p.point[0].x == 120 &&
          p.point[0].y == 92 && p.point[3].x == 136 && p.point[3].y == 108 && p.priority == 144 &&
          p.winding == 256 && p.depth[0] == 512);
    t.z = 1024;
    CHECK(SotnBook_Project(s->executable, t, v, 4, &p) && p.point[0].x == 136 &&
          p.point[0].y == 92 && p.winding == 256);
    t.x = 1024;
    t.z = 0;
    CHECK(SotnBook_Project(s->executable, t, v, 4, &p) && p.depth[0] == 504 && p.depth[3] == 520 &&
          p.priority == 144);
    const SotnBookVertex center = {0, 0, 16};
    t.x = 0;
    CHECK(SotnBook_Project(s->executable, t, &center, 1, &p) && p.priority == 140 &&
          p.depth[0] == 528);
    const SotnBookProjection saved = p;
    t.depth = -1;
    CHECK(!SotnBook_Project(s->executable, t, v, 4, &p) && !SDL_memcmp(&p, &saved, sizeof(p)));
    t.depth = 769;
    CHECK(!SotnBook_Project(s->executable, t, v, 4, &p));
    t.depth = 0;
    v[0].x = 33;
    CHECK(!SotnBook_Project(s->executable, t, v, 4, &p));
    v[0].x = -8;
    CHECK(!SotnBook_Project(s->executable, t, v, 2, &p));
    CHECK(!SotnBook_Project((SotnDataView){0}, t, v, 4, &p));
    CHECK(!SotnBook_Project(s->executable, t, NULL, 4, &p));
    CHECK(!SotnBook_Project(s->executable, t, v, 4, NULL));
    t.anchor_x = 16385;
    CHECK(!SotnBook_Project(s->executable, t, v, 4, &p));
    SotnRoomActors_Destroy(s->spawns);
    return true;
}
static bool Cycle(Fixture *f, SotnActors *s, Uint16 id)
{
    CHECK(Open(f, s, id));
    Actor *a = &s->slots[64], *beam = &s->slots[65];
    CHECK(ActorLibrary(s, a) && a->state.step == 2 && a->state.primitive_count == 12 &&
          a->book.depth == 768 && !a->state.hitbox_state);
    CHECK(beam->state.id == 0x3e && beam->state.parameters == (id == 0x30 ? 0 : 2) &&
          beam->state.x == 128 * 65536 && beam->state.y == 128 * 65536 && !beam->state.facing_left);
    CHECK(ActorLibrary(s, beam) && beam->state.step == 1);
    CHECK(ActorLibrary(s, a) && a->state.step == 3);
    for (Uint32 i = 0; i < 48; ++i)
        CHECK(ActorLibrary(s, a) && a->state.step == 3);
    CHECK(a->book.depth == 0 && !a->state.hitbox_state);
    CHECK(ActorLibrary(s, a) && a->state.step == 4 && a->state.hitbox_state == 3);
    a->hit_flags = 1;
    CHECK(ActorLibrary(s, a));
    if (id == 0x30)
        CHECK(a->book.magnitude == -49152);
    else
        CHECK(a->vx == 0 && a->vy == -131072 && a->substep == 2);
    a->hit_flags = 0;
    // Keep the flight within this synthetic room while preserving its original timer.
    a->book.wait = 1;
    a->vx = a->vy = 0;
    CHECK(ActorLibrary(s, a) && a->state.step == 5);
    bool active = false, zero = false, commanded = false;
    for (Uint32 i = 0; i < 400 && a->state.step == 5; ++i)
    {
        s->combat.sound_count = 0;
        CHECK(ActorLibrary(s, a) && ActorLibrary(s, beam));
        active |= beam->state.hitbox_state == 1;
        zero |= beam->state.scale_set && beam->state.frame == 21 && !beam->state.scale_x;
        commanded |= beam->book.command;
    }
    CHECK(a->state.step == 4 && a->book.opening == 0 && active && zero && commanded &&
          !beam->book.command && beam->state.frame == 0 && beam->state.scale_set);
    const SotnActorPrimitive *p = &ActorPrimitiveGet(s, a, 0)->state;
    CHECK(p->quad && p->textured && p->screen_space && p->priority_set && p->page == 0x15 &&
          p->u[0] == 4 && p->u[1] == 20);
    CHECK(!ActorPrimitiveGet(s, a, 5)->state.active && !ActorPrimitiveGet(s, a, 11)->state.active);
    // A stale owner generation must retire the beam rather than attach to a new book.
    ++a->state.generation;
    CHECK(ActorLibrary(s, beam) && beam->remove);
    SotnRoomActors_Destroy(s->spawns);
    return true;
}
static bool WakeAndTurn(Fixture *f, SotnActors *s)
{
    for (Uint16 kind = 0; kind < 2; ++kind)
    {
        CHECK(Open(f, s, kind ? 0x32 : 0x30));
        Actor *a = &s->slots[64];
        CHECK(ActorLibrary(s, a));
        const Sint32 limit = kind ? 104 : 64;
        s->player.x = (128 + limit) * 65536;
        CHECK(ActorLibrary(s, a) && a->state.step == 2);
        s->player.x -= 65536;
        s->player.y = (128 + (kind ? 96 : 64)) * 65536;
        CHECK(ActorLibrary(s, a) && a->state.step == 2);
        s->player.y -= 65536;
        CHECK(ActorLibrary(s, a) && a->state.step == 3);
        SetStep(a, 5);
        a->book.angle[0] = SDL_MIN_SINT16;
        CHECK(ActorLibrary(s, a) && a->book.delta[0] == -108 && a->book.angle[0] == 32660);
        Actor *beam = &s->slots[65];
        CHECK(ActorLibrary(s, beam));
        beam->book.command = true;
        CHECK(ActorLibrary(s, beam) && beam->state.step == 2);
        CHECK(ActorLibrary(s, beam) && beam->state.scale_x == 8);
        a->state.flags |= 0x100U;
        CHECK(ActorLibrary(s, beam) && beam->substep == 2 && !beam->state.hitbox_state);
        CHECK(ActorLibrary(s, beam) && beam->state.frame == 0 && !beam->book.command);
        CHECK(ActorLibrary(s, beam) && beam->remove);
        SotnRoomActors_Destroy(s->spawns);
    }
    return true;
}
static bool Death(Fixture *f, SotnActors *s)
{
    CHECK(Open(f, s, 0x30));
    Actor *a = &s->slots[64];
    CHECK(ActorLibrary(s, a));
    CHECK(ActorLibrary(s, &s->slots[65]));
    a->state.flags |= 0x100U;
    for (Uint32 i = 160; i < 192; ++i)
        s->slots[i].state.generation = 99;
    for (Uint32 i = 0; i < 128; ++i)
    {
        s->combat.sound_count = 0;
        s->game_frame = (Uint8)i;
        CHECK(ActorLibrary(s, a) && !a->state.hitbox_state);
    }
    CHECK(a->state.step == 8 && a->substep == 2 && (a->state.flags & 0x80000000U) &&
          a->book.opening == 512);
    CHECK(ActorLibrary(s, a) && a->vy == 4096 && a->book.opening == 520);
    CHECK(ActorLibrary(s, &s->slots[65]) && s->slots[65].remove);
    Actor page = {.state = {.id = 0x31, .generation = 2, .rotation = 1024}};
    CHECK(ActorLibrary(s, &page) && page.state.frame >= 1 && page.state.frame <= 16 &&
          !page.state.rotated);
    const Sint32 x = page.state.x, y = page.state.y, vx = page.vx, vy = page.vy;
    CHECK(ActorLibrary(s, &page) && page.state.x == x + vx && page.state.y == y + vy &&
          page.vy == vy + 8192);
    page.state.x = SDL_MAX_SINT32;
    page.vx = 1;
    CHECK(!ActorLibraryBookMove(&page) && page.state.x == SDL_MAX_SINT32);
    SotnRoomActors_Destroy(s->spawns);
    return true;
}
static bool Failures(Fixture *f, SotnActors *s)
{
    CHECK(Open(f, s, 0x32));
    Actor *a = &s->slots[64];
    Put32(f->stage + 0x310c, 0x80184000);
    CHECK(!ActorLibrary(s, a));
    SotnRoomActors_Destroy(s->spawns);
    CHECK(Open(f, s, 0x30));
    a = &s->slots[64];
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 900;
    CHECK(ActorLibrary(s, a) && a->remove && !a->state.primitive_count);
    SotnRoomActors_Destroy(s->spawns);
    CHECK(Open(f, s, 0x30));
    a = &s->slots[64];
    CHECK(ActorLibrary(s, a));
    Put32(f->stage + 0x2fcc, 0x80182f45);
    CHECK(!ActorLibrary(s, a));
    Actor *beam = &s->slots[65];
    beam->state.parameters = 3;
    CHECK(!ActorLibrary(s, beam));
    beam->state.parameters = 0;
    Put16(f->stage + 0x3174, -1);
    CHECK(!ActorLibrary(s, beam));
    CHECK(!ActorLibraryBook(NULL, a) && !ActorLibraryBook(s, NULL));
    a->state.id = 0x40;
    CHECK(!ActorLibraryBook(s, a));
    SotnRoomActors_Destroy(s->spawns);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    SotnActors *s = SDL_calloc(1, sizeof(*s));
    const bool ok = f && s && Projection(f, s) && Cycle(f, s, 0x30) && Cycle(f, s, 0x32) &&
                    WakeAndTurn(f, s) && Death(f, s) && Failures(f, s);
    SDL_free(s);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_LIBRARY_BOOK_TEST: PASS");
    return 0;
}
