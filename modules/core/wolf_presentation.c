// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/4B758.c:RenderEntities bank15 multipart paths and SDK rotation,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Definitions remain private.
#include <SDL3/SDL.h>
#include <sotn/wolf_presentation.h>
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static Uint32 Read32(const Uint8 *p)
{
    return p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
}
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint32 Signed16(const Uint8 *p)
{
    return Word(Read16(p));
}
static Sint16 Whole(Sint32 value)
{
    return (Sint16)(value >= 0 ? value / 65536 : -((-(Sint64)value + 65535) / 65536));
}
static bool View(SotnDataView dra, Uint32 address, size_t size, Uint32 alignment, SotnDataView *out)
{
    const size_t offset = address >= 0x800a0000U ? address - 0x800a0000U : dra.size;
    if (!dra.bytes || address < 0x800a0000U || address % alignment || offset > dra.size ||
        size > dra.size - offset)
        return SDL_SetError("Wolf multipart definition points outside its private DRA image");
    *out = (SotnDataView){dra.bytes + offset, size};
    return true;
}
static bool Frame(SotnDataView dra, Uint16 frame, SotnDataView *out)
{
    SotnDataView bank = {0}, table = {0}, first = {0};
    if (!dra.bytes || dra.size < 0x3bb0 || !View(dra, Read32(dra.bytes + 0x3bac), 8, 4, &bank))
        return SDL_SetError("Wolf presentation requires its private bank-fifteen table");
    const Uint32 address = Read32(dra.bytes + 0x3bac), first_address = Read32(bank.bytes + 4);
    if (first_address <= address || (first_address - address) % 4U ||
        (first_address - address) / 4U > 1024 || frame >= (first_address - address) / 4U ||
        !View(dra, address, first_address - address, 4, &table) ||
        !View(dra, Read32(table.bytes + (size_t)frame * 4U), 2, 2, &first))
        return SDL_SetError("Wolf frame exceeds its bounded private pointer table");
    const Uint16 pieces = Read16(first.bytes);
    if (pieces > 2)
        return SDL_SetError("Wolf frame requires its selected original one/two-piece artwork");
    return View(dra, Read32(table.bytes + (size_t)frame * 4U), 2U + pieces * 22U, 2, out);
}
static bool Packet(SotnDataView executable, SotnWolfDrawInput input, const Uint8 *part,
                   SotnPlayerEffectQuad *out)
{
    const Uint16 flags = Read16(part);
    Sint32 pivot_x = Signed16(part + 2), pivot_y = Signed16(part + 4), width = Signed16(part + 6),
           height = Signed16(part + 8);
    if ((flags & ~0x13fU) || width <= 0 || width > 512 || height <= 0 || height > 512)
        return SDL_SetError("Wolf multipart piece has unsupported flags or dimensions");
    if (flags & 4U)
    {
        --width;
        if (flags & 2U)
            ++pivot_x;
    }
    if (flags & 8U)
    {
        --height;
        if (flags & 1U)
            ++pivot_y;
    }
    if (flags & 16U)
    {
        --width;
        if (!(flags & 2U))
            ++pivot_x;
    }
    if (flags & 32U)
    {
        --height;
        if (!(flags & 1U))
            ++pivot_y;
    }
    if (width <= 0 || height <= 0)
        return SDL_SetError("Wolf cropping removes the complete multipart piece");
    const SotnWolfPart p = input.part;
    const Sint32 palette = p.palette & 0x8000U ? (Sint32)(p.palette & 0x7fffU)
                                               : (Sint32)p.palette + Signed16(part + 10);
    const Uint16 texture = (Uint16)(Read16(part + 12) + (Uint32)p.texture_offset);
    const Uint32 page = (texture >> 2) + (p.blend & 0x60U);
    const Uint32 priority = p.priority + ((flags & 0x100U) ? 4U : 0U);
    if (palette < 0 || palette >= 768 || page > 0x1ff || priority > 511)
        return SDL_SetError("Wolf piece exceeds original palette/page/priority bounds");
    Sint32 u0 = Read16(part + 14) + ((texture & 1U) ? 128 : 0),
           u1 = Read16(part + 18) + ((texture & 1U) ? 128 : 0),
           v0 = Read16(part + 16) + ((texture & 2U) ? 128 : 0),
           v1 = Read16(part + 20) + ((texture & 2U) ? 128 : 0);
    if (flags & 4U)
        --u1;
    if (flags & 8U)
        --v1;
    if (flags & 16U)
        ++u0;
    if (flags & 32U)
        ++v0;
    if (((flags & 2U) != 0) != p.facing_left)
    {
        Sint32 old = u0;
        u0 = u1 - 1;
        u1 = old - 1;
    }
    if (flags & 1U)
    {
        Sint32 old = v0;
        v0 = v1 - 1;
        v1 = old - 1;
    }
    const Sint16 x = Whole(p.x), y = Whole(p.y);
    const Sint32 left = p.facing_left ? x - pivot_x - width + 1 : x + pivot_x, top = y + pivot_y;
    SotnPlayerEffectQuad q = {.page = (Uint16)page,
                              .palette = (Uint16)palette,
                              .priority = (Uint16)priority,
                              .draw_mode =
                                  (Uint16)((p.blend ? 1U : 0U) | ((p.draw_flags & 8U) ? 4U : 0U))};
    for (Uint16 i = 0; i < 4; ++i)
    {
        q.x[i] = Word(left + ((i & 1U) ? width : 0));
        q.y[i] = Word(top + ((i & 2U) ? height : 0));
        q.u[i] = (Uint8)(Uint32)((i & 1U) ? u1 : u0);
        q.v[i] = (Uint8)(Uint32)((i & 2U) ? v1 : v0);
        // Raw packet colors are semantically ignored; initialize them instead of copying scratch
        // bytes.
        q.red[i] = q.green[i] = q.blue[i] = (p.draw_flags & 8U) ? p.opacity : 128;
        if (p.draw_flags & 8U)
        {
            if (p.draw_flags & 16U)
                q.red[i] = 128;
            if (p.draw_flags & 32U)
                q.green[i] = 128;
            if (p.draw_flags & 64U)
                q.blue[i] = 128;
        }
    }
    if ((p.draw_flags & 4U) &&
        !SotnPlayerEffect_Rotate(executable,
                                 Word(x + (p.facing_left ? -(Sint32)p.pivot_x : p.pivot_x)),
                                 Word(y + p.pivot_y), p.rotation, p.facing_left, &q))
        return false;
    *out = q;
    return true;
}
bool SotnWolfPresentation_Packets(SotnDataView dra, SotnDataView executable,
                                  SotnWolfDrawInput input, SotnWolfDrawPacket *out, size_t capacity,
                                  size_t *count)
{
    if (!count || input.slot > 255 || input.available > 768 || input.player_priority > 511)
        return SDL_SetError("Wolf presentation requires actual bounded actor/GPU context");
    const Sint16 x = Whole(input.part.x), y = Whole(input.part.y);
    const Uint16 frame = input.part.sprite & 0x7fffU;
    if (!frame || !input.part.bank || x < -512 || x > 512 || y < -512 || y > 512 ||
        ((input.part.draw_flags & 128U) && ((input.slot ^ input.game_timer) & 1U)))
    {
        *count = 0;
        return true;
    }
    if (input.part.bank != 15 || (input.part.draw_flags & 3U) || (input.part.blend & ~0x70U))
        return SDL_SetError("Wolf presentation requires original bank15 rotation/color attributes");
    SotnDataView definition = {0};
    if (!Frame(dra, frame, &definition))
        return false;
    const Uint16 pieces = Read16(definition.bytes);
    const size_t required = SDL_min((size_t)pieces * 2U, input.available);
    if (capacity < required || (required && !out))
        return SDL_SetError(
            "Wolf packet output cannot hold the actual available normal/capture rows");
    SotnWolfDrawPacket packets[SOTN_WOLF_DRAW_MAX_PACKETS] = {0};
    size_t emitted = 0;
    for (Uint16 i = 0; i < pieces && emitted < required; ++i)
    {
        SotnPlayerEffectQuad q = {0};
        if (!Packet(executable, input, definition.bytes + 2U + (size_t)i * 22U, &q))
            return false;
        packets[emitted++] = (SotnWolfDrawPacket){.quad = q};
        if (emitted == required)
            break;
        const Sint32 priority = (Sint32)q.priority + 0x1a0 - input.player_priority;
        if (priority < 0 || priority > 511)
            return SDL_SetError(
                "Wolf captured piece escapes the original environment priority interval");
        for (Uint16 v = 0; v < 4; ++v)
        {
            q.x[v] = Word(q.x[v] + (input.part.facing_left ? 20 : 44) - input.player_x);
            q.y[v] = Word(q.y[v] + 24 - input.player_y);
        }
        q.palette = 0x104;
        q.priority = (Uint16)priority;
        packets[emitted++] = (SotnWolfDrawPacket){.quad = q, .capture = true};
    }
    if (emitted)
        SDL_memcpy(out, packets, emitted * sizeof(*out));
    *count = emitted;
    return true;
}
