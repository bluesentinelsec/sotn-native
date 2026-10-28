#ifndef SOTN_WOLF_ROOT_EFFECT_H
#define SOTN_WOLF_ROOT_EFFECT_H
#include <sotn/player_blink.h>
#include <sotn/wolf_root.h>
/** @brief Original retained image phase, timer and captured facing. */
typedef struct SotnWolfRootImage
{
    Sint32 state, timer;
    bool facing_left;
} SotnWolfRootImage;
/** @brief Copied six actual root primitives and their original shared image cursors.
 * The caller owns the real six-row reservation and shared captured texture; this owns neither. */
typedef struct SotnWolfRootEffect
{
    SotnPlayerEffectQuad quads[6];
    SotnWolfRootImage images[6];
} SotnWolfRootEffect;
/** @brief Advance original five retained images and the sixth speed-dependent overlay.
 * @param effect Actual copied primitive/image snapshots, retained on error.
 * @param request Current original root request. Negative image indices select no image.
 * @param x Original screen whole-position word. @param y Original screen whole-position word.
 * @param velocity_x Original signed 16.16 velocity, bounded to sixteen pixels/tick.
 * @param priority Current original player priority, at least six. @param facing_left Player facing.
 * @return Success or SDL error, atomically. Preserve first-initialization ordering, captured
 * image facing/positions, byte color wrap and repeated sixth-overlay replacement. Native owner
 * must reserve real primitives, supply the current original Wolf capture and rebase retained quads
 * without advancing them. This enables no live Wolf or successful allocation signal. */
bool SotnWolfRootEffect_Tick(SotnWolfRootEffect *effect, SotnWolfRootEvents request, Sint16 x,
                             Sint16 y, Sint32 velocity_x, Uint16 priority, bool facing_left);
#endif
