#ifndef SOTN_IMMUNITY_TEXT_H
#define SOTN_IMMUNITY_TEXT_H
#include <sotn/player_blink.h>

/** @brief Original entity37 parameter0 text, stored in its real shared child.
 * The other text parameters have separate producers and are not enabled here. */
typedef struct SotnImmunityText
{
    SotnPlayerEffectQuad quad;
    Sint32 x, y, velocity_x;
    Sint16 half_width, half_height, timer, angle;
    Uint8 phase;
    bool removed;
} SotnImmunityText;
/** @brief Advance the original immunity text with actual screen-space player/ground feedback.
 * @param text Actual child cursor. @param dra Private text metadata, never copied into source.
 * @param executable Private SDK trig tables. @param x Current player screen16.16 X.
 * @param y Current player screen16.16 Y. @param grounded Current or previous ground contact.
 * @param allocated Actual eighteen-row reservation outcome, used only at phase2.
 * @return Success or SDL error preserving the cursor. Birth and ground-wait return before
 * allocation; optional exhaustion retires the visual. No sound/RNG or substitute rows. */
bool SotnImmunityText_Tick(SotnImmunityText *text, SotnDataView dra, SotnDataView executable,
                           Sint32 x, Sint32 y, bool grounded, bool allocated);
/** @brief Translate live text/packet coordinates without advancing its controller. */
bool SotnImmunityText_Rebase(SotnImmunityText *text, Sint16 x, Sint16 y);
#endif
