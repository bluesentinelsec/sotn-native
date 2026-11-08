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
    SOTN_BIBLE_TRAIL_ENTITY = 44,
    SOTN_SUBWEAPON_CROSS = 4,
    SOTN_CROSS_BLUEPRINT = 6,
    SOTN_CROSS_ENTITY = 8,
    /** Subweapon 12, g_SubwpnDefs[4].crashId: EntityGiantSpinningCross's ext.subweaponId. */
    SOTN_GIANT_CROSS = 12,
    SOTN_GIANT_CROSS_BLUEPRINT = 7,
    SOTN_GIANT_CROSS_ENTITY = 7,
    SOTN_GIANT_CROSS_ROWS = 46, /**< func_800EDB58(PRIM_GT4, LEN(D_800B0CB4)), contiguous. */
    /** Native key for EntitySubwpnCrashCrossParticles (entity 9, blueprint 8, subweaponId 0). */
    SOTN_CROSS_PARTICLES = 0x109,
    SOTN_CROSS_PARTICLES_BLUEPRINT = 8,
    SOTN_CROSS_PARTICLES_ENTITY = 9,
    SOTN_CROSS_PARTICLE_ROWS = 64, /**< AllocPrimitives(PRIM_GT4, 64), first free. */
    SOTN_SUBWEAPON_DAGGER = 1,
    SOTN_DAGGER_BLUEPRINT = 3,
    SOTN_DAGGER_ENTITY = 4,
    SOTN_DAGGER_ROWS = 2, /**< AllocPrimitives(PRIM_GT4, 2): the blade, then a LINE_G2 trail. */
    SOTN_SUBWEAPON_AXE = 2,
    SOTN_AXE_BLUEPRINT = 9,
    SOTN_AXE_ENTITY = 10,
    SOTN_AXE_ROWS = 5, /**< AllocPrimitives(PRIM_GT4, 5): the axe and four afterimages. */
    SOTN_SUBWEAPON_HOLY_WATER = 3, /**< The bottle: a bank-9 sprite without rows. */
    SOTN_HOLY_WATER_BLUEPRINT = 27,
    SOTN_HOLY_WATER_ENTITY = 22,
    /** Subweapon 11, g_SubwpnDefs[3].crashId: EntitySubwpnHolyWaterFlame's ext.subweaponId. */
    SOTN_HOLY_WATER_FLAME = 11,
    SOTN_HOLY_WATER_FLAME_BLUEPRINT = 28,
    SOTN_HOLY_WATER_FLAME_ENTITY = 23,
    SOTN_HOLY_WATER_FLAME_ROWS = 4,
    /** Native key for EntitySubwpnHolyWaterBreakGlass (entity 40, blueprint 59, subweaponId 0). */
    SOTN_HOLY_WATER_GLASS = 0x128,
    SOTN_HOLY_WATER_GLASS_BLUEPRINT = 59,
    SOTN_HOLY_WATER_GLASS_ENTITY = 40,
    SOTN_HOLY_WATER_GLASS_ROWS = 16, /**< Eight hidden FakePrim carriers, then eight G4 shards. */
    SOTN_HOLY_WATER_SMOKE = 0x70004, /**< FACTORY(BP_4, 7): profile-7 green smoke. */
    SOTN_SUBWEAPON_STOPWATCH = 6,
    SOTN_STOPWATCH_BLUEPRINT = 71,
    SOTN_STOPWATCH_ENTITY = 41,
    SOTN_STOPWATCH_ROWS = 3, /**< The dial and two countdown digits. */
    /** Native key for EntityStopWatchExpandingCircle (entity 42, blueprint 75, subweaponId 0). */
    SOTN_STOPWATCH_CIRCLE = 0x12a,
    SOTN_STOPWATCH_CIRCLE_BLUEPRINT = 75,
    SOTN_STOPWATCH_CIRCLE_ENTITY = 42,
    SOTN_STOPWATCH_CIRCLE_ROWS = 16,
    SOTN_STOPWATCH_BURST = 0xe0004, /**< FACTORY(BP_4, 14): the profile-14 streak burst. */
    /** BP_120's entity64 (func_80123F78): the player's white tint while enemies are stopped. */
    SOTN_STOPWATCH_TINT = 0x140,
    SOTN_STOPWATCH_TINT_BLUEPRINT = 120,
    SOTN_STOPWATCH_TINT_ENTITY = 64
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

