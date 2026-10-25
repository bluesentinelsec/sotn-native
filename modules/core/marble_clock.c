// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected original clock-room states, st/no0/clock_room.c and clock_room_entities.h,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Tables remain private.
#include "actors_internal.h"
static bool Value(SotnActors *s, size_t at, Uint16 index, Sint32 *out)
{
    SotnDataView value = {0};
    if (index >= 4 || !Slice(s->nz0, at + index * 2U, 2, &value))
        return SDL_SetError("Clock room private position index is invalid");
    *out = Signed16(value.bytes);
    return true;
}
static bool Spawn(SotnActors *s, const Actor *a, Uint16 id, Uint16 parameter, Uint32 slot,
                  Actor **out)
{
    if (!ActorSpawnAt(s, a, id, parameter, slot, out))
        return false;
    (*out)->state.x = Pixel(a->state.x) * 65536;
    (*out)->state.y = Pixel(a->state.y) * 65536;
    return true;
}
static bool Shadow(SotnActors *s, const Actor *a, Uint16 frame, Uint16 priority, Sint32 dy,
                   bool rotating, Uint32 slot, Actor **out)
{
    if (!Spawn(s, a, 0x20, 0, slot, out))
        return false;
    Actor *p = *out;
    p->state.animation_set = 0x8001;
    p->state.frame = frame;
    p->state.priority = priority;
    p->state.palette = a->state.id == 0x19 ? 0x804b : 0;
    p->state.opacity_set = true;
    p->state.blend_mode = 0x10;
    p->state.rotated = rotating;
    p->state.flags = 0x8c000000;
    ShiftY(p, dy);
    return true;
}
static void HandsAndCages(SotnActors *s, const Actor *a)
{
    const Uint32 minutes = (s->game_seconds / 60) % 60, hours = s->game_seconds / 3600;
    s->slots[a->slot + 5].clock.hand = (Sint32)minutes * 60;
    s->slots[a->slot + 6].clock.hand = (Sint32)(hours * 300 + minutes * 5);
    s->slots[a->slot + 7].clock.state = minutes >= 10 && minutes < 30;
    s->slots[a->slot + 8].clock.state = minutes >= 30 && minutes < 50;
}
static bool Controller(SotnActors *s, Actor *a)
{
    if (a->clock.bell)
    {
        if (!a->clock.bell_wait)
        {
            if (!ActorSound(s, 0x7a6))
                return false;
            if (--a->clock.bell)
                a->clock.bell_wait = 64;
        }
        else
            --a->clock.bell_wait;
    }
    const bool stopped = SotnSubweapons_Frozen(s->subweapons);
    if (!stopped)
    {
        if (Pixel(s->player.y) - s->camera_y > 128)
            s->clock_statues[0] = false;
    }
    else if (!a->clock.stopped)
        s->clock_statues[0] = true;
    a->clock.stopped = stopped;
    if ((s->game_seconds / 60) & 1)
    {
        if (Pixel(s->player.y) - s->camera_y > 128)
            s->clock_statues[1] = false;
    }
    else
        s->clock_statues[1] = true;
    if (a->state.step == 0)
    {
        if ((s->ticks % 60) == 0 && !ActorSound(s, 0x7a9))
            return false;
        if ((Uint32)a->slot + 15 >= SOTN_STAGE_ACTOR_SLOTS)
            return SDL_SetError("Clock controller requires its original sixteen-slot reservation");
        if (!ActorInitialize(s, a, 0xae8))
            return false;
        s->combat.music_change = true;
        s->combat.music = 0;
        s->clock_statues[0] = false;
        if (Pixel(s->player.y) - s->camera_y < 64)
        {
            const Sint32 x = Pixel(s->player.x) - s->camera_x;
            if (x < 64)
                s->clock_statues[1] = true;
            else if (x > 192)
                s->clock_statues[0] = true;
        }
        a->state.animation_set = 0x8001;
        a->state.frame = 23;
        a->state.priority = 0x40;
        for (Uint16 i = 0; i < 2; ++i)
        {
            Actor *child = NULL;
            if (!Spawn(s, a, 0x1a, i, a->slot + 5U + i, &child) ||
                !Spawn(s, a, 0x1b, i, a->slot + 7U + i, &child) ||
                !Spawn(s, a, 0x1c, i, a->slot + 1U + i, &child) ||
                !Spawn(s, a, 0x1d, i, a->slot + 12U + i, &child) ||
                !Spawn(s, a, 0x1e, i, a->slot + 14U + i, &child))
                return false;
        }
        Actor *shadow = NULL;
        if (!Shadow(s, a, 23, 0x40, 4, false, a->slot + 9U, &shadow))
            return false;
        HandsAndCages(s, a);
        return true;
    }
    if (a->state.step != 1)
        return SDL_SetError(
            "Clock room ritual states are not integrated in this early-route checkpoint");
    if (!s->game_frame && !ActorSound(s, 0x7a9))
        return false;
    HandsAndCages(s, a);
    if (!(s->game_seconds % 60) && !s->game_frame && (s->game_seconds / 60) % 60 == 0)
        a->clock.bell = (Uint16)(((s->game_seconds / 3600 + 11) % 12) + 1);
    /* Preserve the original US half-hour expression, which never schedules a bell. */
    if (!s->world->flags[SOTN_CLOCK_FLOOR] && s->character != NULL && Pixel(s->player.x) >= 48 &&
        Pixel(s->player.x) < 209)
    {
        SotnCharacterState character = {0};
        if (!SotnCharacter_Get(s->character, &character, NULL))
            return false;
        const Uint16 left = character.equipment[SOTN_EQUIP_RING1],
                     right = character.equipment[SOTN_EQUIP_RING2];
        if ((left == 72 || left == 73) && (right == 72 || right == 73))
            return SDL_SetError(
                "Clock room ring ritual remains unintegrated in this early-route checkpoint");
    }
    return true;
}
static bool Hand(SotnActors *s, Actor *a)
{
    if (a->state.parameters > 1 || (Uint32)a->slot + 5 >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Clock hand index or shadow slot is invalid");
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, 0xae8))
            return false;
        a->state.animation_set = 0x8001;
        a->state.frame = (Uint16)(25 + a->state.parameters);
        a->state.priority = (Uint16)(63 - a->state.parameters);
        a->state.rotated = true;
        Actor *shadow = NULL;
        if (!Shadow(s, a, a->state.frame, a->state.priority, 4, true, a->slot + 5U, &shadow))
            return false;
    }
    a->state.rotation =
        (Sint16)(((Sint64)a->clock.hand * 4096 / 3600 + (a->state.parameters ? 1024 : 0)) & 4095);
    Actor *shadow = &s->slots[a->slot + 5];
    if (shadow->state.id != 0x20 || shadow->enemy_state.owner_generation != a->state.generation)
        return SDL_SetError("Clock hand lost its owned shadow");
    shadow->state.rotation = a->state.rotation;
    return true;
}
static bool Bird(SotnActors *s, Actor *a)
{
    const Uint16 index = a->state.parameters;
    if (index > 1)
        return SDL_SetError("Clock birdcage index is invalid");
    Sint32 frame = 0;
    if (!Value(s, 0x1c5c, a->clock.state & 1, &frame))
        return false;
    if (a->state.step == 0)
    {
        Sint32 x = 0, y = 0;
        if (!ActorInitialize(s, a, 0xae8) || !Value(s, 0x1c34, index, &x) ||
            !Value(s, 0x1c38, index, &y))
            return false;
        a->state.animation_set = 0x8001;
        a->state.frame = (Uint16)frame;
        a->clock.previous = a->clock.state;
        a->state.priority = 60;
        a->state.scale_x = a->state.scale_y = 256;
        a->state.opacity = 128;
        a->state.x = x * 65536;
        a->state.y = y * 65536;
    }
    else if (a->state.step == 1)
    {
        if (a->clock.previous != a->clock.state)
        {
            a->clock.previous = a->clock.state;
            a->clock.timer = 64;
            ++a->state.step;
            return ActorSound(s, 0x608);
        }
    }
    else if (a->state.step <= 5)
    {
        if (!a->clock.timer)
            return SDL_SetError("Clock birdcage transition timer underflow");
        if (a->state.step == 2)
        {
            a->state.scale_x = a->state.scale_y = (Uint16)(a->state.scale_y - 2);
            --a->state.opacity;
        }
        else if (a->state.step == 3)
            a->state.x += 8192;
        else if (a->state.step == 4)
            a->state.y -= 8192;
        else
        {
            a->state.scale_x = a->state.scale_y = (Uint16)(a->state.scale_y + 2);
            ++a->state.opacity;
        }
        if (--a->clock.timer == 0)
        {
            a->clock.timer = 64;
            if (a->state.step == 2)
                a->state.priority = 0;
            else if (a->state.step == 3)
            {
                a->state.frame = (Uint16)frame;
                a->state.x -= 8 * 65536;
                ShiftY(a, 8);
            }
            else if (a->state.step == 4)
                a->state.priority = 60;
            else
            {
                a->state.step = 1;
                return true;
            }
            ++a->state.step;
            return ActorSound(s, 0x608);
        }
    }
    else
        return SDL_SetError("Clock birdcage entered invalid transition state");
    return true;
}
static bool StatueTiles(SotnActors *s, Uint16 index, bool open)
{
    const Uint32 start = index ? 2 : 12;
    for (Uint32 row = 0; row < 6; ++row)
        for (Uint32 col = 0; col < 2; ++col)
            if (!ActorTile(s, start + row * 16 + col, open ? 0 : 0x597))
                return false;
    return true;
}
static bool Statue(SotnActors *s, Actor *a)
{
    const Uint16 index = a->state.parameters;
    if (index > 1 || (Uint32)a->slot + 11 >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Clock statue index or reserved children are invalid");
    Actor *shadow = &s->slots[a->slot + 2], *gear = &s->slots[a->slot + 11];
    if (a->state.step == 0)
    {
        Sint32 x = 0;
        if (!ActorInitialize(s, a, 0xae8) ||
            !Value(s, 0x1c3c, (Uint16)(index + (s->clock_statues[index] ? 2 : 0)), &x) ||
            SDL_abs(x) > 256 || !StatueTiles(s, index, s->clock_statues[index]))
            return false;
        a->state.animation_set = 0x8001;
        a->state.frame = (Uint16)(10 + index);
        a->state.hitbox_width = 16;
        a->state.hitbox_height = 32;
        a->state.priority = 64;
        a->state.x += x * 65536;
        ShiftY(a, -58);
        a->clock.state = s->clock_statues[index];
        if (!Shadow(s, a, a->state.frame, 63, 8, false, a->slot + 2U, &shadow))
            return false;
    }
    else if (a->state.step == 1)
    {
        if (s->clock_statues[index] != (a->clock.state != 0))
        {
            a->clock.state = s->clock_statues[index];
            gear->clock.state = a->clock.state ? 1 : 2;
            a->state.hitbox_state = 2;
            ++a->state.step;
            if (!ActorSound(s, 0x609))
                return false;
        }
    }
    else if (a->state.step == 2)
    {
        Uint16 collision = 0;
        if (!ActorObjectCollision(s, a, 16, 32, 19, &collision))
            return false;
        if (!a->substep)
        {
            if (a->clock.state && !StatueTiles(s, index, true))
                return false;
            a->clock.timer = 96;
            ++a->substep;
        }
        SotnDataView velocity = {0};
        if (!Slice(s->nz0, 0x1c54 + index * 4U, 4, &velocity))
            return false;
        const Sint32 vx = Signed32(velocity.bytes);
        if (vx < -65536 || vx > 65536)
            return SDL_SetError("Clock statue velocity exceeds its private bounds");
        a->state.x += a->clock.state ? vx : -vx;
        if (!a->clock.timer)
            return SDL_SetError("Clock statue movement timer underflow");
        if (--a->clock.timer == 0)
        {
            if (!a->clock.state && !StatueTiles(s, index, false))
                return false;
            gear->clock.state = 0;
            a->state.hitbox_state = a->substep = 0;
            --a->state.step;
        }
    }
    else
        return SDL_SetError("Clock statue entered invalid movement state");
    if (shadow->state.id != 0x20 || shadow->enemy_state.owner_generation != a->state.generation)
        return SDL_SetError("Clock statue lost its owned shadow");
    shadow->state.x = Pixel(a->state.x) * 65536;
    return true;
}
static bool Gear(SotnActors *s, Actor *a)
{
    if (a->state.parameters > 1)
        return SDL_SetError("Clock statue gear index is invalid");
    if (a->state.step == 0)
    {
        Sint32 x = 0;
        if (!ActorInitialize(s, a, 0xae8) || !Value(s, 0x1c44, a->state.parameters, &x) ||
            SDL_abs(x) > 256)
            return false;
        a->state.animation_set = 0x8001;
        a->state.frame = 17;
        a->state.priority = 128;
        a->state.x += x * 65536;
        ShiftY(a, -44);
        if (!ActorPrimitives(s, a, 1))
            return false;
        ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
        if (p == NULL)
            return false;
        p->state = (SotnActorPrimitive){.x = a->state.x - 6 * 65536,
                                        .y = a->state.y - 16 * 65536,
                                        .width = 12,
                                        .height = 32,
                                        .active = true};
    }
    else if (a->state.step == 1)
    {
        if (a->clock.state)
        {
            a->pose =
                a->clock.state == 1 ? (Uint16)(a->state.frame - 17) : (Uint16)(20 - a->state.frame);
            a->state.step = a->clock.state == 1 ? 2 : 3;
        }
        a->remaining = 0;
    }
    else if (a->state.step == 2 || a->state.step == 3)
    {
        Uint8 animation = 0;
        if (!ActorAnimate(s, a, a->state.step == 2 ? 0x1c78 : 0x1c84, &animation))
            return false;
        if (!a->clock.state)
            a->state.step = 1;
    }
    else
        return SDL_SetError("Clock gear entered invalid spin state");
    return true;
}
static bool FloorTiles(SotnActors *s, bool closed)
{
    SotnDataView tiles = {0};
    if (!Slice(s->nz0, 0x1c90, 16, &tiles))
        return false;
    for (Uint32 row = 0; row < 2; ++row)
        for (Uint32 col = 0; col < 8; ++col)
            if (!ActorTile(s, 0xc4 + row * 16 + col,
                           closed ? 0x597 : Read16(tiles.bytes + col * 2)))
                return false;
    return true;
}
static bool Floor(SotnActors *s, Actor *a)
{
    const Uint16 index = a->state.parameters;
    if (index > 1)
        return SDL_SetError("Clock floor door index is invalid");
    const Uint16 open = s->world->flags[SOTN_CLOCK_FLOOR];
    if (a->state.step == 0)
    {
        Sint32 x = 0;
        if (!ActorInitialize(s, a, 0xae8) ||
            !Value(s, 0x1c48, (Uint16)(index + (open ? 2 : 0)), &x) || SDL_abs(x) > 256 ||
            !FloorTiles(s, open == 0))
            return false;
        a->state.animation_set = 0x8001;
        a->state.frame = (Uint16)(27 + index);
        a->state.priority = 64;
        a->state.x += x * 65536;
        ShiftY(a, 88);
        a->clock.previous = open;
    }
    else if (a->state.step == 1)
    {
        if (!a->clock.previous && open)
        {
            ++a->state.step;
            a->clock.timer = 0;
        }
        a->clock.previous = open;
    }
    else if (a->state.step == 2)
    {
        if (!(a->clock.timer & 31) && !ActorSound(s, 0x607))
            return false;
        if (++a->clock.timer & 1)
            a->state.x += (index ? 1 : -1) * 65536;
        if (a->clock.timer > 96)
        {
            if (!FloorTiles(s, false))
                return false;
            --a->state.step;
        }
    }
    else
        return SDL_SetError("Clock floor door entered invalid state");
    return true;
}
bool ActorClock(SotnActors *s, Actor *a)
{
    switch (a->state.id)
    {
    case 0x19:
        return Controller(s, a);
    case 0x1a:
        return Hand(s, a);
    case 0x1b:
        return Bird(s, a);
    case 0x1c:
        return Statue(s, a);
    case 0x1d:
        return Gear(s, a);
    case 0x1e:
        return Floor(s, a);
    case 0x1f:
    case 0x20:
        return true;
    case 0x2b: {
        if (!a->state.step)
        {
            if (!ActorInitialize(s, a, 0xae8))
                return false;
            a->state.animation_set = 0x8001;
            a->state.frame = 33;
            a->state.priority = 80;
            a->state.texture_offset = a->state.palette = 0;
            a->state.rotated = a->state.opacity_set = true;
            a->state.opacity = 96;
        }
        const Sint32 sine = ActorSin((Sint32)(((s->ticks % 120) * 4096 + 60) / 120));
        if (!sine && !ActorSound(s, 0x6b7))
            return false;
        a->state.rotation = (Sint16)(ActorShift(sine, 6) + ActorShift(sine, 7));
        return true;
    }
    case 0x42:
        if (a->state.parameters > 2)
            return SDL_SetError("Distant clock tick variant is invalid");
        if (!a->state.step && !ActorInitialize(s, a, 0xa88))
            return false;
        if ((s->ticks % 60) == 0)
        {
            if (a->state.parameters == 0)
                return ActorSound(s, 0x6b7);
            const Sint32 distance = a->state.parameters == 1
                                        ? ((Pixel(s->player.x) - 448) * 2) / 5
                                        : ((320 - (Pixel(s->player.x) - s->camera_x)) * 2) / 5;
            return ActorSoundVolumePan(s, 0x6b7, (Uint16)SDL_clamp(distance, 0, 127),
                                       a->state.parameters == 1 ? 8 : -8);
        }
        return true;
    default:
        return SDL_SetError("Clock dispatcher received an unrelated entity");
    }
}
