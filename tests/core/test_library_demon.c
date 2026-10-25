#include "library_demon_internal.h"
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
    Uint8 stage[0x4000], dra[0x1000], exe[0x23220], tiles[80 * 48 * 2], types[2];
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
        p[i] = (Uint8)(v >> (i * 8U));
}
static void Stream(Uint8 *p, Uint16 count, bool loop)
{
    for (Uint16 i = 0; i < count; ++i)
    {
        p[i * 2] = 1;
        p[i * 2 + 1] = (Uint8)(i + 1);
    }
    p[count * 2] = (Uint8)(loop ? 0 : 255);
}
static bool Open(Fixture *f, SotnActors *s)
{
    SDL_zero(*f);
    SDL_zero(*s);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 40);
    Put16(f->dra + 0x206, 12);
    Put16(f->dra + 0x20c, 3);
    f->dra[0x222] = 12;
    f->dra[0x223] = 32;
    Put16(f->stage + 0x96a, 1);
    Put16(f->stage + 0x1ad6, 32);
    Put16(f->stage + 0x1ae6, 32);
    Put16(f->stage + 0x1c3c, 0);
    Put16(f->stage + 0x1c3e, 32);
    Put16(f->stage + 0x1c40, 0);
    Put16(f->stage + 0x1c42, 16);
    Put16(f->stage + 0x1c44, 255);
    Stream(f->stage + 0x1b04, 2, true);
    Stream(f->stage + 0x1b14, 11, false);
    Stream(f->stage + 0x1b2c, 1, false);
    Stream(f->stage + 0x1b34, 2, true);
    Stream(f->stage + 0x1b44, 1, false);
    Stream(f->stage + 0x1b4c, 2, true);
    Stream(f->stage + 0x1b68, 2, true);
    Stream(f->stage + 0x1b84, 3, false);
    Stream(f->stage + 0x1b90, 3, false);
    Stream(f->stage + 0x1b9c, 4, false);
    Stream(f->stage + 0x1bac, 2, true);
    Stream(f->stage + 0x1bbc, 2, false);
    for (Uint16 i = 0; i < 4; ++i)
        Put16(f->stage + 0x1c34 + i * 2U, 4);
    for (Uint16 i = 0; i < 7; ++i)
        for (Uint16 j = 0; j < 3; ++j)
            f->stage[0x1c04 + i * 3 + j] = (Uint8)(i * 10 + j * 20);
    for (Uint32 i = 0; i <= 1024; ++i)
    {
        Put16(f->exe + 0x1dbcc + i * 2, (Sint32)SDL_round(SDL_sin(i * SDL_PI_D / 2048) * 4096));
        Put16(f->exe + 0x22a10 + i * 2, (Sint32)SDL_round(SDL_atan(i / 1024.0) * 2048 / SDL_PI_D));
    }
    for (Uint32 i = 0; i < 4096; ++i)
    {
        Put16(f->exe + 0x1ea10 + i * 4, (Sint32)SDL_round(SDL_sin(i * SDL_PI_D / 2048) * 4096));
        Put16(f->exe + 0x1ea12 + i * 4, (Sint32)SDL_round(SDL_cos(i * SDL_PI_D / 2048) * 4096));
    }
    for (Uint32 i = 80 * 44; i < 80 * 48; ++i)
        f->tiles[i * 2] = 1;
    f->types[1] = 3;
    s->stage_id = 2;
    s->nz0 = (SotnDataView){f->stage, sizeof(f->stage)};
    s->dra = (SotnDataView){f->dra, sizeof(f->dra)};
    s->executable = (SotnDataView){f->exe, sizeof(f->exe)};
    s->map = (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 80, 48};
    s->camera_x = 384;
    s->camera_y = 512;
    s->player = (SotnPlayerState){.x = 450 * 65536, .y = 679 * 65536, .hp = 70};
    s->slots[16] = (Actor){
        .slot = 16, .state = {.id = 0x1f, .generation = 1, .x = 512 * 65536, .y = 672 * 65536}};
    return SotnRoomActors_Create(&s->spawns);
}
static bool Begin(Fixture *f, SotnActors *s)
{
    CHECK(Open(f, s));
    Actor *a = &s->slots[16];
    CHECK(ActorLibrary(s, a) && a->state.step == 1 && a->state.priority == 134 &&
          a->state.hit_points == 40 && a->state.hitbox_y == 2);
    CHECK(ActorLibrary(s, a) && a->state.step == 2);
    CHECK(ActorLibrary(s, a) && a->state.step == 2 && !a->state.hitbox_state);
    s->library_enemy_flags = 1;
    CHECK(ActorLibrary(s, a) && a->state.step == 3 && a->state.hitbox_state == 3);
    return true;
}
static bool Movement(Fixture *f, SotnActors *s)
{
    CHECK(Begin(f, s));
    Actor *a = &s->slots[16];
    for (Uint16 i = 0; i < 48; ++i)
        CHECK(ActorLibrary(s, a));
    CHECK(a->state.step == 8);
    for (Uint16 i = 0; i < 100 && a->state.step == 8; ++i)
        CHECK(ActorLibrary(s, a));
    CHECK(a->state.step == 9 && Pixel(a->state.y) - s->camera_y < 80);
    s->player.x = a->state.x;
    CHECK(ActorLibrary(s, a) && a->substep == 1 && a->demon.timer == 3);
    for (Uint16 i = 0; i < 3; ++i)
        CHECK(ActorLibrary(s, a));
    CHECK(a->state.step == 14);
    SetStep(a, 10);
    CHECK(ActorLibrary(s, a) && a->demon.timer == 96 && SDL_abs(a->vx) == 131072);
    for (Uint16 i = 0; i < 96; ++i)
    {
        s->combat.sound_count = 0;
        CHECK(ActorLibrary(s, a));
    }
    CHECK(a->state.step == 9);
    SetStep(a, 6);
    a->state.y = 672 * 65536;
    a->state.facing_left = true;
    for (Uint16 i = 0; i < 180 && a->state.step == 6; ++i)
    {
        s->combat.sound_count = 0;
        CHECK(ActorLibrary(s, a));
    }
    CHECK(a->state.step == 3 && a->substep == 0);
    a->state.x = 100 * 65536;
    a->state.y = 400 * 65536;
    SetStep(a, 2);
    CHECK(ActorLibrary(s, a) && Pixel(a->state.x) == 312 && Pixel(a->state.y) == 560);
    SotnRoomActors_Destroy(s->spawns);
    return true;
}
static bool PunchAndBolt(Fixture *f, SotnActors *s)
{
    CHECK(Begin(f, s));
    Actor *a = &s->slots[16];
    a->state.y = 592 * 65536;
    SetStep(a, 4);
    bool active = false, effect = false, swipe = false;
    for (Uint16 i = 0; i < 100 && a->state.step == 4; ++i)
    {
        s->combat.sound_count = 0;
        CHECK(ActorLibrary(s, a));
        effect |= a->state.primitive_count == 2;
        for (Uint16 j = 0; j < s->combat.sound_count; ++j)
            swipe |= s->combat.sounds[j] == 0x78f;
        if (a->demon.child_slot < SOTN_STAGE_ACTOR_SLOTS)
        {
            Actor *child = &s->slots[a->demon.child_slot];
            active |= child->state.hitbox_state == 1;
            if (child->state.generation)
                CHECK(ActorLibrary(s, child));
        }
    }
    CHECK(a->state.step == 3 && active && effect && swipe && !a->state.primitive_count &&
          a->demon.child_slot == SDL_MAX_UINT16);
    SetStep(a, 13);
    bool star = false, shot = false;
    for (Uint16 i = 0; i < 12 && a->state.step == 13; ++i)
    {
        s->combat.sound_count = 0;
        CHECK(ActorLibrary(s, a));
        star |= a->state.primitive_count == 2;
        for (Uint16 j = 96; j < 128; ++j)
            shot |= s->slots[j].state.id == 0x21;
    }
    CHECK(a->state.step == 3 && star && shot && !a->state.primitive_count);
    SotnRoomActors_Destroy(s->spawns);
    return true;
}
static bool Summon(Fixture *f, SotnActors *s)
{
    CHECK(Begin(f, s));
    Actor *a = &s->slots[16];
    a->state.y = 592 * 65536;
    SetStep(a, 14);
    for (Uint16 i = 0; i < 4 && a->substep == 0; ++i)
        CHECK(ActorLibrary(s, a));
    CHECK(a->substep == 1 && a->state.hit_points == 32767 && a->demon.saved_hp == 40);
    bool spawn = false, gradient = false;
    for (Uint16 i = 0; i < 128; ++i)
    {
        s->combat.sound_count = 0;
        CHECK(ActorLibrary(s, a));
        if (a->state.primitive_count)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
            gradient |= p->state.gouraud && p->state.quad && !p->state.textured;
        }
        for (Uint16 j = 96; j < 128; ++j)
            spawn |= s->slots[j].state.generation && s->slots[j].state.parameters == 16;
    }
    CHECK(a->substep == 2 && a->state.hit_points == 40 && spawn && gradient);
    for (Uint16 i = 0; i < 100 && a->state.step == 14; ++i)
    {
        s->combat.sound_count = 0;
        CHECK(ActorLibrary(s, a));
    }
    CHECK(a->state.step == 9 && !a->state.primitive_count);
    SotnRoomActors_Destroy(s->spawns);
    return true;
}
static bool SpitAndDeath(Fixture *f, SotnActors *s)
{
    CHECK(Begin(f, s));
    Actor *a = &s->slots[16];
    SetStep(a, 12);
    bool particles = false, shot = false;
    for (Uint16 i = 0; i < 70 && a->state.step == 12; ++i)
    {
        s->combat.sound_count = 0;
        CHECK(ActorLibrary(s, a));
        particles |= a->state.primitive_count == 24;
        for (Uint16 j = 96; j < 128; ++j)
            shot |= s->slots[j].state.id == 0x20;
    }
    CHECK(a->state.step == 9 && particles && shot && !a->state.primitive_count);
    a->state.flags |= 0x100U;
    a->state.hit_points = 0;
    bool dead = false, complete = false;
    for (Uint16 i = 0; i < 200 && !a->remove; ++i)
    {
        s->service_ticks = i;
        s->combat.sound_count = 0;
        CHECK(ActorLibrary(s, a));
        dead |= (s->library_enemy_flags & 2U) != 0;
        complete |= (s->library_enemy_flags & 4U) != 0;
    }
    CHECK(a->remove && dead && complete && !a->state.hitbox_state && a->state.opacity == 0 &&
          a->state.frame == 0);
    SotnRoomActors_Destroy(s->spawns);
    return true;
}
static bool SummonPackets(Fixture *f, SotnActors *s)
{
    CHECK(Begin(f, s));
    Actor *a = &s->slots[16];
    CHECK(ActorDemonSummoning(s, a) && a->state.primitive_count == 30 && a->demon.effect_step == 1);
    CHECK(ActorDemonSummoning(s, a) && a->demon.effect_step == 3 && a->demon.effect_timer == 24);
    for (Uint16 i = 0; i < 4; ++i)
        CHECK(ActorDemonSummoning(s, a));
    ActorPrimitive *child = ActorPrimitiveGet(s, a, 2);
    CHECK(child && child->demon.used && child->demon.color_index == 2 && child->demon.phase == 1);
    CHECK(child->state.vertex_red[0] == 42 && child->state.vertex_red[2] == 52 &&
          child->state.vertex_green[0] == 62 && child->state.priority == 138);
    for (Uint16 i = 96; i < 128; ++i)
        s->slots[i].state.generation = 99;
    const Uint32 random = s->random;
    a->demon.effect_step = 2;
    a->demon.effect_timer = 1;
    a->demon.timer = 1;
    CHECK(ActorDemonSummoning(s, a) && a->demon.effect_step == 3 && s->random == random);
    a->demon.effect_step = 4;
    a->demon.effect_timer = 0;
    for (Uint16 i = 0; i < 25; ++i)
        CHECK(ActorDemonSummoning(s, a));
    CHECK(a->demon.effect_step == 5);
    CHECK(ActorDemonSummoning(s, a) && a->demon.effect_step == 6 && !a->state.primitive_count);
    CHECK(ActorDemonSummoning(s, a));
    a->demon.effect_step = 0;
    CHECK(ActorDemonSummoning(s, a));
    CHECK(ActorDemonSummoning(s, a));
    child = ActorPrimitiveGet(s, a, 0);
    CHECK(child);
    child->demon.color_index = 7;
    CHECK(!ActorDemonSummoning(s, a));
    SotnRoomActors_Destroy(s->spawns);
    return true;
}
static bool Capacity(Fixture *f, SotnActors *s)
{
    CHECK(Begin(f, s));
    Actor *a = &s->slots[16];
    for (Uint16 i = 96; i < 128; ++i)
        s->slots[i].state.generation = 99;
    a->state.y = 672 * 65536;
    SetStep(a, 4);
    CHECK(ActorLibrary(s, a) && a->state.step == 3 && a->demon.child_slot == SDL_MAX_UINT16);
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 99;
    a->state.y = 592 * 65536;
    SetStep(a, 14);
    for (Uint16 i = 0; i < 4 && a->substep == 0; ++i)
        CHECK(ActorLibrary(s, a));
    for (Uint16 i = 0; i < 180 && a->state.step == 14; ++i)
    {
        s->combat.sound_count = 0;
        CHECK(ActorLibrary(s, a));
    }
    CHECK(a->state.step == 9 && !a->state.primitive_count && a->state.hit_points == 40);
    CHECK(!ActorLibraryDemon(NULL, a) && !ActorLibraryDemon(s, NULL));
    a->state.step = 32;
    CHECK(!ActorLibrary(s, a));
    a->state.step = 1;
    a->vx = 9 * 65536;
    CHECK(!ActorLibrary(s, a));
    SotnRoomActors_Destroy(s->spawns);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    SotnActors *s = SDL_calloc(1, sizeof(*s));
    const bool ok = f && s && Movement(f, s) && PunchAndBolt(f, s) && Summon(f, s) &&
                    SpitAndDeath(f, s) && SummonPackets(f, s) && Capacity(f, s);
    SDL_free(s);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_LIBRARY_DEMON_TEST: PASS");
    return 0;
}
