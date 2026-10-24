#include "save_internal.h"

struct SotnSaveStore
{
    char *directory;
};

static bool Path(const SotnSaveStore *store, Uint32 slot, char **out)
{
    if (store == NULL || slot == 0 || slot > SOTN_SAVE_SLOTS)
        return SDL_SetError("Native save slot must be between 1 and %d", SOTN_SAVE_SLOTS);
    return SDL_asprintf(out, "%s/laboratory-%02u.sav", store->directory, slot) >= 0;
}

bool SotnSaveStore_OpenAt(const char *directory, SotnSaveStore **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    SDL_PathInfo info = {0};
    if (directory == NULL || !SDL_GetPathInfo(directory, &info) ||
        info.type != SDL_PATHTYPE_DIRECTORY)
        return SDL_SetError("Cannot open the native save directory");
    SotnSaveStore *store = SDL_calloc(1, sizeof(*store));
    if (store == NULL)
        return false;
    store->directory = SDL_strdup(directory);
    if (store->directory == NULL)
    {
        SDL_free(store);
        return false;
    }
    *out = store;
    return true;
}

bool SotnSaveStore_Create(SotnSaveStore **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
#ifdef SDL_PLATFORM_EMSCRIPTEN
    return SDL_SetError(
        "Browser save persistence requires the unintegrated persistent-storage adapter");
#else
    char *directory = SDL_GetPrefPath("bluesentinelsec", "sotn-native");
    if (directory == NULL)
        return false;
    const bool result = SotnSaveStore_OpenAt(directory, out);
    SDL_free(directory);
    return result;
#endif
}

typedef struct FindSlot
{
    const char *name;
    bool found;
} FindSlot;

static SDL_EnumerationResult SDLCALL Find(void *userdata, const char *directory, const char *name)
{
    FindSlot *find = userdata;
    (void)directory;
#ifdef SDL_PLATFORM_WINDOWS
    find->found = SDL_strcasecmp(find->name, name) == 0;
#else
    find->found = SDL_strcmp(find->name, name) == 0;
#endif
    return find->found ? SDL_ENUM_SUCCESS : SDL_ENUM_CONTINUE;
}

bool SotnSaveStore_Exists(const SotnSaveStore *store, Uint32 slot, bool *out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    if (store == NULL || slot == 0 || slot > SOTN_SAVE_SLOTS)
        return SDL_SetError("Native save slot must be between 1 and %d", SOTN_SAVE_SLOTS);
    char name[32] = {0};
    SDL_snprintf(name, sizeof(name), "laboratory-%02u.sav", slot);
    FindSlot find = {.name = name};
    const bool result = SDL_EnumerateDirectory(store->directory, Find, &find);
    if (result)
        *out = find.found;
    return result;
}

bool SotnSaveStore_Read(const SotnSaveStore *store, Uint32 slot, SotnSaveState *out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    char *path = NULL;
    if (!Path(store, slot, &path))
        return false;
    SDL_PathInfo info = {0};
    if (!SDL_GetPathInfo(path, &info) || info.type != SDL_PATHTYPE_FILE ||
        (info.size != SOTN_SAVE_BYTES && info.size != SOTN_SAVE_V3_BYTES &&
         info.size != SOTN_SAVE_LEGACY_BYTES))
    {
        SDL_free(path);
        return SDL_SetError(
            "Cannot load Laboratory slot %u: expected a regular %d-, %d-, or %d-byte save", slot,
            SOTN_SAVE_BYTES, SOTN_SAVE_V3_BYTES, SOTN_SAVE_LEGACY_BYTES);
    }
    SDL_IOStream *io = SDL_IOFromFile(path, "rb");
    SDL_free(path);
    if (io == NULL)
        return false;
    Uint8 bytes[SOTN_SAVE_BYTES] = {0}, extra = 0;
    const bool read = SDL_ReadIO(io, bytes, (size_t)info.size) == (size_t)info.size &&
                      SDL_ReadIO(io, &extra, 1) == 0 && SDL_GetIOStatus(io) == SDL_IO_STATUS_EOF;
    const bool closed = SDL_CloseIO(io);
    if (!read || !closed)
        return SDL_SetError("Cannot finish reading Laboratory slot %u", slot);
    return SotnSave_Decode(bytes, (size_t)info.size, out);
}

bool SotnSaveStore_Write(SotnSaveStore *store, Uint32 slot, SotnSaveState state)
{
    Uint8 bytes[SOTN_SAVE_BYTES] = {0};
    char *path = NULL, *temporary = NULL;
    if (!SotnSave_Encode(state, bytes, sizeof(bytes)) || !Path(store, slot, &path))
        return false;
    if (SDL_asprintf(&temporary, "%s.temporary-%" SDL_PRIu64, path, SDL_GetTicksNS()) < 0)
    {
        SDL_free(path);
        return false;
    }
    SDL_IOStream *io = SDL_IOFromFile(temporary, "wbx");
    bool success = false;
    if (io != NULL)
    {
        const bool written =
            SDL_WriteIO(io, bytes, sizeof(bytes)) == sizeof(bytes) && SDL_FlushIO(io);
        const bool closed = SDL_CloseIO(io);
        success = written && closed && SDL_RenamePath(temporary, path);
        if (!success)
        {
            char detail[512] = {0};
            SDL_strlcpy(detail, SDL_GetError(), sizeof(detail));
            (void)SDL_RemovePath(temporary);
            SDL_SetError("Cannot save Laboratory slot %u: %s; previous slot retained", slot,
                         detail);
        }
    }
    SDL_free(temporary);
    SDL_free(path);
    return success;
}

void SotnSaveStore_Destroy(SotnSaveStore *store)
{
    if (store != NULL)
    {
        SDL_free(store->directory);
        SDL_free(store);
    }
}
