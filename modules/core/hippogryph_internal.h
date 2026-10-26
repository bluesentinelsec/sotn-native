#ifndef SOTN_HIPPOGRYPH_INTERNAL_H
#define SOTN_HIPPOGRYPH_INTERNAL_H
#include "actors_internal.h"
/** @brief Resolve the original body floor probes and room-height fallback. */
bool HippogryphFloor(SotnActors *actors, Actor *actor, bool *landed);
/** @brief Request the original bounded body-particle factory; exhaustion consumes no RNG. */
bool HippogryphParticles(SotnActors *actors, Actor *actor, Uint8 count, Sint16 offset_y);
/** @brief Update the copied player handshake and queue the same value for the scene owner. */
void HippogryphGrab(SotnActors *actors, Actor *actor, SotnPlayerGrabState state);
/** @brief Advance original defeat state36, including optional primitive-allocation fallbacks. */
bool HippogryphDeath(SotnActors *actors, Actor *actor);
#endif
