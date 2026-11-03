#ifndef SOTN_HAND_WEAPON_H
#define SOTN_HAND_WEAPON_H
#include <sotn/equipment.h>
#include <sotn/player_blink.h>

#define SOTN_HAND_WEAPON_TILES 16
/** @brief Actual per-hand writable weapon37/52 program data; never save data. */
typedef struct SotnHandWeaponProgram
{
    Uint32 short_pose;
    Sint32 shield_attack;
} SotnHandWeaponProgram;
/** @brief Selected ordinary shared weapon entity, including optional weapon43 tiles. */
typedef struct SotnHandWeapon
{
    SotnPlayerEffectQuad tiles[SOTN_HAND_WEAPON_TILES];
    Uint8 tile_age[SOTN_HAND_WEAPON_TILES], tile_delay[SOTN_HAND_WEAPON_TILES];
    bool tile_started[SOTN_HAND_WEAPON_TILES];
    SotnEquipment properties;
    Sint32 x, y, velocity_x, velocity_y;
    Uint32 flags;
    Sint16 hitbox_x, hitbox_y, remaining, timer, rotation, scale_y, pivot_y;
    Uint16 bank, texture, palette, priority, frame, animation, pose, parameter;
    Uint16 hitbox_state;
    Uint8 hitbox_width, hitbox_height, phase, draw_flags, blend;
    bool facing_left, removed, allocated;
} SotnHandWeapon;
/** @brief Actual GetEquipProperties adapter. It receives a working RNG cursor.
 * It must not mutate externally committed state; group ownership remains with the caller. */
typedef bool (*SotnHandWeaponProperties)(void *, Uint32 *random, SotnEquipment *);
typedef struct SotnHandWeaponInput
{
    SotnDataView overlay;
    SotnHandWeaponProperties properties;
    void *context;
    Sint32 x, y, velocity_x;
    Uint32 status, contacts;
    Sint16 remaining, scale_y, pivot_y;
    Uint16 hand, family, animation, pose, priority, step, substep, attack_mode;
    Uint8 draw_flags;
    bool facing_left, held, allocated;
} SotnHandWeaponInput;
typedef struct SotnHandWeaponEvents
{
    Uint32 factory;
    Uint16 sound;
    Uint8 shield; /**< Zero: no write; one: original unk48=1; two: original unk48=0. */
    bool properties;
} SotnHandWeaponEvents;
/** @brief Execute a complete selected ordinary weapon0/2/31/37/43/52 or unarmed255 callback.
 * Unarmed255 borrows DRA instead of a weapon overlay and copies no sprite transforms.
 * Private animations remain borrowed. Actual factories, slot16 replacement, weapon properties,
 * groups, optional weapon43 leases, rendering and combat are caller obligations. Outputs and
 * the working RNG/program cursors commit together only on success. No controls are enabled. */
bool SotnHandWeapon_Tick(SotnHandWeapon *, SotnHandWeaponProgram *, SotnHandWeaponInput,
                         Uint32 *random, SotnHandWeaponEvents *);
#endif
