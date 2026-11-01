#ifndef SOTN_STONE_DISSOLVE_H
#define SOTN_STONE_DISSOLVE_H
#include <sotn/death_image.h>
#include <sotn/player_blink.h>

enum
{
    SOTN_STONE_DISSOLVE_ROWS = 36
};
/** @brief Actual entity32 and its original thirty-six textured slices. Color bytes retain
 * original per-row delay/motion metadata; they are not replacement color animations. */
typedef struct SotnStoneDissolve
{
    SotnPlayerEffectQuad quads[SOTN_STONE_DISSOLVE_ROWS];
    Sint32 x, y;
    Uint32 flags;
    Uint16 phase;
    Uint8 cursor, pattern, index, timer;
    bool facing_left, removed;
} SotnStoneDissolve;
/** @brief Actual resident StoreImage and original primary/secondary LoadImage services.
 * The native owner must roll back texture writes if later work fails. */
typedef bool (*SotnStoneDissolveCapture)(void *context, SotnDeathImage *image);
typedef bool (*SotnStoneDissolveUpload)(void *context, const SotnDeathImage *image, bool secondary);
/** @brief Current pre-camera body metadata and real allocation/image ownership. */
typedef struct SotnStoneDissolveInput
{
    SotnDataView dra;
    SotnPlayerImage image;
    Sint32 player_x, player_y;
    Uint32 engine_timer; /**< Actual g_Timer, including paused engine frames. */
    Uint16 player_animation, player_sprite, player_step, palette, priority, tapped;
    bool facing_left, allocated;
    SotnStoneDissolveCapture capture;
    SotnStoneDissolveUpload upload;
    void *context;
} SotnStoneDissolveInput;
/** @brief Actual calls from this callback; frame_load skips artwork upload, never body drawing. */
typedef struct SotnStoneDissolveEvents
{
    bool frame_load, reset_afterimages;
} SotnStoneDissolveEvents;
/** @brief Advance actual entity32 once, sharing the original single5120-byte DRA image.
 * A failed initial allocation retries without RNG. Actual image services are mandatory when
 * reached; requests alone confer no capture/upload readiness. Capture replaces shared pixels
 * while retaining the separate common-death cursor/count, even if the adapter initializes them.
 * State, shared image, RNG and
 * output commit only after success; the native owner separately rolls back service writes.
 * Repeated draws, menus/freeze and room rebases must not invoke this callback. */
bool SotnStoneDissolve_Tick(SotnStoneDissolve *effect, SotnDeathImage *image,
                            SotnStoneDissolveInput input, Uint32 *random,
                            SotnStoneDissolveEvents *out);
/** @brief Translate the actual entity and its camera-owned packets without advancing time. */
bool SotnStoneDissolve_Rebase(SotnStoneDissolve *effect, Sint16 x, Sint16 y, bool room);
#endif
