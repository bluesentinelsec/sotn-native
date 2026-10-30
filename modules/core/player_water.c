// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/692E8.c water context/collision scaling, pinned sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. No private tables are embedded.
#include "player_internal.h"

bool SotnPlayer_SetWaterContext(SotnPlayer *p, SotnPlayerWater water)
{
    if (!p || water.feet > 32767 || water.waist > water.feet || water.head > water.waist)
        return SDL_SetError("Player water requires actual bounded feet/waist/head depths");
    p->water = water;
    return true;
}
Sint32 SotnPlayer_WaterScale(Sint32 velocity, bool restore)
{
    const Uint32 bits = (Uint32)velocity * (restore ? 4U : 3U);
    const Sint32 product =
        bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
    return product / (restore ? 3 : 4);
}
bool SotnPlayer_WaterDrag(const SotnPlayer *p)
{
    const Uint32 status = SotnPlayer_Status(p);
    return !(status & 0x50U) && !p->collision_invincibility &&
           (p->water.feet >= 41 || ((status & 4U) && p->water.feet > 12));
}
