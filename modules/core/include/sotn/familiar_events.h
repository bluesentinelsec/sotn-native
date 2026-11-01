#ifndef SOTN_FAMILIAR_EVENTS_H
#define SOTN_FAMILIAR_EVENTS_H
#include <sotn/game_data.h>
#include <sotn/world.h>

enum
{
    SOTN_FAMILIAR_EVENT_RECORDS = 49,
    SOTN_FAMILIAR_EVENT_BIRTHS = 3
};
/** @brief Borrowed original US shared event records, including dummy0 and final sentinel. */
typedef struct SotnFamiliarEventSource
{
    SotnDataView overlay;
    Uint32 table; /**< Original table address, within the current familiar overlay at80170000. */
    Uint16 count; /**< At most the actual49-record US table; smaller authored fixtures are valid. */
} SotnFamiliarEventSource;
/** @brief Original per-record timer/next ownership, separate from immutable private templates.
 * Index0 is the real dummy queue head; zero next means NULL. This is not an actor pool.
 * Retain through rooms/pause/draws. Reinitialize only when the actual overlay is replaced.
 * None of these transient words belong in ordinary saves. */
typedef struct SotnFamiliarEvents
{
    Uint32 timer[SOTN_FAMILIAR_EVENT_RECORDS];
    Uint16 next[SOTN_FAMILIAR_EVENT_RECORDS];
    Sint32 room_x, room_y;
    Uint16 servant, count;
} SotnFamiliarEvents;
/** @brief Actual current room/camera/stage and original slot5..7 occupancy snapshots. */
typedef struct SotnFamiliarEventInput
{
    const SotnWorldState
        *world; /**< Borrowed actual flags/relics, needed only by conditioned rows. */
    Sint32 room_x, room_y;
    Sint16 camera_x, camera_y;
    Uint16 stage, servant;
    Uint8 occupied; /**< Bits0..2 are actual shared familiar slots5..7, including wings. */
} SotnFamiliarEventInput;
/** @brief One actual-slot birth intent; the native owner must validate/commit its real entity. */
typedef struct SotnFamiliarEventBirth
{
    Uint16 slot, entity, parameter, record;
} SotnFamiliarEventBirth;
/** @brief Original first-empty reservations. Optional exhaustion emits no replacement birth. */
typedef struct SotnFamiliarEventPlan
{
    SotnFamiliarEventBirth births[SOTN_FAMILIAR_EVENT_BIRTHS];
    Uint8 count, occupied;
} SotnFamiliarEventPlan;
/** @brief Validate original immutable templates and initialize their actual timer/link ownership.
 * @param events Owned output, unchanged on failure. @param source Borrowed private table.
 * @return Success or SDL error. This does not activate a familiar or allocate any native slot. */
bool SotnFamiliarEvents_Init(SotnFamiliarEvents *events, SotnFamiliarEventSource source);
/** @brief Original ProcessEvent(NULL,true): clear cached room/servant, retaining timer/links.
 * @param events Actual initialized owner. @return Success or SDL error without mutation. */
bool SotnFamiliarEvents_Reset(SotnFamiliarEvents *events);
/** @brief Advance original US ProcessEvent once, on the caller's actual familiar callback clock.
 * @param events Copied original event ownership. @param source Borrowed immutable templates.
 * @param input Real room/camera/flags/relics/occupancy snapshots. @param out Receives a birth plan.
 * @return Success or SDL error preserving owner/output on late bounds or malformed-link failure.
 * Allocation failure still consumes one-shot events and resets repeating timers as originally.
 * These plans supply no completed native slot, primitive, effect, audio or familiar availability.
 */
bool SotnFamiliarEvents_Tick(SotnFamiliarEvents *events, SotnFamiliarEventSource source,
                             SotnFamiliarEventInput input, SotnFamiliarEventPlan *out);
#endif
