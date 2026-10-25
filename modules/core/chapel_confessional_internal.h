#ifndef SOTN_CHAPEL_CONFESSIONAL_INTERNAL_H
#define SOTN_CHAPEL_CONFESSIONAL_INTERNAL_H
#include "actors_internal.h"
/** @brief Decoded private ghost profile; coordinates are original screen pixels. */
typedef struct ConfessionalProfile
{
    Sint32 left, edge, closed, speed, target, curtain_speed;
    size_t animations;
} ConfessionalProfile;
/** @brief Read one bounded original ghost profile; callers provide valid owner/output storage. */
bool ConfessionalRead(SotnActors *s, Uint16 parameters, ConfessionalProfile *out);
/** @brief Advance selected ghost or blades with valid borrowed stage/slot owners. */
bool ConfessionalGhost(SotnActors *s, Actor *a);
bool ConfessionalBlades(SotnActors *s, Actor *a);
static inline void ConfessionalStep(Actor *a, Uint16 step)
{
    a->state.step = step;
    a->substep = a->pose = a->remaining = 0;
}
static inline bool ConfessionalSeated(const SotnActors *s)
{
    return s->player.step == SOTN_PLAYER_STAND && s->player.substep == 4;
}
#endif
