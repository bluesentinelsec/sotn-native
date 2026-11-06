// Original US tt_004 Sword familiar (servant/tt_004/sword.c and the shared servant helpers,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5): ServantInit, the default follow,
// the menu-switch entrance, the GTE-projected 3D model, the blade trail and level-90 halo, the
// room-event entity and the narrative sound cursor. Private model/UV/CLUT/event tables are read
// from the user's TT_004 image; none are reproduced here.
#include <sotn/combat.h>
#include <sotn/game_math.h>
#include <sotn/sword_familiar.h>

static const Uint32 BASE = 0x80170000U;
static const Uint32 LEVEL_TABLE = 0x801700a0U;
static const Uint32 SMALL_VERTICES = 0x80170224U;
static const Uint32 SMALL_FACES = 0x80170354U;
static const Uint32 SMALL_UV0 = 0x80170594U;
static const Uint32 SMALL_UV1 = 0x801708f4U;
static const Uint32 LARGE_VERTICES = 0x80170c54U;
static const Uint32 LARGE_FACES = 0x80170de4U;
static const Uint32 LARGE_UV0 = 0x801710e4U;
static const Uint32 LARGE_UV1 = 0x80171564U;
static const Uint32 BILLBOARD = 0x80170118U;
static const Uint32 EDGE = 0x80170138U;
static const Uint32 HALO = 0x80170148U;
static const Uint32 TIPS = 0x801701c8U;
static const Uint32 SERVANT_CLUT = 0x80170040U;
static const Uint32 SWORD_CLUT = 0x801701d8U;
static const Uint32 VOICES = 0x80170090U;      // D_us_80170090: seven slash/voice effects
static const Uint32 CIRCLE_HOLD = 0x80170218U; // D_us_80170218 (30)
static const Uint32 DIVE_HOLD = 0x8017021cU;   // D_us_8017021C (60)
static const Uint32 DIVE_GLIDE = 0x80170220U;  // D_us_80170220 (15)
static const Uint16 SFX_SWEEP = 0x672;         // SFX_NOISE_SWEEP_DOWN_A
static const Uint16 SFX_WING = 0x6c6;          // SFX_BOSS_WING_FLAP
static const Uint16 SFX_GIFT = 0x63f;          // SFX_ANIME_SWORD_A
static const Uint16 SFX_SWITCH = 0x69d;        // SFX_MAGIC_SWITCH
static const Uint32 CAMERA_LOCKED = 0x08000000U;
static const Uint32 KEEP_ALIVE = 0x04000000U;

static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? (Sint32)bits : (Sint32)bits - 65536);
}
static Sint32 Long(Uint32 value)
{
    return value <= SDL_MAX_SINT32 ? (Sint32)value : (Sint32)((Sint64)value - 0x100000000LL);
}
static Sint16 Pixel(Sint32 value)
{
    return Word(Long((Uint32)value >> 16));
}
// Portable arithmetic right shift matching the MIPS sra.
static Sint64 Asr(Sint64 v, unsigned int n)
{
    return v >= 0 ? v >> n : -(Sint64)(((Uint64)(-(v + 1)) >> n) + 1U);
}
static Sint32 Fix(Sint32 pixels)
{
    return Long((Uint32)pixels << 16);
}
static bool View(SotnDataView o, Uint32 address, Uint32 size, const Uint8 **out)
{
    if (!o.bytes || address < BASE || address - BASE > o.size || o.size - (address - BASE) < size)
        return SDL_SetError("Sword familiar private overlay data is out of bounds");
    *out = o.bytes + (address - BASE);
    return true;
}
static Sint16 Read16(const Uint8 *p)
{
    return Word((Sint32)(p[0] | (Uint32)p[1] << 8));
}
static Sint32 Read32(const Uint8 *p)
{
    return Long(p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24);
}
/** D_us_801700A0[level/10]: unk0 (level70 attack mode), unk4 (model), unk8 (aura bits). */
static bool Level(SotnDataView o, Sint32 level, Sint32 out[3])
{
    const Uint8 *p = NULL;
    if (level < 1 || level > 99 || !View(o, LEVEL_TABLE + (Uint32)(level / 10) * 12U, 12, &p))
        return SDL_SetError("Sword familiar level row requires level1..99 and its private table");
    for (Uint16 i = 0; i < 3; ++i)
        out[i] = Read32(p + i * 4U);
    return true;
}
static bool Trig(SotnDataView exe, Sint32 angle, Sint32 *sine, Sint32 *cosine)
{
    SotnMathTrig t = {0};
    if (!SotnMath_Trig(exe, angle, &t))
        return false;
    if (sine)
        *sine = t.sine;
    if (cosine)
        *cosine = t.cosine;
    return true;
}
static Sint32 Abs(Sint32 v)
{
    return v < 0 ? Long(0U - (Uint32)v) : v;
}
static Sint32 Add(Sint32 a, Sint32 b)
{
    return Long((Uint32)a + (Uint32)b);
}
static Sint32 Sub(Sint32 a, Sint32 b)
{
    return Long((Uint32)a - (Uint32)b);
}
static Uint16 Rand(SotnSwordEvents *e)
{
    ++e->random_calls;
    return SotnCombat_Random(&e->random);
}
static bool Sound(SotnSwordEvents *e, Uint16 id)
{
    if (e->sound_count >= SDL_arraysize(e->sounds))
        return SDL_SetError("Sword familiar callback exceeded its original sound requests");
    e->sounds[e->sound_count++] = id;
    return true;
}
static bool OverlayWord(SotnDataView o, Uint32 address, Sint32 *out)
{
    const Uint8 *p = NULL;
    if (!View(o, address, 4, &p))
        return false;
    *out = Read32(p);
    return true;
}

// One original statsFamiliars record: level = EXP/100 + 1, EXP0..9899, loads0..9999.
static bool Stats(SotnFamiliarStats s)
{
    return s.experience >= 0 && s.experience <= 9899 && s.level == s.experience / 100 + 1 &&
           s.loads >= 0 && s.loads <= 9999;
}
bool SotnSword_Clut(SotnDataView overlay, Uint16 out[SOTN_SWORD_CLUT_WORDS])
{
    const Uint8 *a = NULL, *b = NULL;
    if (!out || !View(overlay, SERVANT_CLUT, 32, &a) || !View(overlay, SWORD_CLUT, 64, &b))
        return SDL_SetError("Sword familiar CLUT requires its private servant/sword palettes");
    for (Uint16 i = 0; i < 16; ++i)
        out[i] = (Uint16)Read16(a + i * 2U);
    for (Uint16 i = 0; i < 32; ++i)
        out[16 + i] = (Uint16)Read16(b + i * 2U);
    return true;
}

// GTE UNR divider (H/SZ3, saturated at 0x1FFFF) and saturated screen coordinates.
static Uint32 Divide(Uint32 depth, Uint32 distance)
{
    if (depth * 2U <= distance)
        return 0x1ffff;
    Uint32 shift = 0, d = depth;
    while (d < 0x8000)
    {
        d *= 2;
        ++shift;
    }
    const Uint32 n = distance << shift, index = (d - 0x7fc0) >> 7;
    const Sint32 estimate = (Sint32)(((0x40000U / (index + 0x100) + 1) / 2)) - 0x101;
    const Uint32 u = (Uint32)SDL_clamp(estimate, 0, 255) + 0x101;
    d = (0x2000080U - d * u) >> 8;
    d = (0x80U + d * u) >> 8;
    return (Uint32)SDL_min(0x1ffffULL, ((Uint64)n * d + 0x8000) >> 16);
}
typedef struct Matrix
{
    Sint32 m[3][3], t[3];
} Matrix;
// func_us_80172C8C: the overlay's own rotation matrix (FLT_TO_I shifts each product).
static bool Rotation(SotnDataView exe, const Sint16 r[3], Matrix *out)
{
    Sint32 sx = 0, sy = 0, sz = 0, cx = 0, cy = 0, cz = 0;
    if (!Trig(exe, r[0], &sx, &cx) || !Trig(exe, r[1], &sy, &cy) || !Trig(exe, r[2], &sz, &cz))
        return false;
    Matrix m = {0};
    const Sint32 a = (Sint32)Asr((Sint64)cz * sy, 12), b = (Sint32)Asr((Sint64)sz * sy, 12);
    m.m[0][0] = (Sint32)Asr((Sint64)cz * cy, 12);
    m.m[0][1] = (Sint32)(Asr((Sint64)-sz * cx, 12) + Asr((Sint64)sx * a, 12));
    m.m[0][2] = (Sint32)(Asr((Sint64)sz * sx, 12) + Asr((Sint64)cx * a, 12));
    m.m[1][0] = (Sint32)Asr((Sint64)sz * cy, 12);
    m.m[1][1] = (Sint32)(Asr((Sint64)cz * cx, 12) + Asr((Sint64)sx * b, 12));
    m.m[1][2] = (Sint32)(Asr((Sint64)-cz * sx, 12) + Asr((Sint64)cx * b, 12));
    m.m[2][0] = (Sint32)Asr(-(Sint64)sy, 12);
    m.m[2][1] = (Sint32)Asr((Sint64)cy * sx, 12);
    m.m[2][2] = (Sint32)Asr((Sint64)cy * cx, 12);
    // MATRIX stores signed words.
    for (Uint16 i = 0; i < 3; ++i)
        for (Uint16 k = 0; k < 3; ++k)
            m.m[i][k] = Word(m.m[i][k]);
    *out = m;
    return true;
}
// RotTransPers (RTPS) with SetGeomOffset(128,128) and gte_SetGeomScreen(1000): screen SXY
// and gte_stszotz's SZ3>>2.
static void Project(const Matrix *m, const Uint8 *vertex, SotnSwordPoint *out, Sint32 *otz)
{
    const Sint32 v[3] = {Read16(vertex), Read16(vertex + 2), Read16(vertex + 4)};
    Sint64 mac[3] = {0};
    for (Uint16 row = 0; row < 3; ++row)
        mac[row] = m->t[row] + Asr((Sint64)m->m[row][0] * v[0] + (Sint64)m->m[row][1] * v[1] +
                                       (Sint64)m->m[row][2] * v[2],
                                   12);
    const Uint32 sz = (Uint32)SDL_clamp(mac[2], 0, 0xffff);
    const Uint32 ratio = Divide(sz, 1000);
    const Sint64 ir[2] = {SDL_clamp(mac[0], -32768, 32767), SDL_clamp(mac[1], -32768, 32767)};
    out->x = (Sint16)SDL_clamp(128 + Asr((Sint64)ratio * ir[0], 16), -1024, 1023);
    out->y = (Sint16)SDL_clamp(128 + Asr((Sint64)ratio * ir[1], 16), -1024, 1023);
    *otz = (Sint32)(sz >> 2);
}
// GTE NCLIP over three packed SXY words.
static Sint64 Clip(SotnSwordPoint a, SotnSwordPoint b, SotnSwordPoint c)
{
    return (Sint64)a.x * b.y + (Sint64)b.x * c.y + (Sint64)c.x * a.y - (Sint64)a.x * c.y -
           (Sint64)b.x * a.y - (Sint64)c.x * b.y;
}

