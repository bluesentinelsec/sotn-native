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
    return id == 5 || id == 6 || id == 7 || id == 8 || id == 9 || id == 10 || id == 11 ||
           id == 12 || id == 13 || id == 14 || id == 167;
}
// Actual weapon program for each selected ordinary shield.
static Uint8 ShieldFamily(Uint16 id)
{
    return id == 5     ? 8
           : id == 6   ? 9
           : id == 7   ? 10
           : id == 8   ? 11
           : id == 9   ? 23
           : id == 10  ? 24
           : id == 13  ? 27
           : id == 14  ? 28
           : id == 11  ? 25
           : id == 12  ? 26
           : id == 167 ? 58
                       : 52;
}
// Actual US definition bytes for the selected w_002-template programs.
typedef struct PlainSpec
{
    Uint8 category, family, kind, animation, variant, back_forward, special_move, mp;
    Uint8 blueprint; /**< Zero selects the ordinary blueprint48. */
    Uint8 chain;     /**< Zero selects the ordinary unlimited chain 128. */
} PlainSpec;
static PlainSpec PlainWeapon(Uint16 id)
{
    switch (id)
    {
    case 21: // Nunchaku
        return (PlainSpec){5, 33, 11, 0x8b, 0, 0, 0, 0, 0, 0};
    case 26: // Red Rust
        return (PlainSpec){5, 35, 13, 0x99, 0, 0, 0, 0, 0, 0};
    case 141: // Muramasa (kind29: blood-grown attack, low-attack failure roll)
        return (PlainSpec){5, 56, 29, 0x41, 0, 0, 0, 0, 0, 0};
    case 27: // Takemitsu
        return (PlainSpec){5, 36, 14, 0xa0, 0, 0, 0, 0, 0, 0};
    case 124: // Mablung
        return (PlainSpec){1, 57, 1, 0x41, 3, 0, 0, 0, 0, 0};
    case 129: // Morningstar
        return (PlainSpec){4, 3, 2, 0x48, 0, 0, 0, 0, 0, 0};
    case 130: // Holy rod and its back-forward special170
        return (PlainSpec){4, 4, 3, 0x4f, 0, 170, 0, 0, 0, 0};
    case 170:
        return (PlainSpec){4, 4, 4, 0x56, 1, 0, 0, 0, 0, 0};
    case 121: // Gurthang and its QCF special195
        return (PlainSpec){1, 54, 1, 0x41, 0, 0, 195, 0, 0, 0};
    case 195:
        return (PlainSpec){1, 54, 1, 0x41, 1, 0, 0, 0, 0, 0};
    case 4: // Shield rod: back-forward special173 releases its orb (kind2, weapon7)
        return (PlainSpec){4, 7, 2, 0x48, 0, 173, 0, 0, 0, 0};
    case 173:
        return (PlainSpec){4, 7, 2, 0x48, 1, 0, 0, 0, 0, 0};
    case 202: // Heaven sword pair's both-buttons combo (kind5, blueprint62 sweep, five MP)
        return (PlainSpec){2, 13, 5, 0x66, 0, 0, 0, 5, 62, 1};
    case 203: // Leather shield's both-buttons spell (Shield rod/Mablung combo, kind5)
        return (PlainSpec){10, 8, 5, 0xba, 0, 0, 0, 10, 86, 1};
    case 204: // Knight shield's both-buttons spell
        return (PlainSpec){10, 9, 5, 0xba, 0, 0, 0, 10, 86, 1};
    case 205: // Iron shield's both-buttons spell (twenty MP)
        return (PlainSpec){1, 10, 5, 0xba, 0, 0, 0, 20, 86, 1};
    case 207: // Herald shield's both-buttons spell (ten MP)
        return (PlainSpec){10, 23, 5, 0xba, 0, 0, 0, 10, 86, 1};
    case 208: // Dark shield's both-buttons spell (thirty MP)
        return (PlainSpec){7, 24, 5, 0xba, 0, 0, 0, 30, 86, 1};
    case 211: // Medusa shield's both-buttons spell (thirty MP)
        return (PlainSpec){1, 27, 5, 0xba, 0, 0, 0, 30, 86, 1};
    case 212: // Skull shield's both-buttons spell (forty MP)
        return (PlainSpec){1, 28, 5, 0xba, 0, 0, 0, 40, 86, 1};
    case 206: // Axe Lord shield's both-buttons spell (twenty MP)
        return (PlainSpec){1, 11, 5, 0xba, 0, 0, 0, 20, 86, 1};
    case 209: // Goddess shield's both-buttons spell (forty MP)
        return (PlainSpec){10, 25, 5, 0xba, 0, 0, 0, 40, 86, 1};
    case 210: // Shaman shield's both-buttons spell
        return (PlainSpec){10, 26, 5, 0xba, 0, 0, 0, 10, 86, 1};
    case 214: // Alucard shield's both-buttons spell (twenty MP)
        return (PlainSpec){10, 52, 5, 0xba, 0, 0, 0, 20, 86, 1};
    case 215: // Alucart shield's both-buttons spell
        return (PlainSpec){10, 58, 5, 0xba, 0, 0, 0, 10, 86, 1};
    case 17: // Sword of Dawn (two-handed): QCF special177 summons its soldiers (kind5, bp56)
        return (PlainSpec){5, 30, 6, 0x68, 0, 0, 177, 0, 0, 0};
    case 177:
        return (PlainSpec){1, 30, 5, 0xb8, 0, 0, 0, 5, 56, 8};
    case 28: // Shotel: QCF special176 throws it (kind129, blueprint56, chain1, never consumed)
        return (PlainSpec){0, 34, 12, 0x92, 0, 0, 176, 0, 0, 0};
    case 176:
        return (PlainSpec){1, 34, 129, 0x5d, 0, 0, 0, 0, 56, 1};
    case 20: // Combat knife: special175 is both its back-forward and QCF item
        return (PlainSpec){0, 32, 9, 0x7d, 0, 175, 175, 0, 0, 0};
    case 175:
        return (PlainSpec){0, 32, 10, 0x84, 1, 0, 0, 0, 0, 0};
    case 115: // Holy sword and its QCF special196 (kind28, five MP)
        return (PlainSpec){1, 55, 1, 0x41, 0, 0, 196, 0, 0, 0};
    case 196:
        return (PlainSpec){1, 55, 28, 0x41, 1, 0, 0, 5, 0, 0};
    case 112: // Thunderbrand and its QCF special178 (kind19, five MP)
        return (PlainSpec){1, 48, 1, 0x41, 0, 0, 178, 0, 0, 0};
    case 178:
        return (PlainSpec){1, 48, 19, 0x41, 1, 0, 0, 5, 0, 0};
    case 132: // Moon rod and its QCF special172 (kind5, blueprint72, chain1, five MP)
        return (PlainSpec){4, 6, 2, 0x48, 0, 0, 172, 0, 0, 0};
    case 172:
        return (PlainSpec){4, 6, 5, 0x66, 0, 0, 0, 5, 72, 1};
    case 113: // Icebrand and its QCF special181 (kind22, five MP)
        return (PlainSpec){1, 50, 1, 0x41, 0, 0, 181, 0, 0, 0};
    case 181:
        return (PlainSpec){1, 50, 22, 0x41, 1, 0, 0, 5, 0, 0};
    case 111: // Firebrand and its QCF special179 (kind20, five MP)
        return (PlainSpec){1, 49, 1, 0x41, 0, 0, 179, 0, 0, 0};
    case 179:
        return (PlainSpec){1, 49, 20, 0x41, 1, 0, 0, 5, 0, 0};
    case 117: // Marsil and its QCF special180 (kind21, five MP)
        return (PlainSpec){1, 49, 1, 0x41, 2, 0, 180, 0, 0, 0};
    case 180:
        return (PlainSpec){1, 49, 21, 0x41, 3, 0, 0, 5, 0, 0};
    case 139: // Osafune katana: back-forward slash192 and QCF warp193 (kind135, blueprint60)
        return (PlainSpec){5, 53, 1, 0x41, 2, 192, 193, 0, 0, 0};
    case 192:
        return (PlainSpec){5, 53, 1, 0x41, 3, 0, 0, 0, 0, 0};
    case 193:
        return (PlainSpec){5, 53, 135, 0xd8, 0, 0, 0, 0, 60, 0};
    case 140: // Masamune: its ordinary swing already slashes; QCF warp194
        return (PlainSpec){5, 53, 1, 0x41, 4, 0, 194, 0, 0, 0};
    case 194:
        return (PlainSpec){5, 53, 135, 0xd8, 1, 0, 0, 0, 60, 0};
    case 100: // Katana and its QCF slash special189
        return (PlainSpec){5, 53, 1, 0x41, 0, 0, 189, 0, 0, 0};
    case 189:
        return (PlainSpec){5, 53, 1, 0x41, 1, 0, 0, 0, 0, 0};
    case 23: // Rapier and its QCF lunge special191
        return (PlainSpec){0, 38, 16, 0xae, 0, 0, 191, 0, 0, 0};
    case 191:
        return (PlainSpec){0, 38, 26, 0xae, 1, 0, 0, 0, 0, 0};
    case 22: // Were bane and its QCF lunge special190
        return (PlainSpec){0, 39, 16, 0xae, 0, 0, 190, 0, 0, 0};
    case 190:
        return (PlainSpec){0, 39, 26, 0xae, 1, 0, 0, 0, 0, 0};
    case 102: // Iron Fist and its QCF punch rush186 (kind24, blueprint54 slot16)
        return (PlainSpec){3, 1, 0, 0x24, 2, 0, 186, 0, 0, 0};
    case 186:
        return (PlainSpec){3, 1, 24, 0x24, 0, 0, 0, 0, 54, 0};
    case 120: // Fist of Tulkas: back-forward rush187 and QCF fireball188 (kind25, descriptor4)
        return (PlainSpec){3, 1, 0, 0x24, 3, 187, 188, 0, 0, 0};
    case 187:
        return (PlainSpec){3, 1, 24, 0x24, 1, 0, 0, 0, 54, 0};
    case 188:
        return (PlainSpec){2, 1, 25, 0x24, 4, 0, 0, 0, 0, 0};
    case 127: // Great Sword
        return (PlainSpec){5, 51, 6, 0x68, 0, 0, 0, 0, 0, 0};
    case 95: // Estoc: kind27 lunge reading w_051's variant8 descriptor past its table
        return (PlainSpec){5, 51, 27, 0xd6, 8, 0, 0, 0, 0, 0};
    case 98: // Claymore/Flamberge/Zwei hander/Obsidian sword: back-forward specials182..185
        return (PlainSpec){5, 51, 6, 0x68, 0, 182, 0, 0, 0, 0};
    case 101:
        return (PlainSpec){5, 51, 6, 0x68, 1, 183, 0, 0, 0, 0};
    case 103:
        return (PlainSpec){5, 51, 6, 0x68, 2, 184, 0, 0, 0, 0};
    case 107:
        return (PlainSpec){5, 51, 6, 0x68, 3, 185, 0, 0, 0, 0};
    case 182: // Kind23 slot16 specials (blueprint54) costing five MP
    case 183:
    case 184:
    case 185:
        return (PlainSpec){1, 51, 23, 0xd6, (Uint8)(id - 182), 0, 0, 5, 54, 0};
    case 131: // Star flail and its back-forward special171
        return (PlainSpec){4, 5, 3, 0x4f, 0, 171, 0, 0, 0, 0};
    case 171:
        return (PlainSpec){4, 5, 4, 0x56, 1, 0, 0, 0, 0, 0};
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
    return id == 21 || id == 26 || id == 27 || id == 84 || id == 127 || id == 100 || id == 139 ||
           id == 140 || id == 95 || id == 98 || id == 101 || id == 103 || id == 107 || id == 141 ||
           id == 165 || id == 17;
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
        id == 134 || id == 135 || id == 143 || id == 119 || id == 133 || id == 163 || id == 164 ||
        id == 165)
        return SotnEquipment_ReadThrowing(dra, id, out);
    if (id >= 29 && id <= 70)
        return SotnEquipment_ReadFood(dra, id, out);
    if (out == NULL || (id != 0 && !SotnEquipment_IsShield(id) && id != 18 && id != 19 &&
                        id != 174 && !SotnEquipment_IsSword0(id) &&
                        !SotnEquipment_IsBasicFist(id) && !SotnEquipment_IsPlainWeapon(id) &&
                        id != 123 && id != 128 && id != 166 && (id < 144 || id > 162)))
        return SDL_SetError("This weapon checkpoint requires an integrated hand item");
    const bool sword0 = SotnEquipment_IsSword0(id);
    const bool basic_shield = SotnEquipment_IsBasicShield(id);
    const Uint8 shield_family = ShieldFamily(id);
    const bool fist = SotnEquipment_IsBasicFist(id);
    const bool plain = SotnEquipment_IsPlainWeapon(id);
    const PlainSpec spec = PlainWeapon(id);
    const Uint16 sword_variant = sword0                   ? (id == 125   ? 3
                                                             : id >= 116 ? 2
                                                             : id >= 104 ? 1
                                                                         : 0)
                                 : fist                   ? (id == 97 ? 1 : 0)
                                 : plain                  ? spec.variant
                                 : id == 167 || id == 174 ? 1
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
        (id == 174 && (record[15] != 31 || record[17] != 8 || record[18] != 0x76 ||
                       record[19] != 48 || record[22] != 128 || record[23] || record[24])) ||
        (id == 19 && (record[15] != 37 || record[17] != 15 || record[18] != 0xa7)) ||
        (sword0 && (record[15] != 0 || record[17] != 1 || record[18] != 0x41)) ||
        (id == 123 && (record[15] != 43 || record[17] != 1 || record[18] != 0x41)) ||
        (fist && (record[14] != 3 || record[15] != 1 || record[17] != 0 || record[18] != 0x24 ||
                  record[19] != 48 || record[22] != 128 || record[23] || record[24])) ||
        (plain &&
         (record[14] != spec.category || record[15] != spec.family || record[17] != spec.kind ||
          record[18] != spec.animation || record[19] != (spec.blueprint ? spec.blueprint : 48) ||
          record[22] != (spec.chain ? spec.chain : 128) || record[23] != spec.back_forward ||
          record[24] != spec.special_move)) ||
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
        record[21] > 31 || record[25] != (medicine ? 1 : 0) ||
        Read16(record + 36) != (plain ? spec.mp : 0))
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
                                  .back_forward = record[23],
                                  .special_move = record[24],
                                  .lock = record[21],
                                  .invincibility = record[26]};
    *out = result;
    return true;
}

