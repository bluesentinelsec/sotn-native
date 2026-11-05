#ifndef SOTN_MAGIC_MISSILE_H
#define SOTN_MAGIC_MISSILE_H
#include <SDL3/SDL.h>
#include <sotn/game_data.h>
/** @brief Original weapon22 callback words, with the raw animation-set flag retained. */
typedef struct SotnMagicMissile
{
    Sint32 x, y, velocity_x;
    Uint32 flags;
    Uint16 palette, bank, texture, priority, draw_flags, item;
    Sint16 lifetime;
    Uint8 phase;
    bool facing_left, removed;
} SotnMagicMissile;
/** @brief Snapshot of the actual raw retained parent slot, without a generation/family gate. */
typedef struct SotnMagicMissileParent
{
    Uint16 palette, bank, texture, item;
    bool exists, facing_left;
} SotnMagicMissileParent;
/** @brief Actual callback role, player attributes, animation cursor and hit byte. */
typedef struct SotnMagicMissileInput
{
    SotnMagicMissileParent parent;
    Uint16 hand, priority, pose;
    Sint16 remaining;
    Uint8 hit_flags;
    bool arrow, facing_left;
} SotnMagicMissileInput;
/** @brief One-update owner requests; none is a completed native operation. */
typedef struct SotnMagicMissileEvents
{
    Uint16 sound;
    Uint8 animation, use_ticks;
    bool factory, properties, clear_shield;
} SotnMagicMissileEvents;
/** @brief Original private UpdateAnim cursor, independent of rendering calls. */
typedef struct SotnMagicMissileAnimation
{
    Uint16 pose, frame;
    Sint16 remaining;
} SotnMagicMissileAnimation;
/** @brief Run one original bow/arrow callback without allocating or enabling hand controls.
 * @param state Actual callback words, unchanged on error. @param input Copied native context.
 * @param out Requests, unchanged on error. use_ticks belongs to original word10.
 * @return Success or SDL error. A factory request needs an actual reservation before sound625;
 * clear_shield requests the conditional original DestroyEntityWeapon(true) owner. Properties
 * require actual retained item/RNG/combat ownership. Animation runs after this callback.
 */
bool SotnMagicMissile_Tick(SotnMagicMissile *state, SotnMagicMissileInput input,
                           SotnMagicMissileEvents *out);
/** @brief Advance the actual private bow/flight/impact timeline once after a surviving callback.
 * @param animation Owned cursor. @param overlay Borrowed verified weapon22 program bytes.
 * @param timeline Original selector1 bow/2 flight/3 impact.
 * @return Success or SDL error, preserving the cursor on error. On a callback's timeline change,
 * reset pose/remaining to zero and retain the old displayed frame until this update succeeds.
 * This creates no resource, factory, sprite, combat or ordinary-use readiness.
 */
bool SotnMagicMissile_Animate(SotnMagicMissileAnimation *animation, SotnDataView overlay,
                              Uint8 timeline);
#endif
