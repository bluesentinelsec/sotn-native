#include <SDL3/SDL_error.h>
#include <sotn/stage.h>

#define STAGE_BASE 0x80180000U
#define STAGE_CAPACITY 0x80000U
#define MAX_ROOMS 256U

struct SotnStage
{
    SotnDataView image;
    size_t room_offset;
    Uint32 layer_address;
    Uint32 room_count;
};

static Uint16 Read16(const Uint8 *bytes)
{
    return (Uint16)(bytes[0] | (Uint16)bytes[1] << 8);
}

static Uint32 Read32(const Uint8 *bytes)
{
    return bytes[0] | (Uint32)bytes[1] << 8 | (Uint32)bytes[2] << 16 | (Uint32)bytes[3] << 24;
}

static bool Resolve(const SotnStage *stage, Uint32 address, size_t size, Uint32 alignment,
                    SotnDataView *out)
{
    if (address < STAGE_BASE || address % alignment != 0)
        return SDL_SetError("Stage data has an invalid or misaligned PS1 address");
    const size_t offset = address - STAGE_BASE;
    if (offset > stage->image.size || size > stage->image.size - offset)
        return SDL_SetError("Stage data pointer extends outside the supplied overlay");
    *out = (SotnDataView){stage->image.bytes + offset, size};
    return true;
}

static bool ReadLayer(const SotnStage *stage, Uint32 address, Uint32 minimum_definitions,
                      SotnStageLayer *out)
{
    SotnDataView record = {0};
    if (!Resolve(stage, address, 16, 4, &record))
        return false;
    const Uint32 tiles = Read32(record.bytes);
    if (tiles == 0)
    {
        *out = (SotnStageLayer){0};
        return true;
    }
    SotnStageLayer layer = {0};
    const Uint32 rect = Read32(record.bytes + 8);
    layer.left = rect & 63;
    layer.top = (rect >> 6) & 63;
    layer.right = (rect >> 12) & 63;
    layer.bottom = (rect >> 18) & 63;
    layer.parameters = (Uint8)(rect >> 24);
    if (layer.right < layer.left || layer.bottom < layer.top)
        return SDL_SetError("Stage layer has inverted room bounds");
    layer.collision.width = ((Uint32)layer.right - layer.left + 1) * 16;
    layer.collision.height = ((Uint32)layer.bottom - layer.top + 1) * 16;
    const size_t tile_count = (size_t)layer.collision.width * layer.collision.height;
    if (!Resolve(stage, tiles, tile_count * 2, 2, &layer.collision.tiles))
        return false;
    Uint16 max_id = 0;
    for (size_t i = 0; i < tile_count; ++i)
    {
        const Uint16 id = Read16(layer.collision.tiles.bytes + i * 2);
        if (id > max_id)
            max_id = id;
    }
    const size_t definitions = SDL_max((size_t)max_id + 1, (size_t)minimum_definitions);
    SotnDataView pointers = {0};
    if (!Resolve(stage, Read32(record.bytes + 4), 16, 4, &pointers) ||
        !Resolve(stage, Read32(pointers.bytes), definitions, 1, &layer.pages) ||
        !Resolve(stage, Read32(pointers.bytes + 4), definitions, 1, &layer.graphics) ||
        !Resolve(stage, Read32(pointers.bytes + 8), definitions, 1, &layer.palettes) ||
        !Resolve(stage, Read32(pointers.bytes + 12), definitions, 1, &layer.collision.types))
        return false;
    layer.priority = Read16(record.bytes + 12);
    layer.flags = Read16(record.bytes + 14);
    layer.present = true;
    *out = layer;
    return true;
}

bool SotnStage_GetRoomDefinitions(const SotnStage *stage, Uint32 index, Uint32 definitions,
                                  SotnStageRoom *out)
{
    if (stage == NULL || out == NULL || index >= stage->room_count || definitions > 4096)
        return SDL_SetError("Stage room index is outside the validated room table");
    const Uint8 *header = stage->image.bytes + stage->room_offset + (size_t)index * 8;
    SotnStageRoom room = {0};
    room.left = header[0];
    room.top = header[1];
    room.right = header[2];
    room.bottom = header[3];
    room.layout_id = header[4];
    room.tileset_id = header[5];
    room.object_graphics_id = header[6];
    room.object_layout_id = header[7];
    if (room.left > room.right || room.top > room.bottom || room.right >= 64 || room.bottom >= 64)
        return SDL_SetError("Stage room has invalid map bounds");
    room.transition = room.tileset_id == 255;
    if (!room.transition)
    {
        SotnDataView pair = {0};
        if (!Resolve(stage, stage->layer_address + (Uint32)room.layout_id * 8, 8, 4, &pair) ||
            !ReadLayer(stage, Read32(pair.bytes), definitions, &room.foreground) ||
            !ReadLayer(stage, Read32(pair.bytes + 4), 0, &room.background))
            return false;
        if (!room.foreground.present)
            return SDL_SetError("Stage room is missing its foreground tile layer");
    }
    *out = room;
    return true;
}

bool SotnStage_Open(SotnDataView image, SotnStage **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (image.bytes == NULL || image.size < 40 || image.size > STAGE_CAPACITY)
        return SDL_SetError("Stage overlay must contain its header and fit the PS1 stage region");
    SotnStage candidate = {0};
    candidate.image = image;
    SotnDataView first_room = {0};
    if (!Resolve(&candidate, Read32(image.bytes + 16), 1, 4, &first_room))
        return false;
    candidate.room_offset = (size_t)(first_room.bytes - image.bytes);
    candidate.layer_address = Read32(image.bytes + 32);
    for (Uint32 i = 0; i <= MAX_ROOMS; ++i)
    {
        const size_t position = candidate.room_offset + (size_t)i * 8;
        if (position >= image.size)
            return SDL_SetError("Stage room table ended without its terminator");
        if (image.bytes[position] == 64)
        {
            if (i == 0)
                return SDL_SetError("Stage room table is empty");
            candidate.room_count = i;
            break;
        }
        if (i == MAX_ROOMS || image.size - position < 8)
            return SDL_SetError("Stage room table is oversized or truncated");
    }
    for (Uint32 i = 0; i < candidate.room_count; ++i)
    {
        SotnStageRoom room = {0};
        if (!SotnStage_GetRoom(&candidate, i, &room))
        {
            char detail[256] = {0};
            SDL_strlcpy(detail, SDL_GetError(), sizeof(detail));
            return SDL_SetError("Cannot decode stage room %u: %s", (unsigned int)i, detail);
        }
    }
    SotnStage *stage = SDL_malloc(sizeof(*stage));
    if (stage == NULL)
        return false;
    *stage = candidate;
    *out = stage;
    return true;
}

Uint32 SotnStage_RoomCount(const SotnStage *stage)
{
    return stage == NULL ? 0 : stage->room_count;
}

void SotnStage_Close(SotnStage *stage)
{
    SDL_free(stage);
}

bool SotnStage_GetRoom(const SotnStage *stage, Uint32 index, SotnStageRoom *out)
{
    return SotnStage_GetRoomDefinitions(stage, index, 0, out);
}
