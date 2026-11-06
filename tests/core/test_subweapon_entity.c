// EntitySubwpnThrownVibhuti/EntitySubwpnReboundStone (dra/84B88.c) and their shared-runtime owner
// with authored tables only.
#include "actors_internal.h"
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
        Uint32 roll = held;
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
            const Sint32 a = angle + turn + 0x180 - (Sint32)((SotnCombat_Random(&roll) & 3U) << 8);
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
              e.bolt.x[2][2] == x2 && e.bolt.y[2][2] == y2 && again == roll &&
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
    const bool ok =
        Contiguous() && Component((SotnDataView){bytes, 0x23212}) &&
        Stone((SotnDataView){bytes, 0x23212}) && Agunea((SotnDataView){bytes, 0x23212}) &&
        Bolt((SotnDataView){bytes, 0x23212}) && AguneaRuntime((SotnDataView){bytes, 0x23212}) &&
        Runtime((SotnDataView){bytes, 0x23212});
    SDL_free(bytes);
    if (!ok)
        return 1;
    SDL_Log(
        "SUBWEAPON_ENTITIES: PASS Vibhuti birth RNG/velocities, landing, wall, hitbox cycle, "
        "hit/expiry hiding, contiguous rows; Rebound Stone birth order, embed hold, flight, "
        "fifteen reflections with tinks, fade-out; runtime factory/group/power/mask/freeze/prune; "
        "Agunea birth/flight/binding/strikes/hearts/exits, lightning growth/flash/fade, runtime "
        "hit target/raw parent/screen rows/blink hold/prune");
    return 0;
}
