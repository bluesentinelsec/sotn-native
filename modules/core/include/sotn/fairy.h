#ifndef SOTN_FAIRY_H
#define SOTN_FAIRY_H
#include <sotn/character.h>
#include <sotn/familiar_animation.h>

/** @brief Original Fairy modes in the shared entity namespace, not native availability. */
enum
{
    SOTN_FAIRY_FOLLOW = 0xd1,
    SOTN_FAIRY_APPLE,
    SOTN_FAIRY_HAMMER,
    SOTN_FAIRY_UNCURSE,
    SOTN_FAIRY_ANTIVENOM,
    SOTN_FAIRY_RESIST,
    SOTN_FAIRY_POTION,
    SOTN_FAIRY_INTRO,
    SOTN_FAIRY_WINGS,
    SOTN_FAIRY_SHOULDER,
    SOTN_FAIRY_HINT
};
/** @brief Original Fairy entity fields; no replacement slots, resources or primitive pool.
 * The extension fields describe the root. Wings receive their real parent explicitly;
 * its original union pointer is never represented as root ability/timer words here. */
typedef struct SotnFairy
{
    Sint32 x, y, velocity_x,
        velocity_y; /**< Original entity screen coordinates/velocity in16.16.
                     * Native room/camera owners translate without extra ticks. */
    Uint32 flags;
    Uint16 mode, phase, parameter, priority;
    bool facing_left;
    SotnFamiliarAnimation animation;
    Sint16 ability, animation_flag, wave_angle, target_angle, distance, max_angle;
    Sint16 frames, hammer, uncurse_luck, antivenom_luck, potion_luck, timer, idle;
    Uint32 sound_event, tilemap_x, tilemap_y;
    Sint16 sound_flag, song;
} SotnFairy;
/** @brief Original overlay globals, retained separately from the entity's mode fields. */
typedef struct SotnFairyHistory
{
    Sint32 elements, resist_delay, resist_clear, hp_loss[5], cached_hp, room_special;
    Sint16 hp_index, hp_reset;
    Uint16 priority;
    bool restore_priority;
    Sint32 hint_x, hint_y, shoulder_status;
} SotnFairyHistory;
/** @brief Actual player and original collision/previous-step inputs to IsMovementAllowed. */
typedef struct SotnFairyPlayer
{
    Sint32 x, y, velocity_x, velocity_y; /**< Actual original screen-space entity snapshot. */
    Uint32 status, damage_effects;
    Uint16 step, substep, previous_step, previous_substep, collision, priority;
    bool facing_left;
} SotnFairyPlayer;
/** @brief Borrowed immutable private data and actual current ownership snapshots. */
typedef struct SotnFairyInput
{
    SotnDataView overlay, executable;
    SotnFairyPlayer player;
    Uint16 hp, hp_max, level, room_flags, scene;
    Uint8 hand_count[SOTN_HAND_ITEMS]; /**< Actual unequipped counts, including146/147. */
    Sint32 buffs[16];
    bool inverted, cutscene, enemies, medicine_active;
    Sint32 load_count, sound_step, tilemap_x, tilemap_y;
    bool sound_present, global_flash;
} SotnFairyInput;
/** @brief Requests emitted in original callback order; caller commits actual native owners.
 * A bag decrement still occurs when an optional factory/event cannot allocate. SceneFFF,
 * destroy8..63 and full HP/MP restoration are distinct operations, never a revival helper.
 * Requests alone enable no familiar controls, completed healing or player state17. */
typedef struct SotnFairyEvents
{
    Uint32 factory;
    Uint16 sounds[2], consume, event_mode, event_parameter, scene;
    Uint8 sound_count;
    bool scene_write, restore_resources, clear_shared, clear_flash_if29, process_room_events, shake;
    bool pause_write, pause_allowed, release_song;
} SotnFairyEvents;
/** @brief Original familiar narrative event cursor, distinct from VAB voice leases. */
typedef struct SotnFamiliarSfx
{
    Uint32 flags;
    Uint16 parameter, phase;
    bool removed;
} SotnFamiliarSfx;
/** @brief Actual CD-command and XA playback snapshots, supplied by the native audio owner. */
typedef struct SotnFamiliarSfxInput
{
    Uint16 room_flags;
    bool cutscene, cd_queue_empty, command_acknowledged, xa_playing;
} SotnFamiliarSfxInput;
/** @brief Advance original ProcessSfxState once, using real acknowledgements/playback.
 * @param sfx Actual copied event cursor. @param input Current CD/XA service inputs.
 * @param command Receives one original PlaySfx command, or zero; never a completed playback.
 * @param step Receives original g_PlaySfxStep, including99 on retirement.
 * @return Success or SDL error preserving all outputs. Parameter0 is rejected in phase2;
 * original SoundWait is outside this zero-sentinel request interface. No readiness is invented.
 * The caller must commit commands to its actual CD/XA owner, not treat them as VAB voices. */
