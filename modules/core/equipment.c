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

bool SotnEquipment_Read(SotnDataView dra, Uint16 id, SotnEquipment *out)
{
    if (id == 25 || (id >= 71 && id <= 74) || id == 80 || id == 81 || id == 82 || id == 134 ||
        id == 135)
        return SotnEquipment_ReadThrowing(dra, id, out);
    if (id >= 29 && id <= 70)
        return SotnEquipment_ReadFood(dra, id, out);
    if (out == NULL || (id != 0 && id != 16 && id != 18 && id != 19 && id != 86 && id != 123 &&
                        id != 128 && id != 166 && (id < 144 || id > 162)))
        return SDL_SetError("This weapon checkpoint requires an integrated hand item");
    const bool medicine = id == 166 || (id >= 144 && id <= 162);
    const Uint16 variant = id == 166 ? 20 : id >= 157 ? id - 143 : id >= 144 ? id - 144 : 0;
    const Uint8 *pointer = NULL, *record = NULL;
    if (!Resolve(dra, 0x800a0000, 0x800a00c0, 4, &pointer) ||
        !Resolve(dra, 0x800a0000, (Uint64)Read32(pointer) + (Uint32)id * 52, 50, &record))
        return false;
    if ((id == 0 && (record[17] != 0 || record[18] != 0x24)) ||
        (id == 18 && (record[15] != 31 || record[17] != 7 || record[18] != 0x6f)) ||
        (id == 19 && (record[15] != 37 || record[17] != 15 || record[18] != 0xa7)) ||
        (id == 86 && (record[15] != 0 || record[17] != 1 || record[18] != 0x41)) ||
        (id == 123 && (record[15] != 43 || record[17] != 1 || record[18] != 0x41)) ||
        (id == 16 && (record[15] != 52 || record[17] != 128 || record[18] != 0x63)) ||
        (id == 128 && (record[15] != 2 || record[17] != 2 || record[18] != 0x48)) ||
        (medicine &&
         (record[15] != 255 || record[17] != 131 || record[18] != 0xbe || record[19] != 55)) ||
        record[20] != (medicine ? variant : 0) || record[16] != 0 || record[21] > 31 ||
        record[25] != (medicine ? 1 : 0) || Read16(record + 36) != 0)
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
    const bool magic = item == 25;
    const bool thrown = item == 80 || item == 81 || item == 82 || item == 134 || item == 135;
    const Uint8 *pointer = NULL, *p = NULL;
    if (!out || (!magic && !thrown && (item < 71 || item > 74)))
        return SDL_SetError("Throwing definition requires an actual supported original item");
    if (!Resolve(dra, 0x800a0000, 0x800a00c0, 4, &pointer) ||
        !Resolve(dra, 0x800a0000, (Uint64)Read32(pointer) + (Uint32)item * 52U, 50, &p))
        return false;
    const Uint8 blueprint = item == 81 || item == 134 ? 62
                            : item == 82              ? 68
                            : item == 135             ? 63
                                                      : 50;
    const Uint8 chain = thrown ? item == 82 ? 2 : item == 135 ? 4 : 3 : magic ? 4 : 1;
    if (p[14] != (magic || thrown ? 8 : 7) ||
        p[15] != (thrown       ? 16
                  : magic      ? 22
                  : item == 71 ? 19
                  : item == 72 ? 18
                               : 17) ||
        p[16] || p[17] != (thrown && item != 80 ? 129 : 130) || p[18] != (thrown ? 0x5d : 0x60) ||
        p[19] != blueprint || p[20] != (item == 74 || item == 134 ? 1 : 0) || p[21] ||
        p[22] != chain || p[23] || p[24] || p[25] != 1 || Read16(p + 36) || Read16(p + 8) > 999 ||
        Read16(p + 40) != (magic || thrown ? 2 : 0x82))
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
        (equipment.id != 16 && equipment.id != 18 && equipment.id != 19 && equipment.id != 25 &&
         equipment.id != 80 && equipment.id != 81 && equipment.id != 82 && equipment.id != 134 &&
         equipment.id != 135 && equipment.id != 86 && equipment.id != 123 && equipment.id != 197 &&
         equipment.id != 216 && equipment.id != 128 && (equipment.id < 29 || equipment.id > 74)) ||
        equipment.family != (equipment.id == 80 || equipment.id == 81 || equipment.id == 82 ||
                                     equipment.id == 134 || equipment.id == 135
                                 ? 16
                             : equipment.id == 25                         ? 22
                             : equipment.id == 86                         ? 0
                             : equipment.id == 18                         ? 31
                             : equipment.id == 19                         ? 37
                             : equipment.id == 123 || equipment.id == 197 ? 43
                             : equipment.id == 16                         ? 52
                             : equipment.id == 216                        ? 45
                             : equipment.id >= 29 && equipment.id <= 70   ? 21
                             : equipment.id == 71                         ? 19
                             : equipment.id == 72                         ? 18
                             : equipment.id == 73 || equipment.id == 74   ? 17
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
    if (!Resolve(overlay, 0x8017a000, address, 16, &descriptor))
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
