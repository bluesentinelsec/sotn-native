#include "actors_internal.h"
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    Uint8 stage[0x4000], dra[0x400], exe[0x23220], tiles[48 * 32 * 2], types[0x800];
    SotnWorldState world;
    SotnActorLayout *layout;
} Fixture;
static void Put16(Uint8 *p, Sint32 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)((Uint32)value >> 8);
}
static void Put32(Uint8 *p, Uint32 value)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(value >> (i * 8U));
}
static bool Open(Fixture *f, SotnActors **out, Uint16 id)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 32767);
    Put16(f->dra + 0x20c, 2);
    Put32(f->dra + 0x224, 0x80002000U);
    Put16(f->stage + 0x8a4, 0x8002);
    for (Uint16 i = 0; i < 10; ++i)
    {
        Put16(f->stage + 0xdec + i * 2U, 40 + i);
        Put16(f->stage + 0xe00 + i * 2U, i & 1U ? 0 : 2);
    }
    for (Uint16 i = 0; i < 8; ++i)
    {
        Put16(f->stage + 0xe14 + i * 2U, 200 + i);
        Put16(f->stage + 0xe24 + i * 2U, 1);
        Put16(f->stage + 0xe34 + i * 2U, i & 1U ? 0 : 2);
    }
    for (Uint16 i = 0; i < 16; ++i)
    {
        // Authored motion fixture, unrelated to the original launch arrays.
        Put32(f->stage + 0xd48 + i * 8U, (Uint32)((Sint32)i * 512 - 2048));
        Put32(f->stage + 0xd4c + i * 8U, 1024 + i * 128U);
    }
    for (Uint16 i = 0; i < 18; ++i)
        Put16(f->stage + 0xdc8 + i * 2U, (Sint32)i * 7 - 13);
    for (Uint32 i = 0; i < 4096; ++i)
    {
        const Uint32 phase = i & 1023U, c = (i + 1024U) & 4095U, cp = c & 1023U;
        const Sint32 sine = i < 1024   ? (Sint32)phase * 4
                            : i < 2048 ? 4096 - (Sint32)phase * 4
                            : i < 3072 ? -(Sint32)phase * 4
                                       : -4096 + (Sint32)phase * 4;
        const Sint32 cosine = c < 1024   ? (Sint32)cp * 4
                              : c < 2048 ? 4096 - (Sint32)cp * 4
                              : c < 3072 ? -(Sint32)cp * 4
                                         : -4096 + (Sint32)cp * 4;
        Put16(f->exe + 0x1ea10 + i * 4U, sine);
        Put16(f->exe + 0x1ea12 + i * 4U, cosine);
    }
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    f->types[1] = f->types[2] = 3;
    for (Uint32 i = 48 * 12; i < 48 * 32; ++i)
        Put16(f->tiles + i * 2U, 1);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 4, out));
    CHECK(SotnActorLayout_OpenStage((SotnDataView){f->stage, sizeof(f->stage)}, 4, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *out, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 48, 32}, 0,
        0));
    SotnActors *s = *out;
    s->world = &f->world;
    s->executable = (SotnDataView){f->exe, sizeof(f->exe)};
    s->service_ticks = 7;
    s->player = (SotnPlayerState){
        .x = 128 * 65536, .y = 128 * 65536, .hp = 70, .hp_max = 70, .invulnerable = true};
    s->slots[16] =
        (Actor){.slot = 16,
                .dynamic = true,
                .state = {.id = id, .generation = 1, .x = 128 * 65536, .y = 128 * 65536}};
    return true;
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    f->layout = NULL;
}
static bool Tick(SotnActors *s, Actor *a, Uint16 hit)
{
    s->combat = (SotnActorCombat){0};
    a->hit_flags = hit;
    Actor next = *a;
    CHECK(ActorOlroxQuarter(s, &next));
    *a = next;
    ++s->service_ticks;
    return true;
}
static bool Wall(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s, 0x24));
    Actor *a = &s->slots[16];
    CHECK(Tick(s, a, 0) && a->state.hit_points == 32767 && a->state.hitbox_state == 2 &&
          a->state.priority == 168);
    for (Uint16 hit = 1; hit <= 3; ++hit)
    {
        CHECK(Tick(s, a, 2) && a->secret.hits == hit && a->state.step == (hit == 3 ? 3 : 2));
        if (hit < 3)
        {
            for (Uint16 i = 0; i < 16; ++i)
                CHECK(Tick(s, a, 2));
            CHECK(a->state.step == 1 && a->state.frame == 11 + hit);
        }
    }
    CHECK(!a->state.hitbox_state && Tick(s, a, 0) && a->state.step == 4 &&
          a->state.primitive_count == 32);
    CHECK(!f->world.flags[SOTN_NO2_SECRET_WALL]);
    for (Uint16 i = 0; i < 32; ++i)
        CHECK(Tick(s, a, 0));
    CHECK(a->state.step == 5 && !f->world.flags[SOTN_NO2_SECRET_WALL]);
    CHECK(Tick(s, a, 0) && a->state.step == 6 && f->world.flags[SOTN_NO2_SECRET_WALL] == 1 &&
          a->secret.timer == 384);
    for (Uint16 i = 0; i < 10; ++i)
        CHECK(Read16(s->map.tiles.bytes + (40U + i) * 2U) == (i & 1U ? 0 : 2));
    CHECK(s->slots[160].state.id == 2 && s->slots[160].state.parameters == 0xaa13 &&
          s->slots[168].state.id == 6);
    for (Uint16 i = 0; i < 384; ++i)
        CHECK(Tick(s, a, 0));
    CHECK(a->remove && f->world.flags[SOTN_NO2_SECRET_WALL] == 3);
    ActorPrimitivesClear(s, a);
    *a = (Actor){.slot = 16, .state = {.id = 0x24, .generation = 2}};
    CHECK(Tick(s, a, 0) && a->remove && !a->state.primitive_count);
    Close(f, s);
    return true;
}
static bool Ceiling(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s, 0x25));
    Actor *a = &s->slots[16];
    CHECK(Tick(s, a, 0) && a->state.hit_points == 128 && a->state.rotation == 3072 &&
          a->state.primitive_count == 30);
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(Read16(s->map.tiles.bytes + (200U + i) * 2U) == 1);
    CHECK(Tick(s, a, 2));
    Uint16 active = 0;
    for (Uint16 i = 0; i < 30; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        CHECK(p);
        active += (p->secret.tag != 0);
    }
    CHECK(active == 16 && Tick(s, a, 2));
    active = 0;
    for (Uint16 i = 0; i < 30; ++i)
        active += ActorPrimitiveGet(s, a, i)->secret.tag != 0;
    CHECK(active == 30 && !ActorPrimitiveGet(s, a, 29)->state.active);
    a->state.flags |= 0x100U;
    CHECK(Tick(s, a, 0) && a->state.step == 2 && !f->world.flags[SOTN_NO2_SECRET_CEILING]);
    CHECK(Tick(s, a, 0) && a->state.step == 3 && a->state.primitive_count == 24 &&
          f->world.flags[SOTN_NO2_SECRET_CEILING] == 1);
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(Read16(s->map.tiles.bytes + (200U + i) * 2U) == (i & 1U ? 0 : 2));
    for (Uint16 i = 0; i < 500 && !a->remove; ++i)
        CHECK(Tick(s, a, 0));
    CHECK(a->remove);
    ActorPrimitivesClear(s, a);
    *a = (Actor){.slot = 16, .state = {.id = 0x25, .generation = 2}};
    CHECK(Tick(s, a, 0) && a->remove && !a->state.primitive_count);
    Close(f, s);
    return true;
}
static bool Damage(Fixture *f)
{
    for (Uint16 kind = 0; kind < 2; ++kind)
    {
        SotnActors *s = NULL;
        CHECK(Open(f, &s, kind ? 0x25 : 0x24));
        Actor *a = &s->slots[16];
        CHECK(Tick(s, a, 0));
        SotnPlayerState hero = s->player;
        hero.attack = (SotnPlayerAttack){.active = true,
                                         .power = 20,
                                         .hit_type = 2,
                                         .element = 0x20,
                                         .invincibility_frames = 1,
                                         .width = 8,
                                         .height = 8};
        CHECK(SotnActors_Tick(s, hero, 0, 0));
        CHECK(kind ? a->state.hit_points < 128 : a->state.hit_points == 32767);
        if (!kind)
            CHECK(a->secret.hits == 1 && a->state.step == 2);
        Close(f, s);
    }
    return true;
}
static bool FailuresAndAftermath(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s, 0x24));
    Actor *a = &s->slots[16];
    CHECK(Tick(s, a, 0));
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 99;
    a->state.step = 3;
    CHECK(Tick(s, a, 0) && a->remove && f->world.flags[SOTN_NO2_SECRET_WALL] == 1);
    Close(f, s);
    CHECK(Open(f, &s, 0x25));
    a = &s->slots[16];
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 99;
    CHECK(Tick(s, a, 0) && a->remove && !f->world.flags[SOTN_NO2_SECRET_CEILING]);
    Close(f, s);
    CHECK(Open(f, &s, 0x25));
    a = &s->slots[16];
    Put16(f->stage + 0xe14 + 14, 0xffff);
    CHECK(!Tick(s, a, 0) && Read16(s->map.tiles.bytes + 200 * 2U) == 0);
    Close(f, s);
    CHECK(Open(f, &s, 0x26));
    a = &s->slots[16];
    f->world.flags[SOTN_NO2_SECRET_WALL] = 1;
    CHECK(Tick(s, a, 0) && Tick(s, a, 0) && a->state.primitive_count == 8 &&
          f->world.flags[SOTN_NO2_SECRET_WALL] == 3);
    for (Uint16 i = 0; i < 400 && !a->remove; ++i)
        CHECK(Tick(s, a, 0));
    CHECK(a->remove);
    ActorPrimitivesClear(s, a);
    *a = (Actor){.slot = 16, .state = {.id = 0x26, .generation = 2}};
    CHECK(Tick(s, a, 0) && Tick(s, a, 0) && a->remove && !a->state.primitive_count);
    CHECK(!ActorOlroxSecret(NULL, a) && !ActorOlroxSecret(s, NULL));
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    const bool ok = f && Wall(f) && Ceiling(f) && Damage(f) && FailuresAndAftermath(f);
    SDL_free(f);
    if (ok)
        SDL_Log("NO2_SECRETS: PASS synthetic attacks, patches, staged reveals, debris, ownership "
                "and fallback checks");
    return ok ? 0 : 1;
}
