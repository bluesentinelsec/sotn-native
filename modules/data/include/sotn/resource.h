#ifndef SOTN_RESOURCE_H
#define SOTN_RESOURCE_H

#include <sotn/game_data.h>

/** @brief Additional private files loaded on demand, independently of the core snapshot. */
typedef enum SotnResourceId
{
    SOTN_RESOURCE_LAB_GRAPHICS = 0,      /**< US ST/NZ0/F_NZ0.BIN tiles and palettes. */
    SOTN_RESOURCE_COMMON_GRAPHICS,       /**< US BIN/F_GAME.BIN common textures and palettes. */
    SOTN_RESOURCE_ALUCARD_GRAPHICS,      /**< US BIN/ARC_F.BIN player sprite sheets. */
    SOTN_RESOURCE_COMMON_AUDIO,          /**< US shared sound-bank body. */
    SOTN_RESOURCE_ALUCARD_AUDIO_HEADER,  /**< US Alucard instrument metadata. */
    SOTN_RESOURCE_ALUCARD_AUDIO_BODY,    /**< US Alucard sample data. */
    SOTN_RESOURCE_LAB_AUDIO_HEADER,      /**< US Laboratory instrument metadata. */
    SOTN_RESOURCE_LAB_AUDIO_BODY,        /**< US Laboratory sample data. */
    SOTN_RESOURCE_WEAPON_GRAPHICS,       /**< US BIN/WEAPON0.BIN weapon data and textures. */
    SOTN_RESOURCE_GAME_OVER,             /**< US BIN/F_GO.BIN game-over artwork. */
    SOTN_RESOURCE_CASTLE_MAP,            /**< US F_MAP.BIN castle blueprint. */
    SOTN_RESOURCE_ENTRANCE_AUDIO_HEADER, /**< US first-visit Entrance instrument metadata. */
    SOTN_RESOURCE_ENTRANCE_AUDIO_BODY,   /**< US first-visit Entrance sample data. */
    SOTN_RESOURCE_RETURN_AUDIO_HEADER,   /**< US revisited Entrance instruments. */
    SOTN_RESOURCE_RETURN_AUDIO_BODY,     /**< US revisited Entrance sample data. */
    SOTN_RESOURCE_COUNT
} SotnResourceId;

/** @brief Owned, verified immutable resource bytes. */
typedef struct SotnResource SotnResource;

/**
 * @brief Load one fixed, checksum-verified resource from the user's extracted directory.
 * @param directory Borrowed UTF-8 directory, with the same trust boundary as core loading.
 * @param id Supported resource ID; arbitrary filenames are not accepted.
 * @param out Receives owned data on success, NULL on failure when non-NULL.
 * @return True on success, false with SDL_GetError() on invalid input or I/O failure.
 * @note Uses bounded allocation and read-only I/O. No SDL subsystem initialization is needed.
 */
bool SotnResource_Open(const char *directory, SotnResourceId id, SotnResource **out);

/**
 * @brief Borrow bytes from a verified open resource without further filesystem reads.
 * @param resource Borrowed open resource, or NULL.
 * @return An immutable view expiring at SotnResource_Close(), or an empty view for NULL.
 */
SotnDataView SotnResource_View(const SotnResource *resource);

/**
 * @brief Release an owned resource and invalidate its borrowed views.
 * @param resource Owned resource, or NULL. Finish all readers before closing.
 */
void SotnResource_Close(SotnResource *resource);

#endif
