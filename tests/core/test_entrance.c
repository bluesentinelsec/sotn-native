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
    Uint8 dra[0x400], stage[0x5000], tiles[32 * 16 * 2], types[1];
} Fixture;
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static void Put32(Uint8 *p, Uint32 value)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(value >> (i * 8));
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_memset(f, 0, sizeof(*f));
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 10);
    Put16(f->dra + 0x206, 5);
    Put16(f->dra + 0x20c, 3);
    f->dra[0x222] = 8;
    f->dra[0x223] = 16;
    Put16(f->stage + 0xb90, 0x8001);
    Put16(f->stage + 0xba8, 0x8002);
    /* Original algorithm with entirely synthetic frame streams. */
    const size_t streams[] = {0x3c44, 0x3c60, 0x3c7c, 0x3c84, 0x3c98, 0x30fc,
                              0x3178, 0x3148, 0x3130, 0x3180, 0x31a0};
    for (size_t i = 0; i < SDL_arraysize(streams); ++i)
    {
        f->stage[streams[i]] = 1;
        f->stage[streams[i] + 1] = 1;
        f->stage[streams[i] + 2] = 255;
    }
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 65, out));
    (*out)->map =
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16};
    (*out)->player = (SotnPlayerState){.x = 160 * 65536, .y = 100 * 65536};
    return true;
}

static bool Profiles(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(!SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                  (SotnDataView){f->stage, sizeof(f->stage)}, 0, &s));
    CHECK(s == NULL);
    CHECK(Open(f, &s));
    Put32(f->stage + 28, 0x80180040);
    for (Uint32 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 100);
    Put16(f->stage + 0x30c, 100);
    Put16(f->stage + 0x30e, 0x62);
    Put16(f->stage + 0x314, 0xffff);
    Put16(f->stage + 0x316, 0xffff);
    const SotnDataView image = {f->stage, sizeof(f->stage)};
    SotnActorLayout *layout = NULL;
    CHECK(!SotnActorLayout_Open(image, 0, &layout) && layout == NULL);
    CHECK(SotnActorLayout_OpenStage(image, 65, 0, &layout));
    CHECK(SotnActorLayout_Count(layout) == 1);
    SotnActorLayout_Close(layout);
    Put16(f->stage + 0x30e, 0x63);
    CHECK(!SotnActorLayout_OpenStage(image, 65, 0, &layout) && layout == NULL);
    CHECK(!SotnActorLayout_OpenStage(image, 0, 0, &layout));
    SotnActors_Destroy(s);
    return true;
}

static bool Bats(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = {.state = {.id = 0x48, .x = 100 * 65536, .y = 80 * 65536, .generation = 1}};
    CHECK(ActorEntrance(s, &a) && a.state.frame == 31 && !a.state.unsupported);
    s->player.x = 300 * 65536;
    CHECK(ActorEntrance(s, &a) && a.state.step == 1);
    s->player.x = 120 * 65536;
    s->player.y = 40 * 65536;
    CHECK(ActorEntrance(s, &a) && a.state.step == 1);
    s->player.y = 100 * 65536;
    CHECK(ActorEntrance(s, &a) && a.state.step == 2);
    for (Uint32 i = 0; i < 3; ++i)
        CHECK(ActorEntrance(s, &a));
    CHECK(a.state.step >= 3 && a.vy == 57344);
    a.state.step = 4;
    a.gravity = 2048;
    a.vy = 67584;
    a.pose = a.remaining = 0;
    CHECK(ActorEntrance(s, &a) && a.gravity == -2048 && a.vy == 65536);
    a.state.flags |= 0x100;
    CHECK(ActorEntrance(s, &a) && a.remove && s->slots[160].state.id == 2);
    CHECK(s->combat.sound_count == 1 && s->combat.sounds[0] == 0x69c);
    SotnActors_Destroy(s);
    return true;
}

