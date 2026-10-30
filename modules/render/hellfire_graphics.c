// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/84B88.c:EntityHellfire LoadImage boundary and flash G4 packet,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Pixels remain private.
#include <sotn/hellfire_graphics.h>
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
bool SotnHellfireGraphics_Upload(SotnActorGraphics *graphics, SotnDataView dra)
{
    if (!graphics || !dra.bytes || dra.size < 0x10798)
        return SDL_SetError(
            "Hellfire gradients require their actual texture owner and private image");
    SotnActorTarget rows[2] = {0};
    for (Uint16 i = 0; i < 2; ++i)
    {
        const Uint8 *r = dra.bytes + 0x10788U + i * 8U;
        rows[i] = (SotnActorTarget){.x = Read16(r),
                                    .y = Read16(r + 2),
                                    .width = Read16(r + 4),
                                    .height = Read16(r + 6),
                                    .clear = true};
        if (!rows[i].width || rows[i].width > 48 || rows[i].height != 1 ||
            rows[i].x > 1024U - rows[i].width || rows[i].y >= 512)
            return SDL_SetError(
                "Hellfire gradient row escapes its private pixels or original VRAM");
    }
    for (Uint16 i = 0; i < 2; ++i)
    {
        SotnActorSurface target = {0};
        if (!SotnActorGraphics_BeginTarget(graphics, rows[i], &target))
            return false;
        const Uint8 *pixels = dra.bytes + 0x106c8U + i * 96U;
        for (Uint16 x = 0; x < rows[i].width; ++x)
            target.pixels[x] = Read16(pixels + x * 2U);
        if (!SotnActorGraphics_EndTarget(graphics, true))
            return false;
    }
    return true;
}
bool SotnHellfireGraphics_Flash(SotnActorGraphics *g, const SotnPlayerEffectQuad *p, Sint32 x,
                                Sint32 y, SotnActorSurface surface)
{
    if (!g || !p || x < -2048 || x > 2048 || y < -2048 || y > 2048 || !surface.pixels ||
        !surface.width || !surface.height || surface.width > 1024 || surface.height > 512 ||
        surface.count < (size_t)surface.width * surface.height)
        return SDL_SetError("Hellfire flash requires its actual packet and bounded projection");
    if (p->draw_mode & 8U)
        return true;
    SotnTextureQuad q = {.untextured = true,
                         .gouraud = true,
                         .page = (Uint16)(p->draw_mode & 0x60U),
                         .blend = (p->draw_mode & 1U) != 0};
    for (Uint16 v = 0; v < 4; ++v)
    {
        q.x[v] = p->x[v] + x;
        q.y[v] = p->y[v] + y;
        q.vertex_red[v] = p->red[v];
        q.vertex_green[v] = p->green[v];
        q.vertex_blue[v] = p->blue[v];
    }
    return SotnActorGraphics_QuadSurface(g, q, surface);
}
