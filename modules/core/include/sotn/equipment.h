#ifndef SOTN_EQUIPMENT_H
#define SOTN_EQUIPMENT_H

#include <sotn/game_data.h>

/** @brief Selected hand-item metadata copied from the private US DRA image. */
typedef struct SotnEquipment
{
    Uint16 id, element, stun, hit_type, hit_effect, critical_rate;
    Sint16 attack;
    Uint8 family, palette, kind, animation, variant, lock, invincibility, chain_limit;
    /** Original QCF specialMove and back-forward unk17 virtual items; zero when absent. */
    Uint8 special_move, back_forward;
    Uint16 blueprint; /**< Actual copied factory identity; avoids implicit byte-tail padding. */
    /** Original isConsumable byte (ReduceWeapon decrements only when set); 16-bit so the
     * struct keeps no implicit tail padding. Read for kind129/130 items only. */
    Uint16 consumable;
} SotnEquipment;

/** @brief Resolved frame of the original player-locked weapon entity. */
typedef struct SotnWeaponFrame
{
    Sint16 x, y;
    Uint16 width, height, sprite, sound;
} SotnWeaponFrame;

/** @brief Selected Leather5/Knight6/AxeLord8/Goddess11/Shaman12/Alucart167 ordinary shields.
 * Metadata recognition alone is not a resource/factory/blocking availability gate. */
bool SotnEquipment_IsBasicShield(Uint16 id);
/** @brief Selected Nunchaku21, Red Rust26, Takemitsu27, Mablung124, Morningstar129, Holy rod130
 * and Star flail131 (with their back-forward virtual specials170/171): weapon33/35/36/57/3/4/5
 * programs sharing w_002's ordinary callback. */
bool SotnEquipment_IsPlainWeapon(Uint16 id);
/** @brief Selected two-hand items whose one owned copy occupies both actual hand slots. */
bool SotnEquipment_IsTwoHand(Uint16 id);
/** @brief Selected Knuckle duster85/Jewel knuckles97 weapon1 fists without weapon specials.
 * Iron Fist102 and Fist of Tulkas120 require their special owners and are excluded. */
bool SotnEquipment_IsBasicFist(Uint16 id);
/** @brief Selected ordinary shields6/8/12/16 with actual shared callbacks. */
bool SotnEquipment_IsShield(Uint16 id);

/** @brief Selected weapon0 identities with actual shared callback/stat/palette owners.
 * Namakura requires paired inventory slots; Badelaire requires the actual playtime owner. */
bool SotnEquipment_IsSword0(Uint16 id);

/** @brief Read a supported hand-item definition, without importing runtime tables.
 * @param dra Borrowed immutable DRA bytes. @param id Empty hand (0), Basilard (18), the selected
 * weapon0 swords including Namakura/Badelaire, Short sword (19), Mace (128), Alucard sword (123),
 * Alucard shield (16), Leather5/Knight6/AxeLord8/Goddess11/Shaman12/Alucart167 shields, Knuckle
 * duster85/Jewel knuckles97, Nunchaku21/Red Rust26/Takemitsu27/Mablung124/Morningstar129,
 * medicines144..162, or Library Card (166), Karma Coin24, Magic Missile25, food29..70, Neutron
 * Bomb71, Power of Sire72 and Pentagrams73/74, stars/TNT75..79 and throwing items80..82/134/135.
 * @param out Receives copied metadata, unchanged on failure.
 * @return Success, or false with SDL_GetError() for invalid or unsupported definitions.
 * @note Life Apple146/Hammer147 metadata does not enable equipping or hand use.
 * Other items and their special effects require separate integration. */
bool SotnEquipment_Read(SotnDataView dra, Uint16 id, SotnEquipment *out);
/** @brief Read original weapon21 food29..70 metadata for the actual shared hand owner.
 * Raw food ATK is healing, never strength/poison-scaled weapon damage. */
bool SotnEquipment_ReadFood(SotnDataView dra, Uint16 id, SotnEquipment *out);
/** @brief Selected weapon20/item24, weapon22/item25, weapon17/18/19 items71..74,
 * weapon15 items75..79
 * and weapon16 items80..82/134/135 metadata. */
bool SotnEquipment_ReadThrowing(SotnDataView dra, Uint16 item, SotnEquipment *out);
/** @brief Read private weapon22/item25 metadata for the bound native bow/arrow owner. */
bool SotnEquipment_ReadMagicMissile(SotnDataView dra, SotnEquipment *out);
/** @brief Read original weapon19 Neutron Bomb71 metadata for its actual hand owner. */
bool SotnEquipment_ReadNeutron(SotnDataView dra, SotnEquipment *out);
/** @brief Read an original hand item's MP usage word (definition offset36).
 * @param dra Borrowed DRA bytes. @param id Original hand or virtual item below217.
 * @param out Receives the cost, unchanged on failure. @return Success or SDL error. */
bool SotnEquipment_ReadCost(SotnDataView dra, Uint16 id, Uint16 *out);
/** @brief Resolve the original both-buttons combo (menu.c CheckWeaponCombo, D_8013AEE4).
 * @param out Receives the combo item plus 0x80000000 when the right hand owns it, or zero.
 * @return Success or SDL error for unbounded items or truncated definitions. */
bool SotnEquipment_Combo(SotnDataView dra, Uint16 left, Uint16 right, Uint32 *out);
/** @brief Read virtual197 for an actual equipped Alucard sword123 special.
 * @param dra Borrowed private US definitions. @param equipped Actual owned hand item.
 * @param out Copied metadata. @param mp Actual original special cost.
 * @return Success or SDL error without output mutation. Virtual197 is never equippable. */
bool SotnEquipment_ReadSwordSpecial(SotnDataView dra, Uint16 equipped, SotnEquipment *out,
                                    Uint16 *mp);
/** @brief Read internal virtual216 for an actual owned Axe Lord Armor controller.
 * @param dra Private US DRA. @param out Receives weapon45 metadata, unchanged on failure.
 * @return Success or SDL error. This neither equips/grants a hand item nor enables a form. */
bool SotnEquipment_ReadAxeArmor(SotnDataView dra, SotnEquipment *out);

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
