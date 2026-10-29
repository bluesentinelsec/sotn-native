#ifndef SOTN_ACTOR_GRAPHICS_H
#define SOTN_ACTOR_GRAPHICS_H

#include <sotn/actors.h>
#include <sotn/alucard_sprite.h>
#include <sotn/dialogue.h>
#include <sotn/equipment.h>
#include <sotn/mist_effect.h>
#include <sotn/palette.h>
#include <sotn/player_afterimage.h>
#include <sotn/player_blink.h>
#include <sotn/tilemap.h>

/** @brief Owned stage/common actor texture memory, borrowing immutable DRA/NZ0 images. */
typedef struct SotnActorGraphics SotnActorGraphics;

/** @brief Initialize original Alucard/effect palette rows from private character-start job 22.
 * @param graphics Borrowed texture owner, before current equipment colors are applied.
 * @return Success or SDL error with unchanged memory for invalid descriptors/source colors.
 * The raw common artwork palettes do not contain the final Mist source rows. Invoke once
 * when constructing a fresh player texture owner; never reset live palette jobs each frame. */
bool SotnActorGraphics_InitPlayerPalettes(SotnActorGraphics *graphics);

/** @brief Upload actual normal/Bat packed pixels into the original owned player display slot.
 * @param graphics Borrowed texture owner. @param sprite Borrowed actual decoded private image.
 * @return Success or SDL error without texture mutation for invalid pixels/display metadata.
 * Preserves original right/bottom clear margins. Geometry, rotation and effect profiles
 * belong to their respective callers; this operation changes no animation or readiness. */
bool SotnActorGraphics_UploadPlayer(SotnActorGraphics *graphics, const SotnAlucardSprite *sprite);

/** @brief Prepare an original normal-ARC afterimage packet and its actual private display upload.
 * @param graphics Owned texture memory. @param sprite Borrowed immutable decoded ARC pose.
 * @param display Actual global display slot1..3, with a flagged original palette identity.
 * @param x Screen anchor X, including original camera shake. @param y Screen anchor Y.
 * @param out Receives the source packet; hidden on original empty/culled paths.
 * @return Success or SDL error. Invalid metadata leaves texture memory/output unchanged.
 * Reads private display coordinates/flip, preserves packed clear margins and frame-load skips.
 * Does not advance animation, capture/fade, clock or ownership. Caller must own the common lease.
 */
bool SotnActorGraphics_Afterimage(SotnActorGraphics *graphics, const SotnAlucardSprite *sprite,
                                  SotnAfterimageDisplay display, Sint32 x, Sint32 y,
                                  SotnPlayerEffectQuad *out);

/** @brief Capture an unrotated normal Alucard pose into his original owned display slot.
 * @param graphics Borrowed texture owner. @param sprite Actual decoded private ARC pose.
 * @param x Signed-word screen anchor X. @param y Signed-word screen anchor Y.
 * @param facing_left Original facing. @param out Receives eight real snapshot wedges.
 * @return Success or SDL error, leaving texture memory/output unchanged on invalid input.
 * Reproduces original right/bottom clear margins and BlinkWhite's Mist wedge geometry.
 * Rotated, Bat and Wolf captures require their own original adapters. */
bool SotnActorGraphics_CapturePlayer(SotnActorGraphics *graphics, const SotnAlucardSprite *sprite,
                                     Sint32 x, Sint32 y, bool facing_left, SotnMistWedge out[8]);
/** @brief Queue one original selected Mist palette operation (27, 0x11C, 0x11D or 0x11F).
 * @param graphics Borrowed texture owner. @param operation Original operation/flag word.
 * @param accepted Receives false if all 32 selected job slots are occupied; no slot is replaced.
 * @return Success or SDL error with owner/output unchanged on malformed descriptors/colors.
 * A successful full-pool result mirrors the source's omitted optional palette request. */
bool SotnActorGraphics_RequestMistPalette(SotnActorGraphics *graphics, Uint16 operation,
                                          bool *accepted);
/** @brief Cancel the selected jobs matching original palette channel eight, retaining copies.
 * @param graphics Borrowed owner. @return Success or SDL error for a missing owner. */
