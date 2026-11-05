#include "actors_internal.h"
#include "character_fixture.h"
#include <sotn/bat_runtime.h>
#include <sotn/combat.h>
#include <sotn/holy_hit.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static bool Run(Uint8 *dra)
{
    SotnHolyHit a = {0}, b = {0};
    SotnHolyHitShared points = {0};
    Uint32 rng = 17, expected = 17;
    SotnHolyHitInput in = {.dra = {dra, 0xd094 + 48},
                           .source = {.x = -65536 + 123, .y = 100 * 65536 + 456},
                           .player_x = 100 * 65536 + 456,
                           .player_y = 120 * 65536 + 123,
                           .hitbox_x = 3,
                           .hitbox_y = -8,
                           .priority = 148};
    CHECK(SotnHolyHit_Tick(&a, &points, in, &rng) && a.removed && rng == 17);
    a = (SotnHolyHit){0};
    in.allocated = true;
    const SotnHolyHit fresh = a;
    in.dra = (SotnDataView){0};
    CHECK(!SotnHolyHit_Tick(&a, &points, in, &rng) && !SDL_memcmp(&a, &fresh, sizeof(a)) &&
          rng == 17);
    in.dra = (SotnDataView){dra, 0xd094 + 48};
    CHECK(SotnHolyHit_Tick(&a, &points, in, &rng) && !a.removed && a.phase == 1 &&
          a.x == in.source.x && a.y == in.source.y);
    for (Uint16 i = 0; i < 16; ++i)
    {
        const Sint16 x = (Sint16)(103 + SotnCombat_Random(&expected) % 24 - 12),
                     y = (Sint16)(112 + SotnCombat_Random(&expected) % 48 - 24);
        const Uint8 delay = (Uint8)((SotnCombat_Random(&expected) & 7U) + 1U);
        const Uint16 mode = (Uint16)(0x11bU | (SotnCombat_Random(&expected) & 1U ? 0x60U : 0));
        CHECK(points.points[i].x == x && points.points[i].y == y &&
              a.animation[i].delay == delay - 1 && a.animation[i].phase == (delay == 1) &&
              a.quads[i].draw_mode == mode && a.quads[i].priority == 152);
    }
    CHECK(rng == expected);
    in.player_x = 200 * 65536 + 456;
    CHECK(SotnHolyHit_Tick(&b, &points, in, &rng));
    for (Uint16 i = 0; i < 64; ++i)
        SotnCombat_Random(&expected);
    CHECK(rng == expected);
    const SotnHolyHitShared rewritten = points;
    const SotnHolyHit live = a;
    in.status = 0x10000;
    for (Uint16 tick = 0; tick < 30; ++tick)
    {
        CHECK(SotnHolyHit_Tick(&a, &points, in, &rng));
        if (!tick)
            for (Uint16 i = 0; i < 16; ++i)
                if (live.animation[i].phase == 1)
                    CHECK(a.quads[i].x[0] == rewritten.points[i].x - 6 &&
                          a.quads[i].y[0] == rewritten.points[i].y - 6 &&
                          !(a.quads[i].draw_mode & 8U));
        CHECK(SotnHolyHit_Tick(&b, &points, in, &rng));
    }
    CHECK(!a.removed && !b.removed && rng == expected);
    for (Uint16 i = 0; i < 16; ++i)
        CHECK(a.animation[i].phase == 2 && b.animation[i].phase == 2 &&
              points.points[i].x == rewritten.points[i].x &&
              points.points[i].y == rewritten.points[i].y - 26 && (a.quads[i].draw_mode & 8U) &&
              (b.quads[i].draw_mode & 8U));
    const SotnHolyHit before = a;
    CHECK(SotnHolyHit_Rebase(&a, -17, 9, false) && a.x == before.x - 17 * 65536 &&
          a.quads[0].x[0] == before.quads[0].x[0] - 17 &&
          !SDL_memcmp(a.animation, before.animation, sizeof(a.animation)));
    CHECK(SotnHolyHit_Rebase(&a, 23, -7, true) && a.x == before.x + 6 * 65536 &&
          a.quads[0].x[0] == before.quads[0].x[0] + 6 && rng == expected);
    in.status = 0;
    b = live;
    const SotnHolyHitShared retained = points;
    CHECK(SotnHolyHit_Tick(&b, &points, in, &rng) && b.removed && rng == expected &&
          !SDL_memcmp(&points, &retained, sizeof(points)));
    return true;
}
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static bool Shared(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *exe = SDL_calloc(1, 0x24000);
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r)), *saved = SDL_malloc(sizeof(*saved));
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnCharacter *character = NULL;
    CHECK(dra && exe && r && saved && actors);
    TestCharacter_Fill(dra);
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    for (Uint16 i = 0; i <= 1024; ++i)
        Put16(exe + 0x1dbcc + i * 2U, i * 4U);
    for (Uint16 i = 0; i < 48; ++i)
        dra[0xd094 + i] = (Uint8)(i + 1U);
    for (Uint16 i = 0; i < 2; ++i)
    {
        Uint8 *bp = dra + 0xd1d4 + (i ? 44U : 119U) * 6U;
        bp[0] = i ? 11 : 47;
        bp[1] = 1;
        bp[2] = 0x81;
    }
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    Uint8 *blink = dra + 0xd670 + 0x28U * 20U;
    Put16(blink + 6, 0x123);
    Put16(blink + 8, 256);
    Put16(blink + 10, 256);
    Put16(blink + 12, 256);
    Put16(blink + 14, 0x7007);
    Put16(blink + 16, 0x31);
    Put16(blink + 18, 512);
    actors->primitives[0].owner = 77;
    SotnCharacterState before = {0}, after = {0};
    CHECK(SotnCharacter_Get(character, &before, NULL));
    Uint32 expected = before.random;
    SotnBatRuntimeInput in = {.dra = {dra, 0x11000},
                              .executable = {exe, 0x24000},
                              .image = {12, 24, -6, -12},
                              .camera_x = 20,
                              .screen_shift_x = -3,
                              .player = {.x = 100 * 65536 + 123,
                                         .y = 140 * 65536 + 456,
                                         .hp = 70,
                                         .hp_max = 70,
                                         .mp = 20,
                                         .priority = 148,
                                         .step = SOTN_PLAYER_HIT,
                                         .status = 0x10000,
                                         .animation = {.hitbox_x = 3, .hitbox_y = -8, .sprite = 1},
                                         .hurt = {.count = 2, .started = true}}};
    for (Uint16 i = 0; i < 2; ++i)
        in.player.hurt.factories[i].source =
            (SotnPlayerFactorySource){.x = in.player.x, .y = in.player.y, .priority = 148};
    in.player.hurt.factories[0].blueprint = 119;
    in.player.hurt.factories[1].blueprint = 0x68002cU;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          r->children[47].kind == 47 && r->children[47].primitive_count == 16 &&
          r->children[46].kind == 11 && r->children[46].primitive_count == 8 &&
          r->children[47].state.holy.x == 80 * 65536 + 123);
    for (Uint16 i = 0; i < 16; ++i)
    {
        const Sint16 x = (Sint16)(86 + SotnCombat_Random(&expected) % 24 - 12);
        (void)SotnCombat_Random(&expected);
        (void)SotnCombat_Random(&expected);
        (void)SotnCombat_Random(&expected);
        CHECK(r->holy.points[i].x == x);
    }
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected &&
          after.hp == before.hp && after.mp == before.mp);
    in.player.hurt.count = 0;
    in.player.hurt.started = false;
    in.screen_shift_x = 0;
    in.player.x += 7 * 65536;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) &&
          r->children[47].state.holy.x == 80 * 65536 + 123 &&
          SotnActors_CommitBatPrimitives(actors, r));
    const SotnHolyHit before_camera = r->children[47].state.holy;
    const SotnHolyHitShared points_before_camera = r->holy;
    in.camera_x = 22;
    in.screen_shift_x = -2;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) &&
          r->children[47].state.holy.x == 78 * 65536 + 123 &&
          SotnActors_CommitBatPrimitives(actors, r));
    bool visible_camera = false;
    for (Uint16 i = 0; i < 16; ++i)
        if (before_camera.animation[i].phase == 1 && before_camera.animation[i].frame < 6)
        {
            const Sint16 half = before_camera.animation[i].frame >= 3 ? 4 : 6;
            CHECK(r->children[47].state.holy.quads[i].x[0] ==
                      points_before_camera.points[i].x - half - 2 &&
                  r->children[47].state.holy.quads[i].y[0] ==
                      points_before_camera.points[i].y - half);
            visible_camera = true;
        }
    CHECK(visible_camera);
    in.screen_shift_x = 0;
    const SotnHolyHit held = r->children[47].state.holy;
    const SotnHolyHitShared held_points = r->holy;
    CHECK(SotnBatRuntime_PlayerShove(r, 65536 + 23, -32768) &&
          r->children[47].state.holy.x == held.x + 65536 + 23 &&
          !SDL_memcmp(held.quads, r->children[47].state.holy.quads, sizeof(held.quads)) &&
          !SDL_memcmp(&held_points, &r->holy, sizeof(held_points)));
    in.player.hurt.count = 1;
    in.player.hurt.started = true;
    in.player.hurt.factories[0].source.x = in.player.x;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          r->children[45].kind == 47 && r->children[45].primitive_count == 16);
    for (Uint16 i = 0; i < 16; ++i)
    {
        const Sint16 x = (Sint16)(88 + SotnCombat_Random(&expected) % 24 - 12);
        (void)SotnCombat_Random(&expected);
        (void)SotnCombat_Random(&expected);
        (void)SotnCombat_Random(&expected);
        CHECK(r->holy.points[i].x == x);
    }
    in.player.hurt.count = 0;
    in.player.hurt.started = false;
    const Uint8 frame = r->children[47].state.holy.animation[0].frame;
    r->children[47].state.holy.animation[0].frame = 7;
    *saved = *r;
    CHECK(!SotnBatRuntime_Tick(r, character, in) && !SDL_memcmp(saved, r, sizeof(*r)) &&
          SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    r->children[47].state.holy.animation[0].frame = frame;
    in.freeze = 3;
    const SotnHolyHitShared frozen = r->holy;
    const SotnHolyHit frozen_child = r->children[47].state.holy;
    for (Uint16 i = 0; i < 5; ++i)
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
              !SDL_memcmp(&frozen, &r->holy, sizeof(frozen)) &&
              !SDL_memcmp(&frozen_child, &r->children[47].state.holy, sizeof(frozen_child)));
    in.entering = true;
    in.screen_shift_x = in.room_shift_x = 11;
    CHECK(SotnBatRuntime_Tick(r, character, in) &&
          r->children[47].state.holy.x == frozen_child.x + 11 * 65536 &&
          !SDL_memcmp(&frozen, &r->holy, sizeof(frozen)) &&
          !SDL_memcmp(frozen_child.animation, r->children[47].state.holy.animation,
                      sizeof(frozen_child.animation)));
    in.entering = false;
    in.screen_shift_x = in.room_shift_x = 0;
    in.freeze = 0;
    in.player.status = 0;
    in.player.step = SOTN_PLAYER_STAND;
    for (Uint16 i = 0; i < 80; ++i)
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    CHECK(!SotnBatRuntime_Occupied(r) && actors->primitives[0].owner == 77 &&
          !SDL_memcmp(&frozen, &r->holy, sizeof(frozen)));
    for (Uint16 i = 1; i < SOTN_PRIMITIVE_SLOTS; ++i)
        CHECK(!actors->primitives[i].owner);
    in.player.step = SOTN_PLAYER_HIT;
    in.player.status = 0x10000;
    in.player.hurt.count = 1;
    in.player.hurt.started = true;
    SDL_memset(&in.primitive_slots, 255, sizeof(in.primitive_slots));
    CHECK(SotnBatRuntime_Tick(r, character, in) && !SotnBatRuntime_Occupied(r) &&
          !SDL_memcmp(&frozen, &r->holy, sizeof(frozen)) &&
          SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots));
    in.player.hurt.factories[0].source.x = -33 * 65536;
    CHECK(SotnBatRuntime_Tick(r, character, in) && !SotnBatRuntime_Occupied(r) &&
          SotnActors_CommitBatPrimitives(actors, r));
    for (Uint16 i = 0; i < 64; ++i)
        SotnCombat_Random(&expected);
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected &&
          after.hp == before.hp && after.mp == before.mp && actors->primitives[0].owner == 77);
    SotnCharacter_Destroy(character);
    SDL_free(dra);
    SDL_free(exe);
    SDL_free(r);
    SDL_free(saved);
    SDL_free(actors);
    return true;
}
int main(void)
{
    Uint8 *dra = SDL_calloc(1, 0xd094 + 48);
    if (!dra)
        return 1;
    for (Uint16 i = 0; i < 48; ++i)
        dra[0xd094 + i] = (Uint8)(i + 1U);
    const bool ok = Run(dra) && Shared();
    SDL_free(dra);
    return ok ? 0 : 1;
}
