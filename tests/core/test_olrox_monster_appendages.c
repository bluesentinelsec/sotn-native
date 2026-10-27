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
    SDL_memcpy(f->stage + 0x3300, f->stage + 0x3120, 24);
    for (Uint16 i = 1; i < 64; ++i)
        Put32(f->stage + 0x3100 + i * 4U, 0x80183300);
    Put16(f->stage + 0x708, 0x8007);
    Put16(f->stage + 0x70c, 0x50);
    Put16(f->stage + 0x70e, 0x20b);
    Put16(f->stage + 0x710, 55);
    for (Uint16 i = 55; i < 59; ++i)
    {
        Uint8 *record = f->dra + 0x200 + i * 40U;
        Put16(record + 4, 100);
        Put16(record + 6, 12 + (i - 55) * 2);
        Put16(record + 12, 3);
        record[34] = 10;
        record[35] = 12;
        Put32(record + 36, 0x08000000);
    }
    Put32(f->stage + 0x12d0, 0x80181300);
    Put32(f->stage + 0x12d4, 0x80181310);
    for (Uint16 i = 0; i < 8; ++i)
    {
        Put16(f->stage + 0x1300 + i * 2U, -64 + i * 12);
        Put16(f->stage + 0x1310 + i * 2U, 128 - i * 8);
    }
    for (Uint16 i = 1; i <= 9; ++i)
    {
        Actor *child = OlroxChild(*out, body, i);
        const Uint16 id = i <= 2   ? 0x34
                          : i == 3 ? 0x35
                          : i == 4 ? 0x36
                          : i <= 6 ? 0x3b
                          : i == 7 ? 0x37
                                   : 0x38;
        if (!child)
        {
            CHECK(ActorSpawnAt(*out, body, id, i == 8 ? 1 : 0, 32 + i, &child));
            body->olrox.children[i] = child->state.generation;
        }
        child->state.facing_left = false;
    }
    return true;
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    f->layout = NULL;
}
static bool FeetAndOwnership(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *body = &s->slots[32], *foot = &s->slots[33], *knee = &s->slots[37];
    s->olrox_monster.legs[0] = (OlroxLeg){.foot_x = 77 * 65536 + 123,
                                          .foot_y = 90 * 65536 + 456,
                                          .knee_x = 81 * 65536 + 789,
                                          .knee_y = 64 * 65536 + 987};
    CHECK(ActorOlroxAppendage(s, foot) && foot->state.step == 1 && foot->state.hitbox_width == 12 &&
          foot->state.hitbox_x == -12);
    CHECK(ActorOlroxAppendage(s, foot) && foot->state.x == 77 * 65536 + 123 &&
          foot->state.y == 90 * 65536 + 456);
    const Uint32 fraction = (Uint32)knee->state.x & 65535U;
    CHECK(ActorOlroxAppendage(s, knee) && Pixel(knee->state.x) == 81 &&
          Pixel(knee->state.y) == 64 && ((Uint32)knee->state.x & 65535U) == fraction &&
          knee->state.hitbox_width == 4 && knee->state.hitbox_height == 16);
    const Sint32 x = foot->state.x;
    ++body->state.generation;
    CHECK(ActorOlroxAppendage(s, foot) && foot->remove && foot->state.x == x);
    Close(f, s);
    return true;
}
static bool TailAndArms(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *body = &s->slots[32], *tail = &s->slots[36], *arms = &s->slots[39],
          *back = &s->slots[40], *front = &s->slots[41];
    s->olrox_morph.transforming = true;
    CHECK(ActorOlroxAppendage(s, tail) && tail->state.step == 1 &&
          tail->state.primitive_count == 8 && tail->olrox_tail.growth == 0);
    s->olrox_morph.transforming = false;
    for (Uint16 i = 0; i < 11; ++i)
        CHECK(ActorOlroxAppendage(s, tail));
    CHECK(tail->state.step == 3 && tail->olrox_tail.growth == 4096 &&
          Pixel(tail->state.x) == Pixel(body->state.x) + 4);
    for (Uint16 i = 0; i < 200; ++i)
        CHECK(ActorOlroxAppendage(s, tail));
    CHECK(tail->olrox_tail.phase <= 1 && ActorPrimitiveGet(s, tail, 7)->state.active);
    CHECK(ActorOlroxAppendage(s, back) && ActorOlroxAppendage(s, front));
    CHECK(ActorOlroxAppendage(s, arms) && arms->state.primitive_count == 6 && front->attack == 12 &&
          ActorPrimitiveGet(s, arms, 0)->state.priority == 163 &&
          ActorPrimitiveGet(s, arms, 2)->state.priority == 157 &&
          !ActorPrimitiveGet(s, arms, 4)->state.active);
    arms->state.step = 12;
    for (Uint16 i = 0; i < 250; ++i)
    {
        CHECK(ActorOlroxAppendage(s, arms));
        if (arms->state.step == 1)
            break;
    }
    CHECK(arms->state.step == 1 && front->attack == 18 && arms->olrox_arms.extension[1] == 64);
    back->state.flags |= 0x100U;
    CHECK(ActorOlroxAppendage(s, arms) && arms->state.step == 9 && arms->olrox_arms.damaged == 2);
    back->state.flags &= ~0x100U;
    CHECK(ActorOlroxAppendage(s, arms) && arms->state.step == 1);
    ++front->state.generation;
    CHECK(!ActorOlroxAppendage(s, arms));
    Close(f, s);
    return true;
}
static bool HandsAndCapture(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *body = &s->slots[32], *hand = &s->slots[41];
    CHECK(ActorOlroxAppendage(s, hand) && hand->state.hit_points == 128 &&
          hand->state.hitbox_width == 12 && hand->state.frame == 31);
    hand->state.flags |= 0x100U;
    CHECK(ActorOlroxAppendage(s, hand) && hand->state.step == 2 && !hand->state.hitbox_state);
    CHECK(ActorOlroxAppendage(s, hand) && hand->state.frame == 0 && hand->olrox.timer == 256);
    for (Uint16 i = 0; i < 386; ++i)
        CHECK(ActorOlroxAppendage(s, hand));
    CHECK(hand->state.step == 1 && hand->state.hitbox_state == 3 && !(hand->state.flags & 0x100U) &&
          hand->state.transform_flags == SOTN_ACTOR_ROTATE);
    hand->state.frame = 31;
    hand->state.step = 3;
    hand->substep = 0;
    CHECK(ActorOlroxAppendage(s, hand) && hand->substep == 1 && hand->state.primitive_count == 1);
    CHECK(ActorPrimitiveGet(s, hand, 0)->state.active);
    body->olrox.death_phase = 1;
    CHECK(ActorOlroxAppendage(s, hand) && hand->substep == 2 &&
          !ActorPrimitiveGet(s, hand, 0)->state.active);
    body->olrox.death_phase = 3;
    CHECK(ActorOlroxAppendage(s, hand) && ActorPrimitiveGet(s, hand, 0)->state.active &&
          ActorPrimitiveGet(s, hand, 0)->state.page == 0x15);
    CHECK(hand->substep == 3 && ActorOlroxAppendage(s, hand) && hand->substep == 3 &&
          ActorPrimitiveGet(s, hand, 0)->state.active);
    hand->state.parameters = 1;
    CHECK(!ActorOlroxAppendage(s, hand));
    Close(f, s);
    return true;
}
static bool ParticleAndCapacity(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *body = &s->slots[32], *tail = &s->slots[36];
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        if (!s->primitives[i].owner)
            s->primitives[i].owner = 0x12345678;
    CHECK(ActorOlroxAppendage(s, tail) && tail->state.step == 0 &&
          tail->state.primitive_count == 0);
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        if (s->primitives[i].owner == 0x12345678)
            s->primitives[i].owner = 0;
    CHECK(ActorOlroxAppendage(s, tail) && tail->state.primitive_count == 8);
    tail->state.step = 3;
    tail->olrox_tail.phase = 2;
    CHECK(!ActorOlroxAppendage(s, tail));
    Put16(f->stage + 0x60c, 2);
    for (Uint16 i = 0; i < 15; ++i)
    {
        f->stage[0x1cd4 + i * 2U] = 2;
        f->stage[0x1cd5 + i * 2U] = (Uint8)(1U + i);
    }
    f->stage[0x1cd4 + 30] = 255;
    for (Uint16 kind = 0; kind < 4; ++kind)
    {
        Actor *particle = &s->slots[160 + kind];
        *particle = (Actor){.slot = (Uint16)(160 + kind),
                            .state = {.id = 0x28,
                                      .generation = (Uint32)(400 + kind),
                                      .x = 128 * 65536,
                                      .y = 128 * 65536},
                            .olrox = {.angle12 = 256, .angle = (Uint8)kind, .decision = 6}};
        CHECK(ActorOlroxFirework(s, particle) && particle->state.animation_set == 2 &&
              particle->state.palette == 0x81b6 && particle->state.priority == 192 &&
              particle->state.opacity == 112 &&
              particle->state.transform_flags == (SOTN_ACTOR_ROTATE | SOTN_ACTOR_OPACITY));
        CHECK(particle->state.rotation == (kind == 1 ? 2304 : kind == 2 ? 1408 : 256));
        for (Uint16 i = 0; i < 40 && !particle->remove; ++i)
            CHECK(ActorOlroxFirework(s, particle));
        CHECK(particle->remove);
    }
    body->state.id = 0x2b;
    CHECK(ActorOlroxAppendage(s, tail) && tail->remove);
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool okay =
        FeetAndOwnership(f) && TailAndArms(f) && HandsAndCapture(f) && ParticleAndCapacity(f);
    SDL_free(f);
    return okay ? 0 : 1;
}
