#include "data_internal.h"
#include <SDL3/SDL.h>
#include <sotn/stream.h>

struct SotnStream
{
    SDL_IOStream *io;
    Uint32 sectors;
};

bool SotnData_OpenStreamSpec(const char *directory, const SotnDataSpec *spec, SotnStream **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (directory == NULL || directory[0] == '\0' || spec == NULL || spec->size == 0 ||
        spec->size > 1024U * 1024U * 1024U || spec->size % SOTN_CD_SECTOR_BYTES != 0)
        return SDL_SetError(
            "Music requires a private directory and bounded whole-sector reference");
    char *path = NULL;
    if (SDL_asprintf(&path, "%s/%s", directory, spec->path) < 0)
        return false;
    SDL_PathInfo info = {0};
    if (!SDL_GetPathInfo(path, &info) || info.type != SDL_PATHTYPE_FILE)
    {
        SDL_free(path);
        return SDL_SetError("Cannot read private music '%s': expected a regular file", spec->path);
    }
    SotnStream *stream = SDL_calloc(1, sizeof(*stream));
    if (stream == NULL)
    {
        SDL_free(path);
        return false;
    }
    stream->io = SDL_IOFromFile(path, "rb");
    SDL_free(path);
    if (stream->io == NULL || !SotnData_VerifyStream(stream->io, spec))
    {
        char detail[512] = {0};
        SDL_strlcpy(detail, SDL_GetError(), sizeof(detail));
        SotnStream_Close(stream);
        return SDL_SetError("Cannot verify private music '%s': %s", spec->path, detail);
    }
    stream->sectors = (Uint32)(spec->size / SOTN_CD_SECTOR_BYTES);
    *out = stream;
    return true;
}

bool SotnStream_OpenMusic(const char *directory, SotnStream **out)
{
    return SotnData_OpenStreamSpec(directory, SotnData_MusicSpec(), out);
}

Uint32 SotnStream_Sectors(const SotnStream *stream)
{
    return stream == NULL ? 0 : stream->sectors;
}

bool SotnStream_ReadSector(SotnStream *stream, Uint32 sector, Uint8 *output, size_t capacity)
{
    if (stream == NULL || output == NULL || capacity < SOTN_CD_SECTOR_BYTES ||
        sector >= stream->sectors)
        return SDL_SetError("Music sector read exceeds its verified file or output extent");
    const Sint64 offset = (Sint64)sector * SOTN_CD_SECTOR_BYTES;
    if (SDL_SeekIO(stream->io, offset, SDL_IO_SEEK_SET) != offset)
        return false;
    Uint8 scratch[SOTN_CD_SECTOR_BYTES] = {0};
    size_t read = 0;
    while (read < sizeof(scratch))
    {
        const size_t count = SDL_ReadIO(stream->io, scratch + read, sizeof(scratch) - read);
        if (count == 0)
            return SDL_SetError("Private music ended during sector %u", (unsigned int)sector);
        read += count;
    }
    SDL_memcpy(output, scratch, sizeof(scratch));
    return true;
}

void SotnStream_Close(SotnStream *stream)
{
    if (stream != NULL)
    {
        if (stream->io != NULL && !SDL_CloseIO(stream->io))
            SDL_LogWarn(SDL_LOG_CATEGORY_AUDIO, "Cannot close private music stream: %s",
                        SDL_GetError());
        SDL_free(stream);
    }
}
