// SPDX-License-Identifier: AGPL-3.0-or-later
// Original BO4 entry/event/plate/block-door flow from pinned sotn-decomp
// bo4/doors.c and unk_365FC.c. Private timing/art/tile profiles stay in BO4.
#include "actors_internal.h"
static Sint32 Add(Sint32 value, Sint32 delta)
{
    const Uint32 bits = (Uint32)value + (Uint32)delta;
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static void Position(Actor *a, Sint32 x, Sint32 y)
{
    a->state.x = Wrap16(x) * 65536 + (Sint32)((Uint32)a->state.x & 65535U);
    a->state.y = Wrap16(y) * 65536 + (Sint32)((Uint32)a->state.y & 65535U);
}
bool ActorDoppleTrigger(SotnActors *s, Uint32 slot)
{
    if (slot >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("BO4 entry successor exceeds its reserved stage slots");
    // The original last door writes to an empty successor. No executable state observes it.
    if (s->slots[slot].state.generation)
        s->slots[slot].arena.triggered = true;
    return true;
}
static bool Manager(SotnActors *s, Actor *a)
{
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x3d4) || (Uint32)a->slot + 12 >= SOTN_STAGE_ACTOR_SLOTS)
            return SDL_SetError("BO4 entry manager lacks its original reserved effect slots");
        a->state.animation_set = a->state.frame = 0;
        for (Uint16 i = 1; i < 6; ++i)
        {
            Actor *plate = NULL;
            if (!ActorSpawnAt(s, a, 0x17, (Uint16)(i + 0x100U), (Uint32)a->slot + i * 2U - 1,
                              &plate) ||
                !ActorSpawnAt(s, a, 0x17, i, (Uint32)a->slot + i * 2U, &plate))
                return false;
        }
        {
            Actor *effect = NULL;
            if (!ActorSpawnAt(s, a, 0x19, 0, (Uint32)a->slot + 11, &effect) ||
                !ActorSpawnAt(s, a, 0x1a, 0, (Uint32)a->slot + 12, &effect))
                return false;
        }
        break;
    case 1:
        if (Distance(a, s->player) < 64)
        {
            if (s->player.status & 7U)
                return SDL_SetError(
                    "BO4 opening requires the unintegrated player unmorph controls");
            s->control = (SotnActorControl){.timer = 576};
            a->state.step = 3;
        }
        break;
    case 2:
        if (!a->arena.timer)
            return SDL_SetError("BO4 entry wait has an invalid timer");
        if (--a->arena.timer == 0)
            ++a->state.step;
        break;
    case 3:
        if (!ActorDoppleTrigger(s, (Uint32)a->slot + 1) ||
            !ActorDoppleTrigger(s, (Uint32)a->slot + 2))
            return false;
        ++a->state.step;
        break;
    case 4:
        if (s->boss_flags & 2U)
        {
            const Uint32 seconds = s->game_seconds;
            if (!s->dopple.record_time)
                s->dopple.record_time =
                    seconds % 60U + (seconds / 60U % 60U) * 100U + (seconds / 3600U) * 10000U;
            ++a->state.step;
        }
        break;
    case 5: {
        SotnDoppleBody *body = &s->dopple.body;
        const Sint32 dx = Pixel(a->state.x) - Pixel(body->motion.x);
        const Sint32 dy = Pixel(a->state.y) - Pixel(body->motion.y);
        const Sint64 squared = (Sint64)dx * dx + (Sint64)dy * dy;
        Sint16 angle = 0;
        Uint32 radius = 0;
        SotnMathTrig trig = {0};
        if (squared > SDL_MAX_SINT32 || !SotnMath_Atan2(s->executable, -dy, dx, &angle) ||
            !SotnMath_Trig(s->executable, angle, &trig) ||
            !SotnMath_Sqrt12(s->executable, (Uint32)squared, &radius))
            return false;
        body->motion.x = Add(body->motion.x, trig.cosine * 16);
        body->motion.y = Add(body->motion.y, -trig.sine * 16);
        s->slots[0].state.x = body->motion.x;
        s->slots[0].state.y = body->motion.y;
        if (radius / 64U < 8)
            ++a->state.step;
        break;
    }
    case 6:
        s->slots[0].state.priority = 0x56;
        if (s->boss_flags & 4U)
        {
            s->combat.music_change = true;
            s->combat.music = 0x313;
            ++a->state.step;
        }
        break;
    case 7:
        for (Uint16 i = 0; i < 2; ++i)
        {
            Actor *plate = NULL;
            if (!ActorSpawn(s, a, 0x1c, i, 160, 192, &plate))
                return false;
            if (plate)
            {
                plate->state.x = (192 + i * 128) * 65536;
                plate->state.y = 128 * 65536;
            }
        }
        a->arena.timer = 192;
        ++a->state.step;
        break;
    case 8:
        if (!a->arena.timer)
            return SDL_SetError("BO4 reward delay underflow");
        if (--a->arena.timer == 0)
            ++a->state.step;
        break;
    case 9: {
        Actor *reward = NULL;
        if (!ActorSpawn(s, a, 0x1d, 4, 96, 128, &reward))
            return false;
        if (reward)
        {
            reward->state.x = 256 * 65536;
            reward->state.y = 128 * 65536;
            ++a->state.step;
        }
        break;
    }
    case 10:
        ++a->state.step;
        break;
    case 11:
        break;
    default:
        return SDL_SetError("BO4 entry manager entered an invalid step");
    }
    s->dopple.anchor_x = Pixel(a->state.x);
    s->dopple.anchor_y = Pixel(a->state.y);
    return true;
}
static bool Plate(SotnActors *s, Actor *a)
{
    const Uint16 index = a->state.parameters & 15U;
    const bool opposite = (a->state.parameters & 0x100U) != 0;
    if (index < 1 || index > 5 || (a->state.parameters & 0xfef0U))
        return SDL_SetError("BO4 opening plate has invalid original parameters");
    switch (a->state.step)
    {
    case 0: {
        SotnDataView scale = {0};
        if (!ActorInitialize(s, a, 0x458) || !Slice(s->nz0, 0x5c0 + index * 2U, 2, &scale))
            return false;
        const Uint16 size = Read16(scale.bytes);
        if (!size || size > 1024)
            return SDL_SetError("BO4 opening plate scale exceeds its original bounds");
        a->state.opacity_set = true;
        a->state.opacity = index > 1 ? 32 : 128;
        a->state.scale_x = a->state.scale_y = size;
        a->state.rotated = true;
        a->state.rotation = index & 1U ? -1024 : 0;
        a->state.priority = (Uint16)((0x40U - a->state.parameters) & 15U);
        a->state.frame = opposite ? 0x64 : 0x62;
        if (!ActorPrimitives(s, a, 1))
        {
            a->remove = true;
            return true;
        }
        a->arena.anchor_x = Pixel(a->state.x) - s->camera_x;
        a->arena.anchor_y = Pixel(a->state.y) - s->camera_y;
        break;
    }
    case 1:
        a->state.palette = 0x804f;
        if (a->arena.triggered)
        {
            a->arena.timer = 64;
            ++a->state.step;
        }
        break;
    case 2:
        if ((a->state.parameters & 255U) != 1)
            ++a->state.step;
        else
        {
            Uint8 animation = 0;
            if (!ActorAnimate(s, a, opposite ? 0x5d4 : 0x5cc, &animation))
                return false;
            if (!(s->ticks & 15U) && !ActorSound(s, 0x608))
                return false;
            if (!a->arena.timer)
                return SDL_SetError("BO4 opening plate wait underflow");
            if (--a->arena.timer == 0)
                ++a->state.step;
        }
        break;
    case 3:
        if (!(s->ticks & 1U) && ++a->state.palette > 0x8058)
        {
            if (!ActorSound(s, 0x614))
                return false;
            a->state.palette = 0x804f;
            ++a->state.step;
            if (a->state.parameters == 5 && !ActorDoppleTrigger(s, (Uint32)a->slot + 1))
                return false;
        }
        break;
    case 4:
    case 5: {
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(s->executable, a->state.rotation - 1024, &trig))
            return false;
        const Sint32 radius = a->arena.offset / 65536;
        Position(a, s->camera_x + a->arena.anchor_x + ActorShift((Sint64)trig.cosine * radius, 12),
                 s->camera_y + a->arena.anchor_y + ActorShift((Sint64)trig.sine * radius, 12));
        Uint8 animation = 0;
        if (!ActorAnimate(
                s, a, a->state.step == 4 ? (opposite ? 0x5d4 : 0x5cc) : (opposite ? 0x5e4 : 0x5dc),
                &animation))
            return false;
        if (a->state.step == 4 && !(s->ticks & 15U) && !ActorSound(s, 0x608))
            return false;
        SotnDataView speed = {0}, turn = {0};
        if (!Slice(s->nz0, 0x5a8 + index * 2U, 2, &speed) ||
            !Slice(s->nz0, 0x5b4 + index * 2U, 2, &turn))
            return false;
        const Sint32 acceleration = Signed16(speed.bytes);
        if (acceleration < 0 || acceleration > 4096 || Signed16(turn.bytes) < 0 ||
            Signed16(turn.bytes) > 256)
            return SDL_SetError("BO4 opening plate acceleration or spin is corrupt");
        const Uint16 old_step = a->state.step;
        if (old_step == 5)
            a->state.rotation = (Sint16)Wrap16(a->state.rotation + Signed16(turn.bytes));
        a->arena.speed = Add(a->arena.speed, opposite ? -acceleration : acceleration);
        a->arena.offset = Add(a->arena.offset, a->arena.speed);
        if (old_step == 4 && SDL_abs((Sint64)a->arena.offset) > 12 * 65536)
        {
            if (!ActorDoppleTrigger(s, (Uint32)a->slot + 2))
                return false;
            ++a->state.step;
            if (a->state.parameters == 5 && !ActorDoppleTrigger(s, (Uint32)a->slot + 1))
                return false;
        }
        if (old_step == 5 && a->arena.offset > 64 * 65536)
            a->remove = true;
        if ((Uint32)a->slot + 2 >= SOTN_STAGE_ACTOR_SLOTS)
            return SDL_SetError("BO4 opening plate successor exceeds its stage slots");
        Actor *next = &s->slots[(Uint32)a->slot + 2];
        if (next->state.generation && next->state.opacity < 128)
            next->state.opacity = (Uint16)(next->state.opacity + (old_step == 4 ? 1 : 2));
        break;
    }
    default:
        return SDL_SetError("BO4 opening plate entered an invalid step");
    }
    return true;
}
static bool ActorDoppleDoorTiles(SotnActors *s, const Actor *a, bool open)
{
    const Uint32 index = a->state.parameters & 255U;
    const Uint32 side = a->state.parameters & 0x100U ? 0xde : 0xc0;
    return ActorTilePatch(s, side + (3U - index) * 32U, 1, 2, 2,
                          0x670 + (open ? 3U - index : 7U - index) * 4U);
}
static bool Blocks(SotnActors *s, Actor *a)
{
    const Uint16 index = a->state.parameters & 255U;
    const bool right = (a->state.parameters & 0x100U) != 0;
    if (index > 3 || (a->state.parameters & 0xfe00U))
        return SDL_SetError("BO4 block door parameters are invalid");
    switch (a->state.step)
    {
    case 0:
        s->control =
            (SotnActorControl){.buttons = Pixel(s->player.x) < 128 ? 0x2000 : 0x8000, .timer = 24};
        if (!ActorInitialize(s, a, 0x458))
            return false;
        a->state.frame = 0;
        if (!a->state.parameters)
            for (Uint16 i = 1; i < 8; ++i)
            {
                Actor *block = NULL;
                if (!ActorSpawnAt(s, a, 0x1b, i < 4 ? i : (Uint16)(0x100U + i - 4),
                                  (Uint32)a->slot + i, &block))
                    return false;
            }
        Position(a, right ? 496 : 16, 104);
        break;
    case 1:
        if (!a->state.parameters && s->control.timer < 9)
        {
            s->control = (SotnActorControl){.timer = 4};
            a->arena.triggered = true;
        }
        if (a->arena.triggered)
            a->state.step = 2;
        break;
    case 2: {
        s->control = (SotnActorControl){.timer = 4};
        Uint8 animation = 0;
        if (!ActorAnimate(s, a, 0x668, &animation))
            return false;
        if (!animation)
        {
            a->vy = 4 * 65536;
            ++a->state.step;
        }
        break;
    }
    case 3: {
        s->control = (SotnActorControl){.timer = 24};
        a->state.x = Add(a->state.x, a->vx);
        a->state.y = Add(a->state.y, a->vy);
        a->vy = Add(a->vy, 8192);
        const Sint32 target = (3 - index) * 16 + 96;
        if (target < Pixel(a->state.y))
        {
            if (!ActorDoppleTrigger(s, (Uint32)a->slot + 1))
                return false;
            Position(a, Pixel(a->state.x), target);
            if (!right && !ActorSound(s, 0x650))
                return false;
            if (!ActorDoppleDoorTiles(s, a, false))
                return false;
            a->vy = 0;
            ++a->state.step;
        }
        break;
    }
    case 4:
        if (s->boss_flags & 4U)
            ++a->state.step;
        break;
    case 5:
        if (!ActorDoppleDoorTiles(s, a, true))
            return false;
        ++a->state.step;
        break;
    case 6:
        a->vx = right ? 32768 : -32768;
        a->state.x = Add(a->state.x, a->vx);
        if (Pixel(a->state.x) < -32 || Pixel(a->state.x) > 544)
            ++a->state.step;
        break;
    case 7:
        break;
    default:
        return SDL_SetError("BO4 block door entered an invalid step");
    }
    return true;
}
static bool RewardPlates(SotnActors *s, Actor *a)
{
    if (a->state.parameters > 1)
        return SDL_SetError("BO4 reward plate has invalid parameters");
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x458))
            return false;
        a->state.rotated = true;
        a->state.rotation = -1024;
        a->state.frame = a->state.parameters & 1U ? 0x64 : 0x62;
        a->vx = a->state.parameters & 1U ? -32768 : 32768;
        a->state.priority = 0x5e;
    }
    if (a->state.step == 1)
    {
        a->state.x = Add(a->state.x, a->vx);
        Uint8 animation = 0;
        if (!ActorAnimate(s, a, a->state.parameters & 1U ? 0x5d4 : 0x5cc, &animation))
            return false;
        if (a->vx > 0 ? Pixel(a->state.x) > 256 : Pixel(a->state.x) < 256)
            ++a->state.step;
        if (!(s->ticks & 15U) && !ActorSound(s, 0x608))
            return false;
    }
    else if (a->state.step == 2)
    {
        Position(a, 256, Pixel(a->state.y));
        a->state.frame = a->state.parameters & 1U ? 0x64 : 0x62;
    }
    else
        return SDL_SetError("BO4 reward plate entered an invalid step");
    return true;
}
bool ActorDoppleEntry(SotnActors *s, Actor *a)
{
    if (s == NULL || a == NULL || s->stage_id != 26)
        return SDL_SetError("BO4 entry requires its stage service");
    switch (a->state.id)
    {
    case 0x17:
        return Plate(s, a);
    case 0x18:
        return Manager(s, a);
    case 0x19:
        return ActorDoppleLight(s, a);
    case 0x1a:
        return ActorDoppleShock(s, a);
    case 0x1b:
        return Blocks(s, a);
    case 0x1c:
        return RewardPlates(s, a);
    case 0x1d:
        return ActorBossReward(s, a);
    default:
        return SDL_SetError("BO4 entry dispatch received a foreign actor");
    }
}
