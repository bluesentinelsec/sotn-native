#ifndef SOTN_PLAYER_AFTERIMAGE_H
#define SOTN_PLAYER_AFTERIMAGE_H
#include <sotn/game_data.h>
/** @brief Metadata stored in one original hidden tile row; this is not a rendered tile. */
typedef struct SotnAfterimageSample
{
    Sint16 x, y;
    Uint16 sprite, blend, facing, palette;
    Uint8 red, green, blue;
} SotnAfterimageSample;
/** @brief Copied original display entity1..3 and its corresponding player-color descriptor. */
typedef struct SotnAfterimageDisplay
{
    Sint32 x, y;
    Uint16 sprite, palette, priority, bank, texture_slot;
    Uint8 blend, red, green, blue;
    bool facing_left, color_enabled;
} SotnAfterimageDisplay;
/** @brief Selected original six-sample/three-display cursor; initialize with Init.
 * No factory, primitive, image or generation ownership is conferred by this state. */
typedef struct SotnPlayerAfterimage
{
    SotnAfterimageSample samples[6];
    SotnAfterimageDisplay displays[3];
    Uint16 pose, write_index;
    Uint8 index, timer;
    bool disabled, reset;
} SotnPlayerAfterimage;
/** @brief Actual player snapshot at the original pre-collision capture point.
 * Held means the preceding complete button word, not the intersection with current input. */
typedef struct SotnAfterimageInput
{
    Sint32 x, y, velocity_y;
    Uint16 pressed, held, tapped, animation, sprite, palette;
    bool facing_left;
} SotnAfterimageInput;
/** @brief Initialize copied source cursors/displays without allocating their real owners.
 * @param afterimage Writable output. @return Success or SDL error for NULL. */
bool SotnPlayerAfterimage_Init(SotnPlayerAfterimage *afterimage);
/** @brief Apply original disable/reset and timer15 request.
 * @param afterimage Initialized cursor. @param clear Clear retained/display frames if true.
 * @param duration Original signed argument; nonzero values below four request four ticks.
 * @param timer15 Owned player timer word; unchanged for duration zero.
 * @return Success or SDL error with outputs unchanged for NULL inputs.
 * Preserves source color/positions, capture cadence and existing table timer. */
bool SotnPlayerAfterimage_Reset(SotnPlayerAfterimage *afterimage, bool clear, Sint32 duration,
                                Uint16 *timer15);
/** @brief Clear original reset/disable flags; advances no cursor or lifetime.
 * @param afterimage Initialized cursor. @return Success or SDL error for NULL. */
bool SotnPlayerAfterimage_Enable(SotnPlayerAfterimage *afterimage);
/** @brief Force the original activity index to zero without changing its timer.
 * @param afterimage Initialized cursor. @return Success or SDL error for NULL. */
bool SotnPlayerAfterimage_Force(SotnPlayerAfterimage *afterimage);
/** @brief Run original pre-collision input/timer/capture logic once.
 * @param afterimage Initialized writable cursor. @param dra Borrowed private timing table.
 * @param input Actual pre-collision snapshot. @return Success or SDL error without mutation.
 * Disabled/excluded/expired paths retain their original early returns; data is required only
 * when the original timing table is read. This requests no texture upload or child allocation. */
bool SotnPlayerAfterimage_Capture(SotnPlayerAfterimage *afterimage, SotnDataView dra,
                                  SotnAfterimageInput input);
/** @brief Run original post-collision fades and timer-parity display selection once.
 * @param afterimage Initialized cursor. @param dra Borrowed private fade tables.
 * @param timer Original system g_Timer, distinct from gameplay's pause-frozen game timer.
 * @param priority Current player priority, 2..511. @return Success or SDL error without mutation.
 * Retains captured facing/palette, display fractions and the original three display slots.
 * This emits copied display state, not rendered pixels or actual row ownership. */
bool SotnPlayerAfterimage_Present(SotnPlayerAfterimage *afterimage, SotnDataView dra, Uint32 timer,
                                  Uint16 priority);
/** @brief Rebase retained samples/displays by signed-word pixels without another update.
 * @param afterimage Initialized cursor. @param x Horizontal pixels. @param y Vertical pixels.
 * @return Success or SDL error without mutation for out-of-word shifts or NULL input. */
bool SotnPlayerAfterimage_Rebase(SotnPlayerAfterimage *afterimage, Sint32 x, Sint32 y);
#endif
