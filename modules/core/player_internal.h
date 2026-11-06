#ifndef SOTN_PLAYER_INTERNAL_H
#define SOTN_PLAYER_INTERNAL_H

#include <SDL3/SDL.h>
#include <sotn/equipment.h>
#include <sotn/mist_motion.h>
#include <sotn/player.h>
#include <sotn/shield.h>
#include <sotn/spell_commands.h>
#include <sotn/sword_warp.h>
#include <stddef.h>

typedef union PlayerFixed {
    Sint32 value;
    struct
    {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
        Uint16 low;
        Sint16 high;
#else
        Sint16 high;
        Uint16 low;
#endif
    } parts;
} PlayerFixed;

typedef struct PlayerSensor
{
    Sint16 x, y;
} PlayerSensor;

typedef struct PlayerDarkControl
{
    SotnPlayerFactorySource source;
    bool started;
} PlayerDarkControl;

struct SotnPlayer
{
    SotnDataView dra;
    SotnCollisionMap map;
    PlayerFixed x, y;
    SotnPlayerWater water;
    SotnPlayerContact queued_contact;
    SotnWaterDamage water_damage;
    SotnPlayerWaterPain water_pain;
    Uint32 damage_clock, damage_last_clock;
    Uint32 received_damage_effects;
    SotnPlayerStep control_step;
    Uint16 control_substep;
    Uint16 head, armor, cape, talismans;
    Uint16 hearts, hearts_max; /**< Borrowed resource snapshot, refreshed before player control. */
    SotnPlayerHeartGain heart_gain;
    SotnPlayerAbsorption absorption;
    SotnPlayerImmunity immunity;
    SotnPlayerGuard guard;
    SotnPlayerDamageNumber damage_number;
    SotnPlayerDodge dodge;
    SotnPlayerAxeArmor axe_armor;
    bool axe_context, axe_overlay_owned;
    bool damage_context, damage_pending, damage_ticked, contact_dispatch;
    const SotnStatBuffs *stat_buffs;
    Uint16 defense_buff_minimum;
    bool hud_hp_hold;
    Uint16 water_breath_timer;
    Uint8 collision_invincibility; /**< Original entity byte0x49, not the player damage timer. */
    Sint32 vx, vy, contact, previous_contact;
    Uint32 frame_contacts, timer_status;
    Sint16 draw_pivot_x, draw_pivot_y;
    Uint32 status, frame_number, movement_flags;
    SotnPlayerAfterimage afterimage;
    SotnPlayerMovementEffects movement_effects;
    SotnPlayerHealing healing;
    SotnPlayerTeleport teleport;
    SotnPlayerMedicineUse medicine;
    SotnPlayerMedicineUse throwing;
    Uint16 throwing_grunt;
    SotnPlayerMedicineFactory throwing_factory;
    SotnPlayerMedicineConsume throwing_consume;
    SotnPlayerThrowingRandom throwing_random;
    void *throwing_context;
    Uint8 throwing_count[2];
    Uint8 special_count[2]; /**< Live children carrying each hand's QCF special equip id. */
    Uint8 special_cross[2]; /**< The same special's live children with the other hand's params. */
    Uint32 combo;           /**< CheckWeaponCombo's D_8013AEE4 (item, bit31 the right hand). */
    Uint8 combo_count;      /**< Live children carrying the combo item with its hand's params. */
    SotnPlayerMedicineUse food;
    SotnPlayerMedicineFactory food_factory;
    SotnPlayerMedicineFactory hand_factory;
    SotnPlayerThrowingRandom hand_random;
    void *hand_context;
    Uint8 hand_retire;
    Uint16 hand_grunt;
    bool hand_used;
    SotnPlayerMedicineConsume food_consume;
    void *food_context;
    Uint8 food_count[2];
    SotnPlayerMedicineFactory medicine_factory;
    SotnPlayerMedicineConsume medicine_consume;
    void *medicine_context;
    Sint32 hammer_timer;
    bool medicine_present;
    SotnPlayerTeleportFactory teleport_factory;
    SotnPlayerLibraryCardUse library_card_use;
    void *teleport_context;
    Sint32 teleport_origin_x;
    PlayerDarkControl dark;
    SotnSpellCommands spell_commands;
    SotnWeaponCommand sword_command;
    SotnWeaponCommand back_forward; /**< Original COMBO_BF; transient, never saved. */
    Uint16 slot16_item; /**< Actual slot16 weapon equipId seen at hand use (Combat knife). */
    SotnPlayerSwordFactory sword_factory;
    void *sword_context;
    Sint32 sword_recovery;
    bool sword_weapon_present;
    bool dark_context, spirit_context, soul_context, sword_brothers_context;
    SotnPlayerSoul soul;
    SotnPlayerSwordBrothers sword_brothers;
    SotnPlayerSpirit spirit;
    SotnPlayerHellfireFactory hellfire_factory;
    void *hellfire_context;
    Sint32 hellfire_camera_x;
    Uint16 hellfire_signal, hellfire_palette;
    bool hellfire_started;
    Uint32 afterimage_generation, afterimage_clock, afterimage_last_clock;
    bool afterimage_pending, afterimage_ticked;
    SotnAnimation animation;
    SotnPlayerSimulatedPad simulated_pad;
    SotnPlayerScriptReaction script_reaction;
    SotnDataView script_executable;
    Uint16 palette, damage_palette;
    Uint8 body_grey[4];
    bool body_color_blend;
    SotnPlayerStep step, previous_step;
    Uint16 buttons, previous_buttons, tapped, substep;
    Uint16 chair_kind, chair_sleep;
    bool sleep_event;
    SotnPlayerSleep sleep;
    Uint16 attack_mode, attack_button;
    Uint8 attack_lock;
    Sint32 attack_hold; /**< Original BSS D_80139824: kinds19..22/28 pose0 hold count. */
    SotnSubweapons *subweapons;
    bool subweapon_cast;
    Uint16 subweapon_animation;
    SotnPlayerAttack attack, shield;
    SotnPlayerKick kick;
    Uint32 kick_counter;
    Uint16 kick_hit;
    SotnPlayerGravity gravity;
    Uint16 mp, mp_spent, held;
    bool muramasa_blood; /**< This update's func_8010A3F0 counter request. */
    SotnMistMotion mist_motion;
    SotnPlayerMist mist;
    SotnPlayerBat bat;
    SotnPlayerWolf wolf;
    Sint32 wolf_camera_x, wolf_camera_y;
    Sint16 wolf_combo_timer;
    Uint16 wolf_combo;
    bool wolf_combo_facing, wolf_context, wolf_factory_available;
    Sint16 wolf_body_root_y;
    Uint32 wolf_body_commit_frame;
    SotnBatCommand bat_command;
    SotnBatCommandEvents bat_commands;
    Uint32 bat_counter;
    Uint16 animation_bank, form_pose;
    bool bat_context, bat_factory_available, bat_screech, form_rotated, bat_specials;
    Uint32 mist_game_timer, mist_visual_generation;
    Uint8 mist_visual_phase, mist_lockout;
    bool mist_context, mist_visual_present, mist_underwater;
    Uint8 gravity_combo;
    Uint16 gravity_combo_timer;
    bool gravity_spawn;
    Sint32 gravity_origin_x, gravity_origin_y;
    SotnShield shield_cursor;
    Uint16 shield_button;
    SotnDataView weapons;
    SotnEquipment attack_equipment;
    Uint16 equipment[2];
    Uint8 relics[SOTN_RELICS];
    Uint8 attack_critical;
    Uint16 attack_power, shield_power;
    SotnPlayerDeath death;
    SotnPlayerHurt hurt;
    Uint16 stage_id;
    SotnPlayerWaterDeath water_death;
    SotnPlayerFairyRescue fairy_rescue;
    SotnPlayerDeathCapture death_capture;
    SotnPlayerDeathDecay death_decay;
    void *death_context;
    Uint32 death_serial;
    SotnPlayerDissolvePresent dissolve_present;
    void *dissolve_context;
    SotnPlayerStone stone;
    SotnPlayerGrab grab;
    Uint16 hp, hp_max, sound, extra_sound, third_sound;
    bool curse_failure, script_hold, bell_inside;
    bool simulated_input;    /**< US first physical frame rebases taps after simulated controls. */
    bool legacy_script_hold; /**< Boolean stage adapter renews its one-frame mode2 request. */
    SotnCharacterStats attributes;
    Uint32 damage_random;
    // One original word bank. Existing owner names alias their actual indices.
    union {
        Uint16 timers[SOTN_PLAYER_TIMERS];
        struct
        {
            Uint16 poison_ticks, curse_ticks, hit_effect_ticks, palette_ticks, color_ticks;
            Uint16 coyote_ticks, fall_ticks, drop_ticks, impact_ticks, weapon_startup;
            Uint16 subweapon_ticks, dark_ticks, spell_use_ticks, invincibility_flash_ticks;
            Uint16 invincibility, afterimage_timer15;
        };
    };
    Uint8 tight_space;
    bool facing_left, turned, fixed_stage, collision_disabled, failed;
    SotnCollider floor[4], ceiling[4], wall[14];
    PlayerSensor floor_sensors[4], ceiling_sensors[4], wall_sensors[14];
    bool wall_snapshot_owned;
};

