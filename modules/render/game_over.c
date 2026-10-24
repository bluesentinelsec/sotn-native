// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected HandleGameOver/func_800E5358 paths from sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5, src/dra/game_handlers.c.
// Private F_GO.BIN contains all artwork and palettes; no bitmap data is embedded.
#include <SDL3/SDL.h>
#include <sotn/combat.h>
#include <sotn/game_over.h>

typedef struct Column
{
    Uint16 top, bottom;
    Uint8 delay, acceleration;
} Column;

struct SotnGameOver
{
    SotnDataView artwork;
    Uint16 capture[SOTN_FRAME_PIXELS];
    Column columns[256];
    Uint32 random;
    Uint16 timer;
    Uint8 phase, intensity, row, hold_timer, hold_count;
};

static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}

bool SotnGameOver_Create(SotnDataView artwork, const Uint16 *frame, size_t count,
                         SotnGameOver **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (artwork.bytes == NULL || artwork.size != 0x1c000 || frame == NULL ||
        count < SOTN_FRAME_PIXELS)
        return SDL_SetError("Game over requires verified F_GO.BIN and a full scene capture");
    SotnGameOver *game_over = SDL_calloc(1, sizeof(*game_over));
    if (game_over == NULL)
        return false;
    game_over->artwork = artwork;
    SDL_memcpy(game_over->capture, frame, sizeof(game_over->capture));
    game_over->timer = 96;
    for (Uint32 i = 0; i < 256; ++i)
    {
        game_over->columns[i].bottom = 240;
        game_over->columns[i].delay = (Uint8)((SotnCombat_Random(&game_over->random) & 31) + 1);
    }
    *out = game_over;
    return true;
}

bool SotnGameOver_Tick(SotnGameOver *game_over, bool start)
{
    if (game_over == NULL)
        return SDL_InvalidParamError("game_over");
    if (game_over->phase == 5)
        return true;
    for (Uint32 i = 0; i < 256; ++i)
    {
        Column *column = &game_over->columns[i];
        if (column->top > 256)
            continue;
        if (column->delay != 0)
            --column->delay;
        else
        {
            if (column->acceleration < 10 && (SotnCombat_Random(&game_over->random) & 1))
                ++column->acceleration;
            else
                column->top += (Uint16)(1 + ((SotnCombat_Random(&game_over->random) & 3) == 0));
            ++column->bottom;
        }
    }
    switch (game_over->phase)
    {
    case 0:
        if (--game_over->timer == 0)
            game_over->phase = 1;
        break;
    case 1:
        game_over->intensity += 2;
        if (game_over->intensity == 128)
        {
            game_over->phase = 2;
            game_over->timer = 4;
        }
        break;
    case 2:
        if (--game_over->timer == 0)
        {
            game_over->row = (Uint8)(game_over->row + 32);
            game_over->timer = 4;
            if (game_over->row == 96)
            {
                game_over->phase = 3;
                game_over->hold_count = 1;
            }
        }
        break;
    case 3:
        if (start || (--game_over->hold_timer == 0 && game_over->hold_count-- == 0))
            game_over->phase = 4;
        break;
    case 4:
        if (--game_over->intensity == 0)
            game_over->phase = 5;
        break;
    default:
        return SDL_SetError("Game-over presentation entered an invalid phase");
    }
    return true;
}

static Uint16 Shade(Uint16 color, Uint8 intensity)
{
    Uint16 result = color & 0x8000;
    for (Uint32 shift = 0; shift < 15; shift += 5)
        result |= (Uint16)((((color >> shift) & 31U) * intensity / 128) << shift);
    return result;
}

bool SotnGameOver_Draw(const SotnGameOver *game_over, Uint16 *frame, size_t count)
{
    if (game_over == NULL || frame == NULL || count < SOTN_FRAME_PIXELS)
        return SDL_SetError("Game-over drawing requires initialized state and a full framebuffer");
    const Uint8 *art = game_over->artwork.bytes;
    if (game_over->phase == 5)
    {
        SDL_memset(frame, 0, SOTN_FRAME_PIXELS * sizeof(*frame));
        return true;
    }
    for (Uint32 y = 0; y < 240; ++y)
        for (Uint32 x = 0; x < 256; ++x)
        {
            const size_t pixel = y * 256 + x;
            const Uint8 index = art[0x8000 + pixel];
            Uint16 color = Read16(art + 0x1a000 + (size_t)index * 2);
            if (game_over->phase == 4)
                color = Shade(color, game_over->intensity);
            frame[pixel] = color;
        }
    for (Uint32 x = 0; x < 256; ++x)
    {
        const Column column = game_over->columns[x];
        for (Uint32 y = column.top; y < 240 && y < column.bottom; ++y)
        {
            const Uint32 source = (y - column.top) * 240 / (column.bottom - column.top);
            frame[y * 256 + x] = game_over->capture[source * 256 + x];
        }
    }
    if (game_over->intensity != 0)
        for (Uint32 y = 0; y < 32; ++y)
            for (Uint32 x = 0; x < 255; ++x)
            {
                const size_t pixel = ((size_t)game_over->row + y) * 256 + x;
                const Uint8 index = (art[pixel / 2] >> ((pixel & 1) * 4)) & 15;
                const Uint16 color = Read16(art + 0x18000 + (size_t)index * 2);
                if (color != 0)
                    frame[(y + 96) * 256 + x] = Shade(color, game_over->intensity);
            }
    return true;
}

bool SotnGameOver_Finished(const SotnGameOver *game_over)
{
    return game_over != NULL && game_over->phase == 5;
}

void SotnGameOver_Destroy(SotnGameOver *game_over)
{
    SDL_free(game_over);
}
