#include <SDL3/SDL_error.h>
#include <sotn/save_icon.h>

/* Layout metadata from the pinned config/assets.us.yaml; pixel and palette bytes stay private. */
enum
{
    PALETTE_OFFSET = 0x518,
    PIXEL_OFFSET = 0x718,
    PALETTE_BYTES = 32,
    ICON_BYTES = 384
};

static Uint8 ExpandColor(Uint16 color, unsigned int shift)
{
    return (Uint8)(((((Uint32)color >> shift) & 31U) * 255U + 15U) / 31U);
}

bool SotnSaveIcon_Decode(SotnDataView dra, Uint32 index, Uint8 *rgba, size_t capacity)
{
    if (dra.bytes == NULL || rgba == NULL || index >= SOTN_SAVE_ICON_COUNT ||
        capacity < SOTN_SAVE_ICON_RGBA_SIZE)
    {
        return SDL_SetError("Invalid save-icon input, index, or output capacity");
    }
    const size_t palette = PALETTE_OFFSET + (size_t)index * PALETTE_BYTES;
    const size_t pixels = PIXEL_OFFSET + (size_t)index * ICON_BYTES;
    if (pixels > dra.size || ICON_BYTES > dra.size - pixels || palette > dra.size ||
        PALETTE_BYTES > dra.size - palette)
    {
        return SDL_SetError("DRA data is truncated before the selected save icon");
    }
    for (size_t i = 0; i < SOTN_SAVE_ICON_WIDTH * SOTN_SAVE_ICON_HEIGHT; ++i)
    {
        const Uint8 packed = dra.bytes[pixels + i / 2];
        const Uint8 entry = (Uint8)((packed >> ((i % 2) * 4)) & 15U);
        const size_t offset = palette + (size_t)entry * 2;
        const Uint16 color =
            (Uint16)((Uint16)dra.bytes[offset] | ((Uint16)dra.bytes[offset + 1] << 8));
        rgba[i * 4] = ExpandColor(color, 0);
        rgba[i * 4 + 1] = ExpandColor(color, 5);
        rgba[i * 4 + 2] = ExpandColor(color, 10);
        rgba[i * 4 + 3] = color == 0 ? 0 : 255;
    }
    return true;
}