_Static_assert(offsetof(SotnPlayer, poison_ticks) == offsetof(SotnPlayer, timers),
               "Poison must alias original player timer0");
_Static_assert(offsetof(SotnPlayer, dark_ticks) == offsetof(SotnPlayer, timers) + 11U * 2U,
               "Dark duration must alias original player timer11");
_Static_assert(offsetof(SotnPlayer, invincibility) == offsetof(SotnPlayer, timers) + 14U * 2U,
               "Ordinary protection must alias original player timer14");
_Static_assert(offsetof(SotnPlayer, afterimage_timer15) == offsetof(SotnPlayer, timers) + 15U * 2U,
               "Afterimage duration must alias original player timer15");

/** @brief Rebuild supported original status bits from current state/timers, without mutation. */
Uint32 SotnPlayer_Status(const SotnPlayer *player);
/** @brief Advance original word timers0..14 in order; timer15 belongs to AfterimageBegin.
 * All changes occur on the caller's transactional player copy; HUD buffs are separate. */
bool SotnPlayer_TimersTick(SotnPlayer *player);
/** @brief Original signed minimum on timer14, without granting a visual or item. */
void SotnPlayer_Protect(SotnPlayer *player, Sint16 duration);
/** @brief Original bitwise protection gate for timers13 and14. */
bool SotnPlayer_Protected(const SotnPlayer *player);
/** @brief Original signed-word hitbox-center delta, then close-source velocity preference.
 * The source position already includes its facing-dependent hitbox offset. */
