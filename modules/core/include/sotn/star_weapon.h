#ifndef SOTN_STAR_WEAPON_H
#define SOTN_STAR_WEAPON_H
#include <sotn/thrown_weapon.h>
/** @brief Actual weapon15 function-table entries; these differ from weapon16 indices. */
typedef enum SotnStarRole
{
    SOTN_STAR_PROJECTILE,
    SOTN_STAR_TRAIL,
    SOTN_STAR_FLAME,
    SOTN_STAR_TNT,
    SOTN_STAR_EXPLOSION,
    SOTN_STAR_SCATTER
} SotnStarRole;
/** @brief Actual TILE words, retained only with the native TNT32-row lease. */
typedef struct SotnStarSpark
{
    Sint16 x, y, direction, age, delay;
    Uint16 draw_mode;
} SotnStarSpark;
/** @brief Selected original weapon15 words; no factory, animation or combat ownership. */
typedef struct SotnStarWeapon
{
    SotnThrownWeapon motion;
    SotnStarSpark sparks[32];
    Sint32 acceleration_x, acceleration_y;
    Uint16 scale_x, scale_y, item;
    Uint8 hit_flags;
    bool attack_cleared;
} SotnStarWeapon;
/** @brief The actual raw parent slot, without generation equality. */
typedef struct SotnStarParent
{
    SotnThrownParent image;
    Sint32 velocity_x, velocity_y;
    Uint16 item, priority;
    Uint8 group;
    bool exists;
} SotnStarParent;
typedef struct SotnStarInput
{
    SotnStarRole role;
    SotnDataView executable, overlay;
    SotnCollisionMap collision;
    SotnStarParent parent;
    Sint32 player_x, player_y;
    Sint16 offset_x, offset_y, remaining;
    Uint32 game_timer;
    Uint16 player_step, priority, hand, buttons;
    bool facing_left, primitive_allocated;
} SotnStarInput;
typedef struct SotnStarEvents
{
    SotnThrownEvents effects;
    Uint16 animation_offset;
    bool fixed_tnt_properties, inherit_group;
} SotnStarEvents;
/** @brief Execute one weapon15 callback atomically using supplied actual owner results.
 * Primitive allocation, raw parents, private timeline execution, property/group assignment,
 * factories, sound, shake, hit feedback and rendering remain native-owner obligations.
 * TNT exhaustion omits only its optional32-row visual before RNG. This API enables no items. */
bool SotnStarWeapon_Tick(SotnStarWeapon *, SotnStarInput, Uint32 *random, Uint32 *fire_counter,
                         SotnStarEvents *);
/** @brief Execute one private UpdateAnim pair after the actual callback and culling. */
bool SotnStarWeapon_Animate(SotnThrownAnimation *, SotnDataView overlay, Uint16 offset);
#endif
