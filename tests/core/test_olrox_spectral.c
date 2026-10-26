#include "actors_internal.h"
#include "boss_projection.h"
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
    Uint8 dra[0x1000], stage[0x4000], exe[0x23220], tiles[2048], types[2];
    SotnActorLayout *layout;
} Fixture;
static void Put16(Uint8 *p, Sint32 v)
{
    p[0] = (Uint8)(Uint16)v;
    p[1] = (Uint8)((Uint16)v >> 8);
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
    for (Uint16 i = 0; i < 3; ++i)
    {
        Uint8 *p = f->dra + 0x200 + i * 40U;
        Put16(p + 4, i == 2 ? 32767 : 60 - i * 40);
        Put16(p + 6, 24 - i * 5);
        Put16(p + 12, 3);
        p[34] = 8;
        p[35] = 12;
        Put32(p + 36, 0x08000000);
    }
    const size_t init[] = {0x970, 0x988, 0x97c};
    for (Uint16 i = 0; i < 3; ++i)
    {
        Put16(f->stage + init[i], 0x800d);
        Put16(f->stage + init[i] + 4, 0x51);
        Put16(f->stage + init[i] + 6, 0x250 + i);
        Put16(f->stage + init[i] + 8, i);
    }
    for (Uint16 i = 0; i < 17; ++i)
    {
        f->stage[0x336c + i * 4U] = (Uint8)i;
        f->stage[0x336d + i * 4U] = (Uint8)(-1 - (Sint32)i);
        f->stage[0x336e + i * 4U] = (Uint8)(i % 7 + 1);
        f->stage[0x336f + i * 4U] = (Uint8)(i % 5 + 2);
        if (i < 16)
        {
            f->stage[0x33b0 + i * 2U] = (Uint8)(4 + i);
            f->stage[0x33b1 + i * 2U] = (Uint8)(i % 5 + 2);
            Put16(f->stage + 0x33d0 + i * 2U, i * 113);
        }
    }
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    // Synthetic, deliberately non-sinusoidal and biased SDK coefficients.
    for (Uint32 i = 0; i <= 1024; ++i)
    {
        Put16(f->exe + 0x1dbcc + i * 2U, (Sint32)i * 4);
        Put16(f->exe + 0x22a10 + i * 2U, (Sint32)i / 2);
    }
    for (Uint32 i = 0; i < 192; ++i)
        Put16(f->exe + 0x1e3e0 + i * 2U, i ? 5000 : 4096);
    // Authored triangular matrix pairs; private original tables are never copied.
    for (Uint32 i = 0; i < 4096; ++i)
    {
        const Uint32 phase = i & 1023U, next = (i + 1024U) & 4095U;
        const Sint32 sine = i < 1024   ? (Sint32)phase * 4
                            : i < 2048 ? 4096 - (Sint32)phase * 4
                            : i < 3072 ? -(Sint32)phase * 4
                                       : -4096 + (Sint32)phase * 4;
        const Uint32 p = next & 1023U;
        const Sint32 cosine = next < 1024   ? (Sint32)p * 4
                              : next < 2048 ? 4096 - (Sint32)p * 4
                              : next < 3072 ? -(Sint32)p * 4
                                            : -4096 + (Sint32)p * 4;
        Put16(f->exe + 0x1ea10 + i * 4U, sine);
        Put16(f->exe + 0x1ea12 + i * 4U, cosine);
    }
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 4, out));
    CHECK(SotnActors_SetServices(*out, (SotnDataView){f->exe, sizeof(f->exe)}, 1, false));
    CHECK(SotnActorLayout_OpenStage((SotnDataView){f->stage, sizeof(f->stage)}, 4, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *out, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 32}, 0,
        0));
    (*out)->player = (SotnPlayerState){.x = 188 * 65536, .y = 128 * 65536, .hp = 70, .hp_max = 70};
    (*out)->slots[24] = (Actor){
        .slot = 24,
        .dynamic = true,
        .state = {.id = 0x3e, .generation = 1, .x = 128 * 65536 + 123, .y = 128 * 65536 + 456}};
    return true;
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    f->layout = NULL;
}
static bool Family(SotnActors *s)
{
    Actor next = s->slots[24];
    CHECK(ActorOlroxQuarter(s, &next));
    s->slots[24] = next;
    for (Uint16 i = 25; i <= 39; ++i)
        CHECK(ActorOlroxQuarter(s, &s->slots[i]));
    ++s->service_ticks;
    return true;
}
static bool Init(SotnActors *s)
{
    CHECK(Family(s));
    const Actor *a = &s->slots[24];
    CHECK(a->state.step == 1 && a->spectral.count == 15 && !a->no2_spectral.formation &&
          a->state.frame == 1 && a->state.priority == 176 && a->state.blend_mode == 0x30 &&
          a->state.hitbox_y == 4 && a->state.primitive_count == 1);
    for (Uint16 i = 1; i <= 15; ++i)
    {
        const Actor *w = &s->slots[24 + i];
        CHECK(w->state.id == 0x40 && w->state.parameters == i && w->state.frame == i + 1 &&
              w->state.step == 1 && w->state.y == 1408 * 65536 && !w->state.hitbox_state &&
              w->spectral.timer == 64 && ActorNo2SpectralOwner(s, w) == a &&
              w->state.hitbox_width == i % 5 + 2);
    }
    return true;
}
static bool Projection(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    SotnProjectedPoint point = {0};
    Uint16 depth = 0;
    CHECK(SotnNo2_ProjectOrbit(s->executable, 0, 0, 0, 64, 128, 128, &point, &depth) &&
          point.x == 128 && point.y == 192 && depth == 256);
    CHECK(SotnNo2_ProjectOrbit(s->executable, 1024, 0, 0, 64, 128, 128, &point, &depth) &&
          point.x == 128 && point.y == 128 && depth == 272);
    CHECK(SotnNo2_ProjectOrbit(s->executable, 0, 0, 1024, 64, 128, 128, &point, &depth) &&
          point.x == 64 && point.y == 128 && depth == 256);
    point = (SotnProjectedPoint){123, 456};
    depth = 789;
    CHECK(!SotnNo2_ProjectOrbit(s->executable, 0, 0, 0, 177, 0, 0, &point, &depth) &&
          point.x == 123 && point.y == 456 && depth == 789);
    Close(f, s);
    return true;
}
static bool Sequence(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s) && Init(s));
    Actor *a = &s->slots[24];
    CHECK(Family(s) && a->state.step == 2 && Family(s) && a->spectral.timer == 255 &&
          a->no2_spectral.formation == 1);
    for (Uint16 i = 0; i < 64; ++i)
        CHECK(Family(s));
    CHECK(s->slots[25].state.hitbox_state == 3);
    for (Uint16 i = 0; i < 191; ++i)
        CHECK(Family(s));
    CHECK(a->state.step == 4 && Family(s) && a->no2_spectral.formation == 3 &&
          a->spectral.moves == 511 && a->no2_spectral.rot_x == 8);
    CHECK(s->slots[25].state.priority >= 170 && s->slots[25].spectral.target_x != 0);
    for (Uint16 i = 0; i < 511; ++i)
        CHECK(Family(s));
    CHECK(a->state.step == 5 && Family(s) && a->no2_spectral.formation == 2 &&
          a->spectral.timer == 255);
    SetStep(a, 5);
    a->substep = 2;
    s->player.x = a->state.x;
    CHECK(Family(s) && a->substep == 3 && a->no2_spectral.formation == 1 &&
          a->spectral.moves == 31);
    bool thrown = false;
    for (Uint16 i = 0; i < 100; ++i)
    {
        CHECK(Family(s));
        thrown |= s->slots[25].state.step == 3;
    }
    CHECK(thrown && a->no2_spectral.cursor >= 3);
    // Each distinct weapon reduces the orbit count once; empty formations never divide by zero.
    for (Uint16 i = 25; i <= 39; ++i)
        s->slots[i].state.flags |= 0x100U;
    CHECK(Family(s) && !a->spectral.count && Family(s) && !a->no2_spectral.formation);
    CHECK(Family(s) && !a->spectral.count);
    a->state.flags |= 0x100U;
    CHECK(Family(s) && a->state.step == 8 && !a->state.hitbox_state && a->state.rotated);
    const Sint32 y = a->state.y;
    CHECK(Family(s) && a->state.y == y + a->vy - 6144 && a->state.rotation == -4 &&
          (a->state.flags & 0x80000000U));
    Close(f, s);
    return true;
}
static bool ContactAndOwnership(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s) && Init(s));
    Actor *a = &s->slots[24], *w = &s->slots[25];
    const Sint16 hp = a->state.hit_points;
    a->state.hitbox_width = a->state.hitbox_height = 0;
    a->stun = 4;
    w->state.x = 188 * 65536;
    w->state.y = 128 * 65536;
    w->state.hitbox_x = w->state.hitbox_y = 0;
    w->state.hitbox_width = w->state.hitbox_height = 8;
    w->state.hitbox_state = 3;
    SotnPlayerState player = s->player;
    player.invulnerable = true;
    player.attack = (SotnPlayerAttack){.active = true,
                                       .power = 10,
                                       .element = 0x40,
                                       .hit_type = 2,
                                       .invincibility_frames = 8,
                                       .width = 3,
                                       .height = 3};
    CHECK(SotnActors_Tick(s, player, 0, 0) && a->state.hit_points == hp &&
          w->state.hit_points < 20);
    const Sint16 damaged = w->state.hit_points;
    CHECK(SotnActors_Tick(s, player, 0, 0) && w->state.hit_points == damaged);
    w->state.flags |= 0x100U;
    CHECK(ActorOlroxQuarter(s, w) && a->spectral.count == 14 && !w->state.hitbox_state &&
          ActorOlroxQuarter(s, w) && a->spectral.count == 14);
    Actor *aura = NULL;
    CHECK(ActorSpawn(s, a, 0x3f, 0, 160, 192, &aura) && aura);
    aura->enemy_state.owner_slot = a->slot;
    aura->enemy_state.owner_generation = a->state.generation;
    CHECK(ActorOlroxQuarter(s, aura) && aura->state.palette == 0x254 &&
          aura->state.opacity == 124 && aura->state.scale_x == 262);
    ++a->state.generation;
    CHECK(!ActorNo2SpectralOwner(s, w) && ActorOlroxQuarter(s, w) && w->remove &&
          ActorOlroxQuarter(s, aura) && aura->remove);
    const Uint32 old = w->state.generation;
    // Room re-entry releases the previous body presentation before recreating it.
    ActorPrimitivesClear(s, a);
    a->state.primitive_count = 0;
    SetStep(a, 0);
    CHECK(Family(s) && s->slots[25].state.generation != old &&
          s->slots[25].enemy_state.owner_generation == a->state.generation);
    Close(f, s);
    return true;
}
static bool Failures(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[24];
    s->slots[39] = (Actor){.state = {.id = 0x28, .generation = 10}};
    CHECK(!ActorOlroxQuarter(s, a) && !s->slots[25].state.generation &&
          s->slots[39].state.generation == 10);
    Close(f, s);
    CHECK(Open(f, &s));
    a = &s->slots[24];
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 0xdeadbeef;
    CHECK(ActorOlroxQuarter(s, a) && a->remove && !s->slots[25].state.generation);
    Close(f, s);
    CHECK(Open(f, &s) && Init(s));
    a = &s->slots[24];
    Actor *w = &s->slots[25];
    w->state.flags |= 0x100U;
    a->spectral.count = 0;
    CHECK(!ActorOlroxQuarter(s, w) && !a->spectral.count);
    a->spectral.count = 16;
    CHECK(!ActorOlroxQuarter(s, a));
    a->spectral.count = 15;
    a->no2_spectral.formation = 4;
    CHECK(!ActorOlroxQuarter(s, a));
    a->no2_spectral.formation = 0;
    a->state.step = 4;
    a->substep = 9;
    CHECK(!ActorOlroxQuarter(s, a));
    a->state.step = 6;
    CHECK(!ActorOlroxQuarter(s, a));
    CHECK(!ActorNo2Spectral(NULL, a) && !ActorNo2Spectral(s, NULL) &&
          !ActorNo2SpectralOwner(NULL, w));
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    const bool ok = f && Projection(f) && Sequence(f) && ContactAndOwnership(f) && Failures(f);
    SDL_free(f);
    if (ok)
        SDL_Log("NO2_SPECTRAL: PASS synthetic fifteen-weapon formation/contact/ownership checks");
    return ok ? 0 : 1;
}
