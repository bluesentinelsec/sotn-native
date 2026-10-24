#ifndef SOTN_PLAYBACK_INTERNAL_H
#define SOTN_PLAYBACK_INTERNAL_H

#include <SDL3/SDL.h>
#include <sotn/playback.h>

struct SotnPlayback
{
    SDL_AudioStream *stream;
    bool started;
};

#endif
