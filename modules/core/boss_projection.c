// SPDX-License-Identifier: MPL-2.0
// Selected fixed matrix and UNR projection math adapted from PSY-Z
// b52003e55c0d1b20719e984dd19bd4d760eeb14a, psyz/src/psyz/libgte.c.
// Selected bounded reward, intro and book paths, not a general GTE emulator.
#include "boss_projection.h"
#include "actors_internal.h"
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
    // The divider's 257 entries are generated from its reciprocal definition.
    const Sint32 estimate = (Sint32)(((0x40000U / (index + 0x100) + 1) / 2)) - 0x101;
    const Uint32 u = (Uint32)SDL_clamp(estimate, 0, 255) + 0x101;
    d = (0x2000080U - d * u) >> 8;
    d = (0x80U + d * u) >> 8;
    return (Uint32)SDL_min(0x1ffffULL, ((Uint64)n * d + 0x8000) >> 16);
}
static Sint32 Coordinate(Sint32 anchor, Sint32 value, Uint32 ratio)
{
    const Sint32 ir = SDL_clamp(value, -32768, 32767);
    return SDL_clamp(anchor + ActorShift((Sint64)ratio * ir, 16), -1024, 1023);
}
bool SotnBoss_ProjectReward(SotnDataView executable, SotnRewardProjection p, SotnProjectedRect *out)
{
    if (out == NULL || p.radius < -128 || p.radius > 128 || p.anchor_x < -16384 ||
        p.anchor_x > 16384 || p.anchor_y < -16384 || p.anchor_y > 16384)
        return SDL_SetError("Reward projection exceeds its bounded transform");
    SotnMathTrig x = {0}, y = {0}, z = {0};
    if (!SotnMath_MatrixTrig(executable, p.x, &x) || !SotnMath_MatrixTrig(executable, p.y, &y) ||
        !SotnMath_MatrixTrig(executable, p.z, &z))
        return false;
    const Sint32 sx = x.sine, cx = x.cosine, sy = y.sine, cy = y.cosine, sz = z.sine, cz = z.cosine;
    const Sint32 t = ActorShift((Sint64)cz * -sy, 12);
    const Sint32 mx = ActorShift((Sint64)cz * cy, 12),
                 my = ActorShift((Sint64)sz * cx, 12) - ActorShift((Sint64)t * sx, 12),
                 mz = ActorShift((Sint64)sz * sx, 12) + ActorShift((Sint64)t * cx, 12);
    const Sint32 vx = ActorShift((Sint64)mx * p.radius, 12),
                 vy = ActorShift((Sint64)my * p.radius, 12),
                 vz = 512 + ActorShift((Sint64)mz * p.radius, 12);
    const Uint32 depth = (Uint32)SDL_clamp(vz, 0, 65535), ratio = Divide(depth, 512);
    const Sint32 px = Coordinate(p.anchor_x, vx, ratio) - p.anchor_x,
                 py = Coordinate(p.anchor_y, vy, ratio) - p.anchor_y;
    const Uint32 second = Divide((depth >> 2) * 4, 512);
    const Sint32 low = ActorShift((Sint64)-8 * p.scale, 12),
                 high = ActorShift((Sint64)8 * p.scale, 12);
    *out = (SotnProjectedRect){
        Coordinate(p.anchor_x, px + low, second), Coordinate(p.anchor_y, py + low, second),
        Coordinate(p.anchor_x, px + high, second), Coordinate(p.anchor_y, py + high, second)};
    return true;
}

