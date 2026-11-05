// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected original servant/tt_002/faerie.c, common servant event helpers and
// dra/7879C.c UpdatePlayerEntities/func_8011A870, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Native slots/rows use actual owners.
#include <SDL3/SDL.h>
#include <sotn/actors.h>
#include <sotn/fairy_runtime.h>

static Uint32 Generation(SotnFairyRuntime *r)
{
    for (;;)
    {
        if (!++r->generation)
            ++r->generation;
        bool used = false;
        for (Uint16 i = 0; i < 4; ++i)
            used |= r->slots[i].generation == r->generation;
        if (!used)
            return r->generation;
    }
}
static bool Fairy(Uint16 identity)
{
    return identity >= SOTN_FAIRY_FOLLOW && identity <= SOTN_FAIRY_HINT;
}
static void Source(SotnFairyRuntimeSlot *c)
{
    if (Fairy(c->identity))
    {
        const SotnFairy *f = &c->cursor.fairy;
        c->identity = f->mode;
        c->source = (SotnPlayerFactorySource){.x = f->x,
                                              .y = f->y,
                                              .flags = f->flags,
                                              .priority = f->priority,
                                              .facing_left = f->facing_left};
    }
    else if (c->identity == 0xdd && c->cursor.apple.phase)
    {
        c->source.x = c->cursor.apple.x;
        c->source.y = c->cursor.apple.y;
        c->source.flags = c->cursor.apple.flags;
    }
    else if (c->identity == 0xdf && c->cursor.item.phase)
    {
        c->source.x = c->cursor.item.x;
        c->source.y = c->cursor.item.y;
        c->source.flags = c->cursor.item.flags;
    }
    else if (c->identity == 0xde && c->cursor.sound.flags)
        c->source.flags = c->cursor.sound.flags;
}
static bool Create(SotnFairyRuntime *r, Uint16 index, const SotnFairyRuntimeSlot *parent,
                   Uint16 identity, Uint16 parameter)
{
    if (index < 1 || index > 3 || r->slots[index].identity ||
        !(Fairy(identity) || (identity >= 0xdc && identity <= 0xdf)) || !parent->generation)
        return SDL_SetError("Fairy event birth lost its actual5..7 slot, parent or callback");
    SotnFairyRuntimeSlot next = {.generation = Generation(r),
                                 .parent_generation = parent->generation,
                                 .identity = identity,
                                 .source = parent->source};
    next.source.flags = 0x04000000;
    if (Fairy(identity))
        next.cursor.fairy = (SotnFairy){.mode = identity,
                                        .parameter = parameter,
                                        .x = next.source.x,
                                        .y = next.source.y,
                                        .flags = next.source.flags,
                                        .priority = next.source.priority,
                                        .facing_left = next.source.facing_left};
    else if (identity == 0xde)
        next.cursor.sound.parameter = parameter;
    else if (identity == 0xdf)
        next.cursor.item.parameter = parameter;
    else if (identity == 0xdc)
        next.cursor.fairy.parameter = parameter; // DC has only its actual parameter, no animation.
    r->slots[index] = next;
    return true;
}
static bool LocalEvent(SotnFairyRuntime *r, const SotnFairyRuntimeSlot *parent, Uint16 identity,
                       Uint16 parameter)
{
    if (!identity)
        return true;
    for (Uint16 i = 1; i < 4; ++i)
        if (!r->slots[i].identity)
            return Create(r, i, parent, identity, parameter);
    return true; // Optional event exhaustion neither replaces wings nor prevents item spending.
}
static Uint8 EventOccupied(const SotnFairyRuntime *r)
{
    Uint8 occupied = 0;
    for (Uint16 i = 1; i < 4; ++i)
        if (r->slots[i].identity)
            occupied |= (Uint8)(1U << (i - 1U));
    return occupied;
}
static Sint16 SoundIndex(const SotnFairyRuntime *r)
{
    for (Sint16 i = 1; i < 4; ++i)
        if (r->slots[i].identity == 0xde)
            return i;
    return -1;
}
Uint64 SotnFairyRuntime_Occupied(const SotnFairyRuntime *r)
{
    Uint64 occupied = 0;
    if (r && r->initialized)
        for (Uint16 i = 0; i < 4; ++i)
            if (r->slots[i].identity)
                occupied |= (Uint64)1 << (i + 4U);
    return occupied;
}
bool SotnFairyRuntime_Init(SotnFairyRuntime *r, SotnActors *actors, SotnFairyRuntimeInput in,
                           Uint16 mode)
{
    if (!r || !actors || mode > 3 || (mode == 3 && !r->initialized))
        return SDL_SetError("Fairy initialization requires its actual lifecycle, stage and mode");
    SotnFairyRuntime next = *r;
    if (!next.initialized)
    {
        SDL_zero(next);
        next.sound_step =
            99; // Original shared_globals.h data initializer, not an audio-ready guess.
        if (!SotnFamiliarEvents_Init(&next.events, in.event_source))
            return false;
    }
    SotnFairy root = next.slots[0].cursor.fairy, wings = next.slots[1].cursor.fairy;
    bool reset = false;
    if (!SotnFairy_Init(&root, &wings, &next.history, in.fairy, mode, &reset) ||
        (reset && !SotnFamiliarEvents_Reset(&next.events)))
        return false;
    if (mode != 3)
    {
        if (next.slots[0].lease.count)
            return SDL_SetError("Fairy root4 cannot own a local event's primitive lease");
        next.slots[0] = (SotnFairyRuntimeSlot){
            .generation = Generation(&next), .identity = root.mode, .cursor.fairy = root};
        const SotnFairyEffectLease old = next.slots[1].lease;
        next.slots[1] = (SotnFairyRuntimeSlot){.generation = Generation(&next),
                                               .parent_generation = next.slots[0].generation,
                                               .identity = wings.mode,
                                               .cursor.fairy = wings};
        Source(&next.slots[0]);
        Source(&next.slots[1]);
        if (old.count && !SotnActors_CommitFairyEffect(actors, &old, NULL))
            return false;
    }
    next.initialized = true;
    *r = next;
    return true;
}
static bool FairyLeases(const SotnFairyRuntime *r, SotnFairyEffectLease leases[3])
{
    if (!r)
        return SDL_InvalidParamError("Fairy runtime");
    for (Uint16 i = 0; i < 4; ++i)
    {
        const SotnFairyRuntimeSlot *c = &r->slots[i];
        if ((!c->identity && c->lease.count) ||
            (c->identity &&
             (!r->initialized || !c->generation || c->identity < 0xd0 || c->identity >= 0xe0)) ||
            (c->lease.count && (!i || c->lease.slot != i + 4U || c->lease.identity != c->identity ||
                                c->lease.generation != c->generation)))
            return SDL_SetError(
                "Fairy clearing lost an actual entity/lease identity or generation");
        if (i)
            leases[i - 1U] = c->lease;
    }
    return true;
}
bool SotnFairyRuntime_Validate(const SotnFairyRuntime *r, const SotnActors *actors)
{
    SotnFairyEffectLease leases[3] = {0};
    return FairyLeases(r, leases) && SotnActors_ValidateFairyEffects(actors, leases);
}
bool SotnFairyRuntime_Clear(SotnFairyRuntime *r, SotnActors *actors)
{
    SotnFairyEffectLease leases[3] = {0};
    if (!FairyLeases(r, leases) || !SotnActors_ReleaseFairyEffects(actors, leases))
        return false;
    SDL_zero(r->slots);
    return true;
}
static Sint32 Pixel(Sint32 value)
{
    return value >= 0 ? value / 65536 : (Sint32)(-((-(Sint64)value + 65535) / 65536));
}
bool SotnFairyRuntime_UpdateSlot(SotnFairyRuntime *r, SotnActors *actors, Uint16 slot,
                                 SotnFairyRuntimeInput in, Uint32 *random,
                                 SotnFairyRuntimeEvents *out)
{
    if (!r || !r->initialized || !actors || !random || !out || slot < 4 || slot > 7 ||
        (in.gate_scene != 0 && in.gate_scene != 3 && in.gate_scene != 0xfff))
        return SDL_SetError("Fairy callback requires actual4..7 ownership and a stable scene gate");
    const Uint16 index = (Uint16)(slot - 4U);
    const SotnFairyRuntimeSlot *old = &r->slots[index];
    SotnFairyRuntimeEvents events = {0};
    if (!old->identity || (in.gate_scene == 3 && !(old->source.flags & 0x10000U)))
    {
        *out = events;
        return true;
    }
    if (!old->generation ||
        (old->lease.count && (old->lease.generation != old->generation || old->lease.slot != slot ||
                              old->lease.identity != old->identity)))
        return SDL_SetError("Fairy callback lost its actual event or primitive generation");
    SotnFairyRuntime next = *r;
    SotnFairyRuntimeSlot *c = &next.slots[index];
    const SotnFairyEffectLease before = c->lease;
    Uint32 rng = *random;
    bool removed = false;
    const Sint16 sound_index = SoundIndex(&next);
    in.fairy.sound_present = sound_index >= 0;
    in.fairy.sound_step = next.sound_step;
    in.sound.room_flags = in.fairy.room_flags;
    in.sound.cutscene = in.fairy.cutscene;
    if (Fairy(c->identity))
    {
        if (c->identity == SOTN_FAIRY_WINGS)
        {
            if (!next.slots[0].identity || !Fairy(next.slots[0].identity) ||
                c->parent_generation != next.slots[0].generation ||
                !SotnFairy_Wings(&c->cursor.fairy, &next.slots[0].cursor.fairy, &next.history,
                                 in.fairy.overlay))
                return SDL_SetError("Fairy wings lost their actual root4 parent generation");
        }
        else
        {
            if (!SotnFairy_Tick(&c->cursor.fairy, &next.history, in.fairy, &rng, &events.root))
                return false;
            Source(c);
            if (!LocalEvent(&next, c, events.root.event_mode, events.root.event_parameter))
                return false;
            if (events.root.process_room_events)
            {
                SotnFamiliarEventPlan plan = {0};
                in.room.occupied = EventOccupied(&next);
                if (!SotnFamiliarEvents_Tick(&next.events, in.event_source, in.room, &plan))
                    return false;
                for (Uint16 i = 0; i < plan.count; ++i)
                    if (!Create(&next, (Uint16)(plan.births[i].slot - 4U), c, plan.births[i].entity,
                                plan.births[i].parameter))
                        return false;
            }
            events.root.event_mode = events.root.event_parameter = 0;
            events.root.process_room_events = false;
        }
    }
    else if (c->identity == 0xdc)
    {
        Uint16 *phase = sound_index >= 0 ? &next.slots[sound_index].cursor.sound.phase : NULL;
        if (!SotnFairy_RoomEvent(&next.history, c->cursor.fairy.parameter, sound_index >= 0, phase))
            return false;
        removed = true;
    }
    else if (c->identity == 0xde)
    {
        if (!SotnFamiliarSfx_Tick(&c->cursor.sound, in.sound, &events.cd_command, &next.sound_step))
            return false;
        removed = c->cursor.sound.removed;
    }
    else if (c->identity == 0xdd || c->identity == 0xdf)
    {
        bool allocated = before.count != 0;
        SotnFairyEffectLease lease = before;
        if (!allocated)
        {
            lease = (SotnFairyEffectLease){.generation = c->generation,
                                           .slot = slot,
                                           .identity = c->identity,
                                           .count = SOTN_FAIRY_APPLE_ROWS};
            SotnPrimitiveSlots available = {0};
            if ((c->identity == 0xdf &&
                 !SotnFairyItemEffect_Rows(in.fairy.overlay, c->cursor.item.parameter,
                                           &lease.count)) ||
                !SotnActors_PrimitiveSlots(actors, &available) ||
                !SotnPrimitiveSlots_Reserve(&available, lease.count, lease.rows, &allocated))
                return false;
        }
        if (c->identity == 0xdd)
        {
            if (!SotnFairyAppleEffect_Tick(
                    &c->cursor.apple, (SotnFairyAppleEffectInput){.executable = in.fairy.executable,
                                                                  .source = c->source,
                                                                  .allocated = allocated}))
                return false;
            removed = c->cursor.apple.removed;
        }
        else
        {
            if (!SotnFairyItemEffect_Tick(
                    &c->cursor.item,
                    (SotnFairyItemEffectInput){.overlay = in.fairy.overlay,
                                               .source = c->source,
                                               .player_priority = in.fairy.player.priority,
                                               .allocated = allocated},
                    &rng))
                return false;
            removed = c->cursor.item.removed;
        }
        if (!removed)
            c->lease = lease;
    }
    else
        return SDL_SetError("Unsupported actual Fairy entity callback");
    Source(c);
    if (c->source.flags & 0x100000U)
        return SDL_SetError("Fairy slot requires an unbound original engine animation callback");
    if (!removed && !(c->source.flags & 0x04000000U) &&
        (Pixel(c->source.x) < -32 || Pixel(c->source.x) > 288 || Pixel(c->source.y) < -16 ||
         Pixel(c->source.y) > 256))
        removed = true;
    if ((before.count || c->lease.count) &&
        !SotnActors_CommitFairyEffect(actors, before.count ? &before : NULL,
                                      removed ? NULL : &c->lease))
        return false;
    if (removed)
        SDL_zero(*c);
    *random = rng;
    *r = next;
    events.updated = true;
    *out = events;
    return true;
}
static Sint32 Fixed(Uint32 value)
{
    return value <= SDL_MAX_SINT32 ? (Sint32)value : (Sint32)((Sint64)value - 0x100000000LL);
}
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return bits <= SDL_MAX_SINT16 ? (Sint16)bits : (Sint16)((Sint32)bits - 65536);
}
static void PacketShift(Uint16 mode, Sint16 camera_x, Sint16 camera_y, Sint32 player_x,
                        Sint32 player_y, bool room, Sint32 *x, Sint32 *y)
{
    *x = *y = 0;
    if (!room && (mode & 2U))
    {
        *x = camera_x;
        *y = camera_y;
    }
    else if (mode & (room ? 0x100U : 0x200U))
    {
        *x = -Pixel(player_x);
        *y = -Pixel(player_y);
    }
}
bool SotnFairyRuntime_Translate(SotnFairyRuntime *r, SotnActors *actors, Sint16 camera_x,
                                Sint16 camera_y, Sint32 player_x, Sint32 player_y, bool room)
{
    if (!r || !actors)
        return SDL_SetError("Fairy translation requires its actual namespace and stage pool");
    if (!r->initialized)
        return true;
    SotnFairyRuntime next = *r;
    // Validate every live lease, including sentinel, before retiring any sibling's rows.
    for (Uint16 i = 0; i < 4; ++i)
    {
        const SotnFairyRuntimeSlot *c = &r->slots[i];
        if ((!c->identity && c->lease.count) ||
            (c->identity && (!c->generation || !(Fairy(c->identity) ||
                                                 (c->identity >= 0xdc && c->identity <= 0xdf)))))
            return SDL_SetError("Fairy translation lost its actual slot identity/generation");
        if (c->lease.count && (c->lease.slot != i + 4U || c->lease.identity != c->identity ||
                               c->lease.generation != c->generation ||
                               !SotnActors_FairyEffectRow(actors, &c->lease, 0)))
            return SDL_SetError("Fairy translation lost its complete actual row lease");
        if ((c->identity == 0xdf &&
             (c->cursor.item.removed || c->cursor.item.phase > 2 ||
              c->cursor.item.count > SOTN_FAIRY_ITEM_PARTICLES ||
              (c->cursor.item.phase && c->lease.count != c->cursor.item.count + 1U))) ||
            (c->identity == 0xdd &&
             (c->cursor.apple.removed || c->cursor.apple.phase > 5 ||
              (c->cursor.apple.phase && c->lease.count != SOTN_FAIRY_APPLE_ROWS))))
            return SDL_SetError("Fairy translation lost its initialized particle/light cursor");
    }
    const SotnFairyEffectLease leases[3] = {r->slots[1].lease, r->slots[2].lease,
                                            r->slots[3].lease};
    if (r->slots[0].lease.count || !SotnActors_ValidateFairyEffects(actors, leases))
        return SDL_SetError("Fairy translation lost its complete actual5..7 row namespace");
    for (Uint16 i = 0; i < 4; ++i)
    {
        SotnFairyRuntimeSlot *c = &next.slots[i];
        if (!c->identity)
            continue;
        if (room && !(c->source.flags & 0x20000U))
        {
            SDL_zero(*c);
            continue;
        }
        Sint32 x = c->source.x, y = c->source.y;
        if (!room && (c->source.flags & 0x08000000U))
        {
            x = Fixed((Uint32)x + (Uint32)(Sint32)camera_x * 65536U);
            y = Fixed((Uint32)y + (Uint32)(Sint32)camera_y * 65536U);
        }
        if (room || (c->source.flags & 0x40000U))
        {
            x = Fixed((Uint32)x - (Uint32)player_x);
            y = Fixed((Uint32)y - (Uint32)player_y);
        }
        c->source.x = x;
        c->source.y = y;
        if (Fairy(c->identity))
        {
            c->cursor.fairy.x = x;
            c->cursor.fairy.y = y;
        }
        else if (c->identity == 0xdf && c->cursor.item.phase)
        {
            c->cursor.item.x = x;
            c->cursor.item.y = y;
            for (Uint16 row = 0; row <= c->cursor.item.count; ++row)
            {
                SotnPlayerParticle *p = &c->cursor.item.tiles[row];
                Sint32 dx = 0, dy = 0;
                PacketShift(p->draw_mode, camera_x, camera_y, player_x, player_y, room, &dx, &dy);
                p->screen_x = Word(p->screen_x + dx);
                p->screen_y = Word(p->screen_y + dy);
            }
        }
        else if (c->identity == 0xdd && c->cursor.apple.phase)
        {
            c->cursor.apple.x = x;
            c->cursor.apple.y = y;
            for (Uint16 row = 0; row < SOTN_FAIRY_APPLE_ROWS; ++row)
            {
                SotnPlayerEffectQuad *p = &c->cursor.apple.quads[row];
                Sint32 dx = 0, dy = 0;
                PacketShift(p->draw_mode, camera_x, camera_y, player_x, player_y, room, &dx, &dy);
                for (Uint16 v = 0; v < 4; ++v)
                {
                    p->x[v] = Word(p->x[v] + dx);
                    p->y[v] = Word(p->y[v] + dy);
                }
            }
        }
    }
    for (Uint16 i = 0; i < 4; ++i)
        if (r->slots[i].lease.count && !next.slots[i].identity &&
            !SotnActors_CommitFairyEffect(actors, &r->slots[i].lease, NULL))
            return false; // All real leases were validated before any retirement above.
    *r = next;
    return true;
}