bool SotnActorGraphics_CancelMistPalette(SotnActorGraphics *graphics);
/** @brief Advance queued selected Mist palette jobs once per original simulation tick.
 * @param graphics Borrowed owner. @return Success or SDL error; failed validation changes
 * neither colors nor job cursors. Palette source rows remain live owned data, not stale copies. */
bool SotnActorGraphics_TickMistPalette(SotnActorGraphics *graphics);
/** @brief Refresh normal Alucard's owned palette used by the original Mist restoration job.
 * @param graphics Borrowed owner. @param colors Sixteen copied current character colors.
 * @return Success or SDL error before mutation. Borrowed private artwork stays unchanged. */
bool SotnActorGraphics_SetPlayerPalette(SotnActorGraphics *graphics, const Uint16 colors[16]);
/** @brief Copy the current owned sixteen-color Mist palette for presentation/diagnostics.
 * @param graphics Borrowed owner. @param out Unchanged on failure.
 * @return Success or SDL error for missing owner, output or resident palette colors. */
bool SotnActorGraphics_GetMistPalette(const SotnActorGraphics *graphics, Uint16 out[16]);

/** @brief Borrowed tightly packed RGB555 raster surface; each dimension is 1..256. */
typedef struct SotnActorSurface
{
    Uint16 *pixels;
    size_t count;
    Uint16 width, height;
} SotnActorSurface;

/** @brief Original owned VRAM render target rectangle, excluding borrowed artwork memory. */
typedef struct SotnActorTarget
{
    Uint16 x, y, width, height;
    bool clear;
    Uint16 color;
} SotnActorTarget;

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
/** @brief Refresh BO5's original five-row text cache from private common-font words.
 * @param graphics Borrowed stage-25 texture owner. @param dialogue Copied primitive-text state.
 * @return Success or SDL error; validates glyphs/residency before writing. No strings or artwork
 * are compiled in. Owned dialogue packets select the cache's original rows and clipping. */
bool SotnActorGraphics_LoadDialogue(SotnActorGraphics *graphics, SotnDialogueState dialogue);
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
    Sint16 rotation, scale_x, scale_y;
    Uint16 opacity;
    bool tint; /**< When true, RGB multipliers replace opacity. */
    Uint8 red, green, blue;
} SotnSpriteTransform;
/** @brief Resolve a copied actor's explicit transform flags or its legacy value selectors.
 * @param actor Copied presentation state; no ownership transfers.
 * @return Signed affine/color values. Explicitly disabled values resolve to neutral defaults;
 * enabled zero scales remain zero, so the caller can omit their invisible sprites.
 * @note Does not validate drawing bounds; DrawTransform checks nonzero scales and color ranges.
 * The original selected bits are authoritative only when transform_flags_set is true. */
SotnSpriteTransform SotnActorGraphics_GetTransform(SotnActorState actor);
/** @brief Draw a bounded scaled/rotated/color-modulated multipart actor.
 * @param graphics Borrowed texture service. @param sprite Private sprite selection.
 * @param x Screen anchor. @param y Screen anchor. @param transform Nonzero signed scale
 * -1024..1024 (negative reflects the axis), color 0..255.
 * @param frame Full writable frame. @param count Available pixels.
 * @return Success or SDL error; invalid references leave the frame unchanged. */
bool SotnActorGraphics_DrawTransform(const SotnActorGraphics *graphics, SotnActorSprite sprite,
                                     Sint32 x, Sint32 y, SotnSpriteTransform transform,
                                     Uint16 *frame, size_t count);
/** @brief Draw an original sprite to a bounded borrowed RGB555 surface.
 * @param graphics Borrowed texture owner. @param sprite Original attributes.
 * @param x Surface-space anchor. @param y Surface-space anchor.
 * @param transform Original bounded nonzero transform. @param surface Writable pixels/view.
 * @return Success or SDL error; invalid resources/geometry leave the surface unchanged. */
bool SotnActorGraphics_DrawSurface(const SotnActorGraphics *graphics, SotnActorSprite sprite,
                                   Sint32 x, Sint32 y, SotnSpriteTransform transform,
                                   SotnActorSurface surface);
/** @brief Prepare the original Wall rope cache with bounded CPU copies in texture memory.
 * @param graphics Borrowed initialized texture service, after loading the room bank.
 * @return Success or SDL error; invalid/missing source pixels leave texture memory intact. */
