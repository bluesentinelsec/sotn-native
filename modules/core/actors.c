// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted selected Laboratory behaviors and hit/drop paths from sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5: src/st/nz0/e_breakable.c,
// e_red_eye_bust.c, e_nz0_room2.c; src/st/e_bloody_zombie.h,
// animate_entity.h, st_common.h, st_update.h, collision.h, e_collect.h,
// and collect_subweapon.h. No game tables are embedded.
#include "actors_internal.h"
#include <SDL3/SDL_error.h>
#include <sotn/actors.h>
#include <sotn/combat.h>
#include <sotn/subweapon.h>

bool ActorReplaceExplosion(SotnActors *s, Actor *a, Uint16 kind)
{
    ActorPrimitivesClear(s, a);
    a->state.primitive_count = 0;
    a->state.id = 2;
    a->state.parameters = kind;
    a->state.step = a->state.frame = a->pose = a->remaining = a->substep = 0;
    a->state.rotated = false;
    a->state.scale_x = a->state.scale_y = 0;
    return true;
}

bool ActorInitialize(SotnActors *actors, Actor *actor, size_t offset)
{
    SotnDataView init = {0}, api = {0}, enemy = {0};
    if (!Slice(actors->nz0, offset, 10, &init) || !Slice(actors->dra, 0x98, 4, &api))
        return false;
    const Uint32 address = Read32(api.bytes), id = Read16(init.bytes + 8);
    if (address < 0x800a0000 || address % 4 != 0 || id >= 400 ||
        !Slice(actors->dra, (size_t)(address - 0x800a0000) + (size_t)id * 40, 40, &enemy))
        return SDL_SetError(
            "Laboratory actor initialization references an invalid enemy definition");
    actor->state.animation_set = Read16(init.bytes);
    actor->state.frame = Read16(init.bytes + 2);
    actor->state.texture_offset = Read16(init.bytes + 4);
    actor->state.palette = Read16(init.bytes + 6);
    actor->state.hit_points = (Sint16)Signed16(enemy.bytes + 4);
    actor->attack = Read16(enemy.bytes + 6);
    actor->attack_element = Read16(enemy.bytes + 8);
    actor->level = Read16(enemy.bytes + 22);
    actor->experience = Read16(enemy.bytes + 24);
    actor->enemy.defense = (Sint16)Signed16(enemy.bytes + 10);
    actor->state.hitbox_state = Read16(enemy.bytes + 12);
    actor->enemy.weaknesses = Read16(enemy.bytes + 14);
    actor->enemy.strengths = Read16(enemy.bytes + 16);
    actor->enemy.immunes = Read16(enemy.bytes + 18);
    actor->enemy.absorbs = Read16(enemy.bytes + 20);
    actor->state.hitbox_width = enemy.bytes[34];
    actor->state.hitbox_height = enemy.bytes[35];
    actor->state.flags = Read32(enemy.bytes + 36);
    actor->state.hitbox_x = actor->state.hitbox_y = 0;
    if (actor->state.priority == 0)
        actor->state.priority = 0x88;
    ++actor->state.step;
    actor->substep = 0;
    return true;
}

bool ActorAnimate(SotnActors *actors, Actor *actor, size_t offset, Uint8 *result)
{
    SotnDataView record = {0};
    if (actor->pose >= 512 || !Slice(actors->nz0, offset + (size_t)actor->pose * 2, 2, &record))
        return SDL_SetError("Laboratory actor animation exceeds its bounded frame stream");
    *result = 0;
    if (actor->remaining == 0)
    {
        if (record.bytes[0] == 255)
            return true;
        if (record.bytes[0] == 0)
        {
            if (!Slice(actors->nz0, offset, 2, &record) || record.bytes[0] == 0 ||
                record.bytes[0] == 255)
                return SDL_SetError("Laboratory actor animation cannot loop to a control record");
            actor->remaining = record.bytes[0];
            actor->state.frame = record.bytes[1];
            actor->pose = 1;
            return true;
        }
        actor->remaining = record.bytes[0];
        actor->state.frame = record.bytes[1];
        ++actor->pose;
        *result = 0x80;
    }
    if (actor->pose == 0 || !Slice(actors->nz0, offset + ((size_t)actor->pose - 1) * 2, 2, &record))
        return SDL_SetError("Laboratory actor animation has an invalid cursor");
    --actor->remaining;
    actor->state.frame = record.bytes[1];
    *result |= 1;
    return true;
}

bool ActorQuery(SotnActors *actors, Sint32 x, Sint32 y, SotnCollider *out)
{
    return SotnCollision_Query(&actors->map, x, y, out);
}

static bool Ground(SotnActors *actors, Actor *actor, bool *landed)
{
    SotnDataView sensors = {0};
    if (!Slice(actors->nz0, ActorTable(actors, 0x22d4, 0, 0x25d4), 16, &sensors))
        return false;
    actor->state.x += actor->vx;
    actor->state.y += actor->vy;
    if (actor->vy < 6 * 65536)
        actor->vy += 16384;
    *landed = false;
    if (actor->vy >= 0)
    {
        Sint32 x = Pixel(actor->state.x), y = Pixel(actor->state.y);
        for (Uint32 i = 0; i < 4; ++i)
        {
            x = Wrap16((Sint64)x + Signed16(sensors.bytes + i * 4));
            y = Wrap16((Sint64)y + Signed16(sensors.bytes + i * 4 + 2));
            SotnCollider col = {0}, back = {0};
            if (!ActorQuery(actors, x, y, &col))
                return false;
            if ((col.effects & SOTN_EFFECT_SLOPE_FLOOR) && i == 1)
            {
                if (col.effects & SOTN_EFFECT_SOLID)
                {
                    if (!ActorQuery(actors, x, y - 8, &back))
                        return false;
                    if (!(back.effects & SOTN_EFFECT_SOLID))
                    {
                        ShiftY(actor, (Sint64)4 + col.top);
                        actor->vx = actor->vy = 0;
                        actor->state.flags &= ~0x10000000U;
                        *landed = true;
                        return true;
                    }
                }
                continue;
            }
            if ((col.effects & (SOTN_EFFECT_SOLID | SOTN_EFFECT_SINK)) && i != 1)
            {
                if (col.effects & SOTN_EFFECT_SINK)
                {
                    actor->state.flags &= ~0x10000000U;
                    return true;
                }
                if (!ActorQuery(actors, x, y - 8, &back))
                    return false;
                if (!(back.effects & SOTN_EFFECT_SOLID))
                {
                    ShiftY(actor, col.top);
                    actor->vx = actor->vy = 0;
                    actor->state.flags &= ~0x10000000U;
                    *landed = true;
                    return true;
                }
            }
        }
    }
    actor->state.flags |= 0x10000000U;
    return true;
}

static bool WalkCollision(SotnActors *actors, Actor *actor)
{
    SotnDataView sensors = {0};
    if (!Slice(actors->nz0, ActorTable(actors, 0x22e4, 0, 0x25e4), 8, &sensors))
        return false;
    actor->state.x += actor->vx;
    ShiftY(actor, 3);
    Sint32 x = Wrap16((Sint64)Pixel(actor->state.x) + Signed16(sensors.bytes));
    Sint32 y = Wrap16((Sint64)Pixel(actor->state.y) + Signed16(sensors.bytes + 2));
    SotnCollider col = {0};
    if (!ActorQuery(actors, x, y, &col))
        return false;
    if (!(col.effects & SOTN_EFFECT_SOLID))
        return true;
    ShiftY(actor, col.top);
    if (actor->vx == 0)
        return true;
    const Sint32 sensor_x = Signed16(sensors.bytes + 4);
    x = Wrap16((Sint64)x + (actor->vx < 0 ? -sensor_x : sensor_x));
    y = Wrap16((Sint64)y + Signed16(sensors.bytes + 6) - 7);
    if (!ActorQuery(actors, x, y, &col))
        return false;
    if (col.effects & SOTN_EFFECT_SOLID)
    {
        if ((col.effects & SOTN_EFFECT_SLOPE_FLOOR) || !(col.effects & SOTN_EFFECT_SIDE))
            return true;
        actor->state.x -= actor->vx;
        actor->vx = 0;
        return true;
    }
    y = Wrap16((Sint64)y + 15);
    if (!ActorQuery(actors, x, y, &col))
        return false;
    if (!(col.effects & SOTN_EFFECT_SOLID))
    {
        actor->state.x -= actor->vx;
        actor->vx = 0;
    }
    return true;
}

