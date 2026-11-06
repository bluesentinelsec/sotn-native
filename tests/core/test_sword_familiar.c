#include "actors_internal.h"
#include <SDL3/SDL.h>
#include <sotn/actors.h>
#include <sotn/combat.h>
#include <sotn/sword_familiar.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static Uint8 overlay[0x2400], exe[0x23400];
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static void Put32(Uint8 *p, Uint32 value)
{
    Put16(p, (Uint16)value);
    Put16(p + 2, (Uint16)(value >> 16));
}
static void Vertex(Uint32 at, Sint16 x, Sint16 y, Sint16 z)
{
    Put16(overlay + at, (Uint16)x);
    Put16(overlay + at + 2, (Uint16)y);
    Put16(overlay + at + 4, (Uint16)z);
}
// Authored SDK tables (sine, square root, arctangent) and a synthetic tt_004 image: the level
// table, a pyramid model whose faces alternate winding, UVs, CLUTs and three room events.
static void Fixture(void)
{
    for (Uint32 i = 0; i <= 1024; ++i)
    {
        Put16(exe + 0x1dbcc + i * 2, (Uint16)SDL_lround(4096.0 * SDL_sin(i * SDL_PI_D / 2048.0)));
        Put16(exe + 0x22a10 + i * 2, (Uint16)SDL_lround(SDL_atan(i / 1024.0) * 2048.0 / SDL_PI_D));
    }
    for (Uint32 i = 0; i < 192; ++i)
        Put16(exe + 0x1e3e0 + i * 2, (Uint16)SDL_lround(SDL_sqrt((64.0 + i) / 64.0) * 4096.0));
    const Uint32 rows[10][3] = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1},
                                {0, 1, 1}, {0, 1, 1}, {1, 1, 1}, {1, 1, 1}, {1, 1, 3}};
    for (Uint16 i = 0; i < 10; ++i)
        for (Uint16 k = 0; k < 3; ++k)
            Put32(overlay + 0xa0 + i * 12U + k * 4U, rows[i][k]);
    for (Uint16 i = 0; i < 0x32; ++i)
    {
        const Sint16 a = (Sint16)(i * 37 % 120 - 60), b = (Sint16)(i * 53 % 200 - 100);
        Vertex(0x224 + i * 8U, a, b, (Sint16)(i % 7 * 6 - 18));
        Vertex(0xc54 + i * 8U, (Sint16)(a * 2), b, (Sint16)(i % 5 * 8 - 16));
    }
    for (Uint16 i = 0; i < 96; ++i)
    {
        const Uint16 a = (Uint16)(i % 0x26), b = (Uint16)((i + 5) % 0x26),
                     c = (Uint16)((i + 11) % 0x26);
        const bool flip = i & 1;
        if (i < 72)
        {
            Put16(overlay + 0x354 + i * 8U, a);
            Put16(overlay + 0x356 + i * 8U, i == 0 ? a : flip ? c : b);
            Put16(overlay + 0x358 + i * 8U, flip ? b : c);
        }
        Put16(overlay + 0xde4 + i * 8U, (Uint16)(i % 0x32));
        Put16(overlay + 0xde6 + i * 8U, (Uint16)((i + 7) % 0x32));
        Put16(overlay + 0xde8 + i * 8U, (Uint16)((i + 19) % 0x32));
        for (Uint16 k = 0; k < 6; ++k)
        {
            if (i < 72)
            {
                Put16(overlay + 0x594 + i * 12U + k * 2U, (Uint16)(i + k));
                Put16(overlay + 0x8f4 + i * 12U + k * 2U, (Uint16)(i + k + 100));
            }
            Put16(overlay + 0x10e4 + i * 12U + k * 2U, (Uint16)(i * 2 + k));
            Put16(overlay + 0x1564 + i * 12U + k * 2U, (Uint16)(i * 2 + k + 1));
        }
    }
    const Sint16 quad[4][3] = {{-60, -85, 0}, {61, -85, 0}, {-60, 274, 0}, {61, 274, 0}};
    for (Uint16 i = 0; i < 4; ++i)
        Vertex(0x118 + i * 8U, quad[i][0], quad[i][1], quad[i][2]);
    Vertex(0x138, 0, 90, 0);
    Vertex(0x140, 0, 274, 0);
    for (Uint16 i = 0; i < 16; ++i)
        Vertex(0x148 + i * 8U, (Sint16)((i % 4) * 20 - 30), (Sint16)(80 + i * 10), 0);
    Vertex(0x1c8, 0, 125, 0);
    Vertex(0x1d0, 0, 195, 0);
    for (Uint16 i = 0; i < 7; ++i)
        Put16(overlay + 0x90 + i * 2U, (Uint16)(0x700 + i)); // voices
    Put32(overlay + 0x218, 3);                               // circle hold
    Put32(overlay + 0x21c, 5);                               // dive hold
    Put32(overlay + 0x220, 4);                               // dive glide
    for (Uint16 i = 0; i < 16; ++i)
        Put16(overlay + 0x40 + i * 2U, (Uint16)(0x1000 + i));
    for (Uint16 i = 0; i < 32; ++i)
        Put16(overlay + 0x1d8 + i * 2U, (Uint16)(0x2000 + i));
    // Events: dummy, (49,20) cancel, (41,50) layer raise, sentinel.
    const Sint32 events[4][12] = {
        {0, 0, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0},
        {0, 0, 5, 49, 20, 0, 4, -1, 0, 0xdc, 0, 1},
        {0, 0, 5, 41, 50, 0, 4, (Sint32)(0x80000000U | 66), 0, 0xdc, 15, 1},
        {0, 0, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}};
    for (Uint16 i = 0; i < 4; ++i)
        for (Uint16 k = 0; k < 12; ++k)
            Put32(overlay + 0x19e4 + i * 48U + k * 4U, (Uint32)events[i][k]);
}
// An independent model of func_us_80172C8C, TransMatrix/CompMatrix with (0,0,6500), RTPS
// (UNR divide, offset 128, H1000), gte_stszotz and NCLIP.
static Sint32 Sra(Sint64 v, int n)
{
    return (Sint32)(v >= 0 ? v >> n : -((-v + (1LL << n) - 1) >> n));
}
static void SinCos(Sint32 a, Sint32 *s, Sint32 *c)
{
    SotnMathTrig t = {0};
    (void)SotnMath_Trig((SotnDataView){exe, sizeof(exe)}, a, &t);
    *s = t.sine;
    *c = t.cosine;
}
static Uint32 Unr(Uint32 depth)
{
    if (depth * 2U <= 1000U)
        return 0x1ffff;
    Uint32 shift = 0, d = depth;
    for (; d < 0x8000; ++shift)
        d *= 2;
    const Uint32 n = 1000U << shift, index = (d - 0x7fc0) >> 7;
    const Sint32 e = (Sint32)(((0x40000U / (index + 0x100) + 1) / 2)) - 0x101;
    const Uint32 u = (Uint32)SDL_clamp(e, 0, 255) + 0x101;
    d = (0x2000080U - d * u) >> 8;
    d = (0x80U + d * u) >> 8;
    return (Uint32)SDL_min(0x1ffffULL, ((Uint64)n * d + 0x8000) >> 16);
}
static void Transform(const SotnSwordRuntime *r, Sint32 m[3][3], Sint32 t[3])
{
    Sint32 sx, cx, sy, cy, sz, cz;
    SinCos(r->rotation[0], &sx, &cx);
    SinCos(r->rotation[1], &sy, &cy);
    SinCos(r->rotation[2], &sz, &cz);
    const Sint32 a = Sra((Sint64)cz * sy, 12), b = Sra((Sint64)sz * sy, 12);
    const Sint32 v[3][3] = {
        {Sra((Sint64)cz * cy, 12), Sra((Sint64)-sz * cx, 12) + Sra((Sint64)sx * a, 12),
         Sra((Sint64)sz * sx, 12) + Sra((Sint64)cx * a, 12)},
        {Sra((Sint64)sz * cy, 12), Sra((Sint64)cz * cx, 12) + Sra((Sint64)sx * b, 12),
         Sra((Sint64)-cz * sx, 12) + Sra((Sint64)cx * b, 12)},
        {Sra(-(Sint64)sy, 12), Sra((Sint64)cy * sx, 12), Sra((Sint64)cy * cx, 12)}};
    for (int i = 0; i < 3; ++i)
        for (int k = 0; k < 3; ++k)
            m[i][k] = (Sint16)v[i][k];
    t[0] = r->position[0] / 5;
    t[1] = r->position[1] / 5;
    t[2] = r->position[2] + 6500;
}
static SotnSwordPoint Rtps(Sint32 m[3][3], const Sint32 t[3], Uint32 at, Sint32 *otz)
{
    const Sint16 v[3] = {(Sint16)(overlay[at] | overlay[at + 1] << 8),
                         (Sint16)(overlay[at + 2] | overlay[at + 3] << 8),
                         (Sint16)(overlay[at + 4] | overlay[at + 5] << 8)};
    Sint64 mac[3];
    for (int i = 0; i < 3; ++i)
        mac[i] = t[i] +
                 Sra((Sint64)m[i][0] * v[0] + (Sint64)m[i][1] * v[1] + (Sint64)m[i][2] * v[2], 12);
    const Uint32 sz = (Uint32)SDL_clamp(mac[2], 0, 0xffff), ratio = Unr(sz);
    *otz = (Sint32)(sz / 4);
    return (SotnSwordPoint){
        (Sint16)SDL_clamp(128 + Sra((Sint64)ratio * SDL_clamp(mac[0], -32768, 32767), 16), -1024,
                          1023),
        (Sint16)SDL_clamp(128 + Sra((Sint64)ratio * SDL_clamp(mac[1], -32768, 32767), 16), -1024,
                          1023)};
}
static bool Same(SotnSwordPoint a, SotnSwordPoint b)
{
    return a.x == b.x && a.y == b.y;
}
// Every projected word and row of the small/large models against the model above.
static bool ProjectionCheck(const SotnSwordRuntime *r, Sint32 base)
{
    Sint32 m[3][3], t[3], otz = 0, depth[0x32];
    Transform(r, m, t);
    const bool large = r->slots[0].model != 0;
    const Uint16 vertices = large ? 0x32 : 0x26, faces = large ? 0x60 : 0x48;
    SotnSwordPoint p[0x32];
    Sint32 nearest = SDL_MAX_SINT32;
    for (Uint16 i = 0; i < vertices; ++i)
    {
        p[i] = Rtps(m, t, (large ? 0xc54 : 0x224) + i * 8U, &depth[i]);
        nearest = SDL_min(nearest, depth[i]);
    }
    for (Uint16 i = 0; i < faces; ++i)
    {
        const Uint8 *f = overlay + (large ? 0xde4 : 0x354) + i * 8U;
        const Uint16 a = (Uint16)(f[0] | f[1] << 8), b = (Uint16)(f[2] | f[3] << 8),
                     c = (Uint16)(f[4] | f[5] << 8);
        const Sint64 clip = (Sint64)p[a].x * p[b].y + (Sint64)p[b].x * p[c].y +
                            (Sint64)p[c].x * p[a].y - (Sint64)p[a].x * p[c].y -
                            (Sint64)p[b].x * p[a].y - (Sint64)p[c].x * p[b].y;
        const SotnPlayerEffectQuad *q = &r->model[i];
        CHECK(((q->draw_mode & 8U) != 0) == (clip < 0));
        if (clip >= 0)
            CHECK(q->x[0] == p[a].x && q->y[1] == p[b].y && q->x[2] == p[c].x &&
                  q->x[3] == p[c].x && q->y[3] == p[c].y &&
                  q->priority ==
                      (Uint16)(base -
                               (SDL_max(depth[a], SDL_max(depth[b], depth[c])) - nearest) / 9));
    }
    CHECK(Same(r->edge[0], Rtps(m, t, 0x138, &otz)) && Same(r->edge[1], Rtps(m, t, 0x140, &otz)) &&
          r->edge_priority == base - 10 && Same(r->tips[0], Rtps(m, t, 0x1c8, &otz)) &&
          Same(r->tips[1], Rtps(m, t, 0x1d0, &otz)));
    for (Uint16 k = 0; k < 4; ++k)
    {
        Sint32 far = 0;
        for (Uint16 v = 0; v < 4; ++v)
        {
            CHECK(Same(r->halo_points[k][v], Rtps(m, t, 0x148 + (k * 4U + v) * 8U, &otz)));
            far = v ? SDL_max(far, otz) : otz;
        }
        CHECK(r->halo_priority[k] == base - (far - nearest) / 9 + 1);
    }
    return true;
}
static SotnWorldState world;
static SotnSwordTarget no_targets[SOTN_SWORD_TARGETS];
static SotnSwordInput Input(Sint32 level, Sint32 loads, Uint8 castle, Uint16 room_flags)
{
    return (SotnSwordInput){
        .overlay = {overlay, sizeof(overlay)},
        .executable = {exe, sizeof(exe)},
        .player = {.x = 120 * 65536, .y = 150 * 65536, .priority = 0x94, .collision = 1},
        .stats = {level, (level - 1) * 100, loads},
        .targets = no_targets,
        .room_flags = room_flags,
        .stage = 2,
        .castle_flag = castle,
        .allocated = true,
        .event_source = {{overlay, sizeof(overlay)}, 0x801719e4, 4},
        .room = {.world = &world, .room_x = 10, .room_y = 10, .stage = 2, .servant = 5},
        .sound = {.cd_queue_empty = true}};
}
static bool Run(SotnSwordRuntime *r, SotnSwordInput in, SotnSwordEvents *last)
{
    SotnSwordEvents e = {0};
    *last = (SotnSwordEvents){0};
    for (Uint16 slot = 4; slot < 8; ++slot)
    {
        if (!SotnSwordRuntime_UpdateSlot(r, slot, in, &e))
            return false;
        if (e.cd_command)
            last->cd_command = e.cd_command;
        if (e.castle_flag_write)
        {
            last->castle_flag_write = true;
            last->castle_flag = e.castle_flag;
        }
        if (e.pause_write)
        {
            last->pause_write = true;
            last->pause_allowed = e.pause_allowed;
        }
        last->sound_count = (Uint16)(last->sound_count + e.sound_count);
        last->dismiss = last->dismiss || e.dismiss;
        if (e.gift_item)
            last->gift_item = e.gift_item;
    }
    return true;
}
// CLUT words, ServantInit placement for every mode/room kind, aura spawns and castle writes.
static bool InitTest(void)
{
    Uint16 clut[SOTN_SWORD_CLUT_WORDS] = {0};
    CHECK(SotnSword_Clut((SotnDataView){overlay, sizeof(overlay)}, clut) && clut[0] == 0x1000 &&
          clut[15] == 0x100f && clut[16] == 0x2000 && clut[47] == 0x201f);
    CHECK(!SotnSword_Clut((SotnDataView){overlay, 0x200}, clut));
    SotnSwordRuntime *r = SDL_calloc(1, sizeof(*r));
    CHECK(r != NULL);
    SotnSwordEvents e = {0};
    SotnSwordInput in = Input(12, 1, 0, 0);
    CHECK(SotnSwordRuntime_Init(r, in, 1, &e) && e.clut_upload && !e.castle_flag_write);
    CHECK(r->slots[0].identity == SOTN_SWORD_ENTRANCE && r->slots[0].x == 128 * 65536 &&
          r->slots[0].y == -32 * 65536 && r->slots[0].facing_left && r->slots[0].priority == 0x92 &&
          r->slots[1].identity == SOTN_SWORD_TRAIL && !r->slots[2].identity &&
          r->position[0] == 0 && r->position[1] == -160 * 32 && r->sound_step == 99);
    // Menu switch in a flag40 room: the default sword, castle flag 1 or 2 becomes 4.
    SDL_zerop(r);
    in = Input(95, 3, 2, 0x40);
    CHECK(SotnSwordRuntime_Init(r, in, 1, &e) && e.castle_flag_write && e.castle_flag == 4 &&
          r->slots[0].identity == SOTN_SWORD_DEFAULT && r->slots[1].identity == SOTN_SWORD_TRAIL &&
          r->slots[2].identity == SOTN_SWORD_HALO);
    // Engine init in flag20 rooms: the room side flips between castles; elsewhere beside the
    // player.
    const struct
    {
        Uint16 flags, stage;
        Sint32 x;
    } sides[] = {
        {0x22, 2, 64}, {0x22, 0x21, 192}, {0x20, 2, 192}, {0x20, 0x21, 64}, {0x60, 2, 192}};
    for (Uint16 i = 0; i < SDL_arraysize(sides); ++i)
    {
        SDL_zerop(r);
        in = Input(30, 1, 0, sides[i].flags);
        in.stage = sides[i].stage;
        CHECK(SotnSwordRuntime_Init(r, in, 0, &e) && r->slots[0].identity == SOTN_SWORD_DEFAULT &&
              r->slots[0].x == sides[i].x * 65536 && r->slots[0].y == 144 * 65536 &&
              r->position[0] == (sides[i].x - 128) * 32 && r->position[1] == 16 * 32);
    }
    SDL_zerop(r);
    in = Input(30, 1, 0, 0);
    in.player.facing_left = true;
    CHECK(SotnSwordRuntime_Init(r, in, 0, &e) && r->slots[0].x == 148 * 65536 &&
          r->slots[0].y == 122 * 65536 && !r->slots[0].facing_left);
    // Mode3 (same servant) only resets the event cache; invalid modes/stats fail unchanged.
    const SotnSwordRuntime before = *r;
    CHECK(SotnSwordRuntime_Init(r, in, 3, &e) && !e.clut_upload &&
          !SDL_memcmp(r->slots, before.slots, sizeof(r->slots)));
    CHECK(!SotnSwordRuntime_Init(r, in, 2, &e));
    in.stats.level = 7;
    CHECK(!SotnSwordRuntime_Init(r, in, 0, &e) && !SDL_memcmp(r, &before, sizeof(*r)));
    SDL_free(r);
    return true;
}
// The entrance settles beside a still player, requests its speech by load count/level/castle
// flag, holds pause and returns to the default sword once the speech retires; movement,
// enemies, cutscenes, flashes and the room-event cancel abort it.
static bool EntranceTest(void)
{
    const struct
    {
        Sint32 level, loads;
        Uint8 castle;
        Uint16 speech;
    } cases[] = {{12, 1, 0, 0x4fb},
                 {12, 2, 0, 0x4fc},
                 {60, 2, 4, 0x4ff},
                 {33, 2, 2, 0x4fe},
                 {49, 2, 4, 0x4fc}};
    SotnSwordRuntime *r = SDL_calloc(1, sizeof(*r));
    CHECK(r != NULL);
    for (Uint16 k = 0; k < SDL_arraysize(cases); ++k)
    {
        SDL_zerop(r);
        SotnSwordInput in = Input(cases[k].level, cases[k].loads, cases[k].castle, 0);
        SotnSwordEvents e = {0};
        CHECK(SotnSwordRuntime_Init(r, in, 1, &e));
        Uint16 speech = 0, tick = 0;
        bool paused = false, resumed = false;
        for (; tick < 600 && r->slots[0].identity == SOTN_SWORD_ENTRANCE; ++tick)
        {
            in.sound.command_acknowledged = tick % 3 == 0;
            in.sound.xa_playing = tick % 120 >= 30 && tick % 120 < 90;
            CHECK(Run(r, in, &e));
            if (e.castle_flag_write)
                in.castle_flag = e.castle_flag;
            if (e.cd_command >= 0x4fb && e.cd_command <= 0x4ff)
                speech = e.cd_command;
            paused |= e.pause_write && !e.pause_allowed;
            resumed |= e.pause_write && e.pause_allowed;
        }
        CHECK(r->slots[0].identity == SOTN_SWORD_DEFAULT && speech == cases[k].speech && paused &&
              resumed && r->pause_allowed && tick > 40 &&
              in.castle_flag == (cases[k].castle == 2 ? 4 : cases[k].castle));
    }
    // Aborts during the approach (step1) return to the default sword immediately.
    for (Uint16 k = 0; k < 5; ++k)
    {
        SDL_zerop(r);
        SotnSwordInput in = Input(12, 1, 2, 0);
        SotnSwordEvents e = {0};
        CHECK(SotnSwordRuntime_Init(r, in, 1, &e) && Run(r, in, &e) && r->slots[0].step == 1);
        in.castle_flag = 3;
        in.player.velocity_x = k == 0 ? 0x10000 : 0;
        in.enemies = k == 1;
        in.cutscene = k == 2;
        in.global_flash = k == 3;
        r->entrance_cancel = k == 4;
        CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) &&
              r->slots[0].identity == SOTN_SWORD_DEFAULT && !r->slots[0].step &&
              e.castle_flag_write && e.castle_flag == 4);
    }
    SDL_free(r);
    return true;
}
// The default sword: model setup by level, follow velocity bands, yaw/pitch steering and
// projection with back faces hidden; the trail follows the projected edge.
static bool FollowTest(void)
{
    SotnSwordRuntime *r = SDL_calloc(1, sizeof(*r));
    CHECK(r != NULL);
    for (Uint16 k = 0; k < 2; ++k)
    {
        SDL_zerop(r);
        SotnSwordInput in = Input(k ? 55 : 30, 2, 4, 0);
        SotnSwordEvents e = {0};
        CHECK(SotnSwordRuntime_Init(r, in, 0, &e) && Run(r, in, &e));
        const Uint16 used = k ? 96 : 72;
        CHECK(r->slots[0].step == 1 && r->model_ready && r->slots[0].model == k &&
              r->slots[0].variant == 1 && r->slots[0].flags == 0x0c820000U &&
              r->model[0].palette == 0x140 && r->model[0].page == 0x1b &&
              r->model[0].u[0] == (k ? 1 : 100) && r->model[0].u[2] == r->model[0].u[3]);
        if (!k)
            CHECK((r->model[90].draw_mode & 8U) && r->model[90].draw_mode == 0x108);
        CHECK(r->model[3].u[2] == (k ? 3 * 2 + 4 + 1 : 3 + 4 + 100) &&
              r->model[3].v[2] == (k ? 3 * 2 + 5 + 1 : 3 + 5 + 100) &&
              r->model[3].v[1] == (k ? 3 * 2 + 3 + 1 : 3 + 3 + 100) && r->slots[0].yaw == 0x400 &&
              r->slots[0].yaw_previous == 0x400);
        CHECK(ProjectionCheck(r, r->position[2] < 0 ? 0x9f : r->position[2] > 0 ? 0x8b : 0x93));
        if (!k)
            CHECK(!(r->model[0].draw_mode & 8U));
        // One exact follow step against its model (velocity, heading/yaw, pitch and depth).
        for (Uint16 step = 0; step < 40; ++step)
        {
            if (step == 0 || step == 20)
                r->position[2] = (step == 0) == (k == 0) ? -2400 : 1800;
            in.player.x += (step & 8) ? -0x30000 : 0x24000;
            if (step == 10)
                in.player.x += 120 * 65536; // |dx| dominates the pitch spread.
            const SotnSwordSlot s0 = r->slots[0];
            const Sint32 bob = (r->bob + 16) & 0xfff, z = r->position[2];
            Sint32 sn, cs;
            SinCos(bob, &sn, &cs);
            const Sint32 tx = in.player.x - 28 * 65536, ty = in.player.y - (cs * 256 + 28 * 65536);
            const Sint32 dx = Sra((Sint64)tx - s0.x, 16), dy = Sra((Sint64)ty - s0.y, 16);
            Uint32 root = 0;
            CHECK(SotnMath_Sqrt12((SotnDataView){exe, sizeof(exe)},
                                  (Uint32)(dx * dx + dy * dy) << 12, &root));
            const Sint32 d = (Sint32)root >> 12;
            const int shift = d < 16 ? 6 : d < 64 ? 5 : d < 256 ? 4 : 2;
            Sint16 heading = s0.heading, target = 0;
            if (d > 16)
            {
                CHECK(SotnMath_Atan2Vector((SotnDataView){exe, sizeof(exe)}, dy, dx, &heading));
                heading = (Sint16)(heading & 0xfff);
            }
            const Sint16 yaw = SotnFairy_StepAngle(d > 16 ? heading : 0x400, s0.yaw_previous, 8);
            CHECK(SotnMath_Atan2Vector((SotnDataView){exe, sizeof(exe)}, -z,
                                       SDL_max(SDL_abs(dx) * 32, SDL_abs(dy) * 32), &target));
            const Sint16 pitch =
                SotnFairy_StepAngle((Sint16)(target & 0xfff), s0.pitch_previous, 0x20);
            SinCos(pitch, &sn, &cs);
            CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e));
            CHECK(r->slots[0].velocity_x == Sra((Sint64)tx - s0.x, shift) &&
                  r->slots[0].velocity_y == Sra((Sint64)ty - s0.y, shift) &&
                  r->slots[0].heading == heading && r->slots[0].yaw == yaw &&
                  r->rotation[2] == (Sint16)(yaw - 0x400) && r->slots[0].pitch == pitch &&
                  r->rotation[0] == pitch && r->position[2] == z + Sra((Sint64)sn * 0x60, 12) &&
                  r->bob == bob && r->position[0] == ((r->slots[0].x >> 16) - 128) * 32);
            CHECK(ProjectionCheck(r, r->position[2] < 0 ? 0x9f : r->position[2] > 0 ? 0x8b : 0x93));
        }
        Uint16 visible = 0, hidden = 0;
        for (Uint16 i = 0; i < used; ++i)
            (r->model[i].draw_mode & 8U) ? ++hidden : ++visible;
        CHECK(visible > 0 && hidden > 0);
        // Distance bands: >=256 moves a quarter, then 1/16, 1/32 and 1/64.
        const Sint32 shifts[5][2] = {{300, 2}, {230, 4}, {120, 4}, {40, 5}, {8, 6}};
        for (Uint16 b = 0; b < 5; ++b)
        {
            r->slots[0].x =
                (Sint32)((Uint32)in.player.x - 28U * 65536U - (Uint32)shifts[b][0] * 65536U);
            r->slots[0].y = in.player.y - 44 * 65536;
            r->bob = -16;
            const Sint32 x = r->slots[0].x;
            CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e));
            const Sint32 tx = in.player.x - 28 * 65536;
            CHECK(r->slots[0].velocity_x == (tx - x) >> shifts[b][1] &&
                  r->slots[0].x == x + r->slots[0].velocity_x);
        }
        // Trail: a fresh edge shifts every band up; a stale edge hides them and steps back.
        CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) &&
              SotnSwordRuntime_UpdateSlot(r, 5, in, &e));
        CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && r->edge_fresh);
        const SotnPlayerEffectQuad last = r->trail[15];
        CHECK(SotnSwordRuntime_UpdateSlot(r, 5, in, &e) && !r->edge_fresh &&
              r->trail[14].x[0] == last.x[0] && r->trail[15].x[0] == r->edge[0].x &&
              r->trail[15].x[2] == last.x[0] && !(r->trail[15].draw_mode & 8U) &&
              r->trail[15].priority == (Uint16)r->edge_priority && r->slots[1].step == 2 &&
              r->trail[0].green[0] == 22 && r->trail[0].green[2] == 8 && r->trail[15].red[0] == 72);
        CHECK(SotnSwordRuntime_UpdateSlot(r, 5, in, &e) && (r->trail[3].draw_mode & 8U) &&
              r->slots[1].step == 1);
    }
    SDL_free(r);
    return true;
}
// Level-90 halo colours/phase, room events (cancel, layer raise), the gift hand-off and the
// explicit boundaries of states not yet integrated.
static bool EventsTest(void)
{
    SotnSwordRuntime *r = SDL_calloc(1, sizeof(*r));
    CHECK(r != NULL);
    SotnSwordInput in = Input(95, 2, 4, 0);
    SotnSwordEvents e = {0};
    CHECK(SotnSwordRuntime_Init(r, in, 0, &e) && Run(r, in, &e) && Run(r, in, &e) &&
          r->slots[2].identity == SOTN_SWORD_HALO && r->slots[2].step == 1);
    CHECK(Run(r, in, &e) && r->halo_phase == 0xc0 && !(r->halo[0].draw_mode & 8U) &&
          r->halo[0].palette == 0x141 && r->halo[0].blue[1] == 192 &&
          r->halo[2].red[0] == r->halo[0].red[0] + 64 && r->halo[0].red[0] > 32 &&
          r->halo[0].red[0] == r->halo[0].green[0]);
    const Sint32 base = r->halo_priority[0];
    // Room (49,20) at camera (0,4): event0 sets the entrance cancel.
    in.room.room_x = 49;
    in.room.room_y = 20;
    in.room.camera_y = 4;
    CHECK(Run(r, in, &e) && r->entrance_cancel);
    // Room (41,50): event15 raises the layer around the player.
    in.room.room_x = 41;
    in.room.room_y = 50;
    in.player.priority = 0x70; // Distinct from the fixed 0x8B/0x93/0x9F layers.
    CHECK(Run(r, in, &e));
    r->position[2] = -1500;
    CHECK(Run(r, in, &e) && r->near_player && r->halo_priority[0] != base && r->position[2] < 0);
    CHECK(ProjectionCheck(r, r->position[2] < 0   ? 0x70 + 11
                             : r->position[2] > 0 ? 0x70 - 9
                                                  : 0x70 - 1));
    // A level change respawns a missing aura (CheckSwordLevel).
    SDL_zero(r->slots[1]);
    in.stats = (SotnFamiliarStats){96, 9500, 2};
    CHECK(Run(r, in, &e) &&
          (r->slots[1].identity == SOTN_SWORD_TRAIL || r->slots[3].identity == SOTN_SWORD_TRAIL) &&
          r->level_seen == 96);
    // A missing stage snapshot, Sword Brothers and the level-50 gift fail explicitly without
    // mutation.
    SotnSwordRuntime before = *r;
    in.targets = NULL;
    CHECK(!SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && !SDL_memcmp(r, &before, sizeof(*r)));
    in.targets = no_targets;
    in.brothers = true;
    CHECK(!SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && !SDL_memcmp(r, &before, sizeof(*r)));
    in.brothers = false;
    in.castle_flag = 1;
    CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && r->slots[0].identity == SOTN_SWORD_GIFT &&
          e.castle_flag_write && e.castle_flag == 2 && !e.gift_item);
    CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && r->slots[0].step == 1 &&
          r->slots[0].flags == 0x04820000U);
    CHECK(!SotnSwordRuntime_UpdateSlot(r, 3, in, &e) &&
          !SotnSwordRuntime_UpdateSlot(NULL, 4, in, &e));
    SDL_free(r);
    return true;
}
// Entrance speed bands and the 32-pixel settle; event0 stops a live speech (phase8).
static bool EntranceBandsTest(void)
{
    SotnSwordRuntime *r = SDL_calloc(1, sizeof(*r));
    CHECK(r != NULL);
    SotnSwordInput in = Input(30, 2, 4, 0);
    SotnSwordEvents e = {0};
    CHECK(SotnSwordRuntime_Init(r, in, 1, &e) && SotnSwordRuntime_UpdateSlot(r, 4, in, &e));
    const Sint32 bands[4][2] = {{150, 4}, {80, 5}, {55, 6}, {20, 6}};
    for (Uint16 b = 0; b < 4; ++b)
    {
        Sint32 sn, cs;
        SinCos((r->entrance_bob + 16) & 0xfff, &sn, &cs);
        const Sint32 tx = in.player.x + 24 * 65536, ty = in.player.y - (cs * 256 + 28 * 65536);
        r->slots[0].x = tx - bands[b][0] * 65536;
        r->slots[0].y = ty;
        r->slots[0].step = 1;
        const Sint32 x = r->slots[0].x;
        CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) &&
              r->slots[0].velocity_x == Sra((Sint64)tx - x, bands[b][1]));
        // Settles (step2) once the moved sword is within 32 pixels.
        const Sint32 after = Sra((Sint64)tx - r->slots[0].x, 16);
        CHECK(r->slots[0].step == (after < 32 ? 2 : 1));
    }
    SDL_zerop(r);
    CHECK(SotnSwordRuntime_Init(r, in, 0, &e) && Run(r, in, &e));
    r->slots[2] = (SotnSwordSlot){.generation = 50,
                                  .identity = SOTN_SWORD_SOUND,
                                  .step = 2,
                                  .parameter = 0x4fc,
                                  .flags = 0x04020000U,
                                  .sound = {.flags = 0x04020000U, .parameter = 0x4fc, .phase = 2}};
    in.room.room_x = 49;
    in.room.room_y = 20;
    in.room.camera_y = 4;
    CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && r->slots[3].identity == SOTN_SWORD_EVENT &&
          SotnSwordRuntime_UpdateSlot(r, 7, in, &e) && r->entrance_cancel &&
          r->slots[2].sound.phase == 8 && r->slots[2].step == 8 && !r->slots[3].identity);
    SDL_free(r);
    return true;
}
// Allocation failures, camera/room translation and ServantWorkClear.
static bool LifecycleTest(void)
{
    SotnSwordRuntime *r = SDL_calloc(1, sizeof(*r));
    CHECK(r != NULL);
    SotnSwordInput in = Input(30, 2, 4, 0);
    SotnSwordEvents e = {0};
    CHECK(SotnSwordRuntime_Init(r, in, 0, &e));
    in.allocated = false;
    // The failed model leaves no sword, but the callback still projects from screen (0,0).
    CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && !r->slots[0].identity &&
          r->position[0] == -4096 && r->position[1] == -4096 && r->edge_fresh);
    CHECK(SotnSwordRuntime_UpdateSlot(r, 5, in, &e) && !r->slots[1].identity);
    SDL_zerop(r);
    in.allocated = true;
    CHECK(SotnSwordRuntime_Init(r, in, 0, &e) && Run(r, in, &e) && Run(r, in, &e));
    const SotnSwordRuntime before = *r;
    CHECK(SotnSwordRuntime_Translate(r, 3, -2, 0, 0, false) &&
          r->slots[0].x == before.slots[0].x + 3 * 65536 &&
          r->slots[0].y == before.slots[0].y - 2 * 65536 &&
          r->model[0].x[0] == before.model[0].x[0] + 3 &&
          r->trail[0].x[0] == before.trail[0].x[0] + 3);
    // Room entry keeps 0x20000 entities, moving them by the player displacement; rows follow
    // DRAW_UNK_100.
    r->slots[3] =
        (SotnSwordSlot){.generation = 99, .identity = SOTN_SWORD_SOUND, .flags = 0x04000000U};
    const SotnSwordRuntime moved = *r;
    CHECK(SotnSwordRuntime_Translate(r, 0, 0, 5 * 65536, 0, true) && !r->slots[3].identity &&
          r->slots[0].x == moved.slots[0].x - 5 * 65536 &&
          r->model[0].x[0] == moved.model[0].x[0] - 5);
    CHECK(SotnSwordRuntime_Clear(r) && !r->slots[0].identity && r->initialized &&
          !SotnSwordRuntime_Translate(NULL, 0, 0, 0, 0, false));
    SDL_free(r);
    return true;
}
// Row leases: which callbacks reserve, which owners hold, and the real stage-pool commit.
static bool LeaseTest(void)
{
    SotnSwordRuntime *r = SDL_calloc(1, sizeof(*r));
    SotnActors *a = SDL_calloc(1, sizeof(*a));
    CHECK(r && a);
    SotnSwordLease want[3] = {0}, leases[3] = {0};
    CHECK(SotnSwordRuntime_RowRequest(r, 4) == 0 && SotnSwordRuntime_Leases(r, want) &&
          !want[0].count && !SotnSwordRuntime_Leases(NULL, want));
    SotnSwordInput in = Input(95, 3, 0, 0);
    SotnSwordEvents e = {0};
    CHECK(SotnSwordRuntime_Init(r, in, 0, &e));
    CHECK(SotnSwordRuntime_RowRequest(r, 4) == 96 && SotnSwordRuntime_RowRequest(r, 5) == 16 &&
          SotnSwordRuntime_RowRequest(r, 6) == 4 && SotnSwordRuntime_RowRequest(r, 7) == 0 &&
          SotnSwordRuntime_RowRequest(r, 3) == 0 && SotnSwordRuntime_RowRequest(r, 8) == 0);
    CHECK(SotnSwordRuntime_Leases(r, want) && !want[0].count && !want[1].count && !want[2].count);
    CHECK(Run(r, in, &e) && SotnSwordRuntime_RowRequest(r, 4) == 0 &&
          SotnSwordRuntime_RowRequest(r, 5) == 0 && SotnSwordRuntime_Leases(r, want));
    CHECK(want[0].count == 96 && want[0].slot == 4 &&
          want[0].generation == r->slots[0].generation && want[1].count == 16 &&
          want[1].slot == 5 && want[1].part == 1 && want[2].count == 4 && want[2].slot == 6 &&
          want[2].generation == r->slots[2].generation);
    // A second live model owner is not an original lease.
    SotnSwordRuntime twice = *r;
    twice.slots[3] = twice.slots[0];
    CHECK(!SotnSwordRuntime_Leases(&twice, want));
    CHECK(SotnSwordRuntime_Leases(r, want));
    // Stage rows 0..2 and a Fairy row stay owned by others.
    for (Uint16 i = 0; i < 3; ++i)
        a->primitives[i] = (ActorPrimitive){.owner = 7};
    a->primitives[3] = (ActorPrimitive){.owner = 9, .ability_kind = 0x85dd};
    bool free_rows = false;
    CHECK(SotnActors_FreeRows(a, 1020, &free_rows) && free_rows &&
          SotnActors_FreeRows(a, 1021, &free_rows) && !free_rows);
    CHECK(SotnActors_CommitSwordRows(a, leases, want));
    CHECK(leases[0].count == 96 && leases[0].rows[0] == 4 && leases[0].rows[95] == 99 &&
          leases[1].rows[0] == 100 && leases[2].rows[3] == 119 &&
          a->primitives[4].owner == want[0].generation && a->primitives[4].ability_kind == 0x9004 &&
          a->primitives[100].ability_kind == 0x9105 && a->primitives[119].ability_kind == 0x9206);
    CHECK(SotnActors_SwordRow(a, &leases[0], 95) && !SotnActors_SwordRow(a, &leases[0], 96) &&
          SotnActors_SwordRow(a, &leases[2], 0));
    // Unchanged owners keep their rows; a replaced model takes the first free rows.
    const SotnSwordLease kept = leases[1];
    a->primitives[2] = (ActorPrimitive){0};
    want[0].generation += 50;
    CHECK(SotnActors_CommitSwordRows(a, leases, want));
    CHECK(!SDL_memcmp(&kept, &leases[1], sizeof(kept)) && leases[0].rows[0] == 2 &&
          leases[0].rows[1] == 4 && leases[0].generation == want[0].generation &&
          !a->primitives[99].owner && a->primitives[98].owner == want[0].generation);
    // Stale or unclaimed rows fail before any change.
    SotnActors *copy = SDL_malloc(sizeof(*copy));
    CHECK(copy != NULL);
    SotnSwordLease saved[3] = {0};
    SDL_memcpy(saved, leases, sizeof(saved));
    a->primitives[leases[1].rows[3]].owner ^= 1;
    *copy = *a;
    CHECK(!SotnActors_CommitSwordRows(a, leases, want) && !SDL_memcmp(copy, a, sizeof(*a)) &&
          !SDL_memcmp(saved, leases, sizeof(saved)) && !SotnActors_SwordRow(a, &leases[1], 3));
    a->primitives[leases[1].rows[3]].owner ^= 1;
    a->primitives[900] = (ActorPrimitive){.owner = 4, .ability_kind = 0x9107};
    *copy = *a;
    CHECK(!SotnActors_CommitSwordRows(a, leases, want) && !SDL_memcmp(copy, a, sizeof(*a)));
    a->primitives[900] = (ActorPrimitive){0};
    // A malformed request (wrong part size) is rejected.
    SotnSwordLease bad[3] = {0};
    SDL_memcpy(bad, want, sizeof(bad));
    bad[2].count = 5;
    CHECK(!SotnActors_CommitSwordRows(a, leases, bad));
    // A retired part releases its rows; a new owner finding too few free rows fails, preserving
    // the pool and the previous leases.
    want[2].count = 0;
    CHECK(SotnActors_CommitSwordRows(a, leases, want) && !leases[2].count &&
          !a->primitives[116].owner && leases[0].count == 96);
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        if (!a->primitives[i].owner && i != 117 && i != 300)
            a->primitives[i] = (ActorPrimitive){.owner = 7};
    want[2].count = 4;
    *copy = *a;
    SDL_memcpy(saved, leases, sizeof(saved));
    CHECK(SotnActors_FreeRows(a, 2, &free_rows) && free_rows &&
          SotnActors_FreeRows(a, 3, &free_rows) && !free_rows);
    CHECK(!SotnActors_CommitSwordRows(a, leases, want) && !SDL_memcmp(copy, a, sizeof(*a)) &&
          !SDL_memcmp(saved, leases, sizeof(saved)));
    // Retirement releases every Sword row and nothing else.
    SDL_zero(want);
    want[1].part = 1;
    want[2].part = 2;
    CHECK(SotnActors_CommitSwordRows(a, leases, want) && !leases[0].count && !leases[1].count &&
          !a->primitives[4].owner && a->primitives[3].owner == 9 && a->primitives[0].owner == 7);
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        CHECK((a->primitives[i].ability_kind & 0xf000U) != 0x9000U);
    CHECK(!SotnActors_CommitSwordRows(NULL, leases, want));
    SDL_free(copy);
    SDL_free(a);
    SDL_free(r);
    return true;
}
// Default -> attack: the first live unmarked target is marked; marked ones rotate by cursor.
static bool SearchTest(void)
{
    SotnSwordRuntime *r = SDL_calloc(1, sizeof(*r)), *idle = SDL_malloc(sizeof(*idle));
    CHECK(r && idle);
    SotnSwordInput in = Input(30, 1, 4, 0);
    SotnSwordEvents e = {0};
    CHECK(SotnSwordRuntime_Init(r, in, 0, &e) && Run(r, in, &e) && r->slots[0].step == 1);
    *idle = *r;
    static SotnSwordTarget t[SOTN_SWORD_TARGETS];
    SDL_zeroa(t);
    t[0] = (SotnSwordTarget){1, 0, 0, 100, 100, 5};        // no hitbox
    t[1] = (SotnSwordTarget){1, 1, 0x200000, 100, 100, 5}; // excluded flag
    t[2] = (SotnSwordTarget){1, 1, 0, -17, 100, 5};
    t[3] = (SotnSwordTarget){1, 1, 0, 273, 100, 5};
    t[4] = (SotnSwordTarget){1, 1, 0, 100, 241, 5};
    t[5] = (SotnSwordTarget){1, 1, 0, 100, -1, 5};
    t[6] = (SotnSwordTarget){1, 1, 0, 100, 100, 0x7000};
    t[7] = (SotnSwordTarget){0, 1, 0, 100, 100, 5}; // empty slot
    t[8] = (SotnSwordTarget){1, 1, 0, -16, 0, 0};   // the search itself ignores HP <= 0
    t[9] = (SotnSwordTarget){1, 1, 0, 272, 240, 5};
    in.targets = t;
    CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && e.mark == 9 &&
          r->slots[0].identity == SOTN_SWORD_CIRCLE && !r->slots[0].step &&
          r->slots[0].follow == 9 && !r->target_cursor);
    // The far screen edge (272,240) is still eligible.
    *r = *idle;
    const SotnSwordTarget first = t[8];
    t[8] = (SotnSwordTarget){0};
    CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && e.mark == 10);
    t[8] = first;
    // A cutscene holds the search.
    *r = *idle;
    in.cutscene = true;
    CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && !e.mark &&
          r->slots[0].identity == SOTN_SWORD_DEFAULT);
    in.cutscene = false;
    // Only marked targets: the next one from the cursor, wrapping, without a new mark.
    t[8].flags = 0x80000;
    t[9] = (SotnSwordTarget){0};
    t[100] = (SotnSwordTarget){1, 1, 0x80000, 50, 50, 5};
    const Sint32 cursors[3][3] = {{101, 9, 9}, {100, 101, 101}, {9, 101, 101}};
    for (Uint16 k = 0; k < 3; ++k)
    {
        *r = *idle;
        r->target_cursor = cursors[k][0];
        CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && !e.mark &&
              r->slots[0].follow == cursors[k][1] && r->target_cursor == cursors[k][2] % 128);
    }
    // Level 70 and above dives.
    *r = *idle;
    in.stats = (SotnFamiliarStats){75, 7400, 1};
    CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && r->slots[0].identity == SOTN_SWORD_DIVE);
    SDL_free(idle);
    SDL_free(r);
    return true;
}
// The circle: two rands pick a point, it turns, flies there, waits, sweeps twice slashing, waits,
// then keeps or drops its target.
static bool CircleTest(void)
{
    SotnSwordRuntime *r = SDL_calloc(1, sizeof(*r));
    CHECK(r != NULL);
    SotnSwordInput in = Input(30, 1, 4, 0);
    static SotnSwordTarget t[SOTN_SWORD_TARGETS];
    SDL_zeroa(t);
    t[20] = (SotnSwordTarget){1, 1, 0, 180, 120, 5};
    in.targets = t;
    in.tilemap_x = 0x900;
    in.tilemap_y = 0x204;
    in.random = 0x13572468U;
    SotnSwordEvents e = {0};
    CHECK(SotnSwordRuntime_Init(r, in, 0, &e) && Run(r, in, &e) &&
          SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && r->slots[0].identity == SOTN_SWORD_CIRCLE);
    t[20].flags |= 0x80000;
    CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && r->slots[0].step == 1 &&
          r->slots[0].flags == 0x0c820000U && r->slots[0].camera_x == 0x900 &&
          r->slots[0].camera_y == 0x204 && !e.random_calls);
    Uint32 random = in.random;
    const Uint16 r1 = SotnCombat_Random(&random), r2 = SotnCombat_Random(&random);
    CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && e.random_calls == 2 && e.random == random &&
          r->circle[4] == (Sint32)(r1 % 128 + 64 + 0x900) &&
          r->circle[5] == (Sint32)(r2 % 128 + 64 + 0x204) && r->slots[0].swing &&
          r->slots[0].step == 2);
    in.random = e.random;
    Uint16 slashes = 0, sweeps = 0, guard = 0;
    Sint16 angle = 0;
    bool voiced = false;
    while (r->slots[0].step != 8 && ++guard < 2000)
    {
        const Uint16 step = r->slots[0].step;
        const SotnSwordSlot before = r->slots[0];
        if (step == 4 && r->circle[1] == 3)
        {
            // Force a voice: the swing's first rand()%16 is zero, which plays table[1].
            Uint32 pick = 1;
            for (;; ++pick)
            {
                Uint32 st = pick;
                if (SotnCombat_Random(&st) % 16 == 0)
                    break;
            }
            in.random = pick;
        }
        const Uint32 seed = in.random;
        CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e));
        in.random = e.random;
        const SotnSwordSlot *s = &r->slots[0];
        if (step == 2)
            CHECK(s->yaw == SotnFairy_StepAngle(s->heading, before.yaw_previous, 32));
        if (step == 3)
        {
            const Sint32 tx = r->circle[4] - 0x900, ty = r->circle[5] - 0x204;
            CHECK(s->velocity_x == (Sint32)(((Sint64)tx * 65536 - before.x) >> 4));
            const Sint32 dx = tx - (s->x >> 16), dy = ty - (s->y >> 16);
            CHECK((s->step == 4) == (dx * dx + dy * dy <= 3));
        }
        if (step == 4 && s->step == 5)
        {
            CHECK(e.sound_count >= 1 && e.sounds[0] == 0x672 && r->circle[1] == 4);
            angle = r->rotation[2];
        }
        if (step == 5 || step == 6)
        {
            CHECK(r->rotation[2] == ((angle + 128) & 0xfff));
            angle = r->rotation[2];
            sweeps += s->step != step;
        }
        if (s->step >= 5 && s->step < 7)
        {
            CHECK(e.hitbox_count == 2);
            for (Uint16 i = 0; i < 2; ++i)
                CHECK(e.hitboxes[i].parameter == i && e.hitboxes[i].spell == 19 &&
                      e.hitboxes[i].x == r->tips[i].x && e.hitboxes[i].y == r->tips[i].y &&
                      e.hitboxes[i].priority == s->priority);
            // One voice roll per swing: rand()%16 below 2 picks table[1] (0) or [0] (1).
            if (before.swing)
            {
                Uint32 state = seed;
                const Uint16 roll = SotnCombat_Random(&state) % 16, sweep = step == 4;
                CHECK(e.random_calls == 1 && !s->swing && e.sound_count == sweep + (roll < 2) &&
                      (roll >= 2 || e.sounds[sweep] == (roll ? 0x700 : 0x701)));
                voiced = roll == 0 && e.sounds[1] == 0x701;
            }
            else
                CHECK(!e.random_calls);
            ++slashes;
        }
        else
            CHECK(!e.hitbox_count);
    }
    CHECK(guard < 2000 && slashes == 64 && sweeps == 2 && r->circle[1] == 4 && voiced);
    // Step 8 keeps a valid target (back to 1), drops on a cutscene or with no target left.
    const SotnSwordRuntime at8 = *r;
    CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && r->slots[0].step == 1 &&
          r->slots[0].identity == SOTN_SWORD_CIRCLE);
    *r = at8;
    in.cutscene = true;
    CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && r->slots[0].identity == SOTN_SWORD_DEFAULT &&
          !r->slots[0].step && r->slots[0].follow == 21);
    in.cutscene = false;
    // HP 0 fails CheckEntityValid but not the search: the marked target is found again.
    *r = at8;
    t[20].hit_points = 0;
    CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && r->slots[0].step == 1 &&
          r->target_cursor == 21 && r->slots[0].follow == 21);
    *r = at8;
    t[20].hitbox_state = 0;
    CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && r->slots[0].identity == SOTN_SWORD_DEFAULT &&
          !r->slots[0].follow);
    // The model stays leased while attacking; Sword Brothers still fails explicitly.
    *r = at8;
    SotnSwordLease leases[3] = {0};
    CHECK(SotnSwordRuntime_Leases(r, leases) && leases[0].count == 96 &&
          leases[0].generation == r->slots[0].generation);
    in.brothers = true;
    const SotnSwordRuntime held = *r;
    CHECK(!SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && !SDL_memcmp(&held, r, sizeof(held)));
    SDL_free(r);
    return true;
}
// The dive: two birth rands, a rise behind the player, a hold, then dives through the target
// whose blade slashes only near depth zero.
static bool DiveTest(void)
{
    SotnSwordRuntime *r = SDL_calloc(1, sizeof(*r));
    CHECK(r != NULL);
    SotnSwordInput in = Input(75, 1, 4, 0);
    static SotnSwordTarget t[SOTN_SWORD_TARGETS];
    SDL_zeroa(t);
    t[3] = (SotnSwordTarget){1, 1, 0, 200, 100, 5};
    in.targets = t;
    in.tilemap_x = 0x300;
    in.tilemap_y = 0x100;
    in.random = 0x2468aceU;
    SotnSwordEvents e = {0};
    CHECK(SotnSwordRuntime_Init(r, in, 0, &e) && Run(r, in, &e) &&
          SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && r->slots[0].identity == SOTN_SWORD_DIVE);
    t[3].flags |= 0x80000;
    Uint32 random = in.random;
    const Uint16 r1 = SotnCombat_Random(&random), r2 = SotnCombat_Random(&random);
    CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && e.random_calls == 2 &&
          r->dive[7] == r1 - 0x4000 && r->dive[9] == r2 - 0x4000 && r->slots[0].step == 1 &&
          r->dive[10] == (0x300 - 0x80) * 32 && r->dive[11] == (0x100 - 0x80) * 32);
    in.random = e.random;
    // Case 1 falls through into the first steering update.
    in.player.facing_left = true;
    CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && r->slots[0].step >= 2 &&
          r->dive[0] == 0x800 + r->dive[10] && r->dive[1] == -0x400 + r->dive[11] &&
          r->dive[2] == -0x400);
    Uint16 slashes = 0, wings = 0, guard = 0;
    bool glided = false, homed = false;
    while (++guard < 1500 && !(glided && r->slots[0].step == 1))
    {
        const Uint16 step = r->slots[0].step;
        if (step == 5)
            r->dive[7] = 0; // A zero planar speed counts as rightward.
        CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e));
        in.random = e.random;
        const SotnSwordSlot *s = &r->slots[0];
        CHECK(s->identity == SOTN_SWORD_DIVE);
        if (step == 2)
        {
            SotnMathTrig trig = {0};
            CHECK(SotnMath_Trig((SotnDataView){exe, sizeof(exe)}, s->yaw, &trig) &&
                  r->dive[7] == (trig.cosine * 160) >> 12 &&
                  r->dive[8] == (trig.sine * 160) >> 12 && (s->step == 3) == (r->dive[12] < 0x200));
        }
        if (step == 5)
            CHECK(r->dive[0] == 0x800 + r->dive[10]);
        CHECK((s->x >> 16) == (Sint16)(r->position[0] / 32 + 128) &&
              (s->y >> 16) == (Sint16)(r->position[1] / 32 + 128));
        const bool near = r->position[2] >= -0x200 && r->position[2] <= 0x200;
        CHECK(e.hitbox_count == (s->step >= 5 && near ? 2 : 0));
        if (e.hitbox_count)
            CHECK(e.hitboxes[0].spell == 20 && e.hitboxes[1].parameter == 1);
        slashes += e.hitbox_count != 0;
        if (step == 5)
        {
            CHECK(e.sound_count >= 1 && e.sounds[0] == 0x6c6);
            ++wings;
        }
        if (step == 9)
        {
            homed = true;
            CHECK(r->dive[0] == ((200 - 0x80) << 5) + r->dive[10] && !r->dive[2]);
        }
        glided |= step == 10;
    }
    CHECK(guard < 1500 && glided && homed && wings == 1 && slashes);
    SDL_free(r);
    return true;
}
// Level 50 with no gift yet: the item and castle flag1 in the same callback, which the default
// sword reads at once (D5); the farewell's two passes, then the card turns off.
static bool GiftTest(void)
{
    SotnSwordRuntime *r = SDL_calloc(1, sizeof(*r));
    CHECK(r != NULL);
    SotnSwordInput in = Input(49, 2, 0, 0);
    SotnSwordEvents e = {0};
    CHECK(SotnSwordRuntime_Init(r, in, 0, &e) && Run(r, in, &e) && Run(r, in, &e));
    // Already given (castle flag 4): reaching 50 again grants nothing.
    SotnSwordRuntime *given = SDL_malloc(sizeof(*given));
    CHECK(given != NULL);
    *given = *r;
    SotnSwordInput again = in;
    again.stats = (SotnFamiliarStats){50, 4900, 2};
    again.castle_flag = 4;
    CHECK(SotnSwordRuntime_UpdateSlot(given, 4, again, &e) && !e.gift_item &&
          !e.castle_flag_write && given->slots[0].identity == SOTN_SWORD_DEFAULT);
    // Only exactly level 50 gives the gift.
    *given = *r;
    again.stats = (SotnFamiliarStats){51, 5000, 2};
    again.castle_flag = 0;
    CHECK(SotnSwordRuntime_UpdateSlot(given, 4, again, &e) && !e.gift_item &&
          given->slots[0].identity == SOTN_SWORD_DEFAULT);
    SDL_free(given);
    in.stats = (SotnFamiliarStats){50, 4900, 2};
    CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && e.gift_item == 126 && e.castle_flag_write &&
          e.castle_flag == 2 && r->slots[0].identity == SOTN_SWORD_GIFT && !r->slots[0].step);
    in.castle_flag = 2;
    // The level stays: no second gift.
    CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && !e.gift_item && r->slots[0].step == 1);
    Uint16 sounds = 0, guard = 0, passes = 0;
    bool dismissed = false;
    while (!dismissed && ++guard < 600)
    {
        const Uint16 step = r->slots[0].step;
        const SotnSwordSlot before = r->slots[0];
        CHECK(Run(r, in, &e));
        if ((step == 2 || step == 3) && r->slots[0].identity)
        {
            const SotnSwordSlot *g = &r->slots[0];
            SotnMathTrig yaw = {0}, pitch = {0};
            CHECK(SotnMath_Trig((SotnDataView){exe, sizeof(exe)}, g->yaw, &yaw) &&
                  SotnMath_Trig((SotnDataView){exe, sizeof(exe)}, g->pitch, &pitch));
            CHECK(g->yaw == SotnFairy_StepAngle(g->heading, before.yaw_previous, 0x180) &&
                  g->pitch == SotnFairy_StepAngle(g->pitch_target, before.pitch_previous, 0x40) &&
                  r->gift[7] == (yaw.cosine * 256) >> 12 && r->gift[8] == (yaw.sine * 256) >> 12 &&
                  r->gift[9] == (pitch.sine * 256) >> 12 &&
                  (g->step != step) == (r->gift[10] < 0x100));
        }
        sounds += e.sound_count;
        if (step == 2 && r->slots[0].step == 3)
        {
            ++passes;
            CHECK(r->position[0] == 0x1000 && r->position[2] == -0x1000 && r->gift[0] == -0x1000 &&
                  r->gift[1] == -0x1000 && r->gift[2] == -0x1000 &&
                  r->slots[0].yaw_previous == 0xa00 && !r->slots[0].pitch_previous &&
                  (r->slots[0].x >> 16) == 0x1000 / 32 + 128);
        }
        if (step == 1)
            CHECK(r->gift[0] == 0x1000 && r->gift[2] == -0x1000);
        dismissed = e.dismiss;
    }
    CHECK(dismissed && passes == 1 && sounds >= 2);
    for (Uint16 i = 0; i < 4; ++i)
        CHECK(!r->slots[i].identity);
    SotnSwordLease leases[3] = {0};
    CHECK(SotnSwordRuntime_Leases(r, leases) && !leases[0].count && !leases[1].count);
    SDL_free(r);
    return true;
}
int main(void)
{
    Fixture();
    return InitTest() && EntranceTest() && FollowTest() && EventsTest() && EntranceBandsTest() &&
                   LifecycleTest() && LeaseTest() && SearchTest() && CircleTest() && DiveTest() &&
                   GiftTest()
               ? 0
               : 1;
}
