#ifndef SOTN_SUBWEAPON_ENTITY_H
#define SOTN_SUBWEAPON_ENTITY_H
#include <sotn/collision.h>
#include <sotn/game_data.h>
#include <sotn/spell_target.h>

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
    SOTN_REBOUND_STONE_SPARKS = 10, /**< Bounce factory: three profile0 tink sparks. */
    SOTN_SUBWEAPON_AGUNEA = 9,
    SOTN_AGUNEA_BLUEPRINT = 22,
    SOTN_AGUNEA_ENTITY = 17,
    SOTN_AGUNEA_ROWS = 1, /**< AllocPrimitives(PRIM_GT4, 1), retyped LINE_G2. */
    /** Native key for E_SUBWPN_AGUNEA_HIT_ENEMY: its original ext.subweaponId is zero, so
     * CheckSubwpnChainLimit never counts it. */
    SOTN_AGUNEA_BOLT = 0x112,
    SOTN_AGUNEA_BOLT_BLUEPRINT = 23,
    SOTN_AGUNEA_BOLT_ENTITY = 18,
    SOTN_AGUNEA_BOLT_ROWS = 40,    /**< AllocPrimitives(PRIM_GT4, 0x28), first free. */
    SOTN_AGUNEA_FLASH = 0x52002c,  /**< FACTORY(BP_BLINK_WHITE, 0x52). */
    SOTN_AGUNEA_OUTLINE = 0x2003d, /**< FACTORY(BP_61, 2): three light-blue player outlines. */
    SOTN_AGUNEA_HELD = 0x1080,     /**< PAD_UP | PAD_SQUARE (US words). */
    SOTN_SUBWEAPON_BIBLE = 5,
    SOTN_BIBLE_BLUEPRINT = 77,
    SOTN_BIBLE_ENTITY = 43,
    /** Native key for E_SUBWPN_BIBLE_TRAIL (entity 44, blueprint 79, subweaponId zero). */
    SOTN_BIBLE_TRAIL = 0x12c,
    SOTN_BIBLE_TRAIL_BLUEPRINT = 79,
    SOTN_BIBLE_TRAIL_ENTITY = 44
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

/** @brief Original E_SUBWPN_AGUNEA ext words and its LINE_G2 row. The line runs from the
 * throw point (unk80/unk82) to the bolt; both ends are room-local whole pixels that keep their
 * screen position (no DRAW_UNK02): the owner moves them with the camera after each callback. */
typedef struct SotnAgunea
{
    Sint32 start_x, start_y, end_x, end_y;
    Sint16 counter;         /**< unk7C. */
    Uint16 target;          /**< Raw stage slot (unkB8 at the hit), 0..191 below slot64. */
    Uint16 priority;        /**< Line priority: player + 2 at birth. */
    Uint16 draw_mode;       /**< 0x331 (additive), DRAW_HIDE (8) once blue drops below five. */
    Uint8 red, green, blue; /**< r1/g1/b1; r0/g0/b0 stay zero. */
    bool struck;            /**< unk84: the first lightning bolt costs no hearts. */
} SotnAgunea;

/** @brief Original E_SUBWPN_AGUNEA_HIT_ENEMY (factory parameter zero, so its segments are
 * sixteen pixels wide): forty GT4 rows (page 0x1A, palette 0x194; 0x15F
 * for its one white update) chained toward the parent bolt. Row 0 stays hidden; rows 1..end
 * are drawn (DRAW_COLORS | DRAW_UNK02). Coordinates are room-local pixel words; row 0 keeps its
 * screen position, so the owner moves it with the camera. Every vertex shares one grey. */
typedef struct SotnAguneaBolt
{
    Sint16 x[SOTN_AGUNEA_BOLT_ROWS][4], y[SOTN_AGUNEA_BOLT_ROWS][4];
    Sint16 angle, turn, count; /**< unk84, unk86, unk88. */
    Uint8 end;                 /**< Row index of prim2, the chain's growing end. */
    Uint8 grey;
    bool white;   /**< PAL_FILL_WHITE for the step2 flash update. */
    bool flashed; /**< step_s. */
} SotnAguneaBolt;

/** @brief Original E_SUBWPN_BIBLE ext words. The book orbits the player, then flies off in
 * screen space; it and its one GT4 row (page 0x1E, palette 0x17F, 16x24) keep their screen
 * position (no FLAG_POS_CAMERA_LOCKED / DRAW_UNK02), so the owner moves them with the camera. */