bool ActorSound(SotnActors *actors, Uint16 id)
{
    if (actors->combat.sound_count >= SDL_arraysize(actors->combat.sounds))
        return SDL_SetError("Laboratory sound request budget exceeded");
    const Uint16 slot = actors->combat.sound_count++;
    actors->combat.sounds[slot] = id;
    actors->combat.volumes[slot] = SDL_MAX_UINT16;
    actors->combat.pans[slot] = 0;
    return true;
}

bool ActorSoundVolumePan(SotnActors *actors, Uint16 id, Uint16 volume, Sint8 pan)
{
    if (volume > 127 || pan < -8 || pan > 8)
        return SDL_SetError("Actor sound volume/pan exceeds its original bounds");
    if (!ActorSound(actors, id))
        return false;
    actors->combat.volumes[actors->combat.sound_count - 1] = volume;
    actors->combat.pans[actors->combat.sound_count - 1] = pan;
    return true;
}
bool ActorExplosion(SotnActors *actors, const Actor *source, Uint16 kind)
{
    for (Uint32 i = 160; i < SOTN_STAGE_ACTOR_SLOTS; ++i)
    {
        SotnActorPlacement placement = {0};
        Uint32 generation = 0;
        if (!SotnRoomActors_Get(actors->spawns, i, &placement, &generation))
            return false;
        Actor *effect = &actors->slots[i];
        if (generation != 0 || effect->state.generation != 0)
            continue;
        *effect = (Actor){0};
        ++actors->effect_serial;
        effect->state.generation = 0x80000000U | actors->effect_serial;
        effect->state.id = 2;
        effect->state.parameters = kind;
        effect->state.x = source->state.x;
        effect->state.y = source->state.y;
        effect->dynamic = true;
        effect->slot = (Uint16)i;
        if (kind == 2)
        {
            effect->state.y += 16 * 65536;
            effect->state.x += source->state.facing_left ? -8 * 65536 : 8 * 65536;
        }
        break;
    }
    return true;
}

bool ActorBloodyZombie(SotnActors *actors, Actor *actor, SotnPlayerState player)
{
    if (actors->stage_id != 12 && actors->stage_id != 7)
        return SDL_SetError("Bloody Zombie is not part of this stage profile");
    Uint8 animation = 0;
    if (actor->hit_params && (actor->state.step & 1))
    {
        if (!ActorSound(actors, 0x746) || !ActorSound(actors, 0x748))
            return false;
        SetStep(actor, 6);
    }
    if ((actor->state.flags & 0x100) && actor->state.step < 8)
    {
        if (!ActorSound(actors, 0x747))
            return false;
        actor->state.hitbox_state = 0;
        actor->state.flags &= ~0x20000000U;
        SetStep(actor, 8);
    }
    switch (actor->state.step)
    {
    case 0:
        if (!ActorInitialize(actors, actor, ActorTable(actors, 0xc7c, 0, 0xb38)))
            return false;
        actor->state.hitbox_x = 1;
        actor->state.hitbox_y = 4;
        SetStep(actor, 2);
        break;
    case 2: {
        bool landed = false;
        if (!Ground(actors, actor, &landed))
            return false;
        if (landed)
            SetStep(actor, 1);
        break;
    }
    case 1:
        if (actor->substep == 0)
        {
            actor->timer = 128;
            actor->substep = 1;
        }
        if (!ActorAnimate(actors, actor, ActorTable(actors, 0x22ec, 0, 0x25ec), &animation) ||
            !WalkCollision(actors, actor))
            return false;
        actor->vx = actor->state.facing_left ? 24576 : -24576;
        if (--actor->timer == 0)
        {
            actor->timer = 128;
            actor->state.facing_left = !actor->state.facing_left;
        }
        actors->random = actors->random * 0x01010101U + 1U;
        if (player.facing_left == (Side(actor, player) != 0) && Distance(actor, player) < 128)
        {
            actor->state.facing_left = Side(actor, player) == 0;
            SetStep(actor, 3);
        }
        break;
    case 3:
        if (!ActorAnimate(actors, actor, ActorTable(actors, 0x237c, 0, 0x267c), &animation))
            return false;
        if (animation == 0)
            actor->state.facing_left = Side(actor, player) == 0;
        if (!WalkCollision(actors, actor))
            return false;
        actor->vx = actor->state.facing_left ? 49152 : -49152;
        actors->random = actors->random * 0x01010101U + 1U;
        if (Distance(actor, player) < 40)
            SetStep(actor, 4);
        break;
    case 4:
        if (!ActorAnimate(actors, actor, ActorTable(actors, 0x22fc, 0, 0x25fc), &animation))
            return false;
        if ((animation & 0x80) && actor->pose == 10 && !ActorSound(actors, 0x60b))
            return false;
        if (animation == 0)
            SetStep(actor, 1);
        break;
    case 6:
        if (!ActorAnimate(actors, actor, ActorTable(actors, 0x2320, 0, 0x2620), &animation))
            return false;
        if (animation == 0)
        {
            SetStep(actor, 1);
            actor->substep = 1;
        }
        break;
    case 8:
        if (actor->pose < 13)
        {
            if ((actors->ticks & 7) == 0 && !ActorSound(actors, 0x749))
                return false;
            actor->timer = 0;
        }
        else if (actor->timer++ == 0 && !ActorSound(actors, 0x749))
            return false;
        if (!ActorAnimate(actors, actor, ActorTable(actors, 0x2334, 0, 0x2634), &animation))
            return false;
        if (animation == 0)
        {
            actor->timer = 64;
            actor->state.frame = 0;
            ++actor->state.step;
            if (!ActorExplosion(actors, actor, 2) || !ActorSound(actors, 0x693))
                return false;
        }
        break;
    case 9:
        if (--actor->timer == 0)
            actor->remove = true;
        break;
    default:
        return SDL_SetError("Bloody Zombie entered an invalid behavior state");
    }
    if (actor->state.frame == 11 || actor->state.frame == 12)
    {
        actor->state.hitbox_width = 18;
        actor->state.hitbox_height = 12;
        actor->state.hitbox_x = -12;
        actor->state.hitbox_y = -12;
    }
    else
    {
        actor->state.hitbox_width = 4;
        actor->state.hitbox_height = 22;
        actor->state.hitbox_x = 1;
        actor->state.hitbox_y = 4;
    }
    return true;
}

static SotnCombatBox PlayerBox(SotnPlayerState player, bool attack)
{
    const Sint32 offset_x = attack ? player.attack.offset_x : player.animation.hitbox_x;
    return (SotnCombatBox){Pixel(player.x) + (player.facing_left ? -offset_x : offset_x),
                           Pixel(player.y) +
                               (attack ? player.attack.offset_y : player.animation.hitbox_y),
                           attack ? player.attack.width : player.animation.hitbox_width,
                           attack ? player.attack.height : player.animation.hitbox_height};
}

