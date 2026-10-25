#ifndef SOTN_ACTOR_GRAPHICS_H
#define SOTN_ACTOR_GRAPHICS_H

#include <sotn/equipment.h>
#include <sotn/tilemap.h>

/** @brief Owned stage/common actor texture memory, borrowing immutable DRA/NZ0 images. */
typedef struct SotnActorGraphics SotnActorGraphics;

/** @brief Original untransformed actor sprite selection and rendering attributes. */
typedef struct SotnActorSprite
{
    Uint16 animation_set, frame, palette, texture_offset, blend_mode;
    bool facing_left;
} SotnActorSprite;

/**
 * @brief Initialize actor texture memory and the Laboratory's original bulk palettes.
 * @param dra Borrowed immutable US main image, alive until graphics are destroyed.
 * @param nz0 Borrowed immutable US Laboratory image, alive until graphics are destroyed.
 * @param sources Borrowed verified raw stage/common graphics, read only during creation.
 * @param out Receives ownership, NULL on failure when non-NULL.
 * @return True on success, false with SDL_GetError() for bad input or unsupported palettes.
 * @note This is a bounded texture/palette service, not a complete PS1 GPU or palette animator.
 */
bool SotnActorGraphics_Create(SotnDataView dra, SotnDataView nz0, SotnTileSources sources,
                              SotnActorGraphics **out);
/** @brief Initialize texture memory with the selected stage's explicit sprite-table bound.
 * @param dra Borrowed DRA. @param nz0 Borrowed selected overlay. @param sources Verified art.
 * @param stage Original supported stage ID. @param out Receives ownership or NULL.
 * @return Success or SDL error; borrowed images must outlive the returned service. */
bool SotnActorGraphics_CreateStage(SotnDataView dra, SotnDataView nz0, SotnTileSources sources,
                                   Uint16 stage, SotnActorGraphics **out);
/**
 * @brief Apply an original NZ0 actor graphics bank, retaining other loaded texture memory.
 * @param graphics Borrowed owned texture memory on the main thread.
 * @param bank Bank index 0..24; room object_graphics_id uses index ID minus one.
 * @return True on success, false with SDL_GetError() and unchanged memory on failure.
 * @note Handles bounded raw and dictionary/RLE uploads; validates all transfers before applying.
 */
bool SotnActorGraphics_LoadBank(SotnActorGraphics *graphics, Uint32 bank);
/** @brief Upload one original 48x72 eight-bit dialogue portrait from the borrowed overlay.
 * @param graphics Owned texture memory. @param offset Checked stage-relative pixel offset.
 * @param slot Original portrait slot 0 or 1. @return Success or SDL error; unchanged on error.
 * Portrait palettes must already be installed by the stage's bulk palette loader.
 */
bool SotnActorGraphics_LoadPortrait(SotnActorGraphics *graphics, Uint32 offset, Uint16 slot);
/** @brief Load a hand's supported weapon texture, palettes, and multipart definitions.
 * @param graphics Borrowed owned texture memory. @param weapons Borrowed WEAPON0.BIN; alive
 * until replacement or graphics destruction. @param hand Left (0) or right (1).
 * @param equipment Copied supported hand-item metadata; zero clears the hand's sprite bank.
 * @return Success, or false with SDL_GetError() and unchanged texture memory on bad references. */
bool SotnActorGraphics_LoadWeapon(SotnActorGraphics *graphics, SotnDataView weapons, Uint32 hand,
                                  SotnEquipment equipment);
/**
 * @brief Draw supported common/stage sprites into the shared RGB555 frame.
 * @param graphics Borrowed initialized texture memory and immutable source images.
 * @param sprite Original sprite attributes; a zero low-15-bit frame is hidden.
 * @param x Screen-space actor anchor in pixels.
 * @param y Screen-space actor anchor in pixels.
 * @param frame Borrowed writable full framebuffer, not overlapping graphics or source images.
 * @param count Available pixels, at least SOTN_FRAME_PIXELS.
 * @return True on success, false with SDL_GetError() and unchanged frame for invalid references.
 * @note Preserves pivots, mirroring, part cropping, texture/palette lookup, transparency,
 * and blend arithmetic. BO4 body/bat packed sheets are supported; normal Alucard,
 * modified-VRAM sheet effects and full GPU ordering remain separate paths.
 */
bool SotnActorGraphics_Draw(const SotnActorGraphics *graphics, SotnActorSprite sprite, Sint32 x,
                            Sint32 y, Uint16 *frame, size_t count);
/** @brief Draw multipart sprites rotated about their anchor, using 4096 units per revolution.
 * @param graphics Borrowed texture service. @param sprite Original sprite attributes.
 * @param x Screen anchor. @param y Screen anchor. @param angle Signed original rotation.
 * @param frame Full writable RGB555 frame. @param count Available pixels.
 * @return Success or SDL error; invalid private references leave the frame unchanged. */
