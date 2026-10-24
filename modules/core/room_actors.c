// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted from sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5,
// src/st/create_entity.h and st_common.h: scrolling placement and persistence paths.
#include <SDL3/SDL_error.h>
#include <sotn/actor_layout.h>

typedef struct ActorSlot
{
    SotnActorPlacement placement;
    Uint32 generation;
} ActorSlot;

struct SotnRoomActors
{
    const SotnActorLayout *layout;
    ActorSlot slots[SOTN_STAGE_ACTOR_SLOTS];
    Uint32 persistent[8], serial;
    Sint32 cursor[2], camera[2];
    bool backward[2];
};

static bool CameraValid(Sint32 x, Sint32 y)
{
    return x >= 0 && x <= 16384 && y >= 0 && y <= 16384;
}

static Sint32 Coordinate(const SotnRoomActors *actors, unsigned int axis, Sint32 cursor)
{
    if (cursor < 0)
        return -1;
    if ((Uint32)cursor >= SotnActorLayout_Count(actors->layout))
        return 65535;
    SotnActorPlacement placement = {0};
    (void)SotnActorLayout_Get(actors->layout, axis != 0, (Uint32)cursor, &placement);
    return axis == 0 ? placement.x : placement.y;
}

static void Spawn(SotnRoomActors *actors, unsigned int axis)
{
    SotnActorPlacement placement = {0};
    (void)SotnActorLayout_Get(actors->layout, axis != 0, (Uint32)actors->cursor[axis], &placement);
    if (placement.persistence != 0)
    {
        const Uint32 bit = (Uint32)placement.persistence - 1;
        if (actors->persistent[bit / 32] & (1U << (bit % 32)))
            return;
    }
    const unsigned int other = axis ^ 1U;
    const Sint32 position = axis == 0 ? placement.y : placement.x;
    const Sint32 near = SDL_max(0, actors->camera[other] - 64);
    const Sint32 far = actors->camera[other] + (axis == 0 ? 288 : 320);
    if (position < near || position > far || placement.mode == 0x8000)
        return;
    ActorSlot *slot = &actors->slots[placement.slot];
    if (slot->generation != 0 && placement.mode != 0xa000)
        return;
    ++actors->serial;
    if (actors->serial == 0)
        ++actors->serial;
    *slot = (ActorSlot){placement, actors->serial};
}

static void Scan(SotnRoomActors *actors, unsigned int axis, Sint32 extent, Sint32 delta)
{
    const bool backward = delta < 0;
    if (backward != actors->backward[axis])
    {
        const Sint32 previous = extent - delta;
        if (backward)
            while (actors->cursor[axis] >= 0 &&
                   Coordinate(actors, axis, actors->cursor[axis]) > previous)
                --actors->cursor[axis];
        else
            while (actors->cursor[axis] < (Sint32)SotnActorLayout_Count(actors->layout) &&
                   Coordinate(actors, axis, actors->cursor[axis]) < previous)
                ++actors->cursor[axis];
        actors->backward[axis] = backward;
    }
    const Sint32 count = (Sint32)SotnActorLayout_Count(actors->layout);
    while (actors->cursor[axis] >= 0 && actors->cursor[axis] < count)
    {
        const Sint32 coordinate = Coordinate(actors, axis, actors->cursor[axis]);
        if (backward ? extent > coordinate : extent < coordinate)
            break;
        Spawn(actors, axis);
        actors->cursor[axis] += backward ? -1 : 1;
    }
}

bool SotnRoomActors_Create(SotnRoomActors **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = SDL_calloc(1, sizeof(**out));
    return *out != NULL;
}

bool SotnRoomActors_Enter(SotnRoomActors *actors, const SotnActorLayout *layout, Sint32 camera_x,
                          Sint32 camera_y)
{
    if (actors == NULL || layout == NULL || !CameraValid(camera_x, camera_y))
        return SDL_SetError("Room actor entry requires a layout and bounded camera position");
    actors->layout = layout;
    SDL_memset(actors->persistent, 0, sizeof(actors->persistent));
    SDL_memset(actors->slots, 0, sizeof(actors->slots));
    actors->camera[0] = camera_x;
    actors->camera[1] = camera_y;
    actors->backward[0] = actors->backward[1] = false;
    actors->cursor[0] = actors->cursor[1] = 0;
    const Sint32 left = SDL_max(0, camera_x - 64);
    while (actors->cursor[0] < (Sint32)SotnActorLayout_Count(layout) &&
           Coordinate(actors, 0, actors->cursor[0]) < left)
        ++actors->cursor[0];
    Scan(actors, 0, camera_x + 320, 1);
    while (actors->cursor[1] < (Sint32)SotnActorLayout_Count(layout) &&
           Coordinate(actors, 1, actors->cursor[1]) < camera_y + 288)
        ++actors->cursor[1];
    return true;
}

bool SotnRoomActors_Scroll(SotnRoomActors *actors, Sint32 camera_x, Sint32 camera_y)
{
    if (actors == NULL || actors->layout == NULL || !CameraValid(camera_x, camera_y))
        return SDL_SetError("Room actor scrolling requires an entered room and bounded camera");
    const Sint32 dx = camera_x - actors->camera[0], dy = camera_y - actors->camera[1];
    actors->camera[0] = camera_x;
    actors->camera[1] = camera_y;
    if (dx != 0)
        Scan(actors, 0, dx > 0 ? camera_x + 320 : SDL_max(0, camera_x - 64), dx);
    if (dy != 0)
        Scan(actors, 1, dy > 0 ? camera_y + 288 : SDL_max(0, camera_y - 64), dy);
    return true;
}

bool SotnRoomActors_Get(const SotnRoomActors *actors, Uint32 slot, SotnActorPlacement *out,
                        Uint32 *generation)
{
    if (actors == NULL || out == NULL || generation == NULL || slot >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Room actor slot or output is invalid");
    *out = actors->slots[slot].placement;
    *generation = actors->slots[slot].generation;
    return true;
}

bool SotnRoomActors_Release(SotnRoomActors *actors, Uint32 slot, bool permanent)
{
    if (actors == NULL || slot >= SOTN_STAGE_ACTOR_SLOTS)
        return SDL_SetError("Room actor slot is invalid");
    const Uint32 flag = actors->slots[slot].placement.persistence;
    if (permanent && actors->slots[slot].generation != 0 && flag != 0)
        actors->persistent[(flag - 1) / 32] |= 1U << ((flag - 1) % 32);
    actors->slots[slot] = (ActorSlot){0};
    return true;
}

void SotnRoomActors_Destroy(SotnRoomActors *actors)
{
    SDL_free(actors);
}
