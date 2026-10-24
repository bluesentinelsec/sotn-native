// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected metadata and player-locked weapon paths from sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5: dra/6E42C.c, alu_anim.c,
// weapon/w_000.c and w_002.c. All definitions, frames, and hitboxes remain private data.
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
    if (out == NULL || (id != 0 && id != 86 && id != 128))
        return SDL_SetError("This weapon checkpoint supports empty hands, Gladius, and Mace");
    const Uint8 *pointer = NULL, *record = NULL;
    if (!Resolve(dra, 0x800a0000, 0x800a00c0, 4, &pointer) ||
        !Resolve(dra, 0x800a0000, (Uint64)Read32(pointer) + (Uint32)id * 52, 50, &record))
        return false;
    if ((id == 0 && (record[17] != 0 || record[18] != 0x24)) ||
        (id == 86 && (record[15] != 0 || record[17] != 1 || record[18] != 0x41)) ||
        (id == 128 && (record[15] != 2 || record[17] != 2 || record[18] != 0x48)) ||
        record[20] != 0 || record[16] != 0 || record[21] > 31 || record[25] != 0 ||
        Read16(record + 36) != 0)
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

bool SotnWeapon_Overlay(SotnDataView weapons, SotnEquipment equipment, SotnDataView *out)
{
    if (out == NULL || (equipment.id != 86 && equipment.id != 128) ||
        equipment.family != (equipment.id == 86 ? 0 : 2))
        return SDL_SetError("Weapon overlay requires a supported equipped item");
    const size_t offset = (size_t)equipment.family * 0x7000 + 0x4000;
    if (weapons.bytes == NULL || offset > weapons.size || 0x3000 > weapons.size - offset)
        return SDL_SetError("WEAPON0.BIN is truncated before the selected weapon overlay");
    *out = (SotnDataView){weapons.bytes + offset, 0x3000};
    return true;
}

bool SotnWeapon_ReadFrame(SotnDataView weapons, SotnEquipment equipment, Uint16 animation,
                          Uint16 pose, Sint32 remaining, SotnWeaponFrame *out)
{
    if (out == NULL || pose >= 32 || remaining < -1 || remaining > 32767)
        return SDL_SetError("Weapon animation requires a bounded player pose and output");
    SotnDataView overlay = {0};
    if (!SotnWeapon_Overlay(weapons, equipment, &overlay))
        return false;
    const Uint8 *descriptor = NULL, *pointer = NULL, *packed = NULL, *box = NULL;
    const Uint32 address = equipment.family == 0 ? 0x8017b06cU : 0x8017abc4U;
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
        if (property >= 25 ||
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
