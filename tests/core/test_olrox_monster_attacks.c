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
static bool Projection(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    SotnProjectedPoint p[4] = {0};
    SotnPrimitiveQuadPose pose = {.width = 40,
                                  .height = 16,
                                  .scale_x = 4096,
                                  .scale_y = 4096,
                                  .anchor_x = 128,
                                  .anchor_y = 100};
    CHECK(SotnPrimitive_ProjectQuad(s->executable, pose, p) && p[0].x == 108 && p[0].y == 92 &&
          p[3].x == 148 && p[3].y == 108);
    pose.z_angle = 1024;
    pose.scale_x = 2048;
    CHECK(SotnPrimitive_ProjectQuad(s->executable, pose, p) && p[0].x == 136 && p[0].y == 90);
    pose.x_angle = 1024;
    pose.z_angle = 0;
    CHECK(SotnPrimitive_ProjectQuad(s->executable, pose, p) && p[0].y == 100 && p[3].y == 100);
    const SotnProjectedPoint saved = p[0];
    pose.scale_y = 4097;
    CHECK(!SotnPrimitive_ProjectQuad(s->executable, pose, p) && p[0].x == saved.x &&
          p[0].y == saved.y);
    pose.scale_y = 4096;
    CHECK(!SotnPrimitive_ProjectQuad((SotnDataView){0}, pose, p));
    CHECK(!SotnPrimitive_ProjectQuad(s->executable, pose, NULL));
    Close(f, s);
    return true;
}
static bool HeadTiming(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *body = &s->slots[32], *head = &s->slots[35];
    CHECK(ActorOlroxHead(s, head) && head->state.step == 1 && head->state.rotation == -256);
    head->state.step = 5;
    head->state.rotation = 384;
    body->olrox.roar_busy = 1;
    Uint16 sounds = 0;
    for (Uint16 i = 0; i < 9; ++i)
    {
        s->combat = (SotnActorCombat){0};
        CHECK(ActorOlroxHead(s, head));
        if (s->combat.sound_count)
        {
            CHECK(s->combat.sound_count == 1 && s->combat.sounds[0] == 0x819 && head->pose == 2 &&
                  !head->remaining);
            ++sounds;
        }
    }
    CHECK(sounds == 1 && !body->olrox.roar_busy && head->state.step == 1);
    head->state.step = 3;
    head->substep = 0;
    head->olrox.laser_busy = 1;
    CHECK(ActorOlroxHead(s, head) && head->substep == 1 && body->olrox.laser_busy);
    CHECK(s->slots[0].state.id == 0x39 &&
          s->slots[0].enemy_state.owner_generation == head->state.generation);
    CHECK(ActorOlroxHead(s, head) && body->olrox.laser_busy);
    head->olrox.laser_busy = 0;
    CHECK(ActorOlroxHead(s, head) && !body->olrox.laser_busy);
    ++body->state.generation;
    CHECK(ActorOlroxHead(s, head) && head->remove);
    Close(f, s);
    return true;
}
static bool Projectiles(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *head = &s->slots[35];
    CHECK(ActorOlroxHead(s, head));
    head->state.x = 256 * 65536;
    head->state.y = 200 * 65536;
    Actor *fire = NULL;
    CHECK(ActorSpawnAt(s, head, 0x3a, 0, 100, &fire));
    CHECK(ActorOlroxMonsterAttack(s, fire) && fire->state.primitive_count == 2 &&
          fire->state.scale_x == 320 && fire->state.hitbox_width == 5);
    CHECK(ActorOlroxMonsterAttack(s, fire) && ActorPrimitiveGet(s, fire, 0)->state.active);
    ++head->state.generation;
    for (Uint16 i = 0; i < 200 && !fire->remove; ++i)
        CHECK(ActorOlroxMonsterAttack(s, fire));
    CHECK(fire->remove && (fire->state.flags & 0x100U));
    Actor *drool = NULL;
    head->state.x = 160 * 65536;
    head->state.y = 300 * 65536;
    CHECK(ActorSpawnAt(s, head, 0x3d, 18, 101, &drool));
    CHECK(ActorOlroxMonsterAttack(s, drool) && !drool->state.hitbox_state &&
          ActorPrimitiveGet(s, drool, 0)->state.line);
    for (Uint16 i = 0; i < 240 && !drool->remove; ++i)
    {
        s->combat = (SotnActorCombat){0};
        CHECK(ActorOlroxMonsterAttack(s, drool));
    }
    CHECK(drool->remove && drool->substep == 1 && s->slots[96].state.id == 0x14 &&
          s->slots[96].state.scale_x == 128);
    Actor *invalid = NULL;
    CHECK(ActorSpawnAt(s, head, 0x3d, 18, 102, &invalid));
    CHECK(ActorOlroxMonsterAttack(s, invalid));
    s->camera_x = 32;
    head->state.x = SDL_MIN_SINT32;
    CHECK(!ActorOlroxMonsterAttack(s, invalid));
    Close(f, s);
    return true;
}
static bool Laser(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *head = &s->slots[35], *laser = NULL;
    CHECK(ActorOlroxHead(s, head));
    head->state.x = 256 * 65536;
    head->state.y = 360 * 65536;
    head->olrox.laser_busy = 1;
    CHECK(ActorSpawnAt(s, head, 0x39, 1, 100, &laser));
    CHECK(ActorOlroxMonsterAttack(s, laser) && laser->state.primitive_count == 48 &&
          laser->state.hitbox_height == 56);
    CHECK(ActorOlroxMonsterAttack(s, laser) && laser->state.step == 2);
    SotnActorPrimitive *floor = &ActorPrimitiveGet(s, laser, 1)->state;
    floor->offset_x[0] = floor->offset_x[2] = 120;
    floor->offset_x[1] = floor->offset_x[3] = 200;
    floor->offset_y[0] = floor->offset_y[2] = 464;
    floor->offset_y[1] = floor->offset_y[3] = 464;
    laser->state.y = 464 * 65536;
    laser->state.step = 6;
    CHECK(ActorOlroxMonsterAttack(s, laser) && laser->state.step == 7 && !head->olrox.laser_busy &&
          laser->state.hitbox_state == 1 && laser->state.hitbox_width == 32);
    const SotnActorPrimitive begin = ActorPrimitiveGet(s, laser, 1)->state,
                             restore = ActorPrimitiveGet(s, laser, 2)->state;
    CHECK(begin.target == 3 && begin.clip_left == 64 && begin.clip_right == 128 &&
          begin.clip_bottom == 512 && restore.target == 4);
    for (Uint16 i = 0; i < 400 && !laser->remove; ++i)
    {
        s->combat = (SotnActorCombat){0};
        ++s->ticks;
        CHECK(ActorOlroxMonsterAttack(s, laser));
    }
    CHECK(laser->remove && !laser->state.hitbox_state && laser->state.step == 9);
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool okay = Projection(f) && HeadTiming(f) && Projectiles(f) && Laser(f);
    SDL_free(f);
    return okay ? 0 : 1;
}
