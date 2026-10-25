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
    Uint8 stage[0x4000], dra[0x1000], tiles[16 * 16 * 2], types[2];
} Fixture;
static void Put16(Uint8 *p, Sint32 v)
{
    const Uint16 b = (Uint16)v;
    p[0] = (Uint8)b;
    p[1] = (Uint8)(b >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (8U * i));
}
static bool Open(Fixture *f, SotnActors *s, bool right)
{
    SDL_zero(*f);
    SDL_zero(*s);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 40);
    Put16(f->dra + 0x206, 8);
    Put16(f->dra + 0x20c, 3);
    f->dra[0x222] = 8;
    f->dra[0x223] = 16;
    Put16(f->stage + 0x9e6, 512);
    Put16(f->stage + 0x2e72, 16);
    Put16(f->stage + 0x2e82, 16);
    Put16(f->stage + 0x2e84, 4);
    Put16(f->stage + 0x2d16, 16);
    f->stage[0x2e3c] = 2;
    f->stage[0x2e3d] = 1;
    f->stage[0x2e4c] = 1;
    f->stage[0x2e4d] = 7;
    f->stage[0x2e4e] = 1;
    f->stage[0x2e4f] = 14;
    f->stage[0x2e50] = 1;
    f->stage[0x2e51] = 15;
    f->stage[0x2e52] = 255;
    for (Uint16 i = 0; i < 4; ++i)
    {
        Put32(f->stage + 0x2e88 + i * 8U, (Uint32)(-32768 + (Sint32)i * 16384));
        Put32(f->stage + 0x2e8c + i * 8U, (Uint32)(-65536 - (Sint32)i * 8192));
    }
    for (Uint16 i = 1; i < 3; ++i)
    {
        Put16(f->stage + 0x2ea8 + i * 4U, -(Sint32)i * 5);
        Put16(f->stage + 0x2eaa + i * 4U, -7);
        Put16(f->stage + 0x2eb4 + i * 4U, i * 9);
        Put16(f->stage + 0x2eb6 + i * 4U, 3);
    }
    for (Uint32 i = 16 * 10; i < 16 * 16; ++i)
        f->tiles[i * 2] = 1;
    f->types[1] = 3;
    s->stage_id = 2;
    s->nz0 = (SotnDataView){f->stage, sizeof(f->stage)};
    s->dra = (SotnDataView){f->dra, sizeof(f->dra)};
    s->map = (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 16, 16};
    s->player = (SotnPlayerState){.x = (right ? 180 : 16) * 65536, .y = 144 * 65536, .hp = 70};
    s->slots[64] = (Actor){.slot = 64,
                           .state = {.id = 0x4d,
                                     .generation = 1,
                                     .parameters = 16,
                                     .x = 96 * 65536 + 123,
                                     .y = 128 * 65536 + 456}};
    return SotnRoomActors_Create(&s->spawns);
}
static bool Lifecycle(Fixture *f, SotnActors *s, bool right)
{
    CHECK(Open(f, s, right));
    Actor *a = &s->slots[64];
    CHECK(ActorLibrary(s, a) && a->state.step == 1 && a->state.priority == 120 &&
          !a->state.hitbox_state && a->state.facing_left == right);
    Actor *weapon = &s->slots[0];
    CHECK(weapon->state.id == 0x4e && weapon->enemy_state.owner_generation == 1 &&
          weapon->state.x == 96 * 65536);
    for (Uint32 i = 0; i < 30 && a->state.step == 1; ++i)
        CHECK(ActorLibrary(s, a));
    CHECK(a->state.step == 2 && a->state.primitive_count == 1);
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    CHECK(p && p->state.gouraud && p->state.offset_x[0] == 1 && p->state.offset_y[0] == 20);
    bool emerged = false;
    for (Uint32 i = 0; i < 240 && a->state.step < 6; ++i)
    {
        s->service_ticks = i;
        CHECK(ActorLibrary(s, a));
        emerged |= a->state.step == 3;
    }
    CHECK(emerged && a->state.step == 6 && a->state.primitive_count == 0 &&
          a->vx == (right ? 16384 : -16384));
    CHECK(ActorLibrary(s, a) && a->state.hitbox_state == 3);
    s->player.x = (Pixel(a->state.x) + 32) * 65536;
    CHECK(ActorLibrary(s, a) && a->state.step == 7);
    a->state.frame = 14;
    CHECK(ActorLibrary(s, weapon) && weapon->state.hitbox_x == -5 && weapon->state.hitbox_y == -7 &&
          weapon->state.hitbox_width == 9 && weapon->state.hitbox_height == 3 &&
          weapon->state.hitbox_state == 3);
    a->state.frame = 15;
    CHECK(ActorLibrary(s, weapon) && weapon->state.hitbox_width == 18);
    a->state.frame = 7;
    CHECK(ActorLibrary(s, weapon) && !weapon->state.hitbox_width && !weapon->state.hitbox_height);
    for (Uint16 i = 0; i < 8 && a->state.step == 7; ++i)
        CHECK(ActorLibrary(s, a));
    CHECK(a->state.step == 6 && s->combat.sound_count == 1 && s->combat.sounds[0] == 0x78c);
    a->state.step = 8;
    CHECK(ActorLibrary(s, a) && a->state.step == 9 && !a->state.frame && !a->state.hitbox_state);
    p = ActorPrimitiveGet(s, a, 0);
    CHECK(p && p->state.offset_x[0] == (right ? 17 : -15) && p->state.offset_y[0] == -19);
    for (Uint16 i = 0; i < 40; ++i)
    {
        s->service_ticks = (Uint32)i * 2U;
        CHECK(ActorLibrary(s, a));
    }
    CHECK(a->remove && a->mudman.counter == 40 && p->state.offset_y[0] == 19);
    a->state.generation = 0;
    CHECK(ActorLibrary(s, weapon) && weapon->remove);
    SotnRoomActors_Destroy(s->spawns);
    return true;
}
static bool Death(Fixture *f, SotnActors *s)
{
    CHECK(Open(f, s, true));
    Actor *a = &s->slots[64];
    CHECK(ActorLibrary(s, a));
    a->state.animation_set = 0x800e;
    a->state.palette = 0x819f;
    s->library_enemy_flags = 2;
    CHECK(ActorLibrary(s, a) && a->remove && a->state.step == 10);
    for (Uint16 i = 0; i < 4; ++i)
    {
        Actor *piece = &s->slots[160 + i];
        CHECK(piece->state.id == 0x4d && piece->state.step == 11 && piece->state.frame >= 16 &&
              piece->state.frame <= 17 && piece->state.palette == 512 &&
              piece->state.animation_set == 0x800e && piece->state.flags == 0xcd002000U &&
              !piece->state.facing_left && !piece->state.hitbox_state &&
              piece->state.parameters == 0);
        const Sint32 x = piece->state.x, y = piece->state.y, vx = piece->vx, vy = piece->vy;
        CHECK(ActorLibrary(s, piece) && piece->state.x == x + vx && piece->state.y == y + vy &&
              piece->vy == vy + 16384);
    }
    CHECK(s->slots[164].state.id == 2 && s->slots[164].state.parameters == 2 &&
          s->combat.sounds[0] == 0x62b);
    s->slots[160].state.x = SDL_MAX_SINT32;
    s->slots[160].vx = 1;
    CHECK(!ActorLibrary(s, &s->slots[160]));
    SotnRoomActors_Destroy(s->spawns);
    return true;
}
static bool Skeleton(Fixture *f, SotnActors *s)
{
    CHECK(Open(f, s, true));
    Actor *a = &s->slots[64];
    a->state.id = 0x43;
    CHECK(ActorLibrary(s, a) && a->state.step == 32 && !a->state.frame && !a->state.hitbox_state &&
          a->enemy_state.attack_timer == 80);
    CHECK(ActorLibrary(s, a) && a->substep == 1 && a->state.priority == 120 &&
          a->state.opacity == 2);
    for (Uint16 i = 0; i < 30 && a->substep == 1; ++i)
        CHECK(ActorLibrary(s, a));
    CHECK(a->substep == 2 && a->state.frame == 1);
    for (Uint16 i = 0; i < 40; ++i)
        CHECK(ActorLibrary(s, a));
    CHECK(a->substep == 3 && !a->state.opacity_set && a->summon.timer == 32);
    for (Uint16 i = 0; i < 32; ++i)
        CHECK(ActorLibrary(s, a));
    CHECK(a->state.step == 1 && a->state.hitbox_state == 3 && a->state.palette == 0);
    s->library_enemy_flags = 2;
    CHECK(ActorLibrary(s, a) && a->remove && (a->state.flags & 0x100U) && (a->hit_flags & 3U));
    SotnRoomActors_Destroy(s->spawns);
    return true;
}
static bool Failure(Fixture *f, SotnActors *s)
{
    CHECK(Open(f, s, true));
    Actor *a = &s->slots[64];
    CHECK(ActorLibrary(s, a));
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 99;
    for (Uint16 i = 0; i < 30 && !a->remove; ++i)
        CHECK(ActorLibrary(s, a));
    CHECK(a->remove && !a->state.primitive_count);
    SotnRoomActors_Destroy(s->spawns);
    CHECK(Open(f, s, true));
    a = &s->slots[64];
    CHECK(ActorLibrary(s, a));
    Actor *weapon = &s->slots[0];
    a->state.frame = 14;
    Put16(f->stage + 0x2eb8, 129);
    CHECK(!ActorLibrary(s, weapon));
    ++a->state.generation;
    CHECK(ActorLibrary(s, weapon) && weapon->remove);
    a->state.step = 16;
    CHECK(!ActorLibrary(s, a));
    CHECK(!ActorLibraryMudman(NULL, a) && !ActorLibraryMudman(s, NULL));
    CHECK(!ActorLibrarySummon(NULL, a, 0) && !ActorLibrarySummon(s, NULL, 0));
    a->substep = 0;
    a->state.priority = 15;
    CHECK(!ActorLibrarySummon(s, a, 0));
    a->substep = 3;
    a->summon.timer = 0;
    CHECK(!ActorLibrarySummon(s, a, 0));
    SotnRoomActors_Destroy(s->spawns);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    SotnActors *s = SDL_calloc(1, sizeof(*s));
    const bool ok = f && s && Lifecycle(f, s, false) && Lifecycle(f, s, true) && Death(f, s) &&
                    Skeleton(f, s) && Failure(f, s);
    SDL_free(s);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_LIBRARY_MINION_TEST: PASS");
    return 0;
}
