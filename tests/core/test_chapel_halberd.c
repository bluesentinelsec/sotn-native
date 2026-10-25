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
    Uint8 dra[0x1000], stage[0x2400], tiles[32 * 32 * 2], types[2];
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
static void Animation(Fixture *f, size_t at, Uint16 count, bool loop)
{
    for (Uint16 i = 0; i < count; ++i)
    {
        f->stage[at + i * 2U] = 1;
        f->stage[at + i * 2U + 1] = (Uint8)(i + 1);
    }
    f->stage[at + count * 2U] = (Uint8)(loop ? 0 : 255);
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 30);
    Put16(f->dra + 0x20c, 3);
    f->dra[0x222] = 8;
    f->dra[0x223] = 16;
    Put16(f->stage + 0xa28, 0x8007);
    Put16(f->stage + 0xa2a, 1);
    Animation(f, 0x1d40, 6, true);
    Animation(f, 0x1d50, 6, true);
    Animation(f, 0x1d60, 6, true);
    Animation(f, 0x1d70, 29, false);
    Animation(f, 0x1dac, 4, false);
    Animation(f, 0x1db8, 5, false);
    for (Uint16 i = 0; i < 9; ++i)
    {
        Put16(f->stage + 0x1dc4 + i * 2U, i * 17 + 3);
        f->stage[0x1dd8 + i] = (Uint8)(i + 3);
        Put32(f->stage + 0x1de4 + i * 4U, i * 4096U + 8192U);
        Put32(f->stage + 0x1e08 + i * 4U, (Uint32)(-(Sint32)(i + 1) * 16384));
        Put16(f->stage + 0x1e2c + i * 2U, (Sint32)i - 4);
        Put16(f->stage + 0x1e40 + i * 2U, (Sint32)i * 2 - 8);
    }
    for (Uint16 i = 0; i < 8; ++i)
        f->stage[0x1e54 + i] = (Uint8)(11 + i);
    // Original synthetic room and sensors, deliberately different from runtime arrays.
    Put16(f->stage + 0x1e5e, 16);
    Put16(f->stage + 0x1e62, 2);
    Put16(f->stage + 0x1e64, 6);
    Put16(f->stage + 0x1e66, -2);
    Put16(f->stage + 0x1e68, -12);
    Put16(f->stage + 0x1e6e, 16);
    Put16(f->stage + 0x1e70, 6);
    Put16(f->stage + 0x1e74, -6);
    Put16(f->stage + 0x1e76, 12);
    Put16(f->stage + 0x1e7a, -12);
    Put16(f->stage + 0x1e7e, -12);
    for (Uint16 i = 1; i < 8; ++i)
    {
        Put16(f->stage + 0x1e80 + i * 8U, -(10 + (Sint32)i));
        Put16(f->stage + 0x1e82 + i * 8U, -(1 + (Sint32)i));
        Put16(f->stage + 0x1e84 + i * 8U, i + 2);
        Put16(f->stage + 0x1e86 + i * 8U, i + 3);
    }
    f->types[1] = 3;
    for (Uint32 i = 32 * 10; i < 32 * 32; ++i)
        f->tiles[i * 2] = 1;
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 6, out));
    (*out)->map =
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 32};
    (*out)->player = (SotnPlayerState){.x = 240 * 65536, .y = 144 * 65536};
    (*out)->slots[16] = (Actor){
        .slot = 16, .state = {.id = 0x32, .generation = 1, .x = 128 * 65536, .y = 144 * 65536}};
    return true;
}
static bool Movement(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[16], *weapon = &s->slots[17];
    a->state.x += 123;
    a->state.y += 456;
    CHECK(ActorChapel(s, a) && a->state.step == 1 && a->halberd.timer == 64 &&
          weapon->state.id == 0x33 && weapon->state.x == 128 * 65536 &&
          weapon->state.y == 144 * 65536 && !weapon->state.facing_left);
    a->state.frame = 20;
    a->state.facing_left = true;
    CHECK(ActorChapel(s, weapon) && weapon->state.x == a->state.x &&
          weapon->state.y == a->state.y && weapon->state.hitbox_x == -17 &&
          weapon->state.hitbox_y == -8 && weapon->state.hitbox_width == 9 &&
          weapon->state.hitbox_height == 10 && weapon->state.hitbox_state == 1);
    CHECK(ActorChapel(s, a) && a->state.step == 2);
    const Sint32 x = a->state.x;
    CHECK(ActorChapel(s, a) && a->state.x == x + 32768 && a->vx == 32768 && a->halberd.timer == 63);
    s->player.x = a->state.x + 48 * 65536;
    CHECK(ActorChapel(s, a) && a->state.step == 3 && a->vx == 32768);
    CHECK(ActorChapel(s, a) && a->vx == -32768);
    s->player.x = a->state.x + 72 * 65536;
    a->halberd.timer = 0;
    CHECK(ActorChapel(s, a) && a->state.step == 4);
    for (Uint16 i = 0; i < 30; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(a->state.step == 3 && a->halberd.interval == 1 && a->halberd.timer == 12 &&
          s->combat.sound_count == 2 && s->combat.sounds[0] == 0x6ba &&
          s->combat.sounds[1] == 0x62e);
    SotnActors_Destroy(s);
    return true;
}
static bool LungeJump(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[16];
    CHECK(ActorChapel(s, a));
    SetStep(a, 7);
    CHECK(ActorChapel(s, a) && a->substep == 1 && a->halberd.lunge_timer == 192);
    s->player.x = a->state.x + 40 * 65536;
    a->vx = 32768;
    CHECK(ActorChapel(s, a) && a->substep == 2 && a->halberd.lunge_timer == 1 && !a->pose &&
          !a->remaining && a->state.x == 128 * 65536 + 32768 && a->vx == 131072);
    a->halberd.lunge_timer = 2;
    for (Uint16 i = 0; i < 5; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(!s->combat.sound_count);
    CHECK(ActorChapel(s, a) && s->combat.sounds[0] == 0x6ba);
    for (Uint16 i = 0; i < 100 && a->state.step == 7; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(a->state.step == 3 && a->halberd.timer == 12);
    SetStep(a, 5);
    a->halberd.movement_facing = false;
    s->random = 0;
    for (Uint16 i = 0; i < 5; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(a->substep == 1 && a->vx == 131072 && a->vy == -196608);
    for (Uint16 i = 0; i < 100 && a->state.step == 5; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(a->state.step == 3);
    SotnActors_Destroy(s);
    return true;
}
static bool DeathOwnership(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[16], *weapon = &s->slots[17];
    CHECK(ActorChapel(s, a));
    const Uint32 previous = weapon->state.generation;
    a->state.generation = 2;
    a->state.step = 0;
    CHECK(ActorChapel(s, a) && weapon->state.generation != previous &&
          weapon->enemy_state.owner_generation == 2);
    a->state.flags |= 0x100U;
    a->state.facing_left = true;
    CHECK(ActorChapel(s, a) && a->remove && weapon->remove && s->combat.sounds[0] == 0x62a);
    for (Uint16 i = 0; i < 9; ++i)
    {
        Actor *part = &s->slots[160 + i];
        CHECK(part->state.id == 0x34 && part->timer == i + 3 &&
              part->state.x == (132 - i) * 65536 && part->state.y == (136 + i * 2) * 65536);
        CHECK(ActorChapel(s, part) && part->state.frame == i + 32 && !part->state.hitbox_state &&
              part->vx == -(Sint32)(i * 4096U + 8192U) && part->state.rotated);
        for (Uint16 t = 0; t < i + 3; ++t)
            CHECK(ActorChapel(s, part));
        CHECK(part->state.id == 2 && !part->state.step && part->state.frame == i + 32 &&
              part->state.rotated);
    }
    a->state.flags = 0;
    a->state.generation = 3;
    a->state.step = 0;
    a->remove = false;
    weapon->state.id = 0x35;
    CHECK(!ActorChapel(s, a) && weapon->state.id == 0x35);
    Actor orphan = {.state = {.id = 0x33},
                    .enemy_state = {.owner_slot = 16, .owner_generation = 2}};
    CHECK(ActorChapel(s, &orphan) && orphan.remove);
    Actor *out = NULL;
    CHECK(!ActorOwnedSpawnAt(NULL, a, 0x33, 0, 17, &out));
    CHECK(!ActorOwnedSpawnAt(s, a, 0x33, 0, 16, &out));
    weapon->state.id = 0x33;
    const Uint32 retained = weapon->state.generation;
    s->effect_serial = 0x7fffffffU;
    CHECK(!ActorOwnedSpawnAt(s, a, 0x33, 0, 17, &out) && weapon->state.generation == retained);
    CHECK(!ActorChapelHalberd(NULL, a) && !ActorChapelHalberd(s, NULL));
    SotnActors_Destroy(s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    const bool ok = f && Movement(f) && LungeJump(f) && DeathOwnership(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_CHAPEL_HALBERD: PASS");
    return 0;
}
