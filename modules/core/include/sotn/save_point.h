#ifndef SOTN_SAVE_POINT_H
#define SOTN_SAVE_POINT_H

#include <sotn/player.h>

/** @brief Selected original save-point phases, with a native slot-selection adapter. */
typedef enum SotnSavePointPhase
{
    SOTN_SAVE_POINT_IDLE,
    SOTN_SAVE_POINT_CENTER,
    SOTN_SAVE_POINT_SLOT,
    SOTN_SAVE_POINT_CONFIRM,
    SOTN_SAVE_POINT_GROW,
    SOTN_SAVE_POINT_SPIN,
    SOTN_SAVE_POINT_SCATTER,
    SOTN_SAVE_POINT_COFFIN,
    SOTN_SAVE_POINT_WRITE,
    SOTN_SAVE_POINT_SAVED,
    SOTN_SAVE_POINT_EXIT,
    SOTN_SAVE_POINT_ERROR
} SotnSavePointPhase;

/** @brief Initialized save-point simulation state; contains no filesystem or rendering ownership.
 */
typedef struct SotnSavePoint
{
    SotnSavePointPhase phase;
    Uint16 angle, scale, speed, scatter, previous_buttons, occupied;
    Uint8 slot, slots;
    Uint32 ticks;
    bool yes, armed;
} SotnSavePoint;

/** @brief Requests emitted once for the scene's health/audio/storage services. */
typedef struct SotnSavePointAction
{
    Uint16 sound;
    bool heal, write;
} SotnSavePointAction;

/** @brief Reset a room's save point, retaining a selected native slot.
 * @param point Writable state. @param slots Native slot count, 1..15.
 * @param slot Selected slot, 1..slots. @param occupied Bitset of existing slot paths.
 * @return Success, or false with SDL_GetError() and unchanged state on invalid input. */
bool SotnSavePoint_Reset(SotnSavePoint *point, Uint8 slots, Uint8 slot, Uint16 occupied);
/** @brief Substitute original centering/look-up input while the save point owns control.
 * @param point Borrowed state. @param player Current player snapshot. @param buttons User input.
 * @return Buttons to pass to the player; menu actions are consumed separately by Tick. */
Uint16 SotnSavePoint_Buttons(const SotnSavePoint *point, SotnPlayerState player, Uint16 buttons);
/** @brief Advance activation, manual confirmation, and selected original coffin clocks.
 * @param point Borrowed writable state. @param player Post-player-tick state.
 * @param buttons User input, distinct from simulated centering buttons.
 * @param out Receives one-tick requests, unchanged on failure.
 * @return Success, or false with SDL_GetError(); no writes occur inside the core. */
bool SotnSavePoint_Tick(SotnSavePoint *point, SotnPlayerState player, Uint16 buttons,
                        SotnSavePointAction *out);
/** @brief Complete a requested write; failure remains visible and can be retried or cancelled.
 * @param point Borrowed state in WRITE phase. @param success Checked storage result.
 * @return Success, or false with SDL_GetError() for invalid call order. */
bool SotnSavePoint_Complete(SotnSavePoint *point, bool success);
/** @brief Query whether native slot/confirmation/acknowledgment controls own menu input.
 * @param point Borrowed state, or NULL. @return True for a visible input dialog. */
bool SotnSavePoint_Menu(const SotnSavePoint *point);

#endif
