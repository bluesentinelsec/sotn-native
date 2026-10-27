// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef SOTN_OLROX_INTERNAL_H
#define SOTN_OLROX_INTERNAL_H
#include "actors_internal.h"
/** @brief Advance BO0's reconstructed humanoid body or fading body image.
 * @param actors Borrowed arena services and private tables. @param actor Borrowed owned actor.
 * @return Success, or false with SDL error on invalid identity/state/resources/ownership.
 * Attack children and the transformation use their own original identities. */
bool ActorOlroxFirst(SotnActors *actors, Actor *actor);
/** @brief Reset the original animation cursor when changing an Olrox subphase. */
void OlroxSubstep(Actor *actor, Uint16 substep);
/** @brief Create/replace a generation-owned child in the body's original reserved block.
 * @param actors Borrowed arena owner. @param parent Borrowed active humanoid body.
 * @param relative Original relative slot, 1..13. @param id Original child identity.
 * @param parameters Original child parameters. @param out Receives borrowed child on success.
 * @return Success or SDL error; unrelated placed/dynamic actors are never replaced. */
bool OlroxReserved(SotnActors *actors, Actor *parent, Uint16 relative, Uint16 id, Uint16 parameters,
                   Actor **out);
/** @brief Get a generation-checked child from the body's original reserved block. */
Actor *OlroxChild(SotnActors *actors, const Actor *parent, Uint16 relative);
/** @brief Advance BO0 orbiting projectiles, portals or their owned bats/shadows.
 * @param actors Borrowed arena services and private tables. @param actor Borrowed owned actor.
 * @return Success, or false with SDL error on invalid state/resources/geometry.
 * Missing/reused parent generations retire dependent effects without touching new owners. */
bool ActorOlroxOrbs(SotnActors *actors, Actor *actor);
/** @brief Advance BO0's accelerating fireball or configured retirement effect.
 * @param actors Borrowed BO0 services and private tables. @param actor Borrowed active owner.
 * @return Success or SDL error for invalid identity/state/resources or stale geometry.
 * Optional primitive exhaustion retires only the fireball, preserving original behavior. */
bool ActorOlroxFire(SotnActors *actors, Actor *actor);
/** @brief Advance BO0's reserved skull-column attack and owned beam/feedback quads.
 * @param actors Borrowed BO0/private-table owner. @param actor Borrowed active column.
 * @return Success or SDL error for invalid state, data, math or owned geometry.
 * A missing parent retires an unreleased column; a released beam owns its remaining lifetime. */
bool ActorOlroxColumn(SotnActors *actors, Actor *actor);
#endif
