#ifndef SOTN_SWORD_FAMILIAR_H
#define SOTN_SWORD_FAMILIAR_H
#include <sotn/fairy.h>
#include <sotn/familiar_events.h>
#include <sotn/familiar_stats.h>
#include <sotn/player_blink.h>

/** Original tt_004 primitive counts: the model (72 or 96 used), the blade trail and the halo. */
enum
{
    SOTN_SWORD_MODEL_ROWS = 96,
    SOTN_SWORD_TRAIL_ROWS = 16,
    SOTN_SWORD_HALO_ROWS = 4,
    SOTN_SWORD_CLUT_WORDS = 48,
    SOTN_SWORD_TARGETS = 128,      /**< Stage entity slots64..191 scanned by the target search. */
    SOTN_SWORD_FAMILIAR_ITEM = 126 /**< ITEM_SWORD_FAMILIAR, the level-50 hand-item gift. */
};
/** Original Sword familiar entity identities (SERVANT_ID 1..13). */
enum
{
    SOTN_SWORD_DEFAULT = 0xd1,
    SOTN_SWORD_CIRCLE = 0xd2,
    SOTN_SWORD_DIVE = 0xd3,
    SOTN_SWORD_BROTHERS = 0xd4,
    SOTN_SWORD_GIFT = 0xd5,
    SOTN_SWORD_ENTRANCE = 0xd8,
    SOTN_SWORD_TRAIL = 0xda,
    SOTN_SWORD_HALO = 0xdb,
    SOTN_SWORD_EVENT = 0xdc,
    SOTN_SWORD_SOUND = 0xdd,
    SOTN_SWORD_HITBOX = 0xde,         /**< One-update attack hitbox in the player's slots17..46. */
    SOTN_SWORD_BROTHERS_HITBOX = 0xdf /**< Sword Brothers' nine-update global attack entity. */
};
/** @brief One original packed screen SXY word. */
typedef struct SotnSwordPoint
{
    Sint16 x, y;
} SotnSwordPoint;
/** @brief One original slot4..7 entity: identity, step, params, screen 16.16 position and
 * ext.swordFamiliar words (unk7c kind, unk80 model, unk82 variant, unk84 heading, unk86/88 yaw,
 * currentX/unk8c/targetX pitch). The sound cursor belongs to the 0xDD narrative entity. */
typedef struct SotnSwordSlot
{
    Uint32 generation;
    Uint16 identity, step, parameter, kind, priority;
    Sint32 x, y, velocity_x, velocity_y;
    Uint32 flags;
    bool facing_left;
    Sint16 model, variant, heading, yaw, yaw_previous, pitch_target, pitch, pitch_previous;
    Uint32 camera_x, camera_y; /**< ext posX/posY: world camera origin at the attack's update. */
    Sint16 swing;              /**< ext unk98: the next slash may roll a voice. */
    Uint16 follow; /**< ext follow: stage target index plus one (raw slot, no generation). */
    SotnFamiliarSfx sound;
} SotnSwordSlot;
/** @brief Actual tt_004 overlay state: four slots, their primitive rows and the overlay globals
 * (D_us_80170078 rotation, D_us_80170080 position, the projected blade edge/halo/tips, the
 * s_SwordStats copy and level cursor, the bob/halo phases, the D_us_80178B74..B84 flags,
 * pauseAllowed and g_PlaySfxStep). Rows hold screen words. The native owner supplies real row
 * allocation outcomes; this state is never saved. */
typedef struct SotnSwordRuntime
{
    SotnSwordSlot slots[4];
    SotnPlayerEffectQuad model[SOTN_SWORD_MODEL_ROWS], trail[SOTN_SWORD_TRAIL_ROWS],
        halo[SOTN_SWORD_HALO_ROWS];
    Sint32 position[3];
    Sint16 rotation[3];
    SotnSwordPoint edge[2], halo_points[4][4], tips[2];
    Sint32 edge_priority, halo_priority[4], scratch_depth;
    SotnFamiliarStats stats;
    Sint32 level_seen, bob, entrance_bob, halo_phase, halo_phase2, sound_step;
    bool model_ready, edge_fresh, halo_fresh, entrance_cancel, near_player, pause_allowed;
    SotnFamiliarEvents events;
    Sint32 circle[6]; /**< D_us_80178568..7C: circle attack angle, counter, deltas, target. */
    Sint32 dive[14];  /**< D_us_80178580..B4: dive target, deltas, velocity, origin, distance. */
    Sint32 target_cursor; /**< D_us_80178B88: round-robin over already-marked targets. */
    Sint32 gift[12];      /**< D_us_801785EC..618: the farewell's targets, deltas and speed. */
    Sint32 brothers[13];  /**< D_us_801785B8..E8: Sword Brothers targets, counters and copies. */
    Uint32 generation;
    bool initialized;
} SotnSwordRuntime;
/** @brief Current screen-space stage entity (original slot64+index) seen by the target search. */
typedef struct SotnSwordTarget
{
    Uint16 id, hitbox_state;
    Uint32 flags; /**< Original entity flags; FLAG_UNK_80000 marks an earlier pick. */
    Sint16 x, y;  /**< posX/posY whole screen pixels. */
    Sint16 hit_points;
} SotnSwordTarget;
/** @brief One func_us_801724E8 request: a DE hitbox entity from slots17..46 (GetFreeEntity).
 * Its first update reads the projected tip, takes spell 19/20 through GetServantStats and keeps
 * a 4x4 hitbox for one hit detection; its second update destroys it. */
