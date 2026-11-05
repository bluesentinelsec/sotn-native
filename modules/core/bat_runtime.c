// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/7879C.c factories/BlinkWhite/WingSmashTrail/func_8011B480/func_80118894,
// 84B88.c:EntityBatFireball/func_80129864/EntitySummonSpirit, 8BEF8.c:EntityBatEcho,
// alu_anim.c:UpdateAnim, 7879C.c:UpdatePlayerEntities/func_80118970 and
// 5D5BC.c:GetSpellDef and 692E8.c:AluInit common primitive lease, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Original blueprint/profile/spell/artwork data remains
// private.
#include "bat_runtime_internal.h"
#include <sotn/combat.h>

static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint32 FixedShift(Sint32 value, Sint16 pixels)
{
    const Uint32 bits = (Uint32)value + (Uint32)(Sint32)pixels * 65536U;
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint32 Pixel(Sint32 value)
{
    return value >= 0 ? value / 65536 : (Sint32)(-((-(Sint64)value + 65535) / 65536));
}
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static bool RetainRows(SotnPrimitiveSlots *slots, SotnPrimitiveSlots *retained,
                       const Uint16 *indices, Uint16 count)
{
    for (Uint16 i = 0; i < count; ++i)
    {
        const Uint16 id = indices[i];
        if (id >= SOTN_PRIMITIVE_SLOTS ||
            (retained->occupied[id / 64U] & ((Uint64)1 << (id % 64U))))
            return SDL_SetError("Bat effects have duplicate or invalid actual primitive rows");
        retained->occupied[id / 64U] |= (Uint64)1 << (id % 64U);
        slots->occupied[id / 64U] |= (Uint64)1 << (id % 64U);
    }
    return true;
}
Uint64 SotnBatRuntime_Occupied(const SotnBatRuntime *r)
{
    Uint64 mask = 0;
    if (!r)
        return 0;
    for (Uint16 i = 0; i < 8; ++i)
        if (r->factories[i].cursor.entity_id)
            mask |= (Uint64)1 << (8U + i);
    for (Uint16 i = 0; i < 48; ++i)
        if (r->children[i].kind)
            mask |= (Uint64)1 << (16U + i);
    return mask | SotnWolfRuntime_Occupied(&r->wolf);
}
bool SotnBatRuntime_Available(const SotnBatRuntime *r)
{
    if (!r)
        return false;
    for (Uint16 i = 0; i < 8; ++i)
        if (!r->factories[i].cursor.entity_id)
            return true;
    return false;
}
bool SotnBatRuntime_Context(const SotnBatRuntime *r, Uint32 generation,
                            const SotnPlayerBlink **effect, const SotnPlayerFactory **pending)
{
    if (!r || !effect || !pending)
        return SDL_SetError("Bat runtime context requires its owner and outputs");
    const SotnPlayerBlink *found = NULL;
    const SotnPlayerFactory *waiting = NULL;
    if (generation)
    {
        for (Uint16 i = 0; i < 48; ++i)
            if (r->children[i].kind == 11 && r->children[i].bat_generation == generation)
            {
                if (found)
                    return SDL_SetError("Bat runtime has duplicate transformation child ownership");
                found = &r->children[i].state.blink;
            }
        for (Uint16 i = 0; i < 8; ++i)
            if (r->factories[i].cursor.entity_id && r->factories[i].bat_generation == generation)
            {
                if (waiting || found)
                    return SDL_SetError(
                        "Bat runtime has duplicate pending transformation ownership");
                waiting = &r->factories[i].cursor;
            }
    }
    *effect = found;
    *pending = waiting;
    return true;
}
bool SotnBatRuntime_ReplaceWolfSlots(SotnBatRuntime *r)
{
    if (!r)
        return SDL_SetError("Wolf forced replacement requires the actual selected slot owner");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if ((r->children[i].kind == 36 || r->children[i].kind == 45 || r->children[i].kind == 52 ||
             r->children[i].kind == 53 || r->children[i].kind == 26 || r->children[i].kind == 27 ||
             r->children[i].kind == 61 || r->children[i].neutron || r->children[i].sire ||
             r->children[i].pentagram || r->children[i].magic || r->children[i].thrown ||
             r->children[i].star || r->children[i].karma) &&
            r->children[i].group >= SDL_arraysize(r->groups))
            return SDL_SetError("Wolf replacement found an invalid actual shared attack group");
    for (Uint16 i = 1; i <= 14; ++i)
    {
        r->children[i] = (SotnBatRuntimeChild){0};
        r->spirit_mask &= ~((Uint64)1 << i);
        r->hellfire_mask &= ~((Uint64)1 << i);
        r->sword_mask &= ~((Uint64)1 << i);
        r->neutron_mask &= ~((Uint64)1 << i);
        r->sire_mask &= ~((Uint64)1 << i);
        r->pentagram_mask &= ~((Uint64)1 << i);
        r->magic_mask &= ~((Uint64)1 << i);
        r->thrown_mask &= ~((Uint64)1 << i);
        r->star_mask &= ~((Uint64)1 << i);
        r->karma_mask &= ~((Uint64)1 << i);
        r->secondary_mask &= ~((Uint64)1 << i);
    }
    r->wing_mask &= 0x4000U;
    SDL_zero(r->groups);
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if ((r->children[i].kind == 36 || r->children[i].kind == 45 || r->children[i].kind == 52 ||
             r->children[i].kind == 53 || r->children[i].kind == 26 || r->children[i].kind == 27 ||
             r->children[i].kind == 61 || r->children[i].neutron || r->children[i].sire ||
             r->children[i].pentagram || r->children[i].magic || r->children[i].thrown ||
             r->children[i].star || r->children[i].karma) &&
            r->children[i].group && r->children[i].group < SDL_arraysize(r->groups))
            ++r->groups[r->children[i].group];
    return true;
}
static bool RequestSource(SotnBatRuntime *r, SotnPlayerFactorySource source, Uint32 packed,
                          Uint32 generation)
{
    for (Uint16 i = 0; i < 8; ++i)
        if (!r->factories[i].cursor.entity_id)
        {
            SotnPlayerFactory cursor = {0};
            if (!SotnPlayerFactory_Begin(&cursor, source, packed, 0))
                return false;
            SDL_zero(r->factories[i]);
            r->factories[i].cursor = cursor;
            r->factories[i].bat_generation = generation;
            r->factories[i].sword_parent = 0;
            return true;
        }
    // Original CreateEntFactoryFromEntity returns NULL; optional commands never invent a child.
    if (generation)
        return SDL_SetError("Bat transformation lost its reserved actual factory slot");
    return true;
}
bool SotnBatRuntime_RequestHellfire(SotnBatRuntime *r, SotnPlayerFactorySource source,
                                    Uint32 packed, bool *allocated)
{
    if (!r || !allocated ||
        (packed != 33 && packed != 37 && packed != 0x17002c && packed != 0x16002c))
        return SDL_SetError("Hellfire requires its actual original player factory boundary");
    for (Uint16 i = 0; i < 8; ++i)
        if (!r->factories[i].cursor.entity_id)
        {
            SotnPlayerFactory f = {0};
            if (!SotnPlayerFactory_Begin(&f, source, packed, 0))
                return false;
            SDL_zero(r->factories[i]);
            r->factories[i].cursor = f;
            r->factories[i].bat_generation = 0;
            r->factories[i].sword_parent = 0;
            *allocated = true;
            return true;
        }
    *allocated = false;
    return true;
}
bool SotnBatRuntime_RequestTeleport(SotnBatRuntime *r, SotnPlayerFactorySource source,
                                    Uint32 packed, bool *allocated)
{
    if (!r || !allocated || (packed & 65535U) != 121 || (packed >> 16) > 5)
        return SDL_SetError("Teleport requires actual factory121/high0..5");
    for (Uint16 i = 0; i < SDL_arraysize(r->factories); ++i)
        if (!r->factories[i].cursor.entity_id)
        {
            SotnPlayerFactory f = {0};
            if (!SotnPlayerFactory_Begin(&f, source, packed, 0))
                return false;
            SDL_zero(r->factories[i]);
            r->factories[i].cursor = f;
            *allocated = true;
            return true;
        }
    *allocated = false;
    return true;
}
bool SotnBatRuntime_MedicinePresent(const SotnBatRuntime *r)
{
    if (!r)
        return false;
    for (Uint16 i = 16; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (r->children[i].kind == 39)
            return true;
    return false;
}
bool SotnBatRuntime_RequestMedicine(SotnBatRuntime *r, SotnPlayerFactorySource source,
                                    Uint32 packed, Uint16 item, bool *allocated)
{
    const Uint32 profile = (packed >> 16) & 127U;
    const Uint32 expected = item >= 157 ? item - 143U : item >= 144 ? item - 144U : 0;
    if (!r || !allocated || item < 144 || item > 162 || item == 146 || item == 147 ||
        (packed & 65535U) != 55 || (packed >> 24) || profile != expected)
        return SDL_SetError(
            "Medicine reservation requires actual55, hand/profile and item144..162");
    for (Uint16 i = 0; i < SDL_arraysize(r->factories); ++i)
        if (!r->factories[i].cursor.entity_id)
        {
            SotnPlayerFactory f = {0};
            if (!SotnPlayerFactory_Begin(&f, source, packed, (Sint16)item))
                return false;
            SDL_zero(r->factories[i]);
            r->factories[i].cursor = f;
            *allocated = true;
            return true;
        }
    *allocated = false;
    return true;
}
static bool Request(SotnBatRuntime *r, SotnPlayerState p, Uint32 packed, Uint32 generation)
{
    return RequestSource(
        r,
        (SotnPlayerFactorySource){
            .x = p.x, .y = p.y, .priority = p.priority, .facing_left = p.facing_left},
        packed, generation);
}
bool SotnBatRuntime_RequestFood(SotnBatRuntime *r, SotnPlayerFactorySource source, Uint32 packed,
                                Uint16 item, bool *allocated)
{
    const Uint32 hand = (packed >> 12) & 15U;
    const bool curse = packed == 0x10039 && !item;
    if (!r || !allocated ||
        (!curse &&
         (item < 29 || item > 70 || (hand != 1 && hand != 2) ||
          packed != (50U | hand << 12 | (Uint32)(item - 29U + (hand == 2 ? 128U : 0U)) << 16))))
        return SDL_SetError("Food reservation requires actual50, item29..70 and its owned hand");
    for (Uint16 i = 0; i < SDL_arraysize(r->factories); ++i)
        if (!r->factories[i].cursor.entity_id)
        {
            SotnPlayerFactory f = {0};
            if (!SotnPlayerFactory_Begin(&f, source, packed, (Sint16)item))
                return false;
            SDL_zero(r->factories[i]);
            r->factories[i].cursor = f;
            *allocated = true;
            return true;
        }
    *allocated = false;
    return true;
}
bool SotnBatRuntime_RequestThrowing(SotnBatRuntime *r, SotnPlayerFactorySource source,
                                    Uint32 packed, Uint16 item, bool *allocated)
{
    if (packed == 0x10039 && !item)
        return SotnBatRuntime_RequestFood(r, source, packed, item, allocated);
    const Uint32 hand = (packed >> 12) & 15U;
    const Uint32 parameter = (item == 74 ? 1U : 0U) + (hand == 2 ? 128U : 0U);
    if (!r || !allocated || item < 71 || item > 74 || (hand != 1 && hand != 2) ||
        packed != (50U | hand << 12 | parameter << 16))
        return SDL_SetError("Throwing reservation requires actual50/owned item71..74/hand");
    for (Uint16 i = 0; i < SDL_arraysize(r->factories); ++i)
        if (!r->factories[i].cursor.entity_id)
        {
            SotnPlayerFactory f = {0};
            if (!SotnPlayerFactory_Begin(&f, source, packed, (Sint16)item))
                return false;
            SDL_zero(r->factories[i]);
            r->factories[i].cursor = f;
            *allocated = true;
            return true;
        }
    *allocated = false;
    return true;
}
bool SotnBatRuntime_RequestNeutron(SotnBatRuntime *r, SotnPlayerFactorySource source, Uint32 packed,
                                   Uint16 item, bool *allocated)
{
    if (item != 71 && !(packed == 0x10039 && !item))
        return SDL_SetError("Neutron reservation lost its actual item71");
    return SotnBatRuntime_RequestThrowing(r, source, packed, item, allocated);
}
bool SotnBatRuntime_RequestMagicMissile(SotnBatRuntime *r, SotnPlayerFactorySource source,
                                        Uint32 packed, bool *allocated)
{
    if (!r || !allocated || (packed != 0x1032 && packed != 0x802032))
        return SDL_SetError("Magic Missile requires its actual50/hand/item25 reservation");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        if (!r->factories[i].cursor.entity_id)
        {
            SotnPlayerFactory f = {0};
            if (!SotnPlayerFactory_Begin(&f, source, packed, 25))
                return false;
            SDL_zero(r->factories[i]);
            r->factories[i].cursor = f;
            *allocated = true;
            return true;
        }
    *allocated = false;
    return true;
}
static bool MagicFactory(SotnBatRuntime *r, SotnPlayerFactorySource source, Uint16 slot,
                         Uint16 hand, bool *allocated)
{
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        if (!r->factories[i].cursor.entity_id)
        {
            source.flags = 0;
            SotnPlayerFactory f = {0};
            if (!SotnPlayerFactory_Begin(&f, source, 56U | ((Uint32)hand + 1U) << 12, 0))
                return false;
            SDL_zero(r->factories[i]);
            r->factories[i].cursor = f;
            r->factories[i].magic_factory = true;
            r->factories[i].magic_parent_slot = slot;
            *allocated = true;
            return true;
        }
    *allocated = false;
    return true;
}
static bool MagicParent(const SotnBatRuntime *r, Uint16 slot, SotnMagicMissileParent *out)
{
    if (slot < 17 || slot > 31)
        return SDL_SetError("Magic Missile lost its actual raw parent slot");
    if (slot <= 30 && r->wolf.children[slot - 17U].identity)
    {
        const SotnWolfRuntimeChild *c = &r->wolf.children[slot - 17U];
        const SotnWolfPart *p = &r->wolf.body.parts[slot - 17U];
        if (!r->wolf.generation || !c->generation || c->lineage != r->wolf.generation ||
            r->children[slot - 16U].kind)
            return SDL_SetError("Magic Missile raw Wolf parent lost its actual slot ownership");
        // Wolf's cleared births and body callbacks leave the weapon equipId word untouched.
        *out = (SotnMagicMissileParent){.exists = true,
                                        .facing_left = p->facing_left,
                                        .palette = p->palette,
                                        .bank = p->bank,
                                        .texture = p->texture_offset,
                                        .item = 0};
        return true;
    }
    const SotnBatRuntimeChild *c = &r->children[slot - 16U];
    if (!c->kind)
    {
        *out = (SotnMagicMissileParent){0};
        return true;
    }
    if (c->karma)
    {
        const SotnKarmaCoin *n = &c->state.karma.motion;
        *out = (SotnMagicMissileParent){.exists = true,
                                        .facing_left = n->facing_left,
                                        .palette = n->palette,
                                        .bank = n->bank,
                                        .texture = n->texture,
                                        .item = c->state.karma.item};
        return true;
    }
    if (c->star)
    {
        const SotnStarWeapon *n = &c->state.star.motion;
        const SotnThrownWeapon *m = &n->motion;
        *out = (SotnMagicMissileParent){.exists = true,
                                        .facing_left = m->facing_left,
                                        .palette = m->palette,
                                        .bank = m->bank,
                                        .texture = m->texture,
                                        .item = n->item};
        return true;
    }
    if (c->thrown)
    {
        const SotnThrownWeapon *m = &c->state.thrown.motion;
        *out = (SotnMagicMissileParent){.exists = true,
                                        .facing_left = m->facing_left,
                                        .palette = m->palette,
                                        .bank = m->bank,
                                        .texture = m->texture,
                                        .item = c->state.thrown.item};
        return true;
    }
    if (!c->magic)
        return SDL_SetError("Magic Missile raw parent requires its actual selected field owner");
    const SotnMagicMissile *m = &c->state.magic.motion;
    *out = (SotnMagicMissileParent){.exists = true,
                                    .facing_left = m->facing_left,
                                    .palette = m->palette,
                                    .bank = m->bank,
                                    .texture = m->texture,
                                    .item = m->item};
    return true;
}
static bool ThrownItem(Uint16 item)
{
    return item == 80 || item == 81 || item == 82 || item == 134 || item == 135;
}
bool SotnBatRuntime_RequestThrown(SotnBatRuntime *r, SotnPlayerFactorySource source, Uint32 packed,
                                  Uint16 item, bool *allocated)
{
    const Uint32 hand = (packed >> 12) & 15U;
    const Uint32 blueprint = item == 81 || item == 134    ? 62U
                             : item == 82                 ? 68U
                             : item == 135                ? 63U
                             : item == 133 || item == 176 ? 56U
                             : item == 164                ? 52U
                             : item == 165                ? 68U
                                                          : 50U;
    const Uint32 parameter = (item == 134 || item == 163 ? 1U
                              : item == 164              ? 2U
                                                         : 0U) +
                             (hand == 2 ? 128U : 0U);
    // Runesword143 (weapon14), Heaven sword119 (weapon13), Chakram133 (blueprint56), Vorpal
    // blade163 and Crissaegrim164 (weapon12, blueprints50/52) are thrown but never consumed.
    if (!r || !allocated ||
        (!ThrownItem(item) && item != 143 && item != 119 && item != 133 && item != 163 &&
         item != 164 && item != 165 && item != 176) ||
        (hand != 1 && hand != 2) || packed != (blueprint | hand << 12 | parameter << 16))
        return SDL_SetError(
            "Weapon16/Runesword reservation requires its actual owned item/blueprint/hand");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        if (!r->factories[i].cursor.entity_id)
        {
            SotnPlayerFactory f = {0};
            if (!SotnPlayerFactory_Begin(&f, source, packed, (Sint16)item))
                return false;
            SDL_zero(r->factories[i]);
            r->factories[i].cursor = f;
            *allocated = true;
            return true;
        }
    *allocated = false;
    return true;
}
static bool ThrownFactory(SotnBatRuntime *r, SotnThrownFactory request, Uint16 slot, Uint16 hand)
{
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        if (!r->factories[i].cursor.entity_id)
        {
            SotnPlayerFactory f = {0};
            if (!SotnPlayerFactory_Begin(&f, request.source, request.packed, 0))
                return false;
            if (request.weapon_dispatch)
                f.entity_id = hand ? 0xff : 0xef;
            SDL_zero(r->factories[i]);
            r->factories[i].cursor = f;
            r->factories[i].thrown_factory = f.blueprint == 58 || f.blueprint == 110;
            r->factories[i].thrown_particles = request.weapon_dispatch;
            if (r->factories[i].thrown_factory || r->factories[i].thrown_particles)
                r->factories[i].thrown_parent_slot = slot;
            return true;
        }
    return true; // Original optional descendants do not replace an occupied root.
}
static bool ThrownParent(const SotnBatRuntime *r, Uint16 slot, SotnThrownParent *out, Sint32 *vx)
{
    if (slot < 17 || slot > 31)
        return SDL_SetError("Weapon16 lost its actual raw parent slot");
    *out = (SotnThrownParent){0};
    *vx = 0;
    if (slot <= 30 && r->wolf.children[slot - 17U].identity)
    {
        const SotnWolfRuntimeChild *owner = &r->wolf.children[slot - 17U];
        const SotnWolfPart *p = &r->wolf.body.parts[slot - 17U];
        if (!r->wolf.generation || !owner->generation || owner->lineage != r->wolf.generation ||
            r->children[slot - 16U].kind)
            return SDL_SetError("Weapon16 raw Wolf parent lost its actual slot ownership");
        *out = (SotnThrownParent){.bank = p->bank,
                                  .frame = p->sprite,
                                  .texture = p->texture_offset,
                                  .palette = p->palette,
                                  .rotation = p->rotation,
                                  .facing_left = p->facing_left};
        // These cleared body entities never write their own velocityX; PLAYER is separate.
        return true;
    }
    const SotnBatRuntimeChild *c = &r->children[slot - 16U];
    if (!c->kind)
        return true;
    if (c->karma)
    {
        const SotnKarmaCoin *n = &c->state.karma.motion;
        *out = (SotnThrownParent){.bank = n->bank,
                                  .frame = n->frame,
                                  .palette = n->palette,
                                  .texture = n->texture,
                                  .facing_left = n->facing_left};
        *vx = n->velocity_x;
        return true;
    }
    if (c->star)
    {
        const SotnThrownWeapon *s = &c->state.star.motion.motion;
        *out = (SotnThrownParent){.bank = s->bank,
                                  .frame = s->frame,
                                  .texture = s->texture,
                                  .palette = s->palette,
                                  .rotation = s->rotation,
                                  .facing_left = s->facing_left};
        *vx = s->velocity_x;
        return true;
    }
    if (c->thrown)
    {
        const SotnThrownWeapon *s = &c->state.thrown.motion;
        *out = (SotnThrownParent){.bank = s->bank,
                                  .frame = s->frame,
                                  .texture = s->texture,
                                  .palette = s->palette,
                                  .rotation = s->rotation,
                                  .facing_left = s->facing_left};
        *vx = s->velocity_x;
        return true;
    }
    if (c->magic)
    {
        const SotnMagicMissile *s = &c->state.magic.motion;
        *out = (SotnThrownParent){.bank = s->bank,
                                  .frame = c->state.magic.animation.frame,
                                  .texture = s->texture,
                                  .palette = s->palette,
                                  .facing_left = s->facing_left};
        *vx = s->velocity_x;
        return true;
    }
    return SDL_SetError("Weapon16 raw parent requires its actual selected field owner");
}
bool SotnBatRuntime_RequestKarma(SotnBatRuntime *r, SotnPlayerFactorySource source, Uint32 packed,
                                 Uint16 item, bool *allocated)
{
    if (!r || !allocated || item != 24 || (packed != 0x1032 && packed != 0x802032U))
        return SDL_SetError("Karma Coin reservation requires actual item24/root50/hand");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        if (!r->factories[i].cursor.entity_id)
        {
            SotnPlayerFactory f = {0};
            if (!SotnPlayerFactory_Begin(&f, source, packed, 24))
                return false;
            SDL_zero(r->factories[i]);
            r->factories[i].cursor = f;
            *allocated = true;
            return true;
        }
    *allocated = false;
    return true;
}
static bool KarmaFactory(SotnBatRuntime *r, SotnPlayerFactorySource source, Uint32 packed,
                         Uint16 parent)
{
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        if (!r->factories[i].cursor.entity_id)
        {
            SotnPlayerFactory f = {0};
            if (!SotnPlayerFactory_Begin(&f, source, packed, 0))
                return false;
            SDL_zero(r->factories[i]);
            r->factories[i].cursor = f;
            r->factories[i].karma_factory = true;
            r->factories[i].karma_parent_slot = parent;
            return true;
        }
    return true;
}
bool SotnBatRuntime_KarmaValid(const SotnBatRuntimeChild *c, Uint16 index)
{
    if (!c->karma)
        return true;
    const bool child = (c->kind & 15U) == 2;
    const SotnKarmaCoin *n = &c->state.karma.motion;
    if ((c->kind != 0xe0 && c->kind != 0xf0 && c->kind != 0xe2 && c->kind != 0xf2) || c->food ||
        c->neutron || c->sire || c->pentagram || c->magic || c->thrown || c->star ||
        c->thrown_particle_parent || !c->generation || c->bat_generation || c->draw_count ||
        n->removed || n->priority > 511 || n->pose >= 33 || n->remaining < -1 ||
        n->phase > (child ? 1 : 9) ||
        (child
             ? index < 16 || index > 31 || c->state.karma.item || c->parameter > 3 ||
                   c->state.karma.parent_slot < 17 || c->state.karma.parent_slot > 31
             : index < 1 || index > 15 || c->state.karma.item != 24 || c->state.karma.parent_slot ||
                   c->parameter != (c->kind == 0xf0 ? 0x8000 : 0)) ||
        c->primitive_count != (c->initialized && !child ? SOTN_KARMA_ROWS : 0))
        return false;
    if (!c->initialized)
        return !n->phase && !c->group && !c->state.karma.attack.active;
    if (!n->phase)
        return false;
    if (child || n->phase < 4)
        return !c->group && !c->state.karma.attack.active;
    return c->group >= 3 && c->group <= 6 && c->state.karma.attack.equipment == 24 &&
           c->state.karma.attack.hit_type == (n->phase == 4 ? 2 : 0x82) &&
           c->state.karma.attack.offset_x == n->hitbox_x &&
           c->state.karma.attack.offset_y == n->hitbox_y &&
           c->state.karma.attack.width == n->hitbox_width &&
           c->state.karma.attack.height == n->hitbox_height;
}
bool SotnBatRuntime_PruneKarma(SotnBatRuntime *r)
{
    if (!r)
        return SDL_InvalidParamError("runtime");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (!SotnBatRuntime_KarmaValid(&r->children[i], i))
            return SDL_SetError("Karma cleanup lost actual child/row/raw-parent ownership");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (r->children[i].karma)
            r->children[i] = (SotnBatRuntimeChild){0};
    r->karma_mask = 0;
    return true;
}

static bool StarItem(Uint16 item)
{
    return item >= 75 && item <= 79;
}
bool SotnBatRuntime_RequestStar(SotnBatRuntime *r, SotnPlayerFactorySource source, Uint32 packed,
                                Uint16 item, bool *allocated)
{
    const Uint32 hand = (packed >> 12) & 15U, blueprint = item == 79 ? 68U : 50U;
    const Uint32 parameter = (item == 79 ? 0U : (Uint32)item - 75U) + (hand == 2 ? 128U : 0U);
    if (!r || !allocated || !StarItem(item) || (hand != 1 && hand != 2) ||
        packed != (blueprint | hand << 12 | parameter << 16))
        return SDL_SetError("Weapon15 reservation requires its actual item/hand/blueprint");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        if (!r->factories[i].cursor.entity_id)
        {
            SotnPlayerFactory f = {0};
            if (!SotnPlayerFactory_Begin(&f, source, packed, (Sint16)item))
                return false;
            SDL_zero(r->factories[i]);
            r->factories[i].cursor = f;
            *allocated = true;
            return true;
        }
    *allocated = false;
    return true;
}
static bool StarFactory(SotnBatRuntime *r, SotnThrownFactory request, Uint16 slot, Uint16 hand)
{
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        if (!r->factories[i].cursor.entity_id)
        {
            SotnPlayerFactory f = {0};
            if (!SotnPlayerFactory_Begin(&f, request.source, request.packed, 0))
                return false;
            if (request.weapon_dispatch)
                f.entity_id = hand ? 0xff : 0xef;
            SDL_zero(r->factories[i]);
            r->factories[i].cursor = f;
            if (request.weapon_dispatch)
            {
                r->factories[i].thrown_particles = true;
                r->factories[i].thrown_parent_slot = slot;
            }
            else
            {
                r->factories[i].star_factory = true;
                r->factories[i].star_parent_slot = slot;
            }
            return true;
        }
    return true;
}
static bool StarParent(const SotnBatRuntime *r, Uint16 slot, SotnStarParent *out)
{
    SotnThrownParent image = {0};
    Sint32 vx = 0;
    if (!ThrownParent(r, slot, &image, &vx))
        return false;
    *out = (SotnStarParent){.image = image, .velocity_x = vx};
    if (slot <= 30 && r->wolf.children[slot - 17U].identity)
    {
        const SotnWolfPart *p = &r->wolf.body.parts[slot - 17U];
        out->exists = true;
        out->priority = p->priority;
        out->group = (Uint8)p->enemy_id;
        return true;
    }
    const SotnBatRuntimeChild *c = &r->children[slot - 16U];
    if (!c->kind)
        return true;
    out->exists = true;
    out->priority = c->source.priority;
    out->group = c->group;
    if (c->karma)
    {
        out->item = c->state.karma.item;
        out->velocity_y = c->state.karma.motion.velocity_y;
    }
    else if (c->star)
    {
        out->item = c->state.star.motion.item;
        out->velocity_y = c->state.star.motion.motion.velocity_y;
    }
    else if (c->thrown)
    {
        out->item = c->state.thrown.item;
        out->velocity_y = c->state.thrown.motion.velocity_y;
    }
    else if (c->magic)
    {
        out->item = c->state.magic.motion.item;
        // Magic Missile entities retain their actual cleared velocityY; neither callback writes it.
        out->velocity_y = 0;
    }
    return true;
}
bool SotnBatRuntime_StarValid(const SotnBatRuntimeChild *c, Uint16 index)
{
    if (!c->star)
        return true;
    const Uint16 role = c->kind & 15U;
    const SotnStarWeapon *n = &c->state.star.motion;
    const SotnThrownWeapon *s = &n->motion;
    const bool root = role == 0 || role == 3, attack = root || role == 4 || role == 5;
    if (!((c->kind >= 0xe0 && c->kind <= 0xe5) || (c->kind >= 0xf0 && c->kind <= 0xf5)) ||
        c->food || c->neutron || c->sire || c->pentagram || c->magic || c->thrown || c->karma ||
        c->thrown_particle_parent || !c->generation || c->bat_generation || c->draw_count ||
        s->removed || s->parameter != c->parameter ||
        s->phase > (role == 3   ? 3
                    : role == 0 ? 2
                                : 1))
        return false;
    if (role == 1   ? index < 16 || index > 47
        : role == 2 ? index < 9 || index > 15
        : role == 4 ? index < 1 || index > 47
        : role == 5 ? index < 1 || index > 31
                    : index < 1 || index > 15)
        return false;
    if (root)
    {
        if (!StarItem(n->item) || c->state.star.parent_slot || role != (n->item == 79 ? 3 : 0) ||
            c->parameter !=
                ((c->kind >= 0xf0 ? 0x8000U : 0U) | (n->item == 79 ? 0U : (n->item - 75U) << 8)))
            return false;
    }
    else if (c->state.star.parent_slot < 17 || c->state.star.parent_slot > 31 ||
             ((role == 1 || role == 2 || role == 5) && c->parameter) ||
             (role == 4 && (c->parameter >> 8) > 23))
        return false;
    if (c->primitive_count != (c->initialized && role == 3 && (s->flags & 0x800000U) ? 32 : 0))
        return false;
    if (!c->initialized)
        return !s->phase && !c->group && !c->state.star.attack.active &&
               (!attack || root || !n->item);
    // A real emptied raw slot supplies priority0; the original rowless trail keeps
    // its wrapped -2 word with no bank/frame until its own lifetime retirement.
    const bool empty_trail = role == 1 && !s->bank && !s->frame && s->priority == 65534U;
    if (!s->phase || (s->priority > 511 && !empty_trail))
        return false;
    const Uint16 timeline = role == 2 ? 0x6c0 : role == 4 || role == 5 ? 0x770 : 0;
    if (c->state.star.animation_offset != timeline || c->state.star.animation.pose >= 15 ||
        c->state.star.animation.remaining < -1)
        return false;
    if (!attack)
        return !c->group && !c->state.star.attack.active;
    if (root ? c->group < 3 || c->group > 6 : c->group != 0 && (c->group < 3 || c->group > 10))
        return false;
    return c->state.star.attack.equipment == n->item && c->state.star.attack.hit_type == 2 &&
           c->state.star.attack.width == s->width && c->state.star.attack.height == s->height &&
           (!n->attack_cleared || !c->state.star.attack.active);
}
bool SotnBatRuntime_PruneStar(SotnBatRuntime *r)
{
    if (!r)
        return SDL_InvalidParamError("runtime");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (!SotnBatRuntime_StarValid(&r->children[i], i) ||
            !SotnBatRuntime_KarmaValid(&r->children[i], i))
            return SDL_SetError("Weapon15 cleanup lost actual child/row ownership");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (r->children[i].star)
            r->children[i] = (SotnBatRuntimeChild){0};
    r->star_mask = 0;
    return true;
}

bool SotnBatRuntime_RequestSire(SotnBatRuntime *r, SotnPlayerFactorySource source, Uint32 packed,
                                Uint16 item, bool *allocated)
{
    if (item != 72 && !(packed == 0x10039 && !item))
        return SDL_SetError("Sire reservation lost its actual item72");
    return SotnBatRuntime_RequestThrowing(r, source, packed, item, allocated);
}
typedef struct FoodFactoryContext
{
    SotnBatRuntime *runtime;
    SotnPlayerFactorySource source;
    Sint16 camera_delta_x, camera_delta_y;
} FoodFactoryContext;
static bool SireFactory(SotnBatRuntime *r, SotnPlayerFactorySource source, Uint16 slot, Uint16 hand,
                        Sint16 camera_x, Sint16 camera_y, bool *allocated)
{
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        if (!r->factories[i].cursor.entity_id)
        {
            source.x = FixedShift(source.x, camera_x);
            source.y = FixedShift(source.y, camera_y);
            source.flags = 0; // Original cold factory, born after this frame's root pass.
            SotnPlayerFactory f = {0};
            if (!SotnPlayerFactory_Begin(&f, source, 56U | ((Uint32)hand + 1U) << 12, 0))
                return false;
            SDL_zero(r->factories[i]);
            r->factories[i].cursor = f;
            r->factories[i].sire_factory = true;
            r->factories[i].sire_parent_slot = slot;
            *allocated = true;
            return true;
        }
    *allocated = false;
    return true;
}
static bool FoodFactory(void *context, Sint32 x, Sint32 y, Uint32 packed, bool *allocated)
{
    FoodFactoryContext *c = context;
    if (!c || !allocated || (packed & 0x3fffU) != 56 || (packed >> 16) < 29 ||
        (packed >> 16) > 69 || !(packed & 0xc000U) || (packed & 0xc000U) == 0xc000U)
        return SDL_SetError("Meal Ticket lost its actual inventory-child factory request");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        if (!c->runtime->factories[i].cursor.entity_id)
        {
            SotnPlayerFactorySource source = c->source;
            // Born after the root pass: the original cold factory has no camera-lock flag.
            source.x = FixedShift(x, c->camera_delta_x);
            source.y = FixedShift(y, c->camera_delta_y);
            source.flags = 0;
            SotnPlayerFactory f = {0};
            if (!SotnPlayerFactory_Begin(&f, source, packed, 0))
                return false;
            SDL_zero(c->runtime->factories[i]);
            c->runtime->factories[i].cursor = f;
            *allocated = true;
            return true;
        }
    *allocated = false;
    return true;
}
bool SotnBatRuntime_RequestFamiliarMedicine(SotnBatRuntime *r, SotnPlayerFactorySource source,
                                            Uint32 packed, bool *allocated)
{
    if (!r || !allocated || (packed & 65535U) != 55 || (packed >> 16) >= 20)
        return SDL_SetError("Familiar medicine requires actual55/profile0..19 with no hand bit");
    for (Uint16 i = 0; i < SDL_arraysize(r->factories); ++i)
        if (!r->factories[i].cursor.entity_id)
        {
            SotnPlayerFactory cursor = {0};
            if (!SotnPlayerFactory_Begin(&cursor, source, packed, 0))
                return false;
            SDL_zero(r->factories[i]);
            r->factories[i].cursor = cursor;
            *allocated = true;
            return true;
        }
    *allocated = false;
    return true;
}
// Weapon51's back-forward specials182..185 replace slot16 with its E1/F1 hand child.
static bool Weapon51Special(Uint16 item)
{
    return item >= 182 && item <= 185;
}
static Uint16 HandFamily(Uint16 item)
{
    return !item                          ? 255
           : item == 5                    ? 8
           : item == 6                    ? 9
           : item == 8                    ? 11
           : item == 11                   ? 25
           : item == 12                   ? 26
           : item == 167                  ? 58
           : item == 16                   ? 52
           : item == 18 || item == 174    ? 31
           : item == 19                   ? 37
           : SotnEquipment_IsSword0(item) ? 0
           : item == 85 || item == 97 || item == 102 || item == 120 || item == 186 || item == 187 ||
                   item == 188
               ? 1
           : item == 129                ? 3
           : item == 130 || item == 170 ? 4
           : item == 131 || item == 171 ? 5
           : item == 121 || item == 195 ? 54
           : item == 141                ? 56
           : item == 115 || item == 196 ? 55
           : item == 23 || item == 191  ? 38
           : item == 100 || item == 189 || item == 139 || item == 192 || item == 193 ||
                   item == 140 || item == 194
               ? 53
           : item == 112 || item == 178                               ? 48
           : item == 111 || item == 117 || item == 179 || item == 180 ? 49
           : item == 113 || item == 181                               ? 50
           : item == 132 || item == 172                               ? 6
           : item == 22 || item == 190                                ? 39
           : item == 20 || item == 175                                ? 32
           : item == 127 || item == 95 || item == 98 || item == 101 || item == 103 || item == 107 ||
                   (item >= 182 && item <= 185)
               ? 51
           : item == 21                ? 33
           : item == 26                ? 35
           : item == 28                ? 34
           : item == 17 || item == 177 ? 30
           : item == 202               ? 13
           : item == 4 || item == 173  ? 7
           : item == 7                 ? 10
           : item == 9                 ? 23
           : item == 10                ? 24
           : SotnWeaponSecondary_ShieldSpellFamily(item)
               ? SotnWeaponSecondary_ShieldSpellFamily(item)
           : item == 27  ? 36
           : item == 124 ? 57
           : item == 123 ? 43
           : item == 128 ? 2
                         : 256;
}
static Uint16 HandVariant(Uint16 item)
{
    if (SotnEquipment_IsSword0(item))
        return item == 125 ? 3 : item >= 116 ? 2 : item >= 104 ? 1 : 0;
    switch (item)
    {
    case 97:
    case 167:
    case 174:
    case 170:
    case 171:
    case 195:
    case 175:
    case 196:
    case 190:
    case 191:
    case 189:
    case 178:
    case 194:
    case 187:
    case 101:
    case 183:
    case 179:
    case 181:
    case 173:
        return 1;
    case 139:
    case 102:
    case 103:
    case 184:
    case 117:
        return 2;
    case 124:
    case 192:
    case 120:
    case 107:
    case 185:
    case 180:
        return 3;
    case 140:
    case 188:
        return 4;
    case 95:
        return 8;
    default:
        return 0;
    }
}
bool SotnBatRuntime_RequestHand(SotnBatRuntime *r, SotnPlayerFactorySource source, Uint32 packed,
                                Uint16 item, bool *allocated)
{
    const Uint16 hand = (packed & 0x800000U) != 0;
    // Weapon53's kind135 warps reserve blueprint60 (slot16 E2/F2) and weapon1's kind24 punch
    // rushes blueprint54 (slot16 E1/F1) instead of48.
    const Uint32 blueprint = item == 193 || item == 194                            ? 60U
                             : item == 186 || item == 187 || Weapon51Special(item) ? 54U
                             : item == 172                                         ? 72U
                             : item == 177                                         ? 56U
                             : SotnWeaponSecondary_ShieldSpellFamily(item)         ? 86U
                             : item == 202                                         ? 62U
                                                                                   : 48U;
    // Both-buttons combo spells (203..215, Heaven202) carry their hand bits at <<14.
    const Uint32 expected =
        item ? blueprint |
                   ((Uint32)(hand + 1U) << (SotnWeaponSecondary_ComboSpell(item) ? 14 : 12)) |
                   ((Uint32)HandVariant(item) << 16) | ((Uint32)hand << 23)
             : (42U + hand) | ((Uint32)hand << 23);
    if (!r || !allocated || HandFamily(item) > 255 ||
        (packed != expected && (packed != 0x10039 || item)))
        return SDL_SetError("Ordinary hand lost its actual item/factory/parameter identity");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        if (!r->factories[i].cursor.entity_id)
        {
            SotnPlayerFactory f = {0};
            if (!SotnPlayerFactory_Begin(&f, source, packed, (Sint16)item))
                return false;
            SDL_zero(r->factories[i]);
            r->factories[i].cursor = f;
            *allocated = true;
            return true;
        }
    *allocated = false;
    return true;
}
// Weapon43 tiles and weapon54 lines are optional; weapon1 fist trails are mandatory.
static Uint8 HandRows(Uint16 family)
{
    return (Uint8)(family == 43                   ? SOTN_HAND_WEAPON_TILES
                   : family == 1                  ? SOTN_HAND_WEAPON_FIST_ROWS
                   : family == 54 || family == 56 ? SOTN_HAND_WEAPON_GURTHANG_ROWS
                                                  : 0);
}
bool SotnBatRuntime_HandValid(const SotnBatRuntimeChild *c, Uint16 index)
{
    if (!c->hand_weapon)
        return true;
    const SotnHandWeapon *w = &c->state.hand.motion;
    const Uint16 hand = c->kind >= 0xf0;
    return index == 0 && c->generation && !c->bat_generation && !c->draw_count && !w->removed &&
           !c->food && !c->neutron && !c->sire && !c->pentagram && !c->magic && !c->thrown &&
           !c->star && !c->karma && !c->thrown_particle_parent && c->state.hand.family <= 255 &&
           c->state.hand.family == HandFamily(c->state.hand.item) &&
           c->parameter == ((HandVariant(c->state.hand.item) << 8) | (hand ? 0x8000 : 0)) &&
           w->parameter == c->parameter &&
           (c->kind == (Weapon51Special(c->state.hand.item) ? (hand ? 0xf1 : 0xe1)
                        : c->state.hand.item                ? (hand ? 0xf0 : 0xe0)
                                                            : (hand ? 0xfe : 0xee))) &&
           (c->initialized
                ? w->phase &&
                      w->phase <= (SotnHandWeapon_IsShieldFamily(c->state.hand.family) ? 4
                                   : c->state.hand.family == 32                        ? 2
                                                                                       : 1) &&
                      c->group == hand + 1U && w->properties.id == c->state.hand.item &&
                      c->primitive_count == (w->allocated ? HandRows(c->state.hand.family) : 0) &&
                      (!w->allocated || HandRows(c->state.hand.family)) &&
                      (c->state.hand.family != 1 || w->allocated)
                : !w->phase && !c->group && !c->primitive_count);
}
// Selected hand-program WFACTORY requests: blueprint56 roots, weapon48's58 branches and64
// ring and weapon1's62 fireball, with their actual parameters.
static bool SecondaryRequest(Uint16 family, Uint16 hand, Uint32 request)
{
    const Uint32 bp = request & 0xfffU, param = request >> 16;
    // Shield-spell children and weapon13's combo strike swords use the combo factory's <<14
    // hand bits.
    if (!SotnWeaponSecondary_Supported(family) ||
        (request & 0xf000U) != ((Uint32)hand + 1U) << ((SotnWeaponSecondary_ShieldSpell(family) &&
                                                        !(family == 10 && bp == 56)) ||
                                                               (family == 13 && bp == 0x46)
                                                           ? 14
                                                           : 12))
        return false;
    // Weapon10: the Iron shield guard's WFACTORY(56,0) thunder; the spell's FACTORY(88,0/1)
    // swords.
    if (family == 10)
        return (bp == 56 && param == 0) || (bp == 88 && param <= 1);
    // Weapon24: Dark's spell requests FACTORY(96)'s rocks.
    if (family == 24)
        return bp == 96 && param == 0;
    // Weapon23: Herald's spell requests FACTORY(94)'s sprites and FACTORY(90)'s swirl.
    if (family == 23)
        return (bp == 94 || bp == 90) && param == 0;
    // Weapon11: the spell's blueprint88 walker and the walker's blueprint90 afterimages.
    if (family == 11)
        return (bp == 88 && param == 0) || (bp == 90 && param == 1);
    // Weapon25: Goddess's spell requests its blueprint90 clouds.
    if (family == 25)
        return bp == 90 && param == 1;
    // Weapon52: the expiring armed shield requests its blueprint0x3A letters.
    if (family == 52)
        return bp == 0x3a && param == 0;
    // Weapon26: Shaman's spell requests blueprint92's stars, each star its blueprint90 trail.
    if (family == 26)
        return (bp == 92 && param == 0) || (bp == 90 && param == 1);
    if (family == 48)
        return (bp == 0x38 && (param == 0 || param == 0x10 || param == 0x20)) ||
               (bp == 0x3a && (param & 0xfU) >= 1 && (param & 0xfU) <= 3 && param >> 4 <= 2) ||
               (bp == 0x40 && param == 0);
    // Weapon1 descriptor4 (Tulkas's QCF) and weapon51's special request blueprint62.
    if (family == 1 || family == 51)
        return bp == 0x3e && param == 0;
    // Weapon49: sparks0x5B, swirls0x5F, the param1 ring0x40 and puffs0x5D (param0/1).
    if (family == 49)
        return ((bp == 0x5b || bp == 0x5f) && param == 0) || (bp == 0x40 && param == 1) ||
               (bp == 0x5d && param <= 1);
    // Weapon50: sparkles0x3A, ring0x40, frost0x44 and crystals0x61 (param = descriptor).
    if (family == 50)
        return ((bp == 0x3a || bp == 0x40 || bp == 0x44) && param == 0) ||
               (bp == 0x61 && param <= 3);
    // Weapon14's thrown root requests its four blueprint0x72 hitboxes; weapon13's its0x3A
    // afterimages.
    if (family == 14)
        return bp == 0x72 && param == 0;
    // Weapon7: Shield rod special173's swing requests its 0x4A orb (animation index0..6), the
    // orb its0x40 sparkles.
    if (family == 7)
        return (bp == 0x4a && param <= 6) || (bp == 0x40 && param == 0);
    // Weapon13's root requests its0x3A afterimages; combo202's sweep its0x46 strike swords
    // (D_8017A5E0 palette offsets1..8).
    if (family == 13)
        return (bp == 0x3a && param == 0) || (bp == 0x46 && param >= 1 && param <= 8);
    if (family == 12)
        return bp == 0x40 && param == 0;
    // Weapon30: the circle's soldiers0x3E (type, bit4 the six-soldier palette), the soldiers'
    // arrows/swords0x46 (0..2) and bomb0x6A.
    if (family == 30)
        return (bp == 0x3e && ((param <= 5) || (param >= 0x10 && param <= 0x15))) ||
               (bp == 0x46 && param <= 2) || (bp == 0x6a && param == 0);
    // Weapon56 (Muramasa) requests only its WFACTORY(56,0) root, like the default.
    return bp == 0x38 && (family == 53 ? param == 0 || param == 10 || param == 0x14 : param == 0);
}
bool SotnBatRuntime_WarpValid(const SotnBatRuntimeChild *c, Uint16 index)
{
    if (!c->weapon_warp)
        return true;
    const SotnWeaponWarp *w = &c->state.warp;
    const Uint16 hand = c->kind >= 0xf0;
    return index == 0 && c->generation && !c->bat_generation && !c->draw_count && !w->removed &&
           !c->group && !c->primitive_count && !c->hand_weapon && !c->weapon_secondary &&
           (c->kind == 0xe2 || c->kind == 0xf2) && (w->item == 193 || w->item == 194) &&
           c->parameter == ((w->item == 194 ? 0x100U : 0U) | (hand ? 0x8000U : 0U)) &&
           w->parameter == c->parameter && w->phase <= 3 &&
           (c->initialized ? w->phase != 0 : !w->phase);
}
// Weapon53 chains carry their own slash index (below its48-entry table); others use zero.
static bool SecondaryParameter(Uint16 family, Uint16 parameter)
{
    const Uint16 n = parameter >> 8;
    // Weapon49 children count in the low byte (sparks/swirls0..7, puffs0..7 on base0/1).
    if (family == 49)
        return n <= 1 && (parameter & 0xffU) <= 7;
    if (family == 50)
        return n <= 3 && !(parameter & 0xffU);
    // Weapon6 crescents count 0..4 on the hand bit; their sparkles use zero.
    if (family == 6)
        return (parameter & 0x7f00U) == 0 && (parameter & 0xffU) <= 4;
    // Weapon14's root carries only the hand bit; its four hitboxes count 0..3 on it.
    if (family == 14)
        return (parameter & 0x7f00U) == 0 && (parameter & 0xffU) <= 3;
    // Weapon11: the spell carries the hand bit; its walker params0, afterimages params1.
    // Weapon26: the spell carries the hand bit, its stars their index0..7, trails params1.
    if (family == 26)
        return (parameter & 0x7fffU) == 0x100 ||
               ((parameter & 0x7f00U) == 0 && (parameter & 0xffU) <= 7);
    // Weapon25: the spell carries the hand bit, its clouds params1.
    if (family == 11 || family == 25 || family == 10)
        return !(parameter & 0xffU) && ((parameter >> 8) & 0x7fU) <= 1;
    // Weapon24: the spell carries the hand bit; the rocks their spawn index0..31.
    if (family == 24)
        return !(parameter & 0x7f00U) && (parameter & 0xffU) <= 31;
    // Weapon23: the spell and swirl carry the hand bit; the sprites their spawn index0..23.
    if (family == 23)
        return !(parameter & 0x7f00U) && (parameter & 0xffU) <= 23;
    // Weapon7's orb carries its animation index0..6 (no hand bit); its sparkles zero.
    if (family == 7)
        return !(parameter & 0x80ffU) && (parameter >> 8) <= 6;
    // Weapon13's strike swords carry only their palette offset1..8 (no hand bit).
    if (family == 13 && (parameter & 0x7f00U))
        return !(parameter & 0x80ffU) && (parameter >> 8) <= 8;
    // Weapon13's root, afterimages and combo sweep, and Shotel's weapon34 boomerang, carry only
    // the hand bit.
    if (family == 13 || family == 34 || SotnWeaponSecondary_ShieldSpell(family))
        return !(parameter & 0x7fffU);
    // Weapon30: the circle carries the hand bit; soldiers their type (bit4: six-soldier
    // palette), arrows/swords0..2 and the bomb pair its low-byte index.
    if (family == 30)
        return (parameter & 0xfeU) == 0 &&
               (((parameter >> 8) & 0x7fU) <= 5 ||
                (((parameter >> 8) & 0x7fU) >= 0x10 && ((parameter >> 8) & 0x7fU) <= 0x15));
    // Weapon12's slashes carry descriptor1/2 and Crissaegrim's index0..3; the Chakram and its
    // afterimages carry only the hand bit.
    if (family == 12)
        return (parameter & 0x7f00U) <= 0x200 && !(parameter & 0xfcU);
    // Yasutsuna's blueprint68 root (weapon53 role3) carries the right hand's bit, which w_053
    // masks off; its chained slashes and every weapon56 slash carry only the index.
    return (parameter & 0xffU)                    ? false
           : family == 53 && parameter == 0x8000U ? true
           : family == 53 || family == 56         ? n < 50
           : family == 48                         ? (n & 0xfU) <= 3 && (n >> 4) <= 2
           : family == 51                         ? n <= 9
                                                  : !parameter;
}
bool SotnBatRuntime_SecondaryValid(const SotnBatRuntimeChild *c, Uint16 index)
{
    if (!c->weapon_secondary)
        return true;
    const SotnWeaponSecondary *w = &c->state.secondary.motion;
    const Uint16 family = c->state.secondary.family;
    // Blueprint54's slot16 punch rush owns its own item and hand group; weapon1's role1 exists
    // only there.
    const bool rush = index == 0;
    if (rush != (family == 1 && (c->kind == 0xe1 || c->kind == 0xf1)))
        return false;
    if (rush && (family != 1 || (c->kind != 0xe1 && c->kind != 0xf1) ||
                 (c->state.secondary.item != 186 && c->state.secondary.item != 187) ||
                 c->parameter != ((c->state.secondary.item == 187 ? 0x100U : 0U) |
                                  (c->kind == 0xf1 ? 0x8000U : 0U)) ||
                 (c->initialized && c->group != (c->kind == 0xf1 ? 2U : 1U))))
        return false;
    return index <= 47 && c->generation && !c->bat_generation && !c->draw_count && !w->removed &&
           !c->food && !c->neutron && !c->sire && !c->pentagram && !c->magic && !c->thrown &&
           !c->star && !c->karma && !c->hand_weapon && !c->thrown_particle_parent &&
           ((c->kind & 0xf0U) == 0xe0 || (c->kind & 0xf0U) == 0xf0) &&
           SotnWeaponSecondary_SupportedRole(family, c->kind & 15U) &&
           (rush || SecondaryParameter(family, c->parameter)) && w->parameter == c->parameter &&
           (family == 6 && (c->kind & 15U) == 1
                ? c->state.secondary.own_item && c->state.secondary.item == 172
            : (family == 14 || family == 13) && (c->kind & 15U) == 0
                ? c->state.secondary.own_item &&
                      c->state.secondary.item == (family == 14 ? 143 : 119) &&
                      !(c->parameter & 0xffU) && c->state.secondary.parent_slot == 16
            : family == 13 && (c->kind & 15U) == 2
                ? c->state.secondary.own_item && c->state.secondary.item == 202 &&
                      !(c->parameter & 0x7fffU) && c->state.secondary.parent_slot == 16
            : SotnWeaponSecondary_ShieldSpell(family) && (c->kind & 15U) == 8
                ? c->state.secondary.own_item &&
                      c->state.secondary.item == SotnWeaponSecondary_ShieldSpellItem(family) &&
                      !(c->parameter & 0x7fffU) && c->state.secondary.parent_slot == 16
            : family == 30 && (c->kind & 15U) == 1
                ? c->state.secondary.own_item && c->state.secondary.item == 177 &&
                      !(c->parameter & 0x7fffU) && c->state.secondary.parent_slot == 16
            : family == 34 ? c->state.secondary.own_item && c->state.secondary.item == 176 &&
                                 !(c->parameter & 0x7fffU) && c->state.secondary.parent_slot == 16
            : family == 53 && (c->kind & 15U) == 3
                ? c->state.secondary.own_item && c->state.secondary.item == 165 &&
                      !(c->parameter & 0x7fffU) && c->state.secondary.parent_slot == 16
            : family == 12 && (c->kind & 15U) <= 1
                ? c->state.secondary.own_item && c->state.secondary.parent_slot == 16 &&
                      ((c->kind & 15U) ? c->state.secondary.item == 133 && !(c->parameter & 0x7fffU)
                                       : (c->state.secondary.item == 163 &&
                                          (c->parameter & 0x7fffU) == 0x100) ||
                                             (c->state.secondary.item == 164 &&
                                              (c->parameter & 0x7f00U) == 0x200))
                : c->state.secondary.own_item == rush) &&
           c->state.secondary.parent_slot >= 16 && c->state.secondary.parent_slot <= 47 &&
           SotnWeaponSecondary_Supported(family) &&
           // A weapon30 soldier waits in phase0 for its circle's wave, its rows not yet taken.
           (c->initialized && family == 30 && (c->kind & 15U) == 2 && !w->phase
                ? !c->group && !w->allocated &&
                      c->primitive_count == SotnWeaponSecondary_RoleRows(family, 2)
            : c->initialized
                ? w->phase >= 1 &&
                      w->phase <= (family == 49 && (c->kind & 15U) == 4                     ? 2
                                   : family == 6 && (c->kind & 15U) == 1                    ? 4
                                   : (family == 14 || family == 13) && (c->kind & 15U) == 0 ? 3
                                   : family == 13 && (c->kind & 15U) == 2                   ? 4
                                   : family == 13 && (c->kind & 15U) == 3                   ? 3
                                   : family == 7 && (c->kind & 15U) == 1                    ? 4
                                   : family == 10 && (c->kind & 15U) == 8                   ? 9
                                   : family == 23 && (c->kind & 15U) == 8                   ? 8
                                   : family == 23 && (c->kind & 15U) == 9                   ? 3
                                   : family == 23                                           ? 2
                                   : family == 24 && (c->kind & 15U) == 8                   ? 8
                                   : family == 24                                           ? 3
                                   : (family == 12 || family == 34) && (c->kind & 15U) == 1 ? 2
                                   : family == 30 && (c->kind & 15U) == 1                   ? 4
                                   : family == 30 && (c->kind & 15U) == 2                   ? 8
                                   : family == 30                                           ? 4
                                   : family == 58                                           ? 6
                                   : family == 11 && (c->kind & 15U) == 9                   ? 4
                                   : family == 25 && (c->kind & 15U) == 9                   ? 3
                                   : family == 26 && (c->kind & 15U) == 9                   ? 2
                                   : family == 26                                           ? 8
                                   : family == 52 && (c->kind & 15U) == 1                   ? 2
                                   : family == 52                                           ? 8
                                   : family == 25                                           ? 9
                                   : family == 11                                           ? 7
                                   : family == 8 || family == 9                             ? 7
                                                                                            : 1) &&
                      w->allocated && c->group <= 10 &&
                      c->primitive_count == SotnWeaponSecondary_HeldRows(family, c->kind & 15U,
                                                                         c->parameter, w->phase) &&
                      // Weapon30 children and shield spells set no property roll (weapon11's
                      // afterimages keep the factory's equipId0 and roll none either).
                      // Weapon24's spell rolls its own (item208) when it strikes.
                      (family == 24 && (c->kind & 15U) == 8
                           ? !w->properties.id || w->properties.id == 208
                           : w->properties.id ==
                                 (family == 30 || (SotnWeaponSecondary_ShieldSpell(family) &&
                                                   (c->kind & 15U) == 8)
                                      ? 0
                                      : c->state.secondary.item))
                : !w->phase && !c->group && !c->primitive_count);
}
typedef struct HandPropertyContext
{
    const SotnCharacter *character;
    Uint16 item;
    bool poison;
} HandPropertyContext;
bool SotnBatRuntime_RetireHand(SotnBatRuntime *r, bool shield_only)
{
    if (!r)
        return SDL_InvalidParamError("runtime");
    if (shield_only && !r->hand_shield)
        return true;
    SotnBatRuntimeChild *c = &r->children[0];
    if (c->hand_weapon && !SotnBatRuntime_HandValid(c, 0))
        return SDL_SetError("Ordinary retirement lost its actual hand owner");
    if (shield_only && (!c->hand_weapon || !SotnHandWeapon_IsShieldFamily(c->state.hand.family)))
        return SDL_SetError("Conditional retirement lost its actual shield owner");
    *c = (SotnBatRuntimeChild){0};
    r->hand_shield = 0;
    r->hand_attack = false;
    r->axe_mask &= ~1ULL;
    r->sword_mask &= ~1ULL;
    r->groups[1] = r->groups[2] = 0;
    return true;
}
static bool HandProperties(void *context, Uint32 *random, SotnEquipment *out)
{
    const HandPropertyContext *p = context;
    return SotnCharacter_HandProperties(p->character, p->item, p->poison, random, out);
}
static bool ClearHandShield(SotnBatRuntime *r, SotnPrimitiveSlots *slots)
{
    if (!r->hand_shield)
        return true;
    SotnBatRuntimeChild *c = &r->children[0];
    if (!c->hand_weapon || !SotnHandWeapon_IsShieldFamily(c->state.hand.family) ||
        !SotnBatRuntime_HandValid(c, 0))
        return SDL_SetError("Conditional hand retirement lost its actual shield owner");
    if (c->primitive_count && !SotnPrimitiveSlots_Release(slots, c->primitive_count, c->primitives))
        return false;
    *c = (SotnBatRuntimeChild){0};
    r->hand_shield = 0;
    r->hand_attack = false;
    return true;
}
bool SotnBatRuntime_RequestSword(SotnBatRuntime *r, SotnPlayerFactorySource source, Uint32 packed,
                                 Sint16 argument, bool *allocated)
{
    if (!r || !allocated ||
        !(((packed == 0x1036 || packed == 0x802036) && argument == 197) ||
          ((packed == 0x14003d || packed == 0x15003d) && !argument)))
        return SDL_SetError("Sword special requires its original root/outline factory boundary");
    for (Uint16 i = 0; i < 8; ++i)
        if (!r->factories[i].cursor.entity_id)
        {
            SotnPlayerFactory f = {0};
            if (!SotnPlayerFactory_Begin(&f, source, packed, argument))
                return false;
            SDL_zero(r->factories[i]);
            r->factories[i].cursor = f;
            r->factories[i].bat_generation = 0;
            r->factories[i].sword_parent = 0;
            *allocated = true;
            return true;
        }
    *allocated = false;
    return true;
}
static bool Sound(SotnBatRuntime *r, Uint16 sound)
{
    if (r->sound_count >= SDL_arraysize(r->sounds))
        return SDL_SetError("Shared player effects exceeded their bounded sound requests");
    r->sounds[r->sound_count++] = sound;
    return true;
}
// DRA AddHearts(1): below the maximum, one heart (capped), then the player's
// FACTORY(BP_SMALL_RISING_HEART,0) rising heart and SFX_HEART_PICKUP; at the maximum nothing.
static bool AddHearts(SotnBatRuntime *r, SotnPlayerState p, SotnCharacterState *progress)
{
    if (progress->hearts >= progress->hearts_max)
        return true;
    progress->hearts = (Uint16)SDL_min(progress->hearts_max, progress->hearts + 1U);
    return Request(r, p, 99, 0) && Sound(r, 0x67a);
}
static bool Spell(SotnDataView dra, SotnCharacterState *profile, SotnCharacterStats stats,
                  Uint16 id, SotnPlayerAttack *attack)
{
    const size_t at = 0x8410U + id * 28U;
    if (!dra.bytes || at > dra.size || dra.size - at < 28)
        return SDL_SetError("Bat attack requires its private spell definition");
    const Uint8 *s = dra.bytes + at;
    const Uint16 base = Read16(s + 24);
    if (base > 900 || Read16(s + 16) != (id == 5 ? 0x82U : 2U) || Read16(s + 20) > 255)
        return SDL_SetError("Bat attack has invalid spell attack metadata");
    Uint32 power = base + (stats.total[2] * 2U + SotnCombat_Random(&profile->random) % 12U) / 10U;
    if (profile->equipment[SOTN_EQUIP_ARMOR] == 21)
        power += power / 2U;
    if (power > 999)
        return SDL_SetError("Bat spell attack exceeds original native combat bounds");
    *attack = (SotnPlayerAttack){.offset_x = id == 6 ? 4 : 0,
                                 .width = id == 6                ? 12
                                          : (id == 1 || id == 3) ? 6
                                                                 : 4,
                                 .height = id == 6                ? 12
                                           : (id == 1 || id == 3) ? 6
                                                                  : 8,
                                 .power = (Uint16)power,
                                 .element = Read16(s + 22),
                                 .critical_rate = (Uint8)Read16(s + 20),
                                 .hit_type = Read16(s + 16),
                                 .hit_effect = Read16(s + 18),
                                 .stun_frames = Read16(s + 14),
                                 .invincibility_frames = s[13],
                                 .active = true};
    return true;
}
static bool ChildKind(Uint16 kind)
{
    return kind == 2 || kind == 5 || kind == 11 || kind == 16 || kind == 19 || kind == 30 ||
           kind == 33 || kind == 32 || kind == 31 || kind == 35 || kind == 36 || kind == 37 ||
           kind == 38 || kind == 39 || kind == 45 || kind == 46 || kind == 49 || kind == 50 ||
           kind == 54 || kind == 47 || kind == 51 || kind == 52 || kind == 53 || kind == 61 ||
           kind == 63 || kind == 65 || kind == 0xe0 || kind == 0xf0 || kind == 0xe1 ||
           kind == 0xee || kind == 0xfe || (kind >= 0xe1 && kind <= 0xe5) ||
           (kind >= 0xf1 && kind <= 0xf5) || (kind >= 24 && kind <= 29) || kind == 0xe8 ||
           kind == 0xf8 || kind == 0xe9 || kind == 0xf9 || kind == 0xea || kind == 0xfa;
}
static Uint32 ChildFlags(const SotnBatRuntimeChild *c)
{
    if (!c->initialized)
        return c->source.flags;
    if (c->hand_weapon)
        return c->state.hand.motion.flags;
    if (c->weapon_secondary)
        return c->state.secondary.motion.flags;
    if (c->weapon_warp)
        return c->state.warp.flags;
    if (c->karma)
        return c->state.karma.motion.flags;
    if (c->star)
        return c->state.star.motion.motion.flags;
    if (c->thrown)
        return c->state.thrown.motion.flags;
    if (c->food)
        return c->state.food.flags;
    if (c->neutron)
        return c->state.neutron.motion.flags;
    if (c->pentagram)
        return c->state.pentagram.motion.flags;
    if (c->magic)
        return c->state.magic.motion.flags;
    if (c->sire)
        return c->sire_child ? c->state.sire.child.flags : c->state.sire.parent.flags;
    switch (c->kind)
    {
    case 2:
        return 0x08110000U;
    case 5:
        return c->state.particles.flags;
    case 11:
    case 31:
    case 38:
        return 0x04870000U;
    case 39:
        return 0x04070000U;
    case 46:
        return 0x04830000U;
    case 49:
    case 50:
        return 0x04060000U;
    case 19:
        return 0x04810000U;
    case 37:
        return c->primitive_count ? 0x04830000U : 0x04030000U;
    case 32:
        return c->state.dissolve.flags;
    case 65:
        return 0x0c810000U;
    case 33:
        return c->state.ice.flags;
    case 30:
        return 0x08820000U;
    case 16:
    case 54:
        return 0x08120000U;
    case 47:
        return 0x00860000U;
    case 35:
    case 51:
        return 0x08000000U;
    case 36:
    case 53:
        return 0x00060000U;
    case 24:
        return c->state.hellfire_trail.motion.flags;
    case 25:
        return c->state.hellfire_beam.flags;
    case 26:
    case 27:
        return c->state.hellfire_fire.motion.flags;
    case 28:
    case 29:
        return c->state.hellfire_effect.flags;
    case 45:
        return 0x08100000U;
    case 52:
        return 0x00840000U;
    case 61:
        return c->state.spirit.motion.flags;
    case 63:
        return c->state.portal.flags;
    case 0xe1:
    case 0xf1:
        return 0x04020000U;
    case 0xe2:
    case 0xf2:
        return 0x04160000U;
    default:
        return 0;
    }
}
static bool Flash(SotnBatRuntime *r, SotnDataView dra, Uint16 profile)
{
    const size_t at = 0xcfb4U + profile * 4U;
    if (!dra.bytes || at > dra.size || dra.size - at < 4)
        return SDL_SetError("Player flash requires its original private profile%u", profile);
    const Uint8 *data = dra.bytes + at;
    r->screen_flash_ticks = data[0];
    r->screen_red = data[1];
    r->screen_green = data[2];
    r->screen_blue = data[3];
    return true;
}
static bool SpiritRows(const SotnBatRuntimeChild *c)
{
    if (c->kind == 61)
        return c->parameter == c->state.spirit.motion.parameter &&
               (c->initialized
                    ? c->state.spirit.motion.phase && c->state.spirit.motion.phase <= 3 &&
                          !c->state.spirit.motion.removed && c->primitive_count == 16 &&
                          c->group >= 3 && c->group <= 10
                    : !c->state.spirit.motion.phase && !c->primitive_count) &&
               (!c->state.spirit.target_bound || c->state.spirit.target_slot < SOTN_SPELL_TARGETS);
    if (c->kind == 63)
        return c->parameter == c->state.portal.parameter && !c->state.portal.removed &&
               (c->initialized ? c->state.portal.phase && c->state.portal.phase <= 3 &&
                                     (c->state.portal.phase == 1 ? !c->primitive_count
                                                                 : c->primitive_count == 9)
                               : !c->state.portal.phase && !c->primitive_count);
    return true;
}
bool SotnBatRuntime_HellfireChildValid(const SotnBatRuntimeChild *c, Uint16 index)
{
    if (c->kind == 24)
        return c->parameter == c->state.hellfire_trail.motion.parameter &&
               (c->parameter >> 8) <= 1 && !c->state.hellfire_trail.motion.removed &&
               !c->primitive_count &&
               (c->initialized ? c->state.hellfire_trail.motion.phase == 1
                               : !c->state.hellfire_trail.motion.phase);
    if (c->kind == 25)
        return !c->state.hellfire_beam.removed &&
               (c->initialized ? c->state.hellfire_beam.phase >= 1 &&
                                     c->state.hellfire_beam.phase <= 7 && c->primitive_count == 1
                               : !c->state.hellfire_beam.phase && !c->primitive_count);
    if (c->kind == 26 || c->kind == 27)
        return index >= 1 && index <= 31 &&
               c->parameter == c->state.hellfire_fire.motion.parameter &&
               c->state.hellfire_fire.motion.big == (c->kind == 27) &&
               !c->state.hellfire_fire.motion.removed && !c->primitive_count &&
               (c->initialized
                    ? c->state.hellfire_fire.motion.phase >= 1 &&
                          c->state.hellfire_fire.motion.phase <= (c->kind == 26 ? 2 : 1) &&
                          c->group >= (index < 16 ? 3 : 7) && c->group <= (index < 16 ? 6 : 10)
                    : !c->state.hellfire_fire.motion.phase && !c->group);
    if (c->kind == 28 || c->kind == 29)
        return c->state.hellfire_effect.kind == c->kind && !c->state.hellfire_effect.removed &&
               (c->initialized ? c->state.hellfire_effect.phase == 1 && c->primitive_count == 1
                               : !c->state.hellfire_effect.phase && !c->primitive_count);
    return true;
}
bool SotnBatRuntime_ImmunityTextValid(const SotnBatRuntimeChild *c)
{
    if (c->kind != 37)
        return true;
    const SotnImmunityText *n = &c->state.immunity_text;
    return c->generation &&
           (c->parameter <= 0x300 || (c->parameter >= 0x800 && c->parameter <= 0x1300)) &&
           !(c->parameter & 255U) && !c->bat_generation && !c->group && !c->draw_count &&
           !n->removed && n->phase <= (c->parameter ? 7 : 5) &&
           (c->initialized
                ? n->phase && (n->phase >= 3 ? c->primitive_count == 18 : !c->primitive_count)
                : !n->phase && !c->primitive_count);
}
bool SotnBatRuntime_DissolveValid(const SotnBatRuntimeChild *c)
{
    if (c->kind != 32)
        return true;
    const SotnStoneDissolve *n = &c->state.dissolve;
    return c->generation && !c->bat_generation && !c->parameter && !c->group && !c->draw_count &&
           !n->removed && n->phase <= 4 && n->pattern < 8 && n->index < 8 &&
           (n->phase != 4 || (n->timer && n->timer <= 48)) &&
           (c->initialized ? n->phase && c->primitive_count == SOTN_STONE_DISSOLVE_ROWS &&
                                 n->flags == 0x08800000U
                           : !n->phase && !c->primitive_count);
}
static void PruneRoomFactory(SotnBatRuntime *r, Uint16 blueprint)
{
    for (Uint16 i = 0; i < SDL_arraysize(r->factories); ++i)
        if (r->factories[i].cursor.entity_id && r->factories[i].cursor.blueprint == blueprint &&
            (!r->factories[i].cursor.initialized ||
             (r->factories[i].cursor.origin != 4 && r->factories[i].cursor.origin != 5)))
            SDL_zero(r->factories[i]);
}
bool SotnBatRuntime_PruneDissolve(SotnBatRuntime *r)
{
    if (!r)
        return SDL_InvalidParamError("runtime");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (!SotnBatRuntime_DissolveValid(&r->children[i]))
            return SDL_SetError("Dissolve cleanup lost its actual child/row lifecycle");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (r->children[i].kind == 32)
            r->children[i] = (SotnBatRuntimeChild){0};
    PruneRoomFactory(r, 39);
    return true;
}
bool SotnBatRuntime_TeleportValid(const SotnBatRuntimeChild *c)
{
    if (!c)
        return false;
    if (c->kind != 65)
        return true;
    const SotnTeleportEffect *n = &c->state.teleport;
    return c->generation && !c->bat_generation && !(c->parameter & 255U) &&
           n->parameter == c->parameter && !n->removed && !c->draw_count &&
           (c->initialized ? (n->phase >= 1 && n->phase <= 6) || (n->phase >= 20 && n->phase <= 23)
                           : n->phase == 0) &&
           c->primitive_count == (c->initialized ? SOTN_TELEPORT_ROWS : 0);
}
bool SotnBatRuntime_CircleValid(const SotnBatRuntimeChild *c)
{
    if (!c)
        return false;
    if (c->kind != 38)
        return true;
    const SotnPlayerCircle *n = &c->state.circle;
    return c->generation && !c->bat_generation && !c->group && !c->draw_count && !n->removed &&
           ((c->parameter >> 8) & 63U) < 22 && n->parameter == c->parameter &&
           (c->initialized ? n->phase >= 1 && n->phase <= 3 : !n->phase) &&
           c->primitive_count == (c->initialized ? SOTN_PLAYER_CIRCLE_ROWS : 0);
}
bool SotnBatRuntime_MedicineValid(const SotnBatRuntimeChild *c)
{
    if (!c)
        return false;
    if (c->kind != 39)
        return true;
    const SotnMedicineEffect *n = &c->state.medicine;
    return c->generation && !c->bat_generation && !c->group && !c->draw_count &&
           !c->primitive_count && !n->removed && ((c->parameter >> 8) & 127U) < 20 &&
           n->parameter == c->parameter && n->index >= 0 && n->index <= 7 &&
           (c->initialized ? n->phase >= 1 && n->phase <= 2 : !n->phase);
}
bool SotnBatRuntime_PruneMedicines(SotnBatRuntime *r)
{
    if (!r)
        return SDL_InvalidParamError("runtime");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (!SotnBatRuntime_CircleValid(&r->children[i]) ||
            !SotnBatRuntime_MedicineValid(&r->children[i]) ||
            !SotnBatRuntime_MagicMissileValid(&r->children[i], i))
            return SDL_SetError("Medicine room cleanup lost its actual child/row lifecycle");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if ((r->children[i].kind == 38 || r->children[i].kind == 39) &&
            !(ChildFlags(&r->children[i]) & 0x20000U))
            r->children[i] = (SotnBatRuntimeChild){0};
    return true;
}
bool SotnBatRuntime_PruneRoomFactories(SotnBatRuntime *r)
{
    if (!r)
        return SDL_InvalidParamError("runtime");
    for (Uint16 i = 0; i < SDL_arraysize(r->factories); ++i)
        if (r->factories[i].cursor.entity_id && !(r->factories[i].cursor.source.flags & 0x20000U))
            SDL_zero(r->factories[i]);
    return true;
}
bool SotnBatRuntime_PruneFood(SotnBatRuntime *r)
{
    if (!r)
        return SDL_InvalidParamError("runtime");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (!SotnBatRuntime_FoodChildValid(&r->children[i], i))
            return SDL_SetError("Food room cleanup lost its actual child lifecycle");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (r->children[i].food)
            r->children[i] = (SotnBatRuntimeChild){0};
    return true;
}
bool SotnBatRuntime_NeutronValid(const SotnBatRuntimeChild *c, Uint16 index)
{
    if (!c->neutron)
        return true;
    const SotnNeutron *n = &c->state.neutron.motion;
    return !c->food && (c->kind == 0xe0 || c->kind == 0xf0) && index >= 1 && index <= 15 &&
           c->generation && !c->bat_generation && !n->removed && n->phase <= 3 &&
           c->parameter == (c->kind == 0xf0 ? 0x8000 : 0) && c->state.neutron.animation.pose < 7 &&
           c->state.neutron.animation.remaining >= -1 &&
           (n->phase < 2 ||
            (c->state.neutron.attack.equipment == 71 && c->state.neutron.attack.hit_type == 0x82 &&
             c->group >= 3 && c->group <= 6)) &&
           (c->initialized ? c->primitive_count == 1 && n->phase != 0
                           : !c->primitive_count && !n->phase && !c->draw_count) &&
           (!c->group || (c->group >= 3 && c->group <= 6));
}
bool SotnBatRuntime_PruneNeutron(SotnBatRuntime *r)
{
    if (!r)
        return SDL_InvalidParamError("runtime");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (!SotnBatRuntime_NeutronValid(&r->children[i], i) ||
            !SotnBatRuntime_SireValid(&r->children[i], i))
            return SDL_SetError("Neutron room cleanup lost its actual child lifecycle");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (r->children[i].neutron)
            r->children[i] = (SotnBatRuntimeChild){0};
    r->neutron_mask = 0;
    return true;
}
bool SotnBatRuntime_SireValid(const SotnBatRuntimeChild *c, Uint16 index)
{
    if (!c->sire)
        return !c->sire_child;
    const bool burst = c->sire_child;
    const Uint8 phase = burst ? c->state.sire.child.phase : c->state.sire.parent.phase;
    const Uint8 timeline = burst ? 3 : c->state.sire.parent.animation;
    return !c->food && !c->neutron && index >= 1 && index <= 15 && c->generation &&
           !c->bat_generation && !c->primitive_count && !c->draw_count &&
           (burst ? c->kind == 0xe1 || c->kind == 0xf1 : c->kind == 0xe0 || c->kind == 0xf0) &&
           c->parameter == (!burst && c->kind >= 0xf0 ? 0x8000 : 0) &&
           !(burst ? c->state.sire.child.removed : c->state.sire.parent.removed) &&
           phase <= (burst ? 2 : 3) && c->state.sire.animation.remaining >= -1 &&
           c->state.sire.animation.pose < (timeline == 1   ? 15
                                           : timeline == 2 ? 16
                                                           : 3) &&
           (c->initialized ? phase != 0 && timeline >= 1 && timeline <= 3 : !phase) &&
           (burst ? !c->group && c->state.sire.parent_slot >= 17 && c->state.sire.parent_slot <= 31
                  : !c->initialized ||
                        (c->state.sire.attack.equipment == 72 &&
                         c->state.sire.attack.hit_type == 0x82 && c->group >= 3 && c->group <= 6));
}
bool SotnBatRuntime_PruneSire(SotnBatRuntime *r)
{
    if (!r)
        return SDL_InvalidParamError("runtime");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (!SotnBatRuntime_SireValid(&r->children[i], i))
            return SDL_SetError("Sire room cleanup lost its actual child lifecycle");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (r->children[i].sire ||
            (r->children[i].kind == 5 && (r->children[i].parameter >> 8) == 12))
            r->children[i] = (SotnBatRuntimeChild){0};
    r->sire_mask = 0;
    return true;
}
bool SotnBatRuntime_PentagramValid(const SotnBatRuntimeChild *c, Uint16 index)
{
    if (!c->pentagram)
        return true;
    const SotnPentagram *p = &c->state.pentagram.motion;
    const Uint16 item = c->state.pentagram.item;
    return !c->food && !c->neutron && !c->sire && !c->sire_child && index >= 1 && index <= 15 &&
           c->generation && !c->bat_generation && !c->draw_count && !p->removed && p->phase <= 2 &&
           (c->kind == 0xe0 || c->kind == 0xf0) && (item == 73 || item == 74) &&
           c->parameter == (item == 74 ? 0x100U : 0U) + (c->kind == 0xf0 ? 0x8000U : 0U) &&
           p->parameter == c->parameter &&
           (c->initialized
                ? p->phase && p->lifetime >= 1 && p->lifetime <= (p->phase == 1 ? 24 : 128) &&
                      p->flags == 0x04800000 && c->primitive_count == SOTN_PENTAGRAM_ROWS &&
                      !(p->phase == 2 && c->state.pentagram.attack.active) && c->group >= 3 &&
                      c->group <= 6 && c->state.pentagram.attack.equipment == item &&
                      c->state.pentagram.attack.hit_type == 0x82
                : !p->phase && !c->primitive_count && !c->group);
}
bool SotnBatRuntime_PrunePentagram(SotnBatRuntime *r)
{
    if (!r)
        return SDL_InvalidParamError("runtime");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (!SotnBatRuntime_PentagramValid(&r->children[i], i))
            return SDL_SetError("Pentagram room cleanup lost its actual child lifecycle");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (r->children[i].pentagram)
            r->children[i] = (SotnBatRuntimeChild){0};
    r->pentagram_mask = 0;
    return true;
}
bool SotnBatRuntime_MagicMissileValid(const SotnBatRuntimeChild *c, Uint16 index)
{
    if (!c->magic)
        return true;
    const SotnMagicMissile *m = &c->state.magic.motion;
    const bool arrow = c->kind == 0xe1 || c->kind == 0xf1;
    const bool role = arrow || c->kind == 0xe0 || c->kind == 0xf0;
    return role && !c->food && !c->sire && !c->neutron && !c->pentagram && !c->thrown &&
           index >= 1 && index <= 15 && c->generation && !c->bat_generation &&
           !c->primitive_count && !c->draw_count && !m->removed && m->phase <= 2 &&
           c->parameter == (arrow             ? 0
                            : c->kind == 0xf0 ? 0x8000U
                                              : 0U) &&
           (arrow ? c->state.magic.parent_slot >= 17 && c->state.magic.parent_slot <= 31
                  : !c->state.magic.parent_slot && m->item == 25) &&
           (c->initialized
                ? m->phase && m->flags == (arrow ? 0x08100000U : 0x00140000U) &&
                      c->state.magic.timeline == (arrow ? m->phase == 1 ? 2 : 3 : 1) &&
                      c->state.magic.animation.remaining >= -1 &&
                      c->state.magic.animation.pose < (arrow ? m->phase == 1 ? 2 : 7 : 33) &&
                      (arrow ? c->group >= 3 && c->group <= 6 &&
                                   c->state.magic.attack.equipment == m->item &&
                                   c->state.magic.attack.hit_type == 2
                             : !c->group && !c->state.magic.attack.active &&
                                   (m->phase == 1 || (m->lifetime >= 1 && m->lifetime <= 16)))
                : !m->phase && !c->state.magic.timeline && !c->group &&
                      !c->state.magic.attack.active && (!arrow || !m->item));
}
bool SotnBatRuntime_ThrownValid(const SotnBatRuntimeChild *c, Uint16 index)
{
    if (c->thrown_particle_parent)
        return !c->thrown && c->kind == 5 && (c->parameter >> 8) == 11 && c->generation &&
               !c->bat_generation && !c->particle_parent && !c->group &&
               c->particle_parent_slot >= 17 && c->particle_parent_slot <= 31 && index >= 16 &&
               index < SOTN_BAT_RUNTIME_SLOTS;
    if (!c->thrown)
        return true;
    const Uint16 role = c->kind & 15U;
    const SotnThrownWeapon *s = &c->state.thrown.motion;
    const bool root = role == 0 || role == 2 || role == 3 || role == 5;
    const Uint16 item = c->state.thrown.item;
    return ((c->kind >= 0xe0 && c->kind <= 0xe5) || (c->kind >= 0xf0 && c->kind <= 0xf5)) &&
           !c->food && !c->sire && !c->magic && !c->neutron && !c->pentagram &&
           (role == 1   ? index >= 16 && index <= 47
            : role == 4 ? index >= 9 && index <= 15
                        : index >= 1 && index <= 15) &&
           c->generation && !c->bat_generation && !c->draw_count && !s->removed &&
           s->phase <= (role == 5 ? 4 : 1) && s->parameter == c->parameter &&
           c->primitive_count == (c->initialized && role == 0 ? 1 : 0) &&
           (root ? ThrownItem(item) && !c->state.thrown.parent_slot &&
                       role == (item == 80                  ? 0
                                : item == 81 || item == 134 ? 2
                                : item == 82                ? 3
                                                            : 5) &&
                       c->parameter ==
                           ((c->kind >= 0xf0 ? 0x8000U : 0U) | (item == 134 ? 0x100U : 0U))
                 : !item && !c->parameter && c->state.thrown.parent_slot >= 17 &&
                       c->state.thrown.parent_slot <= 31) &&
           (c->initialized ? s->phase && s->priority <= 511 &&
                                 (root ? c->group >= 3 && c->group <= 6 &&
                                             c->state.thrown.attack.equipment == item &&
                                             c->state.thrown.attack.hit_type == 2 &&
                                             c->state.thrown.attack.width == s->width &&
                                             c->state.thrown.attack.height == s->height
                                       : !c->group && !c->state.thrown.attack.active)
                           : !s->phase && !c->group && !c->state.thrown.attack.active);
}
bool SotnBatRuntime_PruneThrown(SotnBatRuntime *r)
{
    if (!r)
        return SDL_InvalidParamError("runtime");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (!SotnBatRuntime_ThrownValid(&r->children[i], i) ||
            !SotnBatRuntime_StarValid(&r->children[i], i) ||
            !SotnBatRuntime_KarmaValid(&r->children[i], i))
            return SDL_SetError("Weapon16 room cleanup lost its actual child/row ownership");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (r->children[i].thrown || r->children[i].thrown_particle_parent ||
            (r->children[i].kind == 2 && (r->children[i].parameter >> 8) == 9))
            r->children[i] = (SotnBatRuntimeChild){0};
    r->thrown_mask = 0;
    return true;
}
bool SotnBatRuntime_PruneMagicMissile(SotnBatRuntime *r)
{
    if (!r)
        return SDL_InvalidParamError("runtime");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (!SotnBatRuntime_MagicMissileValid(&r->children[i], i))
            return SDL_SetError("Magic Missile cleanup lost its actual callback/parent ownership");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (r->children[i].magic)
            r->children[i] = (SotnBatRuntimeChild){0};
    r->magic_mask = 0;
    r->magic_clear_shield = false;
    return true;
}
static bool ChangedWeapon(Uint16 kind, Uint8 hands)
{
    return (kind >= 0xe0 && kind <= 0xef && (hands & 1)) ||
           (kind >= 0xf0 && kind <= 0xff && (hands & 2)) ||
           (hands && ((kind >= 0xe8 && kind <= 0xed) || (kind >= 0xf8 && kind <= 0xfd)));
}
bool SotnBatRuntime_PruneWeapons(SotnBatRuntime *r, Uint8 hands)
{
    if (!r || hands > 3)
        return SDL_SetError("Weapon retirement requires actual changed hand bits");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (!SotnBatRuntime_SwordChildValid(&r->children[i], i) ||
            !SotnBatRuntime_NeutronValid(&r->children[i], i) ||
            !SotnBatRuntime_SireValid(&r->children[i], i) ||
            !SotnBatRuntime_PentagramValid(&r->children[i], i) ||
            !SotnBatRuntime_MagicMissileValid(&r->children[i], i) ||
            !SotnBatRuntime_ThrownValid(&r->children[i], i) ||
            !SotnBatRuntime_StarValid(&r->children[i], i) ||
            !SotnBatRuntime_KarmaValid(&r->children[i], i))
            return SDL_SetError("Weapon retirement lost its actual child ownership");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (ChangedWeapon(r->children[i].kind, hands))
        {
            r->children[i] = (SotnBatRuntimeChild){0};
            r->sword_mask &= ~((Uint64)1 << i);
            r->neutron_mask &= ~((Uint64)1 << i);
            r->sire_mask &= ~((Uint64)1 << i);
            r->pentagram_mask &= ~((Uint64)1 << i);
            r->magic_mask &= ~((Uint64)1 << i);
            r->thrown_mask &= ~((Uint64)1 << i);
            r->star_mask &= ~((Uint64)1 << i);
            r->karma_mask &= ~((Uint64)1 << i);
            r->secondary_mask &= ~((Uint64)1 << i);
            r->secondary_mask &= ~((Uint64)1 << i);
        }
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        if (ChangedWeapon(r->factories[i].cursor.entity_id, hands))
            SDL_zero(r->factories[i]);
    if (!r->children[0].kind)
    {
        r->hand_attack = false;
        r->hand_shield = 0;
    }
    return true;
}
bool SotnBatRuntime_PruneTeleport(SotnBatRuntime *r)
{
    if (!r)
        return SDL_InvalidParamError("runtime");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (!SotnBatRuntime_TeleportValid(&r->children[i]))
            return SDL_SetError("Teleport cleanup lost its actual child/row lifecycle");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (r->children[i].kind == 65)
            r->children[i] = (SotnBatRuntimeChild){0};
    PruneRoomFactory(r, 121);
    r->teleport_palette = 0;
    r->teleport_warp_mode = 0;
    r->teleport_acknowledged = false;
    return true;
}
bool SotnBatRuntime_IceHitValid(const SotnBatRuntimeChild *c)
{
    if (c->kind != 33)
        return true;
    const SotnIceHit *n = &c->state.ice;
    if (!c->generation || c->bat_generation || c->parameter || c->group || c->draw_count ||
        c->particle_parent || n->removed || n->phase > 2)
        return false;
    if (!c->initialized)
        return !n->phase && !c->primitive_count;
    bool detached = false;
    for (Uint16 row = 0; row < SOTN_ICE_HIT_ROWS; ++row)
    {
        const SotnPlayerEffectQuad *q = &n->quads[row];
        if (q->priority > 511 || q->u[0] > 2 || q->page || q->palette ||
            (q->u[0] == 1 && (!q->v[0] || q->v[0] > 15)) ||
            (q->u[0] < 2 ? (q->draw_mode != 0x315 && q->draw_mode != 0x335)
                         : (q->draw_mode != 0x17 && q->draw_mode != 0x37 && q->draw_mode != 0x1f &&
                            q->draw_mode != 0x3f)))
            return false;
        detached |= q->u[0] == 2;
    }
    return n->phase && c->primitive_count == SOTN_ICE_HIT_ROWS &&
           n->flags == (detached ? 0x08800000U : 0x00860000U) &&
           (n->phase != 2 || (n->timer > 0 && n->timer <= 64));
}
bool SotnBatRuntime_PruneIce(SotnBatRuntime *r)
{
    if (!r)
        return SDL_InvalidParamError("runtime");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (!SotnBatRuntime_IceHitValid(&r->children[i]))
            return SDL_SetError("Ice cleanup lost its actual child/row lifecycle");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (r->children[i].kind == 33 && !(ChildFlags(&r->children[i]) & 0x20000U))
            r->children[i] = (SotnBatRuntimeChild){0};
    PruneRoomFactory(r, 46);
    return true;
}
bool SotnBatRuntime_ThunderHitValid(const SotnBatRuntimeChild *c)
{
    if (c->kind != 30)
        return true;
    const SotnThunderHit *t = &c->state.thunder;
    return c->generation && !c->bat_generation && !c->particle_parent && !c->group &&
           !c->draw_count && !t->removed && t->thickness >= 0 && t->thickness <= 16 &&
           t->sound_count <= 2 &&
           (c->initialized ? t->phase >= 1 && t->phase <= 2 && c->primitive_count == 6 &&
                                 t->parameter == c->parameter && t->speed >= 256 &&
                                 t->speed <= 767 && (t->airborne == 0 || t->airborne == 1) &&
                                 (t->phase == 1 ? t->fade == 0 : t->fade > 0 && t->fade <= 31)
                           : !t->phase && !c->primitive_count);
}
bool SotnBatRuntime_FireHitValid(const SotnBatRuntimeChild *c)
{
    if (c->kind == 5 && (c->parameter >> 8) == 4)
        return c->generation && !c->bat_generation && !c->group && !c->draw_count &&
               c->particle_parent && c->particle_parent_slot < SOTN_BAT_RUNTIME_SLOTS;
    if (c->kind != 16)
        return !c->particle_parent;
    const SotnFireHit *f = &c->state.fire_hit;
    const bool scaled = (c->parameter >> 8) == 1 || (c->parameter & 3U);
    return c->generation && !c->bat_generation && !c->group && !c->draw_count &&
           !c->primitive_count && !c->particle_parent && c->parameter <= 0x2ff && !f->removed &&
           f->pose < 24 && f->remaining >= -1 &&
           (c->initialized
                ? f->phase == 1 && f->parameter == c->parameter && f->remaining != 0 &&
                      f->priority <= 511 && f->bank == (scaled ? 2 : 5) &&
                      f->draw_flags == (scaled ? 3 : 0) && f->velocity_y == -32768 &&
                      (f->blend_mode == 0 || f->blend_mode == 0x10) && f->scale_x == f->scale_y &&
                      (!scaled || (f->scale_x && f->scale_x <= 288))
                : !f->phase);
}
bool SotnBatRuntime_DarkHitValid(const SotnBatRuntimeChild *c)
{
    if (c->kind != 54)
        return true;
    const SotnDarkHit *d = &c->state.dark;
    return c->generation && !c->bat_generation && !c->group && !c->draw_count &&
           !c->primitive_count && !d->removed && d->pose < 13 && d->remaining >= -1 &&
           (c->initialized
                ? d->phase == 1 && d->priority <= 511 && d->remaining != 0 && d->frame &&
                      d->frame <= 511 && d->scale_x >= 64 && d->scale_x <= 272 &&
                      d->scale_y == d->scale_x && !((d->scale_x - 64U) & 7U) &&
                      d->opacity == 319U - d->scale_x && d->velocity_y >= -40959 &&
                      d->velocity_y <= -24576 && (d->blend_mode == 0x10 || d->blend_mode == 0x70)
                : !d->phase);
}
bool SotnBatRuntime_HolyHitValid(const SotnBatRuntimeChild *c)
{
    if (c->kind != 47)
        return true;
    return c->generation && !c->bat_generation && !c->group && !c->draw_count &&
           !c->state.holy.removed &&
           (c->initialized ? c->state.holy.phase == 1 && c->primitive_count == 16
                           : !c->state.holy.phase && !c->primitive_count);
}
bool SotnBatRuntime_PlayerShove(SotnBatRuntime *r, Sint32 x, Sint32 y)
{
    if (!r)
        return SDL_InvalidParamError("runtime");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (!SotnBatRuntime_HolyHitValid(&r->children[i]) ||
            !SotnBatRuntime_IceHitValid(&r->children[i]) ||
            !SotnBatRuntime_CircleValid(&r->children[i]) ||
            !SotnBatRuntime_MedicineValid(&r->children[i]) ||
            !SotnBatRuntime_HandValid(&r->children[i], i) ||
            !SotnBatRuntime_SecondaryValid(&r->children[i], i) ||
            !SotnBatRuntime_MagicMissileValid(&r->children[i], i))
            return SDL_SetError("Player shove lost its actual holy child owner");
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (r->children[i].kind == 47 && r->children[i].initialized &&
            !SotnHolyHit_Displace(&r->children[i].state.holy, x, y))
            return false;
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (r->children[i].kind == 33 && r->children[i].initialized &&
            !SotnIceHit_Displace(&r->children[i].state.ice, x, y))
            return false;
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (r->children[i].kind == 38 && r->children[i].initialized &&
            !SotnPlayerCircle_Displace(&r->children[i].state.circle, x, y))
            return false;
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if (r->children[i].kind == 39 && r->children[i].initialized)
        {
            const Uint32 bx = (Uint32)r->children[i].source.x + (Uint32)x,
                         by = (Uint32)r->children[i].source.y + (Uint32)y;
            SDL_memcpy(&r->children[i].source.x, &bx, sizeof(bx));
            SDL_memcpy(&r->children[i].source.y, &by, sizeof(by));
        }
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
    {
        SotnBatRuntimeChild *c = &r->children[i];
        if (c->hand_weapon && c->initialized && (c->state.hand.motion.flags & 0x40000U))
        {
            const Uint32 bx = (Uint32)c->state.hand.motion.x + (Uint32)x,
                         by = (Uint32)c->state.hand.motion.y + (Uint32)y;
            SDL_memcpy(&c->state.hand.motion.x, &bx, sizeof(bx));
            SDL_memcpy(&c->state.hand.motion.y, &by, sizeof(by));
            c->source.x = c->state.hand.motion.x;
            c->source.y = c->state.hand.motion.y;
        }
        if (c->weapon_secondary && c->initialized && (c->state.secondary.motion.flags & 0x40000U))
        {
            const Uint32 bx = (Uint32)c->state.secondary.motion.x + (Uint32)x,
                         by = (Uint32)c->state.secondary.motion.y + (Uint32)y;
            SDL_memcpy(&c->state.secondary.motion.x, &bx, sizeof(bx));
            SDL_memcpy(&c->state.secondary.motion.y, &by, sizeof(by));
            c->source.x = c->state.secondary.motion.x;
            c->source.y = c->state.secondary.motion.y;
        }
        if (c->magic && c->initialized && (c->state.magic.motion.flags & 0x40000U))
        {
            const Uint32 bx = (Uint32)c->state.magic.motion.x + (Uint32)x,
                         by = (Uint32)c->state.magic.motion.y + (Uint32)y;
            SDL_memcpy(&c->state.magic.motion.x, &bx, sizeof(bx));
            SDL_memcpy(&c->state.magic.motion.y, &by, sizeof(by));
            c->source.x = c->state.magic.motion.x;
            c->source.y = c->state.magic.motion.y;
        }
    }
    return true;
}
bool SotnBatRuntime_HealingNumberValid(const SotnBatRuntimeChild *c, Uint16 index)
{
    if (c->kind != 19)
        return true;
    const SotnHealingNumber *n = &c->state.healing_number;
    return index >= 40 && index < 48 && c->generation && !c->group && !c->draw_count &&
           !n->removed && (n->damage || n->number != 0) && n->extra_digits <= 3 &&
           (c->initialized ? n->phase && n->phase <= 11 && c->primitive_count == 20
                           : !n->phase && !c->primitive_count);
}
bool SotnBatRuntime_SwordChildValid(const SotnBatRuntimeChild *c, Uint16 index)
{
    if (c->hand_weapon)
        return SotnBatRuntime_HandValid(c, index);
    if (c->weapon_secondary)
        return SotnBatRuntime_SecondaryValid(c, index);
    if (c->weapon_warp)
        return SotnBatRuntime_WarpValid(c, index);
    if (!SotnBatRuntime_FoodChildValid(c, index))
        return false;
    if (c->food || c->sire || c->pentagram || c->magic || c->thrown || c->star || c->karma)
        return true;
    if (c->kind == 0xe1 || c->kind == 0xf1)
        return index == 0 && !c->primitive_count && !c->draw_count && !c->group &&
               c->parameter == (c->kind == 0xf1 ? 0x8000 : 0) && !c->state.sword.image.removed &&
               (c->initialized ? c->state.sword.image.phase >= 1 && c->state.sword.image.phase <= 3
                               : !c->state.sword.image.phase);
    if (c->kind == 0xe2 || c->kind == 0xf2)
        return index >= 1 && index <= 15 && !c->primitive_count && !c->draw_count && !c->group &&
               !c->parameter && c->state.sword.parent_generation && !c->bat_generation &&
               !c->state.sword.strike.removed &&
               (c->initialized
                    ? c->state.sword.strike.phase == 1 && c->state.sword.attack.equipment == 197 &&
                          c->state.sword.strike.pose < 20
                    : !c->state.sword.strike.phase);
    return true;
}
bool SotnBatRuntime_FoodChildValid(const SotnBatRuntimeChild *c, Uint16 index)
{
    if (!c->food)
        return c->neutron || c->sire || c->pentagram || c->magic || c->thrown || c->star ||
               c->karma || c->hand_weapon || c->weapon_secondary ||
               (c->kind != 0xe0 && c->kind != 0xf0);
    const SotnFood *f = &c->state.food;
    const bool inventory = c->kind == 0xe1 || c->kind == 0xf1;
    const Uint16 value = (Uint16)((f->parameter >> 8) & 127U);
    return index >= 1 && index <= 15 && c->generation && !c->bat_generation && !c->group &&
           !c->primitive_count && !c->draw_count && !f->removed &&
           (c->kind == 0xe0 || c->kind == 0xf0 || inventory) && f->inventory == inventory &&
           f->parameter == c->parameter && f->priority <= 511 &&
           (inventory ? value >= 29 && value <= 69 && !f->item && f->phase <= 2
                      : value <= 41 && f->item == value + 29U && f->phase <= 6) &&
           (c->initialized ? f->phase != 0 : f->phase == 0);
}
static bool Spawn(SotnBatRuntime *r, SotnPlayerFactoryChild child, Uint32 generation,
                  Uint32 fire_parent, Uint16 fire_parent_slot, bool sire_factory,
                  Uint16 sire_parent_slot, bool magic_factory, Uint16 magic_parent_slot,
                  bool thrown_factory, bool thrown_particles, Uint16 thrown_parent_slot,
                  bool star_factory, Uint16 star_parent_slot, bool karma_factory,
                  Uint16 karma_parent_slot, bool secondary_factory, Uint16 secondary_parent_slot,
                  const SotnBatRuntimeInput *input)
{
    if (child.slot < 16 || child.slot >= 64 || child.replace || !child.entity_id ||
        !ChildKind(child.entity_id) || r->children[child.slot - 16U].kind ||
        r->serial == SDL_MAX_UINT32)
        return SDL_SetError("Bat factory produced an unsupported or occupied actual child slot");
    SotnBatRuntimeChild c = {.source = child.source,
                             .generation = ++r->serial,
                             .bat_generation = generation,
                             .kind = child.entity_id,
                             .parameter = child.parameter};
    if (c.kind == 5 && (c.parameter >> 8) == 4)
    {
        if (!fire_parent || fire_parent_slot >= SOTN_BAT_RUNTIME_SLOTS || generation)
            return SDL_SetError("Fire particles require their actual parent entity identity");
        c.particle_parent = fire_parent;
        c.particle_parent_slot = fire_parent_slot;
    }
    else if (fire_parent)
        return SDL_SetError("Fire descendant factory produced an unrelated child");
    if ((c.kind == 30 || c.kind == 33) && generation)
        return SDL_SetError("Elemental hit requires its independent actual player factory parent");
    if (c.kind == 16 && generation)
        return SDL_SetError("Fire hit requires its independent actual player factory parent");
    // Zero the complete union storage before selecting a member larger than BlinkWhite.
    SDL_zero(c.state);
    c.hand_weapon = child.slot == 16 &&
                    (c.kind == 0xe0 || c.kind == 0xf0 || c.kind == 0xee || c.kind == 0xfe ||
                     ((c.kind == 0xe1 || c.kind == 0xf1) && Weapon51Special(child.argument_low)));
    // Blueprint60 replaces slot16 with weapon53's E2/F2 warp; its factory argument is the item.
    c.weapon_warp = child.slot == 16 && (c.kind == 0xe2 || c.kind == 0xf2) &&
                    (child.argument_low == 193 || child.argument_low == 194);
    if (c.weapon_warp)
    {
        const Uint16 hand = c.kind >= 0xf0 ? 1 : 0;
        if (generation || child.argument_high || secondary_factory ||
            child.parameter !=
                ((child.argument_low == 194 ? 0x100U : 0U) | (hand ? 0x8000U : 0U)) ||
            (input && (!input->weapon_bound[hand] || input->weapon_family[hand] != 53)))
            return SDL_SetError("Weapon53 warp lost its actual slot16/program/item ownership");
        c.state.warp = (SotnWeaponWarp){.x = child.source.x,
                                        .y = child.source.y,
                                        .parameter = child.parameter,
                                        .item = child.argument_low,
                                        .facing_left = child.source.facing_left};
    }
    if (c.hand_weapon)
    {
        const Uint16 family = HandFamily(child.argument_low);
        if (generation || child.argument_high || family > 255 ||
            child.parameter !=
                ((HandVariant(child.argument_low) << 8) | (c.kind >= 0xf0 ? 0x8000 : 0)) ||
            (family == 255) != (c.kind == 0xee || c.kind == 0xfe))
            return SDL_SetError("Ordinary child lost its actual slot16/program/hand ownership");
        c.state.hand.item = child.argument_low;
        c.state.hand.family = family;
        c.state.hand.motion = (SotnHandWeapon){.x = child.source.x,
                                               .y = child.source.y,
                                               .flags = child.source.flags,
                                               .parameter = child.parameter,
                                               .facing_left = child.source.facing_left};
        if (input && family != 255 &&
            (!input->weapon_bound[c.kind >= 0xf0] ||
             input->weapon_family[c.kind >= 0xf0] != family))
            return SDL_SetError("Ordinary child lost its resident native hand program");
    }
    c.neutron = (c.kind == 0xe0 || c.kind == 0xf0) && child.argument_low == 71;
    c.sire_child = sire_factory && (c.kind == 0xe1 || c.kind == 0xf1);
    c.sire = c.sire_child || ((c.kind == 0xe0 || c.kind == 0xf0) && child.argument_low == 72);
    c.pentagram = (c.kind == 0xe0 || c.kind == 0xf0) &&
                  (child.argument_low == 73 || child.argument_low == 74);
    c.magic = ((c.kind == 0xe0 || c.kind == 0xf0) && child.argument_low == 25) ||
              (magic_factory && (c.kind == 0xe1 || c.kind == 0xf1));
    const Uint16 thrown_role = c.kind & 15U;
    c.thrown = ((c.kind >= 0xe0 && c.kind <= 0xe5) || (c.kind >= 0xf0 && c.kind <= 0xf5)) &&
               ((ThrownItem(child.argument_low) &&
                 thrown_role == (child.argument_low == 80                                ? 0
                                 : child.argument_low == 81 || child.argument_low == 134 ? 2
                                 : child.argument_low == 82                              ? 3
                                                                                         : 5)) ||
                (thrown_factory && (thrown_role == 1 || thrown_role == 4)));
    const Uint16 star_role = c.kind & 15U;
    c.star =
        ((c.kind >= 0xe0 && c.kind <= 0xe5) || (c.kind >= 0xf0 && c.kind <= 0xf5)) &&
        ((StarItem(child.argument_low) && star_role == (child.argument_low == 79 ? 3 : 0)) ||
         (star_factory && (star_role == 1 || star_role == 2 || star_role == 4 || star_role == 5)));
    c.karma = ((c.kind == 0xe0 || c.kind == 0xf0) && child.argument_low == 24) ||
              (karma_factory && (c.kind == 0xe2 || c.kind == 0xf2));
    // Blueprint54 replaces slot16 with weapon1's E1/F1 punch rush; blueprint72 creates weapon6's
    // crescents in slots17..31. Their factory argument is the item.
    const bool rush = child.slot == 16 && (c.kind == 0xe1 || c.kind == 0xf1) &&
                      (child.argument_low == 186 || child.argument_low == 187);
    const bool crescent = !secondary_factory && child.slot >= 17 && child.slot <= 31 &&
                          (c.kind == 0xe1 || c.kind == 0xf1) && child.argument_low == 172;
    // Blueprint50 creates Runesword's weapon14 or Heaven sword's weapon13 EntityWeaponAttack in
    // slots17..31.
    const bool rune = !secondary_factory && child.slot >= 17 && child.slot <= 31 &&
                      (c.kind == 0xe0 || c.kind == 0xf0) &&
                      (child.argument_low == 143 || child.argument_low == 119);
    // Weapon12: blueprint50/52 slashes (E0/F0) and blueprint56's Chakram (E1/F1) in17..31.
    const bool vorpal = !secondary_factory && child.slot >= 17 && child.slot <= 31 &&
                        (((c.kind == 0xe0 || c.kind == 0xf0) &&
                          (child.argument_low == 163 || child.argument_low == 164)) ||
                         ((c.kind == 0xe1 || c.kind == 0xf1) && child.argument_low == 133));
    // Blueprint68 creates Yasutsuna's weapon53 E3/F3 slash in17..31.
    const bool yasutsuna = !secondary_factory && child.slot >= 17 && child.slot <= 31 &&
                           (c.kind == 0xe3 || c.kind == 0xf3) && child.argument_low == 165;
    // Blueprint56 also creates Shotel's weapon34 E1/F1 boomerang (QCF special176) in17..31.
    const bool shotel = !secondary_factory && child.slot >= 17 && child.slot <= 31 &&
                        (c.kind == 0xe1 || c.kind == 0xf1) && child.argument_low == 176;
    // Blueprint56 from the QCF177 hand factory creates Sword of Dawn's weapon30 E1/F1 circle.
    const bool dawn = !secondary_factory && child.slot >= 17 && child.slot <= 31 &&
                      (c.kind == 0xe1 || c.kind == 0xf1) && child.argument_low == 177;
    // Blueprint86 from a both-buttons combo creates the shield program's E8/F8 spell in17..31.
    const bool spell = !secondary_factory && child.slot >= 17 && child.slot <= 31 &&
                       (c.kind == 0xe8 || c.kind == 0xf8) &&
                       SotnWeaponSecondary_ShieldSpellFamily(child.argument_low) != 0;
    // Blueprint62 from Heaven sword's both-buttons combo202 creates the E2/F2 sweep in17..31.
    const bool sweep = !secondary_factory && child.slot >= 17 && child.slot <= 31 &&
                       (c.kind == 0xe2 || c.kind == 0xf2) && child.argument_low == 202;
    c.weapon_secondary =
        rush || crescent || rune || vorpal || yasutsuna || shotel || dawn || spell || sweep ||
        (secondary_factory &&
         ((c.kind >= 0xe1 && c.kind <= 0xe5) || (c.kind >= 0xf1 && c.kind <= 0xf5) ||
          c.kind == 0xe9 || c.kind == 0xf9 || c.kind == 0xea || c.kind == 0xfa));
    if (crescent)
    {
        const Uint16 hand = c.kind >= 0xf0 ? 1 : 0;
        if (generation || child.argument_high || (child.parameter & 0x7f00U) ||
            (child.parameter & 0xffU) > 4 || ((child.parameter & 0x8000U) != 0) != (hand != 0) ||
            (input && (!input->weapon_bound[hand] || input->weapon_family[hand] != 6)))
            return SDL_SetError(
                "Weapon6 crescent lost its actual blueprint72/program/item ownership");
        c.state.secondary.family = 6;
        c.state.secondary.parent_slot = 16;
        c.state.secondary.item = 172;
        c.state.secondary.own_item = true;
        c.state.secondary.motion = (SotnWeaponSecondary){.x = child.source.x,
                                                         .y = child.source.y,
                                                         .flags = child.source.flags,
                                                         .parameter = child.parameter,
                                                         .priority = child.source.priority,
                                                         .facing_left = child.source.facing_left};
    }
    if (rune)
    {
        const Uint16 hand = c.kind >= 0xf0 ? 1 : 0;
        const Uint16 family = child.argument_low == 143 ? 14 : 13;
        if (generation || child.argument_high || child.parameter != (hand ? 0x8000U : 0U) ||
            (input && (!input->weapon_bound[hand] || input->weapon_family[hand] != family)))
            return SDL_SetError("Thrown sword lost its actual blueprint50/program/item ownership");
        c.state.secondary.family = family;
        c.state.secondary.parent_slot = 16;
        c.state.secondary.item = child.argument_low;
        c.state.secondary.own_item = true;
        c.state.secondary.motion = (SotnWeaponSecondary){.x = child.source.x,
                                                         .y = child.source.y,
                                                         .flags = child.source.flags,
                                                         .parameter = child.parameter,
                                                         .priority = child.source.priority,
                                                         .facing_left = child.source.facing_left};
    }
    if (yasutsuna)
    {
        const Uint16 hand = c.kind >= 0xf0 ? 1 : 0;
        if (generation || child.argument_high || child.parameter != (hand ? 0x8000U : 0U) ||
            (input && (!input->weapon_bound[hand] || input->weapon_family[hand] != 53)))
            return SDL_SetError("Yasutsuna lost its actual blueprint68/program/item ownership");
        c.state.secondary.family = 53;
        c.state.secondary.parent_slot = 16;
        c.state.secondary.item = 165;
        c.state.secondary.own_item = true;
        c.state.secondary.motion = (SotnWeaponSecondary){.x = child.source.x,
                                                         .y = child.source.y,
                                                         .flags = child.source.flags,
                                                         .parameter = child.parameter,
                                                         .priority = child.source.priority,
                                                         .facing_left = child.source.facing_left};
    }
    if (spell)
    {
        const Uint16 hand = c.kind >= 0xf0 ? 1 : 0;
        if (generation || child.argument_high || child.parameter != (hand ? 0x8000U : 0U) ||
            (input && (!input->weapon_bound[hand] ||
                       input->weapon_family[hand] !=
                           SotnWeaponSecondary_ShieldSpellFamily(child.argument_low))))
            return SDL_SetError("Shield spell lost its actual blueprint86/program/item ownership");
        c.state.secondary.family = SotnWeaponSecondary_ShieldSpellFamily(child.argument_low);
        c.state.secondary.parent_slot = 16;
        c.state.secondary.item = child.argument_low;
        c.state.secondary.own_item = true;
        c.state.secondary.motion = (SotnWeaponSecondary){.x = child.source.x,
                                                         .y = child.source.y,
                                                         .flags = child.source.flags,
                                                         .parameter = child.parameter,
                                                         .priority = child.source.priority,
                                                         .facing_left = child.source.facing_left};
    }
    if (sweep)
    {
        const Uint16 hand = c.kind >= 0xf0 ? 1 : 0;
        if (generation || child.argument_high || child.parameter != (hand ? 0x8000U : 0U) ||
            (input && (!input->weapon_bound[hand] || input->weapon_family[hand] != 13)))
            return SDL_SetError("Heaven sword sweep lost its actual blueprint62/program/item");
        c.state.secondary.family = 13;
        c.state.secondary.parent_slot = 16;
        c.state.secondary.item = 202;
        c.state.secondary.own_item = true;
        c.state.secondary.motion = (SotnWeaponSecondary){.x = child.source.x,
                                                         .y = child.source.y,
                                                         .flags = child.source.flags,
                                                         .parameter = child.parameter,
                                                         .priority = child.source.priority,
                                                         .facing_left = child.source.facing_left};
    }
    if (dawn)
    {
        const Uint16 hand = c.kind >= 0xf0 ? 1 : 0;
        if (generation || child.argument_high || child.parameter != (hand ? 0x8000U : 0U) ||
            (input && (!input->weapon_bound[hand] || input->weapon_family[hand] != 30)))
            return SDL_SetError("Sword of Dawn circle lost its actual blueprint56/program/item");
        c.state.secondary.family = 30;
        c.state.secondary.parent_slot = 16;
        c.state.secondary.item = 177;
        c.state.secondary.own_item = true;
        c.state.secondary.motion = (SotnWeaponSecondary){.x = child.source.x,
                                                         .y = child.source.y,
                                                         .flags = child.source.flags,
                                                         .parameter = child.parameter,
                                                         .priority = child.source.priority,
                                                         .facing_left = child.source.facing_left};
    }
    if (shotel)
    {
        const Uint16 hand = c.kind >= 0xf0 ? 1 : 0;
        if (generation || child.argument_high || child.parameter != (hand ? 0x8000U : 0U) ||
            (input && (!input->weapon_bound[hand] || input->weapon_family[hand] != 34)))
            return SDL_SetError("Shotel lost its actual blueprint56/program/item ownership");
        c.state.secondary.family = 34;
        c.state.secondary.parent_slot = 16;
        c.state.secondary.item = 176;
        c.state.secondary.own_item = true;
        c.state.secondary.motion = (SotnWeaponSecondary){.x = child.source.x,
                                                         .y = child.source.y,
                                                         .flags = child.source.flags,
                                                         .parameter = child.parameter,
                                                         .priority = child.source.priority,
                                                         .facing_left = child.source.facing_left};
    }
    if (vorpal)
    {
        const Uint16 hand = c.kind >= 0xf0 ? 1 : 0;
        const Uint16 variant = child.argument_low == 163 ? 1 : child.argument_low == 164 ? 2 : 0;
        const Uint16 base = (Uint16)((variant << 8) | (hand ? 0x8000U : 0U));
        if (generation || child.argument_high || child.parameter < base ||
            (Uint32)(child.parameter - base) > (child.argument_low == 164 ? 3U : 0U) ||
            (input && (!input->weapon_bound[hand] || input->weapon_family[hand] != 12)))
            return SDL_SetError("Weapon12 root lost its actual blueprint/program/item ownership");
        c.state.secondary.family = 12;
        c.state.secondary.parent_slot = 16;
        c.state.secondary.item = child.argument_low;
        c.state.secondary.own_item = true;
        c.state.secondary.motion = (SotnWeaponSecondary){.x = child.source.x,
                                                         .y = child.source.y,
                                                         .flags = child.source.flags,
                                                         .parameter = child.parameter,
                                                         .priority = child.source.priority,
                                                         .facing_left = child.source.facing_left};
    }
    if (rush)
    {
        const Uint16 hand = c.kind >= 0xf0 ? 1 : 0;
        if (generation || child.argument_high || secondary_factory ||
            child.parameter !=
                ((child.argument_low == 187 ? 0x100U : 0U) | (hand ? 0x8000U : 0U)) ||
            (input && (!input->weapon_bound[hand] || input->weapon_family[hand] != 1)))
            return SDL_SetError("Weapon1 punch rush lost its actual slot16/program/item ownership");
        c.state.secondary.family = 1;
        c.state.secondary.parent_slot = 16;
        c.state.secondary.item = child.argument_low;
        c.state.secondary.own_item = true;
        c.state.secondary.motion = (SotnWeaponSecondary){.x = child.source.x,
                                                         .y = child.source.y,
                                                         .flags = child.source.flags,
                                                         .parameter = child.parameter,
                                                         .priority = child.source.priority,
                                                         .facing_left = child.source.facing_left};
    }
    if (secondary_factory)
    {
        const Uint16 hand = c.kind >= 0xf0 ? 1 : 0;
        const Uint16 family = input && input->weapon_bound[hand] ? input->weapon_family[hand] : 256;
        if (!c.weapon_secondary || generation || c.thrown || c.star || c.karma || c.magic ||
            c.sire || child.slot < 17 || child.slot > 63 ||
            !SotnWeaponSecondary_SupportedRole(family, c.kind & 15U) || child.argument_low ||
            child.argument_high || !SecondaryParameter(family, child.parameter) ||
            secondary_parent_slot < 16 || secondary_parent_slot > 47 ||
            !SotnWeaponSecondary_Supported(family))
            return SDL_SetError("Weapon secondary lost its actual blueprint56/slot/program owner");
        c.state.secondary.family = family;
        c.state.secondary.parent_slot = secondary_parent_slot;
        c.state.secondary.motion = (SotnWeaponSecondary){.x = child.source.x,
                                                         .y = child.source.y,
                                                         .flags = child.source.flags,
                                                         .parameter = child.parameter,
                                                         .priority = child.source.priority,
                                                         .facing_left = child.source.facing_left};
    }
    if (c.karma)
    {
        const bool auxiliary = (c.kind & 15U) == 2;
        if (generation || child.argument_high ||
            (auxiliary ? child.slot < 32 || child.slot > 47 || child.argument_low ||
                             child.parameter > 3 || karma_parent_slot < 17 || karma_parent_slot > 31
                       : child.slot < 17 || child.slot > 31 ||
                             child.parameter != (c.kind == 0xf0 ? 0x8000 : 0)))
            return SDL_SetError("Karma child lost actual role/item/parameter/raw-parent ownership");
        c.state.karma.item = child.argument_low;
        c.state.karma.parent_slot = auxiliary ? karma_parent_slot : 0;
        c.state.karma.motion = (SotnKarmaCoin){.x = child.source.x,
                                               .y = child.source.y,
                                               .flags = child.source.flags,
                                               .priority = child.source.priority,
                                               .facing_left = child.source.facing_left};
    }
    if (c.star)
    {
        const Uint16 index = (Uint16)(child.slot - 16U);
        if (generation || child.argument_high ||
            (star_role == 1   ? index < 16 || index > 47
             : star_role == 2 ? index < 9 || index > 15
             : star_role == 4 ? index < 1 || index > 47
             : star_role == 5 ? index < 1 || index > 31
                              : index < 1 || index > 15) ||
            (star_factory
                 ? (child.argument_low || star_parent_slot < 17 || star_parent_slot > 31 ||
                    ((star_role == 1 || star_role == 2 || star_role == 5) && child.parameter) ||
                    (star_role == 4 && (child.parameter >> 8) > 23))
                 : child.parameter !=
                       ((c.kind >= 0xf0 ? 0x8000U : 0U) |
                        (child.argument_low == 79 ? 0U : (child.argument_low - 75U) << 8))))
            return SDL_SetError("Weapon15 child lost actual role/item/parameter/raw parent");
        c.state.star.parent_slot = star_factory ? star_parent_slot : 0;
        c.state.star.motion = (SotnStarWeapon){.item = child.argument_low,
                                               .motion = {.x = child.source.x,
                                                          .y = child.source.y,
                                                          .flags = child.source.flags,
                                                          .parameter = child.parameter,
                                                          .priority = child.source.priority,
                                                          .facing_left = child.source.facing_left}};
    }
    if (c.thrown)
    {
        if (generation || child.argument_high ||
            (thrown_role == 1   ? child.slot < 32 || child.slot > 63
             : thrown_role == 4 ? child.slot < 25 || child.slot > 31
                                : child.slot < 17 || child.slot > 31) ||
            (thrown_factory ? child.argument_low || child.parameter || thrown_parent_slot < 17 ||
                                  thrown_parent_slot > 31
                            : child.parameter != ((c.kind >= 0xf0 ? 0x8000U : 0U) |
                                                  (child.argument_low == 134 ? 0x100U : 0U))))
            return SDL_SetError("Weapon16 child lost its actual role/item/parameter/raw parent");
        c.state.thrown.item = child.argument_low;
        c.state.thrown.parent_slot = thrown_factory ? thrown_parent_slot : 0;
        c.state.thrown.motion = (SotnThrownWeapon){.x = child.source.x,
                                                   .y = child.source.y,
                                                   .flags = child.source.flags,
                                                   .parameter = child.parameter,
                                                   .priority = child.source.priority,
                                                   .facing_left = child.source.facing_left};
    }
    if (thrown_particles)
    {
        if (child.entity_id != 5 || child.parameter >> 8 != 11 || generation || fire_parent ||
            thrown_parent_slot < 17 || thrown_parent_slot > 31)
            return SDL_SetError("Weapon16 particles lost their actual common role/raw parent");
        c.thrown_particle_parent = true;
        c.particle_parent_slot = thrown_parent_slot;
    }
    c.food =
        ((c.kind == 0xe0 || c.kind == 0xf0) && !c.neutron && !c.sire && !c.pentagram && !c.magic &&
         !c.thrown && !c.star && !c.karma && !c.hand_weapon && !c.weapon_secondary) ||
        (!c.weapon_secondary && (c.kind == 0xe1 || c.kind == 0xf1) && child.slot >= 17 &&
         child.slot <= 31 && ((child.parameter >> 8) & 127U) >= 29 &&
         ((child.parameter >> 8) & 127U) <= 69);
    if ((c.food || c.neutron || c.sire || c.pentagram || c.magic || c.thrown || c.star ||
         c.karma) &&
        input)
    {
        const Uint16 hand = c.kind >= 0xf0 ? 1 : 0;
        if (input->weapon_bound[hand] && input->weapon_family[hand] != (c.karma       ? 20
                                                                        : c.star      ? 15
                                                                        : c.thrown    ? 16
                                                                        : c.magic     ? 22
                                                                        : c.pentagram ? 17
                                                                        : c.sire      ? 18
                                                                        : c.neutron   ? 19
                                                                                      : 21))
            return SDL_SetError("Hand child lost its actual resident weapon program");
    }
    if (c.neutron)
    {
        if (generation || child.slot < 17 || child.slot > 31 || child.argument_high ||
            child.parameter != (c.kind == 0xf0 ? 0x8000 : 0))
            return SDL_SetError("Neutron child lost its original slot/hand/item71 identity");
        c.state.neutron.motion.x = child.source.x;
        c.state.neutron.motion.y = child.source.y;
    }
    if (c.sire)
    {
        if (generation || child.slot < 17 || child.slot > 31 || child.argument_high ||
            child.argument_low != (c.sire_child ? 0 : 72) ||
            child.parameter != (!c.sire_child && c.kind >= 0xf0 ? 0x8000 : 0) ||
            (c.sire_child && (sire_parent_slot < 17 || sire_parent_slot > 31)))
            return SDL_SetError("Sire child lost its actual hand/slot/parent identity");
        c.state.sire.parent_slot = sire_parent_slot;
        if (c.sire_child)
        {
            c.state.sire.child.x = child.source.x;
            c.state.sire.child.y = child.source.y;
        }
        else
        {
            c.state.sire.parent.x = child.source.x;
            c.state.sire.parent.y = child.source.y;
        }
    }
    if (c.pentagram)
    {
        if (generation || child.slot < 17 || child.slot > 31 || child.argument_high ||
            child.parameter !=
                (child.argument_low == 74 ? 0x100U : 0U) + (c.kind == 0xf0 ? 0x8000U : 0U))
            return SDL_SetError("Pentagram child lost its actual hand/slot/item identity");
        c.state.pentagram.item = child.argument_low;
        c.state.pentagram.motion = (SotnPentagram){.x = child.source.x,
                                                   .y = child.source.y,
                                                   .flags = child.source.flags,
                                                   .parameter = child.parameter};
    }
    if (c.magic)
    {
        const bool arrow = c.kind == 0xe1 || c.kind == 0xf1;
        if (generation || child.slot < 17 || child.slot > 31 || child.argument_high ||
            child.argument_low != (arrow ? 0 : 25) ||
            child.parameter != (arrow            ? 0
                                : c.kind == 0xf0 ? 0x8000U
                                                 : 0U) ||
            (arrow && (magic_parent_slot < 17 || magic_parent_slot > 31)))
            return SDL_SetError("Magic Missile lost its actual slot/role/raw parent identity");
        c.state.magic.parent_slot = arrow ? magic_parent_slot : 0;
        c.state.magic.motion = (SotnMagicMissile){.x = child.source.x,
                                                  .y = child.source.y,
                                                  .flags = child.source.flags,
                                                  .priority = child.source.priority,
                                                  .facing_left = child.source.facing_left,
                                                  .item = arrow ? 0 : 25};
    }
    if (c.food)
    {
        const bool inventory = c.kind == 0xe1 || c.kind == 0xf1;
        if (generation || child.slot < 17 || child.slot > 31 || child.argument_high ||
            (inventory ? child.argument_low != 0 || (child.parameter & 0x80ffU) != 0
                       : child.argument_low < 29 || child.argument_low > 70 ||
                             child.parameter != (child.argument_low - 29U) * 256U +
                                                    (c.kind == 0xf0 ? 0x8000U : 0U)))
            return SDL_SetError("Food child lost its actual weapon21 role, slot or item argument");
        c.state.food = (SotnFood){.x = child.source.x,
                                  .y = child.source.y,
                                  .flags = child.source.flags,
                                  .parameter = child.parameter,
                                  .item = child.argument_low,
                                  .priority = child.source.priority,
                                  .facing_left = child.source.facing_left,
                                  .inventory = inventory};
    }
    if (!c.thrown && !c.star && !c.karma && !c.weapon_secondary && !c.weapon_warp &&
        (c.kind == 0xe2 || c.kind == 0xf2))
    {
        if (child.slot < 17 || child.slot > 31 || !generation || child.parameter ||
            child.argument_low || child.argument_high)
            return SDL_SetError("Sword strike lost its actual range, parameter or parent");
        c.bat_generation = 0;
        c.state.sword.parent_generation = generation;
    }
    if (c.kind == 37 &&
        ((child.parameter > 0x300 && (child.parameter < 0x800 || child.parameter > 0x1300)) ||
         (child.parameter & 255U) || generation))
        return SDL_SetError("Text requires actual immunity/poison/curse/stone parameter and no "
                            "transformation parent");
    if (c.kind == 54 && generation)
        return SDL_SetError("Dark hit requires its independent actual factory parent");
    if (c.kind == 47 && generation)
        return SDL_SetError("Holy hit requires its independent actual factory parent");
    if (c.kind == 32)
    {
        if (generation || child.parameter)
            return SDL_SetError("Dissolve requires its actual independent zero-parameter factory");
        c.state.dissolve.x = child.source.x;
        c.state.dissolve.y = child.source.y;
    }
    if (c.kind == 39)
    {
        if (generation || ((child.parameter >> 8) & 127U) >= 20)
            return SDL_SetError("Medicine child lost its actual independent private profile");
        c.state.medicine.parameter = child.parameter;
    }
    if (c.kind == 38)
    {
        if (generation || ((child.parameter >> 8) & 63U) >= 22)
            return SDL_SetError("Player circle requires its independent actual private profile");
        c.state.circle = (SotnPlayerCircle){
            .x = child.source.x, .y = child.source.y, .parameter = child.parameter};
    }
    if (c.kind == 65)
    {
        if (generation || (child.parameter & 255U))
            return SDL_SetError("Teleport requires its actual independent high-parameter factory");
        c.state.teleport = (SotnTeleportEffect){
            .parameter = child.parameter, .x = child.source.x, .y = child.source.y};
    }
    if (c.kind == 24)
        c.state.hellfire_trail.motion =
            (SotnHellfireTrail){.x = child.source.x,
                                .y = child.source.y,
                                .parameter = child.parameter,
                                .facing_left = child.source.facing_left};
    if (c.kind == 25)
        c.state.hellfire_beam = (SotnHellfireBeam){.x = child.source.x, .y = child.source.y};
    if (c.kind == 26 || c.kind == 27)
        c.state.hellfire_fire.motion = (SotnHellfireFireball){.x = child.source.x,
                                                              .y = child.source.y,
                                                              .parameter = child.parameter,
                                                              .big = c.kind == 27};
    if (c.kind == 28 || c.kind == 29)
        c.state.hellfire_effect =
            (SotnHellfireEffect){.x = child.source.x, .y = child.source.y, .kind = c.kind};
    if (c.kind == 11)
        c.state.blink.parameter = (child.parameter >> 8) & 0x7fU;
    if (c.kind == 46 && (child.slot < 32 || generation || child.parameter > 1 ||
                         child.argument_low || child.argument_high))
        return SDL_SetError("Talisman lost its actual range/parameter and independent factory");
    if (c.kind == 46)
        c.state.talisman = (SotnTalismanEffect){
            .x = child.source.x, .y = child.source.y, .facing_left = child.source.facing_left};
    if (c.kind == 31)
        c.state.outline.parameter = child.parameter;
    if (!c.food && !c.sire && !c.magic && !c.thrown && !c.star && !c.weapon_secondary &&
        !c.hand_weapon && (c.kind == 0xe1 || c.kind == 0xf1))
    {
        if (child.slot != 16 || child.argument_low != 197 || child.argument_high ||
            child.parameter != (c.kind == 0xf1 ? 0x8000 : 0))
            return SDL_SetError("Sword image lost its actual slot16 and virtual equipment");
        c.state.sword.image = (SotnSwordWarp){
            .x = child.source.x, .y = child.source.y, .facing_left = child.source.facing_left};
    }
    if (c.kind == 35)
        c.state.trail = (SotnBatTrail){.x = child.source.x, .y = child.source.y};
    if (c.kind == 45)
        c.state.fire.motion = (SotnBatFireball){.x = child.source.x, .y = child.source.y};
    if (c.kind == 61)
        c.state.spirit.motion.parameter = child.parameter;
    if (c.kind == 63)
        c.state.portal.parameter = child.parameter;
    if (c.kind == 52)
    {
        if (child.slot != 47 || r->echo_generation)
            return SDL_SetError("Bat Echo lost its original single-child slot ownership");
        r->echo = (SotnBatEcho){.facing_left = child.source.facing_left};
    }
    r->children[child.slot - 16U] = c;
    return true;
}
static bool RequestNumber(SotnBatRuntime *r, Uint64 other_occupied, SotnPlayerFactorySource source,
                          Uint16 amount, bool damage)
{
    const Uint64 occupied = SotnBatRuntime_Occupied(r) | other_occupied;
    for (Uint16 slot = 56; slot < 64; ++slot)
        if (!(occupied & ((Uint64)1 << slot)))
        {
            if (!Spawn(r, (SotnPlayerFactoryChild){.slot = slot, .entity_id = 19, .source = source},
                       0, 0, 0, false, 0, false, 0, false, false, 0, false, 0, false, 0, false, 0,
                       NULL))
                return false;
            r->children[slot - 16U].state.healing_number.number = amount;
            r->children[slot - 16U].state.healing_number.damage = damage;
            break;
        }
    return true;
}
static bool Duplicate(const SotnBatRuntime *r, Uint16 index)
{
    const SotnBatRuntimeChild c = r->children[index];
    for (Uint16 i = 0; i < 48; ++i)
        if (i != index && r->children[i].kind == 11 && r->children[i].parameter == c.parameter)
            return true;
    return false;
}
static bool LoopAnimation(SotnDataView dra, Uint16 *cursor, Uint16 *duration, Uint16 *frame)
{
    Uint16 pose = *cursor, remaining = *duration;
    if (pose >= 9 || remaining > 32767 || !dra.bytes || dra.size < 0x10798U + 9U * 4U)
        return SDL_SetError("Bat fireball requires its bounded private animation stream");
    if (remaining && --remaining == 0)
        ++pose;
    if (pose >= 9)
        return SDL_SetError("Bat fireball animation lacks its bounded loop");
    const Uint8 *record = dra.bytes + 0x10798U + pose * 4U;
    if (!remaining)
    {
        if (!Read16(record))
        {
            pose = Read16(record + 2);
            if (pose >= 8)
                return SDL_SetError("Bat fireball animation has invalid loop target");
            record = dra.bytes + 0x10798U + pose * 4U;
        }
        remaining = Read16(record);
    }
    if (!remaining || remaining > 32767 || (Read16(record + 2) & 0xfe00U))
        return SDL_SetError("Bat fireball animation has invalid duration/frame metadata");
    *cursor = pose;
    *duration = remaining;
    *frame = Read16(record + 2);
    return true;
}
static bool HellfireAnimation(SotnDataView dra, Uint32 offset, Uint16 limit, Uint16 *pose,
                              Sint16 *remaining, Uint16 *frame)
{
    if (!dra.bytes || offset > dra.size || (size_t)limit * 4U > dra.size - offset ||
        *pose >= limit || *remaining < -1)
        return SDL_SetError("Hellfire child lost its bounded private animation cursor");
    Uint16 p = *pose;
    Sint16 t = *remaining;
    if (t == 0)
    {
        const Uint16 duration = Read16(dra.bytes + offset + p * 4U);
        if (!duration || duration > 32767)
            return SDL_SetError("Hellfire child animation has an invalid birth duration");
        t = (Sint16)duration;
    }
    else if (t > 0 && --t == 0)
    {
        if (++p >= limit)
            return SDL_SetError("Hellfire child animation escapes its actual timeline");
        Uint16 duration = Read16(dra.bytes + offset + p * 4U);
        if (duration == 0)
        {
            p = Read16(dra.bytes + offset + p * 4U + 2U);
            if (p >= limit)
                return SDL_SetError("Hellfire child animation has an invalid loop target");
            duration = Read16(dra.bytes + offset + p * 4U);
        }
        else if (duration == 0xffffU)
        {
            --p;
            t = -1;
        }
        if (t != -1)
        {
            if (!duration || duration > 32767)
                return SDL_SetError("Hellfire child animation has an unsupported duration/control");
            t = (Sint16)duration;
        }
    }
    if (!t || t < -1)
        return SDL_SetError("Hellfire child animation resolved an invalid birth duration");
    *pose = p;
    *remaining = t;
    *frame = Read16(dra.bytes + offset + p * 4U + 2U) & 511U;
    return true;
}
bool SotnBatRuntime_InitCommon(SotnBatRuntime *runtime, SotnPrimitiveSlots slots)
{
    if (!runtime || runtime->serial || runtime->wolf.serial || runtime->wolf_bound ||
        runtime->common_generation || runtime->wolf.environment_reserved ||
        runtime->wolf.generation || SotnBatRuntime_Occupied(runtime))
        return SDL_SetError("Common player initialization requires its fresh stage lifetime");
    Uint16 rows[8] = {0};
    bool allocated = false;
    if (!SotnPrimitiveSlots_Reserve(&slots, 8, rows, &allocated))
        return false;
    if (!allocated)
        return SDL_SetError("Common player initialization requires eight free primitive rows");
    SotnBatRuntime next = *runtime;
    next.common_generation = next.serial = 1;
    SDL_memcpy(next.afterimage_rows, rows, sizeof(next.afterimage_rows));
    next.wolf.environment_rows[0] = rows[6];
    next.wolf.environment_rows[1] = rows[7];
    next.wolf.environment_reserved = true;
    next.wolf.environment_generation = next.wolf.serial = 1;
    next.wolf_bound = true;
    *runtime = next;
    return true;
}
static bool WolfRows(SotnPrimitiveSlots *slots, const SotnWolfRuntime *before,
                     const SotnWolfRuntime *after)
{
    if (before->root_reserved && !after->root_reserved &&
        !SotnPrimitiveSlots_Release(slots, 6, before->root_rows))
        return false;
    if (after->root_reserved)
        for (Uint16 i = 0; i < 6; ++i)
            slots->occupied[after->root_rows[i] / 64U] |= (Uint64)1 << (after->root_rows[i] % 64U);
    if (after->environment_reserved)
        for (Uint16 i = 0; i < 2; ++i)
            slots->occupied[after->environment_rows[i] / 64U] |=
                (Uint64)1 << (after->environment_rows[i] % 64U);
    return true;
}
static bool TickWolf(SotnBatRuntime *r, SotnCharacterState *profile, SotnCharacterStats stats,
                     SotnPlayerState *p, SotnBatRuntimeInput *in)
{
    const SotnWolfRuntime before = r->wolf;
    SotnWolfControllerFrame frame = p->wolf.controller;
    frame.x = FixedShift(p->x, (Sint16)-in->camera_x);
    frame.y = FixedShift(p->y, (Sint16)-in->camera_y);
    frame.mp = p->mp;
    frame.priority = p->priority;
    const bool active = p->step == SOTN_PLAYER_WOLF && r->wolf.generation;
    if (active && (r->wolf.generation != p->wolf.body_generation || profile->mp != p->mp))
        return SDL_SetError("Shared Wolf body lost its actual lineage or current MP owner");
    if (!SotnWolfRuntime_Tick(
            &r->wolf, &frame,
            (SotnWolfRuntimeInput){.dra = in->dra,
                                   .executable = in->executable,
                                   .primitive_slots = in->primitive_slots,
                                   .query = in->wolf_query,
                                   .context = in->wolf_context,
                                   .other_occupied = (SotnBatRuntime_Occupied(r) &
                                                      ~SotnWolfRuntime_Occupied(&r->wolf)) |
                                                     in->other_occupied,
                                   .palette = p->wolf.palette,
                                   .active = active,
                                   .attack_requested = p->wolf.commands.attack_requested},
            &r->wolf_events) ||
        !WolfRows(&in->primitive_slots, &before, &r->wolf))
        return false;
    r->wolf_frame = frame;
    r->wolf_mask = 0;
    if (!active)
        return true;
    if (r->wolf_events.sound && !Sound(r, r->wolf_events.sound))
        return false;
    if (p->mp < frame.mp || p->mp - frame.mp != r->wolf_events.mp_spent)
        return SDL_SetError("Shared Wolf body MP report differs from its actual player snapshot");
    profile->mp = frame.mp;
    // GetSpellDef uses INT/RNG/Mojo, not MP. Body updates consume no RNG themselves;
    // preserve their front-before-head requests before any later actual entity advances.
    for (Uint16 i = 0; i < r->wolf_events.spell_count; ++i)
    {
        const SotnWolfRuntimeSpell request = r->wolf_events.spells[i];
        const Uint16 part = request.slot == 18 ? 1 : 3;
        const Uint16 attack = part == 1 ? 0 : 1;
        if ((request.slot != 18 && request.slot != 20) ||
            request.generation != r->wolf.children[part].generation ||
            !((request.slot == 18 && request.spell == 13) ||
              (request.slot == 20 && (request.spell == 4 || request.spell == 14))))
            return SDL_SetError("Wolf attack lost its actual child or spell identity");
        if (!Spell(in->dra, profile, stats, request.spell, &r->wolf_attacks[attack]))
            return false;
        Uint8 group = 3;
        for (Uint8 j = 4; j < 7; ++j)
            if (r->groups[j] < r->groups[group])
                group = j;
        if (r->groups[group] == 255)
            return SDL_SetError("Wolf spell attack group overflow");
        ++r->groups[group];
        // Original Wolf overrides the newly selected enemyId after incrementing its count.
        SotnWolfPart *body = &r->wolf.body.parts[part];
        body->enemy_id = 3;
        body->hitbox_state = r->wolf_attacks[attack].hit_type;
        r->wolf_attacks[attack].offset_x = body->hitbox_x;
        r->wolf_attacks[attack].offset_y = body->hitbox_y;
        r->wolf_attacks[attack].width = body->width;
        r->wolf_attacks[attack].height = body->height;
        r->wolf_mask |= (Uint8)(1U << attack);
    }
    if (r->wolf_events.bubble)
    {
        SotnPlayerFactorySource source = r->wolf_events.bubble_source;
        source.x = FixedShift(source.x, (Sint16)in->camera_x);
        source.y = FixedShift(source.y, (Sint16)in->camera_y);
        if (!RequestSource(r, source, 0xd0004, 0))
            return false;
    }
    p->mp = frame.mp;
    p->wolf.controller = frame;
    p->velocity_x = frame.motion.velocity_x;
    p->velocity_y = frame.motion.velocity_y;
    p->animation = frame.motion.animation;
    p->substep = frame.motion.substep;
    p->wolf.palette = r->wolf.root.player_palette;
    p->wolf.draw_flags = r->wolf.root.player_draw_flags;
    p->wolf.blend = r->wolf.root.player_blend;
    p->wolf.opacity = r->wolf.root.player_opacity;
    p->wolf.hidden = p->wolf.palette == 0x810d;
    return true;
}
bool SotnBatRuntime_Tick(SotnBatRuntime *runtime, SotnCharacter *character, SotnBatRuntimeInput in)
{
    if (!runtime || !character || (in.freeze != 0 && in.freeze != 1 && in.freeze != 3) ||
        in.camera_x < 0 || in.camera_x > 16384 || in.camera_y < 0 || in.camera_y > 16384 ||
        in.wolf_command_camera_x < 0 || in.wolf_command_camera_x > 16384 ||
        in.wolf_command_camera_y < 0 || in.wolf_command_camera_y > 16384)
        return SDL_SetError(
            "Bat runtime requires actual owned progress and bounded camera context");
    if (((in.player.wolf.started || in.player.wolf.return_started || in.player.wolf.finish_flash) &&
         !in.player.wolf.generation) ||
        ((in.player.bat.started || in.player.bat.return_started) && !in.player.bat.generation))
        return SDL_SetError("Bat transformation requires a nonzero actual owner generation");
    if (SotnBatRuntime_Occupied(runtime) & in.other_occupied)
        return SDL_SetError("Bat runtime overlaps another actual ability owner");
    SotnBatRuntime r = *runtime;
    r.food_text = in.food_text ? *in.food_text : (SotnFoodTextPlan){0};
    r.food_event_count = 0;
    if (r.wolf_bound && !in.wolf_owned)
        return SDL_SetError("Shared ability owner cannot detach its actual Wolf stage lifetime");
    r.wolf_bound = in.wolf_owned;
    if (in.entering && (!SotnBatRuntime_PruneIce(&r) || !SotnBatRuntime_PruneDissolve(&r) ||
                        !SotnBatRuntime_PruneTeleport(&r) || !SotnBatRuntime_PruneMedicines(&r) ||
                        !SotnBatRuntime_PruneFood(&r) || !SotnBatRuntime_PruneNeutron(&r) ||
                        !SotnBatRuntime_PruneSire(&r) || !SotnBatRuntime_PrunePentagram(&r) ||
                        !SotnBatRuntime_PruneMagicMissile(&r) || !SotnBatRuntime_PruneThrown(&r) ||
                        !SotnBatRuntime_PruneStar(&r) || !SotnBatRuntime_PruneKarma(&r)))
        return false;
    if (r.common_generation &&
        (r.common_generation != r.wolf.environment_generation || !r.wolf.environment_reserved))
        return SDL_SetError("Common player update lost its retained eight-row lease");
    SotnPrimitiveSlots retained = {0};
    if ((r.common_generation &&
         !RetainRows(&in.primitive_slots, &retained, r.afterimage_rows, 6)) ||
        (r.wolf.root_reserved &&
         !RetainRows(&in.primitive_slots, &retained, r.wolf.root_rows, 6)) ||
        (r.wolf.environment_reserved &&
         !RetainRows(&in.primitive_slots, &retained, r.wolf.environment_rows, 2)))
        return false;
    for (Uint16 i = 0; i < 48; ++i)
    {
        const SotnBatRuntimeChild *c = &r.children[i];
        if ((c->kind == 45 &&
             (i < 16 || i >= 32 || (c->initialized && (c->group < 7 || c->group > 10)))) ||
            (c->kind && (!c->generation || !ChildKind(c->kind))) ||
            (c->kind == 52 &&
             (i != 31 || (c->initialized && (r.echo_generation != c->generation ||
                                             (c->group && (c->group < 7 || c->group > 10)))))) ||
            (c->kind == 36 &&
             (i < 1 || i > 15 || (c->initialized && (c->group < 3 || c->group > 6)))))
            return SDL_SetError("Bat runtime has invalid actual child ownership or attack group");
        if ((c->kind == 50 && (i != 0 || c->parameter || c->primitive_count || c->draw_count ||
                               (c->initialized ? c->group != 1 : c->group != 0))) ||
            (c->kind == 49 && (c->parameter || c->primitive_count || c->draw_count || c->group)))
            return SDL_SetError("Axe Armor lost its actual slot16 attack or rowless shadow");
        if (c->kind == 53 &&
            (i != 31 || c->primitive_count ||
             (c->initialized ? (c->group < 7 || c->group > 10 || c->state.attack.hit_type != 0x82 ||
                                c->state.attack.hit_effect != 2)
                             : c->group != 0)))
            return SDL_SetError(
                "Soul Steal attack lost its actual slot47/group/metadata ownership");
        if (!SotnBatRuntime_HellfireChildValid(c, i))
            return SDL_SetError(
                "Hellfire child lost its actual generation/role/row/group lifecycle");
        if (!SotnBatRuntime_ImmunityTextValid(c))
            return SDL_SetError("Immunity text lost its actual child/profile/row ownership");
        if (!SotnBatRuntime_DissolveValid(c))
            return SDL_SetError("Dissolve lost its actual thirty-six-row child ownership");
        if (!SotnBatRuntime_MedicineValid(c))
            return SDL_SetError("Medicine lost its actual rowless child/cursor ownership");
        if (!SotnBatRuntime_CircleValid(c))
            return SDL_SetError("Player circle lost its actual thirty-two-row child ownership");
        if (!SotnBatRuntime_TeleportValid(c))
            return SDL_SetError("Teleport lost its actual thirty-six-row child ownership");
        if (!SotnBatRuntime_IceHitValid(c))
            return SDL_SetError("Ice hit lost its actual twenty-four-row child ownership");
        if (!SotnBatRuntime_ThunderHitValid(c))
            return SDL_SetError("Thunder hit lost its actual six-row child ownership");
        if (!SotnBatRuntime_FireHitValid(c))
            return SDL_SetError("Fire hit/particles lost their actual child/parent ownership");
        if (!SotnBatRuntime_DarkHitValid(c))
            return SDL_SetError("Dark hit lost its actual rowless/global/animation ownership");
        if (!SotnBatRuntime_HolyHitValid(c))
            return SDL_SetError("Holy hit lost its actual child/global/row ownership");
        if (!SotnBatRuntime_TalismanValid(c, i))
            return SDL_SetError("Talisman lost its actual child/lifetime/rows");
        if (!SotnBatRuntime_HealingNumberValid(c, i))
            return SDL_SetError("Healing number lost its actual child/lifetime/row ownership");
        if (!SotnBatRuntime_SwordChildValid(c, i))
            return SDL_SetError("Sword special lost its actual slot/role/lifetime ownership");
        if (!SotnBatRuntime_PentagramValid(c, i))
            return SDL_SetError("Pentagram lost its actual slot/role/lifetime ownership");
        if (!SotnBatRuntime_KarmaValid(c, i))
            return SDL_SetError("Karma lost actual callback/row/raw-parent ownership");
        if (!SotnBatRuntime_SecondaryValid(c, i))
            return SDL_SetError("Weapon secondary lost its actual program/row/group ownership");
        if (!SotnBatRuntime_StarValid(c, i))
            return SDL_SetError("Weapon15 lost actual role/row/callback ownership");
        if (!SotnBatRuntime_ThrownValid(c, i))
            return SDL_SetError("Weapon16 lost its actual role/row/callback ownership");
        if (!SotnBatRuntime_MagicMissileValid(c, i))
            return SDL_SetError("Magic Missile lost its actual raw parent/callback ownership");
        if (!SotnBatRuntime_SireValid(c, i))
            return SDL_SetError("Sire lost its actual slot/role/lifetime ownership");
        if (!SotnBatRuntime_NeutronValid(c, i))
            return SDL_SetError("Neutron lost its actual slot/role/lifetime ownership");
        if (c->kind == 51 &&
            (c->primitive_count || c->draw_count || c->state.heart.removed ||
             c->initialized != c->state.heart.initialized ||
             (c->initialized && (!c->state.heart.timer || c->state.heart.timer > 128 ||
                                 c->state.heart.priority > 509))))
            return SDL_SetError("Rising heart lost its actual child/lifetime ownership");
        if (!SpiritRows(c) ||
            (c->kind == 61 &&
             (i < 1 || i > 31 || (c->initialized && (i < 16 ? c->group > 6 : c->group < 7)))))
            return SDL_SetError("Spirit child lost its actual motion/row/target/group ownership");
        if (c->kind == 31 &&
            (c->state.outline.parameter != c->parameter || c->state.outline.removed ||
             (c->initialized ? !c->state.outline.phase || c->state.outline.phase > 3
                             : c->state.outline.phase != 0)))
            return SDL_SetError("Player outline lost its actual child/profile lifecycle");
        if (c->primitive_count > SDL_arraysize(c->primitives) ||
            (c->primitive_count &&
             (!c->initialized ||
              !(((c->kind == 11 || c->kind == 46) && c->primitive_count == 8) ||
                ((c->kind == 25 || c->kind == 28 || c->kind == 29 || c->kind == 31 ||
                  c->kind == 35) &&
                 c->primitive_count == 1) ||
                (c->neutron && c->primitive_count == 1) ||
                (c->thrown && (c->kind & 15U) == 0 && c->primitive_count == 1) ||
                (c->star && (c->kind & 15U) == 3 && c->primitive_count == 32) ||
                (c->karma && (c->kind & 15U) == 0 && c->primitive_count == SOTN_KARMA_ROWS) ||
                (c->hand_weapon && HandRows(c->state.hand.family) &&
                 c->primitive_count == HandRows(c->state.hand.family)) ||
                (c->weapon_secondary &&
                 c->primitive_count ==
                     SotnWeaponSecondary_HeldRows(c->state.secondary.family, c->kind & 15U,
                                                  c->parameter, c->state.secondary.motion.phase)) ||
                (c->pentagram && c->primitive_count == SOTN_PENTAGRAM_ROWS) ||
                (c->kind == 61 && c->primitive_count == 16) ||
                (c->kind == 63 && c->primitive_count == 9) ||
                (c->kind == 19 && c->primitive_count == 20) ||
                (c->kind == 37 && c->primitive_count == 18) ||
                (c->kind == 47 && c->primitive_count == 16) ||
                (c->kind == 30 && c->primitive_count == 6) ||
                (c->kind == 32 && c->primitive_count == SOTN_STONE_DISSOLVE_ROWS) ||
                (c->kind == 65 && c->primitive_count == SOTN_TELEPORT_ROWS) ||
                (c->kind == 38 && c->primitive_count == SOTN_PLAYER_CIRCLE_ROWS) ||
                (c->kind == 33 && c->primitive_count == 24) ||
                (c->kind == 5 && c->state.particles.phase == 1 && !c->state.particles.removed &&
                 c->state.particles.count &&
                 c->state.particles.count <= SOTN_PLAYER_PARTICLE_LIMIT &&
                 c->primitive_count == c->state.particles.count + 1U)))) ||
            ((c->kind == 5 || c->kind == 11 || c->kind == 46 || c->kind == 25 || c->kind == 28 ||
              c->kind == 29 || c->kind == 31 || c->kind == 35) &&
             c->initialized && !c->primitive_count) ||
            !RetainRows(&in.primitive_slots, &retained, c->primitives, c->primitive_count))
            return SDL_SetError("Bat visual lost its actual primitive reservation");
    }
    if (r.echo_generation &&
        (r.children[31].kind != 52 || r.children[31].generation != r.echo_generation ||
         !r.children[31].initialized))
        return SDL_SetError("Bat Echo primitive owner no longer matches its actual child");
    if (r.echo_generation &&
        !RetainRows(&in.primitive_slots, &retained, r.echo_primitives, SOTN_BAT_ECHO_PRIMITIVES))
        return false;
    for (Uint16 i = 0; i < 8; ++i)
        if (r.factories[i].cursor.entity_id)
        {
            r.factories[i].cursor.source.x =
                FixedShift(r.factories[i].cursor.source.x, in.room_shift_x);
            r.factories[i].cursor.source.y =
                FixedShift(r.factories[i].cursor.source.y, in.room_shift_y);
        }
    for (Uint16 i = 0; i < 48; ++i)
    {
        SotnBatRuntimeChild *c = &r.children[i];
        if (!c->kind)
            continue;
        c->source.x = FixedShift(c->source.x, in.room_shift_x);
        c->source.y = FixedShift(c->source.y, in.room_shift_y);
        if (c->kind == 35)
        {
            c->state.trail.x = FixedShift(c->state.trail.x, in.room_shift_x);
            c->state.trail.y = FixedShift(c->state.trail.y, in.room_shift_y);
        }
        if (c->food)
        {
            c->state.food.x = FixedShift(c->state.food.x, in.room_shift_x);
            c->state.food.y = FixedShift(c->state.food.y, in.room_shift_y);
        }
        else if (c->pentagram)
        {
            c->state.pentagram.motion.x = FixedShift(c->state.pentagram.motion.x, in.room_shift_x);
            c->state.pentagram.motion.y = FixedShift(c->state.pentagram.motion.y, in.room_shift_y);
        }
        else if (c->karma)
        {
            c->state.karma.motion.x = FixedShift(c->state.karma.motion.x, in.room_shift_x);
            c->state.karma.motion.y = FixedShift(c->state.karma.motion.y, in.room_shift_y);
        }
        else if (c->star)
        {
            SotnThrownWeapon *m = &c->state.star.motion.motion;
            m->x = FixedShift(m->x, in.room_shift_x);
            m->y = FixedShift(m->y, in.room_shift_y);
            if (c->initialized && c->primitive_count)
                for (Uint16 j = 0; j < 32; ++j)
                {
                    SotnStarSpark *q = &c->state.star.motion.sparks[j];
                    q->x = Word(q->x + in.room_shift_x);
                    q->y = Word(q->y + in.room_shift_y);
                }
        }
        else if (c->thrown)
        {
            SotnThrownWeapon *s = &c->state.thrown.motion;
            s->x = FixedShift(s->x, in.room_shift_x);
            s->y = FixedShift(s->y, in.room_shift_y);
            if (c->initialized && (c->kind & 15U) == 0)
            {
                s->primitive_x = FixedShift(s->primitive_x, in.room_shift_x);
                s->primitive_y = FixedShift(s->primitive_y, in.room_shift_y);
                s->primitive_anchor_x = Word(s->primitive_anchor_x + in.room_shift_x);
                s->primitive_anchor_y = Word(s->primitive_anchor_y + in.room_shift_y);
            }
        }
        else if (c->magic)
        {
            c->state.magic.motion.x = FixedShift(c->state.magic.motion.x, in.room_shift_x);
            c->state.magic.motion.y = FixedShift(c->state.magic.motion.y, in.room_shift_y);
        }
        else if (c->sire)
        {
            if (c->sire_child)
            {
                c->state.sire.child.x = FixedShift(c->state.sire.child.x, in.room_shift_x);
                c->state.sire.child.y = FixedShift(c->state.sire.child.y, in.room_shift_y);
            }
            else
            {
                c->state.sire.parent.x = FixedShift(c->state.sire.parent.x, in.room_shift_x);
                c->state.sire.parent.y = FixedShift(c->state.sire.parent.y, in.room_shift_y);
            }
        }
        else if (c->neutron)
        {
            c->state.neutron.motion.x = FixedShift(c->state.neutron.motion.x, in.room_shift_x);
            c->state.neutron.motion.y = FixedShift(c->state.neutron.motion.y, in.room_shift_y);
            for (Uint16 v = 0; v < 4; ++v)
            {
                c->state.neutron.motion.halo.x[v] =
                    Word(c->state.neutron.motion.halo.x[v] + in.screen_shift_x);
                c->state.neutron.motion.halo.y[v] =
                    Word(c->state.neutron.motion.halo.y[v] + in.screen_shift_y);
            }
        }
        else if (c->hand_weapon)
        {
            SotnHandWeapon *w = &c->state.hand.motion;
            w->x = FixedShift(w->x, in.room_shift_x);
            w->y = FixedShift(w->y, in.room_shift_y);
            // Weapon43 TILEs use one origin vertex; weapon1 GT4 trails own all four.
            const Uint16 vertices = c->state.hand.family == 1                                  ? 4
                                    : c->state.hand.family == 54 || c->state.hand.family == 56 ? 2
                                                                                               : 1;
            for (Uint16 j = 0; j < SOTN_HAND_WEAPON_TILES; ++j)
                for (Uint16 v = 0; v < vertices; ++v)
                {
                    w->tiles[j].x[v] = Word(w->tiles[j].x[v] + in.screen_shift_x);
                    w->tiles[j].y[v] = Word(w->tiles[j].y[v] + in.screen_shift_y);
                }
        }
        else if (c->weapon_warp)
        {
            c->state.warp.x = FixedShift(c->state.warp.x, in.room_shift_x);
            c->state.warp.y = FixedShift(c->state.warp.y, in.room_shift_y);
        }
        else if (c->weapon_secondary)
        {
            SotnWeaponSecondary *w = &c->state.secondary.motion;
            w->x = FixedShift(w->x, in.room_shift_x);
            w->y = FixedShift(w->y, in.room_shift_y);
            // DRAW_UNK_200 primitives translate every vertex without a callback: LINE_G2 rows,
            // or the GT4s sharing their storage (weapon48's ring, weapon1's punch rush).
            // Weapon53's slash quad is entity-relative; the fireball and weapon49's other
            // children own no rows (their union words hold motion instead).
            const Uint16 family = c->state.secondary.family, role = c->kind & 15U;
            if (family == 14 || family == 13 || family == 12 || family == 34 || family == 30 ||
                SotnWeaponSecondary_ShieldSpell(family))
            {
                // Weapon14's trail corners are world pixels like the entity (DRAW_UNK_100 rows
                // shift with it); its hitboxes own no rows.
                for (Uint16 j = 0; family == 14 && role == 0 && j < 48; ++j)
                {
                    w->rune_x1[j] += in.room_shift_x;
                    w->rune_y1[j] += in.room_shift_y;
                    w->rune_x3[j] += in.room_shift_x;
                    w->rune_y3[j] += in.room_shift_y;
                }
                // Weapon30 soldier rows are world pixels; the circle's follow its entity.
                for (Uint16 j = 0; family == 30 && role == 2 && j < 14; ++j)
                    for (Uint16 v = 0; v < 4; ++v)
                    {
                        w->dawn_quads[j].x[v] = Word(w->dawn_quads[j].x[v] + in.room_shift_x);
                        w->dawn_quads[j].y[v] = Word(w->dawn_quads[j].y[v] + in.room_shift_y);
                    }
                if (family == 14 && role == 0)
                {
                    w->rune_x0 += in.room_shift_x;
                    w->rune_y0 += in.room_shift_y;
                    w->rune_x2 += in.room_shift_x;
                    w->rune_y2 += in.room_shift_y;
                }
            }
            else if (family == 50 && role == 1)
            {
                // Weapon50 sparkles keep compact placements; their GT4 centres translate.
                for (Uint16 j = 0; j < 80; ++j)
                {
                    w->ice_cx[j] = Word(w->ice_cx[j] + in.screen_shift_x);
                    w->ice_cy[j] = Word(w->ice_cy[j] + in.screen_shift_y);
                }
            }
            else if (role == 2   ? family == 48 || family == 49 || family == 50
                     : role == 4 ? family == 50
                                 : family == 1)
            {
                for (Uint16 j = 0; j < c->primitive_count && j < 16; ++j)
                    for (Uint16 v = 0; v < 4; ++v)
                    {
                        w->quads[j].x[v] = Word(w->quads[j].x[v] + in.screen_shift_x);
                        w->quads[j].y[v] = Word(w->quads[j].y[v] + in.screen_shift_y);
                    }
            }
            else if ((role == 1 && family != 53 && family != 56 && family != 49) ||
                     (role == 3 && family == 50))
                for (Uint16 j = 0; j < SOTN_WEAPON_SECONDARY_ROWS; ++j)
                    for (Uint16 v = 0; v < 2; ++v)
                    {
                        w->rows[j].x[v] = Word(w->rows[j].x[v] + in.screen_shift_x);
                        w->rows[j].y[v] = Word(w->rows[j].y[v] + in.screen_shift_y);
                    }
        }
        else if (c->kind == 0xe1 || c->kind == 0xf1)
        {
            c->state.sword.image.x = FixedShift(c->state.sword.image.x, in.room_shift_x);
            c->state.sword.image.y = FixedShift(c->state.sword.image.y, in.room_shift_y);
        }
        if (c->kind == 24)
        {
            c->state.hellfire_trail.motion.x =
                FixedShift(c->state.hellfire_trail.motion.x, in.room_shift_x);
            c->state.hellfire_trail.motion.y =
                FixedShift(c->state.hellfire_trail.motion.y, in.room_shift_y);
        }
        if (c->kind == 26 || c->kind == 27)
        {
            c->state.hellfire_fire.motion.x =
                FixedShift(c->state.hellfire_fire.motion.x, in.room_shift_x);
            c->state.hellfire_fire.motion.y =
                FixedShift(c->state.hellfire_fire.motion.y, in.room_shift_y);
        }
        if (in.entering && (c->kind == 25 || c->kind == 28 || c->kind == 29))
        {
            Sint32 *x = c->kind == 25 ? &c->state.hellfire_beam.x : &c->state.hellfire_effect.x;
            Sint32 *y = c->kind == 25 ? &c->state.hellfire_beam.y : &c->state.hellfire_effect.y;
            SotnPlayerEffectQuad *q =
                c->kind == 25 ? &c->state.hellfire_beam.quad : &c->state.hellfire_effect.quad;
            *x = FixedShift(*x, in.screen_shift_x);
            *y = FixedShift(*y, in.screen_shift_y);
            if (c->initialized)
                for (Uint16 v = 0; v < 4; ++v)
                {
                    q->x[v] = Word(q->x[v] + in.screen_shift_x);
                    q->y[v] = Word(q->y[v] + in.screen_shift_y);
                }
        }
        if (c->kind == 45)
        {
            c->state.fire.motion.x = FixedShift(c->state.fire.motion.x, in.room_shift_x);
            c->state.fire.motion.y = FixedShift(c->state.fire.motion.y, in.room_shift_y);
        }
        if (c->kind == 2 &&
            !SotnPlayerSmoke_Rebase(&c->state.smoke, in.screen_shift_x, in.screen_shift_y))
            return false;
        if (c->kind == 51 &&
            !SotnRisingHeart_Rebase(&c->state.heart, in.screen_shift_x, in.screen_shift_y))
            return false;
        if (c->kind == 46 &&
            !SotnTalismanEffect_Rebase(&c->state.talisman, in.screen_shift_x, in.screen_shift_y))
            return false;
        if (c->kind == 37)
        {
            const bool shifted =
                in.entering ? SotnImmunityText_ChangeRoom(&c->state.immunity_text,
                                                          in.screen_shift_x, in.screen_shift_y)
                            : SotnImmunityText_Rebase(&c->state.immunity_text, in.screen_shift_x,
                                                      in.screen_shift_y);
            if (!shifted)
                return false;
        }
        if (c->kind == 54 && in.entering &&
            !SotnDarkHit_Rebase(&c->state.dark, in.screen_shift_x, in.screen_shift_y))
            return false;
        if (c->kind == 47 && in.entering &&
            !SotnHolyHit_Rebase(&c->state.holy, in.screen_shift_x, in.screen_shift_y, true))
            return false;
        if (c->kind == 19 && !SotnHealingNumber_Rebase(&c->state.healing_number, in.screen_shift_x,
                                                       in.screen_shift_y))
            return false;
        if (c->kind == 38 && in.entering &&
            !SotnPlayerCircle_Rebase(&c->state.circle, in.screen_shift_x, in.screen_shift_y, true))
            return false;
        if (c->kind == 33 && in.entering &&
            !SotnIceHit_Rebase(&c->state.ice, in.screen_shift_x, in.screen_shift_y, true))
            return false;
        if (c->kind == 30 && in.entering &&
            !SotnThunderHit_Rebase(&c->state.thunder, in.screen_shift_x, in.screen_shift_y, true))
            return false;
        if (c->kind == 16 && in.entering &&
            !SotnFireHit_Rebase(&c->state.fire_hit, in.screen_shift_x, in.screen_shift_y))
            return false;
        if (c->kind == 5 &&
            (((c->parameter >> 8) != 4 && (c->parameter >> 8) != 12) || in.entering) &&
            !SotnPlayerParticles_Rebase(&c->state.particles, in.screen_shift_x, in.screen_shift_y))
            return false;
        if (c->kind == 31 &&
            !SotnPlayerOutline_Translate(&c->state.outline, in.screen_shift_x, in.screen_shift_y))
            return false;
        if (in.entering && c->kind == 61)
        {
            if (!SotnSpirit_Translate(&c->state.spirit.motion, in.screen_shift_x,
                                      in.screen_shift_y))
                return false;
            for (Uint16 j = 0; j < 8; ++j)
                c->state.spirit.quads[j] = c->state.spirit.motion.trails[j].quad;
        }
        if (in.entering && c->kind == 63 && c->state.portal.phase)
        {
            c->state.portal.x = FixedShift(c->state.portal.x, in.screen_shift_x);
            c->state.portal.y = FixedShift(c->state.portal.y, in.screen_shift_y);
            if (c->primitive_count)
                for (Uint16 j = 0; j < 8; ++j)
                    for (Uint16 v = 0; v < 4; ++v)
                    {
                        c->state.portal.quads[j].x[v] =
                            Word(c->state.portal.quads[j].x[v] + in.screen_shift_x);
                        c->state.portal.quads[j].y[v] =
                            Word(c->state.portal.quads[j].y[v] + in.screen_shift_y);
                    }
        }
        SotnPlayerEffectQuad *q = c->kind == 11 ? c->state.blink.quads : &c->trail_quad;
        const Uint16 count = c->kind == 11 ? 8 : c->kind == 35 ? 1 : 0;
        for (Uint16 j = 0; j < count; ++j)
            for (Uint16 v = 0; v < 4; ++v)
            {
                q[j].x[v] = Word(q[j].x[v] + in.screen_shift_x);
                q[j].y[v] = Word(q[j].y[v] + in.screen_shift_y);
            }
    }
    if (r.echo_generation)
        for (Uint16 i = 0; i < SOTN_BAT_ECHO_QUADS; ++i)
            for (Uint16 v = 0; v < 4; ++v)
            {
                r.echo.quads[i].x[v] = Word(r.echo.quads[i].x[v] + in.screen_shift_x);
                r.echo.quads[i].y[v] = Word(r.echo.quads[i].y[v] + in.screen_shift_y);
            }
    if (in.wolf_owned && (in.screen_shift_x || in.screen_shift_y))
    {
        SotnWolfRuntimeEvents ignored = {0};
        if (!SotnWolfRuntime_Tick(&r.wolf, &r.wolf_frame,
                                  (SotnWolfRuntimeInput){.primitive_slots = in.primitive_slots,
                                                         .entering = true,
                                                         .screen_shift_x = in.screen_shift_x,
                                                         .screen_shift_y = in.screen_shift_y},
                                  &ignored))
            return false;
        r.wolf_frame.x = FixedShift(r.wolf_frame.x, in.screen_shift_x);
        r.wolf_frame.y = FixedShift(r.wolf_frame.y, in.screen_shift_y);
    }
    r.medicine = (SotnMedicineEffectEvents){0};
    r.teleport_palette = 0;
    r.teleport_warp_mode = 0;
    r.teleport_acknowledged = false;
    if (in.entering)
    {
        r.wolf_events = (SotnWolfRuntimeEvents){0};
        r.target_marks[0] = r.target_marks[1] = 0;
        r.capture_count = 0;
        r.freeze_changed = false;
        r.hellfire_upload = false;
        *runtime = r;
        return true;
    }
    r.hellfire_upload = false;
    r.sound_count = 0;
    r.sound_volume_mask = 0;
    r.wolf_shake_count = 0;
    r.use_ticks = r.spell_use_ticks = r.startup_ticks = 0;
    r.dark_minimum = 0;
    r.magic_clear_shield = false;
    r.afterimage_reset = false;
    r.dissolve_frame_load = false;
    SotnPlayerState p = in.player;
    r.target_marks[0] = r.target_marks[1] = 0;
    r.capture_count = 0;
    r.freeze_changed = false;
    r.freeze = in.freeze;
    if (p.axe_armor.events.factory_count > SDL_arraysize(p.axe_armor.events.factories) ||
        (in.freeze &&
         (p.axe_armor.entered || p.axe_armor.exited || p.axe_armor.events.factory_count)))
        return SDL_SetError("Axe Armor requires this ordinary player's committed requests");
    if (p.axe_armor.entered || p.axe_armor.exited)
    {
        SotnBatRuntimeChild *old = &r.children[0];
        if (old->primitive_count &&
            !SotnPrimitiveSlots_Release(&in.primitive_slots, old->primitive_count, old->primitives))
            return false;
        *old = (SotnBatRuntimeChild){0};
        r.hand_shield = 0;
    }
    for (Uint16 i = 0; i < p.axe_armor.events.factory_count; ++i)
    {
        const Uint32 packed = p.axe_armor.events.factories[i];
        if (!p.axe_armor.active ||
            (packed != 0 && packed != 85 && packed != 87 && packed != 0x10045))
            return SDL_SetError("Axe Armor requested a factory without its actual equipped owner");
        if (!RequestSource(&r, p.axe_armor.source, packed, 0))
            return false;
    }
    if (p.heart_gain.requested)
    {
        if (in.freeze || p.heart_gain.amount > 9999)
            return SDL_SetError("Blood Cloak requires a committed ordinary player contact");
        if (!RequestSource(&r, p.heart_gain.source, 99, 0) || !Sound(&r, 0x67a))
            return false;
    }
    if ((p.water_pain.events.blink && !RequestSource(&r, p.water_pain.source, 0x4e002c, 0)) ||
        (p.water_pain.events.voice && !Sound(&r, 0x6eb)) ||
        (p.water_pain.events.scrape && !Sound(&r, 0x6a3)))
        return false;
    if (p.water_death.started &&
        (!Flash(&r, in.dra, 1) || !RequestSource(&r, p.water_death.source, 0x59002c, 0) ||
         !RequestSource(&r, p.water_death.source, 0x60031, 0)))
        return false;
    if (p.death.started)
    {
        if (in.freeze || !p.death.native || !p.death.generation || p.death.variant > 3 || p.hp ||
            p.step != SOTN_PLAYER_KILL)
            return SDL_SetError("Physical death effects require their actual committed birth");
        const Uint16 flash = p.death.variant == 1   ? 3
                             : p.death.variant == 2 ? 9
                             : p.death.variant == 3 ? 10
                                                    : 1;
        const Uint32 blink = p.death.variant == 1   ? 0x4f002cU
                             : p.death.variant == 2 ? 0x59002cU
                             : p.death.variant == 3 ? 0x5a002cU
                                                    : 0x53002cU;
        const Uint32 child = p.death.variant == 1   ? 0x20033U
                             : p.death.variant == 2 ? 0x1002dU
                             : p.death.variant == 3 ? 46U
                                                    : 0x50031U;
        if (!Flash(&r, in.dra, flash) || !RequestSource(&r, p.death.source, blink, 0) ||
            !RequestSource(&r, p.death.source, child, 0))
            return false;
    }
    if (r.target_cursor >= SOTN_SPELL_TARGETS)
        return SDL_SetError("Spirit selector lost its original circular target cursor");
    if (p.dark.started &&
        (!Flash(&r, in.dra, 11) || !RequestSource(&r, p.dark.source, 0x110028U, 0)))
        return false;
    if (p.spirit.started && ((p.spirit.spell != 1 && p.spirit.spell != 3) ||
                             p.spirit.blueprint != (p.spirit.spell == 1 ? 117U : 0x10075U)))
        return SDL_SetError("Spirit cast lost its original spell/root factory parameter");
    if (p.spirit.started && !RequestSource(&r, p.spirit.source, p.spirit.blueprint, 0))
        return false;
    if ((p.soul.started && !Flash(&r, in.dra, 12)) ||
        (p.soul.attack && (!RequestSource(&r, p.soul.source, 0x160028U, 0) ||
                           !RequestSource(&r, p.soul.source, 112, 0))))
        return false;
    r.wolf_events = (SotnWolfRuntimeEvents){0};
    if (p.healing.applied && p.healing.applied_kind != 1 && p.healing.applied_kind != 2)
        return SDL_SetError("Shared healing effects require their actual original applied kind");
    if (p.healing.applied)
    {
        if (in.freeze || !p.healing.applied_amount || p.healing.applied_amount > 9999)
            return SDL_SetError(
                "Healing number requires this ordinary player's actual applied amount");
        if (!RequestNumber(&r, in.other_occupied, p.healing.source, p.healing.applied_amount,
                           false))
            return false;
    }
    if (p.healing.applied && p.healing.applied_kind == 1 && !Sound(&r, 0x68e))
        return false;
    if (p.healing.effects &&
        (!RequestSource(&r, p.healing.source, 0x48002c, 0) ||
         (p.healing.applied_kind == 1 && !RequestSource(&r, p.healing.source, 0x44002c, 0))))
        return false;
    if (p.dodge.applied)
    {
        if (in.freeze || !p.hp || p.hp > p.hp_max || p.guard.applied || p.immunity.applied ||
            p.absorption.applied)
            return SDL_SetError("Talisman requires its actual living ordinary response2");
        if (!RequestSource(&r, p.dodge.source, 115, 0) ||
            !RequestSource(&r, p.dodge.source, 0x58002c, 0))
            return false;
    }
    if (p.damage_number.applied)
    {
        const Uint8 response = p.damage_number.response;
        const bool grab = p.damage_number.boss_grab, secondary = p.damage_number.secondary_impact,
                   internal = grab || secondary;
        const bool internal_result =
            internal && !(grab && secondary) && !p.axe_armor.active &&
            !p.damage_number.neutral_hurt && (response != 1 || p.damage_number.amount == 1) &&
            (((response == 1 || response == 2 || response == 3) && p.hp &&
              (grab ? p.grab.damage <= 9999 && p.step == SOTN_PLAYER_BOSS_GRAB &&
                          p.grab.state == SOTN_GRAB_HELD
                    : p.step == SOTN_PLAYER_HIT && !p.hurt.started)) ||
             (response == 4 && !p.hp && p.step == SOTN_PLAYER_KILL));
        if (in.freeze || p.hp > p.hp_max || (!internal && p.damage_number.amount > 39996) ||
            p.guard.applied || p.dodge.applied || p.immunity.applied || p.absorption.applied ||
            (internal ? !internal_result
                      : (response != 3 && response != 4 && response != 7 && response != 8)) ||
            (p.damage_number.neutral_hurt && response != 3) ||
            ((response == 4 || (response == 3 && !p.axe_armor.active)) &&
             !p.damage_number.amount) ||
            (!internal && response == 3 &&
             (!p.hp || p.step != (p.axe_armor.active ? SOTN_PLAYER_AXE_HIT : SOTN_PLAYER_HIT))) ||
            (response == 4 && (p.hp || p.step != SOTN_PLAYER_KILL)) ||
            (response == 7 && p.step != SOTN_PLAYER_STONE) ||
            (response == 8 && (!p.hp || SotnPlayerStep_IsDead(p.step))))
            return SDL_SetError(
                "Damage number requires its actual contact/impact/held-grab result");
        if (!RequestNumber(&r, in.other_occupied, p.damage_number.source, p.damage_number.amount,
                           true) ||
            (p.damage_number.neutral_hurt && !p.hurt.started &&
             !RequestSource(&r, p.damage_number.source, 0x58002c, 0)))
            return false;
    }
    if (p.fairy_rescue.recovered)
    {
        if (in.freeze || !p.fairy_rescue.bound || p.fairy_rescue.scene ||
            !p.fairy_rescue.restored || !p.hp || p.death.native || p.water_death.active ||
            (p.step != SOTN_PLAYER_STAND && p.step != SOTN_PLAYER_FALL))
            return SDL_SetError("Fairy recovery flash lost its actual restored player owner");
        if (!RequestSource(&r, p.fairy_rescue.source, 0x4a002cU, 0))
            return false;
    }
    if (p.hurt.count > SDL_arraysize(p.hurt.factories))
        return SDL_SetError("Hurt feedback exceeded its actual bounded factory list");
    if ((in.freeze && (p.hurt.count || p.hurt.started || p.hurt.shake_requested)) ||
        (p.hurt.started && (!p.hp || p.step != SOTN_PLAYER_HIT)) ||
        ((p.hurt.count || p.hurt.shake_requested) && p.step != SOTN_PLAYER_HIT &&
         p.step != SOTN_PLAYER_KILL && p.step != SOTN_PLAYER_STONE))
        return SDL_SetError("Hurt feedback lost its actual ordinary reaction/death owner");
    for (Uint16 i = 0; i < p.hurt.count; ++i)
        if (!RequestSource(&r, p.hurt.factories[i].source, p.hurt.factories[i].blueprint, 0))
            return false;
    if (p.guard.applied)
    {
        if (in.freeze || !p.hp || p.hp > p.hp_max || p.immunity.applied || p.absorption.applied)
            return SDL_SetError("Guard number requires its actual surviving ordinary result");
        if (!RequestNumber(&r, in.other_occupied, p.guard.source, 1, true))
            return false;
    }
    if (p.immunity.applied)
    {
        if (in.freeze || !p.hp || p.hp > p.hp_max || p.absorption.applied)
            return SDL_SetError("Contact immunity requires its actual living ordinary result");
        if (!RequestSource(&r, p.immunity.source, 47, 0) ||
            !RequestSource(&r, p.immunity.source, 0x43002c, 0) ||
            !RequestSource(&r, p.immunity.source, 0x51002c, 0) ||
            !RequestNumber(&r, in.other_occupied, p.immunity.source, 0, true))
            return false;
    }
    if (p.absorption.applied)
    {
        if (in.freeze || !p.hp || p.hp > p.hp_max || !p.absorption.amount ||
            p.absorption.amount > 39996)
            return SDL_SetError("Contact absorption requires its actual living ordinary result");
        // Original response5 is silent and emits both flashes even while stone/full HP.
        if (!RequestSource(&r, p.absorption.source, 0x44002c, 0) ||
            !RequestSource(&r, p.absorption.source, 0x48002c, 0) ||
            !RequestNumber(&r, in.other_occupied, p.absorption.source, p.absorption.amount, false))
            return false;
    }
    if (p.movement_effects.water_bubble &&
        !RequestSource(&r, p.movement_effects.bubble_source, 0xd0004U, 0))
        return false;
    if (p.movement_effects.smoke &&
        !RequestSource(&r, p.movement_effects.smoke_source, p.movement_effects.smoke_blueprint, 0))
        return false;
    if (p.script_reaction.started)
    {
        if (in.freeze || p.step != SOTN_PLAYER_SCRIPT_REACTION || p.substep != 1 ||
            p.animation.id != 0x33 || !p.hp)
            return SDL_SetError("Script reaction lost its actual ordinary player birth");
        if (!RequestSource(&r, p.script_reaction.source, 0x58002c, 0))
            return false;
    }
    if (p.gravity.ceiling_impact && !RequestSource(&r, p.gravity.impact_source, 0x10004, 0))
        return false;
    if (in.wolf_owned && p.wolf.body_started)
    {
        for (Uint16 i = 1; i <= 14; ++i)
            if (r.children[i].primitive_count &&
                !SotnPrimitiveSlots_Release(&in.primitive_slots, r.children[i].primitive_count,
                                            r.children[i].primitives))
                return false;
        if (!SotnBatRuntime_ReplaceWolfSlots(&r) ||
            !SotnWolfRuntime_Begin(&r.wolf, p.wolf.body_generation,
                                   SotnBatRuntime_Occupied(&r) | in.other_occupied))
            return false;
    }
    if ((p.wolf.started && !Request(&r, p, 0x23002cU, p.wolf.generation)) ||
        (p.wolf.return_started && !Request(&r, p, 0x24002cU, p.wolf.generation)) ||
        (p.wolf.finish_flash && !Request(&r, p, 0x1b002cU, p.wolf.generation)) ||
        (p.bat.started && !Request(&r, p, 0x20002cU, p.bat.generation)) ||
        (p.bat_commands.wing_flash && !Request(&r, p, 0x5c002cU, 0)) ||
        (p.bat_commands.wing_attack && !Request(&r, p, 67, 0)) ||
        (p.bat_commands.fire_flash && !Request(&r, p, 0x5002cU, 0)) ||
        (p.bat_commands.fireball && !Request(&r, p, 81, 0)) ||
        (p.bat_commands.echo && !Request(&r, p, 103, 0)) ||
        (p.bat.return_started && !Request(&r, p, 0x21002cU, p.bat.generation)) ||
        (p.bat_commands.trail && !Request(&r, p, 65, 0)))
        return false;
    if (in.wolf_owned)
    {
        if (p.wolf.commands.count > SDL_arraysize(p.wolf.commands.requests))
            return SDL_SetError("Wolf controller exceeded its actual ordered request list");
        for (Uint16 i = 0; i < p.wolf.commands.count; ++i)
        {
            const SotnWolfRequest request = p.wolf.commands.requests[i];
            if (request.kind == SOTN_WOLF_FACTORY)
            {
                SotnPlayerFactorySource source = request.source;
                source.x = FixedShift(FixedShift(source.x, (Sint16)in.wolf_command_camera_x),
                                      in.room_shift_x);
                source.y = FixedShift(FixedShift(source.y, (Sint16)in.wolf_command_camera_y),
                                      in.room_shift_y);
                if (!RequestSource(&r, source, request.parameter, 0))
                    return false;
            }
            else if (request.kind == SOTN_WOLF_SOUND)
            {
                if (request.parameter > 65535)
                    return SDL_SetError("Wolf sound identity exceeds its original word");
                if (!Sound(&r, (Uint16)request.parameter))
                    return false;
            }
            else if (request.kind == SOTN_WOLF_SHAKE && request.parameter <= 255)
                r.wolf_shakes[r.wolf_shake_count++] = (Uint8)request.parameter;
            else
                return SDL_SetError("Wolf controller requested an unsupported actual effect");
        }
    }
    if (SotnBatRuntime_Occupied(&r) & in.other_occupied)
        return SDL_SetError("Bat runtime overlaps another actual ability owner");
    for (Uint16 i = 0; i < 8; ++i)
    {
        if (r.factories[i].cursor.entity_id &&
            (r.factories[i].thrown_factory || r.factories[i].thrown_particles))
        {
            const SotnPlayerFactory *f = &r.factories[i].cursor;
            if (r.factories[i].thrown_parent_slot < 17 || r.factories[i].thrown_parent_slot > 31 ||
                r.factories[i].bat_generation || r.factories[i].sword_parent ||
                r.factories[i].fire_parent || r.factories[i].magic_factory ||
                r.factories[i].sire_factory || f->argument ||
                f->parameter != (r.factories[i].thrown_particles ? 0xb00 : 0) ||
                (r.factories[i].thrown_particles
                     ? r.factories[i].thrown_factory || f->blueprint != 4 || f->entity_modifier
                     : (f->blueprint != 58 && f->blueprint != 110) ||
                           (f->entity_modifier != 0xe0 && f->entity_modifier != 0xf0)) ||
                (f->entity_id != 0xef && f->entity_id != 0xff))
                return SDL_SetError("Weapon16 descendant lost its actual factory/parent/dispatch");
        }
        if (r.factories[i].cursor.entity_id && r.factories[i].secondary_factory &&
            ((r.factories[i].cursor.blueprint != 56 && r.factories[i].cursor.blueprint != 58 &&
              r.factories[i].cursor.blueprint != 64 && r.factories[i].cursor.blueprint != 62 &&
              r.factories[i].cursor.blueprint != 0x5b && r.factories[i].cursor.blueprint != 0x5d &&
              r.factories[i].cursor.blueprint != 0x5f && r.factories[i].cursor.blueprint != 0x65 &&
              r.factories[i].cursor.blueprint != 0x44 && r.factories[i].cursor.blueprint != 0x61 &&
              r.factories[i].cursor.blueprint != 0x72 && r.factories[i].cursor.blueprint != 0x3e &&
              r.factories[i].cursor.blueprint != 0x46 && r.factories[i].cursor.blueprint != 0x6a &&
              r.factories[i].cursor.blueprint != 88 && r.factories[i].cursor.blueprint != 90 &&
              r.factories[i].cursor.blueprint != 92 && r.factories[i].cursor.blueprint != 0x4a &&
              r.factories[i].cursor.blueprint != 94 && r.factories[i].cursor.blueprint != 96) ||
             r.factories[i].bat_generation || r.factories[i].sword_parent ||
             r.factories[i].fire_parent || r.factories[i].sire_factory ||
             r.factories[i].magic_factory || r.factories[i].thrown_factory ||
             r.factories[i].thrown_particles || r.factories[i].star_factory ||
             r.factories[i].karma_factory || r.factories[i].secondary_parent_slot < 16 ||
             r.factories[i].secondary_parent_slot > 47 || r.factories[i].cursor.argument ||
             (r.factories[i].cursor.entity_modifier != 0xe0 &&
              r.factories[i].cursor.entity_modifier != 0xf0)))
            return SDL_SetError("Weapon secondary factory lost its actual blueprint56 owner");
        if (r.factories[i].cursor.entity_id && r.factories[i].sword_parent &&
            (r.factories[i].bat_generation || r.factories[i].cursor.blueprint != 62 ||
             (r.factories[i].cursor.entity_modifier != 0xe0 &&
              r.factories[i].cursor.entity_modifier != 0xf0)))
            return SDL_SetError("Sword factory lost its separate actual parent lineage");
        if (r.factories[i].cursor.entity_id && r.factories[i].magic_factory &&
            (r.factories[i].sire_factory || r.factories[i].bat_generation ||
             r.factories[i].sword_parent || r.factories[i].fire_parent ||
             r.factories[i].cursor.blueprint != 56 || r.factories[i].magic_parent_slot < 17 ||
             r.factories[i].magic_parent_slot > 31))
            return SDL_SetError("Magic Missile factory lost its actual56/raw parent owner");
        if (r.factories[i].cursor.entity_id && r.factories[i].fire_parent &&
            (r.factories[i].cursor.blueprint != 4 || r.factories[i].cursor.parameter != 0x400 ||
             r.factories[i].fire_parent_slot >= SOTN_BAT_RUNTIME_SLOTS ||
             r.factories[i].bat_generation || r.factories[i].sword_parent))
            return SDL_SetError("Fire particle factory lost its actual source identity");
        if (r.factories[i].cursor.entity_id && r.factories[i].karma_factory &&
            (r.factories[i].cursor.blueprint != 84 || r.factories[i].karma_parent_slot < 17 ||
             r.factories[i].karma_parent_slot > 31 || r.factories[i].bat_generation ||
             r.factories[i].sword_parent || r.factories[i].fire_parent ||
             r.factories[i].sire_factory || r.factories[i].magic_factory ||
             r.factories[i].star_factory || r.factories[i].thrown_factory ||
             r.factories[i].thrown_particles))
            return SDL_SetError("Karma factory lost actual84/raw-parent ownership");
        if (r.factories[i].cursor.entity_id && r.factories[i].star_factory)
        {
            const Uint16 bp = r.factories[i].cursor.blueprint;
            if ((bp != 58 && bp != 80 && bp != 76 && bp != 78 && bp != 104) ||
                r.factories[i].star_parent_slot < 17 || r.factories[i].star_parent_slot > 31 ||
                r.factories[i].bat_generation || r.factories[i].thrown_factory ||
                r.factories[i].magic_factory || r.factories[i].sword_parent)
                return SDL_SetError("Weapon15 factory lost actual descendant/raw parent ownership");
        }
        if (!r.factories[i].cursor.entity_id ||
            (in.freeze && !(r.factories[i].cursor.source.flags & 0x10000U)))
            continue;
        SotnPlayerFactoryEvents events = {0};
        if (!SotnPlayerFactory_Tick(&r.factories[i].cursor, in.dra,
                                    (SotnPlayerFactoryInput){
                                        .occupied = SotnBatRuntime_Occupied(&r) | in.other_occupied,
                                        .player_x = p.x,
                                        .player_y = p.y,
                                        .player_step = (Uint16)p.step,
                                        .player_substep = p.substep},
                                    &events))
            return false;
        for (Uint16 j = 0; j < events.count; ++j)
        {
            SotnPlayerFactoryChild child = events.children[j];
            if (child.replace)
            {
                if (child.slot != 16 ||
                    (child.entity_id != 0xe0 && child.entity_id != 0xf0 &&
                     child.entity_id != 0xee && child.entity_id != 0xfe &&
                     child.entity_id != 0xe1 && child.entity_id != 0xf1 &&
                     child.entity_id != 0xe2 && child.entity_id != 0xf2 && child.entity_id != 50) ||
                    (in.other_occupied & ((Uint64)1 << 16)))
                    return SDL_SetError("Shared runtime received an unowned weapon replacement");
                SotnBatRuntimeChild *old = &r.children[0];
                if (old->primitive_count &&
                    !SotnPrimitiveSlots_Release(&in.primitive_slots, old->primitive_count,
                                                old->primitives))
                    return false;
                *old = (SotnBatRuntimeChild){0};
                r.hand_shield = 0;
                child.replace = false;
            }
            if (!Spawn(&r, child,
                       r.factories[i].sword_parent ? r.factories[i].sword_parent
                                                   : r.factories[i].bat_generation,
                       r.factories[i].fire_parent, r.factories[i].fire_parent_slot,
                       r.factories[i].sire_factory, r.factories[i].sire_parent_slot,
                       r.factories[i].magic_factory, r.factories[i].magic_parent_slot,
                       r.factories[i].thrown_factory, r.factories[i].thrown_particles,
                       r.factories[i].thrown_parent_slot, r.factories[i].star_factory,
                       r.factories[i].star_parent_slot, r.factories[i].karma_factory,
                       r.factories[i].karma_parent_slot, r.factories[i].secondary_factory,
                       r.factories[i].secondary_parent_slot, &in))
                return false;
        }
    }
    SotnCharacterState progress = {0};
    SotnCharacterStats stats = {0};
    if (!SotnCharacter_Get(character, &progress, &stats))
        return false;
    SotnDeathImage dissolve_image = in.dissolve_image ? *in.dissolve_image : (SotnDeathImage){0};
    const Uint32 previous_random = progress.random;
    const Uint16 previous_mp = progress.mp, previous_hearts = progress.hearts;
    bool inventory_changed = false;
    r.shield_mp_hold = r.shield_heal = false;
    r.shield_protection = 0;
    r.transform_lockout = 0;
    bool healing_pending = p.healing.pending_kind != 0;
    if (!in.freeze)
        r.player_hitbox_state = SotnPlayerStep_IsDead(p.step) ? 0 : 1;
    r.wing_mask = 0;
    r.fire_mask = r.spirit_mask = r.hellfire_mask = r.sword_mask = r.axe_mask = r.neutron_mask =
        r.sire_mask = r.pentagram_mask = r.magic_mask = r.thrown_mask = r.star_mask = r.karma_mask =
            r.secondary_mask = 0;
    const bool previous_echo_attack = r.echo_attack;
    r.soul_attack = false;
    r.hand_attack = false;
    r.clut_count = 0;
    r.echo_attack = false;
    SotnSpiritTargetSnapshot targets = {0};
    if (in.spirit_targets)
        targets = *in.spirit_targets;
    for (Uint16 i = 0; i < 48; ++i)
    {
        if (i == 1 && in.wolf_owned && !in.freeze && !TickWolf(&r, &progress, stats, &p, &in))
            return false;
        SotnBatRuntimeChild *c = &r.children[i];
        if (!c->kind)
        {
            if (!i)
                r.hand_shield = 0;
            continue;
        }
        if (in.freeze && !(ChildFlags(c) & 0x10000U))
        {
            if (c->initialized && !c->thrown && !c->star && !c->karma && !c->weapon_secondary &&
                !c->weapon_warp && (c->kind == 0xe2 || c->kind == 0xf2))
                r.sword_mask |= (Uint64)1 << i;
            if (c->initialized && c->kind == 36)
                r.wing_mask |= (Uint16)(1U << (i - 1U));
            if (c->initialized && c->kind == 45)
                r.fire_mask |= (Uint64)1 << i;
            if (c->initialized && (c->kind == 26 || c->kind == 27))
                r.hellfire_mask |= (Uint64)1 << i;
            if (c->initialized && c->kind == 52)
                r.echo_attack = previous_echo_attack;
            if (c->initialized && c->kind == 53)
                r.soul_attack = c->state.attack.active;
            if (c->initialized && c->kind == 50 && c->state.attack.active)
                r.axe_mask |= (Uint64)1 << i;
            if (c->initialized && c->pentagram && c->state.pentagram.attack.active)
                r.pentagram_mask |= (Uint64)1 << i;
            if (c->initialized && c->sire && !c->sire_child && c->state.sire.attack.active)
                r.sire_mask |= (Uint64)1 << i;
            if (c->initialized && c->karma && c->state.karma.attack.active)
                r.karma_mask |= (Uint64)1 << i;
            if (c->initialized && c->weapon_secondary && c->state.secondary.attack.active)
                r.secondary_mask |= (Uint64)1 << i;
            if (c->initialized && c->star && c->state.star.attack.active)
                r.star_mask |= (Uint64)1 << i;
            if (c->initialized && c->thrown && c->state.thrown.attack.active)
                r.thrown_mask |= (Uint64)1 << i;
            if (c->initialized && c->magic && c->state.magic.attack.active)
                r.magic_mask |= (Uint64)1 << i;
            if (c->initialized && c->neutron && c->state.neutron.attack.active)
                r.neutron_mask |= (Uint64)1 << i;
            if (c->initialized && c->hand_weapon)
                r.hand_attack = c->state.hand.attack.active;
            continue;
        }
        bool removed = false;
        c->draw_count = 0;
        if (c->hand_weapon)
        {
            const Uint16 hand = c->kind >= 0xf0, family = c->state.hand.family;
            if (family != 255 && (!in.weapon_bound[hand] || in.weapon_family[hand] != family))
                return SDL_SetError("Ordinary callback lost its actual resident hand program");
            SotnHandWeapon *w = &c->state.hand.motion;
            bool allocated = c->primitive_count != 0;
            if (!c->initialized && HandRows(family) &&
                !SotnPrimitiveSlots_Reserve(&in.primitive_slots, HandRows(family), c->primitives,
                                            &allocated))
                return false;
            if (HandRows(family))
                c->primitive_count = (Uint8)(allocated ? HandRows(family) : 0U);
            SotnDataView overlay = in.dra;
            if (family != 255 &&
                !SotnWeapon_Overlay(
                    in.weapons, (SotnEquipment){.id = c->state.hand.item, .family = (Uint8)family},
                    &overlay))
                return false;
            bool started[SOTN_HAND_WEAPON_TILES];
            SDL_memcpy(started, w->tile_started, sizeof(started));
            HandPropertyContext context = {character, c->state.hand.item, p.poison_ticks != 0};
            SotnHandWeaponEvents events = {0};
            if (!SotnHandWeapon_Tick(
                    w, &r.hand_program[hand],
                    (SotnHandWeaponInput){.overlay = overlay,
                                          .executable = in.executable,
                                          .camera_x = in.camera_x,
                                          .camera_y = in.camera_y,
                                          .dark_ticks = p.dark.ticks,
                                          .properties = HandProperties,
                                          .context = &context,
                                          .x = p.x,
                                          .y = p.y,
                                          .velocity_x = p.velocity_x,
                                          .status = p.status,
                                          .contacts = p.contacts,
                                          .remaining = (Sint16)p.animation.remaining,
                                          .scale_y = 256,
                                          .pivot_y = p.draw_pivot_y,
                                          .hand = hand,
                                          .family = family,
                                          .animation = p.animation.id,
                                          .pose = p.animation.pose,
                                          .priority = p.priority,
                                          .step = (Uint16)p.step,
                                          .substep = p.substep,
                                          .attack_mode = p.attack_mode,
                                          .draw_flags = p.wolf.active ? p.wolf.draw_flags
                                                        : p.rotated   ? 4
                                                                      : 0,
                                          .facing_left = p.facing_left,
                                          .held = (p.buttons & (hand ? SOTN_BUTTON_RIGHT_HAND
                                                                     : SOTN_BUTTON_LEFT_HAND)) != 0,
                                          .special = (c->kind & 15U) == 1,
                                          .allocated = allocated,
                                          .game_timer = in.game_timer,
                                          .mp = progress.mp},
                    &progress.random, &events))
                return false;
            for (Uint16 j = 0; j < SOTN_HAND_WEAPON_TILES; ++j)
                if (!started[j] && w->tile_started[j])
                {
                    w->tiles[j].x[0] = Word(w->tiles[j].x[0] - in.camera_x);
                    w->tiles[j].y[0] = Word(w->tiles[j].y[0] - in.camera_y);
                }
            if (events.properties)
                c->group = (Uint8)(hand + 1U);
            if (events.clut)
            {
                if (r.clut_count >= SDL_arraysize(r.clut))
                    return SDL_SetError("Weapon51 glow exceeded its per-pass palette writes");
                r.clut[r.clut_count].palette = events.clut_palette;
                r.clut[r.clut_count].color = events.clut_color;
                r.clut[r.clut_count].mask = 0;
                r.clut[r.clut_count++].value = events.clut_value;
            }
            if (events.shield)
                r.hand_shield = events.shield == 1;
            // Weapon52's armed shield: D_80097BF8 holds MP regeneration this update; one MP
            // drains on every sixteenth game update (the original's signed MP may reach -1 while
            // the shield lowers; native MP stops at zero); a raised hit requests
            // func_8010E168(0,0x60) (BlinkWhite0x15 from the shield and timer13), kind1 healing
            // and AddHearts(1).
            if (events.mp_hold)
                r.shield_mp_hold = true;
            if (events.mp_drain && progress.mp)
                --progress.mp;
            if (events.shield_hit)
            {
                Uint16 j = 0;
                while (j < SOTN_BAT_RUNTIME_FACTORIES && r.factories[j].cursor.entity_id)
                    ++j;
                if (j < SOTN_BAT_RUNTIME_FACTORIES)
                {
                    SotnPlayerFactory made = {0};
                    if (!SotnPlayerFactory_Begin(
                            &made,
                            (SotnPlayerFactorySource){.x = w->x,
                                                      .y = w->y,
                                                      .flags = w->flags,
                                                      .priority = w->priority,
                                                      .facing_left = w->facing_left},
                            44U | 0x15U << 16, 0))
                        return false;
                    SDL_zero(r.factories[j]);
                    r.factories[j].cursor = made;
                }
                r.shield_protection = 0x60;
                r.shield_heal = true;
                if (!AddHearts(&r, p, &progress))
                    return false;
            }
            // Weapon23 (Herald shield): a blocked projectile's AddHearts(1).
            if (events.hearts && !AddHearts(&r, p, &progress))
                return false;
            // Current selected Short sword uses variant zero. Other weapon37 producers
            // require their real secondary owners before the menu can expose them.
            // Ordered WFACTORY requests from the actual weapon entity; a full factory table
            // silently drops a request, as CreateEntFactoryFromEntity returning NULL does.
            for (Uint16 k = 0; k <= SDL_arraysize(events.more_factories); ++k)
            {
                const Uint32 request = k ? events.more_factories[k - 1U] : events.factory;
                if (!request)
                    break;
                if (!SecondaryRequest(family, hand, request))
                    return SDL_SetError("Ordinary weapon secondary has no actual native owner");
                for (Uint16 j = 0; j < SOTN_BAT_RUNTIME_FACTORIES; ++j)
                    if (!r.factories[j].cursor.entity_id)
                    {
                        SotnPlayerFactory f = {0};
                        if (!SotnPlayerFactory_Begin(
                                &f,
                                (SotnPlayerFactorySource){.x = w->x,
                                                          .y = w->y,
                                                          .flags = w->flags,
                                                          .priority = w->priority,
                                                          .facing_left = w->facing_left},
                                request, 0))
                            return false;
                        SDL_zero(r.factories[j]);
                        r.factories[j].cursor = f;
                        r.factories[j].secondary_factory = true;
                        r.factories[j].secondary_parent_slot = 16;
                        break;
                    }
            }
            // Weapon10's raised hit: func_8010E168(0,0x60) after its WFACTORY(56,0) thunder
            // (BlinkWhite0x15 from the shield and timer13, without healing).
            if (events.guard_blink)
            {
                Uint16 j = 0;
                while (j < SOTN_BAT_RUNTIME_FACTORIES && r.factories[j].cursor.entity_id)
                    ++j;
                if (j < SOTN_BAT_RUNTIME_FACTORIES)
                {
                    SotnPlayerFactory made = {0};
                    if (!SotnPlayerFactory_Begin(
                            &made,
                            (SotnPlayerFactorySource){.x = w->x,
                                                      .y = w->y,
                                                      .flags = w->flags,
                                                      .priority = w->priority,
                                                      .facing_left = w->facing_left},
                            44U | 0x15U << 16, 0))
                        return false;
                    SDL_zero(r.factories[j]);
                    r.factories[j].cursor = made;
                }
                r.shield_protection = 0x60;
            }
            if (events.dark_minimum)
                r.dark_minimum = events.dark_minimum;
            if (events.outline)
            {
                // w_054's FACTORY(61,0x11) begins at the weapon's own position and facing.
                bool outline_allocated = false;
                for (Uint16 j = 0; j < 8 && !outline_allocated; ++j)
                    if (!r.factories[j].cursor.entity_id)
                    {
                        SotnPlayerFactory f = {0};
                        if (!SotnPlayerFactory_Begin(
                                &f,
                                (SotnPlayerFactorySource){.x = w->x,
                                                          .y = w->y,
                                                          .priority = w->priority,
                                                          .facing_left = w->facing_left},
                                events.outline, 0))
                            return false;
                        SDL_zero(r.factories[j]);
                        r.factories[j].cursor = f;
                        outline_allocated = true;
                    }
            }
            if (events.sound && !Sound(&r, events.sound))
                return false;
            for (Uint16 j = 0; j < SDL_arraysize(events.extra_sounds); ++j)
                if (events.extra_sounds[j] && !Sound(&r, events.extra_sounds[j]))
                    return false;
            c->source.x = w->x;
            c->source.y = w->y;
            c->source.facing_left = w->facing_left;
            const SotnEquipment e = w->properties;
            c->state.hand.attack = (SotnPlayerAttack){
                .offset_x = w->hitbox_x,
                .offset_y = w->hitbox_y,
                .width = w->hitbox_width,
                .height = w->hitbox_height,
                .power = (Uint16)e.attack,
                .element = e.element,
                .hit_type = w->hitbox_state,
                .hit_effect = e.hit_effect,
                .stun_frames = e.stun,
                .invincibility_frames = e.invincibility,
                .critical_rate = (Uint8)e.critical_rate,
                .equipment = c->state.hand.item,
                .right_hand = hand != 0,
                .active = w->hitbox_state && w->hitbox_width && w->hitbox_height};
            const Sint32 x = Pixel(w->x) - in.camera_x, y = Pixel(w->y) - in.camera_y;
            removed = w->removed ||
                      (!(w->flags & 0x2000000U) && (x > 288 || x < -32 || y > 256 || y < -16));
            c->initialized = !removed;
            if (!removed)
                r.hand_attack = c->state.hand.attack.active;
        }
        else if (c->weapon_warp)
        {
            const Uint16 hand = c->kind >= 0xf0;
            if (!in.weapon_bound[hand] || in.weapon_family[hand] != 53)
                return SDL_SetError("Weapon53 warp lost its actual resident hand program");
            SotnWeaponWarp *w = &c->state.warp;
            SotnWeaponWarpEvents events = {0};
            if (!SotnWeaponWarp_Tick(
                    w,
                    (SotnWeaponWarpInput){.scale_y = 256,
                                          .pivot_y = p.draw_pivot_y,
                                          .remaining = (Sint16)p.animation.remaining,
                                          .hand = hand,
                                          .pose = p.animation.pose,
                                          .draw_flags = p.rotated ? 4 : 0,
                                          .facing_left = p.facing_left},
                    &events))
                return false;
            // Its slash roots0x28/0x1E copy this entity's equip id and enemyId0.
            for (Uint16 k = 0; k < SDL_arraysize(events.factories) && events.factories[k]; ++k)
                for (Uint16 j = 0; j < SOTN_BAT_RUNTIME_FACTORIES; ++j)
                    if (!r.factories[j].cursor.entity_id)
                    {
                        SotnPlayerFactory f = {0};
                        if (!SotnPlayerFactory_Begin(
                                &f,
                                (SotnPlayerFactorySource){.x = w->x,
                                                          .y = w->y,
                                                          .flags = w->flags,
                                                          .priority = c->source.priority,
                                                          .facing_left = w->facing_left},
                                events.factories[k], 0))
                            return false;
                        SDL_zero(r.factories[j]);
                        r.factories[j].cursor = f;
                        r.factories[j].secondary_factory = true;
                        r.factories[j].secondary_parent_slot = 16;
                        break;
                    }
            // The warp lacks CAMERA_LOCKED: like Neutron it keeps its screen position after the
            // camera work, after spawning its roots from the pre-scroll position.
            w->x = FixedShift(w->x, Word(in.room_shift_x - in.screen_shift_x));
            w->y = FixedShift(w->y, Word(in.room_shift_y - in.screen_shift_y));
            c->source.x = w->x;
            c->source.y = w->y;
            c->source.facing_left = w->facing_left;
            removed = w->removed;
            c->initialized = !removed;
        }
        else if (c->weapon_secondary)
        {
            const Uint16 hand = c->kind >= 0xf0, family = c->state.secondary.family;
            if (!in.weapon_bound[hand] || in.weapon_family[hand] != family)
                return SDL_SetError("Weapon secondary lost its actual resident hand program");
            SotnWeaponSecondary *w = &c->state.secondary.motion;
            const bool cold = !c->initialized;
            const Uint8 rows = SotnWeaponSecondary_ChildRows(family, c->kind & 15U, c->parameter);
            bool allocated = c->primitive_count != 0;
            if (cold && rows &&
                !SotnPrimitiveSlots_Reserve(&in.primitive_slots, rows, c->primitives, &allocated))
                return false;
            if (cold)
                c->primitive_count = allocated ? rows : 0;
            // Weapon13's combo sweep calls AllocPrimitives(LINE_G2, 1) only on the update its
            // step3 timer reaches0x34 (failure destroys it): reserve at its slot-order turn.
            const bool sweep = family == 13 && (c->kind & 15U) == 2;
            if (sweep && w->phase == 3 && w->heaven_timer == 0x33)
            {
                if (c->primitive_count)
                    return SDL_SetError("Heaven sword sweep already holds its late row");
                if (!SotnPrimitiveSlots_Reserve(&in.primitive_slots, 1, c->primitives, &allocated))
                    return false;
                c->primitive_count = allocated ? 1 : 0;
            }
            // The raw ext.weapon.parent (slot16, or a weapon53 root) supplies its current equipId
            // and enemyId without a generation check. An empty slot reads zero words.
            Uint16 item = c->state.secondary.item;
            Uint8 parent_group = 0;
            SotnEquipment parent_properties = {0};
            Uint16 parent_hitbox_state = 0;
            // Weapon13's strike swords copy the raw parent's equipId/enemyId when they launch.
            const bool strike = family == 13 && (c->kind & 15U) == 3;
            if ((cold || strike) && !c->state.secondary.own_item)
            {
                // A chained child born into its retired root's slot points at itself and reads
                // its own freshly cleared words.
                const Uint16 at = c->state.secondary.parent_slot;
                const SotnBatRuntimeChild *parent =
                    at == i + 16U ? &(const SotnBatRuntimeChild){0} : &r.children[at - 16U];
                if ((parent->hand_weapon &&
                     !SotnBatRuntime_HandValid(parent, (Uint16)(at - 16U))) ||
                    (parent->weapon_secondary &&
                     !SotnBatRuntime_SecondaryValid(parent, (Uint16)(at - 16U))))
                    return SDL_SetError("Weapon secondary raw parent lost its actual owner");
                if ((parent->weapon_warp && !SotnBatRuntime_WarpValid(parent, 0)) ||
                    (parent->kind && !parent->hand_weapon && !parent->weapon_secondary &&
                     !parent->weapon_warp))
                    return SDL_SetError("Weapon secondary raw parent is not a selected weapon");
                item = parent->hand_weapon        ? parent->state.hand.item
                       : parent->weapon_secondary ? parent->state.secondary.item
                       : parent->weapon_warp      ? parent->state.warp.item
                                                  : 0;
                // Weapon49 sparks copy the raw parent's live attack fields.
                if (parent->hand_weapon)
                {
                    parent_properties = parent->state.hand.motion.properties;
                    parent_hitbox_state = parent->state.hand.motion.hitbox_state;
                }
                else if (parent->weapon_secondary)
                {
                    parent_properties = parent->state.secondary.motion.properties;
                    parent_hitbox_state = parent->state.secondary.motion.hitbox_state;
                }
                parent_group = parent->kind ? parent->group : 0;
            }
            SotnDataView overlay = {0};
            // Any selected owning item resolves the resident program's overlay slice.
            const Uint16 owner = family == 55   ? 115
                                 : family == 38 ? 23
                                 : family == 53 ? 100
                                 : family == 48 ? 112
                                 : family == 1  ? 85
                                 : family == 51 ? 127
                                 : family == 49 ? 111
                                 : family == 50 ? 113
                                 : family == 6  ? 132
                                 : family == 14 ? 143
                                 : family == 13 ? 119
                                 : family == 7  ? 4
                                 : family == 10 ? 7
                                 : family == 23 ? 9
                                 : family == 24 ? 10
                                 : family == 56 ? 141
                                 : family == 12 ? 133
                                 : family == 34 ? 28
                                 : family == 30 ? 17
                                 : family == 8  ? 5
                                 : family == 9  ? 6
                                 : family == 58 ? 167
                                 : family == 11 ? 8
                                 : family == 25 ? 11
                                 : family == 26 ? 12
                                 : family == 52 ? 16
                                                : 22;
            if (!SotnWeapon_Overlay(
                    in.weapons, (SotnEquipment){.id = owner, .family = (Uint8)family}, &overlay))
                return false;
            // Weapon14 hitboxes and weapon13/12 afterimages read their raw parent slot's live
            // words.
            SotnWeaponSecondaryInput parent = {0};
            // Weapon11's walker/afterimages read theirs only at birth (a later occupant is
            // foreign).
            if (((family == 14 || family == 13) && (c->kind & 15U) == 1) ||
                (family == 12 && (c->kind & 15U) == 2) ||
                (family == 11 && (c->kind & 15U) == 9 && !w->phase))
            {
                const Uint16 at = c->state.secondary.parent_slot;
                const SotnBatRuntimeChild *root = &r.children[at - 16U];
                if (root->kind &&
                    (!root->weapon_secondary || root->state.secondary.family != family ||
                     (family != 11 && (root->kind & 15U) != (family == 12 ? 1U : 0U)) ||
                     !SotnBatRuntime_SecondaryValid(root, (Uint16)(at - 16U))))
                    return SDL_SetError("Thrown sword child raw parent is not its selected root");
                if (root->kind)
                {
                    const SotnWeaponSecondary *m = &root->state.secondary.motion;
                    parent = (SotnWeaponSecondaryInput){
                        .parent_exists = true,
                        .parent_facing_left = m->facing_left,
                        .parent_x = m->x,
                        .parent_y = m->y,
                        .parent_rotate = m->rotate,
                        .parent_draw_flags = m->draw_flags,
                        .parent_frame = m->frame,
                        .parent_bank = m->bank,
                        .parent_texture = m->texture,
                        .parent_child_palette =
                            family == 12 || family == 11 ? m->palette : (Uint16)m->child_palette};
                }
            }
            // Weapon25's clouds read their raw parent slot's entityId and position at birth and
            // every fourth update. The original reads any later occupant's words too; such an
            // occupant supplies its tracked origin here. An empty slot's zeroed words are screen
            // position0.
            if (family == 25 && (c->kind & 15U) == 9)
            {
                const SotnBatRuntimeChild *root = &r.children[c->state.secondary.parent_slot - 16U];
                parent.parent_exists = root->kind != 0;
                parent.parent_x = !root->kind              ? (Sint32)((Uint32)in.camera_x * 65536U)
                                  : root->weapon_secondary ? root->state.secondary.motion.x
                                                           : root->source.x;
                parent.parent_y = !root->kind              ? (Sint32)((Uint32)in.camera_y * 65536U)
                                  : root->weapon_secondary ? root->state.secondary.motion.y
                                                           : root->source.y;
            }
            // Weapon26's stars read their raw parent spell's step each update; its trails their
            // raw parent star's row and unk9C (an empty slot retires them). A foreign occupant
            // reads as step0 and offset0 (hidden rows) here.
            if (family == 26 && (c->kind & 15U) == 9)
            {
                const SotnBatRuntimeChild *root = &r.children[c->state.secondary.parent_slot - 16U];
                const bool own = root->weapon_secondary && root->state.secondary.family == 26;
                parent.parent_exists = root->kind != 0;
                parent.parent_phase = own ? root->state.secondary.motion.phase : 0;
                parent.parent_offset_x =
                    own && (root->kind & 15U) == 9 ? root->state.secondary.motion.star_offset_x : 0;
                if (own && (root->kind & 15U) == 9)
                    parent.parent_quad = root->state.secondary.motion.spell_quads[0];
            }
            HandPropertyContext context = {character, item, p.poison_ticks != 0};
            SotnWeaponSecondaryEvents events = {0};
            // Weapon30's circle reads free factory entities and the raw slots17..31 at its
            // callback (CreateEntFactoryFromEntity and its E2/F2 spacing scan).
            Uint8 free_factories = 0;
            for (Uint16 j = 0; j < SOTN_BAT_RUNTIME_FACTORIES; ++j)
                free_factories += !r.factories[j].cursor.entity_id;
            SotnWeaponSecondaryInput secondary_in = {
                .overlay = overlay,
                .executable = in.executable,
                .properties = HandProperties,
                .context = &context,
                .x = p.x,
                .y = p.y,
                .camera_x = in.camera_x,
                .camera_y = in.camera_y,
                .scale_y = 256,
                .pivot_y = p.draw_pivot_y,
                .hand = hand,
                .family = family,
                .priority = p.priority,
                .animation = p.animation.id,
                .attack_mode = p.attack_mode,
                .substep = p.substep,
                .role = (Uint16)(c->kind & 15U),
                .crouch = (p.status & 0x20U) != 0,
                .draw_flags = p.wolf.active ? p.wolf.draw_flags
                              : p.rotated   ? 4
                                            : 0,
                .facing_left = p.facing_left,
                .allocated = allocated,
                .slot16_parameter = r.children[0].kind ? r.children[0].parameter : 0,
                .parent_properties = parent_properties,
                .parent_hitbox_state = parent_hitbox_state,
                .parent_hitbox_x = p.animation.hitbox_x,
                .parent_hitbox_y = p.animation.hitbox_y,
                .player_velocity_x = p.velocity_x,
                .player_hitbox_width = p.animation.hitbox_width,
                .player_hitbox_height = p.animation.hitbox_height,
                .player_crouch_step = p.step == SOTN_PLAYER_CROUCH,
                .parent_exists = parent.parent_exists,
                .parent_facing_left = parent.parent_facing_left,
                .parent_x = parent.parent_x,
                .parent_y = parent.parent_y,
                .parent_rotate = parent.parent_rotate,
                .parent_draw_flags = parent.parent_draw_flags,
                .parent_frame = parent.parent_frame,
                .parent_bank = parent.parent_bank,
                .parent_texture = parent.parent_texture,
                .parent_child_palette = parent.parent_child_palette,
                .game_timer = in.game_timer,
                .player_substep = p.substep,
                .collision = in.food_collision,
                .free_factories = free_factories,
            };
            // Weapon30 soldiers read their raw parent circle's wave and facing (a retired
            // circle's emptied slot reads zero) and the current camera bounds.
            if (family == 30 && (c->kind & 15U) == 2)
            {
                const Uint16 at = c->state.secondary.parent_slot;
                const SotnBatRuntimeChild *root = &r.children[at - 16U];
                if (root->weapon_secondary && root->state.secondary.family == 30 &&
                    (root->kind & 15U) == 1)
                {
                    secondary_in.parent_wave = root->state.secondary.motion.dawn_wave;
                    secondary_in.parent_facing_left = root->state.secondary.motion.facing_left;
                }
                // Other weapon30 children never write weapon_030.unk84 (zero); a foreign
                // occupant's word is not reconstructed.
                else if (root->kind &&
                         !(root->weapon_secondary && root->state.secondary.family == 30))
                    return SDL_SetError("Sword of Dawn soldier raw parent is not its circle");
            }
            // Weapon10's thunder reads its raw slot16 parent every update (it retires unless that
            // slot holds item7); its swords read their raw parent spell's facing, palette and v
            // band at birth and its presence every update.
            if (family == 10 && (c->kind & 15U) == 1)
            {
                const SotnBatRuntimeChild *root = &r.children[0];
                const Uint16 held = root->hand_weapon        ? root->state.hand.item
                                    : root->weapon_secondary ? root->state.secondary.item
                                    : root->weapon_warp      ? root->state.warp.item
                                                             : 0;
                secondary_in.parent_exists = root->kind != 0 && held == 7;
                if (root->hand_weapon)
                {
                    secondary_in.parent_bank = root->state.hand.motion.bank;
                    secondary_in.parent_texture = root->state.hand.motion.texture;
                    secondary_in.parent_priority = root->state.hand.motion.priority;
                }
            }
            if (family == 10 && (c->kind & 15U) == 9)
            {
                const Uint16 at = c->state.secondary.parent_slot;
                const SotnBatRuntimeChild *root = &r.children[at - 16U];
                const bool own = root->weapon_secondary && root->state.secondary.family == 10 &&
                                 (root->kind & 15U) == 8;
                if (root->kind && !own)
                    return SDL_SetError("Iron shield sword raw parent is not its spell");
                secondary_in.parent_exists = root->kind != 0;
                if (own)
                {
                    secondary_in.parent_facing_left = root->state.secondary.motion.facing_left;
                    secondary_in.parent_child_palette =
                        root->state.secondary.motion.spell_child_palette;
                    secondary_in.parent_band = root->state.secondary.motion.spell_v1;
                }
            }
            // Weapon24's rocks read their raw parent spell's presence, v band and child palette at
            // birth and its step every update.
            if (family == 24 && (c->kind & 15U) == 9)
            {
                const Uint16 at = c->state.secondary.parent_slot;
                const SotnBatRuntimeChild *root = &r.children[at - 16U];
                const bool own = root->weapon_secondary && root->state.secondary.family == 24 &&
                                 (root->kind & 15U) == 8;
                if (root->kind && !own)
                    return SDL_SetError("Dark shield rock raw parent is not its spell");
                secondary_in.parent_exists = root->kind != 0;
                if (own)
                {
                    secondary_in.parent_band = root->state.secondary.motion.spell_v1;
                    secondary_in.parent_child_palette =
                        root->state.secondary.motion.spell_child_palette;
                    secondary_in.parent_phase = root->state.secondary.motion.phase;
                }
            }
            // Weapon23's sprites copy their raw parent's palette and unk5A at birth and its animSet
            // when their wait ends (params0 waits 65536 updates; a later spell may occupy it).
            if (family == 23 && (c->kind & 15U) == 10)
            {
                const Uint16 at = c->state.secondary.parent_slot;
                const SotnBatRuntimeChild *root = &r.children[at - 16U];
                if (root->kind && !root->weapon_secondary && !root->hand_weapon)
                    return SDL_SetError("Herald shield sprite raw parent has no native animSet");
                const SotnWeaponSecondary *ws = &root->state.secondary.motion;
                const SotnHandWeapon *wh = &root->state.hand.motion;
                secondary_in.parent_exists = root->kind != 0;
                secondary_in.parent_palette = !root->kind              ? 0
                                              : root->weapon_secondary ? ws->palette
                                                                       : wh->palette;
                secondary_in.parent_texture = !root->kind              ? 0
                                              : root->weapon_secondary ? ws->texture
                                                                       : wh->texture;
                secondary_in.parent_bank = !root->kind              ? 0
                                           : root->weapon_secondary ? ws->bank
                                                                    : wh->bank;
            }
            // Weapon7's orb reads its raw slot16 parent at birth (facing, bank, unk5A, palette).
            if (family == 7 && (c->kind & 15U) == 1 && cold)
            {
                const SotnBatRuntimeChild *root = &r.children[0];
                if (root->kind && !root->hand_weapon)
                    return SDL_SetError("Shield rod orb raw parent is not its hand weapon");
                secondary_in.parent_exists = root->kind != 0;
                if (root->hand_weapon)
                {
                    const SotnHandWeapon *m = &root->state.hand.motion;
                    secondary_in.parent_facing_left = m->facing_left;
                    secondary_in.parent_bank = m->bank;
                    secondary_in.parent_texture = m->texture;
                    secondary_in.parent_palette = m->palette;
                }
            }
            // Weapon13's strike swords read their raw parent sweep every update (its retirement
            // retires them; birth copies its sprite words and target; they launch at its step3).
            if (strike)
            {
                const Uint16 at = c->state.secondary.parent_slot;
                const SotnBatRuntimeChild *root = &r.children[at - 16U];
                if (root->kind &&
                    (!root->weapon_secondary || root->state.secondary.family != 13 ||
                     (root->kind & 15U) != 2 || !SotnBatRuntime_SecondaryValid(root, at - 16U)))
                    return SDL_SetError("Heaven sword strike raw parent is not its sweep");
                const SotnWeaponSecondary *m = &root->state.secondary.motion;
                secondary_in.parent_exists = root->kind != 0;
                if (root->kind)
                {
                    secondary_in.parent_facing_left = m->facing_left;
                    secondary_in.parent_rotate = m->rotate;
                    secondary_in.parent_draw_flags = m->draw_flags;
                    secondary_in.parent_frame = m->frame;
                    secondary_in.parent_bank = m->bank;
                    secondary_in.parent_texture = m->texture;
                    secondary_in.parent_palette = m->palette;
                    secondary_in.parent_priority = m->priority;
                    secondary_in.parent_target_x = m->heaven_target_x;
                    secondary_in.parent_target_y = m->heaven_target_y;
                    secondary_in.parent_phase = m->phase;
                }
            }
            // Arrows, swords and bombs read their raw parent soldier's facing (and the bombs its
            // presence and pose); homing swords their retained target slot's current words.
            if (family == 30 && (c->kind & 15U) >= 3)
            {
                const Uint16 at = c->state.secondary.parent_slot;
                const SotnBatRuntimeChild *root = &r.children[at - 16U];
                secondary_in.parent_exists = root->kind != 0;
                if (root->weapon_secondary && root->state.secondary.family == 30)
                {
                    secondary_in.parent_facing_left = root->state.secondary.motion.facing_left;
                    secondary_in.parent_pose = root->state.secondary.motion.pose;
                }
                else if (root->kind)
                    return SDL_SetError("Sword of Dawn projectile raw parent is not its soldier");
                if (w->dawn_bound)
                {
                    if (w->dawn_slot >= SOTN_SPELL_TARGETS)
                        return SDL_SetError("Sword of Dawn sword lost its actual target slot");
                    secondary_in.target = targets.slots[w->dawn_slot];
                }
            }
            secondary_in.engine_timer = in.engine_timer;
            secondary_in.retained_camera_x = in.camera_x - in.room_shift_x + in.screen_shift_x;
            secondary_in.retained_camera_y = in.camera_y - in.room_shift_y + in.screen_shift_y;
            // Weapon13 strikes and weapon24 rocks read their raw parent's step above.
            if (!strike && !(family == 24 && (c->kind & 15U) == 9))
                secondary_in.parent_phase = parent.parent_phase;
            secondary_in.parent_offset_x = parent.parent_offset_x;
            secondary_in.parent_quad = parent.parent_quad;
            secondary_in.room_left = in.camera_left;
            secondary_in.room_right = in.camera_right;
            for (Uint16 j = 0; j < 15; ++j)
            {
                const SotnBatRuntimeChild *o = &r.children[1 + j];
                // Only weapon-secondary occupants are compared: two-handed Sword of Dawn leaves
                // no other E2/F2 producer, except a throw still flying from before the equip.
                secondary_in.slot_kind[j] = o->weapon_secondary ? o->kind : 0;
                secondary_in.slot_x[j] =
                    o->weapon_secondary ? Word(Pixel(o->state.secondary.motion.x) - in.camera_x)
                                        : 0;
            }
            if (!SotnWeaponSecondary_Tick(w, &r.hand_program[hand], secondary_in, &progress.random,
                                          &events))
                return false;
            for (Uint16 j = 0; j < SDL_arraysize(events.sounds); ++j)
            {
                if (events.sounds[j] && !Sound(&r, events.sounds[j]))
                    return false;
                if (events.sounds[j] && !j && events.volume_sound)
                {
                    r.sound_volume_mask |= (Uint64)1 << (r.sound_count - 1U);
                    r.sound_volumes[r.sound_count - 1U] = events.volume;
                }
            }
            if (events.startup_ticks)
                r.startup_ticks = events.startup_ticks;
            // Weapon6 crescents and weapon14's root: DestroyEntityWeapon(true) (a raised shield
            // in slot16 retires) and g_Player.timers[10] = 4.
            if (events.clear_shield && !ClearHandShield(&r, &in.primitive_slots))
                return false;
            r.magic_clear_shield |= events.clear_shield;
            // Weapon24's spell holds the transform lockout (g_Player.unk20) at its value.
            if (events.lockout)
                r.transform_lockout = events.lockout;
            if (events.use_ticks)
                r.use_ticks = events.use_ticks;
            // Weapon30's circle creates its soldier factories in order, each moved to its own
            // scanned point; the callback already respected the free factory count.
            if (events.shake)
            {
                if (r.wolf_shake_count >= SDL_arraysize(r.wolf_shakes))
                    return SDL_SetError(
                        "Sword of Dawn exceeded the actual ordered shake request buffer");
                r.wolf_shakes[r.wolf_shake_count++] = events.shake;
            }
            // Shield spells: g_unkGraphicsStruct.unk28. Entering freeze1 makes the engine's
            // func_8010E0D0 create BlinkWhite0x47 and0x40 from the player, flagged0x10000.
            if (events.freeze_set || events.freeze_clear)
            {
                r.freeze_changed = true;
                r.freeze = events.freeze_set ? 1 : 0;
            }
            // Weapon52's spell: BlinkWhite FACTORY(44, n) from the spell entity.
            if (events.blink)
            {
                Uint16 j = 0;
                while (j < SOTN_BAT_RUNTIME_FACTORIES && r.factories[j].cursor.entity_id)
                    ++j;
                if (j < SOTN_BAT_RUNTIME_FACTORIES)
                {
                    SotnPlayerFactory made = {0};
                    if (!SotnPlayerFactory_Begin(
                            &made,
                            (SotnPlayerFactorySource){.x = w->x,
                                                      .y = w->y,
                                                      .flags = w->flags,
                                                      .priority = c->source.priority,
                                                      .facing_left = w->facing_left},
                            44U | (Uint32)events.blink << 16, 0))
                        return false;
                    SDL_zero(r.factories[j]);
                    r.factories[j].cursor = made;
                }
            }
            for (Uint16 b = 0; events.freeze_set && b < 2; ++b)
            {
                Uint16 j = 0;
                while (j < SOTN_BAT_RUNTIME_FACTORIES && r.factories[j].cursor.entity_id)
                    ++j;
                if (j == SOTN_BAT_RUNTIME_FACTORIES)
                    break;
                SotnPlayerFactory made = {0};
                if (!SotnPlayerFactory_Begin(
                        &made,
                        (SotnPlayerFactorySource){.x = p.x,
                                                  .y = p.y,
                                                  .flags = 0x10000U,
                                                  .priority = p.priority,
                                                  .facing_left = p.facing_left},
                        44U | (b ? 0x40U : 0x47U) << 16, 0))
                    return false;
                SDL_zero(r.factories[j]);
                r.factories[j].cursor = made;
            }
            // Weapon26's green cycle: one read-modify g_Clut word, committed after the pass.
            if (events.clut)
            {
                if (r.clut_count >= SDL_arraysize(r.clut))
                    return SDL_SetError("Weapon secondary exceeded its per-pass palette writes");
                r.clut[r.clut_count].palette = events.clut_palette;
                r.clut[r.clut_count].color = events.clut_color;
                r.clut[r.clut_count].mask = events.clut_mask;
                r.clut[r.clut_count++].value = events.clut_add;
            }
            if (events.select_target)
            {
                // func_80118970 marks the first unmarked candidate, shared with the spirits.
                Uint16 selected = SDL_MAX_UINT16;
                if (!SotnSpellTarget_Select(targets.slots, &r.target_cursor, &selected))
                    return false;
                w->dawn_slot = selected;
                w->dawn_bound = selected != SDL_MAX_UINT16;
                if (selected != SDL_MAX_UINT16 && in.spirit_targets &&
                    !(in.spirit_targets->slots[selected].flags & 0x80000U))
                {
                    if (!targets.generations[selected])
                        return SDL_SetError("Sword of Dawn selected a target without a generation");
                    r.target_marks[selected / 64U] |= (Uint64)1 << (selected % 64U);
                    r.target_generations[selected] = targets.generations[selected];
                    r.target_identities[selected] = targets.slots[selected].identity;
                }
            }
            for (Uint16 f = 0; f < events.factory_count; ++f)
            {
                // The rolling bomb's sparks are DRA's FACTORY(0x24,1) (Hellfire's high1 trail).
                const bool spark = family == 30 && events.factories[f] == 0x10024U;
                if (!spark && !SecondaryRequest(family, hand, events.factories[f]))
                    return SDL_SetError("Sword of Dawn requested an unselected factory");
                Uint16 j = 0;
                while (j < SOTN_BAT_RUNTIME_FACTORIES && r.factories[j].cursor.entity_id)
                    ++j;
                if (j == SOTN_BAT_RUNTIME_FACTORIES)
                    return SDL_SetError("Sword of Dawn soldier factory lost its free entity");
                SotnPlayerFactory made = {0};
                if (!SotnPlayerFactory_Begin(
                        &made,
                        (SotnPlayerFactorySource){.x = events.factory_x[f],
                                                  .y = events.factory_y[f],
                                                  .flags = w->flags,
                                                  .priority = c->source.priority,
                                                  .facing_left = w->facing_left},
                        events.factories[f], 0))
                    return false;
                SDL_zero(r.factories[j]);
                r.factories[j].cursor = made;
                r.factories[j].secondary_factory = !spark;
                r.factories[j].secondary_parent_slot = spark ? 0 : (Uint16)(i + 16U);
            }
            if (events.pink)
            {
                // FACTORY(55, n) from the spell: the EntityPlayerPinkEffect profile n, after the
                // spell's factories (Herald's swirl precedes its pink9 in the same update).
                Uint16 j = 0;
                while (j < SOTN_BAT_RUNTIME_FACTORIES && r.factories[j].cursor.entity_id)
                    ++j;
                if (j < SOTN_BAT_RUNTIME_FACTORIES)
                {
                    SotnPlayerFactory made = {0};
                    if (!SotnPlayerFactory_Begin(
                            &made,
                            (SotnPlayerFactorySource){.x = w->x,
                                                      .y = w->y,
                                                      .flags = w->flags,
                                                      .priority = c->source.priority,
                                                      .facing_left = w->facing_left},
                            55U | (Uint32)events.pink << 16, 0))
                        return false;
                    SDL_zero(r.factories[j]);
                    r.factories[j].cursor = made;
                }
            }
            if (events.chain)
            {
                // A weapon53 root requests its next chained slash from its own entity.
                for (Uint16 j = 0; j < SOTN_BAT_RUNTIME_FACTORIES; ++j)
                    if (!r.factories[j].cursor.entity_id)
                    {
                        SotnPlayerFactory f = {0};
                        if (!SotnPlayerFactory_Begin(
                                &f,
                                (SotnPlayerFactorySource){
                                    .x = w->x,
                                    .y = w->y,
                                    .flags = w->flags,
                                    // Weapon7's orb factory copies its
                                    // own zPriority0xA8.
                                    .priority = family == 7 ? w->priority : c->source.priority,
                                    .facing_left = w->facing_left},
                                events.chain, 0))
                            return false;
                        SDL_zero(r.factories[j]);
                        r.factories[j].cursor = f;
                        r.factories[j].secondary_factory = true;
                        r.factories[j].secondary_parent_slot = (Uint16)(i + 16U);
                        break;
                    }
            }
            if (events.group)
            {
                // Weapon49 sparks call func_80118894 alone: the least-used group stays theirs.
                Uint16 group = i < 16 ? 3 : 7;
                for (Uint16 g = (Uint16)(group + 1U); g < (i < 16 ? 7 : 11); ++g)
                    if (r.groups[g] < r.groups[group])
                        group = g;
                if (r.groups[group] == 255)
                    return SDL_SetError("Weapon secondary shared group count overflow");
                ++r.groups[group];
                c->group = (Uint8)group;
                // Weapon30 soldiers and weapon7's orb keep their factory's zero equip id.
                if (family != 30 && family != 7)
                    c->state.secondary.item = item;
            }
            if (events.properties)
            {
                // SetWeaponProperties increments the least-used group before the copy of the
                // raw parent's enemyId; end-of-pass recount removes that transient count.
                // func_80118894 gives slot16 (the punch rush) its hand group without a count;
                // weapon6's crescents keep their least-used group (no parent enemyId copy).
                if (!i)
                    c->group = (Uint8)(hand + 1U);
                else if (c->state.secondary.own_item)
                {
                    Uint16 group = 3;
                    for (Uint16 g = 4; g < 7; ++g)
                        if (r.groups[g] < r.groups[group])
                            group = g;
                    if (r.groups[group] == 255)
                        return SDL_SetError("Weapon secondary shared group count overflow");
                    ++r.groups[group];
                    // Yasutsuna's root then copies its raw parent's enemyId: the player entity's,
                    // which no DRA code writes (zero).
                    c->group = family == 53 ? 0 : (Uint8)group;
                }
                else
                {
                    Uint16 group = i < 16 ? 3 : 7;
                    for (Uint16 g = (Uint16)(group + 1U); g < (i < 16 ? 7 : 11); ++g)
                        if (r.groups[g] < r.groups[group])
                            group = g;
                    if (r.groups[group] == 255)
                        return SDL_SetError("Weapon secondary shared group count overflow");
                    ++r.groups[group];
                    // Weapon11's walker keeps func_80118894's least-used group (no parent copy).
                    // Weapon10's swords call SetWeaponProperties without an enemyId copy too.
                    // Weapon24's rocks too.
                    c->group =
                        family == 11 || family == 10 || family == 24 ? (Uint8)group : parent_group;
                    c->state.secondary.item = item;
                }
            }
            // Shield spells lack CAMERA_LOCKED: like the warp they keep their screen position
            // after the camera work (Axe Lord's outlives its freeze by twenty updates).
            // Weapon13's combo sweep and strike swords lack it too.
            if (((SotnWeaponSecondary_ShieldSpell(family) &&
                  ((c->kind & 15U) == 8 || family == 25 || family == 26 || family == 52 ||
                   family == 23 || family == 24)) ||
                 sweep || strike ||
                 // Weapon10's swords until their delay makes them CAMERA_LOCKED.
                 (family == 10 && (c->kind & 15U) == 9 && !(w->flags & 0x08000000U))) &&
                !w->removed)
            {
                w->x = FixedShift(w->x, Word(in.room_shift_x - in.screen_shift_x));
                w->y = FixedShift(w->y, Word(in.room_shift_y - in.screen_shift_y));
            }
            c->source.x = w->x;
            c->source.y = w->y;
            c->source.facing_left = w->facing_left;
            const SotnEquipment e = w->properties;
            c->state.secondary.attack = (SotnPlayerAttack){
                .offset_x = w->hitbox_x,
                .offset_y = w->hitbox_y,
                .width = w->hitbox_width,
                .height = w->hitbox_height,
                .power = (Uint16)e.attack,
                .element = e.element,
                .hit_type = w->hitbox_state,
                .hit_effect = e.hit_effect,
                .stun_frames = e.stun,
                .invincibility_frames = e.invincibility,
                .critical_rate = (Uint8)e.critical_rate,
                .equipment = c->state.secondary.item,
                .right_hand = hand != 0,
                .active = w->hitbox_state && w->hitbox_width && w->hitbox_height};
            const Sint32 x = Pixel(w->x) - in.camera_x, y = Pixel(w->y) - in.camera_y;
            // UpdatePlayerEntities keeps FLAG_KEEP_ALIVE_OFFCAMERA children off-screen.
            removed = w->removed ||
                      (!(w->flags & 0x4000000U) && (x > 288 || x < -32 || y > 256 || y < -16));
            if (!removed && (w->flags & 0x100000U) &&
                !SotnWeaponSecondary_Animate(w, overlay, family))
                return false;
            c->initialized = !removed;
            if (!removed && c->state.secondary.attack.active)
                r.secondary_mask |= (Uint64)1 << i;
        }
        else if (c->karma)
        {
            const bool child = (c->kind & 15U) == 2;
            const Uint16 hand = c->kind >= 0xf0;
            const bool cold = !c->initialized;
            SotnKarmaCoin *n = &c->state.karma.motion;
            bool allocated = c->primitive_count != 0;
            if (!child && cold &&
                !SotnPrimitiveSlots_Reserve(&in.primitive_slots, SOTN_KARMA_ROWS, c->primitives,
                                            &allocated))
                return false;
            if (!child)
                c->primitive_count = allocated ? SOTN_KARMA_ROWS : 0;
            SotnMagicMissileParent parent = {0};
            if (child && !MagicParent(&r, c->state.karma.parent_slot, &parent))
                return false;
            SotnDataView overlay = {0};
            if (!SotnWeapon_Overlay(in.weapons, (SotnEquipment){.id = 24, .family = 20}, &overlay))
                return false;
            SotnKarmaEvents events = {0};
            if (!SotnKarmaCoin_Tick(n, &r.karma[hand],
                                    (SotnKarmaInput){.overlay = overlay,
                                                     .executable = in.executable,
                                                     .collision = in.food_collision,
                                                     .game_timer = in.game_timer,
                                                     .priority = p.priority,
                                                     .hand = hand,
                                                     .facing_left = p.facing_left,
                                                     .allocated = allocated,
                                                     .child = child,
                                                     .parent_exists = parent.exists,
                                                     .bank = parent.bank,
                                                     .texture = parent.texture},
                                    &progress.random, &events))
                return false;
            const Sint16 dx = Word(in.room_shift_x - in.screen_shift_x),
                         dy = Word(in.room_shift_y - in.screen_shift_y),
                         old_camera_x = Word(in.camera_x - dx),
                         old_camera_y = Word(in.camera_y - dy);
            if ((child && cold && !n->removed) || events.fixed_properties)
            {
                // These callbacks replace only high words with original screen anchors.
                n->x = FixedShift(n->x, old_camera_x);
                n->y = FixedShift(n->y, old_camera_y);
            }
            if (!(n->flags & 0x08000000U))
            {
                n->x = FixedShift(n->x, dx);
                n->y = FixedShift(n->y, dy);
            }
            if (events.properties || events.fixed_properties)
            {
                SotnEquipment equipment = {.attack = 4,
                                           .element = 0x20,
                                           .hit_type = 0x82,
                                           .hit_effect = 2,
                                           .stun = 4,
                                           .invincibility = 80};
                if (events.properties &&
                    !SotnCharacter_ThrowingProperties(character, 24, p.poison_ticks != 0,
                                                      &progress.random, &equipment))
                    return false;
                Uint16 group = 3;
                for (Uint16 g = 4; g < 7; ++g)
                    if (r.groups[g] < r.groups[group])
                        group = g;
                if (r.groups[group] == 255)
                    return SDL_SetError("Karma shared attack group count overflow");
                ++r.groups[group];
                c->group = (Uint8)group;
                c->state.karma.attack =
                    (SotnPlayerAttack){.power = (Uint16)equipment.attack,
                                       .element = equipment.element,
                                       .hit_type = equipment.hit_type,
                                       .hit_effect = equipment.hit_effect,
                                       .stun_frames = equipment.stun,
                                       .invincibility_frames = equipment.invincibility,
                                       .critical_rate = (Uint8)equipment.critical_rate,
                                       .equipment = 24,
                                       .right_hand = hand != 0,
                                       .active = true};
            }
            c->state.karma.attack.offset_x = n->hitbox_x;
            c->state.karma.attack.offset_y = n->hitbox_y;
            c->state.karma.attack.width = n->hitbox_width;
            c->state.karma.attack.height = n->hitbox_height;
            if (events.flash && !Flash(&r, in.dra, events.flash))
                return false;
            if (events.use_ticks)
                r.use_ticks = events.use_ticks;
            if (events.protect_ticks)
                r.spell_use_ticks = events.protect_ticks;
            if (events.clear_shield && !ClearHandShield(&r, &in.primitive_slots))
                return false;
            r.magic_clear_shield |= events.clear_shield;
            if (events.sound)
            {
                if (!Sound(&r, events.sound))
                    return false;
                if (events.volume_sound)
                {
                    r.sound_volume_mask |= (Uint64)1 << (r.sound_count - 1U);
                    r.sound_volumes[r.sound_count - 1U] = events.volume;
                }
            }
            c->source = (SotnPlayerFactorySource){.x = n->x,
                                                  .y = n->y,
                                                  .flags = n->flags,
                                                  .priority = n->priority,
                                                  .facing_left = n->facing_left};
            if (events.factory && !KarmaFactory(&r, c->source, events.factory, (Uint16)(i + 16U)))
                return false;
            const Sint16 x = Word(Pixel(n->x) - in.camera_x), y = Word(Pixel(n->y) - in.camera_y);
            removed = n->removed ||
                      (!(n->flags & 0x04000000U) && (x < -32 || x > 288 || y < -16 || y > 256));
            if (!removed && (n->flags & 0x100000U) && !SotnKarmaCoin_Animate(n, overlay))
                return false;
            c->initialized = !removed;
            if (!removed && c->state.karma.attack.active)
                r.karma_mask |= (Uint64)1 << i;
        }
        else if (c->star)
        {
            const Uint16 role = c->kind & 15U, hand = c->kind >= 0xf0;
            SotnStarWeapon *n = &c->state.star.motion;
            SotnThrownWeapon *s = &n->motion;
            bool allocated = c->primitive_count != 0;
            if (role == 3 && !c->initialized &&
                !SotnPrimitiveSlots_Reserve(&in.primitive_slots, 32, c->primitives, &allocated))
                return false;
            if (role == 3)
                c->primitive_count = allocated ? 32 : 0;
            SotnStarParent parent = {0};
            if ((role == 1 || role == 4 || role == 5) && !s->phase &&
                !StarParent(&r, c->state.star.parent_slot, &parent))
                return false;
            SotnDataView overlay = {0};
            if (!SotnWeapon_Overlay(in.weapons, (SotnEquipment){.id = 75, .family = 15}, &overlay))
                return false;
            SotnStarEvents events = {0};
            if (!SotnStarWeapon_Tick(n,
                                     (SotnStarInput){.role = (SotnStarRole)role,
                                                     .executable = in.executable,
                                                     .overlay = overlay,
                                                     .collision = in.food_collision,
                                                     .parent = parent,
                                                     .player_x = p.x,
                                                     .player_y = p.y,
                                                     .offset_x = p.animation.hitbox_x,
                                                     .offset_y = p.animation.hitbox_y,
                                                     .remaining = c->state.star.animation.remaining,
                                                     .game_timer = in.game_timer,
                                                     .player_step = (Uint16)p.step,
                                                     .priority = p.priority,
                                                     .hand = hand,
                                                     .buttons = p.buttons,
                                                     .facing_left = p.facing_left,
                                                     .primitive_allocated = allocated},
                                     &progress.random, &r.star_fire_counter[hand], &events))
                return false;
            if (events.effects.properties || events.fixed_tnt_properties)
            {
                SotnEquipment equipment = {.attack = 1,
                                           .element = 0x20,
                                           .hit_type = 2,
                                           .hit_effect = 1,
                                           .stun = 4,
                                           .invincibility = 20};
                if (events.effects.properties &&
                    !SotnCharacter_MagicMissileProperties(character, n->item, p.poison_ticks != 0,
                                                          &progress.random, &equipment))
                    return false;
                Uint16 group = 3;
                for (Uint16 j = 4; j < 7; ++j)
                    if (r.groups[j] < r.groups[group])
                        group = j;
                if (r.groups[group] == 255)
                    return SDL_SetError("Weapon15 shared property group overflow");
                ++r.groups[group];
                // Original property assignment increments the least-used group before the
                // explosion overwrites enemyId with its raw parent's current group.
                if (events.inherit_group)
                    group = parent.group;
                if (group != 0 && (group < 3 || group > 10))
                    return SDL_SetError(
                        "Weapon15 requires its actual selected raw parent attack group");
                c->group = (Uint8)group;
                c->state.star.attack =
                    (SotnPlayerAttack){.power = (Uint16)equipment.attack,
                                       .element = equipment.element,
                                       .hit_type = equipment.hit_type,
                                       .hit_effect = equipment.hit_effect,
                                       .stun_frames = equipment.stun,
                                       .invincibility_frames = equipment.invincibility,
                                       .critical_rate = (Uint8)equipment.critical_rate,
                                       .equipment = n->item,
                                       .right_hand = hand != 0,
                                       .active = true};
            }
            c->state.star.attack.width = s->width;
            c->state.star.attack.height = s->height;
            if (n->attack_cleared)
                c->state.star.attack.active = false;
            for (Uint16 j = 0; j < events.effects.factory_count; ++j)
                if (!StarFactory(&r, events.effects.factories[j], (Uint16)(i + 16U), hand))
                    return false;
            for (Uint16 j = 0; j < events.effects.sound_count; ++j)
                if (!Sound(&r, events.effects.sounds[j]))
                    return false;
            for (Uint16 j = 0; j < events.effects.shake_count; ++j)
            {
                if (r.wolf_shake_count >= SDL_arraysize(r.wolf_shakes))
                    return SDL_SetError("Weapon15 exceeded actual bounded shake requests");
                r.wolf_shakes[r.wolf_shake_count++] = events.effects.shakes[j];
            }
            if (events.effects.use_ticks)
                r.use_ticks = events.effects.use_ticks;
            if (events.effects.clear_shield && !ClearHandShield(&r, &in.primitive_slots))
                return false;
            r.magic_clear_shield |= events.effects.clear_shield;
            if (events.animation_offset)
            {
                c->state.star.animation_offset = events.animation_offset;
                c->state.star.animation = (SotnThrownAnimation){0};
            }
            const Sint16 dx = Word(in.room_shift_x - in.screen_shift_x),
                         dy = Word(in.room_shift_y - in.screen_shift_y);
            const Sint16 x = Word(Pixel(s->x) - in.camera_x + dx),
                         y = Word(Pixel(s->y) - in.camera_y + dy);
            removed = s->removed ||
                      (!(s->flags & 0x04000000U) && (x < -32 || x > 288 || y < -16 || y > 256));
            if (c->state.star.animation_offset && !removed)
            {
                if (!SotnStarWeapon_Animate(&c->state.star.animation, overlay,
                                            c->state.star.animation_offset))
                    return false;
                s->frame = c->state.star.animation.frame;
            }
            c->source = (SotnPlayerFactorySource){.x = s->x,
                                                  .y = s->y,
                                                  .flags = s->flags,
                                                  .priority = s->priority,
                                                  .facing_left = s->facing_left};
            c->initialized = !removed;
            if (!removed && c->state.star.attack.active)
                r.star_mask |= (Uint64)1 << i;
        }
        else if (c->thrown)
        {
            const Uint16 role = c->kind & 15U, hand = c->kind >= 0xf0;
            SotnThrownWeapon *s = &c->state.thrown.motion;
            bool allocated = c->primitive_count != 0;
            if (role == 0 && !allocated &&
                !SotnPrimitiveSlots_Reserve(&in.primitive_slots, 1, c->primitives, &allocated))
                return false;
            if (role == 0)
                c->primitive_count = allocated ? 1 : 0;
            SotnThrownParent parent = {0};
            Sint32 ignored_velocity = 0;
            if (role == 1 && !s->phase &&
                !ThrownParent(&r, c->state.thrown.parent_slot, &parent, &ignored_velocity))
                return false;
            SotnThrownEvents events = {0};
            if (!SotnThrownWeapon_Tick(
                    s,
                    (SotnThrownInput){.role = (SotnThrownRole)role,
                                      .executable = in.executable,
                                      .collision = in.food_collision,
                                      .parent = parent,
                                      .player_x = p.x,
                                      .player_y = p.y,
                                      .offset_x = p.animation.hitbox_x,
                                      .offset_y = p.animation.hitbox_y,
                                      .player_step = (Uint16)p.step,
                                      .priority = p.priority,
                                      .hand = hand,
                                      .facing_left = p.facing_left,
                                      .primitive_allocated = allocated,
                                      .remaining = c->state.thrown.animation.remaining},
                    &progress.random, &r.thrown_fire_counter[hand], &events))
                return false;
            if (events.properties)
            {
                SotnEquipment equipment = {0};
                if (!SotnCharacter_ThrowingProperties(character, c->state.thrown.item,
                                                      p.poison_ticks != 0, &progress.random,
                                                      &equipment))
                    return false;
                Uint16 group = 3;
                for (Uint16 j = 4; j < 7; ++j)
                    if (r.groups[j] < r.groups[group])
                        group = j;
                if (r.groups[group] == 255)
                    return SDL_SetError("Weapon16 shared attack group overflow");
                ++r.groups[group];
                c->group = (Uint8)group;
                c->state.thrown.attack =
                    (SotnPlayerAttack){.power = (Uint16)equipment.attack,
                                       .element = equipment.element,
                                       .hit_type = equipment.hit_type,
                                       .hit_effect = equipment.hit_effect,
                                       .stun_frames = equipment.stun,
                                       .invincibility_frames = equipment.invincibility,
                                       .critical_rate = (Uint8)equipment.critical_rate,
                                       .equipment = c->state.thrown.item,
                                       .right_hand = hand != 0,
                                       .active = true};
            }
            c->state.thrown.attack.width = s->width;
            c->state.thrown.attack.height = s->height;
            for (Uint16 j = 0; j < events.factory_count; ++j)
                if (!ThrownFactory(&r, events.factories[j], (Uint16)(i + 16U), hand))
                    return false;
            for (Uint16 j = 0; j < events.sound_count; ++j)
                if (!Sound(&r, events.sounds[j]))
                    return false;
            for (Uint16 j = 0; j < events.shake_count; ++j)
            {
                if (r.wolf_shake_count >= SDL_arraysize(r.wolf_shakes))
                    return SDL_SetError("Weapon16 exceeded actual bounded shared shake requests");
                r.wolf_shakes[r.wolf_shake_count++] = events.shakes[j];
            }
            if (events.use_ticks)
                r.use_ticks = events.use_ticks;
            if (events.clear_shield && !ClearHandShield(&r, &in.primitive_slots))
                return false;
            r.magic_clear_shield |= events.clear_shield;
            if (events.fire_animation)
                c->state.thrown.animation = (SotnThrownAnimation){0};
            const Sint16 dx = Word(in.room_shift_x - in.screen_shift_x),
                         dy = Word(in.room_shift_y - in.screen_shift_y);
            const Sint16 x = Word(Pixel(s->x) - in.camera_x + dx),
                         y = Word(Pixel(s->y) - in.camera_y + dy);
            removed = s->removed ||
                      (!(s->flags & 0x04000000U) && (x < -32 || x > 288 || y < -16 || y > 256));
            if (role == 4 && !removed)
            {
                SotnDataView overlay = {0};
                if (!SotnWeapon_Overlay(in.weapons, (SotnEquipment){.id = 134, .family = 16},
                                        &overlay) ||
                    !SotnThrownWeapon_AnimateFire(&c->state.thrown.animation, overlay))
                    return false;
                s->frame = c->state.thrown.animation.frame;
            }
            c->source = (SotnPlayerFactorySource){.x = s->x,
                                                  .y = s->y,
                                                  .flags = s->flags,
                                                  .priority = s->priority,
                                                  .facing_left = s->facing_left};
            c->initialized = !removed;
            if (!removed && c->state.thrown.attack.active)
                r.thrown_mask |= (Uint64)1 << i;
        }
        else if (c->magic)
        {
            const bool arrow = c->kind == 0xe1 || c->kind == 0xf1;
            const Uint16 hand = c->kind >= 0xf0 ? 1 : 0;
            SotnMagicMissile *m = &c->state.magic.motion;
            SotnMagicMissileParent parent = {0};
            if (arrow && !m->phase && !MagicParent(&r, c->state.magic.parent_slot, &parent))
                return false;
            SotnMagicMissileEvents events = {0};
            if (!SotnMagicMissile_Tick(
                    m,
                    (SotnMagicMissileInput){.arrow = arrow,
                                            .hand = hand,
                                            .priority = p.priority,
                                            .facing_left = p.facing_left,
                                            .pose = c->state.magic.animation.pose,
                                            .remaining = c->state.magic.animation.remaining,
                                            .hit_flags = c->state.magic.hit_flags,
                                            .parent = parent},
                    &events))
                return false;
            if (events.properties)
            {
                SotnEquipment equipment = {0};
                if (!SotnCharacter_MagicMissileProperties(character, m->item, p.poison_ticks != 0,
                                                          &progress.random, &equipment))
                    return false;
                Uint16 group = 3;
                for (Uint16 g = 4; g < 7; ++g)
                    if (r.groups[g] < r.groups[group])
                        group = g;
                if (r.groups[group] == 255)
                    return SDL_SetError("Magic Missile shared attack group overflow");
                ++r.groups[group];
                c->group = (Uint8)group;
                c->state.magic.attack =
                    (SotnPlayerAttack){.offset_x = 8,
                                       .width = 8,
                                       .height = 4,
                                       .power = (Uint16)equipment.attack,
                                       .element = equipment.element,
                                       .hit_type = equipment.hit_type,
                                       .hit_effect = equipment.hit_effect,
                                       .stun_frames = equipment.stun,
                                       .invincibility_frames = equipment.invincibility,
                                       .critical_rate = (Uint8)equipment.critical_rate,
                                       .equipment = m->item,
                                       .right_hand = hand != 0,
                                       .active = true};
            }
            if (events.factory)
            {
                bool allocated = false;
                if (!MagicFactory(&r, c->source, (Uint16)(i + 16U), hand, &allocated) ||
                    (allocated && !Sound(&r, 0x625)))
                    return false;
            }
            if (events.sound && !Sound(&r, events.sound))
                return false;
            if (events.use_ticks)
                r.use_ticks = events.use_ticks;
            if (events.clear_shield && !ClearHandShield(&r, &in.primitive_slots))
                return false;
            r.magic_clear_shield |= events.clear_shield;
            if (events.animation)
            {
                c->state.magic.timeline = events.animation;
                c->state.magic.animation.pose = 0;
                c->state.magic.animation.remaining = 0;
            }
            removed = m->removed;
            if (!removed)
            {
                const Sint16 dx = Word(in.room_shift_x - in.screen_shift_x),
                             dy = Word(in.room_shift_y - in.screen_shift_y);
                const Sint16 x = Word(Pixel(m->x) - in.camera_x + dx),
                             y = Word(Pixel(m->y) - in.camera_y + dy);
                removed = x < -32 || x > 288 || y < -16 || y > 256;
                // UpdatePlayerEntities culls after the callback and before UpdateAnim.
                if (!removed)
                {
                    SotnDataView overlay = {0};
                    if (!SotnWeapon_Overlay(in.weapons, (SotnEquipment){.id = 25, .family = 22},
                                            &overlay) ||
                        !SotnMagicMissile_Animate(&c->state.magic.animation, overlay,
                                                  c->state.magic.timeline))
                        return false;
                }
            }
            c->source.x = m->x;
            c->source.y = m->y;
            c->source.flags = m->flags;
            c->source.priority = m->priority;
            c->source.facing_left = m->facing_left;
            c->initialized = !removed;
            if (!removed && c->state.magic.attack.active)
                r.magic_mask |= (Uint64)1 << i;
        }
        else if (c->pentagram)
        {
            const Uint16 hand = c->kind == 0xf0 ? 1 : 0;
            SotnPentagram *n = &c->state.pentagram.motion;
            const bool cold = !n->phase;
            bool allocated = c->primitive_count != 0;
            if (!allocated && !SotnPrimitiveSlots_Reserve(&in.primitive_slots, SOTN_PENTAGRAM_ROWS,
                                                          c->primitives, &allocated))
                return false;
            c->primitive_count = allocated ? SOTN_PENTAGRAM_ROWS : 0;
            SotnDataView overlay = {0};
            if (!SotnWeapon_Overlay(in.weapons,
                                    (SotnEquipment){.id = c->state.pentagram.item, .family = 17},
                                    &overlay))
                return false;
            SotnPentagramEvents events = {0};
            if (!SotnPentagram_Tick(n, &r.pentagram[hand],
                                    (SotnPentagramInput){.overlay = overlay,
                                                         .executable = in.executable,
                                                         .game_timer = in.game_timer,
                                                         .parameter = c->parameter,
                                                         .hand = hand,
                                                         .allocated = allocated},
                                    &progress.random, &events))
                return false;
            if (events.properties)
            {
                SotnEquipment equipment = {0};
                if (!SotnCharacter_ThrowingProperties(character, c->state.pentagram.item,
                                                      p.poison_ticks != 0, &progress.random,
                                                      &equipment))
                    return false;
                Uint16 group = 3;
                for (Uint16 g = 4; g < 7; ++g)
                    if (r.groups[g] < r.groups[group])
                        group = g;
                if (r.groups[group] == 255)
                    return SDL_SetError("Pentagram shared group count overflow");
                ++r.groups[group];
                c->group = (Uint8)group;
                c->state.pentagram.attack =
                    (SotnPlayerAttack){.power = (Uint16)equipment.attack,
                                       .element = equipment.element,
                                       .hit_type = equipment.hit_type,
                                       .hit_effect = equipment.hit_effect,
                                       .stun_frames = equipment.stun,
                                       .invincibility_frames = equipment.invincibility,
                                       .critical_rate = (Uint8)equipment.critical_rate,
                                       .equipment = equipment.id,
                                       .right_hand = hand != 0,
                                       .active = true};
            }
            if (events.clear_attack)
                c->state.pentagram.attack.active = false;
            if (events.use_ticks)
                r.spell_use_ticks = events.use_ticks;
            if (events.flash && !Flash(&r, in.dra, events.flash))
                return false;
            if (events.shake)
            {
                if (r.wolf_shake_count >= SDL_arraysize(r.wolf_shakes))
                    return SDL_SetError(
                        "Pentagram exceeded the actual ordered shake request buffer");
                r.wolf_shakes[r.wolf_shake_count++] = events.shake;
            }
            if (events.sound && !Sound(&r, events.sound))
                return false;
            const Sint16 dx = Word(in.room_shift_x - in.screen_shift_x),
                         dy = Word(in.room_shift_y - in.screen_shift_y);
            // The original birth anchor is screen128/104; subsequent callbacks keep that screen
            // point.
            if (cold && !n->removed)
            {
                n->x = FixedShift(n->x, Word(in.camera_x - dx));
                n->y = FixedShift(n->y, Word(in.camera_y - dy));
            }
            n->x = FixedShift(n->x, dx);
            n->y = FixedShift(n->y, dy);
            c->source.x = n->x;
            c->source.y = n->y;
            c->source.flags = n->flags;
            removed = n->removed;
            c->initialized = !removed;
            if (!removed && c->state.pentagram.attack.active)
                r.pentagram_mask |= (Uint64)1 << i;
        }
        else if (c->sire)
        {
            const Uint16 hand = c->kind >= 0xf0 ? 1 : 0;
            const Sint16 camera_delta_x = Word(in.room_shift_x - in.screen_shift_x),
                         camera_delta_y = Word(in.room_shift_y - in.screen_shift_y),
                         old_camera_x = Word(in.camera_x - camera_delta_x),
                         old_camera_y = Word(in.camera_y - camera_delta_y);
            Uint8 timeline;
            if (c->sire_child)
            {
                const SotnBatRuntimeChild *parent = &r.children[c->state.sire.parent_slot - 16U];
                const bool actual_parent = parent->sire && !parent->sire_child;
                SotnSireChildEvents events = {0};
                if (!SotnSireChild_Tick(
                        &c->state.sire.child,
                        (SotnSireChildInput){
                            .executable = in.executable,
                            .parent_item = actual_parent ? 72 : 0,
                            .parent_palette = actual_parent ? parent->state.sire.parent.palette : 0,
                            .parent_texture = actual_parent ? parent->state.sire.parent.texture : 0,
                            .priority = p.priority,
                            .hand = hand},
                        &progress.random, &events))
                    return false;
                if (events.sound && !Sound(&r, events.sound))
                    return false;
                const SotnSireChild *n = &c->state.sire.child;
                c->source.x = n->x;
                c->source.y = n->y;
                c->source.flags = n->flags;
                c->source.priority = n->priority;
                c->source.facing_left = n->facing_left;
                const Sint16 x = Word(Pixel(n->x) - old_camera_x),
                             y = Word(Pixel(n->y) - old_camera_y);
                removed = n->removed || x > 288 || x < -32 || y > 256 || y < -16;
                timeline = n->animation;
            }
            else
            {
                SotnSireParent *n = &c->state.sire.parent;
                bool allocated = false;
                if (n->phase == 2 && n->lifetime >= 9 && n->lifetime % 8 == 0 &&
                    !SireFactory(&r, c->source, (Uint16)(i + 16U), hand, camera_delta_x,
                                 camera_delta_y, &allocated))
                    return false;
                SotnSireParentEvents events = {0};
                if (!SotnSireParent_Tick(
                        n,
                        (SotnSireParentInput){.player_x = p.x,
                                              .player_y = p.y,
                                              .remaining = c->state.sire.animation.remaining,
                                              .priority = p.priority,
                                              .hand = hand,
                                              .factory_allocated = allocated},
                        &events))
                    return false;
                c->state.sire.attack.active = false;
                if (events.properties >= 0)
                {
                    SotnEquipment equipment = {0};
                    if (!SotnCharacter_SireProperties(character, p.poison_ticks != 0,
                                                      &progress.random, &equipment))
                        return false;
                    Uint16 group = 3;
                    for (Uint16 g = 4; g < 7; ++g)
                        if (r.groups[g] < r.groups[group])
                            group = g;
                    if (r.groups[group] == 255)
                        return SDL_SetError("Sire shared group count overflow");
                    ++r.groups[group]; // Keep the old identity counted until the original recount.
                    c->group = (Uint8)group;
                    c->state.sire.attack =
                        (SotnPlayerAttack){.power = (Uint16)equipment.attack,
                                           .element = equipment.element,
                                           .hit_type = equipment.hit_type,
                                           .hit_effect = equipment.hit_effect,
                                           .stun_frames = equipment.stun,
                                           .invincibility_frames = equipment.invincibility,
                                           .critical_rate = (Uint8)equipment.critical_rate,
                                           .equipment = 72,
                                           .right_hand = hand != 0,
                                           .active = true};
                }
                if (events.reset_pose)
                    c->state.sire.animation = (SotnSireAnimation){0};
                if (events.use_ticks)
                    r.spell_use_ticks = events.use_ticks;
                if (events.flash && !Flash(&r, in.dra, events.flash))
                    return false;
                if (events.shake)
                {
                    if (r.wolf_shake_count >= SDL_arraysize(r.wolf_shakes))
                        return SDL_SetError(
                            "Sire exceeded the actual ordered shake request buffer");
                    r.wolf_shakes[r.wolf_shake_count++] = events.shake;
                }
                if (events.sound && !Sound(&r, events.sound))
                    return false;
                if (events.particle_factory &&
                    !RequestSource(&r,
                                   (SotnPlayerFactorySource){.x = FixedShift(n->x, camera_delta_x),
                                                             .y = FixedShift(n->y, camera_delta_y),
                                                             .priority = n->priority},
                                   0xc0004, 0))
                    return false;
                n->x = FixedShift(n->x, camera_delta_x);
                n->y = FixedShift(n->y, camera_delta_y);
                c->source.x = n->x;
                c->source.y = n->y;
                c->source.flags = n->flags;
                c->source.priority = n->priority;
                c->source.facing_left = n->facing_left;
                removed = n->removed;
                timeline = n->animation;
                if (!removed && c->state.sire.attack.active)
                    r.sire_mask |= (Uint64)1 << i;
            }
            if (!removed)
            {
                SotnDataView overlay = {0};
                if (!SotnWeapon_Overlay(in.weapons, (SotnEquipment){.id = 72, .family = 18},
                                        &overlay) ||
                    !SotnSire_Animate(&c->state.sire.animation, overlay, timeline))
                    return false;
            }
            c->initialized = !removed;
        }
        else if (c->neutron)
        {
            bool allocated = c->primitive_count != 0;
            if (!allocated &&
                !SotnPrimitiveSlots_Reserve(&in.primitive_slots, 1, c->primitives, &allocated))
                return false;
            c->primitive_count = allocated ? 1 : 0;
            SotnNeutronEvents events = {0};
            if (!SotnNeutron_Tick(
                    &c->state.neutron.motion,
                    (SotnNeutronInput){.player_x = p.x,
                                       .player_y = p.y,
                                       .priority = p.priority,
                                       .pose = c->state.neutron.animation.pose,
                                       .remaining = c->state.neutron.animation.remaining,
                                       .allocated = allocated},
                    &events))
                return false;
            if (events.properties)
            {
                SotnEquipment equipment = {0};
                if (!SotnCharacter_NeutronProperties(character, p.poison_ticks != 0,
                                                     &progress.random, &equipment))
                    return false;
                Uint16 group = 3;
                for (Uint16 g = 4; g < 7; ++g)
                    if (r.groups[g] < r.groups[group])
                        group = g;
                if (r.groups[group] == 255)
                    return SDL_SetError("Neutron shared group count overflow");
                ++r.groups[group];
                c->group = (Uint8)group;
                c->state.neutron.attack =
                    (SotnPlayerAttack){.power = (Uint16)equipment.attack,
                                       .element = equipment.element,
                                       .hit_type = equipment.hit_type,
                                       .hit_effect = equipment.hit_effect,
                                       .stun_frames = equipment.stun,
                                       .invincibility_frames = equipment.invincibility,
                                       .critical_rate = (Uint8)equipment.critical_rate,
                                       .equipment = 71,
                                       .right_hand = c->kind == 0xf0,
                                       .active = true};
            }
            if (events.disable_attack)
                c->state.neutron.attack.active = false;
            if (events.use_ticks)
                r.spell_use_ticks = events.use_ticks;
            for (Uint16 j = 0; j < 2; ++j)
                if (events.sounds[j] && !Sound(&r, events.sounds[j]))
                    return false;
            SotnNeutron *n = &c->state.neutron.motion;
            removed = n->removed;
            if (!removed && (n->flags & 0x100000U))
            {
                SotnDataView overlay = {0};
                if (!SotnWeapon_Overlay(in.weapons, (SotnEquipment){.id = 71, .family = 19},
                                        &overlay) ||
                    !SotnNeutron_Animate(&c->state.neutron.animation, overlay))
                    return false;
            }
            const Sint16 camera_delta_x = Word(in.room_shift_x - in.screen_shift_x),
                         camera_delta_y = Word(in.room_shift_y - in.screen_shift_y),
                         old_camera_x = Word(in.camera_x - camera_delta_x),
                         old_camera_y = Word(in.camera_y - camera_delta_y);
            for (Uint16 v = 0; v < 4; ++v)
            {
                n->halo.x[v] = Word(n->halo.x[v] - old_camera_x);
                n->halo.y[v] = Word(n->halo.y[v] - old_camera_y);
            }
            // This original entity lacks CAMERA_LOCKED: retain screen position after camera work.
            n->x = FixedShift(n->x, camera_delta_x);
            n->y = FixedShift(n->y, camera_delta_y);
            c->source.x = n->x;
            c->source.y = n->y;
            c->source.flags = n->flags;
            c->source.priority = n->priority;
            c->source.facing_left = n->facing_left;
            c->initialized = !removed;
            c->draw_count = removed ? 0 : 1;
            if (!removed && c->state.neutron.attack.active)
                r.neutron_mask |= (Uint64)1 << i;
        }
        else if (c->food)
        {
            SotnEquipment equipment = {0};
            if (!c->state.food.inventory && !c->state.food.phase &&
                !SotnEquipment_ReadFood(in.dra, c->state.food.item, &equipment))
                return false;
            FoodFactoryContext factory = {
                .runtime = &r,
                .source = c->source,
                .camera_delta_x = Word(in.room_shift_x - in.screen_shift_x),
                .camera_delta_y = Word(in.room_shift_y - in.screen_shift_y)};
            SotnFoodEvents events = {0};
            if (!SotnFood_Tick(&c->state.food,
                               (SotnFoodInput){.collision = in.food_collision,
                                               .player = {.x = p.x,
                                                          .y = p.y,
                                                          .offset_x = p.animation.hitbox_x,
                                                          .offset_y = p.animation.hitbox_y,
                                                          .width = p.animation.hitbox_width,
                                                          .height = p.animation.hitbox_height,
                                                          .frame = p.animation.sprite,
                                                          .animation = p.animation.id,
                                                          .priority = p.priority,
                                                          .facing_left = p.facing_left,
                                                          .healing_pending = healing_pending},
                                               .attack = (Uint16)equipment.attack,
                                               .factory = FoodFactory,
                                               .context = &factory},
                               &progress.random, &events))
                return false;
            if (events.collect)
            {
                if (!Sound(&r, 0x67c) ||
                    !SotnCharacter_PlanCollect(character, &progress, false, events.item) ||
                    !SotnFoodTextPlan_Replace(&r.food_text, in.dra, events.item,
                                              &in.primitive_slots))
                    return false;
                inventory_changed = true;
            }
            if (events.heal || events.poison || events.peanut_pose || events.simulated_ticks ||
                events.clear_weapon)
            {
                if (r.food_event_count >= SDL_arraysize(r.food_events))
                    return SDL_SetError("Food callbacks exceeded their actual fifteen slots");
                r.food_events[r.food_event_count++] = events;
            }
            healing_pending |= events.heal;
            if (events.poison)
                r.player_hitbox_state = 0;
            const Sint16 old_camera_x = Word(in.camera_x - factory.camera_delta_x),
                         old_camera_y = Word(in.camera_y - factory.camera_delta_y);
            const Sint32 x = Pixel(FixedShift(c->state.food.x, Word(-old_camera_x))),
                         y = Pixel(FixedShift(c->state.food.y, Word(-old_camera_y)));
            removed = c->state.food.removed || x < -32 || x > 288 || y < -16 || y > 256;
            if (!(c->state.food.flags & 0x08000000U))
            {
                c->state.food.x = FixedShift(c->state.food.x, factory.camera_delta_x);
                c->state.food.y = FixedShift(c->state.food.y, factory.camera_delta_y);
            }
            c->parameter = c->state.food.parameter;
            c->source.x = c->state.food.x;
            c->source.y = c->state.food.y;
            c->source.flags = c->state.food.flags;
            c->source.priority = c->state.food.priority;
            c->source.facing_left = c->state.food.facing_left;
            c->initialized = !removed;
        }
        else if (c->kind == 19)
        {
            bool allocated = c->primitive_count != 0;
            if (!allocated &&
                !SotnPrimitiveSlots_Reserve(&in.primitive_slots, 20, c->primitives, &allocated))
                return false;
            c->primitive_count = allocated ? 20 : 0;
            SotnPlayerFactorySource source = c->source;
            source.x = FixedShift(source.x, (Sint16)-in.camera_x);
            source.y = FixedShift(source.y, (Sint16)-in.camera_y);
            if (!SotnHealingNumber_Tick(&c->state.healing_number, in.executable, source, allocated))
                return false;
            removed = c->state.healing_number.removed;
            c->initialized = !removed;
        }
        else if (c->kind == 37)
        {
            bool allocated = c->primitive_count != 0;
            if (c->state.immunity_text.phase == 2 &&
                !SotnPrimitiveSlots_Reserve(&in.primitive_slots, 18, c->primitives, &allocated))
                return false;
            c->primitive_count = allocated ? 18 : 0;
            if (!SotnImmunityText_TickParameter(
                    &c->state.immunity_text, in.dra, in.executable,
                    FixedShift(p.x, (Sint16)-in.camera_x), FixedShift(p.y, (Sint16)-in.camera_y),
                    ((p.contacts | p.previous_contacts) & 1U) != 0, allocated, c->parameter) ||
                (c->state.immunity_text.sound && !Sound(&r, c->state.immunity_text.sound)))
                return false;
            removed = c->state.immunity_text.removed;
            c->initialized = !removed;
        }
        else if (c->kind == 47)
        {
            bool allocated = c->primitive_count != 0;
            if (!allocated &&
                !SotnPrimitiveSlots_Reserve(&in.primitive_slots, 16, c->primitives, &allocated))
                return false;
            c->primitive_count = allocated ? 16 : 0;
            // Original holy callbacks precede the engine's camera/primitive pass.
            const Sint16 dx = Word(in.screen_shift_x - in.room_shift_x),
                         dy = Word(in.screen_shift_y - in.room_shift_y);
            const Sint16 camera_x = Word(-(Sint32)in.camera_x - dx),
                         camera_y = Word(-(Sint32)in.camera_y - dy);
            SotnPlayerFactorySource source = c->source;
            source.x = FixedShift(source.x, camera_x);
            source.y = FixedShift(source.y, camera_y);
            if (!SotnHolyHit_Tick(&c->state.holy, &r.holy,
                                  (SotnHolyHitInput){.dra = in.dra,
                                                     .source = source,
                                                     .player_x = FixedShift(p.x, camera_x),
                                                     .player_y = FixedShift(p.y, camera_y),
                                                     .hitbox_x = p.animation.hitbox_x,
                                                     .hitbox_y = p.animation.hitbox_y,
                                                     .priority = p.priority,
                                                     .status = p.status,
                                                     .allocated = allocated},
                                  &progress.random))
                return false;
            const Sint32 x = Pixel(c->state.holy.x), y = Pixel(c->state.holy.y);
            removed = c->state.holy.removed || x < -32 || x > 288 || y < -16 || y > 256;
            if (!removed && !SotnHolyHit_Rebase(&c->state.holy, dx, dy, false))
                return false;
            c->initialized = !removed && allocated;
        }
        else if (c->kind == 39)
        {
            SotnMedicineEffectEvents events = {0};
            if (!SotnMedicineEffect_Tick(
                    &c->state.medicine,
                    (SotnMedicineEffectInput){.dra = in.dra,
                                              .player_step = (Uint16)p.step,
                                              .hp_max = p.hp_max,
                                              .enhanced_potion = progress.equipment[5] == 80 ||
                                                                 progress.equipment[6] == 80},
                    &events))
                return false;
            for (Uint16 j = 0; j < events.factory_count; ++j)
                if (!RequestSource(&r, c->source, events.factories[j], 0))
                    return false;
            for (Uint16 j = 0; j < events.sound_count; ++j)
                if (!Sound(&r, events.sounds[j]))
                    return false;
            r.medicine.buff_mask |= events.buff_mask;
            r.medicine.protection = SDL_max(r.medicine.protection, events.protection);
            r.medicine.cure_mask |= events.cure_mask;
            r.medicine.mp_full |= events.mp_full;
            r.medicine.hammer |= events.hammer;
            if (events.heal)
            {
                r.medicine.heal = true;
                r.medicine.heal_amount = events.heal_amount;
                healing_pending = true;
            }
            if (events.protection && p.player_timers[14] < events.protection)
                p.player_timers[14] = events.protection;
            if (events.cure_mask & 1U)
                p.poison_ticks = p.player_timers[0] = 2;
            if (events.cure_mask & 2U)
                p.curse_ticks = p.player_timers[1] = 2;
            removed = c->state.medicine.removed;
            c->initialized = !removed;
        }
        else if (c->kind == 38)
        {
            bool allocated = c->primitive_count != 0;
            if (!allocated &&
                !SotnPrimitiveSlots_Reserve(&in.primitive_slots, SOTN_PLAYER_CIRCLE_ROWS,
                                            c->primitives, &allocated))
                return false;
            c->primitive_count = allocated ? SOTN_PLAYER_CIRCLE_ROWS : 0;
            const Sint16 dx = Word(in.screen_shift_x - in.room_shift_x),
                         dy = Word(in.screen_shift_y - in.room_shift_y),
                         camera_x = Word((Sint32)in.camera_x + dx),
                         camera_y = Word((Sint32)in.camera_y + dy);
            if (!c->state.circle.phase)
            {
                c->state.circle.x = FixedShift(c->source.x, Word(-camera_x));
                c->state.circle.y = FixedShift(c->source.y, Word(-camera_y));
            }
            SotnPlayerCircleEvents events = {0};
            if (!SotnPlayerCircle_Tick(
                    &c->state.circle,
                    (SotnPlayerCircleInput){.dra = in.dra,
                                            .executable = in.executable,
                                            .player_x = FixedShift(p.x, Word(-camera_x)),
                                            .player_y = FixedShift(p.y, Word(-camera_y)),
                                            .priority = p.priority,
                                            .allocated = allocated},
                    &events))
                return false;
            r.afterimage_reset |= events.reset_afterimages;
            removed = c->state.circle.removed;
            if (!removed && !SotnPlayerCircle_Rebase(&c->state.circle, dx, dy, false))
                return false;
            c->initialized = !removed;
        }
        else if (c->kind == 65)
        {
            bool allocated = c->primitive_count != 0;
            if (!allocated && !SotnPrimitiveSlots_Reserve(&in.primitive_slots, SOTN_TELEPORT_ROWS,
                                                          c->primitives, &allocated))
                return false;
            c->primitive_count = allocated ? SOTN_TELEPORT_ROWS : 0;
            const Sint16 dx = Word(in.screen_shift_x - in.room_shift_x),
                         dy = Word(in.screen_shift_y - in.room_shift_y),
                         camera_x = Word((Sint32)in.camera_x + dx),
                         camera_y = Word((Sint32)in.camera_y + dy);
            if (!c->state.teleport.phase)
            {
                c->state.teleport.x = FixedShift(c->source.x, Word(-camera_x));
                c->state.teleport.y = FixedShift(c->source.y, Word(-camera_y));
            }
            SotnTeleportEffectEvents events = {0};
            if (!SotnTeleportEffect_Tick(
                    &c->state.teleport, &r.teleport,
                    (SotnTeleportEffectInput){.dra = in.dra,
                                              .player_x = FixedShift(p.x, Word(-camera_x)),
                                              .player_y = FixedShift(p.y, Word(-camera_y)),
                                              .allocated = allocated},
                    &progress.random, &events))
                return false;
            for (Uint16 sound = 0; sound < events.sound_count; ++sound)
                if (!Sound(&r, events.sounds[sound]))
                    return false;
            if (events.palette)
                r.teleport_palette = events.palette;
            if (events.warp_mode)
                r.teleport_warp_mode = events.warp_mode;
            r.teleport_acknowledged |= events.warp_acknowledged;
            removed = c->state.teleport.removed;
            if (!removed && !SotnTeleportEffect_Rebase(&c->state.teleport, dx, dy, false))
                return false;
            c->initialized = !removed && c->state.teleport.phase;
        }
        else if (c->kind == 32)
        {
            if (!in.dissolve_image)
                return SDL_SetError("Dissolve requires the actual shared DRA image owner");
            bool allocated = c->primitive_count != 0;
            if (!allocated && (p.animation.sprite & 0x7fffU) &&
                !SotnPrimitiveSlots_Reserve(&in.primitive_slots, SOTN_STONE_DISSOLVE_ROWS,
                                            c->primitives, &allocated))
                return false;
            c->primitive_count = allocated ? SOTN_STONE_DISSOLVE_ROWS : 0;
            const Sint16 dx = Word(in.screen_shift_x - in.room_shift_x),
                         dy = Word(in.screen_shift_y - in.room_shift_y),
                         camera_x = Word((Sint32)in.camera_x + dx),
                         camera_y = Word((Sint32)in.camera_y + dy);
            if (!c->state.dissolve.phase)
            {
                c->state.dissolve.x = FixedShift(c->source.x, Word(-camera_x));
                c->state.dissolve.y = FixedShift(c->source.y, Word(-camera_y));
            }
            SotnStoneDissolveEvents events = {0};
            if (!SotnStoneDissolve_Tick(
                    &c->state.dissolve, &dissolve_image,
                    (SotnStoneDissolveInput){.dra = in.dra,
                                             .image = in.image,
                                             .player_x = FixedShift(p.x, Word(-camera_x)),
                                             .player_y = FixedShift(p.y, Word(-camera_y)),
                                             .engine_timer = in.engine_timer,
                                             .player_animation = p.animation.id,
                                             .player_sprite = p.animation.sprite,
                                             .player_step = (Uint16)p.step,
                                             .palette = p.palette,
                                             .priority = p.priority,
                                             .tapped = p.tapped,
                                             .facing_left = p.facing_left,
                                             .allocated = allocated,
                                             .capture = in.dissolve_capture,
                                             .upload = in.dissolve_upload,
                                             .context = in.dissolve_context},
                    &progress.random, &events))
                return false;
            if (events.frame_load)
            {
                p.animation.sprite |= 0x8000U;
                r.dissolve_frame_load = true;
            }
            r.afterimage_reset |= events.reset_afterimages;
            const Sint32 x = Pixel(c->state.dissolve.x), y = Pixel(c->state.dissolve.y);
            removed = c->state.dissolve.removed || x < -32 || x > 288 || y < -16 || y > 256;
            if (!removed && c->state.dissolve.phase &&
                !SotnStoneDissolve_Rebase(&c->state.dissolve, dx, dy, false))
                return false;
            c->initialized = !removed && c->state.dissolve.phase;
        }
        else if (c->kind == 33)
        {
            bool allocated = c->primitive_count != 0;
            if (!allocated && !SotnPrimitiveSlots_Reserve(&in.primitive_slots, SOTN_ICE_HIT_ROWS,
                                                          c->primitives, &allocated))
                return false;
            c->primitive_count = allocated ? SOTN_ICE_HIT_ROWS : 0;
            const Sint16 dx = Word(in.screen_shift_x - in.room_shift_x),
                         dy = Word(in.screen_shift_y - in.room_shift_y),
                         camera_x = Word((Sint32)in.camera_x + dx),
                         camera_y = Word((Sint32)in.camera_y + dy);
            SotnPlayerFactorySource source = c->source;
            source.x = FixedShift(source.x, Word(-camera_x));
            source.y = FixedShift(source.y, Word(-camera_y));
            if (!SotnIceHit_Tick(&c->state.ice,
                                 (SotnIceHitInput){.dra = in.dra,
                                                   .executable = in.executable,
                                                   .source = source,
                                                   .player_x = FixedShift(p.x, Word(-camera_x)),
                                                   .player_y = FixedShift(p.y, Word(-camera_y)),
                                                   .velocity_x = p.velocity_x,
                                                   .velocity_y = p.velocity_y,
                                                   .status = p.status,
                                                   .contacts = p.contacts,
                                                   .priority = p.priority,
                                                   .player_step = (Uint16)p.step,
                                                   .player_substep = p.substep,
                                                   .facing_left = p.facing_left,
                                                   .allocated = allocated},
                                 &progress.random) ||
                (c->state.ice.sound && !Sound(&r, c->state.ice.sound)))
                return false;
            const Sint32 x = Pixel(c->state.ice.x), y = Pixel(c->state.ice.y);
            removed = c->state.ice.removed || x < -32 || x > 288 || y < -16 || y > 256;
            if (!removed && !SotnIceHit_Rebase(&c->state.ice, dx, dy, false))
                return false;
            c->initialized = !removed && allocated;
        }
        else if (c->kind == 30)
        {
            bool allocated = c->primitive_count != 0;
            if (!allocated &&
                !SotnPrimitiveSlots_Reserve(&in.primitive_slots, 6, c->primitives, &allocated))
                return false;
            c->primitive_count = allocated ? 6 : 0;
            const Sint16 dx = Word(in.screen_shift_x - in.room_shift_x),
                         dy = Word(in.screen_shift_y - in.room_shift_y),
                         camera_x = Word((Sint32)in.camera_x + dx),
                         camera_y = Word((Sint32)in.camera_y + dy);
            SotnPlayerFactorySource source = c->source;
            source.x = FixedShift(source.x, Word(-camera_x));
            source.y = FixedShift(source.y, Word(-camera_y));
            if (!SotnThunderHit_Tick(
                    &c->state.thunder,
                    (SotnThunderHitInput){.dra = in.dra,
                                          .executable = in.executable,
                                          .source = source,
                                          .player_x = FixedShift(p.x, Word(-camera_x)),
                                          .player_y = FixedShift(p.y, Word(-camera_y)),
                                          .player_velocity_y = p.velocity_y,
                                          .contacts = p.contacts,
                                          .priority = p.priority,
                                          .player_step = (Uint16)p.step,
                                          .parameter = c->parameter,
                                          .allocated = allocated},
                    &progress.random))
                return false;
            for (Uint16 sound = 0; sound < c->state.thunder.sound_count; ++sound)
                if (!Sound(&r, c->state.thunder.sounds[sound]))
                    return false;
            const Sint32 x = Pixel(c->state.thunder.x), y = Pixel(c->state.thunder.y);
            removed = c->state.thunder.removed || x < -32 || x > 288 || y < -16 || y > 256;
            if (!removed && !SotnThunderHit_Rebase(&c->state.thunder, dx, dy, false))
                return false;
            c->initialized = !removed && allocated;
        }
        else if (c->kind == 16)
        {
            const Sint16 dx = Word(in.screen_shift_x - in.room_shift_x),
                         dy = Word(in.screen_shift_y - in.room_shift_y);
            const Sint16 camera_x = Word((Sint32)in.camera_x + dx),
                         camera_y = Word((Sint32)in.camera_y + dy);
            SotnPlayerFactorySource source = c->source;
            source.x = FixedShift(source.x, Word(-camera_x));
            source.y = FixedShift(source.y, Word(-camera_y));
            if (!SotnFireHit_Tick(&c->state.fire_hit,
                                  (SotnFireHitInput){.source = source,
                                                     .player_x = FixedShift(p.x, Word(-camera_x)),
                                                     .player_y = FixedShift(p.y, Word(-camera_y)),
                                                     .player_velocity_x = p.velocity_x,
                                                     .hitbox_y = p.animation.hitbox_y,
                                                     .priority = p.priority,
                                                     .parameter = c->parameter},
                                  &progress.random))
                return false;
            if (c->state.fire_hit.particle)
                for (Uint16 f = 0; f < SOTN_BAT_RUNTIME_FACTORIES; ++f)
                    if (!r.factories[f].cursor.entity_id)
                    {
                        SotnPlayerFactory cursor = {0};
                        source.x = FixedShift(c->state.fire_hit.x, camera_x);
                        source.y = FixedShift(c->state.fire_hit.y, camera_y);
                        source.priority = c->state.fire_hit.priority;
                        source.flags = 0;
                        if (!SotnPlayerFactory_Begin(&cursor, source, 0x40004, 0))
                            return false;
                        SDL_zero(r.factories[f]);
                        r.factories[f].cursor = cursor;
                        r.factories[f].fire_parent = c->generation;
                        r.factories[f].fire_parent_slot = i;
                        break;
                    }
            const Sint32 x = Pixel(c->state.fire_hit.x), y = Pixel(c->state.fire_hit.y);
            removed = c->state.fire_hit.removed || x < -32 || x > 288 || y < -16 || y > 256;
            if (!removed && (!SotnFireHit_Animate(&c->state.fire_hit, in.dra) ||
                             !SotnFireHit_Rebase(&c->state.fire_hit, dx, dy)))
                return false;
            c->initialized = !removed;
        }
        else if (c->kind == 54)
        {
            const Sint16 dx = Word(in.screen_shift_x - in.room_shift_x),
                         dy = Word(in.screen_shift_y - in.room_shift_y);
            SotnPlayerFactorySource source = c->source;
            source.x = FixedShift(source.x, Word(-(Sint32)in.camera_x - dx));
            source.y = FixedShift(source.y, Word(-(Sint32)in.camera_y - dy));
            if (!SotnDarkHit_Tick(&c->state.dark, source, p.priority, &r.dark_blend_phase,
                                  &progress.random))
                return false;
            const Sint32 x = Pixel(c->state.dark.x), y = Pixel(c->state.dark.y);
            removed = c->state.dark.removed || x < -32 || x > 288 || y < -16 || y > 256;
            // Original flag100000 animation follows callback and offscreen culling.
            if (!removed && (!SotnDarkHit_Animate(&c->state.dark, in.dra) ||
                             !SotnDarkHit_Rebase(&c->state.dark, dx, dy)))
                return false;
            c->initialized = !removed;
        }
        else if (c->kind == 51)
        {
            SotnPlayerFactorySource source = c->source;
            source.x = FixedShift(source.x, (Sint16)-in.camera_x);
            source.y = FixedShift(source.y, (Sint16)-in.camera_y);
            if (!SotnRisingHeart_Tick(&c->state.heart, in.executable, source, p.priority,
                                      in.game_timer))
                return false;
            c->initialized = true;
            const Sint32 x = Pixel(c->state.heart.x), y = Pixel(c->state.heart.y);
            removed = c->state.heart.removed || x < -32 || x > 288 || y < -16 || y > 256;
        }
        else if (c->kind == 2)
        {
            SotnPlayerFactorySource source = c->source;
            source.x = FixedShift(source.x, (Sint16)-in.camera_x);
            source.y = FixedShift(source.y, (Sint16)-in.camera_y);
            SotnPlayerSmokeEnvironment environment = p.smoke_environment;
            environment.x = FixedShift(environment.x, (Sint16)-in.camera_x);
            environment.y = FixedShift(environment.y, (Sint16)-in.camera_y);
            if (!SotnPlayerSmoke_TickShared(&c->state.smoke, in.dra, source, c->parameter,
                                            p.priority, p.contacts, p.status, &environment,
                                            &progress.random))
                return false;
            const Sint32 x = Pixel(c->state.smoke.x), y = Pixel(c->state.smoke.y);
            removed = c->state.smoke.removed || x < -32 || x > 288 || y < -16 || y > 256;
            c->initialized = !removed;
        }
        else if (c->kind == 5)
        {
            bool allocated = c->primitive_count != 0;
            Uint16 count = 0;
            if (!SotnPlayerParticles_Rows(in.dra, c->parameter, &count) ||
                (!allocated && !SotnPrimitiveSlots_Reserve(&in.primitive_slots, count,
                                                           c->primitives, &allocated)))
                return false;
            c->primitive_count = allocated ? (Uint8)count : 0;
            SotnPlayerFactorySource source = c->source;
            const bool fire = (c->parameter >> 8) == 4 || (c->parameter >> 8) == 11;
            const bool sire = (c->parameter >> 8) == 12;
            const Sint16 dx = fire || sire ? Word(in.screen_shift_x - in.room_shift_x) : 0,
                         dy = fire || sire ? Word(in.screen_shift_y - in.room_shift_y) : 0;
            const Sint16 camera_x = Word((Sint32)in.camera_x + dx),
                         camera_y = Word((Sint32)in.camera_y + dy);
            source.x = FixedShift(source.x, Word(-camera_x));
            source.y = FixedShift(source.y, Word(-camera_y));
            Sint32 parent_velocity = 0;
            if (fire && !c->initialized && allocated && c->thrown_particle_parent)
            {
                SotnThrownParent ignored = {0};
                if (!ThrownParent(&r, c->particle_parent_slot, &ignored, &parent_velocity))
                    return false;
            }
            else if (fire && !c->initialized && allocated)
            {
                const SotnBatRuntimeChild *parent = &r.children[c->particle_parent_slot];
                if (parent->kind != 16 || parent->generation != c->particle_parent ||
                    !parent->initialized)
                    return SDL_SetError("Fire particle birth lost its actual retained parent");
                parent_velocity = parent->state.fire_hit.velocity_x;
            }
            if (!SotnPlayerParticles_Tick(
                    &c->state.particles, in.dra,
                    (SotnPlayerParticlesInput){.source = source,
                                               .parent_velocity_x = parent_velocity,
                                               .parameter = c->parameter,
                                               .player_priority = p.priority,
                                               .player_x = Word(Pixel(p.x) - in.camera_x),
                                               .player_y = Word(Pixel(p.y) - in.camera_y),
                                               .player_step = (Uint16)p.step,
                                               .water_level = p.water.feet,
                                               .facing_left = p.facing_left,
                                               .vectors = &r.particle_vectors,
                                               .allocated = allocated},
                    &progress.random))
                return false;
            const Sint32 x = Pixel(c->source.x) - camera_x, y = Pixel(c->source.y) - camera_y;
            removed = c->state.particles.removed || (!(c->state.particles.flags & 0x04000000U) &&
                                                     (x < -32 || x > 288 || y < -16 || y > 256));
            if (!removed && fire && !SotnPlayerParticles_Rebase(&c->state.particles, dx, dy))
                return false;
            if (sire)
            {
                // Profile12 has neither CAMERA_LOCKED nor primitive DRAW_UNK02/200.
                c->source.x = FixedShift(c->source.x, Word(-dx));
                c->source.y = FixedShift(c->source.y, Word(-dy));
            }
            c->initialized = !removed && allocated;
        }
        else if (c->kind == 11)
        {
            if (!c->state.blink.phase && Duplicate(&r, i))
                removed = true;
            else
            {
                bool allocated = c->primitive_count != 0;
                if (!allocated &&
                    !SotnPrimitiveSlots_Reserve(&in.primitive_slots, 8, c->primitives, &allocated))
                    return false;
                c->primitive_count = allocated ? 8 : 0;
                c->initialized = allocated;
                SotnPlayerBlinkEvents events = {0};
                const Uint16 parameter = c->state.blink.parameter;
                if (!SotnPlayerBlink_Tick(
                        &c->state.blink, &r.shared,
                        (SotnPlayerBlinkInput){
                            .dra = in.dra,
                            .executable = in.executable,
                            .image = in.image,
                            .wolf_bounds = in.wolf_owned ? r.wolf.body.capture : in.wolf_bounds,
                            .wolf_form = p.animation_bank == 15,
                            .x = Word(Pixel(p.x) - in.camera_x),
                            .y = Word(Pixel(p.y) - in.camera_y),
                            .rotation = p.death.native ? p.death.rotation
                                        : p.water_death.active
                                            ? (Sint16)p.water_death.motion.rotation
                                        : p.stone.active ? p.stone.rotation
                                                         : p.bat.rotation,
                            .pivot_x = p.draw_pivot_x,
                            .pivot_y = p.draw_pivot_y,
                            .priority = p.priority,
                            .status = p.status,
                            .poison_ticks = p.poison_ticks,
                            .curse_ticks = p.curse_ticks,
                            .water_waist = p.water.waist,
                            .holy_symbol = in.holy_symbol_enabled,
                            .player_animation = p.animation.id,
                            .player_step = (Uint16)p.step,
                            .player_substep = p.substep,
                            .signal = p.wolf.active ? p.wolf.signal : p.bat.signal,
                            .facing_left = p.facing_left,
                            .rotated = p.rotated,
                            .image_present = p.animation_bank == 15 || in.image.width != 0,
                            .allocation_failed = !allocated},
                        &events))
                    return false;
                r.afterimage_reset = r.afterimage_reset || events.reset_afterimages;
                removed = c->state.blink.removed;
                if (parameter != c->state.blink.parameter)
                    c->parameter = (Uint16)(c->state.blink.parameter << 8);
                c->draw_count = removed || !allocated ? 0 : 8;
            }
        }
        else if (c->kind == 46)
        {
            const bool birth = !c->state.talisman.blink.phase;
            if (birth)
            {
                if (!SotnTalismanEffect_Rebase(&c->state.talisman, Word(-in.camera_x),
                                               Word(-in.camera_y)))
                    return false;
                bool allocated = false;
                if (!SotnPrimitiveSlots_Reserve(&in.primitive_slots, 8, c->primitives, &allocated))
                    return false;
                c->primitive_count = allocated ? 8 : 0;
            }
            SotnPlayerBlinkEvents events = {0};
            if (!SotnTalismanEffect_Tick(
                    &c->state.talisman, &r.shared,
                    (SotnPlayerBlinkInput){
                        .dra = in.dra,
                        .executable = in.executable,
                        .image = in.talisman_image,
                        .x = Word(Pixel(p.x) - in.camera_x),
                        .y = Word(Pixel(p.y) - in.camera_y),
                        .priority = p.priority,
                        .rotation = p.gravity.upside_down ? 0x800 : p.bat.rotation,
                        .rotated = p.rotated || p.gravity.upside_down,
                        .status = p.status,
                        .facing_left = p.facing_left,
                        .image_present = !(p.step == SOTN_PLAYER_HELLFIRE && p.palette == 0x810d),
                        .allocation_failed = !c->primitive_count},
                    c->parameter, in.game_timer, &events))
                return false;
            r.afterimage_reset = r.afterimage_reset || events.reset_afterimages;
            removed = c->state.talisman.blink.removed;
            c->initialized = !removed;
            c->draw_count = removed ? 0 : 8;
        }
        else if (c->kind == 31)
        {
            bool allocated = c->primitive_count != 0;
            if (!allocated &&
                !SotnPrimitiveSlots_Reserve(&in.primitive_slots, 1, c->primitives, &allocated))
                return false;
            c->primitive_count = allocated ? 1 : 0;
            SotnPlayerOutlineEvents events = {0};
            if (!SotnPlayerOutline_Tick(
                    &c->state.outline,
                    (SotnPlayerOutlineInput){.dra = in.dra,
                                             .executable = in.executable,
                                             .image = in.image,
                                             .x = Word(Pixel(p.x) - in.camera_x),
                                             .y = Word(Pixel(p.y) - in.camera_y),
                                             .scale_x = 256,
                                             .scale_y = 256,
                                             .pivot_x = p.draw_pivot_x,
                                             .pivot_y = p.draw_pivot_y,
                                             .rotation =
                                                 p.gravity.upside_down ? 0x800 : p.bat.rotation,
                                             .bank = p.animation_bank,
                                             .frame = p.animation.sprite,
                                             .palette = p.palette ? p.palette : 0x8100,
                                             .step = (Uint16)p.step,
                                             .priority = p.priority,
                                             .status = p.status,
                                             .facing_left = p.facing_left,
                                             .allocation_failed = !allocated},
                    &events))
                return false;
            r.afterimage_reset = r.afterimage_reset || events.reset_afterimages;
            removed = c->state.outline.removed;
            c->initialized = allocated && !removed;
            c->draw_count = c->initialized ? 1 : 0;
        }
        else if (c->kind == 0xe1 || c->kind == 0xf1)
        {
            SotnSwordWarpEvents events = {0};
            if (!c->initialized)
            {
                c->state.sword.image.facing_left = p.facing_left;
                c->source.priority = p.priority;
            }
            if (!SotnSwordWarp_Tick(&c->state.sword.image, p.animation.pose, p.animation.remaining,
                                    p.animation.sprite, &events))
                return false;
            c->source.x = c->state.sword.image.x;
            c->source.y = c->state.sword.image.y;
            c->source.facing_left = c->state.sword.image.facing_left;
            c->initialized = true;
            removed = c->state.sword.image.removed;
            if (events.strike)
                for (Uint16 f = 0; f < 8; ++f)
                    if (!r.factories[f].cursor.entity_id)
                    {
                        SotnPlayerFactory cursor = {0};
                        if (!SotnPlayerFactory_Begin(&cursor, c->source,
                                                     c->kind == 0xe1 ? 0x103e : 0x203e, 0))
                            return false;
                        SDL_zero(r.factories[f]);
                        r.factories[f].cursor = cursor;
                        r.factories[f].bat_generation = 0;
                        r.factories[f].sword_parent = c->generation;
                        break;
                    }
        }
        else if (c->kind == 0xe2 || c->kind == 0xf2)
        {
            if (!c->initialized)
            {
                const SotnBatRuntimeChild *parent = &r.children[0];
                if (!c->state.sword.parent_generation ||
                    parent->generation != c->state.sword.parent_generation ||
                    parent->kind != (c->kind == 0xe2 ? 0xe1 : 0xf1))
                    return SDL_SetError("Sword strike lost its actual parent generation");
                SotnEquipment e = {0};
                if (!SotnCharacter_SwordProperties(character, p.poison_ticks != 0, &progress.random,
                                                   &e))
                    return false;
                // SetWeaponProperties increments a least-used counter before copying enemyId0
                // from this original parent. End-of-pass recount removes that transient count.
                Uint16 group = i < 16 ? 3 : 7;
                for (Uint16 g = group + 1; g < (i < 16 ? 7 : 11); ++g)
                    if (r.groups[g] < r.groups[group])
                        group = g;
                if (r.groups[group] == 255)
                    return SDL_SetError("Sword strike shared group count overflow");
                ++r.groups[group];
                c->group = 0;
                c->source.priority = (Uint16)(p.priority - 2U);
                c->state.sword.attack = (SotnPlayerAttack){.offset_x = 27,
                                                           .offset_y = -19,
                                                           .width = 35,
                                                           .power = (Uint16)e.attack,
                                                           .element = e.element,
                                                           .hit_type = e.hit_type,
                                                           .hit_effect = e.hit_effect,
                                                           .stun_frames = e.stun,
                                                           .invincibility_frames = e.invincibility,
                                                           .critical_rate = (Uint8)e.critical_rate,
                                                           .equipment = 197,
                                                           .right_hand = c->kind == 0xf2};
                c->initialized = true;
            }
            SotnEquipment e = {.id = 197, .family = 43};
            SotnDataView overlay = {0};
            Uint16 sound = 0;
            if (!SotnWeapon_Overlay(in.weapons, e, &overlay) ||
                !SotnSwordStrike_Tick(&c->state.sword.strike, overlay, &sound) ||
                (sound && !Sound(&r, sound)))
                return false;
            removed = c->state.sword.strike.removed;
            c->state.sword.attack.height = c->state.sword.strike.height;
            c->state.sword.attack.active = c->state.sword.strike.height != 0;
            if (!removed)
                r.sword_mask |= (Uint64)1 << i;
        }
        else if (c->kind == 35)
        {
            bool allocated = c->primitive_count != 0;
            if (!allocated &&
                !SotnPrimitiveSlots_Reserve(&in.primitive_slots, 1, c->primitives, &allocated))
                return false;
            if (!allocated)
            {
                removed = true;
                goto finish_child;
            }
            c->primitive_count = 1;
            c->initialized = true;
            if (!SotnBatTrail_Tick(&c->state.trail,
                                   (SotnBatTrailInput){.frame = p.animation.sprite,
                                                       .bank = p.animation_bank,
                                                       .priority = p.priority,
                                                       .rotation = p.bat.rotation,
                                                       .wing_smashing = p.step == SOTN_PLAYER_BAT &&
                                                                        p.substep == 3,
                                                       .facing_left = p.facing_left,
                                                       .rotated = p.rotated}))
                return false;
            removed = c->state.trail.removed;
            if (!removed && in.image.width)
            {
                if (!SotnBatTrail_Quad(&c->state.trail, in.image, in.executable,
                                       Word(Pixel(c->state.trail.x) - in.camera_x),
                                       Word(Pixel(c->state.trail.y) - in.camera_y), &c->trail_quad))
                    return false;
                c->draw_count = 1;
            }
            const Sint32 x = Pixel(c->state.trail.x) - in.camera_x,
                         y = Pixel(c->state.trail.y) - in.camera_y;
            removed |= x > 288 || x < -32 || y > 256 || y < -16;
        }
        else if (c->kind == 24)
        {
            SotnHellfireTrailEvents events = {0};
            const bool birth = !c->state.hellfire_trail.motion.phase;
            const Uint16 random = birth ? (Uint16)SotnCombat_Random(&progress.random) : 0;
            const bool stone = birth && !(c->parameter >> 8);
            const Uint16 random_y = stone ? (Uint16)SotnCombat_Random(&progress.random) : 0;
            if (!SotnHellfireTrail_Tick(
                    &c->state.hellfire_trail.motion,
                    (SotnHellfireTrailInput){.player_priority = p.priority,
                                             .random = random,
                                             .random_y = random_y,
                                             .random_ready = birth,
                                             .random_y_ready = stone,
                                             .remaining = c->state.hellfire_trail.remaining},
                    &events))
                return false;
            removed = c->state.hellfire_trail.motion.removed;
            c->source.priority = c->state.hellfire_trail.motion.priority;
            c->source.x = c->state.hellfire_trail.motion.x;
            c->source.y = c->state.hellfire_trail.motion.y;
            const Sint32 x = Pixel(c->source.x) - in.camera_x, y = Pixel(c->source.y) - in.camera_y;
            removed |= x > 288 || x < -32 || y > 256 || y < -16;
            if (!removed)
            {
                if (!HellfireAnimation(in.dra, 0xd5fc, 13, &c->state.hellfire_trail.pose,
                                       &c->state.hellfire_trail.remaining,
                                       &c->state.hellfire_trail.frame))
                    return false;
                c->initialized = true;
            }
        }
        else if (c->kind == 25 || c->kind == 28 || c->kind == 29)
        {
            bool allocated = c->primitive_count != 0;
            if (!allocated && (c->kind == 28 || p.step == SOTN_PLAYER_HELLFIRE) &&
                !SotnPrimitiveSlots_Reserve(&in.primitive_slots, 1, c->primitives, &allocated))
                return false;
            c->primitive_count = allocated ? 1 : 0;
            if (c->kind == 25)
            {
                SotnHellfireBeamEvents events = {0};
                if (!SotnHellfireBeam_Tick(
                        &c->state.hellfire_beam,
                        (SotnHellfireBeamInput){.player_x = Word(Pixel(p.x) - in.camera_x),
                                                .game_timer = in.game_timer,
                                                .casting = p.step == SOTN_PLAYER_HELLFIRE,
                                                .up = in.hellfire_up,
                                                .allocated = allocated},
                        &events))
                    return false;
                removed = c->state.hellfire_beam.removed;
                if (!removed)
                    c->source.priority = 0x1c0;
                c->source.x = FixedShift(c->state.hellfire_beam.x, (Sint16)in.camera_x);
                c->source.y = FixedShift(c->state.hellfire_beam.y, (Sint16)in.camera_y);
                if (events.signal_changed)
                    r.hellfire_signal = events.signal;
                if (events.palette)
                    r.hellfire_palette = events.palette;
                r.hellfire_upload |= events.upload;
                if ((events.factory && !RequestSource(&r, c->source, events.factory, 0)) ||
                    (events.sound && !Sound(&r, events.sound)))
                    return false;
            }
            else
            {
                SotnHellfireEffectEvents events = {0};
                if (!SotnHellfireEffect_Tick(
                        &c->state.hellfire_effect,
                        (SotnHellfireEffectInput){.player_x = Word(Pixel(p.x) - in.camera_x),
                                                  .player_y = Word(Pixel(p.y) - in.camera_y),
                                                  .player_priority = p.priority,
                                                  .game_timer = in.game_timer,
                                                  .casting = p.step == SOTN_PLAYER_HELLFIRE,
                                                  .facing_left = p.facing_left,
                                                  .allocated = allocated},
                        &events))
                    return false;
                removed = c->state.hellfire_effect.removed;
                if (!removed && c->kind == 29)
                    c->source.priority = 0x1c0;
                c->source.x = FixedShift(c->state.hellfire_effect.x, (Sint16)in.camera_x);
                c->source.y = FixedShift(c->state.hellfire_effect.y, (Sint16)in.camera_y);
                if (events.signal_changed)
                    r.hellfire_signal = events.signal;
            }
            c->initialized = !removed && allocated;
            c->draw_count = c->initialized ? 1 : 0;
        }
        else if (c->kind == 26 || c->kind == 27)
        {
            const SotnPlayerFactorySource factory_source = c->source;
            const bool random_ready = c->state.hellfire_fire.motion.phase && c->kind == 27 &&
                                      c->state.hellfire_fire.pose >= 23 && !(in.game_timer & 1U);
            const Uint16 random = random_ready ? (Uint16)SotnCombat_Random(&progress.random) : 0;
            SotnHellfireFireballEvents events = {0};
            if (!SotnHellfireFireball_Tick(
                    &c->state.hellfire_fire.motion,
                    (SotnHellfireFireballInput){.dra = in.dra,
                                                .game_timer = in.game_timer,
                                                .player_priority = p.priority,
                                                .pose = c->state.hellfire_fire.pose,
                                                .random = random,
                                                .random_ready = random_ready,
                                                .hit_flags = c->state.hellfire_fire.hit_flags,
                                                .player_facing_left = p.facing_left},
                    &events))
                return false;
            removed = c->state.hellfire_fire.motion.removed;
            if (events.initialize)
            {
                if (i < 1 || i > 31)
                    return SDL_SetError("Hellfire spell lost its actual child slot range");
                if (!Spell(in.dra, &progress, stats, 2, &c->state.hellfire_fire.attack))
                    return false;
                c->state.hellfire_fire.attack.width = c->state.hellfire_fire.motion.width;
                c->state.hellfire_fire.attack.height = c->state.hellfire_fire.motion.height;
                Uint8 group = i < 16 ? 3 : 7;
                const Uint8 end = i < 16 ? 7 : 11;
                for (Uint8 j = (Uint8)(group + 1U); j < end; ++j)
                    if (r.groups[j] < r.groups[group])
                        group = j;
                if (r.groups[group] == 255)
                    return SDL_SetError("Hellfire spell attack group overflow");
                ++r.groups[group];
                c->group = group;
                c->initialized = true;
            }
            c->source.priority = c->state.hellfire_fire.motion.priority;
            c->source.x = c->state.hellfire_fire.motion.x;
            c->source.y = c->state.hellfire_fire.motion.y;
            c->source.facing_left = c->state.hellfire_fire.motion.facing_left;
            if ((events.factory && !RequestSource(&r, factory_source, events.factory, 0)) ||
                (events.sound && !Sound(&r, events.sound)))
                return false;
            const Sint32 x = Pixel(c->source.x) - in.camera_x, y = Pixel(c->source.y) - in.camera_y;
            removed |= x > 288 || x < -32 || y > 256 || y < -16;
            if (!removed)
            {
                if (!HellfireAnimation(in.dra, c->kind == 26 ? 0x10798 : 0x107c8,
                                       c->kind == 26 ? 9 : 26, &c->state.hellfire_fire.pose,
                                       &c->state.hellfire_fire.remaining,
                                       &c->state.hellfire_fire.frame))
                    return false;
                r.hellfire_mask |= (Uint64)1 << i;
            }
        }
        else if (c->kind == 36)
        {
            removed = p.step != SOTN_PLAYER_BAT || p.substep != 3;
            if (!removed)
            {
                if (!c->initialized)
                {
                    if (i < 1 || i > 15 || !Spell(in.dra, &progress, stats, 6, &c->state.attack))
                        return false;
                    Uint8 group = 3;
                    for (Uint8 j = 4; j < 7; ++j)
                        if (r.groups[j] < r.groups[group])
                            group = j;
                    if (r.groups[group] == 255)
                        return SDL_SetError("Wing Smash attack group count overflow");
                    ++r.groups[group];
                    c->group = group;
                    c->initialized = true;
                }
                c->source.x = (Sint32)((Sint64)Pixel(p.x) * 65536 + (Uint16)(Uint32)c->source.x);
                c->source.y = (Sint32)((Sint64)Pixel(p.y) * 65536 + (Uint16)(Uint32)c->source.y);
                c->source.facing_left = p.facing_left;
                r.wing_mask |= (Uint16)(1U << (i - 1U));
            }
        }
        else if (c->kind == 45)
        {
            SotnBatFireballEvents events = {0};
            if (!SotnBatFireball_Tick(&c->state.fire.motion,
                                      (SotnBatFireballInput){.player_priority = p.priority,
                                                             .player_facing_left = p.facing_left,
                                                             .hit_flags = c->state.fire.hit_flags},
                                      &events))
                return false;
            removed = c->state.fire.motion.removed;
            if (events.initialize)
            {
                if (!Spell(in.dra, &progress, stats, 9, &c->state.fire.attack))
                    return false;
                Uint8 group = 7;
                for (Uint8 j = 8; j < 11; ++j)
                    if (r.groups[j] < r.groups[group])
                        group = j;
                if (r.groups[group] == 255)
                    return SDL_SetError("Bat fireball attack group overflow");
                ++r.groups[group];
                c->group = group;
                c->initialized = true;
                if (!Sound(&r, events.sound))
                    return false;
                r.use_ticks = events.player_use_ticks;
            }
            c->source.x = c->state.fire.motion.x;
            c->source.y = c->state.fire.motion.y;
            c->source.facing_left = c->state.fire.motion.facing_left;
            const Sint32 x = Pixel(c->source.x) - in.camera_x, y = Pixel(c->source.y) - in.camera_y;
            removed |= x > 288 || x < -32 || y > 256 || y < -16;
            if (!removed)
            {
                if (!LoopAnimation(in.dra, &c->state.fire.pose, &c->state.fire.remaining,
                                   &c->state.fire.frame))
                    return false;
                r.fire_mask |= (Uint64)1 << i;
            }
        }
        else if (c->kind == 61)
        {
            if (!in.spirit_targets)
                return SDL_SetError("Live spirit requires its actual stage target snapshot");
            bool allocated = c->primitive_count != 0;
            if (!allocated &&
                !SotnPrimitiveSlots_Reserve(&in.primitive_slots, 16, c->primitives, &allocated))
                return false;
            c->primitive_count = allocated ? 16 : 0;
            const Uint16 slot = c->state.spirit.target_slot;
            const SotnSpellTarget target =
                c->state.spirit.target_bound ? targets.slots[slot] : (SotnSpellTarget){0};
            SotnSpiritEvents events = {0};
            if (!SotnSpirit_Tick(
                    &c->state.spirit.motion,
                    (SotnSpiritInput){.dra = in.dra,
                                      .executable = in.executable,
                                      .x = FixedShift(FixedShift(c->source.x, (Sint16)-in.camera_x),
                                                      Word(-(Sint32)in.screen_shift_x)),
                                      .y = FixedShift(FixedShift(c->source.y, (Sint16)-in.camera_y),
                                                      Word(-(Sint32)in.screen_shift_y)),
                                      .frame = c->state.spirit.frame,
                                      .target_x = target.x,
                                      .target_y = target.y,
                                      .target_bound = c->state.spirit.target_bound,
                                      .target_alive = target.identity != 0,
                                      .allocated = allocated,
                                      .player_facing_left = p.facing_left},
                    &events))
                return false;
            removed = c->state.spirit.motion.removed;
            if (events.initialize_attack)
            {
                if (!Spell(in.dra, &progress, stats, events.spell, &c->state.spirit.attack))
                    return false;
                Uint8 group = i < 16 ? 3 : 7;
                const Uint8 end = i < 16 ? 7 : 11;
                for (Uint8 j = (Uint8)(group + 1U); j < end; ++j)
                    if (r.groups[j] < r.groups[group])
                        group = j;
                if (r.groups[group] == 255)
                    return SDL_SetError("Spirit attack group count overflow");
                ++r.groups[group];
                c->group = group;
                c->initialized = true;
            }
            if (events.select_target)
            {
                Uint16 selected = SDL_MAX_UINT16;
                if (!SotnSpellTarget_Select(targets.slots, &r.target_cursor, &selected))
                    return false;
                c->state.spirit.target_slot = selected;
                c->state.spirit.target_bound = selected != SDL_MAX_UINT16;
                if (selected != SDL_MAX_UINT16 &&
                    !(in.spirit_targets->slots[selected].flags & 0x80000U))
                {
                    if (!targets.generations[selected])
                        return SDL_SetError(
                            "Spirit selected a target without an actual generation");
                    r.target_marks[selected / 64U] |= (Uint64)1 << (selected % 64U);
                    r.target_generations[selected] = targets.generations[selected];
                    r.target_identities[selected] = targets.slots[selected].identity;
                }
            }
            const Sint32 x = Pixel(c->state.spirit.motion.x), y = Pixel(c->state.spirit.motion.y);
            removed |= !(c->state.spirit.motion.flags & 0x04000000U) &&
                       (x > 288 || x < -32 || y > 256 || y < -16);
            if (!removed)
            {
                if (!LoopAnimation(in.dra, &c->state.spirit.pose, &c->state.spirit.remaining,
                                   &c->state.spirit.frame) ||
                    !SotnSpirit_Translate(&c->state.spirit.motion, in.screen_shift_x,
                                          in.screen_shift_y))
                    return false;
                for (Uint16 j = 0; j < 8; ++j)
                    c->state.spirit.quads[j] = c->state.spirit.motion.trails[j].quad;
                c->draw_count = 8;
                c->source.x = FixedShift(c->state.spirit.motion.x, (Sint16)in.camera_x);
                c->source.y = FixedShift(c->state.spirit.motion.y, (Sint16)in.camera_y);
                c->source.facing_left = c->state.spirit.motion.facing_left;
                r.spirit_mask |= (Uint64)1 << i;
            }
        }
        else if (c->kind == 63)
        {
            bool allocated = c->primitive_count != 0;
            if (!allocated && c->state.portal.phase == 1 && c->state.portal.delay == 1 &&
                !SotnPrimitiveSlots_Reserve(&in.primitive_slots, 9, c->primitives, &allocated))
                return false;
            c->primitive_count = allocated ? 9 : 0;
            SotnSpiritPortalEvents events = {0};
            if (!SotnSpiritPortal_Tick(
                    &c->state.portal,
                    (SotnSpiritPortalInput){
                        .dra = in.dra,
                        .executable = in.executable,
                        .x = FixedShift(FixedShift(c->source.x, (Sint16)-in.camera_x),
                                        Word(-(Sint32)in.screen_shift_x)),
                        .y = FixedShift(FixedShift(c->source.y, (Sint16)-in.camera_y),
                                        Word(-(Sint32)in.screen_shift_y)),
                        .player_y = Word(Pixel(p.y) - in.camera_y - in.screen_shift_y),
                        .allocated = allocated},
                    &events))
                return false;
            if (c->state.portal.removed && !events.release_freeze)
                return SDL_SetError("Spirit portal cannot reserve nine real rows; stopping before "
                                    "the original unreleased scene freeze");
            removed = c->state.portal.removed;
            c->initialized = !removed;
            c->source.x =
                FixedShift(FixedShift(c->state.portal.x, (Sint16)in.camera_x), in.screen_shift_x);
            c->source.y =
                FixedShift(FixedShift(c->state.portal.y, (Sint16)in.camera_y), in.screen_shift_y);
            c->source.flags = c->state.portal.flags;
            if (events.freeze || events.release_freeze)
            {
                r.freeze_changed = true;
                r.freeze = events.freeze;
            }
            if ((events.flash && !Flash(&r, in.dra, events.flash)) ||
                (events.sound && !Sound(&r, events.sound)))
                return false;
            for (Uint16 j = 0; j < 2; ++j)
                if (events.factories[j] && !RequestSource(&r, c->source, events.factories[j], 0))
                    return false;
            if (events.capture)
            {
                if (r.capture_count >= SDL_arraysize(r.captures))
                    return SDL_SetError("Spirit capture exceeded its actual root slot count");
                r.captures[r.capture_count++] = (SotnSpiritCapture){.generation = c->generation,
                                                                    .slot = (Uint16)(i + 16U),
                                                                    .x = events.capture_x,
                                                                    .y = events.capture_y};
            }
            c->state.portal.x = FixedShift(c->state.portal.x, in.screen_shift_x);
            c->state.portal.y = FixedShift(c->state.portal.y, in.screen_shift_y);
            if (allocated && !removed)
            {
                c->draw_count = 8;
                for (Uint16 j = 0; j < 8; ++j)
                    for (Uint16 v = 0; v < 4; ++v)
                    {
                        c->state.portal.quads[j].x[v] =
                            Word(c->state.portal.quads[j].x[v] + in.screen_shift_x);
                        c->state.portal.quads[j].y[v] =
                            Word(c->state.portal.quads[j].y[v] + in.screen_shift_y);
                    }
            }
        }
        else if (c->kind == 49)
        {
            removed = !p.axe_armor.active || p.step != SOTN_PLAYER_AXE_HIT;
            if (!removed)
            {
                if (!c->initialized)
                {
                    c->state.axe_shadow.bank = p.animation_bank;
                    c->state.axe_shadow.palette = p.palette;
                    c->state.axe_shadow.texture = 0x64;
                    c->state.axe_shadow.frame = 7;
                    c->source.facing_left = p.facing_left;
                    c->source.priority = p.priority;
                    c->initialized = true;
                }
                const bool early = p.axe_armor.rotation == 512 || p.axe_armor.rotation == -512;
                Sint16 dx = early ? -16 : -24;
                if (p.axe_armor.knock_right)
                    dx = (Sint16)-dx;
                c->source.x =
                    (Sint32)((Sint64)Word(Pixel(p.x) + dx) * 65536 + (Uint16)(Uint32)c->source.x);
                c->source.y = (Sint32)((Sint64)Word(Pixel(p.y) + (early ? 9 : 16) +
                                                    ((in.game_timer >> 1) & 1U)) *
                                           65536 +
                                       (Uint16)(Uint32)c->source.y);
                c->state.axe_shadow.draw_flags = p.axe_armor.draw_flags & 8U;
                c->state.axe_shadow.opacity = p.axe_armor.opacity;
            }
        }
        else if (c->kind == 50)
        {
            removed = !p.axe_armor.active;
            if (!removed)
            {
                if (!c->initialized)
                {
                    if (!Spell(in.dra, &progress, stats, 11, &c->state.attack))
                        return false;
                    c->group = 1; // Original func_80118894's actual left-hand slot16 case.
                    c->initialized = true;
                }
                c->source.x = (Sint32)((Sint64)Pixel(p.x) * 65536 + (Uint16)(Uint32)c->source.x);
                c->source.y = (Sint32)((Sint64)Pixel(p.y) * 65536 + (Uint16)(Uint32)c->source.y);
                const Uint16 sprite = p.animation.sprite;
                if (sprite == 5 || sprite == 6)
                {
                    c->state.attack.offset_x = 28;
                    c->state.attack.offset_y = sprite == 5 ? -12 : 16;
                    c->state.attack.width = sprite == 5 ? 12 : 10;
                    c->state.attack.height = sprite == 5 ? 32 : 10;
                    r.axe_mask |= (Uint64)1 << i;
                }
                else
                    removed = true;
            }
        }
        else if (c->kind == 53)
        {
            if (p.step != SOTN_PLAYER_SOUL)
                removed = true;
            else
            {
                if (!c->initialized)
                {
                    if (!Spell(in.dra, &progress, stats, 5, &c->state.attack))
                        return false;
                    c->state.attack.width = c->state.attack.height = 0;
                    Uint8 group = 7;
                    for (Uint8 j = 8; j < 11; ++j)
                        if (r.groups[j] < r.groups[group])
                            group = j;
                    if (r.groups[group] == 255)
                        return SDL_SetError("Soul Steal attack group overflow");
                    ++r.groups[group];
                    c->group = group;
                    c->initialized = true;
                }
                r.soul_attack = c->state.attack.active;
            }
        }
        else if (c->kind == 52)
        {
            bool allocated = c->initialized;
            if (!allocated &&
                !SotnPrimitiveSlots_Reserve(&in.primitive_slots, SOTN_BAT_ECHO_PRIMITIVES,
                                            r.echo_primitives, &allocated))
                return false;
            SotnBatEchoEvents events = {0};
            if (!SotnBatEcho_Tick(&r.echo,
                                  (SotnBatEchoInput){.dra = in.dra,
                                                     .executable = in.executable,
                                                     .x = Word(Pixel(p.x) - in.camera_x),
                                                     .y = Word(Pixel(p.y) - in.camera_y),
                                                     .rotation = p.bat.rotation,
                                                     .priority = p.priority,
                                                     .primitives_allocated = allocated,
                                                     .force_enabled = in.force_echo_enabled},
                                  &events))
                return false;
            if (events.sound && !Sound(&r, events.sound))
                return false;
            removed = r.echo.removed;
            if (!removed)
            {
                c->initialized = true;
                r.echo_generation = c->generation;
                if (events.initialize_attack)
                {
                    if (!Spell(in.dra, &progress, stats, 10, &c->state.attack))
                        return false;
                    c->state.attack.offset_x = 64;
                    c->state.attack.offset_y = -4;
                    c->state.attack.width = 40;
                    c->state.attack.height = 12;
                    Uint8 group = 7;
                    for (Uint8 j = 8; j < 11; ++j)
                        if (r.groups[j] < r.groups[group])
                            group = j;
                    if (r.groups[group] == 255)
                        return SDL_SetError("Force of Echo attack group overflow");
                    ++r.groups[group];
                    c->group = group;
                }
                if (events.disable_attack)
                    c->state.attack.active = false;
                c->source.x = (Sint32)((Sint64)Pixel(p.x) * 65536 + (Uint16)(Uint32)c->source.x);
                c->source.y = (Sint32)((Sint64)Pixel(p.y) * 65536 + (Uint16)(Uint32)c->source.y);
                r.echo_attack = c->state.attack.active;
            }
            else
            {
                if (r.echo_generation &&
                    !SotnPrimitiveSlots_Release(&in.primitive_slots, SOTN_BAT_ECHO_PRIMITIVES,
                                                r.echo_primitives))
                    return false;
                r.echo_generation = 0;
                r.echo = (SotnBatEcho){0};
                SDL_zero(r.echo_primitives);
            }
        }
        else
            return SDL_SetError("Bat runtime child has an unsupported original identity");
    finish_child:
        if (removed)
        {
            if (c->primitive_count &&
                !SotnPrimitiveSlots_Release(&in.primitive_slots, c->primitive_count, c->primitives))
                return false;
            *c = (SotnBatRuntimeChild){0};
        }
    }
    r.screen_flash = false;
    if (r.screen_flash_ticks)
        r.screen_flash = (--r.screen_flash_ticks & 1U) != 0;
    SDL_memset(r.groups, 0, sizeof(r.groups));
    if (r.wolf.generation)
        for (Uint16 i = 0; i < 14; ++i)
            if (r.wolf.body.parts[i].enemy_id > 2 && r.wolf.body.parts[i].enemy_id < 11)
                ++r.groups[r.wolf.body.parts[i].enemy_id];
    for (Uint16 i = 0; i < 48; ++i)
        if ((r.children[i].kind == 36 || r.children[i].kind == 45 || r.children[i].kind == 26 ||
             r.children[i].kind == 27 || r.children[i].kind == 61 || r.children[i].kind == 52 ||
             r.children[i].kind == 53 || r.children[i].neutron || r.children[i].sire ||
             r.children[i].pentagram || r.children[i].magic || r.children[i].thrown ||
             r.children[i].star || r.children[i].karma || r.children[i].weapon_secondary) &&
            r.children[i].group > 2 && r.children[i].group < 11)
            ++r.groups[r.children[i].group];
    if (r.children[0].kind == 50 && r.children[0].initialized)
        ++r.groups[1];
    if (r.children[0].hand_weapon && r.children[0].group)
        r.groups[r.children[0].group] = 1;
    // Original UpdatePlayerEntities clears actual DRA hitboxState after all callbacks.
    // Retained attack owners stay disabled until their own callback writes a new hitbox.
    if (SotnPlayerStep_IsDead(p.step) || (p.step == SOTN_PLAYER_TELEPORT && !p.substep))
    {
        r.wing_mask = r.wolf_mask = 0;
        r.fire_mask = r.spirit_mask = r.hellfire_mask = r.sword_mask = r.axe_mask = r.neutron_mask =
            r.sire_mask = r.pentagram_mask = r.magic_mask = r.thrown_mask = r.star_mask =
                r.karma_mask = r.secondary_mask = 0;
        r.soul_attack = r.echo_attack = false;
        r.hand_attack = false;
        for (Uint16 i = 0; i < 14; ++i)
            r.wolf.body.parts[i].hitbox_state = 0;
        for (Uint16 i = 0; i < 2; ++i)
            r.wolf_attacks[i].active = false;
        for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        {
            SotnBatRuntimeChild *c = &r.children[i];
            if (c->hand_weapon)
            {
                c->state.hand.motion.hitbox_state = 0;
                c->state.hand.attack.active = false;
            }
            else if (c->weapon_secondary)
            {
                c->state.secondary.motion.hitbox_state = 0;
                c->state.secondary.attack.active = false;
            }
            else if (c->kind == 36 || c->kind == 50 || c->kind == 52 || c->kind == 53)
                c->state.attack.active = false;
            else if (c->kind == 45)
                c->state.fire.attack.active = false;
            else if (c->kind == 26 || c->kind == 27)
                c->state.hellfire_fire.attack.active = false;
            else if (c->kind == 61)
                c->state.spirit.attack.active = false;
            else if (!c->thrown && !c->star && !c->karma && !c->weapon_warp &&
                     (c->kind == 0xe2 || c->kind == 0xf2))
                c->state.sword.attack.active = false;
            else if (c->pentagram)
                c->state.pentagram.attack.active = false;
            else if (c->karma)
                c->state.karma.attack.active = false;
            else if (c->star)
                c->state.star.attack.active = false;
            else if (c->thrown)
                c->state.thrown.attack.active = false;
            else if (c->magic)
                c->state.magic.attack.active = false;
            else if (c->sire)
                c->state.sire.attack.active = false;
            else if (c->neutron)
                c->state.neutron.attack.active = false;
        }
    }
    if ((inventory_changed || progress.random != previous_random || progress.mp != previous_mp ||
         progress.hearts != previous_hearts) &&
        !SotnCharacter_Restore(character, &progress))
        return false;
    *runtime = r;
    if (in.dissolve_image)
        *in.dissolve_image = dissolve_image;
    return true;
}
bool SotnBatRuntime_Visual(const SotnBatRuntime *r, Uint16 index,
                           const SotnPlayerEffectQuad **quads, Uint16 *count)
{
    if (!r || index >= 48 || !quads || !count)
        return SDL_SetError("Bat visual requires its actual bounded slot");
    const SotnBatRuntimeChild *c = &r->children[index];
    if (c->draw_count > 8 ||
        (c->draw_count &&
         (!c->initialized ||
          !(((c->kind == 11 || c->kind == 46) && c->primitive_count == 8) ||
            ((c->kind == 25 || c->kind == 28 || c->kind == 29 || c->kind == 31 || c->kind == 35) &&
             c->primitive_count == 1) ||
            (c->neutron && c->primitive_count == 1) ||
            (c->thrown && (c->kind & 15U) == 0 && c->primitive_count == 1) ||
            (c->kind == 61 && c->primitive_count == 16) ||
            (c->kind == 63 && c->primitive_count == 9)))))
        return SDL_SetError("Bat visual lost its actual packet owner");
    *count = c->draw_count;
    *quads = !c->draw_count                     ? NULL
             : c->neutron                       ? &c->state.neutron.motion.halo
             : c->kind == 46                    ? c->state.talisman.blink.quads
             : c->kind == 11                    ? c->state.blink.quads
             : c->kind == 25                    ? &c->state.hellfire_beam.quad
             : (c->kind == 28 || c->kind == 29) ? &c->state.hellfire_effect.quad
             : c->kind == 31                    ? &c->state.outline.quad
             : c->kind == 61                    ? c->state.spirit.quads
             : c->kind == 63                    ? c->state.portal.quads
                                                : &c->trail_quad;
    return true;
}

bool SotnBatRuntime_Hit(SotnBatRuntime *r, Uint16 i, Uint32 generation, Uint8 flags)
{
    const bool star = r && i < 48 && r->children[i].star && r->children[i].state.star.attack.active;
    const bool magic = r && i < 48 && r->children[i].magic &&
                       (r->children[i].kind == 0xe1 || r->children[i].kind == 0xf1);
    // Weapon1's role2 fireball (Fist of Tulkas) retires on its next callback after any hit.
    // Weapon1's role2 fireball and weapon6's crescents read the actual collision hitFlags.
    // Every weapon secondary and the slot16 hand weapon receive collision.h's attacker hitFlags;
    // only some callbacks read them (fireball, crescents, weapon30 children, weapon11's walker).
    const bool secondary = r && i < 48 && r->children[i].weapon_secondary;
    const bool hand = r && i < 48 && r->children[i].hand_weapon;
    if ((secondary || hand) && flags && flags <= 3 && generation &&
        r->children[i].generation == generation && r->children[i].initialized)
    {
        if (secondary)
            r->children[i].state.secondary.motion.hit_flags = flags;
        else
            r->children[i].state.hand.motion.hit_flags = flags;
        return true;
    }
    if (!r || i >= 48 || !flags || flags > 3 ||
        (!magic && !star && r->children[i].kind != 45 && r->children[i].kind != 26 &&
         r->children[i].kind != 27) ||
        !generation || r->children[i].generation != generation || !r->children[i].initialized)
        return SDL_SetError("Bat fireball feedback requires its actual live child generation");
    if (star)
        r->children[i].state.star.motion.hit_flags = flags;
    else if (magic)
        r->children[i].state.magic.hit_flags = flags;
    else if (r->children[i].kind == 45)
        r->children[i].state.fire.hit_flags = flags;
    else
        r->children[i].state.hellfire_fire.hit_flags = flags;
    return true;
}

bool SotnBatRuntime_TalismanValid(const SotnBatRuntimeChild *c, Uint16 index)
{
    if (c->kind != 46)
        return true;
    return index >= 16 && index < 48 && c->generation && !c->bat_generation && c->parameter <= 1 &&
           !c->state.talisman.blink.removed &&
           (c->initialized
                ? c->primitive_count == 8 && c->draw_count == 8 &&
                      c->state.talisman.blink.parameter == 0x26 &&
                      (c->state.talisman.blink.phase == 16 || c->state.talisman.blink.phase == 17)
                : !c->primitive_count && !c->draw_count && !c->state.talisman.blink.phase);
}
