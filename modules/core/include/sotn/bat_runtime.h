#ifndef SOTN_BAT_RUNTIME_H
#define SOTN_BAT_RUNTIME_H

#include <sotn/bat_echo.h>
#include <sotn/bat_fireball.h>
#include <sotn/bat_trail.h>
#include <sotn/player.h>
#include <sotn/player_outline.h>
#include <sotn/player_particles.h>
#include <sotn/player_smoke.h>
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
        SotnPlayerParticles particles;
        SotnPlayerSmoke smoke;
        SotnPlayerOutline outline;
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
    Uint16 primitives[SOTN_PLAYER_PARTICLE_LIMIT + 1]; /**< Includes particle sentinel rows. */
    Uint8 primitive_count;
} SotnBatRuntimeChild;

/** @brief Owned selected Bat/Wolf factories and children sharing original DRA slot indices.
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
    Uint32 common_generation;  /**< Original global player eight-row lease, not a factory child. */
    Uint16 afterimage_rows[6]; /**< Hidden metadata rows; the final two rows live in wolf. */
    SotnWolfRuntime wolf;
    SotnWolfControllerFrame wolf_frame;
    SotnWolfRuntimeEvents wolf_events;
    SotnPlayerAttack
        wolf_attacks[2]; /**< Actual front18/head20 attacks, sharing enemy group three. */
    Uint8 wolf_mask;
    bool wolf_bound;  /**< This stage owner includes actual Wolf entities/common environment. */
    SotnBatEcho echo; /**< Single original slot-47 wave; not multiplied across the child union. */
    Uint16 echo_primitives[SOTN_BAT_ECHO_PRIMITIVES];
    Uint32 echo_generation;
    Uint32 serial;
    Uint16 wing_mask;
    Uint64 fire_mask;
    Uint16 sounds[48], sound_count;
    Uint8 wolf_shakes[8], wolf_shake_count;
    Uint8 use_ticks;
    Uint8 groups[11];
    bool echo_attack;
    bool afterimage_reset; /**< Actual BlinkWhite reset emitted after player presentation. */
} SotnBatRuntime;

/** @brief Actual owned context and room/camera translation at this update point. */
typedef struct SotnBatRuntimeInput
{
    SotnDataView dra, executable;
    SotnPlayerState player;
    SotnPlayerImage image;
    SotnWolfCaptureBounds wolf_bounds; /**< External bounds only when Wolf is not bound here. */
    SotnWolfCollisionQuery wolf_query;
    void *wolf_context;
    bool wolf_owned; /**< Bind the actual Wolf body to this shared entity update order. */
    Uint64 other_occupied;
    SotnPrimitiveSlots primitive_slots; /**< Snapshot of actual stage/player primitive occupancy. */
    Sint32 camera_x, camera_y;
    Sint32 wolf_command_camera_x, wolf_command_camera_y; /**< Raw controller capture camera;
                                                            zero for owned room-space exports. */
    Sint16 room_shift_x, room_shift_y, screen_shift_x, screen_shift_y;
    bool entering;
    bool force_echo_enabled; /**< Found-and-enabled world snapshot, sampled on real Echo birth. */
} SotnBatRuntimeInput;

/** @brief Reserve the original global player's eight rows before stage actors advance.
 * @param runtime Fresh zeroed stage owner. @param slots Actual stage primitive occupancy.
 * @return Success or SDL error without mutation, including exhausted capacity.
 * Reserves six hidden metadata rows and the same two common environment rows used by Wolf.
 * Commit through SotnActors_CommitBatPrimitives before another owner allocates. These rows
 * persist through body retirement and room changes; they confer no effect readiness or
 * afterimage display/texture ownership. Full stage reset requires a fresh stage owner. */
bool SotnBatRuntime_InitCommon(SotnBatRuntime *runtime, SotnPrimitiveSlots slots);

/** @brief Current selected occupied child/factory slots for shared allocation.
 * @param runtime Borrowed owner. @return Actual nonzero owners, or zero for NULL. */
Uint64 SotnBatRuntime_Occupied(const SotnBatRuntime *runtime);
/** @brief Whether an actual factory slot 8..15 is free in this selected owner. */
bool SotnBatRuntime_Available(const SotnBatRuntime *runtime);
/** @brief Perform original Wolf entry's forced replacement of selected slots17..30.
 * @param runtime Actual shared factory/child owner. @return Success or SDL error.
 * Retires only those original slots, retaining groups' enemy cooldowns in their combat owner.
 * Commit this retirement to the stage pool before reserving new Wolf root/environment rows. */
bool SotnBatRuntime_ReplaceWolfSlots(SotnBatRuntime *runtime);
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
 * Supports selected Bat morphs, Wing/Fire children, Echo and an optional actual Wolf body.
 * Bound Wolf runs at slots17..30; later flashes read its current capture bounds. The owning
 * player must commit wolf_frame once after success. Body MP charges commit here, not again
 * through wolf_events. Selected Wolf spell requests initialize real wolf_attacks here;
 * the native combat owner applies them. Selected Wolf puffs/particles use real factories
 * and primitive rows; the native scene must draw their retained state and consume shakes.
 * Actual entity31 outlines retain full parameters, share one row per child, and propagate
 * real afterimage resets. Normal spell/MP-refill/curse request producers remain separate.
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