bool SotnBoss_ProjectIntro(SotnDataView executable, Sint32 anchor_x, Sint32 anchor_y,
                           Sint16 rotation, Sint16 x, Sint16 y, SotnProjectedPoint *out)
{
    if (out == NULL || anchor_x < -16384 || anchor_x > 16384 || anchor_y < -16384 ||
        anchor_y > 16384 || x < -32 || x > 224 || y < -32 || y > 32)
        return SDL_SetError("BO4 intro projection exceeds its original diamond geometry");
    SotnMathTrig tilt = {0}, turn = {0};
    if (!SotnMath_MatrixTrig(executable, 640, &tilt) ||
        !SotnMath_MatrixTrig(executable, rotation, &turn))
        return false;
    // Identity, then SDK RotMatrixY(640), then RotMatrixZ(rotation).
    const Sint32 mx = ActorShift((Sint64)turn.cosine * tilt.cosine, 12);
    const Sint32 my = ActorShift((Sint64)turn.sine * tilt.cosine, 12);
    const Sint32 vx = ActorShift((Sint64)mx * x - (Sint64)turn.sine * y, 12);
    const Sint32 vy = ActorShift((Sint64)my * x + (Sint64)turn.cosine * y, 12);
    const Sint32 vz = 320 + ActorShift(-(Sint64)tilt.sine * x, 12);
    const Uint32 ratio = Divide((Uint32)SDL_clamp(vz, 0, 65535), 256);
    *out = (SotnProjectedPoint){Coordinate(anchor_x, vx, ratio), Coordinate(anchor_y, vy, ratio)};
    return true;
}

bool SotnBook_Project(SotnDataView executable, SotnBookTransform t, const SotnBookVertex *vertices,
                      Uint32 count, SotnBookProjection *out)
{
    if (!vertices || !out || (count != 1 && count != 4) || t.depth < 0 || t.depth > 768 ||
        t.anchor_x < -16384 || t.anchor_x > 16384 || t.anchor_y < -16384 || t.anchor_y > 16384)
        return SDL_SetError("Book projection exceeds its bounded transform");
    for (Uint32 i = 0; i < count; ++i)
        if (vertices[i].x < -32 || vertices[i].x > 32 || vertices[i].y < -32 ||
            vertices[i].y > 32 || vertices[i].z < -32 || vertices[i].z > 32)
            return SDL_SetError("Book vertex exceeds its bounded private model");
    SotnMathTrig x = {0}, y = {0}, z = {0};
    if (!SotnMath_MatrixTrig(executable, t.x, &x) || !SotnMath_MatrixTrig(executable, t.y, &y) ||
        !SotnMath_MatrixTrig(executable, t.z, &z))
        return false;
    const Sint32 sx = x.sine, cx = x.cosine, sy = y.sine, cy = y.cosine, sz = z.sine, cz = z.cosine;
    const Sint32 a = ActorShift(-(Sint64)cz * sy, 12), b = ActorShift(-(Sint64)sz * sy, 12);
    // Negation precedes the arithmetic shift in the SDK, including negative fractions.
    const Sint32 m[3][3] = {{ActorShift((Sint64)cz * cy, 12), ActorShift(-(Sint64)sz * cy, 12), sy},
                            {ActorShift((Sint64)sz * cx, 12) - ActorShift((Sint64)a * sx, 12),
                             ActorShift((Sint64)cz * cx, 12) + ActorShift((Sint64)b * sx, 12),
                             ActorShift(-(Sint64)cy * sx, 12)},
                            {ActorShift((Sint64)sz * sx, 12) + ActorShift((Sint64)a * cx, 12),
                             ActorShift((Sint64)cz * sx, 12) - ActorShift((Sint64)b * cx, 12),
                             ActorShift((Sint64)cy * cx, 12)}};
    SotnBookProjection result = {0};
    Uint32 sum = 0;
    for (Uint32 i = 0; i < count; ++i)
    {
        const SotnBookVertex v = vertices[i];
        Sint32 transformed[3] = {0};
        for (Uint32 row = 0; row < 3; ++row)
            transformed[row] = ActorShift(
                (Sint64)m[row][0] * v.x + (Sint64)m[row][1] * v.y + (Sint64)m[row][2] * v.z, 12);
        result.depth[i] = (Uint16)SDL_clamp(t.depth + 512 + transformed[2], 0, 65535);
        const Uint32 ratio = Divide(result.depth[i], 512);
        result.point[i] = (SotnProjectedPoint){Coordinate(t.anchor_x, transformed[0], ratio),
                                               Coordinate(t.anchor_y, transformed[1], ratio)};
        sum += result.depth[i];
    }
    if (count == 4)
    {
        const SotnProjectedPoint p = result.point[0], q = result.point[1], r = result.point[2];
        result.winding = (q.x - p.x) * (r.y - p.y) - (q.y - p.y) * (r.x - p.x);
    }
    // InitGeom sets ZSF4=0x100: AVSZ4 divides the four-depth sum by 16, not four.
    const Uint32 ordering_depth = count == 4 ? sum >> 4 : sum >> 2;
    const Sint32 priority = (t.depth + 512) / 4 - (Sint32)ordering_depth + 144;
    if (priority < 0 || priority > 511)
        return SDL_SetError("Book projection exceeds the original ordering table");
    result.priority = (Uint16)priority;
    *out = result;
    return true;
}

