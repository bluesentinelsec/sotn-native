// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted normal-form movement paths from sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5:
// src/dra/6DF70.c, 6E42C.c, 6FDF8.c, 71830.c, 692E8.c,
// and selected unarmed paths in 7879C.c and alu_anim.c.
#include "player_internal.h"
#include <sotn/combat.h>
#include <sotn/subweapon.h>

enum
{
    FIX_ONE = 65536,
    WALK_SPEED = 98304,
    GRAVITY = 11264,
    JUMP_SPEED = -319488
};

_Static_assert(sizeof(PlayerFixed) == 4,
               "Player coordinates require an exact 32-bit representation");

static bool ValidateMap(SotnCollisionMap map)
{
    if (map.width == 0 || map.height == 0 || map.width > 1024 || map.height > 1024 ||
        map.tiles.bytes == NULL || map.types.bytes == NULL || map.types.size == 0 ||
        map.width > (map.tiles.size / 2) / map.height)
        return SDL_SetError("Player requires a bounded original room collision map");
    const size_t count = (size_t)map.width * map.height;
    for (size_t i = 0; i < count; ++i)
    {
        const Uint16 id =
            (Uint16)(map.tiles.bytes[i * 2] | (Uint16)map.tiles.bytes[i * 2 + 1] << 8);
        if (id >= map.types.size)
            return SDL_SetError("Player room references a missing collision definition");
    }
    return true;
}

static bool PositionValid(SotnCollisionMap map, Sint64 x, Sint64 y)
{
    const Sint64 min = -256LL * FIX_ONE;
    const Sint64 max_x = ((Sint64)map.width * 16 + 256) * FIX_ONE;
    const Sint64 max_y = ((Sint64)map.height * 16 + 256) * FIX_ONE;
    return x >= min && x <= max_x && y >= min && y <= max_y;
}

static void SetStep(SotnPlayer *player, SotnPlayerStep step)
{
    player->step = step;
    player->substep = 0;
}

static void Animate(SotnPlayer *player, Uint16 id)
{
    if (!SotnAnimation_Set(&player->animation, id))
        player->failed = true;
}

static void SetSpeed(SotnPlayer *player, Sint32 speed)
{
    player->vx = player->facing_left ? -speed : speed;
}

static void Decelerate(SotnPlayer *player, Sint32 amount)
{
    if (player->vx < 0)
        player->vx = SDL_min(0, player->vx + amount);
    else
        player->vx = SDL_max(0, player->vx - amount);
}

static int Direction(SotnPlayer *player)
{
    if (player->movement_flags & 2)
        return 0;
    if (player->buttons & SOTN_BUTTON_RIGHT)
    {
        if (player->facing_left)
        {
            player->facing_left = false;
            player->turned = true;
            return -1;
        }
        return 1;
    }
    if (player->buttons & SOTN_BUTTON_LEFT)
    {
        if (!player->facing_left)
        {
            player->facing_left = true;
            player->turned = true;
            return -1;
        }
        return 1;
    }
    return 0;
}

void SotnPlayer_FormDirection(SotnPlayer *player)
{
    (void)Direction(player);
}

static void Land(SotnPlayer *player, Sint32 speed)
{
    const bool raised = player->shield_cursor.phase != 0 && player->shield_cursor.phase < 3 &&
                        (player->buttons & player->shield_button);
    const bool ledge = !raised && (player->contact & 0x20) != 0;
    player->vx = speed;
    player->vy = 0;
    SetStep(player, SOTN_PLAYER_STAND);
    if (raised)
        player->substep = 2;
    Uint16 animation = ledge ? 3 : 4;
    if (player->previous_step == SOTN_PLAYER_WALK)
    {
        if (player->animation.id == 9)
        {
            player->animation.id = ledge ? 11 : 10;
            return;
        }
        animation = player->animation.id == 7 ? (ledge ? 5 : 4) : (ledge ? 15 : 14);
    }
    else if (player->previous_step == SOTN_PLAYER_JUMP || player->previous_step == SOTN_PLAYER_FALL)
        animation = ledge ? 30 : 29;
    Animate(player, animation);
}

static void MovementSmoke(SotnPlayer *player, Uint32 blueprint)
{
    if (player->movement_effects.smoke)
    {
        player->failed = true;
        SDL_SetError("Ordinary movement exceeded its selected one-update factory request");
        return;
    }
    player->movement_effects =
        (SotnPlayerMovementEffects){.smoke = true,
                                    .smoke_blueprint = blueprint,
                                    .smoke_source = {.x = player->x.value,
                                                     .y = player->y.value,
                                                     .priority = 148,
                                                     .facing_left = player->facing_left}};
}
static void Walk(SotnPlayer *player, bool force)
{
    const bool ledge = (player->contact & 0x20) != 0;
    SetSpeed(player, WALK_SPEED);
    player->vy = 0;
    SetStep(player, SOTN_PLAYER_WALK);
    if (!force)
        MovementSmoke(player, 0x50001);
    if (force)
    {
        if (player->animation.id != 13)
            Animate(player, 13);
    }
    else
        Animate(player, 7);
    if (player->turned)
        player->animation.id = 9;
    if (player->animation.id == 7 && ledge)
        player->animation.pose = 1;
    if (player->previous_step == SOTN_PLAYER_CROUCH)
        player->animation.pose = 4;
}

static void Crouch(SotnPlayer *player, unsigned int kind, Sint32 speed)
{
    player->vx = speed;
    player->vy = 0;
    SetStep(player, SOTN_PLAYER_CROUCH);
    switch (kind)
    {
    case 0:
        player->substep = 0;
        Animate(player, 17);
        break;
    case 1:
        player->substep = 4;
        Animate(player, 21);
        break;
    case 2:
        player->substep = 1;
        Animate(player, 16);
        break;
    default:
        player->substep = 3;
        Animate(player, 35);
        break;
    }
}

static void Fall(SotnPlayer *player)
{
    SetStep(player, SOTN_PLAYER_FALL);
    if (player->previous_step != SOTN_PLAYER_WALK)
        Animate(player, 28);
    player->vx = 0;
    player->vy = 2 * FIX_ONE;
    player->timers[6] = 8;
    player->timers[5] = (player->previous_contact & 1) ? 8 : 0;
    player->movement_flags = 16;
}

static void Jump(SotnPlayer *player)
{
    if (player->tight_space)
    {
        Fall(player);
        return;
    }
    if (Direction(player))
    {
        Animate(player, 26);
        SetSpeed(player, WALK_SPEED);
        player->movement_flags = 0;
    }
    else
    {
        Animate(player, 22);
        player->vx = 0;
        player->movement_flags = 4;
    }
    player->vy = JUMP_SPEED;
    SetStep(player, SOTN_PLAYER_JUMP);
    if (player->previous_step == SOTN_PLAYER_WALK)
        player->movement_flags |= 0x10;
    player->movement_flags &= ~1U;
}

void SotnPlayer_FormReturn(SotnPlayer *p, bool cramped)
{
    if (cramped)
    {
        if (p->tight_space == 1 || p->tight_space == 2)
        {
            Animate(p, 0xc7);
            p->vx = p->vy = 0;
            SetStep(p, SOTN_PLAYER_STUCK);
            p->sound = 0x6f2;
        }
        else
            Crouch(p, 0, 0);
        return;
    }
    Jump(p);
    if (!p->tight_space)
        p->movement_flags |= 1U;
    p->y.parts.high = (Sint16)(p->y.parts.high - 3);
    if (!(p->contact & 0x8000))
        p->vy = -FIX_ONE;
    p->movement_flags |= 0x100U;
    p->mist_lockout = 24;
}

void SotnPlayer_WolfReturn(SotnPlayer *p, bool cramped)
{
    if (cramped)
    {
        SotnPlayer_FormReturn(p, true);
        return;
    }
    Jump(p);
    if (!p->tight_space)
        p->movement_flags |= 1U;
    if (!(p->contact & 0x8000))
        p->vy = -FIX_ONE;
    p->movement_flags |= 0x100U;
    p->mist_lockout = 24;
}

void SotnPlayer_GravityLaunch(SotnPlayer *p)
{
    if (Direction(p))
        SetSpeed(p, 3 * FIX_ONE);
    else
        p->vx = 0;
    p->movement_flags = p->step == SOTN_PLAYER_JUMP ? p->movement_flags | 1U : 0;
    p->gravity_spawn = true;
    p->gravity_origin_x = p->x.value;
    p->gravity_origin_y = p->y.value;
    SetStep(p, SOTN_PLAYER_HIGH_JUMP);
    p->vy = -12 * FIX_ONE;
    Animate(p, 33);
    p->gravity.timer = 0;
}

static bool AttackView(SotnPlayer *player, Uint64 address, size_t size, const Uint8 **out)
{
    if (player->dra.bytes == NULL || address < 0x800a0000 || address > SDL_MAX_UINT32 ||
        (address & 1) != 0)
        return SDL_SetError("Unarmed attack references an invalid private DRA address");
    const size_t offset = address - 0x800a0000;
    if (offset > player->dra.size || size > player->dra.size - offset)
        return SDL_SetError("Unarmed attack references data outside private DRA");
    *out = player->dra.bytes + offset;
    return true;
}

static Uint16 Attack16(const Uint8 *bytes)
{
    return (Uint16)(bytes[0] | (Uint16)bytes[1] << 8);
}

