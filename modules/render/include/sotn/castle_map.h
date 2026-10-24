#ifndef SOTN_CASTLE_MAP_RENDERER_H
#define SOTN_CASTLE_MAP_RENDERER_H
#include <sotn/tilemap.h>
#include <sotn/world.h>
/** @brief Owned reconstruction scratch borrowing private blueprint, palette, and passage metadata.
 */
typedef struct SotnMapRenderer SotnMapRenderer;
/** @brief Allocate bounded map scratch and validate resource extents.
 * @param blueprint Verified F_MAP.BIN, 32768 bytes. @param common Verified F_GAME.BIN.
 * @param dra Verified main gameplay overlay. All views outlive this owner.
 * @param out Owned renderer, NULL on failure. @return Success or SDL error. */
bool SotnMapRenderer_Create(SotnDataView blueprint, SotnDataView common, SotnDataView dra,
                            SotnMapRenderer **out);
/** @brief Rebuild explored/purchased tiles and composite without advancing simulation.
 * @param renderer Borrowed owner. @param world Valid saved progress; not modified.
 * @param inverted Select the reversed castle's bits and orientation.
 * @param opacity 0..64 original map fade amount. @param frame In/out gameplay capture.
 * @param count At least SOTN_FRAME_PIXELS. @return Success or SDL error; invalid input
 * leaves the frame unchanged. Map data and temporary reveal pixels are never serialized. */
bool SotnMapRenderer_Draw(SotnMapRenderer *renderer, const SotnWorldState *world, bool inverted,
                          Uint8 opacity, Uint16 *frame, size_t count);
/** @brief Free scratch; borrowed data remain owned by caller. @param renderer Owned or NULL. */
void SotnMapRenderer_Destroy(SotnMapRenderer *renderer);
#endif
