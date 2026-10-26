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
    Uint8 dra[0x400], stage[0x1000];
} Fixture;
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    f->dra[0x99] = 2;
    f->dra[0x9a] = 10;
    f->dra[0x9b] = 128;
    Put16(f->stage + 0x8b0, 0x8006);
    Put16(f->stage + 0x8b4, 81);
    Put16(f->stage + 0x8b6, 0x244);
    const Uint16 streams[] = {0xe44, 0xe58, 0xe68, 0xe7c, 0xe84, 0xe8c, 0xe94};
    for (Uint16 i = 0; i < SDL_arraysize(streams); ++i)
    {
        f->stage[streams[i]] = 1;
        f->stage[streams[i] + 1U] = (Uint8)(i + 2U);
        f->stage[streams[i] + 2U] = (i == 0 || i == 1 || i == 6) ? 0 : 255;
    }
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 4, out));
    (*out)->player.x = 128 * 65536;
    (*out)->slots[32] = (Actor){.slot = 32,
                                .dynamic = true,
                                .state = {.id = 0x28,
                                          .generation = 1,
                                          .parameters = 1,
                                          .x = 128 * 65536 + 123,
                                          .y = 100 * 65536 + 456}};
    return true;
}
static bool Init(SotnActors *s)
{
    Actor *a = &s->slots[32], *shadow = &s->slots[33];
    CHECK(ActorOlroxQuarter(s, a) && a->state.step == 1 && a->state.priority == 30 &&
          a->state.palette == 0x245 && a->state.animation_set == 0x8006 &&
          shadow->state.id == 0x28 && shadow->state.parameters == 0x11 &&
          shadow->state.x == 128 * 65536 && shadow->state.y == 100 * 65536);
    CHECK(ActorOlroxQuarter(s, shadow) && shadow->state.step == 8 && shadow->state.priority == 31 &&
          shadow->state.blend_mode == 0x50 && shadow->state.opacity_set &&
          shadow->state.opacity == 128);
    return true;
}
static bool Cycle(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s) && Init(s));
    Actor *a = &s->slots[32], *d = &s->slots[33];
    s->random = 48;
    CHECK(ActorOlroxQuarter(s, a) && a->state.step == 2 && a->prisoner.appeared &&
          a->prisoner.shift == 7 && a->state.x == 135 * 65536 + 123);
    a->state.y = 120 * 65536 + 789;
    d->state.x += 333;
    d->state.y += 456;
    CHECK(ActorOlroxQuarter(s, d) && d->state.x == 135 * 65536 + 333 &&
          d->state.y == 120 * 65536 + 456);
    for (Uint16 i = 0; i < 128; ++i)
    {
        CHECK(ActorOlroxQuarter(s, a) && d->state.opacity == 127 - i);
        CHECK(a->state.step == (i == 127 ? 3 : 2));
    }
    s->player.x = a->state.x;
    CHECK(ActorOlroxQuarter(s, a) && d->state.priority == 106 && !d->state.blend_mode &&
          !d->state.opacity_set && !d->state.opacity && d->state.frame == a->state.frame + 2);
    CHECK(ActorOlroxQuarter(s, a) && s->combat.sound_count == 1 && s->combat.sounds[0] == 0x7c0);
    s->player.x = a->state.x + 64 * 65536;
    CHECK(ActorOlroxQuarter(s, a) && a->state.step == 3);
    s->player.x += 65536;
    CHECK(ActorOlroxQuarter(s, a) && a->state.step == 4 && !a->substep &&
          d->state.blend_mode == 0x50 && d->state.opacity_set && d->state.priority == 31);
    s->random = 1;
    CHECK(ActorOlroxQuarter(s, a) && ActorOlroxQuarter(s, a) && a->state.step == 5 &&
          !d->state.frame);
    for (Uint16 i = 0; i < 129; ++i)
        CHECK(ActorOlroxQuarter(s, a) && d->state.opacity == i + 1U &&
              a->state.step == (i == 128 ? 1 : 5));
    CHECK(a->state.x == 128 * 65536 + 123 && d->prisoner.opacity == 129);
    s->player.x = a->state.x;
    s->random = 48;
    CHECK(ActorOlroxQuarter(s, a) && a->state.step == 2);
    for (Uint16 i = 0; i < 129; ++i)
        CHECK(ActorOlroxQuarter(s, a) && a->state.step == (i == 128 ? 3 : 2));
    SotnActors_Destroy(s);
    return true;
}
static bool Decisions(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s) && Init(s));
    Actor *a = &s->slots[32], *d = &s->slots[33];
    s->random = 1;
    s->player.x = a->state.x + 64 * 65536;
    CHECK(ActorOlroxQuarter(s, a) && a->state.step == 1 && s->random == 1);
    s->player.x -= 65536;
    CHECK(ActorOlroxQuarter(s, a) && a->state.step == 6);
    s->player.x = a->state.x + 64 * 65536;
    CHECK(ActorOlroxQuarter(s, a) && a->state.step == 6);
    s->player.x += 65536;
    CHECK(ActorOlroxQuarter(s, a) && a->state.step == 1);
    // Already-seen prisoners use four random bits, not the first-appearance coin toss.
    s->player.x = a->state.x;
    a->prisoner.appeared = true;
    s->random = 2;
    CHECK(ActorOlroxQuarter(s, a) && a->state.step == 6);
    SetStep(a, 4);
    s->random = 2;
    CHECK(ActorOlroxQuarter(s, a) && ActorOlroxQuarter(s, a) && a->substep == 1);
    CHECK(ActorOlroxQuarter(s, a) && ActorOlroxQuarter(s, a) && a->substep == 2);
    CHECK(ActorOlroxQuarter(s, a) && ActorOlroxQuarter(s, a) && a->state.step == 3);
    SetStep(a, 4);
    a->substep = 1;
    s->player.x = a->state.x + 64 * 65536;
    CHECK(ActorOlroxQuarter(s, a) && ActorOlroxQuarter(s, a) && a->substep == 3);
    CHECK(ActorOlroxQuarter(s, a) && ActorOlroxQuarter(s, a) && a->state.step == 5);
    // Original byte arithmetic wraps, even in artificially resumed phases.
    d->prisoner.opacity = 255;
    CHECK(ActorOlroxQuarter(s, a) && d->prisoner.opacity == 0 && a->state.step == 5);
    SetStep(a, 2);
    CHECK(ActorOlroxQuarter(s, a) && d->prisoner.opacity == 255 && a->state.step == 2);
    a->state.x = 32767 * 65536 + 321;
    a->prisoner.appeared = false;
    SetStep(a, 1);
    s->player.x = a->state.x;
    s->random = 48;
    CHECK(ActorOlroxQuarter(s, a) && a->state.x == -32762 * 65536 + 321);
    SotnActors_Destroy(s);
    return true;
}
static bool OwnershipFailures(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[32], *d = &s->slots[33];
    *d = (Actor){.state = {.id = 0x29, .generation = 9}};
    CHECK(!ActorOlroxQuarter(s, a) && d->state.id == 0x29 && d->state.generation == 9);
    *d = (Actor){0};
    a->state.step = 0;
    CHECK(Init(s));
    const Uint32 old = d->state.generation;
    a->state.generation = 2;
    CHECK(ActorOlroxQuarter(s, d) && d->remove);
    a->state.step = 0;
    CHECK(ActorOlroxQuarter(s, a) && d->state.generation != old && !d->remove &&
          d->enemy_state.owner_generation == 2);
    CHECK(ActorOlroxQuarter(s, d));
    ++d->state.generation;
    CHECK(!ActorOlroxQuarter(s, a));
    --d->state.generation;
    a->state.step = 7;
    CHECK(!ActorOlroxQuarter(s, a));
    SetStep(a, 4);
    a->substep = 4;
    CHECK(!ActorOlroxQuarter(s, a));
    SetStep(a, 2);
    a->pose = 512;
    CHECK(!ActorOlroxQuarter(s, a));
    a->pose = 0;
    f->stage[0xe44] = 0;
    CHECK(!ActorOlroxQuarter(s, a));
    a->state.step = 0;
    s->effect_serial = 0x7fffffffU;
    CHECK(!ActorOlroxQuarter(s, a) && d->state.generation);
    Actor last = {.slot = 191, .state = {.id = 0x28, .generation = 1}};
    CHECK(!ActorOlroxQuarter(s, &last));
    CHECK(!ActorOlroxPrisoner(NULL, a) && !ActorOlroxPrisoner(s, NULL));
    SotnActors_Destroy(s);
    return true;
}
static bool ServiceTick(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    SotnActorLayout *layout = NULL;
    Uint8 tiles[16 * 16 * 2] = {0}, types[1] = {0};
    const Actor parent = s->slots[32];
    f->stage[28] = 0x40;
    f->stage[30] = 0x18;
    f->stage[31] = 0x80;
    for (Uint16 i = 0; i < 106; ++i)
    {
        f->stage[0x40 + i * 4U + 1U] = 3;
        f->stage[0x40 + i * 4U + 2U] = 0x18;
        f->stage[0x40 + i * 4U + 3U] = 0x80;
    }
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    CHECK(SotnActorLayout_OpenStage((SotnDataView){f->stage, sizeof(f->stage)}, 4, 0, &layout));
    CHECK(SotnActors_Enter(
        s, layout, (SotnCollisionMap){{tiles, sizeof(tiles)}, {types, sizeof(types)}, 16, 16}, 0,
        0));
    s->slots[32] = parent;
    CHECK(SotnActors_Tick(s, s->player, 0, 0) && s->slots[32].state.step == 1 &&
          s->slots[33].state.step == 8);
    s->random = 48;
    CHECK(SotnActors_Tick(s, s->player, 0, 0) && s->slots[32].state.step == 2 &&
          s->slots[33].state.x == Pixel(s->slots[32].state.x) * 65536);
    for (Uint16 i = 0; i < 128; ++i)
        CHECK(SotnActors_Tick(s, s->player, 0, 0));
    CHECK(s->slots[32].state.step == 3 && !s->slots[33].prisoner.opacity);
    SotnActors_Destroy(s);
    SotnActorLayout_Close(layout);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    const bool ok = f && Cycle(f) && Decisions(f) && OwnershipFailures(f) && ServiceTick(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_OLROX_PRISONER: PASS");
    return 0;
}
