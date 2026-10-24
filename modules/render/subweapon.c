// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected primitive layout from dra/84B88.c and 8A0A4.c, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. All texture bytes remain private.
#include <SDL3/SDL_error.h>
#include <sotn/subweapon_renderer.h>
static Sint32 Pixel(Sint32 p)
{
    return p >= 0 ? p / 65536 : (Sint32)(-((-(Sint64)p + 65535) / 65536));
}
static SotnTextureQuad Rectangle(Sint32 x, Sint32 y, Sint32 w, Sint32 h, Uint16 page,
                                 Uint16 palette, Uint8 u, Uint8 v, Uint8 tw, Uint8 th)
{
    return (SotnTextureQuad){.x = {x, x + w, x, x + w},
                             .y = {y, y, y + h, y + h},
                             .u = {u, (Uint8)(u + tw), u, (Uint8)(u + tw)},
                             .v = {v, v, (Uint8)(v + th), (Uint8)(v + th)},
                             .page = page,
                             .palette = palette};
}
bool SotnSubweapons_Draw(const SotnSubweapons *service, const SotnActorGraphics *graphics,
                         SotnDataView dra, Uint64 ticks, Sint32 camera_x, Sint32 camera_y,
                         Uint16 *frame, size_t count)
{
    if (service == NULL || graphics == NULL || frame == NULL || count < SOTN_FRAME_PIXELS)
        return SDL_SetError("Subweapon presentation requires state, private textures, and a frame");
    for (Uint32 i = 0; i < SOTN_SUBWEAPON_SLOTS; ++i)
    {
        SotnSubweaponState p = {0};
        if (!SotnSubweapons_Get(service, i, &p))
            return false;
        if (p.generation == 0 || !p.visible)
            continue;
        const Sint32 x = Pixel(p.x) - camera_x, y = Pixel(p.y) - camera_y;
        if (x < -128 || x > 384 || y < -128 || y > 368)
            continue;
        SotnTextureQuad q = {0};
        if (p.id == 3)
        {
            if (!SotnActorGraphics_Draw(graphics,
                                        (SotnActorSprite){.animation_set = 9,
                                                          .frame = 0x1d,
                                                          .facing_left = p.facing_left},
                                        x, y, frame, count))
                return false;
            continue;
        }
        if (p.id == 1)
        {
            q = Rectangle(x - 12, y - 4, 24, 8, 0x1c, 0x1ab, 0, 0, 1, 1);
            const Uint8 a = p.facing_left ? 0 : 24, b = p.facing_left ? 24 : 0;
            q.u[0] = q.u[1] = 24;
            q.u[2] = q.u[3] = 32;
            q.v[0] = q.v[2] = a;
            q.v[1] = q.v[3] = b;
            if (p.phase == 2)
            {
                const double angle = p.angle * (SDL_PI_D / 2048.0);
                const Sint32 ax = (Sint32)SDL_round(12 * SDL_cos(angle)),
                             ay = (Sint32)SDL_round(12 * SDL_sin(angle));
                const Sint32 bx = (Sint32)SDL_round(-4 * SDL_sin(angle)),
                             by = (Sint32)SDL_round(4 * SDL_cos(angle));
                q.x[0] = x - ax - bx;
                q.y[0] = y - ay - by;
                q.x[1] = x + ax - bx;
                q.y[1] = y + ay - by;
                q.x[2] = x - ax + bx;
                q.y[2] = y - ay + by;
                q.x[3] = x + ax + bx;
                q.y[3] = y + ay + by;
            }
        }
        else if (p.id == 2)
        {
            q = Rectangle(x, y, 24, 40, 0x1c, (Uint16)(0x1ab + ((ticks >> 1) & 1)), 0, 0, 24, 40);
            const Sint32 angles[4] = {p.facing_left ? 0x560 : 0x2a0, p.facing_left ? 0x2a0 : 0x560,
                                      p.facing_left ? 0xaa0 : 0xd60, p.facing_left ? 0xd60 : 0xaa0};
            for (Uint32 n = 0; n < 3; ++n)
            {
                const double angle = (angles[n] + p.angle) * (SDL_PI_D / 2048.0);
                q.x[n] = x + (Sint32)SDL_floor(SDL_cos(angle) * 21);
                q.y[n] = y - (Sint32)SDL_floor(SDL_sin(angle) * 21);
            }
            q.x[3] = q.x[1] + q.x[2] - q.x[0];
            q.y[3] = q.y[1] + q.y[2] - q.y[0];
        }
        else if (p.id == 6)
        {
            q = Rectangle(x - 8, y + p.visual_y - 12, 16, 24, 0x1e, 0x17f, 0x98, 0xc0, 16, 24);
            if (!p.facing_left)
            {
                q.u[0] = q.u[2] = 0xa8;
                q.u[1] = q.u[3] = 0x98;
            }
            if (p.phase >= 3 && p.phase < 5 && p.timer < 5)
            {
                const Uint8 u = (Uint8)(0x18 + (p.timer == 0 ? 80 : p.timer * 8));
                SotnTextureQuad digit =
                    Rectangle(x - 3, y + p.visual_y - 25, 6, 9, 0x1a, 0x186, u, 0x40, 6, 9);
                if (!SotnActorGraphics_Quad(graphics, digit, frame, count))
                    return false;
            }
        }
        else if (p.id == 11)
        {
            if (dra.bytes == NULL || dra.size < 0x10698)
                return SDL_SetError("Holy water flame UV table is truncated");
            if (p.visual_y < 7)
                continue;
            const Uint8 u = dra.bytes[0x10688 + (ticks & 7) * 2],
                        v = dra.bytes[0x10689 + (ticks & 7) * 2];
            // Selected four original strip UVs; horizontal wave/trail coloration remains
            // incomplete.
            for (Sint32 strip = 0; strip < 4; ++strip)
            {
                const Sint32 top = y - p.visual_y + (p.visual_y * strip) / 4;
                const Sint32 bottom = y - p.visual_y + (p.visual_y * (strip + 1)) / 4;
                q = Rectangle(x - 8, top, 16, bottom - top, 0x1a, 0x1b2, 0, 0, 1, 1);
                q.u[0] = q.u[1] = (Uint8)(u + 128 - strip * 7);
                q.u[2] = q.u[3] = (Uint8)(u + 128 - (strip + 1) * 7);
                q.v[0] = q.v[2] = (Uint8)(v + 128);
                q.v[1] = q.v[3] = (Uint8)(v + 144);
                q.blend = true;
                if (!SotnActorGraphics_Quad(graphics, q, frame, count))
                    return false;
            }
            continue;
        }
        else
            continue;
        if (!SotnActorGraphics_Quad(graphics, q, frame, count))
            return false;
    }
    return true;
}
