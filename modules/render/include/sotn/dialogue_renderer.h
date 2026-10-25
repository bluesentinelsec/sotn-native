#ifndef SOTN_DIALOGUE_RENDERER_H
#define SOTN_DIALOGUE_RENDERER_H
#include <sotn/actor_graphics.h>
#include <sotn/dialogue.h>
/** @brief Present the selected US Death dialogue with private common glyphs/portraits.
 * @param graphics Borrowed texture service; portraits already uploaded.
 * @param state Bounded copied script presentation. @param frame Writable RGB555 output.
 * @param count At least SOTN_FRAME_PIXELS. @return Success or SDL error.
 * Does not advance scripts or speech. Complete PS1 window/GPU timing remains separate parity work.
 */
bool SotnDialogueRenderer_Draw(const SotnActorGraphics *graphics, SotnDialogueState state,
                               Uint16 *frame, size_t count);
/** @brief Apply the event's background-only transition, before foreground/actors/HUD.
 * @param fade Original 0..240 shade. @param frame RGB555 output. @param count Full frame.
 * @return Success or SDL error. */
bool SotnDialogueRenderer_Background(Uint8 fade, Uint16 *frame, size_t count);
/** @brief Present one stolen equipment icon, borrowing original private item metadata/artwork.
 * @param dra Borrowed DRA. @param stage Borrowed stage image. @param parameter 0..5.
 * @param x Screen center. @param y Screen center. @param width 0..16. @param height 0..16.
 * @param blink Original red blink phase. @param frame RGB555 output. @param count Full frame.
 * @return Success or SDL error. Zero dimensions draw nothing. */
bool SotnDialogueRenderer_Item(SotnDataView dra, SotnDataView stage, Uint16 parameter, Sint32 x,
                               Sint32 y, Uint16 width, Uint16 height, bool blink, Uint16 *frame,
                               size_t count);
#endif
