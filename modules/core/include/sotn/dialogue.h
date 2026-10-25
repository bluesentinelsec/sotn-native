#ifndef SOTN_DIALOGUE_H
#define SOTN_DIALOGUE_H
#include <SDL3/SDL.h>
/** @brief Copied US dialogue presentation state. Text comes only from private runtime scripts. */
typedef struct SotnDialogueState
{
    char lines[5][25];
    Uint32 portraits[2]; /**< Stage-relative offsets, zero until loaded by the script. */
    Uint16 x, y, transition, transition_length, palette;
    Uint16 portrait_page; /**< Original 8-bit portrait page; zero selects the legacy 0x94 page. */
    Uint8 row, column, speaker, portrait_slot, fade;
    bool visible, mirrored, portrait_visible, background_enabled;
} SotnDialogueState;
#endif