bool SotnPlayer_KnockRight(const SotnPlayer *player, Sint32 source_x, Sint32 source_velocity);
/** @brief Consume a resolved original ordinary damage kind with actual contact power/effects. */
bool SotnPlayer_ContactDamage(SotnPlayer *player, Uint16 amount, Sint32 source_x,
                              Sint32 source_velocity, Uint16 kind, Uint16 effects);
/** @brief Advance the actual original hurt controller after the common timer/contact pass. */
void SotnPlayer_HurtTick(SotnPlayer *player);
/** @brief Replace the shared simulated-input interval from an actual Bat/Wolf controller. */
void SotnPlayer_HoldInput(SotnPlayer *player, Uint32 ticks);
/** @brief Apply the selected original hurt/script exit helper after state dispatch. */
void SotnPlayer_ReactionRecovery(SotnPlayer *player);

/** @brief Apply selected original reset intents only when the common owner is bound. */
void SotnPlayer_AfterimageReset(SotnPlayer *player);
/** @brief Run original timer15 before movement, with no display/render update. */
bool SotnPlayer_AfterimageBegin(SotnPlayer *player);
/** @brief Consume actual queued contact/water once after healing and damage timers. */
bool SotnPlayer_ContactTick(SotnPlayer *player, bool *reaction_born);
/** @brief Apply original selected status gates and capture before collision. */
bool SotnPlayer_AfterimageCapture(SotnPlayer *player);
/** @brief Fade/select displays after collision and commit this engine clock once. */
bool SotnPlayer_AfterimagePresent(SotnPlayer *player);

/** @brief Advance original command/owned Wolf entry selection before state dispatch. */
void SotnPlayer_WolfInput(SotnPlayer *player);
/** @brief Advance original Wolf resource, entry and active controller paths. */
void SotnPlayer_WolfTick(SotnPlayer *player);
/** @brief Restore original Wolf sensors and wait on the actual return effect. */
void SotnPlayer_UnmorphWolfTick(SotnPlayer *player);
/** @brief Retire transient Wolf form drawing when original normal states interrupt it. */
void SotnPlayer_WolfCancel(SotnPlayer *player);
/** @brief Invoke selected original normal/stuck Wolf return without Bat/Mist position adjustment.
 */
