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
    Uint8 dra[0x1000], stage[0x4000], tiles[32 * 32 * 2], types[2];
    SotnActorLayout *layout;
} Fixture;
static void Put16(Uint8 *p, Uint16 n)
{
    p[0] = (Uint8)n;
    p[1] = (Uint8)(n >> 8);
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
    Put16(f->dra + 0x200 + 50 * 40 + 4, 400);
    Put16(f->dra + 0x200 + 50 * 40 + 6, 16);
    Put16(f->stage + 0x6cc, 0x8002);
    Put16(f->stage + 0x6ce, 7);
    Put16(f->stage + 0x6d0, 3);
    Put16(f->stage + 0x6d2, 2);
    Put16(f->stage + 0x6d4, 50);
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    // Authored timelines/decisions/positions and a flat floor; no original fixture arrays.
    const size_t timelines[] = {0xc00, 0xc0c, 0xc24, 0xc40, 0xc4c, 0xc58, 0xc6c, 0xc80, 0xcac,
                                0xcbc, 0xcc4, 0xcd0, 0xcdc, 0xcec, 0xd00, 0xd10, 0xd20, 0xd2c};
    for (Uint16 i = 0; i < SDL_arraysize(timelines); ++i)
    {
        f->stage[timelines[i]] = 1;
        f->stage[timelines[i] + 1] = (Uint8)(17 + i);
        f->stage[timelines[i] + 2] = 255;
    }
    for (Uint16 i = 0; i < 16; ++i)
        f->stage[0xbac + i] = i < 8 ? 7 : 9;
    Put16(f->stage + 0xbbe, 28);
    Put16(f->stage + 0xbce, 28);
    Put16(f->stage + 0xbd0, 8);
    for (Uint16 i = 0; i < 9; ++i)
    {
        Put16(f->stage + 0xbd4 + i * 4U, (Uint16)(10 + i * 20));
        Put16(f->stage + 0xbd6 + i * 4U, (Uint16)(80 + i * 10));
    }
    for (Uint32 i = 32 * 29; i < 32 * 32; ++i)
        f->tiles[i * 2] = 1;
    f->types[1] = 3;
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 30, out));
    CHECK(SotnActorLayout_OpenStage((SotnDataView){f->stage, sizeof(f->stage)}, 30, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *out, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 32}, 0,
        0));
    (*out)->slots[32] = (Actor){
        .slot = 32,
        .dynamic = true,
        .state = {.id = 0x2b, .generation = 9, .x = 128 * 65536 + 123, .y = 436 * 65536 + 456}};
    (*out)->player = (SotnPlayerState){.x = 300 * 65536, .y = 436 * 65536};
    return true;
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    f->layout = NULL;
}
static bool Introduction(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[32];
    CHECK(ActorOlroxFirst(s, a) && a->state.step == 1 && a->state.frame == 14 &&
          !a->state.hitbox_state && a->state.hit_points == 400 && a->state.hitbox_height == 28);
    s->player.x = 223 * 65536;
    s->player.contacts = 0x41;
    CHECK(ActorOlroxFirst(s, a) && a->state.step == 1);
    s->player.contacts = 1;
    s->player.x = 224 * 65536;
    CHECK(ActorOlroxFirst(s, a) && a->state.step == 1);
    --s->player.x;
    CHECK(ActorOlroxFirst(s, a) && a->state.step == 2 && a->olrox.timer == 192 &&
          s->world->time_attack_visited == (1U << SOTN_TIME_OLROX) &&
          !s->world->time_attack[SOTN_TIME_OLROX]);
    for (Uint16 i = 0; i < 4; ++i)
        CHECK(ActorOlroxFirst(s, a));
    CHECK(a->state.step == 3 && a->state.hitbox_state == 3 && s->combat.music_change &&
          s->combat.music == 0x334 && !s->olrox_defeated);
    a->state.step = 1;
    s->slots[31].state.flags = 0x100;
    CHECK(ActorOlroxFirst(s, a) && a->state.step == 13 && !a->substep &&
          a->state.hitbox_state == 3);
    Close(f, s);
    return true;
}
static bool Decisions(Fixture *f)
{
    static const struct
    {
        Sint16 x, player;
        Uint16 expected, substep;
    } cases[] = {{50, 70, 13, 0},  {448, 432, 13, 0}, {100, 130, 5, 0}, {100, 180, 4, 0},
                 {400, 320, 4, 0}, {200, 240, 6, 1},  {200, 400, 6, 0}, {200, 100, 7, 0}};
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[32];
    CHECK(ActorOlroxFirst(s, a));
    for (Uint16 i = 0; i < SDL_arraysize(cases); ++i)
    {
        SetStep(a, 3);
        a->state.x = cases[i].x * 65536;
        s->player.x = cases[i].player * 65536;
        CHECK(ActorOlroxFirst(s, a) && a->state.step == cases[i].expected &&
              a->substep == cases[i].substep);
    }
    a->state.x = 200 * 65536;
    s->player.x = 300 * 65536;
    a->state.hit_points = 300;
    SetStep(a, 3);
    CHECK(ActorOlroxFirst(s, a) && a->state.step == 7);
    a->state.hit_points = 299;
    SetStep(a, 3);
    CHECK(ActorOlroxFirst(s, a) && a->state.step == 9);
    f->stage[0xbac + (a->olrox.decision | 8U)] = 2;
    SetStep(a, 3);
    CHECK(!ActorOlroxFirst(s, a));
    Close(f, s);
    return true;
}
static bool MovementAndImages(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[32];
    CHECK(ActorOlroxFirst(s, a));
    for (Uint16 side = 0; side < 2; ++side)
        for (Uint16 backward = 0; backward < 2; ++backward)
        {
            SetStep(a, 6);
            a->substep = backward;
            a->state.facing_left = side != 0;
            CHECK(ActorOlroxFirst(s, a) && a->vx == ((side != backward) ? -147456 : 147456) &&
                  a->state.frame == (backward ? 45 : 25) && a->olrox.timer == 8 && a->substep == 2);
        }
    a->state.x = 256 * 65536 + 123;
    a->state.facing_left = false;
    SetStep(a, 5);
    CHECK(ActorOlroxFirst(s, a) && ActorOlroxFirst(s, a) && a->substep == 1 && a->vx == -262144 &&
          a->vy == -196608);
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(ActorOlroxFirst(s, a));
    Actor *image = &s->slots[160];
    CHECK(image->state.id == 0x2c && image->state.parameters == 45 &&
          ((Uint32)image->state.x & 65535U) == 0 && ((Uint32)image->state.y & 65535U) == 0);
    CHECK(ActorOlroxFirst(s, image) && image->state.opacity == 126 && image->state.opacity_set &&
          image->state.palette == 0x217 && image->state.frame == 45 && !image->state.hitbox_state);
    for (Uint16 i = 0; i < 63; ++i)
        CHECK(ActorOlroxFirst(s, image));
    CHECK(image->remove && image->state.opacity == 0);
    for (Uint16 i = 0; i < 100 && a->state.step != 3; ++i)
        CHECK(ActorOlroxFirst(s, a));
    CHECK(a->state.step == 3 && a->vy == 0);
    // Original state twelve has the same leftward initial velocity for either facing.
    for (Uint16 side = 0; side < 2; ++side)
    {
        SetStep(a, 12);
        a->state.facing_left = side != 0;
        CHECK(ActorOlroxFirst(s, a) && a->vx == -32768);
    }
    Close(f, s);
    return true;
}
static bool FactoriesAndDefeat(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[32];
    CHECK(ActorOlroxFirst(s, a));
    SetStep(a, 8);
    a->substep = 1;
    s->camera_x = 32;
    s->camera_y = 256;
    CHECK(ActorOlroxFirst(s, a) && a->substep == 2);
    for (Uint16 i = 1; i <= 9; ++i)
    {
        const Actor *child = OlroxChild(s, a, i);
        CHECK(child && child->state.id == 0x2d && child->state.parameters == i - 1 &&
              Pixel(child->state.x) == 42 + (i - 1) * 20 &&
              Pixel(child->state.y) == 336 + (i - 1) * 10);
    }
    CHECK(ActorOlroxFirst(s, a) && a->substep == 2);
    // Explicit readiness fixture. Unsupported children must not synthesize readiness.
    for (Uint16 i = 1; i <= 9; ++i)
        OlroxChild(s, a, i)->state.hitbox_state = 1;
    CHECK(ActorOlroxFirst(s, a) && a->substep == 3);
    Actor *child = NULL;
    CHECK(OlroxReserved(s, a, 1, 0x32, 0, &child) && child->state.id == 0x32);
    ++child->enemy_state.owner_generation;
    CHECK(!OlroxReserved(s, a, 1, 0x2d, 0, &child));
    --s->slots[33].enemy_state.owner_generation;
    CHECK(!OlroxReserved(s, a, 14, 0x2d, 0, &child));
    SetStep(a, 14);
    a->substep = 1;
    s->slots[20] = (Actor){.state = {.generation = 17, .id = 3}};
    CHECK(!ActorOlroxFirst(s, a) && !s->slots[20].state.flags);
    s->slots[20].state.id = 0x2a;
    a->state.x = 200 * 65536;
    CHECK(ActorOlroxFirst(s, a) && a->substep == 4 && (s->slots[20].state.flags & 0x100));
    CHECK(ActorOlroxFirst(s, a) && ActorOlroxFirst(s, a) && a->substep == 5 &&
          (s->olrox_flags & 1));
    CHECK(ActorOlroxFirst(s, a) && a->substep == 6 && a->olrox.timer == 160);
    child = OlroxChild(s, a, 1);
    CHECK(child && child->state.id == 0x32 && Pixel(child->state.y) == 464);
    for (Uint16 i = 0; i < 160; ++i)
        CHECK(ActorOlroxFirst(s, a));
    CHECK(a->state.step == 64 && !a->state.frame && !s->olrox_defeated &&
          !s->world->time_attack[SOTN_TIME_OLROX]);
    Close(f, s);
    return true;
}
static bool PackedCursorEvents(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[32];
    CHECK(ActorOlroxFirst(s, a));
    for (Uint16 i = 0; i < 16; ++i)
    {
        f->stage[0xc80 + i * 2U] = 3;
        f->stage[0xc81 + i * 2U] = (Uint8)(i + 1);
    }
    f->stage[0xca0] = 255;
    SetStep(a, 11);
    a->substep = 1;
    a->pose = 16;
    a->remaining = 3;
    CHECK(ActorOlroxFirst(s, a) && a->remaining == 2 && !a->olrox.fire_skulls);
    CHECK(ActorOlroxFirst(s, a) && a->remaining == 1 && !a->olrox.fire_skulls);
    CHECK(ActorOlroxFirst(s, a) && !a->remaining && a->olrox.fire_skulls == 1);
    for (Uint16 i = 0; i < 4; ++i)
    {
        f->stage[0xc24 + i * 2U] = 3;
        f->stage[0xc25 + i * 2U] = (Uint8)(i + 1);
    }
    f->stage[0xc2c] = 255;
    SetStep(a, 10);
    a->substep = 1;
    a->pose = 4;
    a->remaining = 2;
    CHECK(ActorOlroxFirst(s, a) && !s->slots[96].state.generation);
    CHECK(ActorOlroxFirst(s, a) && s->slots[96].state.id == 0x30);
    CHECK(ActorOlroxFirst(s, a) && a->state.step == 3 && !s->slots[97].state.generation);
    Close(f, s);
    return true;
}
static bool Invalid(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[32];
    CHECK(!ActorOlroxFirst(NULL, a) && !ActorOlroxFirst(s, NULL));
    a->state.step = 32;
    CHECK(!ActorOlroxFirst(s, a));
    a->state.step = 13;
    a->substep = 4;
    CHECK(!ActorOlroxFirst(s, a));
    a->substep = 0;
    a->vx = SDL_MAX_SINT32;
    CHECK(!ActorOlroxFirst(s, a));
    a->vx = 0;
    a->state.id = 0x33;
    CHECK(!ActorOlroxFirst(s, a));
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Introduction(f) && Decisions(f) && MovementAndImages(f) &&
                    FactoriesAndDefeat(f) && PackedCursorEvents(f) && Invalid(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("OLROX_FIRST_FORM: PASS");
    return 0;
}
