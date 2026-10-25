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
    Uint8 dra[0x400], stage[0x4000], exe[0x23220], tiles[32 * 32 * 2], types[2];
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
static void ByteAnimation(Fixture *f, size_t at, Uint8 frame)
{
    f->stage[at] = 2;
    f->stage[at + 1] = frame;
    f->stage[at + 2] = 255;
}
static bool Open(Fixture *f, SotnActors **s)
{
    SDL_memset(f, 0, sizeof(*f));
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 40);
    Put16(f->dra + 0x206, 5);
    Put16(f->dra + 0x20c, 3);
    f->dra[0x222] = 8;
    f->dra[0x223] = 16;
    Put32(f->dra + 0x224, 0x08401000);
    const size_t init[] = {0xc2c, 0xbb4, 0xbc0, 0xbcc, 0xc14, 0xc20, 0xaf4, 0xb00, 0xb0c, 0xa7c,
                           0xb6c, 0xb78, 0xa94, 0xa88, 0xb24, 0xb30, 0xb3c, 0xb48, 0xb54};
    for (Uint32 i = 0; i < SDL_arraysize(init); ++i)
    {
        Put16(f->stage + init[i], 0x8001);
        Put16(f->stage + init[i] + 2, 1);
    }
    SDL_memcpy(f->dra + 0x228, f->dra + 0x200, 40);
    Put16(f->dra + 0x22c, 0x7fff);
    Put32(f->dra + 0x24c, 0x0d200000);
    for (Uint32 i = 0; i < 4; ++i)
        f->stage[0x1d84 + i] = (Uint8)(12 + i * 2);
    const size_t impervious[] = {0xb30, 0xb3c, 0xb48, 0xb78};
    for (Uint32 i = 0; i < SDL_arraysize(impervious); ++i)
        Put16(f->stage + impervious[i] + 8, 1);
    f->types[1] = 3;
    Put32(f->stage + 28, 0x80180040);
    for (Uint32 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    ByteAnimation(f, 0x2640, 16);
    ByteAnimation(f, 0x2194, 1);
    ByteAnimation(f, 0x21a4, 2);
    ByteAnimation(f, 0x21b4, 10);
    ByteAnimation(f, 0x21c4, 13);
    ByteAnimation(f, 0x21d0, 14);
    for (Uint32 i = 0; i < 7; ++i)
        f->stage[0x21ec + i] = (Uint8)(8 + i);
    ByteAnimation(f, 0x25f4, 5);
    ByteAnimation(f, 0x350, 1);
    for (Uint32 i = 0; i < 5; ++i)
        Put32(f->stage + 0x1b28 + i * 4, 0x80180350);
    Put32(f->stage + 0x2620, 98304);
    Put32(f->stage + 0x2634, (Uint32)-196608);
    Put16(f->stage + 0x228c, 1);
    Put16(f->stage + 0x228e, 2);
    Put16(f->stage + 0x2290, 0xffff);
    Put16(f->stage + 0x2278, 0x40);
    Put16(f->stage + 0x227a, 1);
    Put16(f->stage + 0x227c, 1);
    Put16(f->stage + 0x227e, 256);
    for (Uint32 i = 0; i <= 1024; ++i)
        Put16(f->exe + 0x22a10 + i * 2, (Uint16)(i / 2));
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 0, s));
    CHECK(SotnActors_SetServices(*s, (SotnDataView){f->exe, sizeof(f->exe)}, 0, true));
    CHECK(SotnActorLayout_OpenStage((*s)->nz0, 0, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *s, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 32}, 0,
        0));
    /* The service borrows this empty layout until the test closes it. */
    return true;
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    f->layout = NULL;
}
static bool FleaAndSlinger(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor flea = {.state = {.id = 0x4c, .generation = 1, .x = 100 * 65536, .y = 100 * 65536}};
    CHECK(ActorFlea(s, &flea) && flea.state.frame == 16);
    s->player = (SotnPlayerState){.x = 120 * 65536, .y = 100 * 65536};
    CHECK(ActorFlea(s, &flea) && flea.state.step == 3);
    flea.hit_params = 1;
    CHECK(ActorFlea(s, &flea) && flea.state.step == 4 && flea.vx == -98304 && flea.vy == -196608);
    flea.state.flags |= 0x100;
    CHECK(ActorFlea(s, &flea) && flea.state.id == 2 && flea.state.parameters == 1 && !flea.remove);
    Actor slinger = {.state = {.id = 0x3a, .generation = 2, .x = 100 * 65536, .y = 100 * 65536}};
    CHECK(ActorSlinger(s, &slinger));
    SetStep(&slinger, 4);
    CHECK(ActorSlinger(s, &slinger) && s->slots[96].state.id == 0x3b);
    CHECK(ActorSlinger(s, &s->slots[96]) && s->slots[96].vy == 0);
    slinger.state.flags |= 0x100;
    CHECK(ActorSlinger(s, &slinger) && slinger.remove);
    for (Uint32 i = 160; i < 167; ++i)
        CHECK(s->slots[i].state.id == 0x3c);
    Actor invalid = {.state = {.id = 0x3c, .parameters = 7, .generation = 4}};
    CHECK(!ActorSlinger(s, &invalid));
    Close(f, s);
    return true;
}
static bool Wide(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Put16(f->stage + 0x1000, 2);
    Put16(f->stage + 0x1002, 31);
    Put16(f->stage + 0x1004, 1);
    Put16(f->stage + 0x1006, 32);
    Put16(f->stage + 0x1008, 0xffff);
    Actor a = {0};
    bool end = false;
    CHECK(ActorWideAnimation(s, &a, 0x1000, &end) && a.state.frame == 31 && !end);
    CHECK(ActorWideAnimation(s, &a, 0x1000, &end) && a.state.frame == 31 && !end);
    CHECK(ActorWideAnimation(s, &a, 0x1000, &end) && a.state.frame == 32 && !end);
    CHECK(ActorWideAnimation(s, &a, 0x1000, &end) && a.state.frame == 32 && end);
    CHECK(ActorWideAnimation(s, &a, 0x1000, &end) && end);
    a = (Actor){0};
    Put16(f->stage + 0x1008, 0);
    Put16(f->stage + 0x100a, 0);
    for (Uint32 i = 0; i < 4; ++i)
        CHECK(ActorWideAnimation(s, &a, 0x1000, &end));
    CHECK(a.state.frame == 31 && !end);
    a = (Actor){0};
    Put16(f->stage + 0x1000, 0xfffe);
    CHECK(!ActorWideAnimation(s, &a, 0x1000, &end));
    a.pose = 512;
    CHECK(!ActorWideAnimation(s, &a, 0x1000, &end));
    Close(f, s);
    return true;
}
static bool LinkedRose(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *root = &s->slots[16];
    *root = (Actor){.state = {.id = 0x3d, .generation = 100, .x = 128 * 65536, .y = 144 * 65536},
                    .slot = 16,
                    .dynamic = true};
    CHECK(ActorRose(s, root));
    for (Uint32 i = 17; i < 28; ++i)
        CHECK(ActorRose(s, &s->slots[i]));
    Actor *head = &s->slots[27];
    CHECK(head->state.hit_points == 40);
    head->stun = 5; // A lethal hit must not stall the linked death behind an older stun.
    const SotnPlayerState player = {.x = head->state.x,
                                    .y = head->state.y,
                                    .hp = 70,
                                    .invulnerable = true,
                                    .attack = {.active = true,
                                               .power = 999,
                                               .element = 0x20,
                                               .hit_type = 2,
                                               .hit_effect = 1,
                                               .invincibility_frames = 12,
                                               .width = 32,
                                               .height = 32},
                                    .animation = {.hitbox_width = 8, .hitbox_height = 20}};
    CHECK(SotnActors_Tick(s, player, 0, 0));
    // The earlier root sees the later head's lethal contact in this same simulation tick.
    CHECK(s->combat.kills == 1);
    for (Uint32 i = 16; i < 28; ++i)
        CHECK(s->slots[i].state.step == 7 && s->slots[i].rose.timer == 191);
    for (Uint32 t = 0; t < 260; ++t)
        CHECK(SotnActors_Tick(s, player, 0, 0));
    CHECK(s->slots[16].state.generation == 0 && s->combat.kills == 1);
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        CHECK(s->primitives[i].owner == 0);
    Close(f, s);
    return true;
}
static bool Ouija(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    f->types[1] = 3;
    for (Uint32 row = 10; row < 32; ++row)
        for (Uint32 column = 0; column < 32; ++column)
            Put16(s->mutable_tiles + (row * 32 + column) * 2, 1);
    Actor *table = &s->slots[16];
    *table = (Actor){.state = {.id = 0x4a, .generation = 200, .x = 128 * 65536, .y = 140 * 65536},
                     .slot = 16,
                     .dynamic = true};
    for (Uint32 t = 0; t < 40 && table->state.step < 2; ++t)
        CHECK(ActorOuija(s, table));
    CHECK(table->state.step == 2);
    for (Uint32 i = 17; i < 20; ++i)
        CHECK(ActorOuija(s, &s->slots[i]));
    CHECK(s->slots[19].state.step == 16 && s->slots[19].state.hitbox_state == 0);
    s->player = (SotnPlayerState){.x = 140 * 65536, .y = 144 * 65536};
    CHECK(ActorOuija(s, table) && table->state.step == 3);
    for (Uint32 t = 0; t < 150; ++t)
    {
        CHECK(ActorOuija(s, table));
        CHECK(ActorOuija(s, &s->slots[17]));
        CHECK(ActorOuija(s, &s->slots[18]));
    }
    CHECK(table->table.timer != 128 && s->slots[18].table.thrown);
    // Begin the death fixture above the floor; this direct behavior test has no
    // player-contact phase to stop a previously thrown component at Alucard.
    for (Uint32 i = 17; i < 19; ++i)
        s->slots[i].state.y = 100 * 65536;
    table->state.flags |= 0x100;
    CHECK(ActorOuija(s, table) && table->remove);
    *table = (Actor){0};
    for (Uint32 t = 0; t < 140; ++t)
        for (Uint32 i = 17; i < 19; ++i)
            if (!s->slots[i].remove)
                CHECK(ActorOuija(s, &s->slots[i]));
    CHECK(s->slots[17].remove && s->slots[18].remove);
    CHECK(ActorOuija(s, &s->slots[19]) && s->slots[19].state.step == 16);
    Close(f, s);
    return true;
}
static bool SeedRecipes(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Put16(f->stage + 0x2268, 0x3e);
    Put16(f->stage + 0x226a, 0x101);
    Put16(f->stage + 0x226c, 1);
    for (Uint32 i = 96; i < 128; ++i)
        s->slots[i].state.generation = 9;
    Actor spawner = {.state = {.id = 0x41, .generation = 10}};
    CHECK(ActorRoseSeed(s, &spawner) && spawner.remove);
    Put16(f->stage + 0x2270, 0x3f);
    Put16(f->stage + 0x2272, 2);
    Put16(f->stage + 0x2274, 1);
    Put16(f->stage + 0x2276, 3);
    spawner = (Actor){.state = {.id = 0x41, .parameters = 1, .generation = 11}};
    CHECK(ActorRoseSeed(s, &spawner) && !spawner.remove && spawner.rose.spawned == 0);
    s->slots[96] = (Actor){0};
    CHECK(ActorRoseSeed(s, &spawner) && !spawner.remove && spawner.rose.spawned == 1);
    s->slots[97] = (Actor){0};
    CHECK(ActorRoseSeed(s, &spawner) && spawner.rose.spawned == 1);
    CHECK(ActorRoseSeed(s, &spawner) && spawner.rose.spawned == 1);
    CHECK(ActorRoseSeed(s, &spawner) && spawner.remove && spawner.rose.spawned == 2);
    CHECK(s->slots[96].state.id == 0x3f && s->slots[97].state.id == 0x3f);
    Actor effect = {.state = {.id = 0x40, .generation = 12, .x = 128 * 65536, .y = 100 * 65536}};
    CHECK(ActorRoseSeed(s, &effect) && effect.state.id == 0x40 && effect.state.step == 1);
    const Sint32 x = effect.state.x;
    CHECK(ActorRoseSeed(s, &effect) && effect.state.x != x && effect.state.id == 0x40);
    spawner = (Actor){.state = {.id = 0x41, .parameters = 3, .generation = 13}};
    CHECK(!ActorRoseSeed(s, &spawner));
    Close(f, s);
    return true;
}
static bool Skelerang(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *root = &s->slots[16];
    *root = (Actor){.state = {.id = 0x2e, .generation = 80, .x = 128 * 65536, .y = 144 * 65536},
                    .slot = 16,
                    .dynamic = true};
    CHECK(ActorSkelerang(s, root));
    for (Uint32 i = 17; i <= 19; ++i)
        CHECK(ActorSkelerang(s, &s->slots[i]));
    CHECK(s->slots[17].state.id == 0x30 && s->slots[18].state.id == 0x2f);
    CHECK(ActorSkelerang(s, &s->slots[18]) && ActorSkelerang(s, &s->slots[19]));
    CHECK(s->slots[18].table.timer == 48 && s->slots[19].table.angle == 0);
    s->player = (SotnPlayerState){.x = 240 * 65536, .y = 144 * 65536};
    ByteAnimation(f, 0x1f00, 30);
    SetStep(root, 4);
    CHECK(ActorSkelerang(s, root));
    CHECK(s->slots[18].state.step == 2 && s->slots[19].state.step == 2);
    Actor *boom = &s->slots[18];
    CHECK(ActorSkelerang(s, boom) && boom->state.hitbox_state == 1);
    boom->table.timer = 0;
    boom->state.x = root->state.x;
    boom->state.y = root->state.y;
    boom->vx = boom->vy = 0;
    CHECK(ActorSkelerang(s, boom) && boom->state.step == 3);
    root->state.flags |= 0x100;
    root->state.frame = 1;
    CHECK(ActorSkelerang(s, root) && root->remove);
    CHECK(s->slots[17].remove && s->slots[19].state.step == 4);
    for (Uint32 i = 160; i < 168; ++i)
    {
        Actor *part = &s->slots[i];
        CHECK(part->state.id == 0x2e && part->state.step == 14);
        for (Uint32 t = 0; t < 16; ++t)
            CHECK(ActorSkelerang(s, part));
        CHECK(part->remove);
    }
    root->state.generation = 81;
    boom->state.step = 2;
    CHECK(ActorSkelerang(s, boom) && boom->remove);
    root->state.generation = 82;
    root->state.flags = 0;
    root->state.step = 0;
    root->remove = false;
    CHECK(ActorSkelerang(s, root) && s->slots[18].enemy_state.owner_generation == 82);
    const Uint32 target_generation = s->slots[17].state.generation;
    s->slots[18].state.id = 0x4c;
    root->state.step = 0;
    CHECK(!ActorSkelerang(s, root) && s->slots[17].state.generation == target_generation);
    Close(f, s);
    return true;
}
static bool Diplo(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    for (Uint32 y = 12; y < 32; ++y)
        for (Uint32 x = 0; x < 32; ++x)
            Put16(f->tiles + (y * 32 + x) * 2, 1);
    SDL_memcpy(s->mutable_tiles, f->tiles, sizeof(f->tiles));
    const size_t sensors[] = {0x1d54, 0x1d64, 0x1d74};
    for (Uint32 i = 0; i < SDL_arraysize(sensors); ++i)
        Put16(f->stage + sensors[i] + 2, 16);
    const size_t anims[] = {0x1d88, 0x1da0, 0x1dac, 0x1db4, 0x1dbc, 0x1ddc,
                            0x1dec, 0x1e0c, 0x1e30, 0x1e3c, 0x1e54};
    for (Uint32 i = 0; i < SDL_arraysize(anims); ++i)
        ByteAnimation(f, anims[i], 1);
    Put16(f->stage + 0x1d30, 255);
    Put16(f->stage + 0x1d3c, 255);
    Put16(f->stage + 0x1d48, 255);
    Actor *root = &s->slots[16];
    *root = (Actor){.state = {.id = 0x21, .generation = 100, .x = 128 * 65536, .y = 150 * 65536},
                    .slot = 16,
                    .dynamic = true};
    CHECK(ActorDiplo(s, root) && root->state.step == 2);
    for (Uint32 i = 17; i < 35; ++i)
        if (s->slots[i].state.generation)
            CHECK(ActorDiplo(s, &s->slots[i]));
    CHECK(s->slots[31].state.id == 0x24 &&
          (s->slots[31].state.step == 5 || s->slots[31].state.step == 11));
    CHECK(s->slots[34].state.id == 0x26);
    const SotnPlayerState player = {
        .x = 128 * 65536, .y = 150 * 65536, .hp = 70, .invulnerable = true};
    bool walking = false;
    for (Uint32 t = 0; t < 100; ++t)
    {
        CHECK(SotnActors_Tick(s, player, 0, 0));
        walking = walking || root->state.step == 6 || root->state.step == 7;
    }
    CHECK(walking);
    // Lethal head contact reaches every linked limb before the body's death update.
    root->stun = 5;
    SotnPlayerState strike = player;
    strike.x = root->state.x;
    strike.y = root->state.y;
    strike.attack = (SotnPlayerAttack){.active = true,
                                       .power = 999,
                                       .element = 0x20,
                                       .hit_type = 2,
                                       .hit_effect = 1,
                                       .invincibility_frames = 12,
                                       .width = 4,
                                       .height = 4};
    CHECK(SotnActors_Tick(s, strike, 0, 0));
    CHECK(s->combat.kills == 1 && root->state.step == 12);
    for (Uint32 i = 17; i < 32; ++i)
        CHECK((s->slots[i].state.flags & 0x100) != 0);
    bool detached = false;
    for (Uint32 t = 0; t < 700; ++t)
    {
        CHECK(SotnActors_Tick(s, player, 0, 0));
        detached = detached || root->state.step == 14;
    }
    CHECK(detached && !root->state.generation && !s->slots[34].state.generation);
    for (Uint32 i = 17; i < 32; ++i)
        CHECK(s->slots[i].state.id != 0x22 && s->slots[i].state.id != 0x23 &&
              s->slots[i].state.id != 0x24);
    Actor fire = {.state = {.id = 0x25, .generation = 300, .x = 128 * 65536, .y = 130 * 65536}};
    CHECK(ActorDiplo(s, &fire) && fire.state.primitive_count == 20 && fire.state.scale_x == 112);
    for (Uint32 t = 0; t < 110; ++t)
    {
        ++s->ticks;
        CHECK(ActorDiplo(s, &fire));
        if (fire.state.step >= 2)
            CHECK(SDL_abs(fire.vx) <= 3 * 65536 && SDL_abs(fire.vy) <= 3 * 65536);
    }
    CHECK(fire.state.step == 3);
    fire.state.flags |= 0x100;
    for (Uint32 t = 0; t < 4 && !fire.remove; ++t)
        CHECK(ActorDiplo(s, &fire));
    CHECK(fire.remove);
    ActorPrimitivesClear(s, &fire);
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        CHECK(s->primitives[i].owner != fire.state.generation);
    Actor invalid = {.state = {.id = 0x21, .generation = 101}, .slot = 180};
    CHECK(!ActorDiplo(s, &invalid));
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (f == NULL)
        return 1;
    const bool ok = FleaAndSlinger(f) && Wide(f) && LinkedRose(f) && Ouija(f) && SeedRecipes(f) &&
                    Skelerang(f) && Diplo(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
