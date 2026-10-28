// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/5D5BC.c:func_800FE044 relic branch and shared e_collect.h, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5; active switching from dra/menu.c. Definitions and names
// remain private.
#include <SDL3/SDL.h>
#include <sotn/relic.h>
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static Uint32 Read32(const Uint8 *p)
{
    return p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
}
static bool View(SotnDataView dra, Uint64 address, size_t size, const Uint8 **out)
{
    if (dra.bytes == NULL || address < 0x800a0000 || address > SDL_MAX_UINT32 ||
        address - 0x800a0000 > dra.size || size > dra.size - (address - 0x800a0000))
        return SDL_SetError("Relic definition references truncated private DRA data");
    *out = dra.bytes + (size_t)(address - 0x800a0000);
    return true;
}
bool SotnRelic_Read(SotnDataView dra, Uint16 id, SotnItem *out)
{
    const Uint8 *pointer = NULL, *record = NULL, *name = NULL;
    if (out == NULL || id >= SOTN_RELICS || !View(dra, 0x800a00e0, 4, &pointer) ||
        !View(dra, (Uint64)Read32(pointer) + id * 16U, 16, &record) ||
        !View(dra, Read32(record), 1, &name))
        return SDL_SetError("Relic requires a bounded original definition and output");
    const size_t available = SDL_min((size_t)160, dra.size - (size_t)(name - dra.bytes));
    size_t length = 0;
    while (length < available && name[length] != 0)
    {
        if (name[length] < 32 || name[length] > 126)
            return SDL_SetError("Relic name contains unsupported US text bytes");
        ++length;
    }
    const Uint16 icon = Read16(record + 8), palette = Read16(record + 10);
    const Uint32 familiar = Read32(record + 12);
    if (length == available || icon >= 320 || palette >= 320 || familiar > 7)
        return SDL_SetError("Relic name, icon, palette, or familiar identity is invalid");
    *out = (SotnItem){.id = id,
                      .name = {name, length},
                      .icon = icon,
                      .palette = palette,
                      .type = (Uint16)familiar,
                      .supported = familiar == 0,
                      .name_ascii = true};
    return true;
}
bool SotnRelic_Collect(SotnWorldState *world, SotnDataView dra, Uint16 id)
{
    SotnItem relic = {0};
    if (world == NULL || !SotnRelic_Read(dra, id, &relic))
        return SDL_SetError(
            "Relic collection requires owned progress and a valid private definition");
    world->relics[id] = relic.type ? 1 : 3;
    return true;
}

bool SotnRelic_CanToggle(Uint16 id)
{
    return id == SOTN_RELIC_BAT || id == SOTN_RELIC_FIRE_BAT || id == SOTN_RELIC_CUBE ||
           id == SOTN_RELIC_GRAVITY || id == SOTN_RELIC_LEAP || id == SOTN_RELIC_JEWEL ||
           id == SOTN_RELIC_MIST || id == SOTN_RELIC_POWER_MIST || id == SOTN_RELIC_GAS_CLOUD;
}

bool SotnRelic_Toggle(SotnWorldState *world, SotnDataView dra, Uint16 id)
{
    SotnItem relic = {0};
    if (world == NULL || id >= SOTN_RELICS || !SotnRelic_Read(dra, id, &relic))
        return SDL_SetError("Relic switch requires owned progress and a valid private definition");
    if (world->relics[id] != 1 && world->relics[id] != 3)
        return SDL_SetError("Only a found relic with valid active bits can be switched");
    if (!SotnRelic_CanToggle(id) || relic.type != 0)
        return SDL_SetError("This relic's active behavior is not integrated yet");
    world->relics[id] ^= 2;
    return true;
}