static bool Zombies(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = {.state = {.id = 0x4c, .x = 100 * 65536, .y = 80 * 65536, .generation = 1}};
    CHECK(ActorEntrance(s, &a) && a.state.hitbox_width == 8 && a.state.hitbox_height == 0);
    a.state.step = 2;
    CHECK(ActorEntrance(s, &a) && a.state.hitbox_y == 14 && a.state.hitbox_height == 2);
    CHECK(ActorEntrance(s, &a) && a.state.step == 3 && a.state.hitbox_height == 4);
    a.state.step = 4;
    a.pose = a.remaining = 0;
    for (Uint32 i = 0; i < 3; ++i)
        CHECK(ActorEntrance(s, &a));
    CHECK(a.remove);
    a = (Actor){.state = {.id = 0x4c, .step = 3, .flags = 0x100, .generation = 2}};
    CHECK(ActorEntrance(s, &a) && a.remove && s->slots[160].state.id == 0x62);
    CHECK(s->slots[160].state.parameters == 3);
    Actor spawner = {.state = {.id = 0x4d, .y = 100 * 65536, .generation = 3}};
    CHECK(ActorEntrance(s, &spawner) && spawner.timer == 1);
    for (Uint32 i = 0; i < 120; ++i)
        CHECK(ActorEntrance(s, &spawner));
    CHECK(s->slots[96].state.generation == 0);
    s->world->flags[55] = 1;
    s->camera->right = 2048;
    s->camera_x = 512;
    CHECK(ActorEntrance(s, &spawner) && s->slots[96].state.id == 0x4c);
    CHECK(spawner.timer >= 32 && spawner.timer <= 95);
    CHECK(Pixel(s->slots[96].state.y) == 52);
    SotnActors_Destroy(s);
    return true;
}

static bool Wargs(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = {.state = {.id = 0x4a, .x = 100 * 65536, .y = 100 * 65536, .generation = 4},
               .slot = 0};
    CHECK(ActorEntrance(s, &a) && !a.state.unsupported && a.state.step == 1);
    CHECK(s->slots[1].state.id == 0x4b && s->slots[1].enemy_state.owner_generation == 4);
    a.state.step = 7;
    a.warg.attack_timer = 1;
    CHECK(ActorEntrance(s, &a) && a.state.step == 6 && a.warg.timer == 32);
    a.state.flags |= 0x100;
    CHECK(ActorEntrance(s, &a) && a.state.step == 11);
    a.substep = 1;
    a.state.opacity = 2;
    CHECK(ActorEntrance(s, &a) && a.substep == 2 && a.state.opacity == 128);
    a.state.opacity = 2;
    CHECK(ActorEntrance(s, &a) && a.remove && s->slots[1].remove);
    SotnActors_Destroy(s);
    return true;
}

static bool Mermen(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = {.state = {.id = 0x41, .x = 192 * 65536, .y = 410 * 65536, .generation = 5}};
    CHECK(ActorEntrance(s, &a) && a.state.step == 1 && a.vy == -65536);
    CHECK(a.state.palette == 0x2b9 && a.state.hitbox_height == 17);
    a.state.step = 6;
    a.substep = 1;
    a.pose = 3;
    f->stage[0x3a9c + 6] = 1;
    f->stage[0x3a9c + 7] = 7;
    CHECK(ActorEntrance(s, &a) && s->slots[96].state.id == 0x42);
    CHECK(s->combat.sound_count == 1 && s->combat.sounds[0] == 0x662);
    Actor projectile = s->slots[96];
    CHECK(ActorEntrance(s, &projectile) && projectile.state.scale_x == 128);
    projectile.state.flags |= 0x100;
    f->stage[0x3af0] = 1;
    f->stage[0x3af1] = 1;
    CHECK(ActorEntrance(s, &projectile) && projectile.remove);
    a = (Actor){.state = {.id = 0x3a, .x = 192 * 65536, .y = 410 * 65536, .generation = 6}};
    CHECK(ActorEntrance(s, &a) && a.state.hitbox_y == 8);
    a.state.step = 6;
    a.substep = 1;
    f->stage[0x396c] = 255;
    CHECK(ActorEntrance(s, &a) && a.substep == 2 && a.state.rotated);
    CHECK(s->slots[97].state.id == 0x3c && s->slots[161].state.id == 0x3e);
    CHECK(s->slots[161].state.parameters == 1);
    a = (Actor){.state = {.id = 0x3b, .x = 192 * 65536, .y = 410 * 65536, .generation = 7}};
    CHECK(ActorEntrance(s, &a));
    for (Uint16 i = 0; i < 7; ++i)
        f->stage[0x39f4 + i] = (Uint8)(i + 1);
    CHECK(ActorEntrance(s, &a) && a.state.primitive_count == 4);
    CHECK(ActorPrimitiveGet(s, &a, 0)->state.active);
    ActorPrimitivesClear(s, &a);
    CHECK(s->primitives[a.primitive[0]].owner == 0);
    a = (Actor){.state = {.id = 0x44, .parameters = 4, .generation = 8}};
    CHECK(!ActorEntrance(s, &a));
    a = (Actor){.state = {.id = 0x3d, .parameters = 0, .generation = 9}};
    CHECK(!ActorEntrance(s, &a));
    a = (Actor){.state = {.id = 0x45, .generation = 10}};
    s->ticks = 64;
    s->player.x = 384 * 65536;
    s->player.y = 300 * 65536;
    CHECK(ActorEntrance(s, &a));
    CHECK(s->slots[98].state.id == 0x41 || s->slots[98].state.id == 0x3a);
    CHECK(Pixel(s->slots[98].state.y) == 496);
    SotnActors_Destroy(s);
    return true;
}

