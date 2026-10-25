// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected DAI e_confessional.c, sotn-decomp 7bc54d8157b6. Position tables remain private.
#include "chapel_confessional_internal.h"
static bool Move(Actor *a)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
        return SDL_SetError("Confessional debris exceeds fixed-point motion bounds");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    return true;
}
static bool Debris(SotnActors *s, Actor *a)
{
    if (a->state.parameters > 1)
        return SDL_SetError("Confessional debris has an invalid side");
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x9d4))
            return false;
        a->state.frame = 42;
        a->state.rotated = true;
        a->state.priority = 158;
        a->vx = (Sint32)(ActorRandom(s) << 8) + 16384;
        if (!a->state.parameters)
            a->vx = -a->vx;
        a->vy = -(Sint32)ActorRandom(s) * 256 - 65536;
        a->timer = (Uint16)((ActorRandom(s) & 63U) + 16U);
    }
    if (a->state.step != 1 || !a->timer || !Move(a))
        return SDL_SetError("Confessional debris has invalid original state/motion");
    a->vy += 8192;
    a->state.rotation =
        (Sint16)Wrap16((Sint32)a->state.rotation + (a->state.parameters ? 32 : -32));
    a->remove = --a->timer == 0;
    return true;
}
bool ConfessionalBlades(SotnActors *s, Actor *a)
{
    if (a->state.id == 0x25)
        return Debris(s, a);
    if (a->state.parameters > 4)
        return SDL_SetError("Confessional blade has an invalid private position row");
    if (a->state.flags & 0x100U)
    {
        if (!ActorExplosionInPool(s, a, 0, 160, 192, 0))
            return false;
        a->remove = true;
        return true;
    }
    SotnDataView row = {0};
    if (!Slice(s->nz0, 0xe00U + a->state.parameters * 6U, 6, &row))
        return false;
    const Sint32 start = Signed16(row.bytes), y = Signed16(row.bytes + 2),
                 end = Signed16(row.bytes + 4);
    if (start < 0 || start > 256 || end < 0 || end > 256 || y < 0 || y > 256)
        return SDL_SetError("Confessional blade position exceeds its private screen bounds");
    const bool parish = a->state.parameters == 4;
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x9d4))
            return false;
        a->state.priority = 158;
        a->state.frame = (Uint16)(a->state.frame + a->state.parameters);
        a->state.x = (start + s->camera_x) * 65536;
        a->state.y = (y + s->camera_y) * 65536;
        a->state.step = parish ? 3 : 1;
        a->timer = 128;
        for (Uint16 i = 0; i < 4; ++i)
        {
            Actor *child = NULL;
            if (!ActorSpawn(s, a, 0x25, parish ? 1 : 0, 160, 192, &child))
                return false;
        }
        return ActorSound(s, 0x655);
    case 1:
    case 3:
        a->state.x += parish ? 229376 : -163840;
        if (parish ? Pixel(a->state.x) - s->camera_x > end : Pixel(a->state.x) - s->camera_x < end)
        {
            a->state.x = (end + s->camera_x) * 65536 + (Sint32)((Uint32)a->state.x & 65535U);
            a->timer = 96;
            ConfessionalStep(a, parish ? 4 : 2);
        }
        return true;
    case 2:
    case 4:
        if (a->timer)
        {
            --a->timer;
            return true;
        }
        a->state.x += parish ? -229376 : 163840;
        a->remove = parish ? Pixel(a->state.x) - s->camera_x < start
                           : Pixel(a->state.x) - s->camera_x > start;
        return true;
    case 5:
        return true;
    default:
        return SDL_SetError("Confessional blade entered an invalid original state");
    }
}
