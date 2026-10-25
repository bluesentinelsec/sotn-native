// SPDX-License-Identifier: AGPL-3.0-or-later
// Normal-Alucard camera path from dra/5087C.c:func_800F0CD8,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5.
#include <sotn/camera.h>
static bool Valid(const SotnCamera *c)
{
    return c != NULL && c->width >= 256 && c->width <= 16384 && c->height >= 256 &&
           c->height <= 16384 && c->x >= 0 && c->x <= 16384 && c->y >= 0 && c->y <= 16384 &&
           c->left >= 0 && c->top >= 0 && c->right <= (Sint32)c->width &&
           c->bottom <= (Sint32)c->height && c->right >= c->left && c->bottom >= c->top &&
           c->right - c->left >= 256 && c->bottom - c->top >= 256 && c->anchor_x >= 1 &&
           c->anchor_x <= 255;
}
bool SotnCamera_Pan(SotnCamera *c, Uint16 target)
{
    if (!Valid(c) || !target || target > 255)
        return SDL_SetError("Camera pan requires an initialized room and screen anchor 1..255");
    if (c->anchor_x < target)
        ++c->anchor_x;
    else if (c->anchor_x > target)
        --c->anchor_x;
    return true;
}
bool SotnCamera_Bounds(SotnCamera *c, Sint32 left, Sint32 top, Sint32 right, Sint32 bottom)
{
    if (c == NULL)
        return SDL_InvalidParamError("camera");
    SotnCamera next = *c;
    next.left = left;
    next.top = top;
    next.right = right;
    next.bottom = bottom;
    if (!Valid(&next))
        return SDL_SetError("Camera constraints exceed the room or original viewport");
    *c = next;
    return true;
}
bool SotnCamera_Tick(SotnCamera *c, Sint32 px, Sint32 py, bool snap)
{
    if (!Valid(c) || px < -256 || py < -256 || px > (Sint32)c->width + 256 ||
        py > (Sint32)c->height + 256)
        return SDL_SetError("Camera tick requires bounded room constraints and player coordinates");
    SotnCamera n = *c;
    n.x = SDL_clamp(px - n.anchor_x, n.left, n.right - 256);
    if (snap)
        n.y = py < n.top + 140 ? n.top + 4 : py > n.bottom - 116 ? n.bottom - 252 : py - 136;
    else if (py < n.top + 140)
    {
        if (n.y - (py - 136) >= 4 && n.top + 8 < n.y)
            n.y -= 4;
        else if (n.y < n.top && n.top != 0)
            n.y += 4;
        else
            n.y = n.top + 4;
    }
    else if (n.bottom - 116 < py)
        n.y = n.bottom - 252;
    else if (n.y - (py - 136) >= 4)
        n.y -= 4;
    else
        n.y = py - 136;
    if (n.x < 0 || n.x > 16384 || n.y < 0 || n.y > 16384)
        return SDL_SetError("Original camera position left the supported room region");
    *c = n;
    return true;
}
bool SotnCamera_Enter(SotnCamera *c, Uint32 w, Uint32 h, Sint32 px, Sint32 py)
{
    if (c == NULL || w % 256 || h % 256 || w > 16384 || h > 16384)
        return SDL_SetError("Camera entry requires whole bounded room cells");
    SotnCamera next = {
        .width = w, .height = h, .right = (Sint32)w, .bottom = (Sint32)h, .anchor_x = 128};
    if (!SotnCamera_Tick(&next, px, py, true))
        return false;
    *c = next;
    return true;
}
