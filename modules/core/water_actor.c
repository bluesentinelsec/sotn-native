// SPDX-License-Identifier: AGPL-3.0-or-later
// st/water_effects.h:EntityAlucardWaterEffect, pinned sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Private stage tables remain borrowed.
#include <SDL3/SDL_error.h>
#include <sotn/water_actor.h>

static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return bits < 32768U ? (Sint16)bits : (Sint16)((Sint32)bits - 65536);
}
static Sint16 Pixel(Sint32 value)
{
    return Word((Sint32)((Uint32)value >> 16));
}
static Sint16 Read(const Uint8 *p)
{
    return Word(p[0] | (Sint32)p[1] << 8);
}
static Sint32 High(Sint16 high)
{
    const Uint32 bits = (Uint32)(Uint16)high << 16;
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static bool Row(SotnDataView data, Uint16 row, const Uint8 **out)
{
    const size_t offset = (size_t)row * 2U;
    if (row % 8 || !data.bytes || offset > data.size || data.size - offset < 16)
        return SDL_SetError("Water controller lost its bounded retained private row");
    *out = data.bytes + offset;
    return true;
}
static bool Shape(const Uint8 *row, Sint16 left, Sint16 right, Uint32 *out)
{
    const Sint16 slope = Read(row + 4), before = Read(row + 10), after = Read(row + 12);
    if (slope < 0 || slope > 7 || before < 0 || before > 7 || after < 0 || after > 7 || left < 0 ||
        right <= 0)
        return SDL_SetError("Water splash requires valid private slopes and contact distances");
    const Sint32 edge = left < 14    ? (14 - left) * 2048 + before * 32
                        : right < 14 ? (right + 14) * 2048 + after * 32
                                     : 0;
    *out = (Uint32)(edge + slope * 256);
    return true;
}
static bool Plan(SotnWaterActorEvents *e, SotnWaterActorInput in, SotnWaterChildPlan child)
{
    ++e->attempts;
    const Uint32 used = in.occupied | e->reserved;
    for (Uint16 i = 0; i < 32; ++i)
    {
        const Uint32 bit = (Uint32)1 << i;
        if (used & bit)
            continue;
        child.slot = (Uint16)(224U + i);
        e->reserved |= bit;
        e->children[e->count++] = child;
        return true;
    }
    return false;
}
static bool Splash(SotnWaterActorEvents *e, SotnWaterActorInput in, const SotnWaterActor *previous,
                   bool leaving)
{
    const Uint8 *row = NULL;
    Uint32 flags = 0;
    if (!Row(in.volume.volumes, leaving ? previous->row : e->volume.row, &row) ||
        !Shape(row, leaving ? previous->left : e->volume.left,
               leaving ? previous->right : e->volume.right, &flags))
        return false;
    Uint16 slope = (Uint16)((flags >> 8) & 7U);
    if (!slope || slope == 7)
    {
        slope = (Uint16)((flags >> 5) & 7U);
        if (!slope || slope == 7)
            slope = 0;
    }
    SotnWaterChildPlan child = {
        .x = High(Pixel(in.x)),
        .y = High(Word(Pixel(in.y) + e->volume.foot_offset - e->volume.feet)),
        .priority = in.priority};
    if (slope)
    {
        child.kind = SOTN_WATER_SIDE_SPLASH;
        for (Uint16 i = 0; i < 8; ++i)
        {
            child.parameters = (Uint16)(Read(row + 14) + slope * 16 + i);
            if (!Plan(e, in, child))
                break;
        }
    }
    else
    {
        child.kind = SOTN_WATER_SPLASH;
        child.parameters = (Uint16)flags;
        if (leaving)
        {
            child.x = High(Word(previous->x - in.volume.scroll_x));
            child.y =
                High(Word(previous->y - (Sint32)(previous->raw & 0x7fffU) - in.volume.scroll_y));
            child.parameters = (Uint16)(flags + (in.velocity_y > -4 * 65536 ? 1U : 0U));
        }
        // Original entering code overwrites its velocity-selected variant with flags.
        (void)Plan(e, in, child);
    }
    return true;
}
bool SotnWaterActor_Tick(SotnWaterActor *actor, SotnWaterActorInput in, SotnWaterActorEvents *out)
{
    if (!actor || !out || actor->phase > 1 || in.priority > 511 ||
        in.volume.player_x != Pixel(in.x) || in.volume.player_y != Pixel(in.y))
        return SDL_SetError("Water controller requires its actual bounded player/origin snapshot");
    SotnWaterActor s = *actor;
    SotnWaterActorEvents e = {0};
    if (!SotnWaterVolume_Query(in.volume, &e.volume))
        return false;
    if (s.phase)
    {
        e.depths_changed = true;
        const Uint32 status = in.volume.status;
        const Sint16 vertical = Pixel(in.velocity_y);
        if (vertical && !(status & 3U))
        {
            const bool leaving = vertical < 0 && !e.volume.raw && s.raw && (s.raw & 0x7fffU) < 17;
            const bool entering = vertical > 0 && e.volume.raw && e.volume.feet < 9 && !s.raw;
            if ((leaving || entering) && !Splash(&e, in, &s, leaving))
                return false;
        }
        else if (!(status & 2U) && e.volume.raw)
        {
            const Uint8 *row = NULL;
            if (!Row(in.volume.volumes, e.volume.row, &row))
                return false;
            const Sint16 current = Read(row + 14);
            if (!s.cooldown && e.volume.feet <= e.volume.body_height && e.volume.left >= 6 &&
                e.volume.right >= 6 && (e.volume.x != s.x || current))
            {
                Uint16 parameter = 0;
                if (e.volume.x != s.x && in.velocity_x)
                {
                    parameter = (Uint16)((e.volume.body_height - e.volume.feet) >> 3);
                    if (parameter == 5)
                        parameter = 4;
                }
                const SotnWaterChildPlan child = {
                    .x = High(Pixel(in.x)),
                    .y = High(Word(Pixel(in.y) + e.volume.foot_offset - e.volume.feet)),
                    .kind = SOTN_WATER_SURFACING,
                    .parameters = (Uint16)(parameter | (Uint16)Read(row + 4) << 8),
                    .priority = in.priority,
                    .row = e.volume.row,
                    .current = current};
                if (Plan(&e, in, child))
                    s.cooldown = 8;
            }
            if (current > 0x1000 || current < -0x1000)
            {
                const Sint16 speed = Word(e.volume.feet >= e.volume.body_height
                                              ? ((status & 1U) ? current * 3 / 4 : current / 2)
                                              : current * e.volume.feet / 80);
                if ((speed < 0 && !(in.contacts & 8U)) || (speed > 0 && !(in.contacts & 4U)))
                    e.current_x = speed * 16;
            }
        }
        if (s.cooldown)
            s.cooldown = Word((Sint32)s.cooldown - 1);
    }
    s.phase = 1;
    s.raw = e.volume.raw;
    s.x = e.volume.x;
    s.y = e.volume.y;
    s.row = e.volume.row;
    s.left = e.volume.left;
    s.right = e.volume.right;
    *actor = s;
    *out = e;
    return true;
}
