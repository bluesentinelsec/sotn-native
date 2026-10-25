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
static void Blueprint(Uint8 *image, Uint8 flags, Uint8 kind)
{
    const Uint8 profile[6] = {5, 3, flags, 2, (Uint8)(0x20 | kind), 0};
    SDL_memcpy(image + 0x14b0, profile, sizeof(profile));
    image[0x1764 + kind * 2U] = kind == 3 ? 80 : 96;
    image[0x1765 + kind * 2U] = kind == 3 ? 80 : 98;
}
static void Clear(SotnActors *s)
{
    for (Uint32 i = 1; i < SOTN_STAGE_ACTOR_SLOTS; ++i)
        ActorPrimitivesClear(s, &s->slots[i]);
    const Actor root = s->slots[0];
    SDL_zero(s->slots);
    s->slots[0] = root;
}
static bool Tests(SotnActors *s, Uint8 *image)
{
    const Actor root = {.state = {.generation = 1, .x = 100, .y = 200, .priority = 128}, .slot = 0};
    const SotnDoppleRequest request = {.kind = SOTN_DOPPLE_FACTORY,
                                       .id = 0,
                                       .parameter = 7,
                                       .x = 300,
                                       .y = 400,
                                       .facing_left = true};
    s->slots[0] = root;
    Blueprint(image, 0xc1, 0);
    s->dopple.body.motion.x = 500;
    s->dopple.body.motion.y = 600;
    CHECK(ActorDoppleFactoryRequest(s, &root, &request));
    Actor *factory = &s->slots[4];
    CHECK(factory->state.id == 0x101 && factory->state.x == 300 && factory->state.facing_left);
    CHECK(ActorDoppleFactory(s, factory));
    CHECK(s->slots[34].state.id == 0x105 && s->slots[34].state.parameters == 0x700 &&
          s->slots[34].state.x == 500 && s->slots[34].state.y == 600 &&
          s->slots[34].enemy_state.owner_generation == 1);
    CHECK(!factory->remove && factory->dopple_factory.spawned == 1);
    CHECK(ActorDoppleFactory(s, factory) && s->slots[33].state.generation == 0);
    CHECK(ActorDoppleFactory(s, factory) && s->slots[33].state.parameters == 0x701);
    CHECK(ActorDoppleFactory(s, factory) && s->slots[32].state.generation == 0);
    CHECK(ActorDoppleFactory(s, factory) && s->slots[32].state.parameters == 0x702 &&
          factory->remove);
    Clear(s);
    Blueprint(image, 1, 0);
    for (Uint32 i = 32; i < 35; ++i)
        s->slots[i].state.generation = 2;
    CHECK(ActorDoppleFactoryRequest(s, &root, &request));
    CHECK(ActorDoppleFactory(s, &s->slots[4]) && !s->slots[4].remove &&
          s->slots[4].dopple_factory.spawned == 0 && s->slots[4].dopple_factory.delay == 2);
    image[0x14b2] = 0x81;
    CHECK(ActorDoppleFactoryRequest(s, &root, &request));
    CHECK(ActorDoppleFactory(s, &s->slots[5]) && s->slots[5].remove);
    Clear(s);
    Blueprint(image, 0xc1, 3);
    s->dopple.body.busy = true;
    s->slots[16] = (Actor){.state = {.generation = 7}, .slot = 16, .dynamic = true};
    CHECK(ActorPrimitives(s, &s->slots[16], 1));
    const Uint16 primitive = s->slots[16].primitive[0];
    CHECK(ActorDoppleFactoryRequest(s, &root, &request));
    CHECK(ActorDoppleFactory(s, &s->slots[4]) && s->slots[16].state.id == 0x105 &&
          s->slots[16].state.generation != 7 && !s->dopple.body.busy &&
          s->primitives[primitive].owner == 0);
    Clear(s);
    Blueprint(image, 0xc1, 0);
    image[0x14b4] = 0x40;
    s->dopple.body.step = 2;
    CHECK(ActorDoppleFactoryRequest(s, &root, &request));
    CHECK(ActorDoppleFactory(s, &s->slots[4]));
    s->dopple.body.step = 1;
    CHECK(ActorDoppleFactory(s, &s->slots[4]) && s->slots[4].remove);
    Clear(s);
    Blueprint(image, 0xc1, 0);
    for (Uint32 i = 4; i < 16; ++i)
        s->slots[i].state.generation = 1;
    const Uint32 generation = s->effect_serial;
    CHECK(ActorDoppleFactoryRequest(s, &root, &request) && s->effect_serial == generation);
    Clear(s);
    CHECK(ActorDoppleFactoryRequest(s, &root, &request));
    image[0x14b4] = 15;
    CHECK(!ActorDoppleFactory(s, &s->slots[4]) && s->slots[4].state.step == 0);
    CHECK(!ActorDoppleFactoryRequest(NULL, &root, &request));
    CHECK(!ActorDoppleFactoryRequest(s, NULL, &request));
    CHECK(!ActorDoppleFactoryRequest(s, &root, NULL));
    CHECK(!ActorDoppleFactory(NULL, &s->slots[4]));
    CHECK(!ActorDoppleFactory(s, NULL));
    return true;
}
static void Put16(Uint8 *bytes, Uint16 value)
{
    bytes[0] = (Uint8)value;
    bytes[1] = (Uint8)(value >> 8);
}
static void Put32(Uint8 *bytes, Uint32 value)
{
    Put16(bytes, (Uint16)value);
    Put16(bytes + 2, (Uint16)(value >> 16));
}
static bool Unarmed(SotnActors *s, Uint8 *image)
{
    Uint8 dra[0x300] = {0};
    Put32(dra + 0x98, 0x800a0200);
    Put16(dra + 0x204, 20);
    Put16(dra + 0x206, 6);
    Put16(dra + 0x20c, 1);
    Put16(image + 0x440, 0x8007);
    Put16(image + 0x448, 0);
    Put32(image + 0x4278, 0x80183000);
    Put32(image + 0x427c, 0x80183100);
    Put16(image + 0x4280, 0x234);
    image[0x4282] = 65;
    image[0x4283] = 0;
    for (size_t i = 0; i < 7; ++i)
        Put32(image + 0x3000 + i * 4, 0x80183200);
    Put16(image + 0x3200, (2U << 9) | 3U);
    Put16(image + 0x3202, 0xffff);
    image[0x3108] = (Uint8)-4;
    image[0x3109] = 3;
    image[0x310a] = 7;
    image[0x310b] = 9;
    s->dra = (SotnDataView){dra, sizeof(dra)};
    s->dopple.body = (SotnDoppleBody){.motion = {.x = 123, .y = 456},
                                      .facing_left = true,
                                      .attack_state = 0x8002,
                                      .animation = {.id = 65, .pose = 0, .remaining = 1}};
    s->slots[0].state.priority = 128;
    Actor child = {.state = {.generation = 77, .id = 0x137},
                   .enemy_state = {.owner_generation = 1}};
    CHECK(ActorDoppleUnarmed(s, &child));
    CHECK(child.attack == 6 && child.state.x == 123 && child.state.y == 456 &&
          child.state.facing_left && child.state.hitbox_x == -4 && child.state.hitbox_y == 3 &&
          child.state.hitbox_width == 7 && child.state.hitbox_height == 9 &&
          child.state.frame == 3 && child.state.priority == 126 && child.state.blend_mode == 0x30);
    CHECK(s->combat.sound_count == 1 && s->combat.sounds[0] == 0x234);
    s->dopple.body.animation.pose = 1;
    CHECK(ActorDoppleUnarmed(s, &child) && child.remove);
    child =
        (Actor){.state = {.generation = 78, .id = 0x137}, .enemy_state = {.owner_generation = 1}};
    s->dopple.body.animation.pose = 0;
    s->dopple.body.motion.status = 0x100;
    CHECK(ActorDoppleUnarmed(s, &child) && child.attack == 3);
    s->dopple.body.attack_state = 0;
    CHECK(ActorDoppleUnarmed(s, &child) && child.remove);
    child = (Actor){.state = {.generation = 79, .id = 0x137, .parameters = 0x200},
                    .enemy_state = {.owner_generation = 1}};
    CHECK(!ActorDoppleUnarmed(s, &child));
    child.enemy_state.owner_generation = 2;
    CHECK(ActorDoppleUnarmed(s, &child) && child.remove);
    CHECK(!ActorDoppleUnarmed(NULL, &child));
    CHECK(!ActorDoppleUnarmed(s, NULL));
    s->dra = (SotnDataView){0};
    return true;
}
static bool Knife(SotnActors *s)
{
    Uint8 dra[0x300] = {0}, tiles[16 * 16 * 2] = {0}, types[2] = {0, 3};
    Put32(dra + 0x98, 0x800a0200);
    Put16(dra + 0x204, 20);
    Put16(dra + 0x206, 6);
    Put16(dra + 0x20c, 1);
    s->dra = (SotnDataView){dra, sizeof(dra)};
    s->map = (SotnCollisionMap){{tiles, sizeof(tiles)}, {types, sizeof(types)}, 16, 16};
    for (Uint32 y = 0; y < 16; ++y)
        for (Uint32 x = 10; x < 16; ++x)
            tiles[(y * 16 + x) * 2] = 1;
    Clear(s);
    s->dopple.body = (SotnDoppleBody){0};
    Actor *a = &s->slots[32];
    *a = (Actor){.state = {.id = 0x104, .generation = 88, .x = 100 * 65536, .y = 109 * 65536},
                 .slot = 32,
                 .dynamic = true};
    CHECK(ActorDoppleKnife(s, a));
    CHECK(a->state.step == 1 && a->vx == 524288 && a->state.y == 100 * 65536 &&
          a->state.hitbox_width == 8 && a->state.hitbox_height == 5 &&
          s->dopple.body.timers[10] == 4 && s->combat.sound_count > 0 &&
          s->combat.sounds[s->combat.sound_count - 1] == 0x60c);
    CHECK(ActorDoppleKnife(s, a));
    ActorPrimitive *picture = ActorPrimitiveGet(s, a, 0), *trail = ActorPrimitiveGet(s, a, 1);
    CHECK(picture != NULL && trail != NULL && picture->state.quad && picture->state.active &&
          trail->state.line && trail->state.active && trail->state.line_blend_mode == 1 &&
          a->state.x == 108 * 65536);
    for (Uint32 tick = 0; tick < 12 && a->state.step == 1; ++tick)
        CHECK(ActorDoppleKnife(s, a));
    CHECK(a->state.step == 2 && a->timer == 64 && a->vx == -65536 && a->vy == -163840 &&
          a->state.hitbox_state == 0 && a->state.x == 152 * 65536 &&
          s->combat.sounds[s->combat.sound_count - 1] == 0x6a4);
    CHECK(ActorDoppleKnife(s, a));
    CHECK(a->timer == 63 && a->state.x == 151 * 65536 && !trail->state.active);
    while (a->timer > 32)
        CHECK(ActorDoppleKnife(s, a));
    CHECK(picture->state.blend && picture->state.modulate && picture->state.red == 94 &&
          picture->state.page == 0x3c);
    while (!a->remove)
        CHECK(ActorDoppleKnife(s, a));
    Clear(s);
    a = &s->slots[32];
    s->dopple.body.facing_left = true;
    s->dopple.body.motion.status = 0x120;
    *a = (Actor){.state = {.id = 0x104, .generation = 89, .x = 100 * 65536, .y = 109 * 65536},
                 .slot = 32,
                 .dynamic = true};
    CHECK(ActorDoppleKnife(s, a));
    CHECK(a->vx == -524288 && a->state.y == 109 * 65536 && a->attack == 3);
    a->hit_flags = 0x80;
    CHECK(ActorDoppleKnife(s, a) && a->state.step == 3 && a->timer == 4);
    for (Uint32 tick = 0; tick < 4; ++tick)
        CHECK(ActorDoppleKnife(s, a));
    CHECK(a->remove);
    a->state.step = 2;
    a->timer = 0;
    CHECK(!ActorDoppleKnife(s, a));
    a->state.step = 1;
    a->vx = 0;
    CHECK(!ActorDoppleKnife(s, a));
    a->state.step = 9;
    CHECK(!ActorDoppleKnife(s, a));
    CHECK(!ActorDoppleKnife(NULL, a));
    CHECK(!ActorDoppleKnife(s, NULL));
    s->dra = (SotnDataView){0};
    s->map = (SotnCollisionMap){0};
    return true;
}
int main(void)
{
    Uint8 image[0x4400] = {0};
    SotnActors *s = SDL_calloc(1, sizeof(*s));
    if (s == NULL)
        return 1;
    s->stage_id = 26;
    s->nz0 = (SotnDataView){image, sizeof(image)};
    const bool pass =
        SotnRoomActors_Create(&s->spawns) && Tests(s, image) && Unarmed(s, image) && Knife(s);
    SotnRoomActors_Destroy(s->spawns);
    SDL_free(s);
    if (!pass)
        return 1;
    SDL_Log("SOTN_DOPPLE_FACTORY_TEST: PASS");
    return 0;
}