static bool EnemyHitbox(const SotnActors *actors, Uint16 id)
{
    if (actors->stage_id == 0)
        return id == 0x47 || id == 0x48 || id == 0x4d || id == 0x4e || id == 0x4c || id == 0x3a ||
               id == 0x3b || (id >= 0x21 && id <= 0x25) || id == 0x2e || id == 0x2f || id == 0x4a ||
               id == 0x4b || id == 0x3d || id == 0x3e || id == 0x3f;
    if (actors->stage_id == 1)
        return id == 0x3e || id == 0x3f || id == 0x44 || id == 0x45 || id == 0x46 || id == 0x47;
    if (actors->stage_id == 7)
        return id == 0x32 || id == 0x34 || id == 0x39 || id == 0x3a || id == 0x3e || id == 0x40 ||
               id == 0x42 || id == 0x48;
    if (actors->stage_id == 65)
        return id == 0x2d || id == 0x2e || id == 0x3a || id == 0x3c || id == 0x3e || id == 0x41 ||
               id == 0x42 || id == 0x46 || id == 0x48 || id == 0x4a || id == 0x4b || id == 0x4c;
    switch (id)
    {
    case 0x26:
    case 0x27:
    case 0x29:
    case 0x2a:
    case 0x2b:
    case 0x2e:
    case 0x2f:
    case 0x31:
    case 0x33:
    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    case 0x45:
    case 0x46:
        return true;
    default:
        return false;
    }
}
static bool Hit(SotnActors *actors, Actor *actor, SotnPlayerState player)
{
    if (player.step == SOTN_PLAYER_KILL || actor->state.step == 0 || actor->state.unsupported ||
        (actor->state.flags & 0x100))
        return true;
    Actor *recipient = actor;
    if (actors->stage_id == 65 && actor->state.id == 0x4b)
    {
        const Uint32 owner = actor->enemy_state.owner_slot;
        if (owner >= SOTN_STAGE_ACTOR_SLOTS ||
            actors->slots[owner].state.generation != actor->enemy_state.owner_generation)
            return true;
        recipient = &actors->slots[owner];
        if (recipient->state.flags & 0x100)
            return true;
    }
    for (Uint32 i = 0; i < SDL_arraysize(actor->cooldown); ++i)
        if (actor->cooldown[i])
            --actor->cooldown[i];
    if (!EnemyHitbox(actors, actor->state.id) && actor->state.id != 1 && actor->state.id != 3 &&
        actor->state.id != 0xa && actor->state.id != 0xb &&
        (actors->stage_id != 12 ||
         (actor->state.id != 0x34 && actor->state.id != 0x17 && actor->state.id != 0x18 &&
          actor->state.id != 0x1a && actor->state.id != 0x25 && actor->state.id != 0x20 &&
          actor->state.id != 0x48 && actor->state.id != 0x1c)))
        return true;
    const SotnCombatBox target = {Pixel(actor->state.x) + (actor->state.facing_left
                                                               ? -actor->state.hitbox_x
                                                               : actor->state.hitbox_x),
                                  Pixel(actor->state.y) + actor->state.hitbox_y,
                                  actor->state.hitbox_width, actor->state.hitbox_height};
    const Sint32 screen_x = target.x - actors->camera_x, screen_y = target.y - actors->camera_y;
    if (screen_x <= -32 || screen_x >= 288 || screen_y <= -32 || screen_y >= 256)
        return true;
    for (Uint32 channel = 0; channel < 2 + SOTN_SUBWEAPON_SLOTS; ++channel)
    {
        SotnPlayerState source = player;
        if (channel < 2)
        {
            if (player.shield.equipment != 0 && channel == (player.shield.right_hand ? 1U : 0U))
                source.attack = player.shield;
            else if (channel != (player.attack.right_hand ? 1U : 0U))
                continue;
        }
        else
        {
            if (actors->subweapons == NULL)
                break;
            SotnSubweaponState projectile = {0};
            if (!SotnSubweapons_Get(actors->subweapons, channel - 2, &projectile))
                return false;
            if (actor->projectile_generation[channel - 2] != projectile.generation)
            {
                actor->projectile_generation[channel - 2] = projectile.generation;
                actor->cooldown[channel] = 0;
            }
            if (projectile.generation == 0)
                continue;
            source.x = projectile.x;
            source.y = projectile.y;
            source.facing_left = projectile.facing_left;
            source.attack = projectile.attack;
        }
        const Uint32 hand = channel;
        const SotnCombatBox attacker = PlayerBox(source, true);
        if (source.attack.active && actor->cooldown[hand] == 0 &&
            (actor->state.hitbox_state & source.attack.hit_type & 0x3e) &&
            SotnCombat_Overlap(attacker, target))
        {
            Uint16 stats[4] = {6, 6, 6, 6};
            Uint16 luck = 6;
            bool dragon_helm = false;
            if (actors->character != NULL)
            {
                SotnCharacterStats attributes = {0};
                SotnCharacterState progress = {0};
                if (!SotnCharacter_Get(actors->character, &progress, &attributes))
                    return false;
                SDL_memcpy(stats, progress.base, sizeof(stats));
                luck = attributes.total[3];
                dragon_helm = progress.equipment[SOTN_EQUIP_HEAD] == 45;
            }
            Uint16 damage = 0;
            SotnCombatEnemy enemy = recipient->enemy;
            if (dragon_helm)
                enemy.defense /= 2;
            if (source.attack.equipment == 16)
            {
                actor->hit_flags = 0x10;
                actor->hit_params = 1;
                actor->cooldown[hand] = source.attack.invincibility_frames;
                // Original shields destroy blockable projectiles without dealing enemy damage.
                if (actor->state.hitbox_state & 8)
                {
                    recipient->state.hit_points = 0;
                    recipient->state.flags |= 0x100;
                }
                return true;
            }
            if (recipient->state.hit_points != 0x7fff && recipient->state.hit_points != 0 &&
                !SotnCombat_DamageAttributes(enemy, source.attack.power, source.attack.element,
                                             source.attack.critical_rate, stats, luck, 1,
                                             &actors->combat_random, &damage))
                return false;
            actor->hit_flags = source.attack.hit_type;
            recipient->hit_flags = source.attack.hit_type;
            actor->hit_params = source.attack.hit_effect & 127;
            if (channel >= 2)
            {
                Uint16 flags = (actor->state.hitbox_state & 8) ? 3 : 1;
                if ((actor->hit_params == 3 && (recipient->state.flags & 0x8000)) ||
                    (actor->hit_params == 4 && (recipient->state.flags & 0x4000)))
                    flags = 2;
                if (!SotnSubweapons_Hit(actors->subweapons, channel - 2, flags))
                    return false;
            }
            actor->cooldown[hand] = source.attack.invincibility_frames;
            if (EnemyHitbox(actors, actor->state.id) && damage != 0xc000)
            {
                if (damage & 0x8000)
                    recipient->state.hit_points =
                        (Sint16)SDL_min(32767, recipient->state.hit_points + (damage & 0x3fff));
                else
                {
                    recipient->state.hit_points =
                        (Sint16)SDL_max(-32768, recipient->state.hit_points - (damage & 0x3fff));
                    if (recipient->state.hit_points <= 0)
                    {
                        recipient->state.flags |= 0x10c100;
                        recipient->state.flags &= ~0x20000000U;
                        if ((recipient->state.flags & 0x1000) && actors->combat.kills < 999999)
                            ++actors->combat.kills;
                        if (actors->character != NULL)
                        {
                            Uint16 levels = 0;
                            if (!SotnCharacter_GainExperience(actors->character,
                                                              recipient->experience,
                                                              recipient->level, &levels))
                                return false;
                            if (levels != 0)
                            {
                                SotnCharacterState progress = {0};
                                if (!SotnCharacter_Get(actors->character, &progress, NULL))
                                    return false;
                                actors->combat.hearts_max = progress.hearts_max;
                                if (!ActorSound(actors, 0x687))
                                    return false;
                                SDL_Log("Level up: %u (HP %u, MP %u, hearts %u)", progress.level,
                                        progress.hp_max, progress.mp_max, progress.hearts_max);
                            }
                        }
                    }
                }
                if (recipient->state.hit_points != 0x7fff &&
                    !(recipient->state.flags & (0x400000 | 0x100)))
                    recipient->stun = source.attack.stun_frames;
            }
            if (actors->stage_id == 0 &&
                (actor->state.id == 0x3d || (actor->state.id >= 0x21 && actor->state.id <= 0x24)))
            {
                const bool rose = actor->state.id == 0x3d;
                const Uint32 count = rose ? 12 : 16;
                Uint32 base = actor->slot;
                if (rose)
                {
                    if (actor->state.parameters >= count || actor->slot < actor->state.parameters)
                        return SDL_SetError("Stone Rose contact has an invalid segment chain");
                    base -= actor->state.parameters;
                }
                else if (actor->state.id != 0x21)
                {
                    const Actor *root = ActorDiploOwner(actors, actor);
                    if (root == NULL)
                        return SDL_SetError("Diplocephalus contact lost its original body");
                    base = root->slot;
                }
                if (base + count > SOTN_STAGE_ACTOR_SLOTS)
                    return SDL_SetError("Linked enemy contact exceeds its reserved segment range");
                if (actor->state.hit_points == 0x7fff)
                    actor->hit_flags = 0x10;
                for (Uint32 i = 0; i < count; ++i)
                {
                    Actor *peer = &actors->slots[base + i];
                    const bool family = rose ? peer->state.id == 0x3d
                                             : peer->state.id >= 0x21 && peer->state.id <= 0x24;
                    if (peer == actor || !family || !peer->state.generation)
                        continue;
                    if (actor->state.hit_points != 0x7fff || peer->state.hit_points == 0x7fff)
                        peer->cooldown[hand] =
                            (Uint8)SDL_min(255U, source.attack.invincibility_frames +
                                                     (peer->slot > actor->slot ? 1U : 0U));
                    if (actor->state.hit_points != 0x7fff)
                    {
                        peer->stun = actor->stun;
                        if (actor->state.flags & 0x100)
                        {
                            peer->state.flags |= 0x10c100;
                            peer->state.flags &= ~0x20000000U;
                        }
                    }
                }
            }
            return true;
        }
    }
    if (actors->stage_id == 12 && (actor->state.id == 0x20 || actor->state.id == 0x48) &&
        SotnCombat_Overlap(PlayerBox(player, false), target))
        actor->hit_params = 0x80;
    if ((actor->state.hitbox_state & 1) &&
        (actor->state.id == 3 || actor->state.id == 0xa || actor->state.id == 0xb ||
         !player.invulnerable) &&
        SotnCombat_Overlap(PlayerBox(player, false), target))
    {
        if ((actor->state.id == 3 || actor->state.id == 0xa || actor->state.id == 0xb) &&
            actor->state.step > (actor->state.id == 0xb ? 0U : 1U) && actor->state.step < 5 &&
            (actor->state.id == 0xb || actor->timer == 0))
            actor->state.step = 5;
        else if (EnemyHitbox(actors, actor->state.id) ||
                 (actors->stage_id == 12 && (actor->state.id == 0x1a || actor->state.id == 0x25)))
        {
            if ((actors->stage_id == 0 && actor->state.id == 0x4b) ||
                (actors->stage_id == 1 && actor->state.id == 0x45))
                actor->hit_flags |= 0x80;
            if (actor->attack > actors->combat.contact_damage)
            {
                actors->combat.contact_damage = actor->attack;
                actors->combat.contact_x = target.x;
                actors->combat.contact_velocity = actor->vx;
            }
        }
    }
    return true;
}
static void HeartFall(Actor *actor)
{
    if (actor->vy >= 0)
    {
        actor->fall_speed += actor->gravity;
        actor->vx = actor->fall_speed;
        if (actor->vx == 65536 || actor->vx == -65536)
            actor->gravity = -actor->gravity;
    }
    if (actor->vy < 16384)
        actor->vy += 8192;
}

