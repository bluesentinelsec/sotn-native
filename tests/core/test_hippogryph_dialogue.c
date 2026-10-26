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
    Uint8 dra[0x1000], stage[0x6000], tiles[32 * 16 * 2], types[2];
} Fixture;
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    Put32(f->stage + 0xb74, 1);
    f->stage[0xb79] = 64;
    Put16(f->stage + 0xb80, 0x210);
    Put16(f->stage + 0xb82, 0x218);
    Put16(f->stage + 0xb86, 32);
    Put16(f->stage + 0xb88, 0x1a1);
    Put16(f->stage + 0xb8a, 0x1a1);
    for (Uint16 i = 0; i < 72; ++i)
        Put16(f->stage + 0xb8c + i * 2U, 80);
    Put32(f->stage + 0xc1c, 0x80182200);
    Put32(f->stage + 0xc20, 0x80182210);
    const Uint8 name[] = {0x11, 0x12, 255, 0};
    SDL_memcpy(f->stage + 0x2200, name, sizeof(name));
    SDL_memcpy(f->stage + 0x2210, name, sizeof(name));
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
    return true;
}
static Actor Owner(void)
{
    return (Actor){.slot = 136, .state = {.id = 0x2a, .generation = 1}};
}
static bool Presentation(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    const Uint8 script[] = {7, 60, 28, 5, 0, 0, 'A', 1, 'B', 6, 8, 0};
    SDL_memcpy(f->stage + 0x1d5c, script, sizeof(script));
    Actor a = Owner();
    CHECK(ActorHippogryphDialogue(s, &a) && a.state.step == 1 && a.state.primitive_count == 7 &&
          s->death_event.active && s->death_event.display.primitive_text);
    CHECK(ActorHippogryphDialogue(s, &a) && a.state.step == 5 && a.substep == 0);
    Uint16 opening = 0;
    while (a.state.step == 5 && opening < 200)
    {
        CHECK(ActorHippogryphDialogue(s, &a));
        ++opening;
    }
    // Original instructions: one setup, 35 line-motion and 17 color-transition ticks.
    CHECK(opening == 53 && a.state.step == 1 && a.state.primitive_count == 7 && a.dialogue.started);
    CHECK(ActorHippogryphDialogue(s, &a) && a.state.step == 3 && a.state.primitive_count == 9 &&
          a.timer == 6);
    ActorPrimitive *portrait = ActorPrimitiveGet(s, &a, 5), *name = ActorPrimitiveGet(s, &a, 7);
    CHECK(portrait && name && portrait->state.palette == 0x210 && portrait->state.page == 0x90 &&
          portrait->state.offset_x[0] == 30 && !name->state.active);
    for (Uint16 i = 0; i < 6; ++i)
        CHECK(ActorHippogryphDialogue(s, &a));
    CHECK(a.state.step == 1 && portrait->state.offset_x[0] == 6 &&
          portrait->state.offset_x[1] == 54 && name->state.active);
    CHECK(ActorHippogryphDialogue(s, &a) && s->death_event.display.lines[0][0] == 'A');
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(ActorHippogryphDialogue(s, &a));
    CHECK(ActorHippogryphDialogue(s, &a) && s->death_event.display.row == 1 &&
          s->death_event.display.lines[1][0] == 'B');
    for (Uint16 i = 0; i < 100 && a.state.step != 7; ++i)
        CHECK(ActorHippogryphDialogue(s, &a));
    CHECK(a.state.step == 7 && !a.remove && !s->world->flags[SOTN_MARIA_HIPPOGRYPH]);
    CHECK(ActorHippogryphDialogue(s, &a) && a.remove && !s->death_event.active &&
          s->world->flags[SOTN_MARIA_HIPPOGRYPH] && s->world->flags[SOTN_LAB_MARIA] &&
          s->world->cutscenes_seen == 0x300);
    ActorPrimitivesClear(s, &a);
    a = Owner();
    s->hippogryph_encounter = 1;
    CHECK(ActorHippogryphDialogue(s, &a) && a.remove && !s->hippogryph_encounter &&
          !a.state.primitive_count);
    SotnActors_Destroy(s);
    return true;
}
static bool Events(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    const Uint8 script[] = {12, 0x82, 0x24, 0x40, 0, 24, 3, 0};
    const Uint8 events[] = {0, 0, 0, 132, 0x2b, 10,   0,  6, 0, 0, 2,   2,
                            5, 0, 2, 0,   133,  0x2c, 12, 0, 9, 0, 255, 255};
    SDL_memcpy(f->stage + 0x1d5c, script, sizeof(script));
    SDL_memcpy(f->stage + 0x2400, events, sizeof(events));
    Actor a = Owner();
    CHECK(ActorHippogryphDialogue(s, &a) && ActorHippogryphDialogue(s, &a));
    CHECK(s->death_event.events == 0x2400);
    for (Uint16 i = 0; i < 10; ++i)
        CHECK(ActorHippogryphDialogue(s, &a));
    CHECK(s->death_event.event_timer == 1 && s->slots[132].state.id == 0x2b &&
          s->slots[132].state.x == 352 * 65536 && !s->slots[133].state.generation);
    s->death_event.flags |= 1U << 5;
    CHECK(ActorHippogryphDialogue(s, &a) && s->death_event.event_timer == 2 &&
          s->slots[133].state.id == 0x2c && s->slots[133].state.x == 384 * 65536 &&
          !(s->death_event.flags & (1U << 5)));
    s->death_event.flags |= 1U << 3;
    CHECK(ActorHippogryphDialogue(s, &a) && a.state.step == 7 &&
          (s->death_event.flags & (1U << 3)));
    SotnActors_Destroy(s);
    return true;
}
static bool Skip(Fixture *f)
{
    for (Uint16 mode = 0; mode < 3; ++mode)
    {
        SotnActors *s = NULL;
        CHECK(Open(f, &s));
        const Uint8 script[] = {7, 60, 28, 23, 0};
        SDL_memcpy(f->stage + 0x1d5c, script, sizeof(script));
        Actor a = Owner();
        CHECK(ActorHippogryphDialogue(s, &a) && ActorHippogryphDialogue(s, &a) &&
              ActorHippogryphDialogue(s, &a));
        s->world->cutscenes_seen = mode == 1 ? 0x200 : 0;
        s->world->game_clear = mode == 2;
        s->tapped_buttons = SOTN_BUTTON_START | SOTN_BUTTON_RIGHT;
        CHECK(ActorHippogryphDialogue(s, &a) && !s->hippogryph_skip);
        s->tapped_buttons = SOTN_BUTTON_START;
        CHECK(ActorHippogryphDialogue(s, &a));
        CHECK(s->hippogryph_skip == (mode != 0));
        if (mode)
        {
            CHECK(!a.state.primitive_count && a.state.step == 1 && s->combat.music_change &&
                  !s->combat.music && s->combat.voice_stop);
            for (Uint16 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
                CHECK(!s->primitives[i].owner);
            CHECK(ActorHippogryphDialogue(s, &a) && a.state.step == 7 && !a.remove);
            CHECK(ActorHippogryphDialogue(s, &a) && a.remove);
        }
        SotnActors_Destroy(s);
    }
    return true;
}
static bool Failures(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = Owner();
    CHECK(!ActorHippogryphDialogue(NULL, &a) && !ActorHippogryphDialogue(s, NULL));
    for (Uint16 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 99;
    CHECK(ActorHippogryphDialogue(s, &a) && !a.state.step && !a.remove);
    SDL_zero(s->primitives);
    CHECK(ActorHippogryphDialogue(s, &a));
    s->death_event.script = s->nz0.size;
    CHECK(!ActorHippogryphDialogue(s, &a));
    s->death_event.script = 0x1d5c;
    f->stage[0x1d5c] = 14;
    CHECK(!ActorHippogryphDialogue(s, &a));
    f->stage[0x1d5c] = 5;
    f->stage[0x1d5d] = 2;
    s->death_event.script = 0x1d5c;
    CHECK(!ActorHippogryphDialogue(s, &a));
    a.state.step = 8;
    CHECK(!ActorHippogryphDialogue(s, &a));
    a.state.id = 0x2b;
    CHECK(!ActorHippogryphDialogue(s, &a));
    a.state.id = 0x2a;
    s->stage_id = 6;
    CHECK(!ActorHippogryphDialogue(s, &a));
    SotnActors_Destroy(s);
    CHECK(Open(f, &s));
    a = Owner();
    CHECK(ActorHippogryphDialogue(s, &a));
    const Uint8 opening[] = {7, 60, 28};
    SDL_memcpy(f->stage + 0x1d5c, opening, sizeof(opening));
    CHECK(ActorHippogryphDialogue(s, &a));
    Put16(f->stage + 0xb8c, 0xffff);
    CHECK(!ActorHippogryphDialogue(s, &a));
    Put16(f->stage + 0xb8c, 80);
    for (Uint16 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        if (!s->primitives[i].owner)
            s->primitives[i].owner = 99;
    CHECK(ActorHippogryphDialogue(s, &a) && a.remove);
    SotnActors_Destroy(s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    const bool ok = f && Presentation(f) && Events(f) && Skip(f) && Failures(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_HIPPOGRYPH_DIALOGUE: PASS");
    return 0;
}
