// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected direct-stream UpdateAnim rules from dra/alu_anim.c at the pinned reference.
// Frame properties and animation-switch opcodes require their own explicit adapters.
#include "actors_internal.h"
static bool Frame(SotnActors *s, const Actor *a, size_t offset, SotnDataView *out)
{
    if (a->pose >= 512 || offset > s->nz0.size || (size_t)a->pose * 4 > s->nz0.size - offset)
        return SDL_SetError("Wide actor animation cursor exceeds its bounded private stream");
    return Slice(s->nz0, offset + (size_t)a->pose * 4, 4, out);
}
bool ActorWideAnimation(SotnActors *s, Actor *a, size_t offset, bool *ended)
{
    SotnDataView frame = {0};
    if (ended == NULL || !Frame(s, a, offset, &frame))
        return false;
    *ended = a->remaining == SDL_MAX_UINT16;
    if (!*ended)
    {
        if (a->remaining == 0)
        {
            const Uint16 duration = Read16(frame.bytes);
            if (duration == 0 || duration >= 0x8000)
                return SDL_SetError("Wide animation must start on a positive duration");
            a->remaining = duration;
        }
        else if (--a->remaining == 0)
        {
            ++a->pose;
            if (!Frame(s, a, offset, &frame))
                return false;
            Uint16 duration = Read16(frame.bytes);
            if (duration == 0)
            {
                a->pose = Read16(frame.bytes + 2);
                if (!Frame(s, a, offset, &frame))
                    return false;
                duration = Read16(frame.bytes);
                if (duration == 0 || duration >= 0x8000)
                    return SDL_SetError("Wide animation loop has an invalid target");
            }
            else if (duration == SDL_MAX_UINT16)
            {
                --a->pose;
                a->remaining = SDL_MAX_UINT16;
                *ended = true;
                if (!Frame(s, a, offset, &frame))
                    return false;
            }
            else if (duration >= 0x8000)
                return SDL_SetError("Wide animation contains an unintegrated switch opcode");
            if (!*ended)
                a->remaining = duration;
        }
    }
    a->state.frame = Read16(frame.bytes + 2) & 0x1ff;
    return true;
}
