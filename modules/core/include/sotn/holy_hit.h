#ifndef SOTN_HOLY_HIT_H
#define SOTN_HOLY_HIT_H
#include <sotn/player_blink.h>
#include <sotn/player_factory.h>
enum
{
    SOTN_HOLY_HIT_ROWS = 16
};
/** @brief Actual DRA global points shared by every entity47, not a particle pool. */
typedef struct SotnHolyHitShared
{
    struct
    {
        Sint16 x, y;
    } points[SOTN_HOLY_HIT_ROWS];
} SotnHolyHitShared;
/** @brief Original entity47 and its sixteen real GT4 packets. */
typedef struct SotnHolyHit
{
    SotnPlayerEffectQuad quads[SOTN_HOLY_HIT_ROWS];
    struct
    {
        Uint8 phase, delay, frame, age;
    } animation[SOTN_HOLY_HIT_ROWS];
    Sint32 x, y;
    Uint8 phase;
    bool removed;
} SotnHolyHit;
/** @brief Actual player/engine feedback and real sixteen-row reservation outcome. */
typedef struct SotnHolyHitInput
{
    SotnDataView dra;
    SotnPlayerFactorySource source;
    Sint32 player_x, player_y;
    Sint32 entity_shift_x, entity_shift_y; /**< Original pre-controller player-lock shift. */
    Sint16 hitbox_x, hitbox_y;
    Uint16 priority;
    Uint32 status;
    bool allocated;
} SotnHolyHitInput;
/** @brief Advance original holy-hit birth/animation/status once with its actual shared globals.
 * @param effect Zeroed actual child or retained owner. @param shared Single actual DRA point table.
 * @param input Borrowed private UV bytes, live player/engine feedback and real row outcome.
 * @param random Actual character RNG, charged64 calls only after successful allocation.
 * @return Success or SDL error preserving effect/globals/RNG. Optional row failure retires before
 * RNG. All sprite phases completing does not retire the child: actual status10000 owns that gate.
 * This component allocates no entity/rows and provides no damage/contact readiness. */
bool SotnHolyHit_Tick(SotnHolyHit *effect, SotnHolyHitShared *shared, SotnHolyHitInput input,
                      Uint32 *random);
/** @brief Shift cached flag2 camera or flag100 room packets and the actual entity position.
 * Shared points do not move. The owner invokes the ordinary camera pass after the controller. */
bool SotnHolyHit_Rebase(SotnHolyHit *effect, Sint16 x, Sint16 y, bool room);
/** @brief Apply an actual stage-object player-lock shove without advancing any effect state.
 * @param x Signed16.16 displacement. @param y Signed16.16 displacement. */
bool SotnHolyHit_Displace(SotnHolyHit *effect, Sint32 x, Sint32 y);
#endif
