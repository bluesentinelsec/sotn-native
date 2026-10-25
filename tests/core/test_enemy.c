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
    Uint8 dra[0x1000], nz0[0x3000], tiles[32 * 16 * 2], types[2];
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
static bool Open(Fixture *f, Uint16 id, SotnActors **s, SotnActorLayout **layout)
{
    SDL_memset(f, 0, sizeof(*f));
    Put32(f->nz0 + 28, 0x80180040);
    for (Uint32 i = 0; i < 106; ++i)
        Put32(f->nz0 + 0x40 + i * 4, 0x80180300);
    Put16(f->nz0 + 0x300, 0xfffe);
    Put16(f->nz0 + 0x302, 0xfffe);
    Put16(f->nz0 + 0x30a, 120);
    Put16(f->nz0 + 0x30c, 140);
    Put16(f->nz0 + 0x30e, id);
    Put16(f->nz0 + 0x314, 0xffff);
    Put16(f->nz0 + 0x316, 0xffff);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 10);
    Put16(f->dra + 0x206, 5);
    Put16(f->dra + 0x20c, 3);
    f->dra[0x222] = 8;
    f->dra[0x223] = 18;
    Put32(f->dra + 0x224, 0xa8001400);
    for (size_t i = 0xc40; i <= 0xca0; i += 12)
    {
        Put16(f->nz0 + i, 0x8001);
        Put16(f->nz0 + i + 2, 1);
    }
    Put16(f->nz0 + 0xca0 + 8, 1);
    Put16(f->dra + 0x22c, 1);
    Put16(f->dra + 0x22e, 5);
    Put16(f->dra + 0x234, 3);
    f->dra[0x24a] = 4;
    f->dra[0x24b] = 4;
    Put32(f->dra + 0x24c, 0xa8000400);
    for (Uint32 i = 0; i < 5; ++i)
        Put32(f->nz0 + 0x1e54 + i * 4, 0x80180700);
    f->nz0[0x700] = 1;
    f->nz0[0x701] = 1;
    f->nz0[0x702] = 255;
    const size_t walks[] = {0x2090, 0x20a0, 0x23dc, 0x23ec, 0x2610};
    for (size_t i = 0; i < SDL_arraysize(walks); ++i)
    {
        f->nz0[walks[i]] = 2;
        f->nz0[walks[i] + 1] = 1;
    }
    const size_t ends[] = {0x20b0, 0x20cc, 0x20d8, 0x23fc, 0x240c, 0x2418, 0x2638, 0x2654, 0x2670};
    for (size_t i = 0; i < SDL_arraysize(ends); ++i)
    {
        f->nz0[ends[i]] = 1;
        f->nz0[ends[i] + 1] = 1;
        f->nz0[ends[i] + 2] = 255;
    }
    f->nz0[0x20b0 + 1] = 12;
    f->nz0[0x23fc + 1] = 10;
    const size_t grounds[] = {0x215c, 0x24a8, 0x2694};
    for (size_t i = 0; i < SDL_arraysize(grounds); ++i)
    {
        Put16(f->nz0 + grounds[i] + 2, 20);
        Put16(f->nz0 + grounds[i] + 6, 4);
        Put16(f->nz0 + grounds[i] + 8, 8);
        Put16(f->nz0 + grounds[i] + 10, (Uint16)-4);
        Put16(f->nz0 + grounds[i] + 12, (Uint16)-16);
    }
    const size_t specials[] = {0x216c, 0x24b8};
    for (size_t i = 0; i < SDL_arraysize(specials); ++i)
    {
        Put16(f->nz0 + specials[i] + 2, 20);
        Put16(f->nz0 + specials[i] + 4, 8);
    }
    const size_t fields[] = {0x2174, 0x24c0};
    for (size_t i = 0; i < SDL_arraysize(fields); ++i)
    {
        Put16(f->nz0 + fields[i], (Uint16)-12);
        Put16(f->nz0 + fields[i] + 2, 16);
        Put16(f->nz0 + fields[i] + 6, (Uint16)-16);
        Put16(f->nz0 + fields[i] + 10, (Uint16)-16);
    }
    for (Uint32 i = 0; i < 8; ++i)
    {
        f->nz0[0x2480 + i] = 9;
        f->nz0[0x2154 + i] = 9;
        Put32(f->nz0 + 0x2488 + i * 4, (Uint32)-65536);
    }
    for (Uint32 i = 0; i < 7; ++i)
    {
        f->nz0[0x2430 + i] = 8;
        f->nz0[0x20f4 + i] = 8;
        Put32(f->nz0 + 0x2438 + i * 4, 65536);
        Put32(f->nz0 + 0x20fc + i * 4, 65536);
    }
    Put16(f->nz0 + 0x26ac, 10);
    Put16(f->nz0 + 0x26ae, 28);
    Put16(f->nz0 + 0x26b0, 10);
    Put16(f->nz0 + 0x26b2, 14);
    Put16(f->nz0 + 0x26b4, 255);
    f->types[1] = 3;
    for (Uint32 y = 10; y < 16; ++y)
        for (Uint32 x = 0; x < 32; ++x)
            f->tiles[(y * 32 + x) * 2] = 1;
    const SotnCollisionMap map = {
        {f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16};
    CHECK(SotnActorLayout_Open((SotnDataView){f->nz0, sizeof(f->nz0)}, 0, layout));
    CHECK(SotnActors_Create((SotnDataView){f->dra, sizeof(f->dra)},
                            (SotnDataView){f->nz0, sizeof(f->nz0)}, s));
    CHECK(SotnActors_Enter(*s, *layout, map, 0, 0));
    return true;
}
static bool Skeletons(Fixture *f, Uint16 id)
{
    SotnActors *s = NULL;
    SotnActorLayout *layout = NULL;
    CHECK(Open(f, id, &s, &layout));
    SotnPlayerState p = {.x = 180 * 65536,
                         .y = 135 * 65536,
                         .hp = 70,
                         .hp_max = 70,
                         .animation = {.hitbox_width = 8, .hitbox_height = 16}};
    bool attacking = false, projectile = false;
    for (Uint32 i = 0; i < 150; ++i)
    {
        if (id == 0x27 && i > 90)
            p.x = s->slots[0].state.x + 40 * 65536;
        CHECK(SotnActors_Tick(s, p, 0, 0));
        SotnActorState a = {0};
        CHECK(SotnActors_Get(s, 0, &a));
        attacking |= a.step == 4;
        if (id == 0x27 && a.step == 4 && a.frame == 12)
            CHECK(a.hitbox_width == 20 && a.hitbox_x == -11);
        for (Uint32 j = 96; j < 128; ++j)
        {
            CHECK(SotnActors_Get(s, j, &a));
            projectile |= a.generation != 0 && a.id == 0x2f;
        }
    }
    CHECK(attacking);
    if (id == 0x2e)
        CHECK(projectile);
    SotnActorState a = {0};
    CHECK(SotnActors_Get(s, 0, &a));
    p.x = a.x;
    p.y = a.y;
    p.attack = (SotnPlayerAttack){.active = true,
                                  .width = 64,
                                  .height = 64,
                                  .hit_type = 2,
                                  .power = 100,
                                  .invincibility_frames = 2,
                                  .hit_effect = 3};
    CHECK(SotnActors_Tick(s, p, 0, 0));
    CHECK(SotnActors_Get(s, 0, &a) && a.generation == 0);
    SotnActorCombat combat = {0};
    CHECK(SotnActors_GetCombat(s, &combat) && combat.kills == 1);
    Uint32 parts = 0;
    for (Uint32 i = 160; i < 192; ++i)
    {
        CHECK(SotnActors_Get(s, i, &a));
        if (a.generation != 0 && a.id == (id == 0x27 ? 0x28 : 0x30))
        {
            ++parts;
            CHECK(a.rotated);
        }
    }
    CHECK(parts == (id == 0x27 ? 7U : 6U));
    SotnActors_Destroy(s);
    SotnActorLayout_Close(layout);
    return true;
}
static bool Recovery(Fixture *f)
{
    SotnActors *s = NULL;
    SotnActorLayout *layout = NULL;
    CHECK(Open(f, 0x26, &s, &layout));
    SotnPlayerState p = {.x = 200 * 65536, .y = 135 * 65536, .hp = 70, .hp_max = 70};
    for (Uint32 t = 0; t < 8; ++t)
        CHECK(SotnActors_Tick(s, p, 0, 0));
    SotnActorState a = {0};
    CHECK(SotnActors_Get(s, 0, &a) && a.step == 2);
    p.x = a.x;
    p.y = a.y;
    p.attack = (SotnPlayerAttack){.active = true,
                                  .width = 64,
                                  .height = 64,
                                  .hit_type = 2,
                                  .power = 100,
                                  .invincibility_frames = 1};
    CHECK(SotnActors_Tick(s, p, 0, 0));
    CHECK(SotnActors_Get(s, 0, &a) && a.step == 3 && a.hitbox_state == 0);
    p.attack.active = false;
    for (Uint32 t = 0; t < 300; ++t)
        CHECK(SotnActors_Tick(s, p, 0, 0));
    CHECK(SotnActors_Get(s, 0, &a) && a.generation != 0 && a.step == 2 && a.hitbox_state == 3 &&
          a.hit_points == 0);
    CHECK(!a.rotated);
    CHECK(ActorRandom(s) <= 255);
    Actor bad = {.state = {.id = 0x30, .parameters = 6}};
    CHECK(!ActorSkeleton(s, &bad, p));
    SotnActors_Destroy(s);
    SotnActorLayout_Close(layout);
    return true;
}
static bool Knight(Fixture *f)
{
    SotnActors *s = NULL;
    SotnActorLayout *layout = NULL;
    CHECK(Open(f, 0x29, &s, &layout));
    SotnPlayerState player = {.x = 180 * 65536, .y = 135 * 65536, .hp = 70, .hp_max = 70};
    for (Uint16 type = 0; type < 3; ++type)
    {
        Put32(f->nz0 + 0x22bc + type * 4, type == 2 ? 65536 : 131072);
        Put32(f->nz0 + 0x22c8 + type * 4, type == 2 ? (Uint32)-262144 : 0);
    }
    for (Uint16 type = 0; type < 3; ++type)
    {
        Actor axe = {.state = {.x = 120 * 65536,
                               .y = 100 * 65536,
                               .id = 0x2a,
                               .parameters = type,
                               .facing_left = true}};
        CHECK(ActorKnight(s, &axe, player));
        CHECK(axe.state.rotated);
        CHECK(ActorKnight(s, &axe, player));
        CHECK(axe.state.rotation == (type == 0 ? 3968 : 128));
        if (type == 2)
            CHECK(axe.vy == -262144 + 8192);
        else
        {
            for (Uint32 t = 1; t < 96; ++t)
                CHECK(ActorKnight(s, &axe, player));
            CHECK(axe.vx == -131072 && axe.state.x == 246 * 65536);
        }
    }
    Actor invalid = {.state = {.id = 0x2a, .parameters = 3}};
    CHECK(!ActorKnight(s, &invalid, player));
    // One synthetic multipart death piece, unrelated to original artwork.
    Put32(f->nz0 + 20, 0x80180400);
    Put32(f->nz0 + 0x400 + 4 * 4, 0x80180500);
    for (Uint32 i = 0; i < 48; ++i)
        Put32(f->nz0 + 0x500 + i * 4, 0x801805c0);
    Put16(f->nz0 + 0x5c0, 1);
    Put16(f->nz0 + 0x5c0 + 2 + 6, 8);
    Put16(f->nz0 + 0x5c0 + 2 + 8, 8);
    f->nz0[0x2268] = 1;
    f->nz0[0x2269] = 32;
    f->nz0[0x226a] = 255;
    Actor knight = {.state = {.x = 120 * 65536,
                              .y = 128 * 65536,
                              .generation = 42,
                              .id = 0x29,
                              .animation_set = 0x8004,
                              .step = 2,
                              .flags = 0x100,
                              .priority = 0x74},
                    .slot = 7};
    CHECK(ActorKnight(s, &knight, player));
    CHECK(ActorKnight(s, &knight, player));
    CHECK(knight.state.primitive_count == 2);
    ActorPrimitive *part = ActorPrimitiveGet(s, &knight, 0);
    CHECK(part != NULL && part->state.active);
    ActorPrimitive *companion = ActorPrimitiveGet(s, &knight, 1);
    CHECK(companion != NULL && !companion->state.active && companion->owner == 42);
    s->slots[7] = knight;
    SotnActorPrimitive copy = {0};
    CHECK(SotnActors_GetPrimitive(s, 7, 0, &copy) && copy.active && copy.width == 8);
    for (Uint32 t = 0; t < 280 && !knight.remove; ++t)
        CHECK(ActorKnight(s, &knight, player));
    CHECK(knight.remove);
    ActorPrimitivesClear(s, &knight);
    CHECK(s->primitives[knight.primitive[0]].owner == 0);
    CHECK(s->primitives[knight.primitive[1]].owner == 0);
    CHECK(!SotnActors_GetPrimitive(s, 7, ACTOR_PRIMITIVE_LIMIT, &copy));
    SotnActors_Destroy(s);
    SotnActorLayout_Close(layout);
    return true;
}
static bool Spittle(Fixture *f)
{
    SotnActors *s = NULL;
    SotnActorLayout *layout = NULL;
    CHECK(Open(f, 0x31, &s, &layout));
    Put16(f->nz0 + 0xcac, 0x8001);
    Put16(f->nz0 + 0xcac + 2, 52);
    Put16(f->nz0 + 0xcb8, 0x8001);
    Put16(f->nz0 + 0xcb8 + 2, 64);
    f->nz0[0x2524] = 2;
    f->nz0[0x2525] = 52;
    f->nz0[0x2534] = 2;
    f->nz0[0x2535] = 64;
    for (Uint32 x = 0; x < 32; ++x)
        for (Uint32 y = 0; y < 2; ++y)
            CHECK(ActorTile(s, y * 32 + x, 1));
    SotnPlayerState player = {
        .x = 200 * 65536, .y = 135 * 65536, .hp = 70, .hp_max = 70, .invulnerable = true};
    CHECK(SotnActors_Tick(s, player, 0, 0));
    Actor *parent = &s->slots[0];
    parent->state.step = 2;
    parent->state.y = 32 * 65536;
    parent->vx = 65536;
    parent->state.rotation = 0x800;
    parent->enemy_state.counter = 1;
    Actor *child = NULL;
    CHECK(ActorSpawnAt(s, parent, 0x32, 0, 1, &child));
    child->state.step = 1;
    child->state.y = 32 * 65536;
    child->vx = 65536;
    child->enemy_state.counter = 1;
    child->state.rotation = 0x800;
    s->random = 0;
    bool spit = false, splash = false;
    for (Uint32 t = 0; t < 250; ++t)
    {
        CHECK(SotnActors_Tick(s, player, 0, 0));
        for (Uint32 i = 96; i < 128; ++i)
        {
            const Actor *a = &s->slots[i];
            if (a->state.id == 0x33 && a->state.generation != 0)
            {
                spit = true;
                splash |= a->state.primitive_count == 9;
            }
        }
    }
    CHECK(spit && splash);
    parent->state.generation = 0;
    CHECK(ActorSpittle(s, child, player) && child->remove);
    SotnActors_Destroy(s);
    SotnActorLayout_Close(layout);
    return true;
}

