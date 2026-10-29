// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/4A538.c glow job and 692E8.c:func_801092E8,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Colors remain private.
#include <SDL3/SDL.h>
#include <sotn/player_glow.h>
static Uint32 Read32(const Uint8 *p)
{
    return p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
}
static bool Pair(SotnPlayerGlow *g, SotnDataView dra, bool absorbing)
{
    const size_t at = 0xce48U + (absorbing ? 8U : 0U);
    if (!dra.bytes || dra.size < at + 8 || Read32(dra.bytes + at) > 65535 ||
        Read32(dra.bytes + at + 4) > 65535)
        return SDL_SetError("Player glow requires bounded private absorption color pairs");
    g->targets[0] = (Uint16)Read32(dra.bytes + at);
    g->targets[1] = (Uint16)Read32(dra.bytes + at + 4);
    g->absorbing = absorbing;
    g->color = g->targets[0];
    return true;
}
bool SotnPlayerGlow_Init(SotnPlayerGlow *out, SotnDataView dra)
{
    if (!out || !dra.bytes || dra.size < 0x3bb8 + 24U * 4U)
        return SDL_SetError("Player glow requires its owner and private original palette job23");
    const Uint32 address = Read32(dra.bytes + 0x3bb8 + 23U * 4U);
    if (address % 4U || address < 0x800a0000U || address - 0x800a0000U > dra.size ||
        dra.size - (address - 0x800a0000U) < 24)
        return SDL_SetError("Player glow descriptor exceeds its private main image");
    const Uint8 *p = dra.bytes + (address - 0x800a0000U);
    const Uint32 op = Read32(p);
    if ((op & 65535U) != 4 || !(op >> 16) || Read32(p + 4) != 0x100a || Read32(p + 8) != 1 ||
        Read32(p + 12) != 0x800a37d8U || Read32(p + 16) != 0x800a37daU ||
        Read32(p + 20) != SDL_MAX_UINT32)
        return SDL_SetError("Player glow requires its actual one-color two-target original job");
    SotnPlayerGlow next = {.interval = (Uint16)(op >> 16), .initialized = true};
    if (!Pair(&next, dra, false))
        return false;
    *out = next;
    return true;
}
bool SotnPlayerGlow_Set(SotnPlayerGlow *g, SotnDataView dra, bool absorbing)
{
    if (!g || !g->initialized)
        return SDL_SetError("Player absorption palette requires its real glow owner");
    if (g->absorbing == absorbing)
        return true;
    SotnPlayerGlow next = *g;
    if (!Pair(&next, dra, absorbing))
        return false;
    *g = next;
    return true;
}
bool SotnPlayerGlow_Tick(SotnPlayerGlow *g)
{
    if (!g || !g->initialized || g->phase > 1 || !g->interval || g->delay >= g->interval)
        return SDL_SetError("Player glow lost its initialized bounded palette job cursor");
    if (g->delay)
    {
        --g->delay;
        return true;
    }
    const Uint16 before = g->color, target = g->targets[g->phase];
    Uint16 color = before & 0x8000U;
    for (Uint16 shift = 0; shift < 15; shift += 5)
    {
        const Sint32 from = (before >> shift) & 31, to = (target >> shift) & 31;
        color |= (Uint16)((Uint16)(from + (from < to) - (from > to)) << shift);
    }
    g->color = color;
    g->delay = (Uint16)(g->interval - 1U);
    if (color == before)
        g->phase ^= 1;
    return true;
}