typedef struct SotnBible
{
    Sint16 count;  /**< unk7C: updates since the orbit began; launch at 0x12C. */
    Sint16 radius; /**< unk7E: grows to 0x30. */
    Sint16 angle;  /**< unk80. */
    Sint16 tilt;   /**< unk82, swinging between about +-0x200. */
    Sint16 swing;  /**< unk84: +-32. */
    Sint16 mode;   /**< unk86: 0 hidden, 1 orbit, 2 launched. */
    Sint32 velocity_y;
} SotnBible;

/** @brief Original E_SUBWPN_BIBLE_TRAIL: one fading half-transparent GT4 (page 0x1C, palette
 * 0x19D) placed once at birth around its room pixel position, which keeps its screen position. */
typedef struct SotnBibleTrail
{
    Sint32 x, y;  /**< Quad centre (birth position) in room pixels. */
    Sint16 count; /**< unk7C. */
    Sint16 grey;  /**< unk7E: 0x60, minus eight per update. */
} SotnBibleTrail;

/** @brief One DRA subweapon entity owned by the shared player runtime (slots32..47).
 * Coordinates are room-local 16.16 pixels; zero-initialize before the first callback. */
typedef struct SotnSubweaponEntity
{
    Sint32 x, y;
    Uint32 flags;
    Uint16 id, step, timer, priority;
    Sint16 hitbox_x, hitbox_y;
    Uint8 hitbox_width, hitbox_height;
    Sint32 velocity_x;
    bool facing_left, removed;
    union {
        SotnVibhutiParticle particles[SOTN_VIBHUTI_PARTICLES];
        SotnReboundStone stone;
        SotnAgunea agunea;
        SotnAguneaBolt bolt;
        SotnBible bible;
        SotnBibleTrail trail;
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
    Uint8 hit_flags;   /**< Attacker hitFlags from the previous combat pass. */
    bool allocated;    /**< The birth callback's real row reservation succeeded. */
    Uint16 hit_target; /**< Raw stage slot0..191 (minus 64) collision wrote to unkB8. */
    /** Actual pre-stage view of slots 64..255 in screen words; Agunea's retained target. */
    const SotnSpellTarget *targets;
    Sint32 player_x, player_y; /**< Actual live player 16.16 room position. */
    Sint16 player_hitbox_y;    /**< PLAYER.hitboxOffY. */
    Uint32 player_status;      /**< Original g_Player.status. */
    Uint16 buttons;            /**< g_Player.padPressed (US words). */
    Uint16 hearts;             /**< g_Status.hearts. */
    Uint16 bolt_cost;          /**< Agunea's five hearts after Heart broach division. */
    Uint16 parent_entity;      /**< Raw parent slot's entityId (the bolt's Agunea). */
    Sint32 parent_x, parent_y; /**< Raw parent's room pixel position. */
    Uint32 game_timer;         /**< g_GameTimer: the Bible swishes when it is a multiple of 10. */
} SotnSubweaponInput;

/** @brief Requests for the owner, committed only after a successful callback. */
typedef struct SotnSubweaponEvents
{
    Uint16 sound;
    Uint16 sound_after; /**< A second PlaySfx later in the same callback. */
    Uint8 use_ticks;
    bool disable_attack; /**< hitboxState = 0. */
    /** CreateEntFactoryFromEntity(self, factory_packed[i], 0), in order. */
    Uint8 factories;
    Uint32 factory_packed[2];
    Sint32 factory_x, factory_y; /**< The entity's 16.16 position at those calls. */
    Uint16 hearts_spent;         /**< g_Status.hearts decrement. */
} SotnSubweaponEvents;

/** @brief Rows a supported subweapon entity leases at birth (zero when unsupported). */
Uint16 SotnSubweaponEntity_Rows(Uint16 id);
/** @brief Run one DRA subweapon callback (EntitySubwpnThrownVibhuti, EntitySubwpnReboundStone,
 * EntitySubwpnAgunea, EntityAguneaHitEnemy, EntitySubwpnBible, EntitySubwpnBibleTrail).
 * @param entity Owned state. @param input Actual context; properties is required.
 * @param random Character libc cursor in original order. @param events Receives requests.
 * @return Success or SDL error without mutation. The owner reserves the rows before the birth
 * callback, clears hit flags after each callback, culls entities without KEEP_ALIVE_OFFCAMERA
 * and moves hidden rows with the screen. A failed birth reservation retires without RNG. */
bool SotnSubweaponEntity_Tick(SotnSubweaponEntity *entity, SotnSubweaponInput input, Uint32 *random,
                              SotnSubweaponEvents *events);
#endif
