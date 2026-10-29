#ifndef SOTN_SPIRIT_SPELL_H
#define SOTN_SPIRIT_SPELL_H
#include <sotn/player_blink.h>
enum
{
    SOTN_SPIRIT_TRAILS = 8,
    SOTN_SPIRIT_ROWS = 16,
    SOTN_SPIRIT_PORTAL_ROWS = 9
};
/** @brief Original paired metadata and retained textured trail packet. */
typedef struct SotnSpiritTrail
{
    SotnPlayerEffectQuad quad;
    Sint16 x, y, angle, scale, state;
} SotnSpiritTrail;
/** @brief One actual spirit child; the owner supplies sixteen real primitive rows. */
typedef struct SotnSpirit
{
    SotnSpiritTrail trails[SOTN_SPIRIT_TRAILS];
    Sint32 x, y, vx, vy;
    Uint32 flags;
    Sint16 angle, wobble, age, cursor, rotation;
    Uint16 parameter, phase;
    bool facing_left, removed;
} SotnSpirit;
/** @brief Live player, real row allocation, private tables and retained original target slot. */
typedef struct SotnSpiritInput
{
    SotnDataView dra, executable;
    Sint32 x, y; /**< Screen16.16 source on actual birth. */
    Sint16 target_x, target_y;
    Uint16 frame; /**< Current pre-animation sprite;0 is original birth, then1..8. */
    bool allocated, player_facing_left, target_bound, target_alive;
} SotnSpiritInput;
/** @brief Requests for actual attack initialization and original target selection. */
typedef struct SotnSpiritEvents
{
    Uint16 spell;
    bool initialize_attack, select_target;
} SotnSpiritEvents;
/** @brief Advance original spirit motion and eight retained trail packets once.
 * @param spirit Actual owned child, initially zeroed except original full parameter.
 * @param input Copied current actual-owner context. @param out Receives one-update intents.
 * @return Success or SDL error preserving state/output. Allocation failure retires before
 * initialization. The owner applies actual spell/group/RNG metadata, target-slot selection,
 * animation, row leases, culling and native combat. No gameplay controls are enabled here. */
bool SotnSpirit_Tick(SotnSpirit *spirit, SotnSpiritInput input, SotnSpiritEvents *out);
/** @brief Rebase body and retained metadata/packets without advancing motion or fading. */
bool SotnSpirit_Translate(SotnSpirit *spirit, Sint32 dx, Sint32 dy);
/** @brief Original background portal, borrowing an actual nine-row owner. */
typedef struct SotnSpiritPortal
{
    SotnPlayerEffectQuad quads[8];
    Sint32 x, y;
    Uint32 flags;
    Sint16 timer, delay;
    Uint16 parameter, phase;
    bool removed;
} SotnSpiritPortal;
/** @brief Actual source, player height, row allocation and private shape/SDK tables. */
typedef struct SotnSpiritPortalInput
{
    SotnDataView dra, executable;
    Sint32 x, y;
    Sint16 player_y;
    bool allocated;
} SotnSpiritPortalInput;
/** @brief Original freeze/flash/capture/factory intents; none manufacture readiness. */
typedef struct SotnSpiritPortalEvents
{
    Uint32 factories[2];
    Sint16 capture_x, capture_y; /**< Original63x63 screen source; destination512,448. */
    Uint16 sound;
    Uint8 freeze, flash;
    bool capture, release_freeze;
} SotnSpiritPortalEvents;
/** @brief Advance the original portal, including delayed allocation and frame capture request.
 * @param portal Owned actual root, initially zeroed except full parameter.
 * @param input Actual source/current player/row context. @param out Receives current intents.
 * @return Success or SDL error preserving state/output. Original allocation failure retires
 * WITHOUT releasing its freeze request; an eventual native owner must handle that boundary
 * explicitly, never fake capture/factory readiness. This component enables no spell controls. */
bool SotnSpiritPortal_Tick(SotnSpiritPortal *portal, SotnSpiritPortalInput input,
                           SotnSpiritPortalEvents *out);
#endif
