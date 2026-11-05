#ifndef SOTN_IMMUNITY_TEXT_H
#define SOTN_IMMUNITY_TEXT_H
#include <sotn/player_blink.h>

/** @brief Original entity37 immunity/poison/curse text in its actual shared child.
 * The sixteen tiles use rows2..17 of the same eighteen-row lease. */
typedef struct SotnImmunityText
{
    SotnPlayerEffectQuad quad, suffix;
    struct
    {
        Sint16 x, y;
        Uint16 draw_mode;
        Uint8 timer;
        bool active;
    } tiles[16];
    Sint32 x, y, velocity_x, target;
    Sint16 half_width, half_height, timer, angle;
    Uint16 sound;
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
/** @brief Advance actual text parameter0/1/2, with its original packed target and sound.
 * @param parameter Full child parameter: upper byte selects immunity0/poison1/curse2/stone3 or
 * medicine labels8..19; lower byte must be zero. Labels own the actual second GT4 from the same
 * lease. Other arguments have the same ownership as Tick. Sound is a one-update request for the
 * real shared audio service. */
bool SotnImmunityText_TickParameter(SotnImmunityText *text, SotnDataView dra,
                                    SotnDataView executable, Sint32 x, Sint32 y, bool grounded,
                                    bool allocated, Uint16 parameter);
/** @brief Apply the original camera shift only to tile packets still carrying flag2.
 * Text coordinates and its screen-fixed GT4 do not follow the camera. No tick/sound. */
bool SotnImmunityText_Rebase(SotnImmunityText *text, Sint16 x, Sint16 y);
/** @brief Apply the room-entry flag100 packet shift without advancing text coordinates.
 * Unlike an ordinary camera move, room entry translates the GT4 and holds its controller. */
bool SotnImmunityText_ChangeRoom(SotnImmunityText *text, Sint16 x, Sint16 y);
#endif
