// SPDX-License-Identifier: AGPL-3.0-or-later
// Font addressing follows dra/menu.c:MenuDrawChar at sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Font artwork remains private.
// Filesystem launch UI, labels and slot layout are original native adaptation.
#include <SDL3/SDL.h>
#include <sotn/launch_renderer.h>
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static void Text(SotnDataView common, Uint16 *f, const char *text, Sint32 x, Sint32 y, Uint16 tint)
{
    for (; *text; ++text, x += 8)
    {
        const Uint32 c = (Uint8)*text - 32U, u = (c & 15) * 8, v = (c & 240) >> 1;
        for (Uint32 dy = 0; dy < 8; ++dy)
            for (Uint32 dx = 0; dx < 8; ++dx)
            {
                if (x + (Sint32)dx < 0 || x + (Sint32)dx >= 256 || y + (Sint32)dy < 0 ||
                    y + (Sint32)dy >= 240)
                    continue;
                const Uint32 px = 384 + (u + dx) / 4, py = v + dy,
                             block = px / 64 * 4 + (px % 64) / 32 + (py / 128) * 2;
                const Uint32 offset = block * 8192 + (py % 128) * 64 + (px % 32) * 2;
                const Uint16 word = Read16(common.bytes + offset);
                const Uint32 index = (word >> (((u + dx) & 3) * 4)) & 15;
                const Uint16 color = Read16(common.bytes + 0x412c0 + index * 2);
                if (color)
                    f[(size_t)(y + (Sint32)dy) * 256 + (size_t)(x + (Sint32)dx)] = color & tint;
            }
    }
}
bool SotnLaunchRenderer_Draw(SotnDataView common, SotnLaunchMenu menu,
                             const SotnLaunchSlot slots[15], const char *error, Uint16 *frame,
                             size_t count)
{
    if (common.bytes == NULL || common.size != 0x42000 || slots == NULL || frame == NULL ||
        count < SOTN_FRAME_PIXELS || menu.phase < SOTN_LAUNCH_HOME ||
        menu.phase > SOTN_LAUNCH_ERROR || menu.slot < 1 || menu.slot > 15 || menu.choice > 1)
        return SDL_SetError(
            "Launch presentation requires common artwork, valid slots and a full frame");
    size_t error_length = 0;
    if (error != NULL)
    {
        error_length = SDL_strnlen(error, 160);
        if (error_length == 160)
            return SDL_SetError("Launch error message exceeds its bounded display");
        for (size_t i = 0; i < error_length; ++i)
            if ((Uint8)error[i] < 32 || (Uint8)error[i] > 126)
                return SDL_SetError("Launch error message must contain printable ASCII");
    }
    for (Uint32 y = 0; y < 240; ++y)
        for (Uint32 x = 0; x < 256; ++x)
            frame[y * 256 + x] = (Uint16)((6 + 12 * y / 239) << 10);
    Text(common, frame, "SYMPHONY OF THE NIGHT", 48, 12, 0x7fff);
    if (menu.phase == SOTN_LAUNCH_HOME)
    {
        Text(common, frame, "CONTINUE", 40, 84, menu.choice == 0 ? 0x7fff : 0x4210);
        Text(common, frame, "LABORATORY CHECKPOINT", 40, 112, menu.choice == 1 ? 0x7fff : 0x4210);
        Text(common, frame, ">", 24, menu.choice == 0 ? 84 : 112, 0x7fff);
        Text(common, frame, "FULL NEW GAME NOT AVAILABLE", 24, 164, 0x4210);
    }
    else if (menu.phase == SOTN_LAUNCH_SLOTS)
    {
        Text(common, frame, "CONTINUE", 96, 32, 0x7fff);
        for (Uint32 i = 0; i < 15; ++i)
        {
            char text[40] = {0};
            if (slots[i].valid)
                SDL_snprintf(text, sizeof(text), "%02u  L%02u HP%04u LAB%02u", i + 1,
                             slots[i].level, slots[i].hp, slots[i].room);
            else
                SDL_snprintf(text, sizeof(text), "%02u  %s", i + 1,
                             slots[i].exists ? "INVALID SAVE" : "EMPTY");
            const Sint32 y = 48 + (Sint32)i * 10;
            Text(common, frame, text, 32, y, menu.slot == i + 1 ? 0x7fff : 0x4210);
            if (menu.slot == i + 1)
                Text(common, frame, ">", 16, y, 0x7fff);
        }
    }
    else
    {
        Text(common, frame, "CANNOT CONTINUE", 64, 56, 0x7fff);
        const char *message = error_length ? error : "This slot has no valid save.";
        for (Uint32 row = 0; row < 6 && *message; ++row)
        {
            char line[29] = {0};
            size_t len = SDL_min(SDL_strlen(message), 28U);
            SDL_memcpy(line, message, len);
            message += len;
            Text(common, frame, line, 16, 84 + (Sint32)row * 12, 0x7fff);
        }
    }
    Text(common, frame, "ARROWS: SELECT  X: CONFIRM", 16, 216, 0x7fff);
    Text(common, frame, "A: BACK        ESC: QUIT", 16, 228, 0x7fff);
    return true;
}
