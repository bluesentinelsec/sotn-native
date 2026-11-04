// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected metadata and player-locked weapon paths from sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5: dra/6E42C.c, alu_anim.c,
// weapon/w_000.c, w_002.c and w_037.c. Definitions, frames and hitboxes remain private.
#include <SDL3/SDL.h>
#include <sotn/equipment.h>

static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}

static Uint32 Read32(const Uint8 *p)
{
    return p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
}

static bool Resolve(SotnDataView image, Uint32 base, Uint64 address, size_t count,
                    const Uint8 **out)
{
    if (image.bytes == NULL || address < base || address > SDL_MAX_UINT32 || (address & 1) ||
        address - base > image.size || count > image.size - (address - base))
        return SDL_SetError("Equipment references an invalid or truncated private data address");
    *out = image.bytes + (size_t)(address - base);
    return true;
}

bool SotnEquipment_IsBasicShield(Uint16 id)
{
    return id == 5 || id == 6 || id == 8 || id == 11 || id == 12 || id == 167;
}
// Actual weapon program for each selected ordinary shield.
static Uint8 ShieldFamily(Uint16 id)
{
    return id == 5     ? 8
           : id == 6   ? 9
           : id == 8   ? 11
           : id == 11  ? 25
           : id == 12  ? 26
           : id == 167 ? 58
                       : 52;
}
// Actual US definition bytes for the selected w_002-template programs.
typedef struct PlainSpec
{
    Uint8 category, family, kind, animation;
} PlainSpec;
static PlainSpec PlainWeapon(Uint16 id)
{
    switch (id)
    {
    case 21: // Nunchaku
        return (PlainSpec){5, 33, 11, 0x8b};
    case 26: // Red Rust
        return (PlainSpec){5, 35, 13, 0x99};
    case 27: // Takemitsu
        return (PlainSpec){5, 36, 14, 0xa0};
    case 124: // Mablung
        return (PlainSpec){1, 57, 1, 0x41};
    case 129: // Morningstar
        return (PlainSpec){4, 3, 2, 0x48};
    default:
        return (PlainSpec){0};
    }
}
bool SotnEquipment_IsPlainWeapon(Uint16 id)
{
    return PlainWeapon(id).family != 0;
}
bool SotnEquipment_IsTwoHand(Uint16 id)
{
    return id == 21 || id == 26 || id == 27 || id == 84;
}
bool SotnEquipment_IsBasicFist(Uint16 id)
{
    return id == 85 || id == 97;
}
bool SotnEquipment_IsShield(Uint16 id)
{
    return id == 16 || SotnEquipment_IsBasicShield(id);
}
bool SotnEquipment_IsSword0(Uint16 id)
{
    return id == 83 || id == 84 || (id >= 86 && id <= 94) || id == 96 || id == 99 ||
           (id >= 104 && id <= 106) || id == 108 || id == 110 || id == 116 || id == 118 ||
           id == 122 || id == 125;
}

