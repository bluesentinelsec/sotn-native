#ifndef SOTN_PLAYER_INTERNAL_H
#define SOTN_PLAYER_INTERNAL_H

#include <SDL3/SDL.h>
#include <sotn/equipment.h>
#include <sotn/player.h>
#include <sotn/shield.h>

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
    Uint32 status, frame_number, movement_flags;
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

void SotnPlayer_StoneTick(SotnPlayer *player);
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
