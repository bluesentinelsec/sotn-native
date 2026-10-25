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
    Uint8 dra[1024], stage[0x1800], exe[0x23220], tiles[32 * 160 * 2], types[2048];
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
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_memset(f, 0, sizeof(*f));
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 0x7fff);
    Put32(f->dra + 0x224, 0x0d002000);
    Put16(f->stage + 0x9fa, 64);
    Put32(f->stage + 28, 0x80180040);
    for (Uint32 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    f->stage[0x1550] = 1;
    f->stage[0x1551] = 65;
    f->stage[0x1552] = 255;
    f->stage[0x155c] = 1;
    f->stage[0x155d] = 64;
    f->stage[0x155e] = 255;
    Put16(f->stage + 0x1568, 700);
    Put16(f->stage + 0x156a, 500);
    Put16(f->stage + 0x156c, 300);
    for (Uint32 i = 0; i < 9; ++i)
    {
        f->stage[0x13fc + i] = (Uint8)(i + 1);
        Put32(f->stage + 0x1408 + i * 4, 0x801813ac);
        Put16(f->stage + 0x1508 + i * 2, (Uint16)i);
        Put16(f->stage + 0x151c + i * 2, 2);
        Put16(f->stage + 0x152e + i * 2, 1);
    }
    f->stage[0x13ac] = 1;
    f->stage[0x13ad] = 2;
    for (Uint32 i = 0; i <= 1024; ++i)
        Put16(f->exe + 0x22a10 + i * 2, (Uint16)(i / 2));
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 1, out));
    CHECK(SotnActors_SetServices(*out, (SotnDataView){f->exe, sizeof(f->exe)}, 0, true));
    CHECK(SotnActorLayout_OpenStage((*out)->nz0, 1, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *out, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 160}, 0,
        0));
    return true;
}
static bool Trip(SotnActors *s, Uint8 destination)
{
    Actor *cart = &s->slots[88];
    bool started = false, halted = false, guarded = false;
    for (Uint32 t = 0; t < 1200; ++t)
    {
        SotnPlayerState player = {
            .x = cart->state.x, .y = cart->state.y - 8 * 65536, .hp = 70, .invulnerable = true};
        Uint16 buttons = 0;
        if (!started && cart->state.step == 2)
            buttons = destination > cart->outer_lift.current ? SOTN_BUTTON_UP : SOTN_BUTTON_DOWN;
        CHECK(SotnActors_SetButtons(s, buttons) && SotnActors_Tick(s, player, 0, 0));
        started = started || cart->state.step == 3;
        halted = halted || s->motion.stop_velocity;
        guarded = guarded || s->motion.protect;
        if (started && cart->state.step == 2)
        {
            CHECK(cart->outer_lift.current == destination &&
                  cart->outer_lift.target == destination);
            CHECK(halted && guarded && !cart->outer_lift.called);
            return true;
        }
    }
    CHECK(false);
}
static bool Ride(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    s->world->flags[SOTN_OUTER_ELEVATOR] = 1;
    Actor *cart = &s->slots[88];
    *cart = (Actor){.state = {.id = 0x29, .generation = 100, .x = 300 * 65536, .y = 700 * 65536},
                    .slot = 88,
                    .dynamic = true};
    CHECK(Trip(s, 1) && Trip(s, 2) && Trip(s, 1) && Trip(s, 0));
    CHECK(cart->state.primitive_count == 40 && s->slots[89].state.id == 0x2a &&
          s->slots[89].state.primitive_count == 32);
    Actor *door = &s->slots[16];
    *door = (Actor){.state = {.id = 0x2b,
                              .generation = 101,
                              .parameters = 2,
                              .x = 280 * 65536,
                              .y = 300 * 65536},
                    .slot = 16,
                    .dynamic = true};
    SotnPlayerState player = {.x = 260 * 65536, .y = 300 * 65536, .hp = 70, .invulnerable = true};
    CHECK(SotnActors_Tick(s, player, 0, 0));
    CHECK(s->mutable_tiles[12] == 2 && door->state.primitive_count == 2);
    door->outer_lift.wait = 32;
    bool called = false, arrived = false;
    for (Uint32 t = 0; t < 1200; ++t)
    {
        CHECK(SotnActors_SetButtons(s, 0) && SotnActors_Tick(s, player, 0, 0));
        called = called || cart->outer_lift.called;
        if (called && cart->state.step == 2 && cart->outer_lift.current == 2 &&
            door->state.frame == 0)
        {
            arrived = true;
            break;
        }
    }
    CHECK(called && arrived && s->mutable_tiles[12] == 1);
    for (Uint32 t = 0; t < 24; ++t)
        CHECK(SotnActors_Tick(s, player, 0, 0));
    ActorPrimitive *upper = ActorPrimitiveGet(s, door, 0);
    CHECK(upper != NULL && upper->state.quad && upper->state.offset_y[2] == -10 &&
          upper->state.offset_y[3] == -24);
    CHECK(SotnActors_Enter(
        s, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 160}, 0,
        0));
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        CHECK(!s->primitives[i].owner);
    Actor invalid = {.state = {.id = 0x29, .parameters = 3, .generation = 102}};
    CHECK(!ActorOuterElevator(s, &invalid));
    invalid = (Actor){.state = {.id = 0x2b, .parameters = 3, .generation = 103}};
    CHECK(!ActorOuterLiftDoors(s, &invalid));
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    return true;
}
static bool Switch(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *base = &s->slots[2];
    *base = (Actor){.state = {.id = 0x33, .generation = 200, .x = 200 * 65536, .y = 100 * 65536},
                    .slot = 2,
                    .dynamic = true};
    SotnPlayerState player = {
        .x = 205 * 65536,
        .y = 145 * 65536,
        .hp = 70,
        .invulnerable = true,
        .attack = {
            .active = true, .hit_type = 2, .width = 40, .height = 40, .invincibility_frames = 1}};
    CHECK(SotnActors_Tick(s, player, 0, 0));
    Actor *handle = &s->slots[3];
    CHECK(handle->state.id == 0x33 && handle->state.parameters == 1 &&
          handle->state.rotation == -128);
    for (Uint32 t = 0; t < 80; ++t)
        CHECK(SotnActors_Tick(s, player, 0, 0));
    CHECK(s->world->flags[SOTN_OUTER_ELEVATOR] && handle->state.step == 2 &&
          !handle->state.hitbox_state);
    CHECK(s->combat.ambience_change && s->combat.ambience_volume > 0 &&
          handle->state.hit_points == 0x7fff);
    CHECK(SotnWorld_Validate(s->world));
    s->world->flags[SOTN_OUTER_ELEVATOR] = 2;
    CHECK(!SotnWorld_Validate(s->world));
    s->world->flags[SOTN_OUTER_ELEVATOR] = 1;
    s->world->flags[SOTN_OUTER_PRESSURE] = 2;
    CHECK(!SotnWorld_Validate(s->world));
    s->world->flags[SOTN_OUTER_PRESSURE] = 1;
    Actor plate = {.state = {.id = 0x2c, .generation = 201}};
    CHECK(ActorOuterLiftSwitch(s, &plate) && plate.state.y == 195 * 65536 && plate.state.step == 3);
    Actor gear = {.state = {.id = 0x32, .generation = 202}};
    CHECK(ActorOuterLiftSwitch(s, &gear) && gear.state.frame == 1);
    CHECK(ActorOuterLiftSwitch(s, &gear) && gear.state.step == 2);
    CHECK(ActorOuterLiftSwitch(s, &gear) && gear.state.frame == 2);
    Actor invalid = {.state = {.id = 0x32, .generation = 203, .parameters = 9}};
    CHECK(!ActorOuterLiftSwitch(s, &invalid));
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Ride(f) && Switch(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
