// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/lib/e_dhuron.c:func_us_801CC984 from the pinned reconstruction.
// Native owned quads replace packet pointers; SDK trigonometric tables stay private.
#include "actors_internal.h"
static Actor *Owner(SotnActors *s, const Actor *a)
{
    if (a->enemy_state.owner_slot >= SOTN_STAGE_ACTOR_SLOTS)
        return NULL;
    Actor *p = &s->slots[a->enemy_state.owner_slot];
    return p->state.id == 0x33 && p->state.generation != 0 &&
                   p->state.generation == a->enemy_state.owner_generation
               ? p
               : NULL;
}
bool ActorLibraryDhuronCharge(SotnActors *s, Actor *a)
{
    Actor *parent = Owner(s, a);
    if (!parent)
    {
        a->remove = true;
        return true;
    }
    switch (a->state.step)
    {
    case 0: {
        a->state.y = s->camera_y * 65536 + (Sint32)((Uint32)a->state.y & 65535U);
        if (!ActorInitialize(s, a, 0x830))
            return false;
        if (!ActorPrimitives(s, a, 40))
        {
            a->remove = true;
            return true;
        }
        if (!a->state.priority)
            return SDL_SetError("Dhuron charge primitive priority underflow");
        for (Uint16 i = 0; i < 40; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (!p)
                return false;
            const Uint8 u = (Uint8)((i % 6) * 16 + 144);
            p->state = (SotnActorPrimitive){.x = Pixel(a->state.x) * 65536,
                                            .y = s->camera_y * 65536,
                                            .quad = true,
                                            .textured = true,
                                            .blend = true,
                                            .modulate = true,
                                            .page = 0x3a,
                                            .palette = 0x194,
                                            .u = {u, u, (Uint8)(u + 16), (Uint8)(u + 16)},
                                            .v = {208, 192, 208, 192},
                                            .red = 80,
                                            .green = 80,
                                            .blue = 80,
                                            .priority_set = true,
                                            .priority = (Uint16)(a->state.priority - 1)};
        }
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
        if (!p)
            return false;
        p->state.offset_y[1] = p->state.offset_y[3] = -16;
        a->dhuron.angle = 3072;
        a->dhuron.turn_ticks = 0;
        a->dhuron.beam_index = 0;
        break;
    }
    case 1: {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, a->dhuron.beam_index);
        if (!p)
            return false;
        Sint32 angle = a->dhuron.angle;
        Sint32 dx = Pixel(parent->state.x) + (parent->state.facing_left ? -7 : 7) -
                    Pixel(p->state.x) - p->state.offset_x[2];
        Sint32 dy = Pixel(parent->state.y) - 48 - Pixel(p->state.y) - p->state.offset_y[2];
        if (SDL_abs(dx) < 8 && SDL_abs(dy) < 8)
        {
            a->state.step = 2;
            return true;
        }
        const bool near = SDL_abs(dx) < 64 && SDL_abs(dy) < 64;
        if (!a->dhuron.turn_ticks)
        {
            a->dhuron.turn_ticks = near ? 2 : 4;
            Sint16 target = 0;
            if (!SotnMath_Atan2(s->executable, Wrap16(-(Sint64)dy), Wrap16(dx), &target))
                return false;
            Sint32 delta = Wrap16((Sint32)target - angle);
            if (delta > 2048)
                delta = 4096 - delta;
            if (delta < -2048)
                delta += 4096;
            a->dhuron.delta = (Sint16)(delta / (near ? 2 : 4));
        }
        angle = Wrap16(angle + a->dhuron.delta);
        if (!near)
            angle += 384 - (Sint32)((ActorRandom(s) & 3U) << 8);
        angle &= 4095;
        if (a->dhuron.beam_index >= 39)
        {
            a->state.step = 2;
            return true;
        }
        ActorPrimitive *next = ActorPrimitiveGet(s, a, ++a->dhuron.beam_index);
        if (!next)
            return false;
        next->state.offset_x[0] = p->state.offset_x[2];
        next->state.offset_y[0] = p->state.offset_y[2];
        next->state.offset_x[1] = p->state.offset_x[3];
        next->state.offset_y[1] = p->state.offset_y[3];
        SotnMathTrig forward = {0}, normal = {0};
        if (!SotnMath_Trig(s->executable, angle, &forward) ||
            !SotnMath_Trig(s->executable, angle - 1024, &normal))
            return false;
        next->state.offset_x[2] = (Sint16)Wrap16((Sint32)next->state.offset_x[0] +
                                                 ActorShift((Sint64)forward.cosine * 12, 12));
        next->state.offset_y[2] = (Sint16)Wrap16((Sint32)next->state.offset_y[0] -
                                                 ActorShift((Sint64)forward.sine * 12, 12));
        next->state.offset_x[3] = (Sint16)Wrap16((Sint32)next->state.offset_x[2] +
                                                 ActorShift((Sint64)normal.cosine * 16, 12));
        next->state.offset_y[3] = (Sint16)Wrap16((Sint32)next->state.offset_y[2] -
                                                 ActorShift((Sint64)normal.sine * 16, 12));
        next->state.active = true;
        a->dhuron.angle = (Sint16)angle;
        --a->dhuron.turn_ticks;
        break;
    }
    case 2:
        for (Uint16 i = 0; i < a->state.primitive_count; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (!p)
                return false;
            p->state.active = false;
        }
        parent->dhuron.charged = true;
        a->state.x = Pixel(parent->state.x) * 65536 + (Sint32)((Uint32)a->state.x & 65535U);
        a->state.y = Wrap16((Sint64)Pixel(parent->state.y) + 24) * 65536 +
                     (Sint32)((Uint32)a->state.y & 65535U);
        a->state.scale_y = 384;
        a->state.opacity = 128;
        a->state.opacity_set = true;
        a->state.blend_mode = 0x70;
        a->state.palette = 0x815b;
        a->state.texture_offset = 0x4e;
        a->state.animation_set = 0x8004;
        a->state.frame = a->pose = a->remaining = 0;
        a->state.step = 3;
        break;
    case 3: {
        Uint8 animation = 0;
        if (!ActorAnimate(s, a, 0x2acc, &animation))
            return false;
        if (!animation)
            a->state.step = 4;
        break;
    }
    case 4:
        if (a->state.y < SDL_MIN_SINT32 + 16384 || a->state.opacity < 8)
            return SDL_SetError("Dhuron charge fade exceeds its original bounded lifetime");
        a->state.y -= 16384;
        a->state.opacity = (Uint16)(a->state.opacity - 8);
        if (!a->state.opacity)
            a->remove = true;
        break;
    default:
        return SDL_SetError("Dhuron charge entered an invalid step");
    }
    return true;
}