static bool PrizeSides(SotnActors *actors, Actor *actor)
{
    if (actor->vx == 0)
        return true;
    SotnDataView sensors = {0};
    if (!Slice(actors->nz0, ActorTable(actors, 0x1d9c, 0x24e8, 0x1e74), 8, &sensors))
        return false;
    Sint32 x = Pixel(actor->state.x), y = Pixel(actor->state.y);
    for (Uint32 i = 0; i < 2; ++i)
    {
        const Sint32 dx = Signed16(sensors.bytes + i * 4);
        x = Wrap16((Sint64)x + (actor->vx < 0 ? dx : -dx));
        y = Wrap16((Sint64)y + Signed16(sensors.bytes + i * 4 + 2));
        SotnCollider col = {0};
        if (!ActorQuery(actors, x, y, &col))
            return false;
        if ((col.effects & SOTN_EFFECT_SIDE) &&
            (!(col.effects & SOTN_EFFECT_SLOPE_FLOOR) || i != 0))
        {
            const Sint32 high =
                Wrap16((Sint64)Pixel(actor->state.x) + (actor->vx < 0 ? col.right : col.left));
            actor->state.x = (Sint32)((Sint64)high * 65536 + ((Uint32)actor->state.x & 65535));
            break;
        }
    }
    return true;
}

static bool PrizeGround(SotnActors *actors, Actor *actor, Uint16 item, bool resting)
{
    SotnCollider col = {0};
    if (actor->vy < 0 && !resting)
    {
        if (!ActorQuery(actors, Pixel(actor->state.x), Pixel(actor->state.y) - 7, &col))
            return false;
        if (col.effects & (SOTN_EFFECT_SOLID | SOTN_EFFECT_SINK))
            actor->vy = 0;
    }
    if (!resting)
    {
        actor->state.x += actor->vx;
        actor->state.y += actor->vy;
    }
    if (!ActorQuery(actors, Pixel(actor->state.x), Pixel(actor->state.y) + 7, &col))
        return false;
    if ((col.effects & (SOTN_EFFECT_SOLID | SOTN_EFFECT_SINK)) &&
        (item == 0 || actor->vy > 0 || resting))
    {
        if (item != 0)
            actor->vx = actor->vy = 0;
        if (resting && item != 0 && (col.effects & SOTN_EFFECT_SINK))
            actor->state.y += 8192;
        else if (item != 0 || !resting)
            ShiftY(actor, col.top);
        if (!resting)
        {
            actor->life_timer = item == 0 ? 96 : 240;
            ++actor->state.step;
        }
    }
    else
    {
        if (resting)
        {
            actor->state.x += actor->vx;
            actor->state.y += actor->vy;
        }
        if (item == 0)
            HeartFall(actor);
        else if (actor->vy < 6 * 65536)
            actor->vy += 16384;
    }
    return resting || item == 0 || PrizeSides(actors, actor);
}

static bool Collectible(SotnActors *actors, Actor *actor);

static bool Prize(SotnActors *actors, Actor *actor, SotnPlayerState player)
{
    Uint16 item = actor->state.parameters & 0x7fff;
    if (item > 23)
        return SDL_SetError("Prize references an invalid original item ID");
    if (item == 13 || ((item == 12 || item == 23) && actors->character == NULL))
    {
        actor->state.unsupported = true;
        return true;
    }
    if (actor->state.step != 0)
    {
        SotnDataView pointer = {0};
        Uint8 result = 0;
        if (!Slice(actors->nz0, ActorTable(actors, 0x1d3c, 0x2488, 0x1e14) + (size_t)item * 4, 4,
                   &pointer) ||
            Read32(pointer.bytes) < 0x80180000 ||
            !ActorAnimate(actors, actor, Read32(pointer.bytes) - 0x80180000, &result))
            return false;
    }
    switch (actor->state.step)
    {
    case 0:
        if (!ActorInitialize(actors, actor, ActorTable(actors, 0xbd4, 0xab8, 0xa48)))
            return false;
        actor->state.priority = 0x6c;
        actor->state.blend_mode = 0;
        if (item >= 14 && item < 23)
        {
            SotnDataView old = {0};
            if (actors->combat.subweapon > 9 ||
                !Slice(actors->nz0,
                       ActorTable(actors, 0x1cd8, 0x2424, 0x1db0) + actors->combat.subweapon * 2U,
                       2, &old))
                return SDL_SetError("Development subweapon ID is invalid");
            if (item == Read16(old.bytes))
                item = actor->state.parameters = 1;
        }
        if (item == 0 || item == 2)
            actor->state.hitbox_width = 4;
        return true;
    case 1: {
        SotnCollider col = {0};
        if (!ActorQuery(actors, Pixel(actor->state.x), Pixel(actor->state.y), &col))
            return false;
        if (col.effects & (SOTN_EFFECT_SOLID | SOTN_EFFECT_SINK | SOTN_EFFECT_SIDE))
            actor->remove = true;
        else
            ++actor->state.step;
        if (item == 0)
        {
            actor->fall_speed = -65536;
            actor->gravity = 2048;
        }
        return true;
    }
    case 2:
        return PrizeGround(actors, actor, item, false);
    case 3:
    case 4:
        if (!PrizeGround(actors, actor, item, true))
            return false;
        if (!(actor->state.parameters & 0x8000) && --actor->life_timer == 0)
        {
            if (actor->state.step == 3)
            {
                actor->life_timer = item == 0 ? 64 : 80;
                ++actor->state.step;
            }
            else
                actor->remove = true;
        }
        if (actor->state.step == 4 && (actor->life_timer & 2))
            actor->state.frame = 0;
        return true;
    case 5: {
        SotnDataView prize = {0};
        if (item < 2)
        {
            if (!Slice(actors->nz0, ActorTable(actors, 0x1da4, 0x24f0, 0x1e7c) + item, 1, &prize) ||
                !ActorSound(actors, 0x67a))
                return false;
            actors->combat.hearts =
                (Uint16)SDL_min(actors->combat.hearts_max, actors->combat.hearts + prize.bytes[0]);
        }
        else if (item < 12)
        {
            if (!Slice(actors->nz0,
                       ActorTable(actors, 0x1d14, 0x2460, 0x1dec) + (size_t)(item - 2) * 4, 4,
                       &prize) ||
                !ActorSound(actors, 0x6a9))
                return false;
            const Uint32 amount = Read32(prize.bytes);
            if (amount > 999999)
                return SDL_SetError("Gold prize exceeds the original wallet limit");
            actors->combat.gold = SDL_min(999999U, actors->combat.gold + amount);
        }
        else if (item == 12 || item == 23)
        {
            if (!SotnCharacter_CollectVessel(actors->character, item == 23) ||
                !ActorSound(actors, 0x67a))
                return false;
            SotnCharacterState profile = {0};
            if (!SotnCharacter_Get(actors->character, &profile, NULL))
                return false;
            actors->combat.hearts = profile.hearts;
            actors->combat.hearts_max = profile.hearts_max;
        }
        else
        {
            if (!Slice(actors->nz0,
                       ActorTable(actors, 0x1cc4, 0x2410, 0x1d9c) + (size_t)(item - 14) * 2, 2,
                       &prize) ||
                !ActorSound(actors, 0x67c))
                return false;
            const Uint16 previous = actors->combat.subweapon;
            actors->combat.subweapon = Read16(prize.bytes);
            if (previous != 0)
            {
                if (previous > 9 ||
                    !Slice(actors->nz0, ActorTable(actors, 0x1cd8, 0x2424, 0x1db0) + previous * 2U,
                           2, &prize))
                    return SDL_SetError("Previous subweapon ID is invalid");
                actor->state.parameters = Read16(prize.bytes);
                actor->state.y = player.y + 12 * 65536;
                actor->state.frame = 0;
                actor->vx = player.facing_left ? 2 * 65536 : -2 * 65536;
                actor->vy = -163840;
                actor->timer = 96;
                actor->fall_speed = 5;
                SetStep(actor, 7);
                return true;
            }
        }
        if (!Collectible(actors, actor))
            return false;
        actor->remove = true;
        return true;
    }
    case 7:
        if (actor->substep == 0)
        {
            actor->substep = 1;
            return true;
        }
        if (actor->substep == 1)
        {
            const Uint16 before = actor->state.step;
            if (!PrizeGround(actors, actor, item, false))
                return false;
            if (actor->state.step != before)
            {
                actor->state.step = 7;
                actor->substep = 2;
            }
            actor->state.frame = 0;
            if (actor->fall_speed != 0)
                --actor->fall_speed;
        }
        else
        {
            if (!PrizeGround(actors, actor, item, true))
                return false;
            if (++actor->fall_speed >= 32)
            {
                actor->life_timer = 208;
                actor->state.step = 3;
                actor->substep = 0;
            }
            else
                actor->state.frame = 0;
        }
        return true;
    default:
        return SDL_SetError("Prize entered an unsupported state");
    }
}