typedef struct SotnSwordHitbox
{
    Uint16 parameter, spell;
    Sint16 x, y; /**< D_us_801786C0[parameter], the projected blade tip. */
    Uint16 priority;
    bool facing_left;
} SotnSwordHitbox;
/** @brief Current actual snapshots for one Sword callback. */
typedef struct SotnSwordInput
{
    SotnDataView overlay;           /**< Actual TT_004 image at 0x80170000. */
    SotnDataView executable;        /**< Private SDK trigonometry/square-root tables. */
    SotnFairyPlayer player;         /**< Original screen-space player entity snapshot. */
    const SotnSwordTarget *targets; /**< SOTN_SWORD_TARGETS stage snapshots, borrowed. */
    Uint32 tilemap_x, tilemap_y;    /**< g_Tilemap left/top*256 plus whole scroll. */
    Uint32 random;                  /**< Original libc rand() state before the callback. */
    SotnFamiliarStats stats;        /**< GetServantStats: g_Status.statsFamiliars[4]. */
    Uint16 room_flags, stage;
    Uint8 castle_flag; /**< g_CastleFlags[SWORD_FAMILIAR]. */
    bool cutscene, enemies, global_flash;
    bool brothers;  /**< g_Player.unk70: this update's Sword Brothers cast pulse. */
    bool allocated; /**< The requested row lease is available this callback. */
    SotnFamiliarEventSource event_source;
    SotnFamiliarEventInput room;
    SotnFamiliarSfxInput sound;
} SotnSwordInput;
/** @brief Real stage-pool rows held by one live Sword owner. Part0 is slot4's model (96 rows,
 * held once its first initialization reserved them), part1 the DA trail (16) and part2 the DB
 * halo (4). Count0 means no rows; the native pool owner fills rows. */
typedef struct SotnSwordLease
{
    Uint32 generation;
    Uint16 slot, part, count;
    Uint16 rows[SOTN_SWORD_MODEL_ROWS];
} SotnSwordLease;
/** @brief Requests for native owners, committed after a successful callback. */
typedef struct SotnSwordEvents
{
    Uint16 cd_command;
    Uint8 castle_flag;
    bool castle_flag_write, pause_write, pause_allowed;
    bool clut_upload; /**< ServantInit's 48-word LoadImage at row0xF4. */
    Uint16 mark;      /**< Target index plus one that receives FLAG_UNK_80000, or zero. */
    Uint16 sounds[2], sound_count;     /**< PlaySfx requests in order. */
    SotnSwordHitbox hitboxes[2];       /**< DE requests in order. */
    Uint16 hitbox_count, random_calls; /**< rand() calls this callback consumed. */
    Uint32 random;                     /**< libc rand() state after the callback. */
    Uint16 gift_item; /**< AddToInventory(ITEM_SWORD_FAMILIAR, EQUIP_HAND) at level 50, or 0. */
    bool dismiss; /**< The farewell ended: card off (found only), g_Servant 0, slots4..7 gone. */
    bool brothers_hitbox; /**< func_us_801724E8(self, 1, 0): Sword Brothers' DF attack entity. */
} SotnSwordEvents;
/** @brief Read ServantInit's g_ServantClut (16) and g_SwordClut (32) words for row0xF4.
 * @return Success or SDL error for a malformed private overlay. */
bool SotnSword_Clut(SotnDataView overlay, Uint16 out[SOTN_SWORD_CLUT_WORDS]);
/** @brief Original ServantInit for InitializeMode0 (engine), 1 (menu switch) or 3 (same).
 * @param runtime Actual owner, unchanged on failure. @param input Current snapshots.
 * @param mode Original mode. @param out Requests (CLUT upload, castle flag).
 * Mode0/1 replace slot4 with the default sword or, on a menu switch outside flag20/40 rooms,
 * the entrance, and spawn the level auras into slots5..7. */
bool SotnSwordRuntime_Init(SotnSwordRuntime *runtime, SotnSwordInput input, Uint16 mode,
                           SotnSwordEvents *out);
/** @brief Run one actual slot4..7 callback in original slot order.
 * @return Success or SDL error preserving runtime/output. The circle (D2) and dive (D3) attacks
 * the level-50 gift and farewell (D5) and Sword Brothers (D4) run. */
bool SotnSwordRuntime_UpdateSlot(SotnSwordRuntime *runtime, Uint16 slot, SotnSwordInput input,
                                 SotnSwordEvents *out);
/** @brief Apply original camera/player or room-entry translation to entities and their rows.
 * @param camera_x/@param camera_y Old minus new camera, whole pixels (ignored for rooms).
 * @param player_x/@param player_y Old minus new player displacement in 16.16.
 * @param room Room entry: retire non-0x20000 slots first. No callback or clock advances. */
bool SotnSwordRuntime_Translate(SotnSwordRuntime *runtime, Sint16 camera_x, Sint16 camera_y,
                                Sint32 player_x, Sint32 player_y, bool room);
/** @brief Rows the next callback of one slot reserves through AllocPrimitives, or zero.
 * @param runtime Actual owner. @param slot Original slot4..7.
 * @return 96 for an uninitialized model owner, 16/4 for a DA/DB at step0, else zero. The caller
 * passes whether that many actual rows are free as SotnSwordInput::allocated. */
Uint16 SotnSwordRuntime_RowRequest(const SotnSwordRuntime *runtime, Uint16 slot);
/** @brief Report the row leases the runtime's live owners hold (rows left zero).
 * @param runtime Actual owner. @param out Three parts indexed model/trail/halo.
 * @return Success or SDL error when two live owners claim one part. */
bool SotnSwordRuntime_Leases(const SotnSwordRuntime *runtime, SotnSwordLease out[3]);
/** @brief Original ServantWorkClear: retire slots4..7 and their rows, keeping overlay globals. */
bool SotnSwordRuntime_Clear(SotnSwordRuntime *runtime);
#endif
