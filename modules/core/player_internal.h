#ifndef SOTN_PLAYER_INTERNAL_H
#define SOTN_PLAYER_INTERNAL_H

#include <SDL3/SDL.h>
#include <sotn/player.h>

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
    Uint16 attack_mode, attack_button;
    Uint8 attack_lock;
    SotnPlayerAttack attack;
    Uint16 hp, invincibility, sound;
    Uint32 damage_random;
    Uint8 timers[8], tight_space;
    bool facing_left, turned, fixed_stage, collision_disabled, failed;
    SotnCollider floor[4], ceiling[4], wall[14];
    PlayerSensor floor_sensors[4], ceiling_sensors[4], wall_sensors[14];
};

void SotnPlayer_Collide(SotnPlayer *player);
Uint8 SotnPlayer_TightSpace(SotnPlayer *player);

#endif
