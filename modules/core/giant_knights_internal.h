#ifndef SOTN_GIANT_KNIGHTS_INTERNAL_H
#define SOTN_GIANT_KNIGHTS_INTERNAL_H
#include "actors_internal.h"

/** @brief Resolve an articulated body-relative slot without exposing another actor's storage.
 * @param actors Borrowed stage owner. @param root Borrowed current body, including its update copy.
 * @param offset Body-relative index 0..19. @param out Receives a borrowed body/owned part.
 * @return Success or SDL error for a missing, stale or unrelated required joint. */
bool GiantPart(SotnActors *actors, Actor *root, Uint16 offset, Actor **out);
/** @brief Place a joint from its parent's pose, or reconstruct its parent from the joint.
 * @param actors Borrowed owner and SDK tables. @param root Borrowed current body update.
 * @param part Borrowed owned joint. @param inverse Select child-to-parent placement.
 * @return Success or SDL error; fixed-point wrap is explicitly defined. */
bool GiantPlace(SotnActors *actors, Actor *root, Actor *part, bool inverse);
/** @brief Advance a bounded original private pose record and its rotation velocities.
 * @param actors Borrowed private metadata. @param root Borrowed current body update.
 * @param list Offset of the terminated private joint list. @param poses Private pose-record offset.
 * @param channel Original independent pose counter 0..1.
 * @return Success or SDL error for invalid ownership, indices, pointers or zero-duration poses. */
bool GiantPose(SotnActors *actors, Actor *root, size_t list, size_t poses, Uint16 channel);
/** @brief Apply original joint velocities and selected forward/inverse anchoring order.
 * @param actors Borrowed private metadata. @param root Borrowed current body update.
 * @param list Private terminated joint-list offset.
 * @param mode 0 rotates only, 1 rotates/forward-places, 2 inverse-anchors two joints,
 * 3 forward-anchors four joints, 4 inverse-anchors the extra Blade root at offset 18.
 * @return Success or SDL error for an invalid bounded list or missing required joint. */
bool GiantAdvance(SotnActors *actors, Actor *root, size_t list, Uint16 mode);
/** @brief Complete unplaced articulated joints using an original terminated private list.
 * @param actors Borrowed owner. @param root Borrowed current body update.
 * @param list Private list offset. @param finish Clear placement marks and copy body facing.
 * @return Success or SDL error; no ownership transfers. */
bool GiantParts(SotnActors *actors, Actor *root, size_t list, bool finish);
/** @brief Reset the two actual original pose channels and the body's ordinary step cursor.
 * @param root Borrowed current body. @param step Original target state.
 * @note Original loops also clear unused adjacent padding; native C clears only owned channels. */
void GiantStep(Actor *root, Uint16 step);
/** @brief Clear only articulated pose counters, retaining the ordinary state/substate.
 * @param root Borrowed body or joint with two owned pose channels. */
void GiantClearPose(Actor *root);
/** @brief Set the original shortest-turn velocity using signed division toward zero.
 * @param part Borrowed joint. @param target Original signed-angle target.
 * @param duration Positive original interpolation duration, no greater than 1024.
 * @return Success or SDL error for an invalid joint/duration. */
bool GiantAim(Actor *part, Sint16 target, Sint16 duration);
/** @brief Advance one independently interpolated joint and place it from its parent.
 * @param actors Borrowed owner/private SDK table. @param root Borrowed current body update.
 * @param part Borrowed owned joint. @param duration Original interpolation duration.
 * @return Success or SDL error; retains quantization rather than snapping the last angle. */
bool GiantBend(SotnActors *actors, Actor *root, Actor *part, Sint16 duration);
/** @brief Resolve a two-link original inverse-kinematics target using private SDK math.
 * @param actors Borrowed owner/SDK data. @param root Borrowed current body update.
 * @param first Borrowed proximal joint. @param second Borrowed distal target joint.
 * @return Success or SDL error for invalid lengths, ownership or SDK arithmetic bounds. */
bool GiantSolve(SotnActors *actors, Actor *root, Actor *first, Actor *second);
/** @brief Query original joint-ground classification and relative surface displacement.
 * @param actors Borrowed collision owner. @param root Borrowed current body update.
 * @param part Borrowed joint. @param collider In/out initial top offset and resulting query.
 * @return Success or SDL error; stores original flat/left/right terrain classification. */
bool GiantGround(SotnActors *actors, const Actor *root, Actor *part, SotnCollider *collider);
/** @brief Test the original sixty-four-pixel wall/ledge probes relative to a foot.
 * @param actors Borrowed collision owner. @param part Borrowed foot position.
 * @param facing Probe right when true. @param out Receives wall bit 2 and gap bit 4.
 * @return Success or SDL error; unchanged output on failure. */
bool GiantAhead(SotnActors *actors, const Actor *part, bool facing, Uint16 *out);
/** @brief Advance an original signed-angle target without declaring exact-step completion early.
 * @param value Borrowed signed angle. @param target Original target. @param step Positive
 * increment.
 * @return True only when the original strict threshold accepts the target. */
bool GiantToward(Sint16 *value, Sint16 target, Sint16 step);
#endif
