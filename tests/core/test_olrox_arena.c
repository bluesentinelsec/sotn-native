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
    Uint8 stage[0x4000], dra[0x1000], tiles[32 * 32 * 2], types[256];
    SotnActorLayout *layout;
} Fixture;
static void Put16(Uint8 *p, Uint16 n)
{
    p[0] = (Uint8)n;
    p[1] = (Uint8)(n >> 8);
}
static void Put32(Uint8 *p, Uint32 n)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(n >> (i * 8));
}
static bool Open(Fixture *f, SotnActors **out, Uint16 side)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->stage + 0x738, 0x8002);
    Put16(f->stage + 0x73a, 14);
    Put16(f->stage + 0x740, 5);
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    // Public authored tile values; each table is distinct without embedding game assets.
    for (Uint16 i = 0; i < 48; ++i)
        Put16(f->stage + 0x1374 + i * 2U, (Uint16)(20 + i));
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 30, out));
    CHECK(SotnActorLayout_OpenStage((SotnDataView){f->stage, sizeof(f->stage)}, 30, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *out, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 32}, 256,
        0));
    SotnActors *s = *out;
    s->player = (SotnPlayerState){.x = 488 * 65536, .y = 180 * 65536};
    s->slots[16] = (Actor){.slot = 16,
                           .dynamic = true,
                           .state = {.id = 0x49,
                                     .parameters = side,
                                     .generation = 9,
                                     .x = -24 * 65536 + 123,
                                     .y = 128 * 65536 + 456}};
    return true;
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    f->layout = NULL;
}
static bool Tiles(const Fixture *f, const SotnActors *s, bool right, bool closed)
{
    const Uint32 start = right ? 0xdd : 0xc0;
    const size_t table = (right ? 0x13a4U : 0x1374U) + (closed ? 24U : 0U);
    for (Uint32 i = 0; i < 12; ++i)
        CHECK(Read16(s->mutable_tiles + (start + i / 3 * 32 + i % 3) * 2) ==
              Read16(f->stage + table + i * 2));
    return true;
}
static bool Cycle(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s, 1));
    const SotnStageProfile *p = SotnStageProfile_Get(30);
    CHECK(p && !p->connected && p->sprite_banks == 50 && p->actor_limit == 0x4d &&
          p->prize_count == 10 && p->prize_bit == 0 && p->door_types == 4 && p->camera_count == 7 &&
          !SotnStageProfile_SaveRoom(30, 0));
    CHECK(ActorStageOffset(s, 0xbc8) == 0x5f4 && ActorStageOffset(s, 0x1cc4) == 0x1bc8 &&
          ActorStageOffset(s, 0x13b0) == 0x1444 && ActorStageOffset(s, 0x1960) == 0x1864 &&
          ActorStageOffset(s, 0xe04) == s->nz0.size);
    Actor *a = &s->slots[16], *b = &s->slots[17];
    CHECK(ActorOlroxArena(s, a) && a->state.step == 1 && Pixel(a->state.x) == 536 &&
          ((Uint32)a->state.x & 65535) == 123 && a->state.animation_set == 0x8002 &&
          a->state.frame == 14 && a->state.priority == 0x69 && Tiles(f, s, true, false));
    CHECK(ActorOlroxArena(s, a) && a->state.step == 1 && !b->state.generation);
    s->player.x -= 1;
    CHECK(ActorOlroxArena(s, a) && a->state.step == 2 && b->state.id == 0x49 &&
          b->state.parameters == 0 && b->state.x == -24 * 65536 && b->state.y == 128 * 65536 &&
          b->enemy_state.owner_slot == a->slot &&
          b->enemy_state.owner_generation == a->state.generation);
    CHECK(ActorOlroxArena(s, b) && Tiles(f, s, false, false));
    CHECK(ActorOlroxArena(s, b) && b->state.step == 2);
    for (Uint16 i = 0; i < 99; ++i)
    {
        s->combat = (SotnActorCombat){0};
        ++s->ticks;
        CHECK(ActorOlroxArena(s, a) && ActorOlroxArena(s, b));
    }
    CHECK(a->state.step == 4 && b->state.step == 4 && Pixel(a->state.x) == 488 &&
          ((Uint32)a->state.x & 65535) == 32891 && b->state.x == 24 * 65536 &&
          Tiles(f, s, true, true) && Tiles(f, s, false, true));
    // The foundation must never synthesize defeat from an absent/unintegrated boss.
    for (Uint16 i = 0; i < 120; ++i)
        CHECK(ActorOlroxArena(s, a) && ActorOlroxArena(s, b));
    CHECK(!s->olrox_defeated && a->state.step == 4 && b->state.step == 4);
    s->olrox_defeated = true; // Explicit component fixture; no boss-defeat claim.
    CHECK(ActorOlroxArena(s, a) && ActorOlroxArena(s, b) && a->state.step == 5 &&
          b->state.step == 5);
    CHECK(ActorOlroxArena(s, a) && ActorOlroxArena(s, b) && a->state.step == 6 &&
          b->state.step == 6 && Tiles(f, s, true, false) && Tiles(f, s, false, false));
    for (Uint16 i = 0; i < 98; ++i)
    {
        if (!a->remove)
            CHECK(ActorOlroxArena(s, a));
        if (!b->remove)
            CHECK(ActorOlroxArena(s, b));
    }
    CHECK(a->remove && b->remove && Pixel(a->state.x) == 537 && Pixel(b->state.x) == -25);
    // Preserve the overlay-lifetime signal across an ordinary room reset.
    CHECK(SotnActors_Enter(s, f->layout,
                           (SotnCollisionMap){
                               {f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 32},
                           0, 0) &&
          s->olrox_defeated);
    a = &s->slots[16];
    *a = (Actor){.state = {.id = 0x49}};
    CHECK(ActorOlroxArena(s, a) && a->remove && !a->state.step);
    Close(f, s);
    return true;
}
static bool FailureAndCollision(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s, 2));
    Actor *a = &s->slots[16];
    CHECK(!ActorOlroxArena(s, a) && !a->state.step);
    a->state.parameters = 1;
    Uint8 before[sizeof(f->tiles)] = {0};
    SDL_memcpy(before, s->mutable_tiles, sizeof(before));
    Put16(f->stage + 0x13a4 + 22, 256);
    CHECK(!ActorOlroxArena(s, a) && SDL_memcmp(before, s->mutable_tiles, sizeof(before)) == 0);
    Close(f, s);
    CHECK(Open(f, &s, 1));
    a = &s->slots[16];
    CHECK(ActorOlroxArena(s, a));
    s->slots[17] = (Actor){.state = {.id = 3, .generation = 123}};
    const Actor previous = s->slots[17];
    s->player.x = 487 * 65536;
    CHECK(!ActorOlroxArena(s, a) && a->state.step == 1 &&
          SDL_memcmp(&previous, &s->slots[17], sizeof(previous)) == 0);
    s->slots[17] = (Actor){0};
    a->slot = SOTN_STAGE_ACTOR_SLOTS - 1;
    CHECK(!ActorOlroxArena(s, a));
    a->slot = 16;
    a->state.step = 7;
    CHECK(!ActorOlroxArena(s, a));
    a->state.step = 2;
    a->state.x = 450 * 65536;
    s->player =
        (SotnPlayerState){.x = 470 * 65536, .y = a->state.y, .solid_width = 8, .solid_height = 16};
    CHECK(ActorOlroxArena(s, a) && s->player.x == 482 * 65536 && s->motion.x == 12 * 65536 &&
          s->motion.contacts == 0x48 && a->state.x == 488 * 65536 + 32768);
    CHECK(!ActorOlroxArena(NULL, a) && !ActorOlroxArena(s, NULL));
    Close(f, s);
    // Validate the private initial global before allocating/exposing an arena owner.
    Put32(f->stage + 0x1190, 2);
    s = (SotnActors *)(uintptr_t)1;
    CHECK(!SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                  (SotnDataView){f->stage, sizeof(f->stage)}, 30, &s) &&
          !s);
    Put32(f->stage + 0x1190, 1);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 30, &s) &&
          s->olrox_defeated);
    SotnActors_Destroy(s);
    return true;
}
static bool RegistryAndArt(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s, 0));
    // Boss/attack/enemy identities must not alias the Laboratory's existing registry.
    const Uint16 unsupported[] = {1, 0x16, 0x20, 0x21, 0x4a, 0x44};
    for (Uint32 i = 0; i < SDL_arraysize(unsupported); ++i)
    {
        Actor a = {.state = {.id = unsupported[i], .generation = 17}};
        CHECK(ActorOlroxArena(s, &a) && a.state.unsupported && !a.state.step);
    }
    s->world->time_attack[SOTN_TIME_OLROX] = 120;
    Actor defeated_body = {.state = {.id = 0x2b, .generation = 17}},
          defeated_door = {.state = {.id = 0x49, .generation = 18}};
    CHECK(ActorOlroxArena(s, &defeated_body) && defeated_body.remove);
    CHECK(ActorOlroxArena(s, &defeated_door) && defeated_door.remove);
    s->world->time_attack[SOTN_TIME_OLROX] = 0;
    const size_t table[] = {0x748, 0x259c};
    for (Uint16 i = 0; i < 2; ++i)
    {
        Uint8 *record = f->stage + table[i] + (i ? 5 * 20 : 0);
        Put16(record, 0x8002);
        Put16(record + 2, 105);
        Put16(record + 4, 7);
        Put32(record + 16, 0x80183200);
        f->stage[0x3200] = 3;
        f->stage[0x3201] = 8;
        f->stage[0x3202] = 255;
        Actor a = {.state = {.id = i ? 8 : 0x11, .parameters = i ? 5 : 0}};
        CHECK(ActorOlroxArena(s, &a) && a.state.frame == 8 && a.state.priority == 105 &&
              a.state.texture_offset == 7 && a.state.rotated == (i != 0));
        a.state.parameters = i ? 6 : 1;
        CHECK(!ActorOlroxArena(s, &a));
    }
    s->stage_id = 4;
    Actor a = {.state = {.id = 0x49}};
    CHECK(!ActorOlroxArena(s, &a));
    Close(f, s);
    return true;
}
static bool Furnishings(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s, 0));
    for (Uint16 kind = 1; kind <= 4; ++kind)
    {
        Uint8 *p = f->stage + 0xd94 + (kind - 1U) * 12;
        Put16(p, 9 + kind);
        Put16(p + 2, 12);
        Put16(p + 4, (Uint16)-3);
        Put16(p + 6, 4);
        Put16(p + 8, 8);
        Put16(p + 10, 107 + kind);
        s->slots[16] = (Actor){.slot = 16,
                               .state = {.id = 0x2a,
                                         .parameters = kind,
                                         .generation = 19,
                                         .x = 128 * 65536,
                                         .y = 180 * 65536}};
        Actor *a = &s->slots[16];
        CHECK(ActorOlroxArena(s, a) && a->state.step == 1 && a->state.frame == kind &&
              a->state.animation_set == 0x8006 && a->state.texture_offset == 0x48 &&
              a->state.palette == 0x209 && a->state.hit_points == 8 && a->state.hitbox_x == -3 &&
              a->state.hitbox_y == 4 && a->state.hitbox_width == 9 + kind &&
              a->state.hitbox_height == 12 && a->state.priority == 107 + kind);
        CHECK(ActorOlroxArena(s, a) && a->state.step == 1);
        a->state.flags |= 0x100U;
        CHECK(ActorOlroxArena(s, a) && a->state.step == 1 + kind && !a->state.hitbox_state);
        if (kind == 2)
        {
            CHECK(!ActorOlroxArena(s, a) && a->state.step == 3);
            for (Uint16 i = 1; i <= 3; ++i)
                s->slots[16 + i] =
                    (Actor){.slot = (Uint16)(16 + i), .state = {.id = 0x2a, .generation = 20U + i}};
        }
        CHECK(ActorOlroxArena(s, a));
        if (kind == 1)
            CHECK(a->state.step == 11 && a->state.frame == 10 && a->vx == -65536);
        else
            CHECK(a->state.step == 32 && !a->state.frame);
        if (kind == 2)
            for (Uint16 i = 1; i <= 3; ++i)
                CHECK(s->slots[16 + i].state.flags & 0x100U);
    }
    for (Uint16 kind = 5; kind <= 13; ++kind)
    {
        Actor a = {.state = {.id = 0x2a,
                             .generation = 19,
                             .parameters = kind,
                             .x = 128 * 65536,
                             .y = 180 * 65536}};
        CHECK(ActorOlroxArena(s, &a) && a.state.step == kind + 1 && a.state.frame == kind);
        CHECK(ActorOlroxArena(s, &a));
    }
    Actor *chair = &s->slots[24];
    *chair = (Actor){
        .slot = 24,
        .state = {
            .id = 0x4b, .generation = 55, .parameters = 1, .x = 128 * 65536, .y = 180 * 65536}};
    s->player = (SotnPlayerState){.x = 126 * 65536, .y = 180 * 65536};
    CHECK(ActorOlroxArena(s, chair) && chair->state.step == 1);
    s->buttons = SOTN_BUTTON_UP | SOTN_BUTTON_RIGHT;
    CHECK(ActorOlroxArena(s, chair) && chair->state.step == 1);
    s->buttons = SOTN_BUTTON_UP;
    CHECK(ActorOlroxArena(s, chair) && chair->state.step == 2 && s->control.timer == 10 &&
          s->control.buttons == SOTN_BUTTON_UP && s->motion.chair_kind == 1);
    CHECK(ActorOlroxArena(s, chair) && s->motion.x == 65536);
    s->player.x = chair->state.x;
    CHECK(ActorOlroxArena(s, chair) && chair->state.step == 3);
    s->player.substep = 4;
    s->player.animation.remaining = -1;
    CHECK(ActorOlroxArena(s, chair) && chair->state.step == 4);
    s->player.substep = 0;
    CHECK(ActorOlroxArena(s, chair) && chair->state.step == 1);
    s->slots[23].state.flags = 0x100;
    CHECK(ActorOlroxArena(s, chair) && chair->remove);
    Actor invalid = {.state = {.id = 0x2a, .generation = 2, .parameters = 0}};
    CHECK(!ActorOlroxArena(s, &invalid));
    invalid.state.parameters = 14;
    CHECK(!ActorOlroxArena(s, &invalid));
    // Preserve rotating debris into its original explosion identity on floor contact.
    f->types[0] = 3;
    Actor debris = {.state = {.id = 0x2a,
                              .parameters = 8,
                              .generation = 3,
                              .step = 9,
                              .rotation = 500,
                              .x = 128 * 65536,
                              .y = 180 * 65536}};
    CHECK(ActorOlroxArena(s, &debris) && debris.state.id == 2 && !debris.state.step &&
          debris.state.rotated && debris.state.rotation == 484 && !debris.state.parameters);
    debris.state.id = 0x2a;
    debris.state.parameters = 8;
    debris.state.step = 7;
    CHECK(ActorOlroxArena(s, &debris) && debris.state.step == 32);
    debris.state.step = 7;
    debris.vx = SDL_MAX_SINT32;
    const Sint32 previous_x = debris.state.x;
    CHECK(!ActorOlroxArena(s, &debris) && debris.state.x == previous_x);
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Cycle(f) && FailureAndCollision(f) && RegistryAndArt(f) && Furnishings(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("OLROX_ARENA_FOUNDATION: PASS");
    return 0;
}
