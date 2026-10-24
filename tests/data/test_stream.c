#include "data_internal.h"
#include <SDL3/SDL.h>

#define CHECK(condition)                                                                           \
    do                                                                                             \
    {                                                                                              \
        if (!(condition))                                                                          \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #condition, SDL_GetError());                      \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

static bool Fingerprints(void)
{
    Uint8 *bytes = SDL_malloc(66001);
    CHECK(bytes != NULL);
    for (size_t i = 0; i < 66001; i++)
        bytes[i] = (Uint8)i;
    const size_t sizes[] = {0, 3, 55, 56, 63, 64, 65, 32768, 33001, 66001};
    for (size_t i = 0; i < SDL_arraysize(sizes); i++)
    {
        char digest[41] = {0};
        SotnData_Sha1(bytes, sizes[i], digest);
        const SotnDataSpec spec = {"synthetic", sizes[i], digest};
        SDL_IOStream *io = SDL_IOFromConstMem(bytes, sizes[i]);
        CHECK(io != NULL);
        CHECK(SotnData_VerifyStream(io, &spec));
        CHECK(SDL_TellIO(io) == 0);
        CHECK(SDL_CloseIO(io));
    }
    const SotnDataSpec wrong = {"synthetic", 3, "0000000000000000000000000000000000000000"};
    SDL_IOStream *io = SDL_IOFromConstMem(bytes, 3);
    CHECK(io != NULL);
    CHECK(!SotnData_VerifyStream(io, &wrong));
    CHECK(SDL_CloseIO(io));
    SDL_free(bytes);
    return true;
}

static bool Sectors(const char *directory)
{
    Uint8 bytes[SOTN_CD_SECTOR_BYTES * 2] = {0};
    for (size_t i = 0; i < sizeof(bytes); i++)
        bytes[i] = (Uint8)(i % 127);
    char digest[41] = {0};
    SotnData_Sha1(bytes, sizeof(bytes), digest);
    char *path = NULL;
    CHECK(SDL_asprintf(&path, "%s/music.txt", directory) > 0);
    CHECK(SDL_SaveFile(path, bytes, sizeof(bytes)));
    const SotnDataSpec spec = {"music.txt", sizeof(bytes), digest};
    SotnStream *stream = NULL;
    CHECK(SotnData_OpenStreamSpec(directory, &spec, &stream));
    CHECK(SotnStream_Sectors(stream) == 2);
    Uint8 output[SOTN_CD_SECTOR_BYTES] = {0};
    CHECK(SotnStream_ReadSector(stream, 1, output, sizeof(output)));
    CHECK(SDL_memcmp(output, bytes + SOTN_CD_SECTOR_BYTES, sizeof(output)) == 0);
    CHECK(!SotnStream_ReadSector(stream, 2, output, sizeof(output)));
    CHECK(SDL_memcmp(output, bytes + SOTN_CD_SECTOR_BYTES, sizeof(output)) == 0);
    CHECK(!SotnStream_ReadSector(stream, 0, output, 1));
    SotnStream_Close(stream);
    const SotnDataSpec invalid = {"music.txt", 3, digest};
    CHECK(!SotnData_OpenStreamSpec(directory, &invalid, &stream) && stream == NULL);
    CHECK(!SotnStream_OpenMusic(NULL, &stream) && stream == NULL);
    CHECK(SDL_RemovePath(path));
    SDL_free(path);
    return true;
}

int main(int argc, char **argv)
{
    if (argc != 2 || !Fingerprints() || !Sectors(argv[1]))
        return 1;
    SDL_Log("SOTN_STREAM_TEST: PASS");
    return 0;
}
