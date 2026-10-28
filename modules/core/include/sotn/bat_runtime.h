#ifndef SOTN_BAT_RUNTIME_H
#define SOTN_BAT_RUNTIME_H

#include <sotn/bat_echo.h>
#include <sotn/bat_fireball.h>
#include <sotn/bat_trail.h>
#include <sotn/player.h>
#include <sotn/primitive_slots.h>

enum
{
    SOTN_BAT_RUNTIME_SLOTS = 48,
    SOTN_BAT_RUNTIME_FACTORIES = 8
};

/** @brief One actual selected DRA child, indexed by original slot minus sixteen. */
typedef struct SotnBatRuntimeChild
{
    SotnPlayerFactorySource source;
    Uint32 generation, bat_generation;
    Uint16 kind, parameter;
    Uint8 group;
    bool initialized;
    union {
        SotnPlayerBlink blink;
        SotnBatTrail trail;
        SotnPlayerAttack attack;
        struct
        {
            SotnBatFireball motion;
            SotnPlayerAttack attack;
            Uint16 pose, remaining, frame;
            Uint8 hit_flags;
        } fire;
    } state;
    SotnPlayerEffectQuad trail_quad;
    Uint8 draw_count;
    Uint16 primitives[8]; /**< Actual shared rows; eight for BlinkWhite, one for a trail. */
    Uint8 primitive_count;
} SotnBatRuntimeChild;

/** @brief Owned selected Bat factories/children sharing original DRA slot indices.
 * Zero-initialize for a fresh stage. The owner supplies other actual occupied slots;
 * this service never overrides their availability or invents visual readiness. */
typedef struct SotnBatRuntime
{
    struct
    {
        SotnPlayerFactory cursor;
        Uint32 bat_generation;
    } factories[SOTN_BAT_RUNTIME_FACTORIES];
    SotnBatRuntimeChild children[SOTN_BAT_RUNTIME_SLOTS];
    SotnPlayerBlinkShared shared;
    SotnBatEcho echo; /**< Single original slot-47 wave; not multiplied across the child union. */
    Uint16 echo_primitives[SOTN_BAT_ECHO_PRIMITIVES];
    Uint32 echo_generation;
    Uint32 serial;
    Uint16 wing_mask;
    Uint64 fire_mask;
    Uint16 sounds[48], sound_count;
    Uint8 use_ticks;
    Uint8 groups[11];
    bool echo_attack;
} SotnBatRuntime;

/** @brief Actual owned context and room/camera translation at this update point. */
typedef struct SotnBatRuntimeInput
{
    SotnDataView dra, executable;
    SotnPlayerState player;
    SotnPlayerImage image;
    Uint64 other_occupied;
    SotnPrimitiveSlots primitive_slots; /**< Snapshot of actual stage/player primitive occupancy. */
    Sint32 camera_x, camera_y;
    Sint16 room_shift_x, room_shift_y, screen_shift_x, screen_shift_y;
    bool entering;
    bool force_echo_enabled; /**< Found-and-enabled world snapshot, sampled on real Echo birth. */
} SotnBatRuntimeInput;

/** @brief Current selected occupied child/factory slots for shared allocation.
 * @param runtime Borrowed owner. @return Actual nonzero owners, or zero for NULL. */
Uint64 SotnBatRuntime_Occupied(const SotnBatRuntime *runtime);
/** @brief Whether an actual factory slot 8..15 is free in this selected owner. */
bool SotnBatRuntime_Available(const SotnBatRuntime *runtime);
/** @brief Find the actual current transformation child or pending real factory.
 * @param runtime Borrowed owner. @param generation Current nonzero Bat identity.
 * @param effect Receives borrowed child or NULL. @param pending Receives borrowed factory or NULL.
 * @return Success or SDL error for duplicate ownership/invalid outputs. Views last until next Tick.
 */
bool SotnBatRuntime_Context(const SotnBatRuntime *runtime, Uint32 generation,
                            const SotnPlayerBlink **effect, const SotnPlayerFactory **pending);
/** @brief Run actual selected factory requests/children once in original slot order.
 * @param runtime Owned stage lifetime. @param character Borrowed owner of spell stats/RNG.
 * @param input Actual player/artwork/context. Entering only rebases and never advances time.
 * @return Success or SDL error. Malformed inputs preserve runtime and character progress.
 * Supports selected Bat morphs, Wing/Fire children and the actual single Echo wave.
 * The owner must commit the planned rows to the actual primitive pool before advancing
 * other owners; no pool storage is mutated by this transactional update. */
bool SotnBatRuntime_Tick(SotnBatRuntime *runtime, SotnCharacter *character,
                         SotnBatRuntimeInput input);
/** @brief Borrow one child's current draw packets, valid until the next runtime update.
 * @param runtime Borrowed owner. @param index Original slot minus sixteen, 0..47.
 * @param quads Receives a borrowed contiguous view or NULL for no drawing. @param count 0..8.
 * @return Success or SDL error for invalid inputs. No allocation or simulation advances. */
bool SotnBatRuntime_Visual(const SotnBatRuntime *runtime, Uint16 index,
                           const SotnPlayerEffectQuad **quads, Uint16 *count);

/** @brief Deliver actual enemy contact to one generation-owned fireball.
 * @param runtime Borrowed owner. @param index Original slot minus sixteen.
 * @param generation Observed child identity. @param flags Original contact word, 1..3.
 * @return Success or SDL error without mutation on stale/non-fire ownership. */
bool SotnBatRuntime_Hit(SotnBatRuntime *runtime, Uint16 index, Uint32 generation, Uint8 flags);

#endif