bool SotnEquipment_Read(SotnDataView dra, Uint16 id, SotnEquipment *out)
{
    if (id == 24 || id == 25 || (id >= 71 && id <= 79) || id == 80 || id == 81 || id == 82 ||
        id == 134 || id == 135)
        return SotnEquipment_ReadThrowing(dra, id, out);
    if (id >= 29 && id <= 70)
        return SotnEquipment_ReadFood(dra, id, out);
    if (out == NULL || (id != 0 && !SotnEquipment_IsShield(id) && id != 18 && id != 19 &&
                        !SotnEquipment_IsSword0(id) && !SotnEquipment_IsBasicFist(id) &&
                        !SotnEquipment_IsPlainWeapon(id) && id != 123 && id != 128 && id != 166 &&
                        (id < 144 || id > 162)))
        return SDL_SetError("This weapon checkpoint requires an integrated hand item");
    const bool sword0 = SotnEquipment_IsSword0(id);
    const bool basic_shield = SotnEquipment_IsBasicShield(id);
    const Uint8 shield_family = ShieldFamily(id);
    const bool fist = SotnEquipment_IsBasicFist(id);
    const bool plain = SotnEquipment_IsPlainWeapon(id);
    const PlainSpec spec = PlainWeapon(id);
    const Uint16 sword_variant = sword0      ? (id == 125   ? 3
                                                : id >= 116 ? 2
                                                : id >= 104 ? 1
                                                            : 0)
                                 : fist      ? (id == 97 ? 1 : 0)
                                 : id == 124 ? 3
                                 : id == 167 ? 1
                                             : 0;
    const Uint8 palette = id == 116 ? 2 : id == 122 ? 1 : 0;
    const bool medicine = id == 166 || (id >= 144 && id <= 162);
    const Uint16 variant = id == 166 ? 20 : id >= 157 ? id - 143 : id >= 144 ? id - 144 : 0;
    const Uint8 *pointer = NULL, *record = NULL;
    if (!Resolve(dra, 0x800a0000, 0x800a00c0, 4, &pointer) ||
        !Resolve(dra, 0x800a0000, (Uint64)Read32(pointer) + (Uint32)id * 52, 50, &record))
        return false;
    if ((id == 0 && (record[17] != 0 || record[18] != 0x24)) ||
        (id == 18 && (record[15] != 31 || record[17] != 7 || record[18] != 0x6f)) ||
        (id == 19 && (record[15] != 37 || record[17] != 15 || record[18] != 0xa7)) ||
        (sword0 && (record[15] != 0 || record[17] != 1 || record[18] != 0x41)) ||
        (id == 123 && (record[15] != 43 || record[17] != 1 || record[18] != 0x41)) ||
        (fist && (record[14] != 3 || record[15] != 1 || record[17] != 0 || record[18] != 0x24 ||
                  record[19] != 48 || record[22] != 128 || record[23] || record[24])) ||
        (plain && (record[14] != spec.category || record[15] != spec.family ||
                   record[17] != spec.kind || record[18] != spec.animation || record[19] != 48 ||
                   record[22] != 128 || record[23] || record[24])) ||
        (basic_shield && (record[14] != 9 || record[15] != shield_family || record[17] != 128 ||
                          record[18] != 0x63 || record[19] != 48 || record[21] ||
                          record[22] != 128 || record[23] || record[24])) ||
        (id == 16 && (record[15] != 52 || record[17] != 128 || record[18] != 0x63)) ||
        (id == 128 && (record[15] != 2 || record[17] != 2 || record[18] != 0x48)) ||
        (medicine &&
         (record[15] != 255 || record[17] != 131 || record[18] != 0xbe || record[19] != 55)) ||
        (sword0 && id != 86 &&
         (record[14] != (id == 84 ? 5 : 1) || record[19] != 48 ||
          record[21] != (id == 83 || id == 84 ? 18 : 13) || record[22] != 128 || record[23] ||
          record[24])) ||
        record[20] != (medicine ? variant : sword_variant) || record[16] != palette ||
        record[21] > 31 || record[25] != (medicine ? 1 : 0) || Read16(record + 36) != 0)
        return SDL_SetError("Selected hand-item definition does not match its supported US path");
    const Uint16 attack = Read16(record + 8);
    const SotnEquipment result = {.id = id,
                                  .element = Read16(record + 12),
                                  .stun = Read16(record + 38),
                                  .hit_type = Read16(record + 40),
                                  .hit_effect = Read16(record + 42),
                                  .critical_rate = Read16(record + 48),
                                  .attack =
                                      (Sint16)(attack < 32768 ? attack : (Sint32)attack - 65536),
                                  .family = record[15],
                                  .palette = record[16],
                                  .kind = record[17],
                                  .animation = record[18],
                                  .variant = record[20],
                                  .blueprint = record[19],
                                  .chain_limit = record[22],
                                  .lock = record[21],
                                  .invincibility = record[26]};
    *out = result;
    return true;
}

