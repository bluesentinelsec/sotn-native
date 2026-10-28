// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/7879C.c factories/BlinkWhite/WingSmashTrail/func_8011B480/func_80118894,
// 84B88.c:EntityBatFireball, 8BEF8.c:EntityBatEcho, alu_anim.c:UpdateAnim and
// 5D5BC.c:GetSpellDef, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Original blueprint/profile/spell/artwork data remains
// private.
#include <sotn/bat_runtime.h>
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
    return mask;
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
static bool Request(SotnBatRuntime *r, SotnPlayerState p, Uint32 packed, Uint32 generation)
{
    for (Uint16 i = 0; i < 8; ++i)
        if (!r->factories[i].cursor.entity_id)
        {
            if (!SotnPlayerFactory_Begin(
                    &r->factories[i].cursor,
                    (SotnPlayerFactorySource){
                        .x = p.x, .y = p.y, .priority = p.priority, .facing_left = p.facing_left},
                    packed, 0))
                return false;
            r->factories[i].bat_generation = generation;
            return true;
        }
    // Original CreateEntFactoryFromEntity returns NULL; optional commands never invent a child.
    if (generation)
        return SDL_SetError("Bat transformation lost its reserved actual factory slot");
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
    if (base > 900 || Read16(s + 16) != 2 || Read16(s + 20) > 255)
        return SDL_SetError("Bat attack has invalid spell attack metadata");
    Uint32 power = base + (stats.total[2] * 2U + SotnCombat_Random(&profile->random) % 12U) / 10U;
    if (profile->equipment[SOTN_EQUIP_ARMOR] == 21)
        power += power / 2U;
    if (power > 999)
        return SDL_SetError("Bat spell attack exceeds original native combat bounds");
    *attack = (SotnPlayerAttack){.offset_x = id == 6 ? 4 : 0,
                                 .width = id == 6 ? 12 : 4,
                                 .height = id == 6 ? 12 : 8,
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
static bool Spawn(SotnBatRuntime *r, SotnPlayerFactoryChild child, Uint32 generation)
{
    if (child.slot < 16 || child.slot >= 64 || child.replace || !child.entity_id ||
        (child.entity_id != 11 && child.entity_id != 35 && child.entity_id != 36 &&
         child.entity_id != 45 && child.entity_id != 52) ||
        r->children[child.slot - 16U].kind || r->serial == SDL_MAX_UINT32)
        return SDL_SetError("Bat factory produced an unsupported or occupied actual child slot");
    SotnBatRuntimeChild c = {.source = child.source,
                             .generation = ++r->serial,
                             .bat_generation = generation,
                             .kind = child.entity_id,
                             .parameter = child.parameter};
    if (c.kind == 11)
        c.state.blink.parameter = (child.parameter >> 8) & 0x7fU;
    if (c.kind == 35)
        c.state.trail = (SotnBatTrail){.x = child.source.x, .y = child.source.y};
    if (c.kind == 45)
        c.state.fire.motion = (SotnBatFireball){.x = child.source.x, .y = child.source.y};
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
static bool FireAnimation(SotnDataView dra, SotnBatRuntimeChild *c)
{
    Uint16 pose = c->state.fire.pose, remaining = c->state.fire.remaining;
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
    c->state.fire.pose = pose;
    c->state.fire.remaining = remaining;
    c->state.fire.frame = Read16(record + 2);
    return true;
}
bool SotnBatRuntime_Tick(SotnBatRuntime *runtime, SotnCharacter *character, SotnBatRuntimeInput in)
{
    if (!runtime || !character || in.camera_x < 0 || in.camera_x > 16384 || in.camera_y < 0 ||
        in.camera_y > 16384)
        return SDL_SetError(
            "Bat runtime requires actual owned progress and bounded camera context");
    if ((in.player.bat.started || in.player.bat.return_started) && !in.player.bat.generation)
        return SDL_SetError("Bat transformation requires a nonzero actual owner generation");
    if (SotnBatRuntime_Occupied(runtime) & in.other_occupied)
        return SDL_SetError("Bat runtime overlaps another actual ability owner");
    SotnBatRuntime r = *runtime;
    for (Uint16 i = 0; i < 48; ++i)
    {
        const SotnBatRuntimeChild *c = &r.children[i];
        if ((c->kind == 45 &&
             (i < 16 || i >= 32 || (c->initialized && (c->group < 7 || c->group > 10)))) ||
            (c->kind && (!c->generation || (c->kind != 11 && c->kind != 35 && c->kind != 36 &&
                                            c->kind != 45 && c->kind != 52))) ||
            (c->kind == 52 &&
             (i != 31 || (c->initialized && (r.echo_generation != c->generation ||
                                             (c->group && (c->group < 7 || c->group > 10)))))) ||
            (c->kind == 36 &&
             (i < 1 || i > 15 || (c->initialized && (c->group < 3 || c->group > 6)))))
            return SDL_SetError("Bat runtime has invalid actual child ownership or attack group");
    }
    if (r.echo_generation &&
        (r.children[31].kind != 52 || r.children[31].generation != r.echo_generation ||
         !r.children[31].initialized))
        return SDL_SetError("Bat Echo primitive owner no longer matches its actual child");
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
        if (c->kind == 45)
        {
            c->state.fire.motion.x = FixedShift(c->state.fire.motion.x, in.room_shift_x);
            c->state.fire.motion.y = FixedShift(c->state.fire.motion.y, in.room_shift_y);
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
    if (in.entering)
    {
        *runtime = r;
        return true;
    }
    r.sound_count = 0;
    r.use_ticks = 0;
    const SotnPlayerState p = in.player;
    if ((p.bat.started && !Request(&r, p, 0x20002cU, p.bat.generation)) ||
        (p.bat_commands.wing_flash && !Request(&r, p, 0x5c002cU, 0)) ||
        (p.bat_commands.wing_attack && !Request(&r, p, 67, 0)) ||
        (p.bat_commands.fire_flash && !Request(&r, p, 0x5002cU, 0)) ||
        (p.bat_commands.fireball && !Request(&r, p, 81, 0)) ||
        (p.bat_commands.echo && !Request(&r, p, 103, 0)) ||
        (p.bat.return_started && !Request(&r, p, 0x21002cU, p.bat.generation)) ||
        (p.bat_commands.trail && !Request(&r, p, 65, 0)))
        return false;
    if (SotnBatRuntime_Occupied(&r) & in.other_occupied)
        return SDL_SetError("Bat runtime overlaps another actual ability owner");
    for (Uint16 i = 0; i < 8; ++i)
    {
        if (!r.factories[i].cursor.entity_id)
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
    r.wing_mask = 0;
    r.fire_mask = 0;
    r.echo_attack = false;
    for (Uint16 i = 0; i < 48; ++i)
    {
        SotnBatRuntimeChild *c = &r.children[i];
        if (!c->kind)
            continue;
        bool removed = false;
        c->draw_count = 0;
        if (c->kind == 11)
        {
            if (!c->state.blink.phase && Duplicate(&r, i))
                removed = true;
            else
            {
                SotnPlayerBlinkEvents events = {0};
                const Uint16 parameter = c->state.blink.parameter;
                if (!SotnPlayerBlink_Tick(
                        &c->state.blink, &r.shared,
                        (SotnPlayerBlinkInput){.dra = in.dra,
                                               .executable = in.executable,
                                               .image = in.image,
                                               .x = Word(Pixel(p.x) - in.camera_x),
                                               .y = Word(Pixel(p.y) - in.camera_y),
                                               .rotation = p.bat.rotation,
                                               .priority = p.priority,
                                               .status = p.status,
                                               .player_step = (Uint16)p.step,
                                               .player_substep = p.substep,
                                               .signal = p.bat.signal,
                                               .facing_left = p.facing_left,
                                               .rotated = p.rotated,
                                               .image_present = in.image.width != 0},
                        &events))
                    return false;
                removed = c->state.blink.removed;
                if (parameter != c->state.blink.parameter)
                    c->parameter = (Uint16)(c->state.blink.parameter << 8);
                c->draw_count = removed ? 0 : 8;
            }
        }
        else if (c->kind == 35)
        {
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
                r.sounds[r.sound_count++] = events.sound;
                r.use_ticks = events.player_use_ticks;
            }
            c->source.x = c->state.fire.motion.x;
            c->source.y = c->state.fire.motion.y;
            c->source.facing_left = c->state.fire.motion.facing_left;
            const Sint32 x = Pixel(c->source.x) - in.camera_x, y = Pixel(c->source.y) - in.camera_y;
            removed |= x > 288 || x < -32 || y > 256 || y < -16;
            if (!removed)
            {
                if (!FireAnimation(in.dra, c))
                    return false;
                r.fire_mask |= (Uint64)1 << i;
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
            if (events.sound)
                r.sounds[r.sound_count++] = events.sound;
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
                r.echo_generation = 0;
                r.echo = (SotnBatEcho){0};
                SDL_zero(r.echo_primitives);
            }
        }
        else
            return SDL_SetError("Bat runtime child has an unsupported original identity");
        if (removed)
            *c = (SotnBatRuntimeChild){0};
    }
    SDL_memset(r.groups, 0, sizeof(r.groups));
    for (Uint16 i = 0; i < 48; ++i)
        if (r.children[i].kind == 36 || r.children[i].kind == 45 ||
            (r.children[i].kind == 52 && r.children[i].group))
            ++r.groups[r.children[i].group];
    if (progress.random != previous_random && !SotnCharacter_Restore(character, &progress))
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
    if (c->draw_count > 8 || (c->draw_count && c->kind != 11 && c->kind != 35))
        return SDL_SetError("Bat visual lost its actual packet owner");
    *count = c->draw_count;
    *quads = !c->draw_count ? NULL : c->kind == 11 ? c->state.blink.quads : &c->trail_quad;
    return true;
}

bool SotnBatRuntime_Hit(SotnBatRuntime *r, Uint16 i, Uint32 generation, Uint8 flags)
{
    if (!r || i >= 48 || !flags || flags > 3 || r->children[i].kind != 45 || !generation ||
        r->children[i].generation != generation || !r->children[i].initialized)
        return SDL_SetError("Bat fireball feedback requires its actual live child generation");
    r->children[i].state.fire.hit_flags = flags;
    return true;
}
