#ifndef SOTN_SHIELD_H
#define SOTN_SHIELD_H
#include <sotn/equipment.h>
/** @brief Independent original shield animation; contains no borrowed pointers. */
typedef struct SotnShield
{
    Uint16 phase, animation, pose;
    Sint32 remaining;
    SotnWeaponFrame frame;
} SotnShield;
/** @brief Advance selected Alucard-shield raise, movement, hold, and lowering frames.
 * @param weapons Borrowed private WEAPON0.BIN. @param player_animation Original Alucard animation.
 * @param player_pose Original Alucard pose. @param crouched Original crouching condition.
 * @param held Shield hand button held. @param disabled Hit/death or unequipped shield.
 * @param cursor Owned initialized cursor, unchanged on malformed data.
 * @return Success or SDL error. Shield-rod spells are outside this ordinary shield path. */
bool SotnShield_Tick(SotnDataView weapons, Uint16 player_animation, Uint16 player_pose,
                     bool crouched, bool held, bool disabled, SotnShield *cursor);
#endif
