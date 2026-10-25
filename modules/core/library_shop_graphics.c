// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/lib/e_shop.c panel/font geometry, sotn-decomp 7bc54d8157b6.
// Direct glyph quads replace temporary PS1 render textures; all glyphs/text stay private.
#include "actors_internal.h"
typedef struct Canvas
{
    SotnActors *s;
    Actor *a;
    Uint16 used, priority;
} Canvas;
static bool Primitive(Canvas *c, SotnActorPrimitive **out)
{
    ActorPrimitive *p = ActorPrimitiveGet(c->s, c->a, c->used++);
    if (!p)
        return false;
    p->state = (SotnActorPrimitive){
        .active = true, .screen_space = true, .priority_set = true, .priority = c->priority};
    *out = &p->state;
    return true;
}
static bool Quad(Canvas *c, const Sint16 x[4], const Sint16 y[4], Uint8 r, Uint8 g, Uint8 b,
                 bool blend)
{
    SotnActorPrimitive *p = NULL;
    if (!Primitive(c, &p))
        return false;
    p->quad = true;
    p->red = r;
    p->green = g;
    p->blue = b;
    p->blend = blend;
    for (Uint16 i = 0; i < 4; ++i)
    {
        p->offset_x[i] = x[i];
        p->offset_y[i] = y[i];
    }
    return true;
}
static bool Line(Canvas *c, Sint16 x0, Sint16 y0, Sint16 x1, Sint16 y1)
{
    SotnActorPrimitive *p = NULL;
    if (!Primitive(c, &p))
        return false;
    p->line = true;
    p->offset_x[0] = x0;
    p->offset_x[1] = x1;
    p->offset_y[0] = y0;
    p->offset_y[1] = y1;
    p->red = p->green = p->blue = p->line_red = p->line_green = p->line_blue = 128;
    return true;
}
static bool Border(Canvas *c, const Sint16 x[4], const Sint16 y[4])
{
    return Line(c, x[0], y[0], x[1], y[1]) && Line(c, x[1], y[1], x[3], y[3]) &&
           Line(c, x[3], y[3], x[2], y[2]) && Line(c, x[2], y[2], x[0], y[0]);
}
static bool Rect(Canvas *c, Sint16 x, Sint16 y, Sint16 w, Sint16 h, Uint8 r, Uint8 g, Uint8 b)
{
    const Sint16 xs[4] = {x, (Sint16)(x + w), x, (Sint16)(x + w)};
    const Sint16 ys[4] = {y, y, (Sint16)(y + h), (Sint16)(y + h)};
    return Quad(c, xs, ys, r, g, b, false);
}
static bool Glyph(Canvas *c, Uint8 ch, Sint16 x, Sint16 y, Uint16 palette)
{
    if (!ch)
        return true;
    SotnActorPrimitive *p = NULL;
    if (!Primitive(c, &p))
        return false;
    p->textured = true;
    p->x = (x + 4) * 65536;
    p->y = (y + 4) * 65536;
    p->width = p->height = 8;
    p->page = 0x1e;
    p->palette = palette;
    const Uint8 u = (Uint8)((ch & 15U) * 8), v = (Uint8)((ch & 240U) >> 1);
    p->u[0] = p->u[2] = u;
    p->u[1] = p->u[3] = (Uint8)(u + 8U);
    p->v[0] = p->v[1] = v;
    p->v[2] = p->v[3] = (Uint8)(v + 8U);
    return true;
}
static bool Text(Canvas *c, SotnDataView text, Sint16 x, Sint16 y, Uint16 palette, bool ascii)
{
    if (!text.bytes || text.size > 160)
        return SDL_SetError("Shop text exceeds its bounded private string");
    for (size_t i = 0; i < text.size; ++i)
    {
        const Uint8 raw = text.bytes[i];
        if (ascii && (raw < 32 || raw > 126))
            return SDL_SetError("Shop label has an unsupported US glyph");
        const Uint8 ch = ascii ? (Uint8)(raw - 32U) : raw;
        const bool slim = !ascii && (ch == 'i' - 32 || ch == 'l' - 32);
        if (!ascii && (slim || ch == 'f' - 32 || ch == 'I' - 32))
            --x;
        else if (!ascii && ch == '\'' - 32)
            x = (Sint16)(x - 2);
        if (!Glyph(c, ch, x, y, palette))
            return false;
        x = (Sint16)(x + (!ch ? 4 : slim ? 7 : (!ascii && ch == '\'' - 32) ? 6 : 8));
    }
    return true;
}
static bool String(SotnDataView lib, Uint32 address, SotnDataView *out, bool encoded)
{
    if (address < 0x80180000 || address - 0x80180000U >= lib.size)
        return SDL_SetError("Shop label references an invalid private overlay address");
    const size_t offset = address - 0x80180000U, limit = SDL_min((size_t)160, lib.size - offset);
    for (size_t i = 0; i < limit; ++i)
        if (lib.bytes[offset + i] == (encoded ? 255 : 0))
        {
            *out = (SotnDataView){lib.bytes + offset, i};
            return true;
        }
    return SDL_SetError("Shop private label has no bounded terminator");
}
static bool Number(Canvas *c, Uint32 value, Sint16 right, Sint16 y, Uint16 digits, Uint16 palette)
{
    char text[16] = {0};
    SDL_snprintf(text, sizeof(text), "%u", value);
    const size_t count = SDL_strlen(text);
    if (count > digits)
        return SDL_SetError("Shop number exceeds its original digit field");
    return Text(c, (SotnDataView){(const Uint8 *)text, count}, (Sint16)(right - (Sint32)count * 8),
                y, palette, true);
}
static bool Root(Canvas *c)
{
    SotnActors *s = c->s;
    const Actor *a = c->a;
    if (a->state.step < 3)
        return true;
    const Sint32 half = s->shop.count * 8 + 4;
    Sint16 xs[4] = {150, 234, 150, 234};
    Sint16 ys[4] = {(Sint16)(96 - half), (Sint16)(96 - half), (Sint16)(96 + half),
                    (Sint16)(96 + half)};
    if (a->state.step == 3 || a->state.step == 8)
    {
        const Sint32 t = a->librarian.timer;
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(s->executable, a->pose, &trig))
            return false;
        const Sint32 wide = SDL_min(42, t * 42 / 32), high = SDL_min(half, t * half / 32);
        const Sint32 wx = trig.cosine * (t * 42 / 64) / 4096;
        const Sint32 hy = trig.cosine * (t * half / 64) / 4096;
        if (a->state.step == 3)
        {
            xs[0] = (Sint16)(192 - wx);
            xs[1] = (Sint16)(192 + wide);
            xs[2] = (Sint16)(192 - wide);
            xs[3] = (Sint16)(192 + wx);
            ys[0] = (Sint16)(96 - hy);
            ys[1] = (Sint16)(96 - high);
            ys[2] = (Sint16)(96 + high);
            ys[3] = (Sint16)(96 + hy);
        }
        else
        {
            xs[0] = (Sint16)(192 - wide);
            xs[1] = (Sint16)(192 + wx);
            xs[2] = (Sint16)(192 - wx);
            xs[3] = (Sint16)(192 + wide);
            ys[0] = (Sint16)(96 - high);
            ys[1] = (Sint16)(96 - hy);
            ys[2] = (Sint16)(96 + hy);
            ys[3] = (Sint16)(96 + high);
        }
    }
    else if (a->state.step >= 9)
    {
        const Sint32 t = a->state.step == 11 ? 24 : a->librarian.timer;
        const Sint32 selected = s->shop.cursor * 16;
        const Sint32 dy = t * (half + 24 - selected) / 24, dx = -t * 152 / 24;
        const Sint32 shrink = s->shop.cursor < 3 ? t / 3 : 0;
        xs[0] = xs[2] = (Sint16)(150 + dx + shrink);
        xs[1] = xs[3] = (Sint16)(234 + dx - shrink);
        ys[0] = ys[1] = (Sint16)(96 - half + t * (selected + 2) / 24 + dy);
        ys[2] = ys[3] = (Sint16)(96 + half - t * (half * 2 - (selected + 21)) / 24 + dy);
    }
    c->priority = 0xbf;
    if (!Quad(c, xs, ys, 0, 0, 64, true))
        return false;
    c->priority = 0xc1;
    if (!Border(c, xs, ys))
        return false;
    // Text follows the panel's changing bounds; source uses an offscreen texture for this.
    const Uint16 begin = c->used;
    for (Uint16 row = 0; row < s->shop.count; ++row)
    {
        SotnDataView pointer = {0}, label = {0};
        if (!Slice(s->nz0, 0x1310U + s->shop.options[row] * 4U, 4, &pointer) ||
            !String(s->nz0, Read32(pointer.bytes), &label, false) || label.size < 1)
            return false;
        const Sint16 x = (Sint16)(150 + label.bytes[0] * 4U);
        label.bytes++;
        label.size--;
        if (!Text(c, label, x, (Sint16)(104 - half + row * 16), 0x1a1, true))
            return false;
    }
    // Bilinear mapping of the original texture rectangle onto each opening-panel glyph.
    for (Uint16 i = begin; i < c->used; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        SotnActorPrimitive *g = &p->state;
        const Sint32 source_x = Pixel(g->x) - 4, source_y = Pixel(g->y) - 4;
        if (a->state.step >= 9)
        {
            const Sint32 t = a->state.step == 11 ? 24 : a->librarian.timer;
            const Sint32 selected = s->shop.cursor * 16, shrink = s->shop.cursor < 3 ? t / 3 : 0;
            const Sint32 dx = -t * 152 / 24, dy = t * (half + 24 - selected) / 24;
            const Sint32 left = SDL_max(source_x, 150 + shrink),
                         right = SDL_min(source_x + 8, 234 - shrink);
            const Sint32 top = SDL_max(source_y, 96 - half + t * (selected + 2) / 24);
            const Sint32 bottom =
                SDL_min(source_y + 8, 96 + half - t * (half * 2 - (selected + 21)) / 24);
            if (left >= right || top >= bottom)
            {
                g->active = false;
                continue;
            }
            g->quad = true;
            g->x = g->y = 0;
            g->offset_x[0] = g->offset_x[2] = (Sint16)(left + dx);
            g->offset_x[1] = g->offset_x[3] = (Sint16)(right + dx);
            g->offset_y[0] = g->offset_y[1] = (Sint16)(top + dy);
            g->offset_y[2] = g->offset_y[3] = (Sint16)(bottom + dy);
            g->u[0] = g->u[2] = (Uint8)(g->u[0] + left - source_x);
            g->u[1] = g->u[3] = (Uint8)(g->u[1] - (source_x + 8 - right));
            g->v[0] = g->v[1] = (Uint8)(g->v[0] + top - source_y);
            g->v[2] = g->v[3] = (Uint8)(g->v[2] - (source_y + 8 - bottom));
            continue;
        }
        const Sint32 left = source_x - 150, top = source_y - (96 - half);
        g->x = g->y = 0;
        g->quad = true;
        for (Uint16 v = 0; v < 4; ++v)
        {
            const Sint32 u = left + ((v & 1U) ? 8 : 0), vv = top + (v >= 2 ? 8 : 0);
            g->offset_x[v] =
                (Sint16)((xs[0] * (84 - u) * (half * 2 - vv) + xs[1] * u * (half * 2 - vv) +
                          xs[2] * (84 - u) * vv + xs[3] * u * vv) /
                         (84 * half * 2));
            g->offset_y[v] =
                (Sint16)((ys[0] * (84 - u) * (half * 2 - vv) + ys[1] * u * (half * 2 - vv) +
                          ys[2] * (84 - u) * vv + ys[3] * u * vv) /
                         (84 * half * 2));
        }
    }
    if (a->state.step == 4)
    {
        c->priority = 0xc0;
        return Rect(c, 152, (Sint16)(100 - half + s->shop.cursor * 16), 80, 16, 64, 16, 16);
    }
    return true;
}
static bool Name(Canvas *c, SotnShopOffer offer, Sint16 x, Sint16 y, Uint16 palette)
{
    SotnItem item = {0};
    if (offer.category < 5)
    {
        if (!SotnCharacter_Item(c->s->character, offer.category != 0, offer.item, &item))
            return false;
        return Text(c, item.name, x, y, palette, false);
    }
    SotnDataView pointer = {0}, name = {0};
    const size_t at = offer.category == 5 ? 0x16ac : 0x16b0U + offer.item * 4U;
    return Slice(c->s->nz0, at, 4, &pointer) &&
           String(c->s->nz0, Read32(pointer.bytes), &name, true) &&
           Text(c, name, x, y, palette, false);
}
static bool List(Canvas *c)
{
    SotnActors *s = c->s;
    const Actor *a = c->a;
    if (a->state.step < 2 || a->remove)
        return true;
    const Sint32 t = a->librarian.timer;
    SotnMathTrig trig = {0};
    if (!SotnMath_Trig(s->executable, a->pose, &trig))
        return false;
    const Sint16 width = (Sint16)(trig.sine * 116 / 4096),
                 shear = (Sint16)(trig.cosine * 32 / 4096);
    const Sint16 left[4] = {(Sint16)(128 - width), 128, (Sint16)(128 - width), 128};
    const Sint16 right[4] = {128, (Sint16)(128 + width), 128, (Sint16)(128 + width)};
    const Sint16 yl[4] = {(Sint16)(24 - shear), 24, (Sint16)(120 + shear), 120};
    const Sint16 yr[4] = {24, (Sint16)(24 - shear), 120, (Sint16)(120 + shear)};
    c->priority = 0x1fa;
    for (Uint16 side = 0; side < 2; ++side)
    {
        if (!Quad(c, side ? right : left, side ? yr : yl, 0, 0, 0, true))
            return false;
        ActorPrimitive *p = ActorPrimitiveGet(s, a, (Uint16)(c->used - 1U));
        if (!p)
            return false;
        p->state.gouraud = true;
        p->state.vertex_green[2] = p->state.vertex_green[3] = (Uint8)(side ? t + 48 : t * 2 + 32);
    }
    c->priority = 0x1fc;
    if (!Border(c, left, yl) || !Border(c, right, yr))
        return false;
    if (a->state.step == 2 || a->state.step == 8)
        return true;
    c->priority = 0x1fb;
    if (!Rect(c, 17, (Sint16)(31 + s->shop.item_cursor * 12), 222, 12, 64, 16, 16))
        return false;
    c->priority = 0x1fc;
    for (Uint16 row = 0; row < 7 && row + s->shop.scroll < s->shop.catalogue.count; ++row)
    {
        const Uint16 index = (Uint16)(row + s->shop.scroll), quantity = s->shop.quantities[index];
        const SotnShopOffer offer = s->shop.catalogue.offers[index];
        const Sint16 y = (Sint16)(33 + row * 12);
        const Uint16 palette = !s->shop.sell || offer.available ? 0x196 : 0x191;
        if (!Name(c, offer, 17, y, palette))
            return false;
        if (!s->shop.sell && !quantity)
        {
            SotnDataView sold = {0};
            if (!Slice(s->nz0, 0x16a4, 8, &sold) || !Text(c, sold, 159, y, 0x196, false))
                return false;
        }
        else if (!Glyph(c, 'x' - 32, 135, y, palette) || !Number(c, quantity, 159, y, 2, palette) ||
                 !Number(c, quantity * offer.price, 233, y, 8, palette))
            return false;
    }
    SotnCharacterState state = {0};
    if (!SotnCharacter_Get(s->character, &state, NULL))
        return false;
    // Original quantity/list and owned gold remain visible; complete detail panels follow.
    return Number(c, state.gold, 232, 136, 6, 0x196);
}
bool ActorLibraryShopGraphics(SotnActors *s, Actor *a)
{
    if (!s || !a || !a->state.primitive_count)
        return SDL_SetError("Shop presentation needs its owned primitive reservation");
    for (Uint16 i = 0; i < a->state.primitive_count; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        p->state.active = false;
    }
    Canvas c = {.s = s, .a = a};
    return a->state.id == 0x26 ? Root(&c) : List(&c);
}
