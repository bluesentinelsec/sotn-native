// SPDX-License-Identifier: AGPL-3.0-or-later
// servant/process_event.h, shared_globals.h and create_event_entity.h (US), sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Event templates remain private.
#include <sotn/familiar_events.h>

typedef struct Event
{
    Sint32 servant, room_x, room_y, camera_x, camera_y;
    Uint32 condition, delay;
    Uint16 entity, parameter;
    bool once;
} Event;
static Uint32 Read32(const Uint8 *p)
{
    return p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
}
static Sint32 Signed(Uint32 bits)
{
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static bool Record(SotnFamiliarEventSource source, Uint16 index, Event *out)
{
    const Uint64 address = (Uint64)source.table + (Uint64)index * 48U;
    if (!source.overlay.bytes || source.count < 2 || source.count > SOTN_FAMILIAR_EVENT_RECORDS ||
        index >= source.count || address < 0x80170000U || address > SDL_MAX_UINT32 ||
        address - 0x80170000U > source.overlay.size ||
        source.overlay.size - (address - 0x80170000U) < 48)
        return SDL_SetError("Familiar room event references a truncated private record");
    const Uint8 *p = source.overlay.bytes + (size_t)(address - 0x80170000U);
    Event event = {.servant = Signed(Read32(p + 8)),
                   .room_x = Signed(Read32(p + 12)),
                   .room_y = Signed(Read32(p + 16)),
                   .camera_x = Signed(Read32(p + 20)),
                   .camera_y = Signed(Read32(p + 24)),
                   .condition = Read32(p + 28),
                   .delay = Read32(p + 32),
                   .entity = (Uint16)Read32(p + 36),
                   .parameter = (Uint16)Read32(p + 40),
                   .once = Read32(p + 44) != 0};
    if (index && index + 1U < source.count &&
        (event.servant < -1 || event.servant > 7 || event.room_x == -1 ||
         event.room_x == SDL_MIN_SINT32 || Read32(p + 36) < 0xd0 || Read32(p + 36) > 0xdf))
        return SDL_SetError("Familiar room event has invalid original identity or room metadata");
    if ((!index || index + 1U == source.count) && event.room_x != -1)
        return SDL_SetError("Familiar room events require original dummy and final sentinel");
    *out = event;
    return true;
}
bool SotnFamiliarEvents_Init(SotnFamiliarEvents *events, SotnFamiliarEventSource source)
{
    if (!events)
        return SDL_InvalidParamError("events");
    for (Uint16 i = 0; i < source.count; ++i)
    {
        Event record = {0};
        if (!Record(source, i, &record))
            return false;
    }
    if (source.count < 2 || source.count > SOTN_FAMILIAR_EVENT_RECORDS)
        return SDL_SetError("Familiar room events require the bounded actual original table");
    *events = (SotnFamiliarEvents){.count = source.count};
    return true;
}
bool SotnFamiliarEvents_Reset(SotnFamiliarEvents *events)
{
    if (!events || events->count < 2 || events->count > SOTN_FAMILIAR_EVENT_RECORDS)
        return SDL_SetError("Familiar event reset requires its actual initialized owner");
    events->servant = 0;
    events->room_x = events->room_y = 0;
    return true;
}
static bool Condition(Event event, const SotnWorldState *world, bool *allowed)
{
    *allowed = false;
    if (event.condition != SDL_MAX_UINT32)
    {
        const Uint16 index = (Uint16)event.condition;
        if (event.condition & 0x80000000U)
        {
            if (!world || index >= SOTN_CASTLE_FLAGS)
                return SDL_SetError("Familiar event lost its actual bounded castle flag");
            if (world->flags[index])
                return true;
        }
        if (event.condition & 0x40000000U)
        {
            if (!world || index >= SOTN_RELICS)
                return SDL_SetError("Familiar event lost its actual bounded relic ownership");
            if (world->relics[index] & 1U)
                return true;
        }
    }
    *allowed = true;
    return true;
}
static void Birth(SotnFamiliarEventPlan *plan, Event event, Uint16 record)
{
    for (Uint16 i = 0; i < SOTN_FAMILIAR_EVENT_BIRTHS; ++i)
        if (!(plan->occupied & (1U << i)))
        {
            plan->births[plan->count++] = (SotnFamiliarEventBirth){.slot = (Uint16)(5U + i),
                                                                   .entity = event.entity,
                                                                   .parameter = event.parameter,
                                                                   .record = record};
            plan->occupied |= (Uint8)(1U << i);
            return;
        }
}
static bool Rebuild(SotnFamiliarEvents *events, SotnFamiliarEventSource source,
                    SotnFamiliarEventInput in, SotnFamiliarEventPlan *plan)
{
    events->servant = in.servant;
    events->room_x = in.room_x;
    events->room_y = in.room_y;
    Uint16 tail = 0;
    for (Uint16 i = 1; i + 1U < source.count; ++i)
    {
        Event event = {0};
        if (!Record(source, i, &event))
            return false;
        if (event.servant != -1 && event.servant != in.servant)
            continue;
        const bool inverted = (in.stage & 32U) != 0;
        if ((event.room_x < 0) != inverted || SDL_abs(event.room_x) != in.room_x ||
            event.room_y != in.room_y)
            continue;
        if (event.camera_x == in.camera_x && event.camera_y == in.camera_y)
        {
            bool allowed = false;
            if (!Condition(event, in.world, &allowed))
                return false;
            if (allowed)
            {
                events->timer[i] = 0;
                if (!event.delay)
                {
                    Birth(plan, event, i);
                    if (event.once)
                        continue;
                }
            }
        }
        if (event.delay)
            events->timer[i] = event.delay - 1U;
        events->next[tail] = i;
        tail = i;
    }
    events->next[tail] = 0;
    return true;
}
static bool Advance(SotnFamiliarEvents *events, SotnFamiliarEventSource source,
                    SotnFamiliarEventInput in, SotnFamiliarEventPlan *plan)
{
    Uint16 previous = 0;
    Uint64 visited = 0;
    while (events->next[previous])
    {
        const Uint16 i = events->next[previous];
        if (i + 1U >= source.count || (visited & ((Uint64)1 << i)))
            return SDL_SetError("Familiar event queue contains an invalid link or cycle");
        visited |= (Uint64)1 << i;
        Event event = {0};
        if (!Record(source, i, &event))
            return false;
        if (event.camera_x == in.camera_x && event.camera_y == in.camera_y)
        {
            bool allowed = false;
            if (!Condition(event, in.world, &allowed))
                return false;
            if (allowed)
            {
                if (events->timer[i])
                    --events->timer[i];
                else
                {
                    Birth(plan, event, i);
                    if (event.once)
                    {
                        events->next[previous] = events->next[i];
                        continue;
                    }
                    events->timer[i] = event.delay;
                }
            }
        }
        previous = i;
    }
    return true;
}
bool SotnFamiliarEvents_Tick(SotnFamiliarEvents *events, SotnFamiliarEventSource source,
                             SotnFamiliarEventInput in, SotnFamiliarEventPlan *out)
{
    if (!events || !out || events->count != source.count || source.count < 2 ||
        source.count > SOTN_FAMILIAR_EVENT_RECORDS || in.occupied > 7 || in.servant > 7 ||
        in.stage > 255)
        return SDL_SetError("Familiar event update requires actual initialized source and slots");
    SotnFamiliarEvents next = *events;
    SotnFamiliarEventPlan plan = {.occupied = in.occupied};
    const bool rebuild =
        next.servant != in.servant || next.room_x != in.room_x || next.room_y != in.room_y;
    if (!(rebuild ? Rebuild(&next, source, in, &plan) : Advance(&next, source, in, &plan)))
        return false;
    *events = next;
    *out = plan;
    return true;
}
