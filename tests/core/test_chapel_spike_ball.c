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
    Uint8 dra[0x1000], stage[0x2400], tiles[2048], types[2];
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
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 90);
    Put16(f->dra + 0x206, 29);
    Put16(f->dra + 0x20c, 3);
    Put16(f->stage + 0xa1c, 0x8005);
    Put16(f->stage + 0xa20, 0x4b);
    Put16(f->stage + 0xa22, 0x40);
    Put16(f->stage + 0x1c86, 14);
    Put16(f->stage + 0x1c88, 6);
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    for (Uint32 i = 320; i < 1024; ++i)
        f->tiles[i * 2] = 1;
    f->types[1] = 3;
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 6, out));
    CHECK(SotnActorLayout_OpenStage((SotnDataView){f->stage, sizeof(f->stage)}, 6, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *out, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 32}, 0,
        0));
    return true;
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    f->layout = NULL;
}
static bool Ball(SotnActors *s, Uint16 slot, Uint32 generation)
{
    s->slots[slot] = (Actor){
        .slot = slot,
        .dynamic = true,
        .state = {
            .id = 0x2c, .generation = generation, .x = 128 * 65536 + 123, .y = 144 * 65536 + 456}};
    return ActorChapel(s, &s->slots[slot]);
}
static bool Release(SotnActors *s, Uint16 slot)
{
    s->slots[slot].pillar.bottom_dead = true;
    return ActorChapel(s, &s->slots[slot]);
}
static bool Motion(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s) && Ball(s, 16, 1));
    Actor *a = &s->slots[16];
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    CHECK(a->state.step == 1 && a->state.frame == 22 && a->state.hitbox_width == 13 && p &&
          p->state.page == 18 && p->state.u[0] == 208 && p->state.v[3] == 208 &&
          p->state.palette == 0x42 && p->state.offset_x[1] == 17);
    CHECK(Release(s, 16) && a->state.step == 2 && a->state.hit_points == 32767 &&
          a->state.hitbox_state == 1 && a->vx == -49152 && a->vy == -65536 &&
          p->state.palette == 0x43);
    SotnPlayerState attack = {0};
    Uint32 group = 99;
    bool active = false;
    CHECK(ActorChapelBallAttack(s, 0, &attack, &group, &active) && active && group == 0 &&
          attack.x == 128 * 65536 && attack.attack.power == 29 && attack.attack.element == 0x8000 &&
          attack.attack.invincibility_frames == 16);
    s->service_ticks = 2;
    CHECK(ActorChapel(s, a) && a->state.rotation == -64 && a->state.palette == 0x40 &&
          p->state.palette == 0x42);
    for (Uint16 i = 0; i < 100 && a->state.step == 2; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(a->state.step == 3 && a->pillar.bounced && s->combat.sounds[0] == 0x761);
    Sint16 angle = a->state.rotation;
    CHECK(ActorChapel(s, a) && a->vx == -114688 && a->state.rotation == angle - 128);
    a->state.x = 289 * 65536;
    CHECK(ActorChapelBallAttack(s, 0, &attack, &group, &active) && !active &&
          s->chapel_ball[0].generation == 1);
    a->state.x = 128 * 65536;
    a->state.flags |= 0x100U;
    CHECK(ActorChapel(s, a) && a->remove && s->slots[160].state.id == 2 &&
          s->slots[160].state.parameters == 3);
    a->state.generation = 2;
    CHECK(ActorChapelBallAttack(s, 0, &attack, &group, &active) && !active &&
          !s->chapel_ball[0].generation);
    CHECK(!ActorChapelBallAttack(NULL, 0, &attack, &group, &active) &&
          !ActorChapelBallAttack(s, 15, &attack, &group, &active));
    CHECK(!ActorChapelSpikeBall(NULL, a) && !ActorChapelSpikeBall(s, NULL));
    Close(f, s);
    return true;
}
static bool Groups(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    for (Uint16 i = 0; i < 16; ++i)
        CHECK(Ball(s, (Uint16)(16 + i), i + 1) && Release(s, (Uint16)(16 + i)));
    Uint16 groups[4] = {0};
    for (Uint16 i = 0; i < 15; ++i)
    {
        CHECK(s->chapel_ball[i].generation == i + 1 && s->chapel_ball[i].group < 4);
        ++groups[s->chapel_ball[i].group];
    }
    CHECK(groups[0] == 4 && groups[1] == 4 && groups[2] == 4 && groups[3] == 3);
    s->slots[16].state.generation = 100;
    CHECK(Ball(s, 32, 99) && Release(s, 32) && s->chapel_ball[0].slot == 32 &&
          s->chapel_ball[0].group == 0);
    s->chapel_ball[0].group = 4;
    SotnPlayerState out = {0};
    Uint32 group = 0;
    bool active = false;
    CHECK(!ActorChapelBallAttack(s, 0, &out, &group, &active));
    Close(f, s);
    CHECK(Open(f, &s));
    for (Uint16 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 900;
    CHECK(Ball(s, 16, 1) && s->slots[16].remove);
    Close(f, s);
    return true;
}
static bool Damage(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s) && Ball(s, 16, 1) && Release(s, 16));
    Actor *ball = &s->slots[16];
    ball->stun = 200;
    ball->state.flags = 0;
    Actor *target = &s->slots[40];
    *target = (Actor){.slot = 40,
                      .dynamic = true,
                      .stun = 200,
                      .enemy = {.defense = 3},
                      .state = {.id = 0x35,
                                .step = 1,
                                .generation = 2,
                                .x = 128 * 65536,
                                .y = 144 * 65536,
                                .hit_points = 90,
                                .hitbox_state = 2,
                                .hitbox_width = 8,
                                .hitbox_height = 16}};
    SotnPlayerState player = {.x = 400 * 65536, .y = 144 * 65536, .invulnerable = true};
    CHECK(SotnActors_Tick(s, player, 0, 0) && target->state.hit_points == 64 &&
          target->cooldown[18] == 16 && target->stun == 3);
    target->stun = 200;
    for (Uint16 i = 0; i < 15; ++i)
        CHECK(SotnActors_Tick(s, player, 0, 0));
    CHECK(target->state.hit_points == 64 && target->cooldown[18] == 1);
    CHECK(SotnActors_Tick(s, player, 0, 0) && target->state.hit_points == 38 &&
          target->cooldown[18] == 16);
    target->stun = 200;
    ball->state.generation = 100;
    CHECK(Ball(s, 17, 3) && Release(s, 17));
    s->slots[17].stun = 200;
    s->slots[17].state.flags = 0;
    CHECK(SotnActors_Tick(s, player, 0, 0) && target->state.hit_points == 38 &&
          target->cooldown[18] == 15); // Reused group does not reset original invulnerability.
    target->enemy.immunes = 0x8000;
    target->cooldown[18] = 0;
    CHECK(SotnActors_Tick(s, player, 0, 0) && target->state.hit_points == 38);
    target->enemy.immunes = 0;
    target->enemy.absorbs = 0x8000;
    target->cooldown[18] = 0;
    CHECK(SotnActors_Tick(s, player, 0, 0) && target->state.hit_points == 67);
    CHECK(SotnActors_Enter(s, f->layout, s->map, 0, 0) && !s->chapel_ball[0].generation);
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    bool ok = f && Motion(f) && Groups(f) && Damage(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_CHAPEL_SPIKE_BALL: PASS");
    return 0;
}
