#include "playback_internal.h"

#define CHECK(condition)                                                                           \
    do                                                                                             \
    {                                                                                              \
        if (!(condition))                                                                          \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #condition, SDL_GetError());                      \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

static bool Lifecycle(void)
{
    Sint16 pcm[SOTN_SOUND_FRAMES * 2] = {0};
    CHECK(!SotnPlayback_Create(NULL));
    CHECK(!SotnPlayback_Reset(NULL));
    CHECK(!SotnPlayback_Queue(NULL, pcm, SDL_arraysize(pcm)));
    SotnPlayback_Destroy(NULL);
    CHECK(SDL_InitSubSystem(SDL_INIT_AUDIO));
    for (Uint32 i = 0; i < 8; ++i)
    {
        SotnPlayback *playback = NULL;
        CHECK(SotnPlayback_Create(&playback));
        CHECK(SDL_AudioStreamDevicePaused(playback->stream));
        CHECK(SDL_GetAudioStreamQueued(playback->stream) == 0);
        CHECK(!SotnPlayback_Queue(playback, pcm, SDL_arraysize(pcm) - 1));
        CHECK(!SotnPlayback_Queue(playback, NULL, SDL_arraysize(pcm)));
        CHECK(SDL_AudioStreamDevicePaused(playback->stream));
        CHECK(SotnPlayback_Queue(playback, pcm, SDL_arraysize(pcm)));
        CHECK(!SDL_AudioStreamDevicePaused(playback->stream));
        CHECK(SotnPlayback_Reset(playback));
        CHECK(SDL_AudioStreamDevicePaused(playback->stream));
        CHECK(SDL_GetAudioStreamQueued(playback->stream) == 0);
        CHECK(SotnPlayback_Reset(playback));
        CHECK(SotnPlayback_Queue(playback, pcm, SDL_arraysize(pcm)));
        CHECK(!SDL_AudioStreamDevicePaused(playback->stream));
        CHECK(SDL_PauseAudioStreamDevice(playback->stream));
        for (Uint32 tick = 0; tick < 20; ++tick)
            CHECK(SDL_PutAudioStreamData(playback->stream, pcm, sizeof(pcm)));
        CHECK(SDL_GetAudioStreamQueued(playback->stream) > 44100);
        CHECK(SotnPlayback_Queue(playback, pcm, SDL_arraysize(pcm)));
        CHECK(!SDL_AudioStreamDevicePaused(playback->stream));
        CHECK(SDL_GetAudioStreamQueued(playback->stream) <= (int)sizeof(pcm));
        SotnPlayback_Destroy(playback);
        CHECK(SDL_WasInit(SDL_INIT_AUDIO) == SDL_INIT_AUDIO);
    }
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    CHECK(SDL_WasInit(SDL_INIT_AUDIO) == 0);
    return true;
}

static bool FailedInitialization(void)
{
    SotnPlayback *playback = NULL;
    CHECK(SDL_SetHintWithPriority(SDL_HINT_AUDIO_DRIVER, "sotn-missing-driver", SDL_HINT_OVERRIDE));
    CHECK(!SotnPlayback_Create(&playback));
    CHECK(playback == NULL && SDL_WasInit(SDL_INIT_AUDIO) == 0);
    CHECK(SDL_strstr(SDL_GetError(), "sotn-missing-driver") != NULL);
    CHECK(SDL_SetHintWithPriority(SDL_HINT_AUDIO_DRIVER, "dummy", SDL_HINT_OVERRIDE));
    CHECK(SotnPlayback_Create(&playback));
    SotnPlayback_Destroy(playback);
    CHECK(SDL_WasInit(SDL_INIT_AUDIO) == 0);
    return true;
}

int main(void)
{
    if (!Lifecycle() || !FailedInitialization())
        return 1;
    SDL_Log("SOTN_PLAYBACK_TEST: PASS");
    return 0;
}
