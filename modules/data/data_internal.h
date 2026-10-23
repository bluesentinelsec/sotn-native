#ifndef SOTN_DATA_INTERNAL_H
#define SOTN_DATA_INTERNAL_H

#include <SDL3/SDL_iostream.h>
#include <sotn/game_data.h>

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

#endif
