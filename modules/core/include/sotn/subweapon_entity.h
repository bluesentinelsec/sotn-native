#ifndef SOTN_SUBWEAPON_ENTITY_H
#define SOTN_SUBWEAPON_ENTITY_H
#include <sotn/collision.h>
#include <sotn/game_data.h>

enum
{
    SOTN_SUBWEAPON_VIBHUTI = 8,
    SOTN_VIBHUTI_PARTICLES = 12,
    SOTN_VIBHUTI_ROWS = 13, /**< Twelve particles and the zero-width terminator TILE. */
    SOTN_VIBHUTI_BLUEPRINT = 26,
    SOTN_VIBHUTI_ENTITY = 12
};

/** @brief One original FakePrim ash TILE. Positions are room-local 16.16 pixels whose whole
 * words mirror x0/y0; DRAW_UNK02 (draw_mode 2) rows follow the world, DRAW_HIDE (8) rows keep
 * their screen position. */
typedef struct SotnVibhutiParticle
{
    Sint32 x, y, velocity_x, velocity_y;
    Uint16 draw_mode;
    Uint8 width, height;
    bool falling; /**< FakePrim delay: nonzero until the ash lands. */
} SotnVibhutiParticle;

/** @brief One DRA subweapon entity owned by the shared player runtime (slots32..47).
 * Coordinates are room-local 16.16 pixels; zero-initialize before the first callback. */
typedef struct SotnSubweaponEntity
{
    Sint32 x, y;
    Uint32 flags;
    Uint16 id, step, timer, priority;
    Sint16 hitbox_y;
    Uint8 hitbox_width, hitbox_height;
    bool facing_left, removed;
    SotnVibhutiParticle particles[SOTN_VIBHUTI_PARTICLES];
} SotnSubweaponEntity;

/** @brief Actual context for one callback. */
typedef struct SotnSubweaponInput
{
    SotnDataView executable;    /**< Private SDK rsin/rcos tables. */
    SotnCollisionMap collision; /**< Current room collision (CheckCollision). */
    Uint16 player_priority;
    Uint8 hit_flags; /**< Attacker hitFlags from the previous combat pass. */
    bool allocated;  /**< The birth callback's real func_800EDB58 reservation succeeded. */
} SotnSubweaponInput;

/** @brief Requests for the owner, committed only after a successful callback. */
typedef struct SotnSubweaponEvents
{
    Uint16 sound;
    Uint8 use_ticks;
} SotnSubweaponEvents;

/** @brief Run one EntitySubwpnThrownVibhuti callback.
 * @param entity Owned state. @param input Actual context. @param random Character libc cursor:
 * the birth callback draws two values per particle after the owner's property roll.
 * @param events Receives requests. @return Success or SDL error without mutation.
 * The owner reserves the rows, rolls func_8011A290's properties and attack group before birth,
 * clears the hit flags after each callback, culls offscreen entities and moves hidden rows with
 * the screen. A failed birth reservation retires the entity without RNG. */
bool SotnSubweaponEntity_Tick(SotnSubweaponEntity *entity, SotnSubweaponInput input, Uint32 *random,
                              SotnSubweaponEvents *events);
#endif
