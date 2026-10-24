// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted normal-form movement paths from sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5:
// src/dra/6DF70.c, 6E42C.c, 6FDF8.c, 71830.c, and 692E8.c.
#include "player_internal.h"

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

static bool Transitions(SotnPlayer *player, Uint32 flags)
{
    if (flags & 8)
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
        return true;
    }
    if ((flags & 4) && !(player->contact & 1))
    {
        Fall(player);
        return true;
    }
    if ((flags & 0x10) && (player->tapped & SOTN_BUTTON_JUMP))
    {
        Jump(player);
        return true;
    }
    if ((flags & 0x2000) && (player->buttons & SOTN_BUTTON_DOWN))
    {
        Crouch(player, 2, 0);
        return true;
    }
    return false;
}

static void StandTick(SotnPlayer *player)
{
    if (Transitions(player, 0x4301c))
        return;
    Decelerate(player, 8192);
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
    if (player->tapped & SOTN_BUTTON_JUMP)
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
            return;
        }
    }
    if (Transitions(player, 0x100c))
        return;
    Decelerate(player, 8192);
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
    const Uint16 supported =
        SOTN_BUTTON_UP | SOTN_BUTTON_RIGHT | SOTN_BUTTON_DOWN | SOTN_BUTTON_LEFT | SOTN_BUTTON_JUMP;
    if (player == NULL || (buttons & (Uint16)~supported) != 0)
        return SDL_SetError(
            "This player slice supports normal movement; combat and abilities are not integrated");
    SotnPlayer next = *player;
    next.failed = false;
    next.turned = false;
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
            Jump(&next);
        else if (!Transitions(&next, 0x9029))
        {
            Decelerate(&next, 4096);
            if (Direction(&next))
                SetSpeed(&next, 49152);
        }
        break;
    default:
        return SDL_SetError("Unsupported Alucard movement state");
    }
    if (next.failed || !SotnAnimation_Tick(next.dra, &next.animation))
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
    *out = (SotnPlayerState){player->x.value, player->y.value,         player->vx,
                             player->vy,      (Uint32)player->contact, player->animation,
                             player->step,    player->facing_left};
    return true;
}

void SotnPlayer_Close(SotnPlayer *player)
{
    SDL_free(player);
}
