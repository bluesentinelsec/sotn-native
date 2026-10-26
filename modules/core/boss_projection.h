#ifndef SOTN_BOSS_PROJECTION_H
#define SOTN_BOSS_PROJECTION_H
#include <SDL3/SDL.h>
#include <sotn/game_data.h>
/** @brief Bounded original reward effect transform; angles use 4096 units and scale uses 4096. */
typedef struct SotnRewardProjection
{
    Sint16 x, y, z, radius, scale;
    Sint32 anchor_x, anchor_y;
} SotnRewardProjection;
/** @brief Copied projected opposite corners; negative scale reverses both pairs. */
typedef struct SotnProjectedRect
{
    Sint32 left, top, right, bottom;
} SotnProjectedRect;
/** @brief Project the selected reward sprite through the original two matrix/projection passes.
 * @param executable Borrowed verified private matrix-pair table.
 * @param transform Initialized fixed-point effect state, radius within +/-128, anchor +/-16384.
 * @param out Copied screen-space corners, unchanged on failure.
 * @return Success or SDL error. No global GTE state or original tables are stored. */
bool SotnBoss_ProjectReward(SotnDataView executable, SotnRewardProjection transform,
                            SotnProjectedRect *out);
/** @brief Original GTE screen coordinates after signed saturation. */
typedef struct SotnProjectedPoint
{
    Sint32 x, y;
} SotnProjectedPoint;
/** @brief Project one original BO4 intro diamond vertex using private SDK trigonometry.
 * @param executable Borrowed verified boot math tables. @param anchor_x Room-local anchor.
 * @param anchor_y Room-local anchor. @param rotation Original 4096-unit Z angle.
 * @param x Original diamond abscissa, -32..224. @param y Ordinate, -32..32.
 * @param out Receives original saturated point, unchanged on failure.
 * @return Success or SDL error. Retains fixed Y tilt, depth320 and screen distance256;
 * this is a selected non-overflowing transform, not a general GTE implementation. */
bool SotnBoss_ProjectIntro(SotnDataView executable, Sint32 anchor_x, Sint32 anchor_y,
                           Sint16 rotation, Sint16 x, Sint16 y, SotnProjectedPoint *out);
/** @brief Original book transform; angles use 4096 units, depth is 0..768. */
typedef struct SotnBookTransform
{
    Sint16 x, y, z, depth;
    Sint32 anchor_x, anchor_y;
} SotnBookTransform;
/** @brief One bounded model vertex, with each axis within +/-32. */
typedef struct SotnBookVertex
{
    Sint16 x, y, z;
} SotnBookVertex;
/** @brief Copied screen vertices, winding and original ordering-table priority. */
typedef struct SotnBookProjection
{
    SotnProjectedPoint point[4];
    Uint16 depth[4];
    Sint32 winding;
    Uint16 priority;
} SotnBookProjection;
/** @brief Project a book face or its center with the original SDK matrix/divider.
 * @param executable Borrowed verified private matrix trigonometry.
 * @param transform Initialized selected book pose and screen anchor (+/-16384).
 * @param vertices Borrowed model coordinates; no original geometry is compiled in.
 * @param count Exactly four for a face (AVSZ4), or one for the center (SZ3/4).
 * @param out Copied result, unchanged on failure. Negative winding hides a face.
 * @return Success or SDL error. Supports the bounded book path, not general GTE overflow. */
bool SotnBook_Project(SotnDataView executable, SotnBookTransform transform,
                      const SotnBookVertex *vertices, Uint32 count, SotnBookProjection *out);
/** @brief Original Lesser Demon paired-quad controls, uniform scale in units of 4096. */
typedef struct SotnDemonQuadPose
{
    Sint16 angle, width, height, scale;
    Sint32 anchor_x, anchor_y;
} SotnDemonQuadPose;
/** @brief Project a selected Demon effect at equal depth/screen distance 1024.
 * @param executable Borrowed private matrix-pair table. @param pose Bounded initialized controls.
 * Width/height are 0..64, scale 0..16384, screen anchors +/-16384.
 * @param out Four copied saturated corners, unchanged on error.
 * @return Success or SDL error. Retains SDK column-scale rounding; no global GTE state. */
bool SotnDemon_ProjectQuad(SotnDataView executable, SotnDemonQuadPose pose,
                           SotnProjectedPoint out[4]);
/** @brief Project an original Chapel bell face with the selected SDK Z rotation and depth1024.
 * @param executable Borrowed private matrix-pair table. @param chapel Borrowed DAI model/pointers.
 * @param face Original face 0..2. @param angle Signed 4096-unit rotation.
 * @param x Screen anchor, +/-16384. @param y Screen anchor, +/-16384.
 * @param out Four saturated screen corners, unchanged on failure.
 * @return Success or SDL error for invalid pointers, vertices or unselected translation. */
