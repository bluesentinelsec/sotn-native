#ifndef SOTN_ACTOR_GRAPHICS_H
#define SOTN_ACTOR_GRAPHICS_H

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
/**
 * @brief Apply an original NZ0 actor graphics bank, retaining other loaded texture memory.
 * @param graphics Borrowed owned texture memory on the main thread.
 * @param bank Bank index 0..24; room object_graphics_id uses index ID minus one.
 * @return True on success, false with SDL_GetError() and unchanged memory on failure.
 * @note Handles bounded raw and dictionary/RLE uploads; validates all transfers before applying.
 */
bool SotnActorGraphics_LoadBank(SotnActorGraphics *graphics, Uint32 bank);
/**
 * @brief Draw original common/stage multipart sprites into the shared RGB555 frame.
 * @param graphics Borrowed initialized texture memory and immutable source images.
 * @param sprite Original sprite attributes; frame zero is hidden.
 * @param x Screen-space actor anchor in pixels.
 * @param y Screen-space actor anchor in pixels.
 * @param frame Borrowed writable full framebuffer, not overlapping graphics or source images.
 * @param count Available pixels, at least SOTN_FRAME_PIXELS.
 * @return True on success, false with SDL_GetError() and unchanged frame for invalid references.
 * @note Preserves pivots, mirroring, part cropping, texture/palette lookup, transparency,
 * and blend arithmetic. Packed Alucard sheets, transforms, and full GPU ordering are separate
 * paths.
 */
bool SotnActorGraphics_Draw(const SotnActorGraphics *graphics, SotnActorSprite sprite, Sint32 x,
                            Sint32 y, Uint16 *frame, size_t count);
/** @brief Release texture memory without freeing borrowed images. @param graphics Owned object or
 * NULL. */
void SotnActorGraphics_Destroy(SotnActorGraphics *graphics);

#endif