static bool Free(const SotnSwordRuntime *r, Uint16 identity, Uint16 *slot)
{
    for (Uint16 i = 1; i < 4; ++i)
        if (r->slots[i].identity == identity)
        {
            *slot = i;
            return true;
        }
    return false;
}
static void Birth(SotnSwordRuntime *r, Uint16 index, const SotnSwordSlot *parent, Uint16 identity,
                  Uint16 parameter)
{
    r->slots[index] = (SotnSwordSlot){.generation = ++r->generation,
                                      .identity = identity,
                                      .parameter = parameter,
                                      .priority = parent->priority,
                                      .facing_left = parent->facing_left,
                                      .flags = KEEP_ALIVE,
                                      .x = parent->x,
                                      .y = parent->y};
}
// func_us_80172420: an aura in the first empty slot5..7, unless one of 5..7 already holds it.
static bool Aura(SotnSwordRuntime *r, Uint16 identity)
{
    Uint16 i = 1;
    for (; i < 4; ++i)
    {
        if (r->slots[i].identity == identity)
            return true;
        if (!r->slots[i].identity)
            break;
    }
    if (i == 4)
        return true;
    Uint16 other = 0;
    if (Free(r, identity, &other))
        return SDL_SetError("Sword familiar duplicate aura is not a supported original lease");
    Birth(r, i, &r->slots[0], identity, 0);
    return true;
}
// CreateEventEntity: the first empty slot5..7, or nothing.
static void Event(SotnSwordRuntime *r, const SotnSwordSlot *parent, Uint16 identity,
                  Uint16 parameter)
{
    for (Uint16 i = 1; i < 4; ++i)
        if (!r->slots[i].identity)
        {
            Birth(r, i, parent, identity, parameter);
            return;
        }
}

bool SotnSwordRuntime_Init(SotnSwordRuntime *runtime, SotnSwordInput in, Uint16 mode,
                           SotnSwordEvents *out)
{
    if (!runtime || !out || (mode != 0 && mode != 1 && mode != 3) || !Stats(in.stats))
        return SDL_SetError("Sword familiar init requires its actual owner, mode and stats");
    SotnSwordRuntime r = *runtime;
    SotnSwordEvents e = {.random = in.random};
    if (!r.initialized)
    {
        if (!SotnFamiliarEvents_Init(&r.events, in.event_source))
            return false;
        // A freshly loaded overlay's data: g_PlaySfxStep starts at 99, pauseAllowed is DRA's.
        r.sound_step = 99;
        r.initialized = true;
    }
    if (mode == 1 || mode == 3)
    {
        if (!SotnFamiliarEvents_Reset(&r.events))
            return false;
        if (mode == 3)
        {
            *runtime = r;
            *out = e;
            return true;
        }
    }
    e.clut_upload = true;
    SotnSwordSlot *s = &r.slots[0];
    *s = (SotnSwordSlot){.generation = ++r.generation,
                         .priority = (Uint16)(in.player.priority - 2U),
                         .facing_left = !in.player.facing_left};
    SDL_memset(r.model, 0, sizeof(r.model));
    if (mode == 1)
    {
        if (in.room_flags & 0x60U)
        {
            s->identity = SOTN_SWORD_DEFAULT;
            if (in.castle_flag == 1 || in.castle_flag == 2)
            {
                e.castle_flag = 4;
                e.castle_flag_write = true;
            }
        }
        else
            s->identity = SOTN_SWORD_ENTRANCE;
        s->x = Fix(128);
        s->y = Fix(-32);
    }
    else
    {
        s->identity = SOTN_SWORD_DEFAULT;
        if (in.room_flags & 0x20U)
        {
            const bool castle2 = in.stage >= 0x20 && in.stage < 0x35;
            const Uint16 side = in.room_flags == 0x22   ? (castle2 ? 1 : 0)
                                : in.room_flags == 0x20 ? (castle2 ? 0 : 1)
                                                        : 2;
            s->x = Fix(side ? 192 : 64);
            s->y = Fix(144);
        }
        else
        {
            s->x = Long((Uint32)in.player.x + (Uint32)Fix(in.player.facing_left ? 28 : -28));
            s->y = Long((Uint32)in.player.y - (Uint32)Fix(28));
        }
    }
    r.position[0] = Long((Uint32)(Pixel(s->x) - 128) << 5);
    r.position[1] = Long((Uint32)(Pixel(s->y) - 128) << 5);
    r.position[2] = 0;
    r.stats = in.stats;
    Sint32 row[3] = {0};
    if (!Level(in.overlay, r.stats.level, row) || ((row[2] & 1) && !Aura(&r, SOTN_SWORD_TRAIL)) ||
        ((row[2] & 2) && !Aura(&r, SOTN_SWORD_HALO)))
        return false;
    r.entrance_cancel = r.near_player = r.halo_fresh = r.edge_fresh = false;
    *runtime = r;
    *out = e;
    return true;
}

