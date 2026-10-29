#ifndef SOTN_SPIRIT_CAPTURE_H
#define SOTN_SPIRIT_CAPTURE_H
#include <sotn/actor_graphics.h>
/** @brief Copy the preceding ordered frame into the original shared spirit/Wolf target.
 * @param graphics Actual owned texture memory, with no active render target.
 * @param frame Borrowed complete RGB555 frame, unchanged by this operation.
 * @param count Available pixels, at least SOTN_FRAME_PIXELS.
 * @param x Original source rectangle left. @param y Original source rectangle top.
 * @return Success or SDL error without writes for invalid bounds/ownership.
 * Copies exactly63x63 words to512,448, preserving adjacent texels. Capture occurs once
 * at the simulation's real MoveImage intent, before this frame's drawing. Drawing must
 * not repeat capture or substitute a placeholder. Off-frame VRAM sources fail explicitly. */
bool SotnSpiritCapture_Frame(SotnActorGraphics *graphics, const Uint16 *frame, size_t count,
                             Sint32 x, Sint32 y);
#endif
