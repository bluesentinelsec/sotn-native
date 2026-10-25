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
    Uint8 dra[0x400], stage[0x4000], tiles[32 * 16 * 2], types[2];
    SotnActorLayout *layout;
} Fixture;
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8));
}
static void Anim(Fixture *f, size_t at, Uint8 first, Uint8 count)
{
    for (Uint8 i = 0; i < count; ++i)
    {
        f->stage[at + i * 2] = 1;
        f->stage[at + i * 2 + 1] = (Uint8)(first + i);
    }
    f->stage[at + count * 2] = 255;
}
static bool Open(Fixture *f, SotnActors **s)
{
    SDL_memset(f, 0, sizeof(*f));
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 40);
    Put16(f->dra + 0x206, 6);
    Put16(f->dra + 0x20c, 3);
    f->dra[0x222] = 8;
    f->dra[0x223] = 16;
    Put32(f->dra + 0x224, 0x08401000);
    Put32(f->stage + 28, 0x80180040);
    for (Uint32 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    const size_t initializers[] = {0xb24, 0xb30, 0xb3c};
    for (Uint32 i = 0; i < SDL_arraysize(initializers); ++i)
    {
        Put16(f->stage + initializers[i], 0x8001);
        Put16(f->stage + initializers[i] + 2, 1);
    }
    const size_t ground[] = {0x3230, 0x332c, 0x337c, 0x338c, 0x339c, 0x33ac};
    for (Uint32 i = 0; i < SDL_arraysize(ground); ++i)
        Put16(f->stage + ground[i] + 2, 16);
    Put16(f->stage + 0x3242, 16);
    Anim(f, 0x3268, 1, 3);
    Anim(f, 0x3280, 7, 6);
    Anim(f, 0x3294, 1, 3);
    Anim(f, 0x329c, 15, 3);
    Anim(f, 0x32b4, 15, 6);
    Anim(f, 0x32c4, 29, 7);
    Anim(f, 0x32e8, 1, 3);
    Anim(f, 0x3324, 40, 3);
    for (Uint32 i = 0; i < 5; ++i)
    {
        Put16(f->stage + 0x3310 + i * 4, 8);
        Put16(f->stage + 0x3312 + i * 4, 12);
    }
    for (Uint32 i = 0; i < 4; ++i)
    {
        Put32(f->stage + 0x334c + i * 4, 32768);
        Put32(f->stage + 0x335c + i * 4, (Uint32)-65536);
        Put16(f->stage + 0x336c + i * 2, (Uint16)(40 + i));
        Put16(f->stage + 0x3374 + i * 2, 12);
    }
    for (Uint32 i = 0; i < 5; ++i)
        Put32(f->stage + 0x2324 + i * 4, 0x80180380);
    Anim(f, 0x380, 1, 1);
    f->types[1] = 3;
    for (Uint32 y = 12; y < 16; ++y)
        for (Uint32 x = 0; x < 32; ++x)
            Put16(f->tiles + (y * 32 + x) * 2, 1);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 1, s));
    CHECK(SotnActorLayout_OpenStage((*s)->nz0, 1, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *s, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16}, 0,
        0));
    return true;
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
}
static Actor *Spawn(SotnActors *s, Uint16 variant)
{
    Actor *a = &s->slots[16];
    *a = (Actor){.state = {.id = 0x53,
                           .generation = 100,
                           .parameters = variant,
                           .x = 120 * 65536,
                           .y = 176 * 65536},
                 .slot = 16,
                 .dynamic = true};
    return a;
}
static bool Encounters(Fixture *f)
{
    for (Uint16 variant = 0; variant < 3; ++variant)
    {
        SotnActors *s = NULL;
        CHECK(Open(f, &s));
        Actor *a = Spawn(s, variant);
        SotnPlayerState player = {
            .x = 140 * 65536, .y = 170 * 65536, .hp = 70, .invulnerable = true};
        bool thrown = false, punch = false, pieces = false;
        for (Uint32 t = 0; t < 240; ++t)
        {
            if (t == 80)
                player.x = 100 * 65536;
            CHECK(SotnActors_Tick(s, player, 0, 0));
            punch = punch || a->state.step == 4 || a->state.step == 5;
            for (Uint32 i = 96; i < 128; ++i)
                thrown = thrown || (s->slots[i].state.id == 0x55 && s->slots[i].state.step == 3);
            for (Uint32 i = 160; i < 192; ++i)
                pieces = pieces || (s->slots[i].state.id == 0x56 && s->slots[i].state.generation);
        }
        CHECK(punch);
        CHECK(!variant || (thrown && pieces));
        CHECK(a->state.parameters == 0);
        a->state.flags = (a->state.flags | 0x10c100) & ~0x20000000U;
        for (Uint32 t = 0; t < 25; ++t)
            CHECK(SotnActors_Tick(s, player, 0, 0));
        CHECK(!s->slots[16].state.generation && !s->slots[17].state.generation);
        Close(f, s);
    }
    return true;
}
static bool Ownership(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = Spawn(s, 1);
    s->player = (SotnPlayerState){.x = 250 * 65536, .hp = 70};
    CHECK(ActorSkeletonApe(s, a));
    Actor *punch = &s->slots[17], *barrel = &s->slots[a->ape.barrel_slot];
    CHECK(ActorSkeletonApe(s, punch) && ActorSkeletonApe(s, barrel));
    ++a->state.generation;
    CHECK(ActorSkeletonApe(s, punch) && punch->remove);
    CHECK(ActorSkeletonApe(s, barrel) && barrel->remove);
    Close(f, s);
    CHECK(Open(f, &s));
    a = Spawn(s, 2);
    for (Uint32 i = 96; i < 128; ++i)
        s->slots[i].state.generation = 1;
    CHECK(ActorSkeletonApe(s, a) && a->state.parameters == 0 && a->state.hitbox_state != 0);
    Close(f, s);
    CHECK(Open(f, &s));
    a = Spawn(s, 3);
    CHECK(!ActorSkeletonApe(s, a));
    a->state.parameters = 0;
    s->slots[17].state.generation = 1;
    CHECK(!ActorSkeletonApe(s, a));
    Close(f, s);
    CHECK(Open(f, &s));
    a = Spawn(s, 0);
    CHECK(ActorSkeletonApe(s, a));
    punch = &s->slots[17];
    a->state.frame = 14;
    CHECK(!ActorSkeletonApe(s, punch));
    a->state.frame = 10;
    Put16(f->stage + 0x3314, 100);
    CHECK(!ActorSkeletonApe(s, punch));
    Actor piece = {.state = {.id = 0x56, .parameters = 4}};
    CHECK(!ActorSkeletonApe(s, &piece));
    piece.state.parameters = 0;
    Put32(f->stage + 0x334c, 0x7fffffff);
    CHECK(!ActorSkeletonApe(s, &piece));
    Close(f, s);
    return true;
}
static bool PhysicsAndDamage(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor barrel = {
        .state = {.id = 0x55, .step = 3, .x = 200 * 65536, .y = 176 * 65536, .facing_left = true},
        .vx = 2 * 65536,
        .vy = 65536};
    CHECK(ActorSkeletonApe(s, &barrel));
    CHECK(barrel.ape.bounces == 1 && barrel.vy == -32768 && barrel.vx == 2 * 65536);
    for (Uint32 i = 0; i < 2; ++i)
    {
        barrel.state.y = 176 * 65536;
        barrel.vy = 65536;
        CHECK(ActorSkeletonApe(s, &barrel));
    }
    CHECK(barrel.state.step == 4 && barrel.ape.bounces == 3);
    Actor piece = {.state = {.id = 0x56, .x = 200 * 65536, .y = 176 * 65536}};
    CHECK(ActorSkeletonApe(s, &piece));
    piece.vy = 65536;
    CHECK(ActorSkeletonApe(s, &piece));
    CHECK(piece.vy == -32768 && piece.ape.speed == 16384);
    Actor *body = Spawn(s, 0);
    s->player = (SotnPlayerState){.x = 220 * 65536, .hp = 70};
    CHECK(ActorSkeletonApe(s, body));
    const SotnPlayerState player = {.x = 120 * 65536,
                                    .y = 176 * 65536,
                                    .hp = 70,
                                    .animation = {.hitbox_width = 8, .hitbox_height = 20},
                                    .attack = {.active = true,
                                               .power = 100,
                                               .element = 32,
                                               .width = 24,
                                               .height = 24,
                                               .hit_type = 2,
                                               .invincibility_frames = 8}};
    CHECK(SotnActors_Tick(s, player, 0, 0));
    CHECK(body->state.hit_points <= 0 && body->state.step == 7);
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    bool ok = Encounters(f) && Ownership(f) && PhysicsAndDamage(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