/** @brief Original EntitySubwpnCrashCross ext words and its one GT4 row. The beam samples the
 * uploaded 48x1 gradient as a 15-bit direct page (0x11C, u 1..0x30, v 0xF8). Its row keeps
 * screen words (x0 = x2, x1 = x3, y0 = y1, y2 = y3): the entity has no FLAG_POS_CAMERA_LOCKED. */
typedef struct SotnCrossBeam
{
    Sint16 half_width;  /**< unk7C. */
    Sint16 half_height; /**< unk7E; its low byte ends the rise at 0x70. */
    Uint8 width;        /**< unk80 (u8). */
    Uint8 height;       /**< unk82 (u8). */
    Sint32 hold;        /**< unk84 (s32). */
    Sint16 x0, x1, y0, y2;
    Uint16 draw_mode; /**< 0x31 (additive), 0x35 with DRAW_COLORS once it spreads. */
    Uint8 grey;       /**< prim b3; every channel once it spreads (+4 up to 0xF8). */
    Uint8 crash;      /**< unkB2 from func_8011A290: the definition's crashId. */
} SotnCrossBeam;

/** @brief Original EntityGiantSpinningCross ext words and its 46 GT4 rows (page 0x1C). Rows are
 * GTE screen words (x0, x1, x2, x3 = vectors 0, 1, 3, 2); unset bits of visible are DRAW_HIDE. */
typedef struct SotnGiantCross
{
    Sint16 x[SOTN_GIANT_CROSS_ROWS][4], y[SOTN_GIANT_CROSS_ROWS][4];
    Uint64 visible;  /**< Rows drawn (DRAW_DEFAULT) after the last NCLIP pass. */
    Uint16 priority; /**< Shared priority of the visible rows, from the centre's depth. */
    Sint16 angle;    /**< unk7C: orbit angle, +0x60 per update. */
    Sint16 spin;     /**< unk7E: Y rotation, +0x60 per update. */
    Sint32 velocity_y;
} SotnGiantCross;

/** @brief One EntitySubwpnCrashCrossParticles GT4 (page 0x1A, palette 0x1B0, opaque). The
 * original keeps its state in colour bytes: r0 active, r1 born, g0/g1 screen centre, g3 rise,
 * b0 frame, b1 parity. A visible row's quad was placed this update by func_80119E78. */
typedef struct SotnCrossParticle
{
    Uint8 active, born, x, y, speed, frame, parity;
    Uint8 shown;  /**< Frame whose size/UVs func_80119E78 last wrote. */
    Uint8 placed; /**< Quad centre Y at that call (g1). */
    bool visible; /**< drawMode DRAW_DEFAULT (false: never placed or DRAW_HIDE). */
    Uint16 priority;
} SotnCrossParticle;

/** @brief Original EntitySubwpnCrashCrossParticles: 192 updates of rising sparks. */
typedef struct SotnCrossParticles
{
    SotnCrossParticle rows[SOTN_CROSS_PARTICLE_ROWS];
    Sint16 timer; /**< ext.timer.t. */
} SotnCrossParticles;

/** @brief Original EntitySubwpnKnife ext words and its two rows. Both rows have DRAW_UNK02
 * (the entity has FLAG_POS_CAMERA_LOCKED), so their words are room-local pixels. */
typedef struct SotnDagger
{
    Sint32 x[4], y[4];           /**< Blade GT4 (page 0x1C, palette 0x1AB). */
    Sint32 line_x[2], line_y[2]; /**< Trail LINE_G2 (0x7F/0x3F/0 to black, additive). */
    Sint32 velocity_y;
    Sint16 timer;                /**< ext.timer.t. */
    Sint16 rotate;               /**< Entity rotate while bouncing. */
    Uint16 row_priority;         /**< Both rows: the player's priority + 2 at birth. */
    Uint16 quad_mode, line_mode; /**< drawMode words. */
    Uint8 grey;                  /**< Blade PGREY once its fade starts (DRAW_COLORS). */
} SotnDagger;

