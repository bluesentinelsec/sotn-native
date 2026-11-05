// SPDX-License-Identifier: MPL-2.0
// XA ADPCM decoding adapted from PSY-Z b52003e55c0d1b20719e984dd19bd4d760eeb14a,
// psyz/src/psyz/libcd.c. Stream bounds, ownership, and SDL conversion are native services.
// Original track/filter/loop interpretation follows the pinned game's src/dra/sound.c.
#include <SDL3/SDL.h>
#include <sotn/music.h>

typedef struct MusicTrack
{
    Uint32 start, ticks;
    Uint16 id;
    Uint8 file, channel, volume, mode;
} MusicTrack;

struct SotnMusic
{
    SotnStream *stream;
    SDL_AudioStream *converter;
    MusicTrack intro, loop;
    Uint32 sector, remaining;
    Sint32 history[2][2];
    bool looping, once, continuous;
    Uint64 source_frames, frame_limit;
    bool flushed;
};

static Uint32 Read32(const Uint8 *p)
{
    return p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
}

static bool Track(SotnDataView dra, Uint32 id, Uint32 sectors, MusicTrack *out)
{
    const size_t offset = 0x1d224 + (size_t)id * 16;
    if (dra.bytes == NULL || offset > dra.size || dra.size - offset < 16)
        return SDL_SetError("Laboratory music metadata is outside private DRA");
    const Uint8 *record = dra.bytes + offset;
    const Uint32 start = Read32(record), duration = Read32(record + 4);
    if (start >= sectors || record[9] >= sectors - start || duration == 0 || duration > 432000 ||
        record[10] > 127 || Read32(record + 12) > 2)
        return SDL_SetError("Laboratory music start/duration/filter is invalid");
    *out = (MusicTrack){.start = start + record[9],
                        .ticks = duration,
                        .id = (Uint16)id,
                        .file = record[8],
                        .channel = (Uint8)(record[9] % 16),
                        .volume = record[10],
                        .mode = (Uint8)Read32(record + 12)};
    return true;
}

static Sint16 Clamp(Sint32 sample)
{
    return (Sint16)SDL_clamp(sample, -32768, 32767);
}

static void Decode28(const Uint8 *block, Uint32 sub, Uint32 nibble, Sint32 history[2], Sint16 *pcm)
{
    const Uint8 header = block[4 + sub * 2 + nibble];
    const Uint32 shift = (header & 15) > 12 ? 9 : 12 - (header & 15);
    const Sint32 positive[4] = {0, 60, 115, 98}, negative[4] = {0, 0, -52, -55};
    const Uint32 filter = (header >> 4) & 3;
    for (Uint32 j = 0; j < 28; ++j)
    {
        const Uint32 bits = (block[16 + sub + j * 4] >> (nibble * 4)) & 15;
        const Sint32 signed_value = bits < 8 ? (Sint32)bits : (Sint32)bits - 16;
        const Sint64 prediction =
            ((Sint64)history[0] * positive[filter] + (Sint64)history[1] * negative[filter] + 32) /
            64;
        const Sint64 decoded = signed_value * (1 << shift) + prediction;
        const Sint32 sample = (Sint32)SDL_clamp(decoded, SDL_MIN_SINT32, SDL_MAX_SINT32);
        pcm[j * 2] = Clamp(sample);
        history[1] = history[0];
        history[0] = sample;
    }
}

static bool Feed(SotnMusic *music)
{
    Uint8 raw[SOTN_CD_SECTOR_BYTES] = {0};
    const MusicTrack track = music->looping ? music->loop : music->intro;
    for (Uint32 scanned = 0; scanned < 128; ++scanned)
    {
        if (music->sector >= SotnStream_Sectors(music->stream) ||
            !SotnStream_ReadSector(music->stream, music->sector++, raw, sizeof(raw)))
            return SDL_SetError("Private XA music ended before its declared track duration");
        if (raw[15] != 2 || (raw[18] & 0x44) != 0x44 || raw[16] != track.file ||
            raw[17] != track.channel)
            continue;
        if (SDL_memcmp(raw + 16, raw + 20, 4) != 0 || raw[19] != 1)
            return SDL_SetError(
                "Laboratory music requires an intact 4-bit stereo 37.8 kHz XA sector");
        Sint16 pcm[2016 * 2] = {0};
        Uint32 frame = 0;
        for (Uint32 group = 0; group < 18; ++group)
        {
            const Uint8 *block = raw + 24 + group * 128;
            for (Uint32 sub = 0; sub < 4; ++sub)
            {
                Decode28(block, sub, 0, music->history[0], pcm + frame * 2);
                Decode28(block, sub, 1, music->history[1], pcm + frame * 2 + 1);
                frame += 28;
            }
        }
        if (!SDL_PutAudioStreamData(music->converter, pcm, sizeof(pcm)))
            return false;
        music->source_frames += 2016;
        const Uint64 limit = music->once ? (Uint64)music->intro.ticks * 630 : music->frame_limit;
        if (limit && music->source_frames >= limit)
        {
            /* Release SDL's resampling tail at the end of a finite XA clip. Otherwise a
             * clip ending exactly on a sector boundary requests another, unrelated sector. */
            if (!SDL_FlushAudioStream(music->converter))
                return false;
            music->flushed = true;
        }
        return true;
    }
    return SDL_SetError("No matching Laboratory music sector within the bounded scan window");
}