bool SotnFamiliarSfx_Tick(SotnFamiliarSfx *sfx, SotnFamiliarSfxInput input, Uint16 *command,
                          Sint32 *step);
/** @brief Apply original one-callback room-special event, before its real slot retires.
 * @param history Actual copied overlay globals. @param parameter Original DC event parameter.
 * @param sound_present Actual DE occupancy. @param sound_phase Actual copied DE phase if present.
 * @return Success or SDL error preserving outputs. Parameter0 aborts a phase<5 sound via8;
 * parameter15 restores root priority. Unknown parameters still retire in the caller.
 * This helper creates no event, slot, audio acknowledgement or room condition. */
bool SotnFairy_RoomEvent(SotnFairyHistory *history, Uint16 parameter, bool sound_present,
                         Uint16 *sound_phase);
/** @brief Initialize original root/wings cursors for engine0/2 or menu switch1/same3.
 * @param root Copied real slot4 cursor. @param wings Copied real slot5 cursor.
 * @param history Actual overlay globals; unrelated history survives initialization.
 * @param input Current player and room snapshots. @param mode Original InitializeMode0..3.
 * @param reset_events Receives whether the caller must reset its actual room event queue.
 * @return Success or SDL error preserving all outputs. Same3 keeps both cursors unchanged.
 * Caller must separately retire replaced real slots/rows and commit bank20/VRAM/bank2 bindings.
 * This supplies no activation, graphics uploads, saved progress or native slot allocation. */
bool SotnFairy_Init(SotnFairy *root, SotnFairy *wings, SotnFairyHistory *history,
                    SotnFairyInput input, Uint16 mode, bool *reset_events);
/** @brief Original signed-word winding helper, including exact half-turn and narrowing. */
Sint16 SotnFairy_StepAngle(Sint16 current, Sint16 target, Sint16 step);
/** @brief Original velocity/collision and optional standing-history gates. */
bool SotnFairy_MovementAllowed(SotnFairyPlayer player, bool standing_history);
/** @brief Run original death-first, level/probability/history item selection once.
 * @param fairy Actual copied entity. @param history Actual copied overlay globals.
 * @param input Private tables and current snapshots. @param random Copied original libc RNG.
 * @return Success, or SDL error without mutation of entity/history/RNG on invalid data.
 * This selects a mode only; it consumes no medicine and supplies no native lifecycle. */
bool SotnFairy_SelectAbility(SotnFairy *fairy, SotnFairyHistory *history, SotnFairyInput input,
                             Uint32 *random);
/** @brief Advance original US Fairy root callbacks, including intro, shoulder and hint, once.
 * @param fairy Actual copied cursor. @param history Copied retained overlay globals.
 * @param input Borrowed immutable data and real ownership inputs. @param random Copied libc RNG.
 * @param out Requests unchanged on failure. Caller commits slots, rows, counts and resources
 * together after success. Actual event slots/rendering/CD-XA acknowledgements remain required.
 * The US Fairy never enters the alternate Yousei's song phases2..4; these fail explicitly.
 * @return Success or SDL error preserving every output/cursor/RNG on failure.
 * This component does not dispatch native slots, alter controls or advance clocks on draws. */
bool SotnFairy_Tick(SotnFairy *fairy, SotnFairyHistory *history, SotnFairyInput input,
                    Uint32 *random, SotnFairyEvents *out);
/** @brief Copy the actual parent pose/facing to the wings and run the original private selector.
 * @param wings Copied real child cursor. @param parent Actual root after its callback.
 * @param history Original retained root priority. @param overlay Borrowed TT_002 data.
 * @return Success or SDL error preserving wings. No parent pointer or slot is invented. */
bool SotnFairy_Wings(SotnFairy *wings, const SotnFairy *parent, SotnFairyHistory *history,
                     SotnDataView overlay);
#endif