static Uint32 Attack32(const Uint8 *bytes)
{
    return bytes[0] | (Uint32)bytes[1] << 8 | (Uint32)bytes[2] << 16 | (Uint32)bytes[3] << 24;
}

static void ClearAttack(SotnPlayer *player)
{
    player->subweapon_cast = false;
    player->curse_failure = false;
    player->attack_mode = 0;
    player->attack = (SotnPlayerAttack){0};
}

static void Backdash(SotnPlayer *player)
{
    SetStep(player, SOTN_PLAYER_STAND);
    player->substep = 3;
    SetSpeed(player, -229376);
    player->vy = 0;
    Animate(player, 0xdb);
    MovementSmoke(player, 0);
    ClearAttack(player);
    player->sound = 0x617;
}

// Selected empty-hand paths in 6E42C.c, 71830.c, and EntityUnarmedAttack.
static bool StartAttack(SotnPlayer *player)
{
    Uint16 pressed = player->tapped & (SOTN_BUTTON_LEFT_HAND | SOTN_BUTTON_RIGHT_HAND);
    if (player->curse_ticks && player->attack_mode == 0)
    {
        if (player->equipment[0] == 16 && (player->buttons & SOTN_BUTTON_LEFT_HAND))
            pressed |= SOTN_BUTTON_LEFT_HAND;
        if (player->equipment[1] == 16 && (player->buttons & SOTN_BUTTON_RIGHT_HAND))
            pressed |= SOTN_BUTTON_RIGHT_HAND;
    }
    if (pressed == 0)
        return false;
    if (player->subweapons != NULL && (player->buttons & SOTN_BUTTON_UP) && player->timers[0] == 0)
    {
        bool used = false;
        Uint16 animation = 0;
        SotnPlayerState state = {0};
        if (!SotnPlayer_GetState(player, &state) ||
            !SotnSubweapons_Use(player->subweapons, state, &used, &animation))
        {
            player->failed = true;
            return true;
        }
        if (used)
        {
            player->timers[0] = 4;
            if (player->step == SOTN_PLAYER_STAND && player->attack_mode == 0)
            {
                Animate(player, (Uint16)(animation + ((player->contact & 0x20) ? 1 : 0)));
                player->subweapon_cast = true;
                player->subweapon_animation = player->animation.id;
                player->substep = 1;
            }
            return true;
        }
    }
    if (player->attack_mode & 0x8000)
        return false;
    const Uint32 hand = pressed == SOTN_BUTTON_RIGHT_HAND ? 1U : 0U;
    SotnEquipment equipment = {0};
    if (!SotnEquipment_Read(player->dra, player->equipment[hand], &equipment))
    {
        player->failed = true;
        return true;
    }
    if (player->curse_ticks && equipment.id != 0)
    {
        ClearAttack(player);
        player->curse_failure = true;
        player->attack_mode = 0x8012;
        player->substep = 0x51;
        player->sound = 0x6e7;
        if (player->step == SOTN_PLAYER_STAND || player->step == SOTN_PLAYER_WALK)
        {
            player->step = SOTN_PLAYER_STAND;
            Animate(player, 0xb5);
        }
        else if (player->step == SOTN_PLAYER_CROUCH)
            Animate(player, 0xb6);
        else
        {
            player->step = SOTN_PLAYER_JUMP;
            Animate(player, 0xb7);
        }
        return true;
    }
    if (equipment.kind == 128)
    {
        player->shield_button = hand == 0 ? SOTN_BUTTON_LEFT_HAND : SOTN_BUTTON_RIGHT_HAND;
        if (player->step == SOTN_PLAYER_STAND)
        {
            player->substep = 2;
            Animate(player, (Uint16)(0x63 + ((player->contact & 0x20) ? 1 : 0)));
        }
        else if (player->step == SOTN_PLAYER_CROUCH)
            Animate(player, 0x65);
        return true;
    }
    player->subweapon_cast = false;
    player->attack_equipment = equipment;
    Sint32 critical = equipment.critical_rate;
    if (equipment.id != 0)
    {
        const Uint16 value = (Uint16)(player->attributes.total[3] * 2 +
                                      (SotnCombat_Random(&player->damage_random) & 15));
        Sint32 root = 0;
        while ((root + 1) * (root + 1) <= value)
            ++root;
        critical += root - 5;
    }
    player->attack_critical = (Uint8)SDL_clamp(critical, 0, 255);
    player->attack_power =
        (Uint16)(player->attributes.attack[hand] >> (player->poison_ticks ? 1 : 0));
    Uint16 variant = (player->contact & 0x20) ? 1 : 0;
    if (player->step == SOTN_PLAYER_STAND || player->step == SOTN_PLAYER_WALK)
    {
        player->vx = player->vx >= 0 ? player->vx / 2 : -((-player->vx + 1) / 2);
        player->step = SOTN_PLAYER_STAND;
    }
    else if (player->step == SOTN_PLAYER_CROUCH)
    {
        variant = (player->buttons & (SOTN_BUTTON_LEFT | SOTN_BUTTON_RIGHT)) ? 3 : 2;
        if (player->tight_space == 0 && player->substep == 2)
        {
            variant = (player->contact & 0x20) ? 1 : 0;
            player->step = SOTN_PLAYER_STAND;
        }
    }
    else
    {
        variant = player->vy > 0 ? 5 : 4;
        if (player->vy > 0 && (player->buttons & SOTN_BUTTON_DOWN) &&
            (equipment.id != 0 || (player->buttons & (SOTN_BUTTON_LEFT | SOTN_BUTTON_RIGHT))))
            variant = 6;
    }
    Animate(player, (Uint16)(equipment.animation + variant));
    player->substep = (Uint16)(0x40 + equipment.kind);
    player->attack_mode = (Uint16)(0x8001 + equipment.kind);
    player->attack_lock = equipment.lock;
    player->weapon_startup = 4;
    player->attack_button =
        pressed == SOTN_BUTTON_RIGHT_HAND ? SOTN_BUTTON_RIGHT_HAND : SOTN_BUTTON_LEFT_HAND;
    return true;
}

static bool TickShield(SotnPlayer *player)
{
    Uint16 button = player->shield_button;
    if (button == 0)
        button = player->equipment[0] == 16 ? SOTN_BUTTON_LEFT_HAND : SOTN_BUTTON_RIGHT_HAND;
    const Uint32 hand = button == SOTN_BUTTON_RIGHT_HAND ? 1U : 0U;
    const bool disabled =
        player->equipment[hand] != 16 || player->curse_ticks != 0 || player->curse_failure ||
        player->step == SOTN_PLAYER_KILL || player->step == SOTN_PLAYER_HIT ||
        player->step == SOTN_PLAYER_STONE || player->step == SOTN_PLAYER_MIST ||
        player->step == SOTN_PLAYER_UNMORPH_MIST || player->step == SOTN_PLAYER_WOLF ||
        player->step == SOTN_PLAYER_UNMORPH_WOLF || player->step == SOTN_PLAYER_BAT ||
        player->step == SOTN_PLAYER_UNMORPH_BAT || player->step == SOTN_PLAYER_STUCK ||
        player->step == SOTN_PLAYER_BOSS_GRAB;
    const bool starting = player->shield_cursor.phase == 0;
    if (!SotnShield_Tick(player->weapons, player->animation.id, player->animation.pose,
                         player->step == SOTN_PLAYER_CROUCH, (player->buttons & button) != 0,
                         disabled, &player->shield_cursor))
        return false;
    if (starting && player->shield_cursor.phase)
        player->shield_power = player->poison_ticks ? 0 : 1;
    const SotnWeaponFrame f = player->shield_cursor.frame;
    player->shield = (SotnPlayerAttack){.offset_x = f.x,
                                        .offset_y = f.y,
                                        .width = f.width,
                                        .height = f.height,
                                        .sprite = f.sprite,
                                        .active = f.width != 0 && f.height != 0,
                                        .equipment = player->shield_cursor.phase ? 16 : 0,
                                        .right_hand = hand != 0,
                                        .hit_type = 4,
                                        .hit_effect = 1,
                                        .invincibility_frames = 6,
                                        .power = player->shield_power};
    if (player->step == SOTN_PLAYER_CROUCH && player->attack_mode == 0)
    {
        const bool raised = player->shield_cursor.phase == 1 || player->shield_cursor.phase == 2;
        if (raised && player->animation.id == 17)
        {
            player->animation.id = 0x65;
            player->animation.remaining = 2;
            player->animation.pose = 3;
        }
        else if (!raised && player->animation.id == 0x65)
            Animate(player, 17);
    }
    return true;
}

