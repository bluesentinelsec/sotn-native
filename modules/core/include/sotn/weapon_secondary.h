#ifndef SOTN_WEAPON_SECONDARY_H
#define SOTN_WEAPON_SECONDARY_H
#include <sotn/equipment.h>
#include <sotn/game_data.h>
#include <sotn/hand_weapon.h>
#include <sotn/player_blink.h>

/** Maximum original LINE_G2 rows owned by one selected weapon secondary. */
#define SOTN_WEAPON_SECONDARY_ROWS 64
/** Largest single child row lease (weapon50's eighty sparkle GT4s). */
#define SOTN_BAT_CHILD_ROWS 80

/** @brief One original LINE_G2 row. Endpoint colors start zero (DestroyPrimitive); angle/length/
 * growth are weapon55's x2/x3/y2 scratch words; timer/phase are weapon38/39's r2/r3 bytes.
 * Coordinates are screen words. */
typedef struct SotnWeaponSecondaryRow
{
    Sint16 x[2], y[2];
    Sint16 angle, length, growth;
    Uint8 red[2], green[2], blue[2];
    Uint8 timer, phase;
    bool hidden; /**< Original DRAW_HIDE. */
} SotnWeaponSecondaryRow;

/** @brief Selected weapon func_ptr_80170004 child (blueprint56 E1/F1) of a resident program.
 * Positions are world coordinates; rows hold screen coordinates. Never saved. */
typedef struct SotnWeaponSecondary
{
    /** A child uses LINE_G2 rows, GT4s or weapon49 motion words, never two of them. */
    union {
        SotnWeaponSecondaryRow rows[SOTN_WEAPON_SECONDARY_ROWS];
        struct
        {
            /** Weapon48 role2 ring wedges or weapon1 role1 punches: sixteen screen-space GT4s. */
            SotnPlayerEffectQuad quads[16];
            Uint8 quad_timer[16], quad_phase[16]; /**< Weapon1 punch r0/r1 scratch bytes. */
        };
        /** Weapon50 E1 sparkles: eighty GT4 rows kept as centre, UVs and the original prim
         * scratch bytes g0/g1/g2/b0/b1 (rebuilt into packets at draw time). */
        struct
        {
            Sint16 ice_cx[80], ice_cy[80];
            Uint8 ice_uv[80][8];
            Uint8 ice_g0[80], ice_g1[80], ice_g2[80], ice_b0[80], ice_b1[80];
            Uint8 ice_size[80]; /**< Half-extent of the last placement (zero: hidden). */
        };
        /** Weapon49 sparks/puffs/swirls/pillars, which own no rows or GT4s. */
        struct
        {
            Sint32 velocity_y;       /**< Puff/pillar rise. */
            Sint16 prev_x, prev_y;   /**< Spark: last whole screen position (ratan2 heading). */
            Sint16 angle, magnitude; /**< Spark/swirl orbit unk80/unk7E. */
        };
    };
    /** Weapon53 GT4 slash. Vertices are offsets from the entity's whole-pixel position: the
     * original screen vertices carry DRAW_UNK02 and follow the camera like the entity. */
    SotnPlayerEffectQuad quad;
    SotnEquipment properties;
    Sint32 x, y;
    Uint32 flags;
    Sint16 hitbox_x, hitbox_y, scale_y, pivot_y;
    Sint16 remaining; /**< Original poseTimer; -1 after the END control. */
    Uint16 parameter; /**< Actual factory child params (paramsBase plus index). */
    Uint16 bank, palette, texture, priority, row_priority, blend, pose, frame, hitbox_state;
    Uint16 draw_mode; /**< Shared original row drawMode without DRAW_HIDE (ABR in bits5..6). */
    Sint16 rotate, lifetime; /**< Weapon53/51 rotation and private-table radius/timer. */
    Sint32 velocity_x;       /**< Weapon1 role2 fireball speed. */
    Sint16 scale_x;          /**< Weapon1 role2 fireball X scale (scale_y shared). */
    Uint8 hit_flags;         /**< Actual collision hitFlags recorded since the last callback. */
    Uint16 age;              /**< Weapon53 unk7E: completed callbacks, retired at nine. */
    Uint16 timeline;         /**< Private AnimationFrame list chosen at birth (zero: family's). */
    Uint8 role;              /**< Child callback recorded at birth (1..5). */
    Uint8 hitbox_width, hitbox_height, draw_flags, phase;
    bool facing_left, removed, allocated;
} SotnWeaponSecondary;