static bool Create(SotnDataView dra, SotnStream *stream, Uint32 intro, Uint32 loop, bool once,
                   SotnMusic **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    SotnMusic *music = SDL_calloc(1, sizeof(*music));
    if (music == NULL)
        return false;
    music->stream = stream;
    music->once = once;
    if (!Track(dra, intro, SotnStream_Sectors(stream), &music->intro) ||
        !Track(dra, loop, SotnStream_Sectors(stream), &music->loop))
    {
        SotnMusic_Destroy(music);
        return false;
    }
    const SDL_AudioSpec input = {.format = SDL_AUDIO_S16, .channels = 2, .freq = 37800};
    const SDL_AudioSpec output = {.format = SDL_AUDIO_S16, .channels = 2, .freq = 44100};
    music->converter = SDL_CreateAudioStream(&input, &output);
    if (music->converter == NULL)
    {
        SotnMusic_Destroy(music);
        return false;
    }
    music->sector = music->intro.start;
    music->remaining = music->intro.ticks;
    *out = music;
    return true;
}

bool SotnMusic_CreateXa(SotnDataView dra, SotnStream *stream, Uint16 id, SotnMusic **out)
{
    if (!out)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (id <= 0x300 || id >= 0x533)
        return SDL_SetError("XA command requires original US sound identity301..532");
    MusicTrack track = {0};
    if (!Track(dra, id - 0x300U, SotnStream_Sectors(stream), &track))
        return false;
    const bool once = track.mode == 2;
    if (!once && id == 0x532)
        return SDL_SetError("XA loop record exceeds the original US table");
    SotnMusic *next = NULL;
    if (!Create(dra, stream, id - 0x300U, id - 0x300U + (once ? 0U : 1U), once, &next))
        return false;
    if (!once && (next->intro.file != next->loop.file || next->intro.channel != next->loop.channel))
    {
        SotnMusic_Destroy(next);
        return SDL_SetError("XA continuation changed the actual configured file/channel");
    }
    next->continuous = !once;
    if (!once)
        next->frame_limit =
            ((Uint64)next->intro.ticks + (track.mode == 1 ? next->loop.ticks : 0U)) * 630;
    *out = next;
    return true;
}
bool SotnMusic_ContinueXa(SotnMusic *music)
{
    if (!music || !music->continuous ||
        (music->looping ? music->loop.mode : music->intro.mode) != 1)
        return SDL_SetError("XA metadata continuation requires its actual mode1 command transport");
    music->looping = true;
    music->remaining = music->loop.ticks;
    return true;
}
bool SotnMusic_PrepareXaLoop(const SotnMusic *music, SotnMusic **out)
{
    if (!out)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (!music || !music->continuous)
        return SDL_SetError("XA loop seek requires its actual continuous command transport");
    SotnMusic *next = SDL_calloc(1, sizeof(*next));
    if (!next)
        return false;
    *next = (SotnMusic){.stream = music->stream,
                        .intro = music->intro,
                        .loop = music->loop,
                        .sector = music->loop.start,
                        .remaining = music->loop.ticks,
                        .frame_limit = (Uint64)music->loop.ticks * 630,
                        .looping = true,
                        .continuous = true};
    const SDL_AudioSpec input = {.format = SDL_AUDIO_S16, .channels = 2, .freq = 37800};
    const SDL_AudioSpec output = {.format = SDL_AUDIO_S16, .channels = 2, .freq = 44100};
    next->converter = SDL_CreateAudioStream(&input, &output);
    if (!next->converter)
    {
        SotnMusic_Destroy(next);
        return false;
    }
    *out = next;
    return true;
}

bool SotnMusic_Create(SotnDataView dra, SotnStream *stream, SotnMusic **out)
{
    return Create(dra, stream, 46, 47, false, out);
}

bool SotnMusic_CreateGameOver(SotnDataView dra, SotnStream *stream, SotnMusic **out)
{
    return Create(dra, stream, 62, 62, true, out);
}

