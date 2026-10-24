// SPDX-License-Identifier: MPL-2.0
// Selected fixed matrix and UNR projection math adapted from PSY-Z
// b52003e55c0d1b20719e984dd19bd4d760eeb14a, psyz/src/psyz/libgte.c.
// Implements the bounded, non-overflowing reward path, not a general GTE emulator.
#include "boss_projection.h"
#include "actors_internal.h"
static Uint32 Divide(Uint32 depth)
{
    if (depth <= 256)
        return 0x1ffff;
    Uint32 shift = 0, d = depth;
    while (d < 0x8000)
    {
        d *= 2;
        ++shift;
    }
    const Uint32 n = 512U << shift, index = (d - 0x7fc0) >> 7;
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
bool SotnBoss_ProjectReward(SotnRewardProjection p, SotnProjectedRect *out)
{
    if (out == NULL || p.radius < -128 || p.radius > 128 || p.anchor_x < -16384 ||
        p.anchor_x > 16384 || p.anchor_y < -16384 || p.anchor_y > 16384)
        return SDL_SetError("Reward projection exceeds its bounded transform");
    const Sint32 sx = ActorSin(p.x), cx = ActorSin(p.x + 1024), sy = ActorSin(p.y),
                 cy = ActorSin(p.y + 1024), sz = ActorSin(p.z), cz = ActorSin(p.z + 1024);
    const Sint32 t = ActorShift((Sint64)cz * -sy, 12);
    const Sint32 mx = ActorShift((Sint64)cz * cy, 12),
                 my = ActorShift((Sint64)sz * cx, 12) - ActorShift((Sint64)t * sx, 12),
                 mz = ActorShift((Sint64)sz * sx, 12) + ActorShift((Sint64)t * cx, 12);
    const Sint32 vx = ActorShift((Sint64)mx * p.radius, 12),
                 vy = ActorShift((Sint64)my * p.radius, 12),
                 vz = 512 + ActorShift((Sint64)mz * p.radius, 12);
    const Uint32 depth = (Uint32)SDL_clamp(vz, 0, 65535), ratio = Divide(depth);
    const Sint32 px = Coordinate(p.anchor_x, vx, ratio) - p.anchor_x,
                 py = Coordinate(p.anchor_y, vy, ratio) - p.anchor_y;
    const Uint32 second = Divide((depth >> 2) * 4);
    const Sint32 low = ActorShift((Sint64)-8 * p.scale, 12),
                 high = ActorShift((Sint64)8 * p.scale, 12);
    *out = (SotnProjectedRect){
        Coordinate(p.anchor_x, px + low, second), Coordinate(p.anchor_y, py + low, second),
        Coordinate(p.anchor_x, px + high, second), Coordinate(p.anchor_y, py + high, second)};
    return true;
}
