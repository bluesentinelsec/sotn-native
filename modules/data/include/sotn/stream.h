#ifndef SOTN_STREAM_H
#define SOTN_STREAM_H

#include <sotn/game_data.h>

/** @brief Raw CD sector size used by the supported private XA container. */
enum
{
    SOTN_CD_SECTOR_BYTES = 2352
};
/** @brief Owned read-only verified music stream; keeps no entire-file memory allocation. */
typedef struct SotnStream SotnStream;

/** @brief Open and fingerprint the supported US music container with bounded scratch storage.
 * @param directory Borrowed UTF-8 private game-data root.
 * @param out Receives ownership on success, NULL on failure when non-NULL.
 * @return Success, or false with SDL_GetError() for missing, modified, or invalid input.
 * @note Verifies the complete file once. Keep it unchanged while the stream is open. */
bool SotnStream_OpenMusic(const char *directory, SotnStream **out);
/** @brief Query the verified sector extent. @param stream Borrowed open stream or NULL.
 * @return Number of whole sectors, or zero for NULL. */
Uint32 SotnStream_Sectors(const SotnStream *stream);
/** @brief Read one checked sector without exposing the file descriptor.
 * @param stream Borrowed owned stream; calls must run on one thread.
 * @param sector Zero-based sector below SotnStream_Sectors().
 * @param output Receives one complete sector on success, unchanged on failure.
 * @param capacity Output bytes, at least SOTN_CD_SECTOR_BYTES.
 * @return Success, or false with SDL_GetError() for invalid input or I/O failure. */
bool SotnStream_ReadSector(SotnStream *stream, Uint32 sector, Uint8 *output, size_t capacity);
/** @brief Close the owned stream. @param stream Owned stream or NULL. */
void SotnStream_Close(SotnStream *stream);

#endif