/** @brief Copied native context for one secondary callback. */
typedef struct SotnWeaponSecondaryInput
{
    SotnDataView overlay;    /**< Actual resident hand program bytes. */
    SotnDataView executable; /**< Private SDK rsin/rcos tables. */
    /** GetEquipProperties adapter for the raw parent's equip id with this hand. */
    SotnHandWeaponProperties properties;
    void *context;
    Sint32 x, y;               /**< Actual player world position (16.16). */
    Sint32 camera_x, camera_y; /**< Actual camera; rows are screen-space. */
    Sint16 scale_y, pivot_y;
    Uint16 hand, family, priority, animation, attack_mode;
    Uint16 role; /**< Child callback: 1 func_ptr_80170004 (E1/F1), 2 func_ptr_80170008 (E2/F2). */
    Uint16 substep; /**< Actual player step_s; weapon38/39 retire outside0x5A. */
    Uint8 draw_flags;
    bool facing_left;
    bool crouch;    /**< Actual player status crouch bit0x20 (weapon53 vertical offset). */
    bool allocated; /**< The mandatory rows were reserved before the first callback. */
    /** Raw current slot16 params (zero when empty): weapon51's fan colour index. */
    Uint16 slot16_parameter;
    /** Raw parent's current attack fields (weapon49 sparks copy them instead of a roll). */
    SotnEquipment parent_properties;
    Uint16 parent_hitbox_state;
} SotnWeaponSecondaryInput;

/** @brief One-update requests; none is a completed native operation. */
typedef struct SotnWeaponSecondaryEvents
{
    Uint16 sounds[2]; /**< Ordered original PlaySfx requests; zero when absent. */
    /** Weapon53 roots: packed WFACTORY(0x38,n) from this child (its own raw parent). */
    Uint32 chain;
    /** SetWeaponProperties ran: the caller increments the least-used group, then copies the raw
     * parent's enemyId over it. */
    bool properties;
    /** Weapon49 sparks: func_80118894 alone (least-used group kept, no parent enemyId copy). */
    bool group;
} SotnWeaponSecondaryEvents;

/** @brief Identify selected secondary programs (Holy sword55, Rapier38, Were bane39, Katana53,
 * Thunderbrand48). */
bool SotnWeaponSecondary_Supported(Uint16 family);
/** @brief Whether a resident program owns the given child callback role (1 or 2). */
bool SotnWeaponSecondary_SupportedRole(Uint16 family, Uint16 role);
/** @brief Mandatory rows for one role's child; zero if unsupported. */
Uint8 SotnWeaponSecondary_RoleRows(Uint16 family, Uint16 role);
/** @brief Number of mandatory rows (LINE_G2, or weapon53's one GT4) reserved before the first
 * callback; zero if none. */
Uint8 SotnWeaponSecondary_Rows(Uint16 family);

/** @brief Run one original selected func_ptr_80170004 callback.
 * @param state Actual child words, unchanged on error.
 * @param program Actual per-hand program BSS, committed only on success. @param input Context.
 * @param random Working character libc cursor, committed only on success.
 * @param events Requests, unchanged on error.
 * @return Success or SDL error. Exhausted mandatory rows retire the child before any RNG or
 * property roll. Culling and the post-callback UpdateAnim follow separately. */
bool SotnWeaponSecondary_Tick(SotnWeaponSecondary *state, SotnHandWeaponProgram *program,
                              SotnWeaponSecondaryInput input, Uint32 *random,
                              SotnWeaponSecondaryEvents *events);

/** @brief Weapon53 kind135 warp afterimage: the slot16 E2/F2 child of blueprint60 that redraws
 * Alucard's current frame while sliding out and back. Positions are world 16.16. Never saved. */
typedef struct SotnWeaponWarp
{
    Sint32 x, y, velocity_x, velocity_y;
    Uint32 flags;
    Sint16 scale_y, pivot_y;
    Uint16 parameter, phase, item; /**< item: factory argument (the special's equip id). */
    Uint8 blend, draw_flags;
    bool facing_left, removed;
} SotnWeaponWarp;
/** @brief Copied player context for one warp callback. */
typedef struct SotnWeaponWarpInput
{
    Sint16 scale_y, pivot_y;
    Sint16 remaining; /**< Player poseTimer. */
    Uint16 hand, pose;
    Uint8 draw_flags;
    bool facing_left;
} SotnWeaponWarpInput;
/** @brief One-update warp requests: ordered WFACTORY(0x38,0x28/0x1E) slash roots. */
typedef struct SotnWeaponWarpEvents
{
    Uint32 factories[2];
} SotnWeaponWarpEvents;
/** @brief Run one original w_053 func_ptr_80170008 callback.
 * @param state Actual child, unchanged on error. @param input Copied player context.
 * @param events Requests, unchanged on error. @return Success or SDL error. */
bool SotnWeaponWarp_Tick(SotnWeaponWarp *state, SotnWeaponWarpInput input,
                         SotnWeaponWarpEvents *events);

/** @brief Original post-callback UpdateAnim for children flagged 0x100000 after culling.
 * @param state Actual child, unchanged on error. @param overlay Resident program bytes.
 * @param family Actual resident program. @return Success or SDL error. Draws never advance it. */
bool SotnWeaponSecondary_Animate(SotnWeaponSecondary *state, SotnDataView overlay, Uint16 family);
#endif
