#ifndef SOTN_THROWN_WEAPON_H
#define SOTN_THROWN_WEAPON_H
#include <sotn/collision.h>
#include <sotn/game_math.h>
#include <sotn/player_factory.h>
/** @brief Actual weapon16 function-table indices, independent of an equipped hand's item. */
typedef enum SotnThrownRole
{
    SOTN_THROWN_KNIFE,
    SOTN_THROWN_TRAIL,
    SOTN_THROWN_BOOMERANG,
    SOTN_THROWN_JAVELIN,
    SOTN_THROWN_FIRE,
    SOTN_THROWN_IRON
} SotnThrownRole;
/** @brief Original weapon16 callback words; the knife's hidden TILE needs an actual row lease. */
typedef struct SotnThrownWeapon
{
    Sint32 x, y, velocity_x, velocity_y;
    Sint32 primitive_x, primitive_y, primitive_velocity_x, speed;
    Sint16 primitive_anchor_x, primitive_anchor_y;
    Sint16 lifetime, radius, angle, turn, acceleration, timer, rotation;
    Uint32 flags;
    Uint16 bank, texture, palette, frame, priority, parameter;
    Uint8 phase, opacity, blend, draw_flags, width, height;
    bool facing_left, removed;
} SotnThrownWeapon;
/** @brief Actual raw parent-slot fields; an empty slot supplies its actual zeroed fields. */
typedef struct SotnThrownParent
{
    Uint16 bank, texture, palette, frame;
    Sint16 rotation;
    bool facing_left;
} SotnThrownParent;
typedef struct SotnThrownInput
{
    SotnThrownRole role;
    SotnDataView executable;
    SotnCollisionMap collision;
    SotnThrownParent parent;
    Sint32 player_x, player_y;
    Sint16 offset_x, offset_y, remaining;
    Uint16 player_step, priority, hand;
    bool facing_left, primitive_allocated;
} SotnThrownInput;
typedef struct SotnThrownFactory
{
    SotnPlayerFactorySource source;
    Uint32 packed;
    bool weapon_dispatch; /**< Fire smoke changes actual factory entityId to EF/FF only. */
} SotnThrownFactory;
/** @brief Ordered requests; native factories, leases, sounds, combat and shield owners commit them.
 */
typedef struct SotnThrownEvents
{
    SotnThrownFactory factories[2];
    Uint16 sounds[3];
    Uint8 shakes[3], factory_count, sound_count, shake_count, use_ticks;
    bool properties, clear_shield, fire_animation;
} SotnThrownEvents;
/** @brief Run one original weapon16 callback, without enabling equipment or allocating anything.
 * @param state Actual callback/hidden-row words. @param input Current native owner snapshots.
 * @param random Actual shared libc cursor. @param fire_counter Actual per-hand weapon16 BSS,
 * required only for the fire callback. @param out Copied requests. All unchanged on error.
 * Primitive exhaustion removes only the knife before properties/sound/timer work. Native owners
 * must preserve original factory order, captured sources, raw parents, post-callback culling,
 * animation, shared groups and commit/rollback. No pool, grants or save fields are introduced. */
bool SotnThrownWeapon_Tick(SotnThrownWeapon *state, SotnThrownInput input, Uint32 *random,
                           Uint32 *fire_counter, SotnThrownEvents *out);
typedef struct SotnThrownAnimation
{
    Uint16 pose, frame;
    Sint16 remaining;
} SotnThrownAnimation;
/** @brief Advance the original private weapon16 fire timeline once after callback/culling. */
bool SotnThrownWeapon_AnimateFire(SotnThrownAnimation *, SotnDataView overlay);
#endif
