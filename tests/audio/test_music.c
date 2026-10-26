#include "data_internal.h"
#include <SDL3/SDL.h>
#include <sotn/music.h>
#include <sotn/playback.h>

#define CHECK(condition)                                                                           \
    do                                                                                             \
    {                                                                                              \
        if (!(condition))                                                                          \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #condition, SDL_GetError());                      \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

static void Put32(Uint8 *p, size_t offset, Uint32 value)
{
    for (Uint32 i = 0; i < 4; i++)
        p[offset + i] = (Uint8)(value >> (i * 8));
}

static bool Playback(const char *directory)
{
    Uint8 *dra = SDL_calloc(1, 0x20000);
    CHECK(dra != NULL);
    for (Uint32 id = 46; id <= 47; id++)
    {
        const size_t o = 0x1d224 + id * 16;
        Put32(dra, o, id - 46);
        Put32(dra, o + 4, id == 46 ? 2 : 3);
        dra[o + 8] = 1;
        dra[o + 10] = 127;
        Put32(dra, o + 12, id == 46 ? 1 : 0);
    }
    for (Uint32 id = 29; id <= 30; ++id)
        SDL_memcpy(dra + 0x1d224 + id * 16, dra + 0x1d224 + (id + 17) * 16, 16);
    const Uint32 stage_tracks[] = {13, 14, 19, 20, 9, 10, 5, 6, 52, 53, 23, 24};
    for (Uint32 i = 0; i < SDL_arraysize(stage_tracks); ++i)
        SDL_memcpy(dra + 0x1d224 + stage_tracks[i] * 16, dra + 0x1d224 + (46 + (i & 1)) * 16, 16);
    Uint8 *raw = SDL_calloc(2, SOTN_CD_SECTOR_BYTES);
    CHECK(raw != NULL);
    for (Uint32 i = 0; i < 2; i++)
    {
        Uint8 *s = raw + i * SOTN_CD_SECTOR_BYTES;
        s[15] = 2;
        s[16] = 1;
        s[18] = 0x64;
        s[19] = 1;
        SDL_memcpy(s + 20, s + 16, 4);
        for (Uint32 group = 0; group < 18; group++)
        {
            Uint8 *b = s + 24 + group * 128;
            for (Uint32 j = 4; j < 12; j++)
                b[j] = 4;
            for (Uint32 j = 16; j < 128; j++)
                b[j] = (Uint8)(i == 0 ? 0x11 : 0x33);
        }
    }
    char digest[41] = {0};
    SotnData_Sha1(raw, SOTN_CD_SECTOR_BYTES * 2, digest);
    char *path = NULL;
    CHECK(SDL_asprintf(&path, "%s/music.txt", directory) > 0);
    CHECK(SDL_SaveFile(path, raw, SOTN_CD_SECTOR_BYTES * 2));
    SotnDataSpec spec = {"music.txt", SOTN_CD_SECTOR_BYTES * 2, digest};
    SotnStream *stream = NULL;
    CHECK(SotnData_OpenStreamSpec(directory, &spec, &stream));
    SotnMusic *music = NULL;
    const SotnDataView image = {dra, 0x20000};
    CHECK(SotnMusic_Create(image, stream, &music));
    Sint16 pcm[SOTN_SOUND_FRAMES * 2] = {0};
    const Uint16 stages[] = {0, 1, 2, 4, 6, 25};
    for (Uint16 i = 0; i < SDL_arraysize(stages); ++i)
    {
        SotnMusic *selected = NULL;
        CHECK(SotnMusic_CreateStage(image, stream, stages[i], &selected));
        CHECK(SotnMusic_Tick(selected, pcm, SDL_arraysize(pcm)));
        CHECK(SotnMusic_Playing(selected));
        SotnMusic_Destroy(selected);
    }
    SotnMusic *invalid = NULL;
    CHECK(!SotnMusic_CreateStage(image, stream, 3, &invalid) && invalid == NULL);

    Sint64 intro = 0, loop = 0;
    for (Uint32 tick = 0; tick < 12; tick++)
    {
        CHECK(SotnMusic_Tick(music, pcm, SDL_arraysize(pcm)));
        for (Uint32 i = 0; i < SDL_arraysize(pcm); i++)
        {
            if (tick == 1)
                intro += pcm[i];
            if (tick == 3)
                loop += pcm[i];
        }
    }
    CHECK(intro > 0 && loop > intro * 2);
    CHECK(!SotnMusic_Tick(music, pcm, 1));
    SotnMusic_Destroy(music);
    CHECK(SotnMusic_CreateBoss(image, stream, &music));
    for (Uint32 i = 0; i < 6; ++i)
        CHECK(SotnMusic_Tick(music, pcm, SDL_arraysize(pcm)));
    SotnMusic_Destroy(music);
    SotnStream_Close(stream);
    CHECK(!SotnMusic_Create((SotnDataView){0}, NULL, &music) && music == NULL);
    CHECK(SotnData_OpenStreamSpec(directory, &spec, &stream));
    const size_t game_over = 0x1d224 + 62 * 16;
    Put32(dra, game_over, 0);
    Put32(dra, game_over + 4, 2);
    dra[game_over + 8] = 1;
    dra[game_over + 10] = 127;
    Put32(dra, game_over + 12, 2);
    SotnPlayback *playback = NULL;
    CHECK(SotnPlayback_Create(&playback));
    CHECK(SotnMusic_CreateGameOver(image, stream, &music));
    for (Uint32 tick = 0; tick < 5; ++tick)
    {
        CHECK(SotnMusic_Tick(music, pcm, SDL_arraysize(pcm)));
        CHECK(SotnPlayback_Queue(playback, pcm, SDL_arraysize(pcm)));
        if (tick >= 2)
            for (Uint32 i = 0; i < SDL_arraysize(pcm); ++i)
                CHECK(pcm[i] == 0);
    }
    SotnMusic_Destroy(music);
    SotnPlayback_Destroy(playback);
    SotnStream_Close(stream);
    Uint8 *finite = SDL_calloc(5, SOTN_CD_SECTOR_BYTES);
    CHECK(finite != NULL);
    for (Uint32 i = 0; i < 5; ++i)
        SDL_memcpy(finite + i * SOTN_CD_SECTOR_BYTES, raw, SOTN_CD_SECTOR_BYTES);
    char finite_hash[41] = {0};
    SotnData_Sha1(finite, 5 * SOTN_CD_SECTOR_BYTES, finite_hash);
    CHECK(SDL_SaveFile(path, finite, 5 * SOTN_CD_SECTOR_BYTES));
    const SotnDataSpec finite_spec = {"music.txt", 5 * SOTN_CD_SECTOR_BYTES, finite_hash};
    CHECK(SotnData_OpenStreamSpec(directory, &finite_spec, &stream));
    const size_t speech = 0x1d224 + 95 * 16;
    SDL_memcpy(dra + speech, dra + game_over, 16);
    Put32(dra, speech + 4, 16); // Exactly five XA sectors; SDL must flush its resampling tail.
    CHECK(SotnMusic_CreateDeathSpeech(image, stream, 0x35f, &music));
    CHECK(SotnMusic_Playing(music) && !SotnMusic_Playing(NULL));
    for (Uint32 tick = 0; tick < 16; ++tick)
        CHECK(SotnMusic_Tick(music, pcm, SDL_arraysize(pcm)));
    CHECK(!SotnMusic_Playing(music));
    CHECK(SotnMusic_Tick(music, pcm, SDL_arraysize(pcm)));
    for (Uint32 i = 0; i < SDL_arraysize(pcm); ++i)
        CHECK(pcm[i] == 0);
    SotnMusic_Destroy(music);
    CHECK(!SotnMusic_CreateDeathSpeech(image, stream, 0x35e, &music) && music == NULL);
    CHECK(!SotnMusic_CreateDeathSpeech(image, stream, 0x364, &music) && music == NULL);
    for (Uint16 id = 0x369; id <= 0x374; ++id)
    {
        SDL_memcpy(dra + 0x1d224 + (id - 0x300U) * 16, dra + speech, 16);
        CHECK(SotnMusic_CreateSpeech(image, stream, id, &music));
        CHECK(SotnMusic_Tick(music, pcm, SDL_arraysize(pcm)));
        SotnMusic_Destroy(music);
        music = NULL;
    }
    const Uint16 library_speech[] = {0x3a3, 0x3a4, 0x3a5, 0x3a6, 0x3a7,
                                     0x3a8, 0x3a9, 0x3ad, 0x3b2, 0x3b9};
    for (Uint32 i = 0; i < SDL_arraysize(library_speech); ++i)
    {
        const Uint16 id = library_speech[i];
        SDL_memcpy(dra + 0x1d224 + (id - 0x300U) * 16, dra + speech, 16);
        CHECK(SotnMusic_CreateSpeech(image, stream, id, &music));
        CHECK(SotnMusic_Tick(music, pcm, SDL_arraysize(pcm)));
        SotnMusic_Destroy(music);
        music = NULL;
    }
    CHECK(!SotnMusic_CreateSpeech(image, stream, 0x3a2, &music) && music == NULL);
    CHECK(!SotnMusic_CreateSpeech(image, stream, 0x3aa, &music) && music == NULL);
    CHECK(!SotnMusic_CreateSpeech(image, stream, 0x368, &music) && music == NULL);
    for (Uint16 id = 0x375; id <= 0x379; ++id)
    {
        SDL_memcpy(dra + 0x1d224 + (id - 0x300U) * 16, dra + speech, 16);
        CHECK(SotnMusic_CreateSpeech(image, stream, id, &music));
        for (Uint16 tick = 0; tick < 16; ++tick)
            CHECK(SotnMusic_Tick(music, pcm, SDL_arraysize(pcm)));
        CHECK(!SotnMusic_Playing(music));
        SotnMusic_Destroy(music);
        music = NULL;
    }
    CHECK(!SotnMusic_CreateSpeech(image, stream, 0x37a, &music) && music == NULL);
    SotnStream_Close(stream);
    SDL_free(finite);
    raw[19] = raw[23] = 0;
    SotnData_Sha1(raw, SOTN_CD_SECTOR_BYTES * 2, digest);
    CHECK(SDL_SaveFile(path, raw, SOTN_CD_SECTOR_BYTES * 2));
    CHECK(SotnData_OpenStreamSpec(directory, &spec, &stream));
    CHECK(SotnMusic_Create(image, stream, &music));
    CHECK(!SotnMusic_Tick(music, pcm, SDL_arraysize(pcm)));
    SotnMusic_Destroy(music);
    SotnStream_Close(stream);
    CHECK(SDL_RemovePath(path));
    SDL_free(path);
    SDL_free(raw);
    SDL_free(dra);
    return true;
}

int main(int argc, char **argv)
{
    if (argc != 2 || !Playback(argv[1]))
        return 1;
    SDL_Log("SOTN_MUSIC_TEST: PASS");
    return 0;
}
