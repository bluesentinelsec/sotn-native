#ifndef SOTN_PLAYER_INTERNAL_H
#define SOTN_PLAYER_INTERNAL_H

#include <SDL3/SDL.h>
#include <sotn/equipment.h>
#include <sotn/mist_motion.h>
#include <sotn/player.h>
#include <sotn/shield.h>
#include <sotn/spell_commands.h>

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

struct SotnPlayer
{
    SotnDataView dra;
    SotnCollisionMap map;
    PlayerFixed x, y;
    Sint32 vx, vy, contact, previous_contact;
    Sint16 draw_pivot_x, draw_pivot_y;
    Uint32 status, frame_number, movement_flags;
    SotnPlayerAfterimage afterimage;
    SotnPlayerMovementEffects movement_effects;
    SotnPlayerHealing healing;
    SotnPlayerDark dark;
    SotnSpellCommands spell_commands;
    bool dark_context, spirit_context, soul_context;
    SotnPlayerSoul soul;
    SotnPlayerSpirit spirit;
    SotnPlayerHellfireFactory hellfire_factory;
    void *hellfire_context;
    Sint32 hellfire_camera_x;
    Uint16 hellfire_signal, hellfire_palette;
    bool hellfire_started;
    Uint8 spell_use_ticks;
    Uint32 afterimage_generation, afterimage_clock, afterimage_last_clock;
    Uint16 afterimage_timer15;
    bool afterimage_pending, afterimage_ticked;
    SotnAnimation animation;
    SotnPlayerStep step, previous_step;
    Uint16 buttons, previous_buttons, tapped, substep;
    Uint16 chair_kind, chair_sleep;
    bool sleep_event;
    SotnPlayerSleep sleep;
    Uint16 attack_mode, attack_button;
    Uint8 attack_lock, weapon_startup;
    SotnSubweapons *subweapons;
    bool subweapon_cast;
    Uint16 subweapon_animation;
    SotnPlayerAttack attack, shield;
    SotnPlayerKick kick;
    Uint32 kick_counter;
    Uint16 kick_hit;
    SotnPlayerGravity gravity;
    Uint16 mp, mp_spent, held;
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
    Uint8 bat_input_hold;
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
    SotnPlayerStone stone;
    SotnPlayerGrab grab;
    Uint16 hp, hp_max, invincibility, sound, extra_sound, curse_ticks, poison_ticks;
    bool curse_failure, script_hold, bell_inside;
    SotnCharacterStats attributes;
    Uint32 damage_random;
    Uint8 timers[8], tight_space;
    bool facing_left, turned, fixed_stage, collision_disabled, failed;
    SotnCollider floor[4], ceiling[4], wall[14];
    PlayerSensor floor_sensors[4], ceiling_sensors[4], wall_sensors[14];
};

/** @brief Rebuild supported original status bits from current state/timers, without mutation. */
Uint32 SotnPlayer_Status(const SotnPlayer *player);

/** @brief Apply selected original reset intents only when the common owner is bound. */
void SotnPlayer_AfterimageReset(SotnPlayer *player);
/** @brief Run original timer15 before movement, with no display/render update. */
bool SotnPlayer_AfterimageBegin(SotnPlayer *player);
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

void SotnPlayer_StoneTick(SotnPlayer *player);
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
void SotnPlayer_Collide(SotnPlayer *player);
Uint8 SotnPlayer_TightSpace(SotnPlayer *player);

#endif
