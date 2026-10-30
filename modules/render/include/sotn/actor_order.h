#ifndef SOTN_ACTOR_ORDER_H
#define SOTN_ACTOR_ORDER_H
#include <SDL3/SDL.h>
/** @brief Stage draw command; grouped actors retain their existing sprite/primitive order. */
typedef enum SotnActorDrawKind
{
    SOTN_DRAW_ACTOR = 0,
    SOTN_DRAW_SPRITE,
    SOTN_DRAW_PRIMITIVE,
    SOTN_DRAW_MIST,         /**< Owned player visual; primitive indexes 0..79, actor is unused. */
    SOTN_DRAW_BAT_FIREBALL, /**< Actual child slot index 0..47. */
    SOTN_DRAW_PLAYER_BLINK, /**< Original slots 16..63, eight packets each; indexes 0..383. */
    SOTN_DRAW_BAT_ECHO,     /**< Actual row 0..1023, owned by the single Echo child. */
    SOTN_DRAW_WOLF_PACKET,  /**< Actual prepared player/body packet, index0..59. */
    SOTN_DRAW_WOLF_IMAGE,   /**< Actual retained root image, index0..5. */
    SOTN_DRAW_WOLF_TARGET,  /**< Actual common environment begin0/restore1. */
    SOTN_DRAW_PLAYER_SMOKE, /**< Actual owned child index0..47. */
    SOTN_DRAW_PLAYER_PARTICLE, /**< Actual owned child/tile, index0..767. */
    SOTN_DRAW_SPIRIT,          /**< Actual initialized spirit child index0..47. */
    SOTN_DRAW_SPIRIT_PORTAL,   /**< Actual initialized portal child index0..47, black row. */
    SOTN_DRAW_HELLFIRE_SPRITE  /**< Actual initialized projectile/trail child index0..47. */
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
