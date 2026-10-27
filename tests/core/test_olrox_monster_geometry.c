#include "olrox_monster_internal.h"
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
    for (Uint16 i = 0; i <= 1024; ++i)
    {
        Put16(f->exe + 0x1dbcc + i * 2U, i * 4);
        Put16(f->exe + 0x22a10 + i * 2U, i / 2);
    }
    for (Uint16 i = 0; i < 192; ++i)
        Put16(f->exe + 0x1e3e0 + i * 2U, 4096);
    Put32(f->stage + 20, 0x80183000);
    Put32(f->stage + 0x3000 + 7U * 4U, 0x80183100);
    Put32(f->stage + 0x3104, 0x80183120);
    Put16(f->stage + 0x3120, 1);
    Put16(f->stage + 0x3122, 12);
    Put16(f->stage + 0x3124, -3);
    Put16(f->stage + 0x3126, -5);
    Put16(f->stage + 0x3128, 10);
    Put16(f->stage + 0x312a, 18);
    Put16(f->stage + 0x312c, 12);
    Put16(f->stage + 0x312e, 1);
    Put16(f->stage + 0x3130, 7);
    Put16(f->stage + 0x3132, 13);
    Put16(f->stage + 0x3134, 17);
    Put16(f->stage + 0x3136, 31);
    Put16(f->stage + 0x1194, 5);
    Put16(f->stage + 0x1196, 20);
    Put16(f->stage + 0x1198, 30);
    Put16(f->stage + 0x119a, 8);
    Put16(f->stage + 0x119c, 0);
    Put16(f->stage + 0x119e, 12);
    Put16(f->stage + 0x11a0, 1024);
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
            .id = 0x33, .generation = 3, .x = 128 * 65536, .y = 128 * 65536, .priority = 136}};
    (*out)->player = (SotnPlayerState){.x = 160 * 65536, .y = 128 * 65536};
    (*out)->olrox_monster.priority = 160;
    Actor *body = &(*out)->slots[32];
    CHECK(ActorPrimitives(*out, body, 17));
    Actor *third = NULL, *seventh = NULL;
    CHECK(ActorSpawnAt(*out, body, 0x35, 0, 35, &third));
    CHECK(ActorSpawnAt(*out, body, 0x37, 0, 39, &seventh));
    body->olrox.children[3] = third->state.generation;
    body->olrox.children[7] = seventh->state.generation;
    third->state.x = 123;
    third->state.y = 456;
    seventh->state.priority = 200;
    return true;
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    f->layout = NULL;
}
static bool Geometry(Fixture *f)
{
    for (Uint16 facing = 0; facing < 2; ++facing)
    {
        SotnActors *s = NULL;
        CHECK(Open(f, &s));
        Actor *body = &s->slots[32];
        body->state.facing_left = facing != 0;
        CHECK(OlroxMonster_BodyGeometry(s, body, 10));
        const SotnActorPrimitive p = ActorPrimitiveGet(s, body, 10)->state;
        CHECK(p.active && p.page == 0x14 && p.palette == 0x20b && p.priority == 159 &&
              p.offset_x[0] == (facing ? 115 : 141) && p.offset_x[1] == (facing ? 155 : 101) &&
              p.offset_y[0] == 80 && p.offset_y[2] == 112 && p.u[0] == 60 && p.u[1] == 20);
        CHECK(s->slots[35].state.x == (facing ? 147 : 109) * 65536 + 123 &&
              s->slots[35].state.y == 80 * 65536 + 456 && s->slots[35].state.priority == 158);
        CHECK(s->slots[39].state.x == (facing ? 155 : 101) * 65536 &&
              s->slots[39].state.y == 68 * 65536 && s->slots[39].state.priority == 200);
        s->olrox_monster.legs[0] = (OlroxLeg){.upper = -512,
                                              .lower = 768,
                                              .knee_x = 124 * 65536,
                                              .knee_y = 170 * 65536,
                                              .foot_x = 120 * 65536,
                                              .foot_y = 200 * 65536};
        CHECK(OlroxMonster_LegGeometry(s, body, 0, 2, 0));
        CHECK(ActorPrimitiveGet(s, body, 2)->state.active &&
              ActorPrimitiveGet(s, body, 2)->state.palette == 0x218 &&
              ActorPrimitiveGet(s, body, 2)->state.priority == 160 &&
              ActorPrimitiveGet(s, body, 5)->state.priority == 161);
        s->olrox_monster.legs[0].foot_y = 460 * 65536;
        CHECK(OlroxMonster_LegGeometry(s, body, 0, 6, 1) &&
              ActorPrimitiveGet(s, body, 6)->state.palette == 0x20b &&
              ActorPrimitiveGet(s, body, 9)->state.priority == 163);
        s->olrox_morph.transforming = true;
        CHECK(OlroxMonster_LegGeometry(s, body, 0, 2, 0));
        CHECK(!OlroxMonster_LegGeometry(s, body, 2, 2, 0));
        CHECK(!OlroxMonster_LegGeometry(s, body, 0, 16, 0));
        body->olrox.body_style = 4;
        CHECK(!OlroxMonster_BodyGeometry(s, body, 10));
        body->olrox.body_style = 0;
        ++s->slots[35].state.generation;
        CHECK(!OlroxMonster_BodyGeometry(s, body, 10));
        Close(f, s);
    }
    return true;
}
static bool Sprite(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *body = &s->slots[32];
    body->state.x = 100 * 65536;
    body->state.y = 150 * 65536;
    body->state.frame = 1;
    body->state.texture_offset = 3;
    body->state.palette = 777;
    SotnActorPrimitive *p = &ActorPrimitiveGet(s, body, 16)->state;
    *p = (SotnActorPrimitive){
        .active = true, .page = 0x15, .priority_set = true, .priority = 77, .blend = true};
    CHECK(OlroxMonster_SpriteGeometry(s, body, body, 16) && p->active && p->blend &&
          p->page == 0x15 && p->priority == 77 && p->palette == 12 && p->offset_x[0] == 97 &&
          p->offset_x[1] == 107 && p->offset_y[0] == 145 && p->offset_y[2] == 163 && p->u[0] == 5 &&
          p->u[1] == 15 && p->v[0] == 140 && p->v[2] == 158);
    body->state.facing_left = true;
    CHECK(OlroxMonster_SpriteGeometry(s, body, body, 16) && p->offset_x[0] == 94 &&
          p->offset_x[1] == 104 && p->u[0] == 15 && p->u[1] == 5);
    Actor *child = &s->slots[35];
    child->state.frame = 1;
    CHECK(OlroxMonster_SpriteGeometry(s, child, body, 16));
    ++child->state.generation;
    CHECK(!OlroxMonster_SpriteGeometry(s, child, body, 16));
    body->state.frame = 2;
    CHECK(!OlroxMonster_SpriteGeometry(s, body, body, 16));
    s->executable = (SotnDataView){0};
    CHECK(!OlroxMonster_LegGeometry(s, body, 0, 2, 0));
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool okay = Geometry(f) && Sprite(f);
    SDL_free(f);
    return okay ? 0 : 1;
}
