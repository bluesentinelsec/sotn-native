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
static void Animation(Fixture *f, size_t at, Uint16 count, Uint8 first)
{
    for (Uint16 i = 0; i < count; ++i)
    {
        f->stage[at + i * 2U] = 1;
        f->stage[at + i * 2U + 1] = (Uint8)(first + i % 7);
    }
    f->stage[at + count * 2U] = 255;
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 30);
    Put16(f->dra + 0x20c, 3);
    f->dra[0x222] = 8;
    f->dra[0x223] = 16;
    const size_t init[] = {0x9f8, 0xa04, 0xa10, 0xa1c};
    for (Uint16 i = 0; i < 4; ++i)
    {
        Put16(f->stage + init[i], 0x8005);
        Put16(f->stage + init[i] + 6, 0x30 + i);
    }
    Put16(f->stage + 0x1c56, 16);
    Put16(f->stage + 0x1c5a, 2);
    Put16(f->stage + 0x1c5c, 6);
    Put16(f->stage + 0x1c5e, -2);
    Put16(f->stage + 0x1c60, -12);
    for (Uint16 i = 0; i < 4; ++i)
    {
        Put16(f->stage + 0x1c8c + i * 4U, -7 + i * 3);
        Put16(f->stage + 0x1c8e + i * 4U, 5 + i * 4);
        Put16(f->stage + 0x1c9c + i * 2U, 16 + i * 2);
        Put16(f->stage + 0x1ca4 + i * 2U, 200 + i * 100);
    }
    for (Uint16 i = 0; i < 8; ++i)
        Put16(f->stage + 0x1d30 + i * 2U, 3 + i);
    Animation(f, 0x1cac, 18, 1);
    Animation(f, 0x1ce4, 18, 9);
    Animation(f, 0x1d18, 4, 1);
    Put32(f->stage + 20, 0x80184000);
    Put32(f->stage + 0x4014, 0x80184100);
    for (Uint16 i = 0; i < 28; ++i)
        Put32(f->stage + 0x4100 + i * 4U, 0x80184170);
    Put16(f->stage + 0x4170, 1);
    Uint8 *d = f->stage + 0x4172;
    Put16(d + 6, 24);
    Put16(d + 8, 20);
    Put16(d + 14, 11);
    Put16(d + 16, 22);
    Put16(d + 18, 33);
    Put16(d + 20, 44);
    for (Uint32 i = 0; i <= 1024; ++i)
        Put16(f->exe + 0x1dbcc + i * 2U, (Sint32)i * 4);
    for (Uint32 i = 320; i < 1024; ++i)
        f->tiles[i * 2] = 1;
    f->types[1] = 3;
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 6, out));
    CHECK(SotnActors_SetServices(*out, (SotnDataView){f->exe, sizeof(f->exe)}, 0, false));
    (*out)->map =
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 32};
    (*out)->player = (SotnPlayerState){.x = 96 * 65536, .y = 144 * 65536};
    (*out)->slots[16] = (Actor){
        .slot = 16,
        .state = {.id = 0x29, .generation = 1, .x = 128 * 65536 + 123, .y = 144 * 65536 + 456}};
    return true;
}
static bool Skulls(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[16], *top = &s->slots[17];
    CHECK(ActorChapel(s, a) && a->state.step == 2 && a->state.palette == 0x31 &&
          a->state.hitbox_y == 13 && top->state.id == 0x29 && top->state.parameters == 1 &&
          top->state.x == 128 * 65536);
    CHECK(ActorChapel(s, top) && top->state.step == 2 && top->state.frame == 9 &&
          top->state.hitbox_y == -14);
    CHECK(ActorChapel(s, a) && a->state.step == 3);
    CHECK(ActorChapel(s, a) && a->pillar.interval == 1 && a->pillar.timer == 2);
    CHECK(ActorChapel(s, a) && a->pillar.timer == 1);
    CHECK(ActorChapel(s, a) && a->state.step == 4);
    for (Uint16 i = 0; i < 19; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(a->state.step == 3 && s->slots[160].state.id == 0x2a &&
          !s->slots[160].state.facing_left && Pixel(s->slots[160].state.y) == 152);
    CHECK(ActorChapel(s, top) && top->state.step == 3);
    CHECK(ActorChapel(s, top) && top->pillar.timer == 3);
    CHECK(ActorChapel(s, top) && top->pillar.timer == 3);
    s->player.x = 176 * 65536;
    CHECK(ActorChapel(s, top) && top->pillar.timer == 2);
    s->player.x = 300 * 65536;
    CHECK(ActorChapel(s, top) && top->state.step == 3 && !top->substep);
    CHECK(ActorChapel(s, top) && top->pillar.interval == 2);
    a->state.flags |= 0x100U;
    CHECK(ActorChapel(s, a) && a->remove && top->pillar.bottom_dead &&
          s->slots[161].state.id == 0x2b && s->slots[161].state.parameters == 2 &&
          Pixel(s->slots[161].state.x) == 127 && Pixel(s->slots[161].state.y) == 157);
    CHECK(ActorChapel(s, top) && top->state.step == 10 && top->state.frame == 24 &&
          top->state.rotated && top->vx == -49152 && top->state.rotation == -32 &&
          !top->pillar.bottom_dead);
    top->state.flags |= 0x100U;
    CHECK(ActorChapel(s, top) && top->state.id == 2 && top->state.parameters == 1 &&
          !top->state.rotated && !top->state.step && top->state.frame == 24);
    SotnActors_Destroy(s);
    return true;
}
static bool Fire(Fixture *f, bool facing)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[16];
    a->state.id = 0x2a;
    a->state.facing_left = facing;
    CHECK(ActorChapel(s, a) && a->state.primitive_count == 16 && a->state.animation_set == 0 &&
          a->state.hitbox_x == -32 && a->state.hitbox_y == (facing ? -24 : 30));
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    CHECK(p && p->state.page == 19 && p->state.palette == 0x221 && p->state.u[0] == 11 &&
          p->state.v[3] == 44 && p->state.offset_x[0] == (facing ? 48 : -64) &&
          p->state.offset_y[2] == (facing ? -32 : 64) && !p->state.blend);
    for (Uint16 i = 0; i < 4; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(a->state.step == 2 && !a->state.hitbox_state);
    CHECK(ActorChapel(s, a) && a->state.step == 3 && a->pillar.timer == 47);
    for (Uint16 i = 0; i < 16; ++i)
    {
        ActorPrimitive *e = ActorPrimitiveGet(s, a, i);
        CHECK(e && e->state.active && !e->state.textured && e->state.width == 1 &&
              e->state.page == 32 && e->state.red == 222 && e->state.green == 158 &&
              e->state.blue == 134 && e->timer >= 1 && e->timer <= 8);
    }
    for (Uint16 i = 0; i < 47; ++i)
    {
        s->service_ticks++;
        CHECK(ActorChapel(s, a));
    }
    CHECK(a->remove && p->state.red == 128 && p->state.green == 64 && p->state.blue == 40);
    SotnActors_Destroy(s);
    CHECK(Open(f, &s));
    a = &s->slots[16];
    a->state.id = 0x2a;
    for (Uint16 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 900;
    CHECK(ActorChapel(s, a) && a->remove);
    SotnActors_Destroy(s);
    return true;
}
static bool Parts(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[16];
    a->state.id = 0x2b;
    a->state.parameters = 3;
    CHECK(ActorChapel(s, a) && a->state.frame == 21 && a->pillar.timer == 24 &&
          !a->state.hitbox_state);
    for (Uint16 i = 0; i < 24; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(a->state.step == 2);
    for (Uint16 i = 0; i < 100 && a->state.step == 2; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(a->state.step == 3 && a->pillar.timer == 14);
    for (Uint16 i = 0; i < 14; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(a->state.id == 2 && !a->state.step && !a->state.parameters && a->state.frame == 21);
    CHECK(!ActorChapelPillar(NULL, a) && !ActorChapelPillar(s, NULL));
    SotnActors_Destroy(s);
    CHECK(Open(f, &s));
    a = &s->slots[16];
    a->state.parameters = 256;
    CHECK(ActorChapel(s, a) && a->state.parameters == 0 && s->slots[17].state.id == 0x2c &&
          Pixel(s->slots[17].state.x) == 136 && Pixel(s->slots[17].state.y) == 136);
    a->state.generation = 2;
    a->state.step = 0;
    a->state.parameters = 0;
    CHECK(!ActorChapel(s, a) && s->slots[17].state.id == 0x2c);
    SotnActors_Destroy(s);
    CHECK(Open(f, &s));
    a = &s->slots[16];
    a->state.step = 3;
    a->pillar.interval = 8;
    CHECK(!ActorChapel(s, a));
    SotnActors_Destroy(s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    bool ok = f && Skulls(f) && Fire(f, false) && Fire(f, true) && Parts(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_CHAPEL_PILLAR: PASS");
    return 0;
}