// CheckSwordLevel: a level change spawns the missing auras; reaching 50 grants item126.
static bool CheckLevel(SotnSwordRuntime *r, const SotnSwordSlot *s, SotnSwordInput *input,
                       SotnSwordEvents *e)
{
    const SotnSwordInput in = *input;
    if (!s->step)
        return true;
    r->stats = in.stats;
    if (r->level_seen == r->stats.level)
        return true;
    r->level_seen = r->stats.level;
    Sint32 row[3] = {0};
    Uint16 found = 0;
    if (!Level(in.overlay, r->stats.level, row) ||
        ((row[2] & 1) && !Free(r, SOTN_SWORD_TRAIL, &found) && !Aura(r, SOTN_SWORD_TRAIL)) ||
        ((row[2] & 2) && !Free(r, SOTN_SWORD_HALO, &found) && !Aura(r, SOTN_SWORD_HALO)))
        return false;
    // AddToInventory(ITEM_SWORD_FAMILIAR, EQUIP_HAND); the castle flag is read again later in
    // this callback.
    if (r->stats.level == 50 && !in.castle_flag)
    {
        e->gift_item = SOTN_SWORD_FAMILIAR_ITEM;
        e->castle_flag = input->castle_flag = 1;
        e->castle_flag_write = true;
    }
    return true;
}
// func_us_8017259C: model UVs, CLUT, drawMode and the face index triples.
static bool Faces(SotnSwordRuntime *r, Sint16 model, Sint16 variant, SotnDataView o)
{
    const Uint16 count = model ? 96 : 72;
    const Uint32 uv = model ? (variant ? LARGE_UV1 : LARGE_UV0) : (variant ? SMALL_UV1 : SMALL_UV0);
    const Uint8 *p = NULL;
    if (!View(o, uv, count * 12U, &p))
        return false;
    for (Uint16 i = 0; i < count; ++i, p += 12)
    {
        SotnPlayerEffectQuad *q = &r->model[i];
        q->u[0] = (Uint8)Read16(p);
        q->v[0] = (Uint8)Read16(p + 2);
        q->u[1] = (Uint8)Read16(p + 4);
        q->v[1] = (Uint8)Read16(p + 6);
        q->u[2] = q->u[3] = (Uint8)Read16(p + 8);
        q->v[2] = q->v[3] = (Uint8)Read16(p + 10);
        q->palette = 0x140;
        q->draw_mode = 0x10a;
    }
    for (Uint16 i = count; i < SOTN_SWORD_MODEL_ROWS; ++i)
        r->model[i].draw_mode |= 8;
    return true;
}
// func_us_80172940: first initialization reserves the 96-row model; later ones set flags.
static bool Ready(SotnSwordRuntime *r, SotnSwordSlot *s, SotnSwordInput in, bool *removed)
{
    if (!s->kind)
    {
        if (s->identity == SOTN_SWORD_DEFAULT || s->identity == SOTN_SWORD_ENTRANCE)
        {
            if (!in.allocated)
            {
                *removed = true;
                return true;
            }
            s->flags = 0x0c820000U;
            for (Uint16 i = 0; i < SOTN_SWORD_MODEL_ROWS; ++i)
                r->model[i] =
                    (SotnPlayerEffectQuad){.page = 0x1b, .palette = 0x140, .draw_mode = 0x108};
            r->stats = in.stats;
            Sint32 row[3] = {0};
            if (!Level(in.overlay, r->stats.level, row))
                return false;
            s->model = Word(row[1]);
            s->variant = 1;
            if (!Faces(r, s->model, s->variant, in.overlay))
                return false;
            s->yaw = s->yaw_previous = 0x400;
            s->pitch = s->pitch_previous = 0;
            r->level_seen = 0;
            r->model_ready = true;
            ++s->step;
        }
    }
    else if (s->identity == SOTN_SWORD_DEFAULT)
    {
        s->flags = 0x0c820000U;
        ++s->step;
    }
    else if (s->identity == SOTN_SWORD_CIRCLE || s->identity == SOTN_SWORD_DIVE)
    {
        s->flags = 0x0c820000U;
        s->camera_x = in.tilemap_x;
        s->camera_y = in.tilemap_y;
        ++s->step;
    }
    else if (s->identity == SOTN_SWORD_GIFT || s->identity == SOTN_SWORD_BROTHERS)
    {
        s->flags = 0x04820000U;
        ++s->step;
    }
    else
        return SDL_SetError("Sword familiar state %#x has no initialization", s->identity);
    s->kind = s->identity;
    r->entrance_cancel = false;
    return true;
}
// func_us_80172E84: project the model, blade edge, halo quads and tips.
static bool Render(SotnSwordRuntime *r, Sint16 model, SotnSwordInput in, bool leased)
{
    Matrix m = {0};
    if (!Rotation(in.executable, r->rotation, &m))
        return false;
    // TransMatrix (vx/5, vy/5, vz), then CompMatrix with RotMatrix(0)'s identity and (0,0,6500).
    m.t[0] = r->position[0] / 5;
    m.t[1] = r->position[1] / 5;
    m.t[2] = r->position[2] + 6500;
    const Sint32 base = r->near_player       ? (r->position[2] < 0   ? in.player.priority + 11
                                                : r->position[2] > 0 ? in.player.priority - 9
                                                                     : in.player.priority - 1)
                        : r->position[2] < 0 ? 0x9f
                        : r->position[2] > 0 ? 0x8b
                                             : 0x93;
    const Uint8 *p = NULL;
    Sint32 otz = 0;
    // After a failed allocation the original keeps projecting through the destroyed entity's
    // zero primIndex into another owner's packets; those out-of-lease writes are omitted.
    SotnPlayerEffectQuad scratch[SOTN_SWORD_MODEL_ROWS] = {0};
    SotnPlayerEffectQuad *rows = leased ? r->model : scratch;
    if (!r->model_ready)
    {
        SotnPlayerEffectQuad *q = &rows[0];
        q->priority = (Uint16)(in.player.priority + (r->position[2] < 0 ? 1 : -1));
        q->draw_mode &= 0xfff7U;
        for (Uint16 v = 0; v < 4; ++v)
        {
            SotnSwordPoint point = {0};
            if (!View(in.overlay, BILLBOARD + v * 8U, 8, &p))
                return false;
            Project(&m, p, &point, &otz);
            q->x[v] = point.x;
            q->y[v] = point.y;
        }
    }
    else
    {
        const Uint16 vertices = model ? 0x32 : 0x26, faces = model ? 0x60 : 0x48;
        SotnSwordPoint point[0x32] = {0};
        Sint32 depth[0x32] = {0};
        r->scratch_depth = SDL_MAX_SINT32;
        for (Uint16 i = 0; i < vertices; ++i)
        {
            if (!View(in.overlay, (model ? LARGE_VERTICES : SMALL_VERTICES) + i * 8U, 8, &p))
                return false;
            Project(&m, p, &point[i], &depth[i]);
            r->scratch_depth = SDL_min(r->scratch_depth, depth[i]);
        }
        for (Uint16 i = 0; i < faces; ++i)
        {
            if (!View(in.overlay, (model ? LARGE_FACES : SMALL_FACES) + i * 8U, 8, &p))
                return false;
            const Sint16 a = Read16(p), b = Read16(p + 2), c = Read16(p + 4);
            if (a < 0 || b < 0 || c < 0 || a >= vertices || b >= vertices || c >= vertices)
                return SDL_SetError("Sword familiar model face references a missing vertex");
            SotnPlayerEffectQuad *q = &rows[i];
            if (Clip(point[a], point[b], point[c]) >= 0)
            {
                q->x[0] = point[a].x;
                q->y[0] = point[a].y;
                q->x[1] = point[b].x;
                q->y[1] = point[b].y;
                q->x[2] = q->x[3] = point[c].x;
                q->y[2] = q->y[3] = point[c].y;
                const Sint32 far = SDL_max(depth[a], SDL_max(depth[b], depth[c]));
                q->priority = (Uint16)(base - (far - r->scratch_depth) / 9);
                q->draw_mode &= 0xfff7U;
            }
            else
                q->draw_mode |= 8;
        }
    }
    for (Uint16 i = 0; i < 2; ++i)
    {
        if (!View(in.overlay, EDGE + i * 8U, 8, &p))
            return false;
        Project(&m, p, &r->edge[i], &otz);
    }
    r->edge_priority = base - 10;
    for (Uint16 k = 0; k < 4; ++k)
    {
        Sint32 far = 0;
        for (Uint16 v = 0; v < 4; ++v)
        {
            if (!View(in.overlay, HALO + (k * 4U + v) * 8U, 8, &p))
                return false;
            Project(&m, p, &r->halo_points[k][v], &otz);
            far = v ? SDL_max(far, otz) : otz;
        }
        r->halo_priority[k] = base - (far - r->scratch_depth) / 9 + 1;
    }
    r->edge_fresh = r->halo_fresh = true;
    for (Uint16 i = 0; i < 2; ++i)
    {
        if (!View(in.overlay, TIPS + i * 8U, 8, &p))
            return false;
        Project(&m, p, &r->tips[i], &otz);
    }
    return true;
}
// The shared follow arithmetic of UpdateServantDefault and the entrance: velocity by distance,
// the heading/yaw steering and the pitch from the model depth.
static bool Follow(SotnSwordRuntime *r, SotnSwordSlot *s, SotnSwordInput in, Sint32 tx, Sint32 ty,
                   const Sint32 limits[3], const Uint16 shifts[4], Sint32 *distance)
{
    Sint32 dx = (Sint32)Asr((Sint64)tx - s->x, 16), dy = (Sint32)Asr((Sint64)ty - s->y, 16);
    // FLT_TO_I(SquareRoot12(I_TO_FLT(dx*dx+dy*dy))), with the original 32-bit wrap.
    Uint32 root = 0;
    if (!SotnMath_Sqrt12(in.executable, (Uint32)(dx * dx + dy * dy) << 12, &root))
        return false;
    *distance = Long(root) >> 12;
    const Uint16 k = *distance < limits[0]   ? shifts[0]
                     : *distance < limits[1] ? shifts[1]
                     : *distance < limits[2] ? shifts[2]
                                             : shifts[3];
    s->velocity_x = (Sint32)Asr((Sint64)tx - s->x, k);
    s->velocity_y = (Sint32)Asr((Sint64)ty - s->y, k);
    s->x = Long((Uint32)s->x + (Uint32)s->velocity_x);
    s->y = Long((Uint32)s->y + (Uint32)s->velocity_y);
    if (*distance > 16)
    {
        if (!SotnMath_Atan2Vector(in.executable, dy, dx, &s->heading))
            return false;
        s->heading = (Sint16)(s->heading & 0xfff);
        s->yaw = SotnFairy_StepAngle(s->heading, s->yaw_previous, 8);
    }
    else
        s->yaw = SotnFairy_StepAngle(0x400, s->yaw_previous, 8);
    s->yaw_previous = s->yaw;
    r->rotation[2] = Word(s->yaw - 0x400);
    const Sint32 depth = -r->position[2];
    const Sint32 spread = SDL_max(SDL_abs(dx) << 5, SDL_abs(dy) << 5);
    if (!SotnMath_Atan2Vector(in.executable, depth, spread, &s->pitch_target))
        return false;
    s->pitch_target = (Sint16)(s->pitch_target & 0xfff);
    s->pitch = SotnFairy_StepAngle(s->pitch_target, s->pitch_previous, 0x20);
    s->pitch_previous = s->pitch;
    Sint32 sine = 0;
    if (!Trig(in.executable, s->pitch, &sine, NULL))
        return false;
    r->position[2] = Long((Uint32)r->position[2] + (Uint32)(Sint32)Asr((Sint64)sine * 0x60, 12));
    r->rotation[0] = s->pitch;
    return true;
}
static bool RoomEvents(SotnSwordRuntime *r, const SotnSwordSlot *s, SotnSwordInput in)
{
    SotnFamiliarEventPlan plan = {0};
    in.room.occupied = 0;
    for (Uint16 i = 1; i < 4; ++i)
        if (r->slots[i].identity)
            in.room.occupied |= (Uint8)(1U << (i - 1U));
    if (!SotnFamiliarEvents_Tick(&r->events, in.event_source, in.room, &plan))
        return false;
    for (Uint16 i = 0; i < plan.count; ++i)
    {
        const SotnFamiliarEventBirth b = plan.births[i];
        if (b.slot < 5 || b.slot > 7 || r->slots[b.slot - 4U].identity)
            return SDL_SetError("Sword familiar room event lost its actual empty slot");
        Birth(r, (Uint16)(b.slot - 4U), s, b.entity, b.parameter);
    }
    return true;
}
static bool Model(SotnSwordRuntime *r, const SotnSwordSlot *s, SotnSwordInput in, bool leased)
{
    r->position[0] = Long((Uint32)(Pixel(s->x) - 128) << 5);
    r->position[1] = Long((Uint32)(Pixel(s->y) - 128) << 5);
    return Render(r, s->model, in, leased);
}
// func_us_80173AA0: the first live unmarked target (marking it), else the next marked one
// after the round-robin cursor, else none.
static bool Search(SotnSwordRuntime *r, SotnSwordInput in, SotnSwordEvents *e, Uint16 *out)
{
    if (!in.targets)
        return SDL_SetError("Sword familiar target search requires the actual stage snapshot");
    bool marked[SOTN_SWORD_TARGETS] = {0};
    Uint16 count = 0;
    for (Uint16 i = 0; i < SOTN_SWORD_TARGETS; ++i)
    {
        const SotnSwordTarget *t = &in.targets[i];
        if (t->id && t->hitbox_state && !(t->flags & 0x200000U) && t->x >= -16 && t->x <= 272 &&
            t->y <= 240 && t->y >= 0 && t->hit_points < 0x7000)
        {
            if (t->flags & 0x80000U)
            {
                marked[i] = true;
                ++count;
            }
            else
            {
                if (e->mark)
                    return SDL_SetError("Sword familiar callback marked a second target");
                e->mark = (Uint16)(i + 1U);
                *out = (Uint16)(i + 1U);
                return true;
            }
        }
    }
    *out = 0;
    if (count)
    {
        Sint32 index = r->target_cursor % SOTN_SWORD_TARGETS;
        for (Uint16 i = 0; i < SOTN_SWORD_TARGETS; ++i)
        {
            if (index < 0)
                return SDL_SetError("Sword familiar target cursor is negative");
            if (marked[index])
            {
                r->target_cursor = (index + 1) % SOTN_SWORD_TARGETS;
                *out = (Uint16)(index + 1);
                return true;
            }
            index = (index + 1) % SOTN_SWORD_TARGETS;
        }
    }
    return true;
}
// CheckEntityValid (US): hitbox, on screen and 0 < HP < 0x7000; the raw slot keeps no identity.
static bool Valid(SotnSwordInput in, Uint16 follow, bool *out)
{
    if (!follow || follow > SOTN_SWORD_TARGETS || !in.targets)
        return SDL_SetError("Sword familiar attack lost its original target pointer");
    const SotnSwordTarget *t = &in.targets[follow - 1U];
    *out = t->hitbox_state && t->x >= -16 && t->x <= 272 && t->y <= 240 && t->y >= 0 &&
           t->hit_points < 0x7000 && t->hit_points > 0;
    return true;
}
// UpdateServantDefault: follow the player (or a flag20 room side) with a bobbing height.
static bool Default(SotnSwordRuntime *r, SotnSwordSlot *s, SotnSwordInput in, SotnSwordEvents *e,
                    bool *removed)
{
    if (!CheckLevel(r, s, &in, e))
        return false;
    if (in.castle_flag == 1)
    {
        s->identity = SOTN_SWORD_GIFT;
        s->step = 0;
        e->castle_flag = 2;
        e->castle_flag_write = true;
        return true;
    }
    if (in.brothers)
    {
        // g_Player.unk70: the Sword Brothers spell takes over from any state.
        s->identity = SOTN_SWORD_BROTHERS;
        s->step = 0;
        return true;
    }
    r->bob = (r->bob + 16) & 0xfff;
    Sint32 cosine = 0;
    if (!Trig(in.executable, r->bob, NULL, &cosine))
        return false;
    const Sint32 wave = Long((Uint32)cosine << 4) * 16;
    Sint32 tx = 0, ty = 0;
    if (in.room_flags & 0x20U)
    {
        const bool castle2 = in.stage >= 0x20 && in.stage < 0x35;
        const Uint16 side = in.room_flags == 0x22   ? (castle2 ? 1 : 0)
                            : in.room_flags == 0x20 ? (castle2 ? 0 : 1)
                                                    : 2;
        tx = side == 0           ? Fix(0x40)
             : side == 1         ? Fix(0xc0)
             : Pixel(s->x) > 128 ? Fix(0xc0)
                                 : Fix(0x40);
        ty = Long((Uint32)wave + (Uint32)Fix(0x90));
    }
    else
    {
        tx = Long((Uint32)in.player.x + (Uint32)Fix(in.player.facing_left ? 0x1c : -0x1c));
        ty = Long((Uint32)in.player.y - ((Uint32)wave + (Uint32)Fix(0x1c)));
    }
    switch (s->step)
    {
    case 0:
        if (!Ready(r, s, in, removed))
            return false;
        r->bob = 0;
        break;
    default: {
        static const Sint32 limits[3] = {16, 64, 256};
        static const Uint16 shifts[4] = {6, 5, 4, 2};
        Sint32 distance = 0;
        if (!Follow(r, s, in, tx, ty, limits, shifts, &distance))
            return false;
        if (!in.cutscene)
        {
            Sint32 row[3] = {0};
            if (!Search(r, in, e, &s->follow) || !Level(in.overlay, r->stats.level, row))
                return false;
            if (s->follow)
            {
                // Level 70 and above dives; below, it circles.
                s->identity = row[0] ? SOTN_SWORD_DIVE : SOTN_SWORD_CIRCLE;
                s->step = 0;
            }
        }
        break;
    }
    }
    // DestroyEntity only returns from func_us_80172940: the callback continues on the zeroed
    // entity (ProcessEvent, position and projection).
    if (*removed)
        SDL_zero(*s);
    return RoomEvents(r, s, in) && Model(r, s, in, !*removed);
}
// func_us_80176674: the menu-switch entrance; it settles beside the player, then speaks.
static bool Entrance(SotnSwordRuntime *r, SotnSwordSlot *s, SotnSwordInput in, SotnSwordEvents *e,
                     bool *removed)
{
    if (!CheckLevel(r, s, &in, e))
        return false;
    r->entrance_bob = (r->entrance_bob + 16) & 0xfff;
    Sint32 cosine = 0;
    if (!Trig(in.executable, r->entrance_bob, NULL, &cosine))
        return false;
    const Sint32 tx = Long((Uint32)in.player.x + (Uint32)Fix(in.player.facing_left ? -24 : 24));
    const Sint32 ty =
        Long((Uint32)in.player.y - ((Uint32)Long((Uint32)cosine << 4 << 4) + (Uint32)Fix(28)));
    static const Sint32 limits[3] = {60, 100, 100};
    static const Uint16 shifts[4] = {6, 5, 4, 4};
    Sint32 distance = 0;
    if (!Follow(r, s, in, tx, ty, limits, shifts, &distance))
        return false;
    switch (s->step)
    {
    case 0:
        if (!Ready(r, s, in, removed))
            return false;
        if (!*removed && (in.castle_flag == 1 || in.castle_flag == 2))
        {
            e->castle_flag = 3;
            e->castle_flag_write = true;
        }
        break;
    case 1:
        if (SotnFairy_MovementAllowed(in.player, true) || in.enemies || r->entrance_cancel ||
            in.cutscene || in.global_flash)
        {
            s->identity = SOTN_SWORD_DEFAULT;
            s->step = 0;
            if (in.castle_flag == 3)
            {
                e->castle_flag = 4;
                e->castle_flag_write = true;
            }
            return true;
        }
        // The original re-measures with the follow's retained deltas after the move.
        {
            const Sint32 dx = (Sint32)Asr((Sint64)tx - s->x, 16),
                         dy = (Sint32)Asr((Sint64)ty - s->y, 16);
            Uint32 root = 0;
            if (!SotnMath_Sqrt12(in.executable, (Uint32)(dx * dx + dy * dy) << 12, &root))
                return false;
            if ((Long(root) >> 12) < 32)
                ++s->step;
        }
        break;
    case 2:
        e->pause_write = true;
        e->pause_allowed = false;
        r->pause_allowed = false;
        {
            Uint16 parameter = 0x4fb;
            if (r->stats.loads == 1)
                parameter = 0x4fb;
            else if (in.castle_flag == 3)
            {
                parameter = 0x4fe;
                e->castle_flag = 4;
                e->castle_flag_write = true;
            }
            else if (r->stats.level < 50)
                parameter = 0x4fc;
            else
                parameter = 0x4ff;
            Event(r, s, SOTN_SWORD_SOUND, parameter);
        }
        ++s->step;
        break;
    case 3:
        if (r->sound_step == 99)
            ++s->step;
        break;
    default:
        e->pause_write = true;
        e->pause_allowed = true;
        r->pause_allowed = true;
        s->identity = SOTN_SWORD_DEFAULT;
        s->step = 0;
        break;
    }
    // DestroyEntity only returns from func_us_80172940: the callback continues on the zeroed
    // entity (ProcessEvent, position and projection).
    if (*removed)
        SDL_zero(*s);
    return RoomEvents(r, s, in) && Model(r, s, in, !*removed);
}
// The slash: at most one voice roll per swing, then func_us_801724E8's two DE hitboxes at the
// projected tips (GetFreeEntity's exhaustion belongs to the session).
static bool Slash(SotnSwordRuntime *r, SotnSwordSlot *s, SotnSwordInput in, SotnSwordEvents *e)
{
    if (s->swing)
    {
        const Uint16 roll = Rand(e) % 16;
        if (roll < 2)
        {
            const Uint8 *p = NULL;
            if (!View(in.overlay, VOICES + (roll ? 0U : 2U), 2, &p) || !Sound(e, (Uint16)Read16(p)))
                return false;
        }
        s->swing = 0;
    }
    Sint32 row[3] = {0};
    if (!Level(in.overlay, r->stats.level, row))
        return false;
    for (Uint16 i = 0; i < 2; ++i)
        e->hitboxes[e->hitbox_count++] = (SotnSwordHitbox){.parameter = i,
                                                           .spell = row[0] ? 20 : 19,
                                                           .x = r->tips[i].x,
                                                           .y = r->tips[i].y,
                                                           .priority = s->priority,
                                                           .facing_left = s->facing_left};
    return true;
}
// CalculateDistance: SquareRoot12 of the squared whole-pixel distance, with the 32-bit wrap.
static bool Distance(SotnSwordInput in, const SotnSwordSlot *s, Sint32 x, Sint32 y, Sint32 *out)
{
    const Uint32 dx = (Uint32)x - (Uint32)(Sint32)Pixel(s->x),
                 dy = (Uint32)y - (Uint32)(Sint32)Pixel(s->y);
    Uint32 root = 0;
    if (!SotnMath_Sqrt12(in.executable, (dx * dx + dy * dy) << 12, &root))
        return false;
    *out = Long(root) >> 12;
    return true;
}
// Whether the attack keeps its target: the old one if still valid, else a fresh search.
static bool Retarget(SotnSwordRuntime *r, SotnSwordSlot *s, SotnSwordInput in, SotnSwordEvents *e,
                     bool *keep)
{
    bool valid = false;
    if (!Valid(in, s->follow, &valid))
        return false;
    if (!valid && !Search(r, in, e, &s->follow))
        return false;
    *keep = (valid || s->follow) && !in.cutscene;
    if (!*keep)
    {
        s->identity = SOTN_SWORD_DEFAULT;
        s->step = 0;
    }
    return true;
}
// func_us_801746BC: below level 70 the sword flies to a random point and spins twice.
static bool Circle(SotnSwordRuntime *r, SotnSwordSlot *s, SotnSwordInput in, SotnSwordEvents *e,
                   bool *removed)
{
    if (!CheckLevel(r, s, &in, e))
        return false;
    if (in.brothers)
    {
        // g_Player.unk70: the Sword Brothers spell takes over from any state.
        s->identity = SOTN_SWORD_BROTHERS;
        s->step = 0;
        return true;
    }
    Sint32 *c = r->circle, hold = 0;
    r->position[0] = Long((Uint32)(Pixel(s->x) - 128) << 5);
    r->position[1] = Long((Uint32)(Pixel(s->y) - 128) << 5);
    s->camera_x = in.tilemap_x;
    s->camera_y = in.tilemap_y;
    switch (s->step)
    {
    case 0:
        if (!Ready(r, s, in, removed))
            return false;
        break;
    case 1:
        s->swing = 1;
        c[4] = Long(Rand(e) % 128U + 64U + s->camera_x);
        c[5] = Long(Rand(e) % 128U + 64U + s->camera_y);
        ++s->step;
        break;
    case 2:
        c[2] = Sub(Long((Uint32)c[4] - s->camera_x), Pixel(s->x));
        c[3] = Sub(Long((Uint32)c[5] - s->camera_y), Pixel(s->y));
        if (!SotnMath_Atan2Vector(in.executable, c[3], c[2], &s->heading))
            return false;
        s->heading = (Sint16)(s->heading & 0xfff);
        s->yaw = SotnFairy_StepAngle(s->heading, s->yaw_previous, 32);
        s->yaw_previous = s->yaw;
        if (s->heading == s->yaw)
            ++s->step;
        r->rotation[2] = Word(s->yaw - 0x400);
        break;
    case 3: {
        const Sint32 tx = Long((Uint32)c[4] - s->camera_x), ty = Long((Uint32)c[5] - s->camera_y);
        s->velocity_x = (Sint32)Asr(Sub(Long((Uint32)tx << 16), s->x), 4);
        s->velocity_y = (Sint32)Asr(Sub(Long((Uint32)ty << 16), s->y), 4);
        s->x = Add(s->x, s->velocity_x);
        s->y = Add(s->y, s->velocity_y);
        Sint32 distance = 0;
        if (!Distance(in, s, tx, ty, &distance))
            return false;
        if (distance < 2)
        {
            c[1] = 0;
            c[0] = r->rotation[2] & 0xfff;
            ++s->step;
        }
        break;
    }
    case 4:
        if (!OverlayWord(in.overlay, CIRCLE_HOLD, &hold))
            return false;
        if (++c[1] > hold)
        {
            if (!Sound(e, SFX_SWEEP))
                return false;
            ++s->step;
        }
        break;
    case 5:
    case 6:
        r->rotation[2] = (Sint16)((r->rotation[2] + 128) & 0xfff);
        if (r->rotation[2] == c[0])
        {
            c[1] = 0;
            ++s->step;
        }
        break;
    case 7:
        if (!OverlayWord(in.overlay, CIRCLE_HOLD, &hold))
            return false;
        if (++c[1] > hold)
            ++s->step;
        break;
    case 8: {
        bool keep = false;
        if (!Retarget(r, s, in, e, &keep))
            return false;
        if (keep)
            s->step = 1;
        break;
    }
    default:
        return SDL_SetError("Sword familiar circle attack reached an unknown step");
    }
    if (!RoomEvents(r, s, in) || !Model(r, s, in, true))
        return false;
    return s->step < 5 || s->step >= 7 || Slash(r, s, in, e);
}
// One dive steering update toward d[0..2]: heading/yaw, the planar speed, pitch and depth speed.
static bool Steer(SotnSwordRuntime *r, SotnSwordSlot *s, SotnSwordInput in, Sint16 yaw_step,
                  Sint32 speed, Sint16 pitch_step, Sint32 depth_speed)
{
    Sint32 *d = r->dive, sine = 0, cosine = 0;
    d[3] = Sub(Sub(d[0], d[10]), r->position[0]);
    d[4] = Sub(Sub(d[1], d[11]), r->position[1]);
    d[5] = Sub(d[2], r->position[2]);
    if (!SotnMath_Atan2Vector(in.executable, d[4], d[3], &s->heading))
        return false;
    s->heading = (Sint16)(s->heading & 0xfff);
    s->yaw = SotnFairy_StepAngle(s->heading, s->yaw_previous, yaw_step);
    s->yaw_previous = s->yaw;
    if (!Trig(in.executable, s->yaw, &sine, &cosine))
        return false;
    d[7] = (Sint32)Asr((Sint64)cosine * speed, 12);
    d[8] = (Sint32)Asr((Sint64)sine * speed, 12);
    r->rotation[2] = Word(s->yaw - 0x400);
    d[3] = Abs(d[3]);
    d[4] = Abs(d[4]);
    d[6] = SDL_max(d[3], d[4]);
    if (!SotnMath_Atan2Vector(in.executable, d[5], d[6], &s->pitch_target))
        return false;
    s->pitch_target = (Sint16)(s->pitch_target & 0xfff);
    s->pitch = SotnFairy_StepAngle(s->pitch_target, s->pitch_previous, pitch_step);
    s->pitch_previous = s->pitch;
    if (!Trig(in.executable, s->pitch, &sine, NULL))
        return false;
    d[9] = (Sint32)Asr((Sint64)sine * depth_speed, 12);
    r->rotation[0] = s->pitch;
    for (Uint16 i = 0; i < 3; ++i)
        r->position[i] = Add(r->position[i], d[7 + i]);
    d[3] = Sub(Sub(d[0], d[10]), r->position[0]);
    d[4] = Sub(Sub(d[1], d[11]), r->position[1]);
    d[5] = Sub(d[2], r->position[2]);
    // SquareRoot0(SQ(dx) + SQ(dy) + SQ(dz)) with the original 32-bit products and sum.
    const Uint32 sum =
        (Uint32)d[3] * (Uint32)d[3] + (Uint32)d[4] * (Uint32)d[4] + (Uint32)d[5] * (Uint32)d[5];
    Uint32 root = 0;
    if (!SotnMath_Sqrt0(in.executable, sum, &root))
        return false;
    d[12] = Long(root);
    return true;
}
// func_us_80174B6C: from level 70 the sword rises behind the player, dives through its target in
// 3D and glides past it.
static bool Dive(SotnSwordRuntime *r, SotnSwordSlot *s, SotnSwordInput in, SotnSwordEvents *e,
                 bool *removed)
{
    if (!CheckLevel(r, s, &in, e))
        return false;
    if (in.brothers)
    {
        // g_Player.unk70: the Sword Brothers spell takes over from any state.
        s->identity = SOTN_SWORD_BROTHERS;
        s->step = 0;
        return true;
    }
    Sint32 *d = r->dive, hold = 0;
    r->position[0] = Long((Uint32)(Pixel(s->x) - 0x80) << 5);
    r->position[1] = Long((Uint32)(Pixel(s->y) - 0x80) << 5);
    s->camera_x = in.tilemap_x;
    s->camera_y = in.tilemap_y;
    d[10] = Long((s->camera_x - 0x80U) << 5);
    d[11] = Long((s->camera_y - 0x80U) << 5);
    switch (s->step)
    {
    case 0:
        if (!Ready(r, s, in, removed))
            return false;
        d[7] = (Sint32)Rand(e) - 0x4000;
        d[9] = (Sint32)Rand(e) - 0x4000;
        break;
    case 1:
        d[0] = Add(in.player.facing_left ? 0x800 : -0x800, d[10]);
        d[1] = Add(-0x400, d[11]);
        d[2] = -0x400;
        ++s->step;
        // fallthrough
    case 2:
        if (!Steer(r, s, in, 0x80, 160, 0x40, 0xa0))
            return false;
        if (d[12] < 0x200)
            ++s->step;
        break;
    case 3:
        d[7] = (Sint32)Asr(Sub(Sub(d[0], d[10]), r->position[0]), 4);
        d[8] = (Sint32)Asr(Sub(Sub(d[1], d[11]), r->position[1]), 4);
        d[9] = (Sint32)Asr(Sub(d[2], r->position[2]), 4);
        for (Uint16 i = 0; i < 3; ++i)
            r->position[i] = Add(r->position[i], d[7 + i]);
        s->yaw = SotnFairy_StepAngle(0x400, s->yaw_previous, 32);
        s->yaw_previous = s->yaw;
        r->rotation[2] = Word(s->yaw - 0x400);
        s->pitch = SotnFairy_StepAngle(0, s->pitch_previous, 32);
        s->pitch_previous = s->pitch;
        r->rotation[0] = s->pitch;
        if (s->yaw == 0x400 && !s->pitch)
        {
            d[13] = 0;
            ++s->step;
        }
        break;
    case 4:
        if (!OverlayWord(in.overlay, DIVE_HOLD, &hold))
            return false;
        if (++d[13] > hold)
            ++s->step;
        break;
    case 5: {
        bool keep = false;
        if (!Retarget(r, s, in, e, &keep))
            return false;
        if (!keep)
            break;
        d[0] = Add(d[7] >= 0 ? 0x800 : -0x800, d[10]);
        d[1] = Add(-0x800, d[11]);
        d[2] = d[9] >= 0 ? 0x800 : -0x800;
        if (!Sound(e, SFX_WING))
            return false;
        ++s->step;
    }
        // fallthrough
    case 6:
        if (!Steer(r, s, in, 0x80, 192, 0x80, 192))
            return false;
        if (d[12] < 0x200)
        {
            d[13] = 0;
            ++s->step;
        }
        break;
    case 7:
    case 10: {
        for (Uint16 i = 0; i < 3; ++i)
            r->position[i] = Add(r->position[i], d[7 + i]);
        if (!SotnMath_Atan2Vector(in.executable, d[8], d[7], &s->heading))
            return false;
        s->heading = (Sint16)(s->heading & 0xfff);
        s->yaw = SotnFairy_StepAngle(s->heading, s->yaw_previous, 0x80);
        s->yaw_previous = s->yaw;
        r->rotation[2] = Word(s->yaw - 0x400);
        d[5] = d[9];
        d[3] = Abs(d[7]);
        d[4] = Abs(d[8]);
        d[6] = SDL_max(d[3], d[4]);
        if (!SotnMath_Atan2Vector(in.executable, d[5], d[6], &s->pitch_target))
            return false;
        s->pitch_target = (Sint16)(s->pitch_target & 0xfff);
        s->pitch = SotnFairy_StepAngle(s->pitch_target, s->pitch_previous, 0x80);
        s->pitch_previous = s->pitch;
        r->rotation[0] = s->pitch;
        if (!OverlayWord(in.overlay, DIVE_GLIDE, &hold))
            return false;
        if (++d[13] > hold)
            s->step = s->step == 10 ? 1 : (Uint16)(s->step + 1U);
        break;
    }
    case 8: {
        bool keep = false;
        if (!Retarget(r, s, in, e, &keep))
            return false;
        if (!keep)
            break;
        s->swing = 1;
        ++s->step;
    }
        // fallthrough
    case 9: {
        const SotnSwordTarget *t = &in.targets[s->follow - 1U];
        d[0] = Add(Long((Uint32)(t->x - 0x80) << 5), d[10]);
        d[1] = Add(Long((Uint32)(t->y - 0x80) << 5), d[11]);
        d[2] = 0;
        if (!Steer(r, s, in, 0x80, 192, 0x80, 192))
            return false;
        if (d[12] < 0x200)
            ++s->step;
        break;
    }
    default:
        return SDL_SetError("Sword familiar dive attack reached an unknown step");
    }
    if (!RoomEvents(r, s, in))
        return false;
    // posX.i.hi = vx / 32 + 128 keeps the fraction; the projection uses the globals directly.
    s->x =
        Long((Uint32)(Word(r->position[0] / 32 + 128) & 0xffff) << 16 | ((Uint32)s->x & 0xffffU));
    s->y =
        Long((Uint32)(Word(r->position[1] / 32 + 128) & 0xffff) << 16 | ((Uint32)s->y & 0xffffU));
    if (!Render(r, s->model, in, true))
        return false;
    if (s->step < 5 || r->position[2] < -0x200 || r->position[2] > 0x200)
        return true;
    return Slash(r, s, in, e);
}
// func_us_8017280C(self, model, 1): row0 becomes the flat billboard of the sword's artwork and
// every other row hides.
static void Billboard(SotnSwordRuntime *r, Sint16 model)
{
    SotnPlayerEffectQuad *q = &r->model[0];
    q->page = 0x1b;
    q->palette = 0x140;
    q->u[0] = q->u[2] = model ? 0x60 : 0x30;
    q->u[1] = q->u[3] = model ? 0x8f : 0x5f;
    q->v[0] = q->v[1] = 0;
    q->v[2] = q->v[3] = 0x70;
    q->draw_mode = 0x108;
    for (Uint16 i = 1; i < SOTN_SWORD_MODEL_ROWS; ++i)
        r->model[i].draw_mode |= 8;
}
// func_us_801758C8: Sword Brothers. The sword rises to the screen centre, turns into a flat
// billboard, spins while copying itself into up to 64 rows, releases its DF attack and spins the
// copies out in a growing ring before the model returns.
static bool Brothers(SotnSwordRuntime *r, SotnSwordSlot *s, SotnSwordInput in, SotnSwordEvents *e,
                     bool *removed)
{
    if (!CheckLevel(r, s, &in, e))
        return false;
    Sint32 *b = r->brothers, sine = 0, cosine = 0;
    r->position[0] = Long((Uint32)(Pixel(s->x) - 0x80) << 5);
    r->position[1] = Long((Uint32)(Pixel(s->y) - 0x80) << 5);
    switch (s->step)
    {
    case 0:
        if (!Ready(r, s, in, removed))
            return false;
        break;
    case 1:
        b[0] = 0;
        b[1] = -0x1000;
        b[2] = -0xc00;
        if (!Sound(e, SFX_GIFT))
            return false;
        ++s->step;
        // fallthrough
    case 2: {
        b[3] = Sub(b[0], r->position[0]);
        b[4] = Sub(b[1], r->position[1]);
        b[5] = Sub(b[2], r->position[2]);
        if (!SotnMath_Atan2Vector(in.executable, b[4], b[3], &s->heading))
            return false;
        s->heading = (Sint16)(s->heading & 0xfff);
        s->yaw = SotnFairy_StepAngle(s->heading, s->yaw_previous, 0x180);
        s->yaw_previous = s->yaw;
        if (!Trig(in.executable, s->yaw, &sine, &cosine))
            return false;
        b[7] = (Sint32)Asr(Long((Uint32)cosine << 7), 12);
        b[8] = (Sint32)Asr(Long((Uint32)sine << 7), 12);
        r->rotation[2] = Word(s->yaw - 0x400);
        b[3] = Abs(b[3]);
        b[4] = Abs(b[4]);
        b[6] = SDL_max(b[3], b[4]);
        if (!SotnMath_Atan2Vector(in.executable, b[5], b[6], &s->pitch_target))
            return false;
        s->pitch_target = (Sint16)(s->pitch_target & 0xfff);
        s->pitch = SotnFairy_StepAngle(s->pitch_target, s->pitch_previous, 0x40);
        s->pitch_previous = s->pitch;
        if (!Trig(in.executable, s->pitch, &sine, NULL))
            return false;
        b[9] = (Sint32)Asr(Long((Uint32)sine << 7), 12);
        r->rotation[0] = s->pitch;
        for (Uint16 i = 0; i < 3; ++i)
            r->position[i] = Add(r->position[i], b[7 + i]);
        b[3] = Sub(b[0], r->position[0]);
        b[4] = Sub(b[1], r->position[1]);
        b[5] = Sub(b[2], r->position[2]);
        const Uint32 sum =
            (Uint32)b[3] * (Uint32)b[3] + (Uint32)b[4] * (Uint32)b[4] + (Uint32)b[5] * (Uint32)b[5];
        Uint32 root = 0;
        if (!SotnMath_Sqrt0(in.executable, sum, &root))
            return false;
        b[10] = Long(root);
        if (b[10] < 0x100) // FIX(1/256)
        {
            r->position[0] = 0;
            r->position[1] = -0x1000;
            r->position[2] = -0xc00;
            ++s->step;
        }
        break;
    }
    case 3:
        Billboard(r, s->model);
        r->model_ready = false;
        r->rotation[0] = s->pitch_previous = 0;
        r->rotation[2] = 0;
        s->yaw_previous = 0x400;
        if (!Sound(e, SFX_GIFT))
            return false;
        ++s->step;
        // fallthrough
    case 4:
        r->position[1] = Add(r->position[1], 0x100);
        if (r->position[1] >= 0)
        {
            const Uint8 *p = NULL;
            r->position[1] = 0;
            b[11] = 0;
            if (!View(in.overlay, VOICES + 8U, 2, &p) || !Sound(e, (Uint16)Read16(p)))
                return false;
            e->pause_write = true;
            e->pause_allowed = r->pause_allowed = false;
            ++s->step;
        }
        break;
    case 5:
        if (++b[11] > 70)
            ++s->step;
        break;
    case 6:
        s->yaw_previous = (Sint16)((s->yaw_previous + 0x40) & 0xfff);
        r->rotation[2] = Word(s->yaw_previous - 0x400);
        r->position[2] = Add(r->position[2], 0x60);
        if (r->position[2] >= 0)
        {
            r->position[2] = 0;
            b[12] = 0;
            ++s->step;
        }
        break;
    case 7: {
        s->yaw_previous = (Sint16)((s->yaw_previous + 0x40) & 0xfff);
        r->rotation[2] = Word(s->yaw_previous - 0x400);
        if (s->yaw_previous == 0xc00)
        {
            b[11] = 0;
            ++s->step;
        }
        // The packet after the last copy becomes another copy of the billboard.
        if (b[12] < 0 || b[12] + 1 >= SOTN_SWORD_MODEL_ROWS)
            return SDL_SetError("Sword Brothers copies exceed the sword's actual rows");
        SotnPlayerEffectQuad *q = &r->model[b[12] + 1];
        *q = r->model[0];
        q->palette = 0x142;
        q->draw_mode = 0x175;
        for (Uint16 v = 0; v < 4; ++v)
            q->red[v] = q->green[v] = q->blue[v] = 0x80;
        ++b[12];
        break;
    }
    case 8:
        if (++b[11] > 15)
        {
            b[11] = 0;
            e->pause_write = true;
            e->pause_allowed = r->pause_allowed = true;
            e->brothers_hitbox = true;
            if (!Sound(e, SFX_SWITCH))
                return false;
            ++s->step;
        }
        break;
    case 9: {
        const SotnPlayerEffectQuad *base = &r->model[0];
        Sint32 angle = b[11] * 32;
        Sint16 x[4], y[4];
        for (Uint16 v = 0; v < 4; ++v)
        {
            x[v] = Word(base->x[v] - 128);
            y[v] = Word(base->y[v] - 128);
        }
        for (Uint16 i = 0; i < 64; ++i)
        {
            Sint32 ring_sine = 0, ring_cosine = 0;
            if (!Trig(in.executable, angle, &sine, &cosine) ||
                !Trig(in.executable, angle + 0xc00, &ring_sine, &ring_cosine))
                return false;
            b[3] = (Sint32)Asr((Sint64)ring_cosine * (b[11] * 16), 12) + 128;
            b[4] = (Sint32)Asr((Sint64)ring_sine * (b[11] * 16), 12) + 128;
            SotnPlayerEffectQuad *q = &r->model[1 + i];
            for (Uint16 v = 0; v < 4; ++v)
            {
                q->x[v] = Word(b[3] + (Sint32)Asr((Sint64)cosine * x[v] - (Sint64)sine * y[v], 12));
                q->y[v] = Word(b[4] + (Sint32)Asr((Sint64)sine * x[v] + (Sint64)cosine * y[v], 12));
            }
            angle = (angle + 64) & 0xfff;
        }
        if (++b[11] > 15)
        {
            b[11] = 0;
            ++s->step;
        }
        break;
    }
    case 10:
        if (++b[11] > 30)
        {
            if (!Faces(r, s->model, s->variant, in.overlay))
                return false;
            r->model_ready = true;
            s->identity = SOTN_SWORD_DEFAULT;
            s->step = 0;
        }
        break;
    default:
        return SDL_SetError("Sword Brothers reached an unknown step");
    }
    if (!RoomEvents(r, s, in))
        return false;
    s->x =
        Long((Uint32)(Word(r->position[0] / 32 + 128) & 0xffff) << 16 | ((Uint32)s->x & 0xffffU));
    s->y =
        Long((Uint32)(Word(r->position[1] / 32 + 128) & 0xffff) << 16 | ((Uint32)s->y & 0xffffU));
    return Render(r, s->model, in, true);
}
// func_us_80176270: the level-50 farewell. Two fast passes through the screen, then the card
// turns off and slots4..7 are destroyed.
static bool Gift(SotnSwordRuntime *r, SotnSwordSlot *s, SotnSwordInput in, SotnSwordEvents *e,
                 bool *removed)
{
    Sint32 *g = r->gift, sine = 0, cosine = 0;
    r->position[0] = Long((Uint32)(Pixel(s->x) - 128) << 5);
    r->position[1] = Long((Uint32)(Pixel(s->y) - 128) << 5);
    switch (s->step)
    {
    case 0:
        if (!Ready(r, s, in, removed))
            return false;
        break;
    case 1:
        g[0] = g[1] = 0x1000;
        g[2] = -0x1000;
        if (!Sound(e, SFX_GIFT))
            return false;
        ++s->step;
        // fallthrough
    case 2:
    case 3: {
        g[3] = Sub(g[0], r->position[0]);
        g[4] = Sub(g[1], r->position[1]);
        g[5] = Sub(g[2], r->position[2]);
        if (!SotnMath_Atan2Vector(in.executable, g[4], g[3], &s->heading))
            return false;
        s->heading = (Sint16)(s->heading & 0xfff);
        s->yaw = SotnFairy_StepAngle(s->heading, s->yaw_previous, 0x180);
        s->yaw_previous = s->yaw;
        if (!Trig(in.executable, s->yaw, &sine, &cosine))
            return false;
        g[7] = (Sint32)Asr(Long((Uint32)cosine << 8), 12);
        g[8] = (Sint32)Asr(Long((Uint32)sine << 8), 12);
        r->rotation[2] = Word(s->yaw - 0x400);
        g[3] = Abs(g[3]);
        g[4] = Abs(g[4]);
        g[6] = SDL_max(g[3], g[4]);
        if (!SotnMath_Atan2Vector(in.executable, g[5], g[6], &s->pitch_target))
            return false;
        s->pitch_target = (Sint16)(s->pitch_target & 0xfff);
        s->pitch = SotnFairy_StepAngle(s->pitch_target, s->pitch_previous, 0x40);
        s->pitch_previous = s->pitch;
        if (!Trig(in.executable, s->pitch, &sine, NULL))
            return false;
        g[9] = (Sint32)Asr(Long((Uint32)sine << 8), 12);
        r->rotation[0] = s->pitch;
        for (Uint16 i = 0; i < 3; ++i)
            r->position[i] = Add(r->position[i], g[7 + i]);
        g[3] = Sub(g[0], r->position[0]);
        g[4] = Sub(g[1], r->position[1]);
        g[5] = Sub(g[2], r->position[2]);
        const Uint32 sum =
            (Uint32)g[3] * (Uint32)g[3] + (Uint32)g[4] * (Uint32)g[4] + (Uint32)g[5] * (Uint32)g[5];
        Uint32 root = 0;
        if (!SotnMath_Sqrt0(in.executable, sum, &root))
            return false;
        g[10] = Long(root);
        if (g[10] < 0x100)
        {
            if (s->step == 2)
            {
                r->position[0] = r->position[1] = 0x1000;
                r->position[2] = -0x1000;
                g[0] = g[1] = g[2] = -0x1000;
                s->yaw_previous = 0xa00;
                s->pitch_previous = 0;
                if (!Sound(e, SFX_GIFT))
                    return false;
            }
            ++s->step;
        }
        break;
    }
    case 4:
        // The card turns off (found only), g_Servant becomes 0 and DestroyEntity clears4..7.
        e->dismiss = true;
        SDL_zero(r->slots);
        return true;
    default:
        return SDL_SetError("Sword familiar gift reached an unknown step");
    }
    if (!RoomEvents(r, s, in))
        return false;
    s->x =
        Long((Uint32)(Word(r->position[0] / 32 + 128) & 0xffff) << 16 | ((Uint32)s->x & 0xffffU));
    s->y =
        Long((Uint32)(Word(r->position[1] / 32 + 128) & 0xffff) << 16 | ((Uint32)s->y & 0xffffU));
    return Render(r, s->model, in, true);
}
// func_us_80176BF8: sixteen additive bands trailing the projected blade edge.
static bool Trail(SotnSwordRuntime *r, SotnSwordSlot *s, SotnSwordInput in, bool *removed)
{
    switch (s->step)
    {
    case 0:
        if (!in.allocated)
        {
            *removed = true;
            return true;
        }
        s->flags = 0x0c820000U;
        for (Uint16 i = 0; i < SOTN_SWORD_TRAIL_ROWS; ++i)
        {
            SotnPlayerEffectQuad *q = &r->trail[i];
            *q = (SotnPlayerEffectQuad){.page = 0x1b, .palette = 0x141};
            q->u[0] = q->u[2] = 0;
            q->v[0] = q->v[1] = 0;
            q->u[1] = q->u[3] = 16;
            q->v[2] = q->v[3] = 16;
            q->red[0] = q->red[1] = (Uint8)((i + 1) * 4 + 8);
            q->green[0] = q->green[1] = (Uint8)((i + 1) * 14 + 8);
            q->blue[0] = q->blue[1] = (Uint8)((i + 1) * 8 + 8);
            q->red[2] = q->red[3] = (Uint8)(i * 4 + 8);
            q->green[2] = q->green[3] = (Uint8)(i * 14 + 8);
            q->blue[2] = q->blue[3] = (Uint8)(i * 8 + 8);
            q->draw_mode = 0x400 | 0x100 | 0x40 | 0x20 | 0x10 | 8 | 4 | 2 | 1;
        }
        ++s->step;
        break;
    case 1: {
        SotnPlayerEffectQuad *q = &r->trail[SOTN_SWORD_TRAIL_ROWS - 1];
        q->x[0] = r->edge[0].x;
        q->y[0] = r->edge[0].y;
        q->x[1] = r->edge[1].x;
        q->y[1] = r->edge[1].y;
        ++s->step;
        break;
    }
    default:
        if (r->edge_fresh)
        {
            for (Uint16 i = 0; i + 1 < SOTN_SWORD_TRAIL_ROWS; ++i)
            {
                const SotnPlayerEffectQuad next = r->trail[i + 1];
                SotnPlayerEffectQuad *q = &r->trail[i];
                q->priority = next.priority;
                q->draw_mode = next.draw_mode;
                SDL_memcpy(q->x, next.x, sizeof(q->x));
                SDL_memcpy(q->y, next.y, sizeof(q->y));
            }
            SotnPlayerEffectQuad *q = &r->trail[SOTN_SWORD_TRAIL_ROWS - 1];
            q->priority = (Uint16)r->edge_priority;
            q->draw_mode &= 0xfff7U;
            q->x[2] = q->x[0];
            q->y[2] = q->y[0];
            q->x[3] = q->x[1];
            q->y[3] = q->y[1];
            q->x[0] = r->edge[0].x;
            q->y[0] = r->edge[0].y;
            q->x[1] = r->edge[1].x;
            q->y[1] = r->edge[1].y;
        }
        else
        {
            for (Uint16 i = 0; i < SOTN_SWORD_TRAIL_ROWS; ++i)
            {
                SotnPlayerEffectQuad *q = &r->trail[i];
                q->draw_mode |= 8;
                q->x[0] = q->x[1] = r->edge[0].x;
                q->y[0] = q->y[1] = r->edge[0].y;
                q->x[2] = q->x[3] = r->edge[1].x;
                q->y[2] = q->y[3] = r->edge[1].y;
            }
            --s->step;
        }
        break;
    }
    r->edge_fresh = false;
    return true;
}
// func_us_80176F28: four level-90 halo quads pulsing with the halo phase.
static bool Halo(SotnSwordRuntime *r, SotnSwordSlot *s, SotnSwordInput in, bool *removed)
{
    if (!s->step)
    {
        if (!in.allocated)
        {
            *removed = true;
            return true;
        }
        s->flags = 0x04820000U;
        for (Uint16 i = 0; i < SOTN_SWORD_HALO_ROWS; ++i)
        {
            SotnPlayerEffectQuad *q = &r->halo[i];
            *q = (SotnPlayerEffectQuad){.page = 0x1b, .palette = 0x141};
            q->u[1] = q->u[3] = 16;
            q->v[2] = q->v[3] = 16;
            q->draw_mode = 0x100 | 0x20 | 0x10 | 8 | 4 | 2 | 1;
        }
        ++s->step;
    }
    else
    {
        Sint32 s1 = 0, c1 = 0, s2 = 0, c2 = 0;
        if (!Trig(in.executable, r->halo_phase, &s1, &c1) ||
            !Trig(in.executable, r->halo_phase2, &s2, &c2))
            return false;
        const Sint32 sa = Long((Uint32)s1 << 5) >> 12, ca = Long((Uint32)c1 << 5) >> 12,
                     sb = Long((Uint32)s2 << 5) >> 12, cb = Long((Uint32)c2 << 5) >> 12;
        for (Uint16 i = 0; i < SOTN_SWORD_HALO_ROWS; ++i)
        {
            SotnPlayerEffectQuad *q = &r->halo[i];
            if (r->halo_fresh)
            {
                q->priority = (Uint16)r->halo_priority[i];
                q->draw_mode &= 0xfff7U;
                const Sint32 low = i < 2 ? 32 : 96, lowb = i < 2 ? 64 : 192;
                q->red[0] = q->green[0] = (Uint8)(sa + low);
                q->blue[0] = (Uint8)(sb + lowb);
                q->red[1] = q->green[1] = (Uint8)(sa + 96);
                q->blue[1] = (Uint8)(sb + 192);
                q->red[2] = q->green[2] = q->red[3] = q->green[3] = (Uint8)(ca + 32);
                q->blue[2] = q->blue[3] = (Uint8)(cb + 64);
            }
            else
                q->draw_mode |= 8;
            for (Uint16 v = 0; v < 4; ++v)
            {
                q->x[v] = r->halo_points[i][v].x;
                q->y[v] = r->halo_points[i][v].y;
            }
        }
        r->halo_phase = (r->halo_phase + 0x20) & 0xfff;
        r->halo_phase = (r->halo_phase + 0x40) & 0xfff;
    }
    r->halo_fresh = false;
    return true;
}

