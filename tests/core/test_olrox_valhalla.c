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
    Uint8 stage[0x2600], dra[0x400], tiles[32 * 16 * 2], types[2];
    SotnActorLayout *layout;
} Fixture;
static void Put16(Uint8 *p, Sint32 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)((Uint32)v >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
static void Stream(Fixture *f, size_t at, Uint8 frame, bool loop)
{
    for (Uint16 i = 0; i < 3; ++i)
    {
        f->stage[at + i * 2U] = 1;
        f->stage[at + i * 2U + 1U] = (Uint8)(frame + i);
    }
    f->stage[at + 6] = loop ? 0 : 255;
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    for (Uint16 i = 0; i < 4; ++i)
    {
        Uint8 *d = f->dra + 0x200 + i * 40U;
        Put16(d + 4, i == 1 ? 100 : i == 2 ? 32766 : 32767);
        Put16(d + 6, i == 1 ? 16 : i == 2 ? 21 : 38);
        Put16(d + 8, 0x21);
        Put16(d + 12, i == 1 ? 0x23 : i == 2 ? 3 : 1);
        Put32(d + 36, i == 1 ? 0x08003410U : i == 0 ? 0x80002000U : 0x0d202000U);
    }
    const Uint16 inits[] = {0x8f8, 0x904, 0x910, 0x91c};
    for (Uint16 i = 0; i < 4; ++i)
    {
        Put16(f->stage + inits[i], i < 2 ? 0x8005 : 0x8000);
        Put16(f->stage + inits[i] + 4U, 76);
        Put16(f->stage + inits[i] + 6U, 0x207);
        Put16(f->stage + inits[i] + 8U, i == 0 ? 1 : i == 1 ? 0 : i);
    }
    // Original synthetic collision sensors; no private sensor arrays are copied.
    Put16(f->stage + 0x202a, 16);
    Put16(f->stage + 0x202e, 3);
    Put16(f->stage + 0x2030, 8);
    Put16(f->stage + 0x2032, -3);
    Put16(f->stage + 0x2034, -16);
    Put16(f->stage + 0x203a, 16);
    Put16(f->stage + 0x203c, 5);
    Stream(f, 0x2040, 1, true);
    Stream(f, 0x204c, 6, true);
    Stream(f, 0x2058, 11, false);
    Stream(f, 0x2064, 4, false);
    for (Uint16 i = 0; i < 3; ++i)
    {
        f->stage[0x2074 + i * 4U] = (Uint8)(-2 - i);
        f->stage[0x2075 + i * 4U] = (Uint8)(-4 - i);
        f->stage[0x2076 + i * 4U] = (Uint8)(11 + i);
        f->stage[0x2077 + i * 4U] = (Uint8)(12 + i);
        Put16(f->stage + 0x20f4 + i * 4U, i * 7);
        Put16(f->stage + 0x20f6 + i * 4U, -2 - i * 3);
    }
    for (Uint16 i = 0; i < 20; ++i)
    {
        f->stage[0x20b8 + i] = (Uint8)(i % 9);
        f->stage[0x20e0 + i] = (Uint8)(i % 5);
    }
    for (Uint16 i = 0; i < 9; ++i)
    {
        f->stage[0x2094 + i * 4U] = (Uint8)i;
        f->stage[0x2095 + i * 4U] = (Uint8)(-i);
        f->stage[0x2096 + i * 4U] = (Uint8)(i + 1U);
        f->stage[0x2097 + i * 4U] = 6;
        if (i < 5)
        {
            f->stage[0x20cc + i * 4U] = (Uint8)(i + 20U);
            f->stage[0x20ce + i * 4U] = (Uint8)(i + 7U);
            f->stage[0x20cf + i * 4U] = 4;
        }
    }
    for (Uint16 i = 0; i < 13; ++i)
    {
        Put32(f->stage + 0x2100 + i * 12U, (i + 1U) * 4096);
        Put32(f->stage + 0x2104 + i * 12U, (Uint32)(-2048 * (Sint32)i));
        Put16(f->stage + 0x2108 + i * 12U, -3 + i);
        Put16(f->stage + 0x210a + i * 12U, 0xbeef);
    }
    for (Uint16 i = 0; i < 6; ++i)
    {
        Put16(f->stage + 0x1990 + i * 2U, 64 + i);
        Put32(f->stage + 0x19a0 + i * 4U, 1024 + i * 1024U);
    }
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    for (Uint32 i = 32 * 12; i < 32 * 16; ++i)
        Put16(f->tiles + i * 2U, 1);
    f->types[1] = 3;
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 4, out));
    CHECK(SotnActorLayout_OpenStage((SotnDataView){f->stage, sizeof(f->stage)}, 4, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *out, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16}, 0,
        0));
    (*out)->slots[32] =
        (Actor){.slot = 32,
                .dynamic = true,
                .state = {.id = 0x34, .generation = 1, .x = 256 * 65536 + 123, .y = 176 * 65536}};
    (*out)->player = (SotnPlayerState){.x = 400 * 65536,
                                       .y = 176 * 65536,
                                       .hp = 70,
                                       .hp_max = 70,
                                       .animation = {.hitbox_width = 8, .hitbox_height = 16}};
    (*out)->service_ticks = 7;
    return true;
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
}
static bool Init(SotnActors *s)
{
    Actor *a = &s->slots[32];
    CHECK(ActorOlroxQuarter(s, a) && a->state.step == 2 && a->state.hit_points == 100 &&
          !a->state.facing_left && a->valhalla.anchor_x == 256 && a->state.hitbox_x == -2);
    for (Uint16 i = 0; i < 2; ++i)
    {
        Actor *c = &s->slots[33 + i];
        CHECK(c->state.id == 0x35 && c->state.parameters == i && c->state.x == 256 * 65536 &&
              c->enemy_state.owner_generation == 1 && ActorValhallaOwner(s, c) == a);
        CHECK(ActorOlroxQuarter(s, c) && c->state.step == 1 && c->state.x == a->state.x &&
              c->attack == (i ? 38 : 21) && c->state.hitbox_width == (i ? 8 : 2));
    }
    return true;
}
static bool Movement(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s) && Init(s));
    Actor *a = &s->slots[32];
    CHECK(ActorOlroxQuarter(s, a) && a->state.frame == 6 && a->vx == 163840 &&
          a->valhalla.timer == 128);
    for (Uint16 i = 0; i < 60 && a->state.step != 3; ++i)
        CHECK(ActorOlroxQuarter(s, a));
    CHECK(a->state.step == 3 && Pixel(a->state.x) > 352);
    s->player.x = a->state.x;
    s->player.velocity_y = -65536;
    for (Uint16 i = 0; i < 10 && a->state.step == 3; ++i)
        CHECK(ActorOlroxQuarter(s, a));
    CHECK(a->state.step == 5 && !a->substep && a->state.facing_left && a->state.frame == 1);
    s->player.x = a->state.x;
    for (Uint16 i = 0; i < 30 && a->substep != 2; ++i)
        CHECK(ActorOlroxQuarter(s, a));
    CHECK(a->state.step == 5 && a->substep == 2 && !a->valhalla.timer);
    CHECK(ActorOlroxQuarter(s, a) && a->substep == 3 && a->state.frame == 15 && a->vx == -229376 &&
          a->vy == -190464);
    bool apex = false, fall = false, land = false;
    for (Uint16 i = 0; i < 160 && a->state.step == 5; ++i)
    {
        CHECK(ActorOlroxQuarter(s, a));
        apex |= a->state.frame == 16;
        fall |= a->state.frame == 17;
        land |= a->substep == 5;
    }
    CHECK(apex && fall && land && a->state.step == 2);
    CHECK(s->slots[96].state.id == 0x15 && s->slots[96].state.parameters == 0);
    Close(f, s);
    return true;
}
static bool HitboxesCombat(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s) && Init(s));
    Actor *a = &s->slots[32], *c = &s->slots[33], *weapon = &s->slots[34];
    for (Uint16 i = 0; i < 20; ++i)
    {
        a->state.frame = i;
        CHECK(ActorOlroxQuarter(s, c) && ActorOlroxQuarter(s, weapon) &&
              c->state.hitbox_width == i % 9 + 1 && weapon->state.hitbox_width == i % 5 + 7);
    }
    a->state.frame = 1;
    c->state.hitbox_x = 80;
    c->state.hitbox_y = 0;
    c->state.hitbox_width = c->state.hitbox_height = 8;
    SotnPlayerState p = s->player;
    p.x = 336 * 65536;
    p.invulnerable = true;
    p.attack = (SotnPlayerAttack){.active = true,
                                  .power = 10,
                                  .element = 0x40,
                                  .hit_type = 2,
                                  .invincibility_frames = 8,
                                  .width = 3,
                                  .height = 3};
    CHECK(SotnActors_Tick(s, p, 128, 0) && a->state.hit_points < 100 &&
          c->state.hit_points == 32766 && a->cooldown[0] == 8 && c->cooldown[0] == 9);
    const Sint16 hp = a->state.hit_points;
    c->state.hitbox_x = 80;
    c->state.hitbox_y = 0;
    c->state.hitbox_width = c->state.hitbox_height = 8;
    CHECK(SotnActors_Tick(s, p, 128, 0) && a->state.hit_points == hp);
    // The weapon hurts the player independently and does not route damage to the body.
    p.attack.active = false;
    p.invulnerable = false;
    weapon->state.hitbox_x = 80;
    weapon->state.hitbox_y = 0;
    weapon->state.hitbox_width = weapon->state.hitbox_height = 8;
    CHECK(SotnActors_Tick(s, p, 128, 0) && s->combat.contact_damage == 38 &&
          a->state.hit_points == hp);
    ++a->state.generation;
    CHECK(SotnActors_Tick(s, p, 128, 0) && !c->state.generation && !weapon->state.generation &&
          !s->combat.contact_damage);
    Close(f, s);
    return true;
}
static bool LedgesAndThresholds(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s) && Init(s));
    Actor *a = &s->slots[32];
    for (Uint16 row = 12; row < 16; ++row)
        CHECK(ActorTile(s, row * 32U + 16U, 0));
    a->state.x = 250 * 65536;
    a->vx = 163840;
    CHECK(ActorOlroxQuarter(s, a) && a->state.step == 5 && a->substep == 2 &&
          a->state.x == 250 * 65536);
    // The later patrol-bound check overrides an edge-triggered leap in the original.
    SetStep(a, 2);
    a->valhalla.anchor_x = 100;
    a->vx = 163840;
    CHECK(ActorOlroxQuarter(s, a) && a->state.step == 3);
    a->vx = 0;
    a->pose = 3;
    a->remaining = 0;
    s->player.velocity_y = -65536;
    s->player.x = a->state.x + 128 * 65536;
    CHECK(ActorOlroxQuarter(s, a) && a->state.step == 2);
    SetStep(a, 3);
    a->pose = 3;
    s->player.x = a->state.x + 127 * 65536;
    CHECK(ActorOlroxQuarter(s, a) && a->state.step == 5);
    Close(f, s);
    return true;
}
static bool DeathPieces(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s) && Init(s));
    Actor *a = &s->slots[32];
    a->vx = 8192;
    a->vy = -4096;
    a->state.facing_left = true;
    a->state.flags |= 0x100;
    CHECK(ActorOlroxQuarter(s, a) && a->remove && s->combat.sounds[0] == 0x654);
    for (Uint16 i = 0; i < 3; ++i)
        CHECK(s->slots[160 + i].state.id == 2 && s->slots[160 + i].state.parameters == 3 &&
              s->slots[160 + i].state.x == (256 - i * 7) * 65536);
    for (Uint16 i = 0; i < 13; ++i)
    {
        Actor *part = &s->slots[163 + i];
        CHECK(part->state.id == 0x36 && part->state.parameters == i && ActorOlroxQuarter(s, part) &&
              part->state.frame == 18 + i && part->vx == 8192 - (Sint32)(i + 1U) * 4096 &&
              part->vy == -4096 - (Sint32)i * 2048 + 4096 && part->state.rotation == -3 + i &&
              part->state.rotated);
    }
    Actor *part = &s->slots[163];
    s->service_ticks = 16;
    s->random = 0;
    CHECK(ActorOlroxQuarter(s, part) && s->slots[176].state.id == 2 &&
          s->slots[176].state.parameters == 0 &&
          s->slots[176].state.priority == part->state.priority + 1);
    Close(f, s);
    CHECK(Open(f, &s) && Init(s));
    a = &s->slots[32];
    for (Uint16 i = 160; i < 192; ++i)
        s->slots[i] = (Actor){.state = {.id = 1, .generation = 9}};
    a->state.flags |= 0x100U;
    CHECK(ActorOlroxQuarter(s, a) && a->remove && s->slots[160].state.id == 1 &&
          s->slots[191].state.generation == 9);
    part = &s->slots[80];
    *part = (Actor){.state = {.id = 0x36, .parameters = 0}};
    s->service_ticks = 16;
    s->random = 123;
    CHECK(ActorOlroxQuarter(s, part) && s->random == 123);
    Close(f, s);
    return true;
}
static bool Failures(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[32], *c = &s->slots[33];
    *c = (Actor){.state = {.id = 0x28, .generation = 8}};
    CHECK(!ActorOlroxQuarter(s, a) && c->state.id == 0x28 && c->state.generation == 8);
    *c = (Actor){0};
    a->state.step = 0;
    CHECK(Init(s));
    const Uint32 old = c->state.generation;
    a->state.generation = 2;
    a->state.step = 0;
    CHECK(ActorOlroxQuarter(s, a) && c->state.generation != old &&
          c->enemy_state.owner_generation == 2);
    CHECK(ActorOlroxQuarter(s, c));
    a->state.frame = 20;
    CHECK(!ActorOlroxQuarter(s, c));
    a->state.frame = 1;
    f->stage[0x20b9] = 9;
    CHECK(!ActorOlroxQuarter(s, c));
    c->state.parameters = 2;
    CHECK(!ActorOlroxQuarter(s, c));
    c->state.parameters = 0;
    ++a->state.generation;
    CHECK(ActorOlroxQuarter(s, c) && c->remove);
    SetStep(a, 5);
    a->substep = 6;
    CHECK(!ActorOlroxQuarter(s, a));
    a->state.step = 4;
    CHECK(!ActorOlroxQuarter(s, a));
    a->state.step = 1;
    a->state.x = SDL_MAX_SINT32;
    a->vx = 1;
    CHECK(!ActorOlroxQuarter(s, a));
    Actor part = {.state = {.id = 0x36, .parameters = 13}};
    CHECK(!ActorOlroxQuarter(s, &part));
    part.state.parameters = 0;
    part.vx = SDL_MAX_SINT32;
    CHECK(!ActorOlroxQuarter(s, &part));
    CHECK(!ActorValhalla(NULL, a) && !ActorValhalla(s, NULL) && !ActorValhallaOwner(NULL, c));
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    const bool ok = f && Movement(f) && LedgesAndThresholds(f) && HitboxesCombat(f) &&
                    DeathPieces(f) && Failures(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_OLROX_VALHALLA: PASS");
    return 0;
}
