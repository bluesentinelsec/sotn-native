// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected normal-castle st/e_castle_walls.h and st/e_staircase.h at the pinned
// sotn-decomp revision. Artwork/palettes remain private runtime inputs.
#include "actors_internal.h"
static bool Reserve(SotnActors *s, Actor *a, Uint16 count)
{
    if (!ActorInitialize(s, a, 0x968))
        return false;
    if (!ActorPrimitives(s, a, count))
    {
        a->remove = true;
        return true;
    }
    a->state.flags |= 0x800000U;
    return true;
}
static bool Walls(SotnActors *s, Actor *a)
{
    const bool narrow = a->state.id == 0x1c;
    const Uint16 rows = a->state.parameters & 15U;
    if ((narrow && a->state.parameters > 1) ||
        (!narrow && (!rows || (a->state.parameters & ~0x10fU))))
        return SDL_SetError("Chapel wall variant exceeds its original geometry");
    if (a->state.step)
        return a->state.step == 1 || SDL_SetError("Chapel wall entered an invalid state");
    const Uint16 count = narrow ? 2 : (Uint16)(rows * 3U);
    if (!Reserve(s, a, count))
        return false;
    if (a->remove)
        return true;
    Sint32 position = 0;
    for (Uint16 i = 0; i < count; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        p->state = (SotnActorPrimitive){.x = Pixel(a->state.x) * 65536,
                                        .y = Pixel(a->state.y) * 65536,
                                        .page = 15,
                                        .palette = (Uint16)(narrow ? 0x74 : 0xc8),
                                        .active = true,
                                        .textured = true,
                                        .quad = true,
                                        .priority_set = true,
                                        .priority = 160};
        Sint32 left = 0, right = 0, top = 0, bottom = 0;
        if (narrow)
        {
            const Sint32 width = 80 - i * 16;
            const Uint8 u = (Uint8)(115 - i * 16 - a->state.parameters);
            p->state.u[0] = p->state.u[2] = u;
            p->state.u[1] = p->state.u[3] = (Uint8)(u - width);
            p->state.v[0] = p->state.v[1] = 67;
            p->state.v[2] = p->state.v[3] = 163;
            left = position;
            position += a->state.parameters ? width : -width;
            right = position;
            bottom = 96;
        }
        else
        {
            const Sint32 direction = (a->state.parameters & 0x100U) ? -1 : 1;
            left = direction * (i % 3) * 61;
            right = left + direction * 61;
            top = i / 3 * 61;
            bottom = top + 61;
            p->state.u[0] = p->state.u[2] = 193;
            p->state.u[1] = p->state.u[3] = 254;
            p->state.v[0] = p->state.v[1] = 193;
            p->state.v[2] = p->state.v[3] = 254;
        }
        for (Uint16 j = 0; j < 4; ++j)
        {
            p->state.offset_x[j] = (Sint16)((j & 1U) ? right : left);
            p->state.offset_y[j] = (Sint16)((j & 2U) ? bottom : top);
        }
    }
    return true;
}
static bool Staircase(SotnActors *s, Actor *a)
{
    if (!a->state.step && !Reserve(s, a, 1))
        return false;
    if (a->remove)
        return true;
    if (a->state.step != 1)
        return SDL_SetError("Chapel staircase shadow entered an invalid state");
    ActorPrimitive *p = ActorPrimitiveGet(s, a, 0);
    if (!p)
        return false;
    const Sint32 player_y = Pixel(s->player.y) - s->camera_y;
    const Sint32 top = 112 - SDL_max(0, 128 - player_y);
    if (top < SDL_MIN_SINT16 || top > SDL_MAX_SINT16)
        return SDL_SetError("Chapel staircase shadow exceeds screen-coordinate bounds");
    p->state = (SotnActorPrimitive){.offset_x = {104, 256, 256, 256},
                                    .offset_y = {232, (Sint16)top, 240, 240},
                                    .red = 64,
                                    .green = 56,
                                    .blue = 48,
                                    .active = true,
                                    .screen_space = true,
                                    .quad = true,
                                    .priority_set = true,
                                    .priority = 94};
    s->backdrop_color = (16U / 8U) | ((8U / 8U) << 5) | ((56U / 8U) << 10);
    return true;
}
bool ActorChapelScenery(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 6)
        return SDL_SetError("Chapel scenery requires its original stage owner");
    if (a->state.id == 0x1c || a->state.id == 0x1d)
        return Walls(s, a);
    if (a->state.id == 0x1e)
        return Staircase(s, a);
    return SDL_SetError("Chapel scenery identity is invalid");
}
