#ifndef SOTN_DATA_INTERNAL_H
#define SOTN_DATA_INTERNAL_H

#include <SDL3/SDL_iostream.h>
#include <sotn/game_data.h>
#include <sotn/resource.h>
#include <sotn/stream.h>

/* Internal seams allow synthetic tests to exercise the production loader. */
typedef struct SotnDataSpec
{
    const char *path;
    size_t size;
    const char *sha1;
} SotnDataSpec;

bool SotnData_OpenReference(const char *directory, const SotnDataSpec specs[SOTN_GAME_FILE_COUNT],
                            SotnGameData **out);
bool SotnData_ReadVerified(SDL_IOStream *stream, Uint8 *buffer, const SotnDataSpec *spec);
void SotnData_Sha1(const Uint8 *bytes, size_t size, char output[41]);
bool SotnData_LoadFile(const char *directory, const SotnDataSpec *spec, Uint8 *buffer);
const SotnDataSpec *SotnData_ResourceSpec(SotnResourceId id);
bool SotnData_OpenResourceSpec(const char *directory, const SotnDataSpec *spec, SotnResource **out);
bool SotnData_VerifyStream(SDL_IOStream *stream, const SotnDataSpec *spec);
const SotnDataSpec *SotnData_MusicSpec(void);
bool SotnData_OpenStreamSpec(const char *directory, const SotnDataSpec *spec, SotnStream **out);

#endif
