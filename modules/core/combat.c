// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted from sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5:
// src/st/collision.h:HitDetection and src/dra/5D5BC.c:DealDamage.
#include <sotn/combat.h>

bool SotnCombat_Overlap(SotnCombatBox attacker, SotnCombatBox target)
{
    if (attacker.width == 0 || attacker.height == 0 || target.width == 0 || target.height == 0 ||
        attacker.width > 255 || attacker.height > 255 || target.width > 255 || target.height > 255)
        return false;
    const Uint32 width = (Uint32)attacker.width + target.width - 1;
    const Uint32 height = (Uint32)attacker.height + target.height - 1;
    const Uint16 dx = (Uint16)((Uint32)attacker.x - (Uint32)target.x + width);
    const Uint16 dy = (Uint16)((Uint32)attacker.y - (Uint32)target.y + height);
    return dx <= width * 2 && dy <= height * 2;
}

Uint16 SotnCombat_Random(Uint32 *state)
{
    *state = *state * 0x41c64e6dU + 0x3039U;
    return (Uint16)((*state >> 16) & 0x7fff);
}

static Uint16 Root(Uint16 value)
{
    Uint16 result = 0;
    while ((Uint32)(result + 1) * (result + 1) <= value)
        ++result;
    return result;
}

bool SotnCombat_Damage(SotnCombatEnemy enemy, Uint16 attack, Uint16 element, Uint8 critical_rate,
                       const Uint16 stats[4], Uint16 rooms, Uint32 *random, Uint16 *out)
{
    if (random == NULL || out == NULL || stats == NULL || attack > 999 || rooms > 1024 ||
        enemy.defense < 0)
        return SDL_SetError("Combat damage requires bounded original attributes and outputs");
    for (Uint32 i = 0; i < 4; ++i)
        if (stats[i] == 0 || stats[i] > 99)
            return SDL_SetError("Combat base stats must be in the original 1..99 range");
    Uint32 next_random = *random;
    Uint16 damage = attack, result = 0;
    if (element == 0)
        element = 0x20;
    const Uint16 mask = (element & 0xff80) ? 0xff80 : 0x7f;
    const Uint16 masked = element & mask;
    if (masked == (masked & enemy.immunes))
        result = 0xc000;
    else if (masked == (masked & enemy.absorbs))
        result = damage | 0x8000;
    else
    {
        if (masked == (masked & enemy.strengths))
            damage /= 2;
        if (element & enemy.weaknesses)
            damage *= 2;
        if (critical_rate > (SotnCombat_Random(&next_random) & 255))
        {
            Uint16 rolls[4] = {0};
            Uint32 highest = 0;
            for (Uint32 i = 0; i < 4; ++i)
            {
                rolls[i] = (Uint16)((SotnCombat_Random(&next_random) & 63) + stats[i]);
                if (rolls[i] > rolls[highest])
                    highest = i;
            }
            switch (highest)
            {
            case 0:
                damage *= 2;
                break;
            case 1:
                damage =
                    (Uint16)(damage + (enemy.defense > damage ? enemy.defense / 2 : damage / 2));
                break;
            case 2:
                damage = (Uint16)(damage + Root(rooms));
                break;
            default:
                damage = (Uint16)(damage + SotnCombat_Random(&next_random) % stats[3] + 1);
                break;
            }
            result = 0x4000;
        }
        damage = damage > enemy.defense ? (Uint16)(damage - enemy.defense) : 1;
        result = (Uint16)(result + SDL_min(damage, 9999));
    }
    *random = next_random;
    *out = result;
    return true;
}