bool SotnActorGraphics_PrepareOuterRope(SotnActorGraphics *graphics);
/** @brief Stage quad with optional page/palette selection; vertices order TL,TR,BL,BR.
 */
typedef struct SotnTextureQuad
{
    Sint32 x[4], y[4];
    Uint16 u[4], v[4]; /**< Texture edges 0..256, including the mirrored page boundary. */
    Uint16 page, palette;
    bool blend, modulate, tint;
    Uint8 red, green, blue;
    Uint8 opacity;    /**< Color multiplier, 128 neutral, used only when modulate is true. */
    bool gouraud;     /**< Interpolate per-vertex multipliers instead of the uniform tint. */
    bool framebuffer; /**< Sample an owned captured 256x240 frame through direct-color pages. */
    bool untextured;  /**< Use vertex/uniform colors directly; black is opaque, UVs are unused. */
    Uint8 vertex_red[4], vertex_green[4], vertex_blue[4];
} SotnTextureQuad;
/** @brief Draw a bounded textured or untextured quad as two triangles.
 * @param graphics Borrowed texture service; NULL is valid for an untextured quad.
 * @param quad Copied quad, coordinates within +/-2048.
 * @param frame Full RGB555 output. @param count Available pixels.
 * @return Success or SDL error; invalid references leave the frame unchanged. */
bool SotnActorGraphics_Quad(const SotnActorGraphics *graphics, SotnTextureQuad quad, Uint16 *frame,
                            size_t count);
/** @brief Draw a bounded quad to a borrowed 1..256 by 1..256 RGB555 surface.
 * @param graphics Borrowed texture owner, optional for untextured quads.
 * @param quad Surface-space geometry. @param surface Writable tightly packed pixels/view.
 * @return Success or SDL error; validation precedes writes, as with the frame quad API. */
bool SotnActorGraphics_QuadSurface(const SotnActorGraphics *graphics, SotnTextureQuad quad,
                                   SotnActorSurface surface);
/** @brief Draw a selected original player/effect packet into a borrowed raster surface.
 * @param graphics Borrowed texture owner. @param packet Borrowed original packet attributes.
 * @param offset_x Target X offset, -2048..2048. @param offset_y Target Y offset, same bounds.
 * @param surface Borrowed destination, alive for this call only.
 * @return Success or SDL error for invalid input. Hidden packets leave the surface unchanged.
 * Raw packets ignore vertex colors; DRAW_COLORS enables texture modulation and interpolation. */
bool SotnActorGraphics_PlayerEffect(const SotnActorGraphics *graphics,
                                    const SotnPlayerEffectQuad *packet, Sint32 offset_x,
                                    Sint32 offset_y, SotnActorSurface surface);
/** @brief Open one owned offscreen VRAM target for ordered drawing.
 * @param graphics Borrowed texture owner; no other target may be active.
 * @param target Rectangle in 1024x512 owned texture memory, width/height 1..256.
 * @param out Borrowed scratch surface until commit, cancellation or owner destruction.
 * @return Success or SDL error with previous texture memory unchanged. Opening a target
 * never writes texture memory; clear or existing resident pixels initialize the scratch copy.
 * Callers must finish each draw interval before sampling the resulting target texture. */
bool SotnActorGraphics_BeginTarget(SotnActorGraphics *graphics, SotnActorTarget target,
                                   SotnActorSurface *out);
/** @brief Commit or cancel the active target and invalidate its borrowed scratch view.
 * @param graphics Borrowed owner with active target. @param commit Copy pixels to owned VRAM.
 * @return Success or SDL error; cancellation leaves texture memory unchanged. */
bool SotnActorGraphics_EndTarget(SotnActorGraphics *graphics, bool commit);
/** @brief Borrowed ordered-target view; mode 0 is main, 1 strict, 3 replaceable original env.
 * Initialize with zero mode and the main surface. Surface views expire on every target change. */