bool SotnEquipment_ReadFood(SotnDataView dra, Uint16 id, SotnEquipment *out)
{
    const Uint8 *pointer = NULL, *p = NULL;
    if (!out || id < 29 || id > 70)
        return SDL_SetError("Food metadata requires an original weapon21 hand item29..70");
    if (!Resolve(dra, 0x800a0000, 0x800a00c0, 4, &pointer) ||
        !Resolve(dra, 0x800a0000, (Uint64)Read32(pointer) + (Uint32)id * 52, 50, &p))
        return false;
    if (p[14] != 6 || p[15] != 21 || p[16] || p[17] != 132 || p[18] != 0x60 || p[19] != 50 ||
        p[20] != id - 29U || p[21] || p[22] != 2 || p[23] || p[24] || p[25] != 1 ||
        Read16(p + 36) || Read16(p + 8) > 9999)
        return SDL_SetError("Food definition does not match its actual private weapon21 path");
    *out = (SotnEquipment){.id = id,
                           .family = 21,
                           .kind = 132,
                           .animation = p[18],
                           .variant = p[20],
                           .attack = (Sint16)Read16(p + 8),
                           .element = Read16(p + 12),
                           .stun = Read16(p + 38),
                           .hit_type = Read16(p + 40),
                           .hit_effect = Read16(p + 42),
                           .critical_rate = Read16(p + 48),
                           .invincibility = p[26]};
    return true;
}

static bool ThrowingDefinition(SotnDataView dra, Uint16 item, SotnEquipment *out)
{
    const bool karma = item == 24;
    const bool magic = item == 25;
    const bool star = item >= 75 && item <= 79;
    const bool thrown = item == 80 || item == 81 || item == 82 || item == 134 || item == 135;
    const Uint8 *pointer = NULL, *p = NULL;
    if (!out || (!karma && !magic && !thrown && !star && (item < 71 || item > 74)))
        return SDL_SetError("Throwing definition requires an actual supported original item");
    if (!Resolve(dra, 0x800a0000, 0x800a00c0, 4, &pointer) ||
        !Resolve(dra, 0x800a0000, (Uint64)Read32(pointer) + (Uint32)item * 52U, 50, &p))
        return false;
    const Uint8 blueprint = item == 79 || item == 82    ? 68
                            : item == 81 || item == 134 ? 62
                            : item == 82                ? 68
                            : item == 135               ? 63
                                                        : 50;
    const Uint8 chain = star     ? item == 79 ? 1 : 3
                        : thrown ? item == 82    ? 2
                                   : item == 135 ? 4
                                                 : 3
                        : magic  ? 4
                                 : 1;
    if (p[14] != (magic || thrown || star ? 8 : 7) ||
        p[15] != (karma        ? 20
                  : star       ? 15
                  : thrown     ? 16
                  : magic      ? 22
                  : item == 71 ? 19
                  : item == 72 ? 18
                               : 17) ||
        p[16] || p[17] != ((thrown && item != 80) || item == 76 || item == 78 ? 129 : 130) ||
        p[18] != (thrown || item == 76 || item == 78 ? 0x5d : 0x60) || p[19] != blueprint ||
        p[20] != (star && item != 79          ? item - 75U
                  : item == 74 || item == 134 ? 1
                                              : 0) ||
        p[21] || p[22] != chain || p[23] || p[24] || p[25] != 1 || Read16(p + 36) ||
        Read16(p + 8) > 999 || Read16(p + 40) != (karma || magic || thrown || star ? 2 : 0x82))
        return SDL_SetError("Throwing definition does not match its original owned weapon path");
    *out = (SotnEquipment){.id = item,
                           .family = p[15],
                           .kind = p[17],
                           .blueprint = p[19],
                           .animation = p[18],
                           .variant = p[20],
                           .attack = (Sint16)Read16(p + 8),
                           .element = Read16(p + 12),
                           .stun = Read16(p + 38),
                           .hit_type = Read16(p + 40),
                           .hit_effect = Read16(p + 42),
                           .critical_rate = Read16(p + 48),
                           .invincibility = p[26],
                           .chain_limit = p[22]};
    return true;
}

bool SotnEquipment_ReadThrowing(SotnDataView dra, Uint16 item, SotnEquipment *out)
{
    return ThrowingDefinition(dra, item, out);
}

