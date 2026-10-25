#ifndef SOTN_WORLD_H
#define SOTN_WORLD_H
#include <SDL3/SDL.h>
enum
{
    SOTN_CASTLE_FLAGS = 0x300,
    SOTN_CASTLE_MAP = 0x800,
    SOTN_RELICS = 30
};
/** @brief Original-sized persistent castle flags and packed exploration, without game artwork. */
typedef struct SotnWorldState
{
    Uint8 flags[SOTN_CASTLE_FLAGS], map[SOTN_CASTLE_MAP];
    Uint8 relics[SOTN_RELICS]; /**< Original found/active bits, retained by v6 ordinary saves. */
    Uint16 rooms;
    bool laboratory_boss_visited;
    Uint32 laboratory_boss_time;
} SotnWorldState;
/** @brief Original relic identities used by the supported progression services. */
typedef enum SotnRelicId
{
    SOTN_RELIC_WOLF = 4,
    SOTN_RELIC_CUBE = 10,
    SOTN_RELIC_SPIRIT = 11,
    SOTN_RELIC_LEAP = 13,
    SOTN_RELIC_SCROLL = 15,
    SOTN_RELIC_JEWEL = 16
} SotnRelicId;
/** @brief Original US permanent stage flag indices, not private table contents. */
typedef enum SotnWorldFlag
{
    SOTN_ENTRANCE_DEATH = 53,
    SOTN_ENTRANCE_ON = 55,
    SOTN_ENTRANCE_STAIRS = 56,
    SOTN_LAB_SECRET_WALL = 129,
    SOTN_LAB_SECRET_FLOOR = 130,
    SOTN_LAB_CANNON = 131,
    SOTN_LAB_BOSSES = 132,
    SOTN_LAB_MARIA = 133
} SotnWorldFlag;
/** @brief Reveal the original packed map cell and count a first visit exactly once.
 * @param state Borrowed initialized writable progress. @param x Castle cell 0..63.
 * @param y Castle cell 0..63. @param inverted Select original/reversed castle.
 * @return Success or SDL error, leaving state unchanged on invalid coordinates. */
bool SotnWorld_Visit(SotnWorldState *state, Sint32 x, Sint32 y, bool inverted);
/** @brief Validate exploration count and supported permanent event bounds.
 * @param state Borrowed saved progress. @return Success or SDL error. */
bool SotnWorld_Validate(const SotnWorldState *state);
#endif
