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
    Uint8 dra[0x1000], stage[0x4500], exe[0x23220], tiles[2048], types[2];
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
static void Animation(Fixture *f, size_t at, Uint8 frame, bool loop)
{
    f->stage[at] = 2;
    f->stage[at + 1] = frame;
    f->stage[at + 2] = (Uint8)(loop ? 0 : 255);
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 25);
    Put16(f->dra + 0x20c, 3);
    f->dra[0x222] = 8;
    f->dra[0x223] = 16;
    Put16(f->stage + 0x9e0, 0x8003);
    Put16(f->stage + 0x9e6, 0x2a);
    Put16(f->stage + 0x1b8e, 16);
    Put16(f->stage + 0x1b92, 2);
    Put16(f->stage + 0x1b94, 6);
    Put16(f->stage + 0x1b96, -2);
    Put16(f->stage + 0x1b98, -12);
    Put16(f->stage + 0x1b9e, 16);
    Put16(f->stage + 0x1ba0, 6);
    Put16(f->stage + 0x1ba4, -18);
    Put16(f->stage + 0x1ba6, 36);
    Put16(f->stage + 0x1ba8, 18);
    Put16(f->stage + 0x1baa, 24);
    for (Uint16 i = 0; i < 4; ++i)
        Put16(f->stage + 0x1bac + i * 2U, 2 + i);
    Animation(f, 0x1bb4, 2, true);
    Animation(f, 0x1bc0, 5, false);
    Animation(f, 0x1bd4, 11, false);
    Animation(f, 0x1bec, 14, true);
    Animation(f, 0x1bfc, 25, true);
    Animation(f, 0x1c08, 22, false);
    f->stage[0x1c38 + 11] = 2;
    f->stage[0x1c38 + 22] = 3;
    f->stage[0x1c30] = 249;
    f->stage[0x1c31] = 9;
    f->stage[0x1c32] = 6;
    f->stage[0x1c33] = 8;
    f->stage[0x1c34] = 13;
    f->stage[0x1c35] = 251;
    f->stage[0x1c36] = 9;
    f->stage[0x1c37] = 7;
    Put32(f->stage + 20, 0x80184000);
    Put32(f->stage + 0x400c, 0x80184100);
    for (Uint16 i = 0; i < 28; ++i)
        Put32(f->stage + 0x4100 + i * 4U, 0x80184170);
    Put16(f->stage + 0x4170, 2);
    for (Uint16 i = 0; i < 2; ++i)
    {
        Uint8 *d = f->stage + 0x4172 + i * 22U;
        Put16(d + 2, -12 + i * 5);
        Put16(d + 4, -24 + i * 9);
        Put16(d + 6, 80 - i * 10);
        Put16(d + 8, 30 + i * 4);
        Put16(d + 10, 123);
        Put16(d + 12, 7);
        Put16(d + 14, 9);
        Put16(d + 16, 17);
        Put16(d + 18, 39);
        Put16(d + 20, 46);
    }
    // Synthetic identity-like transform, deliberately distinct from the private SDK table.
    for (Uint32 i = 0; i < 4096; ++i)
    {
        Put16(f->exe + 0x1ea10 + i * 4U, (Sint32)(i % 257));
        Put16(f->exe + 0x1ea12 + i * 4U, 4096);
    }
    for (Uint32 i = 32 * 10; i < 32 * 32; ++i)
        f->tiles[i * 2] = 1;
    f->types[1] = 0x81;
    Put16(f->stage + 0x19d2, 77);
    Put32(f->stage + 0x1a14, 12288);
    f->stage[0x1a19] = 7;
    Put16(f->stage + 0x1a1e, 5);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 6, out));
    CHECK(SotnActors_SetServices(*out, (SotnDataView){f->exe, sizeof(f->exe)}, 0, false));
    (*out)->map =
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 32};
    (*out)->player = (SotnPlayerState){.x = 168 * 65536, .y = 144 * 65536};
    (*out)->slots[16] = (Actor){
        .slot = 16,
        .state = {.id = 0x27, .generation = 1, .x = 128 * 65536 + 123, .y = 144 * 65536 + 456}};
    return true;
}
static bool Movement(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[16], *w = &s->slots[17];
    CHECK(ActorChapel(s, a) && a->state.step == 2 && a->state.frame == 1 && w->state.id == 0x28 &&
          w->state.x == 128 * 65536 && !w->state.facing_left);
    a->corner.previous_left = a->corner.movement_left = false;
    CHECK(ActorChapel(s, a) && a->state.step == 4);
    Sint32 x = a->state.x;
    CHECK(ActorChapel(s, a));
    CHECK(a->state.x == x + 32768 && a->vx == 32768 && !a->state.facing_left);
    a->corner.timer = 0;
    CHECK(ActorChapel(s, a) && a->state.step == 6);
    for (Uint16 i = 0; i < 4 && a->state.step == 6; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(a->state.step == 4 && s->combat.sound_count == 1 && s->combat.sounds[0] == 0x66d);
    s->player.x = 96 * 65536;
    a->substep = 1;
    a->corner.timer = 0;
    CHECK(ActorChapel(s, a) && a->state.step == 3);
    for (Uint16 i = 0; i < 4 && a->state.step == 3; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(a->state.step == 5 && a->corner.previous_left && a->corner.movement_left &&
          !a->state.facing_left);
    f->types[1] = 3;
    x = a->state.x;
    CHECK(ActorChapel(s, a) && a->state.x == x && !a->vx);
    a->corner.timer = 0;
    CHECK(ActorChapel(s, a) && a->state.step == 7);
    for (Uint16 i = 0; i < 4 && !a->substep; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(a->substep == 1);
    CHECK(ActorChapel(s, a) && a->substep == 2 && !a->vx && a->state.hitbox_width == 17 &&
          a->state.hitbox_height == 9);
    CHECK(s->combat.sound_count == 3 && s->combat.sounds[1] == 0x6c7 &&
          s->combat.sounds[2] == 0x616);
    CHECK(ActorChapel(s, w) && w->state.x == a->state.x && w->state.y == a->state.y &&
          w->state.hitbox_x == -7 && w->state.hitbox_y == 9 && w->state.hitbox_width == 6);
    f->types[1] = 0x81;
    a->vx = -196608;
    a->state.y = 144 * 65536;
    a->pose = a->remaining = 0;
    CHECK(ActorChapel(s, a) && a->vx == -184320 && s->slots[96].state.id == 0x14);
    a->substep = 4;
    CHECK(ActorChapel(s, a) && a->state.step == 5 && !a->corner.movement_left);
    s->player.x = 300 * 65536;
    CHECK(ActorChapel(s, a));
    a->corner.timer = 0;
    CHECK(ActorChapel(s, a) && a->state.step == 3);
    a->state.frame = 28;
    CHECK(!ActorChapel(s, w));
    a->state.frame = 11;
    f->stage[0x1c32] = 255;
    CHECK(!ActorChapel(s, w));
    CHECK(!ActorChapelCornerGuard(NULL, a) && !ActorChapelCornerGuard(s, NULL));
    SotnActors_Destroy(s);
    return true;
}
static bool DeathOwnership(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[16], *w = &s->slots[17];
    CHECK(ActorChapel(s, a));
    Uint32 gen = w->state.generation;
    a->state.generation = 2;
    a->state.step = 0;
    CHECK(ActorChapel(s, a) && w->state.generation != gen && w->enemy_state.owner_generation == 2);
    a->state.frame = 11;
    a->state.texture_offset = 3;
    a->state.palette = 999;
    a->state.flags |= 0x100U;
    CHECK(ActorChapel(s, a) && a->state.step == 8 && a->substep == 1 && !a->state.frame &&
          !a->state.hitbox_state && w->remove && a->state.primitive_count == 4);
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    CHECK(p && p->state.width == 80 && p->state.page == 18 && p->state.palette == 0x2a &&
          p->state.u[0] == 136 && p->state.v[0] == 144);
    Sint32 x = p->state.x, y = p->state.y;
    CHECK(ActorChapel(s, a) && p->phase == 1 && p->state.x == x && p->state.y == y &&
          p->state.offset_x[0] == -40 && p->state.offset_y[0] == -15 && p->timer >= 7 &&
          p->timer <= 22);
    Sint32 vx = p->vx, vy = p->vy;
    Sint16 angle = p->corner_angle;
    CHECK(ActorChapel(s, a) && p->state.x == x + vx && p->state.y == y + vy && p->vy == vy + 6144 &&
          p->corner_angle == angle + (vx > 0 ? -32 : 32));
    for (Uint16 i = 0; i < 40 && !a->remove; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(a->remove && !p->state.active);
    w->remove = false;
    CHECK(ActorChapel(s, w) && !w->remove); // Parent still exists until slot retirement.
    a->state.generation = 3;
    CHECK(ActorChapel(s, w) && w->remove);
    SotnActors_Destroy(s);
    CHECK(Open(f, &s));
    a = &s->slots[16];
    w = &s->slots[17];
    CHECK(ActorChapel(s, a));
    w->state.id = 0x35;
    a->state.step = 0;
    CHECK(!ActorChapel(s, a) && w->state.id == 0x35);
    w->state.id = 0x28;
    a->state.step = 2;
    a->state.frame = 11;
    a->state.flags |= 0x100U;
    for (Uint16 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 900;
    CHECK(ActorChapel(s, a) && a->remove &&
          !w->remove); // Optional pool exhaustion; orphan retires later.
    a->state.generation = 0;
    CHECK(ActorChapel(s, w) && w->remove);
    SotnActors_Destroy(s);
    CHECK(Open(f, &s));
    a = &s->slots[16];
    CHECK(ActorChapel(s, a));
    Put16(f->stage + 0x4172 + 6, 513);
    a->state.frame = 11;
    a->state.flags |= 0x100U;
    CHECK(!ActorChapel(s, a));
    SotnActors_Destroy(s);
    return true;
}
static bool Dust(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[16];
    CHECK(ActorOuterDust(s, a, 1, 1, 12, 8, 5, 1));
    Actor *d = &s->slots[96];
    CHECK(d->state.id == 0x14 && d->state.scale_x == 77 && d->enemy_state.attack_index == 5 &&
          d->state.x == 140 * 65536);
    CHECK(ActorChapel(s, d) && d->state.frame == 7 && d->vy == 12288);
    for (Uint16 i = 0; i < 6; ++i)
        CHECK(ActorChapel(s, d));
    CHECK(d->remove && d->state.frame == 10);
    SotnActors_Destroy(s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    bool ok = f && Movement(f) && DeathOwnership(f) && Dust(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_CHAPEL_CORNER_GUARD: PASS");
    return 0;
}
