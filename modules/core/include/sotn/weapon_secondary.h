#ifndef SOTN_WEAPON_SECONDARY_H
#define SOTN_WEAPON_SECONDARY_H
#include <sotn/collision.h>
#include <sotn/equipment.h>
#include <sotn/game_data.h>
#include <sotn/hand_weapon.h>
#include <sotn/player_blink.h>
#include <sotn/spell_target.h>

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
        /** Weapon49 sparks/puffs/swirls/pillars, weapon6 crescents and the weapon13 root (no
         * rows or GT4s). */
        struct
        {
            Sint32 velocity_y;       /**< Puff/pillar rise. */
            Sint16 prev_x, prev_y;   /**< Spark: last whole screen position (ratan2 heading). */
            Sint16 angle, magnitude; /**< Spark/swirl orbit unk80/unk7E. */
            Sint32 accel_x, accel_y; /**< Weapon6 crescent accelerations. */
            Sint16 spin, counter;    /**< Weapon6 crescent velocityZ and unk7C. */
            /** Weapon13 root: ext.weapon.childPalette and the s32 unk98 swish volume. */
            Sint16 child_palette;
            Sint32 volume;
        };
        /** Weapon12 E0 slash: ext.sword corner offsets x0..x3 (unk90/92/94/96), y0..y3
         * (unk98/88/9A/8A), spread steps unk82/84, spreads unk9C/9E, shape unk86, drop unkA0. */
        struct
        {
            Sint16 slash_x[4], slash_y[4], slash_step[2], slash_spread[2];
            Sint16 slash_shape, slash_drop;
        };
        /** Weapon14 Runesword root: forty-eight GT4 trail rows kept as blade corners x1/y1 and
         * x3/y3 (row i's x0/x2 repeat row i-1's), UVs and the r0 shown byte. The original
         * DRAW_UNK02|UNK_100 screen vertices follow the camera, so corners are world pixels. */
        struct
        {
            Sint32 rune_x1[48], rune_y1[48], rune_x3[48], rune_y3[48];
            Sint32 rune_x0, rune_y0, rune_x2, rune_y2; /**< Row0's birth corners. */
            Sint32 rune_velocity_y;
            Uint8 rune_u0[48], rune_u1[48]; /**< u0=u2 and u1=u3. */
            bool rune_shown[48];            /**< Original r0: uncovered once. */
            bool rune_hidden[48];           /**< Original DRAW_HIDE. */
            Sint16 rune_count;              /**< ext.weapon_014.unk9C: completed callbacks. */
        };
        /** Weapon30 Sword of Dawn circle (E1) and soldiers (E2): up to fourteen GT4 rows whose
         * vertices are offsets from the entity's whole pixels (DRAW_UNK02 follows the camera).
         * Circle words: ext.weapon unk7E band, unk80/82 half extents, weapon_030.unk84 wave,
         * childPalette tether fade, unk8A count, unk90 soldiers, unk92 next type, unk94/96 the
         * player's birth screen Y/X, unk98/9A the tether's screen target, unk9C target flag.
         * Soldier words: weapon_030 unk7C cooldown, unk7E blink, unk86 shots, unk88/9C rise
         * rates, unk90/A0 rise offsets, unkA4 lives. */
        struct
        {
            SotnPlayerEffectQuad dawn_quads[14];
            bool dawn_line; /**< Circle row10 became the LINE_G2 tether. */
            Sint16 dawn_band, dawn_half_w, dawn_half_h, dawn_wave, dawn_fade, dawn_count;
            Sint16 dawn_soldiers, dawn_type, dawn_birth_y, dawn_birth_x, dawn_target_x,
                dawn_target_y;
            Sint32 dawn_target;
            Sint16 dawn_cooldown, dawn_blink, dawn_shots, dawn_lives;
            Sint32 dawn_rate1, dawn_rate2, dawn_rise1, dawn_rise2;
            Sint32 dawn_velocity_y;
            /** Weapon30 arrows/swords (E3): ext.weapon unk80 heading, unk82 side, unk98 turn
             * limit, unk9A speed and the retained raw target slot (func_80118970's pointer). */
            Sint16 dawn_angle, dawn_side, dawn_limit, dawn_speed, dawn_volume;
            Uint16 dawn_slot;
            bool dawn_bound;
        };
        /** Shield spells (E8): eleven GT4 rows in screen pixels (no DRAW_UNK02) and ext.shield
         * words unk80 timer, unk82 glow half size, unk84/86 card half extents, unk8A angle step,
         * unk90 band brightness, unk92/94 wave amplitudes, childPalette and unk7C/7D rows. */
        struct
        {
            SotnPlayerEffectQuad spell_quads[11];
            Sint16 spell_timer, spell_glow, spell_half_w, spell_half_h, spell_step;
            Sint16 spell_bright, spell_wave_x, spell_wave_y;
            Uint16 spell_child_palette;
            Uint8 spell_v0, spell_v1;
            Sint32 spell_velocity_y;
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
    Uint8 role; /**< Child callback recorded at birth (0..5; 0 is weapon14's thrown root). */
    Uint8 hitbox_width, hitbox_height, draw_flags, phase;
    Uint8 opacity; /**< Entity opacity (ENTITY_OPACITY): weapon13 afterimages. */
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
    /** Actual player animation hitbox offsets (PLAYER.hitboxOffX/Y): weapon6 crescent origin. */
    Sint16 parent_hitbox_x, parent_hitbox_y;
    /** Weapon14 root: PLAYER.velocityX, hitboxWidth/Height and step==Player_Crouch. */
    Sint32 player_velocity_x;
    Uint8 player_hitbox_width, player_hitbox_height;
    bool player_crouch_step;
    /** Weapon14 hitboxes: the raw parent slot's current entity words (absent: entityId0). */
    bool parent_exists, parent_facing_left;
    Sint32 parent_x, parent_y;
    Sint16 parent_rotate;
    Uint8 parent_draw_flags;
    /** Weapon13 afterimages copy the raw parent's frame, animSet, unk5A and childPalette. */
    Uint16 parent_frame, parent_bank, parent_texture, parent_child_palette;
    Uint32 game_timer;     /**< Actual wrapping g_GameTimer (weapon13/12 schedules). */
    Uint16 player_substep; /**< PLAYER.step_s (weapon12 slashes drop while crouched). */
    /** Weapon30: current room collision for CheckCollision (screen plus camera). */
    SotnCollisionMap collision;
    /** Optional collision service replacing `collision` (component traces); NULL uses the map.
     * Receives world pixels; must fill `out` and return false with SDL_GetError() on failure. */
    bool (*query)(void *context, Sint32 x, Sint32 y, SotnCollider *out);
    void *query_context;
    /** Weapon30 circle: free factory entities at this callback (CreateEntFactoryFromEntity
     * returns NULL beyond them) and the raw entityId/screen X of slots17..31. */
    Uint8 free_factories;
    Uint16 slot_kind[15];
    Sint16 slot_x[15];
    /** Weapon30 soldier: its raw parent circle's weapon_030.unk84 wave (zero once retired). */
    Sint16 parent_wave;
    /** Weapon30 walkers: g_Tilemap.x/width, the current camera bounds in world pixels. */
    Sint32 room_left, room_right;
    /** Weapon30 bomb: whether its raw parent soldier slot still holds an entity, and its pose. */
    Uint16 parent_pose;
    /** Weapon30 homing swords: the retained target slot's current stage words (screen). */
    SotnSpellTarget target;
    Uint32 engine_timer; /**< Actual g_Timer (shield spells' flicker, unlike g_GameTimer). */
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
    /** Weapon6 crescents: DestroyEntityWeapon(true) and the player timer10 use request. */
    bool clear_shield;
    Uint8 use_ticks;
    /** Weapon14/13/12: sounds[0] is PlaySfxVolPan(sound, volume, pan0). */
    bool volume_sound;
    Uint16 volume;
    /** Weapon12 slashes: g_Player.timers[9] (hand-weapon startup) request. */
    Uint8 startup_ticks;
    /** Weapon30: factories created this callback, in order, each placed at its own world
     * position (the factory entity's posX/posY high words were overwritten). */
    Uint8 factory_count;
    Uint32 factories[8];
    Sint32 factory_x[8], factory_y[8];
    Uint8 shake; /**< Weapon30 ShakeCamera request (0: none). */
    /** Weapon30 homing swords: func_80118970 selects (and marks) a target now. */
    bool select_target;
    /** Shield spells: g_unkGraphicsStruct.unk28 (1 enters the spell freeze, 0 leaves it). */
    bool freeze_set, freeze_clear;
    /** Shield spells: FACTORY(55, n) pink medicine effect profile, zero when absent. */
    Uint8 pink;
} SotnWeaponSecondaryEvents;

/** @brief Identify selected secondary programs (Holy sword55, Rapier38, Were bane39, Katana53,
 * Thunderbrand48). */
bool SotnWeaponSecondary_Supported(Uint16 family);
/** @brief Whether a resident program owns the given child callback role (0..5; role0 is only
 * weapon14's factory-born thrown EntityWeaponAttack). */
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