static bool ReadAttack(SotnPlayer *player)
{
    player->attack = (SotnPlayerAttack){0};
    if (player->subweapon_cast || player->curse_failure)
        return true;
    if (player->attack_mode != 0 && player->attack_equipment.id != 0)
    {
        SotnWeaponFrame weapon = {0};
        const SotnEquipment equipment = player->attack_equipment;
        if (!SotnWeapon_ReadFrame(player->weapons, equipment, player->animation.id,
                                  player->animation.pose, player->animation.remaining, &weapon))
            return false;
        player->attack =
            (SotnPlayerAttack){.offset_x = weapon.x,
                               .offset_y = weapon.y,
                               .width = weapon.width,
                               .height = weapon.height,
                               .element = equipment.element,
                               .hit_type = equipment.hit_type,
                               .hit_effect = equipment.hit_effect,
                               .stun_frames = equipment.stun,
                               .invincibility_frames = equipment.invincibility,
                               .critical_rate = player->attack_critical,
                               .active = weapon.width != 0 && weapon.height != 0,
                               .right_hand = player->attack_button == SOTN_BUTTON_RIGHT_HAND,
                               .sound = weapon.sound != 0,
                               .sound_id = weapon.sound,
                               .power = player->attack_power,
                               .equipment = equipment.id,
                               .sprite = weapon.sprite};
        if (equipment.id == 18 && player->animation.id < equipment.animation + 2 &&
            player->animation.pose == 6)
            player->attack.hit_type = 4;
        return true;
    }
    if (player->attack_mode == 0 || player->animation.id < 0x24 || player->animation.id > 0x2a ||
        player->animation.remaining < 0)
        return true;
    const Uint8 *descriptor = NULL, *pointer = NULL, *frame = NULL, *box = NULL, *equipment = NULL;
    if (!AttackView(player, 0x800ad53c, 16, &descriptor) ||
        !AttackView(player, (Uint64)Attack32(descriptor) + (player->animation.id - 0x24U) * 4, 4,
                    &pointer))
        return false;
    if (player->animation.pose >= 32 ||
        !AttackView(player, (Uint64)Attack32(pointer) + (Uint32)player->animation.pose * 2, 2,
                    &frame))
        return SDL_SetError("Unarmed attack exceeds its bounded frame stream");
    const Uint16 packed = Attack16(frame);
    if (packed == 0xffff)
        return true;
    const Uint32 property = (packed >> 9) & 127;
    if (property >= 12 ||
        !AttackView(player, (Uint64)Attack32(descriptor + 4) + property * 4, 4, &box) ||
        !AttackView(player, 0x800a00c0, 4, &pointer) ||
        !AttackView(player, Attack32(pointer), 50, &equipment))
        return SDL_SetError("Unarmed hitbox or equipment data is invalid");
    player->attack.offset_x = (Sint16)(box[0] < 128 ? box[0] : (Sint32)box[0] - 256);
    player->attack.offset_y = (Sint16)(box[1] < 128 ? box[1] : (Sint32)box[1] - 256);
    player->attack.width = box[2];
    player->attack.height = box[3];
    player->attack.power = player->attack_power;
    player->attack.sound_id = 0x60c;
    player->attack.element = Attack16(equipment + 12);
    player->attack.hit_type = Attack16(equipment + 40);
    player->attack.hit_effect = Attack16(equipment + 42);
    player->attack.stun_frames = Attack16(equipment + 38);
    player->attack.invincibility_frames = equipment[26];
    player->attack.critical_rate = (Uint8)SDL_min(255, Attack16(equipment + 48));
    player->attack.right_hand = player->attack_button == SOTN_BUTTON_RIGHT_HAND;
    player->attack.active = box[2] != 0 && box[3] != 0;
    player->attack.sound =
        player->animation.remaining == 1 && player->animation.pose == descriptor[13];
    return true;
}

static bool Transitions(SotnPlayer *player, Uint32 flags)
{
    if ((flags & 8) && player->attack_mode == 0)
        (void)Direction(player);
    Sint32 gravity = (flags & 0x8000) ? GRAVITY : 0;
    if (flags & 0x10000)
        gravity = player->vy < 24576 && player->vy > -8192 && !(player->movement_flags & 0x20) &&
                          (player->buttons & SOTN_BUTTON_JUMP)
                      ? 2252
                      : GRAVITY;
    player->vy = SDL_min(7 * FIX_ONE, player->vy + gravity);
    if (player->vy >= 0 && (flags & 1) && (player->contact & 1))
    {
        const bool diving = player->step == SOTN_PLAYER_JUMP && player->substep == 0x70;
        const bool hard =
            player->vy > 450560 || (!player->attack_mode && !(player->movement_flags & 0x10) &&
                                    (player->vx > 2 * FIX_ONE || player->vx < -2 * FIX_ONE));
        if (player->vy > 450560)
            Crouch(player, diving ? 3U : 1U, diving ? player->vx / 2 : 0);
        else if (player->movement_flags & 0x10)
            Walk(player, true);
        else if (diving && (player->vx > 2 * FIX_ONE || player->vx < -2 * FIX_ONE))
            Land(player, player->vx);
        else
            Land(player, 0);
        ClearAttack(player);
        player->sound = hard ? 0x647 : 0x64c;
        if (hard)
            MovementSmoke(player, 0);
        return true;
    }
    if ((flags & 4) && !(player->contact & 1))
    {
        Fall(player);
        ClearAttack(player);
        return true;
    }
    if ((flags & 0x1000) && StartAttack(player))
        return true;
    if ((flags & 0x10) && !(player->attack_mode & 0x8000) && (player->tapped & SOTN_BUTTON_JUMP))
    {
        Jump(player);
        ClearAttack(player);
        return true;
    }
    if ((flags & 0x20) && !(player->attack_mode & 0x8000) &&
        (player->relics[SOTN_RELIC_LEAP] & 2) && (player->tapped & SOTN_BUTTON_JUMP) &&
        !(player->movement_flags & 1))
    {
        // 6E42C.c:func_8010E940 retains Jump/Fall and attack state, unlike an ordinary jump.
        player->movement_flags |= 0x21;
        Animate(player, 32);
        player->substep = 0;
        player->vy = player->tight_space ? 0 : -278528;
        return true;
    }
    if ((flags & 0x2000) && !(player->attack_mode & 0x8000) && (player->buttons & SOTN_BUTTON_DOWN))
    {
        Crouch(player, 2, 0);
        ClearAttack(player);
        return true;
    }
    if ((flags & 0x40000) && !(player->attack_mode & 0x8000) && player->animation.id != 0xdb &&
        ((player->attack_mode & 0x7fff ? player->buttons : player->tapped) & SOTN_BUTTON_CANCEL))
    {
        Backdash(player);
        return true;
    }
    return false;
}

static void StandTick(SotnPlayer *player)
{
    if (player->subweapon_cast && player->animation.id != player->subweapon_animation)
        player->subweapon_cast = false;
    if (Transitions(player, 0x4301c))
        return;
    Decelerate(player, 8192);
    if (player->curse_failure)
    {
        if (player->animation.remaining < 0)
        {
            Land(player, 0);
            ClearAttack(player);
        }
        return;
    }
    if (player->substep == 3)
    {
        if (player->animation.pose >= 7 || player->animation.remaining < 0)
        {
            Land(player, 0);
            if (player->buttons & SOTN_BUTTON_UP)
            {
                Animate(player, (player->contact & 0x20) ? 1 : 0);
                player->substep = 1;
            }
            if (Direction(player))
                Walk(player, false);
        }
        else if ((player->animation.pose >= 4 || player->animation.id == 14) && Direction(player))
            Walk(player, false);
        return;
    }
    if (player->attack_mode != 0)
    {
        SotnPlayer_AfterimageReset(player);
        if (player->animation.pose < player->attack_lock)
        {
            if (player->animation.pose < (player->attack_equipment.id == 0 ? 2 : 3))
            {
                (void)Direction(player);
                if (player->buttons & SOTN_BUTTON_DOWN)
                {
                    player->step = SOTN_PLAYER_CROUCH;
                    player->animation.id = (Uint16)(player->attack_equipment.animation + 2);
                }
            }
            return;
        }
        player->attack_mode &= 0x7fff;
        if (player->attack_equipment.id != 0 && player->animation.remaining < 0)
        {
            Land(player, 0);
            ClearAttack(player);
            return;
        }
        if (player->attack_equipment.id == 0 && player->animation.pose == 7)
        {
            if (player->buttons & player->attack_button)
                player->animation.remaining = 2;
            else
            {
                Land(player, 0);
                ClearAttack(player);
            }
        }
        if (Direction(player))
        {
            Walk(player, false);
            ClearAttack(player);
        }
        return;
    }
    if (player->substep == 2)
    {
        if (!(player->buttons & player->shield_button))
            Land(player, 0);
        else if (player->animation.id != 10 && player->animation.id != 11 &&
                 player->animation.id != 14 && player->animation.id != 15 &&
                 player->animation.id != 0x63)
            Animate(player, 0x63);
    }
    if (player->substep == 4 && player->animation.id == 0xf2 && player->chair_sleep &&
        --player->chair_sleep == 0)
    {
        player->sleep_event = true;
        player->chair_sleep = 0x200;
    }
    if (player->substep == 1 && player->chair_kind)
    {
        Animate(player, player->chair_kind <= 2 ? 0xf2 : 0xf3);
        player->facing_left = (player->chair_kind & 1U) == 0;
        player->substep = 4;
        player->chair_sleep = 0x1800;
    }
    if (player->substep == 1 && !(player->buttons & SOTN_BUTTON_UP))
        Land(player, 0);
    if ((player->buttons & SOTN_BUTTON_UP) && player->substep == 0)
    {
        Animate(player, (player->contact & 0x20) ? 1 : 0);
        player->substep = 1;
        if (player->chair_kind)
        {
            Animate(player, player->chair_kind <= 2 ? 0xf2 : 0xf3);
            player->facing_left = (player->chair_kind & 1U) == 0;
            player->substep = 4;
            player->chair_sleep = 0x1800;
        }
    }
    if (Direction(player))
        Walk(player, false);
}

