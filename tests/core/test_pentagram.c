#include "actors_internal.h"
#include "boss_projection.h"
#include "character_fixture.h"
#include <sotn/combat.h>
#include <sotn/pentagram.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Sint16 value)
{
    p[0] = (Uint8)(Uint16)value;
    p[1] = (Uint8)((Uint16)value >> 8);
}
static bool Projection(void)
{
    Uint8 *exe = SDL_calloc(1, 0x22a10);
    CHECK(exe);
    // Authored cardinal matrix pairs; original SDK lookup data is never compiled in.
    const Sint16 sine[4] = {0, 4096, 0, -4096}, cosine[4] = {4096, 0, -4096, 0};
    for (Uint16 i = 0; i < 4; ++i)
    {
        Put16(exe + 0x1ea10 + (Uint32)i * 4096U, sine[i]);
        Put16(exe + 0x1ea12 + (Uint32)i * 4096U, cosine[i]);
    }
    const SotnDataView image = {exe, 0x22a10};
    SotnTriangleVertex v[3] = {{-8, -8, 0}, {8, -8, 0}, {-8, 8, 0}};
    SotnTriangleTransform t = {.depth = 512, .anchor_x = 128, .anchor_y = 104};
    SotnTriangleProjection p = {0};
    CHECK(SotnTriangle_Project(image, t, v, &p) && p.point[0].x == 120 && p.point[0].y == 96 &&
          p.point[1].x == 136 && p.point[2].y == 112 && p.winding == 256 &&
          p.ordering_depth == 127);
    // AVSZ3 uses341, so raw depth512 gives127 rather than a rounded average128.
    const SotnTriangleVertex swap = v[1];
    v[1] = v[2];
    v[2] = swap;
    CHECK(SotnTriangle_Project(image, t, v, &p) && p.winding == -256 && p.ordering_depth == 127 &&
          p.point[1].x == 120 && p.point[1].y == 112);
    v[1] = v[2];
    v[2] = (SotnTriangleVertex){-8, 8, 0};
    t.depth = 128;
    // The actual GTE divider saturates near the camera; ordinary floating division would differ.
    CHECK(SotnTriangle_Project(image, t, v, &p) && p.point[0].x == 112 && p.point[0].y == 88 &&
          p.point[1].x == 143 && p.ordering_depth == 31);
    t.depth = 512;
    t.z = 1024;
    CHECK(SotnTriangle_Project(image, t, v, &p) && p.point[0].x == 136 && p.point[0].y == 96 &&
          p.winding == 256);
    v[2] = v[0];
    CHECK(SotnTriangle_Project(image, t, v, &p) && !p.winding && !p.ordering_depth);
    const SotnTriangleProjection saved = p;
    t.depth = 1281;
    CHECK(!SotnTriangle_Project(image, t, v, &p) && !SDL_memcmp(&p, &saved, sizeof(p)));
    t.depth = 512;
    v[0].x = 33;
    CHECK(!SotnTriangle_Project(image, t, v, &p) && !SDL_memcmp(&p, &saved, sizeof(p)));
    v[0].x = -8;
    CHECK(!SotnTriangle_Project((SotnDataView){0}, t, v, &p) && !SDL_memcmp(&p, &saved, sizeof(p)));
    CHECK(!SotnTriangle_Project(image, t, NULL, &p));
    CHECK(!SotnTriangle_Project(image, t, v, NULL));
    Put16(exe + 0x1ea12, 4097);
    CHECK(!SotnTriangle_Project(image, t, v, &p) && !SDL_memcmp(&p, &saved, sizeof(p)));
    SDL_free(exe);
    return true;
}
static Sint16 AuthoredSine(Uint32 phase)
{
    phase &= 4095U;
    return phase < 1024   ? (Sint16)(phase * 4U)
           : phase < 3072 ? (Sint16)(8192 - (Sint32)phase * 4)
                          : (Sint16)((Sint32)phase * 4 - 16384);
}
static void Resources(Uint8 *overlay, Uint8 *exe)
{
    for (Uint16 i = 0; i < SOTN_PENTAGRAM_TRIANGLES; ++i)
    {
        // Authored small triangles, independent of the private original UV mesh.
        const Sint16 xy[6] = {56, 56, 72, 56, 56, 72};
        for (Uint16 j = 0; j < 6; ++j)
            Put16(overlay + 0x260 + i * 12U + j * 2U, xy[j]);
    }
    for (Uint16 i = 0; i <= 1024; ++i)
        Put16(exe + 0x1dbcc + i * 2U, (Sint16)(i * 4U));
    for (Uint16 i = 0; i < 4096; ++i)
    {
        Put16(exe + 0x1ea10 + i * 4U, AuthoredSine(i));
        Put16(exe + 0x1ea12 + i * 4U, AuthoredSine(i + 1024U));
    }
}
static bool Callback(void)
{
    Uint8 overlay[0x380] = {0};
    Uint8 *exe = SDL_calloc(1, 0x22a10);
    CHECK(exe);
    Resources(overlay, exe);
    SotnPentagram p = {.x = 30 * 65536 + 123, .y = 40 * 65536 + 456};
    SotnPentagramShared shared = {.shards[0].x = 17}, initial = shared;
    SotnPentagramInput in = {.game_timer = 2};
    SotnPentagramEvents e = {0};
    Uint32 random = 0xfed12345U;
    CHECK(SotnPentagram_Tick(&p, &shared, in, &random, &e) && p.removed && !e.properties &&
          !e.sound && !SDL_memcmp(&shared, &initial, sizeof(shared)) && random == 0xfed12345U);
    p = (SotnPentagram){.x = 123, .y = 456};
    in.overlay = (SotnDataView){overlay, sizeof(overlay)};
    in.executable = (SotnDataView){exe, 0x22a10};
    in.allocated = true;
    CHECK(SotnPentagram_Tick(&p, &shared, in, &random, &e) && p.phase == 1 && p.lifetime == 24 &&
          p.x == 128 * 65536 + 123 && p.y == 104 * 65536 + 456 && p.flags == 0x04800000 &&
          e.properties && e.use_ticks == 4 && e.sound == 0x6e4 && e.flash == 4 && e.shake == 3 &&
          !e.clear_attack);
    CHECK(shared.initialized && shared.vectors[0][3].x == -3 && shared.vectors[0][3].y == -3 &&
          shared.vectors[0][3].z == 640 && shared.shards[0].x == 17 &&
          p.polygons[0].palette == 0x111 && p.polygons[1].priority == 0x1b6 && !p.polygons[1].x[1]);
    for (Uint16 i = 0; i < 23; ++i)
        CHECK(SotnPentagram_Tick(&p, &shared, in, &random, &e) && p.lifetime == 23 - i &&
              !e.clear_attack && !e.properties);
    CHECK(random == 0xfed12345U);
    const SotnPentagram before = p;
    SotnPentagramShared held = shared;
    in.executable = (SotnDataView){0};
    e.use_ticks = 37;
    CHECK(!SotnPentagram_Tick(&p, &shared, in, &random, &e) &&
          !SDL_memcmp(&p, &before, sizeof(p)) && !SDL_memcmp(&shared, &held, sizeof(shared)) &&
          e.use_ticks == 37 && random == 0xfed12345U);
    in.executable = (SotnDataView){exe, 0x22a10};
    CHECK(SotnPentagram_Tick(&p, &shared, in, &random, &e) && p.phase == 2 && p.lifetime == 128 &&
          e.clear_attack && !(p.polygons[0].draw_mode & 8U));
    Uint32 expected = 0xfed12345U;
    for (Uint16 i = 0; i < 96; ++i)
        (void)SotnCombat_Random(&expected);
    CHECK(random == expected && shared.shards[0].x == -3 * 65536 && !shared.shards[0].angle_x);
    held = shared;
    const SotnPentagram breakup = p;
    // A projection failure follows motion in the working copy, and still commits nothing.
    in.executable.size = 0x1dbcc + 2050;
    CHECK(!SotnPentagram_Tick(&p, &shared, in, &random, &e) &&
          !SDL_memcmp(&p, &breakup, sizeof(p)) && !SDL_memcmp(&shared, &held, sizeof(shared)) &&
          random == expected);
    in.executable.size = 0x22a10;
    CHECK(SotnPentagram_Tick(&p, &shared, in, &random, &e) && p.lifetime == 127 &&
          (p.polygons[0].draw_mode & 8U) &&
          shared.shards[0].x == held.shards[0].x + held.shards[0].velocity_x &&
          shared.shards[0].angle_x == held.shards[0].speed_x && random == expected);
    held = shared;
    SotnPentagram other = {0};
    SotnPentagramShared right = {0};
    in.hand = 1;
    in.parameter = 0x8100;
    CHECK(SotnPentagram_Tick(&other, &right, in, &random, &e) &&
          other.polygons[0].palette == 0x19f && other.polygons[0].u[0] == 128 &&
          other.polygons[0].v[0] == 128 && !other.polygons[0].draw_mode &&
          !SDL_memcmp(&shared, &held, sizeof(shared)));
    in.hand = in.parameter = 0;
    for (Uint16 i = 0; i < 126; ++i)
        CHECK(SotnPentagram_Tick(&p, &shared, in, &random, &e) && !p.removed);
    CHECK(p.lifetime == 1);
    const SotnPentagram last = p;
    shared.shards[0].x = SDL_MAX_SINT32 - 1;
    shared.shards[0].velocity_x = 4;
    shared.shards[0].angle_x = 32760;
    shared.shards[0].speed_x = 16;
    in.game_timer = 0;
    in.executable = (SotnDataView){0};
    CHECK(SotnPentagram_Tick(&p, &shared, in, &random, &e) && p.removed &&
          shared.shards[0].x == SDL_MIN_SINT32 + 2 && shared.shards[0].angle_x == -32760 &&
          !SDL_memcmp(p.polygons, last.polygons, sizeof(p.polygons)) && random == expected);
    SDL_free(exe);
    return true;
}
static bool Shared(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000), *weapons = SDL_calloc(1, 20U * 0x7000U),
          *exe = SDL_calloc(1, 0x22a10);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r)), *before = SDL_malloc(sizeof(*before));
    SotnCharacter *character = NULL;
    CHECK(dra && weapons && exe && actors && r && before);
    TestCharacter_Fill(dra);
    for (Uint16 item = 73; item <= 74; ++item)
    {
        Uint8 *record = dra + 0x500 + item * 52U;
        Put16(record + 8, 43);
        record[14] = 7;
        record[15] = 17;
        record[17] = 130;
        record[18] = 0x60;
        record[19] = 50;
        record[20] = (Uint8)(item - 73);
        record[22] = record[25] = 1;
        record[26] = 30;
        Put16(record + 38, 4);
        Put16(record + 40, 0x82);
        Put16(record + 42, 1);
        Put16(record + 48, 9);
    }
    Uint8 *overlay = weapons + 17U * 0x7000U + 0x4000;
    Resources(overlay, exe);
    Uint8 *bp = dra + 0xd1d4 + 50U * 6U;
    bp[1] = bp[2] = 1;
    bp[4] = 0x26;
    dra[0xd4b8 + 12] = 17;
    dra[0xd4b8 + 13] = 31;
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    actors->dra = (SotnDataView){dra, 0x11000};
    actors->entered = true;
    SotnBatRuntimeInput in = {
        .dra = actors->dra,
        .weapons = {weapons, 20U * 0x7000U},
        .executable = {exe, 0x22a10},
        .camera_x = 100,
        .camera_y = 50,
        .player = {.hp = 70, .x = 128 * 65536, .y = 140 * 65536, .priority = 148}};
    CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots));
    bool allocated = false;
    CHECK(SotnBatRuntime_RequestThrowing(r, (SotnPlayerFactorySource){.priority = 148}, 0x1032, 73,
                                         &allocated) &&
          allocated);
    SotnCharacterState initial = {0}, after = {0};
    CHECK(SotnCharacter_Get(character, &initial, NULL));
    // Invalid private geometry fails after the real factory pass without publishing rows or RNG.
    Put16(overlay + 0x260, 128);
    *before = *r;
    CHECK(!SotnBatRuntime_Tick(r, character, in) && !SDL_memcmp(before, r, sizeof(*r)));
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == initial.random);
    Put16(overlay + 0x260, 56);
    CHECK(SotnBatRuntime_Tick(r, character, in) && r->pentagram_mask == 2 &&
          r->spell_use_ticks == 4 && !r->use_ticks && r->children[1].pentagram &&
          r->children[1].primitive_count == 25 &&
          r->children[1].state.pentagram.attack.invincibility_frames == 30 &&
          r->children[1].state.pentagram.motion.x == 228 * 65536 &&
          r->children[1].state.pentagram.motion.y == 154 * 65536 &&
          SotnActors_CommitBatPrimitives(actors, r));
    Uint16 rows[SOTN_PENTAGRAM_ROWS];
    SDL_memcpy(rows, r->children[1].primitives, sizeof(rows));
    for (Uint16 i = 0; i < SDL_arraysize(rows); ++i)
        CHECK(actors->primitives[rows[i]].owner);
    Uint32 expected = initial.random;
    (void)SotnCombat_Random(&expected);
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    *before = *r;
    in.freeze = 3;
    CHECK(SotnBatRuntime_Tick(r, character, in) && r->pentagram_mask == 2 &&
          !SDL_memcmp(&before->children[1], &r->children[1], sizeof(r->children[1])) &&
          !SDL_memcmp(before->pentagram, r->pentagram, sizeof(r->pentagram)));
    in.freeze = 0;
    in.camera_x = 103;
    in.screen_shift_x = -3;
    CHECK(SotnBatRuntime_Tick(r, character, in) &&
          r->children[1].state.pentagram.motion.x == 231 * 65536 &&
          r->children[1].state.pentagram.motion.polygons[0].x[0] == 64);
    in.screen_shift_x = 0;
    for (Uint16 i = 0; i < 22; ++i)
        CHECK(SotnBatRuntime_Tick(r, character, in) && r->pentagram_mask == 2);
    // The failed breakup retains all per-hand BSS and the character's actual libc cursor.
    in.executable = (SotnDataView){0};
    *before = *r;
    CHECK(!SotnBatRuntime_Tick(r, character, in) && !SDL_memcmp(before, r, sizeof(*r)));
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    in.executable = (SotnDataView){exe, 0x22a10};
    CHECK(SotnBatRuntime_Tick(r, character, in) && !r->pentagram_mask &&
          r->children[1].state.pentagram.motion.lifetime == 128 &&
          SotnActors_CommitBatPrimitives(actors, r));
    for (Uint16 i = 0; i < 96; ++i)
        (void)SotnCombat_Random(&expected);
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    const SotnPentagramShared scratch = r->pentagram[0];
    CHECK(SotnBatRuntime_PrunePentagram(r) && !r->children[1].kind &&
          !SDL_memcmp(&scratch, &r->pentagram[0], sizeof(scratch)) &&
          SotnActors_CommitBatPrimitives(actors, r));
    for (Uint16 i = 0; i < SDL_arraysize(rows); ++i)
        CHECK(!actors->primitives[rows[i]].owner);
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    // Actual shared-row exhaustion retires an optional child before its property roll or BSS work.
    SDL_memset(&in.primitive_slots, 0xff, sizeof(in.primitive_slots));
    CHECK(SotnBatRuntime_RequestThrowing(r, (SotnPlayerFactorySource){.priority = 148}, 0x812032,
                                         74, &allocated) &&
          allocated);
    CHECK(SotnBatRuntime_Tick(r, character, in) && !r->children[1].kind && !r->pentagram_mask &&
          !r->pentagram[1].initialized && !SDL_memcmp(&scratch, &r->pentagram[0], sizeof(scratch)));
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == expected);
    SotnCharacter_Destroy(character);
    SDL_free(before);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(exe);
    SDL_free(weapons);
    SDL_free(dra);
    return true;
}
int main(void)
{
    return Projection() && Callback() && Shared() ? 0 : 1;
}
