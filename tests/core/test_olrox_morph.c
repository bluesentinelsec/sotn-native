#include "olrox_internal.h"
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
    Uint8 dra[0x1000], stage[0x4000], exe[0x24000], tiles[32 * 32 * 2], types[2];
    SotnActorLayout *layout;
} Fixture;
static void Put16(Uint8 *p, Sint32 n)
{
    p[0] = (Uint8)n;
    p[1] = (Uint8)((Uint32)n >> 8);
}
static void Put32(Uint8 *p, Uint32 n)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(n >> (i * 8));
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 12);
    Put16(f->dra + 0x20c, 3);
    const size_t inits[] = {0x6cc};
    for (Uint16 i = 0; i < SDL_arraysize(inits); ++i)
    {
        Put16(f->stage + inits[i], 0x8002);
        Put16(f->stage + inits[i] + 2, 3);
        Put16(f->stage + inits[i] + 6, 2);
    }
    for (Uint16 i = 0; i < 66; ++i)
    {
        Put16(f->stage + 0xdc4 + i * 4U, 16 + (i % 8) * 2);
        Put16(f->stage + 0xdc6 + i * 4U, 39 + (i / 8) * 2);
        Put16(f->stage + 0xee0 + i * 4U, 64 + (i % 8) * 4);
        Put16(f->stage + 0xee2 + i * 4U, 112 + (i / 8) * 4);
    }
    for (Uint16 i = 0; i < 104; ++i)
        f->stage[0xffc + i] = (Uint8)(i % 66);
    for (Uint16 i = 0; i < 156; ++i)
        f->stage[0x10bc + i] = (Uint8)(i % 66);
    for (Uint16 i = 0; i < 9; ++i)
        Put16(f->stage + 0x117c + i * 2U, i == 8 ? 512 : (i & 1U) ? 8 : 1);
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 30, out));
    CHECK(SotnActorLayout_OpenStage((SotnDataView){f->stage, sizeof(f->stage)}, 30, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *out, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 32}, 0,
        0));
    (*out)->executable = (SotnDataView){f->exe, sizeof(f->exe)};
    (*out)->slots[32] = (Actor){
        .slot = 32,
        .state = {
            .id = 0x2b, .generation = 3, .x = 128 * 65536, .y = 128 * 65536, .priority = 136}};
    (*out)->player = (SotnPlayerState){.x = 160 * 65536, .y = 128 * 65536};
    return true;
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    f->layout = NULL;
}
static bool Sequence(Fixture *f)
{
    for (Uint16 facing = 0; facing < 2; ++facing)
    {
        SotnActors *s = NULL;
        CHECK(Open(f, &s));
        Actor *a = NULL;
        CHECK(OlroxReserved(s, &s->slots[32], 1, 0x32, 0, &a));
        a->state.x = 200 * 65536 + 123;
        a->state.y = 300 * 65536 + 456;
        a->state.facing_left = facing != 0;
        CHECK(ActorOlroxMorph(s, a) && a->state.step == 2 && a->state.primitive_count == 68 &&
              s->olrox_morph.transforming && s->olrox_morph.generation == a->state.generation &&
              !a->state.frame && !a->state.hitbox_state && a->state.blend_mode == 0x30);
        Actor *child = OlroxChild(s, a, 1);
        CHECK(child && child->state.id == 0x33 && !child->state.facing_left &&
              child->state.parameters == facing && child->state.x == 64 * 65536 &&
              child->state.y == 492 * 65536);
        SotnActorPrimitive *output = &ActorPrimitiveGet(s, a, 0)->state,
                           *begin = &ActorPrimitiveGet(s, a, 1)->state,
                           *restore = &ActorPrimitiveGet(s, a, 2)->state;
        CHECK(!output->active && output->page == 0x110 && output->priority == 0xe0 &&
              begin->target == 1 && begin->clip_top == 256 && begin->clip_bottom == 512 &&
              begin->target_offset_y == 0 && begin->clip_right == 127 && begin->priority == 0xcb &&
              restore->target == 2 && restore->priority == 0xd4);
        CHECK(ActorOlroxMorph(s, a) && a->state.step == 3 &&
              a->state.x == (facing ? 196 : 204) * 65536 + 123 && a->state.y == 300 * 65536 + 456 &&
              a->olrox.morph_timer == 128 && a->olrox.morph_brightness == -32768 &&
              begin->clip_bottom == 384 && begin->target_offset_y == 256 &&
              begin->priority == 0xcf && restore->priority == 0xd1 && s->combat.sounds[0] == 0x819);
        CHECK(ActorPrimitiveGet(s, a, 3)->state.page == 0x33 &&
              ActorPrimitiveGet(s, a, 29)->state.page == 0x130 &&
              ActorPrimitiveGet(s, a, 3)->state.red == 128 &&
              ActorPrimitiveGet(s, a, 29)->state.red == 0);
        Uint16 ticks = 0;
        bool pulsed = false;
        while (!a->remove && ticks++ < 1000)
        {
            ++s->ticks;
            s->combat.sound_count = 0;
            CHECK(ActorOlroxMorph(s, a));
            pulsed |= a->state.step == 4;
        }
        CHECK(a->remove && ticks < 1000 && pulsed && !s->olrox_morph.transforming &&
              !s->olrox_morph.generation && output->active &&
              s->olrox_morph.points[17][0] == 4 * 65536 &&
              s->olrox_morph.points[17][1] == 8 * 65536 &&
              child->state.x == (facing ? 196 : 204) * 65536 && child->state.y == 460 * 65536);
        Close(f, s);
    }
    return true;
}
static bool Boundaries(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = NULL;
    CHECK(OlroxReserved(s, &s->slots[32], 1, 0x32, 0, &a));
    f->stage[0xffc] = 66;
    CHECK(!ActorOlroxMorph(s, a) && !a->state.step && !a->state.primitive_count);
    f->stage[0xffc] = 0;
    Put16(f->stage + 0xdc4, 513);
    CHECK(!ActorOlroxMorph(s, a));
    Put16(f->stage + 0xdc4, 16);
    Put16(f->stage + 0x117c, 0);
    CHECK(!ActorOlroxMorph(s, a));
    Put16(f->stage + 0x117c, 1);
    s->olrox_morph.generation = a->state.generation + 1;
    CHECK(!ActorOlroxMorph(s, a));
    s->olrox_morph.generation = 0;
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 99;
    CHECK(ActorOlroxMorph(s, a) && a->state.step == 0 && !a->state.primitive_count &&
          s->olrox_morph.transforming);
    SDL_zero(s->primitives);
    CHECK(ActorOlroxMorph(s, a) && a->state.step == 2 && a->state.primitive_count == 68);
    a->state.step = 6;
    ++s->slots[a->slot + 1U].state.generation;
    CHECK(!ActorOlroxMorph(s, a) && s->olrox_morph.transforming);
    a->state.step = 9;
    CHECK(!ActorOlroxMorph(s, a));
    a->state.generation = 0;
    CHECK(!ActorOlroxMorph(s, a));
    Close(f, s);
    CHECK(Open(f, &s));
    CHECK(OlroxReserved(s, &s->slots[32], 1, 0x32, 0, &a));
    s->slots[34] = (Actor){.state = {.id = 0x2d, .generation = 18},
                           .enemy_state = {.owner_slot = 1, .owner_generation = 99}};
    CHECK(!ActorOlroxMorph(s, a) && s->slots[34].state.generation == 18);
    Close(f, s);
    CHECK(Open(f, &s));
    CHECK(OlroxReserved(s, &s->slots[32], 1, 0x32, 0, &a));
    Actor *sibling = &s->slots[34];
    *sibling = (Actor){.slot = 34,
                       .state = {.id = 0x2d, .generation = 18},
                       .enemy_state = {.owner_slot = 32, .owner_generation = 3}};
    CHECK(ActorPrimitives(s, sibling, 1));
    const Uint16 old_primitive = sibling->primitive[0];
    CHECK(ActorOlroxMorph(s, a) && s->slots[34].state.id == 0x33 &&
          s->slots[34].state.generation != 18 && !s->primitives[old_primitive].owner);
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool okay = Sequence(f) && Boundaries(f);
    SDL_free(f);
    return okay ? 0 : 1;
}