/** @brief Original EntitySubwpnThrownAxe ext words and its five GT4 rows (page 0x1C, palette
 * 0x1AB/0x1AC by g_GameTimer). No row has DRAW_UNK02, so rows keep original screen words. Row 0
 * is the axe; rows 1..4 are afterimages copying row 0's words when their pose starts, tinted
 * additively from D_800B0628 for ten updates. */
typedef struct SotnAxe
{
    Sint16 x[SOTN_AXE_ROWS][4], y[SOTN_AXE_ROWS][4];
    Uint8 visible;   /**< Row bits without DRAW_HIDE. */
    Uint8 shade[4];  /**< unk8C: an afterimage's colour index, post-incremented each update. */
    Uint8 active[4]; /**< unk90. */
    Uint8 fresh[4];  /**< unk94: copy row 0's words on the next pass. */
    Uint8 tint[4];   /**< Colour index the afterimage shows this update. */
    Sint16 angle;    /**< unk7C. */
    Sint32 volume;   /**< unk98: spin swish volume, 0x7F minus eight per swish. */
    Sint32 velocity_y;
    Uint16 pose;
    Sint16 pose_timer;
    Uint16 palette;      /**< Every row's CLUT this update. */
    Uint16 row_priority; /**< The player's priority - 2 at birth. */
} SotnAxe;

/** @brief Original EntitySubwpnHolyWater ext words. The bottle is an entity sprite (DRA bank 9,
 * frame 0x1D) until it breaks, then a hidden hitbox releasing the flames. */
typedef struct SotnHolyWater
{
    Sint32 velocity_y;
    Sint16 timer; /**< ext.holywater.timer. */
    Uint8 crash;  /**< unkB2 from func_8011A290: the definition's crashId (11). */
    bool shown;   /**< animSet ANIMSET_DRA(9), cleared (0) at the break. */
} SotnHolyWater;

/** @brief Original EntitySubwpnHolyWaterBreakGlass: eight hidden FakePrim carriers (16.16 room
 * positions) and eight G4 shards placed around them from D_800B0658 (DRAW_UNK02 room words). */
typedef struct SotnHolyGlass
{
    Sint32 x[8], y[8], velocity_x[8], velocity_y[8];
    Uint8 red[8], green[8], blue[8];
    Uint16 draw_mode[8]; /**< 0x37 (additive) or 0x06 (opaque), DRAW_COLORS | DRAW_UNK02. */
    Sint16 timer;
    Uint16 row_priority;
} SotnHolyGlass;

/** @brief Original EntitySubwpnHolyWaterFlame: four GT4 strips (page 0x1A, palette 0x1B2,
 * additive, DRAW_UNK02 room words) between five wave points, sharing one fading colour. */
typedef struct SotnHolyFlame
{
    Sint32 x[5], y[5];
    Sint16 timer;  /**< 0x50 down to 0x11. */
    Sint16 angle;  /**< Wave phase, +0x180 per update. */
    Sint16 height; /**< unk80: 0x12..0x21. */
    Uint8 red, green, blue;
    Uint8 frame; /**< g_GameTimer & 7 at the last update: D_800B0688 UV pair. */
    bool shown;  /**< DRAW_HIDE cleared (the flame is at least seven pixels tall). */
    Uint16 row_priority;
} SotnHolyFlame;

/** @brief Original EntityStopWatch ext words and its three GT4 rows (DRAW_UNK02 room words): the
 * dial (page 0x1E, palette 0x17F; PAL_FILL_WHITE and DRAW_COLORS 0x40/0x40/0x60 once it bursts)
 * and two digits (page 0x1A, palette 0x186). Rows with DRAW_UNK_200 follow post-stage shoves. */
typedef struct SotnStopwatch
{
    Sint32 x[3][4], y[3][4];
    Sint32 height;     /**< unk84 (16.16): lift above the hover point. */
    Sint32 velocity;   /**< unk88 (16.16): bounce speed. */
    Sint16 t;          /**< ext t: countdown digit, later the burst clock. */
    Sint16 tick;       /**< unk7E: updates within the current second (to 80). */
    Sint16 angle;      /**< unk80: dial tilt. */
    Sint16 scale;      /**< unk82: grow-in percentage (to 100). */
    Sint16 bounce;     /**< unk8C: the next hop is the higher one. */
    Sint16 two_digits; /**< unk8E. */
    Uint8 u[3][4], v[3][4];
    Uint16 draw_mode[3], priority[3], palette0;
} SotnStopwatch;

