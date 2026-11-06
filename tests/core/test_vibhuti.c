// EntitySubwpnThrownVibhuti (dra/84B88.c) and its shared-runtime owner with authored tables only.
#include "actors_internal.h"
#include "character_fixture.h"
#include <SDL3/SDL.h>
#include <sotn/combat.h>
#include <sotn/game_math.h>
#include <sotn/subweapon.h>
#include <sotn/subweapon_entity.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static Sint32 Floor(Sint64 v, Uint32 n)
{
    return (Sint32)(v >= 0 ? v >> n : -((-v + ((Sint64)1 << n) - 1) >> n));
}
static Sint32 Whole(Sint32 v)
{
    return Floor(v, 16);
}
// Floor: rows10..15 solid; a wall column at x 128..143 (tile8).
static Uint8 tiles[32 * 16 * 2], types[2] = {0, 3};
static SotnCollisionMap Map(void)
{
    SDL_zeroa(tiles);
    for (Uint16 y = 0; y < 16; ++y)
        for (Uint16 x = 0; x < 32; ++x)
            if (y >= 10 || x == 8)
                Put16(tiles + ((size_t)y * 32 + x) * 2U, 1);
    return (SotnCollisionMap){{tiles, sizeof(tiles)}, {types, sizeof(types)}, 32, 16};
}
static bool Component(SotnDataView executable)
{
    SotnSubweaponInput in = {.executable = executable, .collision = Map(), .player_priority = 148};
    SotnSubweaponEntity e = {
        .x = 100 * 65536 + 0x1234, .y = 150 * 65536 + 0x5678, .id = SOTN_SUBWEAPON_VIBHUTI};
    SotnSubweaponEvents events = {0};
    Uint32 random = 77;
    // A failed func_800EDB58 reservation destroys the entity before any RNG.
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &events) && e.removed && random == 77 &&
          !events.sound && !events.use_ticks);
    CHECK(!SotnSubweaponEntity_Tick(&e, in, &random, &events));
    for (Uint16 facing = 0; facing < 2; ++facing)
    {
        e = (SotnSubweaponEntity){.x = 100 * 65536 + 0x1234,
                                  .y = 150 * 65536 + 0x5678,
                                  .id = SOTN_SUBWEAPON_VIBHUTI,
                                  .facing_left = facing != 0};
        in.allocated = true;
        random = 77;
        Uint32 expected = 77;
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &events) && e.step == 1 &&
              e.timer == 0x80 && e.priority == 150 && e.flags == 0x08800000U &&
              e.hitbox_width == 4 && e.hitbox_height == 4 && events.sound == 0x60c &&
              events.use_ticks == 4);
        for (Uint16 i = 0; i < SOTN_VIBHUTI_PARTICLES; ++i)
        {
            const Sint32 angle = (SotnCombat_Random(&expected) & 0xff) + 0x100;
            const Sint32 speed = (SotnCombat_Random(&expected) & 0xff) + 0x80;
            SotnMathTrig t = {0};
            CHECK(SotnMath_Trig(executable, angle, &t));
            const Sint32 vx =
                Floor(((Sint64)Floor((Sint64)t.cosine * 16 * speed, 9) + 0x8000) * 3, 1);
            const SotnVibhutiParticle *p = &e.particles[i];
            CHECK(p->x == 100 * 65536 && p->y == 138 * 65536 && p->falling && p->width == 2 &&
                  p->height == 2 && p->draw_mode == 2 && p->velocity_x == (facing ? -vx : vx) &&
                  p->velocity_y == -Floor((Sint64)t.sine * 16 * speed, 9));
        }
        CHECK(random == expected);
    }
    // Right-facing ash: the hitbox cycles over particles0..7 with the timer.
    e.facing_left = false;
    for (Uint16 i = 0; i < SOTN_VIBHUTI_PARTICLES; ++i)
        e.particles[i].velocity_x = SDL_abs(e.particles[i].velocity_x);
    const Uint32 settled = random;
    const SotnVibhutiParticle first = e.particles[0];
    bool walled = false;
    for (Uint16 tick = 1; tick < 0x80; ++tick)
    {
        in.hit_flags = tick == 40 ? 1 : 0;
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &events) && random == settled &&
              !events.sound && e.timer == 0x80 - tick && !e.removed);
        if (tick == 1)
            CHECK(e.particles[0].velocity_y == first.velocity_y + 0x1800 &&
                  e.particles[0].y == first.y + first.velocity_y &&
                  e.particles[0].x == first.x + first.velocity_x);
        const Uint16 host = e.timer & 7U;
        const SotnVibhutiParticle *p = &e.particles[host];
        CHECK(Whole(e.x) == Whole(p->x) && (e.x & 0xffff) == 0x1234 && Whole(e.y) == Whole(p->y) &&
              (e.y & 0xffff) == 0x5678 && e.hitbox_y == (p->falling ? 0 : -6));
        for (Uint16 i = 0; i < SOTN_VIBHUTI_PARTICLES; ++i)
        {
            walled = walled || (!e.particles[i].velocity_x && Whole(e.particles[i].x) < 128);
            CHECK(Whole(e.particles[i].x) < 128 + 2);
            if (!e.particles[i].falling)
                CHECK(e.particles[i].width == 3 && Whole(e.particles[i].y) < 160 &&
                      Whole(e.particles[i].y) >= 160 - 4);
        }
        if (tick == 40)
            CHECK(e.particles[(e.timer + 1U) & 7U].draw_mode == 8);
        // Ends: one particle hides per update once timer-1 reaches it.
        if (e.timer <= 12)
            CHECK(e.particles[e.timer - 1U].draw_mode == 8);
        // ...and not the next one yet (particle1 was hidden by the hit).
        if (e.timer >= 3 && e.timer <= 13 && e.timer != 3)
            CHECK(e.particles[e.timer - 2U].draw_mode == 2);
    }
    for (Uint16 i = 0; i < SOTN_VIBHUTI_PARTICLES; ++i)
        CHECK(!e.particles[i].falling && e.particles[i].draw_mode == 8);
    CHECK(walled && e.hitbox_width == 0);
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &events) && e.removed && random == settled);
    return true;
}
static bool Runtime(SotnDataView executable)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r)), *before = SDL_malloc(sizeof(*before));
    SotnCharacter *character = NULL;
    CHECK(dra && actors && r && before);
    TestCharacter_Fill(dra);
    Uint8 *def = dra + 0x4a00 + SOTN_SUBWEAPON_VIBHUTI * 20U;
    def[0] = 8;
    def[2] = 3;
    def[6] = 2;
    def[7] = 10;
    def[8] = 8;
    def[10] = 0xbc;
    def[11] = SOTN_VIBHUTI_BLUEPRINT;
    def[12] = 2;
    def[14] = 1;
    Uint8 *bp = dra + 0xd1d4 + SOTN_VIBHUTI_BLUEPRINT * 6U;
    bp[0] = SOTN_VIBHUTI_ENTITY;
    bp[1] = 1;
    bp[2] = 0xc1;
    bp[4] = 2;
    dra[0xd4b8 + 4] = 32;
    dra[0xd4b9 + 4] = 47;
    SotnSubweaponDefinition d = {0};
    CHECK(SotnSubweapon_Read((SotnDataView){dra, 0x11000}, 8, &d) && d.blueprint == 26 &&
          d.chain == 2 && d.hearts == 3 && d.animation == 0xbc && d.attack.hit_type == 2);
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    // Row1 is busy: first-free would split the lease, func_800EDB58 takes rows2..14.
    actors->primitives[1].owner = 777;
    SotnBatRuntimeInput in = {
        .dra = actors->dra,
        .executable = executable,
        .food_collision = Map(),
        .player = {.hp = 70, .x = 100 * 65536, .y = 150 * 65536, .priority = 148}};
    bool allocated = false;
    CHECK(!SotnBatRuntime_RequestSubweapon(r, (SotnPlayerFactorySource){.priority = 148}, 27,
                                           8 << 9, &allocated));
    CHECK(!SotnBatRuntime_RequestSubweapon(r, (SotnPlayerFactorySource){.priority = 148}, 26,
                                           9 << 9, &allocated));
    SotnCharacterState progress = {0};
    SotnCharacterStats stats = {0};
    CHECK(SotnCharacter_Get(character, &progress, &stats));
    Uint32 expected = progress.random;
    const Uint16 roll = SotnCombat_Random(&expected);
    for (Uint16 i = 0; i < 24; ++i)
        (void)SotnCombat_Random(&expected);
    CHECK(
        SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
        SotnBatRuntime_RequestSubweapon(
            r,
            (SotnPlayerFactorySource){.x = 100 * 65536 + 7, .y = 150 * 65536 + 9, .priority = 148},
            26, 8 << 9, &allocated) &&
        allocated && SotnBatRuntime_Tick(r, character, in));
    SotnBatRuntimeChild *c = &r->children[16];
    CHECK(c->subweapon && c->initialized && c->kind == SOTN_VIBHUTI_ENTITY &&
          c->primitive_count == SOTN_VIBHUTI_ROWS && c->primitives[0] == 2 &&
          c->primitives[12] == 14 && c->group == 7 && r->groups[7] == 1 &&
          c->state.subweapon.attack.power == 8 + (stats.total[2] * 2 + roll % 12) / 10 &&
          c->state.subweapon.attack.hit_type == 2 &&
          c->state.subweapon.attack.invincibility_frames == 10 &&
          c->state.subweapon.attack.width == 4 && r->subweapon_mask == (Uint64)1 << 16 &&
          r->use_ticks == 4 && r->sound_count == 1 && r->sounds[0] == 0x60c &&
          !r->factories[0].cursor.entity_id && SotnBatRuntime_SubweaponCount(r, 8) == 1);
    CHECK(SotnCharacter_Get(character, &progress, NULL) && progress.random == expected);
    CHECK(SotnActors_CommitBatPrimitives(actors, r) && actors->primitives[1].owner == 777 &&
          actors->primitives[2].ability_kind == SOTN_VIBHUTI_ENTITY &&
          actors->primitives[14].owner == c->generation);
    // Enemy contact hides the previous hitbox particle on the next callback.
    CHECK(!SotnBatRuntime_Hit(r, 16, c->generation + 1, 1) &&
          SotnBatRuntime_Hit(r, 16, c->generation, 1) && c->state.subweapon.hit_flags == 1);
    in.freeze = 3;
    SDL_memcpy(before, r, sizeof(*r));
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) &&
          !SDL_memcmp(&before->children[16], &r->children[16], sizeof(r->children[16])) &&
          r->subweapon_mask == (Uint64)1 << 16);
    in.freeze = 0;
    CHECK(SotnBatRuntime_Tick(r, character, in) && !c->state.subweapon.hit_flags &&
          c->state.subweapon.motion.timer == 0x7f &&
          c->state.subweapon.motion.particles[0].draw_mode == 8);
    // Dead player: UpdatePlayerEntities clears the hitbox; Vibhuti never sets it again.
    in.player.step = SOTN_PLAYER_TELEPORT;
    CHECK(SotnBatRuntime_Tick(r, character, in) && !c->state.subweapon.attack.active &&
          !r->subweapon_mask);
    in.player.step = 0;
    CHECK(SotnBatRuntime_Tick(r, character, in) && !c->state.subweapon.attack.active &&
          !r->subweapon_mask && c->subweapon);
    CHECK(SotnBatRuntime_PruneSubweapons(r) && !c->kind && !r->subweapon_mask &&
          SotnActors_CommitBatPrimitives(actors, r) && !actors->primitives[2].owner);
    // Without a free run of thirteen rows the birth callback retires the child without RNG.
    SDL_zero(*r);
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; i += 12)
        actors->primitives[i].owner = 777;
    CHECK(SotnCharacter_Get(character, &progress, NULL));
    expected = progress.random;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestSubweapon(r, (SotnPlayerFactorySource){.priority = 148}, 26, 8 << 9,
                                          &allocated) &&
          allocated && SotnBatRuntime_Tick(r, character, in) && !r->children[16].kind &&
          !r->subweapon_mask && !r->sound_count && !r->groups[7] &&
          SotnCharacter_Get(character, &progress, NULL) && progress.random == expected);
    SotnCharacter_Destroy(character);
    SDL_free(before);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(dra);
    return true;
}
static bool Contiguous(void)
{
    SotnPrimitiveSlots slots = {0};
    Uint16 rows[4] = {0};
    bool allocated = false;
    slots.occupied[0] = 0x5; // rows0 and 2 busy
    CHECK(SotnPrimitiveSlots_ReserveContiguous(&slots, 3, rows, &allocated) && allocated &&
          rows[0] == 3 && rows[2] == 5 && slots.occupied[0] == 0x3d);
    SotnPrimitiveSlots full = {0};
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_WORDS; ++i)
        full.occupied[i] = 0x5555555555555555ULL;
    const SotnPrimitiveSlots copy = full;
    CHECK(SotnPrimitiveSlots_ReserveContiguous(&full, 2, rows, &allocated) && !allocated &&
          !SDL_memcmp(&copy, &full, sizeof(full)));
    CHECK(!SotnPrimitiveSlots_ReserveContiguous(&full, 0, rows, &allocated));
    return true;
}
int main(void)
{
    Uint8 *bytes = SDL_calloc(1, 0x23212);
    if (!bytes)
        return 1;
    for (Uint16 i = 0; i <= 1024; ++i)
    {
        Put16(bytes + 0x1dbcc + i * 2U, (Uint16)(i * 4U));
        Put16(bytes + 0x22a10 + i * 2U, (Uint16)(i / 2U));
    }
    const bool ok = Contiguous() && Component((SotnDataView){bytes, 0x23212}) &&
                    Runtime((SotnDataView){bytes, 0x23212});
    SDL_free(bytes);
    if (!ok)
        return 1;
    SDL_Log("VIBHUTI: PASS authored birth RNG/velocities, landing, wall, hitbox cycle, hit/expiry "
            "hiding, contiguous rows, runtime factory/group/power/mask/freeze/prune");
    return 0;
}