static bool Collectible(SotnActors *actors, Actor *actor)
{
    if (actor->collectible == 0 || actors->character == NULL)
        return true;
    SotnCharacterState state = {0};
    if (!SotnCharacter_Get(actors->character, &state, NULL))
        return false;
    const Uint32 index = actor->collectible - 1U;
    if (index >= (SOTN_CASTLE_FLAGS - 0x100) * 8)
        return SDL_SetError("Persistent item bit exceeds original castle progress storage");
    actors->world->flags[0x100 + index / 8] |= (Uint8)(1U << (index & 7));
    if (actors->stage_id != 12)
        return true;
    const Uint32 local = index - 0xf8;
    if (index < 0xf8 || local >= 11)
        return SDL_SetError("Persistent Laboratory bit exceeds its original range");
    state.laboratory_items[local / 8] |= (Uint8)(1U << (local & 7));
    return SotnCharacter_Restore(actors->character, &state);
}

static bool EquipmentPrize(SotnActors *actors, Actor *actor, SotnPlayerState player)
{
    (void)player;
    if (actors->character == NULL)
    {
        actor->state.unsupported = true;
        return true;
    }
    const Uint16 id = actor->state.parameters & 0x7fff;
    if (id >= SOTN_HAND_ITEMS + SOTN_BODY_ITEMS)
        return SDL_SetError("Equipment drop exceeds both original item tables");
    switch (actor->state.step)
    {
    case 0:
        if (!ActorInitialize(actors, actor, ActorTable(actors, 0xbd4, 0xab8, 0xa48)))
            return false;
        actor->state.priority = 0x80;
        actor->state.animation_set = 0;
        actor->state.frame = 0;
        actor->state.blend_mode = 0;
        return true;
    case 1: {
        SotnCollider collider = {0};
        if (!ActorQuery(actors, Pixel(actor->state.x), Pixel(actor->state.y), &collider))
            return false;
        if (collider.effects & (SOTN_EFFECT_BLOCK | SOTN_EFFECT_SINK))
            actor->remove = true;
        else
        {
            actor->state.step = 2;
            actor->state.hitbox_width = actor->state.hitbox_height = 7;
            actor->timer = 0;
        }
        return true;
    }
    case 2: {
        SotnCollider collider = {0};
        if (actor->vy < 0)
        {
            if (!ActorQuery(actors, Pixel(actor->state.x), Pixel(actor->state.y) - 7, &collider))
                return false;
            if (collider.effects & SOTN_EFFECT_SOLID)
                actor->vy = 0;
        }
        actor->state.x += actor->vx;
        actor->state.y += actor->vy;
        if (!ActorQuery(actors, Pixel(actor->state.x), Pixel(actor->state.y) + 7, &collider))
            return false;
        if ((collider.effects & SOTN_EFFECT_SOLID) && actor->vy > 0)
        {
            actor->vx = actor->vy = 0;
            ShiftY(actor, collider.top);
            actor->life_timer = 240;
            actor->state.step = 3;
        }
        else
            actor->vy = SDL_min(6 * 65536, actor->vy + 16384);
        return PrizeSides(actors, actor);
    }
    case 3:
    case 4:
        if (!PrizeGround(actors, actor, 1, true))
            return false;
        if (!(actor->state.parameters & 0x8000) && --actor->life_timer == 0)
        {
            if (actor->state.step == 3)
            {
                actor->state.step = 4;
                actor->life_timer = 80;
            }
            else
                actor->remove = true;
        }
        return true;
    case 5: {
        const bool body = id >= SOTN_HAND_ITEMS;
        const Uint16 item_id = body ? (Uint16)(id - SOTN_HAND_ITEMS) : id;
        SotnItem item = {0};
        char name[161] = {0};
        if (!SotnCharacter_Collect(actors->character, body, item_id) ||
            !Collectible(actors, actor) || !SotnItem_Read(actors->dra, body, item_id, &item) ||
            !SotnItem_Name(item, name, sizeof(name)) || !ActorSound(actors, 0x67c))
            return false;
        SDL_Log("Obtained %s", name);
        actor->remove = true;
        return true;
    }
    default:
        return SDL_SetError("Equipment drop entered an unsupported state");
    }
}

static bool PersistentPrize(SotnActors *actors, Actor *actor, SotnPlayerState player)
{
    if (actors->character == NULL)
    {
        actor->state.unsupported = true;
        return true;
    }
    const Uint16 index = actor->state.parameters;
    const SotnStageProfile *profile = SotnStageProfile_Get(actors->stage_id);
    const Uint16 count = profile->prize_count, base = profile->prize_bit;
    const Uint16 bit = (Uint16)(base + index);
    if (index >= count)
        return SDL_SetError("Persistent drop index exceeds its original stage table");
    SotnCharacterState state = {0};
    SotnDataView prize = {0};
    if (!SotnCharacter_Get(actors->character, &state, NULL))
        return false;
    if ((actors->world->flags[0x100 + bit / 8] & (1U << (bit & 7))) ||
        (actors->stage_id == 12 && (state.laboratory_items[index / 8] & (1U << (index & 7)))))
    {
        actor->remove = true;
        return true;
    }
    if (!Slice(actors->nz0, ActorTable(actors, 0x13b0, 0x1c8c, 0x1618) + index * 2U, 2, &prize))
        return false;
    const Uint16 id = Read16(prize.bytes);
    actor->collectible = (Uint16)(bit + 1);
    actor->state.id = (Uint16)(id < 128 ? 3 : 0xa);
    actor->state.parameters = (Uint16)((id < 128 ? id : id - 128) | 0x8000);
    return id < 128 ? Prize(actors, actor, player) : EquipmentPrize(actors, actor, player);
}

static bool RelicOrb(SotnActors *s, Actor *a)
{
    const Uint16 id = a->state.parameters & 0x7fff;
    SotnItem relic = {0};
    if (!SotnRelic_Read(s->dra, id, &relic))
        return false;
    if (a->state.step == 0)
    {
        if (s->world->relics[id] & 1)
        {
            a->remove = true;
            return true;
        }
        if (!ActorInitialize(s, a, ActorTable(s, 0xbd4, 0xab8, 0xa48)))
            return false;
        a->state.animation_set = 0;
        a->state.frame = 1;
        a->state.hitbox_state = 1;
        a->state.hitbox_width = a->state.hitbox_height = 7;
        a->state.priority = 0x7e;
        a->state.y = Pixel(a->state.y) * 65536 + 32768;
        a->vy = 16384;
        a->gravity = -512;
        a->timer = 64;
        return true;
    }
    if (a->state.step == 1)
    {
        a->vy += a->gravity;
        if (--a->timer == 0)
        {
            a->timer = 64;
            a->gravity = -a->gravity;
        }
        a->state.y += a->vy;
        return true;
    }
    if (a->state.step == 5)
    {
        char name[161] = {0};
        if (!SotnRelic_Collect(s->world, s->dra, id) || !SotnItem_Name(relic, name, sizeof(name)) ||
            !ActorSound(s, 0x687))
            return false;
        SDL_Log("Obtained relic: %s", name);
        a->remove = true;
        return true;
    }
    return SDL_SetError("Relic orb entered an unsupported state");
}