bool SotnDemon_ProjectQuad(SotnDataView executable, SotnDemonQuadPose p, SotnProjectedPoint out[4])
{
    if (!out || p.width < 0 || p.width > 64 || p.height < 0 || p.height > 64 || p.scale < 0 ||
        p.scale > 16384 || p.anchor_x < -16384 || p.anchor_x > 16384 || p.anchor_y < -16384 ||
        p.anchor_y > 16384)
        return SDL_SetError("Lesser Demon effect projection exceeds its bounded controls");
    SotnMathTrig trig = {0};
    if (!SotnMath_MatrixTrig(executable, p.angle, &trig))
        return false;
    const Sint32 xx = ActorShift((Sint64)trig.cosine * p.scale, 12),
                 xy = ActorShift(-(Sint64)trig.sine * p.scale, 12),
                 yx = ActorShift((Sint64)trig.sine * p.scale, 12), yy = xx;
    SotnProjectedPoint projected[4] = {0};
    for (Uint32 i = 0; i < 4; ++i)
    {
        const Sint32 x = (i & 1U) ? p.width / 2 : -p.width / 2,
                     y = (i & 2U) ? p.height / 2 : -p.height / 2;
        const Sint32 vx = ActorShift((Sint64)xx * x + (Sint64)xy * y, 12),
                     vy = ActorShift((Sint64)yx * x + (Sint64)yy * y, 12);
        projected[i] = (SotnProjectedPoint){Coordinate(p.anchor_x, vx, 65536),
                                            Coordinate(p.anchor_y, vy, 65536)};
    }
    SDL_memcpy(out, projected, sizeof(projected));
    return true;
}

// st/lib/e_library_bg.c: the pinned model rotations are identity, with private depth vertices.
bool SotnLibrary_ProjectBackground(SotnDataView library, Uint16 kind, Sint32 x, Sint32 y,
                                   SotnProjectedPoint *out)
{
    if (!out || kind > 3 || x < -32767 || x > 32767 || y < -32767 || y > 32767)
        return SDL_SetError("Library background projection exceeds its selected controls");
    SotnDataView vertex = {0}, rotation = {0};
    const size_t offset = kind == 3 ? 0xdf0 : 0xdc0U + kind * 16U;
    const size_t angles = kind == 3 ? 0xe88 : offset + 8;
    if (!Slice(library, offset, 8, &vertex) || !Slice(library, angles, 6, &rotation))
        return false;
    const Sint32 z = Signed16(vertex.bytes + 4);
    if (Read16(vertex.bytes) || Read16(vertex.bytes + 2) || z < 0 || Read16(rotation.bytes) ||
        Read16(rotation.bytes + 2) || Read16(rotation.bytes + 4))
        return SDL_SetError(
            "Library background requires its original axial vertex and identity rotation");
    const Uint32 ratio = Divide((Uint32)(1024 + z), 1024);
    const bool distant = kind == 3;
    *out = (SotnProjectedPoint){Coordinate(128, x - 128, ratio),
                                Coordinate(distant ? 128 : y, distant ? y - 128 : 0, ratio)};
    return true;
}

