// SPDX-License-Identifier: AGPL-3.0-or-later
// Reconstructed BO0 313A8.c:func_us_801B7CC8 and st/e_misc.h subdivision,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Runtime profiles stay private.
#include "olrox_monster_internal.h"
typedef struct Vertex
{
    Sint16 x, y;
    Uint8 u, v, r, g, b;
} Vertex;
static Sint16 Word(Sint32 n)
{
    return (Sint16)Wrap16(n);
}
static Actor *Body(SotnActors *s, Actor *a)
{
    if (!a->enemy_state.owner_generation || a->enemy_state.owner_slot >= SOTN_STAGE_ACTOR_SLOTS)
        return NULL;
    Actor *body = &s->slots[a->enemy_state.owner_slot];
    return body->state.id == 0x33 && body->state.generation == a->enemy_state.owner_generation &&
                   OlroxOwnsChild(s, body, a, 10)
               ? body
               : NULL;
}
static ActorPrimitive *Part(SotnActors *s, const Actor *a, Uint16 i)
{
    return ActorPrimitiveGet(s, a, i);
}
static Vertex ReadVertex(const SotnActorPrimitive *p, Uint16 i)
{
    return (Vertex){p->offset_x[i],   p->offset_y[i],     p->u[i],          p->v[i],
                    p->vertex_red[i], p->vertex_green[i], p->vertex_blue[i]};
}
static Vertex Midpoint(Vertex a, Vertex b)
{
    return (Vertex){
        Word(ActorShift((Sint64)a.x + b.x + 1, 1)), Word(ActorShift((Sint64)a.y + b.y + 1, 1)),
        (Uint8)(((Uint16)a.u + b.u + 1U) / 2U),     (Uint8)(((Uint16)a.v + b.v + 1U) / 2U),
        (Uint8)(((Uint16)a.r + b.r + 1U) / 2U),     (Uint8)(((Uint16)a.g + b.g + 1U) / 2U),
        (Uint8)(((Uint16)a.b + b.b + 1U) / 2U)};
}
static bool Split(SotnActors *s, Actor *a, const ActorPrimitive *source, Uint16 first)
{
    const ActorPrimitive copy = *source;
    Vertex grid[9] = {0};
    grid[0] = ReadVertex(&copy.state, 0);
    grid[2] = ReadVertex(&copy.state, 1);
    grid[6] = ReadVertex(&copy.state, 2);
    grid[8] = ReadVertex(&copy.state, 3);
    grid[1] = Midpoint(grid[0], grid[2]);
    grid[7] = Midpoint(grid[6], grid[8]);
    grid[3] = Midpoint(grid[0], grid[6]);
    grid[5] = Midpoint(grid[2], grid[8]);
    grid[4] = Midpoint(grid[3], grid[5]);
    if ((Uint32)first + 4U > a->state.primitive_count)
        return SDL_SetError("Olrox subdivision exceeds its four owned output packets");
    for (Uint16 j = 0; j < 4; ++j)
    {
        ActorPrimitive *p = Part(s, a, (Uint16)(first + j));
        if (!p)
            return false;
        p->state = copy.state;
        p->olrox_effect = copy.olrox_effect;
        // Subdivision overwrites packet metadata through color control bytes. Only the final
        // corner retains p3; initialize the three unwritten midpoint control bytes safely.
        p->olrox_effect.tag = j == 3 ? copy.olrox_effect.tag : 0;
        const Uint16 start = (Uint16)((j / 2U) * 3U + j % 2U);
        for (Uint16 i = 0; i < 4; ++i)
        {
            const Vertex v = grid[start + (i / 2U) * 3U + i % 2U];
            p->state.offset_x[i] = v.x;
            p->state.offset_y[i] = v.y;
            p->state.u[i] = v.u;
            p->state.v[i] = v.v;
            p->state.vertex_red[i] = v.r;
            p->state.vertex_green[i] = v.g;
            p->state.vertex_blue[i] = v.b;
        }
    }
    return true;
}
static bool Find(SotnActors *s, Actor *a, Uint16 first, Uint16 count, bool visibility, Uint16 *out)
{
    *out = SDL_MAX_UINT16;
    if (first > a->state.primitive_count || !count || count > 4)
        return SDL_SetError("Olrox fragment search requires a bounded owned range");
    for (Uint16 i = first; (Uint32)i + count <= a->state.primitive_count; ++i)
    {
        bool free = true;
        for (Uint16 j = 0; j < count; ++j)
        {
            ActorPrimitive *p = Part(s, a, (Uint16)(i + j));
            if (!p)
                return false;
            if (visibility ? p->state.active : p->olrox_effect.tag)
                free = false;
        }
        if (free)
        {
            *out = i;
            break;
        }
    }
    return true;
}
static bool CopyParts(SotnActors *s, Actor *a, const Actor *source, Uint16 *next, bool first)
{
    const Uint16 begin = source->state.id == 0x33 ? 2 : 0;
    for (Uint16 i = begin; i < source->state.primitive_count; ++i)
    {
        if (*next >= a->state.primitive_count)
            return SDL_SetError("Olrox capture exceeds its owned fragment budget");
        ActorPrimitive *from = Part(s, source, i), *to = Part(s, a, (*next)++);
        if (!from || !to)
            return false;
        to->state = from->state;
        to->state.palette = first ? 0x20b : 0x219;
        if (first)
        {
            for (Uint16 k = 0; k < 4; ++k)
                to->state.vertex_red[k] = 128;
            to->state.priority = (Uint16)(to->state.priority + 8U);
        }
        to->olrox_effect.tag = from->olrox_effect.tag;
    }
    return true;
}
static bool Capture(SotnActors *s, Actor *a, Actor *body, bool first)
{
    Uint16 next = first ? 0 : 1;
    if (first)
    {
        for (Uint16 i = 0; i < 10; ++i)
        {
            Actor *part = i ? OlroxChild(s, body, i) : body;
            if (!part)
                return SDL_SetError("Olrox initial capture lost an owned body part");
            if (!CopyParts(s, a, part, &next, true))
                return false;
        }
    }
    else
    {
        SotnDataView roles = {0};
        if (!Slice(s->nz0, 0x12d8, 12, &roles))
            return false;
        for (Uint16 i = 0; i < 6; ++i)
        {
            const Uint16 role = Read16(roles.bytes + i * 2U);
            if (role > 9)
                return SDL_SetError("Olrox final capture has an invalid private part role");
            Actor *part = role ? OlroxChild(s, body, role) : body;
            if (!part)
                return SDL_SetError("Olrox final capture lost an owned body part");
            if (!CopyParts(s, a, part, &next, false))
                return false;
        }
    }
    a->olrox_death.copied_end = next;
    for (Uint16 i = next; i < a->state.primitive_count; ++i)
    {
        ActorPrimitive *p = Part(s, a, i);
        if (!p)
            return false;
        p->state.active = false;
    }
    return true;
}
static bool FirstFragments(SotnActors *s, Actor *a, Actor *body)
{
    Uint16 visible = 0;
    const Uint16 end = a->olrox_death.copied_end;
    for (Uint16 i = 0; i < end; ++i)
    {
        ActorPrimitive *p = Part(s, a, i);
        if (!p)
            return false;
        if (!p->state.active)
            continue;
        ++visible;
        if (!(ActorRandom(s) & 15U))
        {
            Uint16 out = SDL_MAX_UINT16;
            if (!Find(s, a, end, 4, true, &out))
                return false;
            if (out != SDL_MAX_UINT16)
            {
                p->state.vertex_red[0] = 176;
                p->state.vertex_red[1] = 152;
                p->state.vertex_red[2] = 160;
                p->state.vertex_red[3] = 136;
                if (!Split(s, a, p, out))
                    return false;
                p->state.active = false;
            }
        }
    }
    for (Uint16 i = end; i < a->state.primitive_count; ++i)
    {
        ActorPrimitive *p = Part(s, a, i);
        if (!p)
            return false;
        if (!p->state.active)
            continue;
        ++visible;
        Uint8 *red = &p->state.vertex_red[0];
        if (*red >= 129)
        {
            --*red;
            if (*red == 128)
            {
                Uint16 out = SDL_MAX_UINT16;
                if (!Find(s, a, end, 4, true, &out))
                    return false;
                if (out != SDL_MAX_UINT16)
                {
                    p->state.vertex_red[0] = 32;
                    p->state.vertex_red[1] = 20;
                    p->state.vertex_red[2] = 24;
                    p->state.vertex_red[3] = 16;
                    if (!Split(s, a, p, out))
                        return false;
                    p->state.active = false;
                    p->olrox_effect.tag = 0;
                }
                else
                    *red = (Uint8)(*red + (ActorRandom(s) & 31U) + 2U);
            }
        }
        else
        {
            --*red;
            if (!*red)
            {
                p->state.active = false;
                p->olrox_effect.tag = 0;
            }
            else if (*red < 16)
                for (Uint16 k = 0; k < 4; ++k)
                    p->state.offset_y[k] = Word(p->state.offset_y[k] - ((Sint32)(*red >> 1) - 8));
        }
    }
    if (!visible)
    {
        if (!body)
            return SDL_SetError("Olrox first fragment release lost its body generation");
        body->olrox.death_phase = 2;
        ++a->state.step;
    }
    return true;
}
static bool PrepareFinal(SotnActors *s, Actor *a, Actor *body)
{
    if (!body)
        return SDL_SetError("Olrox final capture lost its original body generation");
    if (body->olrox.death_phase != 3)
        return true;
    ActorPrimitive *p = Part(s, a, 0);
    if (!p)
        return false;
    const Sint16 x = Word(Pixel(a->state.x) - s->camera_x);
    p->state = (SotnActorPrimitive){.active = true,
                                    .quad = true,
                                    .screen_space = true,
                                    .gouraud = true,
                                    .blend = true,
                                    .page = 0x60,
                                    .priority_set = true,
                                    .priority = (Uint16)(s->olrox_monster.priority + 16U),
                                    .offset_x = {x, x, x, x},
                                    .offset_y = {0, 0, 240, 240},
                                    .vertex_red = {0, 0, 240, 240},
                                    .vertex_green = {0, 0, 240, 240},
                                    .vertex_blue = {0, 0, 240, 240}};
    if (!Capture(s, a, body, false))
        return false;
    a->olrox_death.source_begin = 1;
    a->olrox_death.remaining = 40;
    ++a->state.step;
    return true;
}
static bool Expand(SotnActors *s, Actor *a)
{
    ActorPrimitive *p = Part(s, a, 0);
    if (!p)
        return false;
    const Sint16 left = Word(p->state.offset_x[0] - 6), right = Word(p->state.offset_x[1] + 6);
    p->state.offset_x[0] = p->state.offset_x[2] = left;
    p->state.offset_x[1] = p->state.offset_x[3] = right;
    p->state.offset_y[0] = p->state.offset_y[1] = 0;
    p->state.offset_y[2] = p->state.offset_y[3] = 240;
    if (((Uint16)left & 0xf000U) && ((Uint16)right & 0xf00U))
    {
        p->state.offset_x[0] = p->state.offset_x[2] = 0;
        p->state.offset_x[1] = p->state.offset_x[3] = 256;
        a->olrox_death.timer = 1;
        ++a->state.step;
        Actor *rays = NULL;
        if (!ActorSpawn(s, a, 0x3e, 0, 160, 192, &rays))
            return false;
        if (rays)
        {
            rays->state.facing_left = a->state.facing_left;
            a->olrox_death.effect_slot = rays->slot;
            a->olrox_death.effect_generation = rays->state.generation;
        }
    }
    return true;
}
static bool LastFragments(SotnActors *s, Actor *a)
{
    if (a->state.step == 4 && !--a->olrox_death.timer)
    {
        if (!ActorSound(s, 0x60d))
            return false;
        a->olrox_death.timer = 4;
        const Sint32 index = (Sint32)a->olrox_death.source_begin + a->olrox_death.remaining;
        if (index < 0 || index >= a->state.primitive_count)
            return SDL_SetError("Olrox final fragment cursor exceeds its owned packets");
        ActorPrimitive *p = Part(s, a, (Uint16)index);
        Uint16 out = SDL_MAX_UINT16;
        if (!p || !Find(s, a, a->olrox_death.copied_end, 4, false, &out))
            return false;
        if (out != SDL_MAX_UINT16)
        {
            p->olrox_effect.tag = 1;
            p->state.vertex_red[0] = 36;
            p->state.vertex_red[1] = 40;
            p->state.vertex_red[2] = 44;
            p->state.vertex_red[3] = 33;
            if (!Split(s, a, p, out))
                return false;
            p->state.active = false;
            --a->olrox_death.remaining;
            if (a->olrox_death.remaining < 0)
            {
                if (!ActorSound(s, 0x636))
                    return false;
                ++a->state.step;
            }
        }
    }
    ActorPrimitive *flash = Part(s, a, 0);
    if (!flash)
        return false;
    flash->state.active = (s->ticks & 1U) != 0;
    Sint32 visible = a->olrox_death.remaining;
    ++a->olrox_death.ticks;
    for (Uint16 i = a->olrox_death.copied_end; i < a->state.primitive_count; ++i)
    {
        ActorPrimitive *p = Part(s, a, i);
        if (!p)
            return false;
        if (!p->state.active)
            continue;
        ++visible;
        Uint8 *red = &p->state.vertex_red[0];
        --*red;
        switch (*red >> 4)
        {
        default:
            for (Uint16 k = 0; k < 4; ++k)
                p->state.offset_x[k] =
                    Word(p->state.offset_x[k] + (a->olrox_death.ticks & 1U ? 1 : -1));
            break;
        case 1:
            if (*red == 16)
            {
                Uint16 out = SDL_MAX_UINT16;
                if (!Find(s, a, a->olrox_death.copied_end, 4, false, &out))
                    return false;
                if (out != SDL_MAX_UINT16)
                {
                    p->olrox_effect.tag = 3;
                    p->state.vertex_red[0] = 13;
                    p->state.vertex_red[1] = 6;
                    p->state.vertex_red[2] = 14;
                    p->state.vertex_red[3] = 20;
                    if (!Split(s, a, p, out))
                        return false;
                    p->state.active = false;
                    p->olrox_effect.tag = 0;
                }
                else
                    *red = (Uint8)(*red + (ActorRandom(s) & 15U));
            }
            for (Uint16 k = 0; k < 4; ++k)
                p->state.offset_y[k] = Word(p->state.offset_y[k] - 1);
            break;
        case 0:
            for (Uint16 k = 0; k < 4; ++k)
                p->state.offset_y[k] = Word(p->state.offset_y[k] + ((Sint32)(*red >> 1) - 8));
            if (!*red)
            {
                p->state.active = false;
                p->olrox_effect.tag = 0;
            }
            break;
        }
    }
    if (visible < 0)
    {
        Actor *body = Body(s, a);
        if (body)
            body->olrox.death_phase = 2;
        if (a->olrox_death.effect_generation && a->olrox_death.effect_slot < SOTN_STAGE_ACTOR_SLOTS)
        {
            Actor *rays = &s->slots[a->olrox_death.effect_slot];
            if (rays->state.id == 0x3e &&
                rays->state.generation == a->olrox_death.effect_generation)
                ++rays->state.step;
        }
        ++a->state.step;
    }
    return true;
}
bool ActorOlroxDefeat(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 30 || !a->state.generation)
        return SDL_SetError("Olrox defeat requires an active original BO0 owner");
    if (a->state.id == 0x3e)
        return ActorOlroxDeathRays(s, a);
    if (a->state.id != 0x3c)
        return SDL_SetError("Olrox defeat requires identity 0x3c or 0x3e");
    Actor *body = Body(s, a);
    switch (a->state.step)
    {
    case 0:
        if (!body)
            return SDL_SetError("Olrox initial capture lost its owned body");
        if (!ActorInitialize(s, a, 0x708))
            return false;
        a->state.priority = (Uint16)(s->olrox_monster.priority + 8U);
        a->state.frame = 0;
        a->state.flags |= 0x08000000U;
        if (!ActorPrimitives(s, a, 264))
        {
            SDL_ClearError();
            a->remove = true;
            return true;
        }
        a->state.flags |= 0x800000U;
        return Capture(s, a, body, true);
    case 1:
        return FirstFragments(s, a, body);
    case 2:
        return PrepareFinal(s, a, body);
    case 3:
        return Expand(s, a);
    case 4:
    case 5:
        return LastFragments(s, a);
    case 6: {
        ActorPrimitive *p = Part(s, a, 0);
        if (!p)
            return false;
        p->state.active = true;
        const Sint16 left = Word(p->state.offset_x[0] + 2), right = Word(p->state.offset_x[1] - 2);
        p->state.offset_x[0] = p->state.offset_x[2] = left;
        p->state.offset_x[1] = p->state.offset_x[3] = right;
        if (right < left)
        {
            p->state.active = false;
            a->olrox_death.timer = 1;
            ++a->state.step;
        }
        return true;
    }
    case 7: {
        Actor *reward = NULL;
        if (!ActorSpawn(s, a, 0x3f, 0, 96, 128, &reward))
            return false;
        if (reward)
        {
            reward->state.facing_left = false;
            reward->state.y = Word(Pixel(reward->state.y) - 40) * 65536;
            s->olrox_defeated = true;
            s->combat.music_change = true;
            s->combat.music = 0;
            ++a->state.step;
        }
        return true;
    }
    case 8:
        if (!s->music_playing)
        {
            s->combat.music_change = true;
            s->combat.music = 0x317;
            a->remove = true;
        }
        return true;
    default:
        return SDL_SetError("Olrox capture controller has an invalid original phase");
    }
}
