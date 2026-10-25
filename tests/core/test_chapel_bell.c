#include "actors_internal.h"
#include "boss_projection.h"
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
    Uint8 dra[0x1000], stage[0x1800], exe[0x23220], tiles[2048], types[2];
    SotnActorLayout *layout;
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
static Sint32 LinearSine(Uint32 phase)
{
    phase &= 4095U;
    return phase <= 1024   ? (Sint32)phase * 4
           : phase <= 3072 ? 8192 - (Sint32)phase * 4
                           : (Sint32)phase * 4 - 16384;
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    Put32(f->dra + 0x224, 0x08000000);
    for (Uint16 i = 0; i <= 1024; ++i)
        Put16(f->exe + 0x1dbcc + i * 2U, i * 4);
    for (Uint32 i = 0; i < 4096; ++i)
    {
        Put16(f->exe + 0x1ea10 + i * 4U, LinearSine(i));
        Put16(f->exe + 0x1ea12 + i * 4U, LinearSine(i + 1024U));
    }
    // Deliberately different private profiles, model and spawner order/positions.
    Put16(f->stage + 0xd2c, 2);
    Put16(f->stage + 0xd2e, 14);
    Put16(f->stage + 0xd30, 180);
    Put16(f->stage + 0xd32, 4);
    Put16(f->stage + 0xd34, 17);
    Put16(f->stage + 0xd36, 182);
    Put16(f->stage + 0xd38, 160);
    Put16(f->stage + 0xd3a, 40);
    Put16(f->stage + 0xd3c, 1);
    Put16(f->stage + 0xd3e, 240);
    Put16(f->stage + 0xd40, 64);
    Put16(f->stage + 0xd42, 0);
    Put32(f->stage + 0xd4c, 1024);
    for (Uint16 i = 0; i < 4; ++i)
    {
        Put16(f->stage + 0xcac + i * 8U, (i & 1U) ? 8 : -8);
        Put16(f->stage + 0xcae + i * 8U, (i & 2U) ? 32 : 0);
    }
    for (Uint16 i = 0; i < 12; ++i)
        Put32(f->stage + 0xcfc + i * 4U, 0x80180cacU + i % 4U * 8U);
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 6, out));
    CHECK(SotnActors_SetServices(*out, (SotnDataView){f->exe, sizeof(f->exe)}, 1, false));
    CHECK(SotnActorLayout_OpenStage((SotnDataView){f->stage, sizeof(f->stage)}, 6, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *out, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 32}, 0,
        0));
    (*out)->slots[16] =
        (Actor){.slot = 16,
                .dynamic = true,
                .state = {.id = 0x1b, .generation = 1, .x = 128 * 65536, .y = 32 * 65536}};
    (*out)->player = (SotnPlayerState){.x = 400 * 65536, .y = 240 * 65536, .hp = 70, .hp_max = 70};
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
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[16];
    CHECK(ActorChapel(s, a) && a->state.step == 1 && a->state.primitive_count == 8 &&
          a->state.priority == 156 && a->state.hitbox_width == 56 && a->state.hitbox_y == 84);
    ActorPrimitive *anchor = ActorPrimitiveGet(s, a, 0), *rope = ActorPrimitiveGet(s, a, 1),
                   *body = ActorPrimitiveGet(s, a, 3), *clip = ActorPrimitiveGet(s, a, 6),
                   *restore = ActorPrimitiveGet(s, a, 7);
    CHECK(anchor && rope && body && clip && restore);
    CHECK(anchor->state.priority == 179 && anchor->state.palette == 0x76 &&
          anchor->state.offset_y[0] == -10 && anchor->state.v[2] == 24);
    CHECK(rope->state.screen_space && rope->state.offset_x[0] == 124 &&
          rope->state.offset_x[1] == 132 && rope->state.offset_y[0] == 32 &&
          rope->state.offset_y[2] == 46 && rope->state.v[2] == 79);
    CHECK(body->state.offset_x[0] == 120 && body->state.offset_y[0] == 60 &&
          body->state.offset_x[3] == 136 && body->state.offset_y[3] == 92);
    CHECK(clip->state.clip && !clip->state.active && clip->state.priority == 147 &&
          restore->state.clip && restore->state.priority == 149 && restore->state.clip_top == 20 &&
          restore->state.clip_bottom == 227);
    s->camera_x = 32;
    s->camera_y = 16;
    CHECK(ActorChapel(s, a) && rope->state.offset_x[0] == 92 && rope->state.offset_y[0] == 16 &&
          body->state.offset_y[0] == 44 && anchor->state.x == 128 * 65536);
    Close(f, s);
    return true;
}
static bool Interaction(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[16];
    s->player.x = 148 * 65536;
    s->player.y = 80 * 65536;
    s->player.contacts = 1;
    CHECK(ActorChapel(s, a) && a->bell.phase == 1 && a->bell.velocity == 10 * 65536 &&
          a->bell.ring_timer == 64 && s->combat.sound_count == 1 && s->combat.sounds[0] == 0x7bc);
    CHECK(ActorChapel(s, a) && a->bell.distance == 10 * 65536 && a->bell.velocity == 559104 &&
          a->bell.target == 20 * 65536 && a->bell.ring_timer == 63);
    s->player.x = 108 * 65536;
    CHECK(ActorChapel(s, a) && a->bell.target == -20 * 65536);
    a->bell.ring_timer = 1;
    s->player.contacts = 0;
    CHECK(ActorChapel(s, a) && a->bell.phase == 0 && !a->bell.ring_timer &&
          s->combat.sound_count == 1);
    s->player.contacts = 3;
    CHECK(ActorChapel(s, a) && a->bell.phase == 2 && a->bell.velocity == 8 * 65536 &&
          a->bell.ring_timer == 64 && s->combat.sound_count == 2);
    s->player.contacts = 2;
    CHECK(ActorChapel(s, a) && a->bell.phase == 2 && !a->bell.target);
    s->player.contacts = 0;
    CHECK(ActorChapel(s, a) && !a->bell.phase);
    a->bell.distance = 64 * 65536;
    a->bell.velocity = 10 * 65536;
    s->player.x = 400 * 65536;
    CHECK(ActorChapel(s, a) && a->bell.distance == 64 * 65536);
    a->bell.distance = 1000;
    a->bell.velocity = 100;
    CHECK(ActorChapel(s, a) && !a->bell.distance && !a->bell.velocity);
    Close(f, s);
    return true;
}
static bool Inside(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[16];
    s->player.x = 138 * 65536;
    s->player.y = 140 * 65536;
    CHECK(SotnActors_Tick(s, s->player, 0, 0) && s->motion.protect && s->motion.bell_inside &&
          (s->player.status & 0x100008U) == 0x100008U && s->control.timer == 2 &&
          !s->control.buttons);
    ActorPrimitive *clip = ActorPrimitiveGet(s, a, 6);
    CHECK(clip && clip->state.active && clip->state.clip_top == 180 &&
          clip->state.clip_bottom == 207);
    s->player = (SotnPlayerState){.x = 400 * 65536, .y = 240 * 65536, .hp = 70, .hp_max = 70};
    CHECK(SotnActors_Tick(s, s->player, 0, 0) && !clip->state.active && !s->motion.bell_inside &&
          !s->motion.protect && s->control.timer == 1);
    CHECK(SotnActors_Tick(s, s->player, 0, 0) && !s->control.timer);
    s->player = (SotnPlayerState){.x = 138 * 65536, .y = 140 * 65536, .status = 2};
    CHECK(SotnActors_Tick(s, s->player, 0, 0) && !s->control.timer && s->motion.bell_inside &&
          ActorPrimitiveGet(s, a, 0)->state.priority == 154);
    Close(f, s);
    return true;
}
static bool Spawning(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[16];
    a->state.id = 0x26;
    CHECK(ActorChapel(s, a) && s->slots[17].state.id == 0x1b &&
          s->slots[17].state.parameters == 1 && s->slots[17].state.x == 160 * 65536 &&
          s->slots[17].state.y == 40 * 65536 && s->slots[18].state.parameters == 0);
    CHECK(ActorChapel(s, &s->slots[17]) && s->slots[17].state.primitive_count == 10);
    const Uint32 generation = s->slots[17].state.generation;
    a->state.step = 0;
    a->state.generation = 2;
    CHECK(ActorChapel(s, a) && s->slots[17].state.generation != generation &&
          s->slots[17].enemy_state.owner_generation == 2 && !s->slots[17].state.primitive_count);
    const Uint32 kept = s->slots[17].state.generation;
    a->state.step = 0;
    s->slots[18].state.id = 0x35;
    CHECK(!ActorChapel(s, a) && s->slots[17].state.generation == kept &&
          s->slots[18].state.id == 0x35);
    s->effect_serial = 0x7ffffffeU;
    CHECK(!ActorChapel(s, a) && s->slots[17].state.generation == kept);
    Close(f, s);
    return true;
}
static bool ProjectionFailures(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    SotnProjectedPoint p[4] = {0}, saved[4] = {0};
    CHECK(SotnChapel_ProjectBell(s->executable, s->nz0, 0, 0, 128, 64, p) && p[0].x == 120 &&
          p[0].y == 64 && p[3].x == 136 && p[3].y == 96);
    CHECK(SotnChapel_ProjectBell(s->executable, s->nz0, 1, -1024, 128, 64, p) && p[0].x == 128 &&
          p[0].y == 72 && p[3].x == 160 && p[3].y == 56);
    SDL_memcpy(saved, p, sizeof(p));
    Put32(f->stage + 0xcfc, 0x80180cfd);
    CHECK(!SotnChapel_ProjectBell(s->executable, s->nz0, 0, 0, 0, 0, p) &&
          !SDL_memcmp(saved, p, sizeof(p)));
    Put32(f->stage + 0xcfc, 0x80180cac);
    Put16(f->stage + 0xcae, 129);
    CHECK(!SotnChapel_ProjectBell(s->executable, s->nz0, 0, 0, 0, 0, p));
    Put16(f->stage + 0xcae, 0);
    Put32(f->stage + 0xd4c, 512);
    CHECK(!SotnChapel_ProjectBell(s->executable, s->nz0, 0, 0, 0, 0, p));
    Close(f, s);
    CHECK(Open(f, &s));
    Actor *a = &s->slots[16];
    for (Uint16 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 7;
    CHECK(ActorChapel(s, a) && !a->state.step && !a->state.primitive_count && !a->remove);
    SDL_zero(s->primitives);
    CHECK(ActorChapel(s, a) && a->state.step == 1 && a->state.primitive_count == 8);
    a->bell.phase = 3;
    CHECK(!ActorChapel(s, a));
    a->bell.phase = 0;
    a->state.parameters = 2;
    CHECK(!ActorChapel(s, a));
    CHECK(!ActorChapelBell(NULL, a) && !ActorChapelBell(s, NULL));
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    const bool ok =
        f && Geometry(f) && Interaction(f) && Inside(f) && Spawning(f) && ProjectionFailures(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_CHAPEL_BELL: PASS");
    return 0;
}
