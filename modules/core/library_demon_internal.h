#ifndef SOTN_LIBRARY_DEMON_INTERNAL_H
#define SOTN_LIBRARY_DEMON_INTERNAL_H
#include "actors_internal.h"
/** @brief Advance selected windup particles; out signals the original spit release interval. */
bool ActorDemonWindup(SotnActors *actors, Actor *actor, bool *out);
/** @brief Advance original paired punch quad; packet-pair ownership remains native. */
bool ActorDemonPunchEffect(SotnActors *actors, Actor *actor);
/** @brief Advance original paired bolt windup star and its explicit release. */
bool ActorDemonStarEffect(SotnActors *actors, Actor *actor);
/** @brief Advance original thirty-record colored summoning column and minion factory. */
bool ActorDemonSummoning(SotnActors *actors, Actor *actor);
/** @brief Clear this body's effect reservation and original flag; no actor ownership transfers. */
void ActorDemonReleaseEffects(SotnActors *actors, Actor *actor);
/** @brief Checked fixed-point body movement; failures leave position intact. */
bool ActorDemonMove(Actor *actor);
/** @brief Apply original pose displacement and return the private wall/floor sensor decision. */
bool ActorDemonWalkFrame(SotnActors *actors, Actor *actor, bool *out);
/** @brief Queue the original five gray puffs using their original actor pool and parameters. */
bool ActorDemonPuffs(SotnActors *actors, const Actor *actor);
/** @brief Advance selected original Lesser Demon body or its owned punch hitbox. */
bool ActorLibraryDemon(SotnActors *actors, Actor *actor);
#endif
