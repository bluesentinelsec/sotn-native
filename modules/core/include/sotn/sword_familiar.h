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
    SOTN_SWORD_CLUT_WORDS = 48
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
    SOTN_SWORD_SOUND = 0xdd
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
    Uint32 generation;
    bool initialized;
} SotnSwordRuntime;
/** @brief Current actual snapshots for one Sword callback. */
typedef struct SotnSwordInput
{
    SotnDataView overlay;    /**< Actual TT_004 image at 0x80170000. */
    SotnDataView executable; /**< Private SDK trigonometry/square-root tables. */
    SotnFairyPlayer player;  /**< Original screen-space player entity snapshot. */
    SotnFamiliarStats stats; /**< GetServantStats: g_Status.statsFamiliars[4]. */
    Uint16 room_flags, stage;
    Uint8 castle_flag; /**< g_CastleFlags[SWORD_FAMILIAR]. */
    bool cutscene, enemies, global_flash;
    bool target;    /**< func_us_80173AA0 found a live target (attacks are not integrated). */
    bool brothers;  /**< g_Player.unk70: Sword Brothers (not integrated). */
    bool allocated; /**< The requested row lease is available this callback. */
    SotnFamiliarEventSource event_source;
    SotnFamiliarEventInput room;
    SotnFamiliarSfxInput sound;
} SotnSwordInput;
/** @brief Requests for native owners, committed after a successful callback. */
typedef struct SotnSwordEvents
{
    Uint16 cd_command;
    Uint8 castle_flag;
    bool castle_flag_write, pause_write, pause_allowed;
    bool clut_upload; /**< ServantInit's 48-word LoadImage at row0xF4. */
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
 * @return Success or SDL error preserving runtime/output. States not yet integrated (attacks,
 * Sword Brothers, the level-50 gift and its item) fail explicitly instead of degrading. */
bool SotnSwordRuntime_UpdateSlot(SotnSwordRuntime *runtime, Uint16 slot, SotnSwordInput input,
                                 SotnSwordEvents *out);
/** @brief Apply original camera/player or room-entry translation to entities and their rows.
 * @param camera_x/@param camera_y Old minus new camera, whole pixels (ignored for rooms).
 * @param player_x/@param player_y Old minus new player displacement in 16.16.
 * @param room Room entry: retire non-0x20000 slots first. No callback or clock advances. */
bool SotnSwordRuntime_Translate(SotnSwordRuntime *runtime, Sint16 camera_x, Sint16 camera_y,
                                Sint32 player_x, Sint32 player_y, bool room);
/** @brief Original ServantWorkClear: retire slots4..7 and their rows, keeping overlay globals. */
bool SotnSwordRuntime_Clear(SotnSwordRuntime *runtime);
#endif
