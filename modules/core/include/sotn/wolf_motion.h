#ifndef SOTN_WOLF_MOTION_H
#define SOTN_WOLF_MOTION_H
#include <sotn/animation.h>

/** @brief Original signed whole-position words; fractional positions remain with the owner. */
typedef struct SotnWolfPoint
{
    Sint16 x, y;
} SotnWolfPoint;
/** @brief Copied body positions at the original tail's capture-bounds update point.
 * Parts index actual DRA slots 17..30, in that order. These inputs confer no
 * allocation/readiness; the native owner must validate the complete body generation. */
typedef struct SotnWolfBodyPositions
{
    SotnWolfPoint player, parts[14];
    bool facing_left;
} SotnWolfBodyPositions;
/** @brief Original raw capture offsets in right/bottom/left/top order.
 * These are not normalized rectangle limits. */
typedef struct SotnWolfCaptureBounds
{
    Sint32 right, bottom, left, top;
} SotnWolfCaptureBounds;
/** @brief Derive original US asymmetric Wolf capture bounds from the articulated body.
 * @param positions Borrowed copied positions of actual owned parts.
 * @param out Receives raw original offsets; unchanged on invalid arguments.
 * @return Success or SDL error. Retains source min/max order and one-sided clamps;
 * arbitrary supplied coordinates can produce inverted offsets. No body is allocated. */
bool SotnWolf_CaptureBounds(const SotnWolfBodyPositions *positions, SotnWolfCaptureBounds *out);

/** @brief Copied Wolf controller state used by the original jump transition. */
typedef struct SotnWolfJumpFrame
{
    SotnAnimation animation;
    Sint32 velocity_x, velocity_y, body_angle;
    Uint32 contacts;
    Uint16 substep, mode, buttons;
    bool facing_left;
} SotnWolfJumpFrame;
/** @brief Apply the original Wolf jump helper after its controller's eligibility checks.
 * @param frame In/out copied state, retained on failure. Velocity X is bounded to 16 pixels/tick.
 * @return Success or SDL error for invalid state. Preserves sprint/slope and Up/swim ordering,
 * resets the actual animation cursor, and wraps the original body-angle word.
 * This helper performs no resource, factory, body, collision or rendering update. */
bool SotnWolf_Jump(SotnWolfJumpFrame *frame);

/** @brief Original controller post-step body angle and mutable run-frame durations. */
typedef struct SotnWolfCadence
{
    Sint32 body_angle;
    Uint16 durations[8];
} SotnWolfCadence;
/** @brief Derive the original eight run-frame duration overrides and body-angle clamp.
 * @param velocity_x Signed 16.16 speed, bounded to 16 pixels/tick.
 * @param body_angle Original signed word before the post-controller clamp.
 * @param out Receives overrides only; private sprite/frame words remain with the caller.
 * @return Success or SDL error without output mutation. Does not advance animation or time. */
bool SotnWolf_Cadence(Sint32 velocity_x, Sint32 body_angle, SotnWolfCadence *out);
#endif
