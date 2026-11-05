// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/42398.c:MainGame, sotn-decomp7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// The BIOS rand call uses the existing native libc adapter, not embedded BIOS code.
#include <SDL3/SDL_error.h>
#include <sotn/combat.h>
#include <sotn/main_loop.h>
bool SotnMainLoop_Advance(SotnMainLoopTail *tail, Sint32 vcount, bool menu, bool map_pause)
{
    if (!tail)
        return SDL_InvalidParamError("main-loop tail");
    SotnMainLoopTail next = *tail;
    next.map_cursor += (Uint32)SotnPlaytime_FrameDelta(vcount);
    if (!SotnPlaytime_Advance(&next.playtime, vcount, menu, map_pause))
        return false;
    (void)SotnCombat_Random(&next.random);
    *tail = next;
    return true;
}