bool SotnEquipment_ReadMagicMissile(SotnDataView dra, SotnEquipment *out)
{
    return ThrowingDefinition(dra, 25, out);
}

bool SotnEquipment_ReadNeutron(SotnDataView dra, SotnEquipment *out)
{
    return SotnEquipment_ReadThrowing(dra, 71, out);
}

bool SotnEquipment_ReadAxeArmor(SotnDataView dra, SotnEquipment *out)
{
    const Uint8 *pointer = NULL, *record = NULL;
    if (!out)
        return SDL_InvalidParamError("out");
    if (!Resolve(dra, 0x800a0000, 0x800a00c0, 4, &pointer) ||
        !Resolve(dra, 0x800a0000, (Uint64)Read32(pointer) + 216U * 52U, 50, &record))
        return false;
    if (record[15] != 45 || record[16] || record[17] != 131 || record[18] != 0xbe ||
        record[19] != 55 || record[20] != 14 || Read16(record + 36))
        return SDL_SetError("Axe Armor requires its actual virtual216 weapon45 definition");
    *out = (SotnEquipment){.id = 216, .family = 45, .kind = 131, .animation = record[18]};
    return true;
}

bool SotnWeapon_Overlay(SotnDataView weapons, SotnEquipment equipment, SotnDataView *out)
{
    if (out == NULL ||
        (!SotnEquipment_IsShield(equipment.id) && equipment.id != 18 && equipment.id != 19 &&
         equipment.id != 24 && equipment.id != 25 && equipment.id != 80 && equipment.id != 81 &&
         equipment.id != 82 && equipment.id != 134 && equipment.id != 135 &&
         !SotnEquipment_IsSword0(equipment.id) && !SotnEquipment_IsBasicFist(equipment.id) &&
         !SotnEquipment_IsPlainWeapon(equipment.id) && equipment.id != 123 && equipment.id != 197 &&
         equipment.id != 216 && equipment.id != 128 && (equipment.id < 29 || equipment.id > 79)) ||
        equipment.family != (equipment.id == 24                         ? 20
                             : equipment.id >= 75 && equipment.id <= 79 ? 15
                             : equipment.id == 80 || equipment.id == 81 || equipment.id == 82 ||
                                     equipment.id == 134 || equipment.id == 135
                                 ? 16
                             : equipment.id == 25                      ? 22
                             : SotnEquipment_IsSword0(equipment.id)    ? 0
                             : SotnEquipment_IsBasicFist(equipment.id) ? 1
                             : SotnEquipment_IsPlainWeapon(equipment.id)
                                 ? PlainWeapon(equipment.id).family
                             : equipment.id == 18                         ? 31
                             : equipment.id == 19                         ? 37
                             : equipment.id == 123 || equipment.id == 197 ? 43
                             : SotnEquipment_IsShield(equipment.id)     ? ShieldFamily(equipment.id)
                             : equipment.id == 216                      ? 45
                             : equipment.id >= 29 && equipment.id <= 70 ? 21
                             : equipment.id == 71                       ? 19
                             : equipment.id == 72                       ? 18
                             : equipment.id == 73 || equipment.id == 74 ? 17
                                                                        : 2))
        return SDL_SetError("Weapon overlay requires a supported equipped item");
    const size_t offset = (size_t)equipment.family * 0x7000 + 0x4000;
    if (weapons.bytes == NULL || offset > weapons.size || 0x3000 > weapons.size - offset)
        return SDL_SetError("WEAPON0.BIN is truncated before the selected weapon overlay");
    *out = (SotnDataView){weapons.bytes + offset, 0x3000};
    return true;
}

