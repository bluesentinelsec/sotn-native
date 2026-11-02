// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/sound.c PlaySfx/ExecSoundCommands/AddCdSoundCommand/func_80133FCC,
// CdSoundCommand4/12/CdSoundStartXaPlayback/CdFadeOut2, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Native XA services replace CD hardware calls.
#include "sound_internal.h"
#include <SDL3/SDL.h>
#include <sotn/cd_audio.h>

typedef struct SavedMusic
{
    SotnMusic *music;
    Uint32 duration;
    Uint16 playing;
    Uint8 mode;
    bool finite, managed, native_ready;
} SavedMusic;
struct SotnCdAudio
{
    SotnDataView dra;
    SotnStream *stream;
    SotnMusic *current;
    SotnSound *sound;
    SavedMusic bookmarks[2];
    Uint16 incoming[256], commands[256];
    Uint16 read, write, count, playing, queued, config;
    Uint32 started, duration;
    Sint16 fade_multiplier, fade_counter;
    Sint16 master_volume, master_target;
    Sint32 release_counter;
    Uint8 phase, saved, volume, target_volume, mode;
    bool acknowledge_busy, fade_busy, busy, serial, finite, managed, native_ready;
};
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static bool XaCommand(Uint16 id)
{
    return id > 0x300 && id < 0x533;
}
static bool SpuCommand(Uint16 id)
{
    return id == 7 || id == 0xb || id == 0xe || id == 0xf || id == 0xa1 || id == 0x202 ||
           id == 0x204;
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
    if (command == 6)
        for (Uint16 i = 0; i < a->count; ++i)
            if (a->commands[i] == 12)
            {
                a->commands[a->count++] = 14;
                if (a->count == 256)
                {
                    SDL_memset(a->commands + 1, 0, 255 * sizeof(*a->commands));
                    a->count = 1;
                    a->commands[a->count++] = 14;
                }
                break;
            }
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
static bool SoundCommands(SotnCdAudio *a)
{
    while (a->read != a->write)
    {
        const Uint16 id = a->incoming[a->read];
        a->incoming[a->read] = 0;
        a->read = (Uint16)((a->read + 1U) & 255U);
        if (XaCommand(id))
        {
            if (a->playing)
            {
                if (a->playing == id - 0x300U)
                    return true; // Original return holds every later command until next tick.
                Add(a, 2);
            }
            a->queued = (Uint16)(id - 0x300U);
            Add(a, 4);
        }
        else if (SpuCommand(id))
        {
            switch (id)
            {
            case 7:
                SotnSound_StopSequence(a->sound);
                break;
            case 0xb:
                Add(a, 2);
                SotnSound_StopSequence(a->sound);
                SoundReleaseAll(a->sound);
                break;
            case 0xe:
                SoundPauseScripts(a->sound);
                break;
            case 0xf:
                SoundResumeScripts(a->sound);
                break;
            case 0xa1:
                SoundReleaseAmbience20(a->sound);
                break;
            default:
                if (!SotnSound_PlaySequence(a->sound, id))
                    return false;
                break;
            }
        }
        else if (id >= 0x80 && id <= 0x84)
        {
            a->fade_multiplier = id == 0x80 ? 32 : (Sint16)(1U << (0x84U - id));
            Add(a, 10);
        }
        else if (id >= 0x90 && id <= 0x94)
        {
            a->fade_multiplier = id == 0x90 ? 128 : (Sint16)(1U << (0x94U - id));
            Add(a, 8);
        }
        else
            Add(a, id == 0x10 ? 12 : id == 0x11 ? 14 : id == 0x12 ? 16 : 2);
    }
    return true;
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
        if (!SotnMusic_CreateXa(a->dra, a->stream, (Uint16)(a->queued + 0x300U), &next))
            return false;
        if (!SotnMusic_Get(next, &state))
        {
            SotnMusic_Destroy(next);
            return false;
        }
        SotnMusic_Destroy(a->current);
        a->current = next;
        if (a->queued < 0x40)
            ForgetSaved(a);
        a->busy = true;
        a->managed = true;
        a->native_ready = false;
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
        a->mode = state.mode;
        a->phase = 6;
        break;
    case 6:
        // ReadN acknowledgement follows actual first-sector I/O and ADPCM conversion.
        if (!SotnMusic_Prepare(a->current))
            return false;
        a->native_ready = true;
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
static bool Loop(SotnCdAudio *a, Uint32 clock)
{
    SotnMusicState state = {0};
    switch (a->phase)
    {
    case 0: {
        SotnMusic *next = NULL, *configured = NULL;
        const SotnMusic *source = a->current;
        if (!source)
        {
            // A queued command6 may outlive command2's decoder retirement. The original
            // configured identity remains owned; reconstruct its real metadata/transport,
            // without ticking or enabling serial mix, before the actual loop seek.
            if (!a->config || a->queued != a->config)
                return SDL_SetError("XA loop lost its actual retained configuration");
            if (!SotnMusic_CreateXa(a->dra, a->stream, (Uint16)(a->config + 0x300U), &configured))
                return false;
            source = configured;
        }
        if (!SotnMusic_Get(source, &state) || a->queued != state.id - (state.looping ? 1U : 0U))
        {
            SotnMusic_Destroy(configured);
            return SDL_SetError(
                "XA loop lost its actual configured base; changed-queue contention is unsupported");
        }
        const bool prepared = SotnMusic_PrepareXaLoop(source, &next);
        SotnMusic_Destroy(configured);
        if (!prepared)
            return false;
        SotnMusic_Destroy(a->current);
        a->current = next;
        a->config = a->queued;
        a->busy = true;
        a->native_ready = false;
        a->phase = 2; // Actual native Setloc configuration, before ReadN.
        break;
    }
    case 2:
        if (!SotnMusic_Prepare(a->current))
            return false;
        a->native_ready = true;
        ++a->phase;
        break;
    case 3:
        if (!Current(a, &state))
            return false;
        ++a->phase;
        break;
    case 4:
        if (!Current(a, &state))
            return false;
        if (a->sound && !SotnSound_SetReverbDepth(a->sound, a->sound->reverb_restore_depth,
                                                  a->sound->reverb_restore_depth))
            return false;
        a->started = clock;
        a->duration = state.duration;
        a->playing = a->config;
        Complete(a, false);
        break;
    default:
        return SDL_SetError("Native XA loop has an unsupported original command phase");
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
        *saved = (SavedMusic){.music = a->current,
                              .duration = remaining,
                              .playing = a->playing,
                              .mode = a->mode,
                              .finite = a->finite,
                              .managed = a->managed,
                              .native_ready = a->native_ready};
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
        a->mode = a->bookmarks[a->saved - 1U].mode;
        a->managed = a->bookmarks[a->saved - 1U].managed;
        a->native_ready = a->bookmarks[a->saved - 1U].native_ready;
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
static bool Stop(SotnCdAudio *a, bool reset_master)
{
    SotnMusicState state = {0};
    switch (a->phase)
    {
    case 0:
        if (!a->playing)
        {
            if (reset_master)
                a->master_volume = a->master_target = 127;
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
        if (reset_master)
            a->master_volume = a->master_target = 127;
        Complete(a, false);
        break;
    default:
        return SDL_SetError("Native XA stop has an unsupported original command phase");
    }
    return true;
}
static bool Fade(SotnCdAudio *a)
{
    switch (a->phase)
    {
    case 0:
        a->busy = true;
        a->fade_counter = 0;
        ++a->phase;
        break;
    case 1:
        a->fade_counter = Word(a->fade_counter + 1);
        if (a->volume)
            a->volume = (Uint8)SDL_max(
                0, a->target_volume -
                       ((Sint32)a->target_volume * a->fade_multiplier * a->fade_counter) / 512);
        if (!a->volume)
            ++a->phase;
        break;
    case 2:
        Add(a, 2);
        a->fade_busy = false;
        Complete(a, false);
        break;
    default:
        return SDL_SetError("Native XA volume fade lost its original command8 phase");
    }
    return true;
}
static bool FadeMaster(SotnCdAudio *a)
{
    switch (a->phase)
    {
    case 0:
        a->busy = true;
        a->fade_counter = 0;
        ++a->phase;
        break;
    case 1:
        a->fade_counter = Word(a->fade_counter + 1);
        a->master_volume =
            a->master_volume > 0
                ? Word(a->master_target -
                       ((Sint32)a->master_target * a->fade_multiplier * a->fade_counter) / 512)
                : 0;
        if (a->master_volume < 0)
            a->master_volume = 0;
        if (!a->master_volume)
            ++a->phase;
        break;
    case 2:
        if (!SotnSound_SetReverbDepth(a->sound, 0, 0))
            return false;
        SotnSound_StopSequence(a->sound);
        Add(a, 3); // CD_SOUND_COMMAND_FADE_OUT_1's actual enum value.
        a->release_counter = 3;
        ++a->phase;
        break;
    case 3:
        --a->release_counter;
        if (!a->release_counter)
        {
            SoundReleaseAll(a->sound);
            a->fade_busy = false;
            Complete(a, false);
        }
        break;
    default:
        return SDL_SetError("Native master fade lost its original command10 phase");
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
    a->fade_multiplier = 32; // Original InitSoundVars1, before any queued multiplier change.
    a->master_volume = a->master_target = 127;
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
        a->acknowledge_busy || a->fade_busy || a->busy || a->serial)
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
        a->mode = state.mode;
        a->native_ready = true;
    }
    *music = NULL;
    return true;
}
bool SotnCdAudio_Queue(SotnCdAudio *a, Uint16 command)
{
    if (!a || (command != 0x10 && command != 0x11 && command != 0x12 && command != 0xa &&
               !(command >= 0x80 && command <= 0x84) && !(command >= 0x90 && command <= 0x94) &&
               !XaCommand(command) && !SpuCommand(command)))
        return SDL_SetError("Native sound ring requires selected SPU/CD commands or US XA301..532");
    if (((command >= 0x80 && command <= 0x84) || SpuCommand(command)) && !a->sound)
        return SDL_SetError("SPU commands require the actual configured sound owner");
    if (command == 0x10 || command == 0x11)
        a->acknowledge_busy = true;
    if ((command >= 0x80 && command <= 0x84) || (command >= 0x90 && command <= 0x94))
        a->fade_busy = true;
    a->incoming[a->write] = command;
    a->write = (Uint16)((a->write + 1U) & 255U);
    return true;
}
bool SotnCdAudio_CheckSoundBinding(const SotnCdAudio *a, const SotnSound *sound)
{
    if (!a || !sound || !sound->reverb.configured || a->dra.bytes != sound->dra.bytes ||
        a->dra.size != sound->dra.size)
        return SDL_SetError("CD reverb requires its actual configured SPU owner and shared DRA");
    return true;
}
bool SotnCdAudio_BindSound(SotnCdAudio *a, SotnSound *sound)
{
    if (!SotnCdAudio_CheckSoundBinding(a, sound))
        return false;
    a->sound = sound;
    return true;
}
bool SotnCdAudio_Mute(SotnCdAudio *a)
{
    if (!a || !a->sound || a->sound->sequence_open)
        return SDL_SetError("Sound mute requires its actual owner and closed SEQ");
    // MuteSound: serial off/volume zero then SetMaxVolume/InitSoundVars1. Retire
    // the real software transport, including a pending read, without advancing samples.
    a->serial = false;
    a->playing = 0;
    SotnMusic_Destroy(a->current);
    a->current = NULL;
    ForgetSaved(a);
    SoundResetStopped(a->sound);
    SDL_memset(a->incoming, 0, sizeof(a->incoming));
    SDL_memset(a->commands, 0, sizeof(a->commands));
    a->read = a->write = a->count = 0;
    a->phase = a->mode = 0;
    a->acknowledge_busy = a->fade_busy = a->busy = false;
    a->managed = a->native_ready = a->finite = false;
    a->fade_multiplier = 32;
    a->master_volume = a->master_target = 127;
    return true;
}
bool SotnCdAudio_Tick(SotnCdAudio *a, Uint32 clock, Sint16 *pcm, size_t count)
{
    if (!a || !pcm || count < SOTN_SOUND_FRAMES * 2)
        return SDL_SetError("Native CD tick requires its actual owner and full stereo PCM output");
    if (!SoundCommands(a))
        return false;
    // Original strict comparison and private mode-two twenty-tick tail, before CD dispatch.
    if (a->playing && !a->busy && clock - a->started > a->duration)
    {
        if (!a->managed)
        {
            if (a->finite)
                Add(a, 2);
        }
        else if (a->mode == 2)
            Add(a, 2);
        else if (a->mode == 1)
        {
            SotnMusicState state = {0};
            if (!SotnMusic_ContinueXa(a->current) || !Current(a, &state))
                return false;
            a->queued = a->playing;
            a->mode = state.mode;
            a->duration = state.duration;
            a->started = clock;
        }
        else
        {
            if (a->sound && !SotnSound_SetReverbDepth(a->sound, 127, 127))
                return false;
            a->queued = a->playing;
            Add(a, 6);
        }
    }
    if (a->count)
    {
        bool ok = true;
        switch (a->commands[0])
        {
        case 3:
        case 2:
            ok = Stop(a, a->commands[0] == 3);
            break;
        case 4:
            ok = Start(a, clock);
            break;
        case 6:
            ok = Loop(a, clock);
            break;
        case 8:
            ok = Fade(a);
            break;
        case 10:
            ok = FadeMaster(a);
            break;
        case 12:
            ok = Pause(a, clock);
            break;
        case 14:
            ok = Restore(a, clock);
            break;
        case 16:
            ForgetSaved(a);
            Advance(a);
            break;
        default:
            return SDL_SetError("Native CD queue lost its selected original command identity");
        }
        if (!ok)
            return false;
    }
    SDL_memset(pcm, 0, SOTN_SOUND_FRAMES * 2 * sizeof(*pcm));
    if (a->serial && a->current && (!a->managed || a->native_ready))
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
                              .fade_multiplier = a->fade_multiplier,
                              .fade_counter = a->fade_counter,
                              .master_volume = a->master_volume,
                              .master_target = a->master_target,
                              .release_counter = a->release_counter,
                              .phase = a->phase,
                              .saved = a->saved,
                              .volume = a->volume,
                              .target_volume = a->target_volume,
                              .mode = a->mode,
                              .acknowledged = !a->acknowledge_busy,
                              .queue_empty = !a->count,
                              .playing_xa = a->playing != 0,
                              .busy = a->busy,
                              .serial = a->serial,
                              .managed = a->managed,
                              .fade_acknowledged = !a->fade_busy,
                              .xa_active = a->fade_busy || a->playing != 0,
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
