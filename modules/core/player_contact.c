// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/692E8.c contact ordering and 5D5BC.c water/normal damage dispatch,
// sotn-decomp7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. No private tables embedded.
#include "player_internal.h"
#include <sotn/combat.h>

bool SotnPlayer_QueueContact(SotnPlayer *p, SotnPlayerContact contact)
{
    if (!p || contact.attack > 9999 || contact.source_x < -512 || contact.source_x > 16896 ||
        contact.source_velocity < -16 * 65536 || contact.source_velocity > 16 * 65536)
        return SDL_SetError("Queued player contact requires bounded original power and source");
    if (!SotnPlayerStep_IsDead(p->step) && contact.attack > p->queued_contact.attack)
        p->queued_contact = contact;
    return true;
}
bool SotnPlayer_SetDamageContext(SotnPlayer *p, Uint32 timer)
{
    if (p && p->armor == 22 && !p->stat_buffs)
        return SDL_SetError(
            "Fury Plate damage requires its unintegrated original defense-buff owner");
    if (!p || !p->death_capture || !p->death_decay || !p->death_context ||
        (p->damage_pending && p->damage_clock != timer) ||
        (!p->damage_pending && p->damage_ticked && p->damage_last_clock == timer))
        return SDL_SetError(
            "Damage context requires actual capture owners and a new gameplay clock");
    p->damage_clock = timer;
    p->damage_context = p->damage_pending = true;
    return true;
}
bool SotnPlayer_SetStatBuffContext(SotnPlayer *p, const SotnStatBuffs *buffs,
                                   SotnCharacterStats attributes)
{
    if (!p || !SotnStatBuffs_Mask(buffs, NULL) || attributes.defense > 999 ||
        attributes.attack[0] > 999 || attributes.attack[1] > 999 || attributes.unarmed_attack > 999)
        return SDL_SetError(
            "Stat-buff context requires actual counters and current bounded attributes");
    p->stat_buffs = buffs;
    p->attributes = attributes;
    return true;
}
bool SotnPlayer_SetWaterPainTimer(SotnPlayer *p, Uint8 timer)
{
    if (!p || timer > 32)
        return SDL_SetError("Water pain voice timer must retain its original0..32 range");
    p->water_damage.pain_timer = timer;
    return true;
}
bool SotnPlayer_ContactTick(SotnPlayer *p, bool *born)
{
    if (!p || !born)
        return SDL_SetError("Player contact dispatch requires its actual working owner");
    *born = false;
    if (!p->damage_context)
        return true;
    if (!p->damage_pending)
        return SDL_SetError("Player damage update requires its next gameplay-clock context");
    if (SotnPlayerStep_IsDead(p->step))
    {
        p->queued_contact = (SotnPlayerContact){0};
        return true;
    }
    if (!p->hp)
    {
        if (p->step != SOTN_PLAYER_STONE || !p->invincibility)
            return SDL_SetError("Zero-health damage dispatch lost its protected stone owner");
        if (p->water_damage.pain_timer)
            --p->water_damage.pain_timer;
        p->queued_contact = (SotnPlayerContact){0};
        return true;
    }
    const SotnPlayerContact contact = p->queued_contact;
    SotnWaterDamageEvents water = {0};
    if (!SotnWaterDamage_Tick(
            &p->water_damage,
            (SotnWaterDamageInput){.game_timer = p->damage_clock,
                                   .status = SotnPlayer_Status(p),
                                   .hp = p->hp,
                                   .hp_max = p->hp_max,
                                   .waist = p->water.waist,
                                   .hit_params = contact.parameters,
                                   .holy_symbol = (p->relics[SOTN_RELIC_HOLY_SYMBOL] & 3U) == 3,
                                   .invincible = p->invincibility != 0,
                                   .grabbed = p->grab.state != SOTN_GRAB_NONE || p->script_hold ||
                                              p->step == SOTN_PLAYER_SCRIPT,
                                   .fury_plate = p->armor == 22 && p->stat_buffs != NULL},
            &water))
        return false;
    p->queued_contact = (SotnPlayerContact){0};
    if (water.response == 6)
        return SotnPlayer_StartWaterDeath(p, water);
    if (water.response == 9)
    {
        p->hp = water.hp;
        p->defense_buff_minimum = water.defense_buff_min;
        p->water_pain = (SotnPlayerWaterPain){
            .events = water,
            .source = {
                .x = p->x.value, .y = p->y.value, .priority = 148, .facing_left = p->facing_left}};
        return true;
    }
    if (!water.delegated)
        return true;
    const SotnCharacterStats attributes = p->attributes;
    SotnContactPower power = {0};
    if (!SotnCombat_ContactPower(contact.attack, water.hit_params,
                                 (SotnCombatEnemy){.weaknesses = attributes.weakness,
                                                   .strengths = attributes.resistance,
                                                   .immunes = attributes.immunity,
                                                   .absorbs = attributes.absorption},
                                 &power))
        return false;
    if (power.absorb)
    {
        p->hp = (Uint16)SDL_min(p->hp_max, (Uint32)p->hp + power.attack);
        return true;
    }
    if (power.immune)
        return true;
    const SotnPlayerStep previous = p->step;
    const Uint16 previous_hp = p->hp;
    const SotnPlayerFactorySource source = {
        .x = p->x.value, .y = p->y.value, .priority = 148, .facing_left = p->facing_left};
    const bool stone = (power.effects & 0x200U) || previous == SOTN_PLAYER_STONE;
    // Existing selected reaction helpers normally advance their birth animation for external
    // callers. Inside the player dispatcher, its common animation tail owns that update once.
    p->contact_dispatch = true;
    bool ok = false;
    if (stone)
    {
        const Uint16 amount =
            previous == SOTN_PLAYER_STONE
                ? (Uint16)(p->hp_max / 8U)
                : (Uint16)SDL_clamp((Sint32)power.attack - (Sint32)attributes.defense * 2, 0, 9999);
        ok = SotnPlayer_Petrify(p, amount, contact.source_x, contact.source_velocity);
    }
    else
    {
        Uint16 amount = 0;
        if (!((water.hit_params & 31U) >= 16
                  ? SotnCombat_EnvironmentalContact(power, p->hp_max, p->poison_ticks != 0, &amount)
                  : SotnCombat_NormalContact(power, attributes.defense, p->poison_ticks != 0,
                                             &amount)))
            return false;
        ok = (power.effects & 0x100U)
                 ? SotnPlayer_CurseDamage(p, amount, contact.source_x, contact.source_velocity)
             : (power.effects & 0x80U)
                 ? SotnPlayer_PoisonDamage(p, amount, contact.source_x, contact.source_velocity)
                 : SotnPlayer_Damage(p, amount, contact.source_x, contact.source_velocity);
    }
    p->contact_dispatch = false;
    if (ok && p->hp && p->cape == 54 && p->hearts < p->hearts_max &&
        (p->hp < previous_hp ||
         (stone && p->step == SOTN_PLAYER_STONE && previous != SOTN_PLAYER_HIT)))
        p->heart_gain = (SotnPlayerHeartGain){
            .source = source, .amount = (Uint16)(previous_hp - p->hp), .requested = true};
    if (ok && p->hp && previous != SOTN_PLAYER_HIT && p->armor == 22 && p->stat_buffs)
        p->defense_buff_minimum = 512;
    *born = ok && p->step != previous;
    return ok;
}