bool SotnMusic_Tick(SotnMusic *music, Sint16 *pcm, size_t count)
{
    if (music == NULL || pcm == NULL || count < SOTN_SOUND_FRAMES * 2)
        return SDL_SetError("Music tick requires playback state and a full stereo PCM buffer");
    if (music->remaining == 0 && music->once)
    {
        SDL_memset(pcm, 0, SOTN_SOUND_FRAMES * 2 * sizeof(*pcm));
        return true;
    }
    if (music->remaining == 0 && !music->continuous)
    {
        music->looping = true;
        music->remaining = music->loop.ticks;
        music->sector = music->loop.start;
        SDL_memset(music->history, 0, sizeof(music->history));
        if (!SDL_ClearAudioStream(music->converter))
            return false;
    }
    const int bytes = SOTN_SOUND_FRAMES * 2 * (int)sizeof(*pcm);
    int available = SDL_GetAudioStreamAvailable(music->converter);
    for (Uint32 fed = 0; available >= 0 && available < bytes && fed < 2 && !music->flushed; ++fed)
    {
        if (!Feed(music))
            return false;
        available = SDL_GetAudioStreamAvailable(music->converter);
    }
    int read_bytes = bytes;
    if (music->continuous && music->flushed && available >= 0 && available < bytes)
    {
        // Real decoded PCM can end before the original strict command deadline. Release
        // the resampler tail and hold silence until actual command6 replaces this transport;
        // never scan unrelated sectors or change the command/playing state to fill a tick.
        if (available % 4)
            return SDL_SetError("XA command transport ended with an incomplete stereo frame");
        SDL_memset(pcm, 0, (size_t)bytes);
        read_bytes = available;
    }
    if (available < read_bytes || available < 0 ||
        (read_bytes && SDL_GetAudioStreamData(music->converter, pcm, read_bytes) != read_bytes))
        return SDL_SetError("Music converter could not produce a complete simulation tick");
    const Uint8 volume =
        music->looping && !music->continuous ? music->loop.volume : music->intro.volume;
    for (Uint32 i = 0; i < SOTN_SOUND_FRAMES * 2; ++i)
        pcm[i] = (Sint16)((Sint32)pcm[i] * volume / 127);
    if (music->remaining)
        --music->remaining;
    return true;
}

void SotnMusic_Destroy(SotnMusic *music)
{
    if (music != NULL)
    {
        SDL_DestroyAudioStream(music->converter);
        SDL_free(music);
    }
}

bool SotnMusic_CreateBoss(SotnDataView dra, SotnStream *stream, SotnMusic **out)
{
    return Create(dra, stream, 29, 30, false, out);
}

bool SotnMusic_CreateEntrance(SotnDataView dra, SotnStream *stream, bool castle, SotnMusic **out)
{
    return Create(dra, stream, castle ? 15 : 42, castle ? 16 : 43, false, out);
}

bool SotnMusic_CreateDeathSpeech(SotnDataView dra, SotnStream *stream, Uint16 id, SotnMusic **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (id < 0x35f || id > 0x363)
        return SDL_SetError("Opening Death speech requires an original dialogue ID 0x35f..0x363");
    return Create(dra, stream, id - 0x300U, id - 0x300U, true, out);
}

bool SotnMusic_CreateSpeech(SotnDataView dra, SotnStream *stream, Uint16 id, SotnMusic **out)
{
    if ((id >= 0x369 && id <= 0x379) || (id >= 0x3a3 && id <= 0x3a8) || id == 0x3a9 ||
        id == 0x3ad || id == 0x3b2 || id == 0x3b9 || id == 0x472 || id == 0x473 || id == 0x476 ||
        id == 0x479 || id == 0x47a || id == 0x48a || id == 0x48e || id == 0x492)
        return Create(dra, stream, id - 0x300U, id - 0x300U, true, out);
    return SotnMusic_CreateDeathSpeech(dra, stream, id, out);
}

bool SotnMusic_Playing(const SotnMusic *music)
{
    return music != NULL && (!music->once || music->remaining != 0);
}

bool SotnMusic_Get(const SotnMusic *music, SotnMusicState *out)
{
    if (!music || !out)
        return SDL_SetError("XA cursor read requires actual playback and an output");
    const MusicTrack track = music->looping ? music->loop : music->intro;
    *out = (SotnMusicState){.sector = music->sector,
                            .remaining = music->remaining,
                            .duration = track.ticks,
                            .start = track.start,
                            .id = track.id,
                            .volume = music->continuous ? music->intro.volume : track.volume,
                            .mode = track.mode,
                            .looping = music->looping,
                            .once = music->once};
    return true;
}

bool SotnMusic_Prepare(SotnMusic *music)
{
    if (!music)
        return SDL_SetError("XA read requires actual native playback");
    return music->source_frames || Feed(music);
}

bool SotnMusic_CreateEntranceQuiet(SotnDataView dra, SotnStream *stream, SotnMusic **out)
{
    return Create(dra, stream, 44, 45, false, out);
}

bool SotnMusic_CreateStage(SotnDataView dra, SotnStream *stream, Uint16 stage, SotnMusic **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (stage != 0 && stage != 1 && stage != 2 && stage != 4 && stage != 6 && stage != 25 &&
        stage != 30)
        return SDL_SetError("Stage music requires an integrated original area profile");
    return Create(dra, stream,
                  stage == 0                     ? 13
                  : stage == 1                   ? 19
                  : stage == 4                   ? 23
                  : stage == 6                   ? 5
                  : (stage == 25 || stage == 30) ? 52
                                                 : 9,
                  stage == 0                     ? 14
                  : stage == 1                   ? 20
                  : stage == 4                   ? 24
                  : stage == 6                   ? 6
                  : (stage == 25 || stage == 30) ? 53
                                                 : 10,
                  false, out);
}
