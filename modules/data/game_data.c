#include "data_internal.h"
#include <SDL3/SDL.h>

#include "reference_data.h"

#define SOTN_DATA_BUDGET (16U * 1024U * 1024U)

struct SotnGameData
{
    Uint8 *arena;
    SotnDataView files[SOTN_GAME_FILE_COUNT];
};

bool SotnData_ReadVerified(SDL_IOStream *stream, Uint8 *buffer, const SotnDataSpec *spec)
{
    const Sint64 actual_size = SDL_GetIOSize(stream);
    if (actual_size < 0 || (Uint64)actual_size != (Uint64)spec->size)
    {
        return SDL_SetError("size mismatch (expected %zu bytes)", spec->size);
    }
    size_t offset = 0;
    while (offset < spec->size)
    {
        const size_t count = SDL_ReadIO(stream, buffer + offset, spec->size - offset);
        if (count == 0)
        {
            return SDL_SetError("short read or I/O failure at byte %zu", offset);
        }
        offset += count;
    }
    Uint8 extra = 0;
    if (SDL_ReadIO(stream, &extra, 1) != 0 || SDL_GetIOStatus(stream) != SDL_IO_STATUS_EOF)
    {
        return SDL_SetError("file changed size or failed while checking end of file");
    }
    char digest[41] = {0};
    SotnData_Sha1(buffer, spec->size, digest);
    if (SDL_strcmp(digest, spec->sha1) != 0)
    {
        return SDL_SetError("reference checksum mismatch; supply the unmodified US reference file");
    }
    return true;
}

bool SotnData_LoadFile(const char *directory, const SotnDataSpec *spec, Uint8 *buffer)
{
    char *path = NULL;
    if (SDL_asprintf(&path, "%s/%s", directory, spec->path) < 0)
    {
        return false;
    }
    SDL_PathInfo info = {0};
    if (!SDL_GetPathInfo(path, &info) || info.type != SDL_PATHTYPE_FILE)
    {
        SDL_free(path);
        return SDL_SetError("Cannot read game data '%s': expected a regular file", spec->path);
    }
    SDL_IOStream *stream = SDL_IOFromFile(path, "rb");
    SDL_free(path);
    if (stream == NULL)
    {
        char detail[512] = {0};
        SDL_strlcpy(detail, SDL_GetError(), sizeof(detail));
        return SDL_SetError("Cannot open game data '%s': %s", spec->path, detail);
    }
    const bool valid = SotnData_ReadVerified(stream, buffer, spec);
    char detail[512] = {0};
    SDL_strlcpy(detail, SDL_GetError(), sizeof(detail));
    const bool closed = SDL_CloseIO(stream);
    if (!valid)
    {
        return SDL_SetError("Cannot validate game data '%s': %s", spec->path, detail);
    }
    if (!closed)
    {
        return SDL_SetError("Cannot close game data '%s' after reading", spec->path);
    }
    return true;
}

bool SotnData_OpenReference(const char *directory, const SotnDataSpec specs[SOTN_GAME_FILE_COUNT],
                            SotnGameData **out)
{
    if (out == NULL)
    {
        return SDL_InvalidParamError("out");
    }
    *out = NULL;
    if (directory == NULL || directory[0] == '\0')
    {
        return SDL_InvalidParamError("directory");
    }
    SDL_PathInfo info = {0};
    if (!SDL_GetPathInfo(directory, &info) || info.type != SDL_PATHTYPE_DIRECTORY)
    {
        return SDL_SetError("Game data directory does not exist or is unreadable");
    }
    size_t capacity = 0;
    for (size_t i = 0; i < SOTN_GAME_FILE_COUNT; ++i)
    {
        if (specs[i].size == 0 || specs[i].size > SOTN_DATA_BUDGET - capacity)
        {
            return SDL_SetError("Core game data exceeds the 16 MiB allocation budget");
        }
        capacity += specs[i].size;
    }
    SotnGameData *data = SDL_calloc(1, sizeof(*data));
    if (data == NULL)
    {
        return false;
    }
    data->arena = SDL_malloc(capacity);
    if (data->arena == NULL)
    {
        SotnGameData_Close(data);
        return false;
    }
    size_t offset = 0;
    for (size_t i = 0; i < SOTN_GAME_FILE_COUNT; ++i)
    {
        if (!SotnData_LoadFile(directory, &specs[i], data->arena + offset))
        {
            SotnGameData_Close(data);
            return false;
        }
        data->files[i].bytes = data->arena + offset;
        data->files[i].size = specs[i].size;
        offset += specs[i].size;
    }
    *out = data;
    return true;
}

bool SotnGameData_Open(const char *directory, SotnGameData **out)
{
    return SotnData_OpenReference(directory, reference_files, out);
}

bool SotnGameData_GetFile(const SotnGameData *data, SotnGameFileId file, SotnDataView *out)
{
    if (out == NULL)
    {
        return SDL_InvalidParamError("out");
    }
    *out = (SotnDataView){0};
    if (data == NULL || (unsigned int)file >= (unsigned int)SOTN_GAME_FILE_COUNT)
    {
        return SDL_SetError("Invalid game data snapshot or file ID");
    }
    *out = data->files[file];
    return true;
}

void SotnGameData_Close(SotnGameData *data)
{
    if (data != NULL)
    {
        SDL_free(data->arena);
        SDL_free(data);
    }
}

const char *SotnGameData_ReferenceId(void)
{
    return SOTN_REFERENCE_ID;
}

const SotnDataSpec *SotnData_ResourceSpec(SotnResourceId id)
{
    if ((unsigned int)id >= (unsigned int)SOTN_RESOURCE_COUNT)
    {
        SDL_SetError("Unsupported game resource ID");
        return NULL;
    }
    return &reference_resources[id];
}

const SotnDataSpec *SotnData_MusicSpec(void)
{
    return &reference_music;
}