/** @brief Original EntityStopWatchExpandingCircle: sixteen dithered additive GT4 wedges (page 0x1A,
 * PAL_FILL_WHITE, priority 0xC2) in screen words (no DRAW_UNK02), regrown from the entity. */
typedef struct SotnStopwatchCircle
{
    Sint16 x[16][4], y[16][4];
    Uint8 u[16][2], v[16][2]; /**< Outer corners; the inner ones are (0x20, 0xE0). */
    Sint16 timer, size;
} SotnStopwatchCircle;

/** @brief Original func_80123F78: a rowless additive copy of the player's birth frame (DRA
 * animSet1, unk5A 13, PAL_FILL_WHITE) whose g_PlayerDraw[13] grey fades one step an update. */
typedef struct SotnStopwatchTint
{
    Uint16 sprite;      /**< PLAYER.animCurFrame at birth (the entity never animates). */
    Uint8 grey;         /**< g_PlayerDraw[13] r0..b3 (all equal). */
    Uint8 draw_flags;   /**< PLAYER.drawFlags at birth; a scale flag (zero scale) hides it. */
    bool color_enabled; /**< g_PlayerDraw[13].enableColorBlend. */
} SotnStopwatchTint;

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
        SotnCrossBeam beam;
        SotnGiantCross cross;
        SotnCrossParticles sparks;
        SotnDagger dagger;
        SotnAxe axe;
        SotnHolyWater holy;
        SotnHolyGlass glass;
        SotnHolyFlame flame;
        SotnStopwatch stopwatch;
        SotnStopwatchCircle circle;
        SotnStopwatchTint tint;
    };
} SotnSubweaponEntity;

/** @brief func_8011A290 at its original point in the callback: roll func_800FE3C4's attack with
 * the character libc cursor and take func_80118894's group. */
typedef bool (*SotnSubweaponProperties)(void *context, Uint32 *random);

/** @brief Actual context for one callback. */
typedef struct SotnSubweaponInput
{
    SotnDataView executable;    /**< Private SDK rsin/rcos tables. */
    SotnDataView dra;           /**< Private DRA: the giant cross model and particle UVs. */
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
    Uint32 engine_timer;       /**< g_Timer: the cross beam widens on odd frames. */
    Uint16 crash;              /**< func_8011A290's crashId for this entity's definition. */
    Uint32 flame_counter;      /**< DRA BSS D_8013841C: the holy water flames' factory parameter. */
    bool enemies_paused;       /**< g_unkGraphicsStruct.pauseEnemies (stage scripts). */
    bool stopwatch;            /**< g_unkGraphicsStruct.D_800973FC as it stands in the pass. */
    bool player_normal_set;    /**< PLAYER.animSet == 1 (not Bat13/Wolf15/Axe Armor's overlay). */
    Uint16 player_sprite;      /**< PLAYER.animCurFrame (frame-load bit included). */
    Uint8 player_draw_flags;   /**< PLAYER.drawFlags (2 Secret boots' scaleY, 4 rotation). */
} SotnSubweaponInput;

