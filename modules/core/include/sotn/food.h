#ifndef SOTN_FOOD_H
#define SOTN_FOOD_H
#include <sotn/collision.h>

/** @brief Original weapon21 food cursor, retained by its actual shared entity slot. */
typedef struct SotnFood
{
    Sint32 x, y, velocity_x, velocity_y, bounce;
    Uint32 flags;
    Sint16 timer;
    Uint16 parameter, item, attack, frame, priority, draw_flags;
    Uint8 phase;
    bool inventory, facing_left, removed;
} SotnFood;
/** @brief Actual player geometry at this food callback, in the same room coordinates. */
typedef struct SotnFoodPlayer
{
    Sint32 x, y;
    Sint16 offset_x, offset_y;
    Uint16 width, height, frame, animation, priority;
    bool facing_left, healing_pending;
} SotnFoodPlayer;
/** @brief Real shared-factory reservation service; false reports an actual failure.
 * A successful call reports whether the original optional factory slot was available.
 * Caller owns rollback of its reservation if a later native operation fails. */
typedef bool (*SotnFoodFactory)(void *context, Sint32 x, Sint32 y, Uint32 packed, bool *allocated);
/** @brief Current collision/player/raw item properties, never simulated acquisition. */
typedef struct SotnFoodInput
{
    SotnCollisionMap collision;
    SotnFoodPlayer player;
    Uint16 attack;
    SotnFoodFactory factory;
    void *context;
} SotnFoodInput;
/** @brief One callback's original requests; native owners must commit them in entity order.
 * Heal uses first-pending kind1, without accessory scaling. Poison writes raw hit81/HP0
 * and clears hitbox state. Inventory collection also requires real sound/text ownership. */
typedef struct SotnFoodEvents
{
    Uint16 heal_amount, item;
    Uint8 simulated_ticks;
    bool heal, poison, crouch, peanut_pose, collect, clear_weapon;
} SotnFoodEvents;
/** @brief Reconstruct one weapon21 primary or Meal Ticket inventory-child callback.
 * @param food Actual cursor, zeroed birth phase with captured origin/parameter/item.
 * @param input Actual player/map/owner services. @param random Actual libc random stream.
 * @param out Requests, unchanged with cursor/RNG on invalid data or collision failure.
 * No inventory, healing, player control, sound, rendering or factories are enabled by this
 * component alone. Menus/frozen dispatch/draws belong to its actual entity owner. */
bool SotnFood_Tick(SotnFood *food, SotnFoodInput input, Uint32 *random, SotnFoodEvents *out);
#endif
