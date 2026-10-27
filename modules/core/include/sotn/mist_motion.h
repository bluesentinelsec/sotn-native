#ifndef SOTN_MIST_MOTION_H
#define SOTN_MIST_MOTION_H

#include <sotn/transformation.h>

/** @brief Owned original Mist movement cursor; zero-initialize before supplying entry velocity. */
typedef struct SotnMistMotion
{
    Sint32 vx, vy;    /**< Original signed 16.16 velocities. */
    Uint32 remaining; /**< Original duration, distinct from the global MP cadence. */
    Sint16 settle_y;  /**< Original whole-pixel transformation offset still to consume. */
    Uint8 phase;      /**< Original control substep, 0 initialization or 1 movement. */
    bool facing_left;
    bool ended; /**< Restoration handoff requested; never means restoration completed. */
} SotnMistMotion;

/** @brief Copied per-simulation-tick inputs, independent of rendering and platform controls. */
typedef struct SotnMistInput
{
    const SotnWorldState *world; /**< Borrowed validated relic snapshot; unchanged. */
    Uint32 game_timer;
    Uint16 mp, directions; /**< Only original Up/Right/Down/Left bits. */
    bool cancel, underwater, direction_locked;
} SotnMistInput;

/** @brief Original movement-side requests for the owning player/effect integration. */
typedef struct SotnMistMotionEvents
{
    Uint16 spent;
    Sint16 shift_y; /**< Whole-pixel vertical adjustment, preserving the caller's fraction. */
    bool started;   /**< Perform original animation/weapon/palette/effect initialization. */
    bool ended;  /**< Enter the real unmorph controller; do not skip its effect/collision waits. */
    bool turned; /**< Original CheckMoveDirection facing change, separate from active steering. */
} SotnMistMotionEvents;

/** @brief Advance original Mist movement, duration, MP reduction and restoration requests.
 * @param motion Owned initialized cursor; entry eligibility is the caller's responsibility.
 * @param input Copied original input/clock/progress. @param out Receives this tick's requests.
 * @return Success, or SDL error with motion/output unchanged for malformed inputs or an ended
 * cursor. Rendering, collision, immunity, activation and the unmorph controller are separate
 * integration work; this component does not make Mist playable. */
bool SotnMistMotion_Tick(SotnMistMotion *motion, SotnMistInput input, SotnMistMotionEvents *out);

#endif
