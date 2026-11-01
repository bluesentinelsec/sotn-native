// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/7E4BC.c:EntityPlayerPinkEffect and 5D5BC.c:GetStatusAilmentTimer,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Profiles stay private.
#include <SDL3/SDL.h>
#include <sotn/medicine_effect.h>

static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
bool SotnMedicineEffect_Tick(SotnMedicineEffect *effect, SotnMedicineEffectInput in,
                             SotnMedicineEffectEvents *out)
{
    const Uint16 profile = effect ? (effect->parameter >> 8) & 0x7fU : 0;
    const size_t at = 0xdef0U + profile * 28U;
    if (!effect || !out || effect->removed || effect->phase > 2 || profile >= 20 ||
        effect->index < 0 || effect->index > 7 || !in.hp_max || in.hp_max > 9999 || !in.dra.bytes ||
        at > in.dra.size || in.dra.size - at < 28)
        return SDL_SetError("Medicine effect requires its actual cursor and private profile");
    const Uint8 *row = in.dra.bytes + at;
    const Uint32 type =
        (Uint32)row[24] | (Uint32)row[25] << 8 | (Uint32)row[26] << 16 | (Uint32)row[27] << 24;
    if (!(type < 16 || (type >= 0x80 && type <= 0x87)))
        return SDL_SetError("Medicine effect private type exceeds actual resource owners");
    SotnMedicineEffect n = *effect;
    SotnMedicineEffectEvents events = {0};
    switch (n.phase)
    {
    case 0:
        n.timer = row[0];
        if (type != 0x83)
            events.sounds[events.sound_count++] = 0x668;
        if (type >= 128)
            events.protection = 64;
        else
            events.buff_mask = (Uint16)(1U << type);
        n.phase = 1;
        break;
    case 1:
        n.timer = Word(n.timer - 1);
        if (n.timer)
            break;
        if (row[8 + n.index])
        {
            events.factories[events.factory_count++] = row[8 + n.index] | (Uint32)row[16 + n.index]
                                                                              << 16;
            if (row[8 + n.index] == 40)
                events.sounds[events.sound_count++] = 0x67d;
        }
        n.index = Word(n.index + 1);
        if (n.index > 7)
            return SDL_SetError("Medicine effect lost its private timer sentinel");
        n.timer = row[n.index];
        if (n.timer != 255)
            break;
        switch (type)
        {
        case 0x80:
        case 0x81:
            events.cure_mask = (Uint8)(1U << (type - 0x80));
            events.sounds[events.sound_count++] = 0x669;
            break;
        case 0x82:
            break;
        case 0x83:
            events.hammer = in.player_step == 11;
            break;
        case 0x84:
        case 0x85:
            events.heal = true;
            events.heal_amount = type == 0x84 ? 50 : 100;
            if (in.enhanced_potion)
                events.heal_amount += events.heal_amount / 2U;
            break;
        case 0x86:
            events.heal = true;
            events.heal_amount = in.hp_max;
            break;
        case 0x87:
            events.mp_full = true;
            events.sounds[events.sound_count++] = 0x669;
            break;
        default:
            if (in.dra.size <= 0xe120U + type)
                return SDL_SetError("Medicine effect lost its actual private buff label");
            events.factories[events.factory_count++] = 47U | (Uint32)in.dra.bytes[0xe120U + type]
                                                                 << 16;
            events.sounds[events.sound_count++] = 0x669;
            break;
        }
        n.phase = 2;
        break;
    case 2:
        n.removed = true;
        break;
    }
    *effect = n;
    *out = events;
    return true;
}