bool SotnSwordRuntime_UpdateSlot(SotnSwordRuntime *runtime, Uint16 slot, SotnSwordInput in,
                                 SotnSwordEvents *out)
{
    if (!runtime || !out || slot < 4 || slot > 7 || !runtime->initialized || !Stats(in.stats))
        return SDL_SetError("Sword familiar callback requires its actual owner and slot4..7");
    SotnSwordRuntime r = *runtime;
    SotnSwordEvents e = {.random = in.random};
    SotnSwordSlot *s = &r.slots[slot - 4U];
    bool removed = false;
    switch (s->identity)
    {
    case 0:
        *out = e;
        return true;
    case SOTN_SWORD_DEFAULT:
        if (!Default(&r, s, in, &e, &removed))
            return false;
        break;
    case SOTN_SWORD_ENTRANCE:
        if (!Entrance(&r, s, in, &e, &removed))
            return false;
        break;
    case SOTN_SWORD_CIRCLE:
        if (!Circle(&r, s, in, &e, &removed))
            return false;
        break;
    case SOTN_SWORD_DIVE:
        if (!Dive(&r, s, in, &e, &removed))
            return false;
        break;
    case SOTN_SWORD_GIFT:
        if (!Gift(&r, s, in, &e, &removed))
            return false;
        break;
    case SOTN_SWORD_BROTHERS:
        if (!Brothers(&r, s, in, &e, &removed))
            return false;
        break;
    case SOTN_SWORD_TRAIL:
        if (!Trail(&r, s, in, &removed))
            return false;
        break;
    case SOTN_SWORD_HALO:
        if (!Halo(&r, s, in, &removed))
            return false;
        break;
    case SOTN_SWORD_EVENT: {
        // func_us_801773CC: event0 stops a speaking entrance, event15 raises the sword's layer.
        Uint16 sound = 0;
        if (s->parameter == 0)
        {
            r.entrance_cancel = true;
            if (Free(&r, SOTN_SWORD_SOUND, &sound) && r.slots[sound].sound.phase < 5)
                r.slots[sound].sound.phase = r.slots[sound].step = 8;
        }
        else if (s->parameter == 15)
            r.near_player = true;
        removed = true;
        break;
    }
    case SOTN_SWORD_SOUND:
        s->sound.parameter = s->parameter;
        if (!SotnFamiliarSfx_Tick(&s->sound, in.sound, &e.cd_command, &r.sound_step))
            return false;
        s->flags = s->sound.flags;
        s->step = s->sound.phase;
        removed = s->sound.removed;
        break;
    default:
        return SDL_SetError("Sword familiar entity %#x is not integrated", s->identity);
    }
    if (removed)
        SDL_zero(r.slots[slot - 4U]);
    *runtime = r;
    *out = e;
    return true;
}

