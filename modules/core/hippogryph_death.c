// SPDX-License-Identifier: AGPL-3.0-or-later
// US BO5 func_us_801A1C14 state36, pinned sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Ray/spark speed arrays remain private.
#include "hippogryph_internal.h"

static Sint32 Bits(Uint32 value)
{
    return value <= SDL_MAX_SINT32 ? (Sint32)value : -1 - (Sint32)~value;
}
static Sint32 Shift12(Sint64 value)
{
    return (Sint32)(value >= 0 ? value / 4096 : -((-value + 4095) / 4096));
}
static bool Move(Actor *a)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32 ||
        a->vy > SDL_MAX_SINT32 - 16384)
        return SDL_SetError("Hippogryph defeat motion exceeds bounded fixed-point position");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    a->vy += 16384;
    return true;
}
static void Free(SotnActors *s, Actor *a)
{
    ActorPrimitivesClear(s, a);
    a->state.primitive_count = 0;
    a->state.flags &= ~0x800000U;
}
static bool Reserve(SotnActors *s, Actor *a, Uint16 count, bool *allocated)
{
    *allocated = false;
    if (!a->state.generation || a->state.primitive_count || count > ACTOR_PRIMITIVE_LIMIT)
        return SDL_SetError("Hippogryph defeat requires an active primitive-free owner");
    Uint32 available = 0;
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        available += s->primitives[i].owner == 0;
    if (available < count)
        return true; // The original allocation failure selects another defeat phase.
    if (!ActorPrimitives(s, a, count))
        return false;
    a->state.flags |= 0x800000U;
    *allocated = true;
    return true;
}
static SotnActorPrimitive Line(const Actor *a, Uint8 red, Uint8 green, Uint8 blue, Uint8 end_red,
                               Uint8 end_green, Uint8 end_blue)
{
    return (SotnActorPrimitive){.x = Pixel(a->state.x) * 65536,
                                .y = Pixel(a->state.y) * 65536,
                                .priority = (Uint16)(a->state.priority + 2U),
                                .priority_set = true,
                                .line = true,
                                .active = true,
                                .blend = true,
                                .red = red,
                                .green = green,
                                .blue = blue,
                                .line_red = end_red,
                                .line_green = end_green,
                                .line_blue = end_blue,
                                .line_blend_mode = 1};
}
static bool OpeningLines(SotnActors *s, Actor *a)
{
    bool allocated = false;
    if (!Reserve(s, a, 2, &allocated))
        return false;
    a->substep = allocated ? 4 : 5;
    if (allocated)
        for (Uint16 i = 0; i < 2; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (!p)
                return false;
            p->state = Line(a, 128, 96, 96, 64, 32, 32);
        }
    a->state.priority = (Uint16)((Sint32)a->hippogryph.saved_priority - 8);
    return true;
}
static bool MoveOpeningLines(SotnActors *s, Actor *a)
{
    if (a->state.primitive_count != 2)
        return SDL_SetError("Hippogryph opening flash lost its two original lines");
    a->hippogryph.timer = (Uint8)(a->hippogryph.timer + 6U);
    const Sint32 delta = a->hippogryph.timer <= 128 ? 6 : -8;
    for (Uint16 i = 0; i < 2; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        p->state.offset_x[1] = (Sint16)Wrap16((Sint64)p->state.offset_x[1] + (i ? -delta : delta));
    }
    if (a->hippogryph.timer > 240)
    {
        Free(s, a);
        a->hippogryph.timer = 0;
        ++a->substep;
    }
    return true;
}
static bool Speed(SotnActors *s, size_t offset, Sint32 *out)
{
    SotnDataView data = {0};
    if (!Slice(s->nz0, offset, 4, &data))
        return false;
    const Sint32 speed = Signed32(data.bytes);
    if (speed < 0 || speed > 7 * 65536)
        return SDL_SetError("Hippogryph private defeat speed exceeds its bounded multiplier");
    *out = speed;
    return true;
}
static bool Burst(SotnActors *s, Actor *a)
{
    bool allocated = false;
    if (!Reserve(s, a, 111, &allocated))
        return false;
    if (!allocated)
    {
        a->substep = 8;
        return true;
    }
    for (Uint16 i = 0; i < 31; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p || !Speed(s, 0x6e4U + i * 4U, &p->hippogryph.speed))
            return false;
        p->state = Line(a, 96, 64, 64, 16, 8, 8);
    }
    for (Uint16 i = 31; i < 111; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        p->state = (SotnActorPrimitive){.x = Pixel(a->state.x) * 65536,
                                        .y = Pixel(a->state.y) * 65536,
                                        .width = 2,
                                        .height = 2,
                                        .active = true,
                                        .blend = true,
                                        .red = 112,
                                        .green = 112,
                                        .blue = 112,
                                        .priority = (Uint16)(a->state.priority + 2U),
                                        .priority_set = true};
        const Uint32 index = ActorRandom(s) & 7U;
        const Sint32 angle = (Sint32)(ActorRandom(s) & 15U) * 256;
        Sint32 speed = 0;
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(s->executable, angle, &trig) || !Speed(s, 0x7e4U + index * 4U, &speed))
            return false;
        p->vx = Shift12((Sint64)trig.sine * speed);
        p->vy = Shift12((Sint64)trig.cosine * speed);
        p->hippogryph.delay = (Uint8)(ActorRandom(s) & 0x17U);
    }
    ++a->substep;
    return true;
}
static bool Sparks(SotnActors *s, Actor *a)
{
    if (a->state.primitive_count != 111)
        return SDL_SetError("Hippogryph defeat burst lost its original 111 primitive owners");
    for (Uint16 i = 31; i < 111; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        if (p->hippogryph.delay)
        {
            --p->hippogryph.delay;
            continue;
        }
        p->state.x = Bits((Uint32)p->state.x + (Uint32)p->vx);
        p->state.y = Bits((Uint32)p->state.y + (Uint32)p->vy);
        ++p->hippogryph.age;
        if (p->hippogryph.age > 32)
            p->state.width = p->state.height = 1;
        if (p->hippogryph.age > 48)
            p->state.width = p->state.height = 0;
        p->state.active = p->state.width != 0;
    }
    return true;
}
static bool Rays(SotnActors *s, Actor *a, bool fade)
{
    for (Uint16 i = 0; i < 31; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        Sint32 speed = p->hippogryph.speed;
        if (fade)
        {
            p->state.red = (Uint8)(p->state.red - 2U);
            p->state.green = (Uint8)(p->state.green - 1U);
            p->state.blue = (Uint8)(p->state.blue - 1U);
            // The source negates the two halves independently, without a borrow between them.
            const Uint32 bits = (Uint32)speed;
            speed =
                Bits(((Uint32)(Uint16)(0U - (bits >> 16)) << 16) | (Uint16)(0U - (bits & 65535U)));
        }
        if (a->hippogryph.timer < (fade ? 20 : 12))
            p->hippogryph.radius = Bits((Uint32)p->hippogryph.radius + (Uint32)speed);
        const Sint32 radius = Pixel(p->hippogryph.radius);
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(s->executable, (Sint32)i * 128 + a->hippogryph.timer * 16, &trig))
            return false;
        p->state.offset_x[1] = (Sint16)Wrap16(Shift12((Sint64)radius * trig.cosine));
        p->state.offset_y[1] = (Sint16)Wrap16(Shift12((Sint64)radius * trig.sine));
    }
    return true;
}
static bool Fade(SotnActors *s, Actor *a)
{
    const bool fade = a->substep == 7;
    Uint8 animated = 0;
    if ((!fade && !ActorAnimate(s, a, 0x9d8, &animated)) || !Sparks(s, a) || !Rays(s, a, fade))
        return false;
    if (fade)
    {
        a->state.opacity = (Uint8)(a->state.opacity - 4U);
        if (a->state.opacity > 224)
            a->state.opacity = 0;
    }
    const Uint8 before = a->hippogryph.timer++;
    if (!fade && before >= 49)
    {
        a->state.blend_mode = 0x30;
        a->state.opacity = 128;
        a->state.opacity_set = true;
        ++a->substep;
    }
    if (!(s->service_ticks & 7U) && !ActorPositionalSound(s, a, 0x6af))
        return false;
    if (fade && before >= 78)
    {
        Free(s, a);
        a->hippogryph.timer = 0;
        ++a->substep;
    }
    return true;
}
static bool Reward(SotnActors *s, Actor *a)
{
    Actor *reward = NULL;
    if (!ActorSpawn(s, a, 0x28, 6, 96, 128, &reward))
        return false;
    if (!reward)
        return true;
    reward->state.x = 256 * 65536;
    reward->state.y = 128 * 65536;
    reward->state.facing_left = false;
    s->hippogryph_scene = 0;
    Actor *dialogue = NULL;
    if (!ActorOwnedSpawnAt(s, a, 0x2a, 0, 136, &dialogue))
        return false;
    dialogue->state.x = Pixel(a->state.x) * 65536;
    dialogue->state.y = Pixel(a->state.y) * 65536;
    dialogue->state.facing_left = false;
    ++a->substep;
    return true;
}
bool HippogryphDeath(SotnActors *s, Actor *a)
{
    bool landed = false;
    switch (a->substep)
    {
    case 0:
        if (s->player.grab.state != SOTN_GRAB_NONE)
            HippogryphGrab(s, a, SOTN_GRAB_NONE);
        if (!SotnWorld_RecordTime(s->world, SOTN_TIME_HIPPOGRYPH, s->game_seconds))
            return false;
        a->vy = 0;
        s->hippogryph_flags |= 2U;
        if (Pixel(a->state.x) < 56)
            a->vx = 32768;
        if (Pixel(a->state.x) < 472)
            a->vx = -32768;
        ++a->substep;
        SDL_FALLTHROUGH;
    case 1:
        if (!Move(a))
            return false;
        if (Pixel(a->state.y) >= 129)
            ++a->substep;
        return true;
    case 2:
        if (!HippogryphFloor(s, a, &landed))
            return false;
        if (landed || Pixel(a->state.y) >= 184)
        {
            if (!HippogryphParticles(s, a, 22, -32))
                return false;
            if (!landed)
            {
                ShiftY(a, 183 - Pixel(a->state.y));
                a->vx = a->vy = 0;
            }
            a->substep = 3;
        }
        return true;
    case 3:
        return OpeningLines(s, a);
    case 4:
        return MoveOpeningLines(s, a);
    case 5:
        return Burst(s, a);
    case 6:
    case 7:
        return Fade(s, a);
    case 8:
        return Reward(s, a);
    default:
        return true;
    }
}
