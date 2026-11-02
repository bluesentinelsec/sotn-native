#ifndef SOTN_TITLE_SCREEN_H
#define SOTN_TITLE_SCREEN_H
#include <SDL3/SDL.h>
enum
{
    SOTN_TITLE_SPRITES = 17
};
/** @brief Original SPRT fields; clut is the original index resolved through g_ClutIds. */
typedef struct SotnTitleSprite
{
    Sint16 x, y;
    Uint16 page, clut, priority, draw_mode;
    Uint8 u, v, width, height, red[4], green[4], blue[4], phase, timer;
} SotnTitleSprite;
/** @brief Retail title callback state. Actual resources and seventeen leased rows are external. */
typedef struct SotnTitleScreen
{
    SotnTitleSprite sprites[SOTN_TITLE_SPRITES];
    Sint32 brightness, retained_word;
    Uint8 phase;
    bool completed;
} SotnTitleScreen;
typedef struct SotnTitleScreenInput
{
    Uint32 engine_clock;
    bool start_tapped, disc;
} SotnTitleScreenInput;
/** @brief Original requests, not completed loading/display/sound/menu operations. */
typedef struct SotnTitleScreenEvents
{
    Uint16 sound;
    bool load_graphics, clear, title_display, stage_display, restore_cluts, release_rows, main_menu,
        memory_card;
} SotnTitleScreenEvents;
/** @brief Run one original retail HandleTitleScreen callback, without owning an engine clock.
 * @param state Actual callback state, initially zero; unchanged on error.
 * @param input Actual engine clock, Start edge and disc/native loading context.
 * @param random Actual shared libc cursor; committed only after callback success.
 * @param out Copied owner requests, unchanged on error.
 * @return Success or SDL error. Before phase0 the caller must own seventeen real SPRT rows;
 * actual verified graphics, GPU/display, audio and menu owners must fulfill requests.
 * This component enables no boot, controls or New Game by itself. Unreachable retail phase4
 * is not a supported debug-stage selector. Repeated draws never call this function. */
bool SotnTitleScreen_Tick(SotnTitleScreen *state, SotnTitleScreenInput input, Uint32 *random,
                          SotnTitleScreenEvents *out);
#endif
