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
