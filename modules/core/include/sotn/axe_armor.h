#ifndef SOTN_AXE_ARMOR_H
#define SOTN_AXE_ARMOR_H
#include <sotn/animation.h>
#include <sotn/game_math.h>

/** @brief Weapon45's two original retained words, outside ordinary saves. */
typedef struct SotnAxeArmor
{
    Uint32 angle;
    Sint32 recovery;
} SotnAxeArmor;

/** @brief Live player fields borrowed by the original Axe Armor overlay callbacks. */
typedef struct SotnAxeArmorFrame
{
    SotnAnimation animation;
    Sint32 velocity_x, velocity_y;
    Uint32 movement_flags;
    Uint16 step, substep, draw_flags;
    Sint16 rotation, pivot_y;
    Uint8 opacity;
    bool facing_left, knock_right, turned;
} SotnAxeArmorFrame;

/** @brief Actual preceding collision, physical input and gameplay clock snapshots. */
typedef struct SotnAxeArmorInput
{
    SotnDataView executable;
    Uint32 contacts, clock;
    Uint16 pressed, tapped, simulated_mode;
    bool tight_space;
} SotnAxeArmorInput;

/** @brief Ordered requests only; the native owner supplies real factories and playback. */
typedef struct SotnAxeArmorEvents
{
    Uint16 sounds[2];
    Uint32 factories[2];
    Uint8 sound_count, factory_count;
} SotnAxeArmorEvents;

/** @brief Apply weapon45's entry callback without granting equipment or loading artwork.
 * @param frame Borrowed actual player motion/animation. @param contacts Real preceding contacts.
 * @return Success with standing40 or airborne42, preserving state on invalid input. */
bool SotnAxeArmor_Enter(SotnAxeArmorFrame *frame, Uint32 contacts);

/** @brief Advance original stand40/walk41/jump42/hit43/script50 once before common animation.
 * @param state Actual overlay words. @param frame Actual player fields, committed on success.
 * @param input Actual controller/collision/SDK context. @param out Receives optional intents.
 * @return Success atomically; no private code/data is executed or embedded. Drawing, collisions,
 * attacks, equipment transitions, factory outcomes and availability require their native owners. */
bool SotnAxeArmor_Tick(SotnAxeArmor *state, SotnAxeArmorFrame *frame, SotnAxeArmorInput input,
                       SotnAxeArmorEvents *out);
#endif
