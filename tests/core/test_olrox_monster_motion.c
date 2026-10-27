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
        Put16(f->exe + 0x1dbcc + i * 2U, i * 4);
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
    return true;
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    f->layout = NULL;
}
static bool Angles(void)
{
    Sint16 value = 0, velocity = 0;
    bool arrived = true;
    CHECK(OlroxMonster_Approach(&value, 24, 24, &arrived) && !arrived && value == 24);
    CHECK(OlroxMonster_Approach(&value, 24, 24, &arrived) && arrived);
    value = 48;
    CHECK(OlroxMonster_Approach(&value, 24, 24, &arrived) && !arrived && value == 24);
    CHECK(OlroxMonster_Approach(&value, 12, 24, &arrived) && arrived && value == 12);
    CHECK(!OlroxMonster_Approach(&value, 0, 0, &arrived) && value == 12);
    value = 0;
    CHECK(OlroxMonster_Smooth(&value, &velocity, 3, 16, &arrived) && !arrived && value == 1 &&
          velocity == 1);
    CHECK(OlroxMonster_Smooth(&value, &velocity, 3, 16, &arrived) && !arrived && value == 3 &&
          velocity == 2);
    CHECK(OlroxMonster_Smooth(&value, &velocity, 3, 16, &arrived) && arrived && velocity == 0);
    CHECK(!OlroxMonster_Smooth(&value, &value, 0, 16, &arrived));
    CHECK(!OlroxMonster_Smooth(&value, &velocity, 0, -1, &arrived));
    return true;
}
static bool Poses(Fixture *f)
{
    for (Sint32 selector = OLROX_FORWARD_SWING; selector <= OLROX_SLOW_SUPPORT; ++selector)
    {
        SotnActors *s = NULL;
        CHECK(Open(f, &s));
        Actor *body = &s->slots[32];
        body->olrox.body_angle = 37;
        s->olrox_monster.legs[0] = (OlroxLeg){.upper = -512, .lower = 768};
        s->olrox_monster.legs[1] = (OlroxLeg){.upper = -896, .lower = 256};
        bool arrived = false;
        for (Uint16 i = 0; i < 400; ++i)
            CHECK(OlroxMonster_Pose(s, body, 0, (OlroxLegPose)selector, &arrived));
        const OlroxLegPose pose = (OlroxLegPose)selector;
        if (pose == OLROX_FORWARD_SWING || pose == OLROX_BACKWARD_SWING || pose == OLROX_SLOW_SWING)
            CHECK(s->olrox_monster.legs[0].phase == 3);
        if (pose == OLROX_FORWARD_SUPPORT || pose == OLROX_SLOW_SUPPORT)
            CHECK(s->olrox_monster.legs[0].phase == 2);
        if (pose == OLROX_BACKWARD_SUPPORT)
            CHECK(s->olrox_monster.legs[0].phase == 3);
        if (pose == OLROX_TAKEOFF || pose == OLROX_LOWER_BODY || pose == OLROX_RAISE_BODY ||
            pose == OLROX_WIDE_STANCE || pose == OLROX_LANDING || pose == OLROX_CROUCH ||
            pose == OLROX_RECOVER)
            CHECK(arrived);
        const Sint16 upper = s->olrox_monster.legs[0].upper;
        body->pose = 19;
        body->substep = 4;
        CHECK(OlroxMonster_ChangeStep(s, body, 0x10c) && body->state.step == 12 &&
              body->substep == 0 && body->pose == 19 && s->olrox_monster.legs[0].phase == 0 &&
              s->olrox_monster.legs[0].upper == upper);
        CHECK(!OlroxMonster_Pose(s, body, 2, pose, &arrived));
        CHECK(!OlroxMonster_Pose(s, body, 0, (OlroxLegPose)99, &arrived));
        ++body->state.generation;
        CHECK(!OlroxMonster_Pose(s, body, 0, pose, &arrived));
        Close(f, s);
    }
    return true;
}
static bool Anchoring(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *body = &s->slots[32], *left = NULL, *right = NULL;
    CHECK(ActorSpawnAt(s, body, 0x34, 0, 33, &left));
    CHECK(ActorSpawnAt(s, body, 0x34, 0, 34, &right));
    body->olrox.children[1] = left->state.generation;
    body->olrox.children[2] = right->state.generation;
    left->state.x = 100 * 65536;
    left->state.y = right->state.y = 460 * 65536;
    right->state.x = 120 * 65536;
    CHECK(OlroxMonster_Anchor(s, body, false) && body->state.x == 100 * 65536 &&
          body->state.y == 386 * 65536 && s->olrox_monster.legs[0].knee_y == 418 * 65536 &&
          s->olrox_monster.legs[1].foot_y == 460 * 65536);
    CHECK(OlroxMonster_Anchor(s, body, true) && body->state.y == 386 * 65536 &&
          s->olrox_monster.legs[0].knee_y == 414 * 65536 &&
          s->olrox_monster.legs[0].foot_y == 456 * 65536);
    Sint16 adjustment = 99;
    CHECK(OlroxMonster_Floor(s, body, 0, &adjustment) && adjustment == 0 &&
          !s->olrox_monster.legs[0].contact);
    s->olrox_monster.legs[0].foot_y = 463 * 65536 + 123;
    CHECK(OlroxMonster_Floor(s, body, 0, &adjustment) && adjustment == -3 &&
          s->olrox_monster.legs[0].contact == 1);
    CHECK(!OlroxMonster_Floor(s, body, 2, &adjustment));
    ++right->state.generation;
    CHECK(!OlroxMonster_Anchor(s, body, false));
    --right->state.generation;
    s->executable = (SotnDataView){0};
    CHECK(!OlroxMonster_Anchor(s, body, false));
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool okay = Angles() && Poses(f) && Anchoring(f);
    SDL_free(f);
    return okay ? 0 : 1;
}
