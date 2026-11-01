#include <SDL3/SDL.h>
#include <sotn/familiar_events.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put(Uint8 *p, Uint32 value)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(value >> (i * 8U));
}
static void Event(Uint8 *p, Uint16 index, Sint32 room, Uint32 condition, Uint32 delay, bool once)
{
    p += index * 48U;
    Put(p + 8, 3);
    Put(p + 12, (Uint32)room);
    Put(p + 16, 7);
    Put(p + 20, 10);
    Put(p + 24, 20);
    Put(p + 28, condition);
    Put(p + 32, delay);
    Put(p + 36, 0xdc);
    Put(p + 40, index);
    Put(p + 44, once);
}
static bool Run(void)
{
    Uint8 table[6 * 48] = {0};
    Event(table, 0, -1, 0, 0, false);
    Event(table, 1, 5, SDL_MAX_UINT32, 0, true);
    Event(table, 2, 5, 0x80000002, 2, true);
    Event(table, 3, -5, 0x40000007, 0, true);
    Event(table, 4, 5, SDL_MAX_UINT32, 1, false);
    Event(table, 5, -1, 0, 0, false);
    const SotnFamiliarEventSource source = {{table, sizeof(table)}, 0x80170000, 6};
    SotnFamiliarEvents events = {0};
    SotnWorldState world = {0};
    SotnFamiliarEventInput in = {.world = &world,
                                 .room_x = 5,
                                 .room_y = 7,
                                 .camera_x = 10,
                                 .camera_y = 20,
                                 .servant = 3,
                                 .occupied = 1};
    SotnFamiliarEventPlan plan = {0};
    CHECK(SotnFamiliarEvents_Init(&events, source) && events.count == 6 &&
          SotnFamiliarEvents_Tick(&events, source, in, &plan) && plan.count == 1 &&
          plan.births[0].slot == 6 && plan.births[0].record == 1 && plan.occupied == 3 &&
          events.next[0] == 2 && events.next[2] == 4 && !events.next[4] && events.timer[2] == 1 &&
          !events.timer[4]);
    CHECK(SotnFamiliarEvents_Tick(&events, source, in, &plan) && plan.count == 1 &&
          plan.births[0].record == 4 && !events.timer[2] && events.timer[4] == 1);
    CHECK(SotnFamiliarEvents_Tick(&events, source, in, &plan) && plan.count == 1 &&
          plan.births[0].record == 2 && !events.timer[4] && events.next[0] == 4 &&
          events.next[2] == 4);
    // Optional exhaustion consumes one-shots and resets repeating timers without replacement.
    CHECK(SotnFamiliarEvents_Reset(&events));
    in.occupied = 7;
    CHECK(SotnFamiliarEvents_Tick(&events, source, in, &plan) && !plan.count &&
          events.next[0] == 2 && SotnFamiliarEvents_Tick(&events, source, in, &plan) &&
          !plan.count && events.timer[4] == 1 &&
          SotnFamiliarEvents_Tick(&events, source, in, &plan) && !plan.count &&
          events.next[0] == 4);
    // Delays hold when camera or condition does not match, including an already queued entry.
    CHECK(SotnFamiliarEvents_Reset(&events));
    world.flags[2] = 1;
    in.occupied = 1;
    CHECK(SotnFamiliarEvents_Tick(&events, source, in, &plan) && events.timer[2] == 1);
    for (Uint16 i = 0; i < 4; ++i)
        CHECK(SotnFamiliarEvents_Tick(&events, source, in, &plan) && events.timer[2] == 1);
    world.flags[2] = 0;
    in.camera_x = 11;
    CHECK(SotnFamiliarEvents_Tick(&events, source, in, &plan) && events.timer[2] == 1);
    in.camera_x = 10;
    CHECK(SotnFamiliarEvents_Tick(&events, source, in, &plan) && !events.timer[2]);
    in.stage = 32;
    // Stage alone does not rebuild: the original cache keys are servant and room coordinates.
    CHECK(SotnFamiliarEvents_Tick(&events, source, in, &plan));
    CHECK(SotnFamiliarEvents_Reset(&events) &&
          SotnFamiliarEvents_Tick(&events, source, in, &plan) && plan.count == 1 &&
          plan.births[0].record == 3 && !events.next[0]);
    world.relics[7] = 1;
    CHECK(SotnFamiliarEvents_Reset(&events) &&
          SotnFamiliarEvents_Tick(&events, source, in, &plan) && !plan.count &&
          events.next[0] == 3);
    world.relics[7] = 3;
    CHECK(SotnFamiliarEvents_Tick(&events, source, in, &plan) && !plan.count);
    world.relics[7] = 2; // Found bit matters, not active bit alone.
    CHECK(SotnFamiliarEvents_Tick(&events, source, in, &plan) && plan.count == 1 &&
          plan.births[0].slot == 6 && !events.next[0]);
    in.stage = 0;
    Put(table + 2 * 48 + 28, 0xc0000007);
    world.flags[7] = 1;
    world.relics[7] = 0;
    CHECK(SotnFamiliarEvents_Reset(&events) &&
          SotnFamiliarEvents_Tick(&events, source, in, &plan) && events.timer[2] == 1 &&
          SotnFamiliarEvents_Tick(&events, source, in, &plan) && events.timer[2] == 1);
    world.flags[7] = 0;
    world.relics[7] = 1;
    CHECK(SotnFamiliarEvents_Tick(&events, source, in, &plan) && events.timer[2] == 1);
    world.relics[7] = 2;
    CHECK(SotnFamiliarEvents_Tick(&events, source, in, &plan) && !events.timer[2]);
    // A late bad conditioned row cannot commit an earlier immediate birth or changed queue.
    in.stage = 0;
    CHECK(SotnFamiliarEvents_Reset(&events));
    Put(table + 2 * 48 + 28, 0x8000ffff);
    const SotnFamiliarEvents before = events;
    const SotnFamiliarEventPlan output = {.count = 123};
    plan = output;
    CHECK(!SotnFamiliarEvents_Tick(&events, source, in, &plan) &&
          !SDL_memcmp(&events, &before, sizeof(events)) &&
          !SDL_memcmp(&plan, &output, sizeof(plan)));
    Put(table + 2 * 48 + 28, 0x80000002);
    CHECK(SotnFamiliarEvents_Tick(&events, source, in, &plan));
    events.next[4] = 2;
    const SotnFamiliarEvents cycle = events;
    plan = output;
    CHECK(!SotnFamiliarEvents_Tick(&events, source, in, &plan) &&
          !SDL_memcmp(&events, &cycle, sizeof(events)) &&
          !SDL_memcmp(&plan, &output, sizeof(plan)));
    SotnFamiliarEvents untouched = before;
    Put(table + 5 * 48 + 12, 0);
    CHECK(!SotnFamiliarEvents_Init(&untouched, source) &&
          !SDL_memcmp(&untouched, &before, sizeof(before)));
    return true;
}
int main(void)
{
    return Run() ? 0 : 1;
}
