#include "playback_internal.h"

bool SotnPlayback_Create(SotnPlayback **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    SotnPlayback *playback = SDL_calloc(1, sizeof(*playback));
    if (playback == NULL)
        return false;
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
    {
        SDL_free(playback);
        return false;
    }
    const SDL_AudioSpec spec = {.format = SDL_AUDIO_S16, .channels = 2, .freq = 44100};
    playback->stream =
        SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    if (playback->stream == NULL)
    {
        SotnPlayback_Destroy(playback);
        return false;
    }
    *out = playback;
    return true;
}

bool SotnPlayback_Reset(SotnPlayback *playback)
{
    if (playback == NULL)
        return SDL_InvalidParamError("playback");
    if (!SDL_PauseAudioStreamDevice(playback->stream) || !SDL_ClearAudioStream(playback->stream))
        return false;
    playback->started = false;
    return true;
}

bool SotnPlayback_Queue(SotnPlayback *playback, const Sint16 *pcm, size_t count)
{
    if (playback == NULL || pcm == NULL || count != SOTN_SOUND_FRAMES * 2)
        return SDL_SetError("Playback requires one tick of stereo PCM");
    const int queued = SDL_GetAudioStreamQueued(playback->stream);
    if (queued < 0)
        return false;
    if (queued > 44100 && !SotnPlayback_Reset(playback))
        return false;
    if (!SDL_PutAudioStreamData(playback->stream, pcm, (int)(count * sizeof(*pcm))))
        return false;
    if (!playback->started)
    {
        if (!SDL_ResumeAudioStreamDevice(playback->stream))
            return false;
        playback->started = true;
    }
    return true;
}

void SotnPlayback_Destroy(SotnPlayback *playback)
{
    if (playback != NULL)
    {
        SDL_DestroyAudioStream(playback->stream);
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        SDL_free(playback);
    }
}