bool ActorExplosionTick(SotnActors *actors, Actor *actor)
{
    Uint8 animation = 0;
    const Uint16 kind = actor->state.parameters & 15;
    SotnDataView table = {0};
    if (kind > 4 ||
        !Slice(actors->nz0, ActorTable(actors, 0x1e54, 0x25a0, 0x1f2c) + (size_t)kind * 4, 4,
               &table))
        return SDL_SetError("ActorExplosion references an invalid runtime type");
    if (actor->state.step == 0)
    {
        if (!ActorInitialize(actors, actor, ActorTable(actors, 0xbe0, 0xac4, 0xa54)))
            return false;
        actor->state.animation_set = 2;
        actor->state.blend_mode = 0x30;
        if (actor->state.parameters & 0xf0)
        {
            actor->state.palette = 0x8195;
            actor->state.blend_mode = 0x10;
        }
        if (actor->state.parameters & 0xff00)
            actor->state.priority = (Uint16)(actor->state.parameters >> 8);
        actor->state.parameters = kind;
        return true;
    }
    if (!Slice(actors->nz0, ActorTable(actors, 0x1da8, 0x24f4, 0x1e80) + (size_t)kind * 4, 4,
               &table))
        return false;
    const Uint32 bits = Read32(table.bytes);
    const Sint32 velocity =
        bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 0x100000000LL);
    actor->state.y += velocity;
    if (!Slice(actors->nz0, ActorTable(actors, 0x1e54, 0x25a0, 0x1f2c) + (size_t)kind * 4, 4,
               &table))
        return false;
    if (Read32(table.bytes) < 0x80180000)
        return SDL_SetError("Explosion animation address is invalid");
    if (!ActorAnimate(actors, actor, Read32(table.bytes) - 0x80180000, &animation))
        return false;
    if (animation == 0)
        actor->remove = true;
    return true;
}

static bool Behavior(SotnActors *actors, Actor *actor, SotnPlayerState player)
{
    Uint8 animation = 0;
    if ((actors->stage_id == 65 || actors->stage_id == 7) && actor->state.id != 1 &&
        actor->state.id != 2 && actor->state.id != 3 && actor->state.id != 0xa &&
        actor->state.id != 0xb && actor->state.id != 0xc)
        return ActorEntrance(actors, actor);
    if ((actors->stage_id == 0 || actors->stage_id == 1) && actor->state.id > 0xc)
        return ActorEarlyCastle(actors, actor);
    switch (actor->state.id)
    {
    case 2:
        return ActorExplosionTick(actors, actor);
    case 3:
        return Prize(actors, actor, player);
    case 0xa:
        return EquipmentPrize(actors, actor, player);
    case 0xb:
        return RelicOrb(actors, actor);
    case 0xc:
        return PersistentPrize(actors, actor, player);
    case 1: {
        const Uint32 type = actor->state.parameters >> 12;
        SotnDataView table = {0};
        if (type >= (actors->stage_id == 0    ? 9U
                     : actors->stage_id == 1  ? 10U
                     : actors->stage_id != 12 ? 2U
                                              : 3U) ||
            !Slice(actors->nz0, ActorTable(actors, 0xe04, 0xe50, 0xe28) + type * 4, 4, &table))
            return SDL_SetError("Breakable actor references an unsupported original subtype");
        if (actor->state.step != 0)
        {
            const Uint32 address = Read32(table.bytes);
            if (address < 0x80180000)
                return SDL_SetError("Breakable actor animation address is invalid");
            if (!ActorAnimate(actors, actor, address - 0x80180000, &animation))
                return false;
            if (actor->hit_params)
            {
                if (actors->stage_id == 1 && (type == 7 || type == 8))
                {
                    Actor *drop = NULL;
                    if (!ActorSound(actors, 0x61d) ||
                        !ActorSpawn(actors, actor, 0xc,
                                    type == 8 ? 3 : (Uint16)(actor->state.parameters & 0x1ff), 96,
                                    128, &drop))
                        return false;
                    actor->remove = actor->permanent = true;
                    return true;
                }
                const Uint16 sound = actors->stage_id == 0 ? (type == 1 ? 0x61b : 0x67e)
                                     : actors->stage_id == 1 && type == 9 ? 0x61d
                                                                          : 0x634;
                if (!(actors->world->relics[SOTN_RELIC_CUBE] & 2))
                {
                    actor->remove = true;
                    return ActorExplosion(actors, actor, 0) && ActorSound(actors, sound);
                }
                if (!ActorExplosion(actors, actor, 0) || !ActorSound(actors, sound))
                    return false;
                actor->state.id = 3;
                actor->state.parameters &= 0xfff;
                if (actor->state.parameters >= 128)
                {
                    actor->state.id = 0xa;
                    actor->state.parameters -= 128;
                }
                actor->state.step = actor->pose = actor->remaining = 0;
                actor->state.frame = 0;
                actor->dynamic = actor->permanent = true;
                actor->timer = 16;
            }
            return true;
        }
        if (!ActorInitialize(actors, actor, ActorTable(actors, 0xbc8, 0xaac, 0xa3c)) ||
            !Slice(actors->nz0, ActorTable(actors, 0xe34, 0xe80, 0xe58) + type * 2, 2, &table))
            return false;
        actor->state.animation_set = Read16(table.bytes);
        if (!Slice(actors->nz0, ActorTable(actors, 0xe44, 0xe90, 0xe68) + type, 1, &table))
            return false;
        actor->state.blend_mode = table.bytes[0];
        if (!Slice(actors->nz0, ActorTable(actors, 0xe24, 0xe70, 0xe48) + type, 1, &table))
            return false;
        actor->state.hitbox_height = table.bytes[0];
        actor->state.priority = 0x6c;
        if (actors->stage_id == 1)
        {
            SotnDataView palette = {0}, texture = {0}, y = {0};
            if (!Slice(actors->nz0, 0xe18 + type * 2, 2, &palette) ||
                !Slice(actors->nz0, 0xe40 + type * 2, 2, &texture) ||
                !Slice(actors->nz0, 0xe60 + type * 2, 2, &y))
                return false;
            actor->state.palette = Read16(palette.bytes);
            actor->state.texture_offset = Read16(texture.bytes);
            actor->state.hitbox_y = (Sint16)Signed16(y.bytes);
        }
        else if (type == 2 && actors->stage_id == 12)
        {
            actor->state.texture_offset = 0x4b;
            actor->state.palette = 0x219;
        }
        return true;
    }
    case 0x23:
        if (actor->state.step == 0)
        {
            if (!ActorInitialize(actors, actor, 0xc34))
                return false;
            actor->state.frame = 7;
            actor->state.priority = 0x70;
        }
        return true;
    case 0x34:
        if (actor->state.step == 0)
        {
            if (!ActorInitialize(actors, actor, 0xcc4))
                return false;
            actor->state.priority = 0x6a;
            if (actors->character != NULL)
            {
                actor->state.hitbox_width = 8;
                actor->state.hitbox_height = 12;
                actor->state.hitbox_y = -10;
                actor->state.hitbox_state = 2;
            }
        }
        if (actors->character != NULL && actor->state.step == 1 && actor->hit_params != 0)
        {
            SotnDataView index = {0};
            if (actor->state.parameters >= 5 ||
                !Slice(actors->nz0, 0xf10 + actor->state.parameters * 2U, 2, &index) ||
                !ActorSound(actors, 0x635))
                return false;
            /* next is a copied actor during Tick, so locate its stable generation. */
            size_t parent = SOTN_STAGE_ACTOR_SLOTS;
            for (size_t i = 0; i < SOTN_STAGE_ACTOR_SLOTS; ++i)
                if (actors->slots[i].state.generation == actor->state.generation)
                {
                    parent = i;
                    break;
                }
            if (parent + 1 >= SOTN_STAGE_ACTOR_SLOTS)
                return SDL_SetError("Globe table lacks its original reserved child slot");
            Actor *child = &actors->slots[parent + 1];
            if (child->state.generation != 0)
                return SDL_SetError("Globe table child slot is occupied");
            *child = (Actor){.state = {.x = actor->state.x,
                                       .y = actor->state.y,
                                       .id = 0xc,
                                       .parameters = Read16(index.bytes),
                                       .generation = 0x80000000U | ++actors->effect_serial},
                             .dynamic = true};
            actor->state.step = 2;
            actor->state.hitbox_state = 0;
            actor->pose = actor->remaining = 0;
        }
        return ActorAnimate(actors, actor, actor->state.step == 2 ? 0xef8 : 0xef0, &animation);
    case 0x4b:
    case 0x4c:
    case 0x4d:
        return ActorMaria(actors, actor);
    case 0x49:
        return ActorBossReward(actors, actor);
    case 0x38:
        return ActorBossPuff(actors, actor);
    case 0x3e:
    case 0x3f:
        return ActorBossEvent(actors, actor);
    case 0x40:
    case 0x41:
    case 0x42:
        return ActorSlogra(actors, actor);
    case 0x43:
    case 0x44:
    case 0x45:
    case 0x46:
        return ActorGaibon(actors, actor);
    case 0x12:
        return ActorCamera(actors, actor);
    case 5:
    case 0x4a:
        return ActorDoor(actors, actor);
    case 0x1c:
    case 0x1d:
    case 0x1e:
    case 0x1f:
        return ActorCannon(actors, actor);
    case 0x22:
    case 0x17:
    case 0x18:
    case 0x19:
    case 0x1a:
    case 0x1b:
    case 0x20:
    case 0x21:
    case 0x24:
    case 0x25:
    case 0x47:
    case 0x48:
        return ActorMechanism(actors, actor);
    case 0x2b:
        return ActorBloodyZombie(actors, actor, player);
    case 0x29:
    case 0x2a:
        return ActorKnight(actors, actor, player);
    case 0x31:
    case 0x32:
    case 0x33:
        return ActorSpittle(actors, actor, player);
    case 0x26:
    case 0x27:
    case 0x28:
    case 0x2e:
    case 0x2f:
    case 0x30:
        return ActorSkeleton(actors, actor, player);
    default:
        actor->state.unsupported = true;
        return true;
    }
}