static void JumpTick(SotnPlayer *player)
{
    Decelerate(player, 4096);
    if (player->vy < -FIX_ONE)
    {
        if (!(player->movement_flags & 0x40) && !(player->buttons & SOTN_BUTTON_JUMP))
            player->vy = -FIX_ONE;
        if (player->contact & 2)
        {
            player->vy = -16384;
            player->movement_flags |= 0x20;
        }
    }
    if (Transitions(player, 0x11029))
        return;
    if (player->substep == 0x70)
    {
        if (player->movement_flags & 0x80)
        {
            Jump(player);
            if (!(player->buttons & SOTN_BUTTON_JUMP))
                player->vy = -278528;
            player->movement_flags |= 0x40;
        }
        if (player->movement_flags & 0x100)
            player->vx = 0;
        return;
    }
    if (player->attack_mode != 0 && player->substep >= 0x40)
    {
        SotnPlayer_AfterimageReset(player);
        if (player->buttons & SOTN_BUTTON_LEFT)
            player->vx = -WALK_SPEED;
        if (player->buttons & SOTN_BUTTON_RIGHT)
            player->vx = WALK_SPEED;
        if (player->animation.remaining < 0)
        {
            player->substep = player->vy > FIX_ONE ? 1 : 0;
            Animate(player, player->vy > FIX_ONE ? 31 : 27);
            ClearAttack(player);
        }
        return;
    }
    const int direction = Direction(player);
    if (direction != 0)
    {
        if (player->substep == 0 && (player->animation.id == 22 || player->animation.id == 25))
            Animate(player, 24);
        SetSpeed(player, WALK_SPEED);
    }
    else if (player->substep == 0 && (player->animation.id == 26 || player->animation.id == 24))
        Animate(player, 25);
    if (direction <= 0)
        player->movement_flags &= ~0x10U;
    if (player->substep == 0 && player->vy > 0)
    {
        if (player->animation.id != 27)
            Animate(player, 27);
        player->substep = 1;
    }
    if (player->substep <= 1 && (player->movement_flags & 1) &&
        (player->buttons & SOTN_BUTTON_DOWN) && (player->tapped & SOTN_BUTTON_JUMP))
    {
        Animate(player, 34);
        player->substep = 0x70;
        player->movement_flags &= ~0x80U;
        player->vy = 6 * FIX_ONE;
        if (direction != 0)
            SetSpeed(player, 294912);
        player->sound = 0x6f0;
    }
    if (player->movement_flags & 0x100)
        player->vx = 0;
}

static void CrouchTick(SotnPlayer *player)
{
    if ((player->tapped & SOTN_BUTTON_JUMP) && !(player->attack_mode & 0x8000))
    {
        if (player->buttons & SOTN_BUTTON_DOWN)
        {
            for (int i = 0; i < 4; ++i)
                if (player->floor[i].effects & SOTN_EFFECT_SOLID_FROM_ABOVE)
                {
                    player->timers[7] = 8;
                    return;
                }
        }
        if (!player->tight_space)
        {
            Jump(player);
            ClearAttack(player);
            return;
        }
    }
    if (Transitions(player, 0x100c))
        return;
    Decelerate(player, 8192);
    if (player->curse_failure)
    {
        if (player->animation.remaining < 0)
        {
            Crouch(player, 0, 0);
            ClearAttack(player);
        }
        return;
    }
    if (player->attack_mode != 0)
    {
        SotnPlayer_AfterimageReset(player);
        if (player->animation.pose < player->attack_lock)
        {
            if (player->animation.pose < (player->attack_equipment.id == 0 ? 2 : 3))
            {
                (void)Direction(player);
                if (!(player->buttons & SOTN_BUTTON_DOWN) && player->tight_space == 0)
                {
                    player->step = SOTN_PLAYER_STAND;
                    player->animation.id = (Uint16)(player->attack_equipment.animation +
                                                    ((player->contact & 0x20) ? 1 : 0));
                }
            }
            return;
        }
        player->attack_mode &= 0x7fff;
        if (player->attack_equipment.id != 0 && player->animation.remaining < 0)
        {
            Crouch(player, 0, 0);
            ClearAttack(player);
            return;
        }
        if (player->attack_equipment.id == 0 && player->animation.pose == 7)
        {
            if (player->buttons & player->attack_button)
                player->animation.remaining = 2;
            else
            {
                Crouch(player, 0, 0);
                ClearAttack(player);
            }
        }
        if (!(player->buttons & SOTN_BUTTON_DOWN))
        {
            Animate(player, 19);
            player->substep = 2;
            ClearAttack(player);
        }
        return;
    }
    Uint32 flags = 0;
    switch (player->substep)
    {
    case 0:
        flags = 6;
        break;
    case 1:
        if (!(player->buttons & SOTN_BUTTON_DOWN))
        {
            flags = 1;
            if (player->animation.pose >= 4)
            {
                Animate(player, 19);
                player->substep = 2;
                player->animation.pose = player->tight_space == 1   ? 0
                                         : player->tight_space == 2 ? 1
                                                                    : 2;
                return;
            }
        }
        else if (player->animation.remaining < 0)
            flags = 0x20;
        break;
    case 3:
    case 4:
        SotnPlayer_AfterimageReset(player);
        if (player->animation.remaining < 0)
            flags = 0x20;
        break;
    case 5:
        SotnPlayer_AfterimageReset(player);
        if (player->tight_space == player->animation.pose || player->animation.remaining < 0)
            flags = 0x20;
        break;
    case 2:
        flags = 1;
        if (player->buttons & SOTN_BUTTON_DOWN)
        {
            Crouch(player, 0, player->vx);
            break;
        }
        if (player->tight_space)
        {
            SotnPlayer_AfterimageReset(player);
            const Uint16 limit = (Uint16)(player->tight_space - 1);
            if (player->animation.pose >= limit)
            {
                player->animation.pose = limit;
                player->animation.remaining = 3;
            }
        }
        else if (player->animation.remaining < 0)
            Land(player, 0);
        break;
    default:
        player->failed = true;
        SDL_SetError("Unsupported Alucard crouch substate");
        return;
    }
    if (flags & 0x20)
        Crouch(player, 0, 0);
    if ((flags & 2) && player->turned)
    {
        Animate(player, 20);
        player->substep = 0;
    }
    if ((flags & 4) && !(player->buttons & SOTN_BUTTON_DOWN))
    {
        Animate(player, 19);
        player->substep = 2;
    }
    if ((flags & 1) && Direction(player))
    {
        if (player->tight_space == 0 || player->tight_space >= 3)
            Walk(player, false);
        else if (player->tapped & (SOTN_BUTTON_RIGHT | SOTN_BUTTON_LEFT))
        {
            player->substep = 5;
            Animate(player, 246);
            SetSpeed(player, 49152);
        }
    }
}

static void PhysicalDeath(SotnPlayer *player, bool right)
{
    player->step = SOTN_PLAYER_KILL;
    player->substep = 1;
    player->death = (SotnPlayerDeath){.phase = 1, .red = 128, .blue = 128};
    player->draw_pivot_x = player->draw_pivot_y = 0;
    player->vx = right ? 81920 : -81920;
    player->vy = -212992;
    player->sound = 0x6f6;
    Animate(player, 0xc0);
}

// dra/71830.c:PlayerStepBossGrab. BO5 clears GRAB_NONE rather than using the thrown release.
static void BossGrabTick(SotnPlayer *player)
{
    if (player->substep == 0)
    {
        ClearAttack(player);
        player->movement_flags = 0;
        player->stone = (SotnPlayerStone){0};
        Animate(player, 0x37);
        player->timers[2] = 8;
        player->sound = 0x6eb;
        player->vx = player->vy = 0;
        player->substep = 1;
        player->grab.state = SOTN_GRAB_HELD;
    }
    else if (player->grab.state == SOTN_GRAB_DAMAGE)
    {
        Uint16 damage = 0;
        const SotnContactPower power = {.attack = player->grab.damage};
        if (!SotnCombat_NormalContact(power, player->attributes.defense, player->poison_ticks != 0,
                                      &damage))
        {
            player->failed = true;
            return;
        }
        player->hp = damage >= player->hp ? 0 : (Uint16)(player->hp - damage);
        if (!player->hp)
        {
            const Sint32 center =
                player->x.parts.high +
                (player->facing_left ? -player->animation.hitbox_x : player->animation.hitbox_x);
            const Sint32 delta = center - player->grab.source_x;
            const bool right = SDL_abs(delta) < 16 && player->grab.source_velocity != 0
                                   ? player->grab.source_velocity > 0
                                   : delta >= 0;
            PhysicalDeath(player, right);
            return;
        }
        player->timers[2] = 4;
        player->sound = 0x6e7;
        player->grab.state = SOTN_GRAB_HELD;
    }
    else if (player->grab.state == SOTN_GRAB_NONE)
    {
        player->step = SOTN_PLAYER_HIT;
        player->substep = 1;
    }
    player->invincibility = (Uint16)SDL_max(player->invincibility, 12);
}