static void Shift(Uint16 mode, Sint16 camera_x, Sint16 camera_y, Sint32 player_x, Sint32 player_y,
                  bool room, Sint32 *x, Sint32 *y)
{
    *x = *y = 0;
    if (!room && (mode & 2U))
    {
        *x = camera_x;
        *y = camera_y;
    }
    else if (mode & (room ? 0x100U : 0x200U))
    {
        *x = -Pixel(player_x);
        *y = -Pixel(player_y);
    }
}
static void ShiftRows(SotnPlayerEffectQuad *rows, Uint16 count, Sint16 camera_x, Sint16 camera_y,
                      Sint32 player_x, Sint32 player_y, bool room)
{
    for (Uint16 i = 0; i < count; ++i)
    {
        Sint32 dx = 0, dy = 0;
        Shift(rows[i].draw_mode, camera_x, camera_y, player_x, player_y, room, &dx, &dy);
        for (Uint16 v = 0; v < 4; ++v)
        {
            rows[i].x[v] = Word(rows[i].x[v] + dx);
            rows[i].y[v] = Word(rows[i].y[v] + dy);
        }
    }
}
static bool Holds(const SotnSwordRuntime *r, Uint16 identity, Uint16 step)
{
    for (Uint16 i = 0; i < 4; ++i)
        if (r->slots[i].identity == identity && r->slots[i].step >= step)
            return true;
    return false;
}
bool SotnSwordRuntime_Translate(SotnSwordRuntime *runtime, Sint16 camera_x, Sint16 camera_y,
                                Sint32 player_x, Sint32 player_y, bool room)
{
    if (!runtime)
        return SDL_InvalidParamError("runtime");
    if (!runtime->initialized)
        return true;
    SotnSwordRuntime r = *runtime;
    for (Uint16 i = 0; i < 4; ++i)
    {
        SotnSwordSlot *s = &r.slots[i];
        if (!s->identity)
            continue;
        if (room && !(s->flags & 0x20000U))
        {
            SDL_zero(*s);
            continue;
        }
        if (!room && (s->flags & CAMERA_LOCKED))
        {
            s->x = Long((Uint32)s->x + (Uint32)(Sint32)camera_x * 65536U);
            s->y = Long((Uint32)s->y + (Uint32)(Sint32)camera_y * 65536U);
        }
        if (room || (s->flags & 0x40000U))
        {
            s->x = Long((Uint32)s->x - (Uint32)player_x);
            s->y = Long((Uint32)s->y - (Uint32)player_y);
        }
    }
    // Rows belong to their live owners: slot4's model, the trail and the halo.
    if (r.slots[0].kind)
        ShiftRows(r.model, SOTN_SWORD_MODEL_ROWS, camera_x, camera_y, player_x, player_y, room);
    if (Holds(&r, SOTN_SWORD_TRAIL, 1))
        ShiftRows(r.trail, SOTN_SWORD_TRAIL_ROWS, camera_x, camera_y, player_x, player_y, room);
    if (Holds(&r, SOTN_SWORD_HALO, 1))
        ShiftRows(r.halo, SOTN_SWORD_HALO_ROWS, camera_x, camera_y, player_x, player_y, room);
    *runtime = r;
    return true;
}
bool SotnSwordRuntime_Clear(SotnSwordRuntime *runtime)
{
    if (!runtime)
        return SDL_InvalidParamError("runtime");
    SDL_zero(runtime->slots);
    SDL_memset(runtime->model, 0, sizeof(runtime->model));
    SDL_memset(runtime->trail, 0, sizeof(runtime->trail));
    SDL_memset(runtime->halo, 0, sizeof(runtime->halo));
    return true;
}

