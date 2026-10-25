// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected death helper in st/e_blue_axe_knight.h at the pinned reference.
#include "actors_internal.h"
static bool Burst(SotnActors *s, Actor *a, Uint16 id, Uint16 kind, Sint16 mask, Sint16 x, Sint16 y)
{
    Actor *effect = NULL;
    if (!ActorSpawn(s, a, id, kind, 160, 192, &effect))
        return false;
    if (effect != NULL)
    {
        effect->state.x = (Pixel(a->state.x) + (Sint32)(ActorRandom(s) & (Uint16)mask) + x) * 65536;
        effect->state.y = (Pixel(a->state.y) + (Sint32)(ActorRandom(s) & 31) + y) * 65536;
        effect->state.facing_left = false;
    }
    return true;
}
bool ActorBlueDeath(SotnActors *s, Actor *a, bool *done)
{
    *done = false;
    if (a->substep < 16)
        return ActorKnightShatter(s, a, done);
    switch (a->substep)
    {
    case 16: {
        Actor *effect = NULL;
        if (!ActorSpawn(s, a, 0x23, 0, 160, 192, &effect))
            return false;
        if (effect != NULL)
        {
            effect->state.x = Pixel(a->state.x) * 65536;
            effect->state.y = Pixel(a->state.y) * 65536;
            effect->state.facing_left = false;
        }
        if (!ActorSound(s, 0x695))
            return false;
        ++a->substep;
        return true;
    }
    case 17: {
        Uint16 available = 0;
        for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
            available = (Uint16)(available + (s->primitives[i].owner == 0));
        if (available < 2)
        {
            *done = true;
            return true;
        }
        if (!ActorPrimitives(s, a, 2) || !ActorSound(s, 0x672))
            return false;
        a->blue.size = a->blue.ticks = 0;
        a->blue.brightness = 128;
        ++a->substep;
        break;
    }
    case 18:
        if (a->blue.ticks > 6)
        {
            a->blue.size = (Uint16)(a->blue.size + 14);
            a->blue.brightness = (Uint8)(a->blue.brightness - 4);
        }
        if (++a->blue.ticks > 10)
            ++a->substep;
        break;
    case 19:
        ++a->blue.size;
        a->blue.brightness = (Uint8)(a->blue.brightness - 4);
        if ((s->ticks % 5) == 0 && !Burst(s, a, 6, 0, 31, -16, -16))
            return false;
        if (++a->blue.ticks > 38)
        {
            a->blue.ticks = 0;
            if (!ActorSound(s, 0x695))
                return false;
            ++a->substep;
            a->blue.size = 8;
            a->blue.brightness = 64;
        }
        break;
    case 20:
        if ((s->ticks % 8) == 0 && !Burst(s, a, 2, 1, 15, -8, 0))
            return false;
        a->blue.size = (Uint16)(a->blue.size + 16);
        a->blue.brightness = (Uint8)(a->blue.brightness - 3);
        if (++a->blue.ticks > 20)
        {
            a->state.animation_set = 0;
            *done = true;
        }
        break;
    default:
        return SDL_SetError("Blue Axe Knight death effect substate is invalid");
    }
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    if (p == NULL || !ActorSprite(s, a, 0, Pixel(a->state.x) * 65536, Pixel(a->state.y) * 65536, 0,
                                  192, 63, 63, a->substep == 20 ? 0x3a : 0x1a, 0x19d))
        return false;
    p->state.width = p->state.height = a->blue.size;
    p->state.red = p->state.green = p->state.blue = a->blue.brightness;
    p->state.blend = p->state.modulate = true;
    p->state.active = a->blue.size != 0 && a->blue.brightness != 0;
    return true;
}
