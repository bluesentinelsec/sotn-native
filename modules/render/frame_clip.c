#include <SDL3/SDL_error.h>
#include <sotn/frame_clip.h>
bool SotnFrameClip_CopyOutside(SotnFrameClip clip, const Uint16 *source, Uint16 *destination,
                               size_t count)
{
    if (!source || !destination || source == destination || count < SOTN_FRAME_PIXELS)
        return SDL_SetError("Draw-area preservation requires two independent full frame buffers");
    const Sint32 left = SDL_clamp(clip.left, 0, SOTN_FRAME_WIDTH),
                 right = SDL_clamp(clip.right, 0, SOTN_FRAME_WIDTH),
                 top = SDL_clamp(clip.top, 0, SOTN_FRAME_HEIGHT),
                 bottom = SDL_clamp(clip.bottom, 0, SOTN_FRAME_HEIGHT);
    if (left >= right || top >= bottom)
    {
        SDL_memcpy(destination, source, SOTN_FRAME_PIXELS * sizeof(*source));
        return true;
    }
    for (Sint32 y = 0; y < SOTN_FRAME_HEIGHT; ++y)
    {
        const size_t row = (size_t)y * SOTN_FRAME_WIDTH;
        if (y < top || y >= bottom)
            SDL_memcpy(destination + row, source + row, SOTN_FRAME_WIDTH * sizeof(*source));
        else
        {
            if (left)
                SDL_memcpy(destination + row, source + row, (size_t)left * sizeof(*source));
            if (right < SOTN_FRAME_WIDTH)
                SDL_memcpy(destination + row + (size_t)right, source + row + (size_t)right,
                           (SOTN_FRAME_WIDTH - (size_t)right) * sizeof(*source));
        }
    }
    return true;
}
