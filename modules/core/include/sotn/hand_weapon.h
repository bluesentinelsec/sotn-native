#ifndef SOTN_HAND_WEAPON_H
#define SOTN_HAND_WEAPON_H
#include <sotn/equipment.h>
#include <sotn/player_blink.h>

#define SOTN_HAND_WEAPON_TILES 16
/** Mandatory original weapon1 GT4 punch-trail rows. */
#define SOTN_HAND_WEAPON_FIST_ROWS 4
/** Optional original weapon54 LINE_G2 trail rows. */
#define SOTN_HAND_WEAPON_GURTHANG_ROWS 12
/** @brief Actual per-hand writable weapon37/52 program data; never save data. */
typedef struct SotnHandWeaponProgram
{
    Uint32 short_pose;
    Sint32 shield_attack;
    /** Weapon38/39 secondary BSS: lunge countdown (D_8017B844/D_8017BA28) and weapon39's
     * retained chain endpoints (D_8017BA2C/D_8017BA6C). Zeroed with a genuine program load. */
    Sint32 secondary_counter;
    union {
        struct
        {
            Sint16 secondary_x[32], secondary_y[32];
        };
        /** Weapon50 D_8017CBFC: the sparkle points shared by all of the program's E1/E4. */
        struct
        {
            Sint16 ice_x[80], ice_y[80];
        };
    };
    Sint32 previous_pose; /**< Weapon48 D_8017C3B4: the player pose seen last callback. */
    /** Weapon51 program BSS shared by its attack and special: the glow colour's _r/_g/_b,
     * color_step and _a words (D_8017AD68..D_8017AD70). */
    Sint16 glow_red, glow_blue, glow_green, glow_step, glow_alpha;
} SotnHandWeaponProgram;
/** @brief Selected ordinary shared weapon entity: optional weapon43 TILEs or weapon1 GT4 trails. */
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
    /** Borrowed private executable for weapon1 SDK sqrt/atan2/trig tables. */
    SotnDataView executable;
    /** Actual current camera; weapon1 trails are screen-space GT4s. */
    Sint32 camera_x, camera_y;
    /** Actual player Dark Metamorphosis timer word11 (weapon54 descriptor/attack/request). */
    Uint16 dark_ticks;
    SotnHandWeaponProperties properties;
    void *context;
    Sint32 x, y, velocity_x;
    Uint32 status, contacts;
    Sint16 remaining, scale_y, pivot_y;
    Uint16 hand, family, animation, pose, priority, step, substep, attack_mode;
    Uint8 draw_flags;
    bool facing_left, held, allocated;
    /** Weapon51 slot16 E1/F1 special (func_ptr_80170004) instead of EntityWeaponAttack. */
    bool special;
} SotnHandWeaponInput;
typedef struct SotnHandWeaponEvents
{
    Uint32 factory;
    /** Further ordered WFACTORY requests after factory (weapon48); zero-terminated. */
    Uint32 more_factories[8];
    Uint16 sound;
    /** Ordered additional sounds after sound (weapon33 pose12 pair, weapon50 pose1 trio);
     * zero when absent. */
    Uint16 extra_sounds[3];
    /** Weapon54: raise player timer11 to this minimum (0x38); zero when absent. */
    Uint16 dark_minimum;
    /** Weapon54 special: original FACTORY(61,0x11) outline request; zero when absent. */
    Uint32 outline;
    Uint8 shield; /**< Zero: no write; one: original unk48=1; two: original unk48=0. */
    bool properties;
    /** Weapon51 glow: one g_Clut word written and uploaded this callback (clut set). */
    bool clut;
    Uint16 clut_palette, clut_color, clut_value;
} SotnHandWeaponEvents;
/** @brief Identify selected ordinary shield9/11/26/52 callbacks, without enabling equipment.
 * @param family Actual private program identity. @return Whether this callback is reconstructed.
 * Resource, property, factory, blocking and native presentation ownership remain required. */
bool SotnHandWeapon_IsShieldFamily(Uint16 family);
/** @brief Execute a complete selected ordinary weapon0/1/2/3/9/11/26/31/33/35/36/37/38/39/43/52/
 * 55/57 or unarmed255 callback. Weapon55 descriptor1 (at birth) and weapon38/39 descriptor1
 * (pose4's final update) report their WFACTORY(0x38) secondary requests through events. Unarmed255
 * borrows DRA instead of a weapon overlay and copies no sprite transforms. Weapon1
 * (descriptors0..3) requires its four mandatory rows: allocated false destroys it before
 * flags/properties/RNG; its trails use the supplied camera and executable SDK tables. Private
 * animations remain borrowed. Actual factories, slot16 replacement, weapon properties, groups,
 * optional weapon43 leases, rendering and combat are caller obligations. Outputs and the working
 * RNG/program cursors commit together only on success. No controls are enabled. */
bool SotnHandWeapon_Tick(SotnHandWeapon *, SotnHandWeaponProgram *, SotnHandWeaponInput,
                         Uint32 *random, SotnHandWeaponEvents *);
#endif