typedef struct SotnActorTargetCursor
{
    SotnActorSurface surface;
    Sint32 offset_x, offset_y;
    Uint8 mode;
} SotnActorTargetCursor;
/** @brief Apply a copied actor environment command to one ordered target cursor.
 * @param graphics Borrowed texture owner, exclusively controlled by this cursor while active.
 * @param main Borrowed writable normal surface, dimensions 1..256 and initialized pixels.
 * @param command Original target operation: 1/2 strict begin/restore, 3/4 replaceable
 * begin/restore.
 * @param cursor Initialized borrowed state, updated after each completed commit/open operation.
 * @return Success or SDL error. Strict intervals reject nesting/unmatched restores. Original
 * environment intervals may replace/commit another such interval and repeat their main restore.
 * Invalid bounds/modes fail before writes; a failed subsequent open retains the completed commit.
 * On failure the caller must cancel any interval still identified by a nonzero cursor mode. */
bool SotnActorGraphics_ApplyTarget(SotnActorGraphics *graphics, SotnActorSurface main,
                                   SotnActorPrimitive command, SotnActorTargetCursor *cursor);
/** @brief Copy the current ordered RGB555 frame for subsequent framebuffer-sampling quads.
 * @param graphics Borrowed texture owner. @param frame Borrowed full input frame.
 * @param count Available pixels, at least SOTN_FRAME_PIXELS.
 * @return Success or SDL error; invalid arguments leave the previous snapshot unchanged.
 * The copy is owned until replacement/destruction; no private artwork memory is modified.
 * Snapshot sampling avoids read/write aliasing; PS1 GPU texture-cache parity is unproven. */
bool SotnActorGraphics_CaptureFrame(SotnActorGraphics *graphics, const Uint16 *frame, size_t count);
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

/** @brief Apply a simulation-owned BO4 death erasure mask to a captured display rectangle.
 * @param graphics Owned BO4 texture service. @param sprite Original final body pose.
 * @param erased Borrowed 512-byte bitset for the original 64x64 scratch image.
 * @return Success or SDL error, preserving cache on invalid references.
 * @note The first call snapshots the private display image; subsequent calls rebuild
 * from that snapshot and the full mask, independently of skipped presentation frames. */
bool SotnActorGraphics_DoppleDissolve(SotnActorGraphics *graphics, SotnActorSprite sprite,
                                      SotnDataView erased);
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
/** @brief Draw a bounded two-color line to a borrowed RGB555 surface.
 * @param line Surface-space endpoints/colors. @param surface Writable tightly packed view.
 * @return Success or SDL error; invalid input leaves pixels unchanged. */
bool SotnActorGraphics_LineSurface(SotnColorLine line, SotnActorSurface surface);

/** @brief Release texture memory without freeing borrowed images. @param graphics Owned object or
 * NULL. */
void SotnActorGraphics_Destroy(SotnActorGraphics *graphics);

/** @brief Apply a stage palette interpolation to owned texture memory atomically.
 * @param graphics Borrowed initialized texture service. @param blend Copied original command.
 * Active commands require source/destinations below 768, 1..16 steps and no source overlap.
 * @return Success or SDL error. Missing source colors or invalid bounds leave all VRAM intact;
 * borrowed private images remain unchanged. Inactive commands do nothing. */
bool SotnActorGraphics_BlendPalette(SotnActorGraphics *graphics, SotnPaletteBlend blend);
/** @brief Apply a bounded original red-channel palette increment to owned graphics memory.
 * @param graphics Borrowed owner. @param shift Copied palette rows; zero count does nothing.
 * @return Success or SDL error. All rows/residency are checked before any write;
 * transparency entry zero, RGB555 green/blue and mask bits remain unchanged. */
bool SotnActorGraphics_ReddenPalette(SotnActorGraphics *graphics, SotnPaletteRedShift shift);
/** @brief Draw original multipart scale-only geometry using SDK signed-floor rounding.
 * @param graphics Borrowed texture owner. @param sprite Original multipart frame/attributes.
 * @param x Screen anchor. @param y Screen anchor. @param scale_x 8.8 scale, 1..1024.
 * @param scale_y 8.8 scale, 1..1024. @param surface Owned destination.
 * @return Success or SDL error before drawing on invalid input. No rotation or packed sheets. */
bool SotnActorGraphics_DrawScaled(const SotnActorGraphics *graphics, SotnActorSprite sprite,
                                  Sint32 x, Sint32 y, Uint16 scale_x, Uint16 scale_y,
                                  SotnActorSurface surface);
#endif