bool SotnActorGraphics_DrawRotated(const SotnActorGraphics *graphics, SotnActorSprite sprite,
                                   Sint32 x, Sint32 y, Sint16 angle, Uint16 *frame, size_t count);
/** @brief Original affine scale/rotation/color, with 256 as unit scale and 128 as neutral color. */
typedef struct SotnSpriteTransform
{
    Sint16 rotation;
    Uint16 scale_x, scale_y, opacity;
} SotnSpriteTransform;
/** @brief Draw a bounded scaled/rotated/color-modulated multipart actor.
 * @param graphics Borrowed texture service. @param sprite Private sprite selection.
 * @param x Screen anchor. @param y Screen anchor. @param transform Scale 1..1024, color 0..255.
 * @param frame Full writable frame. @param count Available pixels.
 * @return Success or SDL error; invalid references leave the frame unchanged. */
bool SotnActorGraphics_DrawTransform(const SotnActorGraphics *graphics, SotnActorSprite sprite,
                                     Sint32 x, Sint32 y, SotnSpriteTransform transform,
                                     Uint16 *frame, size_t count);
/** @brief Prepare the original Wall rope cache with bounded CPU copies in texture memory.
 * @param graphics Borrowed initialized texture service, after loading the room bank.
 * @return Success or SDL error; invalid/missing source pixels leave texture memory intact. */
bool SotnActorGraphics_PrepareOuterRope(SotnActorGraphics *graphics);
/** @brief Textured quad using original page/palette selection; vertices order TL,TR,BL,BR.
 */
typedef struct SotnTextureQuad
{
    Sint32 x[4], y[4];
    Uint16 u[4], v[4]; /**< Texture edges 0..256, including the mirrored page boundary. */
    Uint16 page, palette;
    bool blend, modulate, tint;
    Uint8 red, green, blue;
    Uint8 opacity; /**< Color multiplier, 128 neutral, used only when modulate is true. */
    bool gouraud;  /**< Interpolate per-vertex multipliers instead of the uniform tint. */
    Uint8 vertex_red[4], vertex_green[4], vertex_blue[4];
} SotnTextureQuad;
/** @brief Draw a bounded quad as two textured triangles from loaded private texture memory.
 * @param graphics Borrowed texture service. @param quad Copied quad, coordinates within +/-2048.
 * @param frame Full RGB555 output. @param count Available pixels.
 * @return Success or SDL error; invalid references leave the frame unchanged. */
bool SotnActorGraphics_Quad(const SotnActorGraphics *graphics, SotnTextureQuad quad, Uint16 *frame,
                            size_t count);
/** @brief Apply BO4's original mutable horizontal bat-pose pivot.
 * @param graphics Owned service. @param frame Original backed bat pose.
 * @param offset Original form adjustment, -64..64. @return Success or SDL error.
 * @note Updates only this pose's native mirror, retaining prior pose adjustments
 * without changing private sprite definitions. */
bool SotnActorGraphics_UpdateDoppleBat(SotnActorGraphics *graphics, Uint16 frame, Sint16 offset);

/** @brief Upload a private BO4 body/bat pose to its original display scratch slot.
 * @param graphics Owned main-thread texture service. @param sprite Original packed pose.
 * @return Success or SDL error; invalid metadata leaves VRAM unchanged.
 * @note Reads display-slot coordinates from private DRA. Retains tagged poses only
 * after an upload, and applies original right/bottom clear margins. Required by
 * original child effects that sample the displayed body image. */
bool SotnActorGraphics_LoadDopplePose(SotnActorGraphics *graphics, SotnActorSprite sprite);

/** @brief CPU-presented original two-color line; screen coordinates within +/-2048.
 * Color endpoints use original eight-bit channels; blend modes are half/add/subtract/quarter. */
typedef struct SotnColorLine
{
    Sint32 x[2], y[2];
    Uint8 red[2], green[2], blue[2], blend_mode;
    bool blend;
} SotnColorLine;
/** @brief Draw a bounded interpolated color line into a full RGB555 frame.
 * @param line Copied endpoints/colors. @param frame Borrowed writable full frame.
 * @param count Available pixels. @return Success or SDL error, unchanged on invalid input.
 * @note Uses explicit native integer sampling. Hardware endpoint registration and
 * GPU ordering still require console comparison. No texture memory is needed. */
bool SotnActorGraphics_Line(SotnColorLine line, Uint16 *frame, size_t count);

/** @brief Release texture memory without freeing borrowed images. @param graphics Owned object or
 * NULL. */
void SotnActorGraphics_Destroy(SotnActorGraphics *graphics);

#endif
