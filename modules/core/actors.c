// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted selected Laboratory behaviors and hit/drop paths from sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5: src/st/nz0/e_breakable.c,
// e_red_eye_bust.c, e_nz0_room2.c; src/st/e_bloody_zombie.h,
// animate_entity.h, st_common.h, st_update.h, collision.h, e_collect.h,
// and collect_subweapon.h. No game tables are embedded.
#include <SDL3/SDL_error.h>
#include <sotn/actors.h>
#include <sotn/combat.h>

typedef struct Actor
{
    SotnActorState state;
    Sint32 vx, vy;
    Uint16 pose, remaining, timer, substep;
    Uint8 parallax, cooldown[2];
    Uint16 attack, attack_element, stun, hit_params, life_timer, level, experience;
    SotnCombatEnemy enemy;
    Sint32 fall_speed, gravity;
    Uint16 collectible;
    bool dynamic, remove, permanent;
} Actor;

struct SotnActors
{
    SotnDataView dra, nz0;
    SotnCollisionMap map;
    SotnRoomActors *spawns;
    Actor slots[SOTN_STAGE_ACTOR_SLOTS];
    Sint32 camera_x, camera_y;
    Uint32 random, combat_random, ticks, effect_serial;
    SotnActorCombat combat;
    bool entered;
    SotnCharacter *character;
};

static Uint16 Read16(const Uint8 *bytes)
{
    return (Uint16)(bytes[0] | (Uint16)bytes[1] << 8);
}

static Uint32 Read32(const Uint8 *bytes)
{
    return bytes[0] | (Uint32)bytes[1] << 8 | (Uint32)bytes[2] << 16 | (Uint32)bytes[3] << 24;
}

static Sint32 Signed16(const Uint8 *bytes)
{
    const Uint16 value = Read16(bytes);
    return value < 32768 ? value : (Sint32)value - 65536;
}

static Sint32 Pixel(Sint32 value)
{
    return value >= 0 ? value / 65536 : (Sint32)(-((-(Sint64)value + 65535) / 65536));
}

static Sint32 Wrap16(Sint64 value)
{
    const Uint16 bits = (Uint16)(Uint64)value;
    return bits < 32768 ? bits : (Sint32)bits - 65536;
}

static void ShiftY(Actor *actor, Sint64 delta)
{
    const Sint32 high = Wrap16((Sint64)Pixel(actor->state.y) + delta);
    actor->state.y = (Sint32)((Sint64)high * 65536 + ((Uint32)actor->state.y & 65535));
}

static bool Slice(SotnDataView image, size_t offset, size_t size, SotnDataView *out)
{
    if (image.bytes == NULL || offset > image.size || size > image.size - offset)
        return SDL_SetError("Laboratory actor table is outside its private image");
    *out = (SotnDataView){image.bytes + offset, size};
    return true;
}

static void SetStep(Actor *actor, Uint16 step)
{
    actor->state.step = step;
    actor->substep = actor->pose = actor->remaining = 0;
}

static bool Initialize(SotnActors *actors, Actor *actor, size_t offset)
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
    actor->state.priority = 0x74;
    ++actor->state.step;
    actor->substep = 0;
    return true;
}

static bool Animate(SotnActors *actors, Actor *actor, size_t offset, Uint8 *result)
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

static bool Query(SotnActors *actors, Sint32 x, Sint32 y, SotnCollider *out)
{
    return SotnCollision_Query(&actors->map, x, y, out);
}