bool SotnEquipment_ReadSwordSpecial(SotnDataView dra, Uint16 equipped, SotnEquipment *out,
                                    Uint16 *mp)
{
    if (equipped != 123 || !out || !mp)
        return SDL_SetError("Sword special requires its actual equipped Alucard sword");
    SotnEquipment normal = {0};
    const Uint8 *pointer = NULL, *record = NULL, *base = NULL;
    if (!SotnEquipment_Read(dra, equipped, &normal) ||
        !Resolve(dra, 0x800a0000, 0x800a00c0, 4, &pointer) ||
        !Resolve(dra, 0x800a0000, (Uint64)Read32(pointer) + 123U * 52U, 50, &base) ||
        !Resolve(dra, 0x800a0000, (Uint64)Read32(pointer) + 197U * 52U, 50, &record))
        return false;
    if (base[24] != 197 || record[15] != 43 || record[16] || record[17] != 135 ||
        record[18] != 0xd9 || record[19] != 54 || record[20] || record[22] != 128 || record[25] ||
        Read16(record + 36) > 999)
        return SDL_SetError("Sword special definition does not match its original owned path");
    const Uint16 attack = Read16(record + 8);
    *out = (SotnEquipment){.id = 197,
                           .family = 43,
                           .kind = 135,
                           .animation = record[18],
                           .attack = (Sint16)(attack < 32768 ? attack : (Sint32)attack - 65536),
                           .element = Read16(record + 12),
                           .stun = Read16(record + 38),
                           .hit_type = Read16(record + 40),
                           .hit_effect = Read16(record + 42),
                           .critical_rate = Read16(record + 48),
                           .invincibility = record[26]};
    *mp = Read16(record + 36);
    return true;
}

bool SotnWeapon_ReadFrame(SotnDataView weapons, SotnEquipment equipment, Uint16 animation,
                          Uint16 pose, Sint32 remaining, SotnWeaponFrame *out)
{
    if (equipment.id == 16)
        return SDL_SetError("Shield frames require their independent animation cursor");
    if (out == NULL || pose >= 32 || remaining < -1 || remaining > 32767)
        return SDL_SetError("Weapon animation requires a bounded player pose and output");
    SotnDataView overlay = {0};
    if (!SotnWeapon_Overlay(weapons, equipment, &overlay))
        return false;
    const Uint8 *descriptor = NULL, *pointer = NULL, *packed = NULL, *box = NULL;
    const Uint32 address = equipment.family == 43   ? 0x8017b014U
                           : equipment.family == 37 ? 0x8017a6a4U
                           : equipment.family == 0  ? 0x8017b06cU
                           : equipment.family == 31 ? 0x8017a8ecU
                                                    : 0x8017abc4U;
    const Uint16 variants = equipment.family == 0    ? 5
                            : equipment.family == 43 ? 3
                            : equipment.family == 37 ? 4
                            : equipment.family == 31 ? 2
                                                     : 1;
    if (equipment.variant >= variants ||
        !Resolve(overlay, 0x8017a000, address + equipment.variant * 16U, 16, &descriptor))
        return false;
    SotnWeaponFrame result = {0};
    if (animation < descriptor[12] || animation >= descriptor[12] + 7 || remaining < 0)
    {
        *out = result;
        return true;
    }
    if (!Resolve(overlay, 0x8017a000,
                 (Uint64)Read32(descriptor) + (animation - descriptor[12]) * 4U, 4, &pointer) ||
        !Resolve(overlay, 0x8017a000, (Uint64)Read32(pointer) + pose * 2U, 2, &packed))
        return false;
    const Uint16 value = Read16(packed);
    if (value != 0xffff)
    {
        const Uint32 property = (value >> 9) & 127;
        if (property >= (equipment.family == 37 ? 15U : 25U) ||
            !Resolve(overlay, 0x8017a000, (Uint64)Read32(descriptor + 4) + property * 4, 4, &box))
            return SDL_SetError("Weapon hitbox exceeds the supported frame properties");
        result.x = (Sint16)(box[0] < 128 ? box[0] : (Sint32)box[0] - 256);
        result.y = (Sint16)(box[1] < 128 ? box[1] : (Sint32)box[1] - 256);
        result.width = box[2];
        result.height = box[3];
        result.sprite = value & 511;
        if (remaining == 1 && pose == descriptor[13])
            result.sound = Read16(descriptor + 10);
    }
    *out = result;
    return true;
}
