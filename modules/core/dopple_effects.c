// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted from pinned sotn-decomp bo4/unk_49A6C.c:EntitySmokePuff,
// EntityWingSmashTrail, func_us_801CA748 and func_us_801CD178.
// Profiles, animation and sensor order remain private.
#include "actors_internal.h"
static Sint32 Position(Sint32 value, Sint32 delta)
{
    const Uint32 bits = (Uint32)value + (Uint32)delta;
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static void ShiftX(Actor *a, Sint32 pixels)
{
    a->state.x = Position(a->state.x, pixels * 65536);
}
static bool Animation(SotnActors *s, Actor *a)
{
    SotnDataView entry = {0};
    if (a->pose >= 24 || !Slice(s->nz0, 0x17a8 + a->pose * 4U, 4, &entry))
        return SDL_SetError("BO4 smoke pose exceeds its private timeline");
    if (a->remaining == 0)
        a->remaining = Read16(entry.bytes);
    else if (a->remaining != 0xffff && --a->remaining == 0)
    {
        ++a->pose;
        if (!Slice(s->nz0, 0x17a8 + a->pose * 4U, 4, &entry))
            return false;
        if (Read16(entry.bytes) == 0xffff)
        {
            --a->pose;
            a->remaining = 0xffff;
            if (!Slice(s->nz0, 0x17a8 + a->pose * 4U, 4, &entry))
                return false;
        }
        else
            a->remaining = Read16(entry.bytes);
    }
    if (!Read16(entry.bytes) || Read16(entry.bytes) > 32767 || Read16(entry.bytes + 2) >= 512)
        return SDL_SetError("BO4 smoke resolved an invalid original pose");
    a->state.frame = Read16(entry.bytes + 2);
    return true;
}
bool ActorDoppleSmoke(SotnActors *s, Actor *a)
{
    if (s == NULL || a == NULL || s->stage_id != 26 || a->state.id != 0x102)
        return SDL_SetError("BO4 smoke dispatch requires its original child type");
    const Uint16 kind = a->state.parameters >> 8;
    Uint16 index = a->state.parameters & 255U;
    if (kind > 10 || index >= 6)
        return SDL_SetError("BO4 smoke parameters exceed its private profiles");
    if (a->state.step == 0)
    {
        SotnDataView offsets = {0}, speeds = {0}, scales = {0};
        if (!Slice(s->nz0, 0x1778, 12, &offsets) || !Slice(s->nz0, 0x1784, 24, &speeds) ||
            !Slice(s->nz0, 0x179c, 12, &scales))
            return false;
        if (s->slots[0].state.priority > 509)
            return SDL_SetError("BO4 smoke priority exceeds its original ordering table");
        for (Uint16 i = 0; i < 6; ++i)
            if (Signed16(offsets.bytes + i * 2U) < -256 || Signed16(offsets.bytes + i * 2U) > 256 ||
                Signed32(speeds.bytes + i * 4U) < -65536 || Signed32(speeds.bytes + i * 4U) > 0 ||
                !Read16(scales.bytes + i * 2U) || Read16(scales.bytes + i * 2U) > 960)
                return SDL_SetError(
                    "BO4 smoke profile exceeds bounded original offsets, speeds or scales");
        a->state.animation_set = 5;
        a->state.priority = (Uint16)(s->slots[0].state.priority + 2);
        a->state.flags = 0x28000000U;
        a->state.palette = 0x8195;
        a->state.blend_mode = 0x10;
        Sint32 x = Signed16(offsets.bytes + index * 2U);
        if (kind == 0)
            x += 6;
        if (kind == 1)
            x -= 8;
        if (kind == 2 || kind == 10)
            x -= 2;
        if (kind == 5)
            x = -6;
        if (kind == 6)
            x = -24;
        if (kind == 3 || kind == 7 || kind == 9)
        {
            const Uint16 range = kind == 3 ? 30 : kind == 7 ? 60 : 16;
            const Sint16 origin = kind == 3 ? 14 : kind == 7 ? 30 : 8;
            x = (Sint32)(SotnCombat_Random(&s->combat_random) % range) - origin;
            index = (Uint16)((SotnCombat_Random(&s->combat_random) & 3U) + 2);
            ShiftY(a, -(Sint32)(SotnCombat_Random(&s->combat_random) % 4U) - (kind == 9 ? 20 : 0));
        }
        if (kind == 4 || kind == 8)
        {
            SotnDataView order = {0};
            const Uint16 count = kind == 4 ? 14 : 10;
            if (!Slice(s->nz0, kind == 4 ? 0x180c : 0x181c, count, &order))
                return false;
            Uint16 probe = (Uint16)(index * 2U);
            while (probe < count)
            {
                if (order.bytes[probe] >= 14)
                    return SDL_SetError("BO4 smoke wall sensor exceeds its original probes");
                if (s->dopple.body.motion.wall[order.bytes[probe]].effects & SOTN_EFFECT_BLOCK)
                    break;
                ++probe;
            }
            if (probe == count)
            {
                a->remove = true;
                return true;
            }
            const SotnDoppleSensor sensor = s->dopple.body.motion.wall_sensors[order.bytes[probe]];
            a->state.x = (Pixel(s->dopple.body.motion.x) + sensor.x) * 65536 +
                         (Sint32)((Uint32)a->state.x & 65535U);
            a->state.y = (Pixel(s->dopple.body.motion.y) + sensor.y) * 65536 +
                         (Sint32)((Uint32)a->state.y & 65535U);
            a->vy = kind == 4 ? -16384 : Signed32(speeds.bytes + index * 4U);
            a->state.scale_x = (Uint16)(Read16(scales.bytes + (kind == 4 ? 1 : index) * 2U) +
                                        (kind == 4 ? 64 : 32));
        }
        else
        {
            if (kind == 1 && (s->dopple.body.motion.contact & 0x8000U))
                x /= 2;
            if (kind == 6 ? s->dopple.body.motion.vx > 0 : a->state.facing_left)
                x = -x;
            ShiftX(a, x);
            ShiftY(a, 24);
            a->state.scale_x = (Uint16)(Read16(scales.bytes + index * 2U) + 64);
            a->vy = Signed32(speeds.bytes + index * 4U);
            if (kind == 1)
            {
                a->vy = -16384;
                a->vx = a->state.facing_left ? 12288 : -12288;
                a->state.scale_x = (Uint16)(Read16(scales.bytes + 2) + 64);
            }
            if (kind == 5)
            {
                if (index > 2)
                    return SDL_SetError("BO4 smoke alternate velocity index underflows its table");
                a->vy = Signed32(speeds.bytes + (4U - index * 2U) * 4U);
            }
            if (kind == 10)
                ShiftY(a, -6);
        }
        if (a->state.scale_x == 0 || a->state.scale_x > 1024)
            return SDL_SetError("BO4 smoke scale exceeds its supported original bounds");
        a->state.scale_y = a->state.scale_x;
        a->state.step = 1;
    }
    else if (a->state.step == 1)
    {
        a->state.x = Position(a->state.x, a->vx);
        a->state.y = Position(a->state.y, a->vy);
        if (a->remaining == 0xffff)
        {
            a->remove = true;
            return true;
        }
    }
    else
        return SDL_SetError("BO4 smoke entered an invalid step");
    return Animation(s, a);
}
bool ActorDoppleTrail(SotnActors *s, Actor *a)
{
    if (s == NULL || a == NULL || s->stage_id != 26 || a->state.id != 0x123)
        return SDL_SetError("BO4 trail dispatch requires its original child type");
    const SotnDoppleBody *body = &s->dopple.body;
    if (body->step != 6 || body->substep != 3)
    {
        a->remove = true;
        return true;
    }
    a->state.frame = body->animation.sprite | 0x8000U;
    if (!a->state.step)
    {
        a->state.flags = 0x08000000U;
        a->state.animation_set = body->animation_set;
        a->state.priority =
            s->slots[0].state.priority >= 2 ? (Uint16)(s->slots[0].state.priority - 2) : 0;
        a->state.opacity_set = true;
        a->state.opacity = 128;
        a->state.blend_mode = 0x30;
        a->state.rotation = body->rotation;
        a->state.rotated = body->rotated;
        a->state.facing_left = body->facing_left;
        a->state.palette = 0x8202;
        a->state.scale_x = a->state.scale_y = 256;
        a->state.step = 1;
        return true;
    }
    if (a->state.step != 1 || a->state.scale_x < 8 || a->state.scale_y < 8)
        return SDL_SetError("BO4 wing trail has an invalid scale or step");
    a->state.scale_x -= 8;
    a->state.scale_y -= 8;
    if (a->state.opacity >= 5)
        a->state.opacity -= 5;
    else
        a->remove = true;
    return true;
}
bool ActorDoppleWingHitbox(SotnActors *s, Actor *a)
{
    if (s == NULL || a == NULL || s->stage_id != 26 || a->state.id != 0x124)
        return SDL_SetError("BO4 wing hitbox dispatch requires its original child type");
    const SotnDoppleBody *body = &s->dopple.body;
    if (body->step != 6 || body->substep != 3)
    {
        a->remove = true;
        return true;
    }
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x434))
            return false;
        if (body->motion.status & 0x100U)
            a->attack /= 2;
        a->state.hitbox_x = 4;
        ++a->state.step;
    }
    a->state.flags = 0x19000000U;
    a->state.x = body->motion.x;
    a->state.y = body->motion.y;
    a->state.facing_left = body->facing_left;
    return true;
}
static Uint16 Random(SotnActors *s)
{
    return SotnCombat_Random(&s->combat_random);
}
static bool ParticleProfile(SotnActors *s, Uint16 index, SotnDataView *out)
{
    SotnDataView pointer = {0};
    if (index >= 12 || !Slice(s->nz0, 0x1d0c + index * 4U, 4, &pointer))
        return SDL_SetError("BO4 particle parameters exceed their original profiles");
    const Uint32 address = Read32(pointer.bytes);
    if (address < 0x80180000U || (address & 3U) || !Slice(s->nz0, address - 0x80180000U, 16, out))
        return SDL_SetError("BO4 particle profile points outside its private overlay");
    if (!out->bytes[0] || out->bytes[0] > 32 || !out->bytes[4] || !out->bytes[5] ||
        out->bytes[4] > 16 || out->bytes[5] > 16 || Read16(out->bytes + 10) > 9)
        return SDL_SetError("BO4 particle profile has invalid count, size or motion kind");
    return true;
}
static bool ParticleStart(SotnActors *s, Actor *a, ActorPrimitive *p, Uint16 index, Uint16 kind)
{
    p->state.x = Pixel(a->state.x) * 65536;
    p->state.y = Pixel(a->state.y) * 65536;
    switch (kind)
    {
    case 0: {
        const Uint16 angle = Random(s);
        const Sint32 shift = (Sint32)(angle & 1U) + 2;
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(s->executable, angle, &trig))
            return false;
        p->vx = trig.cosine * (1 << shift);
        p->vy = -trig.sine * (1 << shift);
        break;
    }
    case 1:
    case 9:
        p->vx = ((Sint32)(Random(s) & 511U) - 256) * 256;
        p->vy = ((Sint32)(Random(s) & 511U) - 256) * 256;
        break;
    case 2:
        p->vx = ActorShift((Sint32)Random(s) * 2 - 32768, 1);
        p->vy = -(Sint32)(Random(s) & 16383U);
        p->state.x = Position(p->state.x, p->vx * 19);
        p->state.y = Position(p->state.y, ((Sint32)(Random(s) & 7U) - 3) * 65536);
        p->timer = (Uint16)((Random(s) & 15U) + 16);
        break;
    case 8: {
        const Sint32 origin = s->dopple.body.facing_left ? 12 : -12;
        p->vx = (Sint32)Random(s) * 4 - 65536;
        p->vy = -(Sint32)(Random(s) & 16383U);
        p->state.x = Position(p->state.x, p->vx * 35 + origin * 65536);
        p->vx = ActorShift(p->vx, 2);
        p->state.y = Position(p->state.y, -(Sint32)(Random(s) & 31U) * 65536);
        p->timer = (Uint16)((Random(s) & 31U) + 16);
        break;
    }
    case 3:
        p->state.x = Position(p->state.x, ((Sint32)(Random(s) & 15U) - 7) * 65536);
        p->state.y = Position(p->state.y, -(Sint32)(Random(s) & 31U) * 65536);
        p->vy = -49152 - (Sint32)(Random(s) & 32767U);
        if (a->enemy_state.owner_slot >= SOTN_STAGE_ACTOR_SLOTS ||
            !a->enemy_state.owner_generation ||
            s->slots[a->enemy_state.owner_slot].state.generation != a->enemy_state.owner_generation)
            return SDL_SetError("BO4 particle inherited velocity lost its original parent");
        p->vx = s->slots[a->enemy_state.owner_slot].vx;
        p->timer = (Uint16)(index * 2U + 15);
        break;
    case 4: {
        p->state.x = Position(p->state.x, ((Sint32)(Random(s) & 31U) - 16) * 65536);
        p->state.y = Position(p->state.y, ((Sint32)(Random(s) & 31U) - 20) * 65536);
        const Uint16 sample = Random(s) & 31U;
        p->vx = s->dopple.debris_x[sample];
        p->vy = s->dopple.debris_y[sample];
        break;
    }
    default:
        return SDL_SetError("BO4 particle profile selects an unintegrated motion kind");
    }
    return true;
}
bool ActorDoppleParticles(SotnActors *s, Actor *a)
{
    if (s == NULL || a == NULL || s->stage_id != 26 || a->state.id != 0x105)
        return SDL_SetError("BO4 particle dispatch requires its original child type");
    const Uint16 parameter = a->state.parameters >> 8;
    SotnDataView profile = {0};
    if (!ParticleProfile(s, parameter, &profile))
        return false;
    const Uint16 kind = Read16(profile.bytes + 10), count = profile.bytes[0];
    if (a->state.step == 0)
    {
        const Sint32 priority = s->slots[0].state.priority + Signed16(profile.bytes + 6);
        if (priority < 0 || priority > 511)
            return SDL_SetError("BO4 particle priority exceeds the original ordering table");
        // Retain the original final, non-drawing control primitive in the allocation budget.
        if (!ActorPrimitivesContiguous(s, a, (Uint16)(count + 1)))
        {
            a->remove = true;
            return true;
        }
        a->state.flags = Read32(profile.bytes + 12);
        a->state.priority = (Uint16)priority;
        if (parameter == 8)
            a->timer = 63;
        if (parameter == 3 || parameter == 5)
            a->timer = 47;
        if (parameter == 4 || parameter == 7 || parameter == 11)
            a->timer = 31;
        for (Uint16 i = 0; i < count; ++i)
        {
            ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
            if (p == NULL || !ParticleStart(s, a, p, i, kind))
                return false;
            const Uint16 mode = Read16(profile.bytes + 8);
            p->state.active = !(mode & 8U);
            p->state.blend = (mode & 1U) != 0;
            p->state.page = (Uint16)(mode & 0x60U);
            p->state.red = profile.bytes[1];
            p->state.green = profile.bytes[2];
            p->state.blue = profile.bytes[3];
            p->state.width = profile.bytes[4];
            p->state.height = profile.bytes[5];
        }
        a->state.step = 1;
        return true;
    }
    if (a->state.step != 1 || a->state.primitive_count != count + 1U)
        return SDL_SetError("BO4 particle update has an invalid original pool or step");
    Uint16 thickness = 2;
    if (parameter == 3 || parameter == 4 || parameter == 7 || parameter == 8 || parameter == 11)
        if (--a->timer == 0)
        {
            a->remove = true;
            return true;
        }
    if (parameter == 3 || parameter == 4 || parameter == 5 || parameter == 7 || parameter == 8 ||
        parameter == 11)
    {
        if (--a->timer == 0)
        {
            a->remove = true;
            return true;
        }
        if (a->timer < 9)
            --thickness;
    }
    for (Uint16 i = 0; i < count; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (p == NULL)
            return false;
        switch (kind)
        {
        case 0:
        case 1:
        case 9: {
            p->state.y = Position(p->state.y, p->vy);
            p->state.x = Position(p->state.x, p->vx);
            if (kind != 0)
                p->vy = Position(p->vy, 10240);
            const Uint8 fade = kind == 0 ? 6 : 3;
            p->state.red = (Uint8)(p->state.red - fade);
            p->state.green = (Uint8)(p->state.green - fade);
            p->state.blue = (Uint8)(p->state.blue - fade);
            if (p->state.red < 8)
            {
                a->remove = true;
                return true;
            }
            break;
        }
        case 2:
        case 8:
            if (--p->timer == 0)
                p->state.active = false;
            p->state.x = Position(p->state.x, p->vx);
            p->state.y = Position(p->state.y, p->vy);
            p->vy = Position(p->vy, 5120);
            break;
        case 3:
            p->state.y = Position(p->state.y, p->vy);
            --p->timer;
            if (p->timer & 0x8000U)
                p->state.active = false;
            break;
        case 4:
            p->state.x = Position(p->state.x, p->vx);
            p->state.y = Position(p->state.y, p->vy);
            p->vy = Position(p->vy, -ActorShift(p->vy, 5));
            if (!(a->timer & 7U))
            {
                p->vx = ActorShift(p->vx, 1);
                p->vy = ActorShift(p->vy, 1);
                if (a->timer & 32U)
                    p->vy = ActorShift(p->vy, 1);
            }
            p->state.width = p->state.height = thickness;
            break;
        default:
            return SDL_SetError("BO4 particle update selects an unsupported motion kind");
        }
    }
    return true;
}
