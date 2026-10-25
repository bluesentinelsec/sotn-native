#ifndef SOTN_CHAPEL_GLASS_INTERNAL_H
#define SOTN_CHAPEL_GLASS_INTERNAL_H
#include "actors_internal.h"
#include "boss_projection.h"
/** @brief Private glass helpers retain borrowed data and generation-owned primitives.
 * Callers provide a non-NULL entered stage-6 owner and active actor/output storage.
 * Models/poses remain borrowed for the call. All return success/SDL error;
 * projection outputs remain unchanged on invalid models. */
bool ChapelGlass_Reserve(SotnActors *s, Actor *a, Uint16 count);
bool ChapelGlass_Model(SotnActors *s, size_t offset, SotnGlassVertex *out, Uint16 count);
bool ChapelGlass_Project(SotnActors *s, SotnGlassTransform pose, const SotnGlassVertex model[4],
                         SotnActorPrimitive *out);
bool ChapelGlass_Background(SotnActors *s, Actor *a);
#endif
