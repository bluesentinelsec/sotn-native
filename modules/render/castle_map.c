// SPDX-License-Identifier: AGPL-3.0-or-later
// Reviewed map reveal/passage rules: dra/5087C.c:func_800F1B08, RevealSecretPassageOnMap;
// orientation/fade: dra/624DC.c. sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
// Original blueprint, palette, and secret-passage table stay in private resources.
#include <SDL3/SDL.h>
#include <sotn/castle_map.h>
struct SotnMapRenderer
{
    SotnDataView blueprint, common, dra;
    Uint8 pixels[256 * 256], source[256 * 256];
};
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
bool SotnMapRenderer_Create(SotnDataView blueprint, SotnDataView common, SotnDataView dra,
                            SotnMapRenderer **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (blueprint.bytes == NULL || blueprint.size != 32768 || common.bytes == NULL ||
        common.size != 0x42000 || dra.bytes == NULL || dra.size < 0x2c10)
        return SDL_SetError("Map presentation requires complete private blueprint, common palette "
                            "and passage table");
    // Validate every private record before allowing drawing to modify a frame.
    bool terminated = false;
    for (Uint32 i = 0; i < 16; ++i)
    {
        const Uint8 *p = dra.bytes + 0x2bc0 + i * 5;
        if (p[0] == 0)
        {
            terminated = true;
            break;
        }
        if (p[0] >= 64 || p[1] >= 64 || p[2] < 1 || p[2] > 4)
            return SDL_SetError("Private secret-passage metadata is outside the map");
    }
    if (!terminated)
        return SDL_SetError("Private secret-passage table lacks its bounded terminator");
    SotnMapRenderer *r = SDL_calloc(1, sizeof(*r));
    if (r == NULL)
        return false;
    r->blueprint = blueprint;
    r->common = common;
    r->dra = dra;
    *out = r;
    return true;
}
static Uint8 Blueprint(const SotnMapRenderer *r, Uint32 x, Uint32 y)
{
    return (r->blueprint.bytes[(size_t)y * 128 + x / 2] >> ((x & 1) * 4)) & 15;
}
static void Patch(SotnMapRenderer *r, Uint32 cx, Uint32 cy, Sint32 kind)
{
    for (Uint32 y = 0; y < 4; ++y)
        for (Uint32 x = 0; x < 5; ++x)
            r->source[(cy * 4 + y) * 256 + cx * 4 + x] =
                (Uint8)(kind < 0                                           ? 0
                        : ((kind == 1 && x == 0) || (kind != 2 && x == 4)) ? 0
                                                                           : 1);
}
static void Repaint(SotnMapRenderer *r, bool inverted)
{
    for (Uint32 y = 0; y < 256; ++y)
        for (Uint32 x = 0; x < 256; ++x)
            r->source[y * 256 + x] = Blueprint(r, x, y);
    Patch(r, 14, 43, inverted ? 0 : -1);
    Patch(r, 16, 43, inverted ? 1 : -1);
    Patch(r, 43, 42, inverted ? 2 : -1);
    Patch(r, 44, 42, inverted ? 2 : -1);
    Patch(r, 45, 42, inverted ? 2 : -1);
    Patch(r, 48, 42, inverted ? 2 : -1);
}
static void Reveal(SotnMapRenderer *r, Uint32 cx, Uint32 cy, bool purchased)
{
    for (Uint32 dy = 0; dy < 5; ++dy)
        for (Uint32 dx = 0; dx < 5; ++dx)
        {
            const Uint32 x = cx * 4 + dx, y = cy * 4 + dy;
            if (x >= 256 || y >= 256)
                continue;
            Uint8 *pixel = &r->pixels[y * 256 + x];
            const Uint8 source = r->source[y * 256 + x];
            if (purchased)
            {
                if (*pixel == 0)
                    *pixel = source == 0 ? 13 : 3;
            }
            else if (*pixel == 0 || *pixel == 3 || *pixel == 13)
                *pixel = source == 0 ? 14 : source == 2 ? 1 : source;
        }
}
static void Passages(SotnMapRenderer *r, const SotnWorldState *s, bool inverted)
{
    for (Uint32 i = 0; i < 16; ++i)
    {
        const Uint8 *p = r->dra.bytes + 0x2bc0 + i * 5;
        if (p[0] == 0)
            break;
        const Uint32 flag = p[inverted ? 4 : 3];
        if (flag == 255 || !s->flags[flag])
            continue;
        const Uint32 cx = inverted ? 63U - p[0] : p[0], cy = inverted ? 63U - p[1] : p[1];
        const Uint32 idx = cx / 4 + cy * 16 + (inverted ? 1024U : 0U), bit = (3 - (cx & 3)) * 2;
        if (!(s->map[idx] & (1U << bit)))
            continue;
        const Uint32 x = p[0] * 4U, y = p[1] * 4U;
        Uint32 dx = 2, dy = 2, sx = 2, sy = 2;
        // Original WALL_TOP/LEFT/BOTTOM/RIGHT are 1/2/3/4.
        if (p[2] == 1)
        {
            dy = 0;
            sy = 1;
        }
        if (p[2] == 2)
        {
            dx = 0;
            sx = 1;
        }
        if (p[2] == 3)
        {
            dy = 4;
            sy = 3;
        }
        if (p[2] == 4)
        {
            dx = 4;
            sx = 3;
        }
        if (x + dx < 256 && y + dy < 256 && x + sx < 256 && y + sy < 256)
            r->pixels[(y + dy) * 256 + x + dx] = r->pixels[(y + sy) * 256 + x + sx];
    }
}
bool SotnMapRenderer_Draw(SotnMapRenderer *r, const SotnWorldState *s, bool inverted, Uint8 opacity,
                          Uint16 *frame, size_t count)
{
    if (r == NULL || frame == NULL || count < SOTN_FRAME_PIXELS || opacity > 64)
        return SDL_SetError("Map drawing requires a full frame and bounded opacity");
    if (!SotnWorld_Validate(s))
        return false;
    if (opacity == 0)
        return true;
    Repaint(r, inverted);
    SDL_memset(r->pixels, 0, sizeof(r->pixels));
    for (Uint32 cy = 0; cy < 64; ++cy)
        for (Uint32 cx = 0; cx < 64; ++cx)
        {
            const Uint32 idx = cx / 4 + cy * 16 + (inverted ? 1024U : 0U), bit = (3 - (cx & 3)) * 2;
            const Uint8 state = (s->map[idx] >> bit) & 3;
            if (state)
                Reveal(r, inverted ? 63 - cx : cx, inverted ? 63 - cy : cy, (state & 1) == 0);
        }
    Passages(r, s, inverted);
    for (Uint32 y = 0; y < 240; ++y)
        for (Uint32 x = 0; x < 256; ++x)
        {
            const Sint32 sy = inverted ? 240 - (Sint32)y : (Sint32)y - 1;
            const Uint32 sx = inverted ? 255 - x : x;
            Uint16 color = 0;
            if (sy >= 0 && sy < 256)
                color = Read16(r->common.bytes + 0x41800 +
                               (size_t)r->pixels[(size_t)sy * 256 + sx] * 2);
            Uint16 out = 0;
            const Uint16 old = frame[y * 256 + x];
            for (Uint32 shift = 0; shift < 15; shift += 5)
                out |= (Uint16)(((((old >> shift) & 31) * (64U - opacity) +
                                  ((color >> shift) & 31) * opacity) /
                                 64)
                                << shift);
            frame[y * 256 + x] = out;
        }
    return true;
}
void SotnMapRenderer_Destroy(SotnMapRenderer *r)
{
    SDL_free(r);
}
