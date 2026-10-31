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
             r->children[i].kind == 61) &&
            r->children[i].group >= SDL_arraysize(r->groups))
            return SDL_SetError("Wolf replacement found an invalid actual shared attack group");
    for (Uint16 i = 1; i <= 14; ++i)
    {
        r->children[i] = (SotnBatRuntimeChild){0};
        r->spirit_mask &= ~((Uint64)1 << i);
        r->hellfire_mask &= ~((Uint64)1 << i);
    }
    r->wing_mask &= 0x4000U;
    SDL_zero(r->groups);
    for (Uint16 i = 0; i < SOTN_BAT_RUNTIME_SLOTS; ++i)
        if ((r->children[i].kind == 36 || r->children[i].kind == 45 || r->children[i].kind == 52 ||
             r->children[i].kind == 53 || r->children[i].kind == 26 || r->children[i].kind == 27 ||
             r->children[i].kind == 61) &&
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
            if (!SotnPlayerFactory_Begin(&r->factories[i].cursor, source, packed, 0))
                return false;
            r->factories[i].bat_generation = generation;
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
            r->factories[i].cursor = f;
            r->factories[i].bat_generation = 0;
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
    return kind == 2 || kind == 5 || kind == 11 || kind == 31 || kind == 35 || kind == 36 ||
           kind == 45 || kind == 52 || kind == 53 || kind == 61 || kind == 63 ||
           (kind >= 24 && kind <= 29);
}
static Uint32 ChildFlags(const SotnBatRuntimeChild *c)
{
    if (!c->initialized)
        return c->source.flags;
    switch (c->kind)
    {
    case 2:
        return 0x08110000U;
    case 5:
        return c->state.particles.flags;
    case 11:
    case 31:
        return 0x04870000U;
    case 35:
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
               (c->parameter >> 8) == 1 && !c->state.hellfire_trail.motion.removed &&
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
static bool Spawn(SotnBatRuntime *r, SotnPlayerFactoryChild child, Uint32 generation)
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
    // Zero the complete union storage before selecting a member larger than BlinkWhite.
    SDL_zero(c.state);
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
    if (c.kind == 31)
        c.state.outline.parameter = child.parameter;
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
    if (r.wolf_bound && !in.wolf_owned)
        return SDL_SetError("Shared ability owner cannot detach its actual Wolf stage lifetime");
    r.wolf_bound = in.wolf_owned;
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
        if (c->kind == 53 &&
            (i != 31 || c->primitive_count ||
             (c->initialized ? (c->group < 7 || c->group > 10 || c->state.attack.hit_type != 0x82 ||
                                c->state.attack.hit_effect != 2 || !c->state.attack.active)
                             : c->group != 0)))
            return SDL_SetError(
                "Soul Steal attack lost its actual slot47/group/metadata ownership");
        if (!SotnBatRuntime_HellfireChildValid(c, i))
            return SDL_SetError(
                "Hellfire child lost its actual generation/role/row/group lifecycle");
        if (!SpiritRows(c) ||
            (c->kind == 61 &&
             (i < 1 || i > 31 || (c->initialized && (i < 16 ? c->group > 6 : c->group < 7)))))
            return SDL_SetError("Spirit child lost its actual motion/row/target/group ownership");
        if (c->kind == 31 &&
            (c->state.outline.parameter != c->parameter || c->state.outline.removed ||
             (c->initialized ? !c->state.outline.phase || c->state.outline.phase > 3
                             : c->state.outline.phase != 0)))
            return SDL_SetError("Player outline lost its actual child/profile lifecycle");
        if (c->primitive_count > SOTN_PLAYER_PARTICLE_LIMIT + 1 ||
            (c->primitive_count &&
             (!c->initialized || !((c->kind == 11 && c->primitive_count == 8) ||
                                   ((c->kind == 25 || c->kind == 28 || c->kind == 29 ||
                                     c->kind == 31 || c->kind == 35) &&
                                    c->primitive_count == 1) ||
                                   (c->kind == 61 && c->primitive_count == 16) ||
                                   (c->kind == 63 && c->primitive_count == 9) ||
                                   (c->kind == 5 && c->state.particles.phase == 1 &&
                                    !c->state.particles.removed && c->state.particles.count &&
                                    c->state.particles.count <= SOTN_PLAYER_PARTICLE_LIMIT &&
                                    c->primitive_count == c->state.particles.count + 1U)))) ||
            ((c->kind == 5 || c->kind == 11 || c->kind == 25 || c->kind == 28 || c->kind == 29 ||
              c->kind == 31 || c->kind == 35) &&
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
        if (c->kind == 5 &&
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
    r.use_ticks = 0;
    r.afterimage_reset = false;
    SotnPlayerState p = in.player;
    r.target_marks[0] = r.target_marks[1] = 0;
    r.capture_count = 0;
    r.freeze_changed = false;
    r.freeze = in.freeze;
    if ((p.water_pain.events.blink && !RequestSource(&r, p.water_pain.source, 0x4e002c, 0)) ||
        (p.water_pain.events.voice && !Sound(&r, 0x6eb)) ||
        (p.water_pain.events.scrape && !Sound(&r, 0x6a3)))
        return false;
    if (p.water_death.started &&
        (!Flash(&r, in.dra, 1) || !RequestSource(&r, p.water_death.source, 0x59002c, 0) ||
         !RequestSource(&r, p.water_death.source, 0x60031, 0)))
        return false;
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
    if (p.healing.applied && p.healing.applied_kind == 1 && !Sound(&r, 0x68e))
        return false;
    if (p.healing.effects &&
        (!RequestSource(&r, p.healing.source, 0x48002c, 0) ||
         (p.healing.applied_kind == 1 && !RequestSource(&r, p.healing.source, 0x44002c, 0))))
        return false;
    if (p.movement_effects.water_bubble &&
        !RequestSource(&r, p.movement_effects.bubble_source, 0xd0004U, 0))
        return false;
    if (p.movement_effects.smoke &&
        !RequestSource(&r, p.movement_effects.smoke_source, p.movement_effects.smoke_blueprint, 0))
        return false;
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
        if (events.clear_weapon_state)
            return SDL_SetError("Bat runtime received an unintegrated weapon replacement");
        for (Uint16 j = 0; j < events.count; ++j)
            if (!Spawn(&r, events.children[j], r.factories[i].bat_generation))
                return false;
    }
    SotnCharacterState progress = {0};
    SotnCharacterStats stats = {0};
    if (!SotnCharacter_Get(character, &progress, &stats))
        return false;
    const Uint32 previous_random = progress.random;
    const Uint16 previous_mp = progress.mp;
    r.wing_mask = 0;
    r.fire_mask = r.spirit_mask = r.hellfire_mask = 0;
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
            if (c->initialized && c->kind == 36)
                r.wing_mask |= (Uint16)(1U << (i - 1U));
            if (c->initialized && c->kind == 45)
                r.fire_mask |= (Uint64)1 << i;
            if (c->initialized && (c->kind == 26 || c->kind == 27))
                r.hellfire_mask |= (Uint64)1 << i;
            if (c->initialized && c->kind == 52)
                r.echo_attack = previous_echo_attack;
            if (c->initialized && c->kind == 53)
                r.soul_attack = true;
            continue;
        }
        bool removed = false;
        c->draw_count = 0;
        if (c->kind == 2)
        {
            SotnPlayerFactorySource source = c->source;
            source.x = FixedShift(source.x, (Sint16)-in.camera_x);
            source.y = FixedShift(source.y, (Sint16)-in.camera_y);
            if (!SotnPlayerSmoke_Tick(&c->state.smoke, in.dra, source, c->parameter, p.priority,
                                      p.contacts, p.status))
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
            source.x = FixedShift(source.x, (Sint16)-in.camera_x);
            source.y = FixedShift(source.y, (Sint16)-in.camera_y);
            if (!SotnPlayerParticles_Tick(
                    &c->state.particles, in.dra,
                    (SotnPlayerParticlesInput){.source = source,
                                               .parameter = c->parameter,
                                               .player_priority = p.priority,
                                               .player_x = Word(Pixel(p.x) - in.camera_x),
                                               .player_y = Word(Pixel(p.y) - in.camera_y),
                                               .player_step = (Uint16)p.step,
                                               .water_level = p.water.feet,
                                               .vectors = &r.particle_vectors,
                                               .allocated = allocated},
                    &progress.random))
                return false;
            const Sint32 x = Pixel(c->source.x) - in.camera_x, y = Pixel(c->source.y) - in.camera_y;
            removed = c->state.particles.removed || (!(c->state.particles.flags & 0x04000000U) &&
                                                     (x < -32 || x > 288 || y < -16 || y > 256));
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
                            .rotation = p.bat.rotation,
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
                                             .palette = 0x8100,
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
            if (!SotnHellfireTrail_Tick(
                    &c->state.hellfire_trail.motion,
                    (SotnHellfireTrailInput){.player_priority = p.priority,
                                             .random = random,
                                             .random_ready = birth,
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
                r.soul_attack = true;
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
             r.children[i].kind == 53) &&
            r.children[i].group > 2 && r.children[i].group < 11)
            ++r.groups[r.children[i].group];
    if ((progress.random != previous_random || progress.mp != previous_mp) &&
        !SotnCharacter_Restore(character, &progress))
        return false;
    *runtime = r;
    return true;
}
bool SotnBatRuntime_Visual(const SotnBatRuntime *r, Uint16 index,
                           const SotnPlayerEffectQuad **quads, Uint16 *count)
{
    if (!r || index >= 48 || !quads || !count)
        return SDL_SetError("Bat visual requires its actual bounded slot");
    const SotnBatRuntimeChild *c = &r->children[index];
    if (c->draw_count > 8 ||
        (c->draw_count && (!c->initialized || !((c->kind == 11 && c->primitive_count == 8) ||
                                                ((c->kind == 25 || c->kind == 28 || c->kind == 29 ||
                                                  c->kind == 31 || c->kind == 35) &&
                                                 c->primitive_count == 1) ||
                                                (c->kind == 61 && c->primitive_count == 16) ||
                                                (c->kind == 63 && c->primitive_count == 9)))))
        return SDL_SetError("Bat visual lost its actual packet owner");
    *count = c->draw_count;
    *quads = !c->draw_count                     ? NULL
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
    if (!r || i >= 48 || !flags || flags > 3 ||
        (r->children[i].kind != 45 && r->children[i].kind != 26 && r->children[i].kind != 27) ||
        !generation || r->children[i].generation != generation || !r->children[i].initialized)
        return SDL_SetError("Bat fireball feedback requires its actual live child generation");
    if (r->children[i].kind == 45)
        r->children[i].state.fire.hit_flags = flags;
    else
        r->children[i].state.hellfire_fire.hit_flags = flags;
    return true;
}
