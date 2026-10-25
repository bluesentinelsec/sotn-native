// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/lib/first_c_file.c:func_us_801AE8E8, sotn-decomp 7bc54d8157b6.
#include "actors_internal.h"
static bool Initialize(SotnActors *s, Actor *a)
{
    SotnDataView angle = {0}, depth = {0};
    if (a->state.parameters > 3 || !Slice(s->nz0, 0xda8U + a->state.parameters * 2U, 2, &angle) ||
        !Slice(s->nz0, 0xdb0U + a->state.parameters * 2U, 2, &depth))
        return SDL_SetError("Library bookcase has an invalid private parameter");
    const Sint32 distance = Signed16(depth.bytes);
    if (distance < 0 || distance > 128)
        return SDL_SetError("Library bookcase exceeds its selected private depth");
    if (!ActorInitialize(s, a, 0x830) || !ActorPrimitives(s, a, 37))
        return false;
    a->state.priority = 0x70;
    a->state.hitbox_width = 12;
    a->state.hitbox_height = 48;
    a->state.hitbox_y = -48;
    a->shelf.angle = (Sint16)(a->state.parameters ? Signed16(angle.bytes) : -192);
    a->shelf.delay = 96;
    a->shelf.damping = 64;
    a->shelf.depth = (Sint16)distance;
    a->shelf.velocity = a->shelf.force = 0;
    if (a->state.parameters)
        a->state.step = 4;
    for (Uint16 i = 0; i < 37; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        p->state = (SotnActorPrimitive){.screen_space = true, .quad = true, .priority_set = true};
        if (i < 4)
        {
            p->state.textured = p->state.modulate = p->state.gouraud = true;
            p->state.priority = (Uint16)(0x70U - a->state.parameters);
            p->state.page = 15;
            p->state.palette = i < 2 ? 0x32 : 0x33;
            p->state.u[0] = p->state.u[2] = i < 2 ? 4 : 0x24;
            p->state.u[1] = p->state.u[3] = i < 2 ? 0x1c : 0x64;
            p->state.v[0] = p->state.v[1] = 0x84;
            p->state.v[2] = p->state.v[3] = 0xe4;
        }
        if (i == 4 && !a->state.parameters)
        {
            const Sint32 x = Pixel(a->state.x) - s->camera_x, y = Pixel(a->state.y) - s->camera_y;
            if (x < SDL_MIN_SINT16 || x > SDL_MAX_SINT16 || y - 88 < SDL_MIN_SINT16 ||
                y > SDL_MAX_SINT16)
                return SDL_SetError("Library bookcase shadow exceeds signed screen coordinates");
            p->state.active = p->state.blend = true;
            p->state.priority = 0x6f;
            p->state.offset_x[0] = p->state.offset_x[2] = (Sint16)x;
            p->state.offset_x[1] = p->state.offset_x[3] = 256;
            p->state.offset_y[0] = (Sint16)(y - 88);
            p->state.offset_y[1] = (Sint16)(y - 64);
            p->state.offset_y[2] = p->state.offset_y[3] = (Sint16)y;
        }
    }
    return true;
}
static bool Contact(SotnActors *s, Actor *a)
{
    a->shelf.force = 0;
    const Uint16 angle = (Uint16)a->shelf.angle & 4095U;
    Sint16 face = -1;
    if (angle < 0x380 || angle > 0xc80)
        face = 0;
    if (angle > 0x480 && angle < 0xb80)
        face = 1;
    if (face < 0)
        return true;
    const ActorPrimitive *p = ActorPrimitiveGet(s, a, (Uint16)face);
    if (!p)
        return false;
    // Original contact uses the previous simulation tick's projected front/back edges.
    const Sint32 x = Pixel(s->player.x) - s->camera_x;
    const bool left = p->state.offset_x[0] > x;
    const Sint32 dx = x + (left ? 10 : -10) - p->state.offset_x[left ? 0 : 1];
    a->substep = left ? 0 : 1;
    if ((left && dx >= 0) || (!left && dx <= 0))
    {
        if (dx < -64 || dx > 64 || !ActorDisplace(s, -dx * 65536, 0, left ? 0x48 : 0x44))
            return SDL_SetError("Library bookcase contact exceeds the bounded player shove");
        bool push = s->world->flags[SOTN_LIBRARY_SHELF] != 0;
        if (!push)
        {
            if (a->shelf.delay <= 0)
                return SDL_SetError("Library bookcase discovery timer is invalid");
            if (--a->shelf.delay == 0)
            {
                s->world->flags[SOTN_LIBRARY_SHELF] = 1;
                push = true;
            }
        }
        if (push && s->player.velocity_x)
        {
            if (!(s->service_ticks & 15U) && !ActorLibrarySound(s, a, 0x608))
                return false;
            a->shelf.force = left ? -0x600 : 0x600;
        }
    }
    if (SDL_abs(a->shelf.velocity) > 4096)
        a->shelf.damping = 2;
    return true;
}
bool ActorLibraryShelf(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 2 || !s->world || a->state.id != 0x1a)
        return SDL_SetError("Library bookcase requires its original actor and progress owner");
    const bool initializing = a->state.step == 0;
    if (initializing && !Initialize(s, a))
        return false;
    if (!initializing && a->state.step == 1 && !Contact(s, a))
        return false;
    if (a->state.step != 1 && a->state.step != 4)
        return SDL_SetError("Library bookcase entered an invalid original state");
    Sint64 velocity = (Sint64)a->shelf.velocity + a->shelf.force;
    if (velocity)
        velocity -= a->shelf.force / velocity;
    velocity -= velocity * a->shelf.damping / 256;
    if (velocity < -1048576 || velocity > 1048576)
        return SDL_SetError("Library bookcase angular velocity exceeds its bounded oscillator");
    a->shelf.velocity = (Sint32)velocity;
    a->shelf.angle = (Sint16)Wrap16((Sint64)a->shelf.angle + velocity / 4096);
    return ActorLibraryShelfGeometry(s, a);
}
