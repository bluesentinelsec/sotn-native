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
    SOTN_DRAW_PLAYER_PARTICLE, /**< Actual owned child times30 plus tile, index0..1439. */
    SOTN_DRAW_SPIRIT,          /**< Actual initialized spirit child index0..47. */
    SOTN_DRAW_SPIRIT_PORTAL,   /**< Actual initialized portal child index0..47, black row. */
    SOTN_DRAW_HELLFIRE_SPRITE, /**< Actual initialized projectile/trail child index0..47. */
    SOTN_DRAW_RISING_HEART,    /**< Actual child51 index0..47; no primitive lease. */
    SOTN_DRAW_SWORD,           /**< Actual weapon43 callback image/strike child index0..47. */
    SOTN_DRAW_IMMUNITY_TEXT,  /**< Actual entity37 child*18+row; row1 is the actual medicine suffix.
                               */
    SOTN_DRAW_HEALING_NUMBER, /**< Actual child19 index times20 plus digit/tile index. */
    SOTN_DRAW_AXE_SHADOW,     /**< Actual rowless entity49 child index0..47. */
    SOTN_DRAW_FIRE_HIT,       /**< Actual rowless entity16 child index0..47. */
    SOTN_DRAW_DARK_HIT,       /**< Actual rowless entity54 child index0..47. */
    SOTN_DRAW_THUNDER_HIT,    /**< Actual entity30 child*6+GT4 row0..5. */
    SOTN_DRAW_ICE_HIT,        /**< Actual entity33 child*24+G4 shard0..23. */
    SOTN_DRAW_STONE_DISSOLVE, /**< Actual entity32 child*36+textured slice0..35. */
    SOTN_DRAW_HOLY_HIT,       /**< Actual entity47 child*16+GT4 index0..15. */
    SOTN_DRAW_PLAYER_CIRCLE,  /**< Actual entity38 child*32+GT4 index0..31. */
    SOTN_DRAW_TELEPORT,       /**< Actual entity65 child*36+TILE/G4/GT4 index0..35. */
    SOTN_DRAW_FAIRY_SPRITE,   /**< Actual familiar owner index0..3, original slots4..7. */
    SOTN_DRAW_FAIRY_TILE,     /**< Actual DF owner index0..3 and leased tile index. */
    SOTN_DRAW_FAIRY_LIGHT,    /**< Actual DD owner index0..3 and leased ten-row G4 index. */
    SOTN_DRAW_FOOD_TEXT,      /**< Actual global lower-left text row index0..67. */
    SOTN_DRAW_FOOD,           /**< Actual weapon21 child index1..15. */
    SOTN_DRAW_NEUTRON,        /**< Actual weapon19 child index1..15. */
    SOTN_DRAW_SIRE,           /**< Actual weapon18 parent/burst child index1..15. */
    SOTN_DRAW_PENTAGRAM,      /**< Actual weapon17 child index1..15 and row0..24. */
    SOTN_DRAW_MAGIC_MISSILE   /**< Actual rowless weapon22 bow/arrow child index1..15. */
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
