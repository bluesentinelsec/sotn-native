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
    Uint8 dra[0x1000], stage[0x4000], exe[0x23220], tiles[16 * 16 * 2], types[2];
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
static bool Open(Fixture *f, SotnActors **s, Uint16 id)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    for (Uint32 i = 0; i < 4; ++i)
    {
        Uint8 *p = f->dra + 0x200 + i * 40;
        Put16(p + 4, 40);
        Put16(p + 6, 12);
        Put16(p + 8, 0x40);
        Put16(p + 12, 19);
        p[34] = 8;
        p[35] = 16;
    }
    Put16(f->stage + 0x89c + 8, 0);
    Put16(f->stage + 0x8a8 + 8, 1);
    Put16(f->stage + 0x9bc + 8, 2);
    Put16(f->stage + 0x9c8 + 8, 3);
    Put16(f->stage + 0x2d8e, 16);
    Put16(f->stage + 0x2d9e, 16);
    Put16(f->stage + 0x2956, 24);
    Put16(f->stage + 0x2966, 24);
    const size_t streams[] = {0x2d78, 0x2d84, 0x296c, 0x2984, 0x29a4,
                              0x29c4, 0x29d4, 0x29f0, 0x2acc};
    for (Uint32 i = 0; i < SDL_arraysize(streams); ++i)
    {
        f->stage[streams[i]] = 1;
        f->stage[streams[i] + 1] = 1;
        f->stage[streams[i] + 2] = 255;
    }
    Put16(f->stage + 0x2dd8, (Uint16)-10);
    Put16(f->stage + 0x2dda, 16);
    Put16(f->stage + 0x2de0, 7);
    Put16(f->stage + 0x2de2, 11);
    for (Uint32 i = 0; i < 6; ++i)
    {
        Put32(f->stage + 0x2da4 + i * 8, 8192 + i * 1024);
        Put32(f->stage + 0x2da8 + i * 8, (Uint32)-131072);
    }
    for (Uint32 i = 0; i < 7; ++i)
    {
        Put32(f->stage + 0x2a64 + i * 12, (Uint32)-16384);
        Put32(f->stage + 0x2a68 + i * 12, (Uint32)-65536);
        Put16(f->stage + 0x2a6c + i * 12, 64);
        Put16(f->stage + 0x2a6e + i * 12, 2);
    }
    for (Uint32 i = 0; i < 28; ++i)
    {
        f->stage[0x2a14 + i] = 1;
        f->stage[0x2a48 + i] = 1;
    }
    f->stage[0x29fc + 4] = (Uint8)-4;
    f->stage[0x29fc + 5] = 2;
    f->stage[0x29fc + 6] = 8;
    f->stage[0x29fc + 7] = 16;
    f->stage[0x2a30 + 4] = (Uint8)-28;
    f->stage[0x2a30 + 5] = (Uint8)-20;
    f->stage[0x2a30 + 6] = 12;
    f->stage[0x2a30 + 7] = 4;
    f->types[1] = 3;
    for (Uint32 y = 12; y < 16; ++y)
        for (Uint32 x = 0; x < 16; ++x)
            Put16(f->tiles + (y * 16 + x) * 2, 1);
    Put16(f->exe + 0x1dbcc + 1024 * 2, 4096);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 2, s));
    (*s)->executable = (SotnDataView){f->exe, sizeof(f->exe)};
    (*s)->map =
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 16, 16};
    (*s)->player = (SotnPlayerState){.x = 160 * 65536, .y = 160 * 65536};
    (*s)->slots[64] = (Actor){.slot = 64,
                              .state = {.id = id,
                                        .generation = 1,
                                        .x = 128 * 65536 + 123,
                                        .y = (id == 0x4a ? 176 : 168) * 65536 + 456}};
    return true;
}
static bool Armor(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s, 0x4a));
    Actor *a = &s->slots[64], *weapon = &s->slots[65];
    CHECK(ActorLibraryFleaArmor(s, a) && a->state.step == 1 && a->state.blend_mode == 0x30 &&
          weapon->state.id == 0x4b);
    CHECK(weapon->state.x == 128 * 65536 && weapon->state.y == 176 * 65536 &&
          weapon->enemy_state.owner_generation == 1);
    CHECK(ActorLibraryFleaArmor(s, weapon) && weapon->state.hitbox_width == 0);
    a->state.frame = 9;
    CHECK(ActorLibraryFleaArmor(s, weapon) && weapon->state.hitbox_width == 7 &&
          weapon->state.hitbox_height == 11 && weapon->state.hitbox_x == -10);
    CHECK(ActorLibraryFleaArmor(s, a) && a->state.step == 2);
    CHECK(ActorLibraryFleaArmor(s, a) && a->state.step == 3 && a->vx == 2 * 65536 &&
          a->vy == -32768);
    a->state.flags |= 0x100U;
    CHECK(ActorLibraryFleaArmor(s, a) && a->state.id == 0x4c && a->state.step == 0 &&
          a->enemy_state.timer == 16 && a->state.hit_points == 32767);
    CHECK(s->combat.sounds[0] == 0x6ca && s->slots[166].state.id == 2 &&
          s->slots[166].state.parameters == 1);
    for (Uint16 i = 0; i < 6; ++i)
    {
        Actor *p = &s->slots[160 + i];
        CHECK(p->state.id == 0x4a && p->state.step == 9 && p->state.frame == 10 + i &&
              p->state.flags == 0xcd002000U && p->state.rotated == (i == 5));
        CHECK(ActorLibraryFleaArmor(s, p) && p->state.rotation == 256 && p->vy == -114688);
    }
    CHECK(ActorLibraryFleaArmor(s, weapon) && weapon->remove);
    CHECK(ActorFlea(s, a) && a->state.step == 3 && a->enemy_state.timer == 15);
    // LIB has independent floor/side tables; poisoned NO0 offsets must never be consulted.
    a->state.step = 4;
    a->state.x = 128 * 65536;
    a->state.y = 176 * 65536;
    a->vx = a->vy = 0;
    Put16(f->stage + 0x2e1a, 16);
    CHECK(ActorFlea(s, a) && a->state.step == 3 && Pixel(a->state.y) == 176);
    s->library_enemy_flags = 1;
    CHECK(ActorFlea(s, a) && a->state.id == 2);
    SotnActors_Destroy(s);
    CHECK(Open(f, &s, 0x4a));
    a = &s->slots[64];
    a->state.step = 4;
    CHECK(ActorLibraryFleaArmor(s, a) && a->substep == 1 && s->combat.sound_count == 1 &&
          (s->combat.sounds[0] == 0x649 || s->combat.sounds[0] == 0x64a));
    SotnActors_Destroy(s);
    CHECK(Open(f, &s, 0x4a));
    a = &s->slots[64];
    s->library_enemy_flags = 1;
    for (Uint16 i = 160; i < 192; ++i)
        s->slots[i].state.generation = 99;
    CHECK(ActorLibraryFleaArmor(s, a) && a->state.id == 0x4c && a->state.step == 0);
    CHECK(!ActorLibraryFleaArmor(NULL, a) && !ActorLibraryFleaArmor(s, NULL));
    SotnActors_Destroy(s);
    return true;
}
static bool Dhuron(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s, 0x33));
    Actor *a = &s->slots[64], *weapon = &s->slots[65];
    CHECK(ActorLibraryDhuron(s, a) && a->state.step == 1 && weapon->state.id == 0x34 &&
          a->state.hitbox_x == -4);
    CHECK(ActorLibraryDhuron(s, weapon) && weapon->state.step == 1);
    CHECK(ActorLibraryDhuron(s, weapon) && weapon->state.hitbox_x == -28 &&
          weapon->state.hitbox_height == 4);
    CHECK(ActorLibraryDhuron(s, a) && a->state.step == 2);
    CHECK(ActorLibraryDhuron(s, a) && a->state.step == 3);
    CHECK(ActorLibraryDhuron(s, a) && a->vx == -163840 && !a->dhuron.direction);
    a->state.step = 5;
    a->substep = 1;
    a->dhuron.timer = 1;
    CHECK(ActorLibraryDhuron(s, a) && a->state.step == 3 && a->dhuron.charged &&
          weapon->dhuron.charged && a->state.palette == 0x202);
    CHECK(ActorLibraryDhuron(s, weapon) && weapon->attack == 24 &&
          weapon->attack_element == 0x4000 && !weapon->dhuron.charged);
    CHECK(ActorLibraryDhuron(s, weapon) && weapon->attack == 24);
    s->service_ticks = 2;
    a->state.step = 2;
    CHECK(ActorLibraryDhuron(s, a) && a->state.palette == 0x201);
    a->state.step = 6;
    a->substep = 0;
    a->pose = a->remaining = 0;
    CHECK(ActorLibraryDhuron(s, a) && a->substep == 0);
    CHECK(ActorLibraryDhuron(s, a) && a->substep == 1 && a->dhuron.timer == 8 && a->pose == 0);
    CHECK(ActorLibraryDhuron(s, a));
    CHECK(ActorLibraryDhuron(s, a) && a->dhuron.timer == 7);
    a->state.flags |= 0x100U;
    CHECK(ActorLibraryDhuron(s, a) && a->remove && a->state.hitbox_state == 0 &&
          s->combat.sounds[s->combat.sound_count - 1] == 0x629);
    for (Uint16 i = 0; i < 7; ++i)
    {
        Actor *p = &s->slots[160 + i];
        CHECK(p->state.id == 0x36 && p->state.parameters == i);
        CHECK(ActorLibraryDhuron(s, p) && p->state.frame == 27 + i && p->dhuron.timer == 2);
        CHECK(ActorLibraryDhuron(s, p) && p->state.rotation == 64);
        CHECK(ActorLibraryDhuron(s, p) && p->state.id == 2 && p->state.parameters == 0 &&
              !p->state.rotated);
    }
    CHECK(s->slots[167].state.id == 2 && s->slots[167].state.parameters == 3);
    a->state.id = 0;
    CHECK(ActorLibraryDhuron(s, weapon) && weapon->remove);
    a->state.id = 0x33;
    a->state.step = 2;
    a->state.flags = 0;
    a->state.frame = 28;
    CHECK(!ActorLibraryDhuron(s, a));
    SotnActors_Destroy(s);
    return true;
}
static bool Charge(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s, 0x33));
    Actor *root = &s->slots[64], *effect = &s->slots[160];
    root->state.priority = 136;
    *effect = (Actor){.slot = 160,
                      .state = {.id = 0x35,
                                .generation = 100,
                                .x = root->state.x,
                                .y = root->state.y,
                                .priority = 135}};
    effect->enemy_state.owner_slot = 64;
    effect->enemy_state.owner_generation = 1;
    CHECK(ActorLibraryDhuron(s, effect) && effect->state.primitive_count == 40 &&
          effect->state.step == 1);
    ActorPrimitive *p = ActorPrimitiveGet(s, effect, 0);
    CHECK(p && !p->state.active && p->state.priority == 134 && p->state.page == 0x3a &&
          p->state.offset_y[3] == -16);
    CHECK(ActorLibraryDhuron(s, effect) && effect->dhuron.beam_index == 1);
    p = ActorPrimitiveGet(s, effect, 1);
    CHECK(p && p->state.active && p->state.blend && p->state.quad && p->state.red == 80);
    effect->dhuron.beam_index = 39;
    CHECK(ActorLibraryDhuron(s, effect) && effect->state.step == 2);
    CHECK(ActorLibraryDhuron(s, effect) && effect->state.step == 3 && root->dhuron.charged &&
          effect->state.palette == 0x815b && effect->state.opacity == 128 &&
          effect->state.scale_y == 384);
    CHECK(!ActorPrimitiveGet(s, effect, 1)->state.active);
    CHECK(ActorLibraryDhuron(s, effect) && effect->state.step == 3);
    CHECK(ActorLibraryDhuron(s, effect) && effect->state.step == 4);
    for (Uint16 i = 0; i < 16; ++i)
        CHECK(ActorLibraryDhuron(s, effect));
    CHECK(effect->remove && effect->state.opacity == 0);
    effect->remove = false;
    effect->state.step = 1;
    ++root->state.generation;
    CHECK(ActorLibraryDhuron(s, effect) && effect->remove);
    SotnActors_Destroy(s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    const bool ok = f && Armor(f) && Dhuron(f) && Charge(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
