#include "data_internal.h"
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
           a.decoder_present == b.decoder_present && a.decoder.sector == b.decoder.sector &&
           a.decoder.remaining == b.decoder.remaining && a.decoder.duration == b.decoder.duration &&
           a.decoder.start == b.decoder.start && a.decoder.id == b.decoder.id &&
           a.decoder.volume == b.decoder.volume && a.decoder.mode == b.decoder.mode &&
           a.decoder.looping == b.decoder.looping && a.decoder.once == b.decoder.once;
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
          LoopPauseOrder((SotnDataView){dra, 0x20000}, stream));
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