// Selected physical death; the selected stone completion uses its own clocks.
static void DeathTick(SotnPlayer *player)
{
    if (player->stone.active)
    {
        SotnPlayer_StoneDeathTick(player);
        return;
    }
    if (player->death.phase == 1)
    {
        if (player->death.red < 248)
            ++player->death.red;
        if (player->death.blue > 8)
            --player->death.blue;
        player->vy += 5632;
        if (player->vy > 16384)
            player->vy = 4096;
        if (player->animation.remaining < 0)
        {
            player->death.phase = 2;
            player->death.timer = 64;
        }
    }
    else if (player->death.phase == 2)
    {
        for (Uint32 i = 0; i < 4; ++i)
        {
            const Uint16 cell = player->death.cursor;
            player->death.mask[cell / 8] |= (Uint8)(1U << (cell & 7));
            player->death.cursor = (Uint16)((cell + 35) & 255);
        }
        if (--player->death.timer == 0)
        {
            player->vy = 0;
            player->death.phase = 128;
        }
    }
    Decelerate(player, 1024);
    if (player->animation.pose >= 15)
    {
        if (player->animation.pose == 22 && player->animation.remaining == 1)
            player->death.rotation -= 256;
        player->death.rotation = (Sint16)SDL_max(-640, player->death.rotation - 6);
    }
}

bool SotnPlayer_Tick(SotnPlayer *player, Uint16 buttons)
{
    const Uint16 supported =
        SOTN_BUTTON_UP | SOTN_BUTTON_RIGHT | SOTN_BUTTON_DOWN | SOTN_BUTTON_LEFT |
        SOTN_BUTTON_JUMP | SOTN_BUTTON_LEFT_HAND | SOTN_BUTTON_RIGHT_HAND |
        (player && player->mist_context ? SOTN_BUTTON_MIST : 0) |
        (player && player->bat_context ? SOTN_BUTTON_BAT : 0) |
        (player && player->wolf_context ? SOTN_BUTTON_WOLF : 0) | SOTN_BUTTON_CANCEL;
    if (player == NULL || (buttons & (Uint16)~supported) != 0)
        return SDL_SetError("Player input contains unsupported normal-form button bits");
    SotnPlayer next = *player;
    next.failed = false;
    next.turned = false;
    next.sound = 0;
    next.extra_sound = 0;
    next.mp_spent = 0;
    next.gravity.ceiling_impact = false;
    next.bat_commands = (SotnBatCommandEvents){0};
    next.wolf.commands = (SotnWolfControllerEvents){0};
    next.form_rotated = false;
    next.sleep_event = false;
    next.movement_effects = (SotnPlayerMovementEffects){0};
    const bool poison_recovery =
        next.poison_ticks == 1 && next.step != SOTN_PLAYER_STONE && !next.curse_ticks;
    if (next.curse_ticks)
        --next.curse_ticks;
    if (next.poison_ticks)
        --next.poison_ticks;
    if (next.script_hold)
        buttons = 0;
    if (next.bat_input_hold)
    {
        --next.bat_input_hold;
        buttons = 0;
    }
    if (next.bell_inside)
        buttons &= (Uint16) ~(SOTN_BUTTON_LEFT_HAND | SOTN_BUTTON_RIGHT_HAND);
    next.bell_inside = false;
    next.buttons = buttons;
    next.held = next.previous_buttons;
    next.tapped = (Uint16)(buttons & (Uint16)~next.previous_buttons);
    next.previous_buttons = buttons;
    for (size_t i = 0; i < SDL_arraysize(next.timers); ++i)
        if (next.timers[i] != 0)
            --next.timers[i];
    // Original poison expiry requests timer15 before that later timer's own decrement.
    if (poison_recovery && next.afterimage_generation)
        next.afterimage_timer15 = 12;
    if (!SotnPlayer_AfterimageBegin(&next))
        return false;
    if (next.weapon_startup)
        --next.weapon_startup;
    if (next.step != SOTN_PLAYER_KILL)
        next.tight_space = SotnPlayer_TightSpace(&next);
    if (next.failed)
        return false;
    next.previous_step = next.step;
    if (next.invincibility)
        --next.invincibility;
    if (next.script_hold)
    {
        SotnPlayer_GravityCancel(&next);
        SotnPlayer_BatCancel(&next);
        SotnPlayer_WolfCancel(&next);
        // PAD_SIM mode 2 changes step without selecting a new animation (692E8.c/alu_anim.c).
        SetStep(&next, SOTN_PLAYER_SCRIPT);
        next.invincibility = (Uint16)SDL_max(next.invincibility, 12);
        ClearAttack(&next);
    }
    if (next.grab.state == SOTN_GRAB_START)
    {
        SotnPlayer_GravityCancel(&next);
        SotnPlayer_BatCancel(&next);
        SotnPlayer_WolfCancel(&next);
        SetStep(&next, SOTN_PLAYER_BOSS_GRAB);
    }
    SotnPlayer_GravityInput(&next);
    SotnPlayer_MistInput(&next);
    SotnPlayer_BatInput(&next);
    SotnPlayer_WolfInput(&next);
    switch (next.step)
    {
    case SOTN_PLAYER_WOLF:
        SotnPlayer_WolfTick(&next);
        break;
    case SOTN_PLAYER_UNMORPH_WOLF:
        SotnPlayer_UnmorphWolfTick(&next);
        break;
    case SOTN_PLAYER_BAT:
        SotnPlayer_BatTick(&next);
        break;
    case SOTN_PLAYER_UNMORPH_BAT:
        SotnPlayer_UnmorphBatTick(&next);
        break;
    case SOTN_PLAYER_MIST:
        SotnPlayer_MistTick(&next);
        break;
    case SOTN_PLAYER_UNMORPH_MIST:
        SotnPlayer_UnmorphMistTick(&next);
        break;
    case SOTN_PLAYER_STUCK:
        SotnPlayer_StuckTick(&next);
        break;
    case SOTN_PLAYER_BOSS_GRAB:
        BossGrabTick(&next);
        break;
    case SOTN_PLAYER_SCRIPT:
        next.vx = next.vy = 0;
        if (!next.script_hold)
            Land(&next, 0);
        break;
    case SOTN_PLAYER_STAND:
        StandTick(&next);
        break;
    case SOTN_PLAYER_WALK:
        if (!Transitions(&next, 0x4301c))
        {
            SetSpeed(&next, WALK_SPEED);
            if (Direction(&next) == 0)
                Land(&next, 0);
        }
        break;
    case SOTN_PLAYER_JUMP:
        JumpTick(&next);
        break;
    case SOTN_PLAYER_HIGH_JUMP:
        SotnPlayer_GravityTick(&next);
        break;
    case SOTN_PLAYER_CROUCH:
        CrouchTick(&next);
        break;
    case SOTN_PLAYER_FALL:
        if (next.timers[5] && (next.tapped & SOTN_BUTTON_JUMP))
        {
            Jump(&next);
            ClearAttack(&next);
        }
        else if (!Transitions(&next, 0x9029))
        {
            Decelerate(&next, 4096);
            if (Direction(&next))
                SetSpeed(&next, 49152);
        }
        break;
    case SOTN_PLAYER_STONE:
        SotnPlayer_StoneTick(&next);
        break;
    case SOTN_PLAYER_KILL:
        DeathTick(&next);
        break;
    case SOTN_PLAYER_HIT:
        next.invincibility = 12;
        if (next.substep == 1)
        {
            next.vy = SDL_min(7 * FIX_ONE,
                              next.vy + (next.vy < 24576 && next.vy > -8192 ? 5632 : GRAVITY));
            if ((next.contact & 2) && next.vy < -FIX_ONE)
                next.vy = -FIX_ONE;
            if (next.vy >= 0 && (next.contact & 1))
            {
                Crouch(&next, 3, next.vx);
                next.invincibility = 12;
            }
            else if (next.animation.remaining < 0)
            {
                Animate(&next, 28);
                next.facing_left = !next.facing_left;
            }
        }
        else
        {
            Decelerate(&next, 8192);
            if (!(next.contact & 1))
                Fall(&next);
            else if (next.animation.remaining < 0)
            {
                if (next.substep == 6)
                    Land(&next, 0);
                else
                    Crouch(&next, 0, next.vx);
            }
        }
        break;
    default:
        return SDL_SetError("Unsupported Alucard movement state");
    }
    next.status =
        (next.status & ~7U) |
        ((next.step == SOTN_PLAYER_MIST || next.step == SOTN_PLAYER_UNMORPH_MIST) ? 2U : 0U) |
        ((next.step == SOTN_PLAYER_BAT || next.step == SOTN_PLAYER_UNMORPH_BAT) ? 1U : 0U) |
        (next.step == SOTN_PLAYER_WOLF ? 4U : 0U);
    if ((next.status & 2U) || next.step == SOTN_PLAYER_UNMORPH_BAT ||
        next.step == SOTN_PLAYER_UNMORPH_WOLF ||
        (next.step == SOTN_PLAYER_WOLF && SDL_abs(next.vx) > 3 * FIX_ONE) ||
        (next.step == SOTN_PLAYER_BAT && next.substep == 3))
        next.invincibility = (Uint16)SDL_max(next.invincibility, 4);
    if (next.failed)
        return false;
    const bool animated =
        next.wolf.active
            ? SotnAnimation_TickWolf(next.dra, next.form_pose,
                                     next.wolf.controller.cadence.durations, &next.animation)
        : next.bat.active || next.mist.active
            ? SotnAnimation_TickPlayerForm(next.dra, next.form_pose, &next.animation)
            : SotnAnimation_Tick(next.dra, &next.animation);
    if (!animated || !ReadAttack(&next) || !TickShield(&next) || !SotnPlayer_KickTick(&next))
        return false;
    if (!SotnPlayer_AfterimageCapture(&next))
        return false;
    if (next.step == SOTN_PLAYER_KILL)
    {
        next.x.value += next.vx;
        next.y.value += next.vy;
    }
    else if (next.step != SOTN_PLAYER_BOSS_GRAB)
        SotnPlayer_Collide(&next);
    if (next.failed)
        return false;
    if (!SotnPlayer_AfterimagePresent(&next))
        return false;
    if (!PositionValid(next.map, next.x.value, next.y.value))
        return SDL_SetError("Player left the bounded room region without a supported transition");
    if (!SotnPlayer_SleepTick(&next))
        return false;
    SotnPlayer_GravityBeams(&next);
    next.chair_kind = 0;
    ++next.frame_number;
    *player = next;
    return true;
}