static bool Ground(SotnActors *actors, Actor *actor, bool *landed)
{
    SotnDataView sensors = {0};
    if (!Slice(actors->nz0, 0x22d4, 16, &sensors))
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
            if (!Query(actors, x, y, &col))
                return false;
            if ((col.effects & SOTN_EFFECT_SLOPE_FLOOR) && i == 1)
            {
                if (col.effects & SOTN_EFFECT_SOLID)
                {
                    if (!Query(actors, x, y - 8, &back))
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
                if (!Query(actors, x, y - 8, &back))
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
    if (!Slice(actors->nz0, 0x22e4, 8, &sensors))
        return false;
    actor->state.x += actor->vx;
    ShiftY(actor, 3);
    Sint32 x = Wrap16((Sint64)Pixel(actor->state.x) + Signed16(sensors.bytes));
    Sint32 y = Wrap16((Sint64)Pixel(actor->state.y) + Signed16(sensors.bytes + 2));
    SotnCollider col = {0};
    if (!Query(actors, x, y, &col))
        return false;
    if (!(col.effects & SOTN_EFFECT_SOLID))
        return true;
    ShiftY(actor, col.top);
    if (actor->vx == 0)
        return true;
    const Sint32 sensor_x = Signed16(sensors.bytes + 4);
    x = Wrap16((Sint64)x + (actor->vx < 0 ? -sensor_x : sensor_x));
    y = Wrap16((Sint64)y + Signed16(sensors.bytes + 6) - 7);
    if (!Query(actors, x, y, &col))
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
    if (!Query(actors, x, y, &col))
        return false;
    if (!(col.effects & SOTN_EFFECT_SOLID))
    {
        actor->state.x -= actor->vx;
        actor->vx = 0;
    }
    return true;
}

static Uint8 Side(const Actor *actor, SotnPlayerState player)
{
    return (Uint8)(Pixel(actor->state.x) > Pixel(player.x));
}

static Sint64 Distance(const Actor *actor, SotnPlayerState player)
{
    const Sint64 difference = (Sint64)Pixel(actor->state.x) - Pixel(player.x);
    return difference < 0 ? -difference : difference;
}

static bool Sound(SotnActors *actors, Uint16 id)
{
    if (actors->combat.sound_count >= SDL_arraysize(actors->combat.sounds))
        return SDL_SetError("Laboratory sound request budget exceeded");
    actors->combat.sounds[actors->combat.sound_count++] = id;
    return true;
}

static bool Explosion(SotnActors *actors, const Actor *source, Uint16 kind)
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
        if (kind == 2)
        {
            effect->state.y += 16 * 65536;
            effect->state.x += source->state.facing_left ? -8 * 65536 : 8 * 65536;
        }
        break;
    }
    return true;
}

