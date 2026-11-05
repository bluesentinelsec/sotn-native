#include "actors_internal.h"
#include "character_fixture.h"
#include <SDL3/SDL.h>
#include <sotn/bat_runtime.h>
#include <sotn/combat.h>
#include <sotn/teleport_effect.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void P32(Uint8 *p, Uint32 value)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(value >> (i * 8U));
}
static bool Lifecycle(Uint8 *dra)
{
    for (Uint16 mode = 0; mode < 6; ++mode)
    {
        SotnTeleportEffect n = {
            .parameter = (Uint16)(mode << 8), .x = 10 * 65536 + 123, .y = 20 * 65536 + 456};
        SotnTeleportShared points = {0};
        SotnTeleportEffectEvents e = {0};
        SotnTeleportEffectInput in = {
            .dra = {dra, 0xe294}, .player_x = 100 * 65536 + 800, .player_y = 140 * 65536 + 900};
        Uint32 rng = 17, expected = 17;
        const SotnTeleportEffect pending = n;
        CHECK(SotnTeleportEffect_Tick(&n, &points, in, &rng, &e) &&
              !SDL_memcmp(&n, &pending, sizeof(n)) && rng == 17 && !e.sound_count);
        CHECK(SotnTeleportEffect_Rebase(&n, 17, -3, false) && !SDL_memcmp(&n, &pending, sizeof(n)));
        in.allocated = true;
        CHECK(SotnTeleportEffect_Tick(&n, &points, in, &rng, &e));
        for (Uint16 i = 0; i < 96; ++i)
            SotnCombat_Random(&expected);
        CHECK(rng == expected && n.x == 100 * 65536 + 123 && n.y == 140 * 65536 + 456 &&
              n.quads[0].u[0] == 192 && n.quads[1].x[0] == 192 && n.quads[2].priority == 504 &&
              n.quads[35].palette == 0x1ba);
        if (!(mode & 1U))
        {
            CHECK(n.phase == 1 && n.width == 1 && n.height == 16 && e.sound_count == 2 &&
                  e.sounds[0] == 0x635 && e.sounds[1] == 0x8ba && n.quads[2].y[0] == 124 &&
                  n.quads[2].y[2] == 156);
            Uint16 ticks = 1;
            while (!e.warp_mode && ticks++ < 280)
                CHECK(SotnTeleportEffect_Tick(&n, &points, in, &rng, &e) && rng == expected);
            CHECK(ticks == 264 &&
                  e.warp_mode == (mode == 0   ? 6
                                  : mode == 2 ? 4
                                              : 5) &&
                  n.phase == 6 && !n.timer && !n.alpha && e.palette == 0x810d && !n.removed);
            CHECK(SotnTeleportEffect_Tick(&n, &points, in, &rng, &e) && !e.warp_mode &&
                  n.timer == -1);
        }
        else
        {
            CHECK(n.phase == 20 && n.width == 16 && n.height == 256 && n.alpha == 255 &&
                  e.sound_count == 1 && e.sounds[0] == 0x8bb && !(n.quads[0].draw_mode & 8U));
            Uint16 ticks = 1;
            while (!n.removed && ticks++ < 130)
            {
                const SotnTeleportEffect before = n;
                CHECK(SotnTeleportEffect_Tick(&n, &points, in, &rng, &e) && rng == expected &&
                      !e.warp_mode && !e.palette);
                if (n.removed)
                    CHECK(n.x == before.x && n.y == before.y &&
                          !SDL_memcmp(n.quads, before.quads, sizeof(n.quads)));
            }
            CHECK(ticks == 113 && n.phase == 24 && n.timer == 4 && e.warp_acknowledged &&
                  e.sound_count == 1 && e.sounds[0] == 0x636);
        }
    }
    return true;
}
static bool Boundaries(Uint8 *dra)
{
    SotnTeleportEffect n = {0};
    SotnTeleportShared shared = {0};
    SotnTeleportEffectInput in = {.dra = {dra, 0xe294},
                                  .player_x = 32760 * 65536 + 8,
                                  .player_y = -32760 * 65536 + 9,
                                  .allocated = true};
    SotnTeleportEffectEvents e = {0};
    Uint32 rng = 17;
    // A late malformed private color must not commit birth RNG/points/packets/events.
    P32(dra + 0xe270, 256);
    const SotnTeleportEffect before = n;
    const SotnTeleportShared points = shared;
    const SotnTeleportEffectEvents events = e;
    CHECK(!SotnTeleportEffect_Tick(&n, &shared, in, &rng, &e) && rng == 17 &&
          !SDL_memcmp(&n, &before, sizeof(n)) && !SDL_memcmp(&shared, &points, sizeof(shared)) &&
          !SDL_memcmp(&e, &events, sizeof(e)));
    P32(dra + 0xe270, 200);
    CHECK(SotnTeleportEffect_Tick(&n, &shared, in, &rng, &e));
    n.phase = 4;
    n.width = 4;
    n.quads[4].draw_mode = n.quads[5].draw_mode = 0;
    CHECK(SotnTeleportEffect_Tick(&n, &shared, in, &rng, &e) && (n.quads[4].draw_mode & 8U) &&
          !(n.quads[5].draw_mode & 8U));
    n.phase = 6;
    n.timer = SDL_MIN_SINT32;
    CHECK(SotnTeleportEffect_Tick(&n, &shared, in, &rng, &e) && n.timer == SDL_MAX_SINT32 &&
          !e.warp_mode);
    in.player_y = 1000 * 65536;
    n.height = 16;
    CHECK(SotnTeleportEffect_Tick(&n, &shared, in, &rng, &e) && n.quads[2].y[0] == 984 &&
          n.quads[2].y[2] == 240);
    const SotnTeleportShared held = shared;
    const Sint16 row = n.quads[2].x[0];
    const Sint32 x = n.x;
    CHECK(SotnTeleportEffect_Rebase(&n, 17, -3, false) && (Uint32)n.x == (Uint32)x + 17U * 65536U &&
          n.quads[2].x[0] == row && !SDL_memcmp(&shared, &held, sizeof(shared)));
    n.phase = 19;
    const SotnTeleportEffect invalid = n;
    CHECK(!SotnTeleportEffect_Rebase(&n, 17, -3, false) && !SDL_memcmp(&n, &invalid, sizeof(n)));
    return true;
}
static bool SharedRows(Uint8 *dra)
{
    TestCharacter_Fill(dra);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    SotnCharacter *character = NULL;
    CHECK(actors && r && SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    r->serial = 1;
    r->children[47] =
        (SotnBatRuntimeChild){.kind = 65,
                              .generation = 1,
                              .parameter = 0x100,
                              .source = {.x = 100 * 65536 + 123, .y = 140 * 65536 + 456}};
    r->children[47].state.teleport.parameter = 0x100;
    SotnBatRuntimeInput in = {.dra = {dra, 0x11000},
                              .player = {.x = 100 * 65536,
                                         .y = 140 * 65536,
                                         .hp = 70,
                                         .hp_max = 70,
                                         .mp = 20,
                                         .priority = 148,
                                         .animation = {.id = 1, .sprite = 1}}};
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        actors->primitives[i].owner = 99;
    SotnCharacterState before = {0}, after = {0};
    CHECK(SotnCharacter_Get(character, &before, NULL) &&
          SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && !r->children[47].initialized &&
          !r->children[47].primitive_count && !r->sound_count &&
          SotnCharacter_Get(character, &after, NULL) && after.random == before.random);
    for (Uint16 i = 1; i < SOTN_PRIMITIVE_SLOTS; ++i)
        actors->primitives[i].owner = 0;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          r->children[47].initialized && r->children[47].primitive_count == 36 &&
          r->children[47].state.teleport.phase == 20 && r->sound_count == 1 &&
          r->sounds[0] == 0x8bb && actors->primitives[0].owner == 99 &&
          SotnCharacter_Get(character, &after, NULL));
    Uint32 expected = before.random;
    for (Uint16 i = 0; i < 96; ++i)
        SotnCombat_Random(&expected);
    CHECK(after.random == expected);
    for (Uint16 i = 0; i < 36; ++i)
    {
        const Uint16 row = r->children[47].primitives[i];
        CHECK(row && actors->primitives[row].owner == 1 &&
              actors->primitives[row].ability_kind == 65);
    }
    in.freeze = 3;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          r->children[47].state.teleport.phase == 21 &&
          r->children[47].state.teleport.timer == 32 && !r->sound_count);
    const SotnTeleportEffect held = r->children[47].state.teleport;
    const SotnTeleportShared points = r->teleport;
    CHECK(SotnTeleportEffect_Rebase(&r->children[47].state.teleport, 17, -3, false) &&
          r->children[47].state.teleport.phase == held.phase &&
          r->children[47].state.teleport.timer == held.timer &&
          r->children[47].state.teleport.x == held.x + 17 * 65536 &&
          !SDL_memcmp(&r->teleport, &points, sizeof(points)));
    Uint16 updates = 0;
    while (r->children[47].kind && updates++ < 120)
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    CHECK(updates == 111 && r->teleport_acknowledged && r->sound_count == 1 &&
          r->sounds[0] == 0x636 && actors->primitives[0].owner == 99);
    for (Uint16 i = 1; i < SOTN_PRIMITIVE_SLOTS; ++i)
        CHECK(!actors->primitives[i].owner);
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    // A new real leased child is retired, without acknowledgement/RNG, on room entry.
    r->serial = 2;
    r->children[47] = (SotnBatRuntimeChild){.kind = 65, .generation = 2, .parameter = 0x100};
    r->children[47].state.teleport.parameter = 0x100;
    in.freeze = 0;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          r->children[47].primitive_count == 36);
    for (Uint16 i = 0; i < 96; ++i)
        SotnCombat_Random(&expected);
    const SotnTeleportShared retained = r->teleport;
    CHECK(SotnPlayerFactory_Begin(&r->factories[0].cursor, (SotnPlayerFactorySource){0}, 121, 0) &&
          SotnPlayerFactory_Begin(&r->factories[1].cursor, (SotnPlayerFactorySource){0}, 0, 0));
    const SotnPlayerFactory other = r->factories[1].cursor;
    in.entering = true;
    in.screen_shift_x = 17;
    in.screen_shift_y = -3;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          !r->children[47].kind && !r->teleport_acknowledged && !r->teleport_warp_mode &&
          !r->teleport_palette && !SDL_memcmp(&retained, &r->teleport, sizeof(retained)) &&
          !r->factories[0].cursor.entity_id &&
          !SDL_memcmp(&r->factories[1].cursor, &other, sizeof(other)) &&
          SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    for (Uint16 i = 1; i < SOTN_PRIMITIVE_SLOTS; ++i)
        CHECK(!actors->primitives[i].owner);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    return true;
}
int main(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    if (!dra)
        return 1;
    for (Uint16 i = 0; i < 9; ++i)
        P32(dra + 0xe270 + i * 4U, 200U - i * 17U);
    for (Uint16 i = 0; i < 48; ++i)
        dra[0xd094 + i] = (Uint8)(i + 12U);
    const bool ok = Lifecycle(dra) && Boundaries(dra) && SharedRows(dra);
    SDL_free(dra);
    return ok ? 0 : 1;
}
