#ifndef SOTN_HELLFIRE_CAST_H
#define SOTN_HELLFIRE_CAST_H
#include <SDL3/SDL.h>
/** @brief Selected original Hellfire player controller, with no factory or player ownership. */
typedef struct SotnHellfireCast
{
    Sint32 velocity_x;
    Uint16 phase;
    bool facing_left, finished;
} SotnHellfireCast;
/** @brief Actual preceding player collision/animation and original shared signal.
 * root_created is the real phase-zero factory result, never a capacity guess. normal_phase
 * is the actual normal helper's retained shield substate (zero or two). Positions are screen words.
 */
typedef struct SotnHellfireCastInput
{
    Uint32 buttons, movement_flags;
    Uint16 floor_center, floor_right, floor_left, signal, pose, normal_phase;
    Sint16 x, remaining;
    bool root_created;
} SotnHellfireCastInput;
/** @brief Ordered calls required at the original controller boundaries. */
typedef enum SotnHellfireCastActionKind
{
    SOTN_HELLFIRE_FACTORY,
    SOTN_HELLFIRE_NORMAL,
    SOTN_HELLFIRE_ANIMATION,
    SOTN_HELLFIRE_USE_TIMER,
    SOTN_HELLFIRE_RESTORE_FLASHES
} SotnHellfireCastActionKind;
typedef struct SotnHellfireCastAction
{
    SotnHellfireCastActionKind kind;
    Uint16 value;
} SotnHellfireCastAction;
/** @brief Ordered intents, not completed factory/helper/animation operations. */
typedef struct SotnHellfireCastEvents
{
    SotnHellfireCastAction actions[6];
    Uint8 count;
    bool turned;
} SotnHellfireCastEvents;
/** @brief Advance one selected original PlayerStepHellfire controller update.
 * @param cast Zero except actual velocity/facing at original state34 entry.
 * @param input Real owner snapshots, including phase-zero factory outcome.
 * @param out Receives ordered one-update requests. @return Success or SDL error preserving both.
 * The actual owner performs factory33 BEFORE supplying its result, and acknowledges its emitted
 * record without allocating twice. It must invoke real normal/animation/timer/palette-effect
 * helpers, collide/move/animate afterward and provide fresh child signals. This component owns no
 * MP, learning, invincibility, factories, children, gradients or native control availability. */
bool SotnHellfireCast_Tick(SotnHellfireCast *cast, SotnHellfireCastInput input,
                           SotnHellfireCastEvents *out);
#endif