static bool Zombie(SotnActors *actors, Actor *actor, SotnPlayerState player)
{
    Uint8 animation = 0;
    if (actor->hit_params && (actor->state.step & 1))
    {
        if (!Sound(actors, 0x746) || !Sound(actors, 0x748))
            return false;
        SetStep(actor, 6);
    }
    if ((actor->state.flags & 0x100) && actor->state.step < 8)
    {
        if (!Sound(actors, 0x747))
            return false;
        actor->state.hitbox_state = 0;
        actor->state.flags &= ~0x20000000U;
        SetStep(actor, 8);
    }
    switch (actor->state.step)
    {
    case 0:
        if (!Initialize(actors, actor, 0xc7c))
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
        if (!Animate(actors, actor, 0x22ec, &animation) || !WalkCollision(actors, actor))
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
        if (!Animate(actors, actor, 0x237c, &animation))
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
        if (!Animate(actors, actor, 0x22fc, &animation))
            return false;
        if ((animation & 0x80) && actor->pose == 10 && !Sound(actors, 0x60b))
            return false;
        if (animation == 0)
            SetStep(actor, 1);
        break;
    case 6:
        if (!Animate(actors, actor, 0x2320, &animation))
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
            if ((actors->ticks & 7) == 0 && !Sound(actors, 0x749))
                return false;
            actor->timer = 0;
        }
        else if (actor->timer++ == 0 && !Sound(actors, 0x749))
            return false;
        if (!Animate(actors, actor, 0x2334, &animation))
            return false;
        if (animation == 0)
        {
            actor->timer = 64;
            actor->state.frame = 0;
            ++actor->state.step;
            if (!Explosion(actors, actor, 2) || !Sound(actors, 0x693))
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

static bool Hit(SotnActors *actors, Actor *actor, SotnPlayerState player)
{
    if (player.step == SOTN_PLAYER_KILL || actor->state.step == 0 || actor->state.unsupported ||
        (actor->state.flags & 0x100))
        return true;
    for (Uint32 i = 0; i < 2; ++i)
        if (actor->cooldown[i])
            --actor->cooldown[i];
    if (actor->state.id != 1 && actor->state.id != 0x2b && actor->state.id != 3 &&
        actor->state.id != 0xa && actor->state.id != 0x34)
        return true;
    const SotnCombatBox target = {Pixel(actor->state.x) + (actor->state.facing_left
                                                               ? -actor->state.hitbox_x
                                                               : actor->state.hitbox_x),
                                  Pixel(actor->state.y) + actor->state.hitbox_y,
                                  actor->state.hitbox_width, actor->state.hitbox_height};
    const Sint32 screen_x = target.x - actors->camera_x, screen_y = target.y - actors->camera_y;
    if (screen_x <= -32 || screen_x >= 288 || screen_y <= -32 || screen_y >= 256)
        return true;
    const Uint32 hand = player.attack.right_hand ? 1 : 0;
    const SotnCombatBox attacker = PlayerBox(player, true);
    if (player.attack.active && actor->cooldown[hand] == 0 &&
        (actor->state.hitbox_state & player.attack.hit_type & 0x3e) &&
        SotnCombat_Overlap(attacker, target))
    {
        Uint16 stats[4] = {6, 6, 6, 6};
        Uint16 luck = 6;
        if (actors->character != NULL)
        {
            SotnCharacterStats attributes = {0};
            SotnCharacterState progress = {0};
            if (!SotnCharacter_Get(actors->character, &progress, &attributes))
                return false;
            SDL_memcpy(stats, progress.base, sizeof(stats));
            luck = attributes.total[3];
        }
        Uint16 damage = 0;
        if (actor->state.hit_points != 0x7fff && actor->state.hit_points != 0 &&
            !SotnCombat_DamageAttributes(actor->enemy, player.attack.power, player.attack.element,
                                         player.attack.critical_rate, stats, luck, 1,
                                         &actors->combat_random, &damage))
            return false;
        actor->hit_params = player.attack.hit_effect & 127;
        actor->cooldown[hand] = player.attack.invincibility_frames;
        if (actor->state.id == 0x2b && damage != 0xc000)
        {
            if (damage & 0x8000)
                actor->state.hit_points =
                    (Sint16)SDL_min(32767, actor->state.hit_points + (damage & 0x3fff));
            else
            {
                actor->state.hit_points =
                    (Sint16)SDL_max(-32768, actor->state.hit_points - (damage & 0x3fff));
                if (actor->state.hit_points <= 0)
                {
                    actor->state.flags |= 0x100;
                    ++actors->combat.kills;
                    if (actors->character != NULL)
                    {
                        Uint16 levels = 0;
                        if (!SotnCharacter_GainExperience(actors->character, actor->experience,
                                                          actor->level, &levels))
                            return false;
                        if (levels != 0)
                        {
                            SotnCharacterState progress = {0};
                            if (!SotnCharacter_Get(actors->character, &progress, NULL))
                                return false;
                            actors->combat.hearts_max = progress.hearts_max;
                            if (!Sound(actors, 0x687))
                                return false;
                            SDL_Log("Level up: %u (HP %u, MP %u, hearts %u)", progress.level,
                                    progress.hp_max, progress.mp_max, progress.hearts_max);
                        }
                    }
                }
            }
            if (!(actor->state.flags & (0x400000 | 0x100)))
                actor->stun = player.attack.stun_frames;
        }
        return true;
    }
    if ((actor->state.hitbox_state & 1) &&
        (actor->state.id == 3 || actor->state.id == 0xa || !player.invulnerable) &&
        SotnCombat_Overlap(PlayerBox(player, false), target))
    {
        if ((actor->state.id == 3 || actor->state.id == 0xa) && actor->state.step > 1 &&
            actor->state.step < 5 && actor->timer == 0)
            actor->state.step = 5;
        else if (actor->state.id == 0x2b && actor->attack > actors->combat.contact_damage)
        {
            actors->combat.contact_damage = actor->attack;
            actors->combat.contact_x = target.x;
            actors->combat.contact_velocity = actor->vx;
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
    if (!Slice(actors->nz0, 0x1d9c, 8, &sensors))
        return false;
    Sint32 x = Pixel(actor->state.x), y = Pixel(actor->state.y);
    for (Uint32 i = 0; i < 2; ++i)
    {
        const Sint32 dx = Signed16(sensors.bytes + i * 4);
        x = Wrap16((Sint64)x + (actor->vx < 0 ? dx : -dx));
        y = Wrap16((Sint64)y + Signed16(sensors.bytes + i * 4 + 2));
        SotnCollider col = {0};
        if (!Query(actors, x, y, &col))
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
        if (!Query(actors, Pixel(actor->state.x), Pixel(actor->state.y) - 7, &col))
            return false;
        if (col.effects & (SOTN_EFFECT_SOLID | SOTN_EFFECT_SINK))
            actor->vy = 0;
    }
    if (!resting)
    {
        actor->state.x += actor->vx;
        actor->state.y += actor->vy;
    }
    if (!Query(actors, Pixel(actor->state.x), Pixel(actor->state.y) + 7, &col))
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
        if (!Slice(actors->nz0, 0x1d3c + (size_t)item * 4, 4, &pointer) ||
            Read32(pointer.bytes) < 0x80180000 ||
            !Animate(actors, actor, Read32(pointer.bytes) - 0x80180000, &result))
            return false;
    }
    switch (actor->state.step)
    {
    case 0:
        if (!Initialize(actors, actor, 0xbd4))
            return false;
        actor->state.priority = 0x6c;
        actor->state.blend_mode = 0;
        if (item >= 14 && item < 23)
        {
            SotnDataView old = {0};
            if (actors->combat.subweapon > 9 ||
                !Slice(actors->nz0, 0x1cd8 + actors->combat.subweapon * 2U, 2, &old))
                return SDL_SetError("Development subweapon ID is invalid");
            if (item == Read16(old.bytes))
                item = actor->state.parameters = 1;
        }
        if (item == 0 || item == 2)
            actor->state.hitbox_width = 4;
        return true;
    case 1: {
        SotnCollider col = {0};
        if (!Query(actors, Pixel(actor->state.x), Pixel(actor->state.y), &col))
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
            if (!Slice(actors->nz0, 0x1da4 + item, 1, &prize) || !Sound(actors, 0x67a))
                return false;
            actors->combat.hearts =
                (Uint16)SDL_min(actors->combat.hearts_max, actors->combat.hearts + prize.bytes[0]);
        }
        else if (item < 12)
        {
            if (!Slice(actors->nz0, 0x1d14 + (size_t)(item - 2) * 4, 4, &prize) ||
                !Sound(actors, 0x6a9))
                return false;
            const Uint32 amount = Read32(prize.bytes);
            if (amount > 999999)
                return SDL_SetError("Gold prize exceeds the original wallet limit");
            actors->combat.gold = SDL_min(999999U, actors->combat.gold + amount);
        }
        else if (item == 12 || item == 23)
        {
            if (!SotnCharacter_CollectVessel(actors->character, item == 23) ||
                !Sound(actors, 0x67a))
                return false;
            SotnCharacterState profile = {0};
            if (!SotnCharacter_Get(actors->character, &profile, NULL))
                return false;
            actors->combat.hearts = profile.hearts;
            actors->combat.hearts_max = profile.hearts_max;
        }
        else
        {
            if (!Slice(actors->nz0, 0x1cc4 + (size_t)(item - 14) * 2, 2, &prize) ||
                !Sound(actors, 0x67c))
                return false;
            const Uint16 previous = actors->combat.subweapon;
            actors->combat.subweapon = Read16(prize.bytes);
            if (previous != 0)
            {
                if (previous > 9 || !Slice(actors->nz0, 0x1cd8 + previous * 2U, 2, &prize))
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
    if (index >= 11)
        return SDL_SetError("Laboratory item persistence index exceeds its original table");
    state.laboratory_items[index / 8] |= (Uint8)(1U << (index & 7));
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
        if (!Initialize(actors, actor, 0xbd4))
            return false;
        actor->state.priority = 0x80;
        actor->state.animation_set = 0;
        actor->state.frame = 0;
        actor->state.blend_mode = 0;
        return true;
    case 1: {
        SotnCollider collider = {0};
        if (!Query(actors, Pixel(actor->state.x), Pixel(actor->state.y), &collider))
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
            if (!Query(actors, Pixel(actor->state.x), Pixel(actor->state.y) - 7, &collider))
                return false;
            if (collider.effects & SOTN_EFFECT_SOLID)
                actor->vy = 0;
        }
        actor->state.x += actor->vx;
        actor->state.y += actor->vy;
        if (!Query(actors, Pixel(actor->state.x), Pixel(actor->state.y) + 7, &collider))
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
            !SotnItem_Name(item, name, sizeof(name)) || !Sound(actors, 0x67d))
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
    if (index >= 11)
        return SDL_SetError("Persistent Laboratory drop index exceeds its original table");
    SotnCharacterState state = {0};
    SotnDataView prize = {0};
    if (!SotnCharacter_Get(actors->character, &state, NULL))
        return false;
    if (state.laboratory_items[index / 8] & (1U << (index & 7)))
    {
        actor->remove = true;
        return true;
    }
    if (!Slice(actors->nz0, 0x13b0 + (size_t)index * 2, 2, &prize))
        return false;
    const Uint16 id = Read16(prize.bytes);
    actor->collectible = (Uint16)(index + 1);
    actor->state.id = (Uint16)(id < 128 ? 3 : 0xa);
    actor->state.parameters = (Uint16)((id < 128 ? id : id - 128) | 0x8000);
    return id < 128 ? Prize(actors, actor, player) : EquipmentPrize(actors, actor, player);
}

static bool Behavior(SotnActors *actors, Actor *actor, SotnPlayerState player)
{
    Uint8 animation = 0;
    switch (actor->state.id)
    {
    case 2: {
        const Uint16 kind = actor->state.parameters;
        SotnDataView table = {0};
        if (kind > 4 || !Slice(actors->nz0, 0x1e54 + (size_t)kind * 4, 4, &table))
            return SDL_SetError("Explosion references an invalid runtime type");
        if (actor->state.step == 0)
        {
            if (!Initialize(actors, actor, 0xbe0))
                return false;
            actor->state.animation_set = 2;
            actor->state.blend_mode = 0x30;
            return true;
        }
        if (!Slice(actors->nz0, 0x1da8 + (size_t)kind * 4, 4, &table))
            return false;
        const Uint32 bits = Read32(table.bytes);
        const Sint32 velocity =
            bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 0x100000000LL);
        actor->state.y += velocity;
        if (!Slice(actors->nz0, 0x1e54 + (size_t)kind * 4, 4, &table) ||
            Read32(table.bytes) < 0x80180000 ||
            !Animate(actors, actor, Read32(table.bytes) - 0x80180000, &animation))
            return false;
        if (animation == 0)
            actor->remove = true;
        return true;
    }
    case 3:
        return Prize(actors, actor, player);
    case 0xa:
        return EquipmentPrize(actors, actor, player);
    case 0xc:
        return PersistentPrize(actors, actor, player);
    case 1: {
        const Uint32 type = actor->state.parameters >> 12;
        SotnDataView table = {0};
        if (type >= 3 || !Slice(actors->nz0, 0xe04 + type * 4, 4, &table))
            return SDL_SetError("Breakable actor references an unsupported original subtype");
        if (actor->state.step != 0)
        {
            const Uint32 address = Read32(table.bytes);
            if (address < 0x80180000)
                return SDL_SetError("Breakable actor animation address is invalid");
            if (!Animate(actors, actor, address - 0x80180000, &animation))
                return false;
            if (actor->hit_params)
            {
                if (!Explosion(actors, actor, 0) || !Sound(actors, 0x634))
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
        if (!Initialize(actors, actor, 0xbc8) || !Slice(actors->nz0, 0xe34 + type * 2, 2, &table))
            return false;
        actor->state.animation_set = Read16(table.bytes);
        if (!Slice(actors->nz0, 0xe44 + type, 1, &table))
            return false;
        actor->state.blend_mode = table.bytes[0];
        if (!Slice(actors->nz0, 0xe24 + type, 1, &table))
            return false;
        actor->state.hitbox_height = table.bytes[0];
        actor->state.priority = 0x6c;
        if (type == 2)
        {
            actor->state.texture_offset = 0x4b;
            actor->state.palette = 0x219;
        }
        return true;
    }
    case 0x23:
        if (actor->state.step == 0)
        {
            if (!Initialize(actors, actor, 0xc34))
                return false;
            actor->state.frame = 7;
            actor->state.priority = 0x70;
        }
        return true;
    case 0x34:
        if (actor->state.step == 0)
        {
            if (!Initialize(actors, actor, 0xcc4))
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
                !Sound(actors, 0x635))
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
        return Animate(actors, actor, actor->state.step == 2 ? 0xef8 : 0xef0, &animation);
    case 0x2b:
        return Zombie(actors, actor, player);
    default:
        actor->state.unsupported = true;
        return true;
    }
}

bool SotnActors_Create(SotnDataView dra, SotnDataView nz0, SotnActors **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (dra.bytes == NULL || dra.size < 0x9c || dra.size > 0x200000 || nz0.bytes == NULL ||
        nz0.size < 40 || nz0.size > 0x80000)
        return SDL_SetError("Laboratory actors require bounded private US DRA/NZ0 images");
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    if (actors == NULL)
        return false;
    actors->dra = dra;
    actors->nz0 = nz0;
    actors->combat.hearts = 10;
    actors->combat.hearts_max = 50;
    if (!SotnRoomActors_Create(&actors->spawns))
    {
        SotnActors_Destroy(actors);
        return false;
    }
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
    if (!SotnCollision_Query(&map, 0, 0, &probe) ||
        !SotnRoomActors_Enter(actors->spawns, layout, camera_x, camera_y))
        return false;
    actors->map = map;
    actors->camera_x = camera_x;
    actors->camera_y = camera_y;
    actors->entered = true;
    SDL_memset(actors->slots, 0, sizeof(actors->slots));
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
    ++actors->ticks;
    actors->combat.sound_count = 0;
    actors->combat.contact_damage = 0;
    if (player.attack.sound && !Sound(actors, player.attack.sound_id))
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
            *actor = (Actor){0};
            continue;
        }
        if (generation != 0 && actor->state.generation != generation)
        {
            *actor = (Actor){0};
            actor->state.generation = generation;
            actor->state.id = placement.id;
            actor->state.parameters = placement.parameters;
            actor->state.x = (Sint32)placement.x * 65536;
            actor->state.y = (Sint32)placement.y * 65536;
            actor->parallax = placement.variant;
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
                *actor = (Actor){0};
                continue;
            }
            if ((actor->state.flags & 0x20000000) && !(actor->state.flags & 0x10000000) &&
                (x < -64 || x > 320 || y < -64 || y > 288))
                continue;
        }
        Actor next = *actor;
        if (!Hit(actors, &next, player))
            return false;
        if (next.state.id == 3 && next.timer != 0)
            --next.timer;
        bool frozen = false;
        if (next.stun != 0)
        {
            --next.stun;
            frozen = !(next.state.flags & 0x100000);
        }
        if (!frozen && !next.state.unsupported && !Behavior(actors, &next, player))
            return false;
        if (!frozen)
            next.hit_params = 0;
        if (next.permanent)
        {
            if (!SotnRoomActors_Release(actors->spawns, i, true))
                return false;
            next.permanent = false;
        }
        if (next.remove)
        {
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
    if (actors == NULL || !actors->entered || progress.hearts > 50 || progress.subweapon > 9 ||
        progress.gold > 999999)
        return SDL_SetError("Actor progress is outside the supported development profile");
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
        SDL_free(actors);
    }
}
