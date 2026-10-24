#include "data_internal.h"
#include <SDL3/SDL.h>

struct SotnResource
{
    Uint8 *arena;
    size_t size;
};

bool SotnData_OpenResourceSpec(const char *directory, const SotnDataSpec *spec, SotnResource **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (directory == NULL || directory[0] == '\0' || spec == NULL || spec->size == 0 ||
        spec->size > 16U * 1024U * 1024U)
        return SDL_SetError(
            "Game resource requires a directory and a reference within the 16 MiB budget");
    SotnResource *resource = SDL_calloc(1, sizeof(*resource));
    if (resource == NULL)
        return false;
    resource->arena = SDL_malloc(spec->size);
    if (resource->arena == NULL || !SotnData_LoadFile(directory, spec, resource->arena))
    {
        SotnResource_Close(resource);
        return false;
    }
    resource->size = spec->size;
    *out = resource;
    return true;
}

bool SotnResource_Open(const char *directory, SotnResourceId id, SotnResource **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    const SotnDataSpec *spec = SotnData_ResourceSpec(id);
    return spec != NULL && SotnData_OpenResourceSpec(directory, spec, out);
}

SotnDataView SotnResource_View(const SotnResource *resource)
{
    return resource == NULL ? (SotnDataView){0} : (SotnDataView){resource->arena, resource->size};
}

void SotnResource_Close(SotnResource *resource)
{
    if (resource != NULL)
    {
        SDL_free(resource->arena);
        SDL_free(resource);
    }
}
