// SPDX-License-Identifier: AGPL-3.0-or-later
// st/e_life_up.h, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Native primitives pack each original drawable/data pair into one initialized owner record.
#include "actors_internal.h"
#include "boss_projection.h"
static bool Rect(SotnActors *s, Actor *a, Uint16 index, Sint32 left, Sint32 top, Sint32 right,
                 Sint32 bottom, bool flash)
{
    ActorPrimitive *p = ActorPrimitiveGet(s, a, index);
    if (p == NULL)
        return false;
    if (left == right || top == bottom)
    {
        p->state.active = false;
        return true;
    }
    const Sint32 width = SDL_abs(right - left), height = SDL_abs(bottom - top);
    if (width > 2048 || height > 2048)
        return SDL_SetError("Boss reward primitive exceeds its projected bounds");
    const Uint8 u = (Uint8)(flash ? 63 : 16), v = (Uint8)(flash ? 192 : 80),
                vend = (Uint8)(flash ? 255 : 96);
    p->state = (SotnActorPrimitive){
        .x = (Sint32)(((Sint64)left + right) * 32768),
        .y = (Sint32)(((Sint64)top + bottom) * 32768),
        .width = (Uint16)width,
        .height = (Uint16)height,
        .page = 0x7a,
        .palette = (Uint16)(flash ? 0x19f : 0x194),
        .blend = true,
        .active = true,
        .textured = true,
        .red = 32,
        .green = 0,
        .blue = 0,
        .modulate = !flash,
        .u = {(Uint8)(left > right ? u : 0), (Uint8)(left > right ? 0 : u),
              (Uint8)(left > right ? u : 0), (Uint8)(left > right ? 0 : u)},
        .v = {(Uint8)(top > bottom ? vend : v), (Uint8)(top > bottom ? vend : v),
              (Uint8)(top > bottom ? v : vend), (Uint8)(top > bottom ? v : vend)}};
    return true;
}
static bool Flash(SotnActors *s, Actor *a, Sint32 radius)
{
    const Sint32 x = Pixel(a->state.x), y = Pixel(a->state.y);
    return Rect(s, a, 0, x - radius, y - radius, x + radius, y + radius, true);
}
static bool Collapse(SotnActors *s, Actor *a)
{
    Uint16 low = 0;
    for (Uint16 group = 0; group <= a->reward.groups; ++group)
    {
        const Uint16 first = (Uint16)(1 + group * 24);
        ActorPrimitive *root = ActorPrimitiveGet(s, a, first);
        if (root == NULL)
            return false;
        root->reward.radius += root->reward.speed;
        if (root->reward.speed < -16384)
            root->reward.speed += 14336;
        const Sint32 radius = Pixel(root->reward.radius);
        if (radius < 8)
        {
            ++low;
            a->reward.flash += 4;
            for (Uint16 i = 0; i < 24; ++i)
            {
                ActorPrimitive *p = ActorPrimitiveGet(s, a, (Uint16)(first + i));
                if (p == NULL)
                    return false;
                p->state.active = false;
            }
        }
        else
        {
            for (Uint16 i = 0; i < 24; ++i)
            {
                const Uint16 index = (Uint16)(first + i);
                ActorPrimitive *p = ActorPrimitiveGet(s, a, index);
                if (p == NULL)
                    return false;
                SotnProjectedRect rect = {0};
                if (!SotnBoss_ProjectReward(
                        (SotnRewardProjection){
                            p->reward.x, p->reward.y, p->reward.z, (Sint16)radius, p->reward.scale,
                            Pixel(a->state.x) - s->camera_x, Pixel(a->state.y) - s->camera_y},
                        &rect))
                    return false;
                if (!Rect(s, a, index, rect.left + s->camera_x, rect.top + s->camera_y,
                          rect.right + s->camera_x, rect.bottom + s->camera_y, false))
                    return false;
                p->reward.scale = (Sint16)Wrap16((Sint64)p->reward.scale - 16);
                p->reward.z = (Sint16)Wrap16((Sint64)p->reward.z + 8);
                p->reward.x = (Sint16)Wrap16((Sint64)p->reward.x + 16);
                p->reward.y = (Sint16)Wrap16((Sint64)p->reward.y + 32);
            }
        }
    }
    if (low == 8)
        ++a->state.step;
    return Flash(s, a, a->reward.flash > 256 ? 224 : a->reward.flash);
}
bool ActorBossReward(SotnActors *s, Actor *a)
{
    switch (a->state.step)
    {
    case 0:
        if (a->state.parameters != 5 || !ActorInitialize(s, a, 0xbf8))
            return SDL_SetError("Laboratory boss reward has an unsupported drop selection");
        a->state.animation_set = 2;
        a->state.frame = 0;
        a->state.priority = 0xc0;
        if (!ActorPrimitives(s, a, 193))
        {
            // Preserve the original allocation-failure path: still produce the gameplay drop.
            SDL_ClearError();
            a->state.step = 6;
            return true;
        }
        for (Uint16 group = 0; group < 8; ++group)
            for (Uint16 latitude = 0; latitude < 3; ++latitude)
                for (Uint16 angle = 0; angle < 8; ++angle)
                {
                    ActorPrimitive *p =
                        ActorPrimitiveGet(s, a, (Uint16)(1 + group * 24 + latitude * 8 + angle));
                    if (p == NULL)
                        return false;
                    p->reward.radius = 128 * 65536;
                    p->reward.speed = -5 * 65536;
                    p->reward.scale = 4096;
                    p->reward.z = (Sint16)(angle * 512);
                    p->reward.y = (Sint16)(latitude * 1344);
                }
        return ActorSound(s, 0x7d2);
    case 1:
        if (a->reward.cadence == 0)
        {
            a->reward.cadence = 2;
            ++a->reward.groups;
        }
        --a->reward.cadence;
        if (a->reward.groups > 7)
        {
            a->reward.groups = 7;
            ++a->state.step;
        }
        return Collapse(s, a);
    case 2:
        return Collapse(s, a);
    case 4: {
        a->state.x += a->vx;
        a->state.y += a->vy;
        a->vy += 8192;
        SotnCollider c = {0};
        if (!ActorQuery(s, Pixel(a->state.x), Pixel(a->state.y) + 4, &c))
            return false;
        if (c.effects & SOTN_EFFECT_SOLID)
        {
            a->vy = 0;
            ShiftY(a, c.top);
            if (--a->reward.flash == 0)
            {
                a->state.step = 5;
                return true;
            }
        }
        // Original falling variant shares the shrinking-flash update.
        if (a->reward.flash > 0)
            a->reward.flash -= 32;
        else
        {
            a->reward.flash = 16;
            a->state.step = 5;
        }
        return Flash(s, a, SDL_min(224, a->reward.flash));
    }
    case 3:
        if (a->reward.flash > 0)
            a->reward.flash -= 32;
        else
        {
            a->reward.flash = 16;
            a->state.step = 5;
        }
        return Flash(s, a, SDL_min(224, a->reward.flash));
    case 5:
        ActorPrimitivesClear(s, a);
        a->state.primitive_count = 0;
        ShiftY(a, -4);
        a->state.step = 6;
        // The original completes conversion in this same tick.
        break;
    case 6:
        break;
    default:
        return SDL_SetError("Boss reward effect step is invalid");
    }
    SotnDataView drop = {0};
    if (!Slice(s->nz0, 0x2850 + (size_t)a->state.parameters * 2, 2, &drop) ||
        Read16(drop.bytes) != 23)
        return SDL_SetError("Laboratory boss reward is not the supported original Life Max Up");
    a->state.id = 3;
    a->state.parameters = 0x8017;
    a->state.step = 0;
    a->pose = a->remaining = 0;
    return true;
}
