#include "actors_internal.h"
#include "character_fixture.h"
#include <sotn/bat_runtime.h>
#include <sotn/combat.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static void Timeline(Uint8 *dra, Uint32 offset, Uint16 poses)
{
    for (Uint16 i = 0; i < poses; ++i)
    {
        Put16(dra + offset + i * 4U, 2);
        Put16(dra + offset + i * 4U + 2U, (Uint16)(31 + i));
    }
    Put16(dra + offset + poses * 4U, 0xffff);
}
static bool Controller(Uint8 *dra)
{
    const Uint16 params[] = {0, 2, 0x102, 0x201};
    for (Uint16 k = 0; k < 4; ++k)
    {
        SotnFireHit f = {0};
        Uint32 rng = 17, expected = rng;
        SotnFireHitInput in = {.source = {.x = 100 * 65536 + 123, .y = 130 * 65536 + 456},
                               .player_x = 128 * 65536 + 789,
                               .player_y = 140 * 65536 + 987,
                               .player_velocity_x = -1,
                               .hitbox_y = -8,
                               .priority = 148,
                               .parameter = params[k]};
        const Uint16 priority = (Uint16)(148 + (SotnCombat_Random(&expected) & 3U ? 2 : -2));
        const Uint32 x = SotnCombat_Random(&expected);
        const Sint16 px = (Sint16)(128 + (params[k] >> 8 == 2 ? (Sint32)(x % 44U) - 22
                                                              : (Sint32)(x & 15U) - 8)),
                     py = (Sint16)(132 + (Sint32)(SotnCombat_Random(&expected) & 31U) - 16);
        CHECK(SotnFireHit_Tick(&f, in, &rng) && rng == expected && f.priority == priority &&
              f.x == px * 65536 + 123 && f.y == py * 65536 + 456 && f.velocity_x == -1 &&
              f.velocity_y == -32768 &&
              f.scale_x == (k == 0   ? 0
                            : k == 2 ? 192
                                     : 288));
        CHECK(SotnFireHit_Animate(&f, (SotnDataView){dra, 0x11000}));
        const SotnFireHit born = f;
        CHECK(SotnFireHit_Rebase(&f, -17, 9) && f.x == born.x - 17 * 65536 &&
              f.y == born.y + 9 * 65536 && f.remaining == born.remaining);
        Uint16 emits = 0;
        for (Uint16 t = 1; t < 70 && !f.removed; ++t)
        {
            CHECK(SotnFireHit_Tick(&f, in, &rng) && rng == expected);
            emits += (Uint16)f.particle;
            if (!f.removed)
                CHECK(SotnFireHit_Animate(&f, (SotnDataView){dra, 0x11000}));
        }
        CHECK(f.removed && emits == (k == 1 || k == 2 ? 1 : 0));
    }
    return true;
}
static bool Shared(Uint8 *dra)
{
    TestCharacter_Fill(dra);
    SotnCharacter *character = NULL;
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r)), *saved = SDL_malloc(sizeof(*saved));
    SotnActors *a = SDL_calloc(1, sizeof(*a));
    CHECK(r && saved && a && SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    for (Uint16 k = 0; k < 3; ++k)
    {
        Uint8 *bp = dra + 0xd1d4 + (Uint32[]){4, 17, 18}[k] * 6U;
        bp[0] = k ? 16 : 5;
        bp[1] = k ? 3 : 1;
        bp[2] = 0xc1;
        bp[3] = k ? 3 : 0;
        bp[4] = k ? 0x70 : 0;
        bp[5] = k == 2 ? 4 : 0;
    }
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    TestCharacter_Put32(dra + 0xdb98 + 4 * 4, 0x800adaa8);
    Uint8 *profile = dra + 0xdaa8;
    profile[0] = 2;
    profile[1] = 192;
    profile[2] = 96;
    profile[4] = profile[5] = 1;
    Put16(profile + 6, 4);
    Put16(profile + 8, 0x33);
    Put16(profile + 10, 3);
    TestCharacter_Put32(profile + 12, 0x08800000);
    a->primitives[0].owner = 77;
    SotnBatRuntimeInput in = {.dra = {dra, 0x11000},
                              .player = {.x = 128 * 65536 + 123,
                                         .y = 140 * 65536 + 456,
                                         .velocity_x = -1,
                                         .hp = 70,
                                         .hp_max = 70,
                                         .mp = 20,
                                         .priority = 148,
                                         .step = SOTN_PLAYER_HIT,
                                         .status = 0x10000,
                                         .hurt = {.count = 2, .started = true}}};
    for (Uint16 k = 0; k < 2; ++k)
        in.player.hurt.factories[k].source =
            (SotnPlayerFactorySource){.x = in.player.x, .y = in.player.y, .priority = 148};
    in.player.hurt.factories[0].blueprint = 0x10011;
    in.player.hurt.factories[1].blueprint = 18;
    CHECK(SotnActors_PrimitiveSlots(a, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(a, r));
    CHECK(r->children[47].kind == 16 && r->children[47].parameter == 0x100 &&
          r->children[47].state.fire_hit.velocity_x == -1);
    in.player.hurt.count = 0;
    in.player.hurt.started = false;
    in.player.velocity_x = 123456;
    const SotnFireHit born = r->children[47].state.fire_hit;
    in.freeze = 3;
    for (Uint16 k = 0; k < 4; ++k)
        CHECK(SotnBatRuntime_Tick(r, character, in) &&
              !SDL_memcmp(&born, &r->children[47].state.fire_hit, sizeof(born)));
    in.entering = true;
    in.room_shift_x = in.screen_shift_x = 11;
    in.player.x += 11 * 65536;
    CHECK(SotnBatRuntime_Tick(r, character, in) &&
          r->children[47].state.fire_hit.x == born.x + 11 * 65536 &&
          r->children[47].state.fire_hit.remaining == born.remaining);
    in.entering = false;
    in.room_shift_x = in.screen_shift_x = 0;
    in.freeze = 0;
    bool particles = false, parent_velocity = false, rollback = false, frozen = false;
    for (Uint16 t = 0; t < 80; ++t)
    {
        CHECK(SotnActors_PrimitiveSlots(a, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(a, r));
        if (!rollback)
            for (Uint16 f = 0; f < 8; ++f)
                if (r->factories[f].cursor.entity_id && r->factories[f].fire_parent)
                {
                    const SotnFireHit parent =
                        r->children[r->factories[f].fire_parent_slot].state.fire_hit;
                    const Uint32 offset = parent.parameter >> 8 == 1 ? 0xdc10U : 0xdbd4U;
                    Uint8 *frame = dra + offset + parent.pose * 4U + 2U;
                    const Uint16 old = (Uint16)(frame[0] | (Uint16)frame[1] << 8);
                    SotnCharacterState before = {0}, after = {0};
                    CHECK(parent.pose > 0 && SotnCharacter_Get(character, &before, NULL));
                    Put16(frame, 0xffff);
                    *saved = *r;
                    CHECK(!SotnBatRuntime_Tick(r, character, in) &&
                          !SDL_memcmp(r, saved, sizeof(*r)) &&
                          SotnCharacter_Get(character, &after, NULL) &&
                          after.random == before.random);
                    Put16(frame, old);
                    // Its earlier descendant must fail real row reservation before RNG.
                    SDL_memset(&in.primitive_slots, 255, sizeof(in.primitive_slots));
                    CHECK(SotnBatRuntime_Tick(r, character, in) &&
                          SotnCharacter_Get(character, &after, NULL) &&
                          after.random == before.random);
                    for (Uint16 child = 0; child < 48; ++child)
                        CHECK(r->children[child].kind != 5);
                    *r = *saved;
                    CHECK(SotnActors_PrimitiveSlots(a, &in.primitive_slots));
                    rollback = true;
                    break;
                }
        if (particles && !frozen)
        {
            *saved = *r;
            in.freeze = 3;
            for (Uint16 k = 0; k < 3; ++k)
                CHECK(SotnBatRuntime_Tick(r, character, in) &&
                      !SDL_memcmp(r->children, saved->children, sizeof(r->children)) &&
                      !SDL_memcmp(r->factories, saved->factories, sizeof(r->factories)));
            in.freeze = 0;
            frozen = true;
        }
        for (Uint16 i = 0; i < 48; ++i)
            if (r->children[i].kind == 5)
            {
                SotnBatRuntimeChild *c = &r->children[i];
                CHECK(c->parameter == 0x400 && c->particle_parent && c->primitive_count == 3 &&
                      c->state.particles.phase == 1);
                particles = true;
                for (Uint16 q = 0; q < c->state.particles.count; ++q)
                    parent_velocity |= c->state.particles.tiles[q].velocity_x == -1;
            }
    }
    CHECK(particles && parent_velocity && rollback && frozen && !SotnBatRuntime_Occupied(r) &&
          a->primitives[0].owner == 77);
    for (Uint16 i = 1; i < SOTN_PRIMITIVE_SLOTS; ++i)
        CHECK(!a->primitives[i].owner);
    // Root18 must cancel when actual hurt ends, leaving its born sprite to retire.
    in.player.hurt.count = 1;
    in.player.hurt.factories[0].blueprint = 18;
    in.player.hurt.started = true;
    CHECK(SotnBatRuntime_Tick(r, character, in));
    in.player.hurt.count = 0;
    in.player.hurt.started = false;
    in.player.step = SOTN_PLAYER_STAND;
    in.player.status = 0;
    CHECK(SotnBatRuntime_Tick(r, character, in) && !SotnBatRuntime_Occupied(r));
    // Real child-slot exhaustion drops both noncritical roots before child RNG.
    SotnCharacterState before = {0}, after = {0};
    CHECK(SotnCharacter_Get(character, &before, NULL));
    in.player.step = SOTN_PLAYER_HIT;
    in.player.status = 0x10000;
    in.player.hurt.count = 2;
    in.player.hurt.started = true;
    in.player.hurt.factories[0].blueprint = 0x10011;
    in.player.hurt.factories[1].blueprint = 18;
    in.other_occupied = 0xffffffff00000000ULL;
    CHECK(SotnBatRuntime_Tick(r, character, in));
    in.player.hurt.count = 0;
    in.player.hurt.started = false;
    for (Uint16 k = 0; k < 5; ++k)
        CHECK(SotnBatRuntime_Tick(r, character, in));
    CHECK(!SotnBatRuntime_Occupied(r) && SotnCharacter_Get(character, &after, NULL) &&
          after.random == before.random);
    in.other_occupied = 0;
    // Malformed retained child ownership fails without changing the actual runtime.
    r->children[47] = (SotnBatRuntimeChild){.kind = 5, .parameter = 0x400, .generation = 123};
    *saved = *r;
    CHECK(!SotnBatRuntime_Tick(r, character, in) && !SDL_memcmp(r, saved, sizeof(*r)));
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(saved);
    SDL_free(a);
    return true;
}
int main(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    if (!dra)
        return 1;
    Timeline(dra, 0xd57c, 24);
    Timeline(dra, 0xdbd4, 14);
    Timeline(dra, 0xdc10, 12);
    bool ok = Controller(dra) && Shared(dra);
    SDL_free(dra);
    return ok ? 0 : 1;
}
