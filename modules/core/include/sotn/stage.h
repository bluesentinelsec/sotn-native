#ifndef SOTN_STAGE_H
#define SOTN_STAGE_H

#include <sotn/collision.h>

/** @brief Validated US stage metadata borrowing an immutable overlay image. */
typedef struct SotnStage SotnStage;

/** @brief A decoded tile layer; all byte views borrow the stage image. */
typedef struct SotnStageLayer
{
    SotnCollisionMap collision;
    SotnDataView pages, graphics, palettes;
    Uint8 left, top, right, bottom, parameters;
    Uint16 priority, flags;
    bool present;
} SotnStageLayer;

/** @brief A room header and its layers; transition entries have no decoded layers. */
typedef struct SotnStageRoom
{
    Uint8 left, top, right, bottom;
    Uint8 layout_id, tileset_id, object_graphics_id, object_layout_id;
    bool transition;
    SotnStageLayer foreground, background;
} SotnStageRoom;

/**
 * @brief Validate a US PS1 stage overlay's room and tile-layer data at base 0x80180000.
 * @param image Borrowed immutable overlay bytes, alive until after stage is closed.
 * @param out Receives an owned stage on success; NULL on failure when non-NULL.
 * @return True on success, false with SDL_GetError() on invalid pointers, tables, or sizes.
 * @note Validates every room and referenced tile array. PS1 pointers are decoded as
 * file offsets and are never dereferenced as host pointers or executed as host code.
 * No SDL subsystem initialization is required. This does not initialize game entities.
 */
bool SotnStage_Open(SotnDataView image, SotnStage **out);

/**
 * @brief Return the number of room headers, including cross-stage transition entries.
 * @param stage Borrowed open stage, or NULL.
 * @return Validated room count, or zero for NULL.
 */
Uint32 SotnStage_RoomCount(const SotnStage *stage);

/**
 * @brief Decode a room into borrowed, bounded byte views without allocations.
 * @param stage Borrowed open stage and immutable backing image.
 * @param index Room-header index, less than SotnStage_RoomCount().
 * @param out Receives the room on success; unchanged on failure.
 * @return True on success, false with SDL_GetError() for invalid arguments or image data.
 * @note Returned views expire when the backing game-data snapshot is closed. A room
 * with tileset ID 255 is a cross-stage transition, not a playable room or an empty room.
 */
bool SotnStage_GetRoom(const SotnStage *stage, Uint32 index, SotnStageRoom *out);

/**
 * @brief Release stage metadata without freeing its borrowed image.
 * @param stage Owned stage to release, or NULL.
 */
void SotnStage_Close(SotnStage *stage);

#endif
