#ifndef SOTN_SUBWEAPON_ENTITY_H
#define SOTN_SUBWEAPON_ENTITY_H
#include <sotn/collision.h>
#include <sotn/game_data.h>

enum
{
    SOTN_SUBWEAPON_REBOUND_STONE = 7,
    SOTN_SUBWEAPON_VIBHUTI = 8,
    SOTN_VIBHUTI_PARTICLES = 12,
    SOTN_VIBHUTI_ROWS = 13, /**< Twelve particles and the zero-width terminator TILE. */
    SOTN_VIBHUTI_BLUEPRINT = 26,
    SOTN_VIBHUTI_ENTITY = 12,
    SOTN_REBOUND_STONE_LINES = 16, /**< AllocPrimitives(PRIM_LINE_G2, 16), first free. */
    SOTN_REBOUND_STONE_BLUEPRINT = 24,
    SOTN_REBOUND_STONE_ENTITY = 20,
    SOTN_REBOUND_STONE_SPARKS = 10 /**< Bounce factory: three profile0 tink sparks. */
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

/** @brief One original Rebound Stone LINE_G2 trail segment in room-local whole pixels.
 * Both ends share one grey (PGREY(prim, 0) = PGREY(prim, 1)); draw_mode 0x33 or 0x3B hidden. */
typedef struct SotnReboundLine
{
    Sint32 x0, y0, x1, y1;
    Uint16 draw_mode;
    Uint8 grey, timer;
} SotnReboundLine;

/** @brief Original Rebound Stone ext words. */
typedef struct SotnReboundStone
{
    SotnReboundLine lines[SOTN_REBOUND_STONE_LINES];
    Sint16 angle, life;
    Sint16 bounces; /**< unk80: the trail segment index; fifteen ends the flight. */
    Sint16 embed;   /**< unk84: updates held still when born inside solid ground. */
} SotnReboundStone;

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
    union {
        SotnVibhutiParticle particles[SOTN_VIBHUTI_PARTICLES];
        SotnReboundStone stone;
    };
} SotnSubweaponEntity;

/** @brief func_8011A290 at its original point in the callback: roll func_800FE3C4's attack with
 * the character libc cursor and take func_80118894's group. */
typedef bool (*SotnSubweaponProperties)(void *context, Uint32 *random);

/** @brief Actual context for one callback. */
typedef struct SotnSubweaponInput
{
    SotnDataView executable;    /**< Private SDK rsin/rcos tables. */
    SotnCollisionMap collision; /**< Current room collision (CheckCollision). */
    SotnSubweaponProperties properties;
    void *context;
    Sint32 screen_x, screen_y; /**< Room pixel at screen origin during this update. */
    Uint16 player_priority;
    bool player_facing_left;
    Uint8 hit_flags; /**< Attacker hitFlags from the previous combat pass. */
    bool allocated;  /**< The birth callback's real row reservation succeeded. */
} SotnSubweaponInput;

/** @brief Requests for the owner, committed only after a successful callback. */
typedef struct SotnSubweaponEvents
{
    Uint16 sound;
    Uint8 use_ticks;
    bool disable_attack; /**< hitboxState = 0. */
    bool factory;        /**< CreateEntFactoryFromEntity(self, factory_packed, 0). */
    Uint16 factory_packed;
    Sint32 factory_x, factory_y; /**< The entity's 16.16 position at that call. */
} SotnSubweaponEvents;

/** @brief Rows a supported subweapon entity leases at birth (zero when unsupported). */
Uint16 SotnSubweaponEntity_Rows(Uint16 id);
/** @brief Run one DRA subweapon callback (EntitySubwpnThrownVibhuti, EntitySubwpnReboundStone).
 * @param entity Owned state. @param input Actual context; properties is required.
 * @param random Character libc cursor in original order. @param events Receives requests.
 * @return Success or SDL error without mutation. The owner reserves the rows before the birth
 * callback, clears hit flags after each callback, culls entities without KEEP_ALIVE_OFFCAMERA
 * and moves hidden rows with the screen. A failed birth reservation retires without RNG. */
bool SotnSubweaponEntity_Tick(SotnSubweaponEntity *entity, SotnSubweaponInput input, Uint32 *random,
                              SotnSubweaponEvents *events);
#endif
