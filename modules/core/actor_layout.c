#include <SDL3/SDL_error.h>
#include <sotn/actor_layout.h>

#define NZ0_BASE 0x80180000U
#define LAYOUT_COUNT 53U
#define MAX_PLACEMENTS 512U

struct SotnActorLayout
{
    SotnDataView lists[2];
    Uint32 count;
};

static Uint16 Read16(const Uint8 *bytes)
{
    return (Uint16)(bytes[0] | (Uint16)bytes[1] << 8);
}

static Uint32 Read32(const Uint8 *bytes)
{
    return bytes[0] | (Uint32)bytes[1] << 8 | (Uint32)bytes[2] << 16 | (Uint32)bytes[3] << 24;
}

static bool Resolve(SotnDataView image, Uint32 address, size_t size, Uint32 alignment,
                    SotnDataView *out)
{
    if (address < NZ0_BASE || address % alignment != 0 || address - NZ0_BASE > image.size ||
        size > image.size - (address - NZ0_BASE))
        return SDL_SetError("Laboratory actor layout has an invalid or truncated PS1 pointer");
    *out = (SotnDataView){image.bytes + (address - NZ0_BASE), size};
    return true;
}

static bool ReadList(SotnDataView image, Uint32 address, unsigned int axis, SotnDataView *out,
                     Uint32 *count, Uint16 id_limit)
{
    SotnDataView first = {0};
    if (!Resolve(image, address, 10, 2, &first))
        return false;
    if (Read16(first.bytes) != 0xfffe || Read16(first.bytes + 2) != 0xfffe)
        return SDL_SetError("Laboratory actor layout requires a static start sentinel");
    Uint16 previous = 0;
    for (Uint32 i = 0; i <= MAX_PLACEMENTS; ++i)
    {
        const size_t offset = (size_t)(first.bytes - image.bytes) + (size_t)(i + 1) * 10;
        if (offset > image.size || image.size - offset < 10)
            return SDL_SetError("Laboratory actor layout ended without its terminator");
        const Uint8 *record = image.bytes + offset;
        if (Read16(record) == 0xffff && Read16(record + 2) == 0xffff)
        {
            *out = (SotnDataView){first.bytes + 10, (size_t)i * 10};
            *count = i;
            return true;
        }
        const Uint16 coordinate = Read16(record + axis * 2);
        const Uint16 raw_id = Read16(record + 4), id = raw_id & 0x3ff;
        const Uint16 mode = raw_id & 0xe000;
        if (i == MAX_PLACEMENTS || Read16(record) > 32767 || Read16(record + 2) > 32767 ||
            coordinate < previous || id == 0 || id >= id_limit ||
            (Read16(record + 6) & 255) >= SOTN_STAGE_ACTOR_SLOTS ||
            (mode != 0 && mode != 0x8000 && mode != 0xa000))
            return SDL_SetError(
                "Laboratory actor placement has invalid order, coordinates, ID, or slot");
        previous = coordinate;
    }
    return SDL_SetError("Laboratory actor layout exceeds its bounded placement budget");
}

bool SotnActorLayout_Open(SotnDataView image, Uint32 index, SotnActorLayout **out)
{
    return SotnActorLayout_OpenStage(image, 12, index, out);
}

bool SotnActorLayout_OpenStage(SotnDataView image, Uint16 stage_id, Uint32 index,
                               SotnActorLayout **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (stage_id != 12 && stage_id != 65 && stage_id != 7)
        return SDL_SetError("Actor layouts do not support stage %u", stage_id);
    const Uint16 id_limit = stage_id == 65 ? 0x63 : stage_id == 7 ? 0x5c : 0x4e;
    if (image.bytes == NULL || image.size < 32 || image.size > 0x80000 || index >= LAYOUT_COUNT)
        return SDL_SetError(
            "Actor layouts require a bounded supported US stage image and layout ID below 53");
    SotnDataView tables = {0};
    if (!Resolve(image, Read32(image.bytes + 28), LAYOUT_COUNT * 8, 4, &tables))
        return false;
    SotnActorLayout candidate = {0};
    Uint32 vertical_count = 0;
    if (!ReadList(image, Read32(tables.bytes + (size_t)index * 4), 0, &candidate.lists[0],
                  &candidate.count, id_limit) ||
        !ReadList(image, Read32(tables.bytes + (LAYOUT_COUNT + (size_t)index) * 4), 1,
                  &candidate.lists[1], &vertical_count, id_limit))
        return false;
    if (vertical_count != candidate.count)
        return SDL_SetError("Laboratory actor layout lists have different placement counts");
    for (Uint32 i = 0; i < candidate.count; ++i)
    {
        Uint32 horizontal = 0, vertical = 0;
        const Uint8 *record = candidate.lists[0].bytes + (size_t)i * 10;
        for (Uint32 j = 0; j < candidate.count; ++j)
        {
            horizontal += SDL_memcmp(record, candidate.lists[0].bytes + (size_t)j * 10, 10) == 0;
            vertical += SDL_memcmp(record, candidate.lists[1].bytes + (size_t)j * 10, 10) == 0;
        }
        if (horizontal != vertical)
            return SDL_SetError("Laboratory actor layout lists describe different placements");
    }
    SotnActorLayout *layout = SDL_malloc(sizeof(*layout));
    if (layout == NULL)
        return false;
    *layout = candidate;
    *out = layout;
    return true;
}

Uint32 SotnActorLayout_Count(const SotnActorLayout *layout)
{
    return layout != NULL ? layout->count : 0;
}

bool SotnActorLayout_Get(const SotnActorLayout *layout, bool vertical, Uint32 index,
                         SotnActorPlacement *out)
{
    if (layout == NULL || out == NULL || index >= layout->count)
        return SDL_SetError("Actor placement index is outside the validated layout");
    const Uint8 *record = layout->lists[vertical ? 1 : 0].bytes + (size_t)index * 10;
    const Uint16 raw_id = Read16(record + 4), packed_slot = Read16(record + 6);
    *out = (SotnActorPlacement){(Sint16)Read16(record),
                                (Sint16)Read16(record + 2),
                                raw_id & 0x3ff,
                                Read16(record + 8),
                                (Uint8)packed_slot,
                                (Uint8)(packed_slot >> 8),
                                (Uint8)((raw_id >> 10) & 7),
                                raw_id & 0xe000};
    return true;
}

void SotnActorLayout_Close(SotnActorLayout *layout)
{
    SDL_free(layout);
}
