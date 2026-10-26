// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected font, portrait, icon, and status/equipment presentation paths from
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5: dra/menu.c and 4AEA4.c.
// Artwork, strings, icons, and palettes are private runtime inputs.
#include <SDL3/SDL.h>
#include <sotn/inventory.h>
#include <sotn/relic.h>
struct SotnInventoryRenderer
{
    SotnDataView dra, common;
    Uint16 frame[SOTN_MENU_WIDTH * SOTN_MENU_HEIGHT];
};
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static void Rect(Uint16 *f, Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint16 color)
{
    for (Sint32 j = SDL_max(0, y); j < SDL_min(240, y + h); ++j)
        for (Sint32 i = SDL_max(0, x); i < SDL_min(384, x + w); ++i)
            f[(size_t)j * 384 + (size_t)i] = color;
}
static Uint16 Common(SotnDataView common, Uint32 page, Uint32 palette, Uint32 u, Uint32 v,
                     bool eight)
{
    const Uint32 x = (page & 15) * 64 - 512 + u / (eight ? 2U : 4U);
    const Uint32 block = x / 64 * 4 + (x % 64) / 32 + (v / 128) * 2;
    const Uint32 offset = block * 8192 + (v % 128) * 64 + (x % 32) * 2;
    const Uint32 word = Read16(common.bytes + offset),
                 index =
                     (word >> ((u & (eight ? 1U : 3U)) * (eight ? 8U : 4U))) & (eight ? 255U : 15U);
    return Read16(common.bytes + 0x40000 + ((palette - 256) * 16 + index) * 2);
}
static void Text(SotnInventoryRenderer *r, const char *text, Sint32 x, Sint32 y, Uint16 tint)
{
    for (; *text != '\0'; ++text, x += 8)
    {
        const Uint8 ch = (Uint8)((Uint8)*text - 32);
        const Uint32 u = (ch & 15U) * 8, v = (ch & 240U) >> 1;
        for (Sint32 j = 0; j < 8; ++j)
            for (Sint32 i = 0; i < 8; ++i)
                if (x + i >= 0 && x + i < 384 && y + j >= 0 && y + j < 240)
                {
                    Uint16 color =
                        Common(r->common, 0x1e, 0x196, u + (Uint32)i, v + (Uint32)j, false);
                    if (color != 0)
                        r->frame[(size_t)(y + j) * 384 + (size_t)(x + i)] = color & tint;
                }
    }
}
static void Number(SotnInventoryRenderer *r, Uint32 value, Sint32 x, Sint32 y)
{
    char text[16] = {0};
    SDL_snprintf(text, sizeof(text), "%u", value);
    Text(r, text, x, y, 0x7fff);
}
static void Panel(SotnInventoryRenderer *r, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
    for (Sint32 j = 0; j < h; ++j)
        Rect(r->frame, x, y + j, w, 1, (Uint16)((8 + 16 * j / SDL_max(1, h - 1)) << 10));
    Rect(r->frame, x, y, w, 1, 0x7fff);
    Rect(r->frame, x, y + h - 1, w, 1, 0x7fff);
    Rect(r->frame, x, y, 1, h, 0x7fff);
    Rect(r->frame, x + w - 1, y, 1, h, 0x7fff);
}
static bool Icon(SotnDataView dra, SotnItem item, Sint32 x, Sint32 y, Uint16 *f, Uint32 width,
                 Uint32 height)
{
    const size_t pixels = 0x25324 + (size_t)item.icon * 128,
                 palette = 0x388d4 + (size_t)item.palette * 32;
    if (dra.bytes == NULL || item.icon >= 320 || item.palette >= 320 || pixels > dra.size ||
        128 > dra.size - pixels || palette > dra.size || 32 > dra.size - palette)
        return SDL_SetError("Item icon references truncated private pixels or palette");
    for (Sint32 j = 0; j < 16; ++j)
        for (Sint32 i = 0; i < 16; ++i)
        {
            if (x + i < 0 || x + i >= (Sint32)width || y + j < 0 || y + j >= (Sint32)height)
                continue;
            const Uint8 index =
                (dra.bytes[pixels + (size_t)j * 8 + (size_t)i / 2] >> ((i & 1) * 4)) & 15;
            const Uint16 color = Read16(dra.bytes + palette + index * 2U);
            if (color != 0)
                f[(size_t)(y + j) * width + (size_t)(x + i)] = color & 0x7fff;
        }
    return true;
}
bool SotnItemIcon_Draw(SotnDataView dra, SotnItem item, Sint32 x, Sint32 y, Uint16 *f, size_t count)
{
    if (f == NULL || count < SOTN_FRAME_PIXELS || x < -1024 || x > 17408 || y < -1024 || y > 17408)
        return SDL_SetError("Item drawing requires bounded coordinates and a full frame");
    return Icon(dra, item, x, y, f, 256, 240);
}
bool SotnInventoryRenderer_Create(SotnDataView dra, SotnDataView common,
                                  SotnInventoryRenderer **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (dra.bytes == NULL || dra.size < 0x3b0d4 || common.bytes == NULL || common.size != 0x42000)
        return SDL_SetError(
            "Inventory presentation requires complete private item and common resources");
    SotnInventoryRenderer *r = SDL_calloc(1, sizeof(*r));
    if (r == NULL)
        return false;
    r->dra = dra;
    r->common = common;
    *out = r;
    return true;
}
static bool ItemName(SotnInventoryRenderer *r, bool body, Uint16 id, Sint32 x, Sint32 y,
                     Uint16 tint)
{
    SotnItem item = {0};
    char name[161] = {0};
    if (!SotnItem_Read(r->dra, body, id, &item) || !SotnItem_Name(item, name, sizeof(name)))
        return false;
    if (!Icon(r->dra, item, x - 20, y - 4, r->frame, 384, 240))
        return false;
    Text(r, name, x, y, tint);
    return true;
}
bool SotnInventoryRenderer_Draw(SotnInventoryRenderer *r, SotnInventoryMenu menu,
                                const SotnCharacter *c, Uint64 ticks, bool gamepad, Uint8 *rgba,
                                size_t capacity, const SotnWorldState *world)
{
    if (r == NULL || rgba == NULL || world == NULL || capacity < SOTN_MENU_RGBA_BYTES ||
        menu.phase < SOTN_MENU_MAIN || menu.phase > SOTN_MENU_RELICS || menu.slot >= 7 ||
        menu.count > 170 || menu.item_cursor >= 170 || menu.cursor >= 5 || menu.relic_cursor >= 28)
        return SDL_SetError(
            "Inventory rendering requires valid menu state and a full output buffer");
    SotnCharacterState state = {0};
    SotnCharacterStats stats = {0};
    if (!SotnCharacter_Get(c, &state, &stats))
        return false;
    SDL_memset(r->frame, 0, sizeof(r->frame));
    Panel(r, 0, 8, 384, 18);
    Text(r, "ALUCARD", 16, 13, 0x7fff);
    if (menu.phase == SOTN_MENU_MAIN)
    {
        Panel(r, 0, 28, 232, 192);
        Panel(r, 234, 28, 150, 192);
        for (Uint32 y = 0; y < 96; ++y)
            for (Uint32 x = 0; x < 64; ++x)
            {
                Uint16 color = Common(r->common, 0x9c, 0x150, x, 128 + y, true);
                if (color != 0)
                    r->frame[(36 + y) * 384 + 16 + x] = color & 0x7fff;
            }
        Text(r, "HP", 92, 36, 0x7fff);
        Number(r, state.hp, 116, 36);
        Text(r, "/", 148, 36, 0x7fff);
        Number(r, state.hp_max, 160, 36);
        Text(r, "MP", 92, 60, 0x7fff);
        Number(r, state.mp, 116, 60);
        Text(r, "/", 148, 60, 0x7fff);
        Number(r, state.mp_max, 160, 60);
        Text(r, "HEART", 92, 80, 0x7fff);
        Number(r, state.hearts, 140, 80);
        Text(r, "/", 172, 80, 0x7fff);
        Number(r, state.hearts_max, 184, 80);
        Text(r, "LEVEL", 92, 104, 0x7fff);
        Number(r, state.level, 164, 104);
        const char *labels[4] = {"STR", "CON", "INT", "LCK"};
        for (Uint32 i = 0; i < 4; ++i)
        {
            Text(r, labels[i], 32, 144 + (Sint32)i * 14, 0x7fff);
            Number(r, stats.total[i], 88, 144 + (Sint32)i * 14);
        }
        Uint32 remaining = 0;
        if (!SotnCharacter_NextLevel(c, &remaining))
            return false;
        Text(r, "NEXT", 128, 200, 0x7fff);
        Number(r, remaining, 168, 200);
        Text(r, "EXP", 128, 144, 0x7fff);
        Number(r, state.experience, 160, 144);
        Text(r, "GOLD", 128, 164, 0x7fff);
        Number(r, state.gold, 168, 164);
        Text(r, "KILLS", 128, 184, 0x7fff);
        Number(r, state.kills, 176, 184);
        const char *choices[5] = {"Equipment", "Spells", "Relics", "System", "Familiars"};
        for (Uint32 i = 0; i < 5; ++i)
        {
            const Sint32 y = 48 + (Sint32)i * 24;
            if (menu.cursor == i)
                Rect(r->frame, 240, y - 3, 136, 14, 12);
            Text(r, choices[i], 248, y, i == 0 || i == 2 ? 0x7fff : 0x4210);
        }
    }
    else if (menu.phase == SOTN_MENU_RELICS)
    {
        Panel(r, 0, 28, 384, 186);
        const Sint32 scroll = (menu.relic_cursor / 2) * 120 / 14;
        for (Uint16 cursor = 0; cursor < 28; ++cursor)
        {
            const Uint16 id = cursor < 23 ? cursor : (Uint16)(cursor + 2);
            if (world->relics[id] > 3 || world->relics[id] == 2)
                return SDL_SetError("Relic presentation requires valid found/active bits");
            const Sint32 x = 8 + (cursor & 1) * 176, y = 34 + (cursor / 2) * 19 - scroll;
            if (y < 34 || y > 184)
                continue;
            if (cursor == menu.relic_cursor)
                Rect(r->frame, x, y - 2, 168, 18, 12);
            const Uint16 tint = SotnRelic_CanToggle(id) ? 0x7fff : 0x4210;
            Text(r, world->relics[id] & 2 ? "ON" : "OFF", x, y + 2, tint);
            if (world->relics[id] & 1)
            {
                SotnItem item = {0};
                char name[161] = {0}, label[16] = {0};
                if (!SotnRelic_Read(r->dra, id, &item) ||
                    !SotnItem_Name(item, name, sizeof(name)) ||
                    !Icon(r->dra, item, x + 28, y - 2, r->frame, 384, 240))
                    return false;
                SDL_strlcpy(label, name, sizeof(label));
                Text(r, label, x + 48, y + 2, tint);
            }
        }
        const Uint16 id =
            menu.relic_cursor < 23 ? menu.relic_cursor : (Uint16)(menu.relic_cursor + 2);
        Text(r,
             !(world->relics[id] & 1)  ? "Relic not found"
             : SotnRelic_CanToggle(id) ? "Select to switch ON/OFF"
                                       : "Ability not integrated yet",
             8, 202, 0x7fff);
    }
    else
    {
        Panel(r, 0, 28, 94, 188);
        Panel(r, 96, 28, 288, 188);
        const char *labels[4] = {"STR", "CON", "INT", "LCK"};
        for (Uint32 i = 0; i < 4; ++i)
        {
            Text(r, labels[i], 8, 116 + (Sint32)i * 16, 0x7fff);
            Number(r, stats.total[i], 48, 116 + (Sint32)i * 16);
        }
        Text(r, "ATT", 8, 42, 0x7fff);
        Number(r, stats.attack[0], 48, 42);
        Number(r, stats.attack[1], 48, 56);
        Text(r, "DEF", 8, 78, 0x7fff);
        Number(r, stats.defense, 48, 78);
        if (menu.phase == SOTN_MENU_EQUIPMENT)
        {
            const char *slots[7] = {"Left hand", "Right hand",  "Head",       "Body",
                                    "Cape",      "Accessory 1", "Accessory 2"};
            for (Uint32 i = 0; i < 7; ++i)
            {
                const Sint32 y = 44 + (Sint32)i * 22;
                if (menu.slot == i)
                    Rect(r->frame, 100, y - 3, 280, 18, 12);
                if (!ItemName(r, i >= 2, state.equipment[i], 124, y, 0x7fff))
                    return false;
            }
            Text(r, slots[menu.slot], 104, 200, 0x7fff);
        }
        else
        {
            const Uint16 first = (Uint16)((menu.item_cursor / 10) * 10);
            for (Uint16 row = first; row < menu.count && row < first + 10; ++row)
            {
                const Uint16 id = menu.rows[row];
                SotnItem item = {0};
                if (!SotnCharacter_Item(c, menu.slot >= 2, id, &item))
                    return false;
                const Sint32 y = 40 + (Sint32)(row - first) * 17;
                if (row == menu.item_cursor)
                    Rect(r->frame, 100, y - 3, 280, 16, 12);
                if (!ItemName(r, menu.slot >= 2, id, 124, y, item.supported ? 0x7fff : 0x4210))
                    return false;
                const Uint8 owned = menu.slot >= 2 ? state.body_count[id] : state.hand_count[id];
                if (owned != 0)
                    Number(r, owned, 352, y);
            }
        }
    }
    (void)ticks;
    if (menu.message[0] != '\0')
    {
        Panel(r, 0, 214, 384, 26);
        char line[47] = {0};
        SDL_strlcpy(line, menu.message, sizeof(line));
        Text(r, line, 8, 224, 0x7fff);
    }
    else
        Text(r,
             gamepad ? "Start: close   A: select   Y: back"
                     : "Enter: close   X/Space: select   A: back",
             8, 226, 0x7fff);
    for (size_t i = 0; i < 384 * 240; ++i)
    {
        const Uint16 pixel = r->frame[i];
        for (Uint32 channel = 0; channel < 3; ++channel)
        {
            const Uint8 v = (Uint8)((pixel >> (channel * 5)) & 31);
            rgba[i * 4 + channel] = (Uint8)((v << 3) | (v >> 2));
        }
        rgba[i * 4 + 3] = 255;
    }
    return true;
}
void SotnInventoryRenderer_Destroy(SotnInventoryRenderer *r)
{
    SDL_free(r);
}
