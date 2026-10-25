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
    Uint8 dra[0x1200], stage[0x4000], exe[0x23220], tiles[16 * 16 * 2], types[1];
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
        p[i] = (Uint8)(v >> (i * 8U));
}
static bool Open(Fixture *f, SotnActors **s, Uint16 parameter)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->stage + 0x8c8, 88);
    Uint8 *enemy = f->dra + 0x200 + 88 * 40;
    Put16(enemy + 4, 18);
    Put16(enemy + 6, 8);
    Put16(enemy + 8, 0x102);
    Put16(enemy + 12, 19);
    enemy[34] = 8;
    enemy[35] = 12;
    Put32(enemy + 36, 0x08001410);
    const size_t streams[] = {0x2af0, 0x2b04, 0x2b38, 0x2b4c};
    for (Uint16 i = 0; i < 4; ++i)
    {
        Put16(f->stage + streams[i], 2);
        Put16(f->stage + streams[i] + 2, (Uint16)(12 + i * 4));
        Put16(f->stage + streams[i] + 4, i & 1U ? 65535 : 0);
    }
    for (Uint16 i = 0; i < 2; ++i)
    {
        Put16(f->stage + 0x2ae0 + i * 8, (Uint16)(0x38 + (i ? 0x8000 : 0)));
        Put16(f->stage + 0x2ae2 + i * 8, 0x102);
        Put16(f->stage + 0x2ae4 + i * 8, 0x201);
        Put16(f->stage + 0x2ae6 + i * 8, 0x101);
    }
    for (Uint32 i = 0; i <= 1024; ++i)
        Put16(f->exe + 0x1dbcc + i * 2, (Uint16)SDL_round(SDL_sin(i * SDL_PI_D / 2048.0) * 4096.0));
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 65534);
    Put16(f->stage + 0x302, 65534);
    Put16(f->stage + 0x30a, 65535);
    Put16(f->stage + 0x30c, 65535);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 2, s));
    (*s)->executable = (SotnDataView){f->exe, sizeof(f->exe)};
    CHECK(SotnActorLayout_OpenStage((*s)->nz0, 2, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *s, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 16, 16}, 0,
        0));
    (*s)->slots[64] = (Actor){.slot = 64,
                              .dynamic = true,
                              .state = {.id = 0x37,
                                        .generation = 1,
                                        .parameters = parameter,
                                        .x = 128 * 65536,
                                        .y = 128 * 65536}};
    return true;
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActorLayout_Close(f->layout);
    f->layout = NULL;
    SotnActors_Destroy(s);
}
static bool Taps(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s, 0));
    Actor *a = &s->slots[64];
    const SotnPlayerState hero = {
        .x = 64 * 65536, .y = 128 * 65536, .hp = 70, .invulnerable = true};
    CHECK(SotnActors_SetButtons(s, SOTN_BUTTON_LEFT_HAND) && SotnActors_Tick(s, hero, 0, 0));
    CHECK(a->state.step == 1 && a->state.primitive_count == 1 && Pixel(a->state.x) == 184 &&
          a->state.frame == 12);
    CHECK(SotnActors_Tick(s, hero, 0, 0) && s->tapped_buttons == 0 && a->state.step == 1);
    CHECK(SotnActors_SetButtons(s, 0) && SotnActors_Tick(s, hero, 0, 0));
    CHECK(SotnActors_SetButtons(s, SOTN_BUTTON_LEFT_HAND) && SotnActors_Tick(s, hero, 0, 0) &&
          a->state.step == 2 && a->ecto.speed == 15 && a->ecto.excited == 0);
    CHECK(SotnActors_Tick(s, hero, 0, 0) && a->ecto.excited == 32 && s->tapped_buttons == 0);
    for (Uint16 i = 0; i < 200 && a->state.step == 2; ++i)
        CHECK(SotnActors_Tick(s, hero, 0, 0));
    CHECK(a->state.step == 1 && a->ecto.speed == 7);
    Close(f, s);
    return true;
}
static bool BounceDeath(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s, 0));
    Actor *a = &s->slots[64];
    CHECK(ActorLibraryEctoplasm(s, a));
    a->hit_flags = 2;
    CHECK(ActorLibraryEctoplasm(s, a) && a->state.step == 3 && a->substep == 1 &&
          s->combat.sounds[0] == 0x73a);
    a->hit_flags = 0;
    for (Uint16 i = 0; i < 31; ++i)
        CHECK(ActorLibraryEctoplasm(s, a));
    CHECK(a->state.step == 1 && a->ecto.speed == -16 && a->state.scale_x == 256 &&
          a->state.scale_y == 256);
    a->state.flags |= 0x100U;
    a->hit_flags = 2;
    CHECK(ActorLibraryEctoplasm(s, a) && a->state.step == 4 && a->state.hitbox_state == 0 &&
          a->ecto.counter == 1 && a->ecto.scale == 255);
    a->hit_flags = 0;
    for (Uint16 i = 0; i < 256 && !a->remove; ++i)
        CHECK(ActorLibraryEctoplasm(s, a));
    CHECK(a->remove && Pixel(a->state.y) > 256);
    bool sweep = false;
    for (Uint16 i = 0; i < s->combat.sound_count; ++i)
        sweep |= s->combat.sounds[i] == 0x672;
    CHECK(sweep);
    Close(f, s);
    return true;
}
static bool Trails(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s, 0));
    Actor *a = &s->slots[64];
    CHECK(ActorLibraryEctoplasm(s, a));
    a->ecto.counter = 7;
    CHECK(ActorLibraryEctoplasm(s, a) && s->slots[160].state.id == 0x39);
    Actor *emitter = &s->slots[160];
    CHECK(ActorLibraryEctoplasm(s, emitter) && emitter->rose.spawned == 1 &&
          s->slots[161].state.id == 0x38);
    CHECK(ActorLibraryEctoplasm(s, emitter) && emitter->remove && s->slots[162].state.id == 0x38);
    Actor *tail = &s->slots[161];
    CHECK(ActorLibraryEctoplasm(s, tail) && tail->state.hitbox_state == 0 && tail->vy == -65536 &&
          tail->state.frame == 16);
    ++a->state.generation;
    for (Uint16 i = 0; i < 8 && !tail->remove; ++i)
        CHECK(ActorLibraryEctoplasm(s, tail));
    CHECK(tail->remove);
    emitter->remove = false;
    CHECK(ActorLibraryEctoplasm(s, emitter) && emitter->remove);
    Close(f, s);
    return true;
}
static bool SummonCapacity(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s, 16));
    Actor *a = &s->slots[64];
    CHECK(ActorLibraryEctoplasm(s, a) && a->state.step == 32 && a->ecto.radius_x == 0 &&
          a->state.hitbox_state == 0);
    CHECK(ActorLibraryEctoplasm(s, a) && a->state.priority == 120 && a->state.opacity_set &&
          a->state.opacity == 2);
    for (Uint16 i = 0; i < 100 && a->state.step == 32; ++i)
        CHECK(ActorLibraryEctoplasm(s, a));
    CHECK(a->state.step == 1 && a->state.hitbox_state == 3 && !a->state.opacity_set);
    s->library_enemy_flags = 2;
    CHECK(ActorLibraryEctoplasm(s, a) && a->state.step == 4 && (a->state.flags & 0x100U));
    Close(f, s);
    CHECK(Open(f, &s, 0));
    a = &s->slots[64];
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 900;
    CHECK(ActorLibraryEctoplasm(s, a) && a->remove && a->state.primitive_count == 0);
    CHECK(!ActorLibraryEctoplasm(NULL, a) && !ActorLibraryEctoplasm(s, NULL));
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    const bool ok = f && Taps(f) && BounceDeath(f) && Trails(f) && SummonCapacity(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
