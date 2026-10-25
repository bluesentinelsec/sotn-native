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
static void Step(Fixture *f, size_t at, Sint16 x, Sint16 y, Sint16 angle, Sint16 frames)
{
    Put16(f->stage + at, x);
    Put16(f->stage + at + 2, y);
    Put16(f->stage + at + 4, angle);
    Put16(f->stage + at + 6, frames);
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 40);
    Put16(f->dra + 0x206, 18);
    Put16(f->dra + 0x20c, 3);
    Put32(f->dra + 0x224, 0x08000000);
    Put16(f->stage + 0xa94, 0x800c);
    Put16(f->stage + 0xa98, 0x54);
    Put16(f->stage + 0xa9a, 0x232);
    f->stage[0x211c] = 2;
    f->stage[0x211d] = 3;
    f->stage[0x211e] = 3;
    f->stage[0x211f] = 4;
    Put16(f->stage + 0x212e, 20);
    Put16(f->stage + 0x2132, 2);
    Put16(f->stage + 0x2134, 6);
    Put16(f->stage + 0x2136, -2);
    Put16(f->stage + 0x2138, -12);
    f->stage[0x213c] = 9;
    f->stage[0x213d] = 6;
    f->stage[0x213e] = 5;
    for (Uint16 i = 0; i < 5; ++i)
    {
        Uint8 *p = f->stage + 0x2144 + i * 12U;
        Put16(p, i * 17 + 3);
        Put16(p + 2, i * 9 + 7);
        Put16(p + 4, 21 + i);
        Put16(p + 6, 35 + i * 2);
        Put16(p + 8, -9 + i * 2);
        Put16(p + 10, -20 + i * 3);
    }
    Step(f, 0x2180, 4, 0, 256, 4);
    Step(f, 0x2188, 4, 0, 512, 4);
    Step(f, 0x2190, 0xfff, 0, 0, 0);
    Step(f, 0x2198, -3, -2, 128, 3);
    Step(f, 0x21a0, -3, 2, 64, 3);
    Step(f, 0x21a8, 0xfff, 0, 0, 0);
    Step(f, 0x21b0, -6, 4, 640, 3);
    Step(f, 0x21b8, 0, 0, 640, 2);
    Step(f, 0x21c0, 0x7fff, 0, 0, 0);
    Step(f, 0x21c8, -2, 1, 320, 2);
    Step(f, 0x21d0, 0x7fff, 0, 0, 0);
    const size_t patterns[] = {0x21d8, 0x2218, 0x2250};
    for (Uint16 j = 0; j < 3; ++j)
    {
        Uint16 count = j ? 6 : 7;
        for (Uint16 i = 0; i < count; ++i)
            Step(f, patterns[j] + i * 8U, (i & 1U) ? 2 : -2, (i & 1U) ? 1 : -1,
                 (Sint16)(800 + i * 64U), 2);
        Step(f, patterns[j] + count * 8U, 0x7fff, 0, 0, 0);
    }
    // Deliberately different private-pointer order and motion/profile values from the game.
    Put32(f->stage + 0x2288, 0x80182218);
    Put32(f->stage + 0x228c, 0x80182250);
    Put32(f->stage + 0x2290, 0x801821d8);
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    for (Uint32 i = 0; i <= 1024; ++i)
        Put16(f->exe + 0x1dbcc + i * 2U, (Sint32)i * 4);
    for (Uint32 i = 320; i < 1024; ++i)
        f->tiles[i * 2] = 1;
    f->types[1] = 3;
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 6, out));
    CHECK(SotnActors_SetServices(*out, (SotnDataView){f->exe, sizeof(f->exe)}, 0, false));
    CHECK(SotnActorLayout_OpenStage((SotnDataView){f->stage, sizeof(f->stage)}, 6, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *out, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 32}, 0,
        0));
    (*out)->player = (SotnPlayerState){.x = 300 * 65536, .y = 140 * 65536, .hp = 70, .hp_max = 70};
    (*out)->slots[16] =
        (Actor){.slot = 16,
                .dynamic = true,
                .state = {.id = 0x3d, .generation = 1, .x = 128 * 65536, .y = 140 * 65536}};
    return true;
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    f->layout = NULL;
}
static bool Ownership(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[16];
    CHECK(ActorChapel(s, a) && a->state.step == 1 && a->state.priority == 176 &&
          a->state.hitbox_width == 10 && a->state.hitbox_state == 2 &&
          a->state.primitive_count == 32 && a->state.frame == 3 && a->hunt.cycle_timer == 512);
    Uint32 old[3] = {0};
    for (Uint16 i = 1; i <= 3; ++i)
    {
        Actor *c = &s->slots[16 + i];
        old[i - 1] = c->state.generation;
        CHECK(c->state.id == 0x3e && c->state.parameters == i && Pixel(c->state.x) == 128 &&
              Pixel(c->state.y) == 140 + i * 12 && ActorChapel(s, c) &&
              c->state.hitbox_width == 2 && c->attack == 18);
    }
    s->player.status = 1;
    CHECK(ActorChapel(s, &s->slots[17]) && s->slots[17].attack == 36);
    s->player.status = 4;
    CHECK(ActorChapel(s, &s->slots[17]) && s->slots[17].attack == 36);
    s->player.status = 0;
    CHECK(ActorChapel(s, &s->slots[17]) && s->slots[17].attack == 18);
    ActorPrimitivesClear(s, a);
    *a = (Actor){.slot = 16,
                 .dynamic = true,
                 .state = {.id = 0x3d, .generation = 2, .x = 128 * 65536, .y = 140 * 65536}};
    CHECK(ActorChapel(s, a));
    for (Uint16 i = 1; i <= 3; ++i)
        CHECK(s->slots[16 + i].state.generation != old[i - 1] &&
              s->slots[16 + i].enemy_state.owner_generation == 2);
    a->state.generation = 3;
    CHECK(ActorChapel(s, &s->slots[17]) && s->slots[17].remove);
    Close(f, s);
    CHECK(Open(f, &s));
    a = &s->slots[16];
    s->slots[19] = (Actor){.state = {.id = 0x35, .generation = 2}};
    CHECK(!ActorChapel(s, a) && !s->slots[17].state.generation && !s->slots[18].state.generation &&
          s->slots[19].state.id == 0x35);
    Close(f, s);
    CHECK(Open(f, &s));
    a = &s->slots[16];
    s->effect_serial = 0x7ffffffeU;
    CHECK(!ActorChapel(s, a) && !s->slots[17].state.generation);
    Close(f, s);
    CHECK(Open(f, &s));
    a = &s->slots[16];
    for (Uint16 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 900;
    CHECK(ActorChapel(s, a) && a->remove && !s->slots[17].state.generation);
    Close(f, s);
    return true;
}
static bool Graphics(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[16];
    CHECK(ActorChapel(s, a));
    a->state.facing_left = true;
    bool done = false;
    CHECK(ActorHuntingSpirit(s, a, 0, 0, &done));
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    CHECK(p && p->state.offset_x[0] == 9 && p->state.offset_x[1] == -12 && p->state.u[0] == 3 &&
          p->state.v[3] == 42 && !p->state.active);
    CHECK(ActorHuntingSpirit(s, a, 0, 1, &done) && p->state.red == 0 && a->hunt.brightness == 16 &&
          a->hunt.sine_phase == 64);
    CHECK(ActorHuntingSpirit(s, a, 0, 1, &done) && p->state.red == 1 && p->state.page == 53 &&
          p->state.modulate);
    p->state.red = p->state.green = p->state.blue = 7;
    CHECK(ActorHuntingSpirit(s, a, 0, 2, &done) && !done && p->state.active && !p->state.red);
    CHECK(ActorHuntingSpirit(s, a, 0, 2, &done) && done && !p->state.active);
    for (Uint16 i = 0; i < 8; ++i)
    {
        CHECK(ActorHuntingDrawAttack(s, a, 0x2180, &done));
        CHECK(done == (i == 7));
    }
    CHECK(a->state.x == 136 * 65536 && a->state.rotation == 512 && !a->hunt.frames &&
          !a->hunt.attack_step && !a->hunt.trail);
    a->hunt.attacking = true;
    a->hunt.trail = 1;
    s->camera_x = 32;
    Sint32 tipx = 0, tipy = 0;
    CHECK(ActorHuntingTip(s, a, &tipx, &tipy));
    CHECK(ActorHuntingDrawAttack(s, a, 0x2218, &done));
    ActorPrimitive *line = ActorPrimitiveGet(s, a, 1);
    CHECK(line && line->phase == 8 && line->state.active && line->state.screen_space &&
          line->state.offset_x[1] == tipx - 32 && line->state.offset_y[1] == tipy &&
          line->state.line_red == 192);
    CHECK(ActorHuntingDrawAttack(s, a, 0x2218, &done) && line->phase == 2 && a->hunt.trail == 2);
    CHECK(ActorHuntingTrails(s, a) && line->state.red == 192 && line->state.line_red == 183 &&
          line->state.line_green == 58 && line->state.line_blue == 123);
    line->state.line_red = line->state.line_green = line->state.line_blue = 0;
    CHECK(ActorHuntingTrails(s, a) && !line->phase && !line->state.active);
    a->hunt.attacking = false;
    a->hunt.frames = 0;
    a->hunt.attack_step = 2;
    CHECK(!ActorHuntingDrawAttack(s, a, 0x2180, &done));
    CHECK(!ActorHuntingSpirit(s, a, 5, 0, &done));
    Close(f, s);
    return true;
}
static bool Phases(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[16];
    CHECK(ActorChapel(s, a) && ActorChapel(s, a) && a->state.step == 6 && a->hunt.next_step == 2 &&
          a->hunt.reference_y == 140 && a->state.facing_left);
    CHECK(ActorChapel(s, a) && a->substep == 1);
    CHECK(ActorChapel(s, a) && a->state.step == 2);
    for (Uint16 i = 0; i < 16; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(a->state.step == 2 && a->substep == 1 && a->hunt.brightness == 128);
    s->player.x = 196 * 65536;
    CHECK(ActorChapel(s, a) && a->substep == 2);
    for (Uint16 i = 0; i < 40 && a->state.step == 2; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(a->state.step == 6 && a->hunt.next_step == 3);
    SetStep(a, 3);
    a->hunt.attack_step = a->hunt.frames = 0;
    s->player.x = a->state.x + 40 * 65536;
    CHECK(ActorChapel(s, a) && a->state.step == 6 && a->hunt.next_step == 7 &&
          a->hunt.attack_timer == 63);
    SetStep(a, 4);
    a->hunt.attack_step = a->hunt.frames = 0;
    s->player.x = a->state.x + 40 * 65536;
    CHECK(ActorChapel(s, a) && a->state.step == 6 && a->hunt.next_step == 5);
    SetStep(a, 5);
    a->hunt.attack_step = a->hunt.frames = 0;
    a->hunt.attacking = false;
    Uint16 ticks = 0;
    for (; ticks < 100 && a->state.step == 5; ++ticks)
        CHECK(ActorChapel(s, a));
    CHECK(ticks < 100 && a->state.step == 6 && a->hunt.next_step == 3 && !a->hunt.attacking &&
          a->hunt.pattern == 1);
    Uint16 swords = 0;
    bool attack = false;
    for (Uint16 i = 0; i < s->combat.sound_count; ++i)
    {
        swords += (s->combat.sounds[i] == 0x640);
        attack |= s->combat.sounds[i] == 0x72a;
    }
    CHECK(swords == 6 && attack);
    // Original transition mirrors the angle only after vertical restoration.
    a->state.rotation = 512;
    a->state.facing_left = true;
    a->hunt.reference_y = Pixel(a->state.y);
    a->substep = 1;
    s->player.x = a->state.x - 40 * 65536;
    CHECK(ActorChapel(s, a) && !a->state.facing_left && a->state.rotation == -512 &&
          a->state.step == 3);
    Close(f, s);
    return true;
}
static bool HitDeath(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[16];
    CHECK(ActorChapel(s, a));
    a->hit_flags = 2;
    CHECK(ActorChapel(s, a) && a->state.step == 8 && a->substep == 1 && a->vx == -95232 &&
          a->vy == -188416 && s->combat.sounds[0] == 0x72b);
    a->hit_flags = 0;
    for (Uint16 i = 0; i < 100 && a->state.step == 8; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(a->state.step == 6 && a->hunt.next_step == 4);
    Close(f, s);
    CHECK(Open(f, &s));
    a = &s->slots[16];
    CHECK(ActorChapel(s, a));
    Uint16 primitive = a->primitive[0];
    a->state.flags |= 0x100U;
    CHECK(ActorChapel(s, a) && a->state.step == 9 && a->substep == 1 && !a->state.hitbox_state &&
          s->slots[17].remove && s->slots[18].remove && s->slots[19].remove &&
          s->combat.sounds[0] == 0x72c);
    for (Uint16 i = 0; i < 100 && a->substep != 3; ++i)
        CHECK(ActorChapel(s, a));
    CHECK(a->substep == 3 && (a->state.flags & 0x80000000U));
    CHECK(ActorChapel(s, a) && s->slots[160].state.id == 6);
    a->state.y = 500 * 65536;
    CHECK(SotnActors_Tick(s, s->player, 0, 0) && !a->state.generation &&
          !s->primitives[primitive].owner && !s->slots[17].state.generation);
    CHECK(!ActorChapelHunting(NULL, a) && !ActorChapelHunting(s, NULL));
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    bool ok = f && Ownership(f) && Graphics(f) && Phases(f) && HitDeath(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_CHAPEL_HUNTING: PASS");
    return 0;
}
