// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/dai/e_winged_guard.c, pinned sotn-decomp 7bc54d8157b6.
// Animation, velocity and spawning-range arrays remain private runtime inputs.
#include "actors_internal.h"
static bool Move(Actor *a)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Chapel flight exceeds bounded fixed-point motion");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    return true;
}
static bool WingedGuard(SotnActors *s, Actor *a)
{
    if (a->state.flags & 0x100U)
    {
        if (!ActorPositionalSound(s, a, 0x62b) || !ActorExplosionInPool(s, a, 2, 160, 192, 0))
            return false;
        for (Uint16 i = 0; i < 8; ++i)
        {
            Actor *part = NULL;
            if (!ActorSpawn(s, a, 0x36, i, 160, 192, &part))
                return false;
            if (part)
            {
                part->state.x = Pixel(a->state.x) * 65536;
                part->state.y = Pixel(a->state.y) * 65536;
                part->vx = a->vx;
                part->vy = a->vy;
                part->state.priority = a->state.priority;
            }
        }
        a->remove = true;
        return true;
    }
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0xa40))
            return false;
        a->state.hitbox_y = 2;
        a->state.facing_left = Side(a, s->player) == 0;
        a->vx = a->state.facing_left ? 65536 : -65536;
    }
    if (a->state.step != 1)
        return SDL_SetError("Chapel Winged Guard entered an invalid original state");
    Uint8 animation = 0;
    SotnMathTrig trig = {0};
    if (!ActorAnimate(s, a, 0x1ec0, &animation) ||
        (a->remaining == 0 && a->pose == 2 && !ActorPositionalSound(s, a, 0x68c)) || !Move(a) ||
        !SotnMath_Trig(s->executable, a->state.rotation, &trig))
        return false;
    a->vy = trig.sine * 4;
    a->state.rotation = (Sint16)Wrap16((Sint64)a->state.rotation + 16);
    return true;
}
static bool Parts(SotnActors *s, Actor *a)
{
    if (a->state.parameters >= 8)
        return SDL_SetError("Chapel Winged Guard part index is invalid");
    if (!a->state.step)
    {
        SotnDataView velocity = {0};
        if (!Slice(s->nz0, 0x1ed0U + a->state.parameters * 8U, 8, &velocity) ||
            !ActorInitialize(s, a, 0xa4c))
            return false;
        const Sint32 dx = Signed32(velocity.bytes), dy = Signed32(velocity.bytes + 4);
        if (dx < -65536 || dx > 65536 || dy < -131072 || dy > 131072)
            return SDL_SetError("Chapel Winged Guard debris velocity exceeds its selected range");
        const Sint64 vx = (Sint64)a->vx + (a->state.facing_left ? dx : -dx) +
                          (Side(a, s->player) ? 32768 : -32768),
                     vy = (Sint64)a->vy + dy;
        if (vx < SDL_MIN_SINT32 || vx > SDL_MAX_SINT32 || vy < SDL_MIN_SINT32 ||
            vy > SDL_MAX_SINT32)
            return SDL_SetError("Chapel Winged Guard debris launch exceeds fixed-point bounds");
        a->vx = (Sint32)vx;
        a->vy = (Sint32)vy;
        a->state.flags |= 0x40000000U;
        a->state.frame = (Uint16)(a->state.parameters + 5U);
        a->state.rotated = a->state.parameters == 0;
        return true;
    }
    if (a->state.step != 1 || a->vy > SDL_MAX_SINT32 - 12288 || !Move(a))
        return SDL_SetError("Chapel Winged Guard debris entered invalid bounded motion");
    a->state.rotation = (Sint16)Wrap16((Sint64)a->state.rotation + 128);
    a->vy += 12288;
    return (ActorRandom(s) & 63U) != 0 || ActorExplosionInPool(s, a, 0, 160, 192, 0);
}
static bool Spawner(SotnActors *s, Actor *a)
{
    if (a->state.parameters > 2)
        return SDL_SetError("Chapel Winged Guard spawn range index is invalid");
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x95c))
            return false;
        a->timer = 1;
    }
    if (a->state.step != 1 || !a->timer)
        return SDL_SetError("Chapel Winged Guard spawner timer/state is invalid");
    if (--a->timer)
        return true;
    a->timer = (Uint16)((ActorRandom(s) & 47U) + 192U);
    const Sint32 x = Pixel(s->player.x), y = Pixel(s->player.y);
    if (a->state.parameters)
    {
        SotnDataView range = {0};
        if (!Slice(s->nz0, 0x1f10U + (a->state.parameters - 1U) * 8U, 8, &range))
            return false;
        const Sint32 left = Signed16(range.bytes), right = Signed16(range.bytes + 2),
                     top = Signed16(range.bytes + 4), bottom = Signed16(range.bytes + 6);
        if (left < 0 || top < 0 || left > right || top > bottom)
            return SDL_SetError("Chapel Winged Guard has an inverted private spawn range");
        if (x < left || x > right || y < top || y > bottom)
            return true;
    }
    Actor *child = NULL;
    if (!ActorSpawn(s, a, 0x35, 0, 104, 108, &child))
        return false;
    if (child)
    {
        const Sint32 offset_x = (ActorRandom(s) & 1U) ? 288 : -32;
        const Sint32 offset_y = (Sint32)(ActorRandom(s) & 127U) - 64;
        child->state.priority = 176;
        child->state.x = (s->camera_x + offset_x) * 65536;
        child->state.y = (y + offset_y) * 65536;
    }
    return true;
}
bool ActorChapel(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 6)
        return SDL_SetError("Chapel actor dispatch requires its original stage owner");
    switch (a->state.id)
    {
    case 1:
    case 0x44:
        return ActorChapelBreakable(s, a);
    case 8:
    case 0x11:
        return ActorRoomDecoration(s, a);
    case 0x19:
    case 0x1a:
    case 0x21:
        return ActorChapelMechanism(s, a);
    case 0x1c:
    case 0x1d:
    case 0x1e:
        return ActorChapelScenery(s, a);
    case 0x12:
        return ActorCamera(s, a);
    case 0x16:
        return ActorChapelExit(s, a);
    case 6:
        return ActorIntenseExplosion(s, a);
    case 0x14:
    case 0x15:
        return ActorOuterEffect(s, a);
    case 0x3f:
    case 0x40:
    case 0x41:
        return ActorChapelSpectral(s, a);
    case 0x2e:
    case 0x2f:
    case 0x30:
    case 0x31:
        return ActorChapelSpikes(s, a);
    case 0x1b:
    case 0x26:
        return ActorChapelBell(s, a);
    case 0x3d:
    case 0x3e:
        return ActorChapelHunting(s, a);
    case 0x2c:
        return ActorChapelSpikeBall(s, a);
    case 0x29:
    case 0x2a:
    case 0x2b:
        return ActorChapelPillar(s, a);
    case 0x27:
    case 0x28:
        return ActorChapelCornerGuard(s, a);
    case 0x35:
        return WingedGuard(s, a);
    case 0x32:
    case 0x33:
    case 0x34:
        return ActorChapelHalberd(s, a);
    case 0x36:
        return Parts(s, a);
    case 0x37:
        return ActorBat(s, a);
    case 0x38:
    case 0x39:
        return ActorChapelCrow(s, a);
    case 0x3a:
    case 0x3b:
    case 0x3c:
        return ActorSkelerang(s, a);
    case 0x42:
        return Spawner(s, a);
    default:
        a->state.unsupported = true;
        return true;
    }
}
