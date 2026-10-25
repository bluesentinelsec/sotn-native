#ifndef SOTN_FRAME_CLIP_H
#define SOTN_FRAME_CLIP_H
#include <sotn/tilemap.h>
/** @brief Original draw-area bounds in screen pixels; right/bottom are exclusive. */
typedef struct SotnFrameClip
{
    Sint32 left, top, right, bottom;
} SotnFrameClip;
/** @brief Copy pixels outside a draw area between two independent full RGB555 frame buffers.
 * @param clip Draw area, intersected with the frame; inverted or empty areas copy every pixel.
 * @param source Borrowed source, at least count pixels. @param destination Writable nonoverlapping
 * destination, at least count pixels. @param count At least SOTN_FRAME_PIXELS in each buffer.
 * @return Success or SDL error before writing for invalid buffers/count.
 * @note Save outside pixels before an ordered draw, then restore them afterward. No allocation,
 * persistent renderer state or source modification occurs; a full-frame area copies nothing. */
bool SotnFrameClip_CopyOutside(SotnFrameClip clip, const Uint16 *source, Uint16 *destination,
                               size_t count);
#endif
