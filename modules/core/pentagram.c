// SPDX-License-Identifier: AGPL-3.0-or-later
// weapon/w_017.c:EntityWeaponAttack, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. UV models/palettes remain private inputs.
#include "boss_projection.h"
#include <sotn/combat.h>
#include <sotn/pentagram.h>
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return bits < 32768 ? (Sint16)bits : (Sint16)((Sint32)bits - 65536);
}
static Sint32 Fixed(Uint32 bits)
{
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint16 High(Sint32 value)
{
    return Word((Sint32)((Uint32)value >> 16));
}
static Sint32 SetHigh(Sint32 value, Sint16 high)
{
    return Fixed((Uint32)(Uint16)high << 16 | ((Uint32)value & 65535U));
}
static Sint16 ReadWord(const Uint8 *p)
{
    return Word(p[0] | (Sint32)p[1] << 8);
}
static bool Initialize(SotnPentagram *p, SotnPentagramShared *shared, SotnPentagramInput in,
                       Uint16 palette, Uint16 u, Uint16 v)
{
    const size_t offset = 0x260, bytes = SOTN_PENTAGRAM_TRIANGLES * 12U;
    if (!in.overlay.bytes || in.overlay.size < offset + bytes)
        return SDL_SetError("Pentagram requires its actual bounded private triangle model");
    p->polygons[0] =
        (SotnPlayerEffectQuad){.x = {64, 192, 64, 192},
                               .y = {40, 40, 168, 168},
                               .u = {(Uint8)u, (Uint8)(u + 127), (Uint8)u, (Uint8)(u + 127)},
                               .v = {(Uint8)v, (Uint8)v, (Uint8)(v + 127), (Uint8)(v + 127)},
                               .page = 0x19,
                               .palette = palette,
                               .priority = 0x1ba,
                               .draw_mode = u ? 0 : 0x11};
    for (Uint16 i = 0; i < SOTN_PENTAGRAM_TRIANGLES; ++i)
    {
        SotnPentagramVector points[3] = {0};
        Sint32 x = 0, y = 0;
        for (Uint16 j = 0; j < 3; ++j)
        {
            const Uint8 *q = in.overlay.bytes + offset + (i * 3U + j) * 4U;
            points[j] = (SotnPentagramVector){.x = ReadWord(q), .y = ReadWord(q + 2)};
            if (points[j].x < 0 || points[j].x > 127 || points[j].y < 0 || points[j].y > 127)
                return SDL_SetError("Pentagram UV model exceeds its original private tile");
            x += points[j].x;
            y += points[j].y;
        }
        x /= 3;
        y /= 3;
        SotnPlayerEffectQuad *q = &p->polygons[i + 1U];
        *q = (SotnPlayerEffectQuad){
            .page = 0x19, .palette = palette, .priority = 0x1b6, .draw_mode = u ? 0 : 0x11};
        for (Uint16 j = 0; j < 3; ++j)
        {
            const Sint32 vx = points[j].x - x, vy = points[j].y - y;
            if (vx < -32 || vx > 32 || vy < -32 || vy > 32)
                return SDL_SetError("Pentagram model exceeds its selected local projection");
            shared->vectors[i][j] = (SotnPentagramVector){(Sint16)vx, (Sint16)vy, 0};
            q->u[j] = (Uint8)(points[j].x + u);
            q->v[j] = (Uint8)(points[j].y + v);
        }
        shared->vectors[i][3] = (SotnPentagramVector){(Sint16)(x - 64), (Sint16)(y - 64), 640};
    }
    shared->initialized = true;
    return true;
}
bool SotnPentagram_Tick(SotnPentagram *state, SotnPentagramShared *shared, SotnPentagramInput in,
                        Uint32 *random, SotnPentagramEvents *out)
{
    if (!state || !shared || !random || !out || state->removed || state->phase > 2 || in.hand > 1 ||
        (in.parameter & 0x7fffU) > 0x100U || (in.parameter & 255U) ||
        ((in.parameter & 0x8000U) != 0) != (in.hand != 0) ||
        (state->phase && (!shared->initialized || state->parameter != in.parameter ||
                          state->lifetime < 1 || state->lifetime > (state->phase == 1 ? 24 : 128))))
        return SDL_SetError("Pentagram callback requires its actual hand, scratch and lifetime");
    SotnPentagram p = *state;
    SotnPentagramShared s = *shared;
    SotnPentagramEvents e = {0};
    Uint32 rng = *random;
    const bool bat = (in.parameter & 0x7f00U) != 0;
    const Uint16 palette = bat ? 0x19f : in.hand ? 0x128 : 0x110;
    bool project = false;
    switch (p.phase)
    {
    case 0:
        if (!in.allocated)
        {
            p.removed = true;
            goto done;
        }
        if (!Initialize(&p, &s, in, palette, bat ? 128 : 0, in.hand ? 128 : 0))
            return false;
        p.parameter = in.parameter;
        p.flags = 0x04800000;
        p.x = SetHigh(p.x, 128);
        p.y = SetHigh(p.y, 104);
        p.lifetime = 24;
        e.flash = 4;
        e.sound = 0x6e4;
        e.shake = 3;
        e.properties = true;
        e.use_ticks = 4;
        ++p.phase;
        break;
    case 1:
        if (--p.lifetime == 0)
        {
            e.clear_attack = true;
            for (Uint16 i = 0; i < SOTN_PENTAGRAM_TRIANGLES; ++i)
            {
                const SotnPentagramVector center = s.vectors[i][3];
                SotnPentagramShard *q = &s.shards[i];
                q->x = (Sint32)center.x * 65536;
                q->y = (Sint32)center.y * 65536;
                q->z = (Sint32)center.z * 65536;
                const Sint32 angle = SotnCombat_Random(&rng) % 682U + (i / 4U) * 682U;
                SotnMathTrig direction = {0}, depth = {0};
                if (!SotnMath_Trig(in.executable, angle, &direction) ||
                    !SotnMath_Trig(in.executable, angle + 1536, &depth))
                    return false;
                q->velocity_x = direction.cosine * 16;
                q->velocity_y = -direction.sine * 16;
                q->velocity_z = depth.cosine * (i & 1U ? -64 : 64);
                q->angle_x = q->angle_y = q->angle_z = 0;
                q->speed_x = (Sint16)((Sint32)(SotnCombat_Random(&rng) % 160U) - 80);
                q->speed_y = (Sint16)((Sint32)(SotnCombat_Random(&rng) % 160U) - 80);
                q->speed_z = (Sint16)((Sint32)(SotnCombat_Random(&rng) % 160U) - 80);
            }
            p.lifetime = 128;
            ++p.phase;
        }
        break;
    case 2:
        project = true;
        for (Uint16 i = 0; i < SOTN_PENTAGRAM_TRIANGLES; ++i)
        {
            SotnPentagramShard *q = &s.shards[i];
            q->x = Fixed((Uint32)q->x + (Uint32)q->velocity_x);
            q->y = Fixed((Uint32)q->y + (Uint32)q->velocity_y);
            q->z = Fixed((Uint32)q->z + (Uint32)q->velocity_z);
            q->angle_x = Word(q->angle_x + q->speed_x);
            q->angle_y = Word(q->angle_y + q->speed_y);
            q->angle_z = Word(q->angle_z + q->speed_z);
        }
        if (--p.lifetime == 0)
        {
            p.removed = true;
            goto done;
        }
        break;
    }
    const Uint16 color = (Uint16)(palette + ((in.game_timer >> 1) & 1U));
    if (!bat)
        p.polygons[0].palette = color;
    if (project)
    {
        p.polygons[0].draw_mode |= 8;
        for (Uint16 i = 0; i < SOTN_PENTAGRAM_TRIANGLES; ++i)
        {
            const SotnPentagramShard q = s.shards[i];
            SotnTriangleVertex vertices[3] = {0};
            for (Uint16 j = 0; j < 3; ++j)
            {
                const SotnPentagramVector v = s.vectors[i][j];
                vertices[j] = (SotnTriangleVertex){v.x, v.y, v.z};
            }
            SotnTriangleProjection geometry = {0};
            if (!SotnTriangle_Project(in.executable,
                                      (SotnTriangleTransform){.x = q.angle_x,
                                                              .y = q.angle_y,
                                                              .z = q.angle_z,
                                                              .depth = High(q.z),
                                                              .anchor_x = High(q.x) + 128,
                                                              .anchor_y = High(q.y) + 104},
                                      vertices, &geometry))
                return false;
            SotnPlayerEffectQuad *polygon = &p.polygons[i + 1U];
            if (geometry.winding)
                for (Uint16 j = 0; j < 3; ++j)
                {
                    polygon->x[j] = (Sint16)geometry.point[j].x;
                    polygon->y[j] = (Sint16)geometry.point[j].y;
                }
            polygon->draw_mode |= 8;
            if (geometry.ordering_depth > 0 && geometry.ordering_depth <= 446)
            {
                if (!bat)
                    polygon->palette = (Uint16)(color + (geometry.winding < 0 ? 2 : 0));
                polygon->priority = 0x1ba;
                polygon->draw_mode &= 0xfff7U;
            }
        }
    }
done:
    *state = p;
    *shared = s;
    *random = rng;
    *out = e;
    return true;
}
