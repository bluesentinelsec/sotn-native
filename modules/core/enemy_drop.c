// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected enemy drop rolls: dra/5D5BC.c func_800FF460/func_800FF494 and st/collision.h's drop
// tables at sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Tables stay in private overlays.
#include <sotn/combat.h>
#include <sotn/enemy_drop.h>

static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static Sint32 Rand(Uint32 *random)
{
    return (Sint32)SotnCombat_Random(random);
}
Sint32 SotnEnemyDrop_LuckCutoff(Uint32 cutoff, Sint32 luck)
{
    if (!cutoff)
        return 0;
    return (Sint32)(cutoff + cutoff * (Uint32)luck / 0x80U);
}
bool SotnEnemyDrop_Kind(SotnEnemyDropRates rates, Sint32 luck, Uint16 arcana, Uint32 *random,
                        Sint32 *kind)
{
    if (!random || !kind || arcana > 2 || luck < -999 || luck > 999)
        return SDL_SetError("Enemy drop roll requires bounded luck, Ring of Arcana and outputs");
    const Sint32 rare = rates.rare_rate, uncommon = rates.uncommon_rate;
    Sint32 roll = Rand(random) & 0xff;
    roll -= (luck + (Rand(random) & 0x1f)) / 20;
    roll -= rare * arcana;
    if (roll < rare)
    {
        *kind = 0x40;
        return true;
    }
    roll -= rare;
    roll -= uncommon * arcana;
    roll -= (luck + (Rand(random) & 0x1f)) / 20;
    if (roll < uncommon)
    {
        *kind = 0x20;
        return true;
    }
    roll = Rand(random) % 28;
    roll += !rare;
    roll += !uncommon;
    *kind = roll + arcana;
    return true;
}
bool SotnEnemyDrop_ReadTables(const Uint8 *overlay, size_t size, size_t luck_offset,
                              SotnEnemyDropTables *out)
{
    static const Uint8 pattern[20] = {0x00, 0x40, 0x20, 0x10, 0x80, 0x80, 0xa0, 0xa0, 0xc0, 0xc0,
                                      0x00, 0x00, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x00, 0x00};
    if (!overlay || !out || luck_offset < 0x320 || luck_offset > size ||
        size - luck_offset < 0x90 || SDL_memcmp(overlay + luck_offset, pattern, sizeof(pattern)))
        return SDL_SetError("Stage overlay lacks its original collision drop tables");
    SotnEnemyDropTables t = {0};
    SDL_memcpy(t.luck_cutoff, overlay + luck_offset, 4);
    for (Uint16 i = 0; i < 400; ++i)
        t.enemy_lookup[i] = Read16(overlay + luck_offset - 0x320U + i * 2U);
    for (Uint16 i = 0; i < 32; ++i)
        t.prize[i] = Read16(overlay + luck_offset + 0x34U + i * 2U);
    for (Uint16 i = 0; i < 14; ++i)
        t.jewel_sword[i] = Read16(overlay + luck_offset + 0x74U + i * 2U);
    // The search for a threshold reaching rand() & 0xFFF ends at the last pair.
    if (t.jewel_sword[12] != 0xfff)
        return SDL_SetError("Stage overlay's Jewel sword drop table is malformed");
    *out = t;
    return true;
}
Uint16 SotnEnemyDrop_Jewel(const SotnEnemyDropTables *tables, Uint32 *random)
{
    const Uint16 roll = (Uint16)(SotnCombat_Random(random) & 0xfffU);
    for (Uint16 i = 0; i < 7; ++i)
        if (tables->jewel_sword[i * 2U] >= roll)
            return tables->jewel_sword[i * 2U + 1U];
    return tables->jewel_sword[13];
}
