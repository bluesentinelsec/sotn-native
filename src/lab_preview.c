#include "lab_preview.h"
#include <SDL3/SDL.h>
#include <sotn/presenter.h>
#include <sotn/resource.h>
#include <sotn/tilemap.h>

struct LabPreview
{
    SotnStage *stage;
    SotnStageRoom room;
    SotnResource *tiles, *common;
    SotnPresenter *presenter;
    Uint16 frame[SOTN_FRAME_PIXELS];
    Uint8 rgba[SOTN_FRAME_PIXELS * 4];
    Uint32 room_index;
    Sint32 scroll_x, scroll_y;
    bool dirty;
};

static bool SelectRoom(LabPreview *preview, int direction)
{
    const Uint32 count = SotnStage_RoomCount(preview->stage);
    for (Uint32 i = 0; i < count; ++i)
    {
        if (direction < 0)
            preview->room_index = preview->room_index == 0 ? count - 1 : preview->room_index - 1;
        else if (direction > 0)
            preview->room_index = (preview->room_index + 1) % count;
        if (!SotnStage_GetRoom(preview->stage, preview->room_index, &preview->room))
            return false;
        if (!preview->room.transition)
        {
            preview->scroll_x = 0;
            preview->scroll_y = 0;
            preview->dirty = true;
            SDL_Log("Laboratory tile preview: room %u; entities and gameplay are not running",
                    (unsigned int)preview->room_index);
            return true;
        }
        direction = direction == 0 ? 1 : direction;
    }
    return SDL_SetError("Laboratory has no non-transition rooms to preview");
}

bool LabPreview_Create(const SotnGameData *data, const char *directory, LabPreview **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    LabPreview *preview = SDL_calloc(1, sizeof(*preview));
    if (preview == NULL)
        return false;
    SotnDataView stage_image = {0};
    if (!SotnGameData_GetFile(data, SOTN_GAME_NZ0, &stage_image) ||
        !SotnStage_Open(stage_image, &preview->stage) ||
        !SotnResource_Open(directory, SOTN_RESOURCE_LAB_GRAPHICS, &preview->tiles) ||
        !SotnResource_Open(directory, SOTN_RESOURCE_COMMON_GRAPHICS, &preview->common) ||
        !SelectRoom(preview, 0) ||
        !SotnPresenter_Create("Laboratory tile preview - arrows pan, PgUp/PgDn select room", 1024,
                              960, &preview->presenter))
    {
        char detail[512] = {0};
        SDL_strlcpy(detail, SDL_GetError(), sizeof(detail));
        LabPreview_Destroy(preview);
        return SDL_SetError("Cannot create Laboratory tile preview: %s", detail);
    }
    *out = preview;
    return true;
}

bool LabPreview_Event(LabPreview *preview, const SDL_Event *event)
{
    if (event->type != SDL_EVENT_KEY_DOWN)
        return true;
    switch (event->key.key)
    {
    case SDLK_PAGEUP:
    case SDLK_PAGEDOWN:
        if (!event->key.repeat)
            return SelectRoom(preview, event->key.key == SDLK_PAGEUP ? -1 : 1);
        return true;
    case SDLK_LEFT:
        preview->scroll_x -= 16;
        break;
    case SDLK_RIGHT:
        preview->scroll_x += 16;
        break;
    case SDLK_UP:
        preview->scroll_y -= 16;
        break;
    case SDLK_DOWN:
        preview->scroll_y += 16;
        break;
    default:
        return true;
    }
    const Sint32 max_x = (Sint32)preview->room.foreground.collision.width * 16 - SOTN_FRAME_WIDTH;
    const Sint32 max_y = (Sint32)preview->room.foreground.collision.height * 16 - SOTN_FRAME_HEIGHT;
    preview->scroll_x = SDL_clamp(preview->scroll_x, 0, SDL_max(0, max_x));
    preview->scroll_y = SDL_clamp(preview->scroll_y, 0, SDL_max(0, max_y));
    preview->dirty = true;
    return true;
}

bool LabPreview_Draw(LabPreview *preview)
{
    if (preview->dirty)
    {
        SDL_memset(preview->frame, 0, sizeof(preview->frame));
        const SotnTileSources sources = {SotnResource_View(preview->tiles),
                                         SotnResource_View(preview->common)};
        if (!SotnTilemap_Draw(&preview->room.background, sources, preview->scroll_x,
                              preview->scroll_y, preview->frame, SOTN_FRAME_PIXELS) ||
            !SotnTilemap_Draw(&preview->room.foreground, sources, preview->scroll_x,
                              preview->scroll_y, preview->frame, SOTN_FRAME_PIXELS) ||
            !SotnFrame_ToRgba(preview->frame, SOTN_FRAME_PIXELS, preview->rgba,
                              sizeof(preview->rgba)) ||
            !SotnPresenter_SetImage(preview->presenter, preview->rgba, sizeof(preview->rgba),
                                    SOTN_FRAME_WIDTH, SOTN_FRAME_HEIGHT))
            return false;
        preview->dirty = false;
    }
    return SotnPresenter_Draw(preview->presenter);
}

void LabPreview_Destroy(LabPreview *preview)
{
    if (preview != NULL)
    {
        SotnPresenter_Destroy(preview->presenter);
        SotnResource_Close(preview->common);
        SotnResource_Close(preview->tiles);
        SotnStage_Close(preview->stage);
        SDL_free(preview);
    }
}
