#include "actors_internal.h"
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
    Uint8 dra[0x1000], stage[0x2000], types[2048];
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
    Put16(f->stage + 0xca6, -15);
    f->types[1] = 3;
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 6, out));
    (*out)->mutable_tiles = SDL_calloc(32 * 32, 2);
    CHECK((*out)->mutable_tiles);
    (*out)->map = (SotnCollisionMap){
        {(*out)->mutable_tiles, 32 * 32 * 2}, {f->types, sizeof(f->types)}, 32, 32};
    for (Uint32 i = 32 * 11; i < 32 * 32; ++i)
        (*out)->mutable_tiles[i * 2] = 1;
    (*out)->player =
        (SotnPlayerState){.x = 160 * 65536, .y = 135 * 65536, .solid_width = 6, .solid_height = 16};
    return true;
}
static Actor Block(Uint32 generation)
{
    return (Actor){
        .slot = 16,
        .state = {.id = 0x19, .generation = generation, .x = 63 * 65536, .y = 190 * 65536}};
}
static bool Shortcut(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = Block(1);
    CHECK(ActorChapel(s, &a) && a.state.step == 1 && a.state.frame == 1 &&
          !s->world->flags[SOTN_CHAPEL_SHORTCUT]);
    Actor *statue = &s->slots[17];
    CHECK(statue->state.id == 0x1a && statue->state.x == 79 * 65536 &&
          statue->state.y == 206 * 65536 && statue->enemy_state.owner_generation == 1);
    CHECK(ActorChapel(s, statue) && statue->state.hit_points == 4 &&
          statue->state.hitbox_state == 2 && statue->state.hitbox_x == -8 &&
          statue->state.hitbox_y == -32);
    statue->state.flags |= 0x100U;
    CHECK(ActorChapel(s, statue) && statue->remove && s->combat.sounds[0] == 0x655 &&
          s->slots[160].state.parameters == 1 && s->slots[160].state.x == 71 * 65536);
    // The entered simulation releases a removed child before the next parent tick.
    *statue = (Actor){0};
    CHECK(ActorChapel(s, &a) && a.state.step == 2 && s->world->flags[SOTN_CHAPEL_SHORTCUT] == 1);
    CHECK(ActorChapel(s, &a) && a.state.step == 3 && a.state.x == 63 * 65536 + 16384 &&
          s->combat.sounds[1] == 0x644);
    a = Block(2);
    CHECK(ActorChapel(s, &a) && a.state.step == 16 && a.state.x == 95 * 65536 &&
          a.state.y == 222 * 65536 && !statue->state.id);
    CHECK(SotnWorld_Validate(s->world));
    s->world->flags[SOTN_CHAPEL_SHORTCUT] = 2;
    CHECK(!SotnWorld_Validate(s->world));
    s->world->flags[SOTN_CHAPEL_SHORTCUT] = 0;
    a = (Actor){.state = {.id = 0x1a}};
    s->player.x = 64 * 65536;
    CHECK(ActorChapel(s, &a) && !a.state.hitbox_state);
    SotnActors_Destroy(s);
    return true;
}
static bool Ownership(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = Block(1);
    CHECK(ActorChapel(s, &a));
    const Uint32 previous = s->slots[17].state.generation;
    a = Block(2);
    CHECK(ActorChapel(s, &a) && s->slots[17].state.generation != previous &&
          s->slots[17].enemy_state.owner_generation == 2);
    s->slots[17].state.id = 0x35;
    CHECK(!ActorChapel(s, &a) && !s->world->flags[SOTN_CHAPEL_SHORTCUT]);
    a = Block(3);
    CHECK(!ActorChapel(s, &a) && s->slots[17].state.id == 0x35);
    a = Block(4);
    a.slot = 191;
    CHECK(!ActorChapel(s, &a));
    a = Block(0);
    a.slot = 18;
    CHECK(!ActorChapel(s, &a) && !s->slots[19].state.generation);
    CHECK(!ActorChapelMechanism(NULL, &a) && !ActorChapelMechanism(s, NULL));
    SotnActors_Destroy(s);
    return true;
}
static bool Gargoyles(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    for (Uint16 side = 0; side < 2; ++side)
    {
        Actor a = {.slot = (Uint16)(64 + side * 2U),
                   .state = {.id = 0x21,
                             .generation = (Uint32)(10 + side),
                             .parameters = side,
                             .x = (16 + side * 448) * 65536,
                             .y = 32 * 65536}};
        CHECK(ActorChapel(s, &a) && a.state.frame == 12 + side && a.state.priority == 92);
        Actor *child = &s->slots[a.slot + 1U];
        CHECK(child->state.id == 0x21 && child->state.parameters == 2 &&
              child->state.y == 160 * 65536 && child->state.x == (16 + side * 464) * 65536);
        CHECK(ActorChapel(s, child) && child->state.frame == 14);
        child->state.step = 3;
        for (Uint16 i = 0; i < 28; ++i)
            CHECK(ActorChapel(s, child));
        CHECK(child->state.step == 16 && child->state.y == 129 * 65536);
        a.state.step = 2;
        for (Uint16 i = 0; i < 34; ++i)
            CHECK(ActorChapel(s, &a));
        CHECK(a.state.step == 16 && a.state.y == 70 * 65536);
        for (Uint16 i = 0; i < 4; ++i)
            CHECK(Read16(s->mutable_tiles + ((side ? 222U : 193U) + i * 32U) * 2U) == 0x4b3);
    }
    Actor bad = {.state = {.id = 0x21, .parameters = 3}};
    CHECK(!ActorChapel(s, &bad));
    bad.state.parameters = 0;
    bad.state.step = 2;
    bad.state.y = 70 * 65536;
    s->map.width = 16;
    CHECK(!ActorChapel(s, &bad) && bad.state.y == 70 * 65536);
    SotnActors_Destroy(s);
    return true;
}
static bool Scenery(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    for (Uint16 kind = 0; kind < 4; ++kind)
    {
        Actor a = {.state = {.id = (Uint16)(kind < 2 ? 0x1c : 0x1d),
                             .parameters = (Uint16)(kind < 2 ? kind : (kind == 2 ? 3 : 0x103)),
                             .generation = (Uint32)(kind + 1),
                             .x = 128 * 65536,
                             .y = 64 * 65536}};
        CHECK(ActorChapel(s, &a) && a.state.primitive_count == (kind < 2 ? 2 : 9));
        ActorPrimitive *p = ActorPrimitiveGet(s, &a, 0);
        CHECK(p && p->state.page == 15 && p->state.priority == 160 && p->state.quad &&
              ((p->state.offset_x[1] < 0) == (kind == 0 || kind == 3)));
        CHECK(ActorChapel(s, &a));
        ActorPrimitivesClear(s, &a);
    }
    s->camera_y = 384;
    s->player.y = 512 * 65536;
    Actor a = {.state = {.id = 0x1e, .generation = 20}};
    CHECK(ActorChapel(s, &a) && a.state.primitive_count == 1 && s->backdrop_color == 0x1c22);
    ActorPrimitive *p = ActorPrimitiveGet(s, &a, 0);
    CHECK(p && p->state.screen_space && !p->state.textured && p->state.priority == 94 &&
          p->state.offset_y[1] == 112);
    s->player.y = 480 * 65536;
    CHECK(ActorChapel(s, &a) && p->state.offset_y[1] == 80);
    ActorPrimitivesClear(s, &a);
    a = (Actor){.state = {.id = 0x1d, .parameters = 0}};
    CHECK(!ActorChapel(s, &a));
    a.state.parameters = 0x203;
    CHECK(!ActorChapel(s, &a));
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 1;
    a = (Actor){.state = {.id = 0x1e, .generation = 21}};
    CHECK(ActorChapel(s, &a) && a.remove);
    SotnActors_Destroy(s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    const bool ok = f && Shortcut(f) && Ownership(f) && Gargoyles(f) && Scenery(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_CHAPEL_MECHANISMS: PASS");
    return 0;
}
