#ifndef SOTN_GHOST_FAMILIAR_H
#define SOTN_GHOST_FAMILIAR_H
#include <sotn/sword_familiar.h>

/** Original tt_001 Ghost familiar sizes. */
enum
{
    SOTN_GHOST_CLUT_WORDS = 256, /**< ServantInit's LoadImage at row0xF4 (CLUT_INDEX_SERVANT). */
    SOTN_GHOST_CLOUD_ROWS = 8,   /**< UpdateAttackEntities' GT4 sparkle ring. */
    SOTN_GHOST_CONFUSED_ROWS = 3 /**< UpdateConfusedEntities' question marks. */
};
/** Original Ghost entity identities (ServantDesc slots used by the US overlay). */
enum
{
    SOTN_GHOST_DEFAULT = 0xd1,  /**< UpdateServantDefault, slot4. */
    SOTN_GHOST_CLOUD = 0xda,    /**< UpdateAttackEntities: the attack's sparkle ring. */
    SOTN_GHOST_CONFUSED = 0xdb, /**< UpdateConfusedEntities: question marks over a lost ghost. */
    SOTN_GHOST_DESTROY = 0xdf,  /**< DestroyServantEntity: the room event's priority switch. */
    SOTN_GHOST_SOUND = 0x6b0    /**< SFX_GLASS_SHARDS, played by a new attack ring. */
};
/** @brief One original slot4..7 entity. Positions are 16.16 screen words like the original.
 * attack/confused/target hold raw slot links without generations, as the original pointers do:
 * attack/confused are slot5..7 (zero = NULL), target is a stage index plus one. */
typedef struct SotnGhostSlot
{
    Uint32 generation;
    Uint16 identity, step, parameter, priority;
    Sint32 x, y, velocity_x, velocity_y;
    Uint32 flags;
    bool facing_left;
    Uint8 hitbox_width, hitbox_height; /**< 8x8 for the one update that attacked. */
    Uint8 opacity;                     /**< Entity opacity (ENTITY_OPACITY). */
    Uint16 spell;                      /**< Last GetServantStats spell (17/18); zero before. */
    Sint16 bob, target_angle, max_angle, frame_counter, glow, glow_step, confused_step;
    Uint16 attack, confused, target;
    Uint32 animation; /**< AnimationFrame* within the overlay at 0x80170000, or zero. */
    Uint16 pose;
    Sint16 pose_timer, frame; /**< poseTimer and animCurFrame. */
} SotnGhostSlot;
/** @brief Actual tt_001 overlay state: four slots, each child's rows and the overlay globals
 * (s_GhostStats, s_IsServantDestroyed, s_LastTargetedEntityIndex, UpdateServantDefault's
 * targetX/targetY/x/y statics, the ring's timer/angle and the question marks' counter). Rows hold
 * screen words. The native owner supplies real row allocation outcomes; never saved. */
typedef struct SotnGhostRuntime
{
    SotnGhostSlot slots[4];
    SotnPlayerEffectQuad rows[3][SOTN_GHOST_CLOUD_ROWS]; /**< Slot5..7 child rows. */
    SotnFamiliarStats stats;
    bool destroyed;
    Sint32 last_target;
    Sint16 target_x, target_y;
    Sint32 follow_x, follow_y;
    Sint32 cloud_timer, cloud_angle, confused_counter;
    SotnFamiliarEvents events;
    Uint32 generation;
    bool initialized;
} SotnGhostRuntime;
/** @brief Current actual snapshots for one Ghost callback. */
typedef struct SotnGhostInput
{
    SotnDataView overlay;           /**< Actual TT_001 image at 0x80170000. */
    SotnDataView executable;        /**< Private SDK trigonometry/square-root tables. */
    SotnFairyPlayer player;         /**< Original screen-space player entity snapshot. */
    const SotnSwordTarget *targets; /**< SOTN_SWORD_TARGETS stage snapshots (slots64..191). */
    SotnFamiliarStats stats;        /**< GetServantStats: g_Status.statsFamiliars[1]. */
    Uint16 room_flags, stage;
    bool cutscene;  /**< g_CutsceneHasControl. */
    bool allocated; /**< The requested row lease is available this callback. */
    SotnFamiliarEventSource event_source;
    SotnFamiliarEventInput room;
} SotnGhostInput;
/** @brief Real stage-pool rows held by one live Ghost child (slot5..7). */
typedef struct SotnGhostLease
{
    Uint32 generation;
    Uint16 slot, count;
} SotnGhostLease;
/** @brief Requests for native owners, committed after a successful callback. */
typedef struct SotnGhostEvents
{
    bool clut_upload; /**< ServantInit's 256-word LoadImage at row0xF4. */
    Uint16 clut[SOTN_GHOST_CLUT_WORDS];
    Uint16 mark;  /**< Target index plus one that receives FLAG_UNK_80000, or zero. */
    Uint16 spell; /**< GetServantStats(self, spell, 1): slot4 attacks with 8x8 this update. */
    Uint16 sound; /**< PlaySfx request, or zero. */
} SotnGhostEvents;
/** @brief Original ServantInit for InitializeMode0/2 (engine), 1 (menu switch) or 3 (same).
 * @param runtime Actual owner, unchanged on failure. @param input Current snapshots.
 * @param mode Original mode. @param out Requests (the CLUT upload).
 * Modes0..2 replace slot4 with the ghost beside the player, at a flag20 room's side or, on a menu
 * switch, above the screen. The CLUT copies the overlay's g_ServantClut and g_GhostClut words as
 * the original does, past both tables, including the live event words they cover. */
bool SotnGhostRuntime_Init(SotnGhostRuntime *runtime, SotnGhostInput input, Uint16 mode,
                           SotnGhostEvents *out);
/** @brief Run one actual slot4..7 callback in original slot order.
 * @return Success or SDL error preserving runtime/output. */
bool SotnGhostRuntime_UpdateSlot(SotnGhostRuntime *runtime, Uint16 slot, SotnGhostInput input,
                                 SotnGhostEvents *out);
/** @brief Apply original camera/player or room-entry translation to entities and their rows.
 * Same contract as SotnSwordRuntime_Translate. No callback or clock advances. */
bool SotnGhostRuntime_Translate(SotnGhostRuntime *runtime, Sint16 camera_x, Sint16 camera_y,
                                Sint32 player_x, Sint32 player_y, bool room);
/** @brief Rows the next callback of one slot reserves through AllocPrimitives, or zero.
 * @return 8 for a ring or 3 for question marks at step0 without a release parameter. */
Uint16 SotnGhostRuntime_RowRequest(const SotnGhostRuntime *runtime, Uint16 slot);
/** @brief Report the row leases the live children hold, indexed by slot5..7.
 * @return Success or SDL error for an invalid owner. */
bool SotnGhostRuntime_Leases(const SotnGhostRuntime *runtime, SotnGhostLease out[3]);
/** @brief Original ServantWorkClear: retire slots4..7 and their rows, keeping overlay globals. */
bool SotnGhostRuntime_Clear(SotnGhostRuntime *runtime);
#endif
