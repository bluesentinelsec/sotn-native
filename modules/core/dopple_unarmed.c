// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted from sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5,
// src/boss/bo4/unk_50D4C.c:DopplegangerUnarmedAttack and boss/dop_anim.h:UpdateUnarmedAnim.
#include "actors_internal.h"

static bool Resolve(SotnDataView image, Uint32 address, size_t size, Uint32 alignment,
                    SotnDataView *out)
{
    if (address < 0x80180000 || address % alignment != 0)
        return SDL_SetError("BO4 hand attack references an invalid private address");
    return Slice(image, address - 0x80180000, size, out);
}
static Sint16 Signed8(Uint8 value)
{
    return value < 128 ? value : (Sint16)((Sint32)value - 256);
}
bool ActorDoppleUnarmed(SotnActors *s, Actor *a)
{
    if (s == NULL || a == NULL || s->stage_id != 26 || a->state.id != 0x137)
        return SDL_SetError("BO4 unarmed child dispatch requires its original actor type");
    if (a->enemy_state.owner_slot >= SOTN_STAGE_ACTOR_SLOTS ||
        a->enemy_state.owner_generation == 0 ||
        s->slots[a->enemy_state.owner_slot].state.generation != a->enemy_state.owner_generation)
    {
        a->remove = true;
        return true;
    }
    const Uint16 index = (a->state.parameters & 0x7fffU) >> 8;
    SotnDataView profile = {0}, frames = {0}, words = {0};
    if (index >= 2 || !Slice(s->nz0, 0x4278 + index * 12U, 12, &profile))
        return SDL_SetError("BO4 hand attack index exceeds its private profiles");
    const SotnDoppleBody *body = &s->dopple.body;
    a->state.x = body->motion.x;
    a->state.y = body->motion.y;
    a->state.facing_left = body->facing_left;
    const Uint16 base = profile.bytes[10];
    if (body->animation.id < base || body->animation.id >= base + 7U || !body->attack_state)
    {
        a->remove = true;
        return true;
    }
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, index ? 0x44c : 0x440))
            return false;
        if (body->motion.status & 0x100U)
            a->attack /= 2;
        a->state.priority =
            s->slots[0].state.priority >= 2 ? (Uint16)(s->slots[0].state.priority - 2) : 0;
        a->state.blend_mode = 0x30;
        a->state.flags = 0x18000000U;
        a->state.step = 1;
    }
    const Uint16 variant = (Uint16)(body->animation.id - base);
    if (!Resolve(s->nz0, Read32(profile.bytes), 7 * 4, 4, &frames))
        return false;
    const Uint32 address = Read32(frames.bytes + variant * 4U);
    if (body->animation.pose >= 128 ||
        !Resolve(s->nz0, address, ((size_t)body->animation.pose + 1) * 2, 2, &words))
        return SDL_SetError("BO4 hand attack pose is truncated or out of range");
    for (Uint16 pose = 0; pose <= body->animation.pose; ++pose)
        if (Read16(words.bytes + pose * 2U) == 0xffff)
        {
            a->remove = true;
            return true;
        }
    if (body->animation.remaining == 1 && body->animation.pose == profile.bytes[11] &&
        !ActorSound(s, Read16(profile.bytes + 8)))
        return false;
    const Uint16 packed = Read16(words.bytes + body->animation.pose * 2U);
    const Uint16 property = (packed >> 9) & 127U;
    SotnDataView properties = {0};
    const Uint16 count = index ? 15 : 19;
    if (property >= count ||
        !Resolve(s->nz0, Read32(profile.bytes + 4), count * 4U, 2, &properties))
        return SDL_SetError("BO4 hand attack hitbox exceeds its referenced property records");
    const Uint8 *box = properties.bytes + property * 4U;
    a->state.hitbox_x = Signed8(box[0]);
    a->state.hitbox_y = Signed8(box[1]);
    a->state.hitbox_width = box[2];
    a->state.hitbox_height = box[3];
    a->state.frame = packed & 511U;
    if (body->animation.remaining < 0)
        a->remove = true;
    return true;
}
