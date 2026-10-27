// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/7E4BC.c:UnknownEntId48, 7879C.c blueprint/factory/spell initialization and
// 5D5BC.c:GetSpellDef, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Original blueprint and spell definitions remain private runtime inputs.
#include <sotn/combat.h>
#include <sotn/gas_cloud.h>

static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static Sint32 Pixel(Sint32 p)
{
    return p >= 0 ? p / 65536 : (Sint32)(-((-(Sint64)p + 65535) / 65536));
}
bool SotnGasCloud_Begin(SotnGasCloud *cloud, SotnDataView dra, SotnCharacter *character,
                        Uint32 generation, Uint16 available)
{
    const size_t blueprint_offset = 0xd1d4 + 83U * 6U, spell_offset = 0x8410 + 12U * 28U;
    if (!cloud || !character || !generation || !dra.bytes || dra.size < blueprint_offset + 6 ||
        dra.size < spell_offset + 28)
        return SDL_SetError("Gas Cloud requires owned Mist and private factory/spell definitions");
    const Uint8 *bp = dra.bytes + blueprint_offset, *spell = dra.bytes + spell_offset;
    if (bp[0] != 48 || bp[1] != 17 || bp[2] != 0xd1 || bp[3] || bp[4] != 0x22 || bp[5])
        return SDL_SetError(
            "Gas Cloud blueprint requires its original bounded noncritical factory");
    const Uint16 base = Read16(spell + 24), hit_type = Read16(spell + 16);
    if (base > 900 || hit_type != 2 || Read16(spell + 20) != 0)
        return SDL_SetError("Gas Cloud spell has invalid attack, hitbox or room metadata");
    SotnCharacterState profile = {0};
    SotnCharacterStats stats = {0};
    if (!SotnCharacter_Get(character, &profile, &stats))
        return false;
    SotnGasCloud next = {.generation = generation};
    Uint16 remaining = available;
    for (Uint16 parameter = 0; parameter < 17; ++parameter)
    {
        if (!remaining)
            break;
        Uint16 slot = 0;
        while (!(remaining & (Uint16)(1U << slot)))
            ++slot;
        remaining &= (Uint16) ~(1U << slot);
        next.children[parameter].slot = (Uint16)(32U + slot);
        Uint32 power =
            base + (stats.total[2] * 2U + SotnCombat_Random(&profile.random) % 12U) / 10U;
        if (profile.equipment[SOTN_EQUIP_ARMOR] == 21)
            power += power / 2U;
        if (power > 999)
            return SDL_SetError("Gas Cloud spell power exceeds native combat bounds");
        next.children[parameter].attack = (SotnPlayerAttack){.width = parameter ? 8 : 28,
                                                             .height = parameter ? 8 : 28,
                                                             .power = (Uint16)power,
                                                             .element = Read16(spell + 22),
                                                             .hit_type = hit_type,
                                                             .hit_effect = Read16(spell + 18),
                                                             .stun_frames = Read16(spell + 14),
                                                             .invincibility_frames = spell[13],
                                                             .active = true};
        next.mask |= (Uint16)(1U << parameter);
    }
    if (!SotnCharacter_Restore(character, &profile))
        return false;
    *cloud = next;
    return true;
}
bool SotnGasCloud_Occupied(const SotnGasCloud *cloud, Uint64 *out)
{
    if (!cloud || !out || (cloud->mask && !cloud->generation))
        return SDL_SetError("Gas Cloud slots require a generation-owned cursor and output");
    Uint64 occupied = 0;
    for (Uint16 i = 0; i < SOTN_GAS_CLOUD_SLOTS; ++i)
        if (cloud->mask & (Uint16)(1U << i))
        {
            const Uint16 slot = cloud->children[i].slot;
            if (slot < 32 || slot > 47 || (occupied & ((Uint64)1 << slot)))
                return SDL_SetError("Gas Cloud has duplicate or invalid actual DRA slot ownership");
            occupied |= (Uint64)1 << slot;
        }
    *out = occupied;
    return true;
}
bool SotnGasCloud_Tick(SotnGasCloud *cloud, SotnPlayerState player, const SotnMistEffect *effect,
                       Uint32 generation, Sint32 camera_x, Sint32 camera_y, bool enabled)
{
    if (!cloud)
        return SDL_InvalidParamError("cloud");
    if (!(player.status & 2U))
    {
        *cloud = (SotnGasCloud){0};
        return true;
    }
    if (!cloud->generation)
        return true;
    if (!effect || effect->removed || effect->phase > 5 || generation != cloud->generation ||
        generation != player.mist.generation || camera_x < 0 || camera_x > 16384 || camera_y < 0 ||
        camera_y > 16384)
        return SDL_SetError("Gas Cloud lost its actual generation-owned Mist visual or camera");
    SotnGasCloud next = *cloud;
    const Sint32 x = Pixel(player.x) - camera_x, y = Pixel(player.y) - camera_y;
    for (Uint16 i = 0; i < SOTN_GAS_CLOUD_SLOTS; ++i)
    {
        if (!(next.mask & (Uint16)(1U << i)))
            continue;
        next.children[i].x = camera_x + (i ? Pixel(effect->points[i - 1U].x)
                                           : (Sint32)(((Sint64)x + Pixel(effect->x)) / 2));
        next.children[i].y = camera_y + (i ? Pixel(effect->points[i - 1U].y)
                                           : (Sint32)(((Sint64)y + Pixel(effect->y)) / 2));
        next.children[i].attack.active = enabled;
    }
    *cloud = next;
    return true;
}