static bool Revisited(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    s->stage_id = 7;
    Put16(f->stage + 0xafc, 0x8001);
    Actor a = {.state = {.id = 0x40, .x = 100 * 65536, .y = 80 * 65536, .generation = 1}};
    CHECK(ActorEntrance(s, &a) && a.state.frame == 31 && !a.state.unsupported);
    // The same raw ID is a different actor in each overlay. No global remapping.
    a = (Actor){.state = {.id = 0x48, .generation = 2}};
    Put16(f->stage + 0xb38, 0x8002);
    CHECK(ActorEntrance(s, &a) && a.state.step == 2 && !a.state.unsupported);
    const Uint16 steps[] = {1, 3, 4, 6, 8};
    const size_t streams[] = {0x25ec, 0x267c, 0x25fc, 0x2620, 0x2634};
    // Poison Laboratory's streams to catch a stage identity regression.
    SDL_memset(f->stage + 0x22ec, 0, 0xa0);
    for (Uint32 i = 0; i < SDL_arraysize(steps); ++i)
    {
        f->stage[streams[i]] = 1;
        f->stage[streams[i] + 1] = 9;
        a = (Actor){.state = {.id = 0x48, .step = steps[i], .generation = 3}};
        CHECK(ActorEntrance(s, &a) && a.state.frame == 9);
    }
    // Spawned children also retain the return overlay's actual original IDs.
    a = (Actor){.state = {.id = 0x42, .step = 3, .flags = 0x100, .generation = 4}};
    CHECK(ActorEntrance(s, &a) && a.remove && s->slots[160].state.id == 0x4d);
    CHECK(EntranceActorId(s, 0x3a) == 0x32 && EntranceActorId(s, 0x45) == 0x3d &&
          EntranceActorId(s, 0x4c) == 0x42);
    a = (Actor){.state = {.id = 0x57, .generation = 5}};
    CHECK(ActorEntrance(s, &a) && a.state.unsupported); // Not first-visit Death.
    SotnActors_Destroy(s);
    Put32(f->stage + 28, 0x80180040);
    for (Uint32 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 100);
    Put16(f->stage + 0x30c, 100);
    Put16(f->stage + 0x314, 0xffff);
    Put16(f->stage + 0x316, 0xffff);
    Put16(f->stage + 0x30e, 0x5b);
    const SotnDataView image = {f->stage, sizeof(f->stage)};
    SotnActorLayout *layout = NULL;
    CHECK(SotnActorLayout_OpenStage(image, 7, 0, &layout));
    SotnActorLayout_Close(layout);
    Put16(f->stage + 0x30e, 0x5c);
    CHECK(!SotnActorLayout_OpenStage(image, 7, 0, &layout) && layout == NULL);
    return true;
}

int main(void)
{
    Fixture fixture = {0};
    if (!Profiles(&fixture) || !Bats(&fixture) || !Zombies(&fixture) || !Wargs(&fixture) ||
        !Mermen(&fixture) || !Revisited(&fixture))
        return 1;
    SDL_Log("SOTN_ENTRANCE_TEST: PASS");
    return 0;
}
