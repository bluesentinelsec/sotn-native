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
    Uint8 dra[0x1c00], stage[0x4000], exe[0x23220], tiles[16 * 16 * 2], types[2];
} Fixture;
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
static bool Open(Fixture *f, SotnActors **s, Uint16 form)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    const size_t init[] = {0x8cc, 0x8e4, 0x8f0};
    for (Uint16 i = 0; i < 3; ++i)
    {
        Put16(f->stage + init[i] + 8, (Uint16)(157 + i));
        Uint8 *p = f->dra + 0x200 + (157 + i) * 40;
        Put16(p + 4, 20);
        Put16(p + 6, 8);
        Put16(p + 12, 3);
        p[34] = 8;
        p[35] = 16;
    }
    Put16(f->dra + 0x200 + 158 * 40 + 10, 7);
    Put16(f->dra + 0x200 + 158 * 40 + 24, 80);
    for (Uint16 i = 0; i < 4; ++i)
    {
        Put32(f->stage + 0x2bdc + i * 4U, 0x80183000 + i * 8U);
        f->stage[0x3000 + i * 8] = 1;
        f->stage[0x3001 + i * 8] = i < 2 ? 5 : 8;
        f->stage[0x3002 + i * 8] = 255;
    }
    Put16(f->stage + 0x2b82, 8);
    for (Uint16 i = 0; i < 29; ++i)
        f->stage[0x2c18 + i] = 1;
    f->stage[0x2bf0] = 1;
    f->stage[0x2bf1] = 2;
    f->stage[0x2bf2] = 8;
    f->stage[0x2bf3] = 12;
    f->stage[0x2bc0] = f->stage[0x2bc8] = 1;
    f->stage[0x2bc1] = 20;
    f->stage[0x2bc9] = 22;
    f->stage[0x2bd0] = 24;
    f->stage[0x2bd1] = 23;
    f->stage[0x2bd2] = 255;
    Put16(f->exe + 0x1dbcc + 1024 * 2, 4096);
    f->types[1] = 3;
    for (Uint16 y = 12; y < 16; ++y)
        for (Uint16 x = 0; x < 16; ++x)
            Put16(f->tiles + (y * 16 + x) * 2, 1);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 2, s));
    (*s)->executable = (SotnDataView){f->exe, sizeof(f->exe)};
    (*s)->map =
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 16, 16};
    (*s)->player = (SotnPlayerState){.x = 96 * 65536, .y = 160 * 65536};
    (*s)->slots[64] = (Actor){
        .slot = 64,
        .state = {
            .id = 0x3a, .parameters = form, .generation = 1, .x = 128 * 65536, .y = 184 * 65536}};
    return true;
}
static bool Grow(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s, 1));
    Actor *root = &s->slots[64], *head = &s->slots[65];
    CHECK(ActorLibraryPlant(s, root) && root->state.frame == 9);
    CHECK(ActorLibraryPlant(s, root) && root->state.step == 2);
    CHECK(ActorLibraryPlant(s, root) && root->state.step == 3);
    CHECK(ActorLibraryPlant(s, root) && root->state.frame == 5);
    CHECK(ActorLibraryPlant(s, root) && root->state.step == 4 && root->plant.timer == 64);
    for (Uint16 i = 0; i < 63; ++i)
        CHECK(ActorLibraryPlant(s, root) && head->state.id == 0);
    CHECK(ActorLibraryPlant(s, root) && head->state.id == 0x3b && root->plant.spawned &&
          root->enemy.defense == 7 && root->experience == 80 && root->state.hit_points == 20);
    CHECK(head->state.facing_left && head->enemy_state.owner_generation == 1);
    CHECK(ActorLibraryPlant(s, head) && head->state.step == 1 && head->state.primitive_count == 2);
    ActorPrimitive *leaves = ActorPrimitiveGet(s, head, 0), *stalk = ActorPrimitiveGet(s, head, 1);
    CHECK(leaves && stalk && leaves->state.offset_x[0] == -1 && leaves->state.offset_y[0] == -1 &&
          stalk->state.priority == 134);
    for (Uint16 i = 0; i < 400 && head->state.step != 4; ++i)
        CHECK(ActorLibraryPlant(s, head));
    CHECK(head->state.step == 4 && head->plant.scale == 256 && head->state.frame == 19 &&
          head->state.hitbox_state == 3 && head->plant.leaves_grown && head->plant.stalk_grown);
    CHECK(Pixel(head->state.y) == Pixel(stalk->state.y) - 100);
    s->player.x = 96 * 65536;
    for (Uint16 i = 0; i < 128; ++i)
        CHECK(ActorLibraryPlant(s, head));
    CHECK(head->state.step == 5);
    for (Uint16 i = 0; i < 32; ++i)
        CHECK(ActorLibraryPlant(s, head));
    CHECK(head->substep == 2);
    CHECK(ActorLibraryPlant(s, head) && head->state.frame == 21 && head->plant.timer == 15 &&
          s->slots[96].state.id == 0x3c && s->slots[96].state.priority == 137);
    CHECK(s->combat.sounds[s->combat.sound_count - 1] == 0x70d);
    const Uint16 old_primitive = head->primitive[0];
    root->state.flags |= 0x100U;
    CHECK(ActorLibraryPlant(s, root) && root->state.step == 6 && root->state.opacity == 127 &&
          (head->state.flags & 0x100U));
    for (Uint16 i = 0; i < 127; ++i)
    {
        CHECK(ActorLibraryPlant(s, root));
        CHECK(ActorLibraryPlant(s, head));
    }
    CHECK(root->remove);
    root->state.id = 0;
    CHECK(ActorLibraryPlant(s, head) && head->remove);
    ActorPrimitivesClear(s, head);
    CHECK(s->primitives[old_primitive].owner == 0);
    SotnActors_Destroy(s);
    return true;
}
static bool Thorn(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s, 0));
    Actor *a = &s->slots[64];
    CHECK(ActorLibraryPlant(s, a) && a->state.frame == 1);
    a->state.flags |= 0x100U;
    CHECK(ActorLibraryPlant(s, a) && a->remove && s->slots[160].state.id == 2 &&
          s->slots[160].state.parameters == 0 && s->slots[160].state.y == 180 * 65536);
    a->state.flags = 0;
    a->remove = false;
    a->state.frame = 29;
    CHECK(!ActorLibraryPlantBox(s, a));
    a->state.frame = 0;
    f->stage[0x2c18] = 11;
    CHECK(!ActorLibraryPlantBox(s, a));
    a->state.parameters = 2;
    CHECK(!ActorLibraryPlant(s, a));
    CHECK(!ActorLibraryPlant(NULL, a) && !ActorLibraryPlant(s, NULL));
    SotnActors_Destroy(s);
    return true;
}
static bool Projectile(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s, 0));
    Actor *a = &s->slots[96];
    *a = (Actor){.slot = 96,
                 .state = {.id = 0x3c,
                           .generation = 100,
                           .x = 160 * 65536,
                           .y = 176 * 65536,
                           .facing_left = true}};
    CHECK(ActorLibraryPlant(s, a) && a->vx == -64 * 2048 && a->state.primitive_count == 2);
    for (Uint16 i = 0; i < 30 && a->state.step != 2; ++i)
        CHECK(ActorLibraryPlant(s, a));
    CHECK(a->state.step == 2 && s->combat.sounds[s->combat.sound_count - 1] == 0x672);
    CHECK(ActorLibraryPlant(s, a) && a->substep == 2);
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    CHECK(p && p->state.active && p->state.blend && p->state.modulate && p->state.page == 0x34 &&
          p->state.offset_x[1] == -32);
    CHECK(ActorLibraryPlant(s, a) && p->state.offset_x[1] == -40 && p->state.red == 57 &&
          p->state.offset_y[1] == -42);
    for (Uint16 i = 0; i < 24 && !a->remove; ++i)
        CHECK(ActorLibraryPlant(s, a));
    CHECK(a->remove);
    a->remove = false;
    a->state.flags = 0;
    a->hit_flags = 0x80;
    a->state.step = 2;
    a->pose = a->remaining = 0;
    CHECK(ActorLibraryPlant(s, a) && !a->remove);
    a->hit_flags = 1;
    CHECK(ActorLibraryPlant(s, a) && a->remove);
    SotnActors_Destroy(s);
    return true;
}
static bool ReverseAndCapacity(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s, 1));
    Actor *root = &s->slots[64], *head = &s->slots[65];
    s->player.x = 160 * 65536;
    root->state.step = 4;
    root->plant.timer = 1;
    CHECK(ActorLibraryPlant(s, root) && !head->state.facing_left);
    for (Uint16 i = 0; i < 400 && head->state.step != 4; ++i)
        CHECK(ActorLibraryPlant(s, head));
    CHECK(head->state.step == 4 && head->plant.stalk_grown);
    ActorPrimitive *stalk = ActorPrimitiveGet(s, head, 1);
    CHECK(stalk && stalk->state.offset_x[0] > stalk->state.offset_x[1]);
    head->state.flags |= 0x100U;
    for (Uint16 i = 0; i < 100 && head->substep != 2; ++i)
        CHECK(ActorLibraryPlant(s, head));
    CHECK(head->state.step == 6 && head->substep == 2 && (root->state.flags & 0x100U));
    CHECK(ActorLibraryPlant(s, root) && root->state.step == 6 && root->state.opacity == 127);
    SotnActors_Destroy(s);
    CHECK(Open(f, &s, 1));
    root = &s->slots[64];
    head = &s->slots[65];
    root->state.step = 4;
    root->plant.timer = 1;
    CHECK(ActorLibraryPlant(s, root));
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 900;
    CHECK(ActorLibraryPlant(s, head) && head->remove && head->state.primitive_count == 0);
    SotnActors_Destroy(s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    const bool ok = f && Grow(f) && Thorn(f) && Projectile(f) && ReverseAndCapacity(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
