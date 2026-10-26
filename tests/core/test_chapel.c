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
    Uint8 dra[0x1000], stage[0x2400], exe[0x23220];
} Fixture;
static void Put16(Uint8 *p, Sint32 v)
{
    p[0] = (Uint8)(Uint16)v;
    p[1] = (Uint8)((Uint16)v >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (8U * i));
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 20);
    Put16(f->dra + 0x206, 10);
    const Uint16 profiles[] = {0xa40, 0xa4c, 0xa58};
    for (Uint16 i = 0; i < SDL_arraysize(profiles); ++i)
        Put16(f->stage + profiles[i], 0x8008);
    f->stage[0x1ec0] = 2;
    f->stage[0x1ec1] = 1;
    f->stage[0x1ec2] = 2;
    f->stage[0x1ec3] = 2;
    f->stage[0x1ec4] = 2;
    f->stage[0x1ec5] = 3;
    for (Uint16 i = 0; i < 8; ++i)
    {
        Put32(f->stage + 0x1ed0 + i * 8U, i * 1024U);
        Put32(f->stage + 0x1ed4 + i * 8U, (Uint32)-32768);
    }
    Put16(f->stage + 0x1f10, 96);
    Put16(f->stage + 0x1f12, 160);
    Put16(f->stage + 0x1f14, 100);
    Put16(f->stage + 0x1f16, 200);
    // Synthetic bat frames and generated sine values, not original runtime arrays.
    f->stage[0x1f20] = 2;
    f->stage[0x1f21] = 5;
    f->stage[0x1f3c] = 1;
    f->stage[0x1f3d] = 6;
    f->stage[0x1f3e] = 255;
    for (Uint32 i = 0; i <= 1024; ++i)
        Put16(f->exe + 0x1dbcc + i * 2U, (Sint32)SDL_round(SDL_sin(i * SDL_PI_D / 2048.0) * 4096));
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 6, out));
    CHECK(SotnActors_SetServices(*out, (SotnDataView){f->exe, sizeof(f->exe)}, 0, false));
    (*out)->player = (SotnPlayerState){.x = 192 * 65536, .y = 128 * 65536, .hp = 70};
    return true;
}
static bool Guard(SotnActors *s)
{
    Actor *a = &s->slots[64];
    *a = (Actor){
        .slot = 64,
        .state = {.id = 0x35, .generation = 1, .x = 128 * 65536 + 123, .y = 128 * 65536 + 456}};
    CHECK(ActorChapel(s, a) && a->state.step == 1 && a->state.facing_left && a->vx == 65536 &&
          a->state.x == 129 * 65536 + 123 && a->state.rotation == 16 && a->state.hitbox_y == 2);
    CHECK(ActorChapel(s, a) && a->state.rotation == 32 && a->vy > 0);
    CHECK(ActorChapel(s, a));
    CHECK(ActorChapel(s, a) && s->combat.sound_count == 1 && s->combat.sounds[0] == 0x68c);
    a->state.flags |= 0x100U;
    CHECK(ActorChapel(s, a) && a->remove);
    CHECK(s->slots[160].state.id == 2 && s->slots[160].state.parameters == 2);
    for (Uint16 i = 0; i < 8; ++i)
    {
        Actor *part = &s->slots[161 + i];
        CHECK(part->state.id == 0x36 && part->state.parameters == i &&
              (part->state.x & 65535) == 0 && (part->state.y & 65535) == 0 && part->vx == a->vx &&
              part->vy == a->vy);
        const Sint32 before_x = part->state.x, before_y = part->state.y;
        CHECK(ActorChapel(s, part) && part->state.step == 1 && part->state.frame == i + 5 &&
              part->state.rotated == (i == 0) && (part->state.flags & 0x40000000U) &&
              part->state.x == before_x && part->state.y == before_y);
        CHECK(ActorChapel(s, part) && part->state.x != before_x);
    }
    Actor invalid = {.state = {.id = 0x36, .parameters = 8}};
    CHECK(!ActorChapel(s, &invalid));
    return true;
}
static bool Spawner(SotnActors *s)
{
    s->player.x = 128 * 65536;
    s->player.y = 150 * 65536;
    Actor spawn = {.slot = 63, .state = {.id = 0x42, .generation = 2, .parameters = 1}};
    CHECK(ActorChapel(s, &spawn) && spawn.state.step == 1 && spawn.timer >= 192 &&
          spawn.timer <= 239);
    Actor *child = &s->slots[104];
    CHECK(child->state.id == 0x35 && child->state.priority == 176 &&
          (Pixel(child->state.x) == -32 || Pixel(child->state.x) == 288) &&
          Pixel(child->state.y) >= 86 && Pixel(child->state.y) <= 213);
    spawn.timer = 1;
    s->player.x = 20 * 65536;
    CHECK(ActorChapel(s, &spawn) && !s->slots[105].state.generation);
    spawn.state.parameters = 3;
    CHECK(!ActorChapel(s, &spawn));
    return true;
}
static bool Bat(SotnActors *s)
{
    s->player.x = 192 * 65536;
    s->player.y = 128 * 65536;
    Actor bat = {.slot = 80,
                 .state = {.id = 0x37, .generation = 3, .x = 128 * 65536, .y = 120 * 65536}};
    CHECK(ActorChapel(s, &bat) && bat.state.step == 1 && bat.state.frame == 31);
    CHECK(ActorChapel(s, &bat) && bat.state.step == 2);
    for (Uint16 i = 0; i < 2; ++i)
        CHECK(ActorChapel(s, &bat));
    CHECK(bat.state.step == 3 && bat.vy == 57344);
    CHECK(ActorChapel(s, &bat) && bat.state.step == 4);
    bat.state.flags |= 0x100U;
    CHECK(ActorChapel(s, &bat) && bat.remove);
    Actor unsupported = {.state = {.id = 0x43, .generation = 4}};
    CHECK(ActorChapel(s, &unsupported) && unsupported.state.unsupported);
    CHECK(!ActorChapel(NULL, &unsupported) && !ActorChapel(s, NULL));
    return true;
}
static bool Bounds(Fixture *f, SotnActors *s)
{
    for (Uint16 i = 104; i < 108; ++i)
        s->slots[i].state.generation = 0x80000000U + i;
    const Uint32 serial = s->effect_serial;
    Actor spawn = {.state = {.id = 0x42, .step = 1}, .timer = 1};
    CHECK(ActorChapel(s, &spawn) && s->effect_serial == serial);
    spawn.timer = 1;
    spawn.state.parameters = 1;
    Put16(f->stage + 0x1f10, 200);
    Put16(f->stage + 0x1f12, 100);
    CHECK(!ActorChapel(s, &spawn));
    Actor guard = {.state = {.id = 0x35, .step = 1, .x = SDL_MAX_SINT32}, .vx = 65536};
    CHECK(!ActorChapel(s, &guard) && guard.state.x == SDL_MAX_SINT32);
    Actor part = {.state = {.id = 0x36}};
    Put32(f->stage + 0x1ed0, SDL_MAX_SINT32);
    CHECK(!ActorChapel(s, &part));
    CHECK(!ActorPositionalSound(NULL, &part, 1) && !ActorPositionalSound(s, NULL, 1));
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    SotnActors *s = NULL;
    const SotnStageProfile *p = SotnStageProfile_Get(6);
    bool ok = p && p->connected && p->actor_limit == 0x49 && p->sprite_banks == 24 &&
              p->prize_count == 16 && p->prize_bit == 0x98 && p->door_types == 3 &&
              p->camera_count == 7 && SotnStageProfile_SaveRoom(6, 16) &&
              SotnStageProfile_SaveRoom(6, 18) && !SotnStageProfile_SaveRoom(6, 17);
    ok = ok && f && Open(f, &s) && Guard(s) && Spawner(s) && Bat(s);
    if (ok)
        ok = ActorStageOffset(s, 0x1cc4) == 0x17c0 && ActorStageOffset(s, 0x13b0) == 0xec0 &&
             ActorStageOffset(s, 0xdbc) == 0xb0c && ActorStageOffset(s, 0x1960) == 0x145c &&
             ActorStageOffset(s, 0x23dc) == s->nz0.size && Bounds(f, s);
    SotnActors_Destroy(s);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_CHAPEL_TEST: PASS");
    return 0;
}