static bool Shields(Fixture *f)
{
    for (Uint32 facing = 0; facing < 4; ++facing)
    {
        SotnActors *s = NULL;
        SotnActorLayout *layout = NULL;
        CHECK(Open(f, 0x2f, &s, &layout));
        SotnPlayerState player = {.x = 120 * 65536,
                                  .y = 135 * 65536,
                                  .facing_left = (facing & 1) != 0,
                                  .animation = {.hitbox_width = 8, .hitbox_height = 16}};
        CHECK(SotnActors_Tick(s, player, 0, 0));
        Actor *bone = &s->slots[0];
        bone->state.x = (Sint32)((facing & 1) ? 108 : 132) * 65536;
        bone->state.y = player.y;
        const bool projectile = facing < 2;
        bone->state.hitbox_state = projectile ? 13 : 5;
        const Sint16 hp = bone->state.hit_points;
        bone->state.hitbox_width = bone->state.hitbox_height = 8;
        bone->vx = bone->vy = 0;
        player.shield = (SotnPlayerAttack){.equipment = 16,
                                           .right_hand = true,
                                           .active = true,
                                           .offset_x = 14,
                                           .width = 4,
                                           .height = 16,
                                           .hit_type = 4,
                                           .hit_effect = 1,
                                           .invincibility_frames = 6,
                                           .power = 1};
        CHECK(SotnActors_Tick(s, player, 0, 0));
        CHECK(s->combat.contact_damage == 0 && s->combat.kills == 0);
        if (projectile)
            CHECK(bone->state.generation == 0 || bone->remove || (bone->state.flags & 0x100));
        else
            CHECK(bone->state.hit_points == hp && !(bone->state.flags & 0x100));
        SotnActors_Destroy(s);
        SotnActorLayout_Close(layout);
    }
    return true;
}

int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (f == NULL)
        return 1;
    const bool good = Skeletons(f, 0x2e) && Skeletons(f, 0x27) && Recovery(f) && Knight(f) &&
                      Spittle(f) && Shields(f);
    SDL_free(f);
    if (!good)
        return 1;
    SDL_Log("SOTN_ENEMY_RUNTIME_TEST: PASS");
    return 0;
}