bool SotnChapel_ProjectBell(SotnDataView executable, SotnDataView chapel, Uint16 face, Sint16 angle,
                            Sint32 x, Sint32 y, SotnProjectedPoint out[4]);
/** @brief Private table locations for the shared original Chapel/BO5 sky algorithm. */
typedef struct SotnCloudProfile
{
    size_t initialize, uv, vertices, grids, planes, rotation;
} SotnCloudProfile;
/** @brief Select metadata for an original stage without aliasing its runtime tables.
 * @param stage Original Chapel (6) or BO5 (25). @param out Receives copied offsets.
 * @return Success or SDL error; invalid input leaves the output unchanged. */
bool SotnCloud_GetProfile(Uint16 stage, SotnCloudProfile *out);
/** @brief Project a shared original sky tile through the selected private SDK plane model.
 * @param data Borrowed selected overlay. @param stage Original stage 6 or 25.
 * @param x Screen translation +/-16384. @param y Layer translation +/-512.
 * @param depth Original positive band depth, 192..2240. @param out Four copied points.
 * @return Success or SDL error; invalid controls/model/rotation leave output unchanged. */
bool SotnCloud_Project(SotnDataView data, Uint16 stage, Sint32 x, Sint32 y, Uint16 depth,
                       SotnProjectedPoint out[4]);
/** @brief Project an original Chapel cloud tile through the identity SDK matrix/divider.
 * @param chapel Borrowed DAI private vertices/rotation. @param x Screen translation, +/-16384.
 * @param y Layer translation, +/-512. @param depth Original positive band depth, 192..2240.
 * @param out Four saturated screen points, unchanged on failure.
 * @return Success or SDL error for invalid bounds, models or unselected rotation. */
bool SotnChapel_ProjectCloud(SotnDataView chapel, Sint32 x, Sint32 y, Uint16 depth,
                             SotnProjectedPoint out[4]);
/** @brief Bounded original glass vertex; the selected plane models have zero Z. */
typedef struct SotnGlassVertex
{
    Sint16 x, y, z;
} SotnGlassVertex;
/** @brief Original glass Y/Z rotation and screen-relative translation, before SDK projection. */
typedef struct SotnGlassTransform
{
    Sint16 y_angle, z_angle;
    Sint32 x, y, depth;
} SotnGlassTransform;
/** @brief Project selected Chapel glass/light/background models with the original SDK matrix.
 * @param executable Borrowed private matrix-pair table. @param pose Bounded Y/Z rotation,
 * translation +/-32767 and depth 1024..8192. @param vertices Borrowed plane vertices, axes +/-512.
 * @param count One center or four corners. @param out Copied points, unchanged on failure.
 * @return Success or SDL error for invalid controls, model, table or pointer. */
bool SotnChapel_ProjectGlass(SotnDataView executable, SotnGlassTransform pose,
                             const SotnGlassVertex *vertices, Uint16 count,
                             SotnProjectedPoint *out);
/** @brief Project a selected zero-rotation Library background anchor through the SDK divider.
 * @param library Borrowed private LIB model/rotation records. @param kind Background kind 0..3.
 * @param x Screen-space actor X, +/-32767. @param y Screen-space actor Y, +/-32767.
 * @param out Receives saturated SDK screen coordinates, unchanged on failure.
 * @return Success or SDL error for invalid controls, truncated data or nonselected rotations. */
bool SotnLibrary_ProjectBackground(SotnDataView library, Uint16 kind, Sint32 x, Sint32 y,
                                   SotnProjectedPoint *out);
/** @brief Copied rotating Library shelf vertices and SDK-lit RGB colors. */
typedef struct SotnShelfProjection
{
    SotnProjectedPoint point[8];
    Uint8 color[8][3];
} SotnShelfProjection;
/** @brief Project the selected Library shelf model with original fixed lighting and fog.
 * @param executable Borrowed private SDK pair/normalization tables.
 * @param library Borrowed private vertex/normal pointers and color matrix.
 * @param angle Original signed 4096-unit Y rotation. @param depth Extra depth, 0..128.
 * @param x Screen anchor, +/-16384. @param y Screen anchor, +/-16384.
 * @param out Receives copied geometry/colors, unchanged on failure.
 * @return Success or SDL error; rejects unselected model/normal domains. */
bool SotnLibrary_ProjectShelf(SotnDataView executable, SotnDataView library, Sint16 angle,
                              Sint16 depth, Sint32 x, Sint32 y, SotnShelfProjection *out);
#endif
