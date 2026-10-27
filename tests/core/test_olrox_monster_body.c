#include "boss_projection.h"
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
    for (Uint16 i = 0; i < 4096; ++i)
    {
        const double angle = (double)i * SDL_PI_D / 2048.0;
        Put16(f->exe + 0x1ea10 + i * 4U, (Sint32)SDL_round(SDL_sin(angle) * 4096));
        Put16(f->exe + 0x1ea12 + i * 4U, (Sint32)SDL_round(SDL_cos(angle) * 4096));
    }
    for (Uint16 i = 0; i < 3; ++i)
    {
        const size_t offset = 0x714 + i * 12U;
        Put16(f->stage + offset, 0x8007);
        Put16(f->stage + offset + 4, 0x50);
        Put16(f->stage + offset + 6, 0x20b);
        Put16(f->stage + offset + 8, 56 + i);
    }
    f->stage[0x120c] = 2;
    f->stage[0x120d] = 1;
    f->stage[0x120e] = 0;
    for (Uint16 i = 0; i < 3; ++i)
    {
        f->stage[0x1234 + i * 2U] = (Uint8)(i == 1 ? 3 : 2);
        f->stage[0x1235 + i * 2U] = (Uint8)(i + 1U);
    }
    f->stage[0x123a] = 255;
    f->stage[0x12e4] = 2;
    f->stage[0x12e5] = 1;
    for (Uint16 i = 0; i < 4; ++i)
        Put16(f->stage + 0x12f8 + i * 2U, 64);
    Put16(f->stage + 0x1dd6, 128);
    Put32(f->stage + 0x1e14, 4096);
    return true;
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    f->layout = NULL;
}
static bool Requests(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[32], *head = OlroxChild(s, a, 3), *arms = OlroxChild(s, a, 7);
    CHECK(head && arms);
    s->olrox_monster.legs[0] = (OlroxLeg){.upper = -512, .lower = 768};
    s->olrox_monster.legs[1] = (OlroxLeg){.upper = -896, .lower = 256};
    head->state.hit_points = 50;
    a->state.step = 8;
    a->olrox.support_leg = 1;
    CHECK(ActorOlroxMonster(s, a) && a->substep == 1 && head->state.step == 2 &&
          arms->state.step == 6);
    CHECK(a->olrox.laser_rank == 2 && !a->olrox.laser_busy);
    for (Uint16 i = 0; i < 10; ++i)
        CHECK(ActorOlroxMonster(s, a) && a->substep == 1);
    a->olrox.laser_busy = 1;
    CHECK(ActorOlroxMonster(s, a) && a->substep == 2);
    CHECK(ActorOlroxMonster(s, a) && a->substep == 2);
    a->olrox.laser_busy = 0;
    CHECK(ActorOlroxMonster(s, a) && a->substep == 3 && a->olrox.laser_pattern == 8);
    CHECK(ActorOlroxMonster(s, a) && a->state.step == 10 && head->state.step == 1 &&
          arms->state.step == 2);
    CHECK(OlroxMonster_ChangeStep(s, a, 9));
    CHECK(ActorOlroxMonster(s, a) && a->substep == 1 && head->state.step == 4 &&
          head->olrox.fire_rank == 2);
    for (Uint16 i = 0; i < 10; ++i)
        CHECK(ActorOlroxMonster(s, a) && a->substep == 1);
    a->olrox.fire_busy = 1;
    CHECK(ActorOlroxMonster(s, a) && a->substep == 2);
    CHECK(ActorOlroxMonster(s, a) && a->substep == 2);
    a->olrox.fire_busy = 0;
    CHECK(ActorOlroxMonster(s, a) && a->substep == 3);
    CHECK(ActorOlroxMonster(s, a) && a->state.step == 10);
    CHECK(OlroxMonster_ChangeStep(s, a, 9));
    head->state.hit_points = 101;
    CHECK(!ActorOlroxMonster(s, a));
    CHECK(OlroxMonster_ChangeStep(s, a, 17));
    a->substep = 2;
    a->state.x = 128 * 65536;
    a->vx = SDL_MAX_SINT32;
    CHECK(!ActorOlroxMonster(s, a) && a->state.x == 128 * 65536);
    Close(f, s);
    return true;
}
static bool Handoff(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[32];
    for (Uint16 i = 1; i <= 9; ++i)
    {
        ActorPrimitivesClear(s, &s->slots[32 + i]);
        s->slots[32 + i] = (Actor){0};
    }
    ActorPrimitivesClear(s, a);
    *a = (Actor){.slot = 32,
                 .state = {.id = 0x33, .generation = 3, .x = 200 * 65536, .y = 460 * 65536}};
    s->olrox_morph.transforming = true;
    CHECK(ActorOlroxMonster(s, a) && a->state.step == 1 && a->substep == 1 &&
          a->state.primitive_count == 17);
    CHECK(s->olrox_monster.priority == 0xd0 && OlroxChild(s, a, 8)->state.parameters == 1);
    CHECK(ActorOlroxMonster(s, a) && a->substep == 2 && s->olrox_monster.priority == 0xa0);
    CHECK(ActorOlroxMonster(s, a) && a->state.step == 1 && a->substep == 2);
    s->olrox_morph.transforming = false;
    CHECK(ActorOlroxMonster(s, a) && a->state.step == 12);
    for (Uint16 i = 0; i < 8; ++i)
        CHECK((i ? OlroxChild(s, a, i) : a)->state.flags & 0x08000000U);
    CHECK(OlroxMonster_ChangeStep(s, a, 17));
    CHECK(ActorOlroxMonster(s, a));
    CHECK(s->olrox_monster.legs[1].lower == 288 && s->olrox_monster.legs[1].lower_velocity == 32);
    a->olrox.support_leg = 2;
    CHECK(!ActorOlroxMonster(s, a));
    Close(f, s);
    CHECK(Open(f, &s));
    a = &s->slots[32];
    ActorPrimitivesClear(s, a);
    *a = (Actor){.slot = 32,
                 .state = {.id = 0x33, .generation = 3, .x = 200 * 65536, .y = 460 * 65536}};
    s->slots[33].enemy_state.owner_generation = 99;
    const Uint32 old_generation = s->slots[33].state.generation;
    CHECK(!ActorOlroxMonster(s, a) && s->slots[33].state.generation == old_generation);
    Close(f, s);
    return true;
}
static bool WorkingCopies(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *body = &s->slots[32];
    Actor head = *OlroxChild(s, body, 3);
    CHECK(OlroxOwnsChild(s, body, &head, 3));
    CHECK(ActorOlroxHead(s, &head) && !head.remove && head.state.step == 1);
    s->slots[35] = head;
    head.state.step = 8;
    head.substep = 0;
    CHECK(ActorOlroxHead(s, &head) && !head.remove && head.state.primitive_count == 1);
    s->slots[35] = head;
    Actor foot = *OlroxChild(s, body, 1);
    CHECK(ActorOlroxAppendage(s, &foot) && !foot.remove && foot.state.step == 1);
    ++foot.state.generation;
    CHECK(!OlroxOwnsChild(s, body, &foot, 1));
    CHECK(ActorOlroxAppendage(s, &foot) && foot.remove);
    Close(f, s);
    return true;
}
static bool SharedDamage(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *body = &s->slots[32], *head = OlroxChild(s, body, 3), *hand = OlroxChild(s, body, 8);
    for (Uint16 i = 0; i <= 6; ++i)
    {
        Actor *part = i ? OlroxChild(s, body, i) : body;
        part->state.hit_points = i == 3 ? 80 : 0x7fff;
        CHECK(ActorOlroxMonsterOwner(s, part) == body);
    }
    CHECK(ActorOlroxMonsterOwner(s, hand) == body);
    ActorOlroxMonsterHit(s, head, body, 0, 12);
    CHECK(body->cooldown[0] == 12 && body->hit_flags == 0x10 && !head->cooldown[0]);
    CHECK(OlroxChild(s, body, 4)->cooldown[0] == 13 && !hand->cooldown[0]);
    ActorOlroxMonsterHit(s, head, head, 1, 14);
    CHECK(body->cooldown[1] == 14 && head->cooldown[1] == 14 &&
          OlroxChild(s, body, 6)->cooldown[1] == 15);
    head->state.flags |= 0x100U;
    ActorOlroxMonsterHit(s, head, body, 0, 12);
    for (Uint16 i = 0; i <= 6; ++i)
        CHECK((i ? OlroxChild(s, body, i) : body)->state.flags & 0x100U);
    CHECK(!(hand->state.flags & 0x100U));
    head->enemy_state.owner_generation = 99;
    CHECK(!ActorOlroxMonsterOwner(s, head));
    Close(f, s);
    return true;
}
static bool DefeatAndRays(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *body = &s->slots[32], *capture = NULL;
    CHECK(ActorSpawnAt(s, body, 0x3c, 0, 42, &capture));
    body->olrox.children[10] = capture->state.generation;
    CHECK(!SotnActors_HasTargets(s) && !SotnActors_HasTargets(NULL));
    ActorPrimitive *target = ActorPrimitiveGet(s, body, 0);
    CHECK(target);
    target->state.active = true;
    target->state.target = 3;
    CHECK(SotnActors_HasTargets(s));
    target->state.active = false;
    CHECK(!SotnActors_HasTargets(s));
    target->state.target = 0;
    Actor copy = *capture;
    CHECK(ActorOlroxDefeat(s, &copy) && !copy.remove && copy.state.primitive_count == 264);
    *capture = copy;
    CHECK(ActorOlroxDefeat(s, capture) && capture->state.step == 2 && body->olrox.death_phase == 2);
    CHECK(ActorOlroxDefeat(s, capture) && capture->state.step == 2);
    body->olrox.death_phase = 3;
    CHECK(ActorOlroxDefeat(s, capture) && capture->state.step == 3);
    bool awarded = false;
    for (Uint16 tick = 0; tick < 1000 && !capture->remove; ++tick)
    {
        s->ticks = tick;
        CHECK(ActorOlroxDefeat(s, capture));
        if (capture->state.step == 8)
            awarded = true;
    }
    CHECK(capture->remove && awarded && s->olrox_defeated);
    Actor *reward = NULL, *rays = NULL;
    for (Uint16 i = 0; i < SOTN_STAGE_ACTOR_SLOTS; ++i)
    {
        if (s->slots[i].state.id == 0x3f)
            reward = &s->slots[i];
        if (s->slots[i].state.id == 0x3e)
            rays = &s->slots[i];
    }
    CHECK(reward && rays && reward->state.parameters == 0);
    reward->state.step = 6; // Selected visual-allocation fallback's conversion boundary.
    CHECK(!ActorBossReward(s, reward));
    Put16(f->stage + 0x24d4, 23);
    CHECK(ActorBossReward(s, reward) && reward->state.id == 3 &&
          reward->state.parameters == 0x8017);
    rays->state.step = 0;
    CHECK(ActorOlroxDeathRays(s, rays) && rays->state.primitive_count == 128);
    ActorPrimitive *line = ActorPrimitiveGet(s, rays, 0);
    CHECK(line && line->state.active && line->state.line && line->state.red == 96 &&
          line->state.green == 96 && line->state.blue == 64 && line->state.line_blend_mode == 1);
    rays->state.step = 2;
    for (Uint16 tick = 0; tick < 160; ++tick)
        CHECK(ActorOlroxDeathRays(s, rays));
    for (Uint16 i = 0; i < rays->state.primitive_count; ++i)
        CHECK(!ActorPrimitiveGet(s, rays, i)->state.active);
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok =
        Requests(f) && Handoff(f) && WorkingCopies(f) && SharedDamage(f) && DefeatAndRays(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
