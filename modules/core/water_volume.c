// SPDX-License-Identifier: AGPL-3.0-or-later
// st/water_effects.h:func_801C4144 and EntityAlucardWaterEffect depth selection,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. All tables stay private.
#include <SDL3/SDL_error.h>
#include <sotn/water_volume.h>

static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return bits < 32768U ? (Sint16)bits : (Sint16)((Sint32)bits - 65536);
}
static Sint16 Read(const Uint8 *p)
{
    return Word(p[0] | (Sint32)p[1] << 8);
}
bool SotnWaterVolume_Query(SotnWaterVolumeInput in, SotnWaterVolume *out)
{
    const Uint16 first = in.parameters & 255U, count = in.parameters >> 8;
    const Uint32 limit = (Uint32)first + count;
    if (!out || !in.volumes.bytes || !in.volumes.size || in.volumes.size > 4096 ||
        in.volumes.size % 16 || !in.aspects.bytes || in.aspects.size != 16 || !count ||
        limit > in.volumes.size / 16U)
        return SDL_SetError(
            "Water query requires bounded private volumes, slopes and row selector");
    SotnWaterVolume result = {.body_height = 40, .foot_offset = 25};
    Uint16 waist = 16, head = 48;
    if (in.status & 0x27U)
    {
        if (in.status & 0x20U)
        {
            result.body_height = (in.status & 4U) ? 10 : 20;
            waist = (in.status & 4U) ? 4 : 8;
            head = (in.status & 4U) ? 12 : 24;
        }
        else if (in.status & 3U)
        {
            result.body_height = 12;
            result.foot_offset = 5;
            waist = 6;
            head = 16;
        }
        else
        {
            result.body_height = 20;
            waist = 8;
            head = 24;
        }
    }
    result.x = Word(in.player_x + in.scroll_x);
    result.y = Word(in.player_y + in.scroll_y + result.foot_offset);
    for (Uint32 i = first; i < limit; ++i)
    {
        const Uint8 *row = in.volumes.bytes + i * 16U;
        const Sint16 left = Word(result.x - Read(row));
        if (left < 0)
            continue;
        const Sint16 right = Word(Read(row + 2) - left);
        if (right <= 0)
            continue;
        const Sint16 slope = Read(row + 4);
        if (slope < 0 || slope >= 8)
            return SDL_SetError(
                "Water volume references a slope outside its private eight-word table");
        const Sint16 aspect = Read(in.aspects.bytes + (Uint16)slope * 2U),
                     rise = aspect ? Word(left / aspect) : 0, surface = Word(Read(row + 6) - rise);
        if (surface > result.y || result.y >= Read(row + 8))
            continue;
        const Uint16 raw = (Uint16)(result.y + 32768 - surface);
        if (!raw)
            continue;
        result.contact = true;
        result.raw = raw;
        result.feet = raw & 0x7fffU;
        result.waist = result.feet > waist ? (Uint16)(result.feet - waist) : 0;
        result.head = result.feet > head ? (Uint16)(result.feet - head) : 0;
        result.row = (Uint16)(i * 8U);
        result.left = left;
        result.right = right;
        break;
    }
    *out = result;
    return true;
}