// Fog arithmetic follows MIT SDK src/main/psxsdk/libgte/fog_01.c.
// Selected RTPS/NCDS path: SDK instruction 0x4ae80413 uses sf=1, lm=1.
// The pinned PSY-Z NormalColorDpq literal reverses bytes; do not inherit that defect.
bool SotnLibrary_ProjectShelf(SotnDataView executable, SotnDataView library, Sint16 angle,
                              Sint16 extra, Sint32 x, Sint32 y, SotnShelfProjection *out)
{
    if (!out || extra < 0 || extra > 128 || x < -16384 || x > 16384 || y < -16384 || y > 16384)
        return SDL_SetError("Library shelf projection exceeds its selected controls");
    SotnDataView models = {0}, normals = {0}, matrix = {0}, coefficient = {0};
    SotnMathTrig trig = {0};
    if (!Slice(library, 0xc88, 32, &models) || !Slice(library, 0xd28, 32, &normals) ||
        !Slice(library, 0xd88, 18, &matrix) || !Slice(executable, 0x1e5f4, 2, &coefficient) ||
        !SotnMath_MatrixTrig(executable, angle, &trig))
        return false;
    // The selected normals have equal +/-4096 X/Z components. Their SDK normalization
    // uses reciprocal-root index 64, with a shift of 12; retain the private coefficient.
    const Sint32 normal_scale = Read16(coefficient.bytes);
    if (normal_scale < 2048 || normal_scale > 4096)
        return SDL_SetError("Library shelf has an invalid private reciprocal-root coefficient");
    SotnShelfProjection result = {0};
    for (Uint16 i = 0; i < 8; ++i)
    {
        const Uint32 model = Read32(models.bytes + i * 4U), normal = Read32(normals.bytes + i * 4U);
        SotnDataView vertex = {0}, direction = {0};
        if (model < 0x80180c48 || model > 0x80180c80 || (model - 0x80180c48) % 8U ||
            normal < 0x80180ca8 || normal > 0x80180d18 || (normal - 0x80180ca8) % 16U ||
            !Slice(library, model - 0x80180000U, 6, &vertex) ||
            !Slice(library, normal - 0x80180000U, 12, &direction))
            return SDL_SetError("Library shelf model references invalid private data");
        const Sint32 vx = Signed16(vertex.bytes), vy = Signed16(vertex.bytes + 2),
                     vz = Signed16(vertex.bytes + 4);
        const Sint32 nx = Signed32(direction.bytes), ny = Signed32(direction.bytes + 4),
                     nz = Signed32(direction.bytes + 8);
        if (vx < -32 || vx > 32 || vy < -96 || vy > 0 || vz < -32 || vz > 32 ||
            (nx != -4096 && nx != 4096) || ny || (nz != -4096 && nz != 4096))
            return SDL_SetError("Library shelf vertex or normal exceeds its selected model");
        const Sint32 tx = ActorShift((Sint64)trig.cosine * vx + (Sint64)trig.sine * vz, 12),
                     tz = 552 + extra +
                          ActorShift(-(Sint64)trig.sine * vx + (Sint64)trig.cosine * vz, 12);
        const Uint32 ratio = Divide((Uint32)tz, 512);
        result.point[i] = (SotnProjectedPoint){Coordinate(x, tx, ratio), Coordinate(y, vy, ratio)};
        const Sint32 cue = SDL_clamp(ActorShift(20971520LL - (Sint64)335 * ratio, 12), 0, 4096);
        const Sint32 ax = nx < 0 ? -normal_scale : normal_scale,
                     az = nz < 0 ? -normal_scale : normal_scale;
        const Sint32 light[3] = {
            SDL_clamp(ActorShift((Sint64)trig.cosine * ax + (Sint64)trig.sine * az, 12), 0, 32767),
            0,
            SDL_clamp(ActorShift(-(Sint64)trig.sine * ax + (Sint64)trig.cosine * az, 12), 0,
                      32767)};
        for (Uint16 row = 0; row < 3; ++row)
        {
            Sint64 sum = 88LL * 16 * 4096;
            for (Uint16 col = 0; col < 3; ++col)
                sum += (Sint64)Signed16(matrix.bytes + (row * 3U + col) * 2U) * light[col];
            const Sint32 lit = SDL_clamp(ActorShift(sum, 12), 0, 32767);
            const Sint32 input = (144 * 16) * lit;
            const Sint32 difference =
                SDL_clamp(ActorShift(64LL * 16 * 4096 - input, 12), -32768, 32767);
            const Sint32 color = ActorShift((Sint64)input + (Sint64)cue * difference, 12);
            result.color[i][row] = (Uint8)SDL_clamp(ActorShift(color, 4), 0, 255);
        }
    }
    *out = result;
    return true;
}
