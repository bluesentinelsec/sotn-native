#ifndef SOTN_EQUIPMENT_H
#define SOTN_EQUIPMENT_H

#include <sotn/game_data.h>

/** @brief Selected hand-item metadata copied from the private US DRA image. */
typedef struct SotnEquipment
{
    Uint16 id, element, stun, hit_type, hit_effect, critical_rate;
    Sint16 attack;
    Uint8 family, palette, kind, animation, variant, lock, invincibility;
} SotnEquipment;

/** @brief Resolved frame of the original player-locked weapon entity. */
typedef struct SotnWeaponFrame
{
    Sint16 x, y;
    Uint16 width, height, sprite, sound;
} SotnWeaponFrame;

/** @brief Read a supported hand-item definition, without importing runtime tables.
 * @param dra Borrowed immutable DRA bytes. @param id Empty hand (0), Basilard (18), Gladius (86),
 * Mace (128), Alucard sword (123), or Alucard shield (16).
 * @param out Receives copied metadata, unchanged on failure.
 * @return Success, or false with SDL_GetError() for invalid or unsupported definitions.
 * @note Other items and their special effects require separate integration. */
bool SotnEquipment_Read(SotnDataView dra, Uint16 id, SotnEquipment *out);
/** @brief Borrow the selected weapon overlay from private WEAPON0.BIN.
 * @param weapons Borrowed immutable container. @param equipment Supported nonempty item.
 * @param out Receives an overlay view, expiring with weapons, unchanged on failure.
 * @return Success, or false with SDL_GetError() for unsupported family or truncated data. */
bool SotnWeapon_Overlay(SotnDataView weapons, SotnEquipment equipment, SotnDataView *out);
/** @brief Resolve the weapon entity hitbox, sprite, and one-shot sound at a player pose.
 * @param weapons Borrowed immutable container. @param equipment Selected item definition.
 * @param animation Current original player animation ID. @param pose Current player pose.
 * @param remaining Original pose timer; a negative value ends the weapon entity.
 * @param out Receives the frame, unchanged on malformed data; zero means inactive.
 * @return Success, or false with SDL_GetError() for invalid runtime references. */
bool SotnWeapon_ReadFrame(SotnDataView weapons, SotnEquipment equipment, Uint16 animation,
                          Uint16 pose, Sint32 remaining, SotnWeaponFrame *out);

#endif