Uint16 SotnSwordRuntime_RowRequest(const SotnSwordRuntime *runtime, Uint16 slot)
{
    if (!runtime || !runtime->initialized || slot < 4 || slot > 7)
        return 0;
    const SotnSwordSlot *s = &runtime->slots[slot - 4U];
    if ((s->identity == SOTN_SWORD_DEFAULT || s->identity == SOTN_SWORD_ENTRANCE) && !s->kind &&
        !s->step)
        return SOTN_SWORD_MODEL_ROWS;
    if (s->identity == SOTN_SWORD_TRAIL && !s->step)
        return SOTN_SWORD_TRAIL_ROWS;
    if (s->identity == SOTN_SWORD_HALO && !s->step)
        return SOTN_SWORD_HALO_ROWS;
    return 0;
}
bool SotnSwordRuntime_Leases(const SotnSwordRuntime *runtime, SotnSwordLease out[3])
{
    if (!runtime || !out)
        return SDL_SetError("Sword lease report requires its actual owner and three parts");
    SotnSwordLease leases[3] = {0};
    static const Uint16 counts[3] = {SOTN_SWORD_MODEL_ROWS, SOTN_SWORD_TRAIL_ROWS,
                                     SOTN_SWORD_HALO_ROWS};
    for (Uint16 i = 0; runtime->initialized && i < 4; ++i)
    {
        const SotnSwordSlot *s = &runtime->slots[i];
        const Sint16 part =
            (s->identity == SOTN_SWORD_DEFAULT || s->identity == SOTN_SWORD_ENTRANCE ||
             s->identity == SOTN_SWORD_CIRCLE || s->identity == SOTN_SWORD_DIVE ||
             s->identity == SOTN_SWORD_GIFT || s->identity == SOTN_SWORD_BROTHERS)
                ? (s->kind ? 0 : -1)
            : s->identity == SOTN_SWORD_TRAIL ? (s->step ? 1 : -1)
            : s->identity == SOTN_SWORD_HALO  ? (s->step ? 2 : -1)
                                              : -1;
        if (part < 0)
            continue;
        if (leases[part].count || !s->generation)
            return SDL_SetError("Sword familiar part %d has two live row owners", part);
        leases[part] = (SotnSwordLease){.generation = s->generation,
                                        .slot = (Uint16)(i + 4U),
                                        .part = (Uint16)part,
                                        .count = counts[part]};
    }
    SDL_memcpy(out, leases, sizeof(leases));
    return true;
}
