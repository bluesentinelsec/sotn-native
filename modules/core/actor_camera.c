// SPDX-License-Identifier: AGPL-3.0-or-later
// st/e_lock_camera.h, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
#include "actors_internal.h"
bool ActorCamera(SotnActors *s, Actor *a)
{
    SotnDataView hitboxes = {0}, axes = {0}, bounds = {0};
    const Uint16 parameter = a->state.parameters;
    if (parameter >= 2 || !Slice(s->nz0, 0xdbc, 4, &hitboxes) || !Slice(s->nz0, 0xdc0, 4, &axes))
        return SDL_SetError("Camera lock parameter is outside its private tables");
    if (a->state.step == 0)
    {
        if (!ActorInitialize(s, a, 0xc10))
            return false;
        a->state.hitbox_state = 1;
        a->camera.horizontal = axes.bytes[parameter] != 0;
        a->state.hitbox_width = a->camera.horizontal ? hitboxes.bytes[parameter] : 20;
        a->state.hitbox_height = a->camera.horizontal ? 20 : hitboxes.bytes[parameter];
        a->camera.side = 2;
    }
    if (Distance(a, s->player) <= a->state.hitbox_width &&
        SDL_abs(Pixel(s->player.y) - Pixel(a->state.y)) <= a->state.hitbox_height)
    {
        const Uint16 side =
            (Uint16)(a->camera.horizontal ? (Pixel(a->state.y) > Pixel(s->player.y) ? 4 : 0)
                                          : Side(a, s->player) * 4);
        if (side != a->camera.side)
        {
            const size_t offset = (size_t)parameter * 16 + side * 2;
            // NZ0 supplies 16 total properties; other stages require their own validated tables.
            if (offset > 32 || 32 - offset < 8 || !Slice(s->nz0, 0xdc4 + offset, 8, &bounds))
                return SDL_SetError("Camera lock property selection exceeds the Laboratory table");
            a->camera.side = side;
            a->camera.timer = 16;
            a->camera.pending = 0;
            for (Uint32 i = 0; i < 4; ++i)
                a->camera.target[i] = Read16(bounds.bytes + i * 2);
            SotnCamera next = *s->camera;
            if (!SotnCamera_Bounds(&next, a->camera.target[0], a->camera.target[1],
                                   a->camera.target[2], a->camera.target[3]))
                return false;
            next = *s->camera;
            if (a->camera.horizontal)
            {
                if (next.x != a->camera.target[0])
                {
                    a->camera.pending |= 1;
                    next.left = next.x;
                }
                else
                    next.left = a->camera.target[0];
                next.top = a->camera.target[1];
                if (next.x + 256 != a->camera.target[2])
                {
                    a->camera.pending |= 4;
                    next.right = next.x + 256;
                }
                else
                    next.right = a->camera.target[2];
                next.bottom = a->camera.target[3];
            }
            else
            {
                next.left = a->camera.target[0];
                next.right = a->camera.target[2];
                if (next.y - 4 != a->camera.target[1])
                {
                    a->camera.pending |= 2;
                    next.top = next.y - 4;
                }
                else
                    next.top = a->camera.target[1];
                if (next.y - 4 + 256 != a->camera.target[3])
                {
                    a->camera.pending |= 8;
                    next.bottom = next.y - 4 + 256;
                }
                else
                    next.bottom = a->camera.target[3];
            }
            if (!SotnCamera_Bounds(s->camera, next.left, next.top, next.right, next.bottom))
                return false;
        }
    }
    else
        a->camera.side = 2;
    if (a->camera.pending)
    {
        Sint32 values[4] = {s->camera->left, s->camera->top, s->camera->right, s->camera->bottom};
        if (--a->camera.timer == 0)
        {
            for (Uint32 i = 0; i < 4; ++i)
                values[i] = a->camera.target[i];
            a->camera.pending = 0;
        }
        else
            for (Uint32 i = 0; i < 4; ++i)
            {
                const Sint32 difference = (a->camera.target[i] - values[i]) / 2;
                if (difference)
                    values[i] += difference > 0 ? 2 : -2;
                else
                {
                    values[i] = a->camera.target[i];
                    a->camera.pending &= (Uint16) ~(1U << i);
                }
            }
        if (!SotnCamera_Bounds(s->camera, values[0], values[1], values[2], values[3]))
            return false;
    }
    return true;
}
