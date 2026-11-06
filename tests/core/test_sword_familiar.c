#include <SDL3/SDL.h>
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
static SotnSwordInput Input(Sint32 level, Sint32 loads, Uint8 castle, Uint16 room_flags)
{
    return (SotnSwordInput){
        .overlay = {overlay, sizeof(overlay)},
        .executable = {exe, sizeof(exe)},
        .player = {.x = 120 * 65536, .y = 150 * 65536, .priority = 0x94, .collision = 1},
        .stats = {level, (level - 1) * 100, loads},
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
    // Targets, Sword Brothers and the level-50 gift fail explicitly without mutation.
    SotnSwordRuntime before = *r;
    in.target = true;
    CHECK(!SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && !SDL_memcmp(r, &before, sizeof(*r)));
    in.target = false;
    in.brothers = true;
    CHECK(!SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && !SDL_memcmp(r, &before, sizeof(*r)));
    in.brothers = false;
    in.castle_flag = 1;
    CHECK(SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && r->slots[0].identity == SOTN_SWORD_GIFT &&
          e.castle_flag_write && e.castle_flag == 2);
    before = *r;
    CHECK(!SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && !SDL_memcmp(r, &before, sizeof(*r)));
    SDL_zerop(r);
    in = Input(50, 2, 0, 0);
    CHECK(SotnSwordRuntime_Init(r, in, 0, &e) && SotnSwordRuntime_UpdateSlot(r, 4, in, &e));
    before = *r;
    CHECK(!SotnSwordRuntime_UpdateSlot(r, 4, in, &e) && !SDL_memcmp(r, &before, sizeof(*r)));
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
int main(void)
{
    Fixture();
    return InitTest() && EntranceTest() && FollowTest() && EventsTest() && EntranceBandsTest() &&
                   LifecycleTest()
               ? 0
               : 1;
}