bool SotnPlayer_Create(SotnDataView dra, SotnCollisionMap map, Sint32 x, Sint32 y, SotnPlayer **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (!ValidateMap(map))
        return false;
    const Sint64 start_x = (Sint64)x * FIX_ONE, start_y = (Sint64)y * FIX_ONE;
    if (!PositionValid(map, start_x, start_y))
        return SDL_SetError("Player development start is outside its bounded room");
    SotnPlayer *player = SDL_calloc(1, sizeof(*player));
    if (player == NULL)
        return false;
    player->dra = dra;
    (void)SotnPlayerAfterimage_Init(&player->afterimage);
    player->hp = player->hp_max = 70;
    player->mp = 20;
    player->mist_lockout = 16;
    player->animation_bank = 1;
    player->attributes =
        (SotnCharacterStats){.total = {6, 6, 6, 6}, .attack = {6, 6}, .unarmed_attack = 6};
    player->map = map;
    player->x.value = (Sint32)start_x;
    player->y.value = (Sint32)start_y;
    player->contact = player->previous_contact = 1;
    Land(player, 0);
    if (!SotnAnimation_Tick(dra, &player->animation))
    {
        SotnPlayer_Close(player);
        return false;
    }
    player->animation.remaining = 0;
    *out = player;
    return true;
}

bool SotnPlayer_SetEquipment(SotnPlayer *player, SotnDataView weapons, Uint16 left, Uint16 right)
{
    if (player == NULL || player->attack_mode != 0)
        return SDL_SetError("Equipment can change only when the player has no active attack");
    const Uint16 ids[2] = {left, right};
    Uint16 power[2] = {0};
    for (Uint32 i = 0; i < 2; ++i)
    {
        SotnEquipment equipment = {0};
        SotnDataView overlay = {0};
        if (!SotnEquipment_Read(player->dra, ids[i], &equipment) ||
            (ids[i] != 0 && !SotnWeapon_Overlay(weapons, equipment, &overlay)))
            return false;
        const Uint16 strength = player->attributes.total[0];
        power[i] = (Uint16)SDL_clamp(
            equipment.attack + (equipment.attack <= strength ? strength : strength / 2), 0, 999);
    }
    player->attributes.attack[0] = power[0];
    player->attributes.attack[1] = power[1];
    player->weapons = weapons;
    player->equipment[0] = left;
    player->equipment[1] = right;
    player->shield_cursor = (SotnShield){0};
    player->shield = (SotnPlayerAttack){0};
    player->shield_button = 0;
    return true;
}

bool SotnPlayer_ChangeRoom(SotnPlayer *player, SotnCollisionMap map, Sint32 offset_x,
                           Sint32 offset_y)
{
    if (player == NULL)
        return SDL_InvalidParamError("player");
    if (!ValidateMap(map))
        return false;
    const Sint64 x = player->x.value + (Sint64)offset_x * FIX_ONE;
    const Sint64 y = player->y.value + (Sint64)offset_y * FIX_ONE;
    if (!PositionValid(map, x, y))
        return SDL_SetError("Player room rebase is outside the new bounded room");
    SotnPlayerAfterimage afterimage = player->afterimage;
    if (player->afterimage_generation &&
        !SotnPlayerAfterimage_Rebase(&afterimage, offset_x, offset_y))
        return false;
    if (player->wolf.commands.count > SDL_arraysize(player->wolf.commands.requests))
        return SDL_SetError("Wolf room transfer lost its bounded command list");
    const Sint64 impact_x = (Sint64)player->gravity.impact_source.x + (Sint64)offset_x * FIX_ONE,
                 impact_y = (Sint64)player->gravity.impact_source.y + (Sint64)offset_y * FIX_ONE;
    if (player->gravity.ceiling_impact && (impact_x < SDL_MIN_SINT32 || impact_x > SDL_MAX_SINT32 ||
                                           impact_y < SDL_MIN_SINT32 || impact_y > SDL_MAX_SINT32))
        return SDL_SetError("Room rebase exceeds the captured Gravity Boots impact origin");
    const Sint64 smoke_x =
                     (Sint64)player->movement_effects.smoke_source.x + (Sint64)offset_x * FIX_ONE,
                 smoke_y =
                     (Sint64)player->movement_effects.smoke_source.y + (Sint64)offset_y * FIX_ONE;
    if (player->movement_effects.smoke && (smoke_x < SDL_MIN_SINT32 || smoke_x > SDL_MAX_SINT32 ||
                                           smoke_y < SDL_MIN_SINT32 || smoke_y > SDL_MAX_SINT32))
        return SDL_SetError("Room rebase exceeds the captured ordinary movement source");
    for (Uint16 i = 0; i < player->wolf.commands.count; ++i)
    {
        const Sint64 source_x = (Sint64)player->wolf.commands.requests[i].source.x +
                                (Sint64)offset_x * FIX_ONE,
                     source_y = (Sint64)player->wolf.commands.requests[i].source.y +
                                (Sint64)offset_y * FIX_ONE;
        if (source_x < SDL_MIN_SINT32 || source_x > SDL_MAX_SINT32 || source_y < SDL_MIN_SINT32 ||
            source_y > SDL_MAX_SINT32)
            return SDL_SetError("Wolf room transfer exceeds a captured command origin");
    }
    for (Uint32 i = 0; i < SOTN_PLAYER_GRAVITY_BEAMS; ++i)
    {
        const SotnPlayerGravityBeam b = player->gravity.beams[i];
        if (b.active && ((Sint64)b.x + (Sint64)offset_x * FIX_ONE < SDL_MIN_SINT32 ||
                         (Sint64)b.x + (Sint64)offset_x * FIX_ONE > SDL_MAX_SINT32 ||
                         (Sint64)b.y + (Sint64)offset_y * FIX_ONE < SDL_MIN_SINT32 ||
                         (Sint64)b.y + (Sint64)offset_y * FIX_ONE > SDL_MAX_SINT32))
            return SDL_SetError("Room rebase exceeds the owned Gravity Boots beam coordinates");
    }
    for (Uint32 i = 0; i < SOTN_PLAYER_GRAVITY_BEAMS; ++i)
        if (player->gravity.beams[i].active)
        {
            player->gravity.beams[i].x =
                (Sint32)((Sint64)player->gravity.beams[i].x + (Sint64)offset_x * FIX_ONE);
            player->gravity.beams[i].y =
                (Sint32)((Sint64)player->gravity.beams[i].y + (Sint64)offset_y * FIX_ONE);
        }
    player->map = map;
    player->afterimage = afterimage;
    if (player->movement_effects.smoke)
    {
        player->movement_effects.smoke_source.x = (Sint32)smoke_x;
        player->movement_effects.smoke_source.y = (Sint32)smoke_y;
    }
    if (player->gravity.ceiling_impact)
    {
        player->gravity.impact_source.x = (Sint32)impact_x;
        player->gravity.impact_source.y = (Sint32)impact_y;
    }
    for (Uint16 i = 0; i < player->wolf.commands.count; ++i)
    {
        player->wolf.commands.requests[i].source.x =
            (Sint32)((Sint64)player->wolf.commands.requests[i].source.x +
                     (Sint64)offset_x * FIX_ONE);
        player->wolf.commands.requests[i].source.y =
            (Sint32)((Sint64)player->wolf.commands.requests[i].source.y +
                     (Sint64)offset_y * FIX_ONE);
    }
    player->x.value = (Sint32)x;
    player->y.value = (Sint32)y;
    player->sleep = (SotnPlayerSleep){0};
    return true;
}

