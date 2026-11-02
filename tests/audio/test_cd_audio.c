#include "data_internal.h"
#include "sound_internal.h"
#include <SDL3/SDL.h>
#include <sotn/cd_audio.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (8 * i));
}
static bool SameState(SotnCdAudioState a, SotnCdAudioState b)
{
    return a.started == b.started && a.duration == b.duration && a.playing == b.playing &&
           a.queued == b.queued && a.command == b.command && a.commands == b.commands &&
           a.phase == b.phase && a.saved == b.saved && a.volume == b.volume &&
           a.target_volume == b.target_volume && a.acknowledged == b.acknowledged &&
           a.queue_empty == b.queue_empty && a.playing_xa == b.playing_xa && a.busy == b.busy &&
           a.serial == b.serial && a.mode == b.mode && a.managed == b.managed &&
           a.fade_multiplier == b.fade_multiplier && a.fade_counter == b.fade_counter &&
           a.master_volume == b.master_volume && a.master_target == b.master_target &&
           a.release_counter == b.release_counter && a.fade_acknowledged == b.fade_acknowledged &&
           a.xa_active == b.xa_active && a.decoder_present == b.decoder_present &&
           a.decoder.sector == b.decoder.sector && a.decoder.remaining == b.decoder.remaining &&
           a.decoder.duration == b.decoder.duration && a.decoder.start == b.decoder.start &&
           a.decoder.id == b.decoder.id && a.decoder.volume == b.decoder.volume &&
           a.decoder.mode == b.decoder.mode && a.decoder.looping == b.decoder.looping &&
           a.decoder.once == b.decoder.once;
}
static bool Tick(SotnCdAudio *a, Uint32 *clock, SotnCdAudioState *state)
{
    Sint16 pcm[SOTN_SOUND_FRAMES * 2] = {0};
    CHECK(SotnCdAudio_Tick(a, (*clock)++, pcm, SDL_arraysize(pcm)));
    return SotnCdAudio_Get(a, state);
}
static bool StartVoice(SotnCdAudio *a, Uint32 *clock, Uint16 id, SotnCdAudioState *s)
{
    CHECK(SotnCdAudio_Queue(a, id));
    const Uint8 phases[] = {2, 4, 6, 7, 8, 0};
    for (Uint16 i = 0; i < SDL_arraysize(phases); ++i)
    {
        CHECK(Tick(a, clock, s) && s->phase == phases[i]);
        if (i < 5)
            CHECK(s->busy && !s->playing_xa && !s->serial);
        if (i == 3)
            CHECK(s->decoder.sector == s->decoder.start + 1 && s->decoder.remaining == 16);
    }
    CHECK(s->queue_empty && s->playing_xa && s->playing == id - 0x300U && !s->busy && s->serial &&
          s->decoder.remaining == 15);
    return true;
}
static bool PauseMusic(SotnCdAudio *a, Uint32 *clock, SotnCdAudioState *s, SotnMusicState *held)
{
    const Uint8 saved = s->saved;
    CHECK(SotnCdAudio_Queue(a, 0x10) && SotnCdAudio_Get(a, s) && !s->acknowledged);
    CHECK(Tick(a, clock, s) && s->phase == 1 && s->busy && !s->acknowledged);
    for (Uint16 i = 0; i < 11; ++i)
        CHECK(Tick(a, clock, s));
    CHECK(s->phase == 2 && !s->volume && !s->acknowledged);
    CHECK(Tick(a, clock, s) && s->phase == 3 && s->decoder_present);
    *held = s->decoder;
    CHECK(Tick(a, clock, s) && s->phase == 4 && !s->serial && !s->decoder_present &&
          !s->acknowledged);
    CHECK(Tick(a, clock, s) && s->saved == (saved < 2 ? saved + 1 : 1) && s->queue_empty &&
          s->acknowledged && !s->playing_xa);
    return true;
}
static bool RestoreMusic(SotnCdAudio *a, Uint32 *clock, SotnCdAudioState *s, SotnMusicState held)
{
    const Uint8 saved = s->saved;
    CHECK(saved && SotnCdAudio_Queue(a, 0x11) && SotnCdAudio_Get(a, s) && !s->acknowledged);
    for (Uint16 i = 1; i <= 6; ++i)
    {
        CHECK(Tick(a, clock, s) && s->phase == i && s->playing_xa && s->busy && !s->acknowledged &&
              !s->serial);
        CHECK(s->decoder.remaining == held.remaining && s->decoder.sector == held.sector &&
              s->decoder.id == held.id);
    }
    CHECK(Tick(a, clock, s) && s->phase == 7 && s->serial);
    for (Uint16 i = 0; i < 11; ++i)
        CHECK(Tick(a, clock, s) && !s->acknowledged);
    CHECK(s->phase == 8 && s->volume == 127);
    CHECK(Tick(a, clock, s) && s->saved == saved - 1 && s->queue_empty && s->acknowledged &&
          !s->busy);
    return true;
}
static bool StopMusic(SotnCdAudio *a, Uint32 *clock, SotnCdAudioState *s)
{
    CHECK(SotnCdAudio_Queue(a, 0xa));
    for (Uint16 i = 0; i < 6; ++i)
        CHECK(Tick(a, clock, s));
    CHECK(s->queue_empty && !s->playing_xa && !s->serial && !s->decoder_present);
    return true;
}
static bool LoopVolume(SotnDataView dra, SotnStream *stream)
{
    SotnMusic *reference = NULL, *music = NULL;
    SotnCdAudio *a = NULL;
    CHECK(SotnMusic_Create(dra, stream, &reference) && SotnMusic_Create(dra, stream, &music));
    CHECK(SotnCdAudio_Create(dra, stream, &music, 0, &a) && !music);
    for (Uint32 tick = 0; tick < 40; ++tick)
    {
        Sint16 expected[SOTN_SOUND_FRAMES * 2] = {0}, actual[SOTN_SOUND_FRAMES * 2] = {0};
        SotnMusicState state = {0};
        CHECK(SotnMusic_Tick(reference, expected, SDL_arraysize(expected)) &&
              SotnMusic_Get(reference, &state));
        CHECK(SotnCdAudio_Tick(a, tick, actual, SDL_arraysize(actual)));
        for (Uint32 i = 0; i < SDL_arraysize(actual); ++i)
            CHECK(actual[i] == (Sint16)((Sint32)expected[i] * 127 / state.volume));
        CHECK(state.volume == (tick < 16 ? 127 : 63));
    }
    SotnCdAudio_Destroy(a);
    SotnMusic_Destroy(reference);
    return true;
}
static bool Exercise(SotnDataView dra, SotnStream *stream)
{
    SotnMusic *music = NULL;
    SotnCdAudio *a = NULL;
    SotnCdAudioState s = {0};
    Uint32 clock = 0xfffffff8;
    CHECK(SotnCdAudio_Create(dra, stream, &music, clock, &a) && !music && SotnCdAudio_Get(a, &s));
    CHECK(s.queue_empty && s.acknowledged && !s.playing_xa);
    CHECK(SotnCdAudio_AdoptMusic(a, &music, clock) && !music);
    CHECK(SotnCdAudio_Queue(a, 0x10) && SotnCdAudio_Get(a, &s) && !s.acknowledged && s.queue_empty);
    const SotnCdAudioState pending_adoption = s;
    CHECK(!SotnCdAudio_AdoptMusic(a, &music, clock) && !music && SotnCdAudio_Get(a, &s) &&
          SameState(s, pending_adoption));
    CHECK(Tick(a, &clock, &s) && s.queue_empty && s.acknowledged);
    CHECK(SotnCdAudio_Queue(a, 0x11) && Tick(a, &clock, &s) && s.queue_empty && s.acknowledged);
    const SotnCdAudioState before = s;
    CHECK(!SotnCdAudio_Queue(a, 0x533) && SotnCdAudio_Get(a, &s) && SameState(s, before));
    CHECK(StartVoice(a, &clock, 0x472, &s));
    const Uint32 started = s.started;
    CHECK(s.duration == 36 && s.decoder.duration == 16);
    while (clock - started <= 36)
        CHECK(Tick(a, &clock, &s) && s.playing_xa && !s.busy);
    CHECK(!s.decoder.remaining && s.playing_xa && s.volume == 127);
    CHECK(Tick(a, &clock, &s) && s.busy && s.command == 2 && s.volume == 95);
    for (Uint16 i = 0; i < 5; ++i)
        CHECK(Tick(a, &clock, &s));
    CHECK(!s.playing_xa && s.queue_empty && !s.decoder_present);
    SotnCdAudio_Destroy(a);

    // Real stage PCM continues through the existing native loop service while fading down.
    CHECK(SotnMusic_Create(dra, stream, &music));
    SotnMusic *empty = NULL;
    CHECK(SotnCdAudio_Create(dra, stream, &empty, clock, &a) && !empty && music &&
          SotnCdAudio_Get(a, &s) && !s.decoder_present &&
          SotnCdAudio_AdoptMusic(a, &music, clock) && !music && SotnCdAudio_Get(a, &s) &&
          s.decoder_present);
    const SotnCdAudioState adopted = s;
    CHECK(!SotnCdAudio_AdoptMusic(a, &empty, clock) && SotnCdAudio_Get(a, &s) &&
          SameState(s, adopted));
    for (Uint16 i = 0; i < 3; ++i)
        CHECK(Tick(a, &clock, &s));
    SotnMusicState stage = {0}, voice = {0};
    CHECK(PauseMusic(a, &clock, &s, &stage) && s.saved == 1);
    for (Uint16 i = 0; i < 25; ++i)
        CHECK(Tick(a, &clock, &s) && s.saved == 1 && !s.decoder_present);
    CHECK(StartVoice(a, &clock, 0x473, &s));
    CHECK(PauseMusic(a, &clock, &s, &voice) && s.saved == 2);
    for (Uint16 i = 0; i < 5; ++i)
        CHECK(Tick(a, &clock, &s) && s.saved == 2 && !s.decoder_present);
    CHECK(RestoreMusic(a, &clock, &s, voice) && s.saved == 1 && s.playing == 0x173);
    CHECK(StopMusic(a, &clock, &s));
    CHECK(RestoreMusic(a, &clock, &s, stage) && !s.saved);
    CHECK(s.playing == 46 || s.playing == 47);
    CHECK(SotnCdAudio_Queue(a, 0x11) && Tick(a, &clock, &s) && s.acknowledged && !s.saved);
    SotnCdAudio_Destroy(a);

    // The third pause wraps the original two-position cursor and replaces its old positions.
    CHECK(SotnCdAudio_Create(dra, stream, &music, clock, &a));
    for (Uint16 i = 0; i < 3; ++i)
    {
        CHECK(StartVoice(a, &clock, (Uint16)(0x472 + (i & 1)), &s));
        CHECK(PauseMusic(a, &clock, &s, &voice));
    }
    CHECK(s.saved == 1 && RestoreMusic(a, &clock, &s, voice) && s.saved == 0);
    SotnCdAudio_Destroy(a);

    // Both original queues retain their distinct overflow behavior and immediate ACK flag.
    CHECK(SotnCdAudio_Create(dra, stream, &music, clock, &a));
    for (Uint16 i = 0; i < 256; ++i)
        CHECK(SotnCdAudio_Queue(a, 0x10));
    CHECK(Tick(a, &clock, &s) && s.queue_empty && !s.acknowledged);
    CHECK(SotnCdAudio_Queue(a, 0x10) && Tick(a, &clock, &s) && s.queue_empty && s.acknowledged);
    for (Uint16 i = 0; i < 255; ++i)
        CHECK(SotnCdAudio_Queue(a, 0x10));
    CHECK(Tick(a, &clock, &s) && s.commands == 254 && s.acknowledged);
    CHECK(SotnCdAudio_Queue(a, 0x10) && SotnCdAudio_Queue(a, 0x10));
    CHECK(Tick(a, &clock, &s) && s.queue_empty && s.acknowledged);
    SotnCdAudio_Destroy(a);
    return true;
}
static bool CommandTransport(SotnDataView dra, SotnStream *stream)
{
    SotnMusic *music = NULL, *reference = NULL, *loop = NULL;
    CHECK(SotnMusic_CreateXa(dra, stream, 0x32e, &music) &&
          SotnMusic_CreateXa(dra, stream, 0x32e, &reference));
    Sint16 actual[SOTN_SOUND_FRAMES * 2], expected[SOTN_SOUND_FRAMES * 2];
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(SotnMusic_Tick(music, actual, SDL_arraysize(actual)) &&
              SotnMusic_Tick(reference, expected, SDL_arraysize(expected)) &&
              !SDL_memcmp(actual, expected, sizeof(actual)));
    SotnMusicState before = {0}, after = {0};
    CHECK(SotnMusic_Get(music, &before) && SotnMusic_ContinueXa(music) &&
          SotnMusic_Get(music, &after) && before.sector == after.sector && after.id == 47 &&
          after.looping && after.remaining == 16 && after.volume == 127);
    // Continuing metadata must retain buffered PCM, resampler and ADPCM history exactly.
    for (Uint16 i = 0; i < 24; ++i)
        CHECK(SotnMusic_Tick(music, actual, SDL_arraysize(actual)) &&
              SotnMusic_Tick(reference, expected, SDL_arraysize(expected)) &&
              !SDL_memcmp(actual, expected, sizeof(actual)));
    CHECK(SotnMusic_Get(music, &before));
    for (Uint16 i = 0; i < 2; ++i)
    {
        CHECK(SotnMusic_Tick(music, actual, SDL_arraysize(actual)));
        for (Uint16 j = 0; j < SDL_arraysize(actual); ++j)
            CHECK(!actual[j]);
    }
    CHECK(SotnMusic_Get(music, &after) && before.sector == after.sector &&
          SotnMusic_Playing(music)); // PCM exhaustion never completes the CD command owner.
    CHECK(SotnMusic_Get(music, &before) && SotnMusic_PrepareXaLoop(music, &loop) &&
          SotnMusic_Get(music, &after) && before.sector == after.sector &&
          before.remaining == after.remaining && SotnMusic_Get(loop, &after) &&
          after.sector == after.start && after.remaining == 16 && after.id == 47 && after.looping &&
          !SotnMusic_ContinueXa(loop));
    SotnMusic_Destroy(music);
    SotnMusic_Destroy(reference);
    music = reference = NULL;
    CHECK(SotnMusic_CreateXa(dra, stream, 0x32e, &reference) && SotnMusic_Prepare(loop));
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(SotnMusic_Tick(loop, actual, SDL_arraysize(actual)) &&
              SotnMusic_Tick(reference, expected, SDL_arraysize(expected)) &&
              !SDL_memcmp(actual, expected, sizeof(actual)));
    SotnMusic_Destroy(loop);
    SotnMusic_Destroy(reference);
    CHECK(!SotnMusic_CreateXa(dra, stream, 0x300, &music) && !music &&
          !SotnMusic_CreateXa(dra, stream, 0x533, &music) && !music);
    CHECK(SotnMusic_Create(dra, stream, &music) && !SotnMusic_ContinueXa(music));
    loop = NULL;
    CHECK(!SotnMusic_PrepareXaLoop(music, &loop) && !loop);
    SotnMusic_Destroy(music);
    return true;
}
static bool CommandMusic(SotnDataView dra, SotnStream *stream)
{
    SotnMusic *music = NULL;
    SotnCdAudio *a = NULL;
    SotnCdAudioState s = {0};
    Uint32 clock = SDL_MAX_UINT32 - 12U;
    CHECK(SotnCdAudio_Create(dra, stream, &music, clock, &a) && StartVoice(a, &clock, 0x32e, &s) &&
          s.mode == 1 && s.managed && s.duration == 16);
    const Uint32 started = s.started;
    while (clock - started <= 16)
        CHECK(Tick(a, &clock, &s) && s.started == started && !s.decoder.looping && s.queue_empty);
    const Uint32 sector = s.decoder.sector;
    CHECK(Tick(a, &clock, &s) && s.mode == 0 && s.managed && s.decoder.looping &&
          s.decoder.id == 47 && s.decoder.sector >= sector && s.queue_empty && s.volume == 127 &&
          s.target_volume == 127);
    const Uint32 continued = s.started;
    while (clock - continued <= 16)
        CHECK(Tick(a, &clock, &s) && s.started == continued && s.queue_empty);
    CHECK(Tick(a, &clock, &s) && s.command == 6 && s.phase == 2 && s.busy && s.serial &&
          s.decoder.sector == s.decoder.start && s.decoder.remaining == 16);
    CHECK(Tick(a, &clock, &s) && s.phase == 3 && s.decoder.sector == s.decoder.start + 1 &&
          s.decoder.remaining == 15);
    CHECK(Tick(a, &clock, &s) && s.phase == 4 && Tick(a, &clock, &s) && s.queue_empty && !s.busy &&
          s.playing == 46 && s.mode == 0 && s.duration == 16);
    SotnMusicState held = {0};
    CHECK(PauseMusic(a, &clock, &s, &held) && s.saved == 1 && StartVoice(a, &clock, 0x472, &s) &&
          s.saved == 1 && s.mode == 2 && StopMusic(a, &clock, &s) &&
          RestoreMusic(a, &clock, &s, held) && s.mode == 0 && s.managed && s.playing == 46);
    SotnCdAudio_Destroy(a);
    a = NULL;
    CHECK(SotnCdAudio_Create(dra, stream, &music, clock, &a) && StartVoice(a, &clock, 0x32e, &s) &&
          PauseMusic(a, &clock, &s, &held) && s.saved == 1 && StartVoice(a, &clock, 0x31d, &s) &&
          !s.saved && s.playing == 29);
    SotnCdAudio_Destroy(a);
    return true;
}
static bool LoopPauseOrder(SotnDataView dra, SotnStream *stream)
{
    SotnMusic *music = NULL;
    SotnCdAudio *a = NULL;
    SotnCdAudioState s = {0};
    Uint32 clock = 0;
    CHECK(SotnCdAudio_Create(dra, stream, &music, clock, &a) && StartVoice(a, &clock, 0x32e, &s));
    while (s.mode == 1)
        CHECK(Tick(a, &clock, &s));
    while (clock - s.started <= s.duration)
        CHECK(Tick(a, &clock, &s));
    CHECK(SotnCdAudio_Queue(a, 0x10) && Tick(a, &clock, &s) && s.command == 12 && s.commands == 3 &&
          s.phase == 1);
    bool restored = false, sought = false, complete = false;
    for (Uint16 i = 0; i < 100 && !complete; ++i)
    {
        CHECK(Tick(a, &clock, &s));
        restored |= s.command == 14;
        sought |= s.command == 6;
        complete = restored && sought && s.queue_empty && !s.busy;
    }
    CHECK(complete && s.playing == 46 && !s.saved && s.acknowledged && s.mode == 0);
    SotnCdAudio_Destroy(a);
    return true;
}
static bool VolumeFades(SotnDataView dra, SotnStream *stream)
{
    const Sint16 multipliers[] = {128, 8, 4, 2, 1};
    const Uint16 steps[] = {4, 64, 128, 256, 512};
    for (Uint16 profile = 0; profile < 5; ++profile)
    {
        SotnMusic *music = NULL;
        SotnCdAudio *a = NULL;
        SotnCdAudioState s = {0};
        Uint32 clock = SDL_MAX_UINT32 - 20U;
        CHECK(SotnCdAudio_Create(dra, stream, &music, clock, &a) &&
              StartVoice(a, &clock, 0x32e, &s) && s.fade_acknowledged && s.xa_active &&
              s.fade_multiplier == 32 && SotnCdAudio_Queue(a, (Uint16)(0x90 + profile)) &&
              SotnCdAudio_Get(a, &s) && !s.fade_acknowledged && s.acknowledged && s.queue_empty);
        CHECK(Tick(a, &clock, &s) && s.command == 8 && s.phase == 1 && s.busy &&
              s.fade_multiplier == multipliers[profile] && !s.fade_counter && s.volume == 127);
        for (Uint16 i = 1; i <= steps[profile]; ++i)
            CHECK(Tick(a, &clock, &s) && s.fade_counter == i &&
                  s.volume == 127 - 127 * multipliers[profile] * i / 512 &&
                  s.phase == (i == steps[profile] ? 2 : 1) && s.busy && !s.fade_acknowledged &&
                  s.playing_xa && s.xa_active);
        CHECK(Tick(a, &clock, &s) && s.fade_acknowledged && !s.busy && s.command == 2 &&
              s.commands == 1 && s.playing_xa && s.xa_active);
        for (Uint16 i = 0; i < 3; ++i)
            CHECK(Tick(a, &clock, &s));
        CHECK(s.queue_empty && !s.playing_xa && !s.xa_active && !s.decoder_present);
        SotnCdAudio_Destroy(a);
    }
    SotnMusic *music = NULL;
    SotnCdAudio *a = NULL;
    SotnCdAudioState s = {0};
    Uint32 clock = 0;
    CHECK(SotnCdAudio_Create(dra, stream, &music, clock, &a) && SotnCdAudio_Queue(a, 0x90) &&
          SotnCdAudio_Get(a, &s) && !s.fade_acknowledged && s.xa_active && !s.decoder_present &&
          s.acknowledged && !SotnCdAudio_AdoptMusic(a, &music, clock));
    CHECK(Tick(a, &clock, &s) && s.phase == 1 && Tick(a, &clock, &s) && s.phase == 2 &&
          s.fade_counter == 1 && Tick(a, &clock, &s) && s.fade_acknowledged && !s.xa_active &&
          !s.playing_xa && s.command == 2 && !s.queue_empty && Tick(a, &clock, &s) &&
          s.queue_empty);
    SotnCdAudio_Destroy(a);
    return true;
}
static bool FadeRetargetAndLoop(SotnDataView dra, SotnStream *stream)
{
    SotnMusic *music = NULL;
    SotnCdAudio *a = NULL;
    SotnCdAudioState s = {0};
    Uint32 clock = 0;
    CHECK(SotnCdAudio_Create(dra, stream, &music, clock, &a) && StartVoice(a, &clock, 0x32e, &s) &&
          SotnCdAudio_Queue(a, 0x90) && Tick(a, &clock, &s) && Tick(a, &clock, &s) &&
          s.volume == 96 && SotnCdAudio_Queue(a, 0x94) && Tick(a, &clock, &s) &&
          s.fade_counter == 2 && s.fade_multiplier == 1 && s.volume == 127);
    // The multiplier is shared. Retargeting may raise gain; the acknowledgement is one
    // original global flag and may clear while another fade remains queued.
    Uint16 updates = 0;
    while (!s.fade_acknowledged && updates++ < 520)
        CHECK(Tick(a, &clock, &s));
    CHECK(updates < 520 && s.fade_acknowledged && s.command == 8 && s.commands == 2 &&
          s.fade_counter == 512 && !s.busy);
    SotnCdAudio_Destroy(a);
    a = NULL;
    CHECK(SotnCdAudio_Create(dra, stream, &music, clock, &a) && StartVoice(a, &clock, 0x32e, &s));
    while (s.mode == 1)
        CHECK(Tick(a, &clock, &s));
    CHECK(SotnCdAudio_Queue(a, 0x94));
    updates = 0;
    bool retired = false, loop = false;
    while (updates++ < 550)
    {
        CHECK(Tick(a, &clock, &s));
        retired |= !s.playing_xa && !s.decoder_present && s.command == 6;
        loop |= retired && s.command == 6 && s.phase == 2 && s.decoder_present &&
                s.decoder.sector == s.decoder.start;
        if (loop && s.queue_empty)
            break;
    }
    CHECK(updates < 550 && retired && loop && s.queue_empty && s.playing == 46 && s.playing_xa &&
          s.xa_active && !s.serial && s.fade_acknowledged && !s.volume);
    SotnCdAudio_Destroy(a);
    return true;
}
static void Sound16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static bool BoundControls(SotnSoundSources inputs, SotnDataView image, SotnDataView seq, Uint8 *dra,
                          SotnStream *stream)
{
    SotnSound *sound = NULL;
    SotnCdAudio *a = NULL;
    SotnMusic *music = NULL;
    SotnCdAudioState state = {0};
    SotnSoundFrame frame = {0};
    Sint16 pcm[SOTN_SOUND_FRAMES * 2];
    Uint32 clock = SDL_MAX_UINT32 - 12U;
    CHECK(SotnSound_Create(inputs, &sound) && SotnSound_ConfigureReverb(sound, image) &&
          SotnSound_SetStageSequence(sound, image, seq, 0x202) &&
          SotnCdAudio_Create(inputs.dra, stream, &music, clock, &a) && !SotnCdAudio_Queue(a, 7) &&
          !SotnCdAudio_Queue(a, 0xe) && !SotnCdAudio_Queue(a, 0x202) &&
          SotnCdAudio_BindSound(a, sound));
    const Uint8 record[] = {1, 0, 60, 0, 127, 3, 255};
    SDL_memcpy(dra + 0x1000, record, sizeof(record));
    sound->scripts[0] = (SoundScript){
        .id = 0x601, .allocation_id = 0x601, .address = 0x800a1000, .timer = 1, .volume = 127};
    CHECK(SotnCdAudio_Queue(a, 0xe) && !sound->scripts_paused &&
          SotnSound_UpdateCommands(sound, &frame) && !sound->scripts[0].timer &&
          Tick(a, &clock, &state) && sound->scripts_paused && !sound->scripts[0].id &&
          sound->script_backup[0].id == 0x601 && !sound->script_backup[0].timer &&
          SotnSound_RenderFrame(sound, frame, pcm, SDL_arraysize(pcm)));
    CHECK(SotnCdAudio_Queue(a, 0xf) && SotnSound_UpdateCommands(sound, &frame) &&
          !sound->scripts[0].id && Tick(a, &clock, &state) && !sound->scripts_paused &&
          sound->scripts[0].id == 0x601 && sound->scripts[0].address == 0x800a1000 &&
          !sound->scripts[0].steps && SotnSound_RenderFrame(sound, frame, pcm, SDL_arraysize(pcm)));
    CHECK(SotnSound_UpdateCommands(sound, &frame) && sound->scripts[0].address == 0x800a1006 &&
          sound->scripts[0].timer == 3 && sound->voices[0].active && Tick(a, &clock, &state) &&
          SotnSound_RenderFrame(sound, frame, pcm, SDL_arraysize(pcm)));
    CHECK(SotnCdAudio_Queue(a, 0x202) && !sound->sequence_open &&
          SotnSound_UpdateCommands(sound, &frame) && frame.first_script == 0 &&
          sound->scripts[0].timer == 2 && Tick(a, &clock, &state) && sound->sequence_open &&
          sound->sequence.playing && SotnSound_RenderFrame(sound, frame, pcm, SDL_arraysize(pcm)));
    CHECK(SotnCdAudio_Queue(a, 7) && sound->sequence_open &&
          SotnSound_UpdateCommands(sound, &frame) && frame.first_script == 3 &&
          sound->scripts[0].timer == 2 && Tick(a, &clock, &state) && !sound->sequence_open &&
          !sound->scripts[0].id && sound->voices[23].key_off &&
          SotnSound_RenderFrame(sound, frame, pcm, SDL_arraysize(pcm)));
    CHECK(StartVoice(a, &clock, 0x32e, &state));
    sound->scripts[0] = (SoundScript){
        .id = 0x601, .allocation_id = 0x601, .address = 0x800a1000, .timer = 1, .volume = 127};
    CHECK(SotnCdAudio_Queue(a, 0x32e) && SotnCdAudio_Queue(a, 0xe) &&
          SotnSound_UpdateCommands(sound, &frame) && Tick(a, &clock, &state) &&
          !sound->scripts_paused && sound->scripts[0].id == 0x601 &&
          SotnSound_RenderFrame(sound, frame, pcm, SDL_arraysize(pcm)));
    // The duplicate XA return holds the following SPU command and its live cursor copy.
    CHECK(SotnSound_UpdateCommands(sound, &frame) && sound->scripts[0].timer == 3 &&
          Tick(a, &clock, &state) && sound->scripts_paused && !sound->scripts[0].id &&
          sound->script_backup[0].address == 0x800a1006 && sound->script_backup[0].timer == 3 &&
          SotnSound_RenderFrame(sound, frame, pcm, SDL_arraysize(pcm)));
    for (Uint16 i = 0; i < 24; ++i)
    {
        CHECK(SoundKey(sound, i, 1, 0, 0, 60, 127, 64));
        sound->registers[i].adsr[1] |= 32;
        sound->voices[i].adsr_hi = sound->registers[i].adsr[1];
    }
    CHECK(SotnCdAudio_Queue(a, 0xa1) && SotnSound_UpdateCommands(sound, &frame) &&
          Tick(a, &clock, &state));
    for (Uint16 i = 0; i < 24; ++i)
        CHECK(sound->voices[i].key_off == (i == 20 || i == 21) &&
              (sound->registers[i].adsr[1] & 63U) == ((i == 20 || i == 21) ? 14U : 32U) &&
              sound->voices[i].adsr_hi == sound->registers[i].adsr[1]);
    CHECK(SotnSound_RenderFrame(sound, frame, pcm, SDL_arraysize(pcm)));
    SotnCdAudio_Destroy(a);
    SotnSound_Destroy(sound);
    return true;
}
static bool MasterFades(SotnSoundSources inputs, SotnDataView image, SotnDataView seq,
                        SotnStream *stream)
{
    const Sint16 multipliers[] = {32, 8, 4, 2, 1};
    const Uint16 steps[] = {16, 64, 128, 256, 512};
    for (Uint16 profile = 0; profile < 5; ++profile)
        for (Uint16 context = 0; context < 4; ++context)
        {
            SotnSound *sound = NULL;
            SotnCdAudio *a = NULL;
            SotnMusic *music = NULL;
            SotnCdAudioState s = {0};
            Uint32 clock = SDL_MAX_UINT32 - 20U;
            CHECK(SotnSound_Create(inputs, &sound) && SotnSound_ConfigureReverb(sound, image) &&
                  SotnCdAudio_Create(inputs.dra, stream, &music, clock, &a) &&
                  SotnCdAudio_Get(a, &s));
            const SotnCdAudioState unbound = s;
            CHECK(!SotnCdAudio_Queue(a, 0x80) && SotnCdAudio_Get(a, &s) && SameState(unbound, s) &&
                  SotnCdAudio_BindSound(a, sound));
            if (context & 1U)
                CHECK(StartVoice(a, &clock, 0x32e, &s));
            if (context & 2U)
                CHECK(SotnSound_SetStageSequence(sound, image, seq, 0x202) &&
                      SotnSound_PlaySequence(sound, 0x202));
            for (Uint16 i = 0; i < 24; ++i)
                CHECK(SoundKey(sound, i, 1, 0, 0, 60, 127, 64));
            const SoundScript paused = {.id = 0x602, .address = 0x800b0000, .timer = 9};
            sound->script_backup[0] = paused;
            sound->scripts_paused = true;
            sound->scripts[0] = (SoundScript){.id = 0x601, .allocation_id = 0x601};
            sound->priority[0] = 7;
            sound->ambience_id = 0x603;
            CHECK(SotnSound_Queue(sound, 0x604) && SotnCdAudio_Queue(a, (Uint16)(0x80 + profile)) &&
                  SotnCdAudio_Get(a, &s) && !s.fade_acknowledged && s.master_volume == 127 &&
                  Tick(a, &clock, &s) && s.phase == 1 && s.command == 10 && s.busy &&
                  s.fade_multiplier == multipliers[profile] && !s.fade_counter);
            for (Uint16 i = 1; i <= steps[profile]; ++i)
                CHECK(Tick(a, &clock, &s) && s.fade_counter == i &&
                      s.master_volume == 127 - 127 * multipliers[profile] * i / 512 &&
                      s.master_target == 127 && s.volume == ((context & 1U) ? 127 : 0) &&
                      s.phase == (i == steps[profile] ? 2 : 1) && !s.fade_acknowledged &&
                      !sound->voices[23].key_off);
            CHECK(Tick(a, &clock, &s) && s.phase == 3 && s.release_counter == 3 &&
                  s.commands == 2 && !sound->reverb.depth[0] && !sound->reverb.depth[1] &&
                  !sound->sequence_open && !sound->sequence.playing &&
                  sound->voices[23].key_off == ((context & 2U) != 0));
            // A new actual voice after StopSeq still receives the later all-voice release.
            CHECK(SoundKey(sound, 23, 1, 0, 0, 60, 127, 64));
            sound->scripts[3] = (SoundScript){.id = 0x601, .allocation_id = 0x601};
            CHECK(Tick(a, &clock, &s) && s.release_counter == 2 && !sound->voices[23].key_off &&
                  Tick(a, &clock, &s) && s.release_counter == 1 && !sound->voices[23].key_off &&
                  Tick(a, &clock, &s) && !s.release_counter && s.command == 3 && !s.phase &&
                  !s.busy && s.fade_acknowledged && !s.master_volume);
            for (Uint16 i = 0; i < 24; ++i)
                CHECK(sound->voices[i].key_off && sound->voices[i].active &&
                      (sound->registers[i].adsr[1] & 63U) == 8 &&
                      sound->voices[i].adsr_hi == sound->registers[i].adsr[1]);
            const SoundScript empty = {0};
            for (Uint16 i = 0; i < 4; ++i)
                CHECK(!SDL_memcmp(&sound->scripts[i], &empty, sizeof(empty)) &&
                      !sound->priority[i]);
            CHECK(!sound->ambience_id && sound->queued == 1 && !sound->scripts_paused &&
                  !SDL_memcmp(&sound->script_backup[0], &paused, sizeof(paused)));
            for (Uint16 i = 0; i < 6 && s.command == 3; ++i)
                CHECK(Tick(a, &clock, &s));
            CHECK(s.command != 3 && !s.playing_xa && s.master_volume == 127 &&
                  s.master_target == 127 && !s.serial);
            SotnCdAudio_Destroy(a);
            SotnSound_Destroy(sound);
        }
    return true;
}
static bool BoundReverb(Uint8 *dra, SotnStream *stream)
{
    Uint8 header[3104] = {0}, body[32] = {0};
    Uint8 *image = SDL_calloc(1, 0x24000);
    SotnSound *reference = SDL_malloc(sizeof(*reference));
    CHECK(image && reference);
    Put32(header, 0x56414270);
    Put32(header + 4, 7);
    Sound16(header + 18, 1);
    Sound16(header + 22, 1);
    header[24] = header[33] = header[2082] = 127;
    header[25] = header[36] = header[2083] = 64;
    header[32] = 1;
    header[2081] = 4;
    header[2084] = 60;
    Sound16(header + 2096, 15);
    Sound16(header + 2098, 0x1fc0);
    Sound16(header + 2102, 1);
    Sound16(header + 2594, 4);
    body[0] = body[16] = 4;
    body[1] = 6;
    body[17] = 3;
    for (Uint16 i = 2; i < 16; ++i)
        body[i] = body[i + 16] = 0x11;
    SDL_memcpy(dra + 0x1107c, header, sizeof(header));
    for (Uint32 i = 0; i < 4; ++i)
        Put32(dra + 0x1d1c8 + i * 4, (i + 1) * 0x1000);
    const Uint16 coefficients[32] = {1, 1, 16384, 32767, 0, 0, 0,  8192, 16384, 16384, 2,
                                     3, 2, 3,     2,     3, 2, 3,  4,    5,     2,     3,
                                     2, 3, 4,     5,     8, 9, 12, 13,   32767, 32767};
    Put32(image + 0x23d68 + 12, (SOTN_SPU_RAM_SIZE - 4096) / 8);
    for (Uint32 i = 0; i < 32; ++i)
        Sound16(image + 0x23db8 + 204 + 4 + i * 2, coefficients[i]);
    const SotnSoundSources inputs = {{dra, 0x20000},           {body, sizeof(body)},
                                     {header, sizeof(header)}, {body, sizeof(body)},
                                     {header, sizeof(header)}, {body, sizeof(body)}};
    Uint8 seq[19] = {'p', 'Q', 'E', 'S', 0, 0, 0, 1, 0, 60, 7, 0xa1, 0x20, 4, 2, 0, 0xff, 0x2f, 0};
    for (Uint16 profile = 0; profile < 2; ++profile)
    {
        SotnSound *sound = NULL, *wrong = NULL;
        SotnCdAudio *a = NULL;
        SotnMusic *music = NULL;
        SotnCdAudioState state = {0}, held = {0};
        Uint32 clock = profile ? SDL_MAX_UINT32 - 20U : 0;
        CHECK(SotnSound_Create(inputs, &sound) && SotnSound_Create(inputs, &wrong) &&
              SotnSound_ConfigureReverb(sound, (SotnDataView){image, 0x24000}) &&
              SotnCdAudio_Create(inputs.dra, stream, &music, clock, &a) &&
              SotnCdAudio_Get(a, &held) && SotnCdAudio_BindSound(a, sound) &&
              !SotnCdAudio_BindSound(a, wrong) && SotnCdAudio_Get(a, &state) &&
              SameState(held, state));
        SotnSound_Destroy(wrong);
        if (profile)
        {
            dra[0x1d1e0 + 6] = 127;
            dra[0x1d1e0 + 7] = 64;
            dra[0x1d1e0 + 8] = 0;
            CHECK(SotnSound_SetStageSequence(sound, (SotnDataView){image, 0x24000},
                                             (SotnDataView){seq, sizeof(seq)}, 0x202) &&
                  SotnSound_PlaySequence(sound, 0x202));
        }
        CHECK(sound->reverb_restore_depth == (profile ? 64 : 0) &&
              SoundKey(sound, 14, 1, 0, 0, 60, 127, 64) && StartVoice(a, &clock, 0x32e, &state));
        bool temporary = false, restored = false, replaced = false;
        for (Uint16 i = 0; i < 120 && !restored; ++i)
        {
            const bool restoring = state.command == 6 && state.phase == 4;
            SotnSoundFrame frame = {0};
            Sint16 pcm[SOTN_SOUND_FRAMES * 2], expected[SOTN_SOUND_FRAMES * 2];
            CHECK(SotnSound_UpdateCommands(sound, &frame));
            const SotnReverb before = sound->reverb;
            *reference = *sound;
            CHECK(Tick(a, &clock, &state) && before.cursor == sound->reverb.cursor &&
                  before.phase == sound->reverb.phase);
            const bool starting = !temporary && state.command == 6 && state.phase == 2;
            if (starting || restoring)
            {
                const Sint16 depth = restoring ? (profile ? 64 : 0) : 127;
                CHECK(sound->reverb.depth[0] == (restoring ? (profile ? 16512 : 0) : 32767));
                CHECK(SotnSound_SetReverbDepth(reference, depth, depth));
                SotnSoundFrame ref_frame = frame;
                ref_frame.owner = reference;
                CHECK(
                    SotnSound_RenderFrame(reference, ref_frame, expected, SDL_arraysize(expected)));
            }
            CHECK(SotnSound_RenderFrame(sound, frame, pcm, SDL_arraysize(pcm)));
            if (starting || restoring)
                CHECK(!SDL_memcmp(pcm, expected, sizeof(pcm)));
            if (starting)
            {
                temporary = true;
                SotnSound *next = NULL;
                CHECK(SotnSound_PrepareFamiliarBank(sound, inputs.player_header, inputs.player_body,
                                                    &next) &&
                      SotnCdAudio_BindSound(a, next));
                SotnSound_Destroy(sound);
                sound = next;
                replaced = true;
            }
            restored = restoring;
        }
        CHECK(temporary && restored && replaced && state.queue_empty && state.playing == 46);
        SotnCdAudio_Destroy(a);
        SotnSound_Destroy(sound);
    }
    CHECK(MasterFades(inputs, (SotnDataView){image, 0x24000}, (SotnDataView){seq, sizeof(seq)},
                      stream));
    CHECK(BoundControls(inputs, (SotnDataView){image, 0x24000}, (SotnDataView){seq, sizeof(seq)},
                        dra, stream));
    SDL_free(reference);
    SDL_free(image);
    return true;
}
static bool Run(const char *directory)
{
    Uint8 *dra = SDL_calloc(1, 0x20000), *raw = SDL_calloc(32, SOTN_CD_SECTOR_BYTES);
    CHECK(dra && raw);
    const Uint16 ids[] = {46, 47, 29, 30, 0x172, 0x173, 0x176, 0x179, 0x17a, 0x18a, 0x18e, 0x192};
    for (Uint16 i = 0; i < SDL_arraysize(ids); ++i)
    {
        Uint8 *p = dra + 0x1d224 + ids[i] * 16;
        Put32(p, 0);
        Put32(p + 4, 16);
        p[8] = 1;
        p[10] = 127;
        Put32(p + 12, i < 4 ? ((i & 1U) ? 0 : 1) : 2);
    }
    for (Uint16 i = 0; i < 32; ++i)
    {
        Uint8 *s = raw + i * SOTN_CD_SECTOR_BYTES;
        s[15] = 2;
        s[16] = 1;
        s[18] = 0x64;
        s[19] = 1;
        SDL_memcpy(s + 20, s + 16, 4);
        for (Uint16 g = 0; g < 18; ++g)
        {
            Uint8 *b = s + 24 + g * 128;
            for (Uint16 j = 4; j < 12; ++j)
                b[j] = 4;
            for (Uint16 j = 16; j < 128; ++j)
                b[j] = (Uint8)(0x11 * (1 + i % 7));
        }
    }
    char digest[41] = {0};
    char *path = NULL;
    CHECK(SDL_asprintf(&path, "%s/cd-audio.txt", directory) > 0);
    SotnData_Sha1(raw, 32 * SOTN_CD_SECTOR_BYTES, digest);
    CHECK(SDL_SaveFile(path, raw, 32 * SOTN_CD_SECTOR_BYTES));
    SotnDataSpec spec = {"cd-audio.txt", 32 * SOTN_CD_SECTOR_BYTES, digest};
    SotnStream *stream = NULL;
    CHECK(SotnData_OpenStreamSpec(directory, &spec, &stream));
    dra[0x1d224 + 47 * 16 + 10] = 63;
    CHECK(LoopVolume((SotnDataView){dra, 0x20000}, stream) &&
          CommandTransport((SotnDataView){dra, 0x20000}, stream) &&
          CommandMusic((SotnDataView){dra, 0x20000}, stream) &&
          LoopPauseOrder((SotnDataView){dra, 0x20000}, stream) &&
          VolumeFades((SotnDataView){dra, 0x20000}, stream) &&
          FadeRetargetAndLoop((SotnDataView){dra, 0x20000}, stream) && BoundReverb(dra, stream));
    dra[0x1d224 + 47 * 16 + 10] = 127;
    CHECK(Exercise((SotnDataView){dra, 0x20000}, stream));
    SotnStream_Close(stream);
    // A real corrupt first sector must fail ReadN without entering playing/ready state.
    raw[19] = raw[23] = 0;
    SotnData_Sha1(raw, 32 * SOTN_CD_SECTOR_BYTES, digest);
    CHECK(SDL_SaveFile(path, raw, 32 * SOTN_CD_SECTOR_BYTES));
    CHECK(SotnData_OpenStreamSpec(directory, &spec, &stream));
    SotnMusic *music = NULL;
    SotnCdAudio *a = NULL;
    SotnCdAudioState s = {0};
    Uint32 clock = 0;
    CHECK(SotnCdAudio_Create((SotnDataView){dra, 0x20000}, stream, &music, clock, &a));
    CHECK(SotnCdAudio_Queue(a, 0x472));
    for (Uint16 i = 0; i < 3; ++i)
        CHECK(Tick(a, &clock, &s));
    Sint16 pcm[SOTN_SOUND_FRAMES * 2] = {0};
    CHECK(!SotnCdAudio_Tick(a, clock, pcm, SDL_arraysize(pcm)) && SotnCdAudio_Get(a, &s));
    CHECK(s.phase == 6 && s.busy && !s.playing_xa && !s.serial);
    SotnCdAudio_Destroy(a);
    SotnStream_Close(stream);
    CHECK(SDL_RemovePath(path));
    SDL_free(path);
    SDL_free(dra);
    SDL_free(raw);
    return true;
}
int main(int argc, char **argv)
{
    if (argc != 2 || !Run(argv[1]))
        return 1;
    SDL_Log("SOTN_CD_AUDIO_TEST: PASS");
    return 0;
}
