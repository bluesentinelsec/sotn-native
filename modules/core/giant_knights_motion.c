// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted st/giantbro_helpers.h and giantbro_helpers_2.h at sotn-decomp
// 7bc54d8157b6. All joint lists, lengths and pose records remain private.
#include "giant_knights_internal.h"

static Sint32 Word(Sint64 value)
{
    const Uint32 bits = (Uint32)value;
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 0x100000000LL);
}
bool GiantPart(SotnActors *s, Actor *root, Uint16 offset, Actor **out)
{
    if (!s || !root || !out || s->stage_id != 4 || offset >= 20 ||
        (Uint32)root->slot + offset >= SOTN_STAGE_ACTOR_SLOTS || !root->state.generation ||
        (root->state.id != 0x37 && root->state.id != 0x3a && root->state.id != 0x3c))
        return SDL_SetError("Giant Knight requires a bounded original NO2 body/joint owner");
    if (!offset)
    {
        *out = root;
        return true;
    }
    Actor *part = &s->slots[root->slot + offset];
    if (!part->state.generation || part->state.generation != root->giant.generation[offset] ||
        part->enemy_state.owner_slot != root->slot ||
        part->enemy_state.owner_generation != root->state.generation ||
        (part->state.id != 0x38 && part->state.id != 0x39 && part->state.id != 0x3b &&
         part->state.id != 0x3d))
        return SDL_SetError("Giant Knight lost generation-owned joint %u", offset);
    *out = part;
    return true;
}
static bool List(SotnActors *s, size_t at, Uint16 offsets[20], Uint16 *count)
{
    *count = 0;
    for (Uint16 i = 0; i < 20; ++i)
    {
        SotnDataView item = {0};
        if (at > s->nz0.size || i * 2U > s->nz0.size - at || !Slice(s->nz0, at + i * 2U, 2, &item))
            return SDL_SetError("Giant Knight joint list exceeds its private view");
        const Uint16 offset = Read16(item.bytes);
        if (!offset)
            return true;
        if (offset != 255 && offset >= 20)
            return SDL_SetError("Giant Knight joint list has an invalid body-relative index");
        offsets[(*count)++] = offset;
    }
    return SDL_SetError("Giant Knight joint list has no bounded terminator");
}
bool GiantPlace(SotnActors *s, Actor *root, Actor *part, bool inverse)
{
    if (!s || !root || !part)
        return SDL_SetError("Giant Knight placement requires initialized owners and a joint");
    Actor *parent = NULL;
    SotnMathTrig trig = {0};
    if (!GiantPart(s, root, part->giant.parent, &parent) ||
        !SotnMath_Trig(
            s->executable,
            root->state.facing_left ? -(Sint32)part->giant.rotation : part->giant.rotation, &trig))
        return false;
    const Sint64 dx = (Sint64)part->giant.length * trig.sine * 16,
                 dy = (Sint64)part->giant.length * trig.cosine * 16;
    if (inverse)
    {
        parent->state.x = Word((Sint64)part->state.x + dx);
        parent->state.y = Word((Sint64)part->state.y - dy);
    }
    else
    {
        part->state.x = Word((Sint64)parent->state.x - dx);
        part->state.y = Word((Sint64)parent->state.y + dy);
    }
    part->giant.placed = true;
    return true;
}
bool GiantAim(Actor *part, Sint16 target, Sint16 duration)
{
    if (!part || duration <= 0 || duration > 1024)
        return SDL_SetError(
            "Giant Knight angle interpolation requires a bounded positive duration");
    Sint32 difference = Wrap16((Sint32)target - part->giant.rotation);
    if (difference > 2048)
        difference -= 4096;
    if (difference < -2048)
        difference += 4096;
    part->giant.rotation_velocity = (Sint16)(difference / duration);
    part->giant.target = target;
    return true;
}
bool GiantPose(SotnActors *s, Actor *root, size_t list, size_t poses, Uint16 channel)
{
    Uint16 offsets[20] = {0}, count = 0;
    SotnDataView record = {0};
    if (!s || !root || channel >= 2 || root->giant.pose[channel] >= 256 ||
        !List(s, list, offsets, &count) || poses > s->nz0.size ||
        root->giant.pose[channel] * 8U > s->nz0.size - poses ||
        !Slice(s->nz0, poses + root->giant.pose[channel] * 8U, 8, &record))
        return SDL_SetError("Giant Knight pose cursor exceeds its private records");
    const Sint16 duration = (Sint16)Signed16(record.bytes);
    if (duration <= 0 || duration > 1024)
        return SDL_SetError("Giant Knight active pose has an invalid duration");
    if (!root->giant.remaining[channel])
    {
        const Uint32 pointer = Read32(record.bytes + 4);
        SotnDataView angles = {0};
        if (pointer < 0x80180000 || !Slice(s->nz0, pointer - 0x80180000, count * 2U, &angles))
            return SDL_SetError("Giant Knight pose references an invalid private angle array");
        for (Uint16 i = 0; i < count; ++i)
        {
            Actor *part = NULL;
            if (offsets[i] == 255)
                continue;
            if (!GiantPart(s, root, offsets[i], &part))
                return false;
            if (!GiantAim(part, (Sint16)Signed16(angles.bytes + i * 2U), duration))
                return false;
        }
        root->giant.remaining[channel] = (Uint16)duration;
    }
    if (!--root->giant.remaining[channel])
    {
        SotnDataView next = {0};
        if (!Slice(s->nz0, poses + (root->giant.pose[channel] + 1U) * 8U, 8, &next))
            return false;
        if (!Read16(next.bytes))
            root->giant.pose[channel] = 0;
        else
            ++root->giant.pose[channel];
    }
    return true;
}
bool GiantAdvance(SotnActors *s, Actor *root, size_t list, Uint16 mode)
{
    Uint16 offsets[20] = {0}, count = 0;
    if (!s || !root || mode > 4 || !List(s, list, offsets, &count) || (mode >= 2 && count < 4))
        return SDL_SetError("Giant Knight anchor recipe requires a bounded joint list");
    for (Uint16 i = 0; i < count; ++i)
    {
        Actor *part = NULL;
        if (offsets[i] == 255)
            continue;
        if (!GiantPart(s, root, offsets[i], &part))
            return false;
        part->giant.rotation =
            (Sint16)Wrap16((Sint32)part->giant.rotation + part->giant.rotation_velocity);
        if (mode == 1 && !GiantPlace(s, root, part, false))
            return false;
    }
    if (mode < 2)
        return true;
    if (mode != 3)
    {
        for (Uint16 i = 0; i < 2; ++i)
        {
            Actor *part = NULL;
            if (!GiantPart(s, root, offsets[1U - i], &part) || !GiantPlace(s, root, part, true))
                return false;
        }
        if (mode == 4)
        {
            Actor *part = NULL;
            if (!GiantPart(s, root, 18, &part) || !GiantPlace(s, root, part, true))
                return false;
        }
    }
    for (Uint16 i = mode == 3 ? 0 : 2; i < count; ++i)
    {
        Actor *part = NULL;
        if (offsets[i] == 255)
            continue;
        if (!GiantPart(s, root, offsets[i], &part) || !GiantPlace(s, root, part, false))
            return false;
    }
    return true;
}
bool GiantParts(SotnActors *s, Actor *root, size_t list, bool finish)
{
    Uint16 offsets[20] = {0}, count = 0;
    if (!s || !root)
        return SDL_SetError("Giant Knight completion requires initialized owners");
    if (!List(s, list, offsets, &count))
        return false;
    for (Uint16 i = 0; i < count; ++i)
    {
        Actor *part = NULL;
        if (offsets[i] == 255 || !GiantPart(s, root, offsets[i], &part))
            return SDL_SetError("Giant Knight completion list contains an invalid required joint");
        if (finish)
        {
            part->state.facing_left = root->state.facing_left;
            part->giant.placed = false;
        }
        else if (!part->giant.placed && !GiantPlace(s, root, part, false))
            return false;
    }
    return true;
}
void GiantStep(Actor *root, Uint16 step)
{
    SetStep(root, step);
    GiantClearPose(root);
}
void GiantClearPose(Actor *root)
{
    SDL_zero(root->giant.pose);
    SDL_zero(root->giant.remaining);
}
bool GiantBend(SotnActors *s, Actor *root, Actor *part, Sint16 duration)
{
    if (!part)
        return SDL_SetError("Giant Knight interpolation requires an initialized joint");
    if (!part->giant.remaining[0])
    {
        if (!GiantAim(part, part->giant.target, duration))
            return false;
        part->giant.remaining[0] = (Uint16)duration;
    }
    --part->giant.remaining[0];
    part->giant.rotation =
        (Sint16)Wrap16((Sint32)part->giant.rotation + part->giant.rotation_velocity);
    return GiantPlace(s, root, part, false);
}
static bool Sqrt0(SotnActors *s, Sint64 value, Sint32 *out)
{
    Uint32 root = 0;
    if (value < 0 || value > SDL_MAX_SINT32)
        return SDL_SetError("Giant Knight inverse-kinematics input exceeds the positive SDK range");
    if (!SotnMath_Sqrt12(s->executable, (Uint32)value, &root))
        return false;
    *out = (Sint32)(root >> 6);
    return true;
}
bool GiantSolve(SotnActors *s, Actor *root, Actor *first, Actor *second)
{
    Actor *parent = NULL;
    Sint16 angle = 0, bend = 0;
    Sint32 distance = 0, altitude = 0;
    if (!first || !second || first->giant.length <= 0 || second->giant.length <= 0 ||
        !GiantPart(s, root, first->giant.parent, &parent))
        return SDL_SetError("Giant Knight inverse kinematics require positive owned links");
    Sint32 x = Word((Sint64)second->state.x - parent->state.x);
    if (root->state.facing_left)
        x = Word(-(Sint64)x);
    Sint32 y = Word((Sint64)second->state.y - parent->state.y);
    if (x == SDL_MIN_SINT32)
        return SDL_SetError("Giant Knight inverse direction cannot negate its minimum coordinate");
    if (!SotnMath_Atan2Vector(s->executable, -x, y, &angle))
        return false;
    const Sint32 l1 = first->giant.length * 256, l2 = second->giant.length * 256;
    x = ActorShift(x, 8);
    y = ActorShift(y, 8);
    if (!Sqrt0(s, (Sint64)x * x + (Sint64)y * y, &distance))
        return false;
    distance = (Sint32)SDL_min((Sint64)distance, (Sint64)(l1 + l2) * 256);
    const Sint64 projection = (Sint64)distance * l1 / (l1 + l2);
    if (!Sqrt0(s, (Sint64)l1 * l1 - projection * projection, &altitude) ||
        !SotnMath_Atan2(s->executable, altitude, (Sint32)projection, &bend))
        return false;
    first->giant.target = (Sint16)Wrap16((Sint32)angle + bend);
    SotnMathTrig trig = {0};
    if (!SotnMath_Trig(s->executable,
                       root->state.facing_left ? -(Sint32)first->giant.target : first->giant.target,
                       &trig))
        return false;
    const Sint32 joint_x =
                     Word((Sint64)parent->state.x - (Sint64)first->giant.length * trig.sine * 16),
                 joint_y =
                     Word((Sint64)parent->state.y + (Sint64)first->giant.length * trig.cosine * 16);
    x = Word((Sint64)second->state.x - joint_x);
    if (root->state.facing_left)
        x = Word(-(Sint64)x);
    y = Word((Sint64)second->state.y - joint_y);
    if (x == SDL_MIN_SINT32)
        return SDL_SetError("Giant Knight distal direction cannot negate its minimum coordinate");
    if (!SotnMath_Atan2Vector(s->executable, -x, y, &angle))
        return false;
    second->giant.target = angle;
    return true;
}
bool GiantGround(SotnActors *s, const Actor *root, Actor *part, SotnCollider *c)
{
    if (!s || !root || !part || !c)
        return SDL_SetError("Giant Knight ground query requires initialized owners and a collider");
    if (!ActorQuery(s, Pixel(part->state.x), Wrap16((Sint64)Pixel(part->state.y) + c->top), c))
        return false;
    part->giant.terrain = 0;
    if (c->effects & SOTN_EFFECT_SOLID)
    {
        part->giant.terrain = 1;
        if (c->effects & SOTN_EFFECT_SLOPE_FLOOR)
            part->giant.terrain = (c->effects & SOTN_EFFECT_SLOPE_LEFT)
                                      ? (root->state.facing_left ? 4 : 2)
                                      : (root->state.facing_left ? 2 : 4);
    }
    return true;
}
bool GiantAhead(SotnActors *s, const Actor *part, bool facing, Uint16 *out)
{
    if (!s || !part || !out)
        return SDL_SetError("Giant Knight forward query requires a joint and result");
    SotnCollider c = {0};
    const Sint32 x = Pixel(part->state.x) + (facing ? 64 : -64), y = Pixel(part->state.y) + 9;
    if (!ActorQuery(s, x, y - 6, &c))
        return false;
    Uint16 result = c.effects & SOTN_EFFECT_SOLID ? 2 : 0;
    if (!ActorQuery(s, x, y + 6, &c))
        return false;
    if (!(c.effects & SOTN_EFFECT_SOLID))
        result |= 4;
    *out = result;
    return true;
}
bool GiantToward(Sint16 *value, Sint16 target, Sint16 step)
{
    if (!value || step <= 0)
        return SDL_SetError("Giant Knight angle approach requires a positive increment");
    if (SDL_abs((Sint32)*value - target) < step)
    {
        *value = target;
        return true;
    }
    if (*value > target)
        *value = (Sint16)Wrap16((Sint32)*value - step);
    if (*value < target)
        *value = (Sint16)Wrap16((Sint32)*value + step);
    return false;
}