/** @brief Requests for the owner, committed only after a successful callback. */
typedef struct SotnSubweaponEvents
{
    Uint16 sound;
    Uint16 sound_after; /**< A second PlaySfx later in the same callback. */
    Uint8 use_ticks;
    bool disable_attack; /**< hitboxState = 0. */
    /** CreateEntFactoryFromEntity(self, factory_packed[i], factory_argument[i]), in order. */
    Uint8 factories;
    Uint32 factory_packed[2];
    Uint16 factory_argument[2];
    Sint32 factory_x, factory_y; /**< The entity's 16.16 position at those calls. */
    Uint16 hearts_spent;         /**< g_Status.hearts decrement. */
    bool reset_afterimages;      /**< func_8010DFF0(1, 1), even before a failed birth. */
    Uint8 spell_use_ticks;       /**< g_Player.timers[12]. */
    bool volume_sound;           /**< sound is PlaySfxVolPan(sound, volume, 0). */
    bool flame_counter_step;     /**< D_8013841C++ after a flame factory request. */
    bool attack_set;             /**< hitboxState written this callback (attack_on: 2, else 0). */
    bool stopwatch_set;          /**< g_unkGraphicsStruct.D_800973FC = 1 (enemies stop). */
    bool stopwatch_clear;        /**< g_unkGraphicsStruct.D_800973FC = 0. */
    bool attack_on;
    Uint16 volume;
    bool upload_gradient; /**< LoadImage(&D_800B0788, D_800B06C8): the beam gradient. */
} SotnSubweaponEvents;

/** @brief Rows a supported subweapon entity leases at birth (zero when unsupported or rowless).
 */
Uint16 SotnSubweaponEntity_Rows(Uint16 id);
/** @brief Whether the shared runtime owns this subweapon entity key (the holy water bottle has no
 * rows). */
bool SotnSubweaponEntity_Known(Uint16 id);
/** @brief Read a holy water shard's three private D_800B0658 offset pairs. @param dra Private DRA.
 * @param shard Shard 0..3. @param out Receives x0,y0,x1,y1,x2,y2. @return Success or error. */
bool SotnSubweaponEntity_GlassShard(SotnDataView dra, Uint16 shard, Sint16 out[6]);
/** @brief Read a holy water flame UV origin (D_800B0688). @param dra Private DRA.
 * @param frame g_GameTimer & 7. @param u Receives texX. @param v Receives texY. */
bool SotnSubweaponEntity_FlameTexture(SotnDataView dra, Uint16 frame, Uint8 *u, Uint8 *v);
/** @brief Read a giant cross row's private UVs and palette (D_800B0F94, set at birth).
 * @param dra Private DRA image. @param row Row 0..45. @param u Receives u0..u3.
 * @param v Receives v0..v3. @param palette Receives the CLUT. @return Success or SDL error. */
bool SotnSubweaponEntity_CrossFace(SotnDataView dra, Uint16 row, Uint8 u[4], Uint8 v[4],
                                   Uint16 *palette);
/** @brief Read a particle frame's private UVs (D_800AD094, func_80119E78).
 * @param dra Private DRA image. @param frame Frame 0..5. @param u Receives u0..u3.
 * @param v Receives v0..v3. @return Success or SDL error. */
bool SotnSubweaponEntity_SparkFace(SotnDataView dra, Uint16 frame, Uint8 u[4], Uint8 v[4]);
/** @brief Read an axe afterimage colour (D_800B0628). @param dra Private DRA image.
 * @param index Colour 0..9. @param rgb Receives red, green, blue. @return Success or error. */
bool SotnSubweaponEntity_AxeTint(SotnDataView dra, Uint16 index, Uint8 rgb[3]);
/** @brief Run one DRA subweapon callback (EntitySubwpnThrownVibhuti, EntitySubwpnReboundStone,
 * EntitySubwpnAgunea, EntityAguneaHitEnemy, EntitySubwpnBible, EntitySubwpnBibleTrail,
 * EntitySubwpnCrashCross, EntityGiantSpinningCross, EntitySubwpnCrashCrossParticles,
 * EntitySubwpnKnife, EntitySubwpnThrownAxe, EntitySubwpnHolyWater, EntitySubwpnHolyWaterBreakGlass,
 * EntitySubwpnHolyWaterFlame, EntityStopWatch, EntityStopWatchExpandingCircle).
 * @param entity Owned state. @param input Actual context; properties is required.
 * @param random Character libc cursor in original order. @param events Receives requests.
 * @return Success or SDL error without mutation. The owner reserves the rows before the birth
 * callback, clears hit flags after each callback, culls entities without KEEP_ALIVE_OFFCAMERA
 * and moves hidden rows with the screen. A failed birth reservation retires without RNG. */
bool SotnSubweaponEntity_Tick(SotnSubweaponEntity *entity, SotnSubweaponInput input, Uint32 *random,
                              SotnSubweaponEvents *events);
#endif
