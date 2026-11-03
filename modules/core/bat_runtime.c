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
             r->children[i].pentagram || r->children[i].magic || r->children[i].thrown) &&
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
    }
    r->wing_mask &= 0x4000U;
    SDL_zero(r->groups);
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if ((r->children[i].kind == 36 || r->children[i].kind == 45 || r->children[i].kind == 52 ||
             r->children[i].kind == 53 || r->children[i].kind == 26 || r->children[i].kind == 27 ||
             r->children[i].kind == 61 || r->children[i].neutron || r->children[i].sire ||
             r->children[i].pentagram || r->children[i].magic || r->children[i].thrown) &&
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
    const Uint32 blueprint = item == 81 || item == 134 ? 62U
                             : item == 82              ? 68U
                             : item == 135             ? 63U
                                                       : 50U;
    const Uint32 parameter = (item == 134 ? 1U : 0U) + (hand == 2 ? 128U : 0U);
    if (!r || !allocated || !ThrownItem(item) || (hand != 1 && hand != 2) ||
        packed != (blueprint | hand << 12 | parameter << 16))
        return SDL_SetError("Weapon16 reservation requires its actual owned item/blueprint/hand");
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
           (kind >= 0xe1 && kind <= 0xe5) || (kind >= 0xf1 && kind <= 0xf5) ||
           (kind >= 24 && kind <= 29);
}
static Uint32 ChildFlags(const SotnBatRuntimeChild *c)
{
    if (!c->initialized)
        return c->source.flags;
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
        if (!SotnBatRuntime_ThrownValid(&r->children[i], i))
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
            !SotnBatRuntime_ThrownValid(&r->children[i], i))
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
        }
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_FACTORIES; ++i)
        if (ChangedWeapon(r->factories[i].cursor.entity_id, hands))
            SDL_zero(r->factories[i]);
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
    if (!SotnBatRuntime_FoodChildValid(c, index))
        return false;
    if (c->food || c->sire || c->pentagram || c->magic || c->thrown)
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
        return c->neutron || c->sire || c->pentagram || c->magic || c->thrown ||
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
    c.food = ((c.kind == 0xe0 || c.kind == 0xf0) && !c.neutron && !c.sire && !c.pentagram &&
              !c.magic && !c.thrown) ||
             ((c.kind == 0xe1 || c.kind == 0xf1) && child.slot >= 17 && child.slot <= 31 &&
              ((child.parameter >> 8) & 127U) >= 29 && ((child.parameter >> 8) & 127U) <= 69);
    if ((c.food || c.neutron || c.sire || c.pentagram || c.magic || c.thrown) && input)
    {
        const Uint16 hand = c.kind >= 0xf0 ? 1 : 0;
        if (input->weapon_bound[hand] && input->weapon_family[hand] != (c.thrown      ? 16
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
    if (!c.thrown && (c.kind == 0xe2 || c.kind == 0xf2))
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
    if (!c.food && !c.sire && !c.magic && !c.thrown && (c.kind == 0xe1 || c.kind == 0xf1))
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
                       0, 0, 0, false, 0, false, 0, false, false, 0, NULL))
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
    if (!runtime || !character || (in.freeze != 0 && in.freeze != 3) || in.camera_x < 0 ||
        in.camera_x > 16384 || in.camera_y < 0 || in.camera_y > 16384 ||
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
                        !SotnBatRuntime_PruneMagicMissile(&r) || !SotnBatRuntime_PruneThrown(&r)))
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
    r.wolf_shake_count = 0;
    r.use_ticks = r.spell_use_ticks = 0;
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
                    (child.entity_id != 0xe1 && child.entity_id != 0xf1 && child.entity_id != 50) ||
                    (in.other_occupied & ((Uint64)1 << 16)))
                    return SDL_SetError("Shared runtime received an unowned weapon replacement");
                SotnBatRuntimeChild *old = &r.children[0];
                if (old->primitive_count &&
                    !SotnPrimitiveSlots_Release(&in.primitive_slots, old->primitive_count,
                                                old->primitives))
                    return false;
                *old = (SotnBatRuntimeChild){0};
                child.replace = false;
            }
            if (!Spawn(&r, child,
                       r.factories[i].sword_parent ? r.factories[i].sword_parent
                                                   : r.factories[i].bat_generation,
                       r.factories[i].fire_parent, r.factories[i].fire_parent_slot,
                       r.factories[i].sire_factory, r.factories[i].sire_parent_slot,
                       r.factories[i].magic_factory, r.factories[i].magic_parent_slot,
                       r.factories[i].thrown_factory, r.factories[i].thrown_particles,
                       r.factories[i].thrown_parent_slot, &in))
                return false;
        }
    }
    SotnCharacterState progress = {0};
    SotnCharacterStats stats = {0};
    if (!SotnCharacter_Get(character, &progress, &stats))
        return false;
    SotnDeathImage dissolve_image = in.dissolve_image ? *in.dissolve_image : (SotnDeathImage){0};
    const Uint32 previous_random = progress.random;
    const Uint16 previous_mp = progress.mp;
    bool inventory_changed = false;
    bool healing_pending = p.healing.pending_kind != 0;
    if (!in.freeze)
        r.player_hitbox_state = SotnPlayerStep_IsDead(p.step) ? 0 : 1;
    r.wing_mask = 0;
    r.fire_mask = r.spirit_mask = r.hellfire_mask = r.sword_mask = r.axe_mask = r.neutron_mask =
        r.sire_mask = r.pentagram_mask = r.magic_mask = r.thrown_mask = 0;
    const bool previous_echo_attack = r.echo_attack;
    r.soul_attack = false;
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
            continue;
        if (in.freeze && !(ChildFlags(c) & 0x10000U))
        {
            if (c->initialized && !c->thrown && (c->kind == 0xe2 || c->kind == 0xf2))
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
            if (c->initialized && c->thrown && c->state.thrown.attack.active)
                r.thrown_mask |= (Uint64)1 << i;
            if (c->initialized && c->magic && c->state.magic.attack.active)
                r.magic_mask |= (Uint64)1 << i;
            if (c->initialized && c->neutron && c->state.neutron.attack.active)
                r.neutron_mask |= (Uint64)1 << i;
            continue;
        }
        bool removed = false;
        c->draw_count = 0;
        if (c->thrown)
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
             r.children[i].pentagram || r.children[i].magic || r.children[i].thrown) &&
            r.children[i].group > 2 && r.children[i].group < 11)
            ++r.groups[r.children[i].group];
    if (r.children[0].kind == 50 && r.children[0].initialized)
        ++r.groups[1];
    // Original UpdatePlayerEntities clears actual DRA hitboxState after all callbacks.
    // Retained attack owners stay disabled until their own callback writes a new hitbox.
    if (SotnPlayerStep_IsDead(p.step) || (p.step == SOTN_PLAYER_TELEPORT && !p.substep))
    {
        r.wing_mask = r.wolf_mask = 0;
        r.fire_mask = r.spirit_mask = r.hellfire_mask = r.sword_mask = r.axe_mask = r.neutron_mask =
            r.sire_mask = r.pentagram_mask = r.magic_mask = r.thrown_mask = 0;
        r.soul_attack = r.echo_attack = false;
        for (Uint16 i = 0; i < 14; ++i)
            r.wolf.body.parts[i].hitbox_state = 0;
        for (Uint16 i = 0; i < 2; ++i)
            r.wolf_attacks[i].active = false;
        for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        {
            SotnBatRuntimeChild *c = &r.children[i];
            if (c->kind == 36 || c->kind == 50 || c->kind == 52 || c->kind == 53)
                c->state.attack.active = false;
            else if (c->kind == 45)
                c->state.fire.attack.active = false;
            else if (c->kind == 26 || c->kind == 27)
                c->state.hellfire_fire.attack.active = false;
            else if (c->kind == 61)
                c->state.spirit.attack.active = false;
            else if (!c->thrown && (c->kind == 0xe2 || c->kind == 0xf2))
                c->state.sword.attack.active = false;
            else if (c->pentagram)
                c->state.pentagram.attack.active = false;
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
    if ((inventory_changed || progress.random != previous_random || progress.mp != previous_mp) &&
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
    const bool magic = r && i < 48 && r->children[i].magic &&
                       (r->children[i].kind == 0xe1 || r->children[i].kind == 0xf1);
    if (!r || i >= 48 || !flags || flags > 3 ||
        (!magic && r->children[i].kind != 45 && r->children[i].kind != 26 &&
         r->children[i].kind != 27) ||
        !generation || r->children[i].generation != generation || !r->children[i].initialized)
        return SDL_SetError("Bat fireball feedback requires its actual live child generation");
    if (magic)
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
