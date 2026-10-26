#include "giant_knights_internal.h"
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
    Uint8 stage[0x4000], dra[0x2400], exe[0x23220], tiles[32 * 16 * 2], types[2];
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
static bool Open(Fixture *f, SotnActors **out, bool blade)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    const Uint16 inits[] = {0x928, 0x934, 0x958, 0x964};
    for (Uint16 i = 0; i < 4; ++i)
    {
        Put16(f->stage + inits[i], 0x8006);
        Put16(f->stage + inits[i] + 2U, 2 + i);
        Put16(f->stage + inits[i] + 4U, 76);
        Put16(f->stage + inits[i] + 8U, i);
        Uint8 *enemy = f->dra + 0x200 + i * 40U;
        Put16(enemy + 4, i & 1U ? 32767 : 100);
        Put16(enemy + 6, 10 + i);
        Put16(enemy + 8, 0x21);
        Put16(enemy + 12, i & 1U ? 1 : 0x23);
        Put32(enemy + 36, 0x08003410U);
    }
    Put16(f->dra + 0x200 + 0xbeU * 40U + 6, 19);
    Put16(f->dra + 0x200 + 0xbeU * 40U + 8, 0x21);
    Put16(f->dra + 0x200 + 0xc0U * 40U + 6, 29);
    Put16(f->dra + 0x200 + 0xc0U * 40U + 8, 0x41);
    for (Uint16 kind = 0; kind < 2; ++kind)
    {
        const size_t at = kind ? 0x2c18 : 0x21ac;
        const Uint16 count = kind ? 15 : 14;
        // Authored construction: independent equal-length joints, unrelated to game poses.
        for (Uint16 i = 0; i < count; ++i)
        {
            const Uint16 index = i == 14 ? 18 : (Uint16)(i + 1);
            Put16(f->stage + at + i * 10U, index);
            Put16(f->stage + at + i * 10U + 4U, 8);
            Put16(f->stage + at + i * 10U + 6U, i + 1);
            Put16(f->stage + at + i * 10U + 8U, (Sint32)i - 7);
        }
        const size_t sensor = kind ? 0x2c08 : 0x219c;
        Put16(f->stage + sensor + 2U, 9);
    }
    const size_t lists[] = {0x2244, 0x2264, 0x2280, 0x22dc, 0x22f4, 0x230c, 0x2324, 0x2cb8, 0x2cdc,
                            0x2cfc, 0x2d1c, 0x2d38, 0x2d54, 0x2d6c, 0x2d84, 0x2d94, 0x2da4};
    for (Uint16 i = 0; i < SDL_arraysize(lists); ++i)
        for (Uint16 j = 0; j < 4; ++j)
            Put16(f->stage + lists[i] + j * 2U, j + 1);
    // Complete placement lists are authored in descending order, not copied game arrays.
    for (Uint16 i = 0; i < 14; ++i)
    {
        Put16(f->stage + 0x2244 + i * 2U, 14 - i);
        Put16(f->stage + 0x2cb8 + i * 2U, 14 - i);
    }
    Put16(f->stage + 0x2244 + 28, 15);
    Put16(f->stage + 0x2cb8 + 28, 18);
    Put16(f->stage + 0x2cb8 + 30, 15);
    Put16(f->stage + 0x2cb8 + 32, 16);
    const size_t poses[] = {0x2378, 0x23d8, 0x2410, 0x24c0, 0x24e8, 0x2520, 0x265c, 0x266c,
                            0x2700, 0x2e24, 0x2e5c, 0x2f24, 0x2f44, 0x2fe8, 0x3090, 0x30c0,
                            0x2dcc, 0x3158, 0x2f84, 0x31b4, 0x3248, 0x3270};
    for (Uint16 i = 0; i < SDL_arraysize(poses); ++i)
    {
        Put16(f->stage + poses[i], 1);
        Put32(f->stage + poses[i] + 4U, 0x80183800);
    }
    // SDK fixtures are authored; no original lookup values are embedded.
    for (Uint32 i = 0; i <= 1024; ++i)
    {
        Put16(f->exe + 0x1dbcc + i * 2U, (Sint32)i * 4);
        Put16(f->exe + 0x22a10 + i * 2U, (Sint32)i / 2);
    }
    for (Uint32 i = 0; i < 192; ++i)
        Put16(f->exe + 0x1e3e0 + i * 2U, 4096);
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    f->types[1] = 3;
    for (Uint32 i = 32 * 12; i < 32 * 16; ++i)
        Put16(f->tiles + i * 2U, 1);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 4, out));
    CHECK(SotnActorLayout_OpenStage((SotnDataView){f->stage, sizeof(f->stage)}, 4, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *out, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16}, 0,
        0));
    SotnActors *s = *out;
    s->executable = (SotnDataView){f->exe, sizeof(f->exe)};
    s->service_ticks = 7;
    s->player = (SotnPlayerState){.x = 400 * 65536,
                                  .y = 176 * 65536,
                                  .hp = 70,
                                  .hp_max = 70,
                                  .animation = {.hitbox_width = 8, .hitbox_height = 16}};
    s->slots[32] = (Actor){
        .slot = 32,
        .dynamic = true,
        .state = {
            .id = blade ? 0x3c : 0x37, .generation = 1, .x = 256 * 65536 + 123, .y = 183 * 65536}};
    return true;
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
}
static bool Init(SotnActors *s, bool blade)
{
    Actor *a = &s->slots[32];
    CHECK(ActorOlroxQuarter(s, a) && a->state.step == 2 && a->state.hit_points == 100);
    Actor next = *a;
    CHECK(ActorOlroxQuarter(s, &next));
    *a = next;
    CHECK(a->state.step == 5 && a->state.frame == (blade ? 0 : 3));
    for (Uint16 i = 1; i <= (blade ? 18 : 15); ++i)
    {
        if (blade && i == 17)
            continue;
        Actor *p = &s->slots[32 + i];
        CHECK(ActorGiantOwner(s, p) == a && p->giant.length == (i == 15 || i == 16 ? 12 : 8));
        CHECK(ActorOlroxQuarter(s, p) && p->state.step == 1 && p->state.rotated && !p->remove);
    }
    return true;
}
static bool ConstructionAndContact(Fixture *f, bool blade)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s, blade) && Init(s, blade));
    Actor *a = &s->slots[32], *p = &s->slots[33], *w = &s->slots[47];
    const Uint32 generation = p->state.generation;
    CHECK(!s->slots[49].state.generation && (!blade || s->slots[50].state.id == 0x38));
    a->stun = 3;
    a->state.flags |= 0x100U;
    ActorGiantHit(s, a, p, 0, 5);
    CHECK(a->cooldown[0] == 5 && p->cooldown[0] == 6 && p->stun == 3 && (p->state.flags & 0x100U) &&
          !(w->state.flags & 0x100U) && !w->cooldown[0]);
    a->state.flags &= ~0x100U;
    for (Uint16 i = 0; i < 20; ++i)
    {
        Actor *joint = &s->slots[32 + i];
        joint->state.flags &= ~0x100U;
        SDL_zero(joint->cooldown);
        joint->stun = 0;
        joint->state.x = 256 * 65536;
        joint->state.y = 176 * 65536;
        joint->state.hitbox_x = joint->state.hitbox_y = 0;
        joint->state.hitbox_width = joint->state.hitbox_height = 0;
    }
    p->state.hitbox_width = p->state.hitbox_height = 8;
    SotnPlayerState player = s->player;
    player.x = 320 * 65536;
    player.attack = (SotnPlayerAttack){.active = true,
                                       .offset_x = -64,
                                       .width = 8,
                                       .height = 8,
                                       .power = 20,
                                       .element = 0x20,
                                       .invincibility_frames = 8,
                                       .stun_frames = 4,
                                       .hit_type = 2};
    const Sint16 hp = a->state.hit_points;
    CHECK(SotnActors_Tick(s, player, 128, 0) && a->state.hit_points < hp &&
          p->state.hit_points == 100 && a->cooldown[0] && p->cooldown[0]);
    const Sint16 damaged = a->state.hit_points;
    CHECK(SotnActors_Tick(s, player, 128, 0) && a->state.hit_points == damaged);
    player.attack.active = false;
    player.invulnerable = false;
    for (Uint16 i = 0; i < 20; ++i)
        s->slots[32 + i].state.hitbox_width = s->slots[32 + i].state.hitbox_height = 0;
    w->state.x = player.x;
    w->state.y = player.y;
    w->state.hitbox_x = w->state.hitbox_y = 0;
    w->state.hitbox_width = w->state.hitbox_height = 8;
    CHECK(SotnActors_Tick(s, player, 128, 0) && s->combat.contact_damage == (blade ? 13 : 11) &&
          a->state.hit_points == damaged);
    // An unrelated replacement cannot receive a stale body's damage propagation.
    ++a->state.generation;
    CHECK(!ActorGiantOwner(s, p));
    ActorGiantHit(s, a, p, 1, 9);
    CHECK(!p->cooldown[1]);
    CHECK(ActorOlroxQuarter(s, p) && p->remove);
    // Re-entry replaces only matching owned survivors, assigning fresh generations.
    CHECK(GiantCreate(s, a, blade) && s->slots[33].state.generation != generation &&
          s->slots[33].enemy_state.owner_generation == a->state.generation);
    Close(f, s);
    return true;
}
static bool DeathAndWeapons(Fixture *f, bool blade)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s, blade) && Init(s, blade));
    Actor *a = &s->slots[32];
    a->state.flags |= 0x100U;
    s->slots[42].giant.terrain = 1;
    CHECK(ActorOlroxQuarter(s, a) && a->state.step == 24 && !a->state.hitbox_state);
    if (blade)
    {
        CHECK(a->substep == 2 && ActorOlroxQuarter(s, a) && a->substep == 3 &&
              s->slots[47].giant.released && s->slots[47].state.rotation == 0x400);
        for (Uint16 i = 0; i < 32; ++i)
            CHECK(ActorOlroxQuarter(s, a));
        CHECK(a->substep == 4 && ActorOlroxQuarter(s, a) && a->substep == 5 &&
              a->giant.timer == 63);
    }
    Actor *p = &s->slots[33], *w = &s->slots[47];
    CHECK(p->state.step == 24 && !p->state.hitbox_state && ActorOlroxQuarter(s, p) &&
          p->substep == 1 && p->giant.timer >= 32 && p->giant.timer <= 63);
    CHECK(ActorOlroxQuarter(s, w) && w->state.step == (blade ? 8 : 24));
    const Sint32 weapon_x = w->state.x;
    const Sint16 piece_timer = p->giant.timer;
    a->state.generation = 0;
    CHECK(ActorOlroxQuarter(s, p) && p->giant.timer == piece_timer - 1 && !p->remove);
    CHECK(ActorOlroxQuarter(s, w) && !w->remove && (!blade || w->state.x != weapon_x));
    for (Uint16 i = 0; i < 64 && p->state.id != 2; ++i)
        CHECK(ActorOlroxQuarter(s, p));
    CHECK(p->state.id == 2 && !p->state.step && !p->state.rotated && !p->state.parameters);
    Close(f, s);
    return true;
}
static bool FailuresAndPool(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s, false));
    Actor *a = &s->slots[32];
    s->slots[47] = (Actor){.state = {.id = 0x28, .generation = 8}};
    CHECK(!GiantCreate(s, a, false) && !s->slots[33].state.generation &&
          s->slots[47].state.generation == 8);
    s->slots[47] = (Actor){0};
    Put16(f->stage + 0x21ac + 10, 1);
    CHECK(!GiantCreate(s, a, false) && !s->slots[33].state.generation);
    Close(f, s);
    CHECK(Open(f, &s, true));
    a = &s->slots[32];
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 0xdeadbeef;
    CHECK(Init(s, true));
    Actor *w = &s->slots[47];
    CHECK(!w->state.primitive_count && w->state.hitbox_state && !w->remove);
    w->giant.released = 1;
    const Sint32 x = w->state.x;
    CHECK(ActorOlroxQuarter(s, w) && w->state.x == x + 8 * 65536 && !w->state.primitive_count);
    w->giant.released = 0;
    w->state.step = 9;
    CHECK(!ActorOlroxQuarter(s, w));
    // A thrown sword may be culled or replaced during the body's remaining death wait.
    a->state.step = 24;
    a->substep = 3;
    a->giant.released = 1;
    *w = (Actor){.slot = 47, .state = {.id = 0x28, .generation = 0x1234}};
    CHECK(GiantParts(s, a, 0x2cb8, true) && GiantDisable(s, a, 0x2cb8, true) &&
          w->state.id == 0x28 && w->state.generation == 0x1234 && !w->state.step);
    a->giant.released = 0;
    CHECK(!GiantParts(s, a, 0x2cb8, true));
    CHECK(!GiantCreate(NULL, a, true) && !GiantMove(NULL, 0) && !GiantDisable(NULL, a, 0, true));
    a->state.step = 4;
    CHECK(!ActorOlroxQuarter(s, a));
    a->state.step = 6;
    a->substep = 9;
    CHECK(!ActorOlroxQuarter(s, a));
    CHECK(!ActorGiant(NULL, a) && !ActorGiant(s, NULL) && !ActorGiantOwner(NULL, w));
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    const bool ok = f && ConstructionAndContact(f, false) && ConstructionAndContact(f, true) &&
                    DeathAndWeapons(f, false) && DeathAndWeapons(f, true) && FailuresAndPool(f);
    SDL_free(f);
    if (ok)
        SDL_Log("GIANT_KNIGHTS: PASS synthetic construction/ownership/contact/death/pool checks");
    return ok ? 0 : 1;
}
