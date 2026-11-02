// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/sound.c PlaySfx/ExecSoundCommands/AddCdSoundCommand/func_80133FCC,
// CdSoundCommand4/12/CdSoundStartXaPlayback/CdFadeOut2, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Native XA services replace CD hardware calls.
#include <SDL3/SDL.h>
#include <sotn/cd_audio.h>

typedef struct SavedMusic
{
    SotnMusic *music;
    Uint32 duration;
    Uint16 playing;
    bool finite;
} SavedMusic;
struct SotnCdAudio
{
    SotnDataView dra;
    SotnStream *stream;
    SotnMusic *current;
    SavedMusic bookmarks[2];
    Uint16 incoming[256], commands[256];
    Uint16 read, write, count, playing, queued, config;
    Uint32 started, duration;
    Uint8 phase, saved, volume, target_volume;
    bool acknowledge_busy, busy, serial, finite;
};
static bool Voice(Uint16 id)
{
    return id == 0x472 || id == 0x473 || id == 0x476 || id == 0x479 || id == 0x47a || id == 0x48a ||
           id == 0x48e || id == 0x492;
}
static void ForgetSaved(SotnCdAudio *a)
{
    for (Uint16 i = 0; i < 2; ++i)
    {
        SotnMusic_Destroy(a->bookmarks[i].music);
        a->bookmarks[i] = (SavedMusic){0};
    }
    a->saved = 0;
}
static void Add(SotnCdAudio *a, Uint16 command)
{
    a->commands[a->count++] = command;
    if (a->count == 256)
    {
        // Original overflow preserves the in-flight head/phase and discards the suffix.
        SDL_memset(a->commands + 1, 0, 255 * sizeof(*a->commands));
        a->count = 1;
    }
}
static void Advance(SotnCdAudio *a)
{
    SDL_memmove(a->commands, a->commands + 1, 255 * sizeof(*a->commands));
    --a->count;
}
static void Complete(SotnCdAudio *a, bool acknowledge)
{
    a->busy = false;
    a->phase = 0;
    if (acknowledge)
        a->acknowledge_busy = false;
    Advance(a);
}
static void SoundCommands(SotnCdAudio *a)
{
    while (a->read != a->write)
    {
        const Uint16 id = a->incoming[a->read];
        a->incoming[a->read] = 0;
        a->read = (Uint16)((a->read + 1U) & 255U);
        if (Voice(id))
        {
            if (a->playing)
            {
                if (a->playing == id - 0x300U)
                    return; // Original return holds the remaining sound-command ring until next
                            // tick.
                Add(a, 2);
            }
            a->queued = (Uint16)(id - 0x300U);
            Add(a, 4);
        }
        else
            Add(a, id == 0x10 ? 12 : id == 0x11 ? 14 : 2);
    }
}
static bool Current(SotnCdAudio *a, SotnMusicState *out)
{
    return SotnMusic_Get(a->current, out);
}
static bool Start(SotnCdAudio *a, Uint32 clock)
{
    SotnMusicState state = {0};
    switch (a->phase)
    {
    case 0: {
        SotnMusic *next = NULL;
        // Configure the real decoder before committing a native Setmode acknowledgement.
        if (!SotnMusic_CreateSpeech(a->dra, a->stream, (Uint16)(a->queued + 0x300U), &next))
            return false;
        if (!SotnMusic_Get(next, &state))
        {
            SotnMusic_Destroy(next);
            return false;
        }
        SotnMusic_Destroy(a->current);
        a->current = next;
        a->busy = true;
        a->config = a->queued;
        a->volume = a->target_volume = state.volume;
        a->phase = 2; // Original0 falls through1: native decoder configuration committed.
        break;
    }
    case 2:
        // Original2 falls through3. The decoder owns the actual validated file/channel filter.
        if (!Current(a, &state))
            return false;
        a->phase = 4;
        break;
    case 4:
        // Original4 falls through5. Verify the decoder's real bounded initial stream position.
        if (!Current(a, &state) || state.sector != state.start)
            return SDL_SetError("Native XA start lost its actual initial stream cursor");
        a->duration = state.duration + (state.mode == 2 ? 20U : 0U);
        a->finite = state.once;
        a->phase = 6;
        break;
    case 6:
        // ReadN acknowledgement follows actual first-sector I/O and ADPCM conversion.
        if (!SotnMusic_Prepare(a->current))
            return false;
        ++a->phase;
        break;
    case 7:
        if (!Current(a, &state))
            return false;
        ++a->phase;
        break;
    case 8:
        a->serial = true;
        a->started = clock;
        a->playing = a->config;
        Complete(a, false);
        break;
    default:
        return SDL_SetError("Native XA start has an unsupported original command phase");
    }
    return true;
}
static bool Pause(SotnCdAudio *a, Uint32 clock)
{
    SotnMusicState state = {0};
    switch (a->phase)
    {
    case 0:
        if (a->saved >= 2)
            ForgetSaved(a); // Original two-record cursor wraps before the next save.
        if (!a->playing)
        {
            a->acknowledge_busy = false;
            Advance(a);
        }
        else
        {
            a->busy = true;
            ++a->phase;
        }
        break;
    case 1:
        a->volume = (Uint8)SDL_max(0, (Sint32)a->volume - 12);
        if (!a->volume)
            ++a->phase;
        break;
    case 2:
        if (!Current(a, &state))
            return false; // Actual native Getloc snapshot, without a simulated readiness timer.
        ++a->phase;
        break;
    case 3: {
        if (!Current(a, &state))
            return false;
        const Uint32 elapsed = clock - a->started;
        const Uint32 remaining = a->duration > elapsed ? a->duration - elapsed : 1;
        SavedMusic *saved = &a->bookmarks[a->saved];
        SotnMusic_Destroy(saved->music);
        *saved = (SavedMusic){a->current, remaining, a->playing, a->finite};
        a->current = NULL;
        a->serial = false; // Actual retained decoder stops advancing at native Pause commit.
        ++a->phase;
        break;
    }
    case 4:
        ++a->saved;
        a->playing = 0;
        Complete(a, true);
        break;
    default:
        return SDL_SetError("Native XA pause has an unsupported original command phase");
    }
    return true;
}
static bool Restore(SotnCdAudio *a, Uint32 clock)
{
    SotnMusicState state = {0};
    switch (a->phase)
    {
    case 0:
        if (!a->saved || a->saved > 2 || a->playing)
        {
            a->acknowledge_busy = false;
            Advance(a);
            break;
        }
        if (!SotnMusic_Get(a->bookmarks[a->saved - 1U].music, &state))
            return false;
        SotnMusic_Destroy(a->current);
        a->current = a->bookmarks[a->saved - 1U].music;
        a->bookmarks[a->saved - 1U].music = NULL;
        a->playing = a->bookmarks[a->saved - 1U].playing;
        a->target_volume = state.volume;
        a->volume = 0;
        a->busy = true;
        ++a->phase;
        break;
    case 1:
    case 2:
    case 3:
        // Native mode/filter/seek operate on the actual retained decoder, not a new clip.
        if (!Current(a, &state))
            return false;
        ++a->phase;
        break;
    case 4:
        if (!SotnMusic_Prepare(a->current))
            return false;
        ++a->phase;
        break;
    case 5:
        if (!Current(a, &state))
            return false;
        ++a->phase;
        break;
    case 6:
        a->started = clock;
        a->duration = a->bookmarks[a->saved - 1U].duration;
        a->finite = a->bookmarks[a->saved - 1U].finite;
        a->serial = true;
        ++a->phase;
        break;
    case 7:
        a->volume = (Uint8)SDL_min((Sint32)a->volume + 12, a->target_volume);
        if (a->volume == a->target_volume)
            ++a->phase;
        break;
    case 8:
        --a->saved;
        Complete(a, true);
        break;
    default:
        return SDL_SetError("Native XA restore has an unsupported original command phase");
    }
    return true;
}
static bool Stop(SotnCdAudio *a)
{
    SotnMusicState state = {0};
    switch (a->phase)
    {
    case 0:
        if (!a->playing)
        {
            Advance(a);
            break;
        }
        a->busy = true;
        a->volume = (Uint8)SDL_max(0, (Sint32)a->volume - 32);
        if (!a->volume)
            ++a->phase;
        break;
    case 1:
        if (!Current(a, &state))
            return false;
        a->serial = false;
        a->phase = 3; // Original1 falls through2 and commits native Pause.
        break;
    case 3:
        SotnMusic_Destroy(a->current);
        a->current = NULL;
        a->playing = 0;
        Complete(a, false);
        break;
    default:
        return SDL_SetError("Native XA stop has an unsupported original command phase");
    }
    return true;
}
bool SotnCdAudio_Create(SotnDataView dra, SotnStream *stream, SotnMusic **music, Uint32 clock,
                        SotnCdAudio **out)
{
    if (!out)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (!music || !dra.bytes || !SotnStream_Sectors(stream))
        return SDL_SetError("Native CD owner requires actual private data/stream and music owner");
    SotnCdAudio *a = SDL_calloc(1, sizeof(*a));
    if (!a)
        return false;
    a->dra = dra;
    a->stream = stream;
    if (!SotnCdAudio_AdoptMusic(a, music, clock))
    {
        SDL_free(a);
        return false;
    }
    *out = a;
    return true;
}
bool SotnCdAudio_AdoptMusic(SotnCdAudio *a, SotnMusic **music, Uint32 clock)
{
    if (!a || !music || a->current || a->saved || a->bookmarks[0].music || a->bookmarks[1].music ||
        a->count || a->read != a->write || a->phase || a->playing || a->queued || a->config ||
        a->acknowledge_busy || a->busy || a->serial)
        return SDL_SetError(
            "Music adoption requires an actual empty CD owner without pending work");
    SotnMusicState state = {0};
    if (*music && !SotnMusic_Get(*music, &state))
        return false;
    a->current = *music;
    if (*music)
    {
        a->playing = state.id;
        a->started = clock;
        a->duration = state.remaining + (state.mode == 2 ? 20U : 0U);
        a->volume = a->target_volume = state.volume;
        a->serial = true;
        a->finite = state.once;
    }
    *music = NULL;
    return true;
}
bool SotnCdAudio_Queue(SotnCdAudio *a, Uint16 command)
{
    if (!a || (command != 0x10 && command != 0x11 && command != 0xa && !Voice(command)))
        return SDL_SetError("Native CD owner requires original10/11/A or selected US Fairy speech");
    if (command == 0x10 || command == 0x11)
        a->acknowledge_busy = true;
    a->incoming[a->write] = command;
    a->write = (Uint16)((a->write + 1U) & 255U);
    return true;
}
bool SotnCdAudio_Tick(SotnCdAudio *a, Uint32 clock, Sint16 *pcm, size_t count)
{
    if (!a || !pcm || count < SOTN_SOUND_FRAMES * 2)
        return SDL_SetError("Native CD tick requires its actual owner and full stereo PCM output");
    SoundCommands(a);
    // Original strict comparison and private mode-two twenty-tick tail, before CD dispatch.
    if (a->playing && !a->busy && a->finite && clock - a->started > a->duration)
        Add(a, 2);
    if (a->count)
    {
        bool ok = true;
        switch (a->commands[0])
        {
        case 2:
            ok = Stop(a);
            break;
        case 4:
            ok = Start(a, clock);
            break;
        case 12:
            ok = Pause(a, clock);
            break;
        case 14:
            ok = Restore(a, clock);
            break;
        default:
            return SDL_SetError("Native CD queue lost its selected original command identity");
        }
        if (!ok)
            return false;
    }
    SDL_memset(pcm, 0, SOTN_SOUND_FRAMES * 2 * sizeof(*pcm));
    if (a->serial && a->current)
    {
        SotnMusicState state = {0};
        if (!SotnMusic_Tick(a->current, pcm, count) || !Current(a, &state))
            return false;
        for (Uint32 i = 0; i < SOTN_SOUND_FRAMES * 2; ++i)
            pcm[i] = state.volume ? (Sint16)((Sint32)pcm[i] * a->volume / state.volume) : 0;
    }
    return true;
}
bool SotnCdAudio_Get(const SotnCdAudio *a, SotnCdAudioState *out)
{
    if (!a || !out)
        return SDL_SetError("Native CD snapshot requires its actual owner and output");
    SotnCdAudioState state = {.started = a->started,
                              .duration = a->duration,
                              .playing = a->playing,
                              .queued = a->queued,
                              .command = a->count ? a->commands[0] : 0,
                              .commands = a->count,
                              .phase = a->phase,
                              .saved = a->saved,
                              .volume = a->volume,
                              .target_volume = a->target_volume,
                              .acknowledged = !a->acknowledge_busy,
                              .queue_empty = !a->count,
                              .playing_xa = a->playing != 0,
                              .busy = a->busy,
                              .serial = a->serial,
                              .decoder_present = a->current != NULL};
    if (a->current && !SotnMusic_Get(a->current, &state.decoder))
        return false;
    *out = state;
    return true;
}
void SotnCdAudio_Destroy(SotnCdAudio *a)
{
    if (a)
    {
        SotnMusic_Destroy(a->current);
        ForgetSaved(a);
        SDL_free(a);
    }
}
