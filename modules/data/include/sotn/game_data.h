#ifndef SOTN_GAME_DATA_H
#define SOTN_GAME_DATA_H

#include <SDL3/SDL_stdinc.h>

/** @brief Validated, application-owned snapshot of the five US core files. */
typedef struct SotnGameData SotnGameData;

/** @brief Logical core files; arbitrary filesystem paths are not accepted by the read API. */
typedef enum SotnGameFileId
{
    SOTN_GAME_MAIN = 0,  /**< PS1 boot executable. */
    SOTN_GAME_DRA,       /**< Main gameplay overlay. */
    SOTN_GAME_RIC,       /**< Richter player overlay. */
    SOTN_GAME_ST0,       /**< Prologue stage overlay. */
    SOTN_GAME_NZ0,       /**< Alchemy Laboratory stage overlay. */
    SOTN_GAME_FILE_COUNT /**< Number of supported core files. */
} SotnGameFileId;

/** @brief Borrowed immutable bytes; valid until the owning SotnGameData is closed. */
typedef struct SotnDataView
{
    const Uint8 *bytes; /**< Read-only file bytes; never native structs or executable host code. */
    size_t size;        /**< Number of bytes available at bytes. */
} SotnDataView;

/**
 * @brief Load and verify a bounded snapshot from a user's extracted US game directory.
 * @param directory Borrowed UTF-8 path. Relative paths use the current working directory.
 * @param out Receives ownership on success; set to NULL on failure when non-NULL.
 * @return True if all five files match; otherwise false with context in SDL_GetError().
 * @note Opens fixed reference filenames read-only, follows OS symlink behavior, and writes nothing.
 * The caller chooses a trusted directory; this API is not a filesystem sandbox. SHA-1 identifies
 * the upstream revision, not cryptographic authenticity. No SDL subsystem initialization is needed.
 * @post Close successful results with SotnGameData_Close. Partial failures release all allocations.
 */
bool SotnGameData_Open(const char *directory, SotnGameData **out);

/**
 * @brief Obtain a read-only view of an already validated file without further filesystem access.
 * @param data Borrowed, open data snapshot.
 * @param file Logical file to access.
 * @param out Receives borrowed bytes, or a zeroed view on failure when non-NULL.
 * @return True on success; false for invalid arguments with context in SDL_GetError().
 * @note All views expire when data is closed. Decode on-disc fields explicitly; do not cast
 * these bytes to pointer-containing native structs. Concurrent reads are safe until close.
 */
bool SotnGameData_GetFile(const SotnGameData *data, SotnGameFileId file, SotnDataView *out);

/**
 * @brief Release a snapshot and invalidate every view borrowed from it.
 * @param data Owned snapshot to release, or NULL for no operation.
 * @note The caller must finish all reads before closing. Never close the same snapshot twice.
 */
void SotnGameData_Close(SotnGameData *data);

/**
 * @brief Return the supported reference identifier.
 * @return Borrowed, immutable string with static lifetime; never free it.
 */
const char *SotnGameData_ReferenceId(void);

#endif
