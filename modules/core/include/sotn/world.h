#ifndef SOTN_WORLD_H
#define SOTN_WORLD_H
#include <SDL3/SDL.h>
enum
{
    SOTN_CASTLE_FLAGS = 0x300,
    SOTN_CASTLE_MAP = 0x800,
    SOTN_RELICS = 30,
    SOTN_TIME_ATTACK_EVENTS = 32
};
/** @brief Original-sized persistent castle flags and packed exploration, without game artwork. */
typedef struct SotnWorldState
{
    Uint8 flags[SOTN_CASTLE_FLAGS], map[SOTN_CASTLE_MAP];
    Uint8 relics[SOTN_RELICS]; /**< Original found/active bits, retained by v6 ordinary saves. */
    Uint16 rooms;
    Uint8 game_clear; /**< Original completed-game unlock flag; not a development grant. */
    Uint32 time_attack[SOTN_TIME_ATTACK_EVENTS]; /**< Original HHMMSS defeat records. */
    Uint32 time_attack_visited;                  /**< Original event-visited mask. */
    Uint32
        cutscenes_seen; /**< Original per-save completion word; retained across in-session loads. */
} SotnWorldState;
/** @brief Original event identities used by integrated boss services. */
typedef enum SotnTimeAttackEvent
{
    SOTN_TIME_OLROX = 1,
    SOTN_TIME_DOPPLEGANGER = 2,
    SOTN_TIME_LAB_BOSSES = 6,
    SOTN_TIME_HIPPOGRYPH = 7,
    SOTN_TIME_LESSER_DEMON = 17,
    SOTN_TIME_GALAMOTH = 21,
    SOTN_TIME_LIBRARIAN = 25
} SotnTimeAttackEvent;
/** @brief Set an original HHMMSS event record only when its existing value is zero.
 * @param state Borrowed initialized progress. @param event Original index, 0..31.
 * @param seconds Elapsed simulation seconds, bounded to 99,999 hours.
 * @return Success or SDL error; invalid input leaves progress unchanged.
 * @note Original time zero remains zero; this does not synthesize a defeated flag. */
bool SotnWorld_RecordTime(SotnWorldState *state, Uint16 event, Uint32 seconds);
/** @brief Retain the original visited bit without marking a boss defeated.
 * @param state Borrowed initialized progress. @param event Original index, 0..31.
 * @return Success or SDL error, unchanged on failure. */
bool SotnWorld_VisitEvent(SotnWorldState *state, Uint16 event);
/** @brief Original relic identities used by the supported progression services. */
typedef enum SotnRelicId
{
    SOTN_RELIC_BAT = 0,
    SOTN_RELIC_FIRE_BAT = 1,
    SOTN_RELIC_ECHO_BAT = 2,
    SOTN_RELIC_FORCE_ECHO = 3,
    SOTN_RELIC_WOLF = 4,
    SOTN_RELIC_MIST = 7,
    SOTN_RELIC_POWER_MIST = 8,
    SOTN_RELIC_GAS_CLOUD = 9,
    SOTN_RELIC_CUBE = 10,
    SOTN_RELIC_SPIRIT = 11,
    SOTN_RELIC_GRAVITY = 12,
    SOTN_RELIC_LEAP = 13,
    SOTN_RELIC_SCROLL = 15,
    SOTN_RELIC_JEWEL = 16
} SotnRelicId;
/** @brief Original US permanent stage flag indices, not private table contents. */
typedef enum SotnWorldFlag
{
    SOTN_CLOCK_FLOOR = 0,
    SOTN_OUTER_ELEVATOR = 16,
    SOTN_OUTER_PRESSURE = 17,
    SOTN_NO2_SECRET_WALL = 32,
    SOTN_NO2_SECRET_CEILING = 33,
    SOTN_ENTRANCE_ROCKS = 51,
    SOTN_ENTRANCE_DEATH = 53,
    SOTN_ENTRANCE_ON = 55,
    SOTN_ENTRANCE_STAIRS = 56,
    SOTN_ENTRANCE_JEWEL_DOOR = 58,
    SOTN_CHAPEL_SHORTCUT = 96,
    SOTN_MARIA_HIPPOGRYPH = 98,
    SOTN_CHAPEL_MARIA = 99,
    SOTN_LIBRARY_SHELF = 112,
    SOTN_LIBRARY_MET = 114,
    SOTN_LIBRARY_DROPS = 116,
    SOTN_LIBRARY_MAP = 115,
    SOTN_INVERTED_UNLOCKED = 150,
    SOTN_RICHTER_AFTER_MINOTAUR = 180,
    SOTN_MARIA_CENTER = 216,
    SOTN_DEATH_FIGHT = 220,
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
/** @brief Reveal purchased-map rectangles from a private bounded stream, atomically.
 * @param state Owned progress. @param bytes Borrowed x/y/byte-width/height rectangles,
 * followed by LSB-first row bits and an x=255 terminator. @param size Available bytes.
 * @param inverted Original castle half. @return Success or SDL error without partial reveal.
 * @note Reveals only high map bits; visited cells and room count remain unchanged. */
bool SotnWorld_Reveal(SotnWorldState *state, const Uint8 *bytes, size_t size, bool inverted);
/** @brief Validate exploration count and supported permanent event bounds.
 * @param state Borrowed saved progress. @return Success or SDL error. */
bool SotnWorld_Validate(const SotnWorldState *state);
#endif
