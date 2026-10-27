// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef SOTN_OLROX_MONSTER_INTERNAL_H
#define SOTN_OLROX_MONSTER_INTERNAL_H
#include "olrox_internal.h"
typedef enum OlroxLegPose
{
    OLROX_FORWARD_SWING,
    OLROX_FORWARD_SUPPORT,
    OLROX_BACKWARD_SWING,
    OLROX_BACKWARD_SUPPORT,
    OLROX_TAKEOFF,
    OLROX_LOWER_BODY,
    OLROX_RAISE_BODY,
    OLROX_WIDE_STANCE,
    OLROX_LANDING,
    OLROX_CROUCH,
    OLROX_RECOVER,
    OLROX_SLOW_SWING,
    OLROX_SLOW_SUPPORT
} OlroxLegPose;
/** @brief Validate/bind the BO0 articulation owner without resetting its retained state.
 * @param actors Borrowed service. @param body Borrowed active original identity 0x33.
 * @return Success or SDL error when a different generation owns the service. */
bool OlroxMonster_Bind(SotnActors *actors, const Actor *body);
/** @brief Approach an original signed angle with the strict arrival boundary.
 * @param value Borrowed initialized angle. @param target Signed goal.
 * @param step Positive bounded change. @param arrived Receives original completion result.
 * @return Success or SDL error for invalid arguments, unchanged angle on error. */
bool OlroxMonster_Approach(Sint16 *value, Sint16 target, Sint16 step, bool *arrived);
/** @brief Approach a signed word with original acceleration and retained velocity.
 * @param value Borrowed angle. @param velocity Borrowed velocity word.
 * @param target Signed goal. @param speed Positive original speed cap.
 * @param arrived Receives original completion result. @return Success or SDL error. */
bool OlroxMonster_Smooth(Sint16 *value, Sint16 *velocity, Sint16 target, Sint16 speed,
                         bool *arrived);
/** @brief Advance one original two-leg pose controller without rendering or collision.
 * @param actors Borrowed BO0/owned leg service. @param body Borrowed active identity 0x33.
 * @param leg Active leg 0 or 1. @param pose Selected original controller.
 * @param arrived Original completion result for boolean controllers; false for void controllers.
 * @return Success or SDL error for invalid identity, generation or selector.
 * This does not register or update the monster actor. */
bool OlroxMonster_Pose(SotnActors *actors, Actor *body, Uint16 leg, OlroxLegPose pose,
                       bool *arrived);
/** @brief Change the original body state and reset only the two leg pose phases.
 * @param actors Borrowed owner. @param body Active monster. @param step Original low-byte state.
 * @return Success or SDL error; joint angles, velocities and body pose are retained. */
bool OlroxMonster_ChangeStep(SotnActors *actors, Actor *body, Uint16 step);
/** @brief Reconstruct original fixed-point hip/knee/foot anchoring.
 * @param actors BO0 owner with executable SDK lookup input. @param body Active monster.
 * @param from_hip Use the original airborne forward solution; otherwise anchor to its support foot.
 * @return Success or SDL error for missing owned feet or math input. Preserves the source's
 * four-pixel difference between the grounded and airborne hip origins. */
bool OlroxMonster_Anchor(SotnActors *actors, Actor *body, bool from_hip);
/** @brief Report the original flat arena floor adjustment for one leg.
 * @param actors Borrowed BO0 service. @param body Active monster. @param leg Leg 0 or 1.
 * @param adjustment Receives signed integer shift to the original floor, zero above it.
 * @return Success or SDL error; updates the owned original contact word. */
bool OlroxMonster_Floor(SotnActors *actors, Actor *body, Uint16 leg, Sint16 *adjustment);
/** @brief Reconstruct four original textured leg/foot packets from owned joint state.
 * @param actors Borrowed BO0/SDK inputs. @param body Active owned monster.
 * @param leg Leg 0 or 1. @param first First of four owned primitives. @param side Palette/priority
 * role 0 or 1.
 * @return Success or SDL error for invalid ownership, projection math or primitive capacity. */
bool OlroxMonster_LegGeometry(SotnActors *actors, Actor *body, Uint16 leg, Uint16 first,
                              Uint16 side);
/** @brief Reconstruct the two torso packets and positions of original owned roles 3 and 7.
 * @param actors Borrowed BO0/SDK inputs. @param body Active monster. @param first First of two
 * owned primitives.
 * @return Success or SDL error on private profile, math, primitive or child ownership failure. */
bool OlroxMonster_BodyGeometry(SotnActors *actors, Actor *body, Uint16 first);
/** @brief Convert the original bank-seven single-part selection to an owned sprite packet.
 * @param actors Borrowed BO0 owner. @param sprite Body or its generation-owned child.
 * @param body Original primitive owner. @param index Owned primitive receiving geometry/UV/palette.
 * @return Success or SDL error; preserves caller page, priority, visibility and blend attributes.
 * The original helper ignores part cropping and uses 127-unit texture offsets. */
bool OlroxMonster_SpriteGeometry(SotnActors *actors, const Actor *sprite, Actor *body,
                                 Uint16 index);
#endif
