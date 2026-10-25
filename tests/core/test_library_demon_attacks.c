#include "actors_internal.h"
#include "boss_projection.h"
#include "library_demon_internal.h"
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
    Uint8 dra[0x1000], stage[0x4000], exe[0x23220], tiles[16 * 16 * 2], types[2];
} Fixture;
static void Put16(Uint8 *p, Sint32 v)
{
    const Uint16 value = (Uint16)v;
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
static void Open(Fixture *f, SotnActors *s, Uint16 id, bool facing)
{
    SDL_zero(*f);
    SDL_zero(*s);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 20);
    Put16(f->dra + 0x206, 12);
    Put16(f->dra + 0x208, 0x80);
    Put16(f->dra + 0x20c, 1);
    f->dra[0x222] = 8;
    f->dra[0x223] = 8;
    Put16(f->stage + 0x974, 0x800a);
    Put16(f->stage + 0x976, 25);
    Put16(f->stage + 0x980, 9);
    Put16(f->stage + 0x982, 1);
    Put16(f->stage + 0x1aec, 0);
    Put16(f->stage + 0x1aee, 14);
    Put16(f->stage + 0x1af0, 255);
    const size_t streams[4] = {0x1bcc, 0x1bd8, 0x1be8, 0x1bf0};
    for (Uint16 i = 0; i < 4; ++i)
    {
        Uint8 *p = f->stage + streams[i];
        p[0] = 1;
        p[1] = (Uint8)(25 + i * 4);
        p[2] = 1;
        p[3] = (Uint8)(26 + i * 4);
        p[4] = (Uint8)(i == 1 || i == 2 ? 255 : 0);
    }
    for (Uint32 i = 0; i <= 1024; ++i)
    {
        Put16(f->exe + 0x1dbcc + i * 2, (Sint32)SDL_round(SDL_sin(i * SDL_PI_D / 2048) * 4096));
        Put16(f->exe + 0x22a10 + i * 2, (Sint32)SDL_round(SDL_atan(i / 1024.0) * 2048 / SDL_PI_D));
    }
    for (Uint32 i = 0; i < 4096; ++i)
    {
        const double angle = i * SDL_PI_D / 2048.0;
        Put16(f->exe + 0x1ea10 + i * 4, (Sint32)SDL_round(SDL_sin(angle) * 4096));
        Put16(f->exe + 0x1ea12 + i * 4, (Sint32)SDL_round(SDL_cos(angle) * 4096));
    }
    for (Uint32 i = 16 * 10; i < 16 * 16; ++i)
        f->tiles[i * 2] = 1;
    f->types[1] = 3;
    s->stage_id = 2;
    s->nz0 = (SotnDataView){f->stage, sizeof(f->stage)};
    s->dra = (SotnDataView){f->dra, sizeof(f->dra)};
    s->executable = (SotnDataView){f->exe, sizeof(f->exe)};
    s->map = (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 16, 16};
    s->slots[64] = (Actor){
        .slot = 64,
        .state = {
            .generation = 1, .id = id, .x = 64 * 65536, .y = 96 * 65536, .facing_left = facing}};
}
static bool Spit(Fixture *f, SotnActors *s, bool facing)
{
    Open(f, s, 0x20, facing);
    Actor *a = &s->slots[64];
    CHECK(ActorLibrary(s, a) && a->state.step == 1 && a->state.scale_x == 384 &&
          a->state.scale_y == 384 && (a->state.flags & 0x80000000U));
    CHECK(ActorLibrary(s, a) && a->state.step == 2 && a->vx == (facing ? 262144 : -262144) &&
          a->vy == 262144);
    for (Uint16 i = 0; i < 30 && a->state.step == 2; ++i)
        CHECK(ActorLibrary(s, a));
    CHECK(a->state.step == 3 && a->state.scale_x == 320 && a->state.scale_y == 512 &&
          a->state.hitbox_height == 8);
    for (Uint16 i = 0; i < 8 && a->state.step == 3; ++i)
        CHECK(ActorLibrary(s, a));
    CHECK(a->state.step == 4 && a->state.primitive_count == 1 && s->combat.sounds[0] == 0x65b);
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    CHECK(p && p->state.active && p->state.offset_y[0] == -48 && p->state.blend &&
          p->state.priority == a->state.priority + 2);
    for (Uint16 i = 0; i < 18; ++i)
        CHECK(ActorLibrary(s, a));
    CHECK(a->state.step == 5 && a->demon.timer == 29 && a->state.hitbox_height == 64 &&
          p->state.offset_y[0] == -167 && a->state.hitbox_y == -105);
    CHECK(ActorLibrary(s, a) && a->state.hitbox_height == 71 && a->state.hitbox_y == -71 &&
          p->state.v[2] == 253);
    for (Uint16 i = 0; i < 28; ++i)
        CHECK(ActorLibrary(s, a));
    CHECK(a->state.step == 6 && !a->state.hitbox_state && !p->state.active);
    for (Uint16 i = 0; i < 8 && a->state.step == 6; ++i)
        CHECK(ActorLibrary(s, a));
    CHECK(a->state.step == 7 && a->state.opacity_set && a->state.opacity == 128);
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(ActorLibrary(s, a));
    CHECK(a->remove && a->state.opacity == 0);
    return true;
}
static bool Bolt(Fixture *f, SotnActors *s, bool facing)
{
    Open(f, s, 0x21, facing);
    Actor *a = &s->slots[64];
    CHECK(ActorLibrary(s, a) && a->state.step == 1 && a->state.primitive_count == 60 &&
          a->state.scale_x == 384 && a->vx == (facing ? 327680 : -327680));
    ActorPrimitive *head = ActorPrimitiveGet(s, a, 0), *line = ActorPrimitiveGet(s, a, 1);
    CHECK(head && line && head->state.offset_x[3] == 0 && head->demon.used && line->demon.used &&
          line->demon.hops == 3);
    CHECK(ActorLibrary(s, a) && Pixel(a->state.x) == (facing ? 69 : 59) && line->state.active &&
          line->state.line && line->state.line_blend_mode == 1 &&
          line->state.offset_x[1] == (facing ? 8 : -8));
    CHECK(ActorLibrary(s, a) && line->demon.phase == 2 && line->demon.wait == 8);
    CHECK(ActorPrimitiveGet(s, a, 4)->demon.used);
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(ActorLibrary(s, a));
    // A later link can immediately reuse the released first slot in this update.
    CHECK(line->demon.used && line->demon.phase == 0 && !line->state.active);
    for (Uint16 i = 0; i < 30; ++i)
        CHECK(ActorLibrary(s, a));
    CHECK(head->state.offset_x[1] - head->state.offset_x[2] == 128);
    // Force exhaustion at a link boundary; the original byte timer wraps and retries.
    for (Uint16 i = 1; i < 60; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        p->demon.used = true;
        p->demon.phase = 1;
        p->demon.wait = 1;
    }
    CHECK(ActorLibrary(s, a) && line->demon.phase == 1 && line->demon.wait == 0);
    CHECK(ActorLibrary(s, a) && line->demon.wait == 255);
    a->state.x = SDL_MAX_SINT32;
    a->vx = 1;
    CHECK(!ActorLibrary(s, a));
    return true;
}
static bool BodyEffects(Fixture *f, SotnActors *s)
{
    Open(f, s, 0x1f, true);
    Actor *a = &s->slots[64];
    a->state.priority = 134;
    CHECK(ActorDemonPunchEffect(s, a) && a->state.primitive_count == 2 &&
          a->demon.effect_step == 1);
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    CHECK(p && !p->state.active && Pixel(p->state.x) == 68 && Pixel(p->state.y) == 97);
    CHECK(ActorDemonPunchEffect(s, a) && p->state.active && p->state.offset_x[0] == -20 &&
          p->state.offset_y[0] == -8 && p->state.palette == 0x24a);
    CHECK(ActorDemonPunchEffect(s, a) && p->state.palette == 0x252 && a->demon.width == 44);
    for (Uint16 i = 0; i < 7; ++i)
        CHECK(ActorDemonPunchEffect(s, a));
    CHECK(a->demon.effect_step == 2 && a->demon.width == 58 && a->demon.height == 7);
    CHECK(ActorDemonPunchEffect(s, a) && a->state.primitive_count == 0 &&
          a->demon.effect_step == 3 && !(a->state.flags & 0x800000U));
    a->demon.effect_step = 0;
    CHECK(ActorDemonStarEffect(s, a) && a->state.primitive_count == 2 && a->demon.scale == 256 &&
          s->combat.sounds[0] == 0x614);
    CHECK(ActorDemonStarEffect(s, a) && a->demon.scale == 320 && a->demon.angle == 64);
    a->demon.effect_step = 2;
    CHECK(ActorDemonStarEffect(s, a) && !a->state.primitive_count);
    a->demon.effect_step = 0;
    bool ready = false;
    CHECK(ActorDemonWindup(s, a, &ready) && !ready && a->state.primitive_count == 24 &&
          a->demon.effect_step == 1);
    p = ActorPrimitiveGet(s, a, 0);
    CHECK(p && ActorDemonWindup(s, a, &ready) && p->state.red == 3 && p->state.active);
    CHECK(ActorDemonWindup(s, a, &ready) && p->state.red == 6 && !p->state.active);
    for (Uint16 i = 0; i < 48; ++i)
        CHECK(ActorDemonWindup(s, a, &ready) && !ready);
    CHECK(a->demon.effect_step == 2 && !a->state.primitive_count);
    CHECK(ActorDemonWindup(s, a, &ready) && ready);
    a->demon.effect_step = 0;
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 99;
    CHECK(ActorDemonWindup(s, a, &ready) && ready && a->demon.effect_step == 3);
    a->demon.effect_step = 0;
    CHECK(ActorDemonPunchEffect(s, a) && a->demon.effect_step == 4 && !a->state.primitive_count);
    return true;
}
static bool Projection(Fixture *f, SotnActors *s)
{
    Open(f, s, 0x1f, true);
    SotnProjectedPoint out[4] = {0};
    SotnDemonQuadPose pose = {
        .width = 40, .height = 16, .scale = 4096, .anchor_x = 128, .anchor_y = 100};
    CHECK(SotnDemon_ProjectQuad(s->executable, pose, out) && out[0].x == 108 && out[0].y == 92 &&
          out[3].x == 148 && out[3].y == 108);
    pose.angle = 1024;
    pose.scale = 2048;
    CHECK(SotnDemon_ProjectQuad(s->executable, pose, out) && out[0].x == 132 && out[0].y == 90);
    pose.scale = 0;
    CHECK(SotnDemon_ProjectQuad(s->executable, pose, out) && out[0].x == 128 && out[3].y == 100);
    pose.scale = 16385;
    CHECK(!SotnDemon_ProjectQuad(s->executable, pose, out) && out[0].x == 128);
    pose.scale = 4096;
    pose.width = 65;
    CHECK(!SotnDemon_ProjectQuad(s->executable, pose, out));
    pose.width = 40;
    CHECK(!SotnDemon_ProjectQuad((SotnDataView){0}, pose, out));
    CHECK(!SotnDemon_ProjectQuad(s->executable, pose, NULL));
    return true;
}
static bool Failures(Fixture *f, SotnActors *s)
{
    Open(f, s, 0x21, false);
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 99;
    CHECK(ActorLibrary(s, &s->slots[64]) && s->slots[64].remove &&
          !s->slots[64].state.primitive_count);
    Open(f, s, 0x20, false);
    Actor *a = &s->slots[64];
    a->state.step = 3;
    a->state.scale_y = 512;
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 99;
    for (Uint16 i = 0; i < 4 && !a->remove; ++i)
        CHECK(ActorLibrary(s, a));
    CHECK(a->remove);
    Open(f, s, 0x20, false);
    a = &s->slots[64];
    a->state.step = 6;
    a->state.scale_y = 16;
    CHECK(!ActorLibrary(s, a));
    a->state.step = 7;
    a->state.scale_y = 32;
    a->state.opacity = 0;
    CHECK(!ActorLibrary(s, a));
    a->state.step = 8;
    CHECK(!ActorLibrary(s, a));
    Open(f, s, 0x20, false);
    a = &s->slots[64];
    a->state.step = 3;
    f->stage[0x1bd8] = 0;
    CHECK(!ActorLibrary(s, a));
    Open(f, s, 0x21, false);
    a = &s->slots[64];
    CHECK(ActorLibrary(s, a));
    s->executable = (SotnDataView){0};
    CHECK(!ActorLibrary(s, a));
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    SotnActors *s = SDL_calloc(1, sizeof(*s));
    const bool ok = f && s && Spit(f, s, false) && Spit(f, s, true) && Bolt(f, s, false) &&
                    Bolt(f, s, true) && BodyEffects(f, s) && Projection(f, s) && Failures(f, s);
    SDL_free(s);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_LIBRARY_DEMON_ATTACK_TEST: PASS");
    return 0;
}
