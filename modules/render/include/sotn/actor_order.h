#ifndef SOTN_ACTOR_ORDER_H
#define SOTN_ACTOR_ORDER_H
#include <SDL3/SDL.h>
/** @brief Stage draw command; grouped actors retain their existing sprite/primitive order. */
typedef enum SotnActorDrawKind
{
    SOTN_DRAW_ACTOR = 0,
    SOTN_DRAW_SPRITE,
    SOTN_DRAW_PRIMITIVE,
    SOTN_DRAW_MIST /**< Owned player visual; primitive indexes 0..79, actor is unused. */
} SotnActorDrawKind;
/** @brief Bounded frame-local command; actor indexes the caller's immutable snapshot list. */
typedef struct SotnActorDraw
{
    Uint16 priority, actor, primitive;
    Uint8 kind;
} SotnActorDraw;
/** @brief Append a command in stable, ascending original ordering-table priority.
 * @param items Borrowed writable storage for capacity entries. @param capacity Entry capacity.
 * @param count Owned initialized length. @param item Copied, validated command.
 * @return Success or SDL error; storage/count remain unchanged on failure.
 * @note Higher priorities cover lower priorities. Equal priorities retain insertion order;
 * this adapter does not emulate every PS1 GPU packet/engine reservation. */
bool SotnActorOrder_Add(SotnActorDraw *items, size_t capacity, size_t *count, SotnActorDraw item);
#endif