bool SotnEquipment_Combo(SotnDataView dra, Uint16 left, Uint16 right, Uint32 *out)
{
    const Uint8 *pointer = NULL, *l = NULL, *r = NULL;
    if (!out || left >= 217 || right >= 217)
        return SDL_SetError("Weapon combo requires bounded original hand items");
    if (!Resolve(dra, 0x800a0000, 0x800a00c0, 4, &pointer) ||
        !Resolve(dra, 0x800a0000, (Uint64)Read32(pointer) + (Uint32)left * 52, 50, &l) ||
        !Resolve(dra, 0x800a0000, (Uint64)Read32(pointer) + (Uint32)right * 52, 50, &r))
        return false;
    // menu.c CheckWeaponCombo: the right hand's main against the left hand's sub selects the
    // left hand (hand flag clear); otherwise the reverse pairing marks the right hand.
    Uint32 flag = 0x80000000U, combo = Read32(l + 0x1c) & Read32(r + 0x20);
    if (combo)
        flag = 0;
    combo |= Read32(l + 0x20) & Read32(r + 0x1c);
    if (combo)
        for (Uint16 i = 0xaa; i < 0xd9; ++i)
        {
            const Uint8 *e = NULL;
            if (!Resolve(dra, 0x800a0000, (Uint64)Read32(pointer) + (Uint32)i * 52, 50, &e))
                return false;
            if (combo & Read32(e + 0x1c))
            {
                *out = flag + i;
                return true;
            }
        }
    *out = 0;
    return true;
}
bool SotnEquipment_ReadCost(SotnDataView dra, Uint16 id, Uint16 *out)
{
    const Uint8 *pointer = NULL, *record = NULL;
    if (!out || id >= 217)
        return SDL_SetError("Equipment MP cost requires a bounded original hand item");
    if (!Resolve(dra, 0x800a0000, 0x800a00c0, 4, &pointer) ||
        !Resolve(dra, 0x800a0000, (Uint64)Read32(pointer) + (Uint32)id * 52, 50, &record))
        return false;
    *out = Read16(record + 36);
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
    // Runesword143 (weapon14) and Heaven sword119 (weapon13) are kind129 swords thrown through
    // blueprint50, never consumed.
    const bool rune = item == 143 || item == 119;
    // Weapon12: Chakram133 (kind130, blueprint56, chain2) and the kind129 Vorpal blade163/
    // Crissaegrim164 slashes (blueprints50/52, descriptors1/2, chain3), never consumed.
    const bool chakram = item == 133, vorpal = item == 163 || item == 164;
    // Two-handed Yasutsuna165 (weapon53) throws its blueprint68 slash, never consumed.
    const bool yasutsuna = item == 165;
    const bool kept = rune || chakram || vorpal || yasutsuna;
    const Uint8 *pointer = NULL, *p = NULL;
    if (!out || (!karma && !magic && !thrown && !star && !kept && (item < 71 || item > 74)))
        return SDL_SetError("Throwing definition requires an actual supported original item");
    if (!Resolve(dra, 0x800a0000, 0x800a00c0, 4, &pointer) ||
        !Resolve(dra, 0x800a0000, (Uint64)Read32(pointer) + (Uint32)item * 52U, 50, &p))
        return false;
    const Uint8 blueprint = item == 79 || item == 82    ? 68
                            : item == 81 || item == 134 ? 62
                            : item == 82                ? 68
                            : item == 135               ? 63
                            : chakram                   ? 56
                            : item == 164               ? 52
                            : yasutsuna                 ? 68
                                                        : 50;
    const Uint8 chain = chakram  ? 2
                        : vorpal ? 3
                        : star   ? item == 79 ? 1 : 3
                        : thrown ? item == 82    ? 2
                                   : item == 135 ? 4
                                                 : 3
                        : magic  ? 4
                                 : 1;
    if (p[14] != (yasutsuna                 ? 5
                  : rune || chakram         ? 2
                  : vorpal                  ? 1
                  : magic || thrown || star ? 8
                                            : 7) ||
        p[15] != (rune                ? (item == 143 ? 14 : 13)
                  : chakram || vorpal ? 12
                  : yasutsuna         ? 53
                  : karma             ? 20
                  : star              ? 15
                  : thrown            ? 16
                  : magic             ? 22
                  : item == 71        ? 19
                  : item == 72        ? 18
                                      : 17) ||
        p[16] ||
        p[17] != ((thrown && item != 80) || item == 76 || item == 78 || rune || vorpal || yasutsuna
                      ? 129
                      : 130) ||
        p[18] !=
            (thrown || rune || vorpal || yasutsuna || item == 76 || item == 78 ? 0x5d : 0x60) ||
        p[19] != blueprint ||
        p[20] != (star && item != 79                         ? item - 75U
                  : item == 74 || item == 134 || item == 163 ? 1
                  : item == 164                              ? 2
                                                             : 0) ||
        p[21] || p[22] != chain || p[23] || p[24] || p[25] != (kept ? 0 : 1) || Read16(p + 36) ||
        Read16(p + 8) > 999 ||
        Read16(p + 40) != (karma || magic || thrown || star || kept ? 2 : 0x82))
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
                           .chain_limit = p[22],
                           .consumable = p[25]};
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
        (!SotnEquipment_IsShield(equipment.id) && equipment.id != 18 && equipment.id != 174 &&
         equipment.id != 19 && equipment.id != 24 && equipment.id != 25 && equipment.id != 80 &&
         equipment.id != 81 && equipment.id != 82 && equipment.id != 134 && equipment.id != 135 &&
         equipment.id != 143 && equipment.id != 119 && equipment.id != 133 && equipment.id != 163 &&
         equipment.id != 164 && equipment.id != 165 && !SotnEquipment_IsSword0(equipment.id) &&
         !SotnEquipment_IsBasicFist(equipment.id) && !SotnEquipment_IsPlainWeapon(equipment.id) &&
         equipment.id != 123 && equipment.id != 197 && equipment.id != 216 && equipment.id != 128 &&
         (equipment.id < 29 || equipment.id > 79)) ||
        equipment.family !=
            (equipment.id == 24                         ? 20
             : equipment.id >= 75 && equipment.id <= 79 ? 15
             : equipment.id == 80 || equipment.id == 81 || equipment.id == 82 ||
                     equipment.id == 134 || equipment.id == 135
                 ? 16
             : equipment.id == 25                                                ? 22
             : equipment.id == 143                                               ? 14
             : equipment.id == 119                                               ? 13
             : equipment.id == 133 || equipment.id == 163 || equipment.id == 164 ? 12
             : equipment.id == 165                                               ? 53
             : SotnEquipment_IsSword0(equipment.id)                              ? 0
             : SotnEquipment_IsBasicFist(equipment.id)                           ? 1
             : SotnEquipment_IsPlainWeapon(equipment.id)  ? PlainWeapon(equipment.id).family
             : equipment.id == 18 || equipment.id == 174  ? 31
             : equipment.id == 19                         ? 37
             : equipment.id == 123 || equipment.id == 197 ? 43
             : SotnEquipment_IsShield(equipment.id)       ? ShieldFamily(equipment.id)
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
