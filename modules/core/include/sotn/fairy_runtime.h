#ifndef SOTN_FAIRY_RUNTIME_H
#define SOTN_FAIRY_RUNTIME_H
#include <sotn/fairy.h>
#include <sotn/fairy_effects.h>
#include <sotn/familiar_events.h>

typedef struct SotnActors SotnActors;
/** @brief One real original slot4..7, with a single active identity/cursor and actual row lease. */
typedef struct SotnFairyRuntimeSlot
{
    Uint32 generation, parent_generation;
    Uint16 identity;
    SotnPlayerFactorySource source; /**< Original screen-space source, including full fractions. */
    SotnFairyEffectLease lease;
    union {
        SotnFairy fairy;
        SotnFairyItemEffect item;
        SotnFairyAppleEffect apple;
        SotnFamiliarSfx sound;
    } cursor;
} SotnFairyRuntimeSlot;
/** @brief Actual four-slot familiar namespace and original overlay globals, not another pool.
 * The native lifecycle must bind real resources/selection before initializing these slots.
 * Room initialization retains event6/7 and sound/event history; full replacement retires leases.
 * None of this transient ownership belongs in ordinary saves. */
typedef struct SotnFairyRuntime
{
    SotnFairyRuntimeSlot slots[4];
    SotnFairyHistory history;
    SotnFamiliarEvents events;
    Uint32 generation;
    Sint32 sound_step;
    bool initialized;
} SotnFairyRuntime;
/** @brief Current actual player/world/room and native CD/XA snapshots at this slot callback. */
typedef struct SotnFairyRuntimeInput
{
    SotnFairyInput fairy;
    SotnFamiliarEventSource event_source;
    SotnFamiliarEventInput room;
    SotnFamiliarSfxInput sound;
    Uint16 gate_scene; /**< Snapshot before this4..63 pass:0 ordinary,3 frozen,FFF full4..7. */
} SotnFairyRuntimeInput;
/** @brief Remaining actual native requests; commit before advancing the next original slot.
 * Local events/room plans and real effect leases are already handled by this runtime.
 * Caller commits inventory/factory55/resources/scene/cleanup/pause/shake/VAB sounds and the
 * separate CD command, including immediate10/11 ACK-busy in its real native audio owner.
 * Requests do not complete these operations or enable Fairy controls by themselves. */
typedef struct SotnFairyRuntimeEvents
{
    SotnFairyEvents root;
    Uint16 cd_command;
    bool updated;
} SotnFairyRuntimeEvents;
/** @brief Bind original root4/wings5 for InitializeMode0..3 with real row retirement.
 * @param runtime Actual retained owner. @param actors Real stage pool.
 * @param input Actual private/player/room sources. @param mode Original initialization mode.
 * @return Success or SDL error before mutation on invalid private data or stale rows.
 * Native graphics/bank2 binding, selection/progression and sound lifecycle remain caller-owned. */
bool SotnFairyRuntime_Init(SotnFairyRuntime *runtime, SotnActors *actors,
                           SotnFairyRuntimeInput input, Uint16 mode);
/** @brief Run one actual slot4..7 once, in original slot order and callback phase.
 * @param runtime Actual owner. @param actors Actual stage primitive owner.
 * @param slot Original slot4..7. @param input Current live snapshots and stable pass gate.
 * @param random Copied actual libc RNG, published by the caller after native success.
 * @param out Remaining native operations; unchanged on failure.
 * @return Success or SDL error preserving this call's runtime/RNG/pool on failure.
 * Caller owns the whole-pass checkpoint and native operation rollback, real frame clock,
 * camera/player-lock translations, room cleanup and drawing. Never call this from a draw. */
bool SotnFairyRuntime_UpdateSlot(SotnFairyRuntime *runtime, SotnActors *actors, Uint16 slot,
                                 SotnFairyRuntimeInput input, Uint32 *random,
                                 SotnFairyRuntimeEvents *out);
/** @brief Actual low64 namespace occupancy; no slots are predicted or allocated by this read. */
Uint64 SotnFairyRuntime_Occupied(const SotnFairyRuntime *runtime);
/** @brief Original selected ServantWorkClear: retire actual Fairy4..7 entities and their rows.
 * @param runtime Actual overlay owner. @param actors Actual shared stage pool.
 * Validates every real lease, including late sibling sentinels and absent-lease orphans,
 * before retiring any entity or row. Preserves generation serial, overlay history/event
 * links/sound cursor and initialized binding for a later same-overlay initialization.
 * @return Success or SDL error preserving runtime/pool on failure. No callbacks, RNG,
 * audio, scene, resources or progression change; deselection lifecycle remains caller-owned. */
bool SotnFairyRuntime_Clear(SotnFairyRuntime *runtime, SotnActors *actors);
/** @brief Apply original post-callback camera/player or connected-room translation.
 * @param runtime Actual four-slot owner. @param actors Actual shared primitive pool.
 * @param camera_x Old minus new camera X, in whole pixels; ignored for room entry.
 * @param camera_y Old minus new camera Y, in whole pixels; ignored for room entry.
 * @param player_x Original old minus new player X minus ordinary shove, in16.16.
 * @param player_y Original old minus new player Y minus ordinary shove, in16.16.
 * @param room Retire non20000 slots before translating retained entities and flag100 rows.
 * Ordinary camera/player entity flags apply independently; packet2 precedes packet200.
 * Packet player shifts subtract the floored original displacement, preserving fractions.
 * @return Success or SDL error before mutation on invalid slots/cursors/stale real leases.
 * No callbacks, clocks, events, RNG or audio advance. Uninitialized ownership is a no-op.
 * Room lifecycle still must bind destination resources and initialize the actual root/wings. */
bool SotnFairyRuntime_Translate(SotnFairyRuntime *runtime, SotnActors *actors, Sint16 camera_x,
                                Sint16 camera_y, Sint32 player_x, Sint32 player_y, bool room);
#endif
