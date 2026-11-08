// EntitySubwpnThrownVibhuti/EntitySubwpnReboundStone (dra/84B88.c) and their shared-runtime owner
// with authored tables only.
#include "actors_internal.h"
#include "boss_projection.h"
#include "character_fixture.h"
#include <SDL3/SDL.h>
#include <sotn/combat.h>
#include <sotn/game_math.h>
#include <sotn/player_particles.h>
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
// Floor: rows10..15 solid; ceiling rows0..3; wall columns at x0..15 and 128..143 (tile8).
static Uint8 tiles[32 * 16 * 2], types[2] = {0, 3};
static SotnCollisionMap Map(void)
{
    SDL_zeroa(tiles);
    for (Uint16 y = 0; y < 16; ++y)
        for (Uint16 x = 0; x < 32; ++x)
            if (y >= 10 || y < 4 || x == 8 || x == 0)
                Put16(tiles + ((size_t)y * 32 + x) * 2U, 1);
    return (SotnCollisionMap){{tiles, sizeof(tiles)}, {types, sizeof(types)}, 32, 16};
}
// func_8011A290 stand-in: one roll at its original point, counted.
static bool Roll(void *context, Uint32 *random)
{
    ++*(Uint32 *)context;
    (void)SotnCombat_Random(random);
    return true;
}
static bool Component(SotnDataView executable)
{
    Uint32 rolls = 0;
    SotnSubweaponInput in = {.executable = executable,
                             .collision = Map(),
                             .properties = Roll,
                             .context = &rolls,
                             .player_priority = 148};
    SotnSubweaponEntity e = {
        .x = 100 * 65536 + 0x1234, .y = 150 * 65536 + 0x5678, .id = SOTN_SUBWEAPON_VIBHUTI};
    SotnSubweaponEvents events = {0};
    Uint32 random = 77;
    // A failed func_800EDB58 reservation destroys the entity before any RNG.
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &events) && e.removed && random == 77 &&
          !events.sound && !events.use_ticks && !rolls);
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
        (void)SotnCombat_Random(&expected); // func_8011A290 before the particles
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &events) && rolls == facing + 1U &&
              e.step == 1 && e.timer == 0x80 && e.priority == 150 && e.flags == 0x08800000U &&
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
static Sint16 Angle(Sint32 v)
{
    const Uint16 bits = (Uint16)(Uint32)v;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
// EntitySubwpnReboundStone: angle roll before func_8011A290, 4 px/update flight, floor/wall/
// ceiling reflections with a factory10 tink per bounce, fifteen bounces then the 0x40 fade-out.
static bool Stone(SotnDataView executable)
{
    Uint32 rolls = 0;
    SotnSubweaponInput in = {.executable = executable,
                             .collision = Map(),
                             .properties = Roll,
                             .context = &rolls,
                             .player_priority = 148,
                             .allocated = true};
    SotnSubweaponEvents events = {0};
    for (Uint16 facing = 0; facing < 2; ++facing)
    {
        in.player_facing_left = facing != 0;
        SotnSubweaponEntity e = {
            .x = 100 * 65536 + 0x1234, .y = 150 * 65536, .id = SOTN_SUBWEAPON_REBOUND_STONE};
        Uint32 random = 99, expected = 99;
        const Uint16 first = SotnCombat_Random(&expected);
        (void)SotnCombat_Random(&expected);
        rolls = 0;
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &events) && rolls == 1 &&
              random == expected && e.step == 1 && e.flags == 0x0c800000U && e.priority == 150 &&
              e.stone.angle == Angle((facing ? 0x980 : 0xe80) + (first & 0x7f) - 0x40) &&
              e.stone.life == 0x40 && !e.stone.embed && e.hitbox_width == 4 &&
              events.sound == 0x60c && events.use_ticks == 4 && !events.factories &&
              e.stone.lines[0].draw_mode == 0x33 && e.stone.lines[0].timer == 19 &&
              e.stone.lines[1].draw_mode == 0x3b && e.stone.lines[1].timer == 20 &&
              e.stone.lines[0].x0 == 100 && e.stone.lines[0].x1 == 100 &&
              e.stone.lines[0].grey == 0xff);
    }
    // Born inside the floor: four updates hold the collision loop (the stone still moves).
    SotnSubweaponEntity e = {
        .x = 100 * 65536, .y = 165 * 65536, .id = SOTN_SUBWEAPON_REBOUND_STONE};
    Uint32 random = 5;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &events) && e.stone.embed == 4);
    const Sint32 held = e.x;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &events) && e.stone.embed == 3 && e.x != held &&
          !events.factories);
    // A full flight in the closed box.
    e = (SotnSubweaponEntity){
        .x = 100 * 65536, .y = 150 * 65536, .id = SOTN_SUBWEAPON_REBOUND_STONE};
    in.player_facing_left = false;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &events));
    const Uint32 settled = random;
    Uint16 bounces = 0, disabled = 0, ticks = 0, faded = 0;
    while (!e.removed && ticks++ < 900)
    {
        const SotnSubweaponEntity before = e;
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &events) && random == settled);
        if (e.removed)
            break;
        disabled += events.disable_attack;
        if (events.disable_attack)
            CHECK(e.stone.life == 0x20);
        if (before.step == 1 && events.factories == 1)
        {
            ++bounces;
            const SotnReboundLine *l = &e.stone.lines[e.stone.bounces];
            CHECK(e.stone.bounces == before.stone.bounces + 1 &&
                  events.factory_packed[0] == SOTN_REBOUND_STONE_SPARKS &&
                  events.factory_x == before.x && events.factory_y == before.y &&
                  events.sound == 0x6a4 && !(l->draw_mode & 8U) && l->x0 == Whole(before.x) &&
                  l->y0 == Whole(before.y) && l->x1 == Whole(e.x) && l->y1 == Whole(e.y));
            // Flat floors/ceilings reflect over 0x800, walls over 0x400 (a corner does both).
            const Sint32 a = before.stone.angle, n = e.stone.angle;
            CHECK(n == Angle(0x1000 - a) || n == Angle(0x800 - a) || n == Angle(a + 0x800) ||
                  n == Angle(a - 0x800));
        }
        else if (before.step == 1 && e.step == 1)
        {
            SotnMathTrig t = {0};
            CHECK(SotnMath_Trig(executable, e.stone.angle, &t) &&
                  e.stone.angle == before.stone.angle && !events.sound &&
                  e.x == before.x + Floor((Sint64)t.cosine * 16 * 0x400, 8) &&
                  e.y == before.y + Floor(-((Sint64)t.sine * 16 * 0x400), 8));
        }
        if (before.step == 1 && e.step == 1)
            for (Uint16 i = 0; i < SOTN_REBOUND_STONE_LINES; ++i)
                if (!(before.stone.lines[i].draw_mode & 8U) && !before.stone.lines[i].timer &&
                    before.stone.lines[i].grey > 2)
                    CHECK(e.stone.lines[i].grey + 2 == before.stone.lines[i].grey);
        if (before.step == 2)
        {
            CHECK(e.stone.life == before.stone.life - 1);
            for (Uint16 i = 0; i <= e.stone.bounces; ++i)
                faded += e.stone.lines[i].grey + 4 == before.stone.lines[i].grey;
        }
        for (Uint16 i = 0; i < SOTN_REBOUND_STONE_LINES; ++i)
            CHECK(((e.stone.lines[i].draw_mode & 8U) != 0) == (i > e.stone.bounces));
    }
    CHECK(e.removed && bounces == 15 && e.stone.bounces == 15 && disabled == 1 && faded > 100);
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
    // Rebound Stone: blueprint24 (origin2), first-free rows, angle then property roll, and its
    // bounce factory10's three profile0 tink bursts.
    Uint8 *stone = dra + 0x4a00 + SOTN_SUBWEAPON_REBOUND_STONE * 20U;
    stone[0] = 8;
    stone[2] = 2;
    stone[6] = 3;
    stone[7] = 0x20;
    stone[8] = 4;
    stone[10] = 0x5d;
    stone[11] = SOTN_REBOUND_STONE_BLUEPRINT;
    stone[12] = 2;
    stone[14] = 1;
    const Uint8 stone_bp[6] = {SOTN_REBOUND_STONE_ENTITY, 1, 0xc1, 0, 0x22, 0};
    const Uint8 sparks_bp[6] = {5, 3, 0xc1, 3, 0, 0};
    SDL_memcpy(dra + 0xd1d4 + SOTN_REBOUND_STONE_BLUEPRINT * 6U, stone_bp, 6);
    SDL_memcpy(dra + 0xd1d4 + SOTN_REBOUND_STONE_SPARKS * 6U, sparks_bp, 6);
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    TestCharacter_Put32(dra + 0xdb98, 0x800af000);
    const Uint8 profile0[16] = {8, 0x7f, 0x7f, 0x7f, 2, 2, 2, 0, 0x33, 0, 0, 0, 0, 0, 0x80, 0x08};
    SDL_memcpy(dra + 0xf000, profile0, 16);
    // Profile0 tink: rand()&0x7FFF is the angle; odd values move twice as fast; fade six.
    {
        SotnPlayerParticles t = {0};
        Uint32 rng = 4321, want = 4321;
        CHECK(SotnPlayerParticles_Tick(
                  &t, (SotnDataView){dra, 0x11000},
                  (SotnPlayerParticlesInput){.source = {.x = 50 * 65536, .y = 60 * 65536},
                                             .player_priority = 100,
                                             .executable = executable,
                                             .allocated = true},
                  &rng) &&
              t.count == 8 && t.priority == 102);
        for (Uint16 i = 0; i < 8; ++i)
        {
            const Uint16 angle = SotnCombat_Random(&want) & 32767U;
            SotnMathTrig tr = {0};
            CHECK(SotnMath_Trig(executable, angle, &tr));
            const Sint32 scale = (angle & 1U) ? 8 : 4;
            CHECK(t.tiles[i].velocity_x == tr.cosine * scale &&
                  t.tiles[i].velocity_y == -(tr.sine * scale) && t.tiles[i].red == 0x7f);
        }
        CHECK(rng == want);
        const SotnPlayerParticle first = t.tiles[0];
        Uint16 updates = 0;
        while (!t.removed && updates < 40)
        {
            CHECK(SotnPlayerParticles_Tick(
                &t, (SotnDataView){dra, 0x11000},
                (SotnPlayerParticlesInput){.player_priority = 100, .executable = executable},
                &rng));
            if (!updates)
                CHECK(t.tiles[0].red == 0x79 && t.tiles[0].x == first.x + first.velocity_x &&
                      t.tiles[0].y == first.y + first.velocity_y);
            ++updates;
        }
        CHECK(t.removed && updates == 20 && rng == want);
    }
    SDL_zero(*r);
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        actors->primitives[i].owner = 0;
    actors->primitives[1].owner = 777;
    CHECK(SotnCharacter_Get(character, &progress, &stats));
    expected = progress.random;
    (void)SotnCombat_Random(&expected);
    const Uint16 stone_roll = SotnCombat_Random(&expected);
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestSubweapon(r, (SotnPlayerFactorySource){.priority = 148}, 24, 7 << 9,
                                          &allocated) &&
          allocated && SotnBatRuntime_Tick(r, character, in));
    c = &r->children[16];
    CHECK(c->subweapon && c->initialized && c->kind == SOTN_REBOUND_STONE_ENTITY &&
          c->primitive_count == SOTN_REBOUND_STONE_LINES && c->primitives[0] == 0 &&
          c->primitives[1] == 2 && c->group == 7 &&
          c->state.subweapon.attack.power == 8 + (stats.total[2] * 2 + stone_roll % 12) / 10 &&
          c->state.subweapon.motion.x == in.player.x && r->subweapon_mask == (Uint64)1 << 16 &&
          SotnCharacter_Get(character, &progress, NULL) && progress.random == expected &&
          SotnActors_CommitBatPrimitives(actors, r));
    // A second live stone takes the next least-used group (func_80118894 scans 7..10 only).
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestSubweapon(r, (SotnPlayerFactorySource){.priority = 148}, 24, 7 << 9,
                                          &allocated) &&
          allocated && SotnBatRuntime_Tick(r, character, in) && r->children[17].subweapon &&
          r->children[17].group == 8 && r->groups[7] == 1 && r->groups[8] == 1 &&
          SotnActors_CommitBatPrimitives(actors, r));
    r->children[17] = (SotnBatRuntimeChild){0};
    CHECK(SotnActors_CommitBatPrimitives(actors, r));
    Uint16 sparks = 0, ticks = 0;
    while (!sparks && ticks++ < 60)
    {
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        for (Uint16 i = 16; i < 48; ++i)
            sparks += r->children[i].kind == 5 && r->children[i].initialized &&
                      r->children[i].primitive_count == 9 &&
                      r->children[i].state.particles.tiles[0].red == 0x7f - 6U;
    }
    CHECK(sparks == 1 && r->children[16].state.subweapon.motion.stone.bounces == 1);
    CHECK(SotnBatRuntime_PruneSubweapons(r) && !r->children[16].kind);
    SotnCharacter_Destroy(character);
    SDL_free(before);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(dra);
    return true;
}
static Uint8 Fade(Uint8 v, Uint16 n)
{
    for (Uint16 i = 0; i < n; ++i)
        v = (Uint8)(v >= 4 ? v - 4 : v);
    return v;
}
// EntitySubwpnAgunea: birth, flight, binding, held strikes and their costs, exits.
static bool Agunea(SotnDataView executable)
{
    Uint32 rolls = 0;
    static SotnSpellTarget targets[SOTN_STAGE_TARGETS];
    SDL_zeroa(targets);
    SotnSubweaponInput in = {.executable = executable,
                             .properties = Roll,
                             .context = &rolls,
                             .screen_x = 50,
                             .screen_y = 20,
                             .player_priority = 148,
                             .player_x = 150 * 65536 + 0x4000,
                             .player_y = 140 * 65536,
                             .player_hitbox_y = -6,
                             .targets = targets,
                             .hearts = 7,
                             .bolt_cost = 5,
                             .player_status = 8};
    const SotnSubweaponEntity fresh = {.x = 1 * 65536 + 0x1111,
                                       .y = 2 * 65536 + 0x2222,
                                       .id = SOTN_SUBWEAPON_AGUNEA,
                                       .priority = 148};
    SotnSubweaponEntity e = fresh;
    SotnSubweaponEvents ev = {0};
    Uint32 random = 31;
    // Any transformation (or status0x10000) retires it before allocation or RNG.
    in.allocated = true;
    const Uint32 statuses[4] = {1, 2, 4, 0x10000};
    for (Uint16 i = 0; i < 4; ++i)
    {
        e = fresh;
        in.player_status = statuses[i];
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed && random == 31 &&
              !rolls && !ev.factories && !ev.sound);
    }
    in.player_status = 8;
    in.allocated = false;
    e = fresh;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed && random == 31 && !rolls);
    for (Uint16 facing = 0; facing < 2; ++facing)
    {
        e = fresh;
        in.allocated = true;
        in.player_facing_left = facing != 0;
        Uint32 expected = random;
        (void)SotnCombat_Random(&expected);
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && rolls == facing + 1U &&
              random == expected && e.step == 1 && e.flags == 0x0c800000U &&
              e.facing_left == (facing != 0) && e.hitbox_x == 4 && !e.hitbox_y &&
              e.hitbox_width == 4 && e.hitbox_height == 4 && e.x == 150 * 65536 + 0x1111 &&
              e.y == 126 * 65536 + 0x2222 && e.velocity_x == (facing ? -0x60000 : 0x60000) &&
              e.priority == 148);
        const SotnAgunea *a = &e.agunea;
        CHECK(a->start_x == 150 && a->start_y == 126 && a->end_x == 150 && a->end_y == 126 &&
              a->priority == 150 && a->draw_mode == 0x331 && a->red == 0x5c && !a->green &&
              a->blue == 0x7c && !a->struck && !a->counter && ev.sound == 0x60c &&
              ev.use_ticks == 4 && ev.factories == 1 && ev.factory_packed[0] == SOTN_AGUNEA_FLASH &&
              ev.factory_x == e.x && ev.factory_y == e.y && !ev.hearts_spent && !ev.disable_attack);
    }
    // Flight: six pixels an update while the line's far end follows; it never reads input.
    const Sint32 x = e.x;
    in.player_x += 40 * 65536;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 1 && e.x == x - 0x60000 &&
          e.agunea.end_x == 144 && e.agunea.start_x == 150 && e.agunea.red == 0x58 &&
          e.agunea.blue == 0x78 && !ev.sound && !ev.factories && !ev.use_ticks);
    // The screen window: x -0x40..0x140, y -0x20..0x120 (screen origin 50, 20).
    {
        const struct
        {
            Sint32 x, y;
            bool out;
        } window[6] = {{50 + 0x140 - 6, 100, false}, {50 + 0x141 - 6, 100, true},
                       {100, 20 + 0x120, false},     {100, 20 + 0x121, true},
                       {100, 20 - 0x20, false},      {100, 20 - 0x21, true}};
        for (Uint16 i = 0; i < 6; ++i)
        {
            SotnSubweaponEntity w = e;
            Uint32 copy = random;
            w.velocity_x = 0x60000;
            w.x = window[i].x * 65536;
            w.y = window[i].y * 65536;
            CHECK(SotnSubweaponEntity_Tick(&w, in, &copy, &ev) &&
                  w.step == (window[i].out ? 2 : 1));
        }
    }
    // Leaving the screen window (beyond -0x40) starts the fade; red below five retires it.
    e.x = (50 - 0x3a) * 65536; // screen -0x40 after the move: still inside
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 1);
    e.x = (50 - 0x3b) * 65536;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 2 && e.agunea.red == 0x50);
    const Uint32 quiet = random;
    Uint16 ticks = 0;
    while (!e.removed && ticks++ < 40)
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && random == quiet);
    CHECK(e.removed && ticks == 20);
    // Bind: a hit in flight retains collision's unkB8 slot (even past the window test).
    in.player_facing_left = false;
    e = fresh;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    e.x = 500 * 65536;
    targets[150] = (SotnSpellTarget){.identity = 0x31, .hp = 50, .x = 120, .y = 100};
    in.hit_flags = 2;
    in.hit_target = 150;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 3 && e.agunea.target == 150);
    in.hit_flags = 0;
    in.hit_target = 3;
    in.buttons = SOTN_AGUNEA_HELD | 0x20;
    // The first strike is free: two jitter rolls, lightning + outline factories, THUNDER_B.
    Uint32 expected = random;
    const Uint16 jx = SotnCombat_Random(&expected), jy = SotnCombat_Random(&expected);
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && random == expected && e.step == 3 &&
          Whole(e.x) == 170 && Whole(e.y) == 120 && ev.factories == 2 &&
          ev.factory_packed[0] == SOTN_AGUNEA_BOLT_BLUEPRINT &&
          ev.factory_packed[1] == SOTN_AGUNEA_OUTLINE && ev.sound == 0x665 &&
          Whole(ev.factory_x) == 170 + (jx & 15) - 8 &&
          Whole(ev.factory_y) == 120 + (jy & 15) - 8 && !ev.hearts_spent && e.agunea.struck &&
          e.agunea.counter == 1 && e.agunea.end_x == 170 && e.agunea.end_y == 120);
    // Eleven quiet updates follow the moving target without RNG.
    for (Uint16 i = 0; i < 11; ++i)
    {
        targets[150].x = (Sint16)(targets[150].x + 1);
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && random == expected &&
              !ev.factories && Whole(e.x) == 50 + targets[150].x && e.step == 3);
    }
    // The second strike costs five hearts (exactly five suffice).
    in.hearts = 5;
    expected = random;
    (void)SotnCombat_Random(&expected);
    (void)SotnCombat_Random(&expected);
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && random == expected &&
          ev.factories == 2 && ev.hearts_spent == 5 && e.agunea.counter == 13);
    // Line colours fade by four per update; the line hides once blue drops below five.
    CHECK(e.agunea.red == Fade(0x60, 15) && e.agunea.blue == Fade(0x80, 15) &&
          e.agunea.draw_mode == 0x331);
    {
        SotnSubweaponEntity hidden = e;
        Uint32 copy = random;
        hidden.agunea.blue = 8;
        hidden.agunea.green = 3;
        CHECK(SotnSubweaponEntity_Tick(&hidden, in, &copy, &ev) && hidden.agunea.blue == 4 &&
              hidden.agunea.green == 3 && hidden.agunea.draw_mode == 0x339);
        hidden.agunea.blue = 9;
        hidden.agunea.draw_mode = 0x331;
        CHECK(SotnSubweaponEntity_Tick(&hidden, in, &copy, &ev) && hidden.agunea.blue == 5 &&
              hidden.agunea.draw_mode == 0x331);
    }
    // Too few hearts: no strike (the jitter still rolls) and step4.
    in.hearts = 4;
    for (Uint16 i = 0; i < 11; ++i)
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    expected = random;
    (void)SotnCombat_Random(&expected);
    (void)SotnCombat_Random(&expected);
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && random == expected && e.step == 4 &&
          !ev.factories && !ev.hearts_spent && !ev.sound && e.agunea.counter == 25 &&
          e.agunea.blue == Fade(0x80, 27) && !e.agunea.red);
    // Step4 past sixteen falls through to the red check: retired.
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed);
    // A target that died (hp0, FLAG_DEAD, or hp > 0x7000) after the first strike: step2 and no
    // primitive update that callback.
    const Sint16 hps[3] = {0, 50, 0x7001};
    for (Uint16 k = 0; k < 3; ++k)
    {
        e = fresh;
        in.hearts = 7;
        targets[150] = (SotnSpellTarget){.identity = 0x31, .hp = 50, .x = 120, .y = 100};
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
        in.hit_flags = 1;
        in.hit_target = 150;
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 3);
        in.hit_flags = 0;
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && ev.factories == 2);
        targets[150].hp = hps[k];
        targets[150].flags = k == 1 ? 0x100U : 0;
        const SotnSubweaponEntity before = e;
        const Uint32 held = random;
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 2 && random == held &&
              e.agunea.red == before.agunea.red && e.x == before.x && !ev.factories);
    }
    // A huge-HP target still takes the first (counter0) strike.
    e = fresh;
    targets[150] = (SotnSpellTarget){.identity = 0x31, .hp = 0x7fff, .x = 120, .y = 100};
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    in.hit_flags = 1;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 3);
    in.hit_flags = 0;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && ev.factories == 2 && e.step == 3);
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 2);
    // An emptied slot: step2 immediately; releasing Up+Square: step4, which follows the slot
    // (zeroed: the screen origin) for sixteen counts.
    e = fresh;
    targets[150] = (SotnSpellTarget){.identity = 0x31, .hp = 9, .x = 120, .y = 100};
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    in.hit_flags = 1;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 3);
    in.hit_flags = 0;
    in.buttons = 0x1000;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 4 && ev.factories == 2);
    targets[150] = (SotnSpellTarget){.x = 77, .y = 66};
    for (Uint16 i = 0; i < 13; ++i)
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 4 && Whole(e.x) == 50 &&
              Whole(e.y) == 20 && !e.removed);
    // Below sixteen the follow step skips the red test; at sixteen it falls through to it.
    e.agunea.red = 4;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && !e.removed && e.agunea.counter == 15 &&
          !e.agunea.red);
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed);
    e = fresh;
    in.buttons = SOTN_AGUNEA_HELD;
    targets[150] = (SotnSpellTarget){.identity = 0x31, .hp = 9, .x = 120, .y = 100};
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    in.hit_flags = 1;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 3);
    in.hit_flags = 0;
    targets[150] = (SotnSpellTarget){0};
    const Uint32 held = random;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 2 && random == held &&
          !ev.factories);
    // Retained slots outside the stage view, or no view, fail explicitly.
    in.targets = NULL;
    e.step = 3;
    CHECK(!SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    in.targets = targets;
    in.hit_target = SOTN_STAGE_TARGETS;
    CHECK(!SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    return true;
}
// EntityAguneaHitEnemy: birth roll, two segments an update toward the parent, the white flash and
// the shared-grey fade.
static bool Bolt(SotnDataView executable)
{
    SotnSubweaponInput in = {.executable = executable,
                             .screen_x = 10,
                             .screen_y = 5,
                             .player_priority = 148,
                             .player_x = 150 * 65536 + 0x4000,
                             .player_y = 140 * 65536,
                             .player_hitbox_y = -6,
                             .parent_entity = 0,
                             .parent_x = 2000,
                             .parent_y = 126};
    const SotnSubweaponEntity fresh = {.id = SOTN_AGUNEA_BOLT, .priority = 140};
    SotnSubweaponEntity e = fresh;
    SotnSubweaponEvents ev = {0};
    Uint32 random = 41;
    // Without its Agunea parent the birth callback destroys it before RNG.
    in.allocated = true;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed && random == 41);
    in.parent_entity = SOTN_AGUNEA_ENTITY;
    e = fresh;
    Uint32 expected = random;
    const Uint16 roll = SotnCombat_Random(&expected);
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && random == expected && e.step == 1 &&
          e.flags == 0x04800000U && Whole(e.x) == 150 && Whole(e.y) == 126 &&
          e.bolt.angle == (Sint16)(((Sint32)(roll & 0x3ff) - 0x200) & 0xfff) && !e.bolt.end &&
          e.bolt.grey == 0x80 && !e.bolt.count && e.bolt.x[0][2] == 150 && e.bolt.y[0][2] == 126 &&
          e.bolt.x[0][3] == 150 && e.bolt.y[0][3] == 110 && !ev.factories && !ev.sound);
    // Far from the parent: two segments an update, each wobbling with one roll.
    Uint16 updates = 0;
    while (e.step == 1 && updates++ < 30)
    {
        const SotnSubweaponEntity before = e;
        expected = random;
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
        if (e.step != 1)
            break;
        CHECK(e.bolt.end == before.bolt.end + 2);
        for (Uint8 n = (Uint8)(before.bolt.end + 1); n <= e.bolt.end; ++n)
        {
            const Sint32 dx = e.bolt.x[n][2] - e.bolt.x[n][0], dy = e.bolt.y[n][2] - e.bolt.y[n][0];
            CHECK(e.bolt.x[n][0] == e.bolt.x[n - 1][2] && e.bolt.y[n][0] == e.bolt.y[n - 1][2] &&
                  e.bolt.x[n][1] == e.bolt.x[n - 1][3] && e.bolt.y[n][1] == e.bolt.y[n - 1][3] &&
                  SDL_abs(dx) <= 12 && SDL_abs(dy) <= 12 && (dx || dy));
        }
        (void)SotnCombat_Random(&expected);
        (void)SotnCombat_Random(&expected);
        CHECK(random == expected);
    }
    // Forty rows: the chain stops at row39 and the flash follows.
    CHECK(updates == 20 && e.step == 2 && e.bolt.end == 39 && !e.bolt.white);
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 2 && e.bolt.white &&
          e.bolt.flashed && e.bolt.grey == 0x80);
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 3 && !e.bolt.white &&
          !e.bolt.flashed && e.bolt.grey == 0x60);
    updates = 0;
    while (!e.removed && updates++ < 40)
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) &&
              (e.removed || e.bolt.grey == 0x60 - 4 * updates));
    CHECK(updates == 21);
    // Reaching the parent (within eight pixels) ends growth without a roll; within 64 pixels the
    // segments stop wobbling (no roll); losing the parent skips the flash.
    e = fresh;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    const SotnSubweaponEntity born = e;
    in.parent_x = e.bolt.x[0][2] + 7;
    in.parent_y = e.bolt.y[0][2] - 7;
    const Uint32 held = random;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 2 && !e.bolt.end &&
          random == held);
    const Sint32 gaps[4][2] = {{8, 0}, {0, 8}, {0x3f, 0x3f}, {0x40, 0}};
    for (Uint16 i = 0; i < 4; ++i)
    {
        e = born;
        Uint32 copy = random;
        in.parent_x = e.bolt.x[0][2] + gaps[i][0];
        in.parent_y = e.bolt.y[0][2] - gaps[i][1];
        CHECK(SotnSubweaponEntity_Tick(&e, in, &copy, &ev) && e.bolt.end >= 1);
        // Only the far case (64 pixels) rolls a wobble for its first segment.
        CHECK(i == 3 ? copy != random : copy == random);
    }
    // One far update, exactly: steer a quarter of the turn toward ratan2 (kept for four
    // segments), wobble, a twelve-pixel segment, sixteen pixels wide at the angle minus 0x400.
    {
        in.parent_x = 2000;
        in.parent_y = 126;
        Uint32 cursor = held;
        Sint16 angle = born.bolt.angle, turn = 0;
        Sint32 x2 = born.bolt.x[0][2], y2 = born.bolt.y[0][2];
        for (Uint16 k = 0; k < 2; ++k)
        {
            if (!k)
            {
                Sint16 heading = 0;
                CHECK(SotnMath_Atan2(executable, -(Sint32)(Sint16)(in.parent_y - y2),
                                     (Sint16)(in.parent_x - x2), &heading));
                Sint32 t = (Sint16)(heading - angle);
                if (t > 0x800)
                    t = 0x1000 - t;
                if (t < -0x800)
                    t = 0x1000 + t;
                turn = (Sint16)(t / 4);
            }
            const Sint32 a =
                angle + turn + 0x180 - (Sint32)((SotnCombat_Random(&cursor) & 3U) << 8);
            angle = (Sint16)(a & 0xfff);
            SotnMathTrig t = {0};
            CHECK(SotnMath_Trig(executable, angle, &t));
            x2 += Floor(t.cosine * 12, 12);
            y2 -= Floor(t.sine * 12, 12);
        }
        e = born;
        Uint32 again = held;
        SotnMathTrig w = {0};
        CHECK(SotnSubweaponEntity_Tick(&e, in, &again, &ev) && e.bolt.end == 2 &&
              e.bolt.turn == turn && e.bolt.count == 2 && e.bolt.angle == angle &&
              e.bolt.x[2][2] == x2 && e.bolt.y[2][2] == y2 && again == cursor &&
              SotnMath_Trig(executable, angle - 0x400, &w) &&
              e.bolt.x[2][3] == x2 + Floor(w.cosine * 16, 12) &&
              e.bolt.y[2][3] == y2 - Floor(w.sine * 16, 12));
    }
    e = fresh;
    in.parent_x = 170;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 1 && e.bolt.end == 2);
    in.parent_entity = 0x1e;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 3 && e.bolt.grey == 0x7c &&
          !e.bolt.white);
    return true;
}
// EntitySubwpnBible/EntitySubwpnBibleTrail: birth, reveal, orbit (exact), swing, launch, fall
// and the trail's fade.
static bool Bible(SotnDataView executable)
{
    Uint32 rolls = 0;
    SotnSubweaponInput in = {.executable = executable,
                             .properties = Roll,
                             .context = &rolls,
                             .screen_x = 30,
                             .screen_y = 10,
                             .player_priority = 148,
                             .player_x = 150 * 65536 + 0x4000,
                             .player_y = 140 * 65536,
                             .game_timer = 1};
    const SotnSubweaponEntity fresh = {.x = 152 * 65536 + 0x1234,
                                       .y = 140 * 65536 + 0x5678,
                                       .id = SOTN_SUBWEAPON_BIBLE,
                                       .priority = 148};
    SotnSubweaponEntity e = fresh;
    SotnSubweaponEvents ev = {0};
    Uint32 random = 9;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed && random == 9 && !rolls);
    in.allocated = true;
    for (Uint16 facing = 0; facing < 2; ++facing)
    {
        e = fresh;
        e.facing_left = facing != 0;
        Uint32 expected = random;
        (void)SotnCombat_Random(&expected);
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && random == expected &&
              rolls == facing + 1U && e.step == 1 && e.flags == 0x04820000U &&
              e.bible.swing == (facing ? 32 : -32) && e.hitbox_width == 6 && e.hitbox_height == 6 &&
              !e.bible.mode && !ev.factories && !ev.sound && !ev.use_ticks && e.x == fresh.x &&
              e.y == fresh.y);
    }
    // Reveal: timer10, then each update orbits (exactly), drops a trail factory and swishes on
    // multiples of ten.
    Uint32 held = random;
    for (Uint16 tick = 0; tick < 0x30; ++tick)
    {
        const SotnSubweaponEntity before = e;
        in.game_timer = tick == 3 ? 20 : 1;
        in.player_priority = (Uint16)(148 + tick % 3);
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && random == held &&
              e.bible.mode == 1 && e.bible.count == tick + 1 && e.bible.radius == tick + 1 &&
              e.step == (tick + 1 >= 0x30 ? 3 : 2) && ev.use_ticks == (tick ? 0 : 4) &&
              ev.factories == 1 && ev.factory_packed[0] == SOTN_BIBLE_TRAIL_BLUEPRINT &&
              ev.factory_x == e.x && ev.factory_y == e.y && ev.sound == (tick == 3 ? 0x8c3 : 0) &&
              !ev.sound_after);
        // The original orbit with var_s4 zero.
        SotnMathTrig t = {0}, u = {0};
        CHECK(SotnMath_Trig(executable, before.bible.angle, &t) &&
              SotnMath_Trig(executable, before.bible.tilt, &u));
        const Sint32 r = before.bible.radius + 1;
        const Sint32 s5 = Floor(t.sine * r, 12), s3 = Floor(t.cosine * r, 12);
        const Sint32 a5 = Floor(t.cosine * s5 + t.sine * s3, 12),
                     a3 = Floor(t.cosine * s3 - t.sine * s5, 12);
        const Sint32 x = Floor(u.cosine * a5, 12), d = Floor(-u.sine * a5, 12);
        const Sint32 y = Floor(u.cosine * d + (e.facing_left ? 1 : -1) * u.sine * a3, 12);
        CHECK(Whole(e.x) == 150 + x && Whole(e.y) == 140 + y && (e.x & 0xffff) == 0x1234 &&
              e.priority == in.player_priority + (a3 < 0 ? 2 : -2) &&
              e.bible.angle == ((before.bible.angle + 0x80) & 0xfff));
        const Sint16 tilt = (Sint16)(before.bible.tilt + before.bible.swing);
        CHECK(e.bible.tilt == tilt &&
              e.bible.swing ==
                  (tilt >= 0x200 || tilt <= -0x200 ? -before.bible.swing : before.bible.swing));
    }
    // Facing right the orbit turns the other way and the tilt's height term changes sign.
    {
        SotnSubweaponEntity right = fresh;
        Uint32 copy = random;
        right.facing_left = false;
        CHECK(SotnSubweaponEntity_Tick(&right, in, &copy, &ev) && right.bible.swing == -32);
        for (Uint16 tick = 0; tick < 3; ++tick)
        {
            const SotnSubweaponEntity before = right;
            CHECK(SotnSubweaponEntity_Tick(&right, in, &copy, &ev));
            SotnMathTrig t = {0}, u = {0};
            CHECK(SotnMath_Trig(executable, before.bible.angle, &t) &&
                  SotnMath_Trig(executable, before.bible.tilt, &u));
            const Sint32 r = before.bible.radius + 1;
            const Sint32 s5 = Floor(t.sine * r, 12), s3 = Floor(t.cosine * r, 12);
            const Sint32 a5 = Floor(t.cosine * s5 + t.sine * s3, 12),
                         a3 = Floor(t.cosine * s3 - t.sine * s5, 12);
            const Sint32 d = Floor(-u.sine * a5, 12);
            CHECK(Whole(right.x) == 150 + Floor(u.cosine * a5, 12) &&
                  Whole(right.y) == 140 + Floor(u.cosine * d - u.sine * a3, 12) &&
                  right.bible.angle == ((before.bible.angle - 0x80) & 0xfff) &&
                  right.bible.tilt == before.bible.tilt - 32);
        }
    }
    // The swing turns at +-0x200.
    {
        SotnSubweaponEntity w = e;
        Uint32 copy = random;
        w.bible.tilt = 0x1e0;
        w.bible.swing = 32;
        CHECK(SotnSubweaponEntity_Tick(&w, in, &copy, &ev) && w.bible.tilt == 0x200 &&
              w.bible.swing == -32);
        w.bible.tilt = -0x1df;
        w.bible.swing = -32;
        CHECK(SotnSubweaponEntity_Tick(&w, in, &copy, &ev) && w.bible.tilt == -0x1ff &&
              w.bible.swing == -32);
    }
    // Launch at count 0x12C (facing left): off-camera retirement, FIX(-12)/FIX(-12), the scrape
    // (and a swish after it on a multiple of ten).
    e.bible.count = 0x12a;
    in.game_timer = 1;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 3 && e.bible.mode == 1);
    in.game_timer = 30;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 4 && e.bible.mode == 2 &&
          e.flags == 0x00820000U && e.velocity_x == -0xc0000 && e.bible.velocity_y == -0xe0000 &&
          ev.sound == 0x6b2 && ev.sound_after == 0x8c3 && ev.factories == 1);
    // The fall: FIX(2) gravity after each move.
    for (Uint16 tick = 0; tick < 3; ++tick)
    {
        const SotnSubweaponEntity before = e;
        in.game_timer = 1;
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 4 &&
              e.x == before.x - 0xc0000 && e.y == before.y + before.bible.velocity_y &&
              e.bible.velocity_y == before.bible.velocity_y - 0x20000 && ev.factories == 1 &&
              !ev.sound && random == held);
    }
    // The trail: placed once, grey 0x60 fading by eight, retired on its eighth callback.
    SotnSubweaponEntity t = {
        .x = 77 * 65536 + 0x8000, .y = 66 * 65536, .id = SOTN_BIBLE_TRAIL, .priority = 140};
    CHECK(SotnSubweaponEntity_Tick(&t, in, &random, &ev) && t.step == 1 && t.flags == 0x04820000U &&
          t.trail.x == 77 && t.trail.y == 66 && t.trail.grey == 0x60 && !ev.factories &&
          !ev.sound && random == held);
    for (Uint16 tick = 1; tick <= 6; ++tick)
        CHECK(SotnSubweaponEntity_Tick(&t, in, &random, &ev) && !t.removed &&
              t.trail.count == tick && t.trail.grey == 0x60 - 8 * tick &&
              t.step == (tick > 5 ? 2 : 1));
    CHECK(SotnSubweaponEntity_Tick(&t, in, &random, &ev) && t.removed);
    return true;
}
// Shared runtime: factory/spawn, the hit target, held strikes with their blueprint23 lightning
// (raw parent slot) and payment, screen-anchored rows, the 0x52 blink hold and room cleanup.
static bool AguneaRuntime(SotnDataView executable)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    SotnSpiritTargetSnapshot *targets = SDL_calloc(1, sizeof(*targets));
    SotnCharacter *character = NULL;
    CHECK(dra && actors && r && targets);
    TestCharacter_Fill(dra);
    Uint8 *def = dra + 0x4a00 + SOTN_SUBWEAPON_AGUNEA * 20U;
    def[0] = 10;
    def[2] = 5;
    Put16(def + 4, 0x5000);
    def[6] = 1;
    def[7] = 10;
    def[8] = 4;
    def[10] = 0x5d;
    def[11] = SOTN_AGUNEA_BLUEPRINT;
    def[12] = 2;
    def[14] = 1;
    const Uint8 blueprints[4][7] = {{22, 17, 1, 0xc1, 0, 0x22, 0},
                                    {23, 18, 1, 0xc1, 0, 0x20, 0},
                                    {61, 31, 3, 0xc1, 5, 0x20, 0},
                                    {44, 11, 1, 0x41, 0, 0, 0}};
    for (Uint16 i = 0; i < 4; ++i)
        SDL_memcpy(dra + 0xd1d4 + blueprints[i][0] * 6U, blueprints[i] + 1, 6);
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    dra[0xd4b8 + 4] = 32;
    dra[0xd4b9 + 4] = 47;
    // BlinkWhite profile 0x12 holds 0x7003 while an Agunea lives.
    Uint8 *blink = dra + 0xd670 + 0x12 * 20U;
    Put16(blink + 6, 0x123);
    Put16(blink + 8, 256);
    Put16(blink + 10, 256);
    Put16(blink + 12, 256);
    Put16(blink + 14, 0x7003);
    Put16(blink + 16, 0x31);
    Put16(blink + 18, 512);
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    SotnCharacterState progress = {0};
    CHECK(SotnCharacter_Get(character, &progress, NULL));
    progress.hearts = 40;
    CHECK(SotnCharacter_Restore(character, &progress));
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    targets->slots[70] = (SotnSpellTarget){.identity = 0x31, .hp = 40, .x = 130, .y = 90};
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = executable,
                              .food_collision = Map(),
                              .image = {20, 32, -10, -20},
                              .player = {.hp = 70,
                                         .x = 100 * 65536,
                                         .y = 150 * 65536,
                                         .priority = 148,
                                         .animation_bank = 1},
                              .spirit_targets = targets};
    bool allocated = false;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestSubweapon(
              r, (SotnPlayerFactorySource){.x = 100 * 65536, .y = 150 * 65536, .priority = 148},
              SOTN_AGUNEA_BLUEPRINT, SOTN_SUBWEAPON_AGUNEA << 9, &allocated) &&
          allocated && SotnBatRuntime_Tick(r, character, in));
    SotnBatRuntimeChild *c = &r->children[16];
    CHECK(c->subweapon && c->initialized && c->kind == SOTN_AGUNEA_ENTITY &&
          c->primitive_count == 1 && c->group == 7 && SotnBatRuntime_SubweaponValid(c, 16) &&
          c->state.subweapon.attack.width == 4 && c->state.subweapon.attack.offset_x == 4 &&
          r->subweapon_mask == (Uint64)1 << 16 && r->use_ticks == 4 &&
          SotnBatRuntime_SubweaponCount(r, SOTN_SUBWEAPON_AGUNEA) == 1 &&
          r->factories[0].cursor.blueprint == 44 && !r->factories[0].subweapon_parent_slot);
    CHECK(SotnActors_CommitBatPrimitives(actors, r));
    // The blink lives (slot63) and holds while the Agunea does.
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && r->children[47].kind == 11);
    // Collision records the overlapping stage slot; stale generations/slots are rejected.
    CHECK(!SotnBatRuntime_HitTarget(r, 16, c->generation + 1, 1, 70) &&
          !SotnBatRuntime_HitTarget(r, 16, c->generation, 1, SOTN_STAGE_TARGETS) &&
          SotnBatRuntime_HitTarget(r, 16, c->generation, 1, 70) &&
          c->state.subweapon.hit_target == 70);
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && c->state.subweapon.motion.step == 3 &&
          c->state.subweapon.motion.agunea.target == 70 && !c->state.subweapon.hit_flags);
    // Held Up+Square: the free strike requests lightning (raw parent slot32) and outlines.
    in.player.buttons = SOTN_AGUNEA_HELD;
    in.camera_x = 20;
    in.screen_shift_x = -5; // the camera moved: screen origin 15, rows keep screen position
    const Sint32 start = c->state.subweapon.motion.agunea.start_x;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && Whole(c->state.subweapon.motion.x) == 145 &&
          c->state.subweapon.motion.agunea.start_x == start + 5 &&
          c->state.subweapon.motion.agunea.end_x == 150);
    Uint16 lightning = 0, outline = 0;
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
    {
        lightning += r->factories[i].cursor.entity_id && r->factories[i].cursor.blueprint == 23 &&
                     r->factories[i].subweapon_parent_slot == 32;
        outline += r->factories[i].cursor.entity_id && r->factories[i].cursor.blueprint == 61 &&
                   !r->factories[i].subweapon_parent_slot;
    }
    CHECK(lightning == 1 && outline == 1);
    in.screen_shift_x = 0;
    in.camera_x = 15;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in));
    SotnBatRuntimeChild *b = NULL;
    Uint16 bolt_index = 0;
    for (Uint16 i = 16; i < 48; ++i)
        if (r->children[i].kind == SOTN_AGUNEA_BOLT_ENTITY)
        {
            b = &r->children[i];
            bolt_index = i;
        }
    CHECK(b && b->subweapon && b->initialized && b->state.subweapon.parent == 32 &&
          b->state.subweapon.motion.id == SOTN_AGUNEA_BOLT && b->primitive_count == 40 &&
          !b->group && !b->state.subweapon.attack.active &&
          SotnBatRuntime_SubweaponValid(b, bolt_index) &&
          SotnBatRuntime_SubweaponCount(r, SOTN_SUBWEAPON_AGUNEA) == 1 &&
          !(r->subweapon_mask & ((Uint64)1 << bolt_index)) &&
          SotnActors_CommitBatPrimitives(actors, r));
    const Sint16 row0 = b->state.subweapon.motion.bolt.x[0][2];
    in.screen_shift_x = -3;
    in.camera_x = 18;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) &&
          b->state.subweapon.motion.bolt.x[0][2] == row0 + 3 &&
          b->state.subweapon.motion.bolt.end == 2);
    in.screen_shift_x = 0;
    // Nine more quiet updates, then the paid strike (five hearts).
    for (Uint16 i = 0; i < 9; ++i)
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in));
    CHECK(SotnCharacter_Get(character, &progress, NULL) && progress.hearts == 40 &&
          c->state.subweapon.motion.agunea.counter == 12);
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnCharacter_Get(character, &progress, NULL) &&
          progress.hearts == 35);
    for (Uint16 i = 0; i < 20; ++i)
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && c->kind == SOTN_AGUNEA_ENTITY);
    CHECK(SotnCharacter_Get(character, &progress, NULL) && progress.hearts == 30 &&
          r->children[47].kind == 11 && r->children[47].state.blink.phase == 2);
    // Room entry retires the bolt and lightning (and their pending factories); the blink is
    // released on its next callback.
    CHECK(SotnBatRuntime_PruneSubweapons(r) && !c->kind && !b->kind);
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        CHECK(!r->factories[i].cursor.entity_id ||
              (r->factories[i].cursor.blueprint != 23 && r->factories[i].cursor.blueprint != 22));
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && r->children[47].state.blink.phase == 3);
    SotnCharacter_Destroy(character);
    SDL_free(targets);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(dra);
    return true;
}
// Shared runtime: the book's factory/group, its trails, screen anchoring under camera motion,
// FLAG_UNK_20000 room retention (no room rebase) and the off-camera cull after the launch.
static bool BibleRuntime(SotnDataView executable)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r)), *other = SDL_malloc(sizeof(*r));
    SotnCharacter *character = NULL;
    CHECK(dra && actors && r && other);
    TestCharacter_Fill(dra);
    Uint8 *def = dra + 0x4a00 + SOTN_SUBWEAPON_BIBLE * 20U;
    def[0] = 8;
    def[2] = 5;
    Put16(def + 4, 0x1000);
    def[6] = 1;
    def[7] = 0x18;
    def[8] = 4;
    def[10] = 0x5d;
    def[11] = SOTN_BIBLE_BLUEPRINT;
    def[12] = 2;
    def[14] = 1;
    const Uint8 blueprints[2][7] = {{77, 43, 1, 0xc1, 0, 2, 0}, {79, 44, 1, 0xc1, 0, 0, 0}};
    for (Uint16 i = 0; i < 2; ++i)
        SDL_memcpy(dra + 0xd1d4 + blueprints[i][0] * 6U, blueprints[i] + 1, 6);
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    dra[0xd4b8 + 4] = 32;
    dra[0xd4b9 + 4] = 47;
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    SotnBatRuntimeInput in = {
        .dra = actors->dra,
        .executable = executable,
        .food_collision = Map(),
        .camera_x = 100,
        .player = {.hp = 70, .x = 200 * 65536, .y = 150 * 65536, .priority = 148}};
    bool allocated = false;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestSubweapon(
              r, (SotnPlayerFactorySource){.x = 200 * 65536, .y = 150 * 65536, .priority = 148},
              SOTN_BIBLE_BLUEPRINT, SOTN_SUBWEAPON_BIBLE << 9, &allocated) &&
          allocated && SotnBatRuntime_Tick(r, character, in));
    SotnBatRuntimeChild *c = &r->children[16];
    CHECK(c->subweapon && c->initialized && c->kind == SOTN_BIBLE_ENTITY &&
          c->primitive_count == 1 && c->group == 7 && c->state.subweapon.attack.width == 6 &&
          r->subweapon_mask == (Uint64)1 << 16 && !c->state.subweapon.motion.bible.mode &&
          SotnBatRuntime_SubweaponValid(c, 16));
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          c->state.subweapon.motion.bible.mode == 1 && r->use_ticks == 4 &&
          r->factories[0].cursor.blueprint == SOTN_BIBLE_TRAIL_BLUEPRINT);
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in));
    SotnBatRuntimeChild *t = &r->children[47];
    CHECK(t->subweapon && t->initialized && t->kind == SOTN_BIBLE_TRAIL_ENTITY &&
          t->primitive_count == 1 && !t->group && !t->state.subweapon.attack.active &&
          SotnBatRuntime_SubweaponValid(t, 47) && !(r->subweapon_mask & ((Uint64)1 << 47)) &&
          SotnActors_CommitBatPrimitives(actors, r));
    // A camera move of four pixels: the book and its trails keep their screen position.
    SDL_memcpy(other, r, sizeof(*r));
    in.camera_x = 104;
    in.screen_shift_x = -4;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    in.camera_x = 100;
    in.screen_shift_x = 0;
    CHECK(SotnBatRuntime_Tick(other, character, in) &&
          c->state.subweapon.motion.x == other->children[16].state.subweapon.motion.x + 4 * 65536 &&
          t->state.subweapon.motion.trail.x ==
              other->children[47].state.subweapon.motion.trail.x + 4);
    // Room entry: FLAG_UNK_20000 keeps them; no room rebase (screen anchored), pending trail
    // factories are pruned.
    const Sint32 bx = c->state.subweapon.motion.x, tx = t->state.subweapon.motion.trail.x;
    in.entering = true;
    in.room_shift_x = 64;
    in.screen_shift_x = 64;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          c->kind == SOTN_BIBLE_ENTITY && t->kind == SOTN_BIBLE_TRAIL_ENTITY &&
          t->state.subweapon.motion.trail.x == tx && SotnBatRuntime_SubweaponValid(c, 16));
    CHECK(c->state.subweapon.motion.x != bx + 64 * 65536);
    in.entering = false;
    in.room_shift_x = in.screen_shift_x = 0;
    // Launch: the book loses KEEP_ALIVE and is culled once it leaves the screen window.
    c->state.subweapon.motion.bible.count = 0x12b;
    c->state.subweapon.motion.step = 3;
    c->state.subweapon.motion.bible.radius = 0x30;
    Uint16 ticks = 0;
    while (c->kind == SOTN_BIBLE_ENTITY && ticks++ < 60)
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    CHECK(!c->kind && ticks > 5 && ticks < 30);
    SotnCharacter_Destroy(character);
    SDL_free(other);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(dra);
    return true;
}
// EntitySubwpnCrashCross: afterimage reset before the allocation, birth, rise, factories, odd
// g_Timer widening, hold with clamped edges, spread and retirement.
static bool Beam(SotnDataView executable)
{
    Uint32 rolls = 0;
    SotnSubweaponInput in = {.executable = executable,
                             .properties = Roll,
                             .context = &rolls,
                             .screen_x = 40,
                             .screen_y = 20,
                             .crash = SOTN_GIANT_CROSS};
    const SotnSubweaponEntity fresh = {.x = (40 + 10) * 65536 + 0x1234,
                                       .y = 300 * 65536 + 0x5678,
                                       .id = SOTN_SUBWEAPON_CROSS,
                                       .priority = 148};
    SotnSubweaponEntity e = fresh;
    SotnSubweaponEvents ev = {0};
    Uint32 random = 9;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed && random == 9 && !rolls &&
          ev.reset_afterimages && !ev.upload_gradient);
    in.allocated = true;
    e = fresh;
    Uint32 expected = random;
    (void)SotnCombat_Random(&expected);
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && random == expected && rolls == 1 &&
          e.step == 1 && e.flags == 0x00800000U && e.y == (20 + 0x78) * 65536 + 0x5678 &&
          e.x == fresh.x && e.priority == 0xc2 && e.beam.width == 1 && e.beam.grey == 0x80 &&
          e.beam.draw_mode == 0x31 && e.beam.crash == SOTN_GIANT_CROSS && ev.upload_gradient &&
          ev.reset_afterimages && ev.sound == 0x6df && ev.sound_after == 0x636 &&
          ev.spell_use_ticks == 4 && !ev.factories && e.beam.x0 == 10 && e.beam.x1 == 11 &&
          e.beam.y0 == 0x78 && e.beam.y2 == 0x78 && !e.hitbox_width && !e.hitbox_height);
    // Rise: three pixels each way per update until the half height's low byte reaches 0x70.
    for (Uint16 tick = 1; tick <= 38; ++tick)
    {
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && random == expected &&
              ev.reset_afterimages && !ev.sound && e.beam.half_height == tick * 3 &&
              e.beam.height == tick * 6 && e.hitbox_height == tick * 3 && !e.hitbox_x &&
              e.beam.y0 == 0x78 - tick * 3 && e.beam.y2 == 0x78 + tick * 3 &&
              e.step == (tick == 38 ? 2 : 1) && ev.factories == (tick == 38 ? 2 : 0));
    }
    CHECK(ev.factory_packed[0] == SOTN_GIANT_CROSS_BLUEPRINT &&
          ev.factory_argument[0] == SOTN_GIANT_CROSS << 9 &&
          ev.factory_packed[1] == SOTN_CROSS_PARTICLES_BLUEPRINT && !ev.factory_argument[1] &&
          ev.factory_x == e.x && ev.factory_y == e.y);
    // Widening only on odd g_Timer frames, to a width of 45 (22 steps).
    Uint16 steps = 0;
    for (Uint32 frame = 0; e.step == 2 && frame < 100; ++frame)
    {
        const Sint16 before = e.beam.half_width;
        in.engine_timer = frame;
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
        CHECK(e.beam.half_width == before + (Sint16)(frame & 1U));
        steps += (Uint16)(frame & 1U);
        CHECK(e.hitbox_width == (Uint8)e.beam.half_width && e.beam.x0 == 10 - e.beam.half_width &&
              e.beam.x1 == e.beam.x0 + e.beam.width);
    }
    CHECK(steps == 22 && e.beam.width == 45 && e.beam.half_width == 22 && e.step == 3 &&
          e.beam.hold == 0x80);
    // Hold, then the clamped edges (0..32 around x10) on the update that starts the spread.
    for (Uint16 tick = 1; tick < 0x80; ++tick)
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 3 && !ev.sound &&
              e.beam.grey == 0x80);
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 4 && ev.sound == 0x62f &&
          e.beam.x0 == 0 && e.beam.x1 == 32 && e.hitbox_width == 16 && e.hitbox_x == 6 &&
          e.beam.grey == 0x84 && e.beam.draw_mode == 0x35 && e.beam.half_width == 22);
    // Spread: 3 * (|x - 128| + 128) / 112 = 6 pixels each way per update; the hitbox width is a
    // byte. It retires once both edges pass (-0x80, 0x180) without updating the hitbox.
    Uint16 spread = 0;
    while (!e.removed && spread < 100)
    {
        const SotnSubweaponEntity before = e;
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
        ++spread;
        if (e.removed)
        {
            CHECK(10 + e.beam.half_width > 0x180 && before.hitbox_width == before.hitbox_width &&
                  e.hitbox_width == before.hitbox_width);
            break;
        }
        CHECK(e.beam.half_width == before.beam.half_width + 6 &&
              e.beam.x0 == 10 - e.beam.half_width && e.beam.x1 == 10 + e.beam.half_width &&
              e.hitbox_width == (Uint8)e.beam.half_width && e.hitbox_x == 0 &&
              e.beam.grey == (before.beam.grey < 0xf8 ? before.beam.grey + 4 : before.beam.grey));
    }
    CHECK(e.removed && spread == 59 && e.beam.half_width == 22 + 59 * 6);
    // Right of centre the step uses the same distance; an unclamped hold end keeps both edges.
    e = fresh;
    e.x = (40 + 222) * 65536;
    e.step = 3;
    e.beam = (SotnCrossBeam){.half_width = 22,
                             .half_height = 0x72,
                             .width = 45,
                             .height = 228,
                             .hold = 1,
                             .grey = 0x80,
                             .draw_mode = 0x31};
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.beam.x0 == 200 && e.beam.x1 == 244 &&
          e.hitbox_width == 22 && !e.hitbox_x && e.hitbox_height == 0x72);
    // 3 * (94 + 128) = 666: five pixels (a divisor of 111 would give six).
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.beam.half_width == 27);
    e.x = (40 + 21) * 65536;
    e.step = 3;
    e.beam.half_width = 22;
    e.beam.hold = 1;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.beam.x0 == 0 && e.beam.x1 == 43 &&
          e.hitbox_width == 21 && !e.hitbox_x);
    e.x = (40 + 250) * 65536;
    e.step = 3;
    e.beam.half_width = 22;
    e.beam.hold = 1;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.beam.x0 == 228 && e.beam.x1 == 0xff &&
          e.hitbox_width == 13 && e.hitbox_x == -9);
    return true;
}
// Authored face A(-200,-1000) B(200,-1000) C(-200,1000) D(200,1000): even rows {A,B,D,C} face the
// camera, odd rows reverse it; rows store vectors 0, 1, 3, 2.
static Uint8 *CrossModel(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11200);
    if (!dra)
        return NULL;
    const Sint16 corners[4][3] = {
        {-200, -1000, -4}, {200, -1000, -4}, {-200, 1000, -4}, {200, 1000, -4}};
    for (Uint16 i = 0; i < 4; ++i)
        for (Uint16 k = 0; k < 3; ++k)
            Put16(dra + 0x10b34 + i * 8U + k * 2U, (Uint16)corners[i][k]);
    const Uint8 order[2][4] = {{0, 1, 3, 2}, {1, 0, 2, 3}};
    for (Uint16 row = 0; row < SOTN_GIANT_CROSS_ROWS; ++row)
    {
        for (Uint16 v = 0; v < 4; ++v)
        {
            const Uint32 address = 0x800b0b34U + order[row & 1U][v] * 8U;
            for (Uint16 b = 0; b < 4; ++b)
                dra[0x10cb4 + row * 16U + v * 4U + b] = (Uint8)(address >> (b * 8U));
        }
        Uint8 *face = dra + 0x10f94 + row * 5U;
        face[0] = 0x38;
        face[1] = 8;
        face[2] = face[3] = 16;
        face[4] = (Uint8)(((row % 13U) << 4) | (row % 16U));
    }
    for (Uint16 frame = 0; frame < 6; ++frame)
        for (Uint16 b = 0; b < 8; ++b)
            dra[0xd094 + frame * 8U + b] = (Uint8)(frame * 16U + b);
    return dra;
}
// EntityGiantSpinningCross: birth, rise/acceleration, orbit/spin, projected hitbox/priority,
// face order and NCLIP visibility (hidden faces keep their last words), retirement.
static bool GiantCross(SotnDataView executable)
{
    Uint8 *dra = CrossModel();
    CHECK(dra);
    Uint32 rolls = 0;
    SotnSubweaponInput in = {.executable = executable,
                             .dra = {dra, 0x11200},
                             .properties = Roll,
                             .context = &rolls,
                             .screen_x = 40,
                             .screen_y = 20,
                             .allocated = true};
    SotnSubweaponEntity e = {.x = (40 + 100) * 65536 + 7,
                             .y = 50 * 65536 + 9,
                             .id = SOTN_GIANT_CROSS,
                             .facing_left = true};
    SotnSubweaponEvents ev = {0};
    Uint32 random = 5, expected = 5;
    (void)SotnCombat_Random(&expected);
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && random == expected && rolls == 1 &&
          e.step == 1 && e.flags == 0x04800000U && !e.facing_left &&
          e.y == (20 + 0x160) * 65536 + 9 && e.cross.velocity_y == -0x60000 && !e.cross.angle &&
          e.cross.spin == 0x400 && !e.cross.visible && e.hitbox_width == 0xc &&
          e.hitbox_height == 0x50 && ev.sound == 0x661 && !ev.reset_afterimages);
    bool seen[2] = {false, false}, kept = false;
    Uint16 updates = 0;
    while (!e.removed && updates < 200)
    {
        const SotnSubweaponEntity before = e;
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && random == expected);
        ++updates;
        if (e.removed)
        {
            CHECK(Whole(before.y) - 20 < -0x40);
            break;
        }
        const Sint32 y = Whole(before.y) - 20;
        const Sint32 vy = before.cross.velocity_y - (y < 0 ? 0x8000 : 0);
        CHECK(e.cross.velocity_y == vy && e.y == before.y + vy &&
              e.cross.angle == (Sint16)(before.cross.angle + 0x60) &&
              e.cross.spin == (Sint16)(before.cross.spin + 0x60));
        SotnMathTrig t = {0};
        CHECK(SotnMath_Trig(executable, e.cross.angle, &t));
        const Sint16 sy = (Sint16)(Whole(e.y) - 20);
        const Sint32 transfer[3] = {Floor(-(t.cosine * 0x60), 12), sy - 0x80,
                                    Floor(t.sine * 0x60, 12) + 0x180};
        const Sint16 axis[3] = {0, e.cross.spin, 0x40}, centre[4][3] = {{0}};
        SotnProjectedPoint p[4] = {{0}};
        Sint32 z = 0;
        CHECK(SotnShieldStar_Project(executable, axis, transfer, 100, 120, 320, centre, p, &z));
        CHECK(e.hitbox_x == p[0].x - 100 && e.hitbox_y == p[0].y - sy &&
              e.cross.priority == (Uint16)(0xd0 - (z - 0x50)));
        const Sint16 corners[4][3] = {
            {-200, -1000, -4}, {200, -1000, -4}, {-200, 1000, -4}, {200, 1000, -4}};
        for (Uint16 row = 0; row < 2; ++row)
        {
            const Uint16 o[2][4] = {{0, 1, 2, 3}, {1, 0, 3, 2}};
            const Sint16 *c0 = corners[o[row][0]], *c1 = corners[o[row][1]],
                         *c2 = corners[o[row][2]], *c3 = corners[o[row][3]];
            const Sint16 v[4][3] = {{c0[0], c0[1], c0[2]},
                                    {c1[0], c1[1], c1[2]},
                                    {c2[0], c2[1], c2[2]},
                                    {c3[0], c3[1], c3[2]}};
            CHECK(SotnShieldStar_Project(executable, axis, transfer, 100, 120, 320, v, p, &z));
            const Sint64 nclip = (Sint64)p[0].x * p[1].y + (Sint64)p[1].x * p[2].y +
                                 (Sint64)p[2].x * p[0].y - (Sint64)p[0].x * p[2].y -
                                 (Sint64)p[1].x * p[0].y - (Sint64)p[2].x * p[1].y;
            const bool shown = (e.cross.visible >> row) & 1U;
            CHECK(shown == (nclip >= 0) && ((e.cross.visible >> (row + 2U)) & 1U) == shown);
            seen[row] |= shown;
            for (Uint16 k = 0; k < 4; ++k)
                CHECK(shown ? e.cross.x[row][k] == p[k].x && e.cross.y[row][k] == p[k].y
                            : e.cross.x[row][k] == before.cross.x[row][k] &&
                                  e.cross.y[row][k] == before.cross.y[row][k]);
            kept |= !shown && ((before.cross.visible >> row) & 1U);
        }
    }
    CHECK(e.removed && updates > 60 && updates < 120 && seen[0] && seen[1] && kept);
    Uint8 u[4] = {0}, v[4] = {0};
    Uint16 palette = 0;
    CHECK(SotnSubweaponEntity_CrossFace(in.dra, 4, u, v, &palette) && palette == 0x1a4 &&
          u[0] == 0x48 && v[0] == 0x18 && u[3] == 0x38 && v[3] == 8 && u[1] == 0x48 && v[1] == 8);
    CHECK(SotnSubweaponEntity_CrossFace(in.dra, 0, u, v, &palette) && palette == 0x1a0 &&
          u[0] == 0x38 && v[0] == 8 && u[3] == 0x48 && v[3] == 0x18);
    // Row12's kind 0xC0 (beyond 0x70) takes the default corners.
    CHECK(SotnSubweaponEntity_CrossFace(in.dra, 12, u, v, &palette) && palette == 0x1ac &&
          u[0] == 0x38 && v[0] == 8 && u[1] == 0x48 && v[2] == 0x18);
    CHECK(!SotnSubweaponEntity_CrossFace(in.dra, SOTN_GIANT_CROSS_ROWS, u, v, &palette) &&
          SotnSubweaponEntity_SparkFace(in.dra, 5, u, v) && u[0] == 0x50 && v[3] == 0x57 &&
          !SotnSubweaponEntity_SparkFace(in.dra, 6, u, v));
    e = (SotnSubweaponEntity){.id = SOTN_GIANT_CROSS, .step = 1, .y = (20 - 0x40) * 65536};
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && !e.removed);
    e = (SotnSubweaponEntity){.id = SOTN_GIANT_CROSS, .step = 1, .y = (20 - 0x41) * 65536};
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed);
    Put16(dra + 0x10cb4, 0x1234);
    e = (SotnSubweaponEntity){.id = SOTN_GIANT_CROSS, .step = 1, .y = 100 * 65536};
    CHECK(!SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    SDL_free(dra);
    return true;
}
// EntitySubwpnCrashCrossParticles: 192 updates, a spark every fourth update above eight, three
// rand calls per spark, rise, frame/size, hide and row reuse.
static bool Sparks(void)
{
    SotnSubweaponInput in = {.player_priority = 0x90, .allocated = true};
    SotnSubweaponEntity e = {.id = SOTN_CROSS_PARTICLES};
    SotnSubweaponEvents ev = {0};
    Uint32 random = 77, expected = 77;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 1 && e.flags == 0x04800000U &&
          e.sparks.timer == 192 && random == 77);
    Uint16 births = 0, hides = 0, maximum = 0;
    SotnCrossParticles before = e.sparks;
    for (Uint16 tick = 1; tick <= 192; ++tick)
    {
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
        if (tick == 192)
        {
            CHECK(e.removed);
            break;
        }
        CHECK(e.sparks.timer == 192 - tick);
        Uint16 active = 0;
        for (Uint16 i = 0; i < SOTN_CROSS_PARTICLE_ROWS; ++i)
        {
            const SotnCrossParticle *p = &e.sparks.rows[i], *o = &before.rows[i];
            active += p->active;
            if (p->active && !o->active)
            {
                const Uint16 roll = SotnCombat_Random(&expected) & 0x3f;
                const Uint8 x = (Uint8)(SotnCombat_Random(&expected) % 237 + 9),
                            y = (Uint8)(0xf0 - (SotnCombat_Random(&expected) & 0x20));
                CHECK(p->x == x && p->y == y && p->placed == y && p->shown == 0 && p->parity == 1 &&
                      !p->frame && p->born == 1 && p->visible && p->speed == (roll >> 2) + 4 &&
                      p->priority == 0x90 + roll - 0x20);
                ++births;
            }
            else if (o->active)
            {
                const Uint8 y = (Uint8)(o->y - o->speed);
                if (o->frame >= 6 || y < 24)
                {
                    CHECK(!p->active && !p->visible && p->y == y);
                    ++hides;
                }
                else
                    CHECK(p->active && p->visible && p->y == y && p->placed == y &&
                          p->shown == o->frame && p->parity == o->parity + 1 &&
                          p->frame == o->frame + (p->parity % 2 == 0));
            }
        }
        CHECK(random == expected);
        maximum = SDL_max(maximum, active);
        before = e.sparks;
    }
    CHECK(births == 45 && hides >= 40 && maximum >= 3 && maximum <= 5 &&
          !e.sparks.rows[maximum + 1].born);
    // A spark landing on line 24 stays; one below it hides.
    for (Uint16 below = 0; below < 2; ++below)
    {
        e = (SotnSubweaponEntity){.id = SOTN_CROSS_PARTICLES, .step = 1};
        e.sparks.timer = 7;
        e.sparks.rows[0] = (SotnCrossParticle){.active = 1,
                                               .born = 1,
                                               .x = 50,
                                               .y = (Uint8)(24 - below + 9),
                                               .speed = 9,
                                               .frame = 2,
                                               .visible = true};
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) &&
              e.sparks.rows[0].visible == !below && e.sparks.rows[0].active == !below &&
              e.sparks.rows[0].y == 24 - below);
    }
    e = (SotnSubweaponEntity){.id = SOTN_CROSS_PARTICLES, .step = 1};
    CHECK(!SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    return true;
}
// The cross through the shared runtime: blueprint6 beam, its factories, six giant crosses from
// one screen-anchored 24-update factory, the particles, chain counting and room cleanup.
static bool CrossRuntime(SotnDataView executable)
{
    Uint8 *dra = CrossModel();
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    SotnCharacter *character = NULL;
    CHECK(dra && actors && r);
    TestCharacter_Fill(dra);
    const Uint16 ids[2] = {SOTN_SUBWEAPON_CROSS, SOTN_GIANT_CROSS};
    for (Uint16 i = 0; i < 2; ++i)
    {
        Uint8 *def = dra + 0x4a00 + ids[i] * 20U;
        def[0] = i ? 40 : 5;
        def[2] = i ? 5 : 100;
        Put16(def + 4, 0x1000);
        def[6] = 1;
        def[7] = 0x20;
        def[8] = 4;
        def[10] = 0xdc;
        def[11] = SOTN_CROSS_BLUEPRINT;
        def[12] = 2;
        def[14] = 1;
        def[16] = i ? 0 : SOTN_GIANT_CROSS;
    }
    const Uint8 blueprints[3][7] = {
        {6, 8, 1, 0xc1, 0, 0x12, 0}, {7, 7, 6, 0xc1, 24, 0x12, 0}, {8, 9, 1, 0xc1, 0, 0x02, 0}};
    for (Uint16 i = 0; i < 3; ++i)
        SDL_memcpy(dra + 0xd1d4 + blueprints[i][0] * 6U, blueprints[i] + 1, 6);
    dra[0xd4b8 + 4] = 32;
    dra[0xd4b9 + 4] = 47;
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11200}, &character));
    actors->dra = (SotnDataView){dra, 0x11200};
    actors->entered = true;
    SotnBatRuntimeInput in = {
        .dra = actors->dra,
        .executable = executable,
        .food_collision = Map(),
        .camera_x = 100,
        .player = {.hp = 70, .x = 180 * 65536, .y = 150 * 65536, .priority = 148}};
    bool allocated = false;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestSubweapon(
              r, (SotnPlayerFactorySource){.x = 180 * 65536, .y = 150 * 65536, .priority = 148},
              SOTN_CROSS_BLUEPRINT, SOTN_SUBWEAPON_CROSS << 9, &allocated) &&
          allocated && SotnBatRuntime_Tick(r, character, in) &&
          SotnActors_CommitBatPrimitives(actors, r));
    SotnBatRuntimeChild *b = &r->children[16];
    CHECK(b->subweapon && b->initialized && b->kind == SOTN_CROSS_ENTITY &&
          b->primitive_count == 1 && b->group == 7 &&
          b->state.subweapon.motion.beam.crash == SOTN_GIANT_CROSS && r->cross_upload &&
          r->afterimage_reset && r->spell_use_ticks == 4 &&
          SotnBatRuntime_SubweaponCount(r, SOTN_SUBWEAPON_CROSS) == 1 &&
          SotnBatRuntime_SubweaponValid(b, 16) && Whole(b->state.subweapon.motion.y) == 0x78 &&
          Whole(b->state.subweapon.motion.x) == 180);
    // The rise lasts 38 updates; the giant cross factory then keeps its screen position while
    // the camera moves four pixels each update.
    Uint16 ticks = 0, crosses = 0, previous = 0;
    Sint32 screen_x[6] = {0};
    while (ticks++ < 400)
    {
        const bool moving = ticks > 20;
        in.camera_x = moving ? in.camera_x + 4 : in.camera_x;
        in.screen_shift_x = moving ? -4 : 0;
        in.engine_timer = ticks;
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
              !r->cross_upload && (b->kind != SOTN_CROSS_ENTITY || r->afterimage_reset));
        Uint16 live = 0;
        for (Uint16 i = 16; i < 32; ++i)
        {
            const SotnBatRuntimeChild *c = &r->children[i];
            if (c->kind != SOTN_GIANT_CROSS_ENTITY)
                continue;
            ++live;
            CHECK(SotnBatRuntime_SubweaponValid(c, i) && c->primitive_count == 46 &&
                  c->primitives[45] == c->primitives[0] + 45 && c->group >= 7 &&
                  c->state.subweapon.attack.power >= 40 && c->parameter < 6);
            if (c->state.subweapon.motion.cross.angle == 0x60)
            {
                CHECK(crosses < 6 && c->parameter == crosses);
                screen_x[crosses++] = Whole(c->state.subweapon.motion.x) - in.camera_x;
            }
        }
        if (live > previous && crosses > 1)
            CHECK(ticks - 39 >= (crosses - 1) * 24);
        previous = live;
    }
    CHECK(crosses == 6 && b->kind != SOTN_CROSS_ENTITY);
    for (Uint16 i = 0; i < 6; ++i)
        CHECK(screen_x[i] == 80);
    CHECK(SotnBatRuntime_SubweaponCount(r, SOTN_SUBWEAPON_CROSS) == 0);
    // Room entry prunes every cross entity and pending factory.
    CHECK(SotnBatRuntime_RequestSubweapon(
              r, (SotnPlayerFactorySource){.x = 180 * 65536, .y = 150 * 65536, .priority = 148},
              SOTN_CROSS_BLUEPRINT, SOTN_SUBWEAPON_CROSS << 9, &allocated) &&
          allocated && SotnBatRuntime_PruneSubweapons(r));
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        CHECK(!r->children[i].subweapon);
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        CHECK(!r->factories[i].cursor.entity_id);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(dra);
    return true;
}
// EntitySubwpnKnife: birth (lift/crouch/facing), eight-step flight with blade and trail, the
// wall bounce at the probe, the guard (hitFlags 2) bounce, the enemy hold, spin, fade, retirement.
static bool Dagger(SotnDataView executable)
{
    Uint32 rolls = 0;
    SotnSubweaponInput in = {.executable = executable,
                             .collision = Map(),
                             .properties = Roll,
                             .context = &rolls,
                             .screen_x = 30,
                             .screen_y = 10,
                             .player_priority = 148};
    const SotnSubweaponEntity fresh = {
        .x = 40 * 65536 + 0x1234, .y = 109 * 65536 + 0x5678, .id = SOTN_SUBWEAPON_DAGGER};
    SotnSubweaponEntity e = fresh;
    SotnSubweaponEvents ev = {0};
    Uint32 random = 3;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed && random == 3 && !rolls);
    in.allocated = true;
    for (Uint16 crouch = 0; crouch < 2; ++crouch)
    {
        e = fresh;
        in.player_status = crouch ? 0x20U : 0x8U;
        in.player_facing_left = crouch != 0;
        Uint32 expected = random;
        (void)SotnCombat_Random(&expected);
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && random == expected && e.step == 1 &&
              e.flags == 0x08800000U && e.facing_left == (crouch != 0) &&
              e.y == fresh.y - (crouch ? 0 : 9 * 65536) && e.x == fresh.x && e.hitbox_width == 4 &&
              e.hitbox_height == 2 && e.hitbox_x == 4 && !e.hitbox_y &&
              e.velocity_x == (crouch ? -0x80000 : 0x80000) && e.dagger.row_priority == 150 &&
              e.dagger.quad_mode == 0x0a && e.dagger.line_mode == 0x3b && ev.sound == 0x60c &&
              ev.use_ticks == 4 && !ev.factories);
    }
    // Flight right at y100 (row 6, open) towards the wall column at x128.
    in.player_status = 0;
    in.player_facing_left = false;
    e = fresh;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    Sint32 x = 40;
    for (Sint16 t = 1; x + 8 < 120; ++t)
    {
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 1 && !ev.sound);
        x += 8;
        const SotnDagger *d = &e.dagger;
        CHECK(Whole(e.x) == x && (e.x & 0xffff) == 0x1234 && d->timer == t && d->x[0] == x - 12 &&
              d->x[1] == x + 12 && d->x[2] == x - 12 && d->x[3] == x + 12 && d->y[0] == 96 &&
              d->y[2] == 104 && d->line_x[0] == x - 8 && d->line_x[1] == x - 12 * (t / 2) &&
              d->line_y[0] == 99 && d->line_y[1] == 99 && d->quad_mode == 0x02 &&
              d->line_mode == 0x33);
    }
    // The probe eight pixels ahead meets the wall at x120: bounce one pixel back per update.
    const SotnDagger flown = e.dagger;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 2 && Whole(e.x) == 120 &&
          e.velocity_x == -0x10000 && e.dagger.velocity_y == -0x28000 && e.dagger.timer == 64 &&
          ev.disable_attack && ev.factories == 1 &&
          ev.factory_packed[0] == SOTN_REBOUND_STONE_SPARKS && Whole(ev.factory_x) == 128 &&
          ev.factory_y == e.y && ev.sound == 0x6a4 && e.dagger.x[0] == flown.x[0] &&
          e.dagger.line_mode == 0x33);
    for (Sint16 timer = 63; timer > 0; --timer)
    {
        const SotnSubweaponEntity before = e;
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && !e.removed && !ev.sound &&
              e.dagger.timer == timer);
        CHECK(e.x == before.x - 0x10000 && e.y == before.y + before.dagger.velocity_y &&
              e.dagger.velocity_y == before.dagger.velocity_y + 0x2000 &&
              e.dagger.rotate == (Sint16)(before.dagger.rotate - 0x80) &&
              e.dagger.line_mode == 0x3b && e.dagger.quad_mode == (timer > 0x20 ? 0x02 : 0x37) &&
              e.dagger.grey == (timer > 0x20 ? 0 : 0x60 - 2 * (0x21 - timer)));
        const Sint16 base[4] = {0x800 - 0xd2, 0xd2, 0x800 + 0xd2, -0xd2};
        for (Uint16 v = 0; v < 4; ++v)
        {
            SotnMathTrig t = {0};
            CHECK(SotnMath_Trig(executable, (Sint16)(base[v] + e.dagger.rotate), &t));
            CHECK(e.dagger.x[v] == Whole(e.x) + Floor(t.cosine * 0xca0, 20) &&
                  e.dagger.y[v] == Whole(e.y) - Floor(-(t.sine * 0xca0), 20));
        }
    }
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed);
    // Left-facing spin mirrors the corner angles and turns the other way.
    e = fresh;
    e.step = 2;
    e.facing_left = true;
    e.dagger.timer = 10;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.dagger.rotate == 0x80);
    SotnMathTrig t0 = {0};
    CHECK(SotnMath_Trig(executable, 0xd2 + 0x80, &t0) &&
          e.dagger.x[0] == Whole(e.x) + Floor(t0.cosine * 0xca0, 20));
    // Facing left, the blade and trail mirror; hitFlags 3 is neither a hit nor a guard.
    e = fresh;
    in.player_facing_left = true;
    in.hit_flags = 3;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) &&
          SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 1 && Whole(e.x) == 32 &&
          e.dagger.x[0] == 44 && e.dagger.x[1] == 20 && e.dagger.line_x[0] == 40 &&
          e.dagger.line_x[1] == 32 && !ev.disable_attack);
    in.player_facing_left = false;
    in.hit_flags = 0;
    // A side-only wall (collision type 2, EFFECT_SIDE without EFFECT_SOLID) also bounces it.
    e = fresh;
    in.collision = Map();
    types[1] = 2;
    for (Uint16 i = 0; i < 12 && e.step < 2; ++i)
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    CHECK(e.step == 2 && Whole(e.x) == 120);
    types[1] = 3;
    // An enemy hit holds the blade four updates without moving or redrawing it.
    e = fresh;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) &&
          SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    const SotnSubweaponEntity held = e;
    in.hit_flags = 1;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 3 && e.dagger.timer == 4 &&
          ev.disable_attack && !ev.factories && e.x == held.x &&
          !SDL_memcmp(e.dagger.x, held.dagger.x, sizeof(held.dagger.x)) &&
          e.dagger.line_mode == 0x33);
    in.hit_flags = 0;
    for (Uint16 i = 0; i < 3; ++i)
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && !e.removed && e.x == held.x);
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed);
    // hitFlags 2 (a guard) bounces after the first one-pixel step.
    e = held;
    in.hit_flags = 2;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 2 &&
          e.x == held.x + 0x10000 && ev.factories == 1 && Whole(ev.factory_x) == Whole(e.x) + 8 &&
          ev.sound == 0x6a4);
    in.hit_flags = 0;
    e = (SotnSubweaponEntity){.id = SOTN_SUBWEAPON_DAGGER, .step = 4};
    CHECK(!SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    return true;
}
// The dagger through the shared runtime: blueprint3, two rows, its group, the wall bounce's
// tink factory and room cleanup.
static bool DaggerRuntime(SotnDataView executable)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    SotnCharacter *character = NULL;
    CHECK(dra && actors && r);
    TestCharacter_Fill(dra);
    Uint8 *def = dra + 0x4a00 + SOTN_SUBWEAPON_DAGGER * 20U;
    def[0] = 5;
    def[2] = 1;
    Put16(def + 4, 0x1040);
    def[6] = 3;
    def[7] = 4;
    def[8] = 4;
    def[10] = 0x5d;
    def[11] = SOTN_DAGGER_BLUEPRINT;
    def[12] = 2;
    def[14] = 0x84;
    const Uint8 blueprints[2][7] = {{3, 4, 1, 0xc1, 0, 0x22, 0}, {10, 5, 3, 0xc1, 3, 0x00, 0}};
    for (Uint16 i = 0; i < 2; ++i)
        SDL_memcpy(dra + 0xd1d4 + blueprints[i][0] * 6U, blueprints[i] + 1, 6);
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    dra[0xd4b8 + 4] = 32;
    dra[0xd4b9 + 4] = 47;
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    SotnBatRuntimeInput in = {
        .dra = actors->dra,
        .executable = executable,
        .food_collision = Map(),
        .camera_x = 20,
        .player = {.hp = 70, .x = 60 * 65536, .y = 109 * 65536, .priority = 148}};
    bool allocated = false;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestSubweapon(
              r, (SotnPlayerFactorySource){.x = 60 * 65536, .y = 109 * 65536, .priority = 148},
              SOTN_DAGGER_BLUEPRINT, SOTN_SUBWEAPON_DAGGER << 9, &allocated) &&
          allocated && SotnBatRuntime_Tick(r, character, in) &&
          SotnActors_CommitBatPrimitives(actors, r));
    SotnBatRuntimeChild *c = &r->children[16];
    CHECK(c->subweapon && c->initialized && c->kind == SOTN_DAGGER_ENTITY &&
          c->primitive_count == 2 && c->group == 7 && c->state.subweapon.attack.width == 4 &&
          c->state.subweapon.attack.offset_x == 4 && r->use_ticks == 4 &&
          r->subweapon_mask == (Uint64)1 << 16 && SotnBatRuntime_SubweaponValid(c, 16) &&
          SotnBatRuntime_SubweaponCount(r, SOTN_SUBWEAPON_DAGGER) == 1 &&
          actors->primitives[c->primitives[1]].ability_kind == SOTN_DAGGER_ENTITY);
    Uint16 ticks = 0;
    while (c->state.subweapon.motion.step == 1 && ticks++ < 20)
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    CHECK(c->state.subweapon.motion.step == 2 && !c->state.subweapon.attack.active &&
          r->factories[0].cursor.blueprint == SOTN_REBOUND_STONE_SPARKS &&
          Whole(r->factories[0].cursor.source.x) == 128);
    CHECK(SotnBatRuntime_PruneSubweapons(r) && !c->kind);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(dra);
    return true;
}
// EntitySubwpnThrownAxe: reversed facing, lift/crouch, spin swishes with fading volume, gravity
// cap, line-0x100 retirement, the hitFlags 2 bounce (hitFlags 1 passes through), the pose
// cycle, afterimage copies/tints and the exact corners.
static bool Axe(SotnDataView executable)
{
    Uint32 rolls = 0;
    SotnSubweaponInput in = {.executable = executable,
                             .properties = Roll,
                             .context = &rolls,
                             .screen_x = 30,
                             .screen_y = 10,
                             .player_priority = 148,
                             .game_timer = 2};
    const SotnSubweaponEntity fresh = {
        .x = 130 * 65536 + 0x1234, .y = 150 * 65536 + 0x5678, .id = SOTN_SUBWEAPON_AXE};
    SotnSubweaponEntity e = fresh;
    SotnSubweaponEvents ev = {0};
    Uint32 random = 7;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed && random == 7 && !rolls);
    in.allocated = true;
    for (Uint16 crouch = 0; crouch < 2; ++crouch)
    {
        e = fresh;
        in.player_status = crouch ? 0x20U : 0x8U;
        in.player_facing_left = crouch != 0;
        Uint32 expected = random;
        (void)SotnCombat_Random(&expected);
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && random == expected && e.step == 1 &&
              e.flags == 0x0c820000U && e.facing_left == !crouch &&
              e.velocity_x == (crouch ? -0x20000 : 0x20000) && e.axe.velocity_y == -0x60000 &&
              e.axe.angle == (crouch ? 0xc00 : 0x400) &&
              e.y == fresh.y - (crouch ? 0 : 12 * 65536) && e.hitbox_width == 12 &&
              e.hitbox_height == 12 && e.axe.volume == 0x7f && e.axe.row_priority == 146 &&
              ev.sound == 0x60c && !ev.volume_sound && ev.use_ticks == 4 && e.axe.visible == 0x03 &&
              e.axe.pose == 1 && e.axe.pose_timer == 2 && e.axe.palette == 0x1ac &&
              e.axe.active[0] && !e.axe.fresh[0] && e.axe.shade[0] == 1 && !e.axe.tint[0] &&
              !SDL_memcmp(e.axe.x[1], e.axe.x[0], sizeof(e.axe.x[0])));
    }
    // Flight (thrown facing right, so the axe faces left): exact corners, swishes every quarter
    // turn at a falling volume, the pose cycle and ten-update afterimages.
    in.player_status = 0;
    in.player_facing_left = false;
    e = fresh;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    Uint16 swishes = 0;
    Sint32 volume = 0x7f;
    for (Uint16 tick = 1; tick < 200 && !e.removed; ++tick)
    {
        const SotnSubweaponEntity before = e;
        in.game_timer = tick;
        in.hit_flags = tick == 5 ? 1 : 0; // an ordinary hit does not stop it
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 1);
        if (e.removed)
        {
            CHECK(Whole(before.y + SDL_min(before.axe.velocity_y + 0x4400, 0x80000)) - 10 > 0x100);
            break;
        }
        const Sint32 vy = SDL_min(before.axe.velocity_y + 0x4400, 0x80000);
        CHECK(e.axe.angle == (Sint16)(before.axe.angle - 0x80) && e.axe.velocity_y == vy &&
              e.y == before.y + vy && e.x == before.x + 0x20000 && !ev.disable_attack);
        if (!(e.axe.angle & 0x3ff))
        {
            CHECK(ev.sound == 0x60c && ev.volume_sound && ev.volume == volume);
            volume -= 8;
            ++swishes;
        }
        else
            CHECK(!ev.sound);
        CHECK(e.axe.volume == volume && e.axe.palette == 0x1ab + ((tick >> 1) & 1U) &&
              e.axe.pose_timer == (before.axe.pose_timer ? before.axe.pose_timer - 1 : 2));
        const Sint16 left[4] = {0x560, 0x2a0, 0xaa0, 0xd60};
        for (Uint16 v = 0; v < 4; ++v)
        {
            SotnMathTrig t = {0};
            CHECK(SotnMath_Trig(executable, (Sint16)(left[v] + e.axe.angle), &t));
            CHECK(e.axe.x[0][v] == Whole(e.x) - 30 + Floor(t.cosine * 336, 16) &&
                  e.axe.y[0][v] == Whole(e.y) - 10 - Floor(t.sine * 336, 16));
        }
        for (Uint16 i = 0; i < 4; ++i)
            if (e.axe.active[i] && !before.axe.active[i])
                CHECK(!SDL_memcmp(e.axe.x[i + 1U], e.axe.x[0], sizeof(e.axe.x[0])) &&
                      e.axe.tint[i] == 0 && ((e.axe.visible >> (i + 1U)) & 1U));
            else if (before.axe.active[i] && before.axe.shade[i] >= 10)
                CHECK(!e.axe.active[i] && !((e.axe.visible >> (i + 1U)) & 1U));
            else if (before.axe.active[i])
                CHECK(e.axe.tint[i] == before.axe.shade[i] &&
                      !SDL_memcmp(e.axe.x[i + 1U], before.axe.x[i + 1U], sizeof(e.axe.x[0])));
    }
    CHECK(e.removed && swishes >= 4);
    in.hit_flags = 0;
    // The volume clamps at zero (seven minus eight); a right-facing axe spins forwards.
    e = fresh;
    e.step = 1;
    e.axe.angle = 0x480;
    e.axe.volume = 7;
    e.facing_left = true;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && ev.volume == 7 && !e.axe.volume);
    e.facing_left = false;
    e.axe.angle = 0x100;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.axe.angle == 0x180);
    // Retirement is strictly below screen line 0x100; hitFlags 3 is not a bounce.
    e = fresh;
    e.step = 1;
    e.axe.velocity_y = -0x4400;
    e.y = (10 + 0x101) * 65536;
    in.hit_flags = 3;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed);
    e = fresh;
    e.step = 1;
    e.axe.velocity_y = -0x4400;
    e.y = (10 + 0x100) * 65536;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && !e.removed && e.step == 1 &&
          !ev.disable_attack);
    // A negative odd speed halves towards zero.
    e = fresh;
    e.step = 1;
    e.velocity_x = -0x20001;
    in.hit_flags = 2;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.velocity_x == 0x10000);
    // hitFlags 2: hitbox off, half speed back (truncated), lifted, spinning the other way.
    e = fresh;
    e.step = 1;
    e.velocity_x = 0x20001;
    e.axe.angle = 0x100;
    in.hit_flags = 2;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 2 && ev.disable_attack &&
          e.velocity_x == -0x10000 && e.axe.velocity_y == -0x30000);
    in.hit_flags = 0;
    const SotnSubweaponEntity bounced = e;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) &&
          e.axe.angle == (Sint16)(bounced.axe.angle - 0xc0) &&
          e.axe.velocity_y == -0x30000 + 0x2400 && e.x == bounced.x - 0x10000 && !ev.sound);
    e.facing_left = true;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) &&
          e.axe.angle == (Sint16)(bounced.axe.angle - 0xc0 + 0xc0));
    e.axe.velocity_y = 0x80000;
    e.y = (10 + 0x100 - 8) * 65536;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && !e.removed &&
          e.axe.velocity_y == 0x80000);
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed);
    e = (SotnSubweaponEntity){.id = SOTN_SUBWEAPON_AXE, .step = 3};
    CHECK(!SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    return true;
}
// The axe through the shared runtime: blueprint9, five rows, its group, the volume swish and
// FLAG_UNK_20000 room retention.
static bool AxeRuntime(SotnDataView executable)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    SotnCharacter *character = NULL;
    CHECK(dra && actors && r);
    TestCharacter_Fill(dra);
    Uint8 *def = dra + 0x4a00 + SOTN_SUBWEAPON_AXE * 20U;
    def[0] = 10;
    def[2] = 4;
    Put16(def + 4, 0x1040);
    def[6] = 2;
    def[7] = 32;
    def[8] = 4;
    def[10] = 0x5d;
    def[11] = SOTN_AXE_BLUEPRINT;
    def[12] = 2;
    def[14] = 0x83;
    const Uint8 blueprint[7] = {9, 10, 1, 0xc1, 0, 0x22, 0};
    SDL_memcpy(dra + 0xd1d4 + blueprint[0] * 6U, blueprint + 1, 6);
    dra[0xd4b8 + 4] = 32;
    dra[0xd4b9 + 4] = 47;
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    SotnBatRuntimeInput in = {
        .dra = actors->dra,
        .executable = executable,
        .food_collision = Map(),
        .camera_x = 20,
        .camera_y = 30,
        .player = {.hp = 70, .x = 60 * 65536, .y = 150 * 65536, .priority = 148}};
    bool allocated = false;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestSubweapon(
              r, (SotnPlayerFactorySource){.x = 60 * 65536, .y = 150 * 65536, .priority = 148},
              SOTN_AXE_BLUEPRINT, SOTN_SUBWEAPON_AXE << 9, &allocated) &&
          allocated && SotnBatRuntime_Tick(r, character, in) &&
          SotnActors_CommitBatPrimitives(actors, r));
    SotnBatRuntimeChild *c = &r->children[16];
    CHECK(c->subweapon && c->initialized && c->kind == SOTN_AXE_ENTITY && c->primitive_count == 5 &&
          c->group == 7 && c->state.subweapon.attack.width == 12 && r->use_ticks == 4 &&
          SotnBatRuntime_SubweaponValid(c, 16) &&
          SotnBatRuntime_SubweaponCount(r, SOTN_SUBWEAPON_AXE) == 1 &&
          actors->primitives[c->primitives[4]].ability_kind == SOTN_AXE_ENTITY);
    bool swished = false;
    for (Uint16 tick = 0; tick < 10 && !swished; ++tick)
    {
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        swished = r->sound_count == 1 && r->sounds[0] == 0x60c && (r->sound_volume_mask & 1U) &&
                  r->sound_volumes[0] == 0x7f;
    }
    CHECK(swished);
    // Room entry keeps the axe (FLAG_UNK_20000) and its rows.
    const Sint32 x = c->state.subweapon.motion.x;
    in.entering = true;
    in.room_shift_x = 64;
    in.screen_shift_x = 64;
    CHECK(SotnBatRuntime_PruneSubweapons(r) && c->kind == SOTN_AXE_ENTITY &&
          SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          c->kind == SOTN_AXE_ENTITY && c->state.subweapon.motion.x == x + 64 * 65536 &&
          SotnBatRuntime_SubweaponValid(c, 16));
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(dra);
    return true;
}
// EntitySubwpnHolyWater: birth, gravity, floor landing (no row allocation), the side wall's
// whole-pixel snap, the break, four counted flame factories and retirement.
static bool HolyWater(SotnDataView executable)
{
    Uint32 rolls = 0;
    SotnSubweaponInput in = {.executable = executable,
                             .collision = Map(),
                             .properties = Roll,
                             .context = &rolls,
                             .player_priority = 148,
                             .crash = SOTN_HOLY_WATER_FLAME,
                             .flame_counter = 0x1234};
    const SotnSubweaponEntity fresh = {
        .x = 40 * 65536 + 0x1234, .y = 92 * 65536 + 0x5678, .id = SOTN_SUBWEAPON_HOLY_WATER};
    SotnSubweaponEntity e = fresh;
    SotnSubweaponEvents ev = {0};
    Uint32 random = 3, expected = 3;
    (void)SotnCombat_Random(&expected);
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && random == expected && rolls == 1 &&
          e.step == 1 && e.flags == 0x08000000U && e.holy.shown && e.priority == 146 &&
          e.y == fresh.y + 8 * 65536 && e.velocity_x == 0x14000 && e.holy.velocity_y == -0x32000 &&
          e.holy.crash == SOTN_HOLY_WATER_FLAME && e.hitbox_width == 4 && e.hitbox_height == 4 &&
          ev.use_ticks == 4 && !ev.sound);
    e.facing_left = true;
    e.step = 0;
    e.y = fresh.y;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.velocity_x == -0x14000);
    e = fresh;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    expected = random;
    for (Uint16 tick = 0; tick < 80 && e.step == 1; ++tick)
    {
        const SotnSubweaponEntity before = e;
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && random == expected);
        if (e.step == 1)
            CHECK(e.y == before.y + before.holy.velocity_y &&
                  e.holy.velocity_y ==
                      before.holy.velocity_y + (before.holy.velocity_y < 0x40000 ? 0x3800 : 0) &&
                  e.x == before.x + 0x14000 && !ev.sound && !ev.factories);
    }
    // Landed on the floor's first solid pixel (rows 10.. from y160).
    SotnCollider at = {0}, above = {0};
    CHECK(e.step == 2 && SotnCollision_Query(&in.collision, Whole(e.x), Whole(e.y), &at) &&
          SotnCollision_Query(&in.collision, Whole(e.x), Whole(e.y) - 1, &above) &&
          (at.effects & 1U) && !(above.effects & 1U) && Whole(e.y) == 160 && (e.x & 0xffff) != 0 &&
          ev.sound == 0x69a && ev.factories == 1 &&
          ev.factory_packed[0] == SOTN_HOLY_WATER_GLASS_BLUEPRINT && !ev.factory_argument[0] &&
          !e.holy.shown && e.holy.timer == 16);
    // Flames on the updates whose timer is a multiple of four (16, 12, 8, 4).
    Uint16 flames = 0;
    for (Uint16 tick = 0; tick < 16; ++tick)
    {
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && !e.removed);
        if (ev.factories)
        {
            CHECK(tick % 4 == 0 && ev.flame_counter_step &&
                  ev.factory_packed[0] == (SOTN_HOLY_WATER_FLAME_BLUEPRINT | 0x12340000U) &&
                  ev.factory_argument[0] == SOTN_HOLY_WATER_FLAME << 9 && ev.factory_x == e.x);
            ++flames;
        }
    }
    CHECK(flames == 4 && e.step == 3 && e.holy.timer == 4);
    for (Uint16 tick = 0; tick < 3; ++tick)
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && !e.removed);
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed);
    // A side-only wall: the bottle snaps to its whole-pixel side and breaks.
    types[1] = 2;
    e = fresh;
    e.x = 118 * 65536 + 0x8000;
    e.y = 120 * 65536;
    e.step = 1;
    e.velocity_x = 0x14000;
    e.holy.velocity_y = -0x10000;
    for (Uint16 tick = 0; tick < 8 && e.step == 1; ++tick)
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    types[1] = 3;
    CHECK(e.step == 2 && !(e.x & 0xffff) && Whole(e.x) <= 124 && ev.sound == 0x69a);
    // A still bottle (no horizontal speed) never probes the side.
    e = fresh;
    e.step = 1;
    e.velocity_x = 0;
    e.y = 120 * 65536;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 1);
    // A side-only tile above the floor does not stop the landing (only EFFECT_SOLID does).
    static Uint8 strip_tiles[8 * 12 * 2];
    static Uint8 strip_types[3] = {0, 3, 2};
    for (Uint16 y = 0; y < 12; ++y)
        for (Uint16 x = 0; x < 8; ++x)
            Put16(strip_tiles + ((size_t)y * 8 + x) * 2U, y >= 10 ? 1 : y == 9 ? 2 : 0);
    in.collision = (SotnCollisionMap){
        {strip_tiles, sizeof(strip_tiles)}, {strip_types, sizeof(strip_types)}, 8, 12};
    e = fresh;
    e.step = 1;
    e.velocity_x = 0;
    e.y = 120 * 65536;
    e.holy.velocity_y = 0x40000;
    for (Uint16 tick = 0; tick < 12 && e.step == 1; ++tick)
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    CHECK(e.step == 2 && Whole(e.y) == 160 && ev.sound == 0x69a);
    in.collision = Map();
    e = (SotnSubweaponEntity){.id = SOTN_SUBWEAPON_HOLY_WATER, .step = 4};
    CHECK(!SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    return true;
}
// EntitySubwpnHolyWaterBreakGlass: the 48 birth rolls, the carriers' fall and retirement.
static bool HolyGlass(void)
{
    SotnSubweaponInput in = {.player_priority = 148, .allocated = true};
    SotnSubweaponEntity e = {
        .x = 70 * 65536 + 0x1234, .y = 150 * 65536 + 0x4321, .id = SOTN_HOLY_WATER_GLASS};
    SotnSubweaponEvents ev = {0};
    Uint32 random = 99, expected = 99;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 1 && e.flags == 0x08800000U &&
          e.glass.timer == 20 && e.glass.row_priority == 150);
    for (Uint16 i = 0; i < 8; ++i)
    {
        Sint32 vx = (Sint32)(SotnCombat_Random(&expected) & 0x3fffU) + 0x4000;
        const Sint32 vy = -((Sint32)(SotnCombat_Random(&expected) & 0x7fffU) * 2 + 0x28000);
        if (i & 1U)
            vx = -vx;
        CHECK(e.glass.x[i] == 70 * 65536 && e.glass.y[i] == 150 * 65536 &&
              e.glass.velocity_x[i] == vx && e.glass.velocity_y[i] == vy);
    }
    for (Uint16 i = 0; i < 8; ++i)
    {
        const Uint8 r = (Uint8)((SotnCombat_Random(&expected) & 0xfU) + 0x30),
                    b = (Uint8)((SotnCombat_Random(&expected) & 0x7fU) + 0x80),
                    g = (Uint8)((SotnCombat_Random(&expected) & 0x1fU) + 0x30);
        const Uint16 mode = (SotnCombat_Random(&expected) & 1U) ? 0x37 : 0x06;
        CHECK(e.glass.red[i] == r && e.glass.blue[i] == b && e.glass.green[i] == g &&
              e.glass.draw_mode[i] == mode);
    }
    CHECK(random == expected);
    for (Uint16 tick = 1; tick < 20; ++tick)
    {
        const SotnHolyGlass before = e.glass;
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && !e.removed &&
              e.glass.timer == 20 - tick && random == expected);
        for (Uint16 i = 0; i < 8; ++i)
            CHECK(e.glass.x[i] == before.x[i] + before.velocity_x[i] &&
                  e.glass.y[i] == before.y[i] + before.velocity_y[i] &&
                  e.glass.velocity_y[i] == before.velocity_y[i] + 0x4800);
    }
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed);
    e = (SotnSubweaponEntity){.id = SOTN_HOLY_WATER_GLASS};
    in.allocated = false;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed && random == expected);
    return true;
}
// EntitySubwpnHolyWaterFlame: colour rolls before the allocation, the smoke factory, the height
// roll, the exact wave/height, the alternating hitbox, the fade, the hide and retirement.
static bool HolyFlame(SotnDataView executable)
{
    Uint32 rolls = 0;
    SotnSubweaponInput in = {.executable = executable,
                             .properties = Roll,
                             .context = &rolls,
                             .screen_x = 20,
                             .screen_y = 30,
                             .player_priority = 148,
                             .game_timer = 13};
    const SotnSubweaponEntity fresh = {
        .x = 90 * 65536 + 0x1234, .y = 160 * 65536 + 0x4321, .id = SOTN_HOLY_WATER_FLAME};
    SotnSubweaponEntity e = fresh;
    SotnSubweaponEvents ev = {0};
    Uint32 random = 5, expected = 5;
    for (Uint16 i = 0; i < 3; ++i)
        (void)SotnCombat_Random(&expected);
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed && random == expected &&
          !rolls);
    in.allocated = true;
    e = fresh;
    expected = random;
    const Uint8 red = (Uint8)((SotnCombat_Random(&expected) & 0x1fU) + 0x40),
                green = (Uint8)((SotnCombat_Random(&expected) & 0x1fU) + 0x80),
                blue = (Uint8)((SotnCombat_Random(&expected) & 0x1fU) + 0x60);
    (void)SotnCombat_Random(&expected); // properties
    const Sint16 height = (Sint16)((SotnCombat_Random(&expected) & 0xfU) + 0x12);
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && random == expected && rolls == 1 &&
          e.step == 1 && e.flags == 0x08800000U && e.flame.red == red && e.flame.green == green &&
          e.flame.blue == blue && e.flame.height == height && e.flame.timer == 0x50 &&
          e.hitbox_width == 4 && e.flame.row_priority == 150 && ev.factories == 1 &&
          ev.factory_packed[0] == SOTN_HOLY_WATER_SMOKE && ev.factory_y == fresh.y - 10 * 65536 &&
          e.y == fresh.y && !e.flame.shown);
    for (Sint16 timer = 0x4f; timer >= 0x10; --timer)
    {
        const SotnSubweaponEntity before = e;
        in.game_timer = (Uint32)timer * 3U;
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && random == expected &&
              ev.attack_set && ev.attack_on == !(before.flame.timer & 3));
        if (timer < 0x11)
        {
            CHECK(e.removed);
            break;
        }
        SotnMathTrig t = {0};
        CHECK(SotnMath_Trig(executable, (before.flame.timer - 0x10) * 64 + 0xc00, &t));
        const Sint32 h = Floor(t.sine, 8) + height;
        CHECK(e.flame.timer == timer && e.flame.angle == (Sint16)(before.flame.angle + 0x180) &&
              e.flame.y[0] == 160 - h && e.flame.y[4] == 160 &&
              e.flame.y[2] == 30 + (160 - h - 30 + 130) / 2 && e.flame.x[0] == 90 &&
              e.flame.x[4] == 90 && e.flame.shown == (h >= 7) &&
              e.flame.frame == (in.game_timer & 7U) && e.hitbox_height == (Uint8)Floor(h, 1) &&
              e.hitbox_y == Floor(-h, 1));
        for (Uint16 i = 1; i < 4; ++i)
        {
            CHECK(SotnMath_Trig(executable, (Sint16)(before.flame.angle + (i << 10)), &t));
            CHECK(e.flame.x[i] == 90 + Floor(t.sine, 10));
        }
        const Uint8 fade = timer < 0x29 ? 5 : 0;
        CHECK(e.flame.red == (before.flame.red > 16 ? before.flame.red - fade : before.flame.red) &&
              e.flame.green ==
                  (before.flame.green > 16 ? before.flame.green - fade : before.flame.green));
    }
    CHECK(e.removed);
    return true;
}
// Holy water through the shared runtime: blueprint27 (no rows), the landing, blueprint59 glass
// in 32..63, four counted flames (4 rows, groups) and their profile7 smoke children.
static bool HolyRuntime(SotnDataView executable)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    SotnCharacter *character = NULL;
    CHECK(dra && actors && r);
    TestCharacter_Fill(dra);
    const Uint16 ids[2] = {SOTN_SUBWEAPON_HOLY_WATER, SOTN_HOLY_WATER_FLAME};
    for (Uint16 i = 0; i < 2; ++i)
    {
        Uint8 *def = dra + 0x4a00 + ids[i] * 20U;
        def[0] = i ? 8 : 1;
        def[2] = 3;
        Put16(def + 4, 0x20);
        def[6] = 3;
        def[7] = 32;
        def[8] = 8;
        def[10] = 0xbc;
        def[11] = SOTN_HOLY_WATER_BLUEPRINT;
        def[12] = 2;
        def[14] = 1;
        def[16] = i ? 0 : SOTN_HOLY_WATER_FLAME;
    }
    const Uint8 blueprints[4][7] = {{27, 22, 1, 0xc1, 0, 0x22, 0},
                                    {28, 23, 1, 0xc1, 0, 0x02, 0},
                                    {59, 40, 1, 0xc1, 0, 0x00, 0},
                                    {4, 5, 1, 0xc1, 0, 0x00, 0}};
    for (Uint16 i = 0; i < 4; ++i)
        SDL_memcpy(dra + 0xd1d4 + blueprints[i][0] * 6U, blueprints[i] + 1, 6);
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    dra[0xd4b8 + 4] = 32;
    dra[0xd4b9 + 4] = 47;
    // Profile7 smoke: eight tiles, behaviour3.
    Put16(dra + 0xdb98 + 7 * 4U, 0x0f00);
    Put16(dra + 0xdb98 + 7 * 4U + 2U, 0x800a);
    Uint8 *profile7 = dra + 0xf00;
    profile7[0] = 8;
    profile7[2] = 240;
    profile7[4] = profile7[5] = 1;
    Put16(profile7 + 6, 4);
    Put16(profile7 + 8, 0x73);
    Put16(profile7 + 10, 3);
    Put16(profile7 + 14, 0x880);
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    SotnBatRuntimeInput in = {
        .dra = actors->dra,
        .executable = executable,
        .food_collision = Map(),
        .player = {.hp = 70, .x = 60 * 65536, .y = 100 * 65536, .priority = 148}};
    bool allocated = false;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestSubweapon(
              r, (SotnPlayerFactorySource){.x = 60 * 65536, .y = 100 * 65536, .priority = 148},
              SOTN_HOLY_WATER_BLUEPRINT, SOTN_SUBWEAPON_HOLY_WATER << 9, &allocated) &&
          allocated && SotnBatRuntime_Tick(r, character, in) &&
          SotnActors_CommitBatPrimitives(actors, r));
    SotnBatRuntimeChild *b = &r->children[16];
    CHECK(b->subweapon && b->initialized && b->kind == SOTN_HOLY_WATER_ENTITY &&
          !b->primitive_count && b->group == 7 && b->state.subweapon.attack.width == 4 &&
          SotnBatRuntime_SubweaponValid(b, 16) &&
          SotnBatRuntime_SubweaponCount(r, SOTN_SUBWEAPON_HOLY_WATER) == 1 &&
          b->state.subweapon.motion.holy.crash == SOTN_HOLY_WATER_FLAME);
    Uint16 glass = 0, flames = 0, smoke = 0, live_flames = 0;
    for (Uint16 tick = 0; tick < 120; ++tick)
    {
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        Uint16 live = 0;
        for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        {
            const SotnBatRuntimeChild *c = &r->children[i];
            if (c->kind == SOTN_HOLY_WATER_GLASS_ENTITY)
            {
                CHECK(c->primitive_count == 16 && !c->group && SotnBatRuntime_SubweaponValid(c, i));
                glass = SDL_max(glass, 1);
            }
            if (c->kind == SOTN_HOLY_WATER_FLAME_ENTITY)
            {
                CHECK(c->primitive_count == 4 && c->group >= 7 && c->group <= 10 &&
                      SotnBatRuntime_SubweaponValid(c, i) && i < 32 && !(c->parameter & 0xff));
                ++live;
            }
            smoke +=
                c->kind == 5 && c->parameter == 0x700 && c->initialized && c->primitive_count == 9;
        }
        live_flames = SDL_max(live_flames, live);
        flames = (Uint16)r->holy_counter;
    }
    CHECK(glass && flames == 4 && live_flames == 4 && smoke >= 4 && !b->kind &&
          r->holy_counter == 4);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(dra);
    return true;
}
// EntityStopWatch: pause refusal/cancel, birth, hover lerp, rise/settle, the five eighty-update
// seconds with hops and digits, the exit, burst and retirement.
static bool Stopwatch(SotnDataView executable)
{
    SotnSubweaponInput in = {.executable = executable,
                             .properties = Roll,
                             .context = &(Uint32){0},
                             .screen_x = 20,
                             .screen_y = 10,
                             .player_priority = 148,
                             .player_x = 120 * 65536,
                             .player_y = 110 * 65536,
                             .enemies_paused = true};
    const SotnSubweaponEntity fresh = {
        .x = 120 * 65536, .y = 110 * 65536, .id = SOTN_SUBWEAPON_STOPWATCH};
    SotnSubweaponEntity e = fresh;
    SotnSubweaponEvents ev = {0};
    Uint32 random = 11;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed && ev.stopwatch_clear &&
          !ev.stopwatch_set && !ev.factories);
    in.enemies_paused = false;
    e = fresh;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed && !ev.stopwatch_set);
    in.allocated = true;
    e = fresh;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 1 && e.flags == 0x0c860000U &&
          ev.stopwatch_set && !ev.stopwatch_clear && ev.factories == 1 &&
          ev.factory_packed[0] == SOTN_STOPWATCH_CIRCLE_BLUEPRINT && ev.factory_x == fresh.x &&
          ev.sound == 0x6ad && e.stopwatch.palette0 == 0x17f && e.stopwatch.priority[0] == 149 &&
          e.stopwatch.priority[1] == 151 && e.stopwatch.draw_mode[0] == 0x30a &&
          e.stopwatch.u[0][0] == 0xa8 && e.stopwatch.u[0][1] == 0x98 &&
          e.stopwatch.v[0][2] == 0xd8 && random == 11);
    // The hover point is eight pixels behind and sixteen above; X closes a twelfth a update.
    CHECK(e.x == fresh.x + (-8 * 65536) / 12 && e.y == fresh.y + (-16 * 65536) / 4 &&
          e.stopwatch.scale == 4 && e.stopwatch.angle == 0x80);
    {
        SotnMathTrig t = {0};
        CHECK(SotnMath_Trig(executable, 0x80, &t));
        const Sint32 x = Whole(e.x) - 6, y = Whole(e.y), ox = 4 * 8 / 100, oy = 4 * 12 / 100;
        CHECK(e.stopwatch.x[0][0] == x + Floor(t.cosine * -ox - t.sine * -oy, 12) &&
              e.stopwatch.y[0][3] == y + Floor(t.sine * ox + t.cosine * oy, 12));
    }
    Uint16 rise = 0;
    while (e.step == 1 && rise < 40)
    {
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && !(e.stopwatch.draw_mode[0] & 8U));
        ++rise;
    }
    CHECK(rise == 18 && e.stopwatch.height == 18 * 0x18000);
    {
        // Fully grown: the dial spans (+-8, +-12) about its tilt.
        SotnMathTrig t = {0};
        CHECK(e.stopwatch.scale == 76 && SotnMath_Trig(executable, e.stopwatch.angle, &t));
        const Sint32 x = Whole(e.x) - 6, y = Whole(e.y) - Floor(e.stopwatch.height, 16),
                     ox = 76 * 8 / 100, oy = 76 * 12 / 100;
        const Sint32 sx[4] = {-ox, ox, -ox, ox}, sy[4] = {-oy, -oy, oy, oy};
        for (Uint16 v = 0; v < 4; ++v)
            CHECK(e.stopwatch.x[0][v] == x + Floor(t.cosine * sx[v] - t.sine * sy[v], 12) &&
                  e.stopwatch.y[0][v] == y + Floor(t.sine * sx[v] + t.cosine * sy[v], 12));
    }
    Uint16 settle = 0;
    while (e.step == 2 && settle < 40)
    {
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
        ++settle;
    }
    CHECK(settle == 11 && e.stopwatch.t == 5 && ev.sound == 0x6a1 &&
          (e.stopwatch.draw_mode[1] & 8U) && e.stopwatch.height > 0x100000 &&
          e.stopwatch.velocity > 0);
    // Seconds: eighty-one updates each; a hop rolls two rand calls whenever it lands.
    Uint32 expected = random;
    Sint16 seen[6] = {0};
    Uint16 hops = 0, ticks = 0;
    for (Uint16 tick = 0; tick < 600 && e.step == 3; ++tick)
    {
        const SotnSubweaponEntity before = e;
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
        const bool second = before.stopwatch.tick + 1 > 80;
        ticks += second;
        if (second)
            CHECK(ev.sound == 0x6a1 && !e.stopwatch.tick &&
                  e.stopwatch.t == before.stopwatch.t - 1);
        if (e.step != 3)
            break;
        if (before.stopwatch.height <= 0x100000)
        {
            const Sint32 v = second || before.stopwatch.bounce
                                 ? (Sint32)((SotnCombat_Random(&expected) % 64 + 0x200) << 8)
                                 : (Sint32)((SotnCombat_Random(&expected) % 128 + 0x100) << 8);
            const Sint16 a = before.stopwatch.angle >= 0
                                 ? (Sint16) - (Sint32)(SotnCombat_Random(&expected) % 64 + 0x40)
                                 : (Sint16)(SotnCombat_Random(&expected) % 64 + 0x40);
            CHECK(e.stopwatch.velocity == v - 0x4000 && e.stopwatch.angle == a &&
                  !e.stopwatch.bounce);
            ++hops;
        }
        CHECK(random == expected && (e.stopwatch.t < 5 || (e.stopwatch.draw_mode[1] & 8U)));
        if (e.stopwatch.t < 5 && e.stopwatch.t >= 0)
        {
            const Uint8 u = (Uint8)(e.stopwatch.t ? e.stopwatch.t * 8 + 0x18 : 0x68);
            CHECK(e.stopwatch.u[1][0] == u && e.stopwatch.u[1][1] == u + 6 &&
                  e.stopwatch.u[1][2] == u && e.stopwatch.u[1][3] == u + 6 &&
                  e.stopwatch.v[1][2] == 0x49 && !(e.stopwatch.draw_mode[1] & 8U) &&
                  (e.stopwatch.draw_mode[2] & 8U));
            seen[e.stopwatch.t] = 1;
            const Sint32 x = Whole(e.x) + 4, y = Whole(e.y) - 14, k = e.stopwatch.tick;
            if (k >= 8 && k < 0x48)
                CHECK(e.stopwatch.x[1][0] == x - 5 && e.stopwatch.x[1][1] == x + 5 &&
                      e.stopwatch.y[1][0] == y - 8 && e.stopwatch.y[1][2] == y + 8);
            else if (k >= 0x48)
                CHECK(e.stopwatch.x[1][0] == x - (0x50 - k) / 2 &&
                      e.stopwatch.y[1][0] == y - 8 - (k - 0x48));
            else
                CHECK(e.stopwatch.x[1][0] == x - k / 2 && e.stopwatch.y[1][0] == y - 8 - (7 - k));
        }
    }
    CHECK(e.step == 4 && ticks == 6 && hops > 20 && seen[0] && seen[4]);
    // Exit: unlock, re-anchor above the dial, then the burst four and fifteen updates later.
    const SotnSubweaponEntity exiting = e;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 5 &&
          !(e.flags & 0x00040000U) && e.stopwatch.priority[0] == 0xc2 &&
          e.stopwatch.draw_mode[0] == 0x102 && (e.stopwatch.draw_mode[1] & 8U) &&
          Whole(e.x) == Whole(exiting.x) - 6 &&
          Whole(e.y) == Whole(exiting.y) - Floor(exiting.stopwatch.height, 16) &&
          e.stopwatch.x[0][0] == Whole(e.x) - 8 && e.stopwatch.y[0][0] == Whole(e.y) - 12);
    for (Uint16 i = 1; i <= 4; ++i)
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == (i < 4 ? 5 : 6));
    CHECK(ev.sound == 0x6a4 && e.stopwatch.palette0 == 0x15f && (e.stopwatch.draw_mode[0] & 4U) &&
          e.stopwatch.x[0][0] == Whole(e.x) - 4 && e.stopwatch.y[0][0] == Whole(e.y) - 0x80);
    Uint16 burst = 0;
    while (e.step == 6 && burst < 20)
    {
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
        ++burst;
    }
    CHECK(burst == 11 && ev.factories == 1 && ev.factory_packed[0] == SOTN_STOPWATCH_BURST &&
          ev.factory_x == e.x && e.stopwatch.x[0][0] == Whole(e.x) - 1);
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed && ev.stopwatch_clear);
    // A pause while hovering jumps to the exit and clears the freeze every update.
    e = fresh;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) &&
          SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    in.enemies_paused = true;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 5 && ev.stopwatch_clear);
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 5 && ev.stopwatch_clear);
    // A pause while counting down also jumps to the exit.
    in.enemies_paused = false;
    e = fresh;
    for (Uint16 i = 0; i < 40 && e.step != 3; ++i)
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    in.enemies_paused = true;
    CHECK(e.step == 3 && SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 5);
    // Facing follows the player once the watch has crossed behind.
    in.enemies_paused = false;
    e = fresh;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && !e.facing_left);
    in.player_facing_left = true;
    for (Uint16 i = 0; i < 30 && !e.facing_left; ++i)
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    CHECK(e.facing_left && e.stopwatch.u[0][0] == 0x98 && e.stopwatch.u[0][1] == 0xa8);
    return true;
}
// EntityStopWatchExpandingCircle: UVs, growth by 0x18 and the thirty-update life.
static bool StopwatchCircle(SotnDataView executable)
{
    SotnSubweaponInput in = {
        .executable = executable, .screen_x = 20, .screen_y = 10, .allocated = true};
    SotnSubweaponEntity e = {.x = 120 * 65536, .y = 90 * 65536, .id = SOTN_STOPWATCH_CIRCLE};
    SotnSubweaponEvents ev = {0};
    Uint32 random = 1;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 1 && e.flags == 0x0c820000U &&
          e.priority == 0xc2 && e.circle.size == 0x20);
    for (Uint16 i = 0; i < 16; ++i)
    {
        SotnMathTrig a = {0}, b = {0};
        CHECK(SotnMath_Trig(executable, i << 8, &a) && SotnMath_Trig(executable, (i + 1) << 8, &b));
        CHECK(e.circle.u[i][0] == (Uint8)(Floor(a.sine * 32, 12) + 0x20) &&
              e.circle.v[i][1] == (Uint8)(-Floor(b.cosine * 32, 12) + 0xdf) &&
              e.circle.x[i][0] == 100 + Floor(a.sine * 0x20, 12) &&
              e.circle.y[i][1] == 80 - Floor(b.cosine * 0x20, 12) && e.circle.x[i][2] == 100 &&
              e.circle.y[i][3] == 80);
    }
    for (Uint16 tick = 1; tick <= 30; ++tick)
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && !e.removed &&
              e.circle.size == 0x20 + tick * 0x18 && e.circle.timer == tick);
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed && random == 1);
    return true;
}
// The stopwatch through the shared runtime: blueprint71, the freeze flag, its circle (blueprint75),
// shoves while player-locked, the pause gate and the Fairy cleanup's flag clear.
static bool StopwatchRuntime(SotnDataView executable)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    SotnCharacter *character = NULL;
    CHECK(dra && actors && r);
    TestCharacter_Fill(dra);
    Uint8 *def = dra + 0x4a00 + SOTN_SUBWEAPON_STOPWATCH * 20U;
    def[2] = 20;
    def[6] = 1;
    def[10] = 0xb8;
    def[11] = SOTN_STOPWATCH_BLUEPRINT;
    const Uint8 blueprints[2][7] = {{71, 41, 1, 0xc1, 0, 0x02, 0}, {75, 42, 1, 0xc1, 0, 0x00, 0}};
    for (Uint16 i = 0; i < 2; ++i)
        SDL_memcpy(dra + 0xd1d4 + blueprints[i][0] * 6U, blueprints[i] + 1, 6);
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    dra[0xd4b8 + 4] = 32;
    dra[0xd4b9 + 4] = 47;
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    SotnBatRuntimeInput in = {
        .dra = actors->dra,
        .executable = executable,
        .food_collision = Map(),
        .player = {.hp = 70, .x = 120 * 65536, .y = 110 * 65536, .priority = 148}};
    bool allocated = false;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestSubweapon(
              r, (SotnPlayerFactorySource){.x = 120 * 65536, .y = 110 * 65536, .priority = 148},
              SOTN_STOPWATCH_BLUEPRINT, SOTN_SUBWEAPON_STOPWATCH << 9, &allocated) &&
          allocated && SotnBatRuntime_Tick(r, character, in) &&
          SotnActors_CommitBatPrimitives(actors, r));
    SotnBatRuntimeChild *w = &r->children[16];
    CHECK(w->subweapon && w->initialized && w->kind == SOTN_STOPWATCH_ENTITY &&
          w->primitive_count == 3 && !w->group && !w->state.subweapon.attack.active &&
          r->stopwatch && SotnBatRuntime_SubweaponValid(w, 16) &&
          SotnBatRuntime_SubweaponCount(r, SOTN_SUBWEAPON_STOPWATCH) == 1 && r->sound_count == 1 &&
          r->sounds[0] == 0x6ad);
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
    SotnBatRuntimeChild *c = &r->children[47];
    CHECK(c->kind == SOTN_STOPWATCH_CIRCLE_ENTITY && c->initialized && c->primitive_count == 16 &&
          !c->group && SotnBatRuntime_SubweaponValid(c, 47) && r->stopwatch);
    // A post-stage shove moves the player-locked watch.
    const Sint32 x = w->state.subweapon.motion.x;
    CHECK(SotnBatRuntime_PlayerShove(r, 3 * 65536, 0) &&
          w->state.subweapon.motion.x == x + 3 * 65536);
    // Room entry keeps both (FLAG_UNK_20000).
    CHECK(SotnBatRuntime_PruneSubweapons(r) && w->kind == SOTN_STOPWATCH_ENTITY &&
          c->kind == SOTN_STOPWATCH_CIRCLE_ENTITY);
    // pauseEnemies clears the freeze and sends the watch to its exit.
    in.stopwatch_paused = true;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          !r->stopwatch && w->state.subweapon.motion.step == 5);
    // The Fairy cleanup clears a freeze while a watch lives.
    r->stopwatch = true;
    CHECK(SotnActors_ClearPlayerRuntime(actors, r) && !r->stopwatch && !w->kind);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(dra);
    return true;
}
// func_80123F78: the BP_120 tint. D_800973FC == 0 ends it (even at birth); the birth needs the
// normal animSet, copies the frame/facing/priority-2; g3 then fades 0x80 -> 3 over 125 updates.
static bool StopwatchTint(void)
{
    Uint32 random = 0x77;
    SotnSubweaponEvents ev = {0};
    SotnSubweaponInput in = {.stopwatch = true,
                             .player_normal_set = true,
                             .player_sprite = 0x8023,
                             .player_draw_flags = 2,
                             .player_priority = 148,
                             .player_facing_left = true};
    SotnSubweaponEntity e = {.x = 7 * 65536, .y = 9 * 65536, .id = SOTN_STOPWATCH_TINT};
    CHECK(SotnSubweaponEntity_Known(SOTN_STOPWATCH_TINT) &&
          !SotnSubweaponEntity_Rows(SOTN_STOPWATCH_TINT));
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && !e.removed && e.step == 1 &&
          e.flags == 0x0c030000U && e.tint.sprite == 0x8023 && e.priority == 146 && e.facing_left &&
          e.tint.grey == 0x80 && e.tint.color_enabled && random == 0x77 && !ev.sound &&
          !ev.factories && e.x == 7 * 65536 && e.tint.draw_flags == 2);
    // The frame and copied draw flags stay the birth ones.
    in.player_sprite = 0x44;
    in.player_draw_flags = 4;
    in.player_facing_left = false;
    Uint16 updates = 0;
    while (!e.removed)
    {
        CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev));
        ++updates;
        if (!e.removed)
            CHECK(e.tint.grey == 0x80 - updates && e.tint.sprite == 0x8023 && e.facing_left &&
                  e.tint.color_enabled && e.tint.draw_flags == 2);
    }
    CHECK(updates == 125 && !e.tint.color_enabled && random == 0x77);
    // Another animSet (Bat/Wolf/Axe Armor) retires the birth after its flags.
    in.player_normal_set = false;
    e = (SotnSubweaponEntity){.id = SOTN_STOPWATCH_TINT};
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed && !e.step &&
          e.flags == 0x0c030000U && !e.tint.color_enabled);
    // The freeze ending retires it at once, at birth too (no flags written).
    in.player_normal_set = true;
    e = (SotnSubweaponEntity){.id = SOTN_STOPWATCH_TINT};
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.step == 1);
    in.stopwatch = false;
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed && !e.tint.color_enabled &&
          e.tint.grey == 0x80);
    e = (SotnSubweaponEntity){.id = SOTN_STOPWATCH_TINT};
    CHECK(SotnSubweaponEntity_Tick(&e, in, &random, &ev) && e.removed && !e.flags);
    // Invalid cursors fail without mutation.
    in.stopwatch = true;
    e = (SotnSubweaponEntity){.id = SOTN_STOPWATCH_TINT, .step = 1, .tint = {.grey = 3}};
    const SotnSubweaponEntity kept = e;
    CHECK(!SotnSubweaponEntity_Tick(&e, in, &random, &ev) && !SDL_memcmp(&e, &kept, sizeof(e)));
    e = (SotnSubweaponEntity){.id = SOTN_STOPWATCH_TINT, .step = 2, .tint = {.grey = 9}};
    CHECK(!SotnSubweaponEntity_Tick(&e, in, &random, &ev));
    return true;
}
static bool StopwatchTintRuntime(SotnDataView executable)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    SotnCharacter *character = NULL;
    CHECK(dra && actors && r);
    TestCharacter_Fill(dra);
    const Uint8 blueprint[6] = {64, 1, 0xc1, 0, 0x00, 0};
    SDL_memcpy(dra + 0xd1d4 + SOTN_STOPWATCH_TINT_BLUEPRINT * 6U, blueprint, 6);
    dra[0xd4b8] = 32;
    dra[0xd4b9] = 63;
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    SotnBatRuntimeInput in = {.dra = actors->dra,
                              .executable = executable,
                              .food_collision = Map(),
                              .player = {.hp = 70,
                                         .x = 120 * 65536,
                                         .y = 110 * 65536,
                                         .priority = 148,
                                         .facing_left = true,
                                         .animation_bank = 1,
                                         .animation = {.sprite = 0x31}}};
    r->stopwatch = true;
    bool allocated = false;
    CHECK(!SotnBatRuntime_RequestSubweapon(r, (SotnPlayerFactorySource){.priority = 148},
                                           SOTN_STOPWATCH_TINT_BLUEPRINT, 1, &allocated));
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_RequestSubweapon(
              r,
              (SotnPlayerFactorySource){
                  .x = 120 * 65536, .y = 110 * 65536, .priority = 148, .facing_left = true},
              SOTN_STOPWATCH_TINT_BLUEPRINT, 0, &allocated) &&
          allocated && SotnBatRuntime_Tick(r, character, in) &&
          SotnActors_CommitBatPrimitives(actors, r));
    SotnBatRuntimeChild *t = &r->children[47]; // Kind0: slots 32..63 from the top.
    const SotnSubweaponEntity *m = &t->state.subweapon.motion;
    CHECK(t->subweapon && t->initialized && t->kind == SOTN_STOPWATCH_TINT_ENTITY &&
          !t->primitive_count && !t->group && !t->state.subweapon.attack.active &&
          m->tint.sprite == 0x31 && m->priority == 146 && m->facing_left && m->x == 120 * 65536 &&
          SotnBatRuntime_SubweaponValid(t, 47) && !r->sound_count);
    // FLAG_UNK_10000: frozen updates fade it; FLAG_UNK_20000 keeps it across rooms.
    in.freeze = 1;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          m->tint.grey == 0x7f);
    in.freeze = 0;
    CHECK(SotnBatRuntime_PruneSubweapons(r) && t->kind == SOTN_STOPWATCH_TINT_ENTITY);
    // The freeze ending retires it.
    r->stopwatch = false;
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
          SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r) &&
          !t->kind);
    SotnCharacter_Destroy(character);
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
    // Authored SDK matrix pairs for the giant cross (sine, cosine of angle/4096 turns).
    for (Uint16 i = 0; i < 4096; ++i)
    {
        const double turn = (double)i * SDL_PI_D / 2048.0;
        Put16(bytes + 0x1ea10 + i * 4U, (Uint16)(Sint16)SDL_lround(SDL_sin(turn) * 4096.0));
        Put16(bytes + 0x1ea12 + i * 4U, (Uint16)(Sint16)SDL_lround(SDL_cos(turn) * 4096.0));
    }
    const bool ok =
        Contiguous() && Component((SotnDataView){bytes, 0x23212}) &&
        Stone((SotnDataView){bytes, 0x23212}) && Agunea((SotnDataView){bytes, 0x23212}) &&
        Bolt((SotnDataView){bytes, 0x23212}) && AguneaRuntime((SotnDataView){bytes, 0x23212}) &&
        Bible((SotnDataView){bytes, 0x23212}) && BibleRuntime((SotnDataView){bytes, 0x23212}) &&
        Runtime((SotnDataView){bytes, 0x23212}) && Beam((SotnDataView){bytes, 0x23212}) &&
        Sparks() && GiantCross((SotnDataView){bytes, 0x23212}) &&
        CrossRuntime((SotnDataView){bytes, 0x23212}) && Dagger((SotnDataView){bytes, 0x23212}) &&
        DaggerRuntime((SotnDataView){bytes, 0x23212}) && Axe((SotnDataView){bytes, 0x23212}) &&
        AxeRuntime((SotnDataView){bytes, 0x23212}) && HolyWater((SotnDataView){bytes, 0x23212}) &&
        HolyGlass() && HolyFlame((SotnDataView){bytes, 0x23212}) &&
        HolyRuntime((SotnDataView){bytes, 0x23212}) && Stopwatch((SotnDataView){bytes, 0x23212}) &&
        StopwatchCircle((SotnDataView){bytes, 0x23212}) &&
        StopwatchRuntime((SotnDataView){bytes, 0x23212}) && StopwatchTint() &&
        StopwatchTintRuntime((SotnDataView){bytes, 0x23212});
    SDL_free(bytes);
    if (!ok)
        return 1;
    SDL_Log(
        "SUBWEAPON_ENTITIES: PASS Vibhuti birth RNG/velocities, landing, wall, hitbox cycle, "
        "hit/expiry hiding, contiguous rows; Rebound Stone birth order, embed hold, flight, "
        "fifteen reflections with tinks, fade-out; runtime factory/group/power/mask/freeze/prune; "
        "Agunea birth/flight/binding/strikes/hearts/exits, lightning growth/flash/fade, runtime "
        "hit target/raw parent/screen rows/blink hold/prune; Bible birth/reveal/exact orbit/"
        "swing/launch/fall, trail fade, runtime trails/screen anchoring/room retention/cull; "
        "cross beam afterimage/birth/rise/factories/odd widening/clamped hold/spread/cull, giant "
        "cross rise/orbit/projected hitbox/priority/face order/NCLIP, particles' spark life, "
        "runtime six screen-anchored crosses/contiguous rows/chain/prune; dagger birth/flight/"
        "wall and guard bounces/enemy hold/spin/fade, runtime rows/tink factory/prune; axe "
        "facing/lift/swish volume/gravity/retirement/bounce/afterimages, runtime rows/volume/"
        "room retention; holy water landing/wall/break/flame factories, glass rolls/fall, flame "
        "rolls/wave/hitbox/fade, runtime rowless bottle/glass/flames/smoke; stopwatch pause/"
        "birth/hover/rise/seconds/hops/digits/exit/burst, circle growth, runtime freeze flag/"
        "circle/shove/room retention/pause/Fairy clear");
    return 0;
}
