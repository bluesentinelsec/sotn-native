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
    Uint8 dra[0x1000], stage[0x2000], exe[0x23220], tiles[16 * 16 * 2], types[2];
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
    f->dra[0x222] = 8;
    f->dra[0x223] = 8;
    for (Uint16 i = 0; i < 10; ++i)
    {
        Put32(f->stage + 0xbf0 + i * 4U, 0x80181e00U + i * 4U);
        f->stage[0xc18 + i] = (Uint8)(i + 1);
        f->stage[0xc24 + i] = (Uint8)(i % 3);
        Put16(f->stage + 0xc30 + i * 2U, 0x180 + i);
        Put16(f->stage + 0xc44 + i * 2U, 0x8005);
        Put16(f->stage + 0xc58 + i * 2U, i * 3);
        f->stage[0xc6c + i] = 0x30;
        Put16(f->stage + 0xc78 + i * 2U, -(Sint32)i);
        Put16(f->stage + 0xc90 + i * 2U, i % 4);
        f->stage[0x1e00 + i * 4U] = 1;
        f->stage[0x1e01 + i * 4U] = (Uint8)(i + 31);
    }
    for (Uint32 i = 0; i < 4096; ++i)
    {
        Put16(f->exe + 0x1ea10 + i * 4, (Sint32)SDL_round(SDL_sin(i * SDL_PI_D / 2048) * 4096));
        Put16(f->exe + 0x1ea12 + i * 4, (Sint32)SDL_round(SDL_cos(i * SDL_PI_D / 2048) * 4096));
    }
    for (Uint32 i = 16 * 14; i < 16 * 16; ++i)
        f->tiles[i * 2] = 1;
    f->types[1] = 3;
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 6, out));
    (*out)->executable = (SotnDataView){f->exe, sizeof(f->exe)};
    (*out)->map =
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 16, 16};
    (*out)->player.x = 192 * 65536;
    return true;
}
static bool Types(Fixture *f, Uint16 type, bool cube)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    s->player.x = 64 * 65536;
    if (!cube)
        s->world->relics[SOTN_RELIC_CUBE] = 0;
    Actor a = {.slot = 40,
               .state = {.id = 1,
                         .generation = 123,
                         .parameters = (Uint16)(type << 12),
                         .x = 128 * 65536 + 1,
                         .y = 160 * 65536 + 2}};
    CHECK(ActorChapel(s, &a) && a.state.frame == type + 31 && a.state.priority == 128 &&
          a.state.hitbox_height == type + 1 && a.state.hitbox_y == -(Sint32)type &&
          a.state.animation_set == 0x8005 && a.state.palette == 0x180 + type &&
          a.state.texture_offset == type * 3);
    a.hit_params = 1;
    CHECK(ActorChapel(s, &a) && a.permanent && s->slots[160].state.id == 2 &&
          s->slots[160].state.parameters == type % 3 && s->combat.sound_count == 1 &&
          s->combat.sounds[0] == (type >= 7 ? 0x61d : 0x634));
    if (type == 7 || type == 8)
        CHECK(a.remove && s->slots[96].state.id == 0xc && !s->slots[96].state.parameters);
    else
        CHECK(cube ? a.state.id == 3 && a.dynamic && a.timer == 16 && !a.remove : a.remove);
    if (type == 2 || type == 3)
    {
        const Uint16 count = type == 2 ? 4 : 3;
        for (Uint16 i = 0; i < count; ++i)
        {
            const Actor *part = &s->slots[161 + i * 2U];
            CHECK(part->state.id == 0x44 && part->state.facing_left &&
                  part->state.x == 128 * 65536 && part->state.y == (120 + i * 16) * 65536 &&
                  part->state.parameters == (type == 2 ? i : (i + 5) % 4));
        }
    }
    if (type == 9)
        CHECK(s->slots[96].state.id == 0x44 && s->slots[96].state.parameters == 256);
    SotnActors_Destroy(s);
    return true;
}
static bool Fragments(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = {.state = {.id = 0x44,
                         .parameters = 2,
                         .generation = 777,
                         .x = 128 * 65536,
                         .y = 128 * 65536,
                         .facing_left = true}};
    CHECK(ActorChapel(s, &a) && a.state.step == 1 && a.state.primitive_count == 2 &&
          a.state.x > 128 * 65536 && a.state.y < 128 * 65536 && a.state.rotation == 16);
    ActorPrimitive *p = ActorPrimitiveGet(s, &a, 0);
    CHECK(p && p->state.active && p->state.quad && p->state.page == 22 &&
          p->state.palette == 0x24d && p->state.v[0] == 164 && p->state.offset_x[0] < 0 &&
          !ActorPrimitiveGet(s, &a, 1)->state.active);
    for (Uint16 i = 0; i < 128 && !a.remove; ++i)
        CHECK(ActorChapel(s, &a));
    CHECK(a.remove && s->combat.sounds[0] == 0x683 && s->slots[160].state.id == 2);
    ActorPrimitivesClear(s, &a);
    a = (Actor){.state = {.id = 0x44, .parameters = 256, .generation = 778}};
    CHECK(ActorChapel(s, &a) && a.state.step == 256 && a.state.frame == 21 &&
          a.state.priority == 106 && a.state.palette == 0x189 && !a.state.primitive_count);
    CHECK(ActorChapel(s, &a) && a.state.step == 256);
    a = (Actor){.state = {.id = 0x44, .parameters = 4, .generation = 779}};
    CHECK(!ActorChapel(s, &a));
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 1;
    a = (Actor){.state = {.id = 0x44, .generation = 780}};
    CHECK(ActorChapel(s, &a) && a.remove && !a.state.primitive_count);
    SotnActors_Destroy(s);
    return true;
}
static bool Decorations(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    const size_t tables[] = {0xac8, 0x1b14U + 5U * 20U};
    for (Uint16 i = 0; i < 2; ++i)
    {
        Put16(f->stage + tables[i], 6);
        Put16(f->stage + tables[i] + 2, 123);
        Put16(f->stage + tables[i] + 4, 7);
        Put32(f->stage + tables[i] + 16, 0x80181e00);
        Actor a = {.state = {.id = (Uint16)(i ? 8 : 0x11), .parameters = (Uint16)(i ? 5 : 0)}};
        CHECK(ActorChapel(s, &a) && a.state.frame == 31 && a.state.priority == 123 &&
              a.state.texture_offset == 7 && a.state.rotated == (i != 0));
    }
    Actor a = {.state = {.id = 0x11, .parameters = 1}};
    CHECK(!ActorChapel(s, &a));
    a = (Actor){.state = {.id = 1, .parameters = 0xa000}};
    CHECK(!ActorChapel(s, &a));
    a.state.parameters = 0x7010;
    CHECK(!ActorChapel(s, &a));
    a.state.parameters = 0x50d6;
    a.hit_params = 1;
    CHECK(ActorChapel(s, &a) && a.state.id == 0xa && a.state.parameters == 86 && a.timer == 16 &&
          a.permanent);
    a = (Actor){.state = {.id = 1}};
    a.state.parameters = 0;
    Put32(f->stage + 0xbf0, 0x80180001);
    CHECK(!ActorChapel(s, &a));
    SotnActors_Destroy(s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    bool ok = f != NULL;
    for (Uint16 i = 0; i < 10 && ok; ++i)
        ok = Types(f, i, true) && Types(f, i, false);
    ok = ok && Fragments(f) && Decorations(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_CHAPEL_BREAKABLES: PASS");
    return 0;
}