bool SotnActors_Create(SotnDataView dra, SotnDataView nz0, SotnActors **out)
{
    return SotnActors_CreateStage(dra, nz0, 12, out);
}

bool SotnActors_CreateStage(SotnDataView dra, SotnDataView nz0, Uint16 stage_id, SotnActors **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (SotnStageProfile_Get(stage_id) == NULL)
        return SDL_SetError("Native actors do not support stage %u", stage_id);
    if (dra.bytes == NULL || dra.size < 0x9c || dra.size > 0x200000 || nz0.bytes == NULL ||
        nz0.size < 40 || nz0.size > 0x80000)
        return SDL_SetError("Laboratory actors require bounded private US DRA/NZ0 images");
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    if (actors == NULL)
        return false;
    actors->stage_id = stage_id;
    actors->dra = dra;
    actors->nz0 = nz0;
    actors->combat.hearts = 10;
    actors->combat.hearts_max = 50;
    if (!SotnRoomActors_Create(&actors->spawns))
    {
        SotnActors_Destroy(actors);
        return false;
    }
    actors->world = &actors->fallback_world;
    actors->fallback_world.relics[SOTN_RELIC_CUBE] = 3;
    actors->camera = &actors->fallback_camera;
    *out = actors;
    return true;
}

bool SotnActors_Enter(SotnActors *actors, const SotnActorLayout *layout, SotnCollisionMap map,
                      Sint32 camera_x, Sint32 camera_y)
{
    if (actors == NULL || map.width == 0 || map.height == 0 || map.width > 1024 ||
        map.height > 1024)
        return SDL_SetError("Laboratory actors require a bounded room collision map");
    SotnCollider probe = {0};
    if (!SotnCollision_Query(&map, 0, 0, &probe))
        return false;
    const size_t bytes = (size_t)map.width * map.height * 2;
    if (map.tiles.size < bytes || map.tiles.bytes == NULL)
        return SDL_SetError("Room tile data is truncated");
    Uint8 *tiles = SDL_malloc(bytes);
    if (tiles == NULL)
        return false;
    SDL_memcpy(tiles, map.tiles.bytes, bytes);
    if (!SotnRoomActors_Enter(actors->spawns, layout, camera_x, camera_y))
    {
        SDL_free(tiles);
        return false;
    }
    SDL_free(actors->mutable_tiles);
    actors->mutable_tiles = tiles;
    SDL_memset(&actors->death_event, 0, sizeof(actors->death_event));
    actors->map = map;
    if (actors->camera == &actors->fallback_camera)
    {
        // Synthetic collision fixtures may be smaller than a screen; their camera is unused.
        const Uint32 w = ((map.width + 15) / 16) * 256, h = ((map.height + 15) / 16) * 256;
        if (!SotnCamera_Enter(actors->camera, w, h, camera_x + 128, camera_y + 136))
            return false;
    }
    actors->map.tiles = (SotnDataView){tiles, bytes};
    actors->camera_x = camera_x;
    actors->camera_y = camera_y;
    actors->entered = true;
    SDL_memset(actors->slots, 0, sizeof(actors->slots));
    SDL_memset(actors->primitives, 0, sizeof(actors->primitives));
    return true;
}

