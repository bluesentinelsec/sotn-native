#ifndef SOTN_PLAYER_CIRCLE_H
#define SOTN_PLAYER_CIRCLE_H
#include <sotn/player_blink.h>

enum
{
    SOTN_PLAYER_CIRCLE_ROWS = 32
};
/** @brief Original entity38: two sixteen-quad rings, with signed extension words. */
typedef struct SotnPlayerCircle
{
    SotnPlayerEffectQuad quads[SOTN_PLAYER_CIRCLE_ROWS];
    Sint32 x, y;
    Sint16 timer, angle, outer_x, outer_y, inner_x, inner_y, alpha, stride;
    Uint16 parameter, phase;
    bool removed;
} SotnPlayerCircle;
/** @brief Actual player high words, private profiles/SDK and real32-row allocation outcome. */
typedef struct SotnPlayerCircleInput
{
    SotnDataView dra, executable;
    Sint32 player_x, player_y;
    Uint16 priority;
    bool allocated;
} SotnPlayerCircleInput;
/** @brief Original afterimage reset, including failed optional allocation. */
typedef struct SotnPlayerCircleEvents
{
    bool reset_afterimages;
} SotnPlayerCircleEvents;
/** @brief Advance original circle38 once; private profiles remain external.
 * Optional allocation failure destroys before geometry and consumes no RNG. The caller owns
 * actual factories, stage rows, afterimages and rendering. This enables no consumable controls.
 * @param circle Owned cursor, zeroed with actual origin/parameter before birth.
 * @param input Borrowed immutable private views and actual player/allocation snapshot.
 * @param out Receives this callback's afterimage request; retains no borrowed pointers.
 * @return Success, or SDL error with cursor and output unchanged, including late SDK failures. */
bool SotnPlayerCircle_Tick(SotnPlayerCircle *circle, SotnPlayerCircleInput input,
                           SotnPlayerCircleEvents *out);
/** @brief Rebase the actual player-locked entity; ring packets retain their own draw flags.
 * @param circle Owned live cursor. @param x Signed integer camera/room X delta.
 * @param y Signed integer camera/room Y delta. @param room Actual room entry, not a camera tick.
 * @return Success, or SDL error without mutation for an invalid cursor. */
bool SotnPlayerCircle_Rebase(SotnPlayerCircle *circle, Sint16 x, Sint16 y, bool room);
/** @brief Apply actual post-callback stage shove to the player-locked entity, without ticking.
 * @param circle Owned live cursor. @param x Signed16.16 X delta. @param y Signed16.16 Y delta.
 * @return Success with defined32-bit wrap, or SDL error without mutation for an invalid cursor. */
bool SotnPlayerCircle_Displace(SotnPlayerCircle *circle, Sint32 x, Sint32 y);
#endif
