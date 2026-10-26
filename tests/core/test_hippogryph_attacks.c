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
    Uint8 dra[0x1000], stage[0x2000], tiles[32 * 16 * 2], types[2];
} Fixture;
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static void Put32(Uint8 *p, Uint32 value)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(value >> (i * 8U));
}
static void Animation(Fixture *f, size_t offset, Uint8 frame, Uint8 duration, bool loop)
{
    f->stage[offset] = duration;
    f->stage[offset + 1] = frame;
    f->stage[offset + 2] = loop ? 0 : 255;
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 10);
    Put16(f->dra + 0x206, 8);
    Put16(f->dra + 0x208, 64);
    Put16(f->dra + 0x20c, 3);
    f->dra[0x222] = 8;
    f->dra[0x223] = 16;
    const size_t init[] = {0x46c, 0x484, 0x4e4, 0x4f0};
    for (Uint16 i = 0; i < SDL_arraysize(init); ++i)
        Put16(f->stage + init[i], 0x8003);
    // Synthetic sprite/duration/selector records, not copies of original runtime arrays.
    for (Uint16 i = 0; i < 104; ++i)
        f->stage[0xb0c + i] = (Uint8)(i % 7);
    Animation(f, 0x9f0, 11, 2, false);
    Animation(f, 0x9f8, 12, 1, true);
    Animation(f, 0xa00, 13, 1, true);
    Animation(f, 0xa0c, 14, 3, false);
    Animation(f, 0xa18, 15, 4, false);
    f->types[1] = 3;
    for (Uint32 i = 32 * 12; i < 32 * 16; ++i)
        f->tiles[i * 2] = 1;
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 25, out));
    (*out)->map =
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16};
    (*out)->player = (SotnPlayerState){.x = 192 * 65536, .y = 135 * 65536};
    return true;
}
static bool Metadata(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    const SotnStageProfile *p = SotnStageProfile_Get(25);
    CHECK(p && p->connected && p->actor_limit == 0x2f && p->prize_count == 1 && p->prize_bit == 0 &&
          p->sprite_banks == 24 && !SotnStageProfile_SaveRoom(25, 0));
    CHECK(ActorStageOffset(s, 0xbe0) == 0x46c && ActorStageOffset(s, 0xc10) == 0x49c &&
          ActorStageOffset(s, 0xdbc) == 0x544 && ActorStageOffset(s, 0xdc0) == 0x54c &&
          ActorStageOffset(s, 0xdc4) == 0x554 && ActorStageOffset(s, 0x1cc4) == 0x14b0 &&
          ActorStageOffset(s, 0x13b0) == 0xb78);
    CHECK(ActorStageOffset(s, 0x777) == sizeof(f->stage));
    SotnActors_Destroy(s);
    return true;
}
static bool Weapon(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *body = &s->slots[20], *weapon = NULL;
    *body = (Actor){.state = {.id = 0x22,
                              .generation = 3,
                              .frame = 5,
                              .x = 100 * 65536 + 99,
                              .y = 125 * 65536 + 234,
                              .facing_left = true},
                    .slot = 20};
    CHECK(ActorOwnedSpawnAt(s, body, 0x23, 0, 21, &weapon));
    CHECK(ActorHippogryphAttack(s, weapon) && weapon->state.hit_points == 0x7ffe &&
          weapon->state.hitbox_state == 2 && weapon->state.hitbox_x == 16 &&
          weapon->state.hitbox_y == -32 && weapon->state.hitbox_width == 22 &&
          weapon->state.hitbox_height == 22 && weapon->state.x == 100 * 65536 &&
          weapon->state.y == 125 * 65536 && weapon->state.facing_left && !weapon->attack &&
          !weapon->attack_element);
    body->state.frame = 6;
    body->state.x += 3 * 65536;
    CHECK(ActorHippogryphAttack(s, weapon) && weapon->state.hitbox_x == 2 &&
          weapon->state.hitbox_width == 12 && weapon->state.hitbox_height == 24 &&
          weapon->state.x == 103 * 65536);
    f->stage[0xb12] = 7;
    CHECK(ActorHippogryphAttack(s, weapon) && weapon->state.hitbox_height == 24);
    body->state.frame = 104;
    CHECK(ActorHippogryphAttack(s, weapon) && !weapon->state.hitbox_width &&
          !weapon->state.hitbox_height);
    body->state.generation = 4;
    CHECK(ActorHippogryphAttack(s, weapon) && weapon->remove);
    SotnActors_Destroy(s);
    return true;
}
static bool Flames(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    for (Uint16 param = 0; param < 4; ++param)
    {
        Actor a = {.state = {.id = 0x27,
                             .generation = 1,
                             .parameters = param,
                             .x = 256 * 65536,
                             .y = 100 * 65536,
                             .facing_left = param != 0}};
        CHECK(ActorHippogryphAttack(s, &a) && a.state.step == 1 && a.state.animation_set == 2 &&
              a.state.frame == 1 && a.state.rotation == 0xe00 && a.state.palette == 0 &&
              a.state.scale_x == 32 && a.state.scale_y == 32 &&
              a.state.hitbox_state == (param == 3 ? 1 : 0));
        const Sint32 vx = a.vx, vy = a.vy;
        const Uint32 random = s->random;
        CHECK(ActorHippogryphAttack(s, &a) && a.state.x == 256 * 65536 + vx &&
              a.state.y == 100 * 65536 + vy && a.vy == vy - 5120 && a.state.scale_x == 56 &&
              a.state.opacity == (param >= 2 ? 119 : 123));
        CHECK(s->random == (param == 0 || param == 3 ? random * 0x01010101U + 1U : random));
        for (Uint16 i = 1; i < 9; ++i)
            CHECK(ActorHippogryphAttack(s, &a));
        CHECK(a.state.step == 2 && a.state.scale_x == 248 && a.vx == vx / 4 && !a.pose &&
              !a.remaining);
        for (Uint16 i = 0; i < 4; ++i)
            CHECK(ActorHippogryphAttack(s, &a));
        CHECK(a.remove);
    }
    Actor a = {.state = {.id = 0x27, .generation = 2}};
    s->hippogryph_flags = 2;
    CHECK(ActorHippogryphAttack(s, &a) && a.remove && !a.state.step);
    SotnActors_Destroy(s);
    return true;
}
static bool Particle(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = {.state = {.id = 0x26,
                         .generation = 1,
                         .parameters = 3,
                         .facing_left = true,
                         .x = 100 * 65536,
                         .y = 90 * 65536,
                         .scale_x = 200,
                         .scale_y = 200}};
    CHECK(ActorHippogryphAttack(s, &a) && a.vx == 65536 && a.vy == 32768 &&
          a.state.scale_x == 200 && a.state.opacity == 128 && a.state.rotated);
    CHECK(ActorHippogryphAttack(s, &a) && a.state.x == 101 * 65536 &&
          a.state.y == 90 * 65536 + 32768 && a.vx == 57344 && a.vy == 28672 &&
          a.state.rotation == -64 && a.state.scale_x == 190 && a.state.scale_y == 200 &&
          a.state.opacity == 122);
    for (Uint16 i = 0; i < 4; ++i)
        CHECK(ActorHippogryphAttack(s, &a));
    CHECK(a.remove);
    SotnActors_Destroy(s);
    return true;
}
static bool FlyingObject(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = {.state = {.id = 0x25, .generation = 1, .x = 100 * 65536, .y = 80 * 65536}};
    CHECK(ActorHippogryphAttack(s, &a) && a.state.step == 1 && a.state.rotated &&
          a.hippogryph.rotation_direction == 1 && a.state.hitbox_state == 2);
    a.hippogryph.timer = 8;
    CHECK(ActorHippogryphAttack(s, &a) && a.state.step == 1 && a.hippogryph.timer == 7 &&
          a.vy == 16384 && (a.state.flags & 0x10000000U));
    a.state.step = 2;
    a.hippogryph.timer = 2;
    const Sint16 rotation = a.state.rotation;
    CHECK(ActorHippogryphAttack(s, &a) && a.hippogryph.timer == 1 &&
          a.state.rotation == rotation + 512 && a.hippogryph.rotation_direction == 255);
    CHECK(ActorHippogryphAttack(s, &a) && !a.hippogryph.timer && a.state.rotation == rotation);
    for (Uint16 i = 0; i < 3; ++i)
        CHECK(ActorHippogryphAttack(s, &a));
    CHECK(a.state.step == 3 && !a.state.rotated && a.state.hitbox_state == 3 && a.vx == 163840 &&
          a.vy == -16384 && s->combat.sound_count == 1 && s->combat.sounds[0] == 0x7b7);
    CHECK(ActorHippogryphAttack(s, &a) && a.state.x == 100 * 65536 + 163840 &&
          a.state.y == 80 * 65536 - 16384 && a.vy == -18944);
    s->hippogryph_flags = 2;
    CHECK(ActorHippogryphAttack(s, &a) && a.remove);
    CHECK(s->slots[96].state.id == 2 && s->slots[97].state.id == 2);
    Actor fire = {.state = {.id = 0x24, .generation = 2}};
    CHECK(ActorHippogryphAttack(s, &fire) && fire.state.hitbox_x == -48 &&
          fire.state.hitbox_y == -2 && fire.state.hitbox_width == 24 &&
          fire.state.hitbox_height == 4);
    CHECK(!ActorHippogryphAttack(NULL, &fire) && !ActorHippogryphAttack(s, NULL));
    fire.state.id = 0x22;
    CHECK(!ActorHippogryphAttack(s, &fire));
    SotnActors_Destroy(s);
    return true;
}
static bool FailuresAndCapacity(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    for (Uint16 i = 96; i < 128; ++i)
        s->slots[i] = (Actor){.state = {.id = 1, .generation = 50}};
    for (Uint16 i = 160; i < 192; ++i)
        s->slots[i] = (Actor){.state = {.id = 1, .generation = 51}};
    Actor a = {.state = {.id = 0x25, .generation = 1, .flags = 0x100}};
    CHECK(ActorHippogryphAttack(s, &a) && a.remove);
    s->random = 0x12345678;
    a = (Actor){.state = {.id = 0x27, .generation = 2, .x = 100 * 65536, .y = 80 * 65536}};
    CHECK(ActorHippogryphAttack(s, &a) && ActorHippogryphAttack(s, &a) && a.state.step == 1 &&
          s->random == 0x12345678U * 0x01010101U + 1U);
    for (Uint16 i = 96; i < 128; ++i)
        CHECK(s->slots[i].state.generation == 50);
    for (Uint16 i = 160; i < 192; ++i)
        CHECK(s->slots[i].state.generation == 51);
    a = (Actor){.state = {.id = 0x26, .generation = 3, .step = 1, .x = SDL_MAX_SINT32}, .vx = 1};
    CHECK(!ActorHippogryphAttack(s, &a) && a.state.x == SDL_MAX_SINT32);
    a.state.x = 0;
    a.vx = 0;
    const size_t size = s->nz0.size;
    s->nz0.size = 0xa18;
    CHECK(!ActorHippogryphAttack(s, &a));
    s->nz0.size = size;
    a = (Actor){.state = {.id = 0x24, .generation = 4}};
    Put16(f->stage + 0x4f8, 400);
    CHECK(!ActorHippogryphAttack(s, &a));
    s->stage_id = 6;
    CHECK(!ActorHippogryphAttack(s, &a));
    SotnActors_Destroy(s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Metadata(f) && Weapon(f) && Flames(f) && Particle(f) && FlyingObject(f) &&
                    FailuresAndCapacity(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_HIPPOGRYPH_ATTACKS: PASS");
    return 0;
}