bool SotnActors_Tick(SotnActors *actors, SotnPlayerState player, Sint32 camera_x, Sint32 camera_y)
{
    if (actors == NULL || !actors->entered || camera_x < 0 || camera_x > 16384 || camera_y < 0 ||
        camera_y > 16384)
        return SDL_SetError("Laboratory actor tick requires an entered room and bounded camera");
    const Sint64 min_position = -256LL * 65536;
    const Sint64 max_x = ((Sint64)actors->map.width * 16 + 256) * 65536;
    const Sint64 max_y = ((Sint64)actors->map.height * 16 + 256) * 65536;
    if (player.x < min_position || player.x > max_x || player.y < min_position || player.y > max_y)
        return SDL_SetError("Laboratory actors require a player within the bounded current room");
    if (actors->character != NULL)
    {
        SotnCharacterState state = {0};
        if (!SotnCharacter_Get(actors->character, &state, NULL))
            return false;
        actors->combat.hearts = state.hearts;
        actors->combat.hearts_max = state.hearts_max;
        actors->combat.subweapon = state.subweapon;
        actors->combat.gold = state.gold;
        actors->combat.kills = state.kills;
    }
    if (actors->control.timer)
        --actors->control.timer;
    actors->player = player;
    actors->motion = (SotnActorMotion){0};
    ++actors->ticks;
    actors->combat.music_change = false;
    actors->combat.voice_change = false;
    actors->combat.cd_command = 0;
    actors->combat.sound_count = 0;
    actors->combat.contact_damage = 0;
    if (player.attack.sound && !ActorSound(actors, player.attack.sound_id))
        return false;
    const Sint32 dx = camera_x - actors->camera_x, dy = camera_y - actors->camera_y;
    for (Uint32 i = 0; i < SOTN_STAGE_ACTOR_SLOTS; ++i)
    {
        Actor *actor = &actors->slots[i];
        if (actor->state.generation == 0 || actor->state.step == 0 || actor->state.unsupported)
            continue;
        const Sint64 scale = actor->parallax != 0                ? 65536 - actor->parallax * 256
                             : (actor->state.flags & 0x08000000) ? 0
                                                                 : 65536;
        const Sint64 x = actor->state.x + dx * scale, y = actor->state.y + dy * scale;
        if (x < -1024LL * 65536 || x > 17408LL * 65536 || y < -1024LL * 65536 ||
            y > 17408LL * 65536)
            return SDL_SetError("Laboratory actor left the bounded room region");
        actor->state.x = (Sint32)x;
        actor->state.y = (Sint32)y;
    }
    if (!SotnRoomActors_Scroll(actors->spawns, camera_x, camera_y))
        return false;
    actors->camera_x = camera_x;
    actors->camera_y = camera_y;
    for (Uint32 i = 0; i < SOTN_STAGE_ACTOR_SLOTS; ++i)
    {
        SotnActorPlacement placement = {0};
        Uint32 generation = 0;
        if (!SotnRoomActors_Get(actors->spawns, i, &placement, &generation))
            return false;
        Actor *actor = &actors->slots[i];
        if (generation == 0 && !actor->dynamic)
        {
            ActorPrimitivesClear(actors, actor);
            *actor = (Actor){0};
            continue;
        }
        if (generation != 0 && actor->state.generation != generation)
        {
            ActorPrimitivesClear(actors, actor);
            *actor = (Actor){0};
            actor->state.generation = generation;
            actor->state.id = placement.id;
            actor->state.parameters = placement.parameters;
            actor->state.x = (Sint32)placement.x * 65536;
            actor->state.y = (Sint32)placement.y * 65536;
            actor->parallax = placement.variant;
            actor->slot = (Uint16)i;
        }
        const Sint32 x = Pixel(actor->state.x) - camera_x, y = Pixel(actor->state.y) - camera_y;
        if (actor->state.step != 0)
        {
            const bool barely = (actor->state.flags & 0x40000000) != 0;
            const Sint32 near = barely ? -64 : -128, far_x = barely ? 320 : 384,
                         far_y = barely ? 288 : 352;
            if ((actor->state.flags & 0x80000000) &&
                (x < near || x > far_x || y < near || y > far_y))
            {
                if (!SotnRoomActors_Release(actors->spawns, i, false))
                    return false;
                ActorPrimitivesClear(actors, actor);
                *actor = (Actor){0};
                continue;
            }
            if ((actor->state.flags & 0x20000000) && !(actor->state.flags & 0x10000000) &&
                (x < -64 || x > 320 || y < -64 || y > 288))
                continue;
        }
    }
    /* The original engine runs HitDetection over the stage before Update. Linked
     * actors must all observe this tick's contacts before any behavior clears them. */
    for (Uint32 i = 0; i < SOTN_STAGE_ACTOR_SLOTS; ++i)
    {
        Actor *actor = &actors->slots[i];
        if (actor->state.generation != 0 && !Hit(actors, actor, actors->player))
            return false;
    }
    for (Uint32 i = 0; i < SOTN_STAGE_ACTOR_SLOTS; ++i)
    {
        Actor *actor = &actors->slots[i];
        if (actor->state.generation == 0)
            continue;
        const Sint32 x = Pixel(actor->state.x) - camera_x, y = Pixel(actor->state.y) - camera_y;
        if (actor->state.step != 0 && (actor->state.flags & 0x20000000) &&
            !(actor->state.flags & 0x10000000) && (x < -64 || x > 320 || y < -64 || y > 288))
            continue;
        Actor next = *actor;
        if (next.state.id == 3 && next.timer != 0)
            --next.timer;
        bool frozen = false;
        if (next.stun != 0)
        {
            --next.stun;
            frozen = !(next.state.flags & 0x100000);
        }
        if (SotnSubweapons_Frozen(actors->subweapons) && !(next.state.flags & (0x2000 | 0x100)))
            frozen = frozen || !(next.state.flags & 0x200) || (actors->ticks & 3) != 0;
        if (!frozen && !next.state.unsupported && !Behavior(actors, &next, actors->player))
        {
            char detail[256] = {0};
            SDL_strlcpy(detail, SDL_GetError(), sizeof(detail));
            return SDL_SetError("Laboratory actor 0x%02x slot %u step %u: %s", next.state.id, i,
                                next.state.step, detail);
        }
        if (!frozen)
        {
            next.hit_params = 0;
            next.hit_flags = 0;
        }
        if (next.permanent)
        {
            if (!SotnRoomActors_Release(actors->spawns, i, true))
                return false;
            next.permanent = false;
        }
        if (next.remove)
        {
            ActorPrimitivesClear(actors, &next);
            if (!SotnRoomActors_Release(actors->spawns, i, false))
                return false;
            next = (Actor){0};
        }
        if ((Sint64)next.state.x < -1024LL * 65536 || (Sint64)next.state.x > 17408LL * 65536 ||
            (Sint64)next.state.y < -1024LL * 65536 || (Sint64)next.state.y > 17408LL * 65536)
            return SDL_SetError("Laboratory actor motion left the bounded room region");
        *actor = next;
    }
    if (actors->character != NULL)
    {
        SotnCharacterState state = {0};
        if (!SotnCharacter_Get(actors->character, &state, NULL))
            return false;
        state.hearts = actors->combat.hearts;
        state.subweapon = actors->combat.subweapon;
        state.gold = actors->combat.gold;
        state.kills = actors->combat.kills;
        if (!SotnCharacter_Restore(actors->character, &state))
            return false;
    }
    return true;
}

bool SotnActors_Get(const SotnActors *actors, Uint32 slot, SotnActorState *out)
{
    if (actors == NULL || out == NULL || slot >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Laboratory actor slot or output is invalid");
    *out = actors->slots[slot].state;
    out->slot = (Uint16)slot;
    return true;
}

bool SotnActors_GetCombat(const SotnActors *actors, SotnActorCombat *out)
{
    if (actors == NULL || out == NULL || !actors->entered)
        return SDL_SetError("Actor combat state requires an entered room and output");
    *out = actors->combat;
    return true;
}

bool SotnActors_GetProgress(const SotnActors *actors, SotnActorProgress *out)
{
    if (actors == NULL || !actors->entered || out == NULL)
        return SDL_SetError("Actor progress requires an entered simulation and output");
    *out =
        (SotnActorProgress){actors->combat.hearts, actors->combat.subweapon, actors->combat.gold,
                            actors->combat.kills,  actors->random,           actors->combat_random};
    return true;
}

bool SotnActors_Restore(SotnActors *actors, SotnActorProgress progress)
{
    if (actors == NULL || !actors->entered)
        return SDL_SetError("Actor progress requires an entered stage");
    Uint16 maximum = 50;
    if (actors->character != NULL)
    {
        SotnCharacterState state = {0};
        if (!SotnCharacter_Get(actors->character, &state, NULL))
            return false;
        maximum = state.hearts_max;
    }
    if (progress.hearts > maximum || progress.subweapon > 9 || progress.gold > 999999)
        return SDL_SetError("Actor progress is outside the supported character bounds");
    actors->combat.hearts_max = maximum;
    actors->combat.hearts = progress.hearts;
    actors->combat.subweapon = progress.subweapon;
    actors->combat.gold = progress.gold;
    actors->combat.kills = progress.kills;
    actors->random = progress.random;
    actors->combat_random = progress.combat_random;
    return true;
}

bool SotnActors_SetCharacter(SotnActors *actors, SotnCharacter *character)
{
    if (actors == NULL)
        return SDL_InvalidParamError("actors");
    actors->character = character;
    return true;
}

void SotnActors_Destroy(SotnActors *actors)
{
    if (actors != NULL)
    {
        SotnRoomActors_Destroy(actors->spawns);
        SDL_free(actors->mutable_tiles);
        SDL_free(actors);
    }
}

bool SotnActors_SetSubweapons(SotnActors *actors, SotnSubweapons *service)
{
    if (actors == NULL)
        return SDL_InvalidParamError("actors");
    actors->subweapons = service;
    return true;
}

bool SotnActors_SetWorld(SotnActors *s, SotnWorldState *world)
{
    if (s == NULL)
        return SDL_InvalidParamError("actors");
    if (world != NULL && !SotnWorld_Validate(world))
        return false;
    s->world = world != NULL ? world : &s->fallback_world;
    return true;
}
bool SotnActors_SetButtons(SotnActors *s, Uint16 buttons)
{
    if (s == NULL)
        return SDL_InvalidParamError("actors");
    s->buttons = buttons;
    return true;
}
bool SotnActors_GetMap(const SotnActors *s, SotnCollisionMap *out)
{
    if (s == NULL || !s->entered || out == NULL)
        return SDL_SetError("Room map requires an entered actor service and output");
    *out = s->map;
    return true;
}
bool SotnActors_GetMotion(const SotnActors *s, SotnActorMotion *out)
{
    if (s == NULL || !s->entered || out == NULL)
        return SDL_SetError("Object motion requires an entered actor service and output");
    *out = s->motion;
    return true;
}

bool SotnActors_GetControl(const SotnActors *s, SotnActorControl *out)
{
    if (s == NULL || out == NULL)
        return SDL_SetError("Stage control requires actors and output");
    *out = s->control;
    return true;
}

bool SotnActors_SetCamera(SotnActors *s, SotnCamera *camera)
{
    if (s == NULL)
        return SDL_InvalidParamError("actors");
    s->camera = camera != NULL ? camera : &s->fallback_camera;
    return true;
}

bool SotnActors_SetServices(SotnActors *s, SotnDataView executable, Uint64 ticks,
                            bool music_playing)
{
    if (s == NULL || executable.bytes == NULL || executable.size < 0x22a10 + 2050 ||
        ticks > 99999ULL * 3600 * 60)
        return SDL_SetError(
            "Stage services require private SDK tables and bounded simulation ticks");
    s->executable = executable;
    s->game_seconds = (Uint32)(ticks / 60);
    s->game_frame = (Uint8)(ticks % 60);
    s->music_playing = music_playing;
    return true;
}
