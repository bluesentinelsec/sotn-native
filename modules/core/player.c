// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted normal-form movement paths from sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5:
// src/dra/6DF70.c, 6E42C.c, 6FDF8.c, 71830.c, 692E8.c,
// and selected unarmed paths in 7879C.c and alu_anim.c.
#include "player_internal.h"
#include <sotn/combat.h>

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

static void Land(SotnPlayer *player, Sint32 speed)
{
    const bool ledge = (player->contact & 0x20) != 0;
    player->vx = speed;
    player->vy = 0;
    SetStep(player, SOTN_PLAYER_STAND);
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

static void Walk(SotnPlayer *player, bool force)
{
    const bool ledge = (player->contact & 0x20) != 0;
    SetSpeed(player, WALK_SPEED);
    player->vy = 0;
    SetStep(player, SOTN_PLAYER_WALK);
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
    player->attack_mode = 0;
    player->attack = (SotnPlayerAttack){0};
}

// Selected empty-hand paths in 6E42C.c, 71830.c, and EntityUnarmedAttack.
static bool StartAttack(SotnPlayer *player)
{
    const Uint16 pressed = player->tapped & (SOTN_BUTTON_LEFT_HAND | SOTN_BUTTON_RIGHT_HAND);
    if (pressed == 0 || (player->attack_mode & 0x8000))
        return false;
    const Uint32 hand = pressed == SOTN_BUTTON_RIGHT_HAND ? 1U : 0U;
    SotnEquipment equipment = {0};
    if (!SotnEquipment_Read(player->dra, player->equipment[hand], &equipment))
    {
        player->failed = true;
        return true;
    }
    player->attack_equipment = equipment;
    Sint32 critical = equipment.critical_rate;
    if (equipment.id != 0)
    {
        const Uint16 value = (Uint16)(12 + (SotnCombat_Random(&player->damage_random) & 15));
        Sint32 root = 0;
        while ((root + 1) * (root + 1) <= value)
            ++root;
        critical += root - 5;
    }
    player->attack_critical = (Uint8)SDL_clamp(critical, 0, 255);
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
    player->attack_button =
        pressed == SOTN_BUTTON_RIGHT_HAND ? SOTN_BUTTON_RIGHT_HAND : SOTN_BUTTON_LEFT_HAND;
    return true;
}

static bool ReadAttack(SotnPlayer *player)
{
    player->attack = (SotnPlayerAttack){0};
    if (player->attack_mode != 0 && player->attack_equipment.id != 0)
    {
        SotnWeaponFrame weapon = {0};
        const SotnEquipment equipment = player->attack_equipment;
        if (!SotnWeapon_ReadFrame(player->weapons, equipment, player->animation.id,
                                  player->animation.pose, player->animation.remaining, &weapon))
            return false;
        player->attack = (SotnPlayerAttack){
            .offset_x = weapon.x,
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
            .power = (Uint16)SDL_clamp((Sint32)equipment.attack + (equipment.attack <= 6 ? 6 : 3),
                                       0, 999),
            .equipment = equipment.id,
            .sprite = weapon.sprite};
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
    player->attack.power = 6;
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
        if (player->vy > 450560)
            Crouch(player, 1, 0);
        else if (player->movement_flags & 0x10)
            Walk(player, true);
        else
            Land(player, 0);
        ClearAttack(player);
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
    if ((flags & 0x2000) && !(player->attack_mode & 0x8000) && (player->buttons & SOTN_BUTTON_DOWN))
    {
        Crouch(player, 2, 0);
        ClearAttack(player);
        return true;
    }
    return false;
}

static void StandTick(SotnPlayer *player)
{
    if (Transitions(player, 0x4301c))
        return;
    Decelerate(player, 8192);
    if (player->attack_mode != 0)
    {
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
    if (player->substep == 1 && !(player->buttons & SOTN_BUTTON_UP))
        Land(player, 0);
    if ((player->buttons & SOTN_BUTTON_UP) && player->substep == 0)
    {
        Animate(player, (player->contact & 0x20) ? 1 : 0);
        player->substep = 1;
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
    if (player->attack_mode != 0)
    {
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
    if (player->attack_mode != 0)
    {
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
        if (player->animation.remaining < 0)
            flags = 0x20;
        break;
    case 5:
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

bool SotnPlayer_Tick(SotnPlayer *player, Uint16 buttons)
{
    const Uint16 supported = SOTN_BUTTON_UP | SOTN_BUTTON_RIGHT | SOTN_BUTTON_DOWN |
                             SOTN_BUTTON_LEFT | SOTN_BUTTON_JUMP | SOTN_BUTTON_LEFT_HAND |
                             SOTN_BUTTON_RIGHT_HAND;
    if (player == NULL || (buttons & (Uint16)~supported) != 0)
        return SDL_SetError("This player slice supports normal movement and empty-hand attacks; "
                            "abilities are not integrated");
    SotnPlayer next = *player;
    next.failed = false;
    next.turned = false;
    next.sound = 0;
    next.buttons = buttons;
    next.tapped = (Uint16)(buttons & (Uint16)~next.previous_buttons);
    next.previous_buttons = buttons;
    for (size_t i = 0; i < SDL_arraysize(next.timers); ++i)
        if (next.timers[i] != 0)
            --next.timers[i];
    next.tight_space = SotnPlayer_TightSpace(&next);
    if (next.failed)
        return false;
    next.previous_step = next.step;
    if (next.invincibility)
        --next.invincibility;
    switch (next.step)
    {
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
    if (next.failed || !SotnAnimation_Tick(next.dra, &next.animation) || !ReadAttack(&next))
        return false;
    SotnPlayer_Collide(&next);
    if (next.failed)
        return false;
    if (!PositionValid(next.map, next.x.value, next.y.value))
        return SDL_SetError("Player left the bounded room region without a supported transition");
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
    player->hp = 70;
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
    for (Uint32 i = 0; i < 2; ++i)
    {
        SotnEquipment equipment = {0};
        SotnDataView overlay = {0};
        if (!SotnEquipment_Read(player->dra, ids[i], &equipment) ||
            (ids[i] != 0 && !SotnWeapon_Overlay(weapons, equipment, &overlay)))
            return false;
    }
    player->weapons = weapons;
    player->equipment[0] = left;
    player->equipment[1] = right;
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
    player->map = map;
    player->x.value = (Sint32)x;
    player->y.value = (Sint32)y;
    return true;
}

bool SotnPlayer_GetState(const SotnPlayer *player, SotnPlayerState *out)
{
    if (player == NULL || out == NULL)
        return SDL_SetError("Player state requires a simulation and output");
    *out = (SotnPlayerState){player->x.value,
                             player->y.value,
                             player->vx,
                             player->vy,
                             (Uint32)player->contact,
                             player->animation,
                             player->step,
                             player->facing_left,
                             player->attack,
                             player->hp,
                             70,
                             player->invincibility != 0 || player->step == SOTN_PLAYER_HIT,
                             player->sound};
    return true;
}

bool SotnPlayer_Damage(SotnPlayer *player, Uint16 amount, Sint32 source_x, Sint32 source_velocity)
{
    if (player == NULL || amount > 9999 || source_x < -512 || source_x > 16896 ||
        source_velocity < -16 * FIX_ONE || source_velocity > 16 * FIX_ONE)
        return SDL_SetError("Player damage requires a bounded physical attack and source");
    if (amount == 0 || player->invincibility != 0 || player->step == SOTN_PLAYER_HIT)
        return true;
    if (player->hp <= amount)
    {
        player->hp = 0;
        return SDL_SetError("Alucard died; this development checkpoint has no death/continue flow");
    }
    SotnPlayer next = *player;
    next.hp = (Uint16)(next.hp - amount);
    const Sint32 center_x =
        next.x.parts.high + (next.facing_left ? -next.animation.hitbox_x : next.animation.hitbox_x);
    const Sint32 delta = center_x - source_x;
    const bool right =
        SDL_abs(delta) < 16 && source_velocity != 0 ? source_velocity > 0 : delta >= 0;
    ClearAttack(&next);
    next.movement_flags = 0;
    next.invincibility = 12;
    const bool strong = (Uint32)amount * 20 > 70;
    const Uint16 pain = (Uint16)((SotnCombat_Random(&next.damage_random) & 1) + (strong ? 3 : 5));
    const Uint8 *sound = NULL;
    if (!AttackView(&next, 0x800acf84 + pain * 2U, 2, &sound))
        return false;
    next.sound = Attack16(sound);
    Uint16 animation = 0;
    if (strong || next.step == SOTN_PLAYER_JUMP || next.step == SOTN_PLAYER_FALL)
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

void SotnPlayer_Close(SotnPlayer *player)
{
    SDL_free(player);
}
