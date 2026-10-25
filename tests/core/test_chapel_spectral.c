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
    Uint8 dra[0x1000], stage[0x2400], exe[0x23220], tiles[2048], types[2];
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
    const size_t init[] = {0xaa0, 0xab8, 0xaac};
    for (Uint16 i = 0; i < 3; ++i)
    {
        Put16(f->stage + init[i], 0x800d);
        Put16(f->stage + init[i] + 4, 0x51);
        Put16(f->stage + init[i] + 6, 0x250 + i);
        Put16(f->stage + init[i] + 8, i);
    }
    for (Uint16 i = 2; i < 9; ++i)
    {
        f->stage[0x2294 + i * 2U] = (Uint8)(5 + i);
        f->stage[0x2295 + i * 2U] = (Uint8)(2 + i);
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
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 6, out));
    CHECK(SotnActors_SetServices(*out, (SotnDataView){f->exe, sizeof(f->exe)}, 1, false));
    CHECK(SotnActorLayout_OpenStage((SotnDataView){f->stage, sizeof(f->stage)}, 6, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *out, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 32}, 0,
        0));
    (*out)->player = (SotnPlayerState){.x = 188 * 65536, .y = 128 * 65536, .hp = 70, .hp_max = 70};
    (*out)->slots[16] = (Actor){
        .slot = 16,
        .dynamic = true,
        .state = {.id = 0x3f, .generation = 1, .x = 128 * 65536 + 123, .y = 128 * 65536 + 456}};
    return true;
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    f->layout = NULL;
}
static bool Init(SotnActors *s)
{
    if (!ActorChapel(s, &s->slots[16]))
        return false;
    for (Uint16 i = 17; i <= 32; ++i)
        if (!ActorChapel(s, &s->slots[i]))
            return false;
    return true;
}
static bool TickFamily(SotnActors *s)
{
    if (!ActorChapel(s, &s->slots[16]))
        return false;
    for (Uint16 i = 17; i <= 32; ++i)
        if (!ActorChapel(s, &s->slots[i]))
            return false;
    return true;
}
static bool Initialization(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s) && Init(s));
    Actor *a = &s->slots[16];
    CHECK(a->state.step == 1 && a->state.frame == 1 && a->state.blend_mode == 0x30 &&
          a->state.hitbox_y == 4 && a->spectral.count == 16 && a->spectral.radius == 48 &&
          !a->spectral.has_weapons);
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    CHECK(p && p->state.screen_space && !p->state.active && p->state.width == 2 &&
          p->state.priority == 192);
    for (Uint16 i = 1; i <= 16; ++i)
    {
        const Actor *c = &s->slots[16 + i];
        CHECK(c->state.id == 0x41 && c->state.frame == (i > 12 ? 2 : 3) && c->state.step == 1 &&
              !c->state.hitbox_state && c->state.parameters == (i > 12 ? 1 : 0) &&
              Pixel(c->state.x) == 128 && ((Uint32)c->state.x & 65535U) == 0 &&
              c->enemy_state.owner_generation == 1);
    }
    CHECK(ActorChapel(s, a) && a->state.step == 2);
    CHECK(ActorChapel(s, a) && a->spectral.has_weapons);
    Actor *w = &s->slots[17];
    CHECK(w->spectral.target_x == 160 * 65536 + 123 && w->spectral.target_y == 128 * 65536 + 456 &&
          w->state.rotation == -1024);
    CHECK(s->slots[18].spectral.target_x == a->state.x + 32 * 2732 * 16 &&
          s->slots[18].spectral.target_y == a->state.y + 32 * 1364 * 16 &&
          s->slots[18].state.rotation == -683);
    CHECK(s->slots[29].spectral.target_x == 176 * 65536 + 123 &&
          s->slots[29].state.rotation == -1024);
    w->state.step = 4;
    w->state.x = 0x123;
    w->state.y = 0x234;
    CHECK(ActorChapel(s, w) && w->state.x == 160 * 65536 + 0x123 &&
          w->state.y == 128 * 65536 + 0x234 && w->state.hitbox_x == 8 && w->state.hitbox_y == 0 &&
          w->state.hitbox_width == 5);
    for (Uint16 i = 0; i < 160 && a->state.step == 2; ++i)
        CHECK(TickFamily(s));
    CHECK(a->state.step == 3);
    for (Uint16 i = 17; i <= 32; ++i)
        CHECK(s->slots[i].state.hitbox_state == 3);
    for (Uint16 i = 0; i < 16; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(a->state.step == 4);
    Close(f, s);
    return true;
}
static bool MotionAttack(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s) && Init(s));
    Actor *a = &s->slots[16];
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    CHECK(p);
    a->state.x = 128 * 65536;
    a->state.y = 128 * 65536;
    SetStep(a, 4);
    a->substep = 3;
    a->spectral.timer = 128;
    a->spectral.moves = 2;
    a->vx = 65536;
    a->vy = 0;
    p->state.x = 149 * 65536;
    p->state.y = 128 * 65536;
    CHECK(ActorChapel(s, a) && a->state.x == 129 * 65536 && a->vx == 36864 &&
          a->spectral.spin == 4 && a->spectral.timer == 127);
    a->state.x = 128 * 65536;
    a->vx = 65536;
    s->camera_x = 32;
    a->spectral.spin = 0;
    CHECK(ActorChapel(s, a) &&
          a->vx ==
              77824); // Screen target follows the camera; biased lookup differs from host sqrt.
    s->camera_x = 0;
    a->vx = a->vy = 0;
    p->state.x = a->state.x;
    p->state.y = a->state.y;
    a->spectral.moves = 1;
    CHECK(ActorChapel(s, a) && a->state.step == 5);
    a->spectral.spin = 126;
    CHECK(ActorChapel(s, a) && a->substep == 1 && a->spectral.spin == 128);
    CHECK(ActorChapel(s, a) && a->substep == 2); // Exact-step arrival completes one call later.
    Uint16 ticks = 0;
    while (a->substep == 2 && ticks++ < 100)
        CHECK(ActorChapel(s, a));
    CHECK(a->substep == 3 && a->spectral.timer == 32);
    for (Uint16 i = 0; i < 32; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(a->substep == 4);
    for (Uint16 i = 0; i < 15; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(a->substep == 5 && a->spectral.radius == 168);
    for (Uint16 i = 0; i < 120; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(a->state.step == 5 && a->spectral.radius == 48);
    CHECK(ActorChapel(s, a) && a->state.step == 4);
    bool twirl = false, scrape = false;
    for (Uint16 i = 0; i < s->combat.sound_count; ++i)
    {
        twirl |= s->combat.sounds[i] == 0x6d4;
        scrape |= s->combat.sounds[i] == 0x689;
    }
    CHECK(twirl && scrape);
    SetStep(a, 3);
    a->substep = 1;
    a->spectral.timer = 10;
    a->spectral.count = 15;
    a->spectral.has_weapons = true;
    a->spectral.spin = 0;
    a->state.rotation = 0;
    CHECK(ActorChapel(s, a) && a->state.rotation == -2);
    SetStep(a, 4);
    a->spectral.count = 0;
    CHECK(ActorChapel(s, a) && !a->spectral.has_weapons && a->substep == 2 &&
          a->spectral.timer == 128);
    Close(f, s);
    return true;
}
static bool Lifecycle(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s) && Init(s));
    Actor *a = &s->slots[16], *w = &s->slots[17];
    w->state.flags |= 0x100U;
    w->vx = 65536;
    w->vy = -32768;
    Sint32 x = w->state.x, y = w->state.y;
    CHECK(ActorChapel(s, w) && w->state.step == 16 && w->substep == 1 && !w->state.hitbox_state &&
          w->state.x == x + 65536 && w->state.y == y - 32768 && w->vy == -24576 &&
          a->spectral.count == 15);
    CHECK(ActorChapel(s, w) && a->spectral.count == 15);
    s->camera_y = 100;
    w->state.y = 421 * 65536;
    w->vy = 0;
    CHECK(ActorChapel(s, w) && w->substep == 2 && !w->state.frame && !w->remove);
    CHECK(ActorChapel(s, w) && !w->state.frame && a->spectral.count == 15);
    a->state.flags |= 0x100U;
    a->vx = 65536;
    a->vy = 0;
    a->state.y = 453 * 65536;
    CHECK(ActorChapel(s, a) && a->state.step == 8 && a->substep == 1 && a->vx == 65536 &&
          !a->state.hitbox_state);
    for (Uint16 i = 18; i <= 32; ++i)
    {
        s->slots[i].state.y = 421 * 65536;
        s->slots[i].vy = 0;
        CHECK(ActorChapel(s, &s->slots[i]) && s->slots[i].substep == 2 && !s->slots[i].state.frame);
    }
    CHECK(!a->spectral.count);
    x = a->state.x;
    CHECK(ActorChapel(s, a) && a->state.x == x + 65536 && a->vy == 6144 && a->substep == 2);
    Uint16 primitive = a->primitive[0];
    CHECK(ActorChapel(s, a) && a->remove);
    for (Uint16 i = 17; i <= 32; ++i)
        CHECK(s->slots[i].remove);
    CHECK(SotnActors_Tick(s, s->player, 0, 100) && !s->slots[16].state.generation &&
          !s->primitives[primitive].owner);
    for (Uint16 i = 17; i <= 32; ++i)
        CHECK(!s->slots[i].state.generation);
    Close(f, s);
    return true;
}
static bool AuraOwnership(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s) && Init(s));
    Actor *a = &s->slots[16];
    s->service_ticks = 16;
    CHECK(ActorChapel(s, a) && s->slots[160].state.id == 0x40);
    Actor *glow = &s->slots[160];
    CHECK(ActorChapel(s, glow) && glow->state.x == a->state.x && glow->state.y == a->state.y &&
          glow->state.scale_set && glow->state.opacity_set && glow->state.scale_x == 262 &&
          glow->state.opacity == 124 && glow->state.palette == 0x25f);
    for (Uint16 i = 0; i < 23; ++i)
        CHECK(ActorChapel(s, glow));
    CHECK(glow->state.opacity == 32 && !glow->remove);
    CHECK(ActorChapel(s, glow) && glow->remove && glow->state.opacity == 28);
    Uint32 old = s->slots[17].state.generation;
    ActorPrimitivesClear(s, a);
    *a = (Actor){.slot = 16,
                 .dynamic = true,
                 .state = {.id = 0x3f, .generation = 2, .x = 128 * 65536, .y = 128 * 65536}};
    s->service_ticks = 1;
    CHECK(Init(s) && s->slots[17].state.generation != old &&
          s->slots[17].enemy_state.owner_generation == 2);
    a->state.generation = 3;
    CHECK(ActorChapel(s, &s->slots[17]) && s->slots[17].remove);
    CHECK(ActorChapel(s, glow) && glow->remove);
    Close(f, s);
    CHECK(Open(f, &s));
    a = &s->slots[16];
    s->slots[32] = (Actor){.state = {.id = 0x35, .generation = 9}};
    CHECK(!ActorChapel(s, a) && !s->slots[17].state.generation && s->slots[32].state.id == 0x35);
    Close(f, s);
    CHECK(Open(f, &s));
    s->effect_serial = 0x7ffffff8U;
    CHECK(!ActorChapel(s, &s->slots[16]) && !s->slots[17].state.generation);
    Close(f, s);
    CHECK(Open(f, &s));
    for (Uint16 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 900;
    CHECK(ActorChapel(s, &s->slots[16]) && s->slots[16].remove);
    Close(f, s);
    CHECK(Open(f, &s) && Init(s));
    a = &s->slots[17];
    a->state.frame = 10;
    CHECK(!ActorChapel(s, a));
    a->state.frame = 3;
    a->state.flags |= 0x100U;
    s->slots[16].spectral.count = 0;
    CHECK(!ActorChapel(s, a));
    CHECK(!ActorChapelSpectral(NULL, a) && !ActorChapelSpectral(s, NULL));
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    bool ok = f && Initialization(f) && MotionAttack(f) && Lifecycle(f) && AuraOwnership(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_CHAPEL_SPECTRAL: PASS");
    return 0;
}
