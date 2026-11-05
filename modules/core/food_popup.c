// SPDX-License-Identifier: AGPL-3.0-or-later
// weapon/w_021.c:func_97000_8017AB54, lowerLeft1, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Encoded names/fonts remain private.
#include <sotn/food_popup.h>
static Uint32 Read32(const Uint8 *p)
{
    return p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
}
static bool Byte(SotnDataView dra, size_t *at, Uint8 *out)
{
    if (*at >= dra.size)
        return SDL_SetError("Food name is truncated before its original terminator");
    *out = dra.bytes[(*at)++];
    return true;
}
static SotnActorPrimitive Sprite(Sint16 x, Sint16 y, Uint16 w, Uint16 h, Uint16 page,
                                 Uint16 palette, Uint8 u, Uint8 v, Uint16 priority)
{
    return (SotnActorPrimitive){.x = x * 65536,
                                .y = y * 65536,
                                .width = w,
                                .height = h,
                                .page = page,
                                .palette = palette,
                                .u = {u, (Uint8)(u + w), u, (Uint8)(u + w)},
                                .v = {v, v, (Uint8)(v + h), (Uint8)(v + h)},
                                .active = true,
                                .textured = true,
                                .quad = true,
                                .offset_x = {0, (Sint16)w, 0, (Sint16)w},
                                .offset_y = {0, 0, (Sint16)h, (Sint16)h},
                                .screen_space = true,
                                .priority_set = true,
                                .priority = priority};
}
bool SotnFoodPopup_Build(SotnDataView dra, Uint16 item, SotnFoodPopup *out)
{
    if (!out || !dra.bytes || dra.size < 0xc4 || item < 29 || item > 69)
        return SDL_SetError("Food popup requires its actual private name and collected item29..69");
    const Uint32 table = Read32(dra.bytes + 0xc0);
    const Uint64 record = (Uint64)table + item * 52U;
    if (record < 0x800a0000U || record - 0x800a0000U > dra.size ||
        dra.size - (size_t)(record - 0x800a0000U) < 4)
        return SDL_SetError("Food popup definition points outside private DRA");
    const Uint32 name = Read32(dra.bytes + (size_t)(record - 0x800a0000U));
    if (name < 0x800a0000U || name - 0x800a0000U >= dra.size)
        return SDL_SetError("Food popup name points outside private DRA");
    size_t at = name - 0x800a0000U;
    Uint8 glyphs[64] = {0};
    Uint16 index = 0, characters = 0, pairs = 0;
    for (;;)
    {
        Uint8 ch = 0;
        if (!Byte(dra, &at, &ch))
            return false;
        if (ch == 255)
        {
            if (!Byte(dra, &at, &ch))
                return false;
            if (!ch)
                break;
        }
        if (index >= sizeof(glyphs))
            return SDL_SetError("Food popup exceeds its original64-byte glyph scratch");
        glyphs[index++] = ch;
        ++pairs;
        characters += ch != 0;
        if (at >= dra.size)
            return SDL_SetError("Food name is truncated before its original terminator");
        if (dra.bytes[at] == 255)
        {
            ++at;
            if (!Byte(dra, &at, &ch))
                return false;
            if (!ch)
                break;
            if (ch == 255)
                at -= 2;
            else
            {
                if (index >= sizeof(glyphs))
                    return SDL_SetError("Food popup exceeds its original64-byte glyph scratch");
                glyphs[index] = ch;
                characters += ch != 0;
            }
        }
        ++index;
    }
    SotnFoodPopup p = {.count = (Uint16)(characters + 4)};
    const Uint16 width = (Uint16)((pairs * 8U) | 4U);
    p.rows[0] =
        (SotnActorPrimitive){.x = 7 * 65536,
                             .y = 208 * 65536,
                             .width = (Uint16)(width + 32U),
                             .height = 15,
                             .active = true,
                             .blend = true,
                             .gouraud = true,
                             .quad = true,
                             .offset_x = {0, (Sint16)(width + 32U), 0, (Sint16)(width + 32U)},
                             .offset_y = {0, 0, 15, 15},
                             .vertex_blue = {175, 175, 0, 0},
                             .screen_space = true,
                             .priority_set = true,
                             .priority = 0x1ee};
    p.rows[1] = Sprite(1, 203, 16, 24, 0x1f, 0x197, 128, 0, 0x1ef);
    p.rows[2] = Sprite((Sint16)(width + 29), 203, 16, 24, 0x1f, 0x197, 168, 0, 0x1ef);
    p.rows[3] = Sprite(17, 205, (Uint16)(width + 14U), 20, 0x1f, 0x197, 152, 2, 0x1ef);
    p.rows[3].u[1] = p.rows[3].u[3] = 156;
    Sint16 x = 23;
    Uint16 row = 4;
    for (Uint16 i = 0; i < sizeof(glyphs) && row < p.count; i += 2)
    {
        for (Uint16 half = 0; half < 2; ++half)
        {
            const Uint8 ch = glyphs[i + half];
            if (ch)
                p.rows[row++] = Sprite(x, half ? 204 : 212, 8, 8, 0x1e, 0x196,
                                       (Uint8)((ch & 15U) * 8U), (Uint8)((ch & 240U) >> 1), 0x1f0);
        }
        x += (Uint8)(glyphs[i + 1] + 16U) < 13U ? 12 : 8;
    }
    *out = p;
    return true;
}

bool SotnFoodTextPlan_Replace(SotnFoodTextPlan *plan, SotnDataView dra, Uint16 item,
                              SotnPrimitiveSlots *slots)
{
    if (!plan || !slots || !plan->owner || plan->lease.count > SOTN_FOOD_POPUP_ROWS ||
        (plan->lease.count != 0) != (plan->lease.generation != 0) ||
        (plan->lease.count != 0) != (plan->lease.timer != 0))
        return SDL_SetError("Food text plan requires its actual stage lease and shared snapshot");
    SotnFoodPopup popup = {0};
    if (!SotnFoodPopup_Build(dra, item, &popup))
        return false;
    SotnFoodTextPlan next = *plan;
    SotnPrimitiveSlots rows = *slots;
    if (next.lease.count && !SotnPrimitiveSlots_Release(&rows, next.lease.count, next.lease.rows))
        return false;
    next.lease = (SotnFoodTextLease){0};
    next.popup = (SotnFoodPopup){0};
    next.changed = true;
    Uint16 indices[SOTN_FOOD_POPUP_ROWS] = {0};
    bool allocated = false;
    if (!SotnPrimitiveSlots_Reserve(&rows, popup.count, indices, &allocated))
        return false;
    if (allocated)
    {
        Uint32 generation = ++next.serial;
        if (!generation)
            generation = ++next.serial;
        next.lease.generation = generation;
        next.lease.count = popup.count;
        next.lease.timer = 304;
        SDL_memcpy(next.lease.rows, indices, popup.count * sizeof(*indices));
        next.popup = popup;
    }
    *plan = next;
    *slots = rows;
    return true;
}