void SotnPlayer_WolfReturn(SotnPlayer *player, bool cramped);

/** @brief Select eligible original Bat entry after shared transformation lockout. */
void SotnPlayer_BatInput(SotnPlayer *player);
/** @brief Advance selected original Bat initialization and ordinary flight. */
void SotnPlayer_BatTick(SotnPlayer *player);
/** @brief Restore sensors and wait for the actual Bat-return effect handshake. */
void SotnPlayer_UnmorphBatTick(SotnPlayer *player);
/** @brief Reset transient Bat drawing on original normal-form interruptions. */
void SotnPlayer_BatCancel(SotnPlayer *player);
/** @brief Apply source facing selection for transformation completion. */
void SotnPlayer_FormDirection(SotnPlayer *player);

/** @brief Actual Axe contact kind/direction; converted stone may retain zero damage. */
bool SotnPlayer_AxeContactDamage(SotnPlayer *player, Uint16 amount, Sint32 source_x,
                                 Sint32 source_velocity, Uint16 kind, bool zero_stone,
                                 Uint16 effects);
void SotnPlayer_StoneTick(SotnPlayer *player);
/** @brief Original heavy landing helper, transferring its smoke before later stone factories. */
bool SotnPlayer_StoneLanding(SotnPlayer *player);
/** @brief Append actual reaction calls without allocating a replacement effect or sound owner. */
void SotnPlayer_ReactionFactory(SotnPlayer *player, Uint32 blueprint);
void SotnPlayer_ReactionShake(SotnPlayer *player, Uint8 kind);
void SotnPlayer_ReactionSound(SotnPlayer *player, Uint16 sound);
/** @brief Delegate stone's no-dissolve lethal branch to the selected physical death owner. */
void SotnPlayer_StonePhysicalDeath(SotnPlayer *player, bool right);
/** @brief Select eligible original Mist entry after timers/input, before player dispatch. */
void SotnPlayer_MistInput(SotnPlayer *player);
/** @brief Advance selected original Mist steering/duration/MP and owned restoration requests. */
void SotnPlayer_MistTick(SotnPlayer *player);
/** @brief Restore original sensors and wait for the actual generation-owned visual child. */
void SotnPlayer_UnmorphMistTick(SotnPlayer *player);
/** @brief Advance original cramped-space recovery after restoration. */
void SotnPlayer_StuckTick(SotnPlayer *player);
/** @brief Apply original shared Jump/Fall/Crouch return helpers without bypassing effect waits. */
void SotnPlayer_FormReturn(SotnPlayer *player, bool cramped);
/** @brief Advance the owned sleep effect from private DRA frames; false reports invalid data. */
bool SotnPlayer_SleepTick(SotnPlayer *player);
/** @brief Advance the selected original owned dive-kick child after player motion selection. */
bool SotnPlayer_KickTick(SotnPlayer *player);
/** @brief Check the original down/up/jump combo before dispatching the player state. */
void SotnPlayer_GravityInput(SotnPlayer *player);
/** @brief Launch the selected original high jump, preserving the original movement flags. */
void SotnPlayer_GravityLaunch(SotnPlayer *player);
/** @brief Advance original high-jump states and selected ceiling effects. */
void SotnPlayer_GravityTick(SotnPlayer *player);
/** @brief Advance owned cosmetic beams after player collision. */
void SotnPlayer_GravityBeams(SotnPlayer *player);
/** @brief Restore original facing when another player state interrupts an inverted high jump. */
void SotnPlayer_GravityCancel(SotnPlayer *player);
void SotnPlayer_StoneDeathTick(SotnPlayer *player);
/** @brief Actual state16 phase1/common capture work, requiring bound resident image services. */
void SotnPlayer_PhysicalDeathTick(SotnPlayer *player);
void SotnPlayer_Collide(SotnPlayer *player);
/** @brief Original wrapping/truncating pre/post-collision water speed scaling. */
Sint32 SotnPlayer_WaterScale(Sint32 velocity, bool restore);
/** @brief Actual feet/form/status/entity-byte gate, independent of player damage immunity. */
bool SotnPlayer_WaterDrag(const SotnPlayer *player);
/** @brief Original eligible Stand/Crouch0 breathing branch with its shared retained counter. */
void SotnPlayer_WaterBreath(SotnPlayer *player, bool crouched, bool shielded);
Uint8 SotnPlayer_TightSpace(SotnPlayer *player);

#endif