Uint32 SotnPlayer_Status(const SotnPlayer *player)
{
    Uint32 status = 0;
    if (player->bell_inside)
        status |= 0x100008U;
    if (player->curse_ticks)
        status |= 0x28108000U;
    if (player->poison_ticks)
        status |= 0x28104000U;
    if (player->step == SOTN_PLAYER_STAND || player->step == SOTN_PLAYER_WALK)
    {
        status |= 0x10000000U;
        if (player->step == SOTN_PLAYER_WALK)
            status |= 0x4000000U;
        if (player->substep == 4)
            status |= 0x8100000U;
    }
    if (player->step == SOTN_PLAYER_CROUCH)
        status |= 0x10000000U | (player->substep != 2 ? 0x20U : 0U);
    if (player->step == SOTN_PLAYER_FALL || player->step == SOTN_PLAYER_JUMP)
        status |= 0x10002000U;
    if (player->step == SOTN_PLAYER_HIT)
        status |= 0x38110000U;
    if (player->step == SOTN_PLAYER_KILL)
        status |= 0x18150000U | (player->substep == 0x80 ? 0x80000U : 0U);
    if (player->step == SOTN_PLAYER_STONE)
        status |= 0x38110080U;
    if (player->step == SOTN_PLAYER_BOSS_GRAB)
        status |= 0x38110040U;
    if (player->step == SOTN_PLAYER_SCRIPT)
        status |= 0x38110000U;
    if (player->step == SOTN_PLAYER_HIGH_JUMP)
        status |= 0x38000000U;
    if (player->step == SOTN_PLAYER_BAT)
        status |= 0x28100001U;
    if (player->step == SOTN_PLAYER_UNMORPH_BAT)
        status |= 0x28500001U;
    if (player->step == SOTN_PLAYER_MIST)
        status |= 0x28100002U;
    if (player->step == SOTN_PLAYER_UNMORPH_MIST)
        status |= 0x28900002U;
    if (player->step == SOTN_PLAYER_WOLF)
    {
        status |= 0x2c100204U;
        if (!player->substep || player->substep == 8)
            status |= 0x40000000U;
        if (player->substep == 3)
            status |= 0x20U;
    }
    if (player->step == SOTN_PLAYER_UNMORPH_WOLF)
        status |= 0x68100000U;
    if (player->step == SOTN_PLAYER_STUCK)
        status |= 0x18100030U;
    if (player->invincibility)
        status |= 0x100U;
    if (player->timers[0])
        status |= 0x800U;
    return status;
}

bool SotnPlayer_GetState(const SotnPlayer *player, SotnPlayerState *out)
{
    if (player == NULL || out == NULL)
        return SDL_SetError("Player state requires a simulation and output");
    const Uint32 status = SotnPlayer_Status(player);
    *out = (SotnPlayerState){
        .x = player->x.value,
        .y = player->y.value,
        .velocity_x = player->vx,
        .velocity_y = player->vy,
        .contacts = (Uint32)player->contact,
        .status = status,
        .animation = player->animation,
        .step = player->step,
        .substep = player->substep,
        .sleep_event = player->sleep_event,
        .sleep = player->sleep,
        .facing_left = player->facing_left,
        .attack = player->attack,
        .hp = player->hp,
        .hp_max = player->hp_max,
        .invulnerable = player->invincibility != 0 || player->step == SOTN_PLAYER_HIT ||
                        player->step == SOTN_PLAYER_KILL || player->grab.state != SOTN_GRAB_NONE,
        .sound = player->sound,
        .death = player->death,
        .looking_up = player->step == SOTN_PLAYER_STAND && player->substep == 1,
        .can_pause = player->attack_mode == 0 && player->step != SOTN_PLAYER_KILL &&
                     player->step != SOTN_PLAYER_HIT && player->step != SOTN_PLAYER_STONE &&
                     player->step != SOTN_PLAYER_SCRIPT && player->grab.state == SOTN_GRAB_NONE &&
                     player->step != SOTN_PLAYER_BOSS_GRAB,
        .random = player->damage_random,
        .solid_width = (Uint16)SDL_max(0, player->wall_sensors[0].x),
        .solid_height = (Uint16)SDL_max(0, player->floor_sensors[1].y - 1),
        .shield = player->shield,
        .subweapon_starting = player->timers[0] != 0,
        .weapon_starting = player->weapon_startup != 0,
        .stone = player->stone,
        .curse_ticks = player->curse_ticks,
        .poison_ticks = player->poison_ticks,
        .extra_sound = player->extra_sound,
        .grab = player->grab,
        .priority = player->wolf.active ? player->wolf.priority : 148,
        .kick = player->kick,
        .gravity = player->gravity,
        .mp = player->mp,
        .mp_spent = player->mp_spent,
        .mist = player->mist,
        .bat = player->bat,
        .wolf = player->wolf,
        .afterimage = player->afterimage,
        .afterimage_generation = player->afterimage_generation,
        .movement_effects = player->movement_effects,
        .bat_commands = player->bat_commands,
        .animation_bank = player->animation_bank,
        .rotated = player->form_rotated,
        .draw_pivot_x = player->draw_pivot_x,
        .draw_pivot_y = player->draw_pivot_y};
    if (player->wolf.active)
    {
        out->wolf.controller.x =
            (Sint32)((Sint64)player->x.value - (Sint64)player->wolf_camera_x * 65536);
        out->wolf.controller.y =
            (Sint32)((Sint64)player->y.value - (Sint64)player->wolf_camera_y * 65536);
        out->wolf.controller.motion.animation = player->animation;
        out->wolf.controller.motion.velocity_x = player->vx;
        out->wolf.controller.motion.velocity_y = player->vy;
        out->wolf.controller.motion.contacts = (Uint32)player->contact;
        out->wolf.controller.motion.substep = player->substep;
        out->wolf.controller.motion.facing_left = player->facing_left;
        out->wolf.controller.mp = player->mp;
        out->wolf.controller.priority = player->wolf.priority;
    }
    return true;
}

bool SotnPlayer_SetArrivalFacing(SotnPlayer *player, bool facing_left)
{
    if (player == NULL)
        return SDL_InvalidParamError("player");
    player->facing_left = facing_left;
    return true;
}

bool SotnPlayer_Damage(SotnPlayer *player, Uint16 amount, Sint32 source_x, Sint32 source_velocity)
{
    if (player == NULL || amount > 9999 || source_x < -512 || source_x > 16896 ||
        source_velocity < -16 * FIX_ONE || source_velocity > 16 * FIX_ONE)
        return SDL_SetError("Player damage requires a bounded physical attack and source");
    if (amount == 0 || player->hp == 0 || player->invincibility != 0 ||
        player->step == SOTN_PLAYER_HIT || player->grab.state != SOTN_GRAB_NONE ||
        player->step == SOTN_PLAYER_BOSS_GRAB)
        return true;
    if (player->step == SOTN_PLAYER_STONE)
        return SotnPlayer_Petrify(player, amount, source_x, source_velocity);
    SotnPlayer next = *player;
    SotnPlayer_GravityCancel(&next);
    SotnPlayer_BatCancel(&next);
    SotnPlayer_WolfCancel(&next);
    next.hp = (Uint16)(amount >= next.hp ? 0 : next.hp - amount);
    const Sint32 center_x =
        next.x.parts.high + (next.facing_left ? -next.animation.hitbox_x : next.animation.hitbox_x);
    const Sint32 delta = center_x - source_x;
    const bool right =
        SDL_abs(delta) < 16 && source_velocity != 0 ? source_velocity > 0 : delta >= 0;
    ClearAttack(&next);
    next.movement_flags = 0;
    next.invincibility = 12;
    if (next.hp == 0)
    {
        PhysicalDeath(&next, right);
        if (next.failed || !SotnAnimation_Tick(next.dra, &next.animation))
            return false;
        *player = next;
        return true;
    }
    const bool strong = (Uint32)amount * 20 > next.hp_max;
    const Uint16 pain = (Uint16)((SotnCombat_Random(&next.damage_random) & 1) + (strong ? 3 : 5));
    const Uint8 *sound = NULL;
    if (!AttackView(&next, 0x800acf84 + pain * 2U, 2, &sound))
        return false;
    next.sound = Attack16(sound);
    Uint16 animation = 0;
    if (strong || next.step == SOTN_PLAYER_JUMP || next.step == SOTN_PLAYER_FALL ||
        next.step == SOTN_PLAYER_HIGH_JUMP || next.step == SOTN_PLAYER_BAT)
    {
        next.vx = strong ? 109226 : 81920;
        next.vy = strong ? ((next.step == SOTN_PLAYER_JUMP || next.step == SOTN_PLAYER_FALL)
                                ? -3 * FIX_ONE
                                : -4 * FIX_ONE)
                         : -2 * FIX_ONE;
        const Uint16 value = SotnCombat_Random(&next.damage_random);
        animation = (Uint16)(0x2e + value % 3);
        if (strong && value % 16 == 0)
            animation = 0x40;
        next.substep = 1;
    }
    else
    {
        next.vx = 163840;
        next.vy = 0;
        if (next.step == SOTN_PLAYER_CROUCH)
        {
            animation = 35;
            next.substep = 7;
        }
        else
        {
            animation = (Uint16)(0x31 + (SotnCombat_Random(&next.damage_random) & 1) +
                                 (right != next.facing_left ? 2 : 0));
            next.substep = 6;
        }
    }
    if (!right)
        next.vx = -next.vx;
    next.step = SOTN_PLAYER_HIT;
    Animate(&next, animation);
    if (next.failed || !SotnAnimation_Tick(next.dra, &next.animation))
        return false;
    *player = next;
    return true;
}

bool SotnPlayer_Restore(SotnPlayer *player, Uint16 hp, Uint32 random)
{
    if (player == NULL || hp == 0 || hp > player->hp_max || player->step == SOTN_PLAYER_KILL ||
        player->attack_mode != 0)
        return SDL_SetError("Health restore requires a living, non-attacking player and health "
                            "within the current maximum");
    player->hp = hp;
    player->damage_random = random;
    return true;
}

bool SotnPlayer_SetCharacter(SotnPlayer *player, const SotnCharacterState *state,
                             SotnCharacterStats stats)
{
    if (player == NULL || state == NULL || state->hp_max == 0 || state->hp_max > 9999 ||
        state->hp > state->hp_max || state->mp > state->mp_max || state->mp_max > 9999 ||
        stats.attack[0] > 999 || stats.attack[1] > 999 || stats.unarmed_attack > 999 ||
        player->step == SOTN_PLAYER_KILL)
        return SDL_SetError(
            "Player character update requires bounded living progress and attributes");
    player->hp = state->hp;
    player->hp_max = state->hp_max;
    player->mp = state->mp;
    player->attributes = stats;
    return true;
}

