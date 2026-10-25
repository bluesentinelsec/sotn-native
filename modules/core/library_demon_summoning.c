// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/lib/e_lesser_demon.c:func_us_801BCFD4/func_us_801BD268.
// Original rainbow profiles stay in private LIB data; initialized native records
// retain packet iteration/reuse order without original linked pointers.
#include "library_demon_internal.h"
static ActorPrimitive *Available(SotnActors *s, const Actor *a)
{
    for (Uint16 i = 0; i < a->state.primitive_count; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return NULL;
        if (!p->demon.used)
            return p;
    }
    return NULL;
}
static Sint32 ScreenY(SotnActors *s, ActorPrimitive *p, Uint16 index)
{
    return Pixel(p->state.y) - s->camera_y + p->state.offset_y[index];
}
static bool Color(SotnActors *s, Actor *a, ActorPrimitive *p)
{
    switch (p->demon.phase)
    {
    case 0: {
        if (p->demon.color_index < 1 || p->demon.color_index > 6)
            return SDL_SetError("Lesser Demon column color exceeds its private rainbow profile");
        SotnDataView colors = {0};
        if (!Slice(s->nz0, 0x1c04 + (p->demon.color_index - 1U) * 3, 6, &colors))
            return false;
        Uint8 *channels[3] = {p->state.vertex_red, p->state.vertex_green, p->state.vertex_blue};
        for (Uint16 channel = 0; channel < 3; ++channel)
        {
            channels[channel][0] = channels[channel][1] = (Uint8)(colors.bytes[channel] + 32U);
            channels[channel][2] = channels[channel][3] = (Uint8)(colors.bytes[channel + 3] + 32U);
        }
        ++p->demon.phase;
        break;
    }
    case 1:
        if (ScreenY(s, p, 1) < 240)
        {
            ActorPrimitive *child = Available(s, a);
            if (child)
            {
                child->demon.used = true;
                child->demon.phase = 0;
                child->demon.side = p->demon.side;
                child->demon.color_index =
                    p->demon.color_index == 6 ? 1 : (Uint8)(p->demon.color_index + 1);
                // A reused packet retains its previous colors until its next color update.
                child->state.x = p->state.x;
                child->state.y = p->state.y;
                child->state.active = child->state.quad = child->state.gouraud =
                    child->state.blend = child->state.priority_set = true;
                child->state.page = p->state.page;
                child->state.priority = p->state.priority;
                for (Uint16 i = 0; i < 4; ++i)
                    child->state.offset_x[i] = p->state.offset_x[i];
                child->state.offset_y[0] = p->state.offset_y[2];
                child->state.offset_y[1] = p->state.offset_y[3];
                child->state.offset_y[2] = (Sint16)(child->state.offset_y[0] + 32);
                child->state.offset_y[3] = (Sint16)(child->state.offset_y[1] + 32);
            }
            ++p->demon.phase;
        }
        break;
    case 2: {
        Uint8 *channels[3] = {p->state.vertex_red, p->state.vertex_green, p->state.vertex_blue};
        for (Uint16 channel = 0; channel < 3; ++channel)
            for (Uint16 i = 0; i < 4; ++i)
                channels[channel][i] = (Uint8)SDL_max(0, (Sint32)channels[channel][i] - 5);
        if (ScreenY(s, p, 2) < 0)
        {
            p->demon.phase = 0;
            p->demon.used = false;
            p->state.active = false;
        }
        break;
    }
    default:
        return SDL_SetError("Lesser Demon column packet entered an invalid color phase");
    }
    return true;
}
static bool Update(SotnActors *s, Actor *a, Uint8 movement)
{
    for (Uint16 i = 0; i < 30; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        if (p->demon.used && !Color(s, a, p))
            return false;
    }
    for (Uint16 i = 0; i < 30; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        if (!p->demon.used)
            continue;
        const Sint16 outward = p->demon.side ? -1 : 1;
        const Sint16 shift = (Sint16)(movement == 2 ? -outward : movement == 1 ? outward : 0);
        p->state.offset_x[0] += shift;
        p->state.offset_x[2] = p->state.offset_x[0];
        p->state.offset_x[1] -= shift;
        p->state.offset_x[3] = p->state.offset_x[1];
        for (Uint16 v = 0; v < 4; ++v)
            p->state.offset_y[v] -= 8;
    }
    return true;
}
static bool Minion(SotnActors *s, Actor *a)
{
    Actor *child = NULL;
    // The original draws RNG only after finding a free actor; reserve before selecting ID.
    if (!ActorSpawn(s, a, 0x4d, 16, 96, 128, &child))
        return false;
    if (child)
    {
        const Uint32 choice = ActorRandom(s) & 3U;
        child->state.id = (Uint16)(choice < 2 ? 0x4d : choice == 2 ? 0x37 : 0x43);
        child->state.facing_left = a->state.facing_left;
        child->state.x =
            Wrap16((Sint64)Pixel(a->state.x) + (a->state.facing_left ? -8 : 8)) * 65536;
        child->state.y = Wrap16((Sint64)Pixel(a->state.y) + 48) * 65536;
    }
    return true;
}
bool ActorDemonSummoning(SotnActors *s, Actor *a)
{
    switch (a->demon.effect_step)
    {
    case 0:
        if (!ActorPrimitives(s, a, 30))
        {
            a->demon.effect_step = 7;
            return true;
        }
        a->state.flags |= 0x800000U;
        if (a->state.priority < 2 || a->state.priority > 503)
            return SDL_SetError("Lesser Demon summoning priorities exceed ordering bounds");
        for (Uint16 i = 0; i < 30; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (!p)
                return false;
            p->state.priority_set = true;
            p->state.priority = (Uint16)(a->state.priority + 8);
        }
        ++a->demon.effect_step;
        break;
    case 1:
        for (Uint16 i = 0; i < 2; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (!p)
                return false;
            const Sint16 offset = a->state.facing_left ? -8 : 8;
            const Sint16 x = (Sint16)(offset + (i ? 1 : -1)), y = (Sint16)(288 + i * 32);
            p->demon.used = true;
            p->demon.side = i == 0;
            p->demon.color_index = 1;
            p->demon.phase = 0;
            p->state = (SotnActorPrimitive){
                .x = Pixel(a->state.x) * 65536,
                .y = s->camera_y * 65536,
                .active = true,
                .quad = true,
                .gouraud = true,
                .blend = true,
                .priority_set = true,
                .page = (Uint16)(i ? 0 : 0x20),
                .priority = (Uint16)(i ? a->state.priority - 2 : a->state.priority + 4),
                .offset_x = {x, (Sint16)(x + (i ? -2 : 2)), x, (Sint16)(x + (i ? -2 : 2))},
                .offset_y = {y, (Sint16)(y - 32), (Sint16)(y + 32), y},
                .vertex_red = {0, 0, 160, 160},
                .vertex_green = {0, 0, 96, 96},
                .vertex_blue = {0, 0, 96, 96}};
        }
        a->demon.effect_timer = 24;
        a->demon.effect_step = 3;
        break;
    case 2:
        if (!Update(s, a, (a->demon.timer & 1U) ? 1 : 0))
            return false;
        if (a->demon.timer & 1U)
        {
            a->demon.effect_timer = (Uint8)(a->demon.effect_timer - 1);
            if (!a->demon.effect_timer)
            {
                ++a->demon.effect_step;
                return Minion(s, a);
            }
        }
        break;
    case 3:
        if (!Update(s, a, 0))
            return false;
        if (a->demon.effect_timer)
        {
            if (--a->demon.effect_timer == 0)
            {
                a->demon.effect_timer = 24;
                --a->demon.effect_step;
            }
        }
        break;
    case 4:
        if (!Update(s, a, 2))
            return false;
        a->demon.effect_timer = (Uint8)(a->demon.effect_timer + 1);
        if (a->demon.effect_timer > 24)
            ++a->demon.effect_step;
        break;
    case 5:
        ActorDemonReleaseEffects(s, a);
        ++a->demon.effect_step;
        break;
    case 6:
    case 7:
    case 8:
        break;
    default:
        return SDL_SetError("Lesser Demon summoning entered an invalid effect phase");
    }
    return true;
}
