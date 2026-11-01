#ifndef SOTN_FAMILIAR_ANIMATION_H
#define SOTN_FAMILIAR_ANIMATION_H
#include <sotn/game_data.h>

/** @brief Original familiar animation state; addresses refer to private PS1 data, not host code. */
typedef struct SotnFamiliarAnimation
{
    Uint32 address;
    Uint16 pose, sprite;
    Sint16 remaining;
    Sint16 hitbox_x, hitbox_y;
    Uint8 hitbox_width, hitbox_height;
} SotnFamiliarAnimation;
/** @brief Borrowed private familiar timelines and optional original four-byte frame properties. */
typedef struct SotnFamiliarAnimationSource
{
    SotnDataView overlay, properties;
    Uint32 table; /**< Original pointer-table address within the overlay at80170000. */
    Uint16 count;
} SotnFamiliarAnimationSource;
/** @brief Select a real timeline with the original conditional or unconditional restart.
 * @param animation Owned cursor; no pointers into source bytes are retained.
 * @param source Borrowed immutable overlay/table description. @param index Actual table index.
 * @param restart Restart even when the target pointer is unchanged; false matches Fairy's helper.
 * @return Success, or SDL error without mutation for invalid metadata/index/address.
 * This component supplies no familiar activation, native slots, artwork or controls. */
bool SotnFamiliarAnimation_Set(SotnFamiliarAnimation *animation, SotnFamiliarAnimationSource source,
                               Uint16 index, bool restart);
/** @brief Advance the original ServantUpdateAnim callback, including its single control jump.
 * @param animation Owned actual cursor. @param source Borrowed private timelines/properties.
 * @param status Optional original result:0 ordinary,-1 end hold,-2 timeline transfer.
 * @return Success, or SDL error with cursor and status unchanged after any bounds failure.
 * Caller owns update clocks, entity lifecycle and rendering; draws must never call this. */
bool SotnFamiliarAnimation_Tick(SotnFamiliarAnimation *animation,
                                SotnFamiliarAnimationSource source, Sint32 *status);
#endif
