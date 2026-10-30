#ifndef SOTN_DEATH_IMAGE_H
#define SOTN_DEATH_IMAGE_H
#include <sotn/game_data.h>

enum
{
    SOTN_DEATH_IMAGE_BYTES = 5120
};
/** @brief Original retained packed death image and common disintegration cursor.
 * Pixels belong to this copied runtime owner, never the immutable private artwork. */
typedef struct SotnDeathImage
{
    Uint8 pixels[SOTN_DEATH_IMAGE_BYTES];
    Uint16 remaining;
    Uint8 cursor;
    bool captured;
} SotnDeathImage;
/** @brief Begin the original common64-update image decay after an actual StoreImage copy.
 * @param image Actual runtime owner. @param pixels Complete original5120-byte capture.
 * @return Success or SDL error preserving output. This copies supplied pixels only; the owner
 * must actually capture the current display texture before calling, never private source art. */
bool SotnDeathImage_Begin(SotnDeathImage *image, SotnDataView pixels);
/** @brief Advance original state16/phase2 packed-nibble decay once without RNG or rendering.
 * @param image Actual captured image, with1..64 remaining updates.
 * @return Success or SDL error preserving the owner. The actual simulation owner must upload
 * the changed packed image before committing this copied cursor. Zero remaining requests the
 * original velocity/color/substate completion; it is not completed upload or game-over flow.
 * Menus, freeze, room rebases and repeated draws must not call this function. */
bool SotnDeathImage_Tick(SotnDeathImage *image);
#endif
