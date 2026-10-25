#include "actors_internal.h"
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
    Uint8 stage[0x4000], dra[0x1000], exe[0x23220], tiles[16 * 16 * 2], types[2];
} Fixture;
static void Put16(Uint8 *p, Uint16 n)
{
    p[0] = (Uint8)n;
    p[1] = (Uint8)(n >> 8);
}
static void Put32(Uint8 *p, Uint32 n)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(n >> (i * 8U));
}
static bool Open(Fixture *f, SotnActors **s)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    for (Uint32 i = 14; i < 16; ++i)
    {
        Uint8 *p = f->dra + 0x200 + i * 40;
        Put16(p + 4, (Uint16)(i + 10));
        Put16(p + 12, 3);
        p[34] = 8;
        p[35] = 12;
    }
    Put16(f->stage + 0x878 + 8, 14);
    Put16(f->stage + 0x884 + 8, 15);
    Put16(f->stage + 0x282c, 255);
    Put16(f->stage + 0x2808, 255);
    f->types[1] = 3;
    const size_t streams[] = {0x283c, 0x2848, 0x2854, 0x2868, 0x2874, 0x2884, 0x288c};
    for (Uint32 i = 0; i < SDL_arraysize(streams); ++i)
    {
        f->stage[streams[i]] = 1;
        f->stage[streams[i] + 1] = 7;
        f->stage[streams[i] + 2] = 255;
    }
    for (Uint32 i = 0; i < 4; ++i)
        Put16(f->stage + 0x2898 + i * 2, 3);
    for (Uint32 i = 0; i < 24; ++i)
        Put16(f->stage + 0x28a0 + i * 2, i & 1U ? 1 : (Uint16)-1);
    for (Uint32 i = 0; i < 4096; ++i)
        Put16(f->exe + 0x1ea12 + i * 4, 4096);
    Put16(f->exe + 0x1ea10 + 512 * 4, 2000);
    Put16(f->exe + 0x1ea12 + 512 * 4, 3000);
    Put16(f->exe + 0x1dbcc + 1024 * 2, 4096);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 2, s));
    (*s)->executable = (SotnDataView){f->exe, sizeof(f->exe)};
    (*s)->map =
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 16, 16};
    (*s)->player = (SotnPlayerState){.x = 160 * 65536, .y = 128 * 65536};
    (*s)->slots[64] = (Actor){.slot = 64,
                              .state = {.id = 0x1b,
                                        .generation = 1,
                                        .x = 128 * 65536 + 123,
                                        .y = 128 * 65536 + 456,
                                        .facing_left = true}};
    return true;
}
static bool Body(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[64], *b = &s->slots[65];
    CHECK(ActorLibraryFlying(s, a) && a->state.priority == 134 && a->state.step == 1 &&
          a->state.hitbox_y == 10);
    CHECK(b->state.id == 0x1c && !b->state.facing_left && b->state.x == 128 * 65536 &&
          b->state.y == 128 * 65536);
    CHECK(a->flying.partner_generation == b->state.generation && b->flying.partner_generation == 1);
    CHECK(ActorLibraryFlying(s, b) && b->state.priority == 136 && b->state.hitbox_y == -14);
    b->state.x += 77;
    b->state.y += 99;
    a->state.x += 65536;
    a->state.y += 65536;
    CHECK(ActorLibraryFlying(s, b) && b->state.x == 129 * 65536 + 77 &&
          b->state.y == 129 * 65536 + 99);
    a->hit_flags = 1;
    a->state.flags |= 0x100;
    CHECK(ActorLibraryFlying(s, a) && a->state.step == 3 && a->flying.separated &&
          a->state.hit_points == 25 && !(a->state.flags & 0x100));
    CHECK(s->combat.sound_count == 3 && s->combat.sounds[0] == 0x714 &&
          s->combat.sounds[1] == 0x714 && s->combat.sounds[2] == 0x716);
    CHECK(b->state.step == 2 && b->state.hitbox_state == 0);
    CHECK(s->slots[160].state.id == 0x1e && !s->slots[160].state.facing_left &&
          s->slots[160].state.x == 133 * 65536);
    CHECK(s->slots[161].state.id == 0x1e && s->slots[161].state.facing_left &&
          s->slots[161].state.x == 125 * 65536);
    a->hit_flags = 0;
    CHECK(ActorLibraryFlying(s, a) && a->state.step == 5 && a->state.hit_points == 24 &&
          a->state.hitbox_state == 3);
    CHECK(b->state.step == 3 && b->state.y == 103 * 65536 + 99 && b->state.hitbox_y == 0);
    CHECK(ActorLibraryFlying(s, b) && b->state.hitbox_state == 3 && b->vy == -51200 &&
          b->state.step == 3);
    CHECK(ActorLibraryFlying(s, b) && b->state.step == 6);
    for (Uint16 i = 0; i < 50 && b->state.step != 7; ++i)
        CHECK(ActorLibraryFlying(s, b));
    CHECK(b->state.step == 7 && b->vy == 0);
    CHECK(ActorLibraryFlying(s, a) && a->state.step == 5);
    CHECK(ActorLibraryFlying(s, a) && a->state.step == 6);
    CHECK(ActorLibraryFlying(s, a) && a->substep == 1 && a->flying.timer == 3);
    a->state.flags |= 0x100;
    CHECK(ActorLibraryFlying(s, a) && a->state.step == 7 && a->remove &&
          s->combat.sounds[s->combat.sound_count - 1] == 0x694);
    CHECK(!b->remove && b->state.step == 7);
    CHECK(s->slots[162].state.id == 2 && s->slots[162].state.parameters == 2 &&
          s->slots[162].state.x == Pixel(a->state.x) * 65536 &&
          s->slots[162].state.y == Pixel(a->state.y) * 65536);
    // Death direction uses fixed-point vectors, not the bounded pixel-only angle API.
    b->state.flags |= 0x100;
    CHECK(ActorLibraryFlying(s, b) && b->state.step == 5 && b->substep == 1 &&
          b->flying.timer == 2);
    CHECK(s->combat.sounds[s->combat.sound_count - 2] == 0x715 &&
          s->combat.sounds[s->combat.sound_count - 1] == 0x691);
    CHECK(ActorLibraryFlying(s, b) && b->flying.timer == 1);
    CHECK(ActorLibraryFlying(s, b) && b->flying.timer == 6 && s->slots[96].state.id == 2 &&
          s->slots[96].state.rotated);
    SotnActors_Destroy(s);
    return true;
}
static bool UpperHit(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[64], *b = &s->slots[65];
    CHECK(ActorLibraryFlying(s, a) && ActorLibraryFlying(s, b));
    b->hit_flags = 1;
    CHECK(ActorLibraryFlying(s, b) && b->state.step == 2 && a->state.step == 4 &&
          b->state.hit_points == 24);
    CHECK(ActorLibraryFlying(s, a) && a->flying.separated && b->state.priority == 128 &&
          a->flying.timer == 8);
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(ActorLibraryFlying(s, a));
    CHECK(a->substep == 2 && a->flying.timer == 2);
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(ActorLibraryFlying(s, a));
    CHECK(a->state.step == 5 && b->state.generation == 0 && a->flying.partner_generation == 0);
    CHECK(s->slots[160].state.id == 2 && s->slots[161].state.id == 2 &&
          s->slots[161].state.y == Pixel(a->state.y) * 65536 - 8 * 65536);
    SotnActors_Destroy(s);
    CHECK(Open(f, &s));
    a = &s->slots[64];
    b = &s->slots[65];
    CHECK(ActorLibraryFlying(s, a) && ActorLibraryFlying(s, b));
    ++a->state.generation;
    CHECK(ActorLibraryFlying(s, b) && b->remove);
    b->remove = false;
    b->state.step = 2;
    CHECK(ActorLibraryFlying(s, b) && !b->remove); // Detached torso no longer follows its owner.
    SotnActors_Destroy(s);
    return true;
}
static bool Flight(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[64];
    a->state.id = 0x1c;
    a->state.step = 7;
    a->flying.hovering = true;
    s->service_ticks = 1;
    CHECK(ActorLibraryFlying(s, a) && a->flying.timer == 96 && a->vx == 32768 &&
          a->flying.hover == 0);
    s->service_ticks = 3;
    CHECK(ActorLibraryFlying(s, a) && a->flying.hover == 1 && a->flying.timer == 95);
    a->flying.hover = 23;
    a->flying.hovering = true;
    CHECK(ActorLibraryFlying(s, a) && a->flying.hover == 0 && !a->flying.hovering);
    a->state.step = 8;
    a->substep = 0;
    a->flying.hovering = false;
    s->player.x = a->state.x + 16 * 65536;
    s->player.y = a->state.y;
    Put16(f->stage + 0x210c + 64 * 2, 3000);
    Put16(f->stage + 0x210c, 1000);
    CHECK(ActorLibraryFlying(s, a) && a->flying.timer == 64 && a->vx == 18000 && a->vy == 6000);
    a->flying.timer = 1;
    CHECK(ActorLibraryFlying(s, a) && a->state.step == 7 && a->vy == 0 && a->substep == 0);
    a->state.step = 8;
    a->substep = 0;
    s->player.y = a->state.y;
    Put16(f->stage + 0x210c + 64 * 2, 5000);
    CHECK(!ActorLibraryFlying(s, a));
    a->state.step = 7;
    a->substep = 1;
    a->flying.timer = 2;
    a->flying.hovering = false;
    a->state.x = SDL_MAX_SINT32;
    a->vx = 32768;
    CHECK(!ActorLibraryFlying(s, a));
    CHECK(!ActorLibraryFlying(NULL, a) && !ActorLibraryFlying(s, NULL));
    SotnActors_Destroy(s);
    return true;
}
static bool Blood(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[64];
    a->state.id = 0x1e;
    a->state.facing_left = false;
    for (Uint16 i = 0; i < 2; ++i)
    {
        const size_t at = i ? 0x27e4 : 0x27cc;
        f->stage[at] = 2;
        Put16(f->stage + at + 1, 0x12ff);
        Put16(f->stage + at + 3, 0x0804);
        f->stage[at + 5] = 255;
    }
    CHECK(ActorLibraryBlood(s, a) && a->state.step == 1);
    CHECK(ActorLibraryBlood(s, a) && a->state.primitive_count == 8 && a->state.step == 2);
    CHECK(ActorLibraryBlood(s, a) && a->state.step == 3);
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0), *big = ActorPrimitiveGet(s, a, 2);
    CHECK(p && big && !p->state.active && p->state.priority == 138 && p->state.modulate &&
          !p->state.blend);
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(!ActorPrimitiveGet(s, a, i)->state.active);
    CHECK(p->state.x == 124 * 65536 && big->state.x == 128 * 65536);
    s->camera_x = 3;
    s->camera_y = 5;
    s->service_ticks = 1;
    CHECK(ActorLibraryBlood(s, a) && p->state.active && big->state.active);
    CHECK(p->state.x == 124 * 65536 + 24571 && p->state.y == 128 * 65536 + 40957);
    CHECK(p->state.offset_x[0] == -10 && p->state.offset_y[0] == -2 && p->state.red == 96 &&
          p->blood.brightness == 94);
    CHECK(p->blood.width == 17 && p->blood.height == 17 && p->state.palette == 0x1b7);
    CHECK(p->state.u[0] == 255 && p->state.v[0] == 0x12 && p->state.u[1] == 3 &&
          p->state.v[1] == 0x13 && p->state.u[3] == 3 && p->state.v[3] == 0x1b);
    CHECK(p->blood.pose == 1 && p->blood.remaining == 1);
    CHECK(ActorLibraryBlood(s, a) && p->blood.remaining == 0 && p->state.active);
    CHECK(ActorLibraryBlood(s, a) && !p->state.active && p->blood.ended && !big->state.active);
    const Sint32 old_x = p->state.x;
    s->camera_x += 20;
    CHECK(ActorLibraryBlood(s, a) && p->state.x == old_x);
    a->timer = 129;
    CHECK(ActorLibraryBlood(s, a) && a->remove);
    ActorPrimitivesClear(s, a);
    CHECK(p->owner == 0);
    SotnActors_Destroy(s);
    CHECK(Open(f, &s));
    a = &s->slots[64];
    a->state.id = 0x1e;
    a->state.facing_left = false;
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 999;
    CHECK(ActorLibraryBlood(s, a) && ActorLibraryBlood(s, a) && a->remove &&
          a->state.primitive_count == 0);
    SotnActors_Destroy(s);
    return true;
}
static bool Drips(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[64];
    a->state.id = 0x1d;
    CHECK(ActorLibraryBlood(s, a) && a->state.step == 1 && a->state.primitive_count == 1);
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    CHECK(p && p->state.line && p->state.blend && p->state.line_blend_mode == 1 &&
          p->state.red == 255 && p->state.line_red == 32);
    for (Uint16 i = 0; i < 20; ++i)
        CHECK(ActorLibraryBlood(s, a));
    CHECK(a->vy == 122880 && p->blood.tail_y == Pixel(a->state.y - a->vy) - 8);
    for (Uint32 i = 0; i < 16 * 16; ++i)
        Put16(f->tiles + i * 2, 1);
    const Sint32 head = Pixel(a->state.y), tail = p->blood.tail_y;
    CHECK(ActorLibraryBlood(s, a) && a->substep == 1 && p->blood.tail_y == tail + 2 &&
          Pixel(a->state.y) == head);
    for (Uint16 i = 0; i < 20 && !a->remove; ++i)
        CHECK(ActorLibraryBlood(s, a));
    CHECK(a->remove);
    bool out = true;
    CHECK(!ActorLibraryBlocked(s, a, sizeof(f->stage) - 2, false, &out) && out);
    SotnActors_Destroy(s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    const bool ok = f && Body(f) && UpperHit(f) && Flight(f) && Blood(f) && Drips(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
