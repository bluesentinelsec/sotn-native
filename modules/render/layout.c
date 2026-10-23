#include "layout.h"

bool SotnRender_FitImage(int window_width, int window_height, int width, int height, SDL_Rect *out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = (SDL_Rect){0};
    if (window_width <= 0 || window_height <= 0 || width <= 0 || height <= 0)
        return SDL_SetError("Image and presentation dimensions must be positive");
    const int scale = SDL_min(window_width / width, window_height / height);
    if (scale > 0)
    {
        out->w = width * scale;
        out->h = height * scale;
    }
    else
    {
        out->w = window_width;
        out->h = (int)((Sint64)window_width * height / width);
        if (out->h > window_height)
        {
            out->h = window_height;
            out->w = (int)((Sint64)window_height * width / height);
        }
        /* At tiny sizes the nearest representable image still occupies one pixel. */
        out->w = SDL_max(1, out->w);
        out->h = SDL_max(1, out->h);
    }
    out->x = (window_width - out->w) / 2;
    out->y = (window_height - out->h) / 2;
    return true;
}
