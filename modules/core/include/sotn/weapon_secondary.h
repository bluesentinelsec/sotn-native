#ifndef SOTN_WEAPON_SECONDARY_H
#define SOTN_WEAPON_SECONDARY_H
#include <sotn/equipment.h>
#include <sotn/game_data.h>
#include <sotn/hand_weapon.h>

/** Maximum original LINE_G2 rows owned by one selected weapon secondary. */
#define SOTN_WEAPON_SECONDARY_ROWS 64

/** @brief One original LINE_G2 row. Its first endpoint colors stay zero (DestroyPrimitive);
 * angle/length/growth are the callback's x2/x3/y2 scratch words. Coordinates are screen words. */
typedef struct SotnWeaponSecondaryRow
{
    Sint16 x[2], y[2];
    Sint16 angle, length, growth;
    Uint8 red, green, blue;
} SotnWeaponSecondaryRow;

/** @brief Selected weapon func_ptr_80170004 child (blueprint56 E1/F1) of a resident program.
 * Positions are world coordinates; rows hold screen coordinates. Never saved. */
typedef struct SotnWeaponSecondary
{
    SotnWeaponSecondaryRow rows[SOTN_WEAPON_SECONDARY_ROWS];
    SotnEquipment properties;
    Sint32 x, y;
    Uint32 flags;
    Sint16 hitbox_x, hitbox_y, scale_y, pivot_y;
    Sint16 remaining; /**< Original poseTimer; -1 after the END control. */
    Uint16 parameter; /**< Actual factory child params (paramsBase plus index). */
    Uint16 bank, palette, texture, priority, row_priority, blend, pose, frame, hitbox_state;
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
    Uint8 draw_flags;
    bool facing_left;
    bool allocated; /**< The mandatory rows were reserved before the first callback. */
} SotnWeaponSecondaryInput;

/** @brief One-update requests; none is a completed native operation. */
typedef struct SotnWeaponSecondaryEvents
{
    /** SetWeaponProperties ran: the caller increments the least-used group, then copies the raw
     * parent's enemyId over it. */
    bool properties;
} SotnWeaponSecondaryEvents;

/** @brief Identify selected secondary programs (Holy sword weapon55). */
bool SotnWeaponSecondary_Supported(Uint16 family);
/** @brief Number of mandatory LINE_G2 rows reserved before the first callback; zero if none. */
Uint8 SotnWeaponSecondary_Rows(Uint16 family);

/** @brief Run one original selected func_ptr_80170004 callback.
 * @param state Actual child words, unchanged on error. @param input Copied context.
 * @param random Working character libc cursor, committed only on success.
 * @param events Requests, unchanged on error.
 * @return Success or SDL error. Exhausted mandatory rows retire the child before any RNG or
 * property roll. Culling and the post-callback UpdateAnim follow separately. */
bool SotnWeaponSecondary_Tick(SotnWeaponSecondary *state, SotnWeaponSecondaryInput input,
                              Uint32 *random, SotnWeaponSecondaryEvents *events);

/** @brief Original post-callback UpdateAnim for children flagged 0x100000 after culling.
 * @param state Actual child, unchanged on error. @param overlay Resident program bytes.
 * @param family Actual resident program. @return Success or SDL error. Draws never advance it. */
bool SotnWeaponSecondary_Animate(SotnWeaponSecondary *state, SotnDataView overlay, Uint16 family);
#endif
