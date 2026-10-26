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
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    Put32(f->stage + 0xb74, 1);
    Put32(f->dra + 0x98, 0x800a0200);
    const size_t streams[] = {0xc24, 0xc48, 0xc5c, 0xc6c, 0xc74, 0xc84,
                              0xc90, 0xc98, 0xca8, 0xcc0, 0xccc, 0xcd4};
    for (Uint16 i = 0; i < SDL_arraysize(streams); ++i)
    {
        // Synthetic timelines exercise state ordering without original runtime frames.
        f->stage[streams[i]] = 1;
        f->stage[streams[i] + 1] = (Uint8)(20 + i);
        f->stage[streams[i] + 2] = i == 11 ? 0 : 255;
    }
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 25, out));
    SotnActorLayout *layout = NULL;
    CHECK(SotnActorLayout_OpenStage((*out)->nz0, 25, 0, &layout));
    const bool entered = SotnActors_Enter(
        *out, layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16}, 192,
        0);
    SotnActorLayout_Close(layout);
    CHECK(entered);
    (*out)->player = (SotnPlayerState){.hp = 70, .x = 300 * 65536 + 123, .y = 135 * 65536};
    return true;
}
static bool Manager(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = {.slot = 137, .state = {.id = 0x2b, .generation = 1}};
    CHECK(ActorHippogryphMaria(s, &a) && a.state.step == 1 && !s->control.timer);
    s->hippogryph_encounter = 1;
    s->slots[96].state = (SotnActorState){.id = 0x28, .generation = 10};
    CHECK(ActorHippogryphMaria(s, &a) && !a.substep);
    s->slots[96].state.id = 3;
    CHECK(ActorHippogryphMaria(s, &a) && a.substep == 1 && a.hippogryph.linked_slot == 96 &&
          a.hippogryph.linked_generation == 10);
    CHECK(ActorHippogryphMaria(s, &a) && a.substep == 1 && s->hippogryph_encounter);
    s->slots[96] = (Actor){0};
    CHECK(ActorHippogryphMaria(s, &a) && a.substep == 2 && !s->hippogryph_encounter &&
          (s->death_event.flags & 0x8000U));
    CHECK(ActorHippogryphMaria(s, &a) && a.state.step == 2 && !a.substep);
    CHECK(ActorHippogryphMaria(s, &a) && a.state.step == 3 && s->control.timer == 1 &&
          s->control.pause_disabled && s->control.block_stopwatch &&
          s->control.buttons == SOTN_BUTTON_RIGHT);
    CHECK(ActorHippogryphMaria(s, &a) && a.state.step == 3 && (s->death_event.flags & 1U) &&
          s->camera->anchor_x == 127);
    s->player.x = 321 * 65536 + 123;
    CHECK(ActorHippogryphMaria(s, &a) && a.state.step == 4 && !s->control.buttons &&
          s->motion.align_horizontal && s->motion.alignment_x == 320 &&
          s->player.x == 320 * 65536 + 123);
    for (Uint16 i = 0; i < 70; ++i)
        CHECK(ActorHippogryphMaria(s, &a));
    CHECK(s->camera->anchor_x == 64 && !a.remove);
    s->death_event.flags |= 2U;
    CHECK(ActorHippogryphMaria(s, &a) && a.state.step == 5);
    for (Uint16 i = 0; i < 64; ++i)
        CHECK(ActorHippogryphMaria(s, &a));
    CHECK(a.remove && s->camera->anchor_x == 128 && !s->control.pause_disabled &&
          !s->control.block_stopwatch && s->control.timer == 1);
    SotnActors_Destroy(s);
    return true;
}
static bool ManagerVariants(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = {.state = {.id = 0x2b, .generation = 1, .step = 2}};
    s->player.status = 4;
    CHECK(ActorHippogryphMaria(s, &a) && a.state.step == 3 && s->control.buttons == 2);
    CHECK(ActorHippogryphMaria(s, &a) && !s->control.buttons && s->camera->anchor_x == 128);
    s->service_ticks = 1;
    CHECK(ActorHippogryphMaria(s, &a) && s->control.buttons == 2);
    a.state.step = 1;
    s->slots[96].state = (SotnActorState){.id = 3, .generation = 10};
    s->slots[97].state = (SotnActorState){.id = 3, .generation = 11};
    CHECK(ActorHippogryphMaria(s, &a) && a.substep == 2 && a.hippogryph.linked_slot == 97);
    CHECK(ActorHippogryphMaria(s, &a) && a.state.step == 2);
    a.state.step = 1;
    a.substep = 1;
    a.hippogryph.linked_slot = 96;
    a.hippogryph.linked_generation = 9;
    s->hippogryph_encounter = 1;
    CHECK(ActorHippogryphMaria(s, &a) && a.substep == 2 && !s->hippogryph_encounter &&
          s->slots[96].state.generation == 10);
    SotnActors_Destroy(s);
    return true;
}
static bool Maria(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = {.state = {.id = 0x2c, .generation = 1, .x = 160 * 65536 + 7, .y = 160 * 65536}};
    CHECK(ActorHippogryphMaria(s, &a) && a.state.step == 1 && a.state.x == 448 * 65536 + 7 &&
          a.state.animation_set == 0x8004 && a.state.frame == 1 && a.state.texture_offset == 80 &&
          a.state.palette == 0x220 && !a.state.hitbox_state);
    CHECK(ActorHippogryphMaria(s, &a) && a.state.step == 1);
    s->death_event.flags = 4;
    CHECK(ActorHippogryphMaria(s, &a) && a.state.step == 2);
    s->death_event.flags = 8;
    CHECK(ActorHippogryphMaria(s, &a) && a.state.step == 3 && a.state.frame == 2);
    for (Uint16 step = 3; step <= 12; ++step)
    {
        s->death_event.flags = 0;
        CHECK(ActorHippogryphMaria(s, &a) && a.state.step == step);
        s->death_event.flags = 1U << (step + 1U);
        CHECK(ActorHippogryphMaria(s, &a) && a.state.step == step + 1 && !a.pose && !a.remaining);
    }
    CHECK(ActorHippogryphMaria(s, &a) && ActorHippogryphMaria(s, &a) && a.state.step == 14 &&
          a.vx == 49152);
    for (Uint16 i = 0; i < 40 && !a.remove; ++i)
        CHECK(ActorHippogryphMaria(s, &a));
    CHECK(a.remove && (s->death_event.flags & 2U));
    a = (Actor){.state = {.id = 0x2c, .generation = 2, .step = 5, .x = 448 * 65536}};
    s->hippogryph_skip = true;
    CHECK(ActorHippogryphMaria(s, &a) && a.state.step == 13 && a.pose == 1);
    CHECK(ActorHippogryphMaria(s, &a) && a.state.step == 14 && a.vx == 49152);
    SotnActors_Destroy(s);
    return true;
}
static bool Failures(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = {.state = {.id = 0x2b, .generation = 1, .step = 1}, .substep = 1};
    CHECK(!ActorHippogryphMaria(NULL, &a) && !ActorHippogryphMaria(s, NULL));
    CHECK(!ActorHippogryphMaria(s, &a));
    a.state.id = 0x2c;
    a.state.step = 3;
    f->stage[0xc24] = 0;
    CHECK(!ActorHippogryphMaria(s, &a));
    a.state.step = 14;
    a.state.x = SDL_MAX_SINT32;
    a.vx = 1;
    CHECK(!ActorHippogryphMaria(s, &a) && a.state.x == SDL_MAX_SINT32);
    a.state.id = 0x2b;
    a.state.step = 3;
    s->player.x = 500 * 65536;
    CHECK(!ActorHippogryphMaria(s, &a));
    a.state.generation = 0;
    CHECK(!ActorHippogryphMaria(s, &a));
    a.state.generation = 1;
    s->stage_id = 6;
    CHECK(!ActorHippogryphMaria(s, &a));
    s->stage_id = 25;
    a.state.id = 0x2a;
    CHECK(!ActorHippogryphMaria(s, &a));
    SotnActors_Destroy(s);
    return true;
}
static bool Memory(Fixture *f)
{
    for (Uint16 kind = 0; kind < 2; ++kind)
    {
        SotnActors *s = NULL;
        CHECK(Open(f, &s));
        for (Uint16 i = 0; i < 4; ++i)
        {
            // Original synthetic layouts and colors, deliberately different between pictures.
            Put16(f->stage + 0xcf0 + kind * 16U + i * 4U, (Uint16)(50 + kind * 30 + i * 8));
            Put16(f->stage + 0xcf2 + kind * 16U + i * 4U, (Uint16)(70 + i * 12));
            Put16(f->stage + 0xd30 + kind * 8U + i * 2U, (Uint16)(0x200 + kind * 4 + i));
            f->stage[0xd10 + kind * 16U + i * 4U] = (Uint8)(i * 16);
            f->stage[0xd11 + kind * 16U + i * 4U] = 20;
            f->stage[0xd12 + kind * 16U + i * 4U] = (Uint8)(i * 16 + 15);
            f->stage[0xd13 + kind * 16U + i * 4U] = 51;
        }
        Actor a = {.slot = 134, .state = {.id = (Uint16)(0x2d + kind), .generation = 1}};
        CHECK(ActorHippogryphMemory(s, &a) && a.state.step == 1 &&
              a.state.animation_set == 0x8000 && a.state.primitive_count == 4 &&
              (a.state.flags & 0x800000U));
        ActorPrimitive *first = ActorPrimitiveGet(s, &a, 0);
        ActorPrimitive *last = ActorPrimitiveGet(s, &a, 3);
        CHECK(first && last && first->state.palette == 0x200 + kind * 4 &&
              first->state.priority == 136 && first->state.page == 20 &&
              first->state.screen_space && first->state.offset_x[0] == 50 + kind * 30 &&
              first->state.offset_x[3] == first->state.offset_x[0] && first->state.u[1] == 15 &&
              first->state.u[2] == 0 && first->state.v[2] == 51);
        CHECK(ActorHippogryphMemory(s, &a) && first->state.offset_x[0] == 50 + kind * 30);
        CHECK(ActorHippogryphMemory(s, &a) && first->state.offset_x[0] == 49 + kind * 30);
        for (Uint16 i = 2; i < 32; ++i)
            CHECK(ActorHippogryphMemory(s, &a));
        CHECK(a.state.step == 5 && !a.timer && first->state.offset_x[0] == 46 + kind * 30 &&
              last->state.offset_x[0] == 58 + kind * 30 && last->state.offset_y[0] == 86 &&
              last->state.offset_y[3] == 126);
        for (Uint16 i = 0; i < 192; ++i)
            CHECK(ActorHippogryphMemory(s, &a));
        CHECK(a.state.step == 6 && !a.timer && !a.remove);
        for (Uint16 i = 0; i < 32; ++i)
            CHECK(ActorHippogryphMemory(s, &a));
        CHECK(a.remove && a.state.step == 9 && a.timer == 8 &&
              first->state.offset_x[0] == 50 + kind * 30 &&
              first->state.offset_x[3] == first->state.offset_x[0] &&
              last->state.offset_y[0] == 106 && last->state.offset_y[3] == 106);
        ActorPrimitivesClear(s, &a);
        for (Uint16 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
            CHECK(!s->primitives[i].owner);
        SotnActors_Destroy(s);
    }
    return true;
}
static bool MemoryFailures(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = {.slot = 134, .state = {.id = 0x2d, .generation = 1}};
    CHECK(!ActorHippogryphMemory(NULL, &a) && !ActorHippogryphMemory(s, NULL));
    for (Uint16 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 99;
    CHECK(ActorHippogryphMemory(s, &a) && !a.state.step && !a.state.primitive_count && !a.remove);
    s->primitives[0].owner = s->primitives[1].owner = s->primitives[2].owner =
        s->primitives[3].owner = 0;
    CHECK(ActorHippogryphMemory(s, &a) && a.state.step == 1 && a.state.primitive_count == 4);
    a.timer = 256;
    CHECK(ActorHippogryphMemory(s, &a) && a.state.step == 2 && !a.timer);
    a.timer = 65535;
    CHECK(ActorHippogryphMemory(s, &a) && a.state.step == 2 && !a.timer);
    a.state.step = 0;
    CHECK(!ActorHippogryphMemory(s, &a));
    a.state.step = 10;
    CHECK(!ActorHippogryphMemory(s, &a));
    a.state.step = 1;
    s->primitives[a.primitive[2]].owner = 2;
    a.state.step = 3;
    CHECK(!ActorHippogryphMemory(s, &a));
    s->hippogryph_skip = true;
    CHECK(ActorHippogryphMemory(s, &a) && a.remove);
    a.state.id = 0x2c;
    CHECK(!ActorHippogryphMemory(s, &a));
    s->hippogryph_skip = false;
    a = (Actor){.slot = 135, .state = {.id = 0x2e, .generation = 3}};
    Put16(f->stage + 0xd38, 0x300);
    CHECK(!ActorHippogryphMemory(s, &a) && !a.state.primitive_count);
    Put16(f->stage + 0xd38, 0);
    Put16(f->stage + 0xd00, 0x7fff);
    CHECK(!ActorHippogryphMemory(s, &a));
    Put16(f->stage + 0xd00, 0);
    s->nz0.size = 0xd3f;
    CHECK(!ActorHippogryphMemory(s, &a));
    s->stage_id = 6;
    CHECK(!ActorHippogryphMemory(s, &a));
    SotnActors_Destroy(s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Manager(f) && ManagerVariants(f) && Maria(f) && Failures(f) && Memory(f) &&
                    MemoryFailures(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_HIPPOGRYPH_MARIA: PASS");
    return 0;
}