void SotnPlayer_Close(SotnPlayer *player)
{
    SDL_free(player);
}

bool SotnPlayer_SetSubweapons(SotnPlayer *player, SotnSubweapons *service)
{
    if (player == NULL)
        return SDL_InvalidParamError("player");
    player->subweapons = service;
    return true;
}

bool SotnPlayer_SetRelics(SotnPlayer *player, const SotnWorldState *world)
{
    if (player == NULL || world == NULL)
        return SDL_SetError("Player relic update requires a simulation and persistent relics");
    for (Uint32 i = 0; i < SOTN_RELICS; ++i)
        if (world->relics[i] > 3 || world->relics[i] == 2)
            return SDL_SetError("Player relic %u has invalid found/active bits", i);
    SDL_memcpy(player->relics, world->relics, sizeof(player->relics));
    return true;
}

bool SotnPlayer_Displace(SotnPlayer *p, Sint32 dx, Sint32 dy, Uint32 contacts)
{
    if (p == NULL || dx < -64 * 65536 || dx > 64 * 65536 || dy < -64 * 65536 || dy > 64 * 65536 ||
        (contacts & ~0x4fU) != 0)
        return SDL_SetError("Stage-object displacement exceeds the bounded physical contacts");
    const Sint64 x = (Sint64)p->x.value + dx, y = (Sint64)p->y.value + dy;
    if (!PositionValid(p->map, x, y))
        return SDL_SetError("Stage object pushed Alucard outside the room margin");
    p->x.value = (Sint32)x;
    p->y.value = (Sint32)y;
    p->contact |= (Sint32)contacts;
    return true;
}

bool SotnPlayer_StageStop(SotnPlayer *p)
{
    if (p == NULL)
        return SDL_InvalidParamError("player");
    if (p->step != SOTN_PLAYER_KILL)
        p->vx = p->vy = 0;
    return true;
}

bool SotnPlayer_StageStopHorizontal(SotnPlayer *p)
{
    if (p == NULL)
        return SDL_InvalidParamError("player");
    p->vx = 0;
    return true;
}

bool SotnPlayer_StageGuard(SotnPlayer *p, bool halt_vertical)
{
    if (p == NULL)
        return SDL_InvalidParamError("player");
    if (p->step == SOTN_PLAYER_KILL)
        return true;
    if (halt_vertical)
        p->vy = 0;
    p->invincibility = (Uint16)SDL_max(p->invincibility, 32);
    return true;
}

bool SotnPlayer_BellInside(SotnPlayer *player, bool inside)
{
    if (!player)
        return SDL_InvalidParamError("player");
    player->bell_inside = inside;
    return true;
}

// Selected status timer and hit ordering from dra/{5D5BC,71830,692E8,6E42C}.c,
// pinned reconstruction. Native adapters retain the transient timer across stage arrivals.
bool SotnPlayer_RestoreCurse(SotnPlayer *player, Uint16 ticks)
{
    if (player == NULL || ticks > 1024 || player->step == SOTN_PLAYER_KILL)
        return SDL_SetError("Curse timer restore requires a living player and 0..1024 ticks");
    player->curse_ticks = ticks;
    return true;
}
bool SotnPlayer_CurseDamage(SotnPlayer *player, Uint16 amount, Sint32 source_x,
                            Sint32 source_velocity)
{
    if (player == NULL || amount > 9999 || source_x < -512 || source_x > 16896 ||
        source_velocity < -16 * FIX_ONE || source_velocity > 16 * FIX_ONE)
        return SDL_SetError("Curse contact requires bounded damage and source motion");
    if (!amount || !player->hp || player->invincibility || player->step == SOTN_PLAYER_HIT ||
        player->step == SOTN_PLAYER_KILL || player->step == SOTN_PLAYER_BOSS_GRAB ||
        player->grab.state != SOTN_GRAB_NONE)
        return true;
    if (player->step == SOTN_PLAYER_STONE)
        return SotnPlayer_Damage(player, amount, source_x, source_velocity);
    SotnPlayer next = *player;
    if (!SotnPlayer_Damage(&next, amount, source_x, source_velocity))
        return false;
    if (next.hp)
    {
        next.curse_ticks = (Uint16)SDL_max(64, 1024 - (Sint32)next.attributes.total[1] * 4);
        next.extra_sound = 0x6f2;
    }
    *player = next;
    return true;
}

bool SotnPlayer_SetScriptHold(SotnPlayer *player, bool hold)
{
    if (player == NULL ||
        (hold && (player->step == SOTN_PLAYER_KILL || player->step == SOTN_PLAYER_BOSS_GRAB ||
                  player->grab.state != SOTN_GRAB_NONE)))
        return SDL_SetError("Script hold requires a living normal-form player owner");
    player->script_hold = hold;
    return true;
}

bool SotnPlayer_StageAlignX(SotnPlayer *p, Sint32 x)
{
    if (!p)
        return SDL_InvalidParamError("player");
    const Sint64 target = (Sint64)x * FIX_ONE + p->x.parts.low;
    if (!PositionValid(p->map, target, p->y.value))
        return SDL_SetError("Script horizontal alignment exceeds the bounded player room");
    p->x.value = (Sint32)target;
    return true;
}
bool SotnPlayer_StageAlignY(SotnPlayer *p, Sint32 y)
{
    if (!p)
        return SDL_InvalidParamError("player");
    const Sint64 target = (Sint64)y * FIX_ONE + p->y.parts.low;
    if (!PositionValid(p->map, p->x.value, target))
        return SDL_SetError("Boss vertical alignment exceeds the bounded player room");
    p->y.value = (Sint32)target;
    return true;
}

bool SotnPlayer_RequestGrab(SotnPlayer *p, SotnPlayerGrab request)
{
    if (!p || request.damage > 9999 || request.source_x < -512 || request.source_x > 16896 ||
        request.source_velocity < -16 * FIX_ONE || request.source_velocity > 16 * FIX_ONE ||
        (request.state != SOTN_GRAB_NONE && request.state != SOTN_GRAB_START &&
         request.state != SOTN_GRAB_DAMAGE))
        return SDL_SetError("Boss grab requires a supported request and bounded damage/source");
    if (request.state == SOTN_GRAB_NONE)
    {
        p->grab.state = SOTN_GRAB_NONE;
        return true;
    }
    if (!p->hp || p->step == SOTN_PLAYER_KILL || p->script_hold || p->step == SOTN_PLAYER_SCRIPT ||
        p->step == SOTN_PLAYER_STONE)
        return SDL_SetError("Boss grab requires a living normal-form player outside a script");
    if ((request.state == SOTN_GRAB_START &&
         (p->grab.state != SOTN_GRAB_NONE || p->step == SOTN_PLAYER_BOSS_GRAB)) ||
        (request.state == SOTN_GRAB_DAMAGE &&
         (p->step != SOTN_PLAYER_BOSS_GRAB || p->grab.state != SOTN_GRAB_HELD)))
        return SDL_SetError("Boss grab request does not match the player's acknowledgement");
    p->grab = request;
    return true;
}
bool SotnPlayer_SetChairKind(SotnPlayer *player, Uint16 kind)
{
    if (player == NULL || kind > 4)
        return SDL_SetError("Chair hint requires a player and original pose kind 0..4");
    player->chair_kind = kind;
    return true;
}

// dra/71830.c applies poison after the normal surviving hit; curse takes precedence in scene.
bool SotnPlayer_RestorePoison(SotnPlayer *player, Uint16 ticks)
{
    if (!player || ticks > 4095 || player->step == SOTN_PLAYER_KILL)
        return SDL_SetError("Poison timer restore requires a living player and 0..4095 ticks");
    player->poison_ticks = ticks;
    return true;
}
bool SotnPlayer_PoisonDamage(SotnPlayer *player, Uint16 amount, Sint32 source_x,
                             Sint32 source_velocity)
{
    if (!player || amount > 9999 || source_x < -512 || source_x > 16896 ||
        source_velocity < -16 * FIX_ONE || source_velocity > 16 * FIX_ONE)
        return SDL_SetError("Poison contact requires bounded damage and source motion");
    if (!amount || !player->hp || player->invincibility || player->step == SOTN_PLAYER_HIT ||
        player->step == SOTN_PLAYER_KILL || player->step == SOTN_PLAYER_BOSS_GRAB ||
        player->grab.state != SOTN_GRAB_NONE)
        return true;
    if (player->step == SOTN_PLAYER_STONE)
        return SotnPlayer_Damage(player, amount, source_x, source_velocity);
    SotnPlayer next = *player;
    if (!SotnPlayer_Damage(&next, amount, source_x, source_velocity))
        return false;
    if (next.hp)
    {
        next.poison_ticks = (Uint16)SDL_max(256, 4095 - (Sint32)next.attributes.total[1] * 16);
        next.extra_sound = 0x6f2;
    }
    *player = next;
    return true;
}

bool SotnPlayer_SetUseTimer(SotnPlayer *p, Uint8 ticks)
{
    if (!p || ticks > 32)
        return SDL_SetError("Player use timer requires its owner and bounded ticks");
    p->timers[0] = ticks;
    return true;
}
