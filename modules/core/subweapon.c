// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected g_SubwpnDefs reads and func_800FE3C4's subweapon cost/attack in dra/5D5BC.c and
// 6E42C.c at sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Definitions remain private
// runtime data; every subweapon entity runs on the shared player runtime (subweapon_entity.c).
#include <sotn/combat.h>
#include <sotn/subweapon.h>

static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
bool SotnSubweapon_Read(SotnDataView dra, Uint16 id, SotnSubweaponDefinition *out)
{
    if (!out || id >= 13 || dra.bytes == NULL || dra.size < 0x4a00 + 13 * 20)
        return SDL_SetError("Subweapon definition is outside the private DRA table");
    const Uint8 *p = dra.bytes + 0x4a00 + id * 20;
    const Uint16 power = Read16(p), hearts = Read16(p + 2);
    // CheckSubwpnChainLimit counts at most the sixteen slots 32..47.
    if (power > 999 || hearts > 9999 || p[6] > 16 || Read16(p + 18) > 255 ||
        (id != 0 && p[6] == 0) || p[11] >= 123)
        return SDL_SetError(
            "Subweapon %u has invalid attack, heart cost, chain, blueprint or critical bounds", id);
    *out = (SotnSubweaponDefinition){.attack = {.power = power,
                                                .element = Read16(p + 4),
                                                .invincibility_frames = p[7],
                                                .stun_frames = Read16(p + 8),
                                                .hit_type = Read16(p + 12),
                                                .hit_effect = Read16(p + 14),
                                                .critical_rate = (Uint8)Read16(p + 18),
                                                .active = true},
                                     .hearts = hearts,
                                     .chain = p[6],
                                     .animation = p[10],
                                     .blueprint = p[11],
                                     .crash = p[16]};
    return true;
}
Uint16 SotnSubweapon_Cost(SotnCharacterState profile, Uint16 cost)
{
    const Uint16 broaches =
        (Uint16)((profile.equipment[5] == 77 ? 1 : 0) + (profile.equipment[6] == 77 ? 1 : 0));
    return (Uint16)SDL_max(1, cost / (broaches + 1));
}
bool SotnSubweapons_Power(Uint16 id, Uint16 base, const SotnCharacterState *profile,
                          Uint16 intelligence, Uint32 *random, Uint16 *power)
{
    if (!profile || !random || !power || id == 0 || id >= 13)
        return SDL_SetError("Subweapon power requires a subweapon, profile, random and output");
    Uint16 value = base;
    // Brilliant mail20 in the armor slot adds ten to every subweapon.
    if (profile->equipment[3] == 20)
        value = (Uint16)(value + 10U);
    // US: each Staurolite61 accessory doubles/triples the cross (4) and subweapon12.
    if (id == 4 || id == 12)
    {
        const Uint16 count =
            (Uint16)((profile->equipment[5] == 61 ? 1 : 0) + (profile->equipment[6] == 61 ? 1 : 0));
        value = (Uint16)(value * (count + 1U));
    }
    *power = (Uint16)(value + (intelligence * 2 + SotnCombat_Random(random) % 12) / 10);
    return true;
}
