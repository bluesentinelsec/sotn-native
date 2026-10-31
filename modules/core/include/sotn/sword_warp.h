#ifndef SOTN_SWORD_WARP_H
#define SOTN_SWORD_WARP_H
#include <sotn/game_data.h>

/** @brief Original quarter-circle weapon command, retained outside ordinary saves. */
typedef struct SotnWeaponCommand
{
    Sint16 phase, timer;
    bool facing_left;
} SotnWeaponCommand;
/** @brief Advance the original exact-direction QCF recognizer once before player dispatch.
 * @param command Actual transient command. @param buttons Original action word.
 * @param facing_left Current facing. @param cramped Actual restoration-space result.
 * Completion is phase255; the owner still needs equipped metadata, MP and real factories. */
void SotnWeaponCommand_Tick(SotnWeaponCommand *command, Uint16 buttons, bool facing_left,
                            bool cramped);

/** @brief Actual Alucard-sword special's slot16 image; coordinates are signed16.16. */
typedef struct SotnSwordWarp
{
    Sint32 x, y, velocity_x, velocity_y;
    Uint16 phase, frame, blend;
    bool facing_left, removed;
} SotnSwordWarp;
/** @brief One root update's optional child request; never successful allocation. */
typedef struct SotnSwordWarpEvents
{
    bool strike;
} SotnSwordWarpEvents;
/** @brief Advance weapon43 callback04 without moving the actual player or allocating children.
 * @param state Actual initialized slot16 cursor, phase0..3. @param pose Live player pose.
 * @param remaining Live pose timer. @param frame Live player image word.
 * @param out Receives one captured strike request. @return Success or SDL error, atomically.
 * Birth sets velocities without moving; the original pose5/14/19 gates retain their timing.
 * The native owner must supply actual factory/slot lifetime, artwork and draw attributes. */
bool SotnSwordWarp_Tick(SotnSwordWarp *state, Uint16 pose, Sint32 remaining, Uint16 frame,
                        SotnSwordWarpEvents *out);

/** @brief Actual weapon43 callback08 private animation cursor, independent of the player. */
typedef struct SotnSwordStrike
{
    Uint16 phase, pose, frame;
    Sint16 remaining;
    Uint8 height;
    bool removed;
} SotnSwordStrike;
/** @brief Advance strike callback and its subsequent original four-byte animation update.
 * @param state Actual child cursor. @param overlay Borrowed immutable weapon43 executable.
 * @param sound Receives the original swish request or zero. @return Success or SDL error,
 * preserving state/output on malformed private timelines. Hitbox height observes the preceding
 * image, including zero-height birth; no original animation/table bytes are embedded. */
bool SotnSwordStrike_Tick(SotnSwordStrike *state, SotnDataView overlay, Uint16 *sound);
#endif
