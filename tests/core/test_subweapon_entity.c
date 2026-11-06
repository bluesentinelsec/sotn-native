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
              events.sound == 0x60c && events.use_ticks == 4 && !events.factory &&
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
          !events.factory);
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
        if (before.step == 1 && events.factory)
        {
            ++bounces;
            const SotnReboundLine *l = &e.stone.lines[e.stone.bounces];
            CHECK(e.stone.bounces == before.stone.bounces + 1 &&
                  events.factory_packed == SOTN_REBOUND_STONE_SPARKS &&
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
                    Stone((SotnDataView){bytes, 0x23212}) &&
                    Runtime((SotnDataView){bytes, 0x23212});
    SDL_free(bytes);
    if (!ok)
        return 1;
    SDL_Log(
        "SUBWEAPON_ENTITIES: PASS Vibhuti birth RNG/velocities, landing, wall, hitbox cycle, "
        "hit/expiry hiding, contiguous rows; Rebound Stone birth order, embed hold, flight, "
        "fifteen reflections with tinks, fade-out; runtime factory/group/power/mask/freeze/prune");
    return 0;
}
